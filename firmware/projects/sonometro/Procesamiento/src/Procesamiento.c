/*
 * Proc.c

 *
 *  Created on: Oct 20, 2018
 *      Author: chaca
 */


#include "board.h"
#include "arm_math.h"
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "main.h"

extern xSemaphoreHandle SemMenu;
int fResultad = 0;
/*==================[internal data declaration]==============================*/

typedef struct{
		uint32_t intResult;
		uint32_t commaResult;
		uint32_t flagOpen;
		uint32_t flagClose;
	} data;



uint32_t N = 20000;	//Numero de muestras a tomar

/*Paso N como parametro en una tarea.
 * Tuve que ponerla como global o no funcionaba
 * (asi esta en la documentacion de amazon)
 */


///*Cálculo de Moving RMS*/


extern xQueueHandle SampleQ, ResultQ;
extern xSemaphoreHandle SemI2S;
float acum = 0, acumNeg = 0;
float muestra = 0, AverageRMS = 0;
uint32_t close = 0;
xQueueHandle xResult, sdQ;
//extern uint32_t sd_init = 0;

//q31_t IntToQ31 (int mag)
//{
//	uint32_t QShift= 0x100;	//2^8
//
//	return (q31_t) mag*QShift;
//}

/*N representa la cantidad de muestras
 * equivalentes al tiempo durante el cual
 * se tomará informacion.
 * N = tiempo/Ts
 */

void ProcessingTask (void *pvN)	//Pasarle N como parametro solamente. Descartar la primer medicion
{
	int actualSample = 0;
	float actualSamplef = 0;
	static int i = 0;
	static float Average = 0;
	static uint32_t n = 0;

	float N = 0;
	N = *((uint32_t *)pvN);

	float SquaredMagnitude = 0;






	while(1)
	{

		xQueueReceive(SampleQ, &actualSample, portMAX_DELAY);	//Recibo el dato

		actualSamplef = (float) (actualSample >> 8);

		/*Desplazo 8 bits a la derecha, ya que usamos I2S de 32 bits,
		 * el ADC es de 24 bits, y el protocolo rellena el resto
		 * con ceros.
		 */

//		if(actualSamplef > muestra && actualSamplef > 0)	//Codigo para calibrar el ADC
//		{
//			muestra = actualSamplef;
//
//		}
//		if(actualSamplef == 0x7fffff)
//		{
//			muestra = actualSamplef;
//		}

		actualSamplef /= (pow(2,24) - 1);
		actualSamplef *= 3;
		actualSamplef *= 0.99833; //delta de ganancia
//		a7ctualSamplef -= 2.88e-3;
//
//		if(actualSamplef >= 0)	//codigo para verificar el funcionamiento del ADC
//			actualSamplef -= 2.88e-3;
//		if(actualSamplef < 0)
//			actualSamplef += 2.88e-3;


//		actualSamplef *= 0.845;
		/*Conversion a Volts*
		 *
		 */


		//Usamos dos modos:
		//En el primero, calculamos el valor rms de la señal
		//y lo pasamos a Pa y luego a dB SPL.
		//En el otro, moving average.

//		if(actualSamplef > acum && actualSamplef > 0)	//codigo para verificar el funcionamiento del ADC
//			acum = actualSamplef;
//		if(actualSamplef < acumNeg && actualSamplef < 0)
//			acumNeg = actualSamplef;

		SquaredMagnitude = powf(actualSamplef, 2);
		Average += SquaredMagnitude/(float)N;
//		AverageRMS = sqrtf(Average);

		/*Hacemos un promedio de la señal al cuadrado
		 * dividido el numero de muestras, para calcular un valor
		 * RMS, y luego pasarlo a presion mediante la sensibilidad especificada
		 * en el datasheet del microfono.
		 */

		n++;

		if(n == N)	//Si leí todas las muestras -> Muestro el resultado
		{
			n = 0;
			i++;
			if(i == 199)	//Agregamos codigo para probar la SD y el display
			{
				close = 1;

			}

			xQueueSendToBack(ResultQ, &Average, portMAX_DELAY);	//Mando el promedio a la task que calcula el resultado
			Average = 0;


		}


	}

}



float fTaskResultado (void* pvParameters)
{

	float fResult = 0, ResultSqrd = 0;
	int fResultComma = 0;
	int fResultInt = 0;
	pantalla_t pantalla;
	uint32_t i = 0;

	data sdData;
	data *pvData;

	sdData.flagOpen = 0;
	sdData.flagClose = 0;


	while(1)
	{
		xQueueReceive(ResultQ, &ResultSqrd, portMAX_DELAY);

		fResult = 10*log10f(ResultSqrd*((1e12)/400)*pow(1000/(7.8529*6.309), 2));	//7.174, 6.309 la sensibilidad5.656854

		/*Dividimos por la presion de referencia al cuadrado, y por
		 * la sensibilidad del microfono y la ganancia del preamplificador,
		 * ambas al cuadrado
		 */


		fResultComma = (int)((fResult - (int)fResult)*10);	//1 decimal
		if(fResultComma < 0)
			fResultComma *= -1;

		fResultInt = (int)fResult;
//		fResultad= (int)fResult;

//		fResultInt = 105;	//Hardcodeado para probar la SD cuando el adc no andaba
//		fResultComma = 7;

		pantalla.medicion_coma= fResultComma;	//Guardamos la medicion para mostrarla en el display y escribirla en la SD
		pantalla.medicion_entero = fResultInt;
		sdData.intResult = fResultInt;
		sdData.commaResult = fResultComma;

		if(close)	//Si terminamos con la medicion actual y queremos cerrar el archivo en la SD
		{
			sdData.flagClose = 1;
		}

		/*Agregar uno para abrir?
		 * tengo que mirar el algoritmo
		 */

		pvData = &sdData;

		xQueueSend (xResult, &pantalla,portMAX_DELAY);	//mandamos la medicion al display

//		if(sd_init)
		xQueueSend(sdQ,&pvData, portMAX_DELAY);	//Mandamos un puntero con la medicion para escribir en la SD


	}
}



void initTaskDSP ()
{



	xTaskCreate(fTaskResultado, (const signed char *)"Result", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+2, 0);
	xTaskCreate(ProcessingTask, (const signed char *)"Proc", configMINIMAL_STACK_SIZE,(void *) &N, tskIDLE_PRIORITY+1, 0);


	SampleQ = xQueueCreate(1000, sizeof(float));	//El numero de datos queda a definir, por ahora 10
	ResultQ = xQueueCreate(1, sizeof(float));
	xResult = xQueueCreate(1, sizeof(pantalla_t));
	sdQ = xQueueCreate(4, sizeof(data *));


}


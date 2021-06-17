/*
 * adc.c
 *
 *  Created on: 15 feb. 2019
 *      Author: User
 */

#include "I2S.h"

#include "board.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "main.h"

typedef struct{
	uint16_t datos;
	uint8_t canal;
} bat_data;

xQueueHandle batDisplayQ, adcQ;

void adcInit(void);



void adcInit(void)
{
	ADC_CLOCK_SETUP_T adc;

	Chip_ADC_Init(LPC_ADC, &adc);
#ifdef lpc4337_m4
	Chip_ADC_SetSampleRate(LPC_ADC, &adc, 88000);
#else
	Chip_ADC_SetSampleRate(LPC_ADC, &adc, 22000);
	/* P0.2 -> AD7
	 * P0.3 -> AD6
	 * P1.31 -> AD5
	 * */
	Chip_IOCON_PinMux(LPC_IOCON, 0, 3, IOCON_MODE_INACT, IOCON_FUNC1);
	Chip_IOCON_PinMux(LPC_IOCON, 0, 2, IOCON_MODE_INACT, IOCON_FUNC1);
	Chip_IOCON_PinMux(LPC_IOCON, 1, 31, IOCON_MODE_INACT, IOCON_FUNC1);
#endif

	Chip_ADC_EnableChannel(LPC_ADC, ADC_CH5, ENABLE);
	Chip_ADC_Int_SetChannelCmd(LPC_ADC, ADC_CH5, ENABLE);
	Chip_ADC_EnableChannel(LPC_ADC, ADC_CH6, ENABLE);
	Chip_ADC_Int_SetChannelCmd(LPC_ADC, ADC_CH6, ENABLE);
	Chip_ADC_EnableChannel(LPC_ADC, ADC_CH7, ENABLE);
	Chip_ADC_Int_SetChannelCmd(LPC_ADC, ADC_CH7, ENABLE);
	Chip_ADC_SetBurstCmd(LPC_ADC, ENABLE);

	NVIC_SetPriority(ADC_IRQn, 25);
	NVIC_EnableIRQ(ADC_IRQn);
}

void vADCTask()
{
	bat_data info;
	info.datos = 0;
	info.canal = 0;
	static float batUno = 0, batDos = 0, batTres = 0;
	float promedio = 0;
	uint32_t batTotal = 100;

	while(1)
	{
		xQueueReceive(adcQ, &info, portMAX_DELAY);

		info.datos = ((float)info.datos*3.3/(pow(2,12) - 1));

		/*Las baterias son de 3,7V, y en 3V se consideran
		 * descargadas.
		 * Promediamos la tension de las 3, y sacamos un
		 * porcentaje de bateria total
		 */

		if(info.canal == 6)
			batTres = info.datos;	//Asigno el dato a la bateria que corresponda segun el canal

		if(info.canal == 7)
			batDos = info.datos;

		if(info.canal == 5)
			batUno = info.datos;


		promedio = (batUno + batDos + batTres)/3;
		batTotal = (promedio - 3)*100/(0.7*3.7);	//Asumo que la capacidad es lineal con la tension (calculo porcentual)

//		xQueueSend(batDisplayQ, &batTotal, portMAX_DELAY);	//Envio el dato para actualizar el display

	}
}

void ADC_IRQHandler()	//Envia los datos a una tarea para que actualice el display (bateria)
{
	bat_data info;
	info.datos = 0;
	info.canal = 0;
	static uint32_t conversiones = 0;
	static portBASE_TYPE TaskSwitchRequired = 0;

	conversiones++;

	if(conversiones == 3)
	{
		Chip_ADC_SetBurstCmd(LPC_ADC, DISABLE);
		conversiones = 0;
	}

//	xSemaphoreGiveFromISR(semADC, &TaskSwitchRequired);
//	portEND_SWITCHING_ISR(TaskSwitchRequired);


	//Dejo de muestrear hasta que pase otro minuto



	info.canal = ((LPC_ADC -> GDR)>>24) & 0x07;
	//Cada canal corresponde a una bateria diferente
	Chip_ADC_ReadValue(LPC_ADC, info.canal, &info.datos);



	if(info.canal == 6)
	{
		Chip_ADC_ReadValue(LPC_ADC, 5, &info.datos);
		Chip_ADC_ReadValue(LPC_ADC, 7, &info.datos);
		xQueueSendFromISR(adcQ, &info, &TaskSwitchRequired);
		portEND_SWITCHING_ISR(TaskSwitchRequired);

	}
	if(info.canal == 7)
	{
		Chip_ADC_ReadValue(LPC_ADC, 5, &info.datos);
		Chip_ADC_ReadValue(LPC_ADC, 6, &info.datos);
		xQueueSendFromISR(adcQ, &info, &TaskSwitchRequired);
		portEND_SWITCHING_ISR(TaskSwitchRequired);

	}
	if(info.canal == 5)
	{
		Chip_ADC_ReadValue(LPC_ADC, 6, &info.datos);
		Chip_ADC_ReadValue(LPC_ADC, 8, &info.datos);
		xQueueSendFromISR(adcQ, &info, &TaskSwitchRequired);
		portEND_SWITCHING_ISR(TaskSwitchRequired);

	}



}

void taskADCInit()
{
	xTaskCreate(vADCTask, (const signed char *)"ADC_LPC", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
//	vSemaphoreCreateBinary(semADC);
////	xSemaphoreTake(semADC, 0);
	adcQ = xQueueCreate(6, sizeof(bat_data));
	batDisplayQ = xQueueCreate(1, sizeof(uint32_t));
}

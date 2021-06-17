/* Copyright 2015, Pablo Ridolfi
 * All rights reserved.
 *
 * This file is part of lpc1769_template.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 */

/** @brief Blinky using FreeRTOS.
 *
 *
 * NOTE: It's interesting to check behavior differences between standard and
 * tickless mode. Set @ref configUSE_TICKLESS_IDLE to 1, increment a counter
 * in @ref vApplicationTickHook and print the counter value every second
 * inside a task. In standard mode the counter will have a value around 1000.
 * In tickless mode, it will be around 25.
 *
 */

/** \addtogroup rtos_blink FreeRTOS blink example
 ** @{ */

/*==================[inclusions]=============================================*/

#include "I2S.h"

#include "board.h"
#include "FreeRTOSConfig.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "main.h"


/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/

/*==================[internal functions declaration]=========================*/

/** @brief hardware initialization function
 *	@return none
 */




uint32_t ok=0;
extern volatile xQueueHandle SampleQ;
/*extern*/ xSemaphoreHandle SemI2S;

void Init_I2S()
{
	I2S_AUDIO_FORMAT_T I2S_configuracion; //Podría declarar una estructura y pasar el argumento a Chip_I2S_RxConfig(..,&I2S_configuracioN)
	I2S_configuracion.ChannelNumber=1; //Elijo mono
	I2S_configuracion.SampleRate=44100; //Elijo 44.1Khz
	I2S_configuracion.WordWidth=32; //Elijo 32 bits de palabra




	Chip_I2S_Init(LPC_I2S); //Activo PCONP



	Chip_Clock_SetPCLKDiv(SYSCTL_PCLK_I2S, 1);//1  //En teoría esto configura el PCLK de I2S para igualarlo al CCLK
	/*PINSEL para pines de I2S*/

	Chip_IOCON_PinMux(LPC_IOCON, 0 , 23, IOCON_MODE_INACT, IOCON_FUNC2 ); //Configuro P0.23 como I2SRX_CLK
	Chip_IOCON_PinMux(LPC_IOCON, 0 , 24, IOCON_MODE_INACT, IOCON_FUNC2 ); //Configuro P0.24 como I2SRX_WS
	Chip_IOCON_PinMux(LPC_IOCON, 0 , 25, IOCON_MODE_INACT, IOCON_FUNC2 ); //Configuro P0.25 como I2SRX_SDA
	Chip_IOCON_PinMux(LPC_IOCON, 4 , 28, IOCON_MODE_INACT, IOCON_FUNC1 ); //Configuro P4.28 como RX_MCLK

	//NVIC_EnableIRQ(I2S_IRQn); //Dejarlo en esta linea o no funciona.

	/*Acá inicializaría el DMA si utilizase*/

	//Configuración de la recepción
	/*----------------------------------------------------------------------------------------*/
	Chip_I2S_RxConfig(LPC_I2S, &I2S_configuracion);
	LPC_I2S -> DAI |= 0x10;	//WS Master

	//Elijo divisores x(14) e y(31)
	Chip_I2S_SetRxXYDivider(LPC_I2S, 4, 17);	//4 y 17
	//Elijo el divisor para lograr bitrate deseado
	Chip_I2S_SetRxBitRate(LPC_I2S,3); // 7
	//Elijo la señal BCLK del transmisor, ~habilito 4-Pin comunication, habilito RX_MCLK
	Chip_I2S_RxModeConfig(LPC_I2S, I2S_RXMODE_CLKSEL(0), 0, I2S_RXMODE_MCENA);
	/*------------------------------------------------------------------------------------------*/
	//Chip_I2S_RxStop(LPC_I2S);
	NVIC_SetPriority(27, 21);
	Chip_I2S_Int_RxCmd(LPC_I2S,ENABLE,1); //Configuro para habilitar la IRQ (ENABLE) y una depth, profundidad de 8 bits para que interrumpa la FIFO. Máximo de 15 (1111)


	//Configuración de la transmisión
	//Chip_I2S_RxStop(LPC_I2S);
	Chip_I2S_RxStart(LPC_I2S); //Toggle por si las moscas

}

//void vTaskI2S (void* pvParameters)
//{
//	uint32_t Buffer = 0;
////	static uint32_t superBuffer [1000], i = 0;
//
//	while(1)
//	{
//
//		xSemaphoreTake(SemI2S, portMAX_DELAY);
//		//Buffer=Chip_I2S_Receive(LPC_I2S);	//Aca podriamos agregar un while para limpiar todo el buffer. Ver que devuelve si esta vacio
////		superBuffer [i] = Buffer;
////		xQueueSendToBack(SampleQ, &Buffer, portMAX_DELAY);
////		i++;
//	}
//}

void I2S_IRQHandler(void){

	int Buffer = 0;
	uint32_t i, rx_lvl;


	static portBASE_TYPE TaskSwitchRequired = pdFALSE;
	rx_lvl = Chip_I2S_GetRxLevel(LPC_I2S);

	for(i = 0; i < rx_lvl; i++)
	{
		Buffer=Chip_I2S_Receive(LPC_I2S);
		xQueueSendToBackFromISR(SampleQ, &Buffer, &TaskSwitchRequired);	//Me parecio mas rapido que usar un sem y una queue

	}






//	xSemaphoreGiveFromISR(SemI2S, &TaskSwitchRequired);
	portEND_SWITCHING_ISR(TaskSwitchRequired);


}

/*==================[internal data definition]===============================*/

/*==================[external data definition]===============================*/

/*==================[internal functions definition]==========================*/



/*static void task(void * a){
	uint8_t anda=0;
	while(1){
		Chip_I2S_Send(LPC_I2S,0xAAAAAAAA);
		if(xSemaphoreTake(sem1,portMAX_DELAY)==pdTRUE){
			//Buffer=Chip_I2S_Receive(LPC_I2S);
			anda=1;
		}

	}
}
*/





////void initTaskI2S(void)
//{
//
//	xTaskCreate(vTaskI2S, (const signed char *)"vTaskI2S", configMINIMAL_STACK_SIZE*4, 0, tskIDLE_PRIORITY+3, 0);
//	vSemaphoreCreateBinary(SemI2S);
//	xSemaphoreTake (SemI2S, 0);
//
//}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/

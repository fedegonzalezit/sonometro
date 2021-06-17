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

#include "main.h"
#include "statechart.h"
#include "Procesamiento.h"
#include "arm_math.h"
#include "sd_spi.h"

/*==================[macros and definitions]=================================*/

/*==================[internal data declaration]==============================*/
xQueueHandle SampleQ, ResultQ, I2SQ;
xSemaphoreHandle SemResult, SemProcessing,SemI2S;
uint32_t sd_init = 0;




/*==================[internal functions declaration]=========================*/


static void initHardware(void)
{
    SystemCoreClockUpdate();

    Chip_GPIO_WritePortBit(LPC_GPIO, LED_ROJO, 0);
    Chip_GPIO_WritePortBit(LPC_GPIO, LED_VERDE, 0);



}

void GPIOInterrupcionesInit (void)
{
	Chip_GPIOINT_Init(LPC_GPIOINT);

	//Setea pines de teclado para interrupciones por flanco ascendente

	//switch del proyecto
	Chip_GPIOINT_SetIntFalling(LPC_GPIOINT, GPIOINT_PORT0, (1<<10 | 1<<30 | 1<<29 | 1<<4 | 1<<5) );

	Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT, GPIOINT_PORT0, (1<<29 | 1<<10 | 1 << 4));
//	Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT, GPIOINT_PORT0, 1<<10);
//	Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT, GPIOINT_PORT0, 1<<4);

//	Chip_GPIOINT_

//	//Switchs de placa info
//	Chip_GPIOINT_SetIntFalling(LPC_GPIOINT, GPIOINT_PORT0, 1<<10);
//	Chip_GPIOINT_SetIntFalling(LPC_GPIOINT, GPIOINT_PORT0, 1<<13);
//	Chip_GPIOINT_SetIntFalling(LPC_GPIOINT, GPIOINT_PORT0, 1<<18);
//	Chip_GPIOINT_SetIntFalling(LPC_GPIOINT, GPIOINT_PORT0, 1<<11);




    NVIC_EnableIRQ(EINT3_IRQn);



}



int rtc_initialize (void);
/*==================[external functions definition]==========================*/

int main(void)
{

	uint32_t error = 0, status = 0;

	initHardware();


 	rtcTaskInit();
 	initTaskDSP();
 	rtc_initialize();
	Init_I2S();


//	adcInit();
//	taskADCInit();
	initDisplay();



	/*Inicializacion SD*/
//


	spiInitTask();
//	spiInit();


	/*Inicializacion SD*/

	status = disk_initialize(0);	//Ver (Inicializacion SD) (0)
	if(status == STA_NOINIT || status == STA_NODISK)
	{
		error = 1;
	}
//	else
//	{
//		sd_init = 1;
//		spiInitTask();
//		Chip_SSP_SetBitRate(LPC_SSP0, 12500000);	//12.5 Mhz
//	}

	Chip_SSP_SetBitRate(LPC_SSP0, 12500000);	//12.5 Mhz
	/*******************************/

	NVIC_EnableIRQ(I2S_IRQn); //Dejarlo en esta linea o no funciona.

	vTaskStartScheduler();

	while (1) {
	}
}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/

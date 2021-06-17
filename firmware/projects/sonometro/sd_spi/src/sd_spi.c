/* Copyright 2014, ChaN
 * Copyright 2016, Matias Marando
 * All rights reserved.
 *
 * This file is part of Workspace.
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

/** @brief This is an SD card write example using the FatFs library
 */

/** \addtogroup spi SD card write via SPI
 ** @{ */

/*==================[inclusions]=============================================*/

#include "board.h"
#include "ff.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "sd_spi.h"
#include "rtc.h"

/*==================[macros and definitions]=================================*/

#define FILENAME "newfile.txt"


typedef struct{
		uint32_t intResult;
		uint32_t commaResult;
		uint32_t flagOpen;
		uint32_t flagClose;
	} data;


/*==================[internal data declaration]==============================*/

static uint32_t ms_ticks;  /**< 1ms timeticks counter */
static FATFS fs;           /**< FatFs work area needed for each volume */
static FIL fp;             /**< File object needed for each open file */
uint32_t flagOpen = 0;
extern xQueueHandle sdQ;
extern uint8_t SwitchTick;

/*==================[internal functions declaration]=========================*/



/** @brief Hardware initialization function
 *  @return none
 */

/*==================[internal functions definition]==========================*/

static void spiInit(void)
{
	Board_Init();
    //SystemCoreClockUpdate();

    /* SPI configuration */
    Board_SSP_Init(LPC_SSP0);
    Chip_SSP_Init(LPC_SSP0);
    Chip_SSP_SetBitRate(LPC_SSP0, 400000);	//400 k

    Chip_SSP_Enable(LPC_SSP0);
}

/*==================[external functions definition]==========================*/

void vApplicationTickHook (void)
{
	SwitchTick++;


	static uint32_t delay = 0;
    ms_ticks++;

    if (ms_ticks >= 10) {
        ms_ticks = 0;
        disk_timerproc();   /* Disk timer process */
    }


    delay++;

//	if(delay == 60000)	//Paso 1 minuto
//	{
//		delay = 0;
//		Chip_ADC_SetBurstCmd(LPC_ADC, ENABLE);
//		//Activo la conversion del adc
//	}
}

typedef struct {
	WORD	year;	/* 1..4095 */
	BYTE	month;	/* 1..12 */
	BYTE	mday;	/* 1.. 31 */
	BYTE	wday;	/* 1..7 */
	BYTE	hour;	/* 0..23 */
	BYTE	min;	/* 0..59 */
	BYTE	sec;	/* 0..59 */
} RTC;

typedef struct {
	BYTE hour;
	BYTE min;
	BYTE sec;
	uint32_t med_entero;
	uint32_t med_coma;
	char eof;
} log;

void vSPITask(void *a)
{
    UINT nbytes;
    data sdData ;
    data *pvData = &sdData;
    char str[4];
    char error = 0;
    RTC time;
    log mediciones;
    uint32_t br;

    mediciones.eof = 0;


    /* Give a work area to the default drive */
    if (f_mount(&fs, "", 0) != FR_OK) {	//0 o 1?
        /* If this fails, it means that the function could
         * not register a file system object.
         * Check whether the SD card is correctly connected */
    	error = 1;
    }

    /* Create/open a file, then write a string and close it */

    while (1) {

    	xQueueReceive(sdQ, &pvData, portMAX_DELAY);

    	if(!(pvData -> flagOpen))	//Abro o creo (si no existe) el archivo
    	{
    		if (f_open(&fp, FILENAME, FA_WRITE | FA_OPEN_ALWAYS) == FR_OK)
    			Board_LED_Toggle(3);	//Blinkear un led o algo para indicar que se pudo abrir

    		pvData -> flagOpen = 1;

    	}

    	rtc_gettime(&time);	//Pido la hora para hacer un log con las mediciones

    	/*Guardo todo en una estructura para escribir
    	 * un archivo binario.
    	 * Despues lo levanto con un script en python o c.
    	 */

    	mediciones.hour = time.hour;
		mediciones.min = time.min;
		mediciones.sec = time.sec;
		mediciones.med_entero = pvData -> intResult;
		mediciones.med_coma = pvData -> commaResult;

		f_write(&fp, &mediciones, sizeof(mediciones), &br);



//    	sprintf(str, "%u", time.hour);
//    	f_puts(str, &fp);
//
//    	f_puts(":", &fp);
//
//    	sprintf(str, "%u", time.min);
//    	f_puts(str, &fp);
//    	f_puts(":", &fp);
//
//    	sprintf(str, "%u", time.sec);
//    	f_puts(str, &fp);
//    	f_puts("\t", &fp);

    	/*Escribo la medicion en la SD.
    	 * Se puede optar por sacar la hora y exportar los datos en formato CSV
    	 */

//    	sprintf(str,"%u",pvData -> intResult); //converts to decimal base
//
//    	f_puts(str, &fp);	//escribe el string al archivo
//
//    	f_puts(".", &fp);	//Coma
//
//    	sprintf(str, "%u", pvData -> commaResult);
//
//    	f_puts(str, &fp);
//
//    	f_puts("\n", &fp);


    	f_sync(&fp); //Evita errores por no cerrar el archivo

    	if(pvData -> flagClose == 1)	//Cierro el archivo
    	{
    		mediciones.eof = 1;
    		f_write(&fp, &mediciones, sizeof(mediciones), &br);

    		f_close(&fp);

    		pvData -> flagClose = 0;


    	}

	}

}


void spiInitTask (void)
{
	xTaskCreate(vSPITask, (const signed char *)"vSPITask", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY + 1, 0);
	spiInit();


}

/** @} doxygen end group definition */

/*==================[end of file]============================================*/

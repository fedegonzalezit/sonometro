/*
 * main.c
 *
 *  Created on: Oct 15, 2018
 *      Author: chaca
 */

#include "board.h"
#define YMWDHM 0XCE
#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include "types.h"
#include "chip.h"
#include "board.h"
#include "rtc.h"
#include "integer.h"

#include "RTC.h"

typedef struct {
	WORD	year;	/* 1..4095 */
	BYTE	month;	/* 1..12 */
	BYTE	mday;	/* 1.. 31 */
	BYTE	wday;	/* 1..7 */
	BYTE	hour;	/* 0..23 */
	BYTE	min;	/* 0..59 */
	BYTE	sec;	/* 0..59 */
} RTC;

/*==================[internal data declaration]==============================*/
xSemaphoreHandle SemRTC;
pantalla_t *fecha;
RTC_TIME_T RTC_time;
xQueueHandle DisplayQ;

void SetTime (RTC_TIME_T *RTC_time, pantalla_t *fecha)
{
	fecha -> anio = (uint16_t)RTC_time->time[RTC_TIMETYPE_YEAR];
	fecha -> dia = (uint8_t)RTC_time->time[RTC_TIMETYPE_DAYOFMONTH];
	fecha -> hora = (uint8_t)RTC_time->time[RTC_TIMETYPE_HOUR];
	fecha -> mes = (uint8_t)RTC_time->time[RTC_TIMETYPE_MONTH];
	fecha -> minuto = (uint8_t)RTC_time->time[RTC_TIMETYPE_MINUTE];
	fecha -> segundos = (uint8_t)RTC_time->time[RTC_TIMETYPE_SECOND];
}



void vRTCTask ()		//Despues agregar año y mes. Esta funcion envia la hora actual al display
{
	RTC time;
	pantalla_t fecha;

	while(1)
	{
		xSemaphoreTake(SemRTC, portMAX_DELAY);

		rtc_gettime(&time);
		fecha.hora = time.hour;
		fecha.minuto = time.min;
		fecha.segundos = time.sec;

		xQueueSend(DisplayQ, &fecha, portMAX_DELAY);	//aca convendria mandar un puntero

	}
}


void rtcTaskInit ()
{
	xTaskCreate(vRTCTask, (const signed char *)"vRTCTask", configMINIMAL_STACK_SIZE*4, 0, tskIDLE_PRIORITY+1, 0);

	vSemaphoreCreateBinary(SemRTC);

	xSemaphoreTake(SemRTC, 0);
	DisplayQ = xQueueCreate(1, sizeof(pantalla_t));
}

void INIT_RTC ()
{
	RTC_TIME_T set_time;


	set_time.time[RTC_TIMETYPE_SECOND] = 0;
	set_time.time[RTC_TIMETYPE_MINUTE] = 0;
	set_time.time[RTC_TIMETYPE_HOUR] = 0;
	set_time.time[RTC_TIMETYPE_MONTH] = 0;
	set_time.time[RTC_TIMETYPE_YEAR] = 0;
	set_time.time[RTC_TIMETYPE_DAYOFMONTH] = 0;
	set_time.time[RTC_TIMETYPE_DAYOFWEEK] = 0;
	set_time.time[RTC_TIMETYPE_DAYOFYEAR] = 0;
	set_time.time[RTC_TIMETYPE_LAST] = 0;


	Chip_RTC_Init(LPC_RTC);
	Chip_RTC_ResetClockTickCounter(LPC_RTC);
	Chip_RTC_CntIncrIntConfig(LPC_RTC, 0x01, ENABLE);	//Habilito interrupciones por contadores relevantes
	Chip_RTC_SetFullTime(LPC_RTC, &set_time);	//Esta línea se ejecuta UNA sola vez. Después comentarla

	NVIC_SetPriority(RTC_IRQn, 30);

	Chip_RTC_Enable(LPC_RTC, ENABLE);



}








void RTC_IRQHandler(void)
{
	static portBASE_TYPE xTaskSwitchRequired = pdFALSE;



	xSemaphoreGiveFromISR(SemRTC, &xTaskSwitchRequired);
	Chip_RTC_ClearIntPending(LPC_RTC, RTC_IRL_RTCCIF); //Limpia el flag de interrupción
	portEND_SWITCHING_ISR(xTaskSwitchRequired);

}

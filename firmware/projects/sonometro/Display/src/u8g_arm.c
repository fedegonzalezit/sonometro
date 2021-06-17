/*
 * display.c
 *
 *  Created on: Sep 12, 2018
 *      Author: federico
 */


/*

  u8g_arm.c


  u8g utility procedures for LPC122x

  Universal 8bit Graphics Library

  Copyright (c) 2013, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification,
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list
    of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice, this
    list of conditions and the following disclaimer in the documentation and/or other
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


  The following delay procedures must be implemented for u8glib. This is done in this file:

  void u8g_Delay(uint16_t val)		Delay by "val" milliseconds
  void u8g_MicroDelay(void)		Delay be one microsecond
  void u8g_10MicroDelay(void)	Delay by 10 microseconds

  Additional requirements:

      SysTick must be enabled, but SysTick IRQ is not required. Any LOAD values are fine,
      it is prefered to have at least 1ms
      Example:
        SysTick->LOAD = (SystemCoreClock/1000UL*(unsigned long)SYS_TICK_PERIOD_IN_MS) - 1;
	SysTick->VAL = 0;
	SysTick->CTRL = 7;   // enable, generate interrupt (SysTick_Handler), do not divide by 2
*/

#include "u8g_arm.h"

#include "ssp_17xx_40xx.h"
#include "clock_17xx_40xx.h"
#include "main.h"

/*========================================================================*/

uint8_t delay = OFF;
uint32_t usTicks;


//uint32_t SystemCoreClock = 100000000; //100MHZ
static uint8_t A0;
static uint8_t rs_last_state = 255;

//static uint8_t rs_last_state = 255;

//xSemaphoreHandle sem_1us, sem_10us;
/*========================================================================*/


/*========================================================================*/


/*========================================================================*/
void SSP1_Clock(uint32_t ns)	//clock siempre tiene que ser múltiplo de 5 (El parámetro es el valor de clock que queremos en Mhz)
{								//Sólo permite clocks entre 200 khz y 25 mhz.
	 /*
	    calculate CPSR
	    SystemCoreClock / CPSR = 100000000 / ns
	    CPSR = SystemCoreClock * ns / 1000000000
	    CPSR = (SystemCoreClock/10000) * ns / 100000
	  */
	  uint32_t cpsr;

	  cpsr = SystemCoreClock;
	  cpsr /= 10000UL;
	  cpsr *= ns;
	  cpsr += 100000UL  - 1UL;		/* round up */
	  cpsr /= 100000UL;
	  /* ensure that cpsr will be between 2 and 254 */
	  if ( cpsr == 0 )
	    cpsr = 1;
	  cpsr++;
	  cpsr &= 0x0feUL;
	  LPC_SSP1->CPSR = cpsr;

}

/*
  setup spi0
  ns is the clock cycle time between 0 and 20000 ns
*/
void spi_init (uint32_t ns) //clock de 0 a 3
{

//    /* SPI configuration */
    Board_SSP_Init(LPC_SSP1);
//      Chip_SSP_Init(LPC_SSP1);
//      Chip_SSP_Enable(LPC_SSP1);
	SSP1_Clock(800);

//	//Por defecto encendido
//		//Dejé el CPHA por defecto, igual que CPOL
		LPC_SYSCTL->PCONP  |= (1 << 21);    		/* Enable power to SSP1 block */
		LPC_SSP1->CR0 &= ~(0x0F);
		LPC_SSP1->CR0 |= 0x07; //Transferencia de 8 bits
		LPC_SSP1->CR0 &= (~(0x03 << 4)); //SPI Mode


		LPC_SYSCTL->PCLKSEL[0] &=  ~(0x03 << 20);
		LPC_SYSCTL->PCLKSEL[0] |= 0x02 << 20;		// CCLK/2


		SSP1_Clock(800);
		//SSP1CPSR = 24; //SPI A 1MHZ segun libreria marlin


		//PINSEL0 &= ((~(0x03)) << 12); //SSEL1, GPIO;
		//FIO0DIR |= (0x01 << 6);	//P0.6
		//SetPINSEL( SCK , PINSEL_FUNC2 );
		//SetPINSEL( MISO , PINSEL_FUNC2 );

		Chip_IOCON_PinMux(LPC_IOCON, 0, 7, IOCON_MODE_INACT, IOCON_FUNC2);
		Chip_IOCON_PinMux(LPC_IOCON, 0, 9, IOCON_MODE_INACT, IOCON_FUNC2);




		LPC_SSP1->CR1 &= ~(0x01 << 2); //Master mode
		LPC_SSP1->CR1 |= 0x02;		//SSP1 enable
		//Acá no se pone CS High. Lo hacemos en la inicialización


}



void spi_out ( uint8_t data)
{

//    Chip_SSP_DATA_SETUP_T xferConfig;
//
//	xferConfig.tx_data = data;
//	xferConfig.tx_cnt  = 0;
//	xferConfig.rx_data = NULL;
//	xferConfig.rx_cnt  = 0;
//	xferConfig.length  = 1;
//
//	Chip_SSP_RWFrames_Blocking(LPC_SSP1, &xferConfig);
	while(LPC_SSP1->SR & BSY);		//Espera mientras el SPI estÃ© ocupado/no tenga contenido para leer.
	LPC_SSP1->DR = (data & 0x00FF);

}

/*========================================================================*/
/*
  The following delay procedures must be implemented for u8glib

  void u8g_Delay(uint16_t val)		Delay by "val" milliseconds
  void u8g_MicroDelay(void)		Delay be one microsecond
  void u8g_10MicroDelay(void)	Delay by 10 microseconds

*/


void LCD_usDelay(uint32_t usec)
{
	Chip_TIMER_SetMatch(LPC_TIMER1, 0, usec);
	Chip_TIMER_Reset(LPC_TIMER1);
	Chip_TIMER_Enable(LPC_TIMER1);

	while (!usTicks) __WFI();
	usTicks = 0;

	Chip_TIMER_Disable(LPC_TIMER1);

}

void u8g_Delay(uint16_t val)
{
  vTaskDelay(val/portTICK_RATE_MS);
}

void u8g_MicroDelay(void)
{
    LCD_usDelay (1);
}

void u8g_10MicroDelay(void)
{
    LCD_usDelay (10);

}


/* Handler de interrupción del Timer 1 */
void TIMER1_IRQHandler(void)
{
	if (Chip_TIMER_MatchPending(LPC_TIMER1, 0)) {
		usTicks++;
	}
	Chip_TIMER_ClearMatch(LPC_TIMER1, 0);
}

/*========================================================================*/
/* u8glib com procedure */

/* gps board */
/*
uint16_t u8g_pin_a0 = PIN(0,11);
uint16_t u8g_pin_cs = PIN(0,6);
uint16_t u8g_pin_rst = PIN(0,5);
*/

/* eval board */
//uint16_t u8g_pin_a0 = PIN(0,0);
uint16_t u8g_pin_cs = PIN(0,6);
//uint16_t u8g_pin_rst = PIN(0,1);


uint8_t u8g_com_hw_spi_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr)
{
	switch(msg)
  {
    case U8G_COM_MSG_STOP:
      break;

    case U8G_COM_MSG_INIT:

       if ( arg_val <= U8G_SPI_CLK_CYCLE_50NS )
      {
	spi_init(50);
      }
      else if ( arg_val <= U8G_SPI_CLK_CYCLE_300NS )
      {
	spi_init(300);
      }
      else if ( arg_val <= U8G_SPI_CLK_CYCLE_400NS )
      {
	spi_init(400);
      }
      else
      {
	spi_init(1200);
      }


       Chip_IOCON_PinMux(LPC_IOCON, 0, 6, IOCON_MODE_PULLDOWN, IOCON_FUNC0);
       Chip_GPIO_SetPinDIROutput(LPC_GPIO, U8G_PIN_CS);

       Chip_GPIO_WritePortBit(LPC_GPIO, U8G_PIN_CS, 0);



      u8g_Delay(5);
      break;

    case U8G_COM_MSG_ADDRESS:                     /* define cmd (arg_val = 0) or data mode (arg_val = 1) */

      A0 = arg_val;


     break;

    case U8G_COM_MSG_CHIP_SELECT:
        Chip_GPIO_WritePortBit(LPC_GPIO, U8G_PIN_CS, arg_val);
      break;

    case U8G_COM_MSG_RESET:

      break;

    case U8G_COM_MSG_WRITE_BYTE:
      u8g_com_LPC1769_st7920_write_byte_hw_spi(A0, arg_val);
      break;

    case U8G_COM_MSG_WRITE_SEQ:
    {
        uint8_t *ptr = (uint8_t*) arg_ptr;
        while (arg_val > 0) {
          u8g_com_LPC1769_st7920_write_byte_hw_spi(A0, *ptr++);
          arg_val--;
        }
    }
    break;
    case U8G_COM_MSG_WRITE_SEQ_P:
      {
          uint8_t *ptr = (uint8_t*) arg_ptr;
          while (arg_val > 0) {
            u8g_com_LPC1769_st7920_write_byte_hw_spi(A0, *ptr++);
            arg_val--;


          }
      }
      break;
  }
  return 1;
}



void u8g_com_LPC1769_st7920_write_byte_hw_spi(uint8_t rs, uint8_t val)
{
  uint8_t i;
  uint8_t data;

  if ( rs != rs_last_state) {  // time to send a command/data byte
    rs_last_state = rs;

    if ( rs == 0 )
    {
      /* command */
    	data = 0x0F8;
    	spi_out(data);
    }
    else
    {
       /* data */
    	data = 0x0FA;
    	spi_out(data);
    }

    for( i = 0; i < 4; i++ )   // give the controller some time to process the data
      u8g_10MicroDelay();      // 2 is bad, 3 is OK, 4 is safe
  }

  data = val & 0x0f0;
  spi_out(data);
  data = val << 4;
  spi_out(data);
}


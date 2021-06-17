/*
 * u8g_arm.h
 *
 *  Created on: Sep 12, 2018
 *      Author: federico
 */

#ifndef PROJECTS_DISPLAY_INC_U8G_ARM_H_
#define PROJECTS_DISPLAY_INC_U8G_ARM_H_


#include "SPI_SD.h"
#include "u8g.h"
#include "main.h"

/* system helper procedures */
void init_system_clock(void);	/* optional: can be called from init code */

void delay_system_ticks(uint32_t sys_ticks);
void delay_micro_seconds(uint32_t us);


#define PIN(base,bit) ((base)*32+(bit))
#define DEFAULT_KEY PIN(0,12)



void spi_init(uint32_t ns) U8G_NOINLINE;
void spi_out(uint8_t data);

extern uint16_t u8g_pin_a0;
extern uint16_t u8g_pin_cs;
extern uint16_t u8g_pin_rst;


#define	U8G_PIN_A0	0,0
#define	U8G_PIN_RST	0,1
#define	U8G_PIN_CS	0,6
#define	SCK			0,7
#define	MISO		0,9

uint8_t u8g_com_hw_spi_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr);
void u8g_com_LPC1769_st7920_write_byte_hw_spi(uint8_t rs, uint8_t val);


#endif



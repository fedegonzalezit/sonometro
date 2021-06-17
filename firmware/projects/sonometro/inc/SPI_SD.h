/*
 * SPI_SD.h
 *
 *  Created on: Sep 12, 2018
 *      Author: federico
 */

#ifndef PROJECTS_DISPLAY_INC_SPI_SD_H_
#define PROJECTS_DISPLAY_INC_SPI_SD_H_

#include "main.h"



/**************************DEFINES*********************/
typedef unsigned char    SD_BOOL;

#define MISO1	FIO0PIN >> 8
#define CMD0	0
#define CMD8	8
#define HIGH 1
#define LOW 0
#define ACMD41 41
//#define NULL 0
#define SSP1CR0 (*((__RW uint32_t*) 0x40030000UL))			//Control registers
#define SSP1CR1 (*((__RW uint32_t*) 0x40030004UL))
#define SSP1CPSR (*((__RW uint32_t*) 0x40030010UL))			//Prescaler
#define SSP1DR		(*((__RW uint32_t*) 0x40030008UL))		//Data register (Sólo se usan los primeros 16 bits)
#define SSP1SR		(*((__RW uint32_t*) 0x4003000CUL))   	//Status register
#define CMD55 55
#define MISO1	FIO0PIN >> 8
#define SPI_CLOCK_HIGH_RATE 12.5
#define SPI_CLOCK_LOW_RATE 0.25
#define SD_TRUE     1
#define SD_FALSE    0
#define BSY			(0x01 << 4)
#define RNE			(0x01 << 2)
#define TFE				(0x01)
//#define	BUFFER_SIZE	512


/*********************PROTOTIPOS***********************/

uint8_t Command_SD (uint8_t cmd, uint32_t arg, uint8_t *buf, uint8_t len);
uint8_t AppCommand_SD (uint8_t cmd, uint32_t arg, uint8_t *buf, uint32_t len);
uint8_t SPI_Write ( uint8_t data);
void CS_Control (uint8_t actividad);
void Init_SSP1 (void);
void SSP1_Clock(uint32_t ns);
void SPI_WriteByte ( uint8_t data);
SD_BOOL SD_WaitForReady (void);



#endif /* PROJECTS_DISPLAY_INC_SPI_SD_H_ */

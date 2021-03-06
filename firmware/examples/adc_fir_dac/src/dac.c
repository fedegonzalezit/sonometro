/*
 * dac.c
 *
 *  Created on: Sep 4, 2013
 *      Author: Pablo
 */


#include "board.h"

/* P0.26 -> AOUT */
void dacInit(void)
{
	Chip_DAC_Init(LPC_DAC);
#ifdef lpc4337_m4
	Chip_DAC_ConfigDAConverterControl(LPC_DAC, DAC_DMA_ENA);
#else
	Chip_IOCON_PinMux(LPC_IOCON, 0, 26, IOCON_MODE_INACT, IOCON_FUNC2);
#endif
}

void dacWrite(uint32_t v)
{
	Chip_DAC_UpdateValue(LPC_DAC, v);
}

/*
 * types.h
 *
 *  Created on: Oct 26, 2018
 *      Author: federico
 */

#ifndef PROJECTS_DECIBELIMETRO_V0_0_INC_TYPES_H_
#define PROJECTS_DECIBELIMETRO_V0_0_INC_TYPES_H_


#include "board.h"

#define		LPEAK		1
#define		LEQ			2

#define 	MENU_ITEMS_MAX		7


#define	ON			1
#define	OFF			0

#define LED_ROJO 	2,5
#define LED_VERDE	2,6

/*
 * Se definen el indice en donde va a estar cada valor para la configuracion
 */
#define		MODO_MODO_INDEX				0
#define		MODO_TIEMPO_INDEX			1
#define		INTENSIDAD_INDEX			0
#define		HORARIO_ANIO_INDEX			0
#define		HORARIO_MES_INDEX			1
#define		HORARIO_DIA_INDEX			2
#define		HORARIO_HORA_INDEX			3
#define		HORARIO_MINUTO_INDEX		4
#define		HORARIO_SEG_INDEX			5
#define		ALARMA_STATUS_INDEX			0
#define		ALARMA_ENCIMADE_INDEX		2
#define		ALARMA_DEBAJODE_INDEX		1






/*
 * Estructura en la que se cargan los valores con los que se actualiza la pantalla
 */
typedef struct
{
uint8_t medicion_entero;
uint8_t medicion_coma;
uint8_t modo;
uint16_t anio;
uint8_t mes;
uint8_t dia;
uint8_t hora;
uint8_t minuto;
uint8_t segundos;
uint8_t bateria;

}pantalla_t;


typedef struct
{
	uint8_t items;
	const char *menu_strings[MENU_ITEMS_MAX];
	uint8_t menu_current;
	uint8_t menu_redraw_required;
	uint8_t last_key_code;
	uint16_t menu_values[MENU_ITEMS_MAX];
	uint8_t menu_double;

}menu_t;



typedef struct
{
	uint8_t modo;
	uint16_t tiempo;
	uint8_t intensidad;
	uint8_t alarma_por_encima;
	uint8_t alarma_por_debajo;
	uint8_t alarma_status;
	uint16_t anio;
	uint8_t mes;
	uint8_t dia;
	uint8_t hora;
	uint8_t minuto;
	uint8_t segundos;

}configuracion_t;

#endif /* PROJECTS_DECIBELIMETRO_V0_0_INC_TYPES_H_ */

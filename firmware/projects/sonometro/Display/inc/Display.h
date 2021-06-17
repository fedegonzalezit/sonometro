/*
 * Display.h
 *
 *  Created on: Oct 15, 2018
 *      Author: federico
 */

#ifndef PROJECTS_DECIBELIMETRO_V0_0_INC_DISPLAY_H_
#define PROJECTS_DECIBELIMETRO_V0_0_INC_DISPLAY_H_

#include "main.h"
#include "u8g_arm.h"
#include "types.h"

#define KEY_NONE 0
#define KEY_PREV 1
#define KEY_NEXT 2
#define KEY_SELECT 3
#define KEY_BACK 4

#define KEY_LEFT	5
#define	KEY_RIGHT	6
#define	KEY_UP		7
#define	KEY_DOWN	8

#define		LPEAK		1
#define		LEQ			2

#define 	MENU_ITEMS_MAX		7
#define		MENU_CANTIDAD		6

#define 	MENU_PRINCIPAL		0
#define 	MENU_HORARIO		1
#define 	MENU_INTENSIDAD		2
#define 	MENU_ALARMAS		3
#define 	MENU_MODO			4
#define 	MENU_HISTORIAL		5


#define		SIMPLE				1
#define		DOBLE				2


void initDisplay(void);


void newMenu (menu_t *menu, uint8_t items, const char **menu_strings, uint16_t *values, uint8_t doble);



void update_menu(menu_t *menu, uint8_t key);
void draw_menu(menu_t *menu);
void draw_menu_double(menu_t *menu, uint16_t *values);
void ActualizarPantalla(pantalla_t valores);

void draw_pantalla (pantalla_t valores);

void menu_to_display (menu_t *menu);

void mostrarValores (char *buff, uint16_t *values, const char *menu_strings);


#endif /* PROJECTS_DECIBELIMETRO_V0_0_INC_DISPLAY_H_ */

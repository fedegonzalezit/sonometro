/*
 * statechart.h
 *
 *  Created on: Oct 16, 2018
 *      Author: federico
 */

#ifndef PROJECTS_DECIBELIMETRO_V0_0_INC_STATECHART_H_
#define PROJECTS_DECIBELIMETRO_V0_0_INC_STATECHART_H_

#include "main.h"
#include "types.h"

#define IDLE				10



#define	MODO			1
#define	ALARMA			2
#define	HISTORIAL		3
#define	INTENSIDAD		4
#define	HORA			5
#define	PRINCIPAL		6
#define ATRAS			6

#define	ON			1
#define	OFF			0

#define	SAVED		1
#define	NOT_SAVED	0




void init_stateChart (menu_t * menus[], configuracion_t *config);
void stateChart_runCycle(pantalla_t *values);
void clearInEvents (void);
void clearOutEvents (void);


void idle_entry_function(pantalla_t *values);
void menuPrincipal_entry_function (void);
void menuModo_entry_function (void);
void menuHistorial_entry_function (void);
void menuIntensidad_entry_function (void);
void menuAlarma_entry_function (void);
void menuHora_entry_function (void);



void modo_out_function (uint8_t isSaved);
void alarma_out_function (uint8_t isSaved);
void hora_out_function (uint8_t isSaved);
void intensidad_out_function (uint8_t isSaved);
void historial_out_function (uint8_t isSaved);
void menuPrincipal_out_function (void);


void keyDetected_function (menu_t *menu, uint8_t key);



void Iface_raise_evMenu (void);
void Iface_raise_evKeyDetected(uint8_t key);





void opKeyDetected (menu_t *menu, uint8_t key);
void opGuardiarCambios(uint8_t isSaved, uint8_t menuSaved, configuracion_t *newConfig, menu_t *menu);
void opActualizarPantalla (pantalla_t *valores);
void opMenuPrincipal (menu_t * menu);
void opMenuModo (menu_t * menu);
void opMenuHistorial (menu_t * menu);
void opMenuIntensidad (menu_t * menu);
void opMenuAlarma (menu_t * menu);
void opMenuHora (menu_t * menu);
void opGuardarModo (uint8_t isSaved, menu_t *menu);
void opGuardarIntensidad(uint8_t isSaved, menu_t *menu);
void opGuardarHora(uint8_t isSaved, menu_t *menu);
void opGuardarAlarma(uint8_t isSaved, menu_t *menu);



#endif /* PROJECTS_DECIBELIMETRO_V0_0_INC_STATECHART_H_ */

/*
 * statechart_required.c
 *
 *  Created on: Oct 16, 2018
 *      Author: federico
 */


#include "statechart.h"
#include "types.h"

extern configuracion_t config;
extern xQueueHandle modoQ, tIntegracionQ;

void opKeyDetected (menu_t *menu, uint8_t key)
{
	update_menu(menu, key);
}





/*
 * Funcion para enviar por una cola la configuracion guardada a la tarea de procesamiento
 * Recibe si el usuario quiso o no guardar los cambios
 * Si el usuario quiso guardar los cambios envia por una cola la nueva configuracion a la tarea correspondiente
 * Si no, resetea los valores de pantalla
 */
void opGuardarModo (uint8_t isSaved, menu_t *menu)
{
		if (isSaved == SAVED)
		{


			config.modo = menu->menu_values[MODO_MODO_INDEX];
			config.tiempo = menu->menu_values[MODO_TIEMPO_INDEX];
		//	xQueueSend (modoQ, &config.modo, 0);
		//	xQueueSend (tIntegracionQ,  &config.tiempo, 0);


		}
		//No guardo la configuracion! se cambian los valores en pantalla por los originales
		else
		{
			menu->menu_values[MODO_MODO_INDEX] = config.modo;
			menu->menu_values[MODO_TIEMPO_INDEX] = config.tiempo;


		}

}


/*
 * Funcion para enviar por una cola la configuracion guardada a la tarea de procesamiento
 * Recibe si el usuario quiso o no guardar los cambios
 * Si el usuario quiso guardar los cambios envia por una cola la nueva configuracion a la tarea correspondiente
 * Si no, resetea los valores de pantalla
 */
void opGuardarAlarma (uint8_t isSaved, menu_t *menu)
{
		if (isSaved == SAVED)
		{

			config.alarma_por_encima = menu->menu_values[ALARMA_ENCIMADE_INDEX];
			config.alarma_por_debajo = menu->menu_values[ALARMA_DEBAJODE_INDEX];
			config.alarma_status = menu->menu_values[ALARMA_STATUS_INDEX];

			//FALTA ENVIAR COLAS


		}
		//No guardo la configuracion! se cambian los valores en pantalla por los originales
		else
		{
			menu->menu_values[ALARMA_ENCIMADE_INDEX] = config.alarma_por_encima;
			menu->menu_values[ALARMA_DEBAJODE_INDEX] = config.alarma_por_debajo;
			menu->menu_values[ALARMA_STATUS_INDEX] = config.alarma_status;
		}
}

/*
 * Funcion para enviar por una cola la configuracion guardada a la tarea de procesamiento
 * Recibe si el usuario quiso o no guardar los cambios
 * Si el usuario quiso guardar los cambios envia por una cola la nueva configuracion a la tarea correspondiente
 * Si no, resetea los valores de pantalla
 */
void opGuardarHora (uint8_t isSaved, menu_t *menu)
{
		if (isSaved == SAVED)
		{

			config.anio = menu->menu_values[HORARIO_ANIO_INDEX];
			config.mes = menu->menu_values[HORARIO_MES_INDEX];
			config.dia = menu->menu_values[HORARIO_DIA_INDEX];
			config.hora =  menu->menu_values[HORARIO_HORA_INDEX];
			config.minuto = menu->menu_values[HORARIO_MINUTO_INDEX];

			//FALTA ENVIAR COLAS


		}
		//No guardo la configuracion! se cambian los valores en pantalla por los originales
		else
		{
				menu->menu_values[HORARIO_ANIO_INDEX] = config.anio;
				menu->menu_values[HORARIO_MES_INDEX] = config.mes;
				menu->menu_values[HORARIO_DIA_INDEX] = config.dia;
				menu->menu_values[HORARIO_HORA_INDEX] = config.hora;
				menu->menu_values[HORARIO_MINUTO_INDEX] = config.minuto;
		}
}


void opGuardarIntensidad(uint8_t isSaved, menu_t *menu)
{
	if (isSaved == SAVED)
	{
		config.intensidad = menu->menu_values[INTENSIDAD_INDEX];

		//FALTA ENVIAR COLAS
	}
	//No guardo la configuracion! se cambian los valores en pantalla por los originales
	else
	{
		menu->menu_values[INTENSIDAD_INDEX] = config.intensidad;

	}
}


void opActualizarPantalla (pantalla_t *valores)
{
	draw_pantalla (*valores);

}
void opMenuPrincipal (menu_t *menu)
{
	menu_to_display (menu);
}
void opMenuModo (menu_t * menu)
{
	menu_to_display (menu);
}
void opMenuHistorial (menu_t * menu)
{
	menu_to_display (menu);
}
void opMenuIntensidad (menu_t * menu)
{
	menu_to_display (menu);
}
void opMenuAlarma (menu_t * menu)
{
	menu_to_display (menu);
}
void opMenuHora (menu_t *menu)
{
	menu_to_display (menu);
}

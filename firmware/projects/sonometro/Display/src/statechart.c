/*
 * statechart.c
 *
 *  Created on: Oct 16, 2018
 *      Author: federico
 */

#include "statechart.h"


//Variables privadas al modulo
volatile static unsigned int estado;

volatile static unsigned int Interface_key, Interface_menu, Interface_menu_option;
//volatile pantalla_t *values;
menu_t *menu_principal, *menu_horario, *menu_intensidad, *menu_alarmas, *menu_modo, *menu_historial, *menu_hora;
extern menu_t allMenus[MENU_CANTIDAD];
uint8_t SwitchTick = 0;

void init_stateChart (menu_t * menus[], configuracion_t *config)
{

	//lleva todos los valores a 0
	estado = IDLE;

	Interface_key = KEY_NONE;
	Interface_menu = OFF;
	Interface_menu_option = OFF;

	//Inicializo menus

//	menu_modo = menus[MENU_MODO];
//	menu_principal = menus[MENU_PRINCIPAL];
//	menu_horario = menus[MENU_HORARIO];
//	menu_intensidad =menus[MENU_INTENSIDAD];
//	menu_alarmas = menus[MENU_ALARMAS];
//	menu_historial = menus[MENU_HISTORIAL];

	menu_modo = &allMenus[MENU_MODO];
	menu_principal = &allMenus[MENU_PRINCIPAL];
	menu_horario = &allMenus[MENU_HORARIO];
	menu_intensidad = &allMenus[MENU_INTENSIDAD];
	menu_alarmas = &allMenus[MENU_ALARMAS];
	menu_historial = &allMenus[MENU_HISTORIAL];

	//Puntero que apunta a la direccion de memoria de la configuracion
	//En caso de que se cambie la configuracion se cambia el puntero
}


void stateChart_runCycle(pantalla_t *values)
{


	clearOutEvents ();

	switch (estado)
	{

	case IDLE:
		idle_entry_function(values);
		if (Interface_menu == ON)
		{
			estado = MENU_PRINCIPAL;
		}
		break;

	case MENU_PRINCIPAL:
		menuPrincipal_entry_function ();

		if (Interface_menu == ON)
		{

			switch (Interface_menu_option)
			{
			case MODO:
				estado = MENU_MODO;
				break;

			case HISTORIAL:
				estado = MENU_HISTORIAL;
				break;

			case INTENSIDAD:
				estado = MENU_INTENSIDAD;
				break;

			case ALARMA:
				estado = MENU_ALARMAS;
				break;

			case HORA:
				estado = MENU_HORARIO;
				break;

			case ATRAS:
				estado = IDLE;



			}
			menuPrincipal_out_function ();
		}
		if (Interface_key != OFF)
		{
			estado = MENU_PRINCIPAL;
			keyDetected_function (menu_principal, Interface_key);
		}


		break;


	case MENU_MODO:
		menuModo_entry_function ();

		if (Interface_menu == ON)
		{

			if (Interface_menu_option == menu_modo->items)//El ultimo item es opcion volver
			{
				estado = IDLE;
				modo_out_function (NOT_SAVED);
			}
			else
			{
				estado = IDLE;
				modo_out_function (SAVED);
			}
		}
		if (Interface_key != OFF)
		{
			estado = MENU_MODO;
			keyDetected_function (menu_modo, Interface_key);
		}

		break;

	case MENU_ALARMAS:
		menuAlarma_entry_function ();


		if (Interface_menu == ON)
		{
			if (Interface_menu_option == menu_alarmas->items)//El ultimo item es opcion volver
			{
				estado = IDLE;
				alarma_out_function (NOT_SAVED);
			}
			else
			{
				estado = IDLE;
				alarma_out_function (SAVED);
			}

		}
		if (Interface_key != OFF)
		{
			estado = MENU_ALARMAS;
			keyDetected_function (menu_alarmas, Interface_key);
		}
		break;

	case MENU_HORARIO:
		menuHora_entry_function ();


		if (Interface_menu == ON)
		{
			if (Interface_menu_option == menu_horario->items)  //El ultimo item es opcion volver
			{
				estado = IDLE;
				hora_out_function (NOT_SAVED);

			}
			else
			{
				estado = IDLE;
				hora_out_function (SAVED);
			}

		}
		if (Interface_key != OFF)
		{
			estado = MENU_HORARIO;
			keyDetected_function (menu_hora, Interface_key);
		}
		break;

	case MENU_HISTORIAL:

		menuHistorial_entry_function ();

		if (Interface_menu == ON)
		{
			if (Interface_menu_option == menu_historial->items)
			{
				estado = IDLE;
				historial_out_function (NOT_SAVED);

			}
			else
			{
				estado = IDLE;
				historial_out_function (SAVED);
			}

		}
		if (Interface_key != OFF)
		{
			estado = MENU_HISTORIAL;
			keyDetected_function (menu_historial, Interface_key);
		}
		break;

	case MENU_INTENSIDAD:

		menuIntensidad_entry_function ();

		if (Interface_menu == ON)
		{
			if (Interface_menu_option == menu_intensidad->items)
			{
				estado = IDLE;
				intensidad_out_function (NOT_SAVED);

			}
			else
			{
				estado = IDLE;
				intensidad_out_function (SAVED);
			}

		}
		if (Interface_key != OFF)
		{
			estado = MENU_INTENSIDAD;
			keyDetected_function (menu_intensidad, Interface_key);
		}
		break;


	}

	clearInEvents ();

}

void clearInEvents (void)
{
//Pone todas las variables de interfaz en OFF
	Interface_key = KEY_NONE;
	Interface_menu = OFF;
	Interface_menu_option = OFF;


}
void clearOutEvents (void)
{
//Pone todas las variables de interfaz en OFF
}


void idle_entry_function(pantalla_t *values)
{
	opActualizarPantalla (values);
}
void menuPrincipal_entry_function (void)
{
	opMenuPrincipal (menu_principal);
}
void menuModo_entry_function (void)
{
	opMenuModo(menu_modo);
}
void menuHistorial_entry_function (void)
{
	opMenuHistorial (menu_historial);
}
void menuIntensidad_entry_function (void)
{
	opMenuIntensidad (menu_intensidad);
}
void menuAlarma_entry_function (void)
{
	opMenuAlarma(menu_alarmas);
}
void menuHora_entry_function (void)
{
	opMenuHora (menu_horario);
}


//Funciones de salida de estado (para guardar datos cambiados)
void modo_out_function (uint8_t isSaved)
{
	opGuardarModo(isSaved,  menu_modo);
}
void alarma_out_function (uint8_t isSaved)
{
	opGuardarAlarma(isSaved, menu_alarmas);

}
void hora_out_function (uint8_t isSaved)
{
	opGuardarHora(isSaved, menu_horario);
}
void intensidad_out_function (uint8_t isSaved)
{
	opGuardarIntensidad(isSaved, menu_intensidad);

}
void historial_out_function (uint8_t isSaved)
{
	//opGuardiarCambios(isSaved, MENU_HISTORIAL, newConfig, menu_historial);

}

void menuPrincipal_out_function (void)
{
//	//Reseteo las variables para moverse visualmente en el menu
	menu_principal->last_key_code = KEY_NONE;
	menu_principal->menu_current = 0;
	menu_principal->menu_redraw_required = 0;
}




void keyDetected_function (menu_t *menu, uint8_t key)
{
	opKeyDetected(menu, key);
}


void Iface_raise_evMenu (void)
{
	Interface_menu = ON;

	//Que elemento del menu principal se selecciono
	Interface_menu_option = menu_principal->menu_current;

	Interface_menu_option++; //Los defines estan desde 1 y el menu current arranca en 0



}


/*
 * Recibe la key que se selecciono
 * KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_NONE PARA NINGUNA KEY
 */
void Iface_raise_evKeyDetected(uint8_t key)
{
	if (SwitchTick > 3)
	{
		Interface_key = key;
	}
	SwitchTick = 0;

}




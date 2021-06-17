/*
 * Display.c
 *
 *  Created on: Oct 15, 2018
 *      Author: federico
 */

#include "Display.h"
#include "statechart.h"
#include "semphr.h"


u8g_t u8g2;
menu_t allMenus[MENU_CANTIDAD];
xSemaphoreHandle SemMenu, SemKeyUp, SemKeyDown, SemKeyLeft, SemKeyRight;
xQueueSetHandle set, setDatos;
configuracion_t config;
pantalla_t pantalla;
xQueueHandle tIntegracionQ, modoQ;
extern xQueueHandle xResult, DisplayQ;



/*
 * Tarea de interfaz, maquina de estados
 */

static void vInterface(void * valores)
{
	xQueueSetMemberHandle s;


	//Inicialización del display
	u8g_InitComFn(&u8g2, &u8g_dev_st7920_128x64_hw_spi, u8g_com_hw_spi_fn);
	u8g_SetDefaultForegroundColor(&u8g2);
	u8g_SetRot180(&u8g2);

	//Inicialización de la maquina de estados
	//Recibe como parametro la dirección en memoria de los menus
	init_stateChart ((menu_t **)&allMenus, &config);


	pantalla.anio= 2018;
	pantalla.bateria= 100;
	pantalla.dia=1;
	pantalla.hora=20;
	pantalla.modo = config.modo;
	pantalla.segundos = 0;


	//Envio tiempo de integracion a procesamiento
//	xQueueSend (tIntegracionQ, &config.tiempo, 0);
	//Envio modo inicial
//	xQueueSend (modoQ, &config.modo, 0);

	while (1)
	{

		/*La maquina de estado recibe estructura con los valores para actualizar la pantalla*/

		stateChart_runCycle(&pantalla);

		s = xQueueSelectFromSet(set,  500 / portTICK_RATE_MS);



		if ( s == SemMenu)
		{
			Iface_raise_evMenu ();
			/* take activated semaphore again */
			xSemaphoreTake(s, 0);
		}

		if ( s == SemKeyUp)
		{
			Iface_raise_evKeyDetected(KEY_UP);
			/* take activated semaphore again */
			xSemaphoreTake(s, 0);
		}
		if ( (xSemaphoreHandle)s == SemKeyDown)
		{
			Iface_raise_evKeyDetected(KEY_DOWN);
			/* take activated semaphore again */
			xSemaphoreTake(s, 0);
		}
		if ( (xSemaphoreHandle)s == SemKeyRight)
		{
			Iface_raise_evKeyDetected(KEY_RIGHT);
			/* take activated semaphore again */
			xSemaphoreTake(s, 0);
		}
		if ( (xSemaphoreHandle)s == SemKeyLeft)
		{
			Iface_raise_evKeyDetected(KEY_LEFT);
			/* take activated semaphore again */
			xSemaphoreTake(s, 0);
		}






		/*La maquina de estado recibe estructura con los valores para actualizar la pantalla*/
		//stateChart_runCycle(&pantalla);
//        /* refresh screen after some delay */
//
		//u8g_Delay(200);
	}

}


//Tarea que recibe datos y actualiza la estructura pantalla pantalla
static void vDatos(void * v)
{
	pantalla_t pantalla_temp;
	xQueueSetMemberHandle s;


	while (1)
	{


	s = xQueueSelectFromSet(setDatos,  portMAX_DELAY);


	if (s == xResult)
	{
		xQueueReceive (xResult, &pantalla_temp,0);
		pantalla.medicion_entero = pantalla_temp.medicion_entero;
		pantalla.medicion_coma = pantalla_temp.medicion_coma;
	}
	if (s == DisplayQ)
	{
		xQueueReceive (DisplayQ, &pantalla_temp, 0);
		pantalla.hora = pantalla_temp.hora;
		pantalla.minuto = pantalla_temp.minuto;
		pantalla.segundos = pantalla_temp.segundos;
	}
	}
}


void initTimers (void)
{
	Chip_TIMER_Init(LPC_TIMER1);
	Chip_TIMER_PrescaleSet(LPC_TIMER1,
		Chip_Clock_GetPeripheralClockRate(SYSCTL_PCLK_TIMER1) / 1000000 - 1);
	Chip_TIMER_MatchEnableInt(LPC_TIMER1, 0);
	Chip_TIMER_ResetOnMatchEnable(LPC_TIMER1, 0);
	Chip_TIMER_StopOnMatchDisable(LPC_TIMER1, 0);

	Chip_TIMER_SetMatch(LPC_TIMER1, 0, 1000);

	NVIC_EnableIRQ(TIMER1_IRQn);
}

/*
 * Inicialización de tareas y menus del display
 */
void initDisplay()
{

	initTimers();

	/********************Sacarlo***************//////
//	u8g_InitComFn(&u8g2, &u8g_dev_st7920_128x64_hw_spi, u8g_com_hw_spi_fn);
//	u8g_SetDefaultForegroundColor(&u8g2);
//	u8g_SetRot180(&u8g2);

	/**********************************************************/
	//Inicializacion de configuracion;

	config.modo = LEQ;
	config.tiempo = 500; //Tiempo de integracion en mili segundos
	config.alarma_por_encima = 100; //alarma por encima de 100 db
	config.alarma_por_debajo = 50; //alarma por debajo de 50 db
	config.alarma_status = OFF;

		const char *itemsMenuPrincipal[6] = {"Seleccionar modo", "Alarmas", "Historial", "Pantalla",
											"Fecha y hora", "Volver"};

		const char *itemsMenuFecha[6] = {"Anio", "Mes", "Dia", "Hora", "Minutos", "Volver"};
		uint16_t valuesFecha[5] = {2000,10,10,1,1}; //Inicializar con valores de RTC

		const char *itemsMenuAlarmas[4] = {"Activado", "Debajo de", "Por encima de", "Volver"};
		uint16_t valuesAlarmas[3] = {config.alarma_status, config.alarma_por_debajo, config.alarma_por_encima};

		const char *itemsMenuModo[3] = {"Modo", "Tiempo", "Volver"};
		uint16_t valuesModo[2] = {config.modo, config.tiempo};


		const char *itemsMenuIntensidad[2] = {"Intensidad", "Volver"};
		uint16_t valuesIntensidad = 0;
		const char *itemsMenuHistorial[2] = {"Historial", "Volver"};


		//Inicializacion de menus de configuracion
		newMenu (&allMenus[MENU_PRINCIPAL], 6, itemsMenuPrincipal,0, OFF);
		newMenu (&allMenus[MENU_HORARIO], 6, itemsMenuFecha, valuesFecha, ON);
		newMenu (&allMenus[MENU_INTENSIDAD], 2, itemsMenuIntensidad, &valuesIntensidad, ON);
		newMenu(&allMenus[MENU_ALARMAS], 4, itemsMenuAlarmas, valuesAlarmas, ON);
		newMenu (&allMenus[MENU_MODO], 3, itemsMenuModo, valuesModo, ON);
		newMenu (&allMenus[MENU_HISTORIAL], 2, itemsMenuHistorial, 0, OFF); //Falta implementar, fijarse
																//si historial se muestra asi





		xTaskCreate(vInterface, (const signed char *)"interface", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+1, 0);
		xTaskCreate(vDatos, (const signed char *)"datos", configMINIMAL_STACK_SIZE*2, 0, tskIDLE_PRIORITY+2, 0);	//ver prioridad


		tIntegracionQ = xQueueCreate(1, sizeof(config.tiempo));
		modoQ = xQueueCreate (1, sizeof (config.modo));


		vSemaphoreCreateBinary(SemMenu);
		vSemaphoreCreateBinary(SemKeyUp);
		vSemaphoreCreateBinary(SemKeyDown);
		vSemaphoreCreateBinary(SemKeyLeft);
		vSemaphoreCreateBinary(SemKeyRight);

		 /* Take the semaphore, so it starts empty.A block time of zero can be used
		       because the semaphore is guaranteed to be available. It has just been created. */
		xSemaphoreTake(SemMenu, 0);
		xSemaphoreTake(SemKeyUp, 0);
		xSemaphoreTake(SemKeyDown, 0);
		xSemaphoreTake(SemKeyLeft, 0);
		xSemaphoreTake(SemKeyRight, 0);




		set = xQueueCreateSet(5);

		xQueueAddToSet(SemMenu, set);
		xQueueAddToSet(SemKeyUp, set);
		xQueueAddToSet(SemKeyDown, set);
		xQueueAddToSet(SemKeyLeft, set);
		xQueueAddToSet(SemKeyRight, set);


		setDatos = xQueueCreateSet (2);
		xQueueAddToSet(xResult, setDatos);
		xQueueAddToSet(DisplayQ, setDatos);

	    GPIOInterrupcionesInit ();

}



/*
 * Interrupciones por GPIO que activan semaforos para controlar la interfaz
 */



void EINT3_IRQHandler(void)
{
uint32_t pines;
static portBASE_TYPE xTaskSwitchRequired;

	/*
	 * Interrupciones externas configuradas para SW de la placa de info
	 * Volver a configurar al momento de usar la placa del proyecto
	 */

//	vTaskDelay(50/portTICK_RATE_MS);
	// Checkea si interrumpió algun pin del puerto 2
	if (Chip_GPIOINT_IsIntPending(LPC_GPIOINT, GPIOINT_PORT0))
    {
		// Pines que interrumpieron por flanco ascendente
		pines = Chip_GPIOINT_GetStatusFalling(LPC_GPIOINT, GPIOINT_PORT0);


		//Interrumpe por flanco ascendente P0[30] (placa info teclado 0)
	  	if((pines) & (1<<30))
	  	{
	  		//Enciendo semaforo

	  		xSemaphoreGiveFromISR(SemKeyRight, &xTaskSwitchRequired);


		  	// Apago flags de interrupciones
	  		Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT, GPIOINT_PORT0, 1<<30);
	  	}

		//Interrumpe por flanco ascendente P0[5]
	  	if((pines) & (0x01<<5))
	  	{
	  		//Enciendo semaforo
	  		xSemaphoreGiveFromISR(SemKeyDown, &xTaskSwitchRequired);

		  	// Apago flags de interrupciones
	  		Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT, GPIOINT_PORT0,1<<5);
	  	}


		//Interrumpe por flanco ascendente P0[10] SW3
	  	if((pines) & (0x01<<10))
	  	{
	  		//Enciendo semaforo
	  		xSemaphoreGiveFromISR(SemMenu, &xTaskSwitchRequired);

		  	// Apago flags de interrupciones
	  		Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT, GPIOINT_PORT0, 1<<10);
	  	}


		//Interrumpe por flanco ascendente P0[29] (keyUp)
	  	if((pines) & (0x01<<29))
	  	{
	  		//Enciendo semaforo
			xSemaphoreGiveFromISR(SemKeyUp, &xTaskSwitchRequired);
		  	// Apago flags de interrupciones
	  		Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT, GPIOINT_PORT0, 1<<29);
	  	}

		//Interrumpe por flanco ascendente P0[4] (keyLeft)
	  	if((pines) & (0x01<<4))
	  	{
	  		//Enciendo semaforo
	  		xSemaphoreGiveFromISR(SemKeyLeft, &xTaskSwitchRequired);
		  	// Apago flags de interrupciones
	  		Chip_GPIOINT_ClearIntStatus(LPC_GPIOINT, GPIOINT_PORT0, 1<<4);
	  	}

  		portEND_SWITCHING_ISR(xTaskSwitchRequired);

    }



}


/*
 * Funcion para inicializar menu
 *
 */
void newMenu (menu_t *menu, uint8_t items, const char **menu_strings, uint16_t *values, uint8_t doble)
{

	uint16_t i;
	menu->items = items;
	menu->last_key_code = KEY_NONE;
	menu->menu_current = 0;
	menu->menu_redraw_required = 0;
	menu->menu_double = doble;

	for (i=0; i<items; i++)
	{
		if (doble)
		{
			menu->menu_values[i] = values[i];
		}
		menu->menu_strings[i] = menu_strings[i];
	}
}

/*
 * Función para actualizar valor en pantalla
 * @param	valores	: Estructura con los valores para actualizar pantalla principal
 */

void ActualizarPantalla(pantalla_t valores)
{
	char medicion[4], fecha[20], bat[9], buff[8];
	u8g_uint_t w;

	u8g_SetFont(&u8g2, u8g_font_unifont);

	u8g_SetFontPosTop(&u8g2);

	if (valores.medicion_coma < 10)
		sprintf (medicion, "%d.%d", valores.medicion_entero, valores.medicion_coma);
	else
		sprintf (medicion, "%d.%d", valores.medicion_entero, valores.medicion_coma);

	sprintf (fecha, "%d/%d/%d %d:%d:%d", valores.dia, valores.mes, valores.anio,
			valores.hora, valores.minuto, valores.segundos);

	sprintf (bat, "Bat:100%%", valores.bateria);


	u8g_DrawStr(&u8g2, 100, 28, "dbA");


	u8g_SetScale2x2(&u8g2);					// Scale up all draw procedures
	u8g_DrawStr(&u8g2, 12, 8, medicion);
	u8g_UndoScale(&u8g2);					// IMPORTANT: Switch back to normal mode

	u8g_SetFont(&u8g2, u8g_font_5x7);

	if (config.modo == LPEAK)
	{
		u8g_DrawStr(&u8g2, 0, 8, "Lpeak");

	}
	if (config.modo == LEQ)
	{
		sprintf (buff, "Leq T:%d", config.tiempo);
		u8g_DrawStr(&u8g2, 0, 8, buff);

	}
	w = u8g_GetWidth(&u8g2) -u8g_GetStrWidth(&u8g2, bat) ;
	u8g_DrawStr(&u8g2, 10, 8, bat);
	u8g_DrawStr(&u8g2, 0, 60, fecha);
}

/*
 * Funcion para dibujar la pantalla principal
 */
void draw_pantalla (pantalla_t valores)
{
	 u8g_FirstPage(&u8g2);
	    do
	    {
	    	ActualizarPantalla(valores);
	    } while ( u8g_NextPage(&u8g2) );

}


/*
 * Función dibujar el menú
 */

void draw_menu(menu_t *menu)
{
  uint8_t i, h;
  u8g_uint_t w, d;

  u8g_SetFont(&u8g2, u8g_font_5x7);
  u8g_SetFontRefHeightText(&u8g2);
  u8g_SetFontPosTop(&u8g2);

  h = u8g_GetFontAscent(&u8g2)-u8g_GetFontDescent(&u8g2);
  w = u8g_GetWidth(&u8g2);
  for( i = 0; i < menu->items; i++ )
  {
    d = (w-u8g_GetStrWidth(&u8g2, menu->menu_strings[i]))/2;
    u8g_SetDefaultForegroundColor(&u8g2);
    if ( i == menu->menu_current )
    {
      u8g_DrawBox(&u8g2, 0, i*h+1, w, h);
      u8g_SetDefaultBackgroundColor(&u8g2);
    }
    u8g_DrawStr(&u8g2, d, i*h, menu->menu_strings[i]);
  }
  //u8g_DrawStr(&u8g2, d, i*h, "Volver");

}



/*
 * Función para actualizar el menu
 */
void update_menu(menu_t *menu, uint8_t key)
{


  switch ( key )
  {
    case KEY_DOWN:
      menu->menu_current++;
      if ( menu->menu_current >= menu->items )
    	  menu->menu_current = 0;
      menu->menu_redraw_required = 1;
      break;
    case KEY_UP:
      if ( menu->menu_current == 0 )
    	  menu->menu_current = menu->items;
      menu->menu_current--;
      menu->menu_redraw_required = 1;
      break;

    case KEY_LEFT:

    	if (menu->menu_double)
    	{
    		menu->menu_values[menu->menu_current] -= 1;
    		menu->menu_redraw_required = 1;
    	}


    	break;
    case KEY_RIGHT:

    	if (menu->menu_double)
    	{
        	menu->menu_values[menu->menu_current] += 1;
        	menu->menu_redraw_required = 1;
    	}


    	break;
  }
}


/*
 * Función dibujar el menú doble
 * Primera columna para titulos
 * 2da columna para valores
 */

void draw_menu_double(menu_t *menu, uint16_t *values)
{
	  uint8_t i, h;
	  u8g_uint_t w, d;
	  char buff[6];

	  u8g_SetFont(&u8g2, u8g_font_5x7);
	  u8g_SetFontRefHeightText(&u8g2);
	  u8g_SetFontPosTop(&u8g2);

	  h = u8g_GetFontAscent(&u8g2)-u8g_GetFontDescent(&u8g2);
	  w = u8g_GetWidth(&u8g2);

	  for( i = 0; i < (menu->items-1); i++ )
	  {
	    u8g_SetDefaultForegroundColor(&u8g2);
	    if ( i == menu->menu_current )
	    {
	      u8g_DrawBox(&u8g2, 0, i*h+1, w, h);
	      u8g_SetDefaultBackgroundColor(&u8g2);
	    }
	    u8g_DrawStr(&u8g2, 0, i*h, menu->menu_strings[i]);

	    //DEPENDIENDO DE QUE MENU SEA LOS VALORES SE MUESTRAN DE DISTINTA FORMA

	    mostrarValores (buff, &values[i], menu->menu_strings[i]);

	  //  sprintf (buff, "%d", values[i]);

	    d = w-u8g_GetStrWidth(&u8g2, buff);
	    u8g_DrawStr(&u8g2, d, i*h, buff);


	  }
	  d = (w-u8g_GetStrWidth(&u8g2, menu->menu_strings[i]))/2;
	    u8g_SetDefaultForegroundColor(&u8g2);
	    if ( i == menu->menu_current )
	    {
	      u8g_DrawBox(&u8g2, 0, i*h+1, w, h);
	      u8g_SetDefaultBackgroundColor(&u8g2);
	    }
	    u8g_DrawStr(&u8g2, d, i*h, menu->menu_strings[i]);

}


/*
 * Funcion para modificar los valores enteros a strings para visualizarlos en display
 * Dependendo de que menu sea los valores significan otra cosa
 */
void mostrarValores (char *buff, uint16_t *values, const char *menu_strings)
{
	//Compara todas las strins y cambia el buffer

	if ( ! strcmp (menu_strings, "Modo") )
	{
		if (*values >= LEQ)
		{
		    strcpy (buff, "Leq");
		    *values = LEQ;

		}
		else if (*values <= LPEAK)
		{
		    strcpy (buff, "Lpeak");
		    *values = LPEAK;
		}
	}

	else if ( ! strcmp (menu_strings, "Tiempo") )
	{
		//LLENAR (DAR SOLO ALGUNOS VALORES DE TIEMPO)
		//AGREGAR TIEMPO EN DISPLAY
	    sprintf (buff, "%d", *values);

	}

	else if ( ! strcmp (menu_strings, "Mes") )
	{
		if (*values > 12)
		{
			*values = 12;
		    sprintf (buff, "%d", *values);
		}
		if (*values < 1)
		{
			*values = 1;
		    sprintf (buff, "%d", *values);
		}
	}


	else if (! strcmp (menu_strings, "Dia"))
	{
		if (*values > 31)
		{
			*values = 31;
		    sprintf (buff, "%d", *values);
		}
		if (*values < 1)
		{
			*values = 1;
		    sprintf (buff, "%d", *values);
		}
	}


	else if ( !strcmp (menu_strings, "Hora"))
	{
		if (*values > 23)
		{
			*values %= 24;
		    sprintf (buff, "%d", *values);
		}
//		if (*values < 1)
//		{
//			*values = 1;
//		    sprintf (buff, "%d", *values);
//		}
	}

	else if ( !strcmp (menu_strings, "Dia"))
	{
		if (*values > 31)
		{
			*values = 31;
		    sprintf (buff, "%d", *values);
		}
		if (*values < 1)
		{
			*values = 1;
		    sprintf (buff, "%d", *values);
		}
	}

	else if ( !strcmp (menu_strings, "Hora"))
	{
		if (*values > 24)
		{
			*values = 24;
		    sprintf (buff, "%d", *values);
		}
		if (*values < 1)
		{
			*values = 1;
		    sprintf (buff, "%d", *values);
		}
	}

	else if (!strcmp (menu_strings, "Minutos"))
	{
		if (*values > 60)
		{
			*values = 60;
		    sprintf (buff, "%d", *values);
		}
//		if (*values < 1)
//		{
//			*values = 1;
//		    sprintf (buff, "%d", *values);
//		}
	}

	else if ( !strcmp (menu_strings, "Activado") )
	{
		if (*values >= ON)
		{
		    strcpy (buff, "ON");
		    *values = ON;
		}
		if (*values <= OFF)
		{
		    strcpy (buff, "OFF");
		    *values = OFF;
		}
	}

}

void menu_to_display (menu_t *menu)
{
	if (menu->menu_double == OFF)
	{
	      u8g_FirstPage(&u8g2);
	      do
	      {
	    	  draw_menu(menu);

	      } while ( u8g_NextPage(&u8g2) );

	}
	else if (menu->menu_double == ON)
	{

	      u8g_FirstPage(&u8g2);
	      do
	      {
	    	  draw_menu_double(menu, menu->menu_values);

	      } while ( u8g_NextPage(&u8g2) );

	}

}



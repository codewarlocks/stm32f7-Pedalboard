/*
 * normal.c
 *
 *  Created on: Mar 22, 2019
 *      Author: gonza
 */
#include "main.h"

//Para dibujar perillas
//EJEMPLO WSLDN4 -> EX03
#include <math.h>
#include "ex03/mainex.h"
//#include "gui/bt_blackangle64_16.h"
//#include "gui/bt_blackangle48_12.h"
#include "ex03/macros.h"

int normal_state = 9;

int efecto_normal(int muestra)
{
	return muestra;
}

void Push_State_Normal (GUIElement *e)
{
	normal_state=e->state;
	//sprintf((char*)text, "%d      ", push_state);
    //BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	//BSP_LCD_DisplayStringAt(15,BSP_LCD_GetYSize()-25,(uint8_t*)text, LEFT_MODE);
}

int Get_State_Normal (void)
{
	 return normal_state;
}

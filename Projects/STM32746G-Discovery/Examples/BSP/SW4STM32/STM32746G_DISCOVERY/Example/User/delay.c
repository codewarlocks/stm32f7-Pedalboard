#include "gui/prototipos.h"

// parametros de desarrollador
#define DELAY_SIZE 60000//60000

int salida_d = 0;

// parametros de usuario
float level_d = 0.3, feedback_d = 0.5;
int time_d = 30000, state_d = 0;

// variables auxiliares
int line[DELAY_SIZE];

int cont_d = 0, flag = 0;

int delay (int entrada)
{
	if (cont_d == DELAY_SIZE)
	{
		flag = 1;
		cont_d = 0;
	}
	if (flag == 0)
	{
		line[cont_d] = entrada * (1-feedback_d);
		salida_d = entrada * (1-level_d);
		cont_d++;
	}
	if (flag == 1)
	{
		if (cont_d >= time_d)
		{
			line[cont_d] = entrada * (1-feedback_d) + line[cont_d-time_d] * feedback_d;
			salida_d = entrada * (1-level_d) + line[cont_d-time_d] * level_d;
		}
		else
		{
			line[cont_d] = entrada * (1-feedback_d) + line[DELAY_SIZE+cont_d-time_d] * feedback_d;
			salida_d = entrada * (1-level_d) + line[DELAY_SIZE+cont_d-time_d] * level_d;
		}
		cont_d++;
	}

	return salida_d;
}

void Delay_Time (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	time_d = (int)((DELAY_SIZE-1)*(db->value));
}

void Delay_Feedback (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
 	feedback_d = (db->value);
}

void Delay_Level (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
  level_d = (db->value);
}

void Push_State_Delay (GUIElement *e)
{
	state_d = e->state;
}

int Get_State_Delay (void)
{
	return state_d;
}

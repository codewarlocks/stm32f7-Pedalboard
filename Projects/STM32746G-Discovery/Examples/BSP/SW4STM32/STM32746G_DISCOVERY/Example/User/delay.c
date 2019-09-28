#include "gui/prototipos.h"

// parametros de desarrollador
#define DELAY_SIZE 3000

static int salida_d = 0;

// parametros de usuario
static float32_t level_d = 0.5, feedback_d = 0;
static int time_d = 20000;

// variables auxiliares
int line[DELAY_SIZE];

static int cont_d = 0, flag_d = 0;

void delayInit ()
{
	delayParametros();
}

void delayParametros()
{

}

int delayEfecto (int entrada)
{
	if (cont_d == DELAY_SIZE)
	{
		flag_d = 1;
		cont_d = 0;
	}
	if (flag_d == 0)
	{
		line[cont_d] = entrada * (1-feedback_d);
		salida_d = entrada * (1-level_d);
		cont_d++;
	}
	if (flag_d == 1)
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

	return 2*salida_d;
}

void delayTime (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	time_d = (int)((DELAY_SIZE-1)*(db->value));
}

void delayFeedback (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
 	feedback_d = (db->value);
}

void delayLevel (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	level_d = (db->value);
}

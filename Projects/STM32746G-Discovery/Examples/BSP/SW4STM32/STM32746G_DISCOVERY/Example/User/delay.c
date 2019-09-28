#include "gui/prototipos.h"

// parametros de desarrollador
#define DELAY_SIZE 50000

static int salida = 0;

// parametros de usuario
static float32_t level = 0.5, feedback = 0;
static int time = 20000;

// variables auxiliares
static int line[DELAY_SIZE], cont = 0, flag = 0;

void delayInit ()
{
	delayParametros();
}

void delayParametros()
{

}

int delayEfecto (int entrada)
{
	if (cont == DELAY_SIZE)
	{
		flag = 1;
		cont = 0;
	}
	if (flag == 0)
	{
		line[cont] = entrada * (1-feedback);
		salida = entrada * (1-level);
		cont++;
	}
	if (flag == 1)
	{
		if (cont >= time)
		{
			line[cont] = entrada * (1-feedback) + line[cont-time] * feedback;
			salida = entrada * (1-level) + line[cont-time] * level;
		}
		else
		{
			line[cont] = entrada * (1-feedback) + line[DELAY_SIZE+cont-time] * feedback;
			salida = entrada * (1-level) + line[DELAY_SIZE+cont-time] * level;
		}
		cont++;
	}

	return 2*salida;
}

void delayTime (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	time = (int)((DELAY_SIZE-1)*(db->value));
}

void delayFeedback (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
 	feedback = (db->value);
}

void delayLevel (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	level = (db->value);
}

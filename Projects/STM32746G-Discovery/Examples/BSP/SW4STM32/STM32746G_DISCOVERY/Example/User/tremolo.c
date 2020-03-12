#include "gui/prototipos.h"
#include "math.h"
#include "arm_math.h"
#include "arm_common_tables.h"

static int salida = 0;

// parametros de desarrollador
static float32_t mid = 0.500;

// parametros de usuario
static float32_t rate = 2, depth = 0.3;
static int modulacion = SINUSOIDAL;

// variables auxiliares
static float32_t max = 0, min = 0, delta = 0, coef = 0, auxcoef=0, lp=0.99;
static int periodo = 0, tiempo = 0, flag = 0;

void tremoloInit ()
{
	tremoloParametros();
}

void tremoloParametros ()
{
	coef=mid;
	min = mid - depth;
	max = mid + depth;
	periodo = (int) SR/rate;
	delta = (max - min)/periodo;
}

int tremoloEfecto (int entrada)
{
	coef = tremoloLFO();
	salida = entrada * coef;
	return salida;
}

float32_t tremoloLFO()
{
	switch(modulacion)
	{
	case CUADRADA:
		if(tiempo >= periodo/2)
		{
			if(auxcoef == min)
				auxcoef = max;
			else
				auxcoef = min;
			tiempo = 0;
		}
		coef=lp*coef+(1-lp) * auxcoef;
		break;

	case TRIANGULAR:
		if(flag == 0)
			coef = coef + 2 * delta;
		if(flag == 1)
			coef = coef - 2 * delta;
		if(coef >= max || coef <= min)
		{
			flag = 1 - flag;
			tiempo = 0;
		}
		break;

	case RAMPA_ASC:
		auxcoef = auxcoef + delta;
		if(auxcoef >= max)
		{
			auxcoef = min;
			tiempo = 0;
		}
		coef=lp*coef+(1-lp) * auxcoef;
		break;

	case SINUSOIDAL:
		coef = mid + depth * arm_sin_f32((float32_t)(2*3.1416*tiempo/periodo));
		if(tiempo == periodo)
			tiempo = 0;
		break;
	}
	tiempo++;
	return coef;
}

void tremoloRate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	rate = 4 * (db->value);
	tremoloParametros();
}

void tremoloDepth (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	depth = 0.45 * (db->value);
	tremoloParametros();
}

void tremoloMod (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	if (db->value <= 0.33)
		modulacion = SINUSOIDAL;
	else if(db->value > 0.33 && db->value <= 0.66)
		modulacion = CUADRADA;
	else if(db->value > 0.66 && db->value < 1)
		modulacion = TRIANGULAR;
	else if(db->value == 1)
		modulacion = RAMPA_ASC;
}

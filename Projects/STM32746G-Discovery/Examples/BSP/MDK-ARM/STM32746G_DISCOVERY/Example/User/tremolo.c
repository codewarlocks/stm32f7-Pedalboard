#include "gui/prototipos.h"
#include "math.h"

#include "arm_math.h"
#include "arm_common_tables.h"

static int32_t salida = 0;

// parametros de desarrollador
static float32_t mid = 0.500f;

// parametros de usuario
static float32_t rate = 2.0f, depth = 0.3f;
static int32_t modulacion = SINUSOIDAL;

// variables auxiliares
static float32_t max = 0.0f, min = 0.0f, delta = 0.0f, coef = 0.0f, auxcoef=0.0f, lp=0.99f;
static int32_t periodo = 0, tiempo = 0, flag = 0;

void tremoloInit ()
{
	tremoloParametros();
}

void tremoloParametros ()
{
	coef=mid;
	min = mid - depth;
	max = mid + depth;
	periodo = (int32_t) SR/rate;
	delta = (max - min)/periodo;
}

int32_t tremoloEfecto (int32_t entrada)
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
		if(tiempo >= periodo/2.0f)
		{
			if(auxcoef == min)
				auxcoef = max;
			else
				auxcoef = min;
			tiempo = 0;
		}
		coef=lp*coef+(1.0f-lp) * auxcoef;
		break;

	case TRIANGULAR:
		if(flag == 0)
			coef = coef + 2.0f * delta;
		if(flag == 1)
			coef = coef - 2.0f * delta;
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
		coef = mid + depth * arm_sin_f32((float32_t)(2.0f*PI*tiempo/periodo));
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
	rate = 4.0f * (db->value);
	tremoloParametros();
}

void tremoloDepth (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	depth = (float32_t)0.45f * (db->value);
	tremoloParametros();
}

void tremoloMod (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	if (db->value <= (float32_t)0.33f)
		modulacion = SINUSOIDAL;
	else if(db->value > (float32_t)0.33f && db->value <= (float32_t)0.66f)
		modulacion = CUADRADA;
	else if(db->value > (float32_t)0.66f && db->value < 1.0f)
		modulacion = TRIANGULAR;
	else if(db->value == 1)
		modulacion = RAMPA_ASC;
}

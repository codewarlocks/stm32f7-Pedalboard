#include "gui/prototipos.h"
#include "math.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"

int salida_t = 0;

// parametros de desarrollador
float mid_t = 0.5;

// parametros de usuario
float rate_t = 2, depth_t = 0.3;
int modulacion_t = SINUSOIDAL;

// variables auxiliares
float32_t max_t = 0, min_t = 0, delta_t = 0, coef_t = 0;
int periodo_t = 0;

int tiempo_t = 0, flag_t = 0;

float32_t auxcoef_t=0, lp_t=0.99;

void tremoloInit ()
{
	tremoloParametros();
}

void tremoloParametros ()
{
	coef_t=mid_t;
	min_t = mid_t - depth_t;
	max_t = mid_t + depth_t;
	periodo_t = (int) SR/rate_t;
	delta_t = (max_t - min_t)/periodo_t;
}

int tremoloEfecto (int entrada)
{
	coef_t = tremoloLFO();
	salida_t = entrada * coef_t;
	return salida_t;
}

float32_t tremoloLFO()
{
	switch(modulacion_t)
	{
	case CUADRADA:
		if(tiempo_t >= periodo_t/2)
		{
			if(auxcoef_t == min_t)
				auxcoef_t = max_t;
			else
				auxcoef_t = min_t;
			tiempo_t = 0;
		}
		coef_t=lp_t*coef_t+(1-lp_t) * auxcoef_t;
		break;

	case TRIANGULAR:
		if(flag_t == 0)
			coef_t = coef_t + 2 * delta_t;
		if(flag_t == 1)
			coef_t = coef_t - 2 * delta_t;
		if(coef_t >= max_t || coef_t <= min_t)
		{
			flag_t = 1 - flag_t;
			tiempo_t = 0;
		}
		break;

	case RAMPA_ASC:
		auxcoef_t = auxcoef_t + delta_t;
		if(auxcoef_t >= max_t)
		{
			auxcoef_t = min_t;
			tiempo_t = 0;
		}
		coef_t=lp_t*coef_t+(1-lp_t) * auxcoef_t;
		break;

	case SINUSOIDAL:
		coef_t = mid_t + depth_t * arm_sin_f32((float32_t)(2*3.1416*tiempo_t/periodo_t));
		if(tiempo_t == periodo_t)
			tiempo_t = 0;
		break;
	}
	tiempo_t++;
	return coef_t;
}

void tremoloRate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	rate_t = 4 * (db->value);
	tremoloParametros();
}

void tremoloDepth (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	depth_t = 0.45 * (db->value);
	tremoloParametros();
}

void tremoloMod (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	if (db->value <= 0.33)
		modulacion_t = SINUSOIDAL;
	else if(db->value > 0.33 && db->value <= 0.66)
		modulacion_t = CUADRADA;
	else if(db->value > 0.66 && db->value < 1)
		modulacion_t = TRIANGULAR;
	else if(db->value == 1)
		modulacion_t = RAMPA_ASC;
}

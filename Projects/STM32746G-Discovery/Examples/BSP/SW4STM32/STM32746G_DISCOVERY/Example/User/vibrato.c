#include "gui/prototipos.h"
#include "math.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"

// parametros de desarrollador
#define VIBRATO_SIZE 150

int salida_v = 0;

// parametros de usuario
float rate_v = 3, depth_v = 30;
int modulacion_v = SINUSOIDAL;

// parametros de desarrollador
int mid_v = VIBRATO_SIZE/2 - 1;

// variables auxiliares
float32_t min_v = 0, max_v = 0;
float32_t demora_v = VIBRATO_SIZE/2 - 1, delta_v = 0;
int periodo_v = 0;

int cont_v = 0, tiempo_v = 0, flag_v = 0, rampflag_v = 0;
float frac_v = 0;
int d_v = 0;
char texto[20];

int line_v[VIBRATO_SIZE];

float32_t auxcoef_v=0, lp_v=0.999;

void vibratoInit ()
{
	vibratoParametros();
}

void vibratoParametros ()
{
	demora_v= mid_v;
	min_v = mid_v - depth_v;
	max_v = mid_v + depth_v;
	periodo_v = (int) SR/rate_v;
	delta_v = (max_v - min_v)/periodo_v;
}

int vibratoEfecto (int entrada)
{
	if (cont_v == VIBRATO_SIZE)
	{
		flag_v = 1;
		cont_v = 0;
	}
	line_v[cont_v] = entrada;
	if (flag_v == 1)
	{
		demora_v = vibratoLFO(modulacion_v);
		d_v = (int) floor(demora_v);
		frac_v = demora_v - d_v;
		if (cont_v-d_v-1 >= 0)
			salida_v = frac_v * line_v[cont_v-d_v-1] + (1-frac_v) * line_v[cont_v-d_v];
		if (cont_v-d_v == 0)
			salida_v = frac_v * line_v[VIBRATO_SIZE-1] + (1-frac_v) * line_v[cont_v-d_v];
		if (cont_v-d_v < 0)
			salida_v = frac_v * line_v[VIBRATO_SIZE-d_v+cont_v-1] + (1-frac_v) * line_v[VIBRATO_SIZE-d_v+cont_v];
	}
	cont_v++;
	return salida_v;
}

float32_t vibratoLFO(int modulacion_v)
{
	switch(modulacion_v)
	{
	case CUADRADA:
		if(tiempo_v >= periodo_v/2)
		{
			if(auxcoef_v == min_v)
				auxcoef_v = max_v;
			else
				auxcoef_v = min_v;
			tiempo_v = 0;
		}
		demora_v=lp_v*demora_v+(1-lp_v) * auxcoef_v;
		break;

	case TRIANGULAR:
		if(rampflag_v == 0)
			demora_v = demora_v + 2*delta_v;
		if(rampflag_v == 1)
			demora_v = demora_v - 2*delta_v;
		if(demora_v >= max_v || demora_v <= min_v)
		{
			rampflag_v = 1 - rampflag_v;
			tiempo_v = 0;
		}
		break;

	case RAMPA_ASC:
		auxcoef_v = auxcoef_v + delta_v;
		if(auxcoef_v >= max_v)
		{
			auxcoef_v = min_v;
			tiempo_v = 0;
		}
		demora_v=lp_v*demora_v+(1-lp_v) * auxcoef_v;
		break;

	case SINUSOIDAL:
		demora_v = mid_v + depth_v * arm_sin_f32((float32_t)(2*3.1416*tiempo_v/periodo_v));
		if(tiempo_v == periodo_v)
			tiempo_v = 0;
		break;
	}
	tiempo_v++;
	return demora_v;
}

void vibratoRate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	rate_v = 5 * (db->value);
	vibratoParametros();
}

void vibratoDepth (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	depth_v = (mid_v - 1) * (db->value);
	vibratoParametros();
}

void vibratoMod (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	if (db->value <= 0.33)
		modulacion_v = CUADRADA;
	else if(db->value > 0.33 && db->value <= 0.66)
		modulacion_v = RAMPA_ASC;
	else if(db->value > 0.66 && db->value < 1)
		modulacion_v = TRIANGULAR;
	else if(db->value == 1)
		modulacion_v = SINUSOIDAL;
}

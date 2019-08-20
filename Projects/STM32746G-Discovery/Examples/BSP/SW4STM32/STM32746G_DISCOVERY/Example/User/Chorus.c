#include "gui/prototipos.h"
#include "math.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"

// parametros de desarrollador
#define CHORUS_SIZE 500

int salida_c = 0;

// parametros de usuario
float32_t manual_c = 110, rate_c = 1.2, depth_c = 40, feedback_c = 0;
int modulacion_c = SINUSOIDAL;

// variables auxiliares
float32_t min_c = 0, max_c = 0, tiempo_c, flag_c;
float32_t demora_c = CHORUS_SIZE/2 - 1, delta_c = 0;
int periodo_c = 0;

int cont_c = 0;
float32_t delay_c = 0, frac_c = 0, aux1_c = 0, aux2_c = 0, aux3_c = 0;
int d_c = 0;

int line1_c[CHORUS_SIZE] = {0};
int line2_c[CHORUS_SIZE] = {0};
int line3_c[CHORUS_SIZE] = {0};

float32_t auxcoef_c=0, lp_c=0.999;

void chorusInit ()
{
	chorusParametros();
}

void chorusParametros()
{
	demora_c= manual_c;
	min_c = manual_c - depth_c;
	max_c = manual_c + depth_c;
	periodo_c = (int) SR/rate_c;
	delta_c = (max_c - min_c)/periodo_c;
}


int chorusEfecto (int entrada)
{
	if (cont_c == CHORUS_SIZE)
		cont_c = 0;

	line1_c[cont_c] = entrada;
	line2_c[cont_c] = entrada;
	line3_c[cont_c] = entrada;

	demora_c = chorusLFO(modulacion_c);
	d_c = (int) floor(demora_c);
	frac_c = demora_c - d_c;
	if (cont_c-d_c-1 >= 0)
		aux1_c = frac_c * line1_c[cont_c-d_c-1] + (1-frac_c) * line1_c[cont_c-d_c];
	if (cont_c-d_c == 0)
		aux1_c = frac_c * line1_c[CHORUS_SIZE-1] + (1-frac_c) * line1_c[cont_c-d_c];
	if (cont_c-d_c < 0)
		aux1_c = frac_c * line1_c[CHORUS_SIZE-d_c+cont_c-1] + (1-frac_c) * line1_c[CHORUS_SIZE-d_c+cont_c];

	d_c = (int) floor(demora_c + 50);
	frac_c = demora_c + 50 - d_c;
	if (cont_c-d_c-1 >= 0)
		aux2_c = frac_c * line2_c[cont_c-d_c-1] + (1-frac_c) * line2_c[cont_c-d_c];
	if (cont_c-d_c == 0)
		aux2_c = frac_c * line2_c[CHORUS_SIZE-1] + (1-frac_c) * line2_c[cont_c-d_c];
	if (cont_c-d_c < 0)
		aux2_c = frac_c * line2_c[CHORUS_SIZE-d_c+cont_c-1] + (1-frac_c) * line2_c[CHORUS_SIZE-d_c+cont_c];

	d_c = (int) floor(demora_c + 100);
	frac_c = demora_c + 100 - d_c;
	if (cont_c-d_c-1 >= 0)
		aux3_c = frac_c * line3_c[cont_c-d_c-1] + (1-frac_c) * line3_c[cont_c-d_c];
	if (cont_c-d_c == 0)
		aux3_c = frac_c * line3_c[CHORUS_SIZE-1] + (1-frac_c) * line3_c[cont_c-d_c];
	if (cont_c-d_c < 0)
		aux3_c = frac_c * line3_c[CHORUS_SIZE-d_c+cont_c-1] + (1-frac_c) * line3_c[CHORUS_SIZE-d_c+cont_c];


	cont_c++;

	salida_c = aux3_c + aux2_c + aux1_c + entrada;
//	salida_c = salida_c;
	return salida_c;
}

float32_t chorusLFO(int modulacion_c)
{
  switch(modulacion_c)
	{
		case CUADRADA:
			if(tiempo_c >= periodo_c/2)
			{
				if(auxcoef_c == min_c)
					auxcoef_c = max_c;
				else
					auxcoef_c = min_c;
				tiempo_c = 0;
			}
			demora_c=lp_c*demora_c+(1-lp_c) * auxcoef_c;
			break;

		case TRIANGULAR:
			if(flag_c == 0)
				demora_c = demora_c + 2*delta_c;
			if(flag_c == 1)
				demora_c = demora_c - 2*delta_c;
			if(demora_c >= max_c || demora_c <= min_c)
			{
				flag_c = 1 - flag_c;
				tiempo_c = 0;
			}
			break;

		case RAMPA_ASC:
			auxcoef_c = auxcoef_c + delta_c;
			if(demora_c >= max_c)
			{
				auxcoef_c = min_c;
				tiempo_c = 0;
			}
			demora_c=lp_c*demora_c+(1-lp_c) * auxcoef_c;
			break;

		case SINUSOIDAL:
			demora_c = manual_c + depth_c * arm_sin_f32((float32_t)(2*3.1416*tiempo_c/periodo_c));
			if(tiempo_c == periodo_c)
				tiempo_c = 0;
			break;
	}
	tiempo_c++;
	return demora_c;
}

void chorusRate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	rate_c = 2 * (db->value);
	chorusParametros();
}

void chorusDepth (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	depth_c = (manual_c - 1) * (db->value);
	chorusParametros();
}

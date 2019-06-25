#include "gui/prototipos.h"
#include "math.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"

// parametros de desarrollador
#define FLANGER_SIZE 300

int salida_f = 0;

// parametros de usuario
float32_t manual_f = 15, rate_f = 0.2, depth_f = 8, feedback_f = -0.95;
int modulacion_f = SINUSOIDAL;

// variables auxiliares
float32_t min_f = 0, max_f = 0, tiempo_f;
float32_t demora_f = FLANGER_SIZE/2 - 1, delta_f = 0;
int periodo_f = 0, flag_f=0;

int cont_f = 0;
float32_t delay_f = 0, frac_f = 0;
float32_t aux1_f = 0, aux2_f = 0, aux3_f = 0, aux4_f = 0, aux5_f = 0;
int d_f = 0;

int line1_f[FLANGER_SIZE];
int line2_f[FLANGER_SIZE];
int line3_f[FLANGER_SIZE];
int line4_f[FLANGER_SIZE];
int line5_f[FLANGER_SIZE];

void flanger_parametros()
{
	demora_f= manual_f;
	min_f = manual_f - depth_f;
	max_f = manual_f + depth_f;
	periodo_f = (int) SR/rate_f;
	delta_f = (max_f - min_f)/periodo_f;
}


int flanger (int entrada)
{
	if (cont_f == FLANGER_SIZE)
		{
			cont_f = 0;
		}
	line1_f[cont_f] = (int)(entrada + aux1_f * feedback_f);
	line2_f[cont_f] = (int)(entrada + aux2_f * feedback_f);
	line3_f[cont_f] = (int)(entrada + aux3_f * feedback_f);
	line4_f[cont_f] = (int)(entrada + aux4_f * feedback_f);
	line5_f[cont_f] = (int)(entrada + aux5_f * feedback_f);

	demora_f = Flanger_LFO(modulacion_f);
	d_f = (int) floor(demora_f);
	frac_f = demora_f - d_f;
	if (cont_f-d_f-1 >= 0)
		aux1_f = frac_f * line1_f[cont_f-d_f-1] + (1-frac_f) * line1_f[cont_f-d_f];
	if (cont_f-d_f == 0)
		aux1_f = frac_f * line1_f[FLANGER_SIZE-1] + (1-frac_f) * line1_f[cont_f-d_f];
	if (cont_f-d_f < 0)
		aux1_f = frac_f * line1_f[FLANGER_SIZE-d_f+cont_f-1] + (1-frac_f) * line1_f[FLANGER_SIZE-d_f+cont_f];

	d_f = (int) floor(demora_f + 10);
	frac_f = demora_f + 10 - d_f;
	if (cont_f-d_f-1 >= 0)
		aux2_f = frac_f * line2_f[cont_f-d_f-1] + (1-frac_f) * line2_f[cont_f-d_f];
	if (cont_f-d_f == 0)
		aux2_f = frac_f * line2_f[FLANGER_SIZE-1] + (1-frac_f) * line2_f[cont_f-d_f];
	if (cont_f-d_f < 0)
		aux2_f = frac_f * line2_f[FLANGER_SIZE-d_f+cont_f-1] + (1-frac_f) * line2_f[FLANGER_SIZE-d_f+cont_f];


	d_f = (int) floor(demora_f + 20);
	frac_f = demora_f + 20 - d_f;
	if (cont_f-d_f-1 >= 0)
		aux3_f = frac_f * line3_f[cont_f-d_f-1] + (1-frac_f) * line3_f[cont_f-d_f];
	if (cont_f-d_f == 0)
		aux3_f = frac_f * line3_f[FLANGER_SIZE-1] + (1-frac_f) * line3_f[cont_f-d_f];
	if (cont_f-d_f < 0)
		aux3_f = frac_f * line3_f[FLANGER_SIZE-d_f+cont_f-1] + (1-frac_f) * line3_f[FLANGER_SIZE-d_f+cont_f];

	d_f = (int) floor(demora_f + 30);
	frac_f = demora_f + 30 - d_f;
	if (cont_f-d_f-1 >= 0)
		aux4_f = frac_f * line4_f[cont_f-d_f-1] + (1-frac_f) * line4_f[cont_f-d_f];
	if (cont_f-d_f == 0)
		aux4_f = frac_f * line4_f[FLANGER_SIZE-1] + (1-frac_f) * line4_f[cont_f-d_f];
	if (cont_f-d_f < 0)
		aux4_f = frac_f * line4_f[FLANGER_SIZE-d_f+cont_f-1] + (1-frac_f) * line4_f[FLANGER_SIZE-d_f+cont_f];

	d_f = (int) floor(demora_f + 40);
	frac_f = demora_f + 40 - d_f;
	if (cont_f-d_f-1 >= 0)
		aux5_f = frac_f * line5_f[cont_f-d_f-1] + (1-frac_f) * line5_f[cont_f-d_f];
	if (cont_f-d_f == 0)
		aux5_f = frac_f * line5_f[FLANGER_SIZE-1] + (1-frac_f) * line5_f[cont_f-d_f];
	if (cont_f-d_f < 0)
		aux5_f = frac_f * line5_f[FLANGER_SIZE-d_f+cont_f-1] + (1-frac_f) * line5_f[FLANGER_SIZE-d_f+cont_f];

	cont_f++;

	salida_f = (int)((aux5_f + aux4_f + aux3_f + aux2_f + aux1_f + entrada)*0.35);
	//salida_f = entrada;
	return salida_f;
}

float32_t Flanger_LFO(int modulacion_f)
{
  switch(modulacion_f)
	{
		case CUADRADA:
			if(tiempo_f == periodo_f/2)
			{
				if(demora_f == min_f)
					demora_f = max_f;
				else
					demora_f = min_f;
				tiempo_f = 0;
			}
			break;

		case TRIANGULAR:
			if(flag_f == 0)
				demora_f = demora_f + 2*delta_f;
			if(flag_f == 1)
				demora_f = demora_f - 2*delta_f;
			if(demora_f >= max_f || demora_f <= min_f)
			{
				flag_f = 1 - flag_f;
				tiempo_f = 0;
			}
			break;

		case RAMPA_ASC:
			demora_f = demora_f + delta_f;
			if(demora_f >= max_f)
			{
				demora_f = min_f;
				tiempo_f = 0;
			}
			break;

		case SINUSOIDAL:
			demora_f = manual_f + depth_f * arm_sin_f32((float32_t)(2*3.1416*tiempo_f/periodo_f));
			if(tiempo_f == periodo_f)
				tiempo_f = 0;
			break;
	}
	tiempo_f++;
	return demora_f;
}



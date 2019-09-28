#include "gui/prototipos.h"
#include "math.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"

// parametros de desarrollador
#define FLANGER_SIZE 300

static int salida = 0;

// parametros de usuario
static float32_t manual = 15, rate = 0.2, depth = 8, feedback = -0.95;
static int modulacion = SINUSOIDAL;

// variables auxiliares
static float32_t min = 0, max = 0, tiempo, demora = FLANGER_SIZE/2 - 1, delta = 0, frac = 0, aux1 = 0, aux2 = 0, aux3 = 0, aux4 = 0, aux5 = 0;
static int periodo = 0, flag=0, cont = 0, d = 0, line1[FLANGER_SIZE], line2[FLANGER_SIZE], line3[FLANGER_SIZE], line4[FLANGER_SIZE], line5[FLANGER_SIZE];

void flangerInit ()
{
	flangerParametros();
}

void flangerParametros ()
{
	demora= manual;
	min = manual - depth;
	max = manual + depth;
	periodo = (int) SR/rate;
	delta = (max - min)/periodo;
}

int flangerEfecto (int entrada)
{
	if (cont == FLANGER_SIZE)
	{
		cont = 0;
	}
	line1[cont] = (int)(entrada + aux1 * feedback);
	line2[cont] = (int)(entrada + aux2 * feedback);
	line3[cont] = (int)(entrada + aux3 * feedback);
	line4[cont] = (int)(entrada + aux4 * feedback);
	line5[cont] = (int)(entrada + aux5 * feedback);

	demora = flangerLFO(modulacion);
	d = (int) floor(demora);
	frac = demora - d;
	if (cont-d-1 >= 0)
		aux1 = frac * line1[cont-d-1] + (1-frac) * line1[cont-d];
	if (cont-d == 0)
		aux1 = frac * line1[FLANGER_SIZE-1] + (1-frac) * line1[cont-d];
	if (cont-d < 0)
		aux1 = frac * line1[FLANGER_SIZE-d+cont-1] + (1-frac) * line1[FLANGER_SIZE-d+cont];

	d = (int) floor(demora + 10);
	frac = demora + 10 - d;
	if (cont-d-1 >= 0)
		aux2 = frac * line2[cont-d-1] + (1-frac) * line2[cont-d];
	if (cont-d == 0)
		aux2 = frac * line2[FLANGER_SIZE-1] + (1-frac) * line2[cont-d];
	if (cont-d < 0)
		aux2 = frac * line2[FLANGER_SIZE-d+cont-1] + (1-frac) * line2[FLANGER_SIZE-d+cont];


	d = (int) floor(demora + 20);
	frac = demora + 20 - d;
	if (cont-d-1 >= 0)
		aux3 = frac * line3[cont-d-1] + (1-frac) * line3[cont-d];
	if (cont-d == 0)
		aux3 = frac * line3[FLANGER_SIZE-1] + (1-frac) * line3[cont-d];
	if (cont-d < 0)
		aux3 = frac * line3[FLANGER_SIZE-d+cont-1] + (1-frac) * line3[FLANGER_SIZE-d+cont];

	d = (int) floor(demora + 30);
	frac = demora + 30 - d;
	if (cont-d-1 >= 0)
		aux4 = frac * line4[cont-d-1] + (1-frac) * line4[cont-d];
	if (cont-d == 0)
		aux4 = frac * line4[FLANGER_SIZE-1] + (1-frac) * line4[cont-d];
	if (cont-d < 0)
		aux4 = frac * line4[FLANGER_SIZE-d+cont-1] + (1-frac) * line4[FLANGER_SIZE-d+cont];

	d = (int) floor(demora + 40);
	frac = demora + 40 - d;
	if (cont-d-1 >= 0)
		aux5 = frac * line5[cont-d-1] + (1-frac) * line5[cont-d];
	if (cont-d == 0)
		aux5 = frac * line5[FLANGER_SIZE-1] + (1-frac) * line5[cont-d];
	if (cont-d < 0)
		aux5 = frac * line5[FLANGER_SIZE-d+cont-1] + (1-frac) * line5[FLANGER_SIZE-d+cont];

	cont++;

	salida = (int)((aux5 + aux4 + aux3 + aux2 + aux1 + entrada)*0.35);
	return salida;
}

float32_t flangerLFO(int modulacion)
{
	switch(modulacion)
	{
	case CUADRADA:
		if(tiempo == periodo/2)
		{
			if(demora == min)
				demora = max;
			else
				demora = min;
			tiempo = 0;
		}
		break;

	case TRIANGULAR:
		if(flag == 0)
			demora = demora + 2*delta;
		if(flag == 1)
			demora = demora - 2*delta;
		if(demora >= max || demora <= min)
		{
			flag = 1 - flag;
			tiempo = 0;
		}
		break;

	case RAMPA_ASC:
		demora = demora + delta;
		if(demora >= max)
		{
			demora = min;
			tiempo = 0;
		}
		break;

	case SINUSOIDAL:
		demora = manual + depth * arm_sin_f32((float32_t)(2*3.1416*tiempo/periodo));
		if(tiempo == periodo)
			tiempo = 0;
		break;
	}
	tiempo++;
	return demora;
}

void flangerManual (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	manual = 35 * (db->value) + 15;
	flangerParametros();
}

void flangerDepth (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	depth = (15 - 1) * (db->value);
	flangerParametros();
}

void flangerRate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	rate = 1 * (db->value);
	flangerParametros();
}

void flangerFeedback (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	rate = - 0.185 * (db->value) - 0.8;
}

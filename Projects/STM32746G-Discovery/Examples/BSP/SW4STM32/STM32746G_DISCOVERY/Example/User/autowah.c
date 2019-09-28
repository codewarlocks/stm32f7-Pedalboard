#include "math.h"
#include "gui/prototipos.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"

#define ENV_SIZE 10
static int salida=0;

// instancias de las variables de entrada y salida
static int x_0 = 0, x_1 = 0, x_2 = 0, y_0 = 0, y_1 = 0, y_2 = 0;

// parametros de usuario
static float32_t rate = 2, depth = 500, volume = 0.6;
static int modulacion = SINUSOIDAL;

// parametros de desarrollador
static float32_t fmedia = 800, fb = 100;

// parametros del filtro
static float32_t c = 0, d = 0;

// variables auxiliares
static float32_t finicial = 0, ffinal = 0, fcentral = 0, deltaf = 0, rateenv=0, periodoenv = 0, deltafenv = 0, auxcoef=0, lp=0.99;
static int periodo = 0, cont = 0, flag = 0;

void autowahInit()
{
	fcentral = fmedia;
	c = (tan(3.1416*fb/SR)-1)/(tan(2*3.1416*fb/SR)+1);
	d = -cos(2*3.1416*fcentral/SR);
	autowahParametros();
}

void autowahParametros()
{
	finicial = fmedia - depth;
	ffinal = fmedia + depth;
	periodo = (int) SR/rate;
	deltaf = (ffinal - finicial)/periodo;
}

int autowahEfecto(int entrada)
{
	fcentral = autowahLFO(modulacion);
	d = -arm_cos_f32((float32_t)(2*3.1416*fcentral/SR));
	x_2 = x_1;
	x_1 = x_0;
	x_0 = entrada;
	y_2 = y_1;
	y_1 = y_0;
	y_0 = (1 + c) * 0.5 * x_0 - (1 + c) * 0.5 * x_2 - d * (1 - c) * y_1 + c * y_2;
	salida = 2.5 * (volume * y_0 + (1 - volume) * x_0);
	return salida;
}

float32_t autowahLFO(int modulacion)
{
	switch(modulacion)
	{
	case CUADRADA:
		if(cont >= periodo/2)
		{
			if(auxcoef == finicial)
				auxcoef = ffinal;
			else
				auxcoef = finicial;
			cont = 0;
		}
		fcentral=lp*fcentral+(1-lp) * auxcoef;
		break;

	case TRIANGULAR:
		if(flag == 0)
			fcentral=fcentral + 2*deltaf;
		if(flag == 1)
			fcentral=fcentral - 2*deltaf;
		if(fcentral >= ffinal || fcentral <= finicial)
		{
			flag = 1 - flag;
			cont = 0;
		}
		break;

	case RAMPA_ASC:
		auxcoef = auxcoef + deltaf;
		if(auxcoef >= ffinal)
		{
			auxcoef = finicial;
			cont = 0;
		}
		fcentral=lp*fcentral+(1-lp) * auxcoef;
		break;

	case RAMPA_DESC:
		auxcoef = auxcoef - deltaf;
		if(auxcoef <= finicial)
		{
			auxcoef = ffinal;
			cont = 0;
		}
		fcentral=lp*fcentral+(1-lp) * auxcoef;
		break;

	case SINUSOIDAL:
		fcentral = fmedia + (ffinal - finicial) * arm_sin_f32((float32_t)(2*3.1416*cont/periodo));
		if(cont == periodo)
			cont = 0;
		break;

	case ENVOLVENTE:
		rateenv = x_0/10000;
		rateenv = rateenv*rateenv;
		rateenv = ((8 * rate - 1000 * rateenv) > rate) ? (8 * rate - 1000 * rateenv) : rate;
		periodoenv = (float32_t) SR/rateenv;
		deltafenv = (ffinal - finicial)/periodoenv;
		if(flag == 0)
			fcentral = fcentral + 2 * deltafenv;
		if(flag == 1)
			fcentral = fcentral - 2 * deltafenv;
		if(fcentral >= ffinal || fcentral <= finicial)
			flag = 1 - flag;
		break;
	}
	cont++;
	return fcentral;
}

void autowahRate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	rate = 6 * (db->value);
	autowahParametros();
}

void autowahDepth (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	depth = (fmedia - 100) * (db->value);
	autowahParametros();
}

void autowahVolume (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	volume = (db->value);
}

void autowahMod (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	if (db->value <= 0.2)
		modulacion = CUADRADA;
	else if(db->value > 0.2 && db->value <= 0.4)
		modulacion = TRIANGULAR;
	else if(db->value >0.4 && db->value <= 0.6)
		modulacion = RAMPA_ASC;
	else if(db->value > 0.6 && db->value <= 0.8)
		modulacion = RAMPA_DESC;
	else if(db->value > 0.8 && db->value < 1)
		modulacion = SINUSOIDAL;
	else if(db->value == 1)
		modulacion = ENVOLVENTE;
}


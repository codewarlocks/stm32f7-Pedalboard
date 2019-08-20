#include "math.h"
#include "gui/prototipos.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"

#define ENV_SIZE 10
int salida_aw=0;

// instancias de las variables de entrada y salida_aw
int x0_aw = 0, x1_aw = 0, x2_aw = 0;
int y0_aw = 0, y1_aw = 0, y2_aw = 0;

// parametros de usuario
float rate_aw = 2, depth_aw = 500, volume_aw = 0.6;
int modulacion_aw = SINUSOIDAL;

// parametros de desarrollador
float fmedia_aw = 800, fb_aw = 100;

// parametros del filtro
float c_aw = 0, d_aw = 0;

// variables auxiliares
float32_t finicial_aw = 0, ffinal_aw = 0, fcentral_aw = 0;
int periodo_aw = 0;
float32_t deltaf_aw = 0;

float32_t rateenv_aw=0, periodoenv_aw = 0, deltafenv_aw = 0;

int cont_aw = 0, flag_aw = 0, i_aw = 0;
float32_t buffer_aw[ENV_SIZE]={0};
float32_t envelope_aw = 0;
char texto[20];

float32_t auxcoef_w=0, lp_w=0.99;

void autowahInit()
{
	fcentral_aw = fmedia_aw;
	c_aw = (tan(3.1416*fb_aw/SR)-1)/(tan(2*3.1416*fb_aw/SR)+1);
	d_aw = -cos(2*3.1416*fcentral_aw/SR);
	autowahParametros();
}

void autowahParametros()
{
	finicial_aw = fmedia_aw - depth_aw;
	ffinal_aw = fmedia_aw + depth_aw;
	periodo_aw = (int) SR/rate_aw;
	deltaf_aw = (ffinal_aw - finicial_aw)/periodo_aw;
}

int autowahEfecto(int entrada)
{
	fcentral_aw = autowahLFO(modulacion_aw);
	d_aw = -arm_cos_f32((float32_t)(2*3.1416*fcentral_aw/SR));
	x2_aw = x1_aw;
	x1_aw = x0_aw;
	x0_aw = entrada;
	y2_aw = y1_aw;
	y1_aw = y0_aw;
	y0_aw = (1 + c_aw) * 0.5 * x0_aw - (1 + c_aw) * 0.5 * x2_aw - d_aw * (1 - c_aw) * y1_aw + c_aw * y2_aw;
	salida_aw = 2.5 * (volume_aw * y0_aw + (1 - volume_aw) * x0_aw);
	return salida_aw;
}

float32_t autowahLFO(int modulacion_aw)
{
	switch(modulacion_aw)
	{
	case CUADRADA:
		if(cont_aw >= periodo_aw/2)
		{
			if(auxcoef_w == finicial_aw)
				auxcoef_w = ffinal_aw;
			else
				auxcoef_w = finicial_aw;
			cont_aw = 0;
		}
		fcentral_aw=lp_w*fcentral_aw+(1-lp_w) * auxcoef_w;
		break;

	case TRIANGULAR:
		if(flag_aw == 0)
			fcentral_aw=fcentral_aw + 2*deltaf_aw;
		if(flag_aw == 1)
			fcentral_aw=fcentral_aw - 2*deltaf_aw;
		if(fcentral_aw >= ffinal_aw || fcentral_aw <= finicial_aw)
		{
			flag_aw = 1 - flag_aw;
			cont_aw = 0;
		}
		break;

	case RAMPA_ASC:
		auxcoef_w = auxcoef_w + deltaf_aw;
		if(auxcoef_w >= ffinal_aw)
		{
			auxcoef_w = finicial_aw;
			cont_aw = 0;
		}
		fcentral_aw=lp_w*fcentral_aw+(1-lp_w) * auxcoef_w;
		break;

	case RAMPA_DESC:
		auxcoef_w = auxcoef_w - deltaf_aw;
		if(auxcoef_w <= finicial_aw)
		{
			auxcoef_w = ffinal_aw;
			cont_aw = 0;
		}
		fcentral_aw=lp_w*fcentral_aw+(1-lp_w) * auxcoef_w;
		break;

	case SINUSOIDAL:
		fcentral_aw = fmedia_aw + (ffinal_aw - finicial_aw) * arm_sin_f32((float32_t)(2*3.1416*cont_aw/periodo_aw));
		if(cont_aw == periodo_aw)
			cont_aw = 0;
		break;

	case ENVOLVENTE:
		rateenv_aw = x0_aw/10000;
		rateenv_aw = rateenv_aw*rateenv_aw;
		rateenv_aw = ((8 * rate_aw - 1000 * rateenv_aw) > rate_aw) ? (8 * rate_aw - 1000 * rateenv_aw) : rate_aw;
		periodoenv_aw = (float32_t) SR/rateenv_aw;
		deltafenv_aw = (ffinal_aw - finicial_aw)/periodoenv_aw;
		if(flag_aw == 0)
			fcentral_aw = fcentral_aw + 2 * deltafenv_aw;
		if(flag_aw == 1)
			fcentral_aw = fcentral_aw - 2 * deltafenv_aw;
		if(fcentral_aw >= ffinal_aw || fcentral_aw <= finicial_aw)
			flag_aw = 1 - flag_aw;
		break;
	}
	cont_aw++;
	return fcentral_aw;
}

void autowahRate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	rate_aw = 6 * (db->value);
	autowahParametros();
}

void autowahDepth (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	depth_aw = (fmedia_aw - 100) * (db->value);
	autowahParametros();
}

void autowahVolume (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	volume_aw = (db->value);
}

void autowahMod (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	if (db->value <= 0.2)
		modulacion_aw = CUADRADA;
	else if(db->value > 0.2 && db->value <= 0.4)
		modulacion_aw = TRIANGULAR;
	else if(db->value >0.4 && db->value <= 0.6)
		modulacion_aw = RAMPA_ASC;
	else if(db->value > 0.6 && db->value <= 0.8)
		modulacion_aw = RAMPA_DESC;
	else if(db->value > 0.8 && db->value < 1)
		modulacion_aw = SINUSOIDAL;
	else if(db->value == 1)
		modulacion_aw = ENVOLVENTE;
}


// parametros de desarrollador
#include "math.h"
#include "gui/prototipos.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"

#define ENV_SIZE 10
int salida_aw=0;

// instancias de las variables de entrada y salida_aw
int x_0_aw = 0, x_1_aw = 0, x_2_aw = 0;
int y_0_aw = 0, y_1_aw = 0, y_2_aw = 0;

// parametros de usuario
float rate_aw = 2, depth_aw = 500, volume_aw = 0.6;
int modulacion_aw = SINUSOIDAL, state_aw = 0;

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

void init_autowah()
{
	fcentral_aw = fmedia_aw;
	c_aw = (tan(3.1416*fb_aw/SR)-1)/(tan(2*3.1416*fb_aw/SR)+1);
	d_aw = -cos(2*3.1416*fcentral_aw/SR);
	autowah_parametros();
}

void autowah_parametros()
{
	finicial_aw = fmedia_aw - depth_aw;
	ffinal_aw = fmedia_aw + depth_aw;
	periodo_aw = (int) SR/rate_aw;
	deltaf_aw = (ffinal_aw - finicial_aw)/periodo_aw;
}

int autowah(int entrada)
{
	fcentral_aw = Autowah_LFO(modulacion_aw);
	d_aw = -arm_cos_f32((float32_t)(2*3.1416*fcentral_aw/SR));
	x_2_aw = x_1_aw;
	x_1_aw = x_0_aw;
	x_0_aw = entrada;
	y_2_aw = y_1_aw;
	y_1_aw = y_0_aw;
	y_0_aw = (1+c_aw) * 0.5 * x_0_aw - (1+c_aw) * 0.5 * x_2_aw - d_aw * (1-c_aw) * y_1_aw + c_aw * y_2_aw;
	salida_aw = 2.5 * (volume_aw * y_0_aw + (1-volume_aw) * x_0_aw);
	return salida_aw;
}

float32_t Autowah_LFO(int modulacion_aw)
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
            rateenv_aw = x_0_aw/10000;
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

void Autowah_Rate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
    rate_aw = 6 * (db->value);
	autowah_parametros();
}

void Autowah_Depth (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
    depth_aw = (fmedia_aw - 100) * (db->value);
	autowah_parametros();
}

void Autowah_Volume (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
    volume_aw = (db->value);
}

void Autowah_Mod (GUIElement *e)
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

void Push_State_Autowah (GUIElement *e)
{
	state_aw = e->state;
}

int Get_State_Autowah (void)
{
	 return state_aw;
}

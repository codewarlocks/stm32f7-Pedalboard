//Phaser.c
#include "gui/prototipos.h"
#include "math.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"
#define N 10

int salida_ph = 0;

// instancias de las variables de entrada y salida_ph
int x1_ph[4],x0_ph[4];
int y1_ph[4],y0_ph[4];

// parametros de usuario
float rate_ph = 3;
int state_ph = 0;

// parametros de desarrollador
float fmedia_ph = 1000, depth_ph = 700, volume_ph = 0.5;
int modulacion_ph = TRIANGULAR;

// parametros del filtro
float c_ph = 0;

// variables auxiliares
float finicial_ph = 0, ffinal_ph = 0, fcentral_ph = 1000;
int periodo_ph = 0;
float deltaf_ph = 0;

int cont_ph = 0, flag_ph = 0;

void phaser_parametros()
{
	finicial_ph = fmedia_ph - depth_ph;
	ffinal_ph = fmedia_ph + depth_ph;
	periodo_ph = (int) SR/rate_ph;
	deltaf_ph = (ffinal_ph - finicial_ph)/periodo_ph;
}

int phaser(int entrada)
{
  fcentral_ph = Phaser_LFO(modulacion_ph);
  c_ph = (1-tan(3.1416*fcentral_ph/SR))/(1+tan(3.1416*fcentral_ph/SR));
  salida_ph = Phaser_AP1(entrada,0);
  salida_ph = Phaser_AP1(salida_ph,1);
  salida_ph = Phaser_AP1(salida_ph,2);
  salida_ph = Phaser_AP1(salida_ph,3);
  salida_ph = volume_ph * salida_ph + (1-volume_ph) * entrada;
  return salida_ph;
}

int Phaser_AP1(int in, int i)
{
  x1_ph[i] = x0_ph[i];
  x0_ph[i] = in;
  y1_ph[i] = y0_ph[i];
  y0_ph[i] = c_ph * x0_ph[i] - x1_ph[i] + c_ph * y1_ph[i];
  return y0_ph[i];
}

float Phaser_LFO(int modulacion_ph)
{
  if(modulacion_ph == TRIANGULAR)
  {
    if(flag_ph == 0)
      fcentral_ph = fcentral_ph + 2 * deltaf_ph;
    if(flag_ph == 1)
      fcentral_ph = fcentral_ph - 2 * deltaf_ph;
    if(fcentral_ph >= ffinal_ph || fcentral_ph <= finicial_ph)
    {
      flag_ph = 1 - flag_ph;
      cont_ph = 0;
    }
  }
  //cont_ph++;
	return fcentral_ph;
}

void Phaser_Rate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
  rate_ph = 4 * (db->value);
	phaser_parametros();
}

void Push_State_Phaser (GUIElement *e)
{
	state_ph = e->state;
}

int Get_State_Phaser (void)
{
	return state_ph;
}

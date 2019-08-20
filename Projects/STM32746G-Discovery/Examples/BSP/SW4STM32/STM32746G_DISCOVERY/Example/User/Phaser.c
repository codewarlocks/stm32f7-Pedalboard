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
float rate_ph = 4;
int state_ph = 0;

// parametros de desarrollador
float fmedia_ph = 1000, depth_ph = 700;
int modulacion_ph = TRIANGULAR;

// parametros del filtro
float c_ph = 0;

// variables auxiliares
float finicial_ph = 0, ffinal_ph = 0, fcentral_ph = 1000;
int periodo_ph = 0;
float deltaf_ph = 0, aux_ph = 0;

int flag_ph = 0;

void phaserInit()
{
	phaserParametros();
}

void phaserParametros()
{
	finicial_ph = fmedia_ph - depth_ph;
	ffinal_ph = fmedia_ph + depth_ph;
	periodo_ph = (int) SR/rate_ph;
	deltaf_ph = (ffinal_ph - finicial_ph)/periodo_ph;
}

int phaserEfecto(int entrada)
{
  fcentral_ph = phaserLFO(modulacion_ph);
  aux_ph = tan(3.1416*fcentral_ph/SR);
  c_ph = (1-aux_ph)/(1+aux_ph);
  salida_ph = phaserAP1(entrada,0);
  salida_ph = phaserAP1(salida_ph,1);
  salida_ph = phaserAP1(salida_ph,2);
  salida_ph = phaserAP1(salida_ph,3);
  salida_ph = 0.5 * salida_ph + 0.5 * entrada + 1 - 1;
  return salida_ph;
}

int phaserAP1(int in, int i)
{
  x1_ph[i] = x0_ph[i];
  x0_ph[i] = in;
  y1_ph[i] = y0_ph[i];
  y0_ph[i] = c_ph * x0_ph[i] - x1_ph[i] + c_ph * y1_ph[i];
  return y0_ph[i];
}

float phaserLFO(int modulacion_ph)
{
  if(modulacion_ph == TRIANGULAR)
  {
    if(flag_ph == 0)
      fcentral_ph = fcentral_ph + 2 * deltaf_ph;
    if(flag_ph == 1)
      fcentral_ph = fcentral_ph - 2 * deltaf_ph;
    if(fcentral_ph >= ffinal_ph || fcentral_ph <= finicial_ph)
      flag_ph = 1 - flag_ph;
  }
	return fcentral_ph;
}

void phaserRate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	rate_ph = 6 * (db->value);
	phaserParametros();
}

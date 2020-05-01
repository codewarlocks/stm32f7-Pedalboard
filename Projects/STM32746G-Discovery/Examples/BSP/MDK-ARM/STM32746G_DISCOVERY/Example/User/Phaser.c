//Phaser.c
#include "gui/prototipos.h"
#include "math.h"

#include "arm_math.h"
#include "arm_common_tables.h"
#define N 10

static int32_t salida = 0;

// instancias de las variables de entrada y salida
static int32_t x_1[4]={0}, x_0[4]={0}, y_1[4]={0},y_0[4]={0};

// parametros de usuario
static float32_t rate = 4.0f;

// parametros de desarrollador
static float32_t fmedia = 1000.0f, depth = 700.0f;
static int32_t modulacion = TRIANGULAR;

// parametros del filtro
static float32_t c = 0.0f;

// variables auxiliares
static float32_t finicial = 0.0f, ffinal = 0.0f, fcentral = 1000.0f, deltaf = 0.0f, aux = 0.0f;
static int32_t periodo = 0, flag = 0;

void phaserInit()
{
	phaserParametros();
}

void phaserParametros()
{
	finicial = fmedia - depth;
	ffinal = fmedia + depth;
	periodo = (int32_t) SR/rate;
	deltaf = (ffinal - finicial)/periodo;
}

int32_t phaserEfecto(int32_t entrada)
{
	fcentral = phaserLFO(modulacion);
	
  aux = tanf((float32_t)PI*fcentral/SR);
	
//	aux1=(float32_t)PI*fcentral/SR;
//	
//	aux=arm_sin_f32(aux1)/arm_cos_f32(aux1);
	
  c = (1-aux)/(1+aux);
  salida = phaserAP1(entrada,0);
  salida = phaserAP1(salida,1);
  salida = phaserAP1(salida,2);
  salida = phaserAP1(salida,3);
  salida = (int32_t)(((float32_t)0.5f *salida) + ((float32_t)0.5f *entrada));
  return salida;
}

int32_t phaserAP1(int32_t in, int32_t i)
{
  x_1[i] = x_0[i];
  x_0[i] = in;
  y_1[i] = y_0[i];
  y_0[i] = c * x_0[i] - x_1[i] + c * y_1[i];
  return y_0[i];
}

float32_t phaserLFO(int32_t modulacion)
{
  if(modulacion == TRIANGULAR)
  {
    if(flag == 0)
      fcentral = fcentral + 2.0f * deltaf;
    if(flag == 1)
      fcentral = fcentral - 2.0f * deltaf;
    if(fcentral >= ffinal || fcentral <= finicial)
      flag = 1 - flag;
  }
	return fcentral;
}

void phaserRate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	rate = 6.0f * (db->value);
	phaserParametros();
}

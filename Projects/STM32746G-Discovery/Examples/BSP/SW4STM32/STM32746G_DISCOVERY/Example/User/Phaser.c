//Phaser.c
#include "gui/prototipos.h"
#include "math.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"
#define N 10

static int salida = 0;

// instancias de las variables de entrada y salida
static int x_1[4], x_0[4], y_1[4],y_0[4];

// parametros de usuario
static float32_t rate = 4;

// parametros de desarrollador
static float32_t fmedia = 1000, depth = 700;
static int modulacion = TRIANGULAR;

// parametros del filtro
static float32_t c = 0;

// variables auxiliares
static float32_t finicial = 0, ffinal = 0, fcentral = 1000, deltaf = 0, aux = 0;
static int periodo = 0, flag = 0;

void phaserInit()
{
	phaserParametros();
}

void phaserParametros()
{
	finicial = fmedia - depth;
	ffinal = fmedia + depth;
	periodo = (int) SR/rate;
	deltaf = (ffinal - finicial)/periodo;
}

int phaserEfecto(int entrada)
{
  fcentral = phaserLFO(modulacion);
  aux = tan(3.1416*fcentral/SR);
  c = (1-aux)/(1+aux);
  salida = phaserAP1(entrada,0);
  salida = phaserAP1(salida,1);
  salida = phaserAP1(salida,2);
  salida = phaserAP1(salida,3);
  salida = (int)((0.5 *salida) + (0.5 *entrada));
  return salida;
}

int phaserAP1(int in, int i)
{
  x_1[i] = x_0[i];
  x_0[i] = in;
  y_1[i] = y_0[i];
  y_0[i] = c * x_0[i] - x_1[i] + c * y_1[i];
  return y_0[i];
}

float phaserLFO(int modulacion)
{
  if(modulacion == TRIANGULAR)
  {
    if(flag == 0)
      fcentral = fcentral + 2 * deltaf;
    if(flag == 1)
      fcentral = fcentral - 2 * deltaf;
    if(fcentral >= ffinal || fcentral <= finicial)
      flag = 1 - flag;
  }
	return fcentral;
}

void phaserRate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	rate = 6 * (db->value);
	phaserParametros();
}

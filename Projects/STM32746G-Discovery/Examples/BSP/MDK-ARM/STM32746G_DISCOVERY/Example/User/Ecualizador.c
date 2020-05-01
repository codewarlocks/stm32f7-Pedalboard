//Ecualizador.c
#include "gui/prototipos.h"
#include "math.h"

#include "arm_math.h"
#include "arm_common_tables.h"
#define N 6

static int32_t salida = 0;

// instanfcias de las variables de entrada y salida
static int32_t x_2[N]={0},x_1[N]={0},x_0[N]={0}, y_2[N]={0},y_1[N]={0},y_0[N]={0};

// parametros de usuario
static float32_t Vo[N]={0};

// parametros de desarrollador
static float32_t fc[N]={0},fb[N]={0};

// parametros de los filtros
static float32_t d[N]={0},c[N]={0};

static int32_t i = 0;

void eqInit()
{
	//frecuencias de corte
	fc[0] = 100; fc[1] = 200;  fc[2] = 400;
	fc[3] = 800; fc[4] = 1600; fc[5] = 3200;
	//anchos de banda
	for(i = 1; i < N-1; i++)
		fb[i] = 20.0f * pow(2, i-1);
	//ganancias
	Vo[0] = 1; Vo[1] = 1; Vo[2] = 1;
	Vo[3] = 1; Vo[4] = 1; Vo[5] = 1;
	eqParametros();
}

void eqParametros()
{
	//parametros low shelve
	if(Vo[0] >= 1)
		c[0] = (1.0f-tanf(PI*fc[0]/SR))/(1+tanf(PI*fc[0]/SR));
	else
		c[0] = (Vo[0]-tanf(PI*fc[0]/SR))/(Vo[0]+tanf(PI*fc[0]/SR));

	//parametros peak
	for(i = 1; i < N-1; i++)
	{
		d[i] = -cos(2.0f*PI*fc[i]/SR);
		if(Vo[i] >= 1)
			c[i] = (1.0f-tanf(PI*fb[i]/SR))/(1.0f+tanf(PI*fb[i]/SR));
		else
			c[i] = (Vo[i]-tanf(PI*fb[i]/SR))/(Vo[i]+tanf(PI*fb[i]/SR));
	}

	//parametros high shelve
	if(Vo[N-1] >= 1)
		c[N-1] = (1.0f-tanf(PI*fc[N-1]/SR))/(1.0f+tanf(PI*fc[N-1]/SR));
	else
		c[N-1] = (1.0f-Vo[N-1]*tanf(PI*fc[N-1]/SR))/(1.0f+Vo[N-1]*tanf(PI*fc[N-1]/SR));
}

int32_t eqEfecto(int32_t entrada)
{
	salida = eqShelve(entrada,0);
	for(int32_t i = 1; i < N-1; i++)
		salida = eqPeak(salida,i);
	salida = eqShelve(salida,N-1);
	return salida;
}

int32_t eqPeak(int32_t in, int32_t i)
{
	x_2[i] = x_1[i];
	x_1[i] = x_0[i];
	x_0[i] = in;
	y_2[i] = y_1[i];
	y_1[i] = y_0[i];
	y_0[i] = c[i] * x_0[i] + d[i] * (1 + c[i]) * x_1[i] + x_2[i] - d[i] * (1 + c[i]) * y_1[i] - c[i] * y_2[i];
	in = (float32_t)0.5f * (Vo[i] - 1) * (in - y_0[i]) + in;
	return in;
}

int32_t eqShelve(int32_t in, int32_t i)
{
	x_1[i] = x_0[i];
	x_0[i] = in;
	y_1[i] = y_0[i];
	y_0[i] = c[i] * x_0[i] - x_1[i] + c[i] * y_1[i];
	if(i == 0)
		in = (float32_t)0.5f * (Vo[i] - 1) * (in - y_0[i]) + in;
	else
		in = (float32_t)0.5f * (Vo[i] - 1) * (in + y_0[i]) + in;
	return in;
}

void eqGain0 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	Vo[0] = pow(10,(12.0f-24.0f*db->value)/20.0f);
	eqParametros();
}

void eqGain1 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	Vo[1] = pow(10,(12.0f-24.0f*db->value)/20.0f);
	eqParametros();
}

void eqGain2 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	Vo[2] = pow(10,(12.0f-24.0f*db->value)/20.0f);
	eqParametros();
}

void eqGain3 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	Vo[3] = pow(10,(12.0f-24.0f*db->value)/20.0f);
	eqParametros();
}

void eqGain4 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	Vo[4] = pow(10,(12.0f-24.0f*db->value)/20.0f);
	eqParametros();
}

void eqGain5 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	Vo[5] = pow(10,(12.0f-24.0f*db->value)/20.0f);
	eqParametros();
}


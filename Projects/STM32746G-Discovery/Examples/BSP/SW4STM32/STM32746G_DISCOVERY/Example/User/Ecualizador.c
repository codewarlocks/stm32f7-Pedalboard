//Ecualizador.c
#include "gui/prototipos.h"
#include "math.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"
#define N 6

static int salida = 0;

// instancias de las variables de entrada y salida
static int x_2[N],x_1[N],x_0[N], y_2[N],y_1[N],y_0[N];

// parametros de usuario
static float32_t Vo[N];

// parametros de desarrollador
static float32_t fc[N],fb[N];

// parametros de los filtros
static float32_t d[N],c[N];

static int i = 0;

void eqInit()
{
	//frecuencias de corte
	fc[0] = 100; fc[1] = 200;  fc[2] = 400;
	fc[3] = 800; fc[4] = 1600; fc[5] = 3200;
	//anchos de banda
	for(i = 1; i < N-1; i++)
		fb[i] = 20 * pow(2, i-1);
	//ganancias
	Vo[0] = 1; Vo[1] = 1; Vo[2] = 1;
	Vo[3] = 1; Vo[4] = 1; Vo[5] = 1;
	eqParametros();
}

void eqParametros()
{
	//parametros low shelve
	if(Vo[0] >= 1)
		c[0] = (1-tan(3.1416*fc[0]/SR))/(1+tan(3.1416*fc[0]/SR));
	else
		c[0] = (Vo[0]-tan(3.1416*fc[0]/SR))/(Vo[0]+tan(3.1416*fc[0]/SR));

	//parametros peak
	for(i = 1; i < N-1; i++)
	{
		d[i] = -cos(2*3.1416*fc[i]/SR);
		if(Vo[i] >= 1)
			c[i] = (1-tan(3.1416*fb[i]/SR))/(1+tan(3.1416*fb[i]/SR));
		else
			c[i] = (Vo[i]-tan(3.1416*fb[i]/SR))/(Vo[i]+tan(3.1416*fb[i]/SR));
	}

	//parametros high shelve
	if(Vo[N-1] >= 1)
		c[N-1] = (1-tan(3.1416*fc[N-1]/SR))/(1+tan(3.1416*fc[N-1]/SR));
	else
		c[N-1] = (1-Vo[N-1]*tan(3.1416*fc[N-1]/SR))/(1+Vo[N-1]*tan(3.1416*fc[N-1]/SR));
}

int eqEfecto(int entrada)
{
	salida = eqShelve(entrada,0);
	for(int i = 1; i < N-1; i++)
		salida = eqPeak(salida,i);
	salida = eqShelve(salida,N-1);
	return salida;
}

int eqPeak(int in, int i)
{
	x_2[i] = x_1[i];
	x_1[i] = x_0[i];
	x_0[i] = in;
	y_2[i] = y_1[i];
	y_1[i] = y_0[i];
	y_0[i] = c[i] * x_0[i] + d[i] * (1 + c[i]) * x_1[i] + x_2[i] - d[i] * (1 + c[i]) * y_1[i] - c[i] * y_2[i];
	in = 0.5 * (Vo[i] - 1) * (in - y_0[i]) + in;
	return in;
}

int eqShelve(int in, int i)
{
	x_1[i] = x_0[i];
	x_0[i] = in;
	y_1[i] = y_0[i];
	y_0[i] = c[i] * x_0[i] - x_1[i] + c[i] * y_1[i];
	if(i == 0)
		in = 0.5 * (Vo[i] - 1) * (in - y_0[i]) + in;
	else
		in = 0.5 * (Vo[i] - 1) * (in + y_0[i]) + in;
	return in;
}

void eqGain0 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	Vo[0] = pow(10,(12-24*db->value)/20);
	eqParametros();
}

void eqGain1 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	Vo[1] = pow(10,(12-24*db->value)/20);
	eqParametros();
}

void eqGain2 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	Vo[2] = pow(10,(12-24*db->value)/20);
	eqParametros();
}

void eqGain3 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	Vo[3] = pow(10,(12-24*db->value)/20);
	eqParametros();
}

void eqGain4 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	Vo[4] = pow(10,(12-24*db->value)/20);
	eqParametros();
}

void eqGain5 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	Vo[5] = pow(10,(12-24*db->value)/20);
	eqParametros();
}


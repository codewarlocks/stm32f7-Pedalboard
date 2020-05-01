#include "gui/prototipos.h"
#include "math.h"

#include "arm_math.h"
#include "arm_common_tables.h"

static int32_t salida = 0;

// parametros de usuario
static float32_t rate = 440.0f;

static int x_1=0,x_0=0, y_1=0, y_0=0;
static float32_t Vo = 1.0f, fc = 20, c=0;

//variables auxiliares
static int32_t cont = 0;

void ringmodInit ()
{
	ringmodParametros();
	
	//parametros low shelve
	if(Vo >= 1)
	c = (1-tanf((float32_t)PI*fc/SR))/(1+tanf((float32_t)PI*fc/SR));
	else
	c = (Vo-tanf((float32_t)PI*fc/SR))/(Vo+tanf((float32_t)PI*fc/SR));
}

void ringmodParametros ()
{

}

int32_t ringmodeShelve(int32_t in)
{
	x_1 = x_0;
	x_0 = in;
	y_1 = y_0;
	y_0 = c * x_0 - x_1 + c * y_1;
	in = (float32_t)0.5f * (Vo - 1.0f) * (in - y_0) + in;
	return in;
}

int32_t ringmodEfecto(int32_t entrada)
{
	if(cont == SR)
	{
		cont = 0;
	}
	
	entrada=ringmodeShelve(entrada);
	
	salida = entrada * arm_sin_f32((float32_t)((2.0f*PI*rate*cont)/SR));
	cont++;
	return salida;
}

void ringmodRate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	rate = 100.0f + 1900.0f * (db->value);
}

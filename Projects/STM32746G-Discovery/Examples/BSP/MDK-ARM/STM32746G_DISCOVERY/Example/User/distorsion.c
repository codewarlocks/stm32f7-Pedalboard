
#include "math.h"
#include "gui/prototipos.h"

static int32_t salida = 0;

// variables auxiliares
static float32_t aux=0;

// parametros de usuario
static float32_t gain = 70.0f, blend = 0.5f;

// variables de filtrado
static int x_1[2]={0},x_0[2]={0}, y_1[2]={0}, y_0[2]={0};
static float32_t Vo[2] = {0.25f , 4}, fc[2] = {700 , 700}, c[2]={0};

void distorsionInit ()
{
    //parametros low shelve
    if(Vo[0] >= 1)
	  c[0] = (1-tanf((float32_t)PI*fc[0]/SR))/(1+tanf((float32_t)PI*fc[0]/SR));
    else
	  c[0] = (Vo[0]-tanf((float32_t)PI*fc[0]/SR))/(Vo[0]+tanf((float32_t)PI*fc[0]/SR));
    distorsionParametros();
}

void distorsionParametros()
{

}

int32_t distorsionEfecto (int32_t entrada)
{
	// de-emphasis de graves
	aux = (float32_t) distorsionShelve(entrada,0);

	// escalar de -gain a +gain
//	aux = (gain * aux)/(float32_t)8388607.0;
	aux =(float32_t) ((gain*aux)/8388607.0f);
//	
//	// waveshaper
//	arm_sqrt_f32(1.0f + aux * aux, &tan_replace);
//	aux = (aux / tan_replace);
	
	aux = (2/PI)*atanf(aux);
	
	// reescalado
	aux = (aux * 3106891.5f); //para que no suba tanto el volumen
	
	// refuerzo de graves
	aux = (float32_t) distorsionShelve(aux,1);

	//control de volumen
	//aux = 1.5 * aux / gain;

	// blend con la entrada
	salida = (int32_t)((blend * aux) + ((1.0f - blend) * entrada));

	return  salida;
}

int distorsionShelve(int32_t in, int32_t i_ds)
{
	x_1[i_ds] = x_0[i_ds];
	x_0[i_ds] = in;
	y_1[i_ds] = y_0[i_ds];
	y_0[i_ds] = c[i_ds] * x_0[i_ds] - x_1[i_ds] + c[i_ds] * y_1[i_ds];
	in = (float32_t)0.5f * (Vo[i_ds] - 1.0f) * (in - y_0[i_ds]) + in;
	return in;
}

void distorsionGain (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	gain = (float32_t)pow(10 , 3 * (db->value));
}

void distorsionBlend (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	blend = (db->value);
}

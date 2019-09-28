
#include "math.h"
#include "gui/prototipos.h"

static int salida = 0;

// variables auxiliares
static float32_t aux=0;

// parametros de usuario
static float32_t gain = 70.0, blend = 0.5;

// variables de filtrado
static int x_1[2],x_0[2], y_1[2], y_0[2];
static float32_t Vo[2] = {0.25 , 4}, fc[2] = {700 , 700}, c[2];

void distorsionInit ()
{
    //parametros low shelve
    if(Vo[0] >= 1)
	  c[0] = (1-tan(3.1416*fc[0]/SR))/(1+tan(3.1416*fc[0]/SR));
    else
	  c[0] = (Vo[0]-tan(3.1416*fc[0]/SR))/(Vo[0]+tan(3.1416*fc[0]/SR));
    distorsionParametros();
}

void distorsionParametros()
{

}

int distorsionEfecto (int entrada)
{
	// de-emphasis de graves
	aux = (float) distorsionShelve(entrada,0);

	// escalar de -gain a +gain
	aux = (gain * aux)/8388607.0;

	// waveshaper
	//salida = ((aux / sqrt(1 + aux * aux))/(gain/1.5))*8388607.0;
	aux = (2/3.1415)*atan(aux);

	// reescalado
	//aux = aux * 8388607.0;
	aux = aux * 3106891.5; //para que no suba tanto el volumen

	// refuerzo de graves
	//aux = (float) distorsionShelve(aux,1);

	//control de volumen
	//aux = 1.5 * aux / gain;

	// blend con la entrada
	salida = blend * aux + (1 - blend) * entrada;

	return  salida;
}

int distorsionShelve(int in, int i_ds)
{
	x_1[i_ds] = x_0[i_ds];
	x_0[i_ds] = in;
	y_1[i_ds] = y_0[i_ds];
	y_0[i_ds] = c[i_ds] * x_0[i_ds] - x_1[i_ds] + c[i_ds] * y_1[i_ds];
	in = 0.5 * (Vo[i_ds] - 1) * (in - y_0[i_ds]) + in;
	return in;
}

void distorsionGain (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	gain = pow(10 , 3 * (db->value));
}

void distorsionBlend (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	blend = (db->value);
}

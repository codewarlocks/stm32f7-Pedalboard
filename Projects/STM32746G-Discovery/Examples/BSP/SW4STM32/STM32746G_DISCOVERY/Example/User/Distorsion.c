
#include "math.h"
#include "gui/prototipos.h"

int salida_ds = 0;

// variables auxiliares
float aux_ds=0;

// parametros de usuario
float gain_ds = 70, blend_ds = 0.5;

// variables de filtrado
int x1_ds[2],x0_ds[2];
int y1_ds[2],y0_ds[2];
float Vo_ds[2] = {0.25 , 4};
float fc_ds[2] = {700 , 700};
float d_ds[2],c_ds[2];

void distorsionInit ()
{
    //parametros low shelve
    if(Vo_ds[0] >= 1)
	  c_ds[0] = (1-tan(3.1416*fc_ds[0]/SR))/(1+tan(3.1416*fc_ds[0]/SR));
    else
	  c_ds[0] = (Vo_ds[0]-tan(3.1416*fc_ds[0]/SR))/(Vo_ds[0]+tan(3.1416*fc_ds[0]/SR));
    distorsionParametros();
}

void distorsionParametros()
{

}

int distorsionEfecto (int entrada)
{
	// de-emphasis de graves
	aux_ds = (float) distorsionShelve(entrada,0);

	// escalar de -gain a +gain
	aux_ds = (gain_ds * aux_ds)/8388607.0;

	// waveshaper
	//salida_ds = ((aux_ds / sqrt(1 + aux_ds * aux_ds))/(gain_ds/1.5))*8388607.0;
	aux_ds = (2/3.1415)*atan(aux_ds);

	// reescalado
	aux_ds = aux_ds * 8388607.0;

	// refuerzo de graves
	//aux_ds = (float) distorsionShelve(aux_ds,1);

	// blend con la entrada
	salida_ds = blend_ds * aux_ds + (1 - blend_ds) * entrada;

	return salida_ds;
}

int distorsionShelve(int in, int i_ds)
{
	x1_ds[i_ds] = x0_ds[i_ds];
	x0_ds[i_ds] = in;
	y1_ds[i_ds] = y0_ds[i_ds];
	y0_ds[i_ds] = c_ds[i_ds] * x0_ds[i_ds] - x1_ds[i_ds] + c_ds[i_ds] * y1_ds[i_ds];
	in = 0.5 * (Vo_ds[i_ds] - 1) * (in - y0_ds[i_ds]) + in;
	return in;
}

void distorsionGain (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	gain_ds = 1000 * (db->value);
}

void distorsionBlend (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	blend_ds = (db->value);
}

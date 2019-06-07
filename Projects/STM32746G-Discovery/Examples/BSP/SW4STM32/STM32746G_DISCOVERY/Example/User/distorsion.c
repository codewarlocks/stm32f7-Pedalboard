
#include "math.h"
#include "gui/prototipos.h"

int salida_ds = 0;
float aux_ds=0;
// parametros de usuario
float gain_ds = 25, state_ds = 0;

int distorsion(int entrada)
{
	aux_ds = (gain_ds * entrada)/8388607.0;
	salida_ds = ((aux_ds / sqrt(1 + aux_ds * aux_ds))/(gain_ds/1.5))*8388607.0;
	return salida_ds;
}

void Distorsion_Gain (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
  gain_ds = 100 * (db->value);
}

void Push_State_Distorsion (GUIElement *e)
{
	state_ds = e->state;
}

int Get_State_Distorsion (void)
{
	 return state_ds;
}

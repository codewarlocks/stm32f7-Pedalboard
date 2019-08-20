#include "gui/prototipos.h"
#include "math.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"

int salida_rin = 0;

// parametros de usuario
float rate_rin = 440;

//variables auxiliares
int cont_rin = 0;

void ringmodInit ()
{
	ringmodParametros();
}

void ringmodParametros ()
{

}

int ringmodEfecto(int entrada)
{
	if(cont_rin == SR)
		cont_rin = 0;
	salida_rin = entrada * arm_sin_f32((float32_t) 2*3.1416*rate_rin*cont_rin/SR);
	cont_rin++;
	return salida_rin;
}

void ringmodRate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	rate_rin = 100 + 1900 * (db->value);
}

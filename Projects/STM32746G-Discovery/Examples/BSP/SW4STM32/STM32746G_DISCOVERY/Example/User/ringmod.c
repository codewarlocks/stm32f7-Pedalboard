#include "gui/prototipos.h"
#include "math.h"
#include "arm_math.h"
#include "arm_common_tables.h"

static int salida = 0;

// parametros de usuario
static float32_t rate = 440.0;

//variables auxiliares
static int cont = 0;

void ringmodInit ()
{
	ringmodParametros();
}

void ringmodParametros ()
{

}

int ringmodEfecto(int entrada)
{
	if(cont == SR)
		cont = 0;
	salida = entrada * arm_sin_f32((float32_t) 2*3.1416*rate*cont/SR);
	cont++;
	return salida;
}

void ringmodRate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
	rate = 100 + 1900 * (db->value);
}

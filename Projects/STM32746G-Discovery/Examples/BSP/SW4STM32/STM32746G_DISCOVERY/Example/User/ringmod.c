#include "gui/prototipos.h"
#include "math.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"

int salida_rin = 0;

// parametros de usuario
float rate_rin = 440;
int state_rin = 0;
float32_t lala;
//variables auxiliares
int cont_rin = 0;

int ringmod(int entrada)
{

	salida_rin = entrada * arm_sin_f32((float32_t) 2*3.1416*rate_rin*cont_rin/SR);//sin(2*3.1416*rate_rin*cont_rin/SR);
	cont_rin++;
	if(cont_rin == SR)
		cont_rin = 0;
	return salida_rin;
}

void Ringmod_Rate (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
  rate_rin = 100 + 1900 * (db->value);  // TODO: verificar
}

void Push_State_Ringmod (GUIElement *e)
{
	state_rin = e->state;
}

int Get_State_Ringmod (void)
{
	return state_rin;
}

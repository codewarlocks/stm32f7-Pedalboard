//Ecualizador.c
#include "gui/prototipos.h"
#include "math.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"
#define N 10

int salida_eq = 0;

// instancias de las variables de entrada y salida_eq
int x2_eq[N],x1_eq[N],x0_eq[N];
int y2_eq[N],y1_eq[N],y0_eq[N];

// parametros de usuario
float Vo_eq[N];

// parametros de desarrollador
float fc_eq[N],fb_eq[N];
int state_eq=0;

// parametros de los filtros
float d_eq[N],c_eq[N];

int i_eq = 0;

void init_eq()
{
	//frecuencias de corte
	fc_eq[0] = 31.25;fc_eq[1] = 62.5;fc_eq[2] = 125;
	fc_eq[3] = 250;fc_eq[4] = 500;fc_eq[5] = 1000;
	fc_eq[6] = 2000;fc_eq[7] = 4000;fc_eq[8] = 8000;fc_eq[9] = 16000;
	//anchos de banda
	for(i_eq = 1; i_eq < N-1; i_eq++)
		fb_eq[i_eq] = 20 * pow(2, i_eq-1);
	//ganancias
	Vo_eq[0] = 0.25;Vo_eq[1] = 2;Vo_eq[2] = 3;
  Vo_eq[3] = 4;Vo_eq[4] = 0.5;Vo_eq[5] = 0.25;
	Vo_eq[6] = 0.75;Vo_eq[7] = 0.75;Vo_eq[8] = 4;Vo_eq[9] = 0.25;
  eq_parametros();
}

void eq_parametros()
{
  //parametros low shelve
  if(Vo_eq[0] >= 1)
    c_eq[0] = (1-tan(3.1416*fc_eq[0]/SR))/(1+tan(3.1416*fc_eq[0]/SR));
  else
    c_eq[0] = (Vo_eq[0]-tan(3.1416*fc_eq[0]/SR))/(Vo_eq[0]+tan(3.1416*fc_eq[0]/SR));
  //parametros peak
  for(i_eq = 1; i_eq < N - 1; i_eq++)
  {
    d_eq[i_eq] = -cos(2*3.1416*fc_eq[i_eq]/SR);
    if(Vo_eq[i_eq] >= 1)
      c_eq[i_eq] = (1-tan(3.1416*fb_eq[i_eq]/SR))/(1+tan(3.1416*fb_eq[i_eq]/SR));
    else
      c_eq[i_eq] = (Vo_eq[i_eq]-tan(3.1416*fb_eq[i_eq]/SR))/(Vo_eq[i_eq]+tan(3.1416*fb_eq[i_eq]/SR));
  }
  //parametros high shelve
  if(Vo_eq[9] >= 1)
    c_eq[9] = (1-tan(3.1416*fc_eq[9]/SR))/(1+tan(3.1416*fc_eq[9]/SR));
  else
    c_eq[9] = (1-Vo_eq[9]*tan(3.1416*fc_eq[9]/SR))/(1+Vo_eq[9]*tan(3.1416*fc_eq[9]/SR));
}

int ecualizador(int entrada)
{
	salida_eq = eq_shelve(entrada,0);
	for(int i_eq = 1; i_eq < N-1; i_eq++)
		salida_eq = eq_peak(salida_eq,i_eq);
	salida_eq = eq_shelve(salida_eq,9);
	return salida_eq;
}

int eq_peak(int in, int i_eq)
{
	x2_eq[i_eq] = x1_eq[i_eq];
	x1_eq[i_eq] = x0_eq[i_eq];
	x0_eq[i_eq] = in;
	y2_eq[i_eq] = y1_eq[i_eq];
	y1_eq[i_eq] = y0_eq[i_eq];
	y0_eq[i_eq] = c_eq[i_eq] * x0_eq[i_eq] + d_eq[i_eq] * (1 + c_eq[i_eq]) * x1_eq[i_eq] + x2_eq[i_eq] - d_eq[i_eq] * (1 + c_eq[i_eq]) * y1_eq[i_eq] - c_eq[i_eq] * y2_eq[i_eq];
	in = 0.5 * (Vo_eq[i_eq] - 1) * (in - y0_eq[i_eq]) + in;
	return in;
}

int eq_shelve(int in, int i_eq)
{
	x1_eq[i_eq] = x0_eq[i_eq];
	x0_eq[i_eq] = in;
	y1_eq[i_eq] = y0_eq[i_eq];
	y0_eq[i_eq] = c_eq[i_eq] * x0_eq[i_eq] - x1_eq[i_eq] + c_eq[i_eq] * y1_eq[i_eq];
	if(i_eq == 0)
		in = 0.5 * (Vo_eq[i_eq] - 1) * (in - y0_eq[i_eq]) + in;
	else
		in = 0.5 * (Vo_eq[i_eq] - 1) * (in + y0_eq[i_eq]) + in;
	return in;
}

void EQ_Gain_0 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
  Vo_eq[0] = 4 * ((db->value)-1);
	eq_parametros();
}

void EQ_Gain_1 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
  Vo_eq[1] = 4 * ((db->value)-1);
	eq_parametros();
}

void EQ_Gain_2 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
  Vo_eq[2] = 4 * ((db->value)-1);
	eq_parametros();
}

void EQ_Gain_3 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
  Vo_eq[3] = 4 * ((db->value)-1);
	eq_parametros();
}

void EQ_Gain_4 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
  Vo_eq[4] = 4 * ((db->value)-1);
	eq_parametros();
}

void EQ_Gain_5 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
  Vo_eq[5] = 4 * ((db->value)-1);
	eq_parametros();
}

void EQ_Gain_6 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
  Vo_eq[6] = 4 * ((db->value)-1);
	eq_parametros();
}

void EQ_Gain_7 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
  Vo_eq[7] = 4 * ((db->value)-1);
	eq_parametros();
}

void EQ_Gain_8 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
  Vo_eq[8] = 4 * ((db->value)-1);
	eq_parametros();
}

void EQ_Gain_9 (GUIElement *e)
{
	DialButtonState *db = (DialButtonState *) (e->userData);
  Vo_eq[9] = 4 * ((db->value)-1);
	eq_parametros();
}
void Push_State_EQ (GUIElement *e)
{
	state_eq = e->state;
}

int Get_State_EQ (void)
{
	return state_eq;
}

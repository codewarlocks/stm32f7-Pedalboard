
#include "gui/prototipos.h"
#include "math.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"

#define N 2645

int line_o[N] = {0};

int salida = 0;

float32_t vol = 0;
int wr = 0, A = 0, B = N/2;
int flag_o = 1;

void octavadorInit()
{
	octavadorParametros();
}

void octavadorParametros()
{

}

int octavadorEfecto(int entrada)
{
	if(wr >= N)
		wr = 0;
	if(A >= N)
		A = A - N;
	if(B >= N)
		B = B - N;

	line_o[wr] = entrada;

	if(A <= (N-1)/2)
		vol = A/((N-1)/2.0);
	else
		vol = ((N-1)-A)/((N-1)/2.0);

	if(wr >= A)
		salida =  vol * line_o[wr - A];
	else
		salida =  vol * line_o[N + wr - A];

	if(wr >= B)
		salida += (1-vol) * line_o[B];
	else
		salida +=  (1-vol) * line_o[N + wr - B];

	if(wr >= N/2)
		salida += line_o[wr - N/2];
	else
		salida += line_o[N + wr - N/2];

	wr++;
	flag_o = 1 - flag_o;
	if(flag_o == 1)
	{
		A = A + 1;
		B = B + 1;
	}
	return salida;
}


#include "gui/prototipos.h"
#include "math.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"

#define N 2645

static int line[N] = {0}, salida = 0, wr = 0, A = 0, B = N/2, flag = 1;
static float32_t vol = 0.0;

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

	line[wr] = entrada;

	if(A <= (N-1)/2)
		vol = A/((N-1)/2.0);
	else
		vol = ((N-1)-A)/((N-1)/2.0);

	if(wr >= A)
		salida =  vol * line[wr - A];
	else
		salida =  vol * line[N + wr - A];

	if(wr >= B)
		salida += (1-vol) * line[B];
	else
		salida +=  (1-vol) * line[N + wr - B];

	if(wr >= N/2)
		salida += line[wr - N/2];
	else
		salida += line[N + wr - N/2];

	wr++;
	flag = 1 - flag;
	if(flag == 1)
	{
		A = A + 1;
		B = B + 1;
	}
	return salida;
}

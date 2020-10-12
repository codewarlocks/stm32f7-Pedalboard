#include "gui/prototipos.h"
#include "math.h"
#include "stdlib.h"

#include "arm_math.h"
#include "arm_common_tables.h"

static int32_t *line, salida = 0, wr = 0, A = 0, B = 0, N_N = 0, flag = 1;
static float32_t vol = 0.0f;

void octavadorInit() {
    N_N = (int32_t)(0.055f * SR);    //55ms
    line = (int32_t *)calloc(N_N, sizeof(int32_t));
    B = N_N / 2.0f;
    octavadorParametros();
}

void octavadorParametros() {
}

int32_t octavadorEfecto(int32_t entrada) {
    if (wr >= N_N) wr = 0;

    if (A >= N_N) A = A - N_N;

    if (B >= N_N) B = B - N_N;

    line[wr] = entrada;

    if (A <= ((N_N - 1) / 2)) vol = A / ((N_N - 1) / 2.0f);
    else vol = ((N_N - 1) - A) / ((N_N - 1) / 2.0f);

    if (wr >= A) salida =  vol * line[wr - A];
    else salida =  vol * line[N_N + wr - A];

    if (wr >= B) salida += (1 - vol) * line[B];
    else salida +=  (1 - vol) * line[N_N + wr - B];

    if (wr >= (N_N / 2)) salida += line[wr - (N_N / 2)];
    else salida += line[N_N + wr - (N_N / 2)];

    wr++;
    flag = 1 - flag;

    if (flag == 1) {
        A = A + 1;
        B = B + 1;
    }

    return salida;
}

#include "gui/prototipos.h"
#include "math.h"

#include "arm_math.h"
#include "arm_common_tables.h"

// parametros de desarrollador
#define CHORUS_SIZE 500

static int32_t salida = 0;

// parametros de usuario
static float32_t manual = 110.0f, rate = 1.2f, depth = 40.0f;
static int32_t modulacion = SINUSOIDAL;

// variables auxiliares
static float32_t min = 0.0f, max = 0.0f, tiempo, flag, demora = CHORUS_SIZE / 2.0f - 1.0f, delta = 0.0f, frac = 0.0f, aux1 = 0.0f, aux2 = 0.0f, aux3 = 0.0f, auxcoef = 0, lp = 0.999f;
static int32_t periodo = 0, cont = 0, d = 0, line1[CHORUS_SIZE] = { 0 }, line2[CHORUS_SIZE] = { 0 }, line3[CHORUS_SIZE] = { 0 };

void chorusInit() {
    chorusParametros();
}

void chorusParametros() {
    demora = manual;
    min = manual - depth;
    max = manual + depth;
    periodo = (int32_t)SR / rate;
    delta = (max - min) / periodo;
}

int32_t chorusEfecto(int32_t entrada) {
    if (cont == CHORUS_SIZE) cont = 0;

    line1[cont] = entrada;
    line2[cont] = entrada;
    line3[cont] = entrada;

    demora = chorusLFO(modulacion);
    d = (int32_t)floor(demora);
    frac = demora - d;

    if (cont - d - 1 >= 0) aux1 = frac * line1[cont - d - 1] + (1 - frac) * line1[cont - d];

    if (cont - d == 0) aux1 = frac * line1[CHORUS_SIZE - 1] + (1 - frac) * line1[cont - d];

    if (cont - d < 0) aux1 = frac * line1[CHORUS_SIZE - d + cont - 1] + (1 - frac) * line1[CHORUS_SIZE - d + cont];

    d = (int32_t)floor(demora + 50);
    frac = demora + 50 - d;

    if (cont - d - 1 >= 0) aux2 = frac * line2[cont - d - 1] + (1 - frac) * line2[cont - d];

    if (cont - d == 0) aux2 = frac * line2[CHORUS_SIZE - 1] + (1 - frac) * line2[cont - d];

    if (cont - d < 0) aux2 = frac * line2[CHORUS_SIZE - d + cont - 1] + (1 - frac) * line2[CHORUS_SIZE - d + cont];

    d = (int32_t)floor(demora + 100);
    frac = demora + 100 - d;

    if (cont - d - 1 >= 0) aux3 = frac * line3[cont - d - 1] + (1 - frac) * line3[cont - d];

    if (cont - d == 0) aux3 = frac * line3[CHORUS_SIZE - 1] + (1 - frac) * line3[cont - d];

    if (cont - d < 0) aux3 = frac * line3[CHORUS_SIZE - d + cont - 1] + (1 - frac) * line3[CHORUS_SIZE - d + cont];

    cont++;

    salida = aux3 + aux2 + aux1 + entrada;
//	salida = salida;
    return salida;
}

float32_t chorusLFO(int32_t modulacion) {
    switch (modulacion) {
        case CUADRADA:

            if (tiempo >= periodo / 2.0f) {
                if (auxcoef == min) auxcoef = max;
                else auxcoef = min;

                tiempo = 0;
            }

            demora = lp * demora + (1 - lp) * auxcoef;
            break;

        case TRIANGULAR:

            if (flag == 0) demora = demora + 2.0f * delta;

            if (flag == 1) demora = demora - 2.0f * delta;

            if (demora >= max || demora <= min) {
                flag = 1 - flag;
                tiempo = 0;
            }

            break;

        case RAMPA_ASC:
            auxcoef = auxcoef + delta;

            if (demora >= max) {
                auxcoef = min;
                tiempo = 0;
            }

            demora = lp * demora + (1 - lp) * auxcoef;
            break;

        case SINUSOIDAL:
            demora = manual + depth * arm_sin_f32((float32_t)(2.0f * PI * tiempo / periodo));

            if (tiempo == periodo) tiempo = 0;

            break;
    }
    tiempo++;
    return demora;
}

void chorusRate(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    rate = 2.0f * (db->value);
    chorusParametros();
}

void chorusDepth(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    depth = (manual - 1.0f) * (db->value);
    chorusParametros();
}

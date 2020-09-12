#include "gui/prototipos.h"
#include "math.h"

#include "arm_math.h"
#include "arm_common_tables.h"

// parametros de desarrollador
#define VIBRATO_SIZE 150

static int32_t salida = 0;

// parametros de usuario
static float32_t rate = 3.0f, depth = 30.0f;
static int32_t modulacion = SINUSOIDAL;

// parametros de desarrollador
static int32_t mid = VIBRATO_SIZE / 2 - 1;

// variables auxiliares
static float32_t min = 0.0f, max = 0.0f, demora = VIBRATO_SIZE / 2.0f - 1.0f, delta = 0.0f, frac = 0.0f, auxcoef = 0.0f, lp = 0.999f;
static int32_t periodo = 0, cont = 0, tiempo = 0, flag = 0, rampflag = 0, d = 0, line[VIBRATO_SIZE];

void vibratoInit() {
    vibratoParametros();
}

void vibratoParametros() {
    demora = mid;
    min = mid - depth;
    max = mid + depth;
    periodo = (int32_t)SR / rate;
    delta = (max - min) / periodo;
}

int32_t vibratoEfecto(int32_t entrada) {
    if (cont == VIBRATO_SIZE) {
        flag = 1;
        cont = 0;
    }

    line[cont] = entrada;

    if (flag == 1) {
        demora = vibratoLFO(modulacion);
        d = (int32_t)floor(demora);
        frac = demora - d;

        if (cont - d - 1 >= 0) salida = frac * line[cont - d - 1] + (1.0f - frac) * line[cont - d];

        if (cont - d == 0) salida = frac * line[VIBRATO_SIZE - 1] + (1.0f - frac) * line[cont - d];

        if (cont - d < 0) salida = frac * line[VIBRATO_SIZE - d + cont - 1] + (1.0f - frac) * line[VIBRATO_SIZE - d + cont];
    }

    cont++;
    return salida;
}

float32_t vibratoLFO(int32_t modulacion) {
    switch (modulacion) {
        case CUADRADA:

            if (tiempo >= periodo / 2.0f) {
                if (auxcoef == min) auxcoef = max;
                else auxcoef = min;

                tiempo = 0;
            }

            demora = lp * demora + (1.0f - lp) * auxcoef;
            break;

        case TRIANGULAR:

            if (rampflag == 0) demora = demora + 2.0f * delta;

            if (rampflag == 1) demora = demora - 2.0f * delta;

            if (demora >= max || demora <= min) {
                rampflag = 1.0f - rampflag;
                tiempo = 0;
            }

            break;

        case RAMPA_ASC:
            auxcoef = auxcoef + delta;

            if (auxcoef >= max) {
                auxcoef = min;
                tiempo = 0;
            }

            demora = lp * demora + (1 - lp) * auxcoef;
            break;

        case SINUSOIDAL:
            demora = mid + depth * arm_sin_f32((float32_t)(2.0f * PI * tiempo / periodo));

            if (tiempo == periodo) tiempo = 0;

            break;
    }
    tiempo++;
    return demora;
}

void vibratoRate(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    rate = 5.0f * (db->value);
    vibratoParametros();
}

void vibratoDepth(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    depth = (mid - 1.0f) * (db->value);
    vibratoParametros();
}

void vibratoMod(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    if (db->value <= (float32_t)0.33f) modulacion = CUADRADA;
    else if (db->value > (float32_t)0.33f && db->value <= (float32_t)0.66f) modulacion = RAMPA_ASC;
    else if (db->value > (float32_t)0.66f && db->value < 1.0f) modulacion = TRIANGULAR;
    else if (db->value == 1) modulacion = SINUSOIDAL;
}

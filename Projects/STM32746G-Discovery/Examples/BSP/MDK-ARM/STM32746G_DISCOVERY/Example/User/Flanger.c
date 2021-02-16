#include "gui/prototipos.h"
#include "math.h"

#include "arm_math.h"
#include "arm_common_tables.h"

// parametros de desarrollador
#define FLANGER_SIZE 300

static int32_t salida = 0;

// parametros de usuario
static float32_t manual = 15.0f, rate = 0.2f, depth = 8.0f, feedback = -0.95f;
static int32_t modulacion = SINUSOIDAL;

// variables auxiliares
static float32_t min = 0.0f, max = 0.0f, tiempo, demora = FLANGER_SIZE / 2.0f - 1.0f, delta = 0.0f, frac = 0.0f, aux1 = 0.0f, aux2 = 0.0f, aux3 = 0.0f, aux4 = 0.0f, aux5 = 0.0f;
static int32_t periodo = 0, flag = 0, cont = 0, d = 0, line1[FLANGER_SIZE] = { 0 }, line2[FLANGER_SIZE] = { 0 }, line3[FLANGER_SIZE] = { 0 }, line4[FLANGER_SIZE] = { 0 }, line5[FLANGER_SIZE] = { 0 };

void flangerInit() {
    flangerParametros();
}

void flangerParametros() {
    demora = manual;
    min = manual - depth;
    max = manual + depth;
    periodo = (int32_t)SR / rate;
    delta = (max - min) / periodo;
}

int32_t flangerEfecto(int32_t entrada) {
    if (cont == FLANGER_SIZE) {
        cont = 0;
    }

    line1[cont] = (int32_t)(entrada + aux1 * feedback);
    line2[cont] = (int32_t)(entrada + aux2 * feedback);
    line3[cont] = (int32_t)(entrada + aux3 * feedback);
    line4[cont] = (int32_t)(entrada + aux4 * feedback);
    line5[cont] = (int32_t)(entrada + aux5 * feedback);

    demora = flangerLFO(modulacion);
    d = (int32_t)floor(demora);
    frac = demora - d;

    if (cont - d - 1 >= 0) aux1 = frac * line1[cont - d - 1] + (1 - frac) * line1[cont - d];

    if (cont - d == 0) aux1 = frac * line1[FLANGER_SIZE - 1] + (1 - frac) * line1[cont - d];

    if (cont - d < 0) aux1 = frac * line1[FLANGER_SIZE - d + cont - 1] + (1 - frac) * line1[FLANGER_SIZE - d + cont];

    d = (int32_t)floor(demora + 10);
    frac = demora + 10 - d;

    if (cont - d - 1 >= 0) aux2 = frac * line2[cont - d - 1] + (1 - frac) * line2[cont - d];

    if (cont - d == 0) aux2 = frac * line2[FLANGER_SIZE - 1] + (1 - frac) * line2[cont - d];

    if (cont - d < 0) aux2 = frac * line2[FLANGER_SIZE - d + cont - 1] + (1 - frac) * line2[FLANGER_SIZE - d + cont];

    d = (int32_t)floor(demora + 20);
    frac = demora + 20 - d;

    if (cont - d - 1 >= 0) aux3 = frac * line3[cont - d - 1] + (1 - frac) * line3[cont - d];

    if (cont - d == 0) aux3 = frac * line3[FLANGER_SIZE - 1] + (1 - frac) * line3[cont - d];

    if (cont - d < 0) aux3 = frac * line3[FLANGER_SIZE - d + cont - 1] + (1 - frac) * line3[FLANGER_SIZE - d + cont];

    d = (int32_t)floor(demora + 30);
    frac = demora + 30 - d;

    if (cont - d - 1 >= 0) aux4 = frac * line4[cont - d - 1] + (1 - frac) * line4[cont - d];

    if (cont - d == 0) aux4 = frac * line4[FLANGER_SIZE - 1] + (1 - frac) * line4[cont - d];

    if (cont - d < 0) aux4 = frac * line4[FLANGER_SIZE - d + cont - 1] + (1 - frac) * line4[FLANGER_SIZE - d + cont];

    d = (int32_t)floor(demora + 40);
    frac = demora + 40 - d;

    if (cont - d - 1 >= 0) aux5 = frac * line5[cont - d - 1] + (1 - frac) * line5[cont - d];

    if (cont - d == 0) aux5 = frac * line5[FLANGER_SIZE - 1] + (1 - frac) * line5[cont - d];

    if (cont - d < 0) aux5 = frac * line5[FLANGER_SIZE - d + cont - 1] + (1 - frac) * line5[FLANGER_SIZE - d + cont];

    cont++;

    salida = (int32_t)((aux5 + aux4 + aux3 + aux2 + aux1 + entrada) * (float32_t)0.35f);
    return salida;
}

float32_t flangerLFO(int32_t modulacion) {
    switch (modulacion) {
        case CUADRADA:

            if (tiempo == periodo / 2.0f) {
                if (demora == min) demora = max;
                else demora = min;

                tiempo = 0;
            }

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
            demora = demora + delta;

            if (demora >= max) {
                demora = min;
                tiempo = 0;
            }

            break;

        case SINUSOIDAL:
            demora = manual + depth * arm_sin_f32((float32_t)(2.0f * PI * tiempo / periodo));

            if (tiempo == periodo) tiempo = 0;

            break;
    }
    tiempo++;
    return demora;
}

void flangerManual(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    manual = 35.0f * (db->value) + 15.0f;
    flangerParametros();
}

void flangerDepth(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    depth = (15.0f - 1.0f) * (db->value);
    flangerParametros();
}

void flangerRate(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    rate = 1.0f * (db->value);
    flangerParametros();
}

void flangerFeedback(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    rate = -(float32_t)0.185f * (db->value) - (float32_t)0.8f;
}

#include "math.h"
#include "gui/prototipos.h"

#include "arm_math.h"
#include "arm_common_tables.h"

#define ENV_SIZE 10
static int32_t salida = 0;

// instancias de las variables de entrada y salida
static int32_t x_2, x_1, x_0, y_2[5] = { 0 }, y_1[5] = { 0 }, y_0[5] = { 0 };

// parametros de usuario
static float32_t rate = 2.0f, depth = 450.0f, volume = 0.6f;
static int32_t modulacion = SINUSOIDAL;

// parametros de desarrollador
static float32_t fmedia = 500.0f, fb = 100.0f;

// parametros del filtro
static float32_t c = 0.0f, d = 0.0f;

// variables auxiliares
static float32_t finicial = 0.0f, ffinal = 0.0f, fcentral = 0.0f, deltaf = 0.0f, rateenv = 0.0f, periodoenv = 0.0f, deltafenv = 0.0f, auxcoef = 0.0f, lp = 0.99f;
static int32_t periodo = 0, cont = 0, flag = 0;

void autowahInit() {
    fcentral = fmedia;
    c = (tanf(PI * fb / SR) - 1.0f) / (tanf(2.0f * PI * fb / SR) + 1.0f);
    d = -cos(2.0f * PI * fcentral / SR);
    autowahParametros();
}

void autowahParametros() {
    finicial = fmedia - depth;
    ffinal = fmedia + depth;
    periodo = (int32_t)SR / rate;
    deltaf = (ffinal - finicial) / periodo;
}

int32_t autowahEfecto(int32_t entrada) {
    fcentral = autowahLFO(modulacion);
    x_2 = x_1;
    x_1 = x_0;
    x_0 = entrada;
    y_0[0] = autowahBP2(fcentral, 0);
    y_0[1] = autowahBP2(400 + fcentral, 1);
//	y_0[2] = autowahBP2(800+fcentral,2);
//	salida = (volume * (y_0[0] + y_0[1] + y_0[2]) + (1 - volume) * x_0);
    salida = (volume * (y_0[0] + y_0[1]) + (1 - volume) * x_0);
    return salida;
}

int32_t autowahBP2(float32_t fc, int32_t i) {
    d = -arm_cos_f32(2.0f * PI * fc / SR);
    y_2[i] = y_1[i];
    y_1[i] = y_0[i];
    y_0[i] = (1.0f + c) * 0.5f * x_0 - (1.0f + c) * 0.5f * x_2 - d * (1.0f - c) * y_1[i] + c * y_2[i];
    return y_0[i];
}

float32_t autowahLFO(int32_t modulacion) {
    switch (modulacion) {
        case CUADRADA:

            if (cont >= periodo / 2.0f) {
                if (auxcoef == finicial) auxcoef = ffinal;
                else auxcoef = finicial;

                cont = 0;
            }

            fcentral = lp * fcentral + (1.0f - lp) * auxcoef;
            break;

        case TRIANGULAR:

            if (flag == 0) fcentral = fcentral + 2.0f * deltaf;

            if (flag == 1) fcentral = fcentral - 2.0f * deltaf;

            if (fcentral >= ffinal || fcentral <= finicial) {
                flag = 1 - flag;
                cont = 0;
            }

            break;

        case RAMPA_ASC:
            auxcoef = auxcoef + deltaf;

            if (auxcoef >= ffinal) {
                auxcoef = finicial;
                cont = 0;
            }

            fcentral = lp * fcentral + (1 - lp) * auxcoef;
            break;

        case RAMPA_DESC:
            auxcoef = auxcoef - deltaf;

            if (auxcoef <= finicial) {
                auxcoef = ffinal;
                cont = 0;
            }

            fcentral = lp * fcentral + (1 - lp) * auxcoef;
            break;

        case SINUSOIDAL:
            fcentral = fmedia + (ffinal - finicial) * arm_sin_f32((float32_t)(2.0f * PI * cont / periodo));

            if (cont == periodo) cont = 0;

            break;

        case ENVOLVENTE:
            rateenv = x_0 / 10000.0f;
            rateenv = rateenv * rateenv;
            rateenv = ((8.0f * rate - 10000.0f * rateenv) > rate) ? (8.0f * rate - 10000.0f * rateenv) : rate;
            periodoenv = (float32_t)SR / rateenv;
            deltafenv = (ffinal - finicial) / periodoenv;

            if (flag == 0) fcentral = fcentral + 2.0f * deltafenv;

            if (flag == 1) fcentral = fcentral - 2.0f * deltafenv;

            if (fcentral >= ffinal) flag = 1;

            if (fcentral <= finicial) flag = 0;

            break;
    }
    cont++;
    return fcentral;
}

void autowahRate(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    rate = 6.0f * (db->value);
    autowahParametros();
}

void autowahDepth(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    depth = (fmedia - 100.0f) * (db->value);
    autowahParametros();
}

void autowahVolume(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    volume = (db->value);
}

void autowahMod(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    if (db->value <= 0.2f) modulacion = CUADRADA;
    else if (db->value > 0.2f && db->value <= 0.4f) modulacion = TRIANGULAR;
    else if (db->value > 0.4f && db->value <= 0.6f) modulacion = RAMPA_ASC;
    else if (db->value > 0.6f && db->value <= 0.8f) modulacion = RAMPA_DESC;
    else if (db->value > 0.8f && db->value < 1.0f) modulacion = SINUSOIDAL;
    else if (db->value == 1) modulacion = ENVOLVENTE;
}

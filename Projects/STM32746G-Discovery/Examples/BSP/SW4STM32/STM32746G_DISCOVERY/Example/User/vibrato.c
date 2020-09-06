#include "gui/prototipos.h"
#include "math.h"
#include "arm_math.h"
#include "arm_common_tables.h"

// parametros de desarrollador
#define VIBRATO_SIZE 150

static int salida = 0;

// parametros de usuario
static float32_t rate = 3, depth = 30;
static int modulacion = SINUSOIDAL;

// parametros de desarrollador
static int mid = VIBRATO_SIZE / 2 - 1;

// variables auxiliares
static float32_t min = 0.0, max = 0.0, demora = VIBRATO_SIZE / 2 - 1, delta = 0.0, frac = 0.0, auxcoef = 0, lp = 0.999;
static int periodo = 0, cont = 0, tiempo = 0, flag = 0, rampflag = 0, d = 0, line[VIBRATO_SIZE];

void vibratoInit() {
    vibratoParametros();
}

void vibratoParametros() {
    demora = mid;
    min = mid - depth;
    max = mid + depth;
    periodo = (int)SR / rate;
    delta = (max - min) / periodo;
}

int vibratoEfecto(int entrada) {
    if (cont == VIBRATO_SIZE) {
        flag = 1;
        cont = 0;
    }

    line[cont] = entrada;

    if (flag == 1) {
        demora = vibratoLFO(modulacion);
        d = (int)floor(demora);
        frac = demora - d;

        if (cont - d - 1 >= 0) {
            salida = frac * line[cont - d - 1] + (1 - frac) * line[cont - d];
        }

        if (cont - d == 0) {
            salida = frac * line[VIBRATO_SIZE - 1] + (1 - frac) * line[cont - d];
        }

        if (cont - d < 0) {
            salida = frac * line[VIBRATO_SIZE - d + cont - 1] + (1 - frac) * line[VIBRATO_SIZE - d + cont];
        }
    }

    cont++;
    return(salida);
}

float32_t vibratoLFO(int modulacion) {
    switch (modulacion) {
        case CUADRADA:

            if (tiempo >= periodo / 2) {
                if (auxcoef == min) {
                    auxcoef = max;
                } else {
                    auxcoef = min;
                }

                tiempo = 0;
            }

            demora = lp * demora + (1 - lp) * auxcoef;
            break;

        case TRIANGULAR:

            if (rampflag == 0) {
                demora = demora + 2 * delta;
            }

            if (rampflag == 1) {
                demora = demora - 2 * delta;
            }

            if (demora >= max || demora <= min) {
                rampflag = 1 - rampflag;
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
            demora = mid + depth * arm_sin_f32((float32_t)(2 * 3.1416 * tiempo / periodo));

            if (tiempo == periodo) {
                tiempo = 0;
            }

            break;
    }
    tiempo++;
    return(demora);
}

void vibratoRate(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    rate = 5 * (db->value);
    vibratoParametros();
}

void vibratoDepth(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    depth = (mid - 1) * (db->value);
    vibratoParametros();
}

void vibratoMod(GUIElement *e) {
    DialButtonState *db = (DialButtonState *)(e->userData);

    if (db->value <= 0.33) {
        modulacion = CUADRADA;
    } else if (db->value > 0.33 && db->value <= 0.66) {
        modulacion = RAMPA_ASC;
    } else if (db->value > 0.66 && db->value < 1) {
        modulacion = TRIANGULAR;
    } else if (db->value == 1) {
        modulacion = SINUSOIDAL;
    }
}

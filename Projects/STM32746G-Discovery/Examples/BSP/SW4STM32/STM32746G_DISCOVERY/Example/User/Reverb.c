/*
 * Reverb.c
 *
 *  Created on: Apr 19, 2019
 *      Author: gonza
 */

#include "gui/prototipos.h"

static int salida = 0;

static int FBCF0_line[1617];
static int FBCF1_line[1617];
static int FBCF2_line[1617];
static int FBCF3_line[1617];
static int FBCF4_line[1617];
static int FBCF5_line[1617];
static int FBCF6_line[1617];
static int FBCF7_line[1617];

static int lpo[8];

static int AP0_line[225];
static int AP1_line[556];
static int AP2_line[441];
static int AP3_line[341];

//variables de usuario
static float32_t dry = 0.8, wet = 0.2, g = 0.5, f = 0.75;

//variables de desarrollador
static float32_t d = 0.2;

//variables auxiliares
static int FBCF_cont = 0;
static int AP0_cont = 0, AP1_cont = 0, AP2_cont = 0, AP3_cont = 0;

void reverbInit ()
{
	reverbParametros();
}

void reverbParametros()
{

}

int reverbEfecto (int entrada)
{
	if(FBCF_cont == 1617)
		FBCF_cont = 0;
	if(AP0_cont == 225)
		AP0_cont = 0;
	if(AP1_cont == 556)
		AP1_cont = 0;
	if(AP2_cont == 441)
		AP2_cont = 0;
	if(AP3_cont == 341)
		AP3_cont = 0;

	salida = 0.125 * reverbFBCF(entrada,FBCF0_line,1556,FBCF_cont,0);
	salida += 0.125 * reverbFBCF(entrada,FBCF1_line,1616,FBCF_cont,1);
	salida += 0.125 * reverbFBCF(entrada,FBCF2_line,1490,FBCF_cont,2);
	salida += 0.125 * reverbFBCF(entrada,FBCF3_line,1421,FBCF_cont,3);
	salida += 0.125 * reverbFBCF(entrada,FBCF4_line,1276,FBCF_cont,4);
	salida += 0.125 * reverbFBCF(entrada,FBCF5_line,1355,FBCF_cont,5);
	salida += 0.125 * reverbFBCF(entrada,FBCF6_line,1187,FBCF_cont,6);
	salida += 0.125 * reverbFBCF(entrada,FBCF7_line,1115,FBCF_cont,7);

	salida = reverbAP(salida,AP0_line,224,AP0_cont);
	salida = reverbAP(salida,AP1_line,555,AP1_cont);
	salida = reverbAP(salida,AP2_line,440,AP2_cont);
	salida = reverbAP(salida,AP3_line,340,AP3_cont);

	salida = dry * entrada + wet * salida;

	FBCF_cont++;
	AP0_cont++;
	AP1_cont++;
	AP2_cont++;
	AP3_cont++;

	return salida;
}

int reverbFBCF(int in, int line[], int delay, int cont, int inst)
{
	line[cont] = in + f * lpo[inst];
	if(cont >= delay)
	{
		lpo[inst] = (1-d) * line[cont-delay] + d * lpo[inst];
		return line[cont-delay];
	}
	else
	{
		lpo[inst] = (1-d) * line[1617+cont-delay] + d * lpo[inst];
		return line[1617+cont-delay];
	}
}

int reverbAP(int in, int* line, int delay, int cont)
{
	if(cont >= delay)
	{
		line[cont] = in + g * line[cont-delay];
		return (1+g) * line[cont-delay] - line[cont];
	}
	else
	{
		line[cont] = in + g * line[delay+1+cont-delay];
		return (1+g) * line[delay+1+cont-delay] - line[cont];
	}
}

void reverbMix (GUIElement *e)
{
    DialButtonState *db = (DialButtonState *) (e->userData);
    wet = 0.5 * (db->value);
    dry = 1 - wet;
}

void reverbDecay (GUIElement *e)
{
    DialButtonState *db = (DialButtonState *) (e->userData);
     f = 0.4 * (db->value) + 0.55;
}


/*
 * Reverb.c
 *
 *  Created on: Apr 19, 2019
 *      Author: gonza
 */

#include "gui/prototipos.h"

static int32_t salida = 0;

static int32_t FBCF0_line[1617]={0};//1617
static int32_t FBCF1_line[1617]={0};
static int32_t FBCF2_line[1617]={0};
static int32_t FBCF3_line[1617]={0};
static int32_t FBCF4_line[1617]={0};
static int32_t FBCF5_line[1617]={0};
static int32_t FBCF6_line[1617]={0};
static int32_t FBCF7_line[1617]={0};

static int32_t lpo[8]={0};

static int32_t AP0_line[225]={0};
static int32_t AP1_line[556]={0};
static int32_t AP2_line[441]={0};
static int32_t AP3_line[341]={0};

//variables de usuario
static float32_t dry = 0.8f, wet = 0.2f, g = 0.5f, f = 0.75f;

//variables de desarrollador
static float32_t d = 0.2f;

//variables auxiliares
static int32_t FBCF_cont = 0;
static int32_t AP0_cont = 0, AP1_cont = 0, AP2_cont = 0, AP3_cont = 0;

void reverbInit ()
{
	reverbParametros();
}

void reverbParametros()
{

}

int32_t reverbEfecto (int32_t entrada)
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

	salida = (float32_t)0.125f * reverbFBCF(entrada,FBCF0_line,1556,FBCF_cont,0);
	salida += (float32_t)0.125f * reverbFBCF(entrada,FBCF1_line,1616,FBCF_cont,1);
	salida += (float32_t)0.125f * reverbFBCF(entrada,FBCF2_line,1490,FBCF_cont,2);
	salida += (float32_t)0.125f * reverbFBCF(entrada,FBCF3_line,1421,FBCF_cont,3);
	salida += (float32_t)0.125f * reverbFBCF(entrada,FBCF4_line,1276,FBCF_cont,4);
	salida += (float32_t)0.125f * reverbFBCF(entrada,FBCF5_line,1355,FBCF_cont,5);
	salida += (float32_t)0.125f * reverbFBCF(entrada,FBCF6_line,1187,FBCF_cont,6);
	salida += (float32_t)0.125f * reverbFBCF(entrada,FBCF7_line,1115,FBCF_cont,7);

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

int32_t reverbFBCF(int32_t in, int32_t line[], int32_t delay, int32_t cont, int32_t inst)
{
	line[cont] = in + f * lpo[inst];
	if(cont >= delay)
	{
		lpo[inst] = (1.0f-d) * line[cont-delay] + d * lpo[inst];
		return line[cont-delay];
	}
	else
	{
		lpo[inst] = (1.0f-d) * line[1617+cont-delay] + d * lpo[inst];
		return line[1617+cont-delay];
	}
}

int32_t reverbAP(int32_t in, int32_t* line, int32_t delay, int32_t cont)
{
	if(cont >= delay)
	{
		line[cont] = in + g * line[cont-delay];
		return (1.0f+g) * line[cont-delay] - line[cont];
	}
	else
	{
		line[cont] = in + g * line[delay+1+cont-delay];
		return (1.0f+g) * line[delay+1+cont-delay] - line[cont];
	}
}

void reverbMix (GUIElement *e)
{
    DialButtonState *db = (DialButtonState *) (e->userData);
    wet = (float32_t)0.5f * (db->value);
    dry = 1.0f - wet;
}

void reverbDecay (GUIElement *e)
{
    DialButtonState *db = (DialButtonState *) (e->userData);
     f = (float32_t)0.4f * (db->value) + (float32_t)0.55f;
}


/*
 * Reverb.c
 *
 *  Created on: Apr 19, 2019
 *      Author: g_ronza
 */

#include "gui/prototipos.h"

int salida_r = 0;

extern int FBCF0_line[1617];
extern int FBCF1_line[1617];
extern int FBCF2_line[1617];
extern int FBCF3_line[1617];
extern int FBCF4_line[1617];
extern int FBCF5_line[1617];
extern int FBCF6_line[1617];
extern int FBCF7_line[1617];//1617

extern int lpo_r[8];

extern int AP0_line[225];
extern int AP1_line[556];
extern int AP2_line[441];
extern int AP3_line[341];

//variables de usuario
float32_t dry_r = 0.8, wet_r = 0.2, g_r = 0.5, f_r = 0.75;

//variables de desarrollador
float32_t d_r = 0.2;

//variables auxiliares
int FBCF_cont = 0;
int AP0_cont = 0, AP1_cont = 0, AP2_cont = 0, AP3_cont = 0;

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

	salida_r = 0.125 * reverbFBCF(entrada,FBCF0_line,1556,FBCF_cont,0);
	salida_r += 0.125 * reverbFBCF(entrada,FBCF1_line,1616,FBCF_cont,1);
	salida_r += 0.125 * reverbFBCF(entrada,FBCF2_line,1490,FBCF_cont,2);
	salida_r += 0.125 * reverbFBCF(entrada,FBCF3_line,1421,FBCF_cont,3);
	salida_r += 0.125 * reverbFBCF(entrada,FBCF4_line,1276,FBCF_cont,4);
	salida_r += 0.125 * reverbFBCF(entrada,FBCF5_line,1355,FBCF_cont,5);
	salida_r += 0.125 * reverbFBCF(entrada,FBCF6_line,1187,FBCF_cont,6);
	salida_r += 0.125 * reverbFBCF(entrada,FBCF7_line,1115,FBCF_cont,7);

	salida_r = reverbAP(salida_r,AP0_line,224,AP0_cont);
	salida_r = reverbAP(salida_r,AP1_line,555,AP1_cont);
	salida_r = reverbAP(salida_r,AP2_line,440,AP2_cont);
	salida_r = reverbAP(salida_r,AP3_line,340,AP3_cont);

	salida_r = dry_r * entrada + wet_r * salida_r;

	FBCF_cont++;
	AP0_cont++;
	AP1_cont++;
	AP2_cont++;
	AP3_cont++;

	return salida_r;
}

int reverbFBCF(int in, int line[], int delay, int cont, int inst)
{
	line[cont] = in + f_r * lpo_r[inst];
	if(cont >= delay)
	{
		lpo_r[inst] = (1-d_r) * line[cont-delay] + d_r * lpo_r[inst];
		return line[cont-delay];
	}
	else
	{
		lpo_r[inst] = (1-d_r) * line[1617+cont-delay] + d_r * lpo_r[inst];
		return line[1617+cont-delay];
	}
}

int reverbAP(int in, int* line, int delay, int cont)
{
	if(cont >= delay)
	{
		line[cont] = in + g_r * line[cont-delay];
		return (1+g_r) * line[cont-delay] - line[cont];
	}
	else
	{
		line[cont] = in + g_r * line[delay+1+cont-delay];
		return (1+g_r) * line[delay+1+cont-delay] - line[cont];
	}
}

void reverbMix (GUIElement *e)
{
    DialButtonState *db = (DialButtonState *) (e->userData);
    wet_r = 0.5 * (db->value);
    dry_r = 1 - wet_r;
}

void reverbDecay (GUIElement *e)
{
    DialButtonState *db = (DialButtonState *) (e->userData);
     f_r = 0.4 * (db->value) + 0.55;
}


/*
 * delay.c
 *
 *  Created on: Mar 21, 2019
 *      Author: Nikola-Tesla
 */

//variables para el delay
float delay_segundos = 0.25, delay_entrada, delay_salida, delay_mix=0.99;
int delay_muestras = 15000, delay_contador = 0, delay_flag = 0;
float delay_line[15000];

int delay (int delay_entrada)
{
	if (delay_contador == delay_muestras)
	{
		delay_flag = 1;
		delay_contador = 0;
	}
	delay_line[delay_contador] = delay_entrada;
	if (delay_flag == 0)
	{
		delay_salida = delay_line[delay_contador] * delay_mix; // (= delay_entrada)
		delay_contador++;
	}
	if (delay_flag == 1)
	{
		if (delay_contador+1 == delay_muestras)  // si está en la última posición, la muestra de delay tiene que ser la 0
			delay_salida = delay_line[delay_contador] * delay_mix + delay_line[0] * (1-delay_mix);
		else
			delay_salida = delay_line[delay_contador] * delay_mix + delay_line[delay_contador+1] * (1-delay_mix);
		delay_contador++;
	}

	return delay_salida;
}


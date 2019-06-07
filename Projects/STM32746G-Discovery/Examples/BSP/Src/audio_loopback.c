 /**
  ******************************************************************************
 * @file    BSP/Src/audio_loopback.c
  * @author  MCD Application Team
  * @brief   This example code shows how to use the audio feature in the
  *          stm32746g_discovery driver
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "main.h"
#include <stdio.h>
#include "string.h"
#include "gui/leds.h"
#include "gui/gui.h"
#include "gui/tremoloondas.h"
#include "gui/vibratoondas.h"
#include "gui/whaondas.h"
#include "gui/perilla35x35x25.h"
#include "gui/perilla42x41x25.h"
#include "gui/perilla52x52x25.h"
#include "gui/link.h"

#define DELAY 0
#define TREMOLO 1
#define VIBRATO 2
#define DISTORSION 3
#define WHA 4
#define RINGMOD 5
#define IZQUIERDA 0
#define MENU 14
#define DERECHA 1
#define HOME 2

//Para dibujar perillas
//EJEMPLO WSLDN4 -> EX03
#include <math.h>
#include "ex03/mainex.h"
#include "gui/prototipos.h"
/** @addtogroup STM32F7xx_HAL_Examples
  * @{
  */

/** @addtogroup BSP
  * @{
  */
//FONDITO
#ifndef __MAIN_H
#define __MAIN_H
/* Includes ------------------------------------------------------------------*/
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include <stdlib.h>
#include "fatfs_storage.h"
/* FatFs includes component */
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#endif /* __MAIN_H */
char SD_Path[4]; /* SD card logical drive path */
char* pDirectoryFiles[25];//MAX_BMP_FILES
uint8_t  ubNumberOfFiles = 0;
uint32_t uwBmplen = 0, uwBmplen2=0;
/* Internal Buffer defined in SDRAM memory */
uint8_t *uwInternelBuffer;
/* Private function prototypes -----------------------------------------------*/
static void LCD_Config(void);
/* Private typedef -----------------------------------------------------------*/
typedef enum
{
  BUFFER_OFFSET_NONE = 0,
  BUFFER_OFFSET_HALF = 1,
  BUFFER_OFFSET_FULL = 2,
}BUFFER_StateTypeDef;

typedef struct
{
  int   (*DemoFunc)(int);
  uint8_t DemoName[50];
  uint32_t DemoIndex;
}func_efectos;

#define AUDIO_BLOCK_SIZE   ((uint32_t)256)
#define AUDIO_BUFFER_IN    AUDIO_REC_START_ADDR     /* In SDRAM */

/* Private define ------------------------------------------------------------*/
//Para dibujar perillas
static TS_StateTypeDef rawTouchState;
static GUITouchState touchState;
__IO uint32_t isPressed = 0;
static SpriteSheet perilla4241 = { .pixels = perilla42x41x25,
		.spriteWidth = 42, .spriteHeight = 41, .numSprites = 25, .format =
				CM_ARGB8888};//CM_RGB888
static SpriteSheet perilla5252 = { .pixels = perilla52x52x25,
		.spriteWidth = 52, .spriteHeight = 52, .numSprites = 25, .format =
				CM_ARGB8888};//CM_RGB888
static SpriteSheet perilla3535 = { .pixels = perilla35x35x25,
		.spriteWidth = 35, .spriteHeight = 35, .numSprites = 25, .format =
				CM_ARGB8888};//CM_RGB888
static SpriteSheet whaonda = { .pixels = whaondas,
		.spriteWidth = 41, .spriteHeight = 41, .numSprites = 6, .format =
				CM_ARGB8888};//CM_RGB888
static SpriteSheet vibratoonda = { .pixels = vibratoondas,
		.spriteWidth = 35, .spriteHeight = 35, .numSprites = 4, .format =
				CM_ARGB8888};//CM_RGB888
static SpriteSheet tremoloonda = { .pixels = tremoloondas,
		.spriteWidth = 41, .spriteHeight = 41, .numSprites = 4, .format =
				CM_ARGB8888};//CM_RGB888

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint8_t j_home=0;
uint8_t text[30];
extern uint32_t  audio_rec_buffer_state;
unsigned int cont=0;
int Buffer_in[AUDIO_BLOCK_SIZE],Buffer_out[AUDIO_BLOCK_SIZE], i=0, i_audio=0;
//Variables Pasabajos
float control_fcorte=1, cuenta;
int entrada_izq=0, salida_izq=0, entrada_der=0, salida_der=0, pedal_individual=0, seleccion_pedal=0, pedal_prendido=0;
/*func_efectos  efectos[] =
  {
		  {efecto_normal, "efecto normal", 0},
		  {efecto_delay, "efecto delay", 1},
  };*/
GUI* gui[7];
LinkElement *link_menu[2][8], *push_menu[2][8], *link_pedales[13][3], *push_pedales[13];
/* Private function prototypes -----------------------------------------------*/
/*variables REVERV----------------------*/
int FBCF0_line[1617];
int FBCF1_line[1617];
int FBCF2_line[1617];
int FBCF3_line[1617];
int FBCF4_line[1617];
int FBCF5_line[1617];
int FBCF6_line[1617];
int FBCF7_line[1617];//1617

int lpo_r[8];

int AP0_line[225];
int AP1_line[556];
int AP2_line[441];
int AP3_line[341];
/*variables REVERV----------------------*/
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Audio Play demo
  * @param  None
  * @retval None
  */
void AudioLoopback_demo (void)
{

  BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
  //BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FRAME_BUFFER);
  //BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);
  vibrato_parametros();
  tremolo_parametros();
  chorus_parametros();
  flanger_parametros();
  init_autowah();
  Demo_fondito();
  BSP_AUDIO_IN_OUT_Init(INPUT_DEVICE_INPUT_LINE_1, OUTPUT_DEVICE_HEADPHONE, DEFAULT_AUDIO_IN_FREQ, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR);
  audio_rec_buffer_state = BUFFER_OFFSET_NONE;

  /* Start Recording */
  BSP_AUDIO_IN_Record((uint16_t*)Buffer_in, AUDIO_BLOCK_SIZE);

  /* Start Playback */
  BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
  BSP_AUDIO_OUT_Play((uint16_t*)Buffer_out, AUDIO_BLOCK_SIZE);
  BSP_LCD_SelectLayer(1);
  initAppGUI();
  init_pedales_link();
  init_pantalla_link();
  init_push();
  while (1)
  {
    cont++;
	  /* Wait end of half block recording */
   while(audio_rec_buffer_state != BUFFER_OFFSET_HALF)
    {
      if(cont==10)
      {
    	  BSP_TS_GetState(&rawTouchState);
    	  guiUpdateTouch(&rawTouchState, &touchState);
    	  if(pedal_individual==1){
    		  	  	  	  guiUpdate(gui[seleccion_pedal], &touchState);
    		      		  handlePushMenuButton(push_pedales[seleccion_pedal], &touchState);
    		      		  linkUpdatePedales(link_pedales[seleccion_pedal], &touchState);

    	     	  }
    	  else if(pedal_individual==0){
    		  linkUpdate(link_menu[0], &touchState);
    		  linkUpdate(push_menu[0], &touchState);
    	  }
    	  cont=0;
     }

      if (CheckForUserInput() > 0)
      {
        /* Stop Player before close Test */
        BSP_AUDIO_IN_Stop(CODEC_PDWN_SW);
        /* Stop Player before close Test */
        BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
        return;
      }
    }
    audio_rec_buffer_state = BUFFER_OFFSET_NONE;//izquierdo -> mono guitarra red bull
    for(i=0;i<128;i++)
    {
    	if(i%2==0)
    	{
        	salida_izq=((int)Buffer_in[i]<<8)>>8;
        	for(i_audio=0 ;i_audio<6;i_audio++)
        	{

        		if(push_menu[0][i_audio]->states==GUI_ON || push_pedales[i_audio]->states==GUI_ON)
        		{

        			switch (push_menu[0][i_audio]->nombre)
        			{
        				case DELAY:
        					salida_izq=chorus(salida_izq);
        					break;
        				case TREMOLO:
        					salida_izq=tremolo(salida_izq);
        					break;
        				case VIBRATO:
        					salida_izq=vibrato(salida_izq);
        					break;
        				case DISTORSION:
        					salida_izq=distorsion(salida_izq);
        					break;
        				case WHA:
        					salida_izq=autowah(salida_izq);
        					break;
        				case RINGMOD:
        					salida_izq=ringmod(salida_izq);
        					break;
        			}
        		}
        	}
        	Buffer_out[i]=((unsigned int)(salida_izq<<8))>>8;
        }
    }
    /* Wait end of one block recording */
    while(audio_rec_buffer_state != BUFFER_OFFSET_FULL)
    {
      if (CheckForUserInput() > 0)
      {
        /* Stop Player before close Test */
        BSP_AUDIO_IN_Stop(CODEC_PDWN_SW);
        /* Stop Player before close Test */
        BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
        return;
      }
    }
    audio_rec_buffer_state = BUFFER_OFFSET_NONE;
    for(i=128;i<256;i++)
        {
        	if(i%2==0)
        	{
        		salida_izq=((int)Buffer_in[i]<<8)>>8;
        		for(i_audio=0;i_audio<6;i_audio++)
            	{
            		if(push_menu[0][i_audio]->states==GUI_ON || push_pedales[i_audio]->states==GUI_ON)
            		{
            			pedal_prendido++;
            			switch (push_menu[0][i_audio]->nombre)
            			{
            				case DELAY:
            					salida_izq=chorus(salida_izq);
            					break;
            				case TREMOLO:
            					salida_izq=tremolo(salida_izq);
            					break;
            				case VIBRATO:
            					salida_izq=vibrato(salida_izq);
            					break;
            				case DISTORSION:
            					salida_izq=distorsion(salida_izq);
            					break;
            				case WHA:
            					salida_izq=autowah(salida_izq);
            					break;
            				case RINGMOD:
            					salida_izq=ringmod(salida_izq);
            					break;
            			}
            		}
            	}
        		Buffer_out[i]=((unsigned int)(salida_izq<<8))>>8;
        	}
        }
    if (CheckForUserInput() > 0)
    {
      /* Stop recorder */
      BSP_AUDIO_IN_Stop(CODEC_PDWN_SW);
      /* Stop Player before close Test */
      BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
      return;
    }
  }
}

/**
  * @brief  Display Audio Record demo hint
  * @param  None
  * @retval None
  */

void PushUpdateState (LinkElement *e1, LinkElement *e2)
{
	if(pedal_individual==1)
	{
		if (e2->states==GUI_ON)e1->states=GUI_OFF|GUI_HOVER|GUI_DIRTY;
		else e1->states=GUI_ON|GUI_HOVER|GUI_DIRTY;
	}
	else if(pedal_individual==0)
	{
		if (e2->states==GUI_ON)e1->states=GUI_OFF|GUI_HOVER|GUI_DIRTY;
				else e1->states=GUI_ON|GUI_HOVER|GUI_DIRTY;
	}
	else	e1->states=e2->states;
}

static void LCD_Config(void)
  {
    /* LCD Initialization */
    BSP_LCD_Init();

    /* LCD Initialization */
    BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
    BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS+(BSP_LCD_GetXSize()*BSP_LCD_GetYSize()*4));

    /* Enable the LCD */
    BSP_LCD_DisplayOn();

    /* Select the LCD Background Layer  */
    BSP_LCD_SelectLayer(1);

    /* Clear the Background Layer */
    BSP_LCD_Clear(LCD_COLOR_TRANSPARENT);

    /* Select the LCD Foreground Layer  */
    BSP_LCD_SelectLayer(0);

    /* Clear the Foreground Layer */
    BSP_LCD_Clear(LCD_COLOR_TRANSPARENT);

    /* Configure the transparency for foreground and background :
       Increase the transparency */
    BSP_LCD_SetTransparency(0, 255);
    BSP_LCD_SetTransparency(1, 255);
  }


void Demo_fondito(void){
	uwInternelBuffer = (uint8_t *)0xC0260000;
	LCD_Config();
	BSP_SD_Init();
	FATFS_LinkDriver(&SD_Driver, SD_Path);
    DrawScreen(MENU);
  }

void DrawScreen(int num)
{
	char nombre[20];
	sprintf(nombre, "Media/%d.bmp",  num);
	Storage_CheckBitmapFile(nombre, &uwBmplen);
	BSP_LCD_SelectLayer(1);
	Storage_OpenReadFile(uwInternelBuffer, nombre);
	BSP_LCD_DrawBitmap(0, 0, uwInternelBuffer);
	BSP_LCD_SelectLayer(0);
	BSP_LCD_DrawBitmap(0, 0, uwInternelBuffer);
}

void initAppGUI() {
	gui[0] = initGUI(3, &UI_FONT, LCD_COLOR_BLACK, LCD_COLOR_LIGHTGRAY);
	gui[0]->items[0] = guiDialButton(0, "", 174, 36, 0.0f, 0.045f, &perilla5252, Delay_Feedback);//manu phaser 18+30, 18+10
	gui[0]->items[1] = guiDialButton(1, "", 253, 36, 0.0f, 0.045f, &perilla5252,Delay_Time);
	gui[0]->items[2] = guiDialButton(2, "", 220, 82, 0.0f, 0.045f, &perilla4241,Delay_Level);

	gui[1] = initGUI(3, &UI_FONT, LCD_COLOR_BLACK, LCD_COLOR_LIGHTGRAY);
	gui[1]->items[0] = guiDialButton(0, "", 167, 40, 0.0f, 0.045f, &perilla4241, Tremolo_Depth);//manu phaser 18+30, 18+10
	gui[1]->items[1] = guiDialButton(1, "", 269, 39, 0.0f, 0.045f, &perilla4241,Tremolo_Rate);
	gui[1]->items[2] = guiDialButton(2, "", 220, 66, 0.0f, 0.045f, &tremoloonda,Tremolo_Mod);

	gui[2] = initGUI(3, &UI_FONT, LCD_COLOR_BLACK, LCD_COLOR_LIGHTGRAY);
	gui[2]->items[0] = guiDialButton(0, "", 180, 43, 0.0f, 0.045f, &perilla4241, Vibrato_Rate);//manu phaser 18+30, 18+10
	gui[2]->items[1] = guiDialButton(1, "", 257, 43, 0.0f, 0.045f, &perilla4241,Vibrato_Depth);
	gui[2]->items[2] = guiDialButton(2, "", 223, 86, 0.0f, 0.045f, &vibratoonda,Vibrato_Mod);

	gui[3] = initGUI(2, &UI_FONT, LCD_COLOR_BLACK, LCD_COLOR_LIGHTGRAY);
	gui[3]->items[0] = guiDialButton(0, "", 179, 26, 0.0f, 0.045f, &perilla5252, NULL);//manu phaser 18+30, 18+10
	gui[3]->items[1] = guiDialButton(1, "", 249, 26, 0.0f, 0.045f, &perilla5252,NULL);

	gui[4] = initGUI(4, &UI_FONT, LCD_COLOR_BLACK, LCD_COLOR_LIGHTGRAY);
	gui[4]->items[0] = guiDialButton(0, "", 119, 41, 0.0f, 0.045f, &perilla4241, Autowah_Depth);//manu phaser 18+30, 18+10
	gui[4]->items[1] = guiDialButton(1, "", 186, 41, 0.0f, 0.045f, &perilla4241,Autowah_Rate);
	gui[4]->items[2] = guiDialButton(2, "", 251, 41, 0.0f, 0.045f, &perilla4241,Autowah_Volume);
	gui[4]->items[3] = guiDialButton(2, "", 319, 41, 0.0f, 0.045f, &whaonda,Autowah_Mod);

	gui[5] = initGUI(1, &UI_FONT, LCD_COLOR_BLACK, LCD_COLOR_LIGHTGRAY);
	gui[5]->items[0] = guiDialButton(0, "", 223, 73, 0.0f, 0.045f, &perilla3535, Ringmod_Rate);//manu phaser 18+30, 18+10

	//gui[0]->items[3] = guiPushButton(4, "", 10, 80+100, 0.0f, &soloSheet, Push_State_Delay);//Push_State
	guiForceRedraw(gui[0]);
}

void handlePushMenuButton(LinkElement *bt, GUITouchState *touch) {
	if (touch->touchDetected) {
		// touch detected...
		uint16_t x = touch->touchX[0];
		uint16_t y = touch->touchY[0];
		if (x >= bt->x && x < bt->x + bt->width && y >= bt->y
				&& y < bt->y + bt->height) {
			switch (bt->states) {
			case GUI_OFF:
				bt->states |= GUI_HOVER | GUI_DIRTY;
				break;
			case GUI_ON:
				bt->states |= GUI_HOVER | GUI_DIRTY;
				break;
			default:
				break;
			}
		}
	} else if (bt->states & GUI_HOVER) {
		// clear hover flag
		bt->states &= ~((uint16_t) GUI_HOVER);
		// mark dirty (force redraw)
		bt->states |= GUI_DIRTY;
		// invert on/off bitmask
		bt->states ^= GUI_ONOFF_MASK;
		if (bt->callback != NULL) {
			bt->callback(bt);
		}
	}
}

void handleLinkButton(LinkElement *bt, GUITouchState *touch)
{
	//BSP_AUDIO_OUT_Pause();
	if (touch->touchDetected)
	{
		// touch detected...
			uint16_t x = touch->touchX[0];
			uint16_t y = touch->touchY[0];
			if (x >= bt->x && x < bt->x + bt->width && y >= bt->y && y < bt->y + bt->height)
			{
				bt->callback(bt);
			}
	}
	//BSP_AUDIO_OUT_Resume();
}

void linkUpdatePedales(LinkElement **link, GUITouchState *touch)
{

		for (uint8_t i = 0; i < 3; i++) {
			LinkElement *e = link[i];
			e->handler(e, touch);
		}
}

void linkUpdate(LinkElement **link, GUITouchState *touch)
{

		for (uint8_t i = 0; i < 6; i++) {
			LinkElement *e = link[i];
			e->handler(e, touch);
		}
}

LinkElement* guiPush(uint8_t nombre,uint16_t x, uint16_t y,uint16_t width,uint16_t height, GUICallbackLink cb)
{
	LinkElement* e=calloc(1,sizeof(LinkElement));
		e->nombre=nombre;
		e->x=x;
		e->y=y;
		e->width=width;
		e->height=height;
		e->callback=cb;
		e->handler=handlePushMenuButton;
		e->states=GUI_OFF;
		return e;
}

LinkElement* guiLink(uint8_t nombre, uint8_t perillas, uint16_t x, uint16_t y,uint16_t width,uint16_t height, GUICallbackLink cb)
{

	LinkElement* e=calloc(1,sizeof(LinkElement));
	e->nombre=nombre;
	e->cant_per=perillas;
	e->x=x;
	e->y=y;
	e->width=width;
	e->height=height;
	e->callback=cb;
	e->handler=handleLinkButton;
	return e;
}

void LinkCallback(LinkElement *e)
{
	switch(e->nombre)
	{
		case DELAY:
			DrawScreen(DELAY);
			pedal_individual=1;
			seleccion_pedal=DELAY;
		break;
	    case TREMOLO:
		    DrawScreen(TREMOLO);
			pedal_individual=1;
			seleccion_pedal=TREMOLO;
	    break;
	    case VIBRATO:
		    DrawScreen(VIBRATO);
			pedal_individual=1;
			seleccion_pedal=VIBRATO;
	    break;
	    case DISTORSION:
	    	DrawScreen(DISTORSION);
	    	seleccion_pedal=DISTORSION;
			pedal_individual=1;
	    break;
	    case WHA:
		    DrawScreen(WHA);
			pedal_individual=1;
			seleccion_pedal=WHA;
	    break;
	    case RINGMOD:
		    DrawScreen(RINGMOD);
			pedal_individual=1;
			seleccion_pedal=RINGMOD;
	    break;
	}
	//HACER FUNCION PARA ESTO
	for( j_home=0;j_home<6;j_home++)
	{
		PushUpdateState(push_pedales[j_home], push_menu[0][j_home]);
	}
	for (uint8_t j_perillas=0; j_perillas<e->cant_per;j_perillas++)
	{
		gui[seleccion_pedal]->items[j_perillas]->state=GUI_HOVER;
	}

}

void PushCallback(LinkElement *e)
{

	if (e->states & (GUI_DIRTY|GUI_HOVER)) {
			uint8_t states = e->states & GUI_ONOFF_MASK;
			if (pedal_individual==0)
			{
				if(states == GUI_ON)
				{
					switch(e->nombre)
					{
						case DELAY:
							drawBitmapRaw(89,99,5,5, (uint8_t*)delayprendidomenu, CM_ARGB8888, 1);
							break;
						case TREMOLO:
							drawBitmapRaw(231,25,8,8, (uint8_t*)tremoloprendidomenu, CM_ARGB8888, 1);
							break;
						case VIBRATO:
							drawBitmapRaw(341,99,5,5, (uint8_t*)vibratoprendidomenu, CM_ARGB8888, 1);
							break;
						case DISTORSION:
							drawBitmapRaw(90,229,5,5, (uint8_t*)distorsionprendidomenu, CM_ARGB8888, 1);
							break;
						case WHA:
							drawBitmapRaw(232,207,6,6, (uint8_t*)whaprendidomenu, CM_ARGB8888, 1);
							break;
						case RINGMOD:
							drawBitmapRaw(340,239,6,6, (uint8_t*)ringprendidomenu, CM_ARGB8888, 1);
							break;
					}
				}
				else
				{
					switch(e->nombre)
					{
 	 	 	 			case DELAY:
 	 	 	 				drawBitmapRaw(89,99,5,5, (uint8_t*)delayapagadomenu, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case TREMOLO:
 	 	 	 				drawBitmapRaw(231,25,8,8, (uint8_t*)tremoloapagadomenu, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case VIBRATO:
 	 	 	 				drawBitmapRaw(341,99,5,5, (uint8_t*)vibratoapagadomenu, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case DISTORSION:
 	 	 	 				drawBitmapRaw(90,229,5,5, (uint8_t*)distorsionapagadomenu, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case WHA:
 	 	 	 				drawBitmapRaw(232,207,6,6, (uint8_t*)whaapagadomenu, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case RINGMOD:
 	 	 	 				drawBitmapRaw(340,239,6,6, (uint8_t*)ringapagadomenu, CM_ARGB8888, 1);
 	 	 	 				break;
					}
				}
				for( j_home=0;j_home<6;j_home++)
				{
				 	PushUpdateState(push_pedales[j_home], push_menu[0][j_home]);
				}
				// clear dirty flag
			e->states &= ~((uint16_t) GUI_DIRTY);
			}
			else
			{
				if(states == GUI_ON)
				{
					switch(e->nombre)
					{
						case DELAY:

							drawBitmapRaw(196,200,10,9, (uint8_t*)Delayledprendido, CM_ARGB8888, 1);
							break;
						case TREMOLO:
							drawBitmapRaw(228,29,23,24, (uint8_t*)LedTremoloprendido, CM_ARGB8888, 1);
							break;
						case VIBRATO:
							drawBitmapRaw(196,200,11,10, (uint8_t*)Vibratoledprendido, CM_ARGB8888, 1);
							break;
						case DISTORSION:
							drawBitmapRaw(198,198,9,9, (uint8_t*)Distorsionledprendido, CM_ARGB8888, 1);
							break;
						case WHA:
							drawBitmapRaw(230,149,19,18, (uint8_t*)Wahledprendido, CM_ARGB8888, 1);
							break;
						case RINGMOD:
							drawBitmapRaw(195,222,10,10, (uint8_t*)Ringledprendido, CM_ARGB8888, 1);
							break;
					}
				}
				else
				{
					switch(e->nombre)
					{
 	 	 	 			case DELAY:
 	 	 	 				drawBitmapRaw(196,200,10,9, (uint8_t*)Delayledapagado, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case TREMOLO:
 	 	 	 				drawBitmapRaw(228,29,23,24, (uint8_t*)LedTremoloapagado, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case VIBRATO:
 	 	 	 				drawBitmapRaw(196,200,11,10, (uint8_t*)Vibratoledapagado, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case DISTORSION:
 	 	 	 				drawBitmapRaw(198,198,9,9, (uint8_t*)Distorsionledapagado, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case WHA:
 	 	 	 				drawBitmapRaw(230,149,19,18, (uint8_t*)Wahledapagado, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case RINGMOD:
 	 	 	 				drawBitmapRaw(195,222,10,10, (uint8_t*)Ringledapagado, CM_ARGB8888, 1);
 	 	 	 				break;
					}
				}
				// clear dirty flag
				for( j_home=0;j_home<6;j_home++)
				{
				 	PushUpdateState(push_menu[0][j_home], push_pedales[j_home]);
				}
			e->states &= ~((uint16_t) GUI_DIRTY);
			}
	}
}

void LinkPedalCallback(LinkElement *e)
{
	switch(e->nombre)
	{
	 	 case IZQUIERDA:
	     seleccion_pedal--;
	 	 DrawScreen(seleccion_pedal);
	     if(push_pedales[seleccion_pedal]->states==GUI_ON)push_pedales[seleccion_pedal]->states=GUI_OFF|GUI_HOVER|GUI_DIRTY;
	 	 break;
	 	 case DERECHA:
	 	 seleccion_pedal++;
	 	 DrawScreen(seleccion_pedal);
	 	 if(push_pedales[seleccion_pedal]->states==GUI_ON)push_pedales[seleccion_pedal]->states=GUI_OFF|GUI_HOVER|GUI_DIRTY;
	 	 break;
	 	 case HOME:
		 pedal_individual=0;
	 	 DrawScreen(MENU);
	 	 for( j_home=0;j_home<6;j_home++)
	 	 {
		 	 if (j_home!=seleccion_pedal)PushUpdateState(push_menu[0][j_home], push_pedales[j_home]);
	 	 }
	 	 seleccion_pedal=20;
	 	 break;
	}
}

void init_push()
{
	//PEDALES MENU
	push_menu[0][0]=guiPush(DELAY,80,16+110/2,58,110/2,PushCallback);
	push_menu[0][1]=guiPush(TREMOLO,195,16+110/2,80,110/2,PushCallback);
	push_menu[0][2]=guiPush(VIBRATO,332,16+110/2,57,110/2,PushCallback);
	push_menu[0][3]=guiPush(DISTORSION,80,147+110/2,58,110/2,PushCallback);
	push_menu[0][4]=guiPush(WHA,185,158+88/2,100,88/2,PushCallback);
	push_menu[0][5]=guiPush(RINGMOD,326,147+110/2,68,110/2,PushCallback);

	//PEDALES
	push_pedales[0]=guiPush(DELAY,225,189,31,33,PushCallback);
	push_pedales[1]=guiPush(TREMOLO,222,203,36,36,PushCallback);
	push_pedales[2]=guiPush(VIBRATO,221,188,37,37,PushCallback);
	push_pedales[3]=guiPush(DISTORSION,223,184,35,35,PushCallback);
	push_pedales[4]=guiPush(WHA,218,213,41,41,PushCallback);
	push_pedales[5]=guiPush(RINGMOD,223,210,35,34,PushCallback);

}

void init_pantalla_link()
{
	link_menu[0][0]=guiLink(DELAY, 3,80,16,58,110/2,LinkCallback);
	link_menu[0][1]=guiLink(TREMOLO, 3,195,16,80,110/2,LinkCallback);
	link_menu[0][2]=guiLink(VIBRATO, 3,332,16,57,110/2,LinkCallback);
	link_menu[0][3]=guiLink(DISTORSION, 2,80,147,58,110/2,LinkCallback);
	link_menu[0][4]=guiLink(WHA,4,185,158,100,88/2,LinkCallback);
	link_menu[0][5]=guiLink(RINGMOD,1,326,147,68,110/2,LinkCallback);
}

void init_pedales_link()
{
	link_pedales[0][0]=guiLink(DERECHA,0,417,84,61,107,LinkPedalCallback);
	link_pedales[0][1]=guiLink(IZQUIERDA,0,0,84,61,107,LinkPedalCallback);
	link_pedales[0][2]=guiLink(HOME,0,0,0,70,70,LinkPedalCallback);
	link_pedales[1][0]=guiLink(DERECHA,0,417,84,61,107,LinkPedalCallback);
	link_pedales[1][1]=guiLink(IZQUIERDA,0,0,84,61,107,LinkPedalCallback);
	link_pedales[1][2]=guiLink(HOME,0,0,0,70,70,LinkPedalCallback);
	link_pedales[2][0]=guiLink(DERECHA,0,417,84,61,107,LinkPedalCallback);
	link_pedales[2][1]=guiLink(IZQUIERDA,0,0,84,61,107,LinkPedalCallback);
	link_pedales[2][2]=guiLink(HOME,0,0,0,70,70,LinkPedalCallback);
	link_pedales[3][0]=guiLink(DERECHA,0,417,84,61,107,LinkPedalCallback);
	link_pedales[3][1]=guiLink(IZQUIERDA,0,0,84,61,107,LinkPedalCallback);
	link_pedales[3][2]=guiLink(HOME,0,0,0,70,70,LinkPedalCallback);
	link_pedales[4][0]=guiLink(DERECHA,0,417,84,61,107,LinkPedalCallback);
	link_pedales[4][1]=guiLink(IZQUIERDA,0,0,84,61,107,LinkPedalCallback);
	link_pedales[4][2]=guiLink(HOME,0,0,0,70,70,LinkPedalCallback);
	link_pedales[5][0]=guiLink(DERECHA,0,417,84,61,107,LinkPedalCallback);
	link_pedales[5][1]=guiLink(IZQUIERDA,0,0,84,61,107,LinkPedalCallback);
	link_pedales[5][2]=guiLink(HOME,0,0,0,70,70,LinkPedalCallback);

}

/**
  * @brief Manages the DMA Transfer complete interrupt.
  * @param None
  * @retval None
  */
void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
  audio_rec_buffer_state = BUFFER_OFFSET_FULL;
  return;
}

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{
  audio_rec_buffer_state = BUFFER_OFFSET_HALF;
  return;
}

/**
  * @brief  Audio IN Error callback function.
  * @param  None
  * @retval None
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

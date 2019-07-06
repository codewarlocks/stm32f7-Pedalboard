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
//gato
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
//Variables pantalla 2
//Declaracion de objeto pedales
PedalElement* Pedales[12];
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
  init_eq();
  phaser_parametros();
  Demo_fondito();
  initPedals();
  BSP_AUDIO_IN_OUT_Init(INPUT_DEVICE_INPUT_LINE_1, OUTPUT_DEVICE_HEADPHONE, DEFAULT_AUDIO_IN_FREQ, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR);
  audio_rec_buffer_state = BUFFER_OFFSET_NONE;

  /* Start Recording */
  BSP_AUDIO_IN_Record((uint16_t*)Buffer_in, AUDIO_BLOCK_SIZE);

  /* Start Playback */
  BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
  BSP_AUDIO_OUT_Play((uint16_t*)Buffer_out, AUDIO_BLOCK_SIZE);
  BSP_LCD_SelectLayer(1);
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
    	  if(pedal_individual==1)
    	  {
    		  guiUpdate(Pedales[seleccion_pedal]->perilla, &touchState);
    		  handlePushIndividualButton(Pedales[seleccion_pedal], &touchState);
    		  linkRequestHandlers_pedal_individual(Pedales[seleccion_pedal], &touchState);
    	  }
    	  else if(pedal_individual==0){
    		  PushRequestHandler_menu(Pedales, &touchState);
			  linkRequestHandler_menu(Pedales, &touchState);
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
    	if(i%2==0)//Para solo usar el canal izquierdo
    	{
        	salida_izq=((int)Buffer_in[i]<<8)>>8;
        	for(i_audio=0;i_audio<6;i_audio++)
        	{
        		if((Pedales[i_audio])->push->push_state==GUI_ON)
        		{
        			salida_izq=Pedales[i_audio]->efecto(salida_izq);
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
        	if(i%2==0)//Para solo usar el canal izquierdo
        	{
        		salida_izq=((int)Buffer_in[i]<<8)>>8;
        		for(i_audio=0;i_audio<6;i_audio++)
            	{
            		if((Pedales[i_audio])->push->push_state==GUI_ON)
            		{
            			salida_izq=Pedales[i_audio]->efecto(salida_izq);
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

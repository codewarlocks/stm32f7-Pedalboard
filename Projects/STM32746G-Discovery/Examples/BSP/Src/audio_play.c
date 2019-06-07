/**
  ******************************************************************************
  * @file    BSP/Src/audio.c
  * @author  MCD Application Team
  * @brief   This example code shows how to use the audio feature in the
  *          stm32746g_discovery driver
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2016 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdio.h>


/** @addtogroup STM32F7xx_HAL_Examples
  * @{
  */

/** @addtogroup BSP
  * @{
  */

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/

/*Since SysTick is set to 1ms (unless to set it quicker) */ 
/* to run up to 48khz, a buffer around 1000 (or more) is requested*/
/* to run up to 96khz, a buffer around 2000 (or more) is requested*/
#define AUDIO_BUFFER_SIZE       2048
#define AUDIO_DEFAULT_VOLUME    70

/* Audio file size and start address are defined here since the audio file is
   stored in Flash memory as a constant table of 16-bit data */
#define AUDIO_FILE_SIZE               524288
#define AUDIO_START_OFFSET_ADDRESS    0            /* Offset relative to audio file header size */
#define AUDIO_FILE_ADDRESS            0x08080000   /* Audio file address */

/* Private typedef -----------------------------------------------------------*/
typedef enum {
  AUDIO_STATE_IDLE = 0,
  AUDIO_STATE_INIT,
  AUDIO_STATE_PLAYING,
}AUDIO_PLAYBACK_StateTypeDef;

typedef enum {
  BUFFER_OFFSET_NONE = 0,  
  BUFFER_OFFSET_HALF,  
  BUFFER_OFFSET_FULL,     
}BUFFER_StateTypeDef;

typedef struct {
  uint8_t buff[AUDIO_BUFFER_SIZE];
  uint32_t fptr;  
  BUFFER_StateTypeDef state;
}AUDIO_BufferTypeDef;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ALIGN_32BYTES (static AUDIO_BufferTypeDef  buffer_ctl);
static AUDIO_PLAYBACK_StateTypeDef  audio_state;
static uint32_t  AudioStartAddress;
static uint32_t  AudioFileSize;
__IO uint32_t uwVolume = 20;
__IO uint32_t uwPauseEnabledStatus = 0;

static uint32_t AudioFreq[9] = {8000 ,11025, 16000, 22050, 32000, 44100, 48000, 96000, 192000};

/* Private function prototypes -----------------------------------------------*/
static void Audio_SetHint(void);
static uint32_t GetData(void *pdata, uint32_t offset, uint8_t *pbuf, uint32_t NbrOfData);
AUDIO_ErrorTypeDef AUDIO_Start(uint32_t audio_start_address, uint32_t audio_file_size);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Audio Play demo
  * @param  None
  * @retval None
  */
void AudioPlay_demo (void)
{
  uint32_t *AudioFreq_ptr;
  uint8_t FreqStr[256] = {0};
  TS_StateTypeDef  TS_State;

  AudioFreq_ptr = AudioFreq+6; /*AF_48K*/
  uwPauseEnabledStatus = 1; /* 0 when audio is running, 1 when Pause is on */
  uwVolume = 70;

  Audio_SetHint();

  if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_HEADPHONE, uwVolume, *AudioFreq_ptr) == 0)
  {
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 95, (uint8_t *)"  AUDIO CODEC   OK  ", CENTER_MODE);
  }
  else
  {
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 95, (uint8_t *)"  AUDIO CODEC  FAIL ", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 80, (uint8_t *)" Try to reset board ", CENTER_MODE);
  }

  if (BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) != TS_OK)
  {
    BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
    BSP_LCD_SetTextColor(LCD_COLOR_RED);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 65, (uint8_t *)"ERROR", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 50, (uint8_t *)"Touchscreen cannot be initialized", CENTER_MODE);
  }

  /* 
  Start playing the file from a circular buffer, once the DMA is enabled, it is 
  always in running state. Application has to fill the buffer with the audio data 
  using Transfer complete and/or half transfer complete interrupts callbacks 
  (DISCOVERY_AUDIO_TransferComplete_CallBack() or DISCOVERY_AUDIO_HalfTransfer_CallBack()...
  */
  AUDIO_Start(AUDIO_FILE_ADDRESS, AUDIO_FILE_SIZE);

  /* Display the state on the screen */
  BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 80, (uint8_t *)"       PLAYING...     ", CENTER_MODE);

  sprintf((char*)FreqStr, "       VOL:    %3lu     ", uwVolume);
  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 65, (uint8_t *)FreqStr, CENTER_MODE);

  sprintf((char*)FreqStr, "      FREQ: %6lu     ", *AudioFreq_ptr);
  BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 50, (uint8_t *)FreqStr, CENTER_MODE);

  BSP_AUDIO_OUT_SetVolume(70);

  BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);

  /* Infinite loop */
  while (1)
  {
    
    AUDIO_Process();
    
    /* Check in polling mode in touch screen the touch status and coordinates */
    /* if touch occurred                                                      */
    BSP_TS_GetState(&TS_State);
    if(TS_State.touchDetected == 1)
    {
      if (TS_State.touchY[0] < 30)
      {
        /* Increase volume by 5% */
        if (uwVolume < 95)
          uwVolume += 5;
        else
          uwVolume = 100;
        sprintf((char*)FreqStr, "       VOL:    %3lu     ", uwVolume);
        BSP_AUDIO_OUT_SetVolume(uwVolume);
        BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 65, (uint8_t *)FreqStr, CENTER_MODE);
      }
      else if (TS_State.touchY[0] > (BSP_LCD_GetYSize() - 30))
      {
        /* Decrease volume by 5% */
        if (uwVolume > 5)
          uwVolume -= 5;
        else
          uwVolume = 0;
        sprintf((char*)FreqStr, "       VOL:    %3lu     ", uwVolume);
        BSP_AUDIO_OUT_SetVolume(uwVolume);
        BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 65, (uint8_t *)FreqStr, CENTER_MODE);
      }
      else if (TS_State.touchX[0] < 30)
      {
        /*Decrease Frequency */
        if (*AudioFreq_ptr != 8000)
        {
          AudioFreq_ptr--;
          sprintf((char*)FreqStr, "      FREQ: %6lu     ", *AudioFreq_ptr);
          BSP_AUDIO_OUT_Pause();
          BSP_AUDIO_OUT_SetFrequency(*AudioFreq_ptr);
          BSP_AUDIO_OUT_Resume();
          BSP_AUDIO_OUT_SetVolume(uwVolume);
        }
        BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 50, (uint8_t *)FreqStr, CENTER_MODE);
      }
      else if (TS_State.touchX[0] > (BSP_LCD_GetXSize() - 30))
      {
        /* Increase Frequency */
        if (*AudioFreq_ptr != 96000)
        {
          AudioFreq_ptr++;
          sprintf((char*)FreqStr, "      FREQ: %6lu     ", *AudioFreq_ptr);
          BSP_AUDIO_OUT_Pause();
          BSP_AUDIO_OUT_SetFrequency(*AudioFreq_ptr);
          BSP_AUDIO_OUT_Resume();
          BSP_AUDIO_OUT_SetVolume(uwVolume);
        }
        BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 50, (uint8_t *)FreqStr, CENTER_MODE);
      }
      
      /* Wait for touch screen no touch detected */
      do
      {
        BSP_TS_GetState(&TS_State);
      }while(TS_State.touchDetected > 0);
    }
    
    if (TS_State.touchDetected >= 2)
    {
      /* Set Pause / Resume */
      if (uwPauseEnabledStatus == 1)
      { /* Pause is enabled, call Resume */
        BSP_AUDIO_OUT_Resume();
        uwPauseEnabledStatus = 0;
        BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 80, (uint8_t *)"       PLAYING...     ", CENTER_MODE);
      }
      else
      { /* Pause the playback */
        BSP_AUDIO_OUT_Pause();
        uwPauseEnabledStatus = 1;
        BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() - 80, (uint8_t *)"       PAUSE  ...     ", CENTER_MODE);
      }
      
      /* Wait for touch screen no touch detected */
      do
      {
        BSP_TS_GetState(&TS_State);
      }while(TS_State.touchDetected > 0);
    }
    
    if (CheckForUserInput() > 0)
    {
      BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
      BSP_AUDIO_OUT_DeInit();
      return;
    }
  }
}

/**
  * @brief  Display Audio demo hint
  * @param  None
  * @retval None
  */
static void Audio_SetHint(void)
{
  /* Clear the LCD */
  BSP_LCD_Clear(LCD_COLOR_WHITE);

  /* Set Audio Demo description */
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
  BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), 90);
  BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
  BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
  BSP_LCD_SetFont(&Font24);
  BSP_LCD_DisplayStringAt(0, 0, (uint8_t *)"AUDIO EXAMPLE", CENTER_MODE);
  BSP_LCD_SetFont(&Font12);
  BSP_LCD_DisplayStringAt(0, 30, (uint8_t *)"Press User button for next menu", CENTER_MODE);
  BSP_LCD_DisplayStringAt(0, 45, (uint8_t *)"Press Top/Bottom screen to change Volume   ", CENTER_MODE);
  BSP_LCD_DisplayStringAt(0, 60, (uint8_t *)"Press Left/Right screen to change Frequency", CENTER_MODE);
  BSP_LCD_DisplayStringAt(0, 75, (uint8_t *)"Press 2 fingers for Pause/Resume           ", CENTER_MODE);
  /* Set the LCD Text Color */
  BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
  BSP_LCD_DrawRect(10, 100, BSP_LCD_GetXSize() - 20, BSP_LCD_GetYSize() - 110);
  BSP_LCD_DrawRect(11, 101, BSP_LCD_GetXSize() - 22, BSP_LCD_GetYSize() - 112);

}


/**
  * @brief  Starts Audio streaming.    
  * @param  None
  * @retval Audio error
  */ 
AUDIO_ErrorTypeDef AUDIO_Start(uint32_t audio_start_address, uint32_t audio_file_size)
{
  uint32_t bytesread;
  
  buffer_ctl.state = BUFFER_OFFSET_NONE;
  AudioStartAddress = audio_start_address;
  AudioFileSize = audio_file_size;
  bytesread = GetData( (void *)AudioStartAddress,
                      0,
                      &buffer_ctl.buff[0],
                      AUDIO_BUFFER_SIZE);
  if(bytesread > 0)
  {
    /* Clean Data Cache to update the content of the SRAM */
    SCB_CleanDCache_by_Addr((uint32_t*)&buffer_ctl.buff[0], AUDIO_BUFFER_SIZE/2);
        
    BSP_AUDIO_OUT_Play((uint16_t*)&buffer_ctl.buff[0], AUDIO_BUFFER_SIZE);
    audio_state = AUDIO_STATE_PLAYING;      
    buffer_ctl.fptr = bytesread;
    return AUDIO_ERROR_NONE;
  }
  return AUDIO_ERROR_IO;
}

/**
  * @brief  Manages Audio process. 
  * @param  None
  * @retval Audio error
  */
uint8_t AUDIO_Process(void)
{
  uint32_t bytesread;
  AUDIO_ErrorTypeDef error_state = AUDIO_ERROR_NONE;  
  
  switch(audio_state)
  {
  case AUDIO_STATE_PLAYING:
    
    if(buffer_ctl.fptr >= AudioFileSize)
    {
      /* Play audio sample again ... */
      buffer_ctl.fptr = 0; 
      error_state = AUDIO_ERROR_EOF;
    }

    /* 1st half buffer played; so fill it and continue playing from bottom*/
    if(buffer_ctl.state == BUFFER_OFFSET_HALF)
    {
      bytesread = GetData((void *)AudioStartAddress,
                          buffer_ctl.fptr,
                          &buffer_ctl.buff[0],
                          AUDIO_BUFFER_SIZE /2);
      
      if( bytesread >0)
      { 
        buffer_ctl.state = BUFFER_OFFSET_NONE;
        buffer_ctl.fptr += bytesread; 
        
        /* Clean Data Cache to update the content of the SRAM */
        SCB_CleanDCache_by_Addr((uint32_t*)&buffer_ctl.buff[0], AUDIO_BUFFER_SIZE/2);
      }
    }
    
    /* 2nd half buffer played; so fill it and continue playing from top */    
    if(buffer_ctl.state == BUFFER_OFFSET_FULL)
    {
      bytesread = GetData((void *)AudioStartAddress,
                          buffer_ctl.fptr, 
                          &buffer_ctl.buff[AUDIO_BUFFER_SIZE /2],
                          AUDIO_BUFFER_SIZE /2);
      if( bytesread > 0)
      {
        buffer_ctl.state = BUFFER_OFFSET_NONE;
        buffer_ctl.fptr += bytesread;
        
        /* Clean Data Cache to update the content of the SRAM */
        SCB_CleanDCache_by_Addr((uint32_t*)&buffer_ctl.buff[AUDIO_BUFFER_SIZE/2], AUDIO_BUFFER_SIZE/2);
      }
    }
    break;
    
  default:
    error_state = AUDIO_ERROR_NOTREADY;
    break;
  }
  
  return (uint8_t) error_state;
}

/**
  * @brief  Gets Data from storage unit.
  * @param  None
  * @retval None
  */
static uint32_t GetData(void *pdata, uint32_t offset, uint8_t *pbuf, uint32_t NbrOfData)
{
  uint8_t *lptr = pdata;
  uint32_t ReadDataNbr;
  
  ReadDataNbr = 0;
  while(((offset + ReadDataNbr) < AudioFileSize) && (ReadDataNbr < NbrOfData))
  {
    pbuf[ReadDataNbr]= lptr [offset + ReadDataNbr];
    ReadDataNbr++;
  }
  return ReadDataNbr;
}

/*------------------------------------------------------------------------------
       Callbacks implementation:
           the callbacks API are defined __weak in the stm32746g_discovery_audio.c file
           and their implementation should be done the user code if they are needed.
           Below some examples of callback implementations.
  ----------------------------------------------------------------------------*/
/**
  * @brief  Manages the full Transfer complete event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
  if(audio_state == AUDIO_STATE_PLAYING)
  {
    /* allows AUDIO_Process() to refill 2nd part of the buffer  */
    buffer_ctl.state = BUFFER_OFFSET_FULL;
  }
}

/**
  * @brief  Manages the DMA Half Transfer complete event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{
  if(audio_state == AUDIO_STATE_PLAYING)
  {
    /* allows AUDIO_Process() to refill 1st part of the buffer  */
    buffer_ctl.state = BUFFER_OFFSET_HALF;
  }
}

/**
  * @brief  Manages the DMA FIFO error event.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_Error_CallBack(void)
{
  /* Display message on the LCD screen */
  BSP_LCD_SetBackColor(LCD_COLOR_RED);
  BSP_LCD_DisplayStringAtLine(14, (uint8_t *)"       DMA  ERROR     ");

  /* Stop the program with an infinite loop */
  while (BSP_PB_GetState(BUTTON_KEY) != RESET)
  {
    return;
  }

  /* could also generate a system reset to recover from the error */
  /* .... */
}



/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

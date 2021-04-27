/**
 ******************************************************************************
 * @file    BSP/Inc/main.h
 * @author  MCD Application Team
 * @brief   Header for main.c module
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "stm32f7xx_hal.h"
#include "stm32f7xx_hal_adc.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_ts.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_sdram.h"
#include "stm32746g_discovery_sd.h"
#include "stm32746g_discovery_eeprom.h"
#include "stm32746g_discovery_camera.h"
#include "stm32746g_discovery_audio.h"
#include "stm32746g_discovery_qspi.h"
#include "fatfs_storage.h"
#include "ffconf.h"
#include "sd_diskio.h"
//Perillas
#include "gui/gui.h"
/* Macros --------------------------------------------------------------------*/
#ifdef USE_FULL_ASSERT
/* Assert activated */
#define ASSERT(__condition__) \
    do { if (__condition__)                   \
         { assert_failed(__FILE__, __LINE__); \
           while (1);                         \
         }                                    \
    } while (0)
#else
/* Assert not activated : macro has no effect */
#define ASSERT(__condition__) \
    do { if (__condition__) \
         { ErrorCounter++;  \
         }                  \
    }while(0)
#endif /* USE_FULL_ASSERT */

#define RGB565_BYTE_PER_PIXEL        2
#define ARBG8888_BYTE_PER_PIXEL      4

/* Camera have a max resolution of VGA : 640x480 */
#define CAMERA_RES_MAX_X             640
#define CAMERA_RES_MAX_Y             480

#define SDRAM_WRITE_READ_ADDR_OFFSET ((uint32_t)0x0800)
#define SRAM_WRITE_READ_ADDR_OFFSET  SDRAM_WRITE_READ_ADDR_OFFSET

/* The Audio file is flashed with ST-Link Utility @ flash address =  AUDIO_SRC_FILE_ADDRESS */
//#define AUDIO_SRC_FILE_ADDRESS       0x08080000   /* Audio file address in flash */


/* Todos los BUFFERS DE SDRAM QUE ESTAMOS USANDO*/

#define AUDIO_BLOCK_SIZE             ((uint32_t)512)
#define AUDIO_BLOCK_HALFSIZE         ((uint32_t)256)

/**
 * @brief  LCD FB_StartAddress
 * LCD Frame buffer start address : starts at beginning of SDRAM
 */
#define LCD_FRAME_BUFFER             LCD_FB_START_ADDRESS

//#define CAMERA_FRAME_BUFFER       ((uint32_t)(LCD_FRAME_BUFFER + (RK043FN48H_WIDTH * RK043FN48H_HEIGHT * ARBG8888_BYTE_PER_PIXEL)))

/**
 * @brief  SDRAM Write read buffer start address after CAM Frame buffer
 * Assuming Camera frame buffer is of size 640x480 and format RGB565 (16 bits per pixel).
 */
//#define SDRAM_WRITE_READ_ADDR        ((uint32_t)(CAMERA_FRAME_BUFFER + (CAMERA_RES_MAX_X * CAMERA_RES_MAX_Y * RGB565_BYTE_PER_PIXEL)))

#define BMP_IMAGE_BUFFER             LCD_FRAME_BUFFER + (RK043FN48H_WIDTH * RK043FN48H_HEIGHT * ARBG8888_BYTE_PER_PIXEL) * 2     /* El buffer tienen 2 layers (2 * size pantalla) */

#define AUDIO_REC_START_ADDR         BMP_IMAGE_BUFFER + 0x07D000                                                                 /* El buffer tienen 512000 bytes */

#define AUDIO_PLAY_BUFFER            AUDIO_REC_START_ADDR + AUDIO_BLOCK_SIZE * 4                                                 /* El buffer tienen 512 muestrras */

#define AUDIO_CUENTAS_BUFFER         AUDIO_PLAY_BUFFER + AUDIO_BLOCK_SIZE * 4                                                    /* El buffer tienen 512 muestrras */

#define AUDIO_DELAY_BUFFER           AUDIO_CUENTAS_BUFFER + AUDIO_BLOCK_SIZE * 4                                                 /* El delay tienen 50k muestrras */

#define PERILLA_35x35x25             AUDIO_DELAY_BUFFER + (50000 * 4)                                                            /* Se movieron las perilles de flash a SDRAM	*/

#define PERILLA_42x41x25             PERILLA_35x35x25 + (122528)                                                                 /* Se movieron las perilles de flash a SDRAM	*/

#define PERILLA_52x52x25             PERILLA_42x41x25 + (172224)                                                                 /* Se movieron las perilles de flash a SDRAM	*/

#define PERILLA_EQ_SLIDER            PERILLA_52x52x25 + (270432)                                                                 /* Se movieron las perilles de flash a SDRAM	*/

#define AUDIO_WAV_RECORD_BUFFER      PERILLA_EQ_SLIDER + (258976)                                                                /* Nos quedan 40 segundos de grabacion aproximadamente */

typedef enum {
    AUDIO_ERROR_NONE = 0,
    AUDIO_ERROR_NOTREADY,
    AUDIO_ERROR_IO,
    AUDIO_ERROR_EOF,
}AUDIO_ErrorTypeDef;

#ifndef USE_FULL_ASSERT
extern uint32_t ErrorCounter;
#endif
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/

#define COUNT_OF_EXAMPLE(x)              (sizeof(x) / sizeof(BSP_DemoTypedef))

/* Definition for ADCx clock resources */
#define ADCx                   ADC2
#define ADCx_CLK_ENABLE()                __HAL_RCC_ADC2_CLK_ENABLE()
#define ADCx_CHANNEL_GPIO_CLOCK_ENABLE() __HAL_RCC_GPIOA_CLK_ENABLE()

#define ADCx_FORCE_RESET()               __HAL_RCC_ADC_FORCE_RESET()
#define ADCx_RELEASE_RESET()             __HAL_RCC_ADC_RELEASE_RESET()

/* Definition for ADCx Channel Pin */
#define ADCx_CHANNEL_PIN       GPIO_PIN_0
#define ADCx_CHANNEL_GPIO_PORT GPIOA

/* Definition for ADCx's Channel */
#define ADCx_CHANNEL           ADC_CHANNEL_0

/* Definition for ADCx's NVIC */
#define ADCx_IRQn              ADC_IRQn
#define ADCx_IRQHandler        ADC_IRQHandler

#define TIMx                   TIM3
#define TIMx_CLK_ENABLE() __HAL_RCC_TIM3_CLK_ENABLE()


/* Definition for TIMx's NVIC */
#define TIMx_IRQn              TIM3_IRQn
#define TIMx_IRQHandler        TIM3_IRQHandler
/* Exported macro ------------------------------------------------------------*/

//void Demo_Delay (int x, int y, int init);
/* Exported functions ------------------------------------------------------- */
//Efectos
//#include "efectos.h"
uint8_t AUDIO_Process(void);
uint8_t CheckForUserInput(void);
void BSP_LCD_DMA2D_IRQHandler(void);

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line);

#endif
#endif /* __MAIN_H */

/* Enable Features ---------------------------------------------------*/
#define EXTERNAL_WHEEL_ENABLE 0

#define AUDIO_ENABLE          1

#define SCREEN_ENABLE         1
/* CODEC VOLUME SETTINGS */
#define CODEC_OUT_VOLUME 80 // at 80 no noise in headphone
#define CODEC_IN_VOLUME	 92

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

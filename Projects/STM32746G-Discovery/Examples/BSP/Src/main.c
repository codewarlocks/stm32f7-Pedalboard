
/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "main.h"
#include <stdio.h>
#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "gui/leds.h"
#include "gui/gui.h"
#include "stm32f7xx_it.h"

#define DELAY 0
#define OCTAVADOR 1
#define CHORUS 2
#define PHASER 3
#define WAH 4
#define REVERB 5
#define DISTORSION 6
#define FLANGER 7
#define TREMOLO 8
#define VIBRATO 9
#define EQ 10
#define RINGMOD 11
#define IZQUIERDA 0
#define MENU_1 14
#define MENU_2 15
#define DERECHA 1
#define HOME 2
#define IZQUIERDA_MENU 16
#define DERECHA_MENU 17

//Para dibujar perillas
//EJEMPLO WSLDN4 -> EX03
#include <math.h>
#include "ex03/mainex.h"
#include "gui/prototipos.h"

/* Global extern variables ---------------------------------------------------*/
#ifndef USE_FULL_ASSERT
uint32_t ErrorCounter = 0;
#endif

/* Private function prototypes -----------------------------------------------*/
void ADC_Config (void);
void Error_Handler(void);
void SystemClock_Config(void);
void CPU_CACHE_Enable(void);

/* TIM handler declaration */

ADC_ChannelConfTypeDef sConfig;

TIM_HandleTypeDef  htimx;

ADC_HandleTypeDef    AdcHandle;

/* Variable used to get converted value */
__IO uint16_t uhADCxConvertedValue = 0;

char SD_Path[4]; /* SD card logical drive path */
static uint32_t uwBmplen = 0;
/* Internal Buffer defined in SDRAM memory */
static uint8_t *uwInternelBuffer = (uint8_t *)0xC0260000;
/* Private function prototypes -----------------------------------------------*/
void LCD_Config(void);
void Tim3_Patalla_Config (void);
/* Private typedef -----------------------------------------------------------*/
typedef enum
{
	NONE_STATE				 = 0,
	BUFFER_OFFSET_HALF = 1,
	BUFFER_OFFSET_FULL = 2,
	SCREEN_REFRESH 		 = 3,
}machine_states_t;

#define AUDIO_BLOCK_SIZE   ((uint32_t)2800)
#define AUDIO_BLOCK_HALFSIZE   ((uint32_t)1400)
#define AUDIO_BUFFER_IN    AUDIO_REC_START_ADDR     /* In SDRAM */
#define AUDIO_BUFFER_OUT   (AUDIO_REC_START_ADDR + (AUDIO_BLOCK_SIZE * 2)) /* In SDRAM */

/* Private define ------------------------------------------------------------*/
/* Para dibujar perillas*/
TS_StateTypeDef rawTouchState;
GUITouchState touchState;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static volatile machine_states_t  machine_state=NONE_STATE;
int32_t Buffer_in[AUDIO_BLOCK_SIZE]={0}, Buffer_out[AUDIO_BLOCK_SIZE]={0}, Buffer_cuentas[AUDIO_BLOCK_HALFSIZE]={0};
volatile int32_t pedal_individual=0, seleccion_pedal=0, seleccion_menu=MENU_1;
static int32_t cont_muestras=0, cont_pedales=0;

//Declaracion de objeto pedales
PedalElement* Pedales[12];
LinkElementMenu	*Flecha_Menu_Izquierda=NULL, *Flecha_Menu_Derecha=NULL;
/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Main program
 * @param  None
 * @retval None
 */
int main(void)
{
		/* Enable the CPU Cache */
		CPU_CACHE_Enable();

		/* STM32F7xx HAL library initialization:
				 - Configure the Flash prefetch, instruction and Data caches
				 - Configure the Systick to generate an interrupt each 1 msec
				 - Set NVIC Group Priority to 4
				 - Global MSP (MCU Support Package) initialization
		 */
		HAL_Init();
		/* Configure the system clock to 200 Mhz */
		SystemClock_Config();
	
		/*Configuracion del Timer para refresco de pantalla*/
		Tim3_Patalla_Config();
	
		/*Configuracion del ADC*/
		//	ADC_Config();

		/*Configuracion del LED*/
		BSP_LED_Init(LED1);
		
		/*Configuracion del display*/
		BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
		BSP_LCD_Init();
		LCD_Config();
		BSP_SD_Init();
		
		/*Inicializo DMA2D para usar el LLconvertline*/
		init_LL_ConvertLine_DMA2D(CM_RGB888);
		
		/*Inicializo estructuras de pedales*/
		initPedals();
		InitEfectos();
		Demo_fondito();

		/* Start Playback */
		BSP_AUDIO_IN_OUT_Init(INPUT_DEVICE_INPUT_LINE_1, OUTPUT_DEVICE_HEADPHONE, DEFAULT_AUDIO_IN_FREQ, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR);
		BSP_AUDIO_IN_Record((uint16_t*)Buffer_in, AUDIO_BLOCK_SIZE);
		BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
		BSP_AUDIO_IN_SetVolume(90);
		BSP_AUDIO_OUT_Play((uint16_t*)Buffer_out, AUDIO_BLOCK_SIZE);

		if (HAL_TIM_Base_Start_IT(&htimx) != HAL_OK)
		{
			/* Starting Error */
			Error_Handler();
		}
		
		while (1)
		{
				switch (machine_state)
				{
						case NONE_STATE:
						{
								break;
						}
						case BUFFER_OFFSET_HALF:
						{						
							memcpy(Buffer_cuentas, Buffer_in, AUDIO_BLOCK_HALFSIZE*sizeof(int32_t));
							for(cont_muestras=0;cont_muestras<AUDIO_BLOCK_HALFSIZE;cont_muestras++)
							{
									if(cont_muestras%2!=0)//Para solo usar el canal izquierdo
									{
											Buffer_cuentas[cont_muestras]=((int32_t)Buffer_cuentas[cont_muestras]<<8)>>8;
											for(cont_pedales=11;cont_pedales>=0;cont_pedales--)
											{
													if((Pedales[cont_pedales])->push->push_state==GUI_ON)
													{
														Buffer_cuentas[cont_muestras]=Pedales[cont_pedales]->efecto(Buffer_cuentas[cont_muestras]);
													}
											}
											Buffer_cuentas[cont_muestras]=((int32_t)(Buffer_cuentas[cont_muestras]<<8))>>8;
									}
							}
							memcpy(Buffer_out, Buffer_cuentas, AUDIO_BLOCK_HALFSIZE*sizeof(int32_t));
							machine_state=NONE_STATE;										
							break;
						}
						case BUFFER_OFFSET_FULL:
						{
								memcpy(Buffer_cuentas, &Buffer_in[AUDIO_BLOCK_HALFSIZE], AUDIO_BLOCK_HALFSIZE*sizeof(int32_t));
								for(cont_muestras=0;cont_muestras<AUDIO_BLOCK_HALFSIZE;cont_muestras++)
								{
										if(cont_muestras%2!=0)//Para solo usar el canal izquierdo
										{
												Buffer_cuentas[cont_muestras]=((int32_t)Buffer_cuentas[cont_muestras]<<8)>>8;
												for(cont_pedales=11;cont_pedales>=0;cont_pedales--)
												{
														if((Pedales[cont_pedales])->push->push_state==GUI_ON)
														{
															Buffer_cuentas[cont_muestras]=Pedales[cont_pedales]->efecto(Buffer_cuentas[cont_muestras]);
														}
												}
												Buffer_cuentas[cont_muestras]=((int32_t)(Buffer_cuentas[cont_muestras]<<8))>>8;
										}
								}
								memcpy(&Buffer_out[AUDIO_BLOCK_HALFSIZE], Buffer_cuentas, AUDIO_BLOCK_HALFSIZE*sizeof(int32_t));
								machine_state=NONE_STATE;						
								break;
						}
						case SCREEN_REFRESH:
						{
						//		if (HAL_ADC_Start_IT(&AdcHandle) != HAL_OK)
						//		{
						//			/* Start Conversation Error */
						//			Error_Handler();
						//		}
							NVIC_DisableIRQ((IRQn_Type)DMA2_Stream7_IRQn); //DMA2_Stream4_IRQn
							NVIC_DisableIRQ((IRQn_Type)DMA2_Stream4_IRQn); //DMA2_Stream4_IRQn
							NVIC_DisableIRQ(TIMx_IRQn);
							BSP_TS_GetState(&rawTouchState);
							guiUpdateTouch(&rawTouchState, &touchState);
							if(pedal_individual==1)
							{
									if((Pedales[seleccion_pedal]->perilla->perillas[0]->id)!=8)
										guiUpdate(Pedales[seleccion_pedal]->perilla, &touchState);
									handlePushIndividualButton(Pedales[seleccion_pedal], &touchState);
									linkRequestHandlers_pedal_individual(Pedales[seleccion_pedal], &touchState);

							}
							else if(pedal_individual==0)
							{
									PushRequestHandler_menu(Pedales, &touchState);
									linkRequestHandler_menu(Pedales, &touchState);
									linkRequestHandler_Flechas_Menu(Flecha_Menu_Izquierda, &touchState);
									linkRequestHandler_Flechas_Menu(Flecha_Menu_Derecha, &touchState);
							}
							machine_state=NONE_STATE;
							NVIC_EnableIRQ(TIMx_IRQn);
							NVIC_EnableIRQ((IRQn_Type)DMA2_Stream4_IRQn);
							NVIC_EnableIRQ((IRQn_Type)DMA2_Stream7_IRQn);
							break;
						}
						default:
						{
								Error_Handler();
								break;
						}
				}
		}
}

void ADC_Config (void)
{
	/*##-1- Configure the ADC peripheral #######################################*/
	AdcHandle.Instance          = ADCx;

	AdcHandle.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV8;//ADC_CLOCKPRESCALER_PCLK_DIV4;
	AdcHandle.Init.Resolution            = ADC_RESOLUTION_12B;
	AdcHandle.Init.ScanConvMode          = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
	AdcHandle.Init.ContinuousConvMode    = DISABLE;                       /* Continuous mode enabled to have continuous conversion  */
	AdcHandle.Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
	AdcHandle.Init.NbrOfDiscConversion   = 0;
	AdcHandle.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;        /* Conversion start trigged at each external event */
	AdcHandle.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
	AdcHandle.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
	AdcHandle.Init.NbrOfConversion       = 1;
	AdcHandle.Init.DMAContinuousRequests = ENABLE;
	AdcHandle.Init.EOCSelection          = ADC_EOC_SINGLE_CONV;

	if (HAL_ADC_Init(&AdcHandle) != HAL_OK)
	{
		Error_Handler();
	}

	/*##-2- Configure ADC regular channel ######################################*/
	sConfig.Channel      = ADCx_CHANNEL;
	sConfig.Rank         = 1;
	sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;//ADC_SAMPLETIME_3CYCLES; //ADC_SAMPLETIME_480CYCLES
	sConfig.Offset       = 0;

	if (HAL_ADC_ConfigChannel(&AdcHandle, &sConfig) != HAL_OK)
	{
		Error_Handler();
	}
}

void Tim3_Patalla_Config (void)
{
	  /* Time Base configuration */
	  htimx.Instance = TIMx;

	  htimx.Init.Period            = 1800;//5400
	  htimx.Init.Prescaler         = 5000;
	  htimx.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV4;
	  htimx.Init.CounterMode       = TIM_COUNTERMODE_UP;
	  htimx.Init.RepetitionCounter = 0x0;
	  htimx.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

	  if (HAL_TIM_Base_Init(&htimx) != HAL_OK)
	  {
	    /* Timer initialization Error */
	    Error_Handler();
	  }
}

/**
 * @brief  System Clock Configuration
 *         The system Clock is configured as follow :
 *            System Clock source            = PLL (HSE)
 *            SYSCLK(Hz)                     = 200000000
 *            HCLK(Hz)                       = 200000000
 *            AHB Prescaler                  = 1
 *            APB1 Prescaler                 = 4
 *            APB2 Prescaler                 = 2
 *            HSE Frequency(Hz)              = 25000000
 *            PLL_M                          = 25
 *            PLL_N                          = 400
 *            PLL_P                          = 2
 *            PLL_Q                          = 8
 *            VDD(V)                         = 3.3
 *            Main regulator output voltage  = Scale1 mode
 *            Flash Latency(WS)              = 5
 * @param  None
 * @retval None
 */
void SystemClock_Config(void)
{
	HAL_StatusTypeDef ret = HAL_OK;
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	/* Enable Power Control clock */
	__HAL_RCC_PWR_CLK_ENABLE();

	/* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 400;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 8;
	ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
	ASSERT(ret != HAL_OK);

	/* activate the OverDrive */
	ret = HAL_PWREx_ActivateOverDrive();
	ASSERT(ret != HAL_OK);

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
	RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
	ASSERT(ret != HAL_OK);
}

/**
 * @brief  CPU L1-Cache enable.
 * @param  None
 * @retval None
 */
void CPU_CACHE_Enable(void)
{
	/* Enable I-Cache */
	SCB_EnableICache(); //Sin esta cache no llega hacer las cuentas de muchos efectos

	/* Enable D-Cache */
//	SCB_EnableDCache(); //si usa esta cache me limita la ram muchisimo (no mas de 5kb)
}

void LCD_Config(void)
{
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

void InitEfectos()
{
	delayInit();
	octavadorInit();
	chorusInit();
	phaserInit();
	autowahInit();
	reverbInit();
	distorsionInit();
	flangerInit();
	tremoloInit();
	vibratoInit();
	eqInit();
	ringmodInit();
}

void Demo_fondito(void){
	/* LCD Initialization */
	FATFS_LinkDriver(&SD_Driver, SD_Path);
	DrawScreen(MENU_1);
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
	machine_state=BUFFER_OFFSET_FULL;
	return;
}

/**
 * @brief  Manages the DMA Half Transfer complete interrupt.
 * @param  None
 * @retval None
 */
void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{
	machine_state=BUFFER_OFFSET_HALF;
	return;
}

/**
  * @brief  Manages the DMA full Transfer complete event.
  * @retval None
  */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void)
{
	return;
}

/**
  * @brief  Manages the DMA Half Transfer complete event.
  * @retval None
  */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void)
{	
	return;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *AdcHandle)
{
  /* Get the converted value of regular channel */
  DialButtonState *db = (DialButtonState *) (Pedales[PHASER]->perilla->perillas[0]->userData);
  uhADCxConvertedValue = HAL_ADC_GetValue(AdcHandle);
  db->value = (float)(uhADCxConvertedValue/4095.0);
  phaserRate(Pedales[PHASER]->perilla->perillas[0]);
  if(pedal_individual==1 && seleccion_pedal==PHASER)
  {
	  Pedales[PHASER]->perilla->perillas[0]->state=GUI_DIRTY;
	  Pedales[PHASER]->perilla->perillas[0]->render(Pedales[PHASER]->perilla->perillas[0]);
  }
  if (HAL_ADC_Stop_IT(AdcHandle)!= HAL_OK)
  {
	/* Start Conversation Error */
	Error_Handler();
  }

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
		machine_state=SCREEN_REFRESH;
		return;
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void Error_Handler(void)
{
		NVIC_DisableIRQ((IRQn_Type)DMA2_Stream7_IRQn); //DMA2_Stream4_IRQn
		NVIC_DisableIRQ((IRQn_Type)DMA2_Stream4_IRQn); //DMA2_Stream4_IRQn
		NVIC_DisableIRQ(TIMx_IRQn);
	
		while (1)
		{
			/* LED1 blinks */
			BSP_LED_Toggle(LED1);
			HAL_Delay(20);
		}
}



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
#include "gui/prototipos.h"

/* Global extern variables ---------------------------------------------------*/
#ifndef USE_FULL_ASSERT
uint32_t ErrorCounter = 0;
#endif

/* Enable Features ---------------------------------------------------*/
#ifndef EXTERNAL_WHEEL_ENABLE
#define EXTERNAL_WHEEL_ENABLE	0
#endif

#ifndef	AUDIO_ENABLE
#define AUDIO_ENABLE	1
#endif

#ifndef	SCREEN_ENABLE
#define SCREEN_ENABLE	1
#endif

/* Private function prototypes -----------------------------------------------*/
#if EXTERNAL_WHEEL_ENABLE
ADC_ChannelConfTypeDef sConfig;
ADC_HandleTypeDef    AdcHandle;
__IO uint16_t uhADCxConvertedValue = 0; /* Variable used to get converted value */

void ADC_Config (void);
#endif

#if SCREEN_ENABLE
char SD_Path[4]; /* SD card logical drive path */
static uint32_t uwBmplen = 0;
/* Internal Buffer defined in SDRAM memory */ 
//static uint8_t *uwInternelBuffer = (uint8_t *)0xC0260000;
static uint8_t *uwInternelBuffer = (uint8_t *)(BMP_IMAGE_BUFFER);
TS_StateTypeDef rawTouchState;
GUITouchState touchState;
volatile int32_t pedal_individual=0,  seleccion_menu=MENU_1;
LinkElementMenu	*Flecha_Menu_Izquierda=NULL, *Flecha_Menu_Derecha=NULL;
TIM_HandleTypeDef  htimx;
volatile int32_t seleccion_pedal=0;
/* Private function prototypes -----------------------------------------------*/
void LCD_Config(void);
void Tim3_Patalla_Config (void);
#endif

#if AUDIO_ENABLE
//int32_t Buffer_in[AUDIO_BLOCK_SIZE]={0}, Buffer_out[AUDIO_BLOCK_SIZE]={0}, Buffer_cuentas[AUDIO_BLOCK_HALFSIZE]={0};
int32_t *Buffer_in=(int32_t*)AUDIO_REC_START_ADDR, *Buffer_out=(int32_t*)(AUDIO_PLAY_BUFFER), *Buffer_cuentas=(int32_t*)(AUDIO_CUENTAS_BUFFER);
static int32_t cont_muestras=0, cont_pedales=0;
static uint8_t* AUDIO_RECORD_BUFFER = (uint8_t*)(AUDIO_DELAY_BUFFER+(50000*4)); //Alberga 10 segundos de audio, uso TIM3 (200ms) para copiarlas
volatile bool estado_grabacion=false;
static uint32_t timer_grabacion=0, offset_grabacion=0;
volatile unsigned long *timer_counter_reg=(unsigned long *)0x40000424;
volatile bool timer_count=false;
volatile bool block_machine=false;
//wav_header audio_file;
#endif

FIL wav_ptr;

void Error_Handler(void);
void SystemClock_Config(void);
void CPU_CACHE_Enable(void);

/* Private typedef -----------------------------------------------------------*/
typedef enum
{
		NONE_STATE				 							= 	0,
		INIT_SCREEN_AUDIO					=	1,
		BUFFER_OFFSET_HALF 		= 	2,
		BUFFER_OFFSET_FULL 		= 	3,
		SCREEN_REFRESH 		 			= 	4,
		INICIAR_GRABACION					=	5,
		GRABACION_TERMINADA	=	6
}machine_states_t;

/* Private define ------------------------------------------------------------*/
/* Para dibujar perillas*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static volatile machine_states_t  machine_state=INICIAR_GRABACION;

//Declaracion de objeto pedales
PedalElement* Pedales[12];
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
		
		BSP_LED_Init(LED1);
	
		#if SCREEN_ENABLE
		/*Configuracion del Timer para refresco de pantalla*/
		Tim3_Patalla_Config();
		/*Configuracion del display*/
		BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
		BSP_LCD_Init();
		LCD_Config();
//		BSP_SD_Init();
		Storage_Init();
		/*Configuracion del LED*/
		BSP_LED_Init(LED1);
		/*Inicializo DMA2D para usar el LLconvertline*/
		init_LL_ConvertLine_DMA2D(CM_RGB888);
		#endif

		#if EXTERNAL_WHEEL_ENABLE
		/*Configuracion del ADC*/
		ADC_Config();
		#endif

		/*Inicializo estructuras de pedales*/
		FATFS_LinkDriver(&SD_Driver, SD_Path);
		initPedals();
		InitEfectos();
		
		init_wav_header(AUDIO_RECORD_BUFFER);
		
		unsigned int aux_cont_bytes=0, cont_aux=0;
		int32_t	bytes_por_escribir=0;

		#if SCREEN_ENABLE
		Demo_fondito();
		#endif
		
		while (1)
		{
				switch (machine_state)
				{
						case NONE_STATE:
						{
								break;
						}
						case INIT_SCREEN_AUDIO:
						{
								/* Start Playback */
								#if AUDIO_ENABLE
								BSP_AUDIO_IN_OUT_Init(INPUT_DEVICE_INPUT_LINE_1, OUTPUT_DEVICE_HEADPHONE, DEFAULT_AUDIO_IN_FREQ, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR);
								BSP_AUDIO_IN_Record((uint16_t*)Buffer_in, AUDIO_BLOCK_SIZE);
								BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
								BSP_AUDIO_OUT_Play((uint16_t*)Buffer_out, AUDIO_BLOCK_SIZE);
								#endif
							
								machine_state=NONE_STATE;								
								#if SCREEN_ENABLE
							HAL_NVIC_SetPriority(TIM3_IRQn, AUDIO_IN_IRQ_PREPRIO-1, 0);
							if (HAL_TIM_Base_Start_IT(&htimx) != HAL_OK)
								{
										/* Starting Error */
										Error_Handler();
								}
								#endif
								break;
						}
						#if AUDIO_ENABLE
						case BUFFER_OFFSET_HALF:
						{						
								memcpy(Buffer_cuentas, Buffer_in, AUDIO_BLOCK_HALFSIZE*sizeof(int32_t));
								for(cont_muestras=1;cont_muestras<AUDIO_BLOCK_HALFSIZE;cont_muestras+=2)
								{
										if ((Buffer_cuentas[cont_muestras]&0x800000)==0x800000)
										{
												Buffer_cuentas[cont_muestras]=((~Buffer_cuentas[cont_muestras]+1)&0xFFFFFF)*-1;
										}
										for(cont_pedales=11;cont_pedales>=0;cont_pedales--)
										{
												#if SCREEN_ENABLE
												if((Pedales[cont_pedales])->push->push_state==GUI_ON)
												{
														Buffer_cuentas[cont_muestras]=Pedales[cont_pedales]->efecto(Buffer_cuentas[cont_muestras]);
												}
												#endif
										}
										if (Buffer_cuentas[cont_muestras]<0)
										{
												if (estado_grabacion==true)
												{
															AUDIO_RECORD_BUFFER[offset_grabacion]=Buffer_cuentas[cont_muestras]&0xFF;
															offset_grabacion++;
															AUDIO_RECORD_BUFFER[offset_grabacion]=(Buffer_cuentas[cont_muestras]&(0xFF<<8))>>8;
															offset_grabacion++;
															AUDIO_RECORD_BUFFER[offset_grabacion]=(Buffer_cuentas[cont_muestras]&(0xFF<<16))>>16;
															offset_grabacion++;
												}
												Buffer_cuentas[cont_muestras]=(~((-1*Buffer_cuentas[cont_muestras])-1))&0xFFFFFF;
										}
										else
										{
												if (estado_grabacion==true)
												{
															AUDIO_RECORD_BUFFER[offset_grabacion]=Buffer_cuentas[cont_muestras]&0xFF;
															offset_grabacion++;
															AUDIO_RECORD_BUFFER[offset_grabacion]=(Buffer_cuentas[cont_muestras]&(0xFF<<8))>>8;
															offset_grabacion++;
															AUDIO_RECORD_BUFFER[offset_grabacion]=(Buffer_cuentas[cont_muestras]&(0xFF<<16))>>16;
															offset_grabacion++;
												}
										}
								}
								memcpy(Buffer_out, Buffer_cuentas, AUDIO_BLOCK_HALFSIZE*sizeof(int32_t));
								if (machine_state==SCREEN_REFRESH)
								{
										break;
								}
								else
								{
										machine_state=NONE_STATE;
										block_machine=false;
								}
								break;
						}
						/* AUDIO */
						case BUFFER_OFFSET_FULL:
						{
								memcpy(Buffer_cuentas, &Buffer_in[AUDIO_BLOCK_HALFSIZE], AUDIO_BLOCK_HALFSIZE*sizeof(int32_t));
								for(cont_muestras=1;cont_muestras<AUDIO_BLOCK_HALFSIZE;cont_muestras+=2)
									{
											if ((Buffer_cuentas[cont_muestras]&0x800000)==0x800000)
											{
													Buffer_cuentas[cont_muestras]=((~Buffer_cuentas[cont_muestras]+1)&0xFFFFFF)*-1;
											}
											for(cont_pedales=11;cont_pedales>=0;cont_pedales--)
											{
													#if SCREEN_ENABLE
													if((Pedales[cont_pedales])->push->push_state==GUI_ON)
													{
														Buffer_cuentas[cont_muestras]=Pedales[cont_pedales]->efecto(Buffer_cuentas[cont_muestras]);
													}
													#endif
											}
											if (Buffer_cuentas[cont_muestras]<0)
											{
													if (estado_grabacion==true)
													{
															AUDIO_RECORD_BUFFER[offset_grabacion]=Buffer_cuentas[cont_muestras]&0xFF;
															offset_grabacion++;
															AUDIO_RECORD_BUFFER[offset_grabacion]=(Buffer_cuentas[cont_muestras]&(0xFF<<8))>>8;
															offset_grabacion++;
															AUDIO_RECORD_BUFFER[offset_grabacion]=(Buffer_cuentas[cont_muestras]&(0xFF<<16))>>16;
															offset_grabacion++;
													}
													Buffer_cuentas[cont_muestras]=(~((-1*Buffer_cuentas[cont_muestras])-1))&0xFFFFFF;
											}
											else
											{
													if (estado_grabacion==true)
													{
															AUDIO_RECORD_BUFFER[offset_grabacion]=Buffer_cuentas[cont_muestras]&0xFF;
															offset_grabacion++;
															AUDIO_RECORD_BUFFER[offset_grabacion]=(Buffer_cuentas[cont_muestras]&(0xFF<<8))>>8;
															offset_grabacion++;
															AUDIO_RECORD_BUFFER[offset_grabacion]=(Buffer_cuentas[cont_muestras]&(0xFF<<16))>>16;
															offset_grabacion++;
													}
											}
									}
									memcpy(&Buffer_out[AUDIO_BLOCK_HALFSIZE], Buffer_cuentas, AUDIO_BLOCK_HALFSIZE*sizeof(int32_t));
								if (machine_state==SCREEN_REFRESH)
								{
										break;
								}
								else
								{
										machine_state=NONE_STATE;
										block_machine=false;
								}				
								break;
						}
						#endif
						#if SCREEN_ENABLE
						case SCREEN_REFRESH:
						{
							timer_count=!timer_count;	
							#if EXTERNAL_WHEEL_ENABLE
								if (HAL_ADC_Start_IT(&AdcHandle) != HAL_OK)
								{
										/* Start Conversation Error */
										Error_Handler();
								}
								#endif
								NVIC_DisableIRQ((IRQn_Type)DMA2_Stream7_IRQn); //DMA2_Stream4_IRQn
								NVIC_DisableIRQ((IRQn_Type)DMA2_Stream4_IRQn); //DMA2_Stream4_IRQn
								NVIC_DisableIRQ(TIMx_IRQn);
							
								BSP_TS_GetState(&rawTouchState);
								guiUpdateTouch(&rawTouchState, &touchState);
								if(pedal_individual==1)
								{
										if((Pedales[seleccion_pedal]->perilla->perillas[0]->id)!=8)
										{
												guiUpdate(Pedales[seleccion_pedal]->perilla, &touchState);
										}
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
								
								if (estado_grabacion==true)	
								{
										if (timer_grabacion<100)			/* 50=10s, 5=1s (200ms) ||	*/
										{
											 timer_grabacion++;
										}
										else
										{
												machine_state=GRABACION_TERMINADA;
												break;
										}
								}
								timer_count=!timer_count;
								block_machine=false;
								NVIC_EnableIRQ(TIMx_IRQn);
								NVIC_EnableIRQ((IRQn_Type)DMA2_Stream4_IRQn);
								NVIC_EnableIRQ((IRQn_Type)DMA2_Stream7_IRQn);
								break;
						}
						#endif
						case INICIAR_GRABACION:
						{
								BSP_LED_Off(LED1);
								estado_grabacion=true;
								timer_grabacion=0;
								offset_grabacion=44;
								machine_state=INIT_SCREEN_AUDIO;
								break;
						}
						case GRABACION_TERMINADA:
						{
								NVIC_DisableIRQ((IRQn_Type)DMA2_Stream7_IRQn); //DMA2_Stream4_IRQn
								NVIC_DisableIRQ((IRQn_Type)DMA2_Stream4_IRQn); //DMA2_Stream4_IRQn
								NVIC_DisableIRQ(TIMx_IRQn);
								
//								AUDIO_RECORD_BUFFER[4]=((offset_grabacion-300)&0xFF)+36;//0xF0;
//								AUDIO_RECORD_BUFFER[5]=((offset_grabacion-300)&(0xFF<<8))>>8;//0x04;
//								AUDIO_RECORD_BUFFER[6]=((offset_grabacion-300)&(0xFF<<16))>>16;
//								AUDIO_RECORD_BUFFER[7]=(uint8_t)(((offset_grabacion-300)&(0xFF<<24))>>24);//0x00;
//								AUDIO_RECORD_BUFFER[40]=(offset_grabacion-300)&0xFF;//0xF0;
//								AUDIO_RECORD_BUFFER[41]=((offset_grabacion-300)&(0xFF<<8))>>8;//0x04;
//								AUDIO_RECORD_BUFFER[42]=((offset_grabacion-300)&(0xFF<<16))>>16;
//								AUDIO_RECORD_BUFFER[43]=((offset_grabacion-300)&(0xFF<<24))>>24;//0x00;
							
								AUDIO_RECORD_BUFFER[4]=0xD4+36;//0xF0;
								AUDIO_RECORD_BUFFER[5]=0xFB;//0x04;
								AUDIO_RECORD_BUFFER[6]=0x13;
								AUDIO_RECORD_BUFFER[7]=0x00;//0x00;
								AUDIO_RECORD_BUFFER[40]=0xD4;//0xF0;
								AUDIO_RECORD_BUFFER[41]=0xFB;//0x04;
								AUDIO_RECORD_BUFFER[42]=0x13;
								AUDIO_RECORD_BUFFER[43]=0x00;//0x00;
								
								f_open(&wav_ptr, "r1.wav", FA_CREATE_ALWAYS | FA_WRITE);
								for (uint32_t cont_i_bytes=0; cont_i_bytes<5116; cont_i_bytes++)
								{
										f_write(&wav_ptr, &(AUDIO_RECORD_BUFFER[cont_i_bytes*256]), 256, &aux_cont_bytes);
								}
								
								f_close(&wav_ptr);
								
//								f_open(&wav_ptr, "r2.txt", FA_CREATE_ALWAYS | FA_WRITE);
//								sprintf((char*)AUDIO_RECORD_BUFFER, "offset_grabacion=%d, timer_grabacion=%d", offset_grabacion-300, timer_grabacion);
//								f_write(&wav_ptr, AUDIO_RECORD_BUFFER, strlen((char*)AUDIO_RECORD_BUFFER), &aux_cont_bytes);
//								f_close(&wav_ptr);
								
								machine_state=NONE_STATE;
								
								estado_grabacion=false;
								timer_grabacion=0;
								offset_grabacion=0;
								BSP_LED_On(LED1);
								
								block_machine=false;
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
		SCB_EnableDCache(); //si usa esta cache me limita la ram muchisimo (no mas de 5kb)
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

#if SCREEN_ENABLE
void Tim3_Patalla_Config (void)
{
		/* Time Base configuration */
		htimx.Instance = TIMx; //TIM3 utilizado, APB1 (100MHZ de clock source), APB1=HSE/2

		htimx.Init.Period            = 999;//2000-1 0->0.2s (200ms) , 999(100ms)
		htimx.Init.Prescaler         = 9999;
		htimx.Init.ClockDivision     = 0;
		htimx.Init.CounterMode       = TIM_COUNTERMODE_UP;
		htimx.Init.RepetitionCounter = 0x0;
		htimx.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

		if (HAL_TIM_Base_Init(&htimx) != HAL_OK)
		{
				/* Timer initialization Error */
				Error_Handler();
		}
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

void Demo_fondito(void){
		/* LCD Initialization */
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
  * @brief  Period elapsed callback in non blocking mode
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
		machine_state=SCREEN_REFRESH;
		block_machine=true;
		return;
}
#endif /* SCREEN_ENABLE */

#if AUDIO_ENABLE
/**
 * @brief Manages the DMA Transfer complete interrupt.
 * @param None
 * @retval None
 */
void BSP_AUDIO_IN_TransferComplete_CallBack(void)
{
		if (block_machine==false)
		{
				machine_state=BUFFER_OFFSET_FULL;
				block_machine=true;
		}
		return;
}

/**
 * @brief  Manages the DMA Half Transfer complete interrupt.
 * @param  None
 * @retval None
 */
void BSP_AUDIO_IN_HalfTransfer_CallBack(void)
{
		if (block_machine==false)
		{
				machine_state=BUFFER_OFFSET_HALF;
				block_machine=true;
		}
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

#endif /* AUDIO_ENABLE */ 

#if EXTERNAL_WHEEL_ENABLE
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
#endif	/*  EXTERNAL_WHEEL_ENABLE */

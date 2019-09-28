
/* Includes ------------------------------------------------------------------*/
#include "stdlib.h"
#include "main.h"
#include <stdio.h>
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
uint8_t NbLoop = 1;
#ifndef USE_FULL_ASSERT
uint32_t    ErrorCounter = 0;
#endif

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void CPU_CACHE_Enable(void);

char SD_Path[4]; /* SD card logical drive path */
char* pDirectoryFiles[25];//MAX_BMP_FILES
uint8_t  ubNumberOfFiles = 0;
uint32_t uwBmplen = 0, uwBmplen2=0;
/* Internal Buffer defined in SDRAM memory */
uint8_t *uwInternelBuffer = (uint8_t *)0xC0260000;
/* Private function prototypes -----------------------------------------------*/
void LCD_Config(void);
/* Private typedef -----------------------------------------------------------*/
typedef enum
{
	BUFFER_OFFSET_NONE = 0,
	BUFFER_OFFSET_HALF = 1,
	BUFFER_OFFSET_FULL = 2,
}BUFFER_StateTypeDef;

#define AUDIO_BLOCK_SIZE   ((uint32_t)256)
#define AUDIO_BUFFER_IN    AUDIO_REC_START_ADDR     /* In SDRAM */

/* Private define ------------------------------------------------------------*/
//Para dibujar perillas
TS_StateTypeDef rawTouchState;
GUITouchState touchState;
__IO uint32_t isPressed = 0;


/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t  audio_rec_buffer_state;
unsigned int cont=0;
int Buffer_in[AUDIO_BLOCK_SIZE],Buffer_out[AUDIO_BLOCK_SIZE], i=0, i_audio=0;
//Variables Pasabajos
float32_t control_fcorte=1, cuenta;
int entrada_izq=0, salida_izq=0, entrada_der=0, salida_der=0, pedal_individual=0, seleccion_pedal=0, pedal_prendido=0, seleccion_menu=MENU_1;

/* Private function prototypes -----------------------------------------------*/

int FBCF0_line[1617];
int FBCF1_line[1617];
int FBCF2_line[1617];
int FBCF3_line[1617];
int FBCF4_line[1617];
int FBCF5_line[1617];
int FBCF6_line[1617];
int FBCF7_line[1617];

int lpo_r[8];

int AP0_line[225];
int AP1_line[556];
int AP2_line[441];
int AP3_line[341];

//Declaracion de objeto pedales
PedalElement* Pedales[12];
LinkElementMenu* Flecha_Menu_Izquierda, *Flecha_Menu_Derecha;

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
	HAL_Delay(500);
	BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
	HAL_Delay(500);
	BSP_LCD_Init();
	HAL_Delay(500);
	BSP_SD_Init();
	HAL_Delay(500);

	initPedals();
	HAL_Delay(100);
	InitEfectos();
	HAL_Delay(100);
	Demo_fondito();
	HAL_Delay(100);

	audio_rec_buffer_state = BUFFER_OFFSET_NONE;

	/* Start Playback */
	BSP_AUDIO_IN_OUT_Init(INPUT_DEVICE_INPUT_LINE_1, OUTPUT_DEVICE_HEADPHONE, DEFAULT_AUDIO_IN_FREQ, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR);
	BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
	BSP_AUDIO_IN_Record((uint16_t*)Buffer_in, AUDIO_BLOCK_SIZE);
	BSP_AUDIO_OUT_Play((uint16_t*)Buffer_out, AUDIO_BLOCK_SIZE);
	BSP_LCD_SelectLayer(1);
	while (1)
	{
		/* Wait end of half block recording */
		while(audio_rec_buffer_state != BUFFER_OFFSET_HALF)
		{
				NVIC_DisableIRQ((IRQn_Type)DMA2_Stream7_IRQn); //DMA2_Stream4_IRQn
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
				NVIC_EnableIRQ((IRQn_Type)DMA2_Stream7_IRQn);
		}
		audio_rec_buffer_state = BUFFER_OFFSET_NONE;//izquierdo -> mono guitarra red bull

		/* Wait end of one block recording */
		//while(audio_rec_buffer_state != BUFFER_OFFSET_FULL);
		audio_rec_buffer_state = BUFFER_OFFSET_NONE;
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
	SCB_EnableICache();

	/* Enable D-Cache */
	SCB_EnableDCache();
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
	LCD_Config();
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
	audio_rec_buffer_state = BUFFER_OFFSET_FULL;
	for(i=128;i<256;i++)
	{
		if(i%2!=0)//Para solo usar el canal izquierdo
		{
			salida_izq=((int)Buffer_in[i]<<8)>>8;
			for(i_audio=0;i_audio<12;i_audio++)
			{
				if((Pedales[i_audio])->push->push_state==GUI_ON)
				{
					salida_izq=Pedales[i_audio]->efecto(salida_izq);
				}
			}
			Buffer_out[i]=((unsigned int)(salida_izq<<8))>>8;
		}
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
	audio_rec_buffer_state = BUFFER_OFFSET_HALF;
	for(i=0;i<128;i++)
	{
		if(i%2!=0)//Para solo usar el canal izquierdo
		{
			salida_izq=((int)Buffer_in[i]<<8)>>8;
			for(i_audio=0;i_audio<12;i_audio++)
			{
				if((Pedales[i_audio])->push->push_state==GUI_ON)
				{
					salida_izq=Pedales[i_audio]->efecto(salida_izq);
				}
			}
			Buffer_out[i]=((unsigned int)(salida_izq<<8))>>8;
		}
	}
	return;
}

 /*
// Define to prevent recursive inclusion -------------------------------------
#ifndef __MAIN_H
#define __MAIN_H

// Includes ------------------------------------------------------------------
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include <stdlib.h>
#include "fatfs_storage.h"

// FatFs includes component
#include "ff_gen_drv.h"
#include "sd_diskio.h"
#endif // __MAIN_H
FATFS SD_FatFs;  // File system object for SD card logical drive
char SD_Path[4]; // SD card logical drive path
char* pDirectoryFiles[MAX_BMP_FILES];
uint8_t  ubNumberOfFiles = 0;
uint32_t uwBmplen = 0;

// Internal Buffer defined in SDRAM memory
uint8_t *uwInternelBuffer;

static void LCD_Config(void);
  void LCD_Demo(){
	uint32_t counter = 0, transparency = 0;
	uint8_t str[30];
	uwInternelBuffer = (uint8_t *)0xC0260000;
	LCD_Config(); 
	BSP_SD_Init();
	while(BSP_SD_IsDetected() != SD_PRESENT)
	{
        BSP_LCD_SetTextColor(LCD_COLOR_RED);
        BSP_LCD_DisplayStringAtLine(8, (uint8_t*)"  Please insert SD Card                  ");
	}
	BSP_LCD_Clear(LCD_COLOR_BLACK);
	FATFS_LinkDriver(&SD_Driver, SD_Path);
	pDirectoryFiles[counter] = malloc(MAX_BMP_FILE_NAME);
	ubNumberOfFiles = Storage_GetDirectoryBitmapFiles("/Media", pDirectoryFiles);
	sprintf ((char*)str, "Media/%-11.11s", pDirectoryFiles[counter]);
    Storage_CheckBitmapFile((const char*)str, &uwBmplen);
    // Format the string
    sprintf ((char*)str, "Media/%-11.11s", pDirectoryFiles[counter]);
	//Set LCD foreground Layer
	BSP_LCD_SelectLayer(1);
	BSP_LCD_SetTransparency(1, 255);
	// Open a file and copy its content to an internal buffer
	Storage_OpenReadFile(uwInternelBuffer, (const char*)str);
	// Write bmp file on LCD frame buffer
	BSP_LCD_DrawBitmap(0, 0, uwInternelBuffer);
	BSP_LCD_FillCircle(100, 100, 50);
	while(1)
	{

	}
  }

  static void LCD_Config(void)
  {
    // LCD Initialization
    BSP_LCD_Init();

    // LCD Initialization
    BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
    BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS+(BSP_LCD_GetXSize()*BSP_LCD_GetYSize()*4));

    // Enable the LCD
    BSP_LCD_DisplayOn();

    // Select the LCD Background Layer
    BSP_LCD_SelectLayer(0);

    // Clear the Background Layer
    BSP_LCD_Clear(LCD_COLOR_BLACK);

    // Select the LCD Foreground Layer
    BSP_LCD_SelectLayer(1);

    //Clear the Foreground Layer
    BSP_LCD_Clear(LCD_COLOR_BLACK);

    //Configure the transparency for foreground and background :
     //  Increase the transparency
    BSP_LCD_SetTransparency(0, 0);
    BSP_LCD_SetTransparency(1, 100);
  }
*/

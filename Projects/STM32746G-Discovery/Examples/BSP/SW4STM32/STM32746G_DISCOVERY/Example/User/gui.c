#include <stdlib.h>
#include <stdio.h>
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "gui/gui.h"
#include "ex03/macros.h"
#include "main.h"
#include "string.h"
#include "gui/leds.h"
#include "gui/whaondas.h"
#include "gui/perilla35x35x25.h"
#include "gui/perilla42x41x25.h"
#include "gui/perilla52x52x25.h"
#include "gui/slider5pos.h"
#include "gui/prototipos.h"

#define DELAY 0
#define OCTAVADOR 1
#define CHORUS 2
#define PHASER 3
#define WHA 4
#define REVERB 5
#define IZQUIERDA 0
#define MENU 14
#define DERECHA 1
#define HOME 2

#define PERILLA 0
#define SLIDER 1
// private functions

extern int seleccion_pedal;
extern int pedal_individual;
extern PedalElement *Pedales[12];

static void drawElementLabel(GUIElement *e);

static DMA2D_HandleTypeDef hDma2dHandler;

// externally defined structures & functions

extern LTDC_HandleTypeDef hLtdcHandler;
static void LL_ConvertLineToARGB8888(void *src,void *dst,uint32_t xstride,uint32_t color_mode);

//Varibales Perillas
//static SpriteSheet slider5xpos = { .pixels = slider5pos,
//		.spriteWidth = 10, .spriteHeight = 83, .numSprites = 5, .format =
//				CM_ARGB8888};//CM_RGB888
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
//static SpriteSheet vibratoonda = { .pixels = vibratoondas,
//		.spriteWidth = 35, .spriteHeight = 35, .numSprites = 4, .format =
//				CM_ARGB8888};//CM_RGB888
//static SpriteSheet tremoloonda = { .pixels = tremoloondas,
//		.spriteWidth = 41, .spriteHeight = 41, .numSprites = 4, .format =
//				CM_ARGB8888};//CM_RGB888
//

// push button functions

void renderPushButton(GUIElement *bt) {
	if (bt->state & (GUI_DIRTY|GUI_HOVER)) {
		SpriteSheet *sprite = bt->sprite;
		uint8_t state = bt->state & GUI_ONOFF_MASK;
		uint8_t id = state == GUI_ON ? 1 : 0;
		drawSprite(bt->x, bt->y, id, sprite);
		drawElementLabel(bt);
		// clear dirty flag
		bt->state &= ~((uint16_t) GUI_DIRTY);
	}
}

// dial button functions

void handleDialButton(GUIElement *bt, GUITouchState *touch) {
	//BSP_AUDIO_OUT_Pause();
	int16_t delta=0;
	float newVal=0;
	if (touch->touchDetected) {
		// touch detected...
		uint16_t x = touch->touchX[0];
		uint16_t y = touch->touchY[0];
		DialButtonState *db = (DialButtonState *) bt->userData;
		if (bt->state == GUI_HOVER) {
			//int16_t dx = (x - db->startX);
			//int16_t dy = (y - db->startY);
			//int16_t delta = abs(dx) > abs(dy) ? dx : dy;
			if(db->orientacion==PERILLA){
				delta = (x - db->startX);
			}
			else if(db->orientacion==SLIDER){
				delta = (y - db->startY);
			}
			newVal = db->startValue + db->sensitivity * delta;
			db->value = CLAMP(newVal, 0.0f, 1.0f);
			bt->state = GUI_DIRTY | GUI_HOVER;
			if (bt->callback != NULL) {
				bt->callback(bt);
			}
		} else if (x >= bt->x && x < bt->x + bt->width && y >= bt->y
				&& y < bt->y + bt->height) {
			bt->state = GUI_HOVER;
			db->startX = x;
			db->startY = y;
			db->startValue = db->value;
		}
	} else if (bt->state == GUI_HOVER) {
		bt->state = GUI_OFF | GUI_DIRTY;
	}
	//BSP_AUDIO_OUT_Resume();
}

void renderDialButton(GUIElement *bt) {
	if (bt->state & GUI_DIRTY) {
		SpriteSheet *sprite = bt->sprite;
		DialButtonState *db = (DialButtonState *) bt->userData;
		uint8_t id = (uint8_t) (db->value * (float) (sprite->numSprites - 1));
		drawSprite(bt->x, bt->y, id, sprite);
		drawElementLabel(bt);
		bt->state &= ~((uint16_t) GUI_DIRTY);
	}
}

// common functionality

static void drawElementLabel(GUIElement *e) {
	if (e->label != NULL) {
		SpriteSheet *sprite = e->sprite;
		BSP_LCD_DisplayStringAt(e->x, e->y + sprite->spriteHeight + 4,
				(uint8_t*) e->label, LEFT_MODE);//
	}
}

static uint8_t colorModeStrides[] = { 4, 3, 2, 2, 2 };

void drawSprite(uint16_t x, uint16_t y, uint8_t id, SpriteSheet *sprite) {
	uint32_t lcdWidth = BSP_LCD_GetXSize();
	uint32_t address = hLtdcHandler.LayerCfg[LTDC_ACTIVE_LAYER].FBStartAdress
			+ (((lcdWidth * y) + x) << 2);
	uint16_t width = sprite->spriteWidth;
	uint16_t height = sprite->spriteHeight;
	uint8_t *pixels = (uint8_t *) sprite->pixels;
	uint32_t stride = colorModeStrides[sprite->format];
	pixels += (id * width * height) * stride;
	stride *= width;
	lcdWidth <<= 2;
	while (--height) {
		LL_ConvertLineToARGB8888(pixels, (uint32_t *) address, width,
				sprite->format);
		address += lcdWidth;
		pixels += stride;
	}
}

void drawBitmapRaw(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
		uint8_t *pixels, uint32_t colorMode, uint8_t active_layer) {
	uint32_t lcdWidth = BSP_LCD_GetXSize();
	uint32_t address = hLtdcHandler.LayerCfg[active_layer].FBStartAdress
			+ (((lcdWidth * y) + x) << 2);//LTDC_ACTIVE_LAYER
	uint16_t stride = width * colorModeStrides[colorMode];
	lcdWidth <<= 2;
	while (--height) {
		LL_ConvertLineToARGB8888(pixels, (uint32_t *) address, width,
				colorMode);
		address += lcdWidth;
		pixels += stride;
	}
}

// GUI element constructors

GUIElement *guiElement(uint8_t id, char *label, uint16_t x, uint16_t y,
		SpriteSheet *sprite, GUICallback cb) {
	GUIElement *e = (GUIElement *) calloc(1, sizeof(GUIElement));
	e->id = id;
	e->x = x;
	e->y = y;
	e->width = sprite->spriteWidth;
	e->height = sprite->spriteHeight;
	e->sprite = sprite;
	e->label = label;
	e->callback = cb;
	e->state = GUI_OFF | GUI_DIRTY;
	return e;
}

GUIElement *guiDialButton(uint8_t id, char *label, uint16_t x, uint16_t y,
		float val, float sens, uint8_t orientacion, SpriteSheet *sprite, GUICallback cb) {
	GUIElement *e = guiElement(id, label, x, y, sprite, cb);
	DialButtonState *db = (DialButtonState *) calloc(1,sizeof(DialButtonState));
	e->handler = handleDialButton;
	e->render = renderDialButton;
	e->userData = db;
	db->value = val;
	db->sensitivity = sens;
	db->orientacion = orientacion;
	return e;
}

GUI *initGUI(uint8_t num, sFONT *font, uint32_t bgCol, uint32_t textCol) {
	GUI *gui = (GUI *) calloc(1, sizeof(GUI));
	gui->items = (GUIElement **) calloc(num, sizeof(GUIElement *));
	gui->numItems = num;
	gui->font = font;
	gui->bgColor = bgCol;
	gui->textColor = textCol;
	return gui;
}

void guiForceRedraw(GUI *gui) {
	for (uint8_t i = 0; i < gui->numItems; i++) {
		gui->items[i]->state |= GUI_DIRTY;
	}
}

void guiUpdate(PerillaElement *gui, GUITouchState *touch) {
	for (uint8_t i = 0; i < gui->numItems; i++) {
		GUIElement *e = gui->perillas[i];
		e->handler(e, touch);
		e->render(e);
	}
}

void guiUpdateTouch(TS_StateTypeDef *raw, GUITouchState *touch) {
	touch->touchDetected = raw->touchDetected;
	touch->touchX[0] = raw->touchX[0];
	touch->touchY[0] = raw->touchY[0];
}

static void LL_ConvertLineToARGB8888(void *src,
                                     void *dst,
                                     uint32_t xstride,
                                     uint32_t color_mode) {
  hDma2dHandler.Init.Mode         = DMA2D_M2M_PFC;
  hDma2dHandler.Init.ColorMode    = DMA2D_ARGB8888;
  hDma2dHandler.Init.OutputOffset = 0;

  hDma2dHandler.LayerCfg[1].AlphaMode      = DMA2D_NO_MODIF_ALPHA;//
  hDma2dHandler.LayerCfg[1].InputAlpha     = 0xFF;//FF
  hDma2dHandler.LayerCfg[1].InputColorMode = color_mode;
  hDma2dHandler.LayerCfg[1].InputOffset    = 0;
  hDma2dHandler.Instance                   = DMA2D;

  if (HAL_DMA2D_Init(&hDma2dHandler) == HAL_OK) {
    if (HAL_DMA2D_ConfigLayer(&hDma2dHandler, 1) == HAL_OK) {
      if (HAL_DMA2D_Start(&hDma2dHandler, (uint32_t)src, (uint32_t)dst, xstride,
                          1) == HAL_OK) {
        HAL_DMA2D_PollForTransfer(&hDma2dHandler, 10);
      }
    }
  }
}

//Escritas por nosotros

void linkRequestHandlers_pedal_individual(PedalElement *bt, GUITouchState *touch)
{
	bt->botones->flecha_derecha->handler(bt, touch);
	bt->botones->flecha_izquierda->handler(bt, touch);
	bt->botones->home->handler(bt, touch);
}

void linkRequestHandler_menu(PedalElement **bt, GUITouchState *touch)
{
		for (uint8_t i = 0; i < 6; i++)
		{
			bt[i]->link->handler(bt[i], touch);
		}
}

void PushRequestHandler_menu(PedalElement **bt, GUITouchState *touch)
{
		for (uint8_t i = 0; i < 6; i++)
		{
			bt[i]->push->push_menu->handler(bt[i], touch);
		}
}

//Init / Redraw
LinkElement* initPushLink(uint8_t nombre,uint16_t x, uint16_t y,uint16_t width,uint16_t height, GUICallbackLink cb, LinkHandler han)
{
	LinkElement* e=calloc(1,sizeof(LinkElement));
		e->nombre=nombre;
		e->x=x;
		e->y=y;
		e->width=width;
		e->height=height;
		e->callback=cb;
		e->handler=han;
		return e;
}

PerillaElement* initPerilla (uint8_t num)
{
	PerillaElement *gui = (PerillaElement *) calloc(1, sizeof(PerillaElement));
	gui->perillas = (GUIElement **) calloc(num, sizeof(GUIElement *));
	gui->numItems = num;
	return gui;
}

void PedalForceRedraw(PedalElement *gui) {
	for (uint8_t i = 0; i < gui->perilla->numItems; i++) {
		gui->perilla->perillas[i]->state |= GUI_DIRTY;
	}
	if(gui->push->push_state==GUI_ON)gui->push->push_state=GUI_DIRTY|GUI_HOVER|GUI_OFF;
}

void MenuForceRedraw(void)
{
	for(int aux_i=0; aux_i<6; aux_i++)
	{
		if(Pedales[aux_i]->push->push_state==GUI_ON)Pedales[aux_i]->push->push_state=GUI_DIRTY|GUI_HOVER|GUI_OFF;
	}
}
//

//Handles
void handlePushMenuButton(PedalElement *bt, GUITouchState *touch) {
	if (touch->touchDetected) {
		// touch detected...
		uint16_t x = touch->touchX[0];
		uint16_t y = touch->touchY[0];
		if (x >= bt->push->push_menu->x && x < bt->push->push_menu->x + bt->push->push_menu->width && y >= bt->push->push_menu->y && y < bt->push->push_menu->y + bt->push->push_menu->height) {
			switch (bt->push->push_state) {
			case GUI_OFF:
				bt->push->push_state |= GUI_HOVER | GUI_DIRTY;
				break;
			case GUI_ON:
				bt->push->push_state |= GUI_HOVER | GUI_DIRTY;
				break;
			default:
				break;
			}
		}
	} else if (bt->push->push_state & GUI_HOVER) {
		// clear hover flag
		bt->push->push_state &= ~((uint16_t) GUI_HOVER);
		// mark dirty (force redraw)
		bt->push->push_state |= GUI_DIRTY;
		// invert on/off bitmask
		bt->push->push_state ^= GUI_ONOFF_MASK;
		if (bt->push->push_menu->callback != NULL) {
			bt->push->push_menu->callback(bt);
		}
	}
}

void handlePushIndividualButton(PedalElement *bt, GUITouchState *touch) {
	if (touch->touchDetected) {
		// touch detected...
		uint16_t x = touch->touchX[0];
		uint16_t y = touch->touchY[0];
		if (x >= bt->push->push_indiv->x && x < bt->push->push_indiv->x + bt->push->push_indiv->width && y >= bt->push->push_indiv->y && y < bt->push->push_indiv->y + bt->push->push_indiv->height) {
			switch (bt->push->push_state) {
			case GUI_OFF:
				bt->push->push_state |= GUI_HOVER | GUI_DIRTY;
				break;
			case GUI_ON:
				bt->push->push_state |= GUI_HOVER | GUI_DIRTY;
				break;
			default:
				break;
			}
		}
	} else if (bt->push->push_state & GUI_HOVER) {
		// clear hover flag
		bt->push->push_state &= ~((uint16_t) GUI_HOVER);
		// mark dirty (force redraw)
		bt->push->push_state |= GUI_DIRTY;
		// invert on/off bitmask
		bt->push->push_state ^= GUI_ONOFF_MASK;
		if (bt->push->push_indiv->callback != NULL) {
			bt->push->push_indiv->callback(bt);
		}
	}
}

void handleLinkButton(PedalElement *bt, GUITouchState *touch)
{
	if (touch->touchDetected)
	{
		// touch detected...
			uint16_t x = touch->touchX[0];
			uint16_t y = touch->touchY[0];
			if (x >= bt->link->x && x < bt->link->x + bt->link->width && y >= bt->link->y && y < bt->link->y + bt->link->height)
			{
				bt->link->callback(bt);
			}
	}
}

void handleDerechaButton(PedalElement *bt, GUITouchState *touch)
{
	if (touch->touchDetected)
	{
		// touch detected...
			uint16_t x = touch->touchX[0];
			uint16_t y = touch->touchY[0];
			if (x >= bt->botones->flecha_derecha->x && x < bt->botones->flecha_derecha->x + bt->botones->flecha_derecha->width && y >= bt->botones->flecha_derecha->y && y < bt->botones->flecha_derecha->y + bt->botones->flecha_derecha->height)
			{
				bt->botones->flecha_derecha->callback(bt);
			}
	}
}

void handleIzquierdaButton(PedalElement *bt, GUITouchState *touch)
{
	if (touch->touchDetected)
	{
		// touch detected...
			uint16_t x = touch->touchX[0];
			uint16_t y = touch->touchY[0];
			if (x >= bt->botones->flecha_izquierda->x && x < bt->botones->flecha_izquierda->x + bt->botones->flecha_izquierda->width && y >= bt->botones->flecha_izquierda->y && y < bt->botones->flecha_izquierda->y + bt->botones->flecha_izquierda->height)
			{
				bt->botones->flecha_izquierda->callback(bt);
			}
	}
}

void handleHomeButton(PedalElement *bt, GUITouchState *touch)
{
	if (touch->touchDetected)
	{
		// touch detected...
			uint16_t x = touch->touchX[0];
			uint16_t y = touch->touchY[0];
			if (x >= bt->botones->home->x && x < bt->botones->home->x + bt->botones->home->width && y >= bt->botones->home->y && y < bt->botones->home->y + bt->botones->home->height)
			{
				bt->botones->home->callback(bt);
			}
	}
}
//

//Callbacks
void LinkCallback(PedalElement *e)
{
	switch(e->link->nombre)
	{
		case DELAY:
			DrawScreen(DELAY);
			pedal_individual=1;
			seleccion_pedal=DELAY;
		break;
	    case OCTAVADOR:
		    DrawScreen(OCTAVADOR);
			pedal_individual=1;
			//seleccion_pedal=OCTAVADOR;
	    break;
	    case CHORUS:
		    DrawScreen(CHORUS);
			pedal_individual=1;
			seleccion_pedal=CHORUS;
	    break;
	    case PHASER:
	    	DrawScreen(PHASER);
	    	seleccion_pedal=PHASER;
			pedal_individual=1;
	    break;
	    case WHA:
		    DrawScreen(WHA);
			pedal_individual=1;
			seleccion_pedal=WHA;
	    break;
	    case REVERB:
		    DrawScreen(REVERB);
			pedal_individual=1;
			seleccion_pedal=REVERB;
	    break;
	}
	//PedalForceRedraw(e);

}

void PushCallback(PedalElement *e)
{

	if (e->push->push_state & (GUI_DIRTY|GUI_HOVER))
	{
			uint8_t states = e->push->push_state & GUI_ONOFF_MASK;
			if (pedal_individual==0)
			{
				if(states == GUI_ON)
				{
					switch(e->push->push_menu->nombre)
					{
						case DELAY:
							drawBitmapRaw(85,96,5,5, (uint8_t*)ledprendidodelaymenu, CM_ARGB8888, 1);
							break;
						case OCTAVADOR:
							drawBitmapRaw(237,78,6,6, (uint8_t*)ledprendidooctavadormenu, CM_ARGB8888, 1);
							break;
						case CHORUS:
							drawBitmapRaw(375,18,3,3, (uint8_t*)ledprendidochorusmenu, CM_ARGB8888, 1);
							break;
						case PHASER:
							drawBitmapRaw(103,210,5,6, (uint8_t*)ledprendidophasermenu, CM_ARGB8888, 1);
							break;
						case WHA:
							drawBitmapRaw(236,209,8,7, (uint8_t*)ledprendidowahmenu, CM_ARGB8888, 1);
							break;
						case REVERB:
							drawBitmapRaw(375,211,5,5, (uint8_t*)ledprendidoreverbmenu, CM_ARGB8888, 1);
							break;
					}
				}
				else
				{
					switch(e->push->push_menu->nombre)
					{
 	 	 	 			case DELAY:
 	 	 	 				drawBitmapRaw(85,96,5,5, (uint8_t*)ledapagadodelaymenu, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case OCTAVADOR:
 	 	 	 				drawBitmapRaw(237,78,6,6, (uint8_t*)ledapagadooctavadormenu, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case CHORUS:
 	 	 	 				drawBitmapRaw(375,18,3,3, (uint8_t*)ledapagadochorusmenu, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case PHASER:
 	 	 	 				drawBitmapRaw(103,210,5,6, (uint8_t*)ledapagadophasermenu, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case WHA:
 	 	 	 				drawBitmapRaw(236,209,8,7, (uint8_t*)ledapagadowahmenu, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case REVERB:
 	 	 	 				drawBitmapRaw(375,211,5,5, (uint8_t*)ledapagadoreverbmenu, CM_ARGB8888, 1);
 	 	 	 				break;
					}
				}
				// clear dirty flag
			e->push->push_state &= ~((uint16_t) GUI_DIRTY);
			}
			else
			{
				if(states == GUI_ON)
				{
					switch(e->push->push_indiv->nombre)
					{
						case DELAY:

							drawBitmapRaw(196,200,10,9, (uint8_t*)ledprendidodelaypedal, CM_ARGB8888, 1);
							break;
						case OCTAVADOR:
							drawBitmapRaw(233,159,15,16, (uint8_t*)ledprendidooctavadorpedal, CM_ARGB8888, 1);
							break;
						case CHORUS:
							drawBitmapRaw(236,23,7,6, (uint8_t*)ledprendidochoruspedal, CM_ARGB8888, 1);
							break;
						case PHASER:
							drawBitmapRaw(235,150,12,12, (uint8_t*)ledprendidophaserpedal, CM_ARGB8888, 1);
							break;
						case WHA:
							drawBitmapRaw(230,148,19,18, (uint8_t*)ledprendidowahpedal, CM_ARGB8888, 1);
							break;
						case REVERB:
							drawBitmapRaw(236,151,13,13, (uint8_t*)ledprendidoreverbpedal, CM_ARGB8888, 1);
							break;
					}
				}
				else
				{
					switch(e->push->push_indiv->nombre)
					{
 	 	 	 			case DELAY:
 	 	 	 				drawBitmapRaw(196,200,10,9, (uint8_t*)ledapagadodelaypedal, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case OCTAVADOR:
 	 	 	 				drawBitmapRaw(233,159,15,16, (uint8_t*)ledapagadooctavadorpedal, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case CHORUS:
 	 	 	 				drawBitmapRaw(236,23,7,6, (uint8_t*)ledapagadochoruspedal, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case PHASER:
 	 	 	 				drawBitmapRaw(235,150,12,12, (uint8_t*)ledapagadophaserpedal, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case WHA:
 	 	 	 				drawBitmapRaw(230,148,19,18, (uint8_t*)ledapagadowahpedal, CM_ARGB8888, 1);
 	 	 	 				break;
 	 	 	 			case REVERB:
 	 	 	 				drawBitmapRaw(236,151,13,13, (uint8_t*)ledapagadoreverbpedal, CM_ARGB8888, 1);
 	 	 	 				break;
					}
				}
				// clear dirty flag
			e->push->push_state &= ~((uint16_t) GUI_DIRTY);
			}
	}
}

void LinkDerechaCallback(PedalElement *e)
{
	 seleccion_pedal++;
	 DrawScreen(seleccion_pedal);
	 //PedalForceRedraw(Pedales[seleccion_pedal]);
}

void LinkIzquierdaCallback(PedalElement *e)
{
	seleccion_pedal--;
    DrawScreen(seleccion_pedal);
    //PedalForceRedraw(Pedales[seleccion_pedal]);
}

void LinkHomeCallback(PedalElement *e)
{
	pedal_individual=0;
	DrawScreen(MENU);
	MenuForceRedraw();
	seleccion_pedal=20;
}

void initPedals() {
	//Inicializo las perillas
	//Delay
	Pedales[0]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[0]->perilla = initPerilla(3);
	Pedales[0]->perilla->perillas[0] = guiDialButton(0, "", 174, 36, 0.0f, 0.045f, PERILLA, &perilla5252, Delay_Feedback);
	Pedales[0]->perilla->perillas[1] = guiDialButton(1, "", 253, 36, 0.0f, 0.045f, PERILLA, &perilla5252,Delay_Time);
	Pedales[0]->perilla->perillas[2] = guiDialButton(2, "", 220, 82, 0.0f, 0.045f, PERILLA, &perilla4241,Delay_Level);
	//Octavador
	Pedales[1]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[1]->perilla = initPerilla(1);
	Pedales[1]->perilla->perillas[0] =NULL;
	//Chorus
	Pedales[2]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[2]->perilla = initPerilla(2);
	Pedales[2]->perilla->perillas[0] = guiDialButton(0, "", 187, 22, 0.0f, 0.045f, PERILLA, &perilla4241, Chorus_Rate);
	Pedales[2]->perilla->perillas[1] = guiDialButton(1, "", 250, 22, 0.0f, 0.045f, PERILLA, &perilla4241, Chorus_Depth);
	//Phaser
	Pedales[3]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[3]->perilla = initPerilla(1);
	Pedales[3]->perilla->perillas[0] = guiDialButton(0, "", 214, 27, 0.0f, 0.045f, PERILLA, &perilla5252, NULL);
	//Autowah
	Pedales[4]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[4]->perilla = initPerilla(4);
	Pedales[4]->perilla->perillas[0] = guiDialButton(0, "", 119, 41, 0.0f, 0.045f, PERILLA, &perilla4241, Autowah_Depth);
	Pedales[4]->perilla->perillas[1] = guiDialButton(1, "", 186, 41, 0.0f, 0.045f, PERILLA, &perilla4241,Autowah_Rate);
	Pedales[4]->perilla->perillas[2] = guiDialButton(2, "", 251, 41, 0.0f, 0.045f, PERILLA, &perilla4241,Autowah_Volume);
	Pedales[4]->perilla->perillas[3] = guiDialButton(3, "", 319, 41, 0.0f, 0.045f, PERILLA, &whaonda,Autowah_Mod);
	//Reverb
	Pedales[5]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[5]->perilla = initPerilla(2);
	Pedales[5]->perilla->perillas[0] = guiDialButton(0, "", 180, 63, 0.0f, 0.045f, PERILLA, &perilla4241, Reverb_Decay);
	Pedales[5]->perilla->perillas[1] = guiDialButton(1, "", 222, 22, 0.0f, 0.045f, PERILLA, &perilla4241, Reverb_Mix);
	//

	//Inicializo los Push (3pdt)
	//PEDALES Menu
	Pedales[0]->push=(PushElement*)calloc(1, sizeof(PushElement));
	Pedales[0]->push->push_menu=initPushLink(DELAY,76,13+110/2,58,110/2,PushCallback, handlePushMenuButton);

	Pedales[1]->push=(PushElement*)calloc(1, sizeof(PushElement));
	Pedales[1]->push->push_menu=initPushLink(OCTAVADOR,198,13+110/2,84,110/2,PushCallback, handlePushMenuButton);

	Pedales[2]->push=(PushElement*)calloc(1, sizeof(PushElement));
	Pedales[2]->push->push_menu=initPushLink(CHORUS,346,13+110/2,61,110/2,PushCallback, handlePushMenuButton);

	Pedales[3]->push=(PushElement*)calloc(1, sizeof(PushElement));
	Pedales[3]->push->push_menu=initPushLink(PHASER,73,149+110/2,64,110/2,PushCallback, handlePushMenuButton);

	Pedales[4]->push=(PushElement*)calloc(1, sizeof(PushElement));
	Pedales[4]->push->push_menu=initPushLink(WHA,190,160+88/2,100,88/2,PushCallback, handlePushMenuButton);

	Pedales[5]->push=(PushElement*)calloc(1, sizeof(PushElement));
	Pedales[5]->push->push_menu=initPushLink(REVERB,343,149+110/2,67,110/2,PushCallback, handlePushMenuButton);

	//PEDALES Individuales
	Pedales[0]->push->push_indiv=initPushLink(DELAY,225,189,31,33,PushCallback, handlePushIndividualButton);
	Pedales[1]->push->push_indiv=initPushLink(OCTAVADOR,219,180,42,42,PushCallback, handlePushIndividualButton);
	Pedales[2]->push->push_indiv=initPushLink(CHORUS,180,163,117,77,PushCallback, handlePushIndividualButton);
	Pedales[3]->push->push_indiv=initPushLink(PHASER,222,171,37,37,PushCallback, handlePushIndividualButton);
	Pedales[4]->push->push_indiv=initPushLink(WHA,218,213,41,41,PushCallback, handlePushIndividualButton);
	Pedales[5]->push->push_indiv=initPushLink(REVERB,180,163,117,77,PushCallback, handlePushIndividualButton);

	//PEDALES estados
	Pedales[0]->push->push_state=GUI_OFF;
	Pedales[1]->push->push_state=GUI_OFF;
	Pedales[2]->push->push_state=GUI_OFF;
	Pedales[3]->push->push_state=GUI_OFF;
	Pedales[4]->push->push_state=GUI_OFF;
	Pedales[5]->push->push_state=GUI_OFF;
	//

	//Inicializo los links de los pedales
	Pedales[0]->link=(LinkElement*)calloc(1,sizeof(LinkElement));
	Pedales[0]->link=initPushLink(DELAY,76,13,58,110/2,LinkCallback, handleLinkButton);

	Pedales[1]->link=(LinkElement*)calloc(1,sizeof(LinkElement));
	Pedales[1]->link=initPushLink(OCTAVADOR,198,13,84,110/2,LinkCallback, handleLinkButton);

	Pedales[2]->link=(LinkElement*)calloc(1,sizeof(LinkElement));
	Pedales[2]->link=initPushLink(CHORUS,346,13,61,110/2,LinkCallback, handleLinkButton);

	Pedales[3]->link=(LinkElement*)calloc(1,sizeof(LinkElement));
	Pedales[3]->link=initPushLink(PHASER,73,149,64,110/2,LinkCallback, handleLinkButton);

	Pedales[4]->link=(LinkElement*)calloc(1,sizeof(LinkElement));
	Pedales[4]->link=initPushLink(WHA,190,160,100,88/2,LinkCallback, handleLinkButton);

	Pedales[5]->link=(LinkElement*)calloc(1,sizeof(LinkElement));
	Pedales[5]->link=initPushLink(REVERB,343,149,67,110/2,LinkCallback, handleLinkButton);
	//

	//Inicializo los botones de las pantallas individuales de los pedales
	Pedales[0]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[0]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallback, handleDerechaButton);
	Pedales[0]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallback, handleIzquierdaButton);
	Pedales[0]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallback, handleHomeButton);

	Pedales[1]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[1]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallback, handleDerechaButton);
	Pedales[1]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallback, handleIzquierdaButton);
	Pedales[1]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallback, handleHomeButton);

	Pedales[2]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[2]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallback, handleDerechaButton);
	Pedales[2]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallback, handleIzquierdaButton);
	Pedales[2]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallback, handleHomeButton);

	Pedales[3]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[3]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallback, handleDerechaButton);
	Pedales[3]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallback, handleIzquierdaButton);
	Pedales[3]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallback, handleHomeButton);

	Pedales[4]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[4]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallback, handleDerechaButton);
	Pedales[4]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallback, handleIzquierdaButton);
	Pedales[4]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallback, handleHomeButton);

	Pedales[5]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[5]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallback, handleDerechaButton);
	Pedales[5]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallback, handleIzquierdaButton);
	Pedales[5]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallback, handleHomeButton);
	//
	
	//Inicializo los pedales con sus funciones
	Pedales[0]->efecto=delay;
	Pedales[1]->efecto=octavador;
	Pedales[2]->efecto=chorus;
	Pedales[3]->efecto=phaser;
	Pedales[4]->efecto=autowah;
	Pedales[5]->efecto=reverb;

}

//




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
#include "gui/Spritesheet_slider.h"
//#include "gui/slider5pos.h"
#include "gui/prototipos.h"
//#include "gui/tremoloondas.h"
//#include "gui/vibratoondas.h"

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

#define PERILLA 0
#define SLIDER 1
// private functions

extern int seleccion_pedal, seleccion_menu;
extern int pedal_individual;
extern PedalElement *Pedales[12];
extern LinkElementMenu * Flecha_Menu_Izquierda, *Flecha_Menu_Derecha;

static void drawElementLabel(GUIElement *e);

static DMA2D_HandleTypeDef hDma2dHandler;

// externally defined structures & functions

extern LTDC_HandleTypeDef hLtdcHandler;
static void LL_ConvertLineToARGB8888(void *src,void *dst,uint32_t xstride,uint32_t color_mode);

//Varibales Perillas Spritesheet_slider
//static SpriteSheet slider5xpos = { .pixels = slider5pos,
//		.spriteWidth = 10, .spriteHeight = 83, .numSprites = 5, .format =
//				CM_ARGB8888};//CM_RGB888
static SpriteSheet Spritesheet13_slider = { .pixels = Spritesheet_slider,
		.spriteWidth = 34, .spriteHeight = 83, .numSprites = 26, .format =
				CM_ARGB8888};//CM_RGB888
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
	int16_t delta=0;
	float newVal=0;
	if (touch->touchDetected)
	{
		// touch detected...
		uint16_t x = touch->touchX[0];
		uint16_t y = touch->touchY[0];
		DialButtonState *db = (DialButtonState *) bt->userData;
		if (bt->state == GUI_HOVER)
		{
			if(db->orientacion==PERILLA)
			{
				delta = (x - db->startX);
				newVal = db->startValue + db->sensitivity * delta;
				db->value = CLAMP(newVal, 0.0f, 1.0f);
			}
			else if(db->orientacion==SLIDER)
			{
				delta = (y - db->startY);
				newVal = db->startValue + (db->sensitivity/2) * delta;
				db->value = CLAMP(newVal, 0.0f, 1.0f);
			}
			bt->state = GUI_DIRTY | GUI_HOVER;
			if (bt->callback != NULL)
			{
				bt->callback(bt);
			}
		}
		else if (x >= bt->x && x < bt->x + bt->width && y >= bt->y && y < bt->y + bt->height)
		{
			bt->state = GUI_HOVER;
			db->startX = x;
			db->startY = y;
			db->startValue = db->value;
		}
	}
	else if (bt->state == GUI_HOVER)
	{
		bt->state = GUI_OFF | GUI_DIRTY;
	}
}

void renderDialButton(GUIElement *bt) {
	uint8_t id=0;
	if (bt->state & GUI_DIRTY)
	{
		SpriteSheet *sprite = bt->sprite;
		DialButtonState *db = (DialButtonState *) bt->userData;
		if (db->orientacion == SLIDER)
		{
			if (Pedales[10]->push->push_state == GUI_ON || Pedales[10]->push->push_state == (GUI_OFF | GUI_HOVER))
			{
				id = (uint8_t) (db->value * (float) (11));
			}
			else if (Pedales[10]->push->push_state == GUI_OFF || Pedales[10]->push->push_state == (GUI_ON | GUI_HOVER))
			{
				id = (uint8_t) 13 + (db->value * (float) (11));
			}
		}
		else
		{
			id = (uint8_t) (db->value * (float) (sprite->numSprites - 1));
		}
		drawSprite(bt->x, bt->y, id, sprite);
		drawElementLabel(bt);
		bt->state &= ~((uint16_t) GUI_DIRTY);
	}
}

static void drawElementLabel(GUIElement *e) {
	if (e->label != NULL)
	{
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

void linkRequestHandler_menu(PedalElement **bt, GUITouchState *touch) //
{
	if(seleccion_menu==MENU_1)
	{
		for (uint8_t i = 0; i < 6; i++)
		{
			bt[i]->link->handler(bt[i], touch);
		}
	}
	else
	{
		for (uint8_t i = 6; i < 12; i++)
		{
			bt[i]->link->handler(bt[i], touch);
		}
	}
}

void linkRequestHandler_Flechas_Menu(LinkElementMenu *bt, GUITouchState *touch)
{
		bt->handler(bt, touch);
}

void PushRequestHandler_menu(PedalElement **bt, GUITouchState *touch)
{
	if(seleccion_menu==MENU_1)
	{
		for (uint8_t i = 0; i < 6; i++)
		{
			bt[i]->push->push_menu->handler(bt[i], touch);
		}
	}
	else
	{
		for (uint8_t i = 6; i < 12; i++)
		{
			bt[i]->push->push_menu->handler(bt[i], touch);
		}
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

LinkElementMenu* initPushLinkMenu(uint8_t nombre,uint16_t x, uint16_t y,uint16_t width,uint16_t height, GUICallbackLinkMenu cb, LinkHandlerMenu han)
{
	LinkElementMenu* e=calloc(1,sizeof(LinkElementMenu));
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
	if(gui->push->push_state == GUI_ON || gui->push->push_state == GUI_OFF)
	{
		gui->push->push_state |= GUI_HOVER;
		gui->push->push_state ^= GUI_ONOFF_MASK;
	}

}

void MenuForceRedraw(void)
{
	if (seleccion_menu==MENU_1)
	{
		for(int aux_i=0; aux_i<6; aux_i++)
		{
			if(Pedales[aux_i]->push->push_state == GUI_ON || Pedales[aux_i]->push->push_state == GUI_OFF)
			{
				Pedales[aux_i]->push->push_state |= GUI_HOVER;
				Pedales[aux_i]->push->push_state ^= GUI_ONOFF_MASK;
			}

		}
	}
	else
	{
		for(int aux_i=6; aux_i<12; aux_i++)
		{
			if(Pedales[aux_i]->push->push_state == GUI_ON)
			{
				Pedales[aux_i]->push->push_state |= GUI_HOVER;
				Pedales[aux_i]->push->push_state ^= GUI_ONOFF_MASK;
			}

		}
	}
}


//Handles
void handlePushMenuButton(PedalElement *bt, GUITouchState *touch) {
	if (touch->touchDetected)
	{
		// touch detected...
		uint16_t x = touch->touchX[0];
		uint16_t y = touch->touchY[0];
		if (x >= bt->push->push_menu->x && x < bt->push->push_menu->x + bt->push->push_menu->width &&
			y >= bt->push->push_menu->y && y < bt->push->push_menu->y + bt->push->push_menu->height)
		{
			switch (bt->push->push_state)
			{
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
	}
	else if (bt->push->push_state & GUI_HOVER)
	{
		// clear hover flag
		bt->push->push_state &= ~((uint16_t) GUI_HOVER);
		// mark dirty (force redraw)
		bt->push->push_state |= GUI_DIRTY;
		// invert on/off bitmask
		bt->push->push_state ^= GUI_ONOFF_MASK;
		if (bt->push->push_menu->callback != NULL)
		{
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
				bt->push->push_state |= GUI_HOVER;
				break;
			case GUI_ON:
				bt->push->push_state |= GUI_HOVER;
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

void handleDerechaButtonPedal(PedalElement *bt, GUITouchState *touch)
{
	if (touch->touchDetected)
	{
		// touch detected...
			uint16_t x = touch->touchX[0];
			uint16_t y = touch->touchY[0];
			if (x >= bt->botones->flecha_derecha->x && x < bt->botones->flecha_derecha->x + bt->botones->flecha_derecha->width &&
				y >= bt->botones->flecha_derecha->y && y < bt->botones->flecha_derecha->y + bt->botones->flecha_derecha->height)
			{
				bt->botones->flecha_derecha->callback(bt);
			}
	}
}

void handleIzquierdaButtonPedal(PedalElement *bt, GUITouchState *touch)
{
	if (touch->touchDetected)
	{
		// touch detected...
			uint16_t x = touch->touchX[0];
			uint16_t y = touch->touchY[0];
			if (x >= bt->botones->flecha_izquierda->x && x < bt->botones->flecha_izquierda->x + bt->botones->flecha_izquierda->width &&
				y >= bt->botones->flecha_izquierda->y && y < bt->botones->flecha_izquierda->y + bt->botones->flecha_izquierda->height)
			{
				bt->botones->flecha_izquierda->callback(bt);
			}
	}
}

void handleHomeButtonPedal(PedalElement *bt, GUITouchState *touch)
{
	if (touch->touchDetected)
	{
		// touch detected...
			uint16_t x = touch->touchX[0];
			uint16_t y = touch->touchY[0];
			if (x >= bt->botones->home->x && x < bt->botones->home->x + bt->botones->home->width &&
				y >= bt->botones->home->y && y < bt->botones->home->y + bt->botones->home->height)
			{
				bt->botones->home->callback(bt);
			}
	}
}
//
//Menu Link Izq y Der
void handleDerechaButtonMenu(LinkElementMenu *bt, GUITouchState *touch)
{
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
}

void handleIzquierdaButtonMenu(LinkElementMenu *bt, GUITouchState *touch)
{
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
}

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
			seleccion_pedal=OCTAVADOR;
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
	    case WAH:
		    DrawScreen(WAH);
			pedal_individual=1;
			seleccion_pedal=WAH;
	    break;
	    case REVERB:
		    DrawScreen(REVERB);
			pedal_individual=1;
			seleccion_pedal=REVERB;
	    break;
	    case DISTORSION:
			DrawScreen(DISTORSION);
			pedal_individual=1;
			seleccion_pedal=DISTORSION;
		break;
		case FLANGER:
			DrawScreen(FLANGER);
			pedal_individual=1;
			seleccion_pedal=FLANGER;
		break;
		case TREMOLO:
			DrawScreen(TREMOLO);
			pedal_individual=1;
			seleccion_pedal=TREMOLO;
		break;
		case VIBRATO:
			DrawScreen(VIBRATO);
			seleccion_pedal=VIBRATO;
			pedal_individual=1;
		break;
		case EQ:
			DrawScreen(EQ);
			pedal_individual=1;
			seleccion_pedal=EQ;
		break;
		case RINGMOD:
			DrawScreen(RINGMOD);
			pedal_individual=1;
			seleccion_pedal=RINGMOD;
		break;

	}
	PedalForceRedraw(e);

}

void PushCallback(PedalElement *e)
{

	if (e->push->push_state & GUI_DIRTY)
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
				case WAH:
					drawBitmapRaw(236,209,8,7, (uint8_t*)ledprendidowahmenu, CM_ARGB8888, 1);
					break;
				case REVERB:
					drawBitmapRaw(375,211,5,5, (uint8_t*)ledprendidoreverbmenu, CM_ARGB8888, 1);
					break;
				case DISTORSION:
					drawBitmapRaw(80,95,5,5, (uint8_t*)ledprendidodistorsionmenu, CM_ARGB8888, 1);
					break;
				case FLANGER:
					drawBitmapRaw(237,65,6,6, (uint8_t*)ledprendidoflangermenu, CM_ARGB8888, 1);
					break;
				case TREMOLO:
					drawBitmapRaw(390,22,8,8, (uint8_t*)ledprendidotremolomenu, CM_ARGB8888, 1);
					break;
				case VIBRATO:
					drawBitmapRaw(80,232,5,5, (uint8_t*)ledprendidovibratomenu, CM_ARGB8888, 1);
					break;
				case RINGMOD:
					drawBitmapRaw(373,241,6,6, (uint8_t*)ledprendidoringmodmenu, CM_ARGB8888, 1);
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
				case WAH:
					drawBitmapRaw(236,209,8,7, (uint8_t*)ledapagadowahmenu, CM_ARGB8888, 1);
					break;
				case REVERB:
					drawBitmapRaw(375,211,5,5, (uint8_t*)ledapagadoreverbmenu, CM_ARGB8888, 1);
					break;
				case DISTORSION:
					drawBitmapRaw(80,95,5,5, (uint8_t*)ledapagadodistorsionmenu, CM_ARGB8888, 1);
					break;
				case FLANGER:
					drawBitmapRaw(237,65,6,6, (uint8_t*)ledapagadoflangermenu, CM_ARGB8888, 1);
					break;
				case TREMOLO:
					drawBitmapRaw(390,22,8,8, (uint8_t*)ledapagadotremolomenu, CM_ARGB8888, 1);
					break;
				case VIBRATO:
					drawBitmapRaw(80,232,5,5, (uint8_t*)ledapagadovibratomenu, CM_ARGB8888, 1);
					break;
				case RINGMOD:
					drawBitmapRaw(373,241,6,6, (uint8_t*)ledapagadoringmodmenu, CM_ARGB8888, 1);
					break;
				}
			}
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
				case WAH:
					drawBitmapRaw(230,148,19,18, (uint8_t*)ledprendidowahpedal, CM_ARGB8888, 1);
					break;
				case REVERB:
					drawBitmapRaw(236,151,13,13, (uint8_t*)ledprendidoreverbpedal, CM_ARGB8888, 1);
					break;
				case DISTORSION:
					drawBitmapRaw(198,198,9,9, (uint8_t*)ledprendidodistorsionpedal, CM_ARGB8888, 1);
					break;
				case FLANGER:
					drawBitmapRaw(234,130,13,13, (uint8_t*)ledprendidoflangerpedal, CM_ARGB8888, 1);
					break;
				case TREMOLO:
					drawBitmapRaw(228,29,23,24, (uint8_t*)ledprendidotremolopedal, CM_ARGB8888, 1);
					break;
				case VIBRATO:
					drawBitmapRaw(196,200,11,10, (uint8_t*)ledprendidovibratopedal, CM_ARGB8888, 1);
					break;
				case EQ:
					PedalForceRedraw(e);
					break;
				case RINGMOD:
					drawBitmapRaw(195,222,10,10, (uint8_t*)ledprendidoringmodpedal, CM_ARGB8888, 1);
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
				case WAH:
					drawBitmapRaw(230,148,19,18, (uint8_t*)ledapagadowahpedal, CM_ARGB8888, 1);
					break;
				case REVERB:
					drawBitmapRaw(236,151,13,13, (uint8_t*)ledapagadoreverbpedal, CM_ARGB8888, 1);
					break;
				case DISTORSION:
					drawBitmapRaw(198,198,9,9, (uint8_t*)ledapagadodistorsionpedal, CM_ARGB8888, 1);
					break;
				case FLANGER:
					drawBitmapRaw(234,130,13,13, (uint8_t*)ledapagadoflangerpedal, CM_ARGB8888, 1);
					break;
				case TREMOLO:
					drawBitmapRaw(228,29,23,24, (uint8_t*)ledapagadotremolopedal, CM_ARGB8888, 1);
					break;
				case VIBRATO:
					drawBitmapRaw(196,200,11,10, (uint8_t*)ledapagadovibratopedal, CM_ARGB8888, 1);
					break;
				case EQ:
					PedalForceRedraw(e);
					break;
				case RINGMOD:
					drawBitmapRaw(195,222,10,10, (uint8_t*)ledapagadoringmodpedal, CM_ARGB8888, 1);
					break;
				}
			}
		}
		e->push->push_state &= ~((uint16_t) GUI_DIRTY);
	}
}

void LinkDerechaCallbackPedal(PedalElement *e)
{
	seleccion_pedal++;
	if(seleccion_pedal>=0 && seleccion_pedal<6)
	{
		seleccion_menu=MENU_1;
		DrawScreen(seleccion_pedal);
		PedalForceRedraw(Pedales[seleccion_pedal]);
	}
	else if (seleccion_pedal>=6 && seleccion_pedal<12)
	{
		seleccion_menu=MENU_2;
		DrawScreen(seleccion_pedal);
		PedalForceRedraw(Pedales[seleccion_pedal]);
	}
	else if(seleccion_pedal==12)
	{
		seleccion_menu=MENU_1;
		seleccion_pedal=0;
		DrawScreen(seleccion_pedal);
		PedalForceRedraw(Pedales[seleccion_pedal]);
	}
}

void LinkIzquierdaCallbackPedal(PedalElement *e)
{
	seleccion_pedal--;
	if(seleccion_pedal>=0 && seleccion_pedal<6)
	{
		seleccion_menu=MENU_1;
		DrawScreen(seleccion_pedal);
		PedalForceRedraw(Pedales[seleccion_pedal]);
	}
	else if (seleccion_pedal>=6 && seleccion_pedal<12)
	{
		seleccion_menu=MENU_2;
		DrawScreen(seleccion_pedal);
		PedalForceRedraw(Pedales[seleccion_pedal]);
	}
	else if(seleccion_pedal==-1)
	{
		seleccion_menu=MENU_2;
		seleccion_pedal=11;
		DrawScreen(seleccion_pedal);
		PedalForceRedraw(Pedales[seleccion_pedal]);
	}
}

void LinkHomeCallbackPedal(PedalElement *e)
{
	if(seleccion_menu==MENU_1)
	{
		pedal_individual=0;
		DrawScreen(MENU_1);
		MenuForceRedraw();
		seleccion_pedal=20;
	}
	else
	{
		pedal_individual=0;
		DrawScreen(MENU_2);
		MenuForceRedraw();
		seleccion_pedal=20;
	}
}

void LinkDerechaCallbackMenu(LinkElementMenu *e)
{
	seleccion_menu=MENU_2;
	DrawScreen(MENU_2);
	MenuForceRedraw();
}

void LinkIzquierdaCallbackMenu(LinkElementMenu *e)
{
    seleccion_menu=MENU_1;
	DrawScreen(MENU_1);
	MenuForceRedraw();
}

void initPedals() {
	//Inicializo las perillas Pantalla1
	//Delay
	Pedales[0]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[0]->perilla = initPerilla(3);
	Pedales[0]->perilla->perillas[0] = guiDialButton(0, "", 174, 36, 0.0f, 0.045f, PERILLA, &perilla5252, delayFeedback);
	Pedales[0]->perilla->perillas[1] = guiDialButton(1, "", 253, 36, 0.400f, 0.045f, PERILLA, &perilla5252, delayTime);
	Pedales[0]->perilla->perillas[2] = guiDialButton(2, "", 220, 82, 0.5f, 0.045f, PERILLA, &perilla4241, delayLevel);
	//Octavador
	Pedales[1]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[1]->perilla = initPerilla(1);
	Pedales[1]->perilla->perillas[0] =guiDialButton(8, "", 0, 0, 0.0f, 0.045f, PERILLA, &perilla5252, NULL);;
	//Chorus
	Pedales[2]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[2]->perilla = initPerilla(2);
	Pedales[2]->perilla->perillas[0] = guiDialButton(0, "", 187, 22, 0.6f, 0.045f, PERILLA, &perilla4241, chorusRate);
	Pedales[2]->perilla->perillas[1] = guiDialButton(1, "", 250, 22, 0.367f, 0.045f, PERILLA, &perilla4241, chorusDepth);
	//Phaser
	Pedales[3]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[3]->perilla = initPerilla(1);
	Pedales[3]->perilla->perillas[0] = guiDialButton(0, "", 214, 27, 0.667f, 0.045f, PERILLA, &perilla5252, phaserRate);
	//Autowah
	Pedales[4]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[4]->perilla = initPerilla(4);
	Pedales[4]->perilla->perillas[0] = guiDialButton(0, "", 119, 41, 0.43f, 0.045f, PERILLA, &perilla4241, autowahDepth);
	Pedales[4]->perilla->perillas[1] = guiDialButton(1, "", 186, 41, 0.33f, 0.045f, PERILLA, &perilla4241, autowahRate);
	Pedales[4]->perilla->perillas[2] = guiDialButton(2, "", 251, 41, 0.6f, 0.045f, PERILLA, &perilla4241, autowahVolume);
	Pedales[4]->perilla->perillas[3] = guiDialButton(3, "", 319, 41, 0.9f, 0.045f, PERILLA, &whaonda, autowahMod);
	//Reverb
	Pedales[5]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[5]->perilla = initPerilla(2);
	Pedales[5]->perilla->perillas[0] = guiDialButton(0, "", 191, 47, 0.5f, 0.045f, PERILLA, &perilla4241, reverbDecay);
	Pedales[5]->perilla->perillas[1] = guiDialButton(1, "", 253, 47, 0.4f, 0.045f, PERILLA, &perilla4241, reverbMix);

	//Inicializo las perillas Pantalla2
	//Distorsion
	Pedales[6]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[6]->perilla = initPerilla(2);
	Pedales[6]->perilla->perillas[0] = guiDialButton(0, "", 179, 26, 0.61f, 0.045f, PERILLA, &perilla5252, distorsionGain);
	Pedales[6]->perilla->perillas[1] = guiDialButton(1, "", 249, 26, 0.5f, 0.045f, PERILLA, &perilla5252, distorsionBlend);
	//Flanger
	Pedales[7]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[7]->perilla = initPerilla(4);
	Pedales[7]->perilla->perillas[0] =guiDialButton(0, "", 116, 55, 0.0f, 0.045f, PERILLA, &perilla5252, flangerManual);
	Pedales[7]->perilla->perillas[1] =guiDialButton(1, "", 182, 55, 0.57f, 0.045f, PERILLA, &perilla5252, flangerDepth);
	Pedales[7]->perilla->perillas[2] =guiDialButton(2, "", 246, 55, 0.2f, 0.045f, PERILLA, &perilla5252, flangerRate);
	Pedales[7]->perilla->perillas[3] =guiDialButton(3, "", 311, 55, 0.81f, 0.045f, PERILLA, &perilla5252, flangerFeedback);
	//Tremolo
	Pedales[8]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[8]->perilla = initPerilla(3);
	Pedales[8]->perilla->perillas[0] = guiDialButton(0, "", 167, 40, 0.67f, 0.045f, PERILLA, &perilla4241, tremoloDepth);
	Pedales[8]->perilla->perillas[1] = guiDialButton(1, "", 220, 66, 0.2f, 0.045f, PERILLA, &perilla4241, tremoloMod);
	Pedales[8]->perilla->perillas[2] = guiDialButton(2, "", 269, 40, 0.5f, 0.045f, PERILLA, &perilla4241, tremoloRate);
	//Vibrato
	Pedales[9]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[9]->perilla = initPerilla(3);
	Pedales[9]->perilla->perillas[0] = guiDialButton(0, "", 180, 43, 0.6f, 0.045f, PERILLA, &perilla4241, vibratoRate);
	Pedales[9]->perilla->perillas[1] = guiDialButton(1, "", 257, 43, 0.41f, 0.045f, PERILLA, &perilla4241, vibratoDepth);
	Pedales[9]->perilla->perillas[2] = guiDialButton(2, "", 223, 86, 1.0f, 0.045f, PERILLA, &perilla3535, vibratoMod);
	//EQ
	Pedales[10]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[10]->perilla = initPerilla(6);
	Pedales[10]->perilla->perillas[0] = guiDialButton(0, "", 111, 97, 0.5f, 0.045f, SLIDER, &Spritesheet13_slider, eqGain0);
	Pedales[10]->perilla->perillas[1] = guiDialButton(1, "", 156, 97, 0.5f, 0.045f, SLIDER, &Spritesheet13_slider, eqGain1);
	Pedales[10]->perilla->perillas[2] = guiDialButton(2, "", 201, 97, 0.5f, 0.045f, SLIDER, &Spritesheet13_slider, eqGain2);
	Pedales[10]->perilla->perillas[3] = guiDialButton(3, "", 246, 97, 0.5f, 0.045f, SLIDER, &Spritesheet13_slider, eqGain3);
	Pedales[10]->perilla->perillas[4] = guiDialButton(4, "", 291, 97, 0.5f, 0.045f, SLIDER, &Spritesheet13_slider, eqGain4);
	Pedales[10]->perilla->perillas[5] = guiDialButton(5, "", 336, 97, 0.5f, 0.045f, SLIDER, &Spritesheet13_slider, eqGain5);
	//Ringmod
	Pedales[11]=(PedalElement*)calloc(1, sizeof(PedalElement));
	Pedales[11]->perilla = initPerilla(1);
	Pedales[11]->perilla->perillas[0] = guiDialButton(0, "", 223, 73, 0.179f, 0.045f, PERILLA, &perilla3535, ringmodRate);

	//Inicializo los Push (3pdt) Pantalla 1
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
	Pedales[4]->push->push_menu=initPushLink(WAH,190,160+88/2,100,88/2,PushCallback, handlePushMenuButton);

	Pedales[5]->push=(PushElement*)calloc(1, sizeof(PushElement));
	Pedales[5]->push->push_menu=initPushLink(REVERB,343,149+110/2,67,110/2,PushCallback, handlePushMenuButton);

	//Inicializo los Push (3pdt) Pantalla2
	//PEDALES Menu
	Pedales[6]->push=(PushElement*)calloc(1, sizeof(PushElement));
	Pedales[6]->push->push_menu=initPushLink(DISTORSION,70,13+110/2,59,110/2,PushCallback, handlePushMenuButton);

	Pedales[7]->push=(PushElement*)calloc(1, sizeof(PushElement));
	Pedales[7]->push->push_menu=initPushLink(FLANGER,179,23+90/2,121,90/2,PushCallback, handlePushMenuButton);

	Pedales[8]->push=(PushElement*)calloc(1, sizeof(PushElement));
	Pedales[8]->push->push_menu=initPushLink(TREMOLO,354,13+110/2,80,110/2,PushCallback, handlePushMenuButton);

	Pedales[9]->push=(PushElement*)calloc(1, sizeof(PushElement));
	Pedales[9]->push->push_menu=initPushLink(VIBRATO,71,149+110/2,57,110/2,PushCallback, handlePushMenuButton);

	Pedales[10]->push=(PushElement*)calloc(1, sizeof(PushElement));
	Pedales[10]->push->push_menu=initPushLink(EQ,181,159+90/2,118,90/2,PushCallback, handlePushMenuButton);

	Pedales[11]->push=(PushElement*)calloc(1, sizeof(PushElement));
	Pedales[11]->push->push_menu=initPushLink(RINGMOD,360,149+110/2,68,110/2,PushCallback, handlePushMenuButton);

	//PEDALES Individuales Pantalla1
	Pedales[0]->push->push_indiv=initPushLink(DELAY,225,189,31,33,PushCallback, handlePushIndividualButton);
	Pedales[1]->push->push_indiv=initPushLink(OCTAVADOR,219,180,42,42,PushCallback, handlePushIndividualButton);
	Pedales[2]->push->push_indiv=initPushLink(CHORUS,180,163,117,77,PushCallback, handlePushIndividualButton);
	Pedales[3]->push->push_indiv=initPushLink(PHASER,222,171,37,37,PushCallback, handlePushIndividualButton);
	Pedales[4]->push->push_indiv=initPushLink(WAH,218,213,41,41,PushCallback, handlePushIndividualButton);
	Pedales[5]->push->push_indiv=initPushLink(REVERB,180,163,117,77,PushCallback, handlePushIndividualButton);

	//PEDALES Individuales Pantalla2
	Pedales[6]->push->push_indiv=initPushLink(DISTORSION,223,184,35,35,PushCallback, handlePushIndividualButton);
	Pedales[7]->push->push_indiv=initPushLink(FLANGER,223,168,37,37,PushCallback, handlePushIndividualButton);
	Pedales[8]->push->push_indiv=initPushLink(TREMOLO,222,203,36,36,PushCallback, handlePushIndividualButton);
	Pedales[9]->push->push_indiv=initPushLink(VIBRATO,221,188,37,37,PushCallback, handlePushIndividualButton);
	Pedales[10]->push->push_indiv=initPushLink(EQ,218,186,37,41,PushCallback, handlePushIndividualButton);
	Pedales[11]->push->push_indiv=initPushLink(RINGMOD,223,210,35,34,PushCallback, handlePushIndividualButton);
	//PEDALES estados
	Pedales[0]->push->push_state=GUI_OFF;
	Pedales[1]->push->push_state=GUI_OFF;
	Pedales[2]->push->push_state=GUI_OFF;
	Pedales[3]->push->push_state=GUI_OFF;
	Pedales[4]->push->push_state=GUI_OFF;
	Pedales[5]->push->push_state=GUI_OFF;
	Pedales[6]->push->push_state=GUI_OFF;
	Pedales[7]->push->push_state=GUI_OFF;
	Pedales[8]->push->push_state=GUI_OFF;
	Pedales[9]->push->push_state=GUI_OFF;
	Pedales[10]->push->push_state=GUI_OFF;
	Pedales[11]->push->push_state=GUI_OFF;

	//Inicializo los links de los pedales Pantalla1
	Pedales[0]->link=(LinkElement*)calloc(1,sizeof(LinkElement));
	Pedales[0]->link=initPushLink(DELAY,76,13,58,110/2,LinkCallback, handleLinkButton);

	Pedales[1]->link=(LinkElement*)calloc(1,sizeof(LinkElement));
	Pedales[1]->link=initPushLink(OCTAVADOR,198,13,84,110/2,LinkCallback, handleLinkButton);

	Pedales[2]->link=(LinkElement*)calloc(1,sizeof(LinkElement));
	Pedales[2]->link=initPushLink(CHORUS,346,13,61,110/2,LinkCallback, handleLinkButton);

	Pedales[3]->link=(LinkElement*)calloc(1,sizeof(LinkElement));
	Pedales[3]->link=initPushLink(PHASER,73,149,64,110/2,LinkCallback, handleLinkButton);

	Pedales[4]->link=(LinkElement*)calloc(1,sizeof(LinkElement));
	Pedales[4]->link=initPushLink(WAH,190,160,100,88/2,LinkCallback, handleLinkButton);

	Pedales[5]->link=(LinkElement*)calloc(1,sizeof(LinkElement));
	Pedales[5]->link=initPushLink(REVERB,343,149,67,110/2,LinkCallback, handleLinkButton);

	//Inicializo los links de los pedales Pantalla2
	Pedales[6]->link=(LinkElement*)calloc(1, sizeof(LinkElement));
	Pedales[6]->link=initPushLink(DISTORSION,70,13,59,110/2,LinkCallback, handleLinkButton);

	Pedales[7]->link=(LinkElement*)calloc(1, sizeof(LinkElement));
	Pedales[7]->link=initPushLink(FLANGER,179,23,121,90/2,LinkCallback, handleLinkButton);

	Pedales[8]->link=(LinkElement*)calloc(1, sizeof(LinkElement));
	Pedales[8]->link=initPushLink(TREMOLO,354,13,80,110/2,LinkCallback, handleLinkButton);

	Pedales[9]->link=(LinkElement*)calloc(1, sizeof(LinkElement));
	Pedales[9]->link=initPushLink(VIBRATO,71,149,57,110/2,LinkCallback, handleLinkButton);

	Pedales[10]->link=(LinkElement*)calloc(1, sizeof(LinkElement));
	Pedales[10]->link=initPushLink(EQ,181,159,118,90/2,LinkCallback, handleLinkButton);

	Pedales[11]->link=(LinkElement*)calloc(1, sizeof(LinkElement));
	Pedales[11]->link=initPushLink(RINGMOD,360,149,68,110/2,LinkCallback, handleLinkButton);

	//

	//Inicializo los botones de las pantallas individuales de los pedales PANTALLA1
	Pedales[0]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[0]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallbackPedal, handleDerechaButtonPedal);
	Pedales[0]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallbackPedal, handleIzquierdaButtonPedal);
	Pedales[0]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallbackPedal, handleHomeButtonPedal);

	Pedales[1]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[1]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallbackPedal, handleDerechaButtonPedal);
	Pedales[1]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallbackPedal, handleIzquierdaButtonPedal);
	Pedales[1]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallbackPedal, handleHomeButtonPedal);

	Pedales[2]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[2]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallbackPedal, handleDerechaButtonPedal);
	Pedales[2]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallbackPedal, handleIzquierdaButtonPedal);
	Pedales[2]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallbackPedal, handleHomeButtonPedal);

	Pedales[3]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[3]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallbackPedal, handleDerechaButtonPedal);
	Pedales[3]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallbackPedal, handleIzquierdaButtonPedal);
	Pedales[3]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallbackPedal, handleHomeButtonPedal);

	Pedales[4]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[4]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallbackPedal, handleDerechaButtonPedal);
	Pedales[4]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallbackPedal, handleIzquierdaButtonPedal);
	Pedales[4]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallbackPedal, handleHomeButtonPedal);

	Pedales[5]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[5]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallbackPedal, handleDerechaButtonPedal);
	Pedales[5]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallbackPedal, handleIzquierdaButtonPedal);
	Pedales[5]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallbackPedal, handleHomeButtonPedal);
	
	//Inicializo los botones de las pantallas individuales de los pedales PANTALLA2
	Pedales[6]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[6]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallbackPedal, handleDerechaButtonPedal);
	Pedales[6]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallbackPedal, handleIzquierdaButtonPedal);
	Pedales[6]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallbackPedal, handleHomeButtonPedal);

	Pedales[7]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[7]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallbackPedal, handleDerechaButtonPedal);
	Pedales[7]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallbackPedal, handleIzquierdaButtonPedal);
	Pedales[7]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallbackPedal, handleHomeButtonPedal);

	Pedales[8]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[8]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallbackPedal, handleDerechaButtonPedal);
	Pedales[8]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallbackPedal, handleIzquierdaButtonPedal);
	Pedales[8]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallbackPedal, handleHomeButtonPedal);

	Pedales[9]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[9]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallbackPedal, handleDerechaButtonPedal);
	Pedales[9]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallbackPedal, handleIzquierdaButtonPedal);
	Pedales[9]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallbackPedal, handleHomeButtonPedal);

	Pedales[10]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[10]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallbackPedal, handleDerechaButtonPedal);
	Pedales[10]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallbackPedal, handleIzquierdaButtonPedal);
	Pedales[10]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallbackPedal, handleHomeButtonPedal);

	Pedales[11]->botones=(MenuElement*)calloc(1, sizeof(MenuElement));
	Pedales[11]->botones->flecha_derecha=initPushLink(DERECHA,417,84,61,107,LinkDerechaCallbackPedal, handleDerechaButtonPedal);
	Pedales[11]->botones->flecha_izquierda=initPushLink(IZQUIERDA,0,84,61,107,LinkIzquierdaCallbackPedal, handleIzquierdaButtonPedal);
	Pedales[11]->botones->home=initPushLink(HOME,0,0,70,70,LinkHomeCallbackPedal, handleHomeButtonPedal);
//

	//Inicializo los pedales con sus funciones PANTALLA1
	Pedales[0]->efecto=delayEfecto;
	Pedales[1]->efecto=octavadorEfecto;
	Pedales[2]->efecto=chorusEfecto;
	Pedales[3]->efecto=phaserEfecto;
	Pedales[4]->efecto=autowahEfecto;
	Pedales[5]->efecto=reverbEfecto;

	//Inicializo los pedales con sus funciones PANTALLA2
	Pedales[6]->efecto=distorsionEfecto;
	Pedales[7]->efecto=flangerEfecto;
	Pedales[8]->efecto=tremoloEfecto;
	Pedales[9]->efecto=vibratoEfecto;
	Pedales[10]->efecto=eqEfecto;
	Pedales[11]->efecto=ringmodEfecto;


	//Link Menu Izq y Der

	Flecha_Menu_Izquierda=(LinkElementMenu*)calloc(1,sizeof(LinkElementMenu));
	Flecha_Menu_Izquierda=initPushLinkMenu(IZQUIERDA_MENU,5,101,39,70,LinkIzquierdaCallbackMenu, handleIzquierdaButtonMenu);
	Flecha_Menu_Derecha=(LinkElementMenu*)calloc(1,sizeof(LinkElementMenu));
	Flecha_Menu_Derecha=initPushLinkMenu(DERECHA_MENU,435,101,39,70,LinkDerechaCallbackMenu, handleDerechaButtonMenu);

}

//




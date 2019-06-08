#include <stdlib.h>
#include <stdio.h>
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
#include "gui/gui.h"
#include "gui/link.h"
#include "ex03/macros.h"
#include "main.h"
#include "string.h"
#include "gui/leds.h"
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

// private functions

static void drawElementLabel(GUIElement *e);

static DMA2D_HandleTypeDef hDma2dHandler;

// externally defined structures & functions

extern LTDC_HandleTypeDef hLtdcHandler;
static void LL_ConvertLineToARGB8888(void *src,void *dst,uint32_t xstride,uint32_t color_mode);

//Varibales Perillas
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
//

// push button functions

void handlePushButton(GUIElement *bt, GUITouchState *touch) {
	//BSP_AUDIO_OUT_Pause();
	if (touch->touchDetected) {
		// touch detected...
		uint16_t x = touch->touchX[0];
		uint16_t y = touch->touchY[0];
		if (x >= bt->x && x < bt->x + bt->width && y >= bt->y
				&& y < bt->y + bt->height) {
			switch (bt->state) {
			case GUI_OFF:
				bt->state |= GUI_HOVER | GUI_DIRTY;
				break;
			case GUI_ON:
				bt->state |= GUI_HOVER | GUI_DIRTY;
				break;
			default:
				break;
			}
		}
	} else if (bt->state & GUI_HOVER) {
		// clear hover flag
		bt->state &= ~((uint16_t) GUI_HOVER);
		// mark dirty (force redraw)
		bt->state |= GUI_DIRTY;
		// invert on/off bitmask
		bt->state ^= GUI_ONOFF_MASK;
		if (bt->callback != NULL) {
			bt->callback(bt);
		}
	}
	//BSP_AUDIO_OUT_Resume();
}

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
	if (touch->touchDetected) {
		// touch detected...
		uint16_t x = touch->touchX[0];
		uint16_t y = touch->touchY[0];
		DialButtonState *db = (DialButtonState *) bt->userData;
		if (bt->state == GUI_HOVER) {
			//int16_t dx = (x - db->startX);
			//int16_t dy = (y - db->startY);
			//int16_t delta = abs(dx) > abs(dy) ? dx : dy;
			int16_t delta = (x - db->startX);
			float newVal = db->startValue + db->sensitivity * delta;
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
		float val, float sens, SpriteSheet *sprite, GUICallback cb) {
	GUIElement *e = guiElement(id, label, x, y, sprite, cb);
	DialButtonState *db = (DialButtonState *) calloc(1,
			sizeof(DialButtonState));
	e->handler = handleDialButton;
	e->render = renderDialButton;
	e->userData = db;
	db->value = val;
	db->sensitivity = sens;
	return e;
}

GUIElement *guiPushButton(uint8_t id, char *label, uint16_t x, uint16_t y,
		float val, SpriteSheet *sprite, GUICallback cb) {
	GUIElement *e = guiElement(id, label, x, y, sprite, cb);
	PushButtonState *pb = (PushButtonState *) calloc(1,
			sizeof(PushButtonState));
	e->handler = handlePushButton;
	e->render = renderPushButton;
	e->userData = pb;
	pb->value = val;
	return e;
}

PerillaElement* initPerilla (uint8_t num)
{
	PerillaElement *gui = (PerillaElement *) calloc(1, sizeof(PerillaElement));
	gui->perillas = (GUIElement **) calloc(num, sizeof(GUIElement *));
	gui->numItems = num;
	return gui;
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

void PedalForceRedraw(PedalElement *gui) {
	for (uint8_t i = 0; i < gui->perilla->numItems; i++) {
		gui->perilla->perillas[i]->state |= GUI_DIRTY;
	}
}

void guiForceRedraw(GUI *gui) {
	for (uint8_t i = 0; i < gui->numItems; i++) {
		gui->items[i]->state |= GUI_DIRTY;
	}
}

void guiUpdate(GUI *gui, GUITouchState *touch) {
	BSP_LCD_SetFont(gui->font);
	BSP_LCD_SetBackColor(gui->bgColor);
	BSP_LCD_SetTextColor(gui->textColor);
	for (uint8_t i = 0; i < gui->numItems; i++) {
		GUIElement *e = gui->items[i];
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

void initAppGUI() {
	Delay = initPerilla(3);
	Delay->perilla->perillas[0] = guiDialButton(0, "", 174, 36, 0.0f, 0.045f, &perilla5252, Delay_Feedback);
	Delay->perilla->perillas[1] = guiDialButton(1, "", 253, 36, 0.0f, 0.045f, &perilla5252,Delay_Time);
	Delay->perilla->perillas[2] = guiDialButton(2, "", 220, 82, 0.0f, 0.045f, &perilla4241,Delay_Level);

	Tremolo = initGUI(3);
	Tremolo->perilla->perillas[0] = guiDialButton(0, "", 167, 40, 0.0f, 0.045f, &perilla4241, Tremolo_Depth);
	Tremolo->perilla->perillas[1] = guiDialButton(1, "", 269, 39, 0.0f, 0.045f, &perilla4241,Tremolo_Rate);
	Tremolo->perilla->perillas[2] = guiDialButton(2, "", 220, 66, 0.0f, 0.045f, &tremoloonda,Tremolo_Mod);

	Vibrato = initGUI(3);
	Vibrato->perilla->perillas[0] = guiDialButton(0, "", 180, 43, 0.0f, 0.045f, &perilla4241, Vibrato_Rate);
	Vibrato->perilla->perillas[1] = guiDialButton(1, "", 257, 43, 0.0f, 0.045f, &perilla4241,Vibrato_Depth);
	Vibrato->perilla->perillas[2] = guiDialButton(2, "", 223, 86, 0.0f, 0.045f, &vibratoonda,Vibrato_Mod);

	Distorsion = initGUI(2);
	Distorsion->perilla->perillas[0] = guiDialButton(0, "", 179, 26, 0.0f, 0.045f, &perilla5252, NULL);
	Distorsion->perilla->perillas[1] = guiDialButton(1, "", 249, 26, 0.0f, 0.045f, &perilla5252,NULL);

	Autowah = initGUI(4);
	Autowah->perilla->perillas[0] = guiDialButton(0, "", 119, 41, 0.0f, 0.045f, &perilla4241, Autowah_Depth);
	Autowah->perilla->perillas[1] = guiDialButton(1, "", 186, 41, 0.0f, 0.045f, &perilla4241,Autowah_Rate);
	Autowah->perilla->perillas[2] = guiDialButton(2, "", 251, 41, 0.0f, 0.045f, &perilla4241,Autowah_Volume);
	Autowah->perilla->perillas[3] = guiDialButton(2, "", 319, 41, 0.0f, 0.045f, &whaonda,Autowah_Mod);

	Ringmode = initGUI(1);
	Ringmode->perilla->perillas[0] = guiDialButton(0, "", 223, 73, 0.0f, 0.045f, &perilla3535, Ringmod_Rate);
}

LinkElement* guiPush(uint8_t nombre,uint16_t x, uint16_t y,uint16_t width,uint16_t height, GUICallbackLink cb, LinkHandler han)
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

void init_push()
{
	//PEDALES Menu
	Delay->push->push_menu=guiPush(DELAY,80,16+110/2,58,110/2,PushCallback, handlePushMenuButton);
	Tremolo->push->push_menu=guiPush(TREMOLO,195,16+110/2,80,110/2,PushCallback, handlePushMenuButton);
	Vibrato->push->push_menu=guiPush(VIBRATO,332,16+110/2,57,110/2,PushCallback, handlePushMenuButton);
	Distorsion->push->push_menu=guiPush(DISTORSION,80,147+110/2,58,110/2,PushCallback, handlePushMenuButton);
	Autowah->push->push_menu=guiPush(WHA,185,158+88/2,100,88/2,PushCallback, handlePushMenuButton);
	Ringmode->push->push_menu=guiPush(RINGMOD,326,147+110/2,68,110/2,PushCallback, handlePushMenuButton);

	//PEDALES Individuales
	Delay->push->push_indiv=guiPush(DELAY,225,189,31,33,PushCallback, handlePushMenuButton);
	Tremolo->push->push_indiv=guiPush(TREMOLO,222,203,36,36,PushCallback, handlePushMenuButton);
	Vibrato->push->push_indiv=guiPush(VIBRATO,221,188,37,37,PushCallback, handlePushMenuButton);
	Distorsion->push->push_indiv=guiPush(DISTORSION,223,184,35,35,PushCallback, handlePushMenuButton);
	Autowah->push->push_indiv=guiPush(WHA,218,213,41,41,PushCallback, handlePushMenuButton);
	Ringmode->push->push_indiv=guiPush(RINGMOD,223,210,35,34,PushCallback, handlePushMenuButton);

	//PEDALES estados
	Delay->push->push_state=GUI_OFF;
	Tremolo->push->push_state=GUI_OFF;
	Vibrato->push->push_state=GUI_OFF;
	Distorsion->push->push_state=GUI_OFF;
	Autowah->push->push_state=GUI_OFF;
	Ringmode->push->push_state=GUI_OFF;
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

void initPedal (void)
{
	initAppGUI();
	init_pedales_link();
	init_pantalla_link();
	init_push();
}


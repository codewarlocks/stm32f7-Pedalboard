#pragma once

#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_ts.h"
//#include "gui/link.h"

#define UI_BG_COLOR   ((uint32_t)0xff59626c)
#define UI_TEXT_COLOR ((uint32_t)0xffcccccc)
#define UI_FONT Font12
#define UI_SENSITIVITY 0.02f

typedef struct {
	uint16_t touchX[1];
	uint16_t touchY[1];
	uint32_t lastTouch;
	uint8_t touchDetected;
	uint8_t touchUpdate;
} GUITouchState;

// forward declaration for function pointers below
typedef struct GUIElement GUIElement;

typedef void (*GUIElementHandler)(GUIElement *button,
		GUITouchState *touchState);
typedef void (*GUIElementRenderer)(GUIElement *button);
typedef void (*GUICallback)(GUIElement *button);


// element state flags
enum {
	GUI_OFF = 1,
	GUI_ON = 2,
	GUI_HOVER = 4,
	GUI_DIRTY = 8,
	GUI_ONOFF_MASK = GUI_ON | GUI_OFF
};

typedef struct {
	const uint8_t *pixels;
	uint16_t spriteWidth;
	uint16_t spriteHeight;
	uint8_t numSprites;
	uint32_t format;
} SpriteSheet;

struct GUIElement {
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	uint32_t state;
	char* label;
	void* userData;
	SpriteSheet *sprite;
	GUIElementHandler handler;
	GUIElementRenderer render;
	GUICallback callback;
	uint8_t id;
};

typedef struct {
	float value;
	float startValue;
	float sensitivity;
	uint16_t startX;
	uint16_t startY;
	uint8_t orientacion;
} DialButtonState;

typedef struct {
	float value;
} PushButtonState;

typedef struct {
	GUIElement **items;
	uint32_t bgColor;
	uint32_t textColor;
	sFONT *font;
	uint8_t numItems;
	int8_t selected;
} GUI;

void handlePushButton(GUIElement *bt, GUITouchState *touch);
void renderPushButton(GUIElement *bt);

void handleDialButton(GUIElement *bt, GUITouchState *touch);
void renderDialButton(GUIElement *bt);

void init_LL_ConvertLine_DMA2D (uint32_t color_mode);
void LL_ConvertLineToARGB8888(void *src,void *dst,uint32_t xstride, uint32_t color_mode);
void drawSprite(uint16_t x, uint16_t y, uint8_t id, SpriteSheet *sprite);
void drawBitmapRaw(uint16_t x, uint16_t y, uint16_t width, uint16_t height,
		uint8_t *pixels, uint32_t colorMode, uint8_t active_label);

GUI *initGUI(uint8_t num, sFONT *font, uint32_t bgCol, uint32_t textCol);
void guiForceRedraw(GUI *gui);

void guiUpdateTouch(TS_StateTypeDef *raw, GUITouchState *touch);

GUIElement *guiElementCommon(uint8_t id, char *label, uint16_t x, uint16_t y,
		SpriteSheet *sprite, GUICallback cb);
GUIElement *guiPushButton(uint8_t id, char *label, uint16_t x, uint16_t y,
		float val, SpriteSheet *sprite, GUICallback cb);
GUIElement *guiDialButton(uint8_t id, char *label, uint16_t x, uint16_t y,
		float val, float sens, uint8_t orientacion, SpriteSheet *sprite, GUICallback cb);

//LINK
typedef struct LinkElement LinkElement;
typedef struct LinkElementMenu LinkElementMenu;
typedef struct PedalElement PedalElement;
typedef void (*LinkHandler)(PedalElement *button,GUITouchState *touchState);
typedef void (*LinkHandlerMenu)(LinkElementMenu *button,GUITouchState *touchState);
typedef void (*GUICallbackLink)(PedalElement *button);
typedef void (*GUICallbackLinkMenu)(LinkElementMenu *button);
typedef int (*Funcion_Efecto)(int entrada);

struct LinkElement {
	uint8_t nombre;
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	GUICallbackLink callback;
	LinkHandler handler;
};

struct LinkElementMenu {
	uint8_t nombre;
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	GUICallbackLinkMenu callback;
	LinkHandlerMenu handler;
};

typedef struct {
	GUIElement **perillas;
	uint8_t numItems;
}PerillaElement;

typedef struct {
	LinkElement *push_menu;
	LinkElement *push_indiv;
	int push_state;
}PushElement;

typedef struct {
	LinkElement *flecha_izquierda;
	LinkElement *flecha_derecha;
	LinkElement *home;
}MenuElement;

struct PedalElement{
	PerillaElement *perilla;
	PushElement *push;
	LinkElement *link;
	MenuElement *botones;
	Funcion_Efecto efecto;
};
/*Escrito por nosotros*/
void gui_init(void);
void Demo_fondito(void);
void DrawScreen(int num);
void linkRequestHandler_menu(PedalElement **bt, GUITouchState *touch);
void linkRequestHandler_Flechas_Menu(LinkElementMenu *bt, GUITouchState *touch);
void linkRequestHandlers_pedal_individual(PedalElement *bt, GUITouchState *touch);
LinkElement* initPushLink(uint8_t nombre,uint16_t x, uint16_t y,uint16_t width,uint16_t height, GUICallbackLink cb, LinkHandler han);
LinkElementMenu* initPushLinkMenu(uint8_t nombre,uint16_t x, uint16_t y,uint16_t width,uint16_t height, GUICallbackLinkMenu cb, LinkHandlerMenu han);
PerillaElement* initPerilla (uint8_t num);
void initPedals (void);
//Handles
void handlePushMenuButton(PedalElement *bt, GUITouchState *touch);
void handlePushIndividualButton(PedalElement *bt, GUITouchState *touch);
void handleLinkButton(PedalElement *bt, GUITouchState *touch);
void handleDerechaButtonPedal(PedalElement *bt, GUITouchState *touch);
void handleIzquierdaButtonPedal(PedalElement *bt, GUITouchState *touch);
void handleHomeButtonPedal(PedalElement *bt, GUITouchState *touch);
void handleDerechaButtonMenu (LinkElementMenu *bt, GUITouchState *touch);
void handleIzquierdaButtonMenu(LinkElementMenu *bt, GUITouchState *touch);
//Callbacks
void LinkCallback(PedalElement *e);
void PushCallback(PedalElement *e);
void LinkDerechaCallbackPedal(PedalElement *e);
void LinkIzquierdaCallbackPedal(PedalElement *e);
void LinkHomeCallbackPedal(PedalElement *e);
void LinkDerechaCallbackMenu(LinkElementMenu *e);
void LinkIzquierdaCallbackMenu(LinkElementMenu *e);
void PushRequestHandler_menu(PedalElement **bt, GUITouchState *touch);
void MenuForceRedraw(void);
void PedalForceRedraw(PedalElement *gui);
/*Escrito por nosotros*/

void guiUpdate(PerillaElement *gui, GUITouchState *touch);

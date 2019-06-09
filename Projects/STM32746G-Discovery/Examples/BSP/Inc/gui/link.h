#include "gui/gui.h"

//LINK
typedef struct LinkElement LinkElement;

typedef void (*LinkHandler)(LinkElement *button,GUITouchState *touchState);
typedef void (*GUICallbackLink)(LinkElement *button);

struct LinkElement {
	uint8_t nombre;
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	GUICallbackLink callback;
	LinkHandler handler;
};

typedef struct {
	GUIElement **perillas;
	uint8_t numItems;
}PerillaElement;

typedef struct {
	LinkElement push_menu;
	LinkElement push_indiv;
	int push_state;
}PushElement;

typedef struct {
	LinkElement flecha_izquierda;
	LinkElement flecha_derecha;
	LinkElement home;
}MenuElement;

typedef struct {
	PerillaElement perilla;
	PushElement push;
	LinkElement link;
	MenuElement botones;
}PedalElement;

void Demo_fondito(void);
void DrawScreen(int num);
void linkUpdate(LinkElement **link, GUITouchState *touch);
void linkUpdatePedales(LinkElement **link, GUITouchState *touch);
LinkElement* initPushLink(uint8_t nombre,uint16_t x, uint16_t y,uint16_t width,uint16_t height, GUICallbackLink cb, LinkHandler han)
PerillaElement* initPerilla (uint8_t num);
void initPedals (void);
void PedalForceRedraw(PedalElement *gui)<
//Handles
void handlePushMenuButton(PedalElement *bt, GUITouchState *touch);
void handlePushIndividualButton(PedalElement *bt, GUITouchState *touch);
void handleLinkButton(PedalElement *bt, GUITouchState *touch);
void handleDerechaButton(PedalElement *bt, GUITouchState *touch);
void handleIzquierdaButton(PedalElement *bt, GUITouchState *touch);
void handleHomeButton(PedalElement *bt, GUITouchState *touch);
//Callbacks
void LinkCallback(PedalElement *e);
void PushCallback(PedalElement *e);
void LinkDerechaCallback(PedalElement *e);
void LinkIzquierdaCallback(PedalElement *e);
void LinkHomeCallback(PedalElement *e);
//

#include "gui/gui.h"

//LINK
typedef struct LinkElement LinkElement;

typedef void (*LinkHandler)(LinkElement *button,GUITouchState *touchState);
typedef void (*GUICallbackLink)(LinkElement *button);

struct LinkElement {
	uint8_t nombre;
	uint8_t cant_per;
	uint16_t x;
	uint16_t y;
	uint16_t width;
	uint16_t height;
	GUICallbackLink callback;
	uint32_t states;
	LinkHandler handler;
};

void PushUpdateState (LinkElement *e1, LinkElement *e2);
void Demo_fondito(void);
void DrawScreen(int num);
void LinkCallback(LinkElement *e);
void linkUpdate(LinkElement **link, GUITouchState *touch);
void linkUpdatePedales(LinkElement **link, GUITouchState *touch);
void handleLinkButton(LinkElement *bt, GUITouchState *touch);
LinkElement* guiLink(uint8_t nombre,uint8_t perillas, uint16_t x, uint16_t y,uint16_t width,uint16_t height, GUICallbackLink cb);
void init_pedales_link(void);
void init_pantalla_link(void);
void init_push(void);
void handlePushMenuButton(LinkElement *bt, GUITouchState *touch);
LinkElement* guiPush(uint8_t nombre, uint16_t x, uint16_t y,uint16_t width,uint16_t height, GUICallbackLink cb);
void PushCallback(LinkElement *e);
void LinkPedalCallback(LinkElement *e);

//
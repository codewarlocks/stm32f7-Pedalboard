#include "gui/gui.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"

#define CUADRADA 0
#define TRIANGULAR 1
#define RAMPA_ASC 2
#define RAMPA_DESC 3
#define SINUSOIDAL 4
#define ENVOLVENTE 5

#define SR 48000 // TODO: cambiar por el define de audio que correponde

//Pedales Individuales
void initAppGUI(void);
// Delay.c
int delay (int);
void Delay_Time (GUIElement*);
void Delay_Feedback (GUIElement*);
void Delay_Level (GUIElement*);
void Push_State_Delay (GUIElement*);
int Get_State_Delay (void);

// Autowah.c
void init_autowah();
void autowah_parametros();
int autowah(int);
float32_t Autowah_LFO(int);
int get_envelope();
void Autowah_Rate (GUIElement*);
void Autowah_Depth (GUIElement*);
void Autowah_Volume (GUIElement*);
void Autowah_Mod (GUIElement*);
void Push_State_Autowah (GUIElement*);
int Get_State_Autowah (void);

// Distorsion.c
int distorsion(int);
void Distorsion_Gain (GUIElement*);
void Push_State_Distorsion (GUIElement*);
int Get_State_Distorsion (void);

// Vibrato.c
void vibrato_parametros();
int vibrato (int);
float32_t Vibrato_LFO(int);
void Vibrato_Rate (GUIElement*);
void Vibrato_Depth (GUIElement*);
void Vibrato_Mod (GUIElement*);
void Push_State_Vibrato (GUIElement*);
int Get_State_Vibrato (void);

// Tremolo.c
void tremolo_parametros();
int tremolo(int);
float32_t Tremolo_LFO();
void Tremolo_Rate (GUIElement*);
void Tremolo_Depth (GUIElement*);
void Tremolo_Mod (GUIElement*);
void Push_State_Tremolo (GUIElement*);
int Get_State_Tremolo (void);

// Ringmod.c
int ringmod(int);
void Ringmod_Rate (GUIElement*);
void Push_State_Ringmod (GUIElement*);
int Get_State_Ringmod (void);

//Chorus.c
void chorus_parametros();
int chorus (int);
float32_t Chorus_LFO(int);

//Flanger.c
void flanger_parametros();
float32_t flanger (float32_t);
float32_t Flanger_LFO(int);

//Octavador.c
int octavador(int);

//Reverb
int AP_filter(int in, int* line, int delay, int cont);
int FBCF_filter(int in, int line[], int delay, int cont, int inst);
int reverb (int entrada);

// Phaser.c
void phaser_parametros();
int phaser(int);
int Phaser_AP1(int, int);
float Phaser_LFO(int);
void Phaser_Rate(GUIElement*);
void Push_State_Phaser(GUIElement*);
int Get_State_Phaser();

// Ecualizador.c
void init_eq();
void eq_parametros();
int ecualizador(int);
int eq_peak(int,int);
int eq_shelve(int,int);
void EQ_Gain_0 (GUIElement*);
void EQ_Gain_1 (GUIElement*);
void EQ_Gain_2 (GUIElement*);
void EQ_Gain_3 (GUIElement*);
void EQ_Gain_4 (GUIElement*);
void EQ_Gain_5 (GUIElement*);
void EQ_Gain_6 (GUIElement*);
void EQ_Gain_7 (GUIElement*);
void EQ_Gain_8 (GUIElement*);
void EQ_Gain_9 (GUIElement*);
void Push_State_EQ (GUIElement*);
int Get_State_EQ (void);

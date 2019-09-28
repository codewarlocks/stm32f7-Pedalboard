#include "gui/gui.h"
#define ARM_MATH_CM7
#include "arm_math.h"
#include "arm_common_tables.h"
#include "stm32746g_discovery_audio.h"

#define CUADRADA 0
#define TRIANGULAR 1
#define RAMPA_ASC 2
#define RAMPA_DESC 3
#define SINUSOIDAL 4
#define ENVOLVENTE 5

#define SR DEFAULT_AUDIO_IN_FREQ

//Pedales Individuales
void initAppGUI(void);
void InitEfectos (void);

// Delay.c
void delayInit ();
void delayParametros ();
int delayEfecto (int);
void delayTime (GUIElement*);
void delayFeedback (GUIElement*);
void delayLevel (GUIElement*);

// Autowah.c
void autowahInit ();
void autowahParametros ();
int autowahEfecto (int);
float32_t autowahLFO (int);
void autowahRate (GUIElement*);
void autowahDepth (GUIElement*);
void autowahVolume (GUIElement*);
void autowahMod (GUIElement*);

// Distorsion.c
void distorsionInit ();
void distorsionParametros ();
int distorsionEfecto (int);
int distorsionShelve (int, int);
void distorsionGain (GUIElement*);
void distorsionBlend (GUIElement*);

// Vibrato.c
void vibratoInit ();
void vibratoParametros ();
int vibratoEfecto (int);
float32_t vibratoLFO (int);
void vibratoRate (GUIElement*);
void vibratoDepth (GUIElement*);
void vibratoMod (GUIElement*);


// Tremolo.c
void tremoloInit ();
void tremoloParametros ();
int tremoloEfecto (int);
float32_t tremoloLFO();
void tremoloRate (GUIElement*);
void tremoloDepth (GUIElement*);
void tremoloMod (GUIElement*);

// Ringmod.c
void ringmodInit ();
void ringmodParametros();
int ringmodEfecto(int);
void ringmodRate (GUIElement*);

//Chorus.c
void chorusInit ();
void chorusParametros();
int chorusEfecto(int);
float32_t chorusLFO(int);
void chorusRate (GUIElement *e);
void chorusDepth (GUIElement *e);

//Flanger.c
void flangerInit();
void flangerParametros();
int flangerEfecto (int);
float32_t flangerLFO(int);
void flangerManual (GUIElement *e);
void flangerDepth (GUIElement *e);
void flangerRate (GUIElement *e);
void flangerFeedback (GUIElement *e);

//Octavador.c
void octavadorInit ();
void octavadorParametros ();
int octavadorEfecto (int);

//Reverb
void reverbInit ();
void reverbParametros ();
int reverbEfecto (int entrada);
int reverbAP (int in, int* line, int delay, int cont);
int reverbFBCF (int in, int line[], int delay, int cont, int inst);
void reverbMix (GUIElement *e);
void reverbDecay (GUIElement *e);

// Phaser.c
void phaserInit ();
void phaserParametros ();
int phaserEfecto (int);
int phaserAP1 (int, int);
float phaserLFO (int);
void phaserRate (GUIElement*);

// Ecualizador.c
void eqInit ();
void eqParametros ();
int eqEfecto (int);
int eqPeak (int,int);
int eqShelve (int,int);
void eqGain0 (GUIElement*);
void eqGain1 (GUIElement*);
void eqGain2 (GUIElement*);
void eqGain3 (GUIElement*);
void eqGain4 (GUIElement*);
void eqGain5 (GUIElement*);

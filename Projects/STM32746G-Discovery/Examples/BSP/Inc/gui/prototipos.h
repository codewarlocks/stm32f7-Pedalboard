#include "gui/gui.h"

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
void delayInit (void);
void delayParametros (void);
int delayEfecto (int);
void delayTime (GUIElement*);
void delayFeedback (GUIElement*);
void delayLevel (GUIElement*);

// Autowah.c
void autowahInit (void);
void autowahParametros (void);
int autowahEfecto (int);
int autowahBP2(float32_t, int);
float32_t autowahLFO (int);
void autowahRate (GUIElement*);
void autowahDepth (GUIElement*);
void autowahVolume (GUIElement*);
void autowahMod (GUIElement*);

// Distorsion.c
void distorsionInit (void);
void distorsionParametros (void);
int32_t distorsionEfecto (int32_t);
int distorsionShelve (int, int);
void distorsionGain (GUIElement*);
void distorsionBlend (GUIElement*);

// Vibrato.c
void vibratoInit (void);
void vibratoParametros (void);
int vibratoEfecto (int);
float32_t vibratoLFO (int);
void vibratoRate (GUIElement*);
void vibratoDepth (GUIElement*);
void vibratoMod (GUIElement*);


// Tremolo.c
void tremoloInit (void);
void tremoloParametros (void);
int tremoloEfecto (int);
float32_t tremoloLFO(void);
void tremoloRate (GUIElement*);
void tremoloDepth (GUIElement*);
void tremoloMod (GUIElement*);

// Ringmod.c
void ringmodInit (void);
void ringmodParametros(void);
int ringmodEfecto(int);
void ringmodRate (GUIElement*);

//Chorus.c
void chorusInit (void);
void chorusParametros(void);
int chorusEfecto(int);
float32_t chorusLFO(int);
void chorusRate (GUIElement *e);
void chorusDepth (GUIElement *e);

//Flanger.c
void flangerInit(void);
void flangerParametros(void);
int flangerEfecto (int);
float32_t flangerLFO(int);
void flangerManual (GUIElement *e);
void flangerDepth (GUIElement *e);
void flangerRate (GUIElement *e);
void flangerFeedback (GUIElement *e);

//Octavador.c
void octavadorInit (void);
void octavadorParametros (void);
int octavadorEfecto (int);

//Reverb
void reverbInit (void);
void reverbParametros (void);
int reverbEfecto (int entrada);
int reverbAP (int in, int* line, int delay, int cont);
int reverbFBCF (int in, int line[], int delay, int cont, int inst);
void reverbMix (GUIElement *e);
void reverbDecay (GUIElement *e);

// Phaser.c
void phaserInit (void);
void phaserParametros (void);
int phaserEfecto (int);
int phaserAP1 (int, int);
float phaserLFO (int);
void phaserRate (GUIElement*);

// Ecualizador.c
void eqInit (void);
void eqParametros (void);
int eqEfecto (int);
int eqPeak (int,int);
int eqShelve (int,int);
void eqGain0 (GUIElement*);
void eqGain1 (GUIElement*);
void eqGain2 (GUIElement*);
void eqGain3 (GUIElement*);
void eqGain4 (GUIElement*);
void eqGain5 (GUIElement*);

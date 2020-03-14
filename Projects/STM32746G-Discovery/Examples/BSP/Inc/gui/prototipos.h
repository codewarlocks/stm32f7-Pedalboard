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
int32_t delayEfecto (int32_t);
void delayTime (GUIElement*);
void delayFeedback (GUIElement*);
void delayLevel (GUIElement*);

// Autowah.c
void autowahInit (void);
void autowahParametros (void);
int32_t autowahEfecto (int32_t);
int32_t autowahBP2(float32_t, int32_t);
float32_t autowahLFO (int32_t);
void autowahRate (GUIElement*);
void autowahDepth (GUIElement*);
void autowahVolume (GUIElement*);
void autowahMod (GUIElement*);

// Distorsion.c
void distorsionInit (void);
void distorsionParametros (void);
int32_t distorsionEfecto (int32_t);
int32_t distorsionShelve (int32_t, int32_t);
void distorsionGain (GUIElement*);
void distorsionBlend (GUIElement*);

// Vibrato.c
void vibratoInit (void);
void vibratoParametros (void);
int32_t vibratoEfecto (int32_t);
float32_t vibratoLFO (int32_t);
void vibratoRate (GUIElement*);
void vibratoDepth (GUIElement*);
void vibratoMod (GUIElement*);


// Tremolo.c
void tremoloInit (void);
void tremoloParametros (void);
int tremoloEfecto (int32_t);
float32_t tremoloLFO(void);
void tremoloRate (GUIElement*);
void tremoloDepth (GUIElement*);
void tremoloMod (GUIElement*);

// Ringmod.c
void ringmodInit (void);
void ringmodParametros(void);
int32_t ringmodEfecto(int32_t);
void ringmodRate (GUIElement*);

//Chorus.c
void chorusInit (void);
void chorusParametros(void);
int32_t chorusEfecto(int32_t);
float32_t chorusLFO(int32_t);
void chorusRate (GUIElement *e);
void chorusDepth (GUIElement *e);

//Flanger.c
void flangerInit(void);
void flangerParametros(void);
int flangerEfecto (int32_t);
float32_t flangerLFO(int32_t);
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
int32_t reverbEfecto (int32_t entrada);
int32_t reverbAP (int32_t in, int32_t* line, int32_t delay, int32_t cont);
int32_t reverbFBCF (int32_t in, int32_t line[], int32_t delay, int32_t cont, int32_t inst);
void reverbMix (GUIElement *e);
void reverbDecay (GUIElement *e);

// Phaser.c
void phaserInit (void);
void phaserParametros (void);
int32_t phaserEfecto (int32_t);
int32_t phaserAP1 (int32_t, int32_t);
float32_t phaserLFO (int32_t);
void phaserRate (GUIElement*);

// Ecualizador.c
void eqInit (void);
void eqParametros (void);
int32_t eqEfecto (int32_t);
int32_t eqPeak (int32_t,int32_t);
int32_t eqShelve (int32_t,int32_t);
void eqGain0 (GUIElement*);
void eqGain1 (GUIElement*);
void eqGain2 (GUIElement*);
void eqGain3 (GUIElement*);
void eqGain4 (GUIElement*);
void eqGain5 (GUIElement*);

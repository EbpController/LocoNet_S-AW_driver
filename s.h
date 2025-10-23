/* 
 * file: s.h
 * author: J. van Hooydonk
 * comments: (Belgium) signal driver
 *
 * revision history:
 *  v1.0 Creation (15/09/2024)
 *  v1.1 Keep state of S in EEPROM, other corrections (23/08/2025)
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef S_H
#define	S_H

#include "config.h"
#include "eeprom.h"

// definitions
#define CVT_ON_TIME 240         // CVT on time, 600msec = 240 (with 2500us)
#define CVT_OFF_TIME 160        // CVT off time, 400msec = 160 (with 2500us)
#define FADE_IN 5               // lamp fade IN time (higher value is faster)
// fade time = 1000msec/value
// (typ. value between 1 and 10)
#define FADE_OUT 6              // lamp fade OUT time (higher value is faster)
// fade time = 1000msec/value
// (typ. value between 1 and 10)
#define INTENSITY_MAX 400       // maximum value intensity
// aspect modes
// 0: R
// 1: W
// 2: Y
// 3: H
// 4: V
// 5: G
// 6: Y + BA1
// 7: H + BA1
// 8: V + BA1
// 9: G + BA1
// 10: Y + BA2
// 11: H + BA2
// 12: V + BA2
// 13: G + BA2
// 14: Y + BA1 + BA2
// 15: H + BA1 + BA2
// 16: V + BA1 + BA2
// 17: G + BA1 + BA2
#define ASPECT_MODES 18         // total different aspect modes 
// led positions
#define LED_W 0x40              // W
#define LED_YV 0x20             // YV
#define LED_R 0x10              // R
#define LED_G 0x08              // G
#define LED_YH 0x04             // YH
#define LED_BA1 0x02            // BA1
#define LED_BA2 0x01            // BA2
// led KFS/KOS
#define LED_KFS 0x40            // KFS
#define LED_KOS 0x20            // KOS

typedef struct {
    uint16_t R;
    uint16_t W;
    uint16_t YH;
    uint16_t YV;
    uint16_t G;
    uint16_t BA1;
    uint16_t BA2;
} sIntensity_t;

typedef struct {
    sIntensity_t intensity;
    uint8_t aspect;
    bool KOS;
    bool KFS;
    bool CVT_mode;
    uint16_t periodCounter;
} SCON_t;

// servo callback definition (as function pointer)
typedef void (*sCallback_t)(uint8_t);

// initialisation
void sInit(sCallback_t);

// ISR timer 3
void sIsrTmr3(void);

// routines
bool periodCounter(uint8_t);
void setIntensity(uint8_t);
void setIntensityMainPanel(uint8_t, uint8_t);
bool fadeIn(uint16_t*);
bool fadeOut(uint16_t*);
void setKOS(uint8_t, bool);
void setKFS(uint8_t, bool);
void setAspect(uint8_t, uint8_t);
bool isAspectValid(uint8_t, uint8_t);
void pwmDriver(void);

// variables
sCallback_t sCallback;
uint16_t pwmCounter;
SCON_t sList[8];

#endif	/* S_H */

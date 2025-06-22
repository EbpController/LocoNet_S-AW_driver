/* 
 * file: s.h
 * author: J. van Hooydonk
 * comments: (Belgium) signal driver
 *
 * revision history:
 *  v1.0 Creation (15/09/2024)
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef S_H
#define	S_H

#include "config.h"

// definitions
#define CVT_ON_TIME 240        // CVT on time, 600msec = 240 (with 2500us)
#define CVT_OFF_TIME 160       // CVT off time, 400msec = 160 (with 2500us)
#define FADE_IN 5               // lamp fade IN time (higher value is faster)
// fade time = 1000msec/value
// (typ. value between 1 and 10)
#define FADE_OUT 6              // lamp fade OUT time (higher value is faster)
// fade time = 1000msec/value
// (typ. value between 1 and 10)
#define INTENSITY_MAX 400     // maximum value intensity

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

// B signal lookup tabel
//  (where led W = bit 0, led YV = bit 1, led R = bit 2, led G = bit 3,
//         led YH = bit 4, led BA1 = bit 5, led BA2 = bit 6)
// 
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
uint8_t aspect[18] = {0b00000100,
    0b00000101,
    0b00010010,
    0b00011000,
    0b00001010,
    0b00001000,
    0b00110010,
    0b00111000,
    0b00101010,
    0b00101000,
    0b01010010,
    0b01011000,
    0b01001010,
    0b01001000,
    0b01110010,
    0b01111000,
    0b01101010,
    0b01101000};

// servo callback definition (as function pointer)
typedef void (*sCallback_t)(uint8_t);

// routines
void sInit(sCallback_t);
void getLastSState(SCON_t s[]);
void sIsrTmr3();
void writeS(uint8_t, bool, uint8_t);
void pwmDriver(void);
bool periodCounter(uint8_t);
void setIntensity(uint8_t);
void setIntensityMainPanel(uint8_t, uint8_t);
bool fadeIn(uint16_t*);
bool fadeOut(uint16_t*);
void setKOS(uint8_t, bool);
void setKFS(uint8_t, bool);
void setAspect(uint8_t, uint8_t);
bool isAspectValid(uint8_t, uint8_t);

// variables
// S used variables
sCallback_t sCallback;
uint16_t pwmCounter;
SCON_t s[8];

#endif	/* S_H */

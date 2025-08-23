/* 
 * file: aw.h
 * author: J. van Hooydonk
 * comments: AW driver
 *
 * revision history:
 *  v1.0 Creation (16/08/2024)
 *  v1.1 Add CAW control, keep state of AW in EEPROM (23/08/2025)
*/

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef AW_H
#define	AW_H

#include "config.h"
#include "servo.h"

// definitions
// sweeptime time in ms from min/max to max/min position
#define SWEEPTIME 4000U             // time in msec (1000 = 1 sec)
// the puls duration of the servo must be between 500µs and 2250µs (SG90)
#define SERVO_MIN 1000U             // abs. max. value = 500 (-90°), 1000 = -45°
#define SERVO_MAX 1800U             // abs. max. value = 2250 (90°), 1800 = 45°
// the period for the servo is 20ms
// so, for a certain sweeptime, the number of steps to add or subtrack is
// equal to the sweeptime divided by the period (SWEEPTIME / 20)
// the value (= GRADIENT) to add or subtract is than calculated as follow
#define GRADIENT (uint8_t)((SERVO_MAX - SERVO_MIN) / (SWEEPTIME / 20))
// led CAW/KAW
#define LED_CAWL 0x80               // LED CAWL
#define LED_KAWL 0x01               // LED KAWL
#define LED_KAWR 0x02               // LED KAWR
#define LED_CAWR 0x04               // LED CAWR
#define SWITCH_CAWL LATC5           // SWITCH CAWL
#define SWITCH_KAWL LATC5           // SWITCH KAWL
#define SWITCH_CAWR LATC4           // SWITCH CAWR
#define SWITCH_KAWR LATC4           // SWITCH KAWR

#define CAW_CONTROL
// #define KAW_CONTROL

// AW status register
typedef struct
    {
        bool CAWL;
        bool CAWR;
        bool KAWL;
        bool KAWR;
    } AWCON_t;

// AW callback definition (as function pointer)
typedef void (*awCallback_t)(AWCON_t*, uint8_t);

// routines
void awInit(awCallback_t);
void awInitPortBC(void);
void getLastCawState(AWCON_t aw[]);
void checkCawState();
void awUpdate(uint8_t);
void awUpdateServo(AWCON_t*, uint16_t*, uint8_t);
void setCAWL(AWCON_t*, uint8_t, bool);
void setCAWR(AWCON_t*, uint8_t, bool);
void setKAWL(AWCON_t*, uint8_t, bool);
void setKAWR(AWCON_t*, uint8_t, bool);
bool getSwitchKAWL(uint8_t);
bool getSwitchKAWR(uint8_t);
void checkSwitchesCAW(AWCON_t*, uint8_t);
bool getSwitchCAWL(uint8_t);
bool getSwitchCAWR(uint8_t);

// variables
awCallback_t awCallback;
AWCON_t aw[8];

#endif	/* AW_H */

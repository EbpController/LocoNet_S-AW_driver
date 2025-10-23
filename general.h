/* 
 * file: general.h
 * author: J. van Hooydonk
 * comments: general variables, settings and routines 
 *
 * revision history:
 *  v1.0 Creation (21/11/2024)
 *  v2.0 Complete rework of the program (05/10/2025)
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef GENERAL_H
#define	GENERAL_H

// include libraries
#include "aw.h"
#include "circular_queue.h"
#include "eeprom.h"
#include "ln.h"
#include "MAX7219.h"
#include "s.h"
#include "servo.h"

// definitions
#define TIMER3_2500us 5000          // timer 3 delay value, 2500µsec = 5000

// routines
void init(void);
void initTmr3(void);
void initCcp1(void);
void initIsr(void);
void initPorts(void);

void isrHigh(void);
void isrLow(void);
void updateLeds(void);
void lnRxMessageHandler(lnQueue_t*);
void awCawHandler(uint8_t, bool);
void awKawHandler(uint8_t);
void sHandler(uint8_t);
uint8_t getDipSwitchAddress(void);
uint16_t getAddressFromOpcImmPacket(uint8_t, uint8_t);

// variables
lnQueue_t lnTxMsg; //ok
uint8_t index; //ok

#endif	/* GENERAL_H */


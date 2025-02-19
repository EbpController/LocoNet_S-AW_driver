/* 
 * file: general.h
 * author: J. van Hooydonk
 * comments: general variables, settings and routines 
 *
 * revision history:
 *  v1.0 Creation (21/11/2024)
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef GENERAL_H
#define	GENERAL_H

// include libraries

#include "ln.h"
#include "aw.h"
#include "s.h"
#include "MAX7219.h"

// definitions
#define TIMER3_2500us 5000          // timer 3 delay value, 2500µsec = 5000

// routines
void init(void);
void initTmr3(void);
void initCcp1(void);
void initIsr(void);
void initPorts(void);

void isrLow(void);
void isrHigh(void);
void updateLeds(void);
void lnRxMessageHandler(lnQueue_t*);
void awHandler(AWCON_t*, uint8_t);
void sHandler(uint8_t);
uint8_t getDipSwitchAddress(void);
uint16_t getAddressFromOpcImmPacket(uint8_t, uint8_t);

// variables
lnQueue_t lnTxMsg;
uint8_t index;

#endif	/* GENERAL_H */


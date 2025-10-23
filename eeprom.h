/* 
 * file: eeprom.h
 * author: J. van Hooydonk
 * comments: eeprom driver
 *
 * revision history:
 *  v1.0 Creation (14/06/2025)
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef EEPROM_H
#define	EEPROM_H

#include "config.h"
#include "aw.h"
#include "s.h"

// initialisation
void initEeprom(void);
void initHlvd(void);

// routines
void updateEepromData(uint8_t);
void readEepromData(void);
uint8_t eepromRead(uint16_t);
void writeEepromData(void);
void eepromWrite(uint16_t, uint8_t);

// variables
uint8_t eepromData[8];

#endif	/* EEPROM_H */

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

// routines
void eepromWrite(uint16_t, uint8_t);
uint8_t eepromRead(uint16_t);
bool eepromVerify(uint16_t, uint8_t);

#endif	/* EEPROM_H */

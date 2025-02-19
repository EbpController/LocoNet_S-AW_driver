/* 
 * file: MAX7219.h
 * author: J. van Hooydonk
 * comments: MAX7219 (SPI - LED matrix) driver
 *
 * revision history:
 *  v1.0 Creation (20/11/2024)
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef MAX7219_H
#define	MAX7219_H

#include "config.h"

// define MAX7219 register addresses
#define MAX7219_MODE_NOP       0x00
#define MAX7219_MODE_DECODE    0x09
#define MAX7219_MODE_INTENSITY 0x0a
#define MAX7219_MODE_SCANLIMIT 0x0b
#define MAX7219_MODE_SHUTDOWN  0x0c
#define MAX7219_MODE_TEST      0x0f

// define MAX7219 commands
#define MAX7219_NO_DECODE      0x00
#define MAX7219_7SEG_DECODE    0xff

// define MAX7219 pins
#define DIN RE0
#define CLK RE1
#define CS RE2

// define functions
void MAX7219_init(void);
void MAX7219_send(uint8_t a, uint8_t d);
void MAX7219_update(void);

// default brightness
uint8_t brightness = 15;


#endif	/* MAX7219_H */


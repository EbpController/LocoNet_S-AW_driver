/* 
 * file: servo.h
 * author: J. van Hooydonk
 * comments: servo motor driver
 *
 * revision history:
 *  v1.0 Creation (16/08/2024)
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef SERVO_H
#define	SERVO_H

#include "config.h"

// servo callback definition (as function pointer)
typedef void (*servoCallback_t)(uint8_t);

// initialisation
void servoInit(servoCallback_t);
void servoInitPortD(void);

// ISR routines
void servoIsrTmr3(uint8_t);
void servoIsrCcp1(void);

// variables
servoCallback_t servoCallback;
uint16_t servoPortD[8];

#endif	/* SERVO_H */

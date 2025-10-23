/*
 * file: servo.c
 * author: J. van Hooydonk
 * comments: servo motor driver
 *
 * revision history:
 *  v1.0 Creation (16/08/2024)
 */

#include "servo.h"

// <editor-fold defaultstate="collapsed" desc="initialisation">

/**
 * servo motor driver initialisation
 * @param fptr: the function pointer to the (callback) servo handler
 */
void servoInit(servoCallback_t fptr)
{
    // init servo callback function (function pointer)
    servoCallback = fptr;

    // initialisation of the servo variables
    for (uint8_t i = 0; i < 8; i++)
    {
        servoPortD[i] = 1500U;
    }

    // init of the other elements (timer, comparator, IST, port)
    servoInitPortD();
}

/**
 * servo motor driver initialisation of the output port D (= 8 servos)
 */
void servoInitPortD(void)
{
    // port D
    TRISD = 0x00; // configure all pins of port D as output
    LATD = 0x00; // set them to 0    
}

// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="ISR routines">

/**
 * interrupt routine for timer 3
 */
void servoIsrTmr3(uint8_t index)
{
    // get servo values (in the callback function)
    (*servoCallback)(index);
    // toggle output port D pin[index]
    LATD = (uint8_t) (0x01 << index);
}

/**
 * interrupt routine for comparator (CCP1)
 */
void servoIsrCcp1(void)
{
    // set (all) pin(s) of port D to 0
    LATD = 0x00;
}

// </editor-fold>

/*
 * file: servo.c
 * author: J. van Hooydonk
 * comments: servo motor driver
 *
 * revision history:
 *  v1.0 Creation (16/08/2024)
*/

#include "MAX7219.h"

// <editor-fold defaultstate="collapsed" desc="initialisation">

/**
 * MAX7219 initialisation
*/
void MAX7219_init()
{
    // init ports
    // set ports E0, E1, E2 as output
    TRISEbits.TRISE0 = false;
    TRISEbits.TRISE1 = false;
    TRISEbits.TRISE2 = false;
    // and disable analog feature
    ANSELEbits.ANSELE0 = false;
    ANSELEbits.ANSELE1 = false;
    ANSELEbits.ANSELE2 = false;
    
    // init MAX7219
    // scan all eight rows
    MAX7219_send(MAX7219_MODE_SCANLIMIT, 0x07);
    MAX7219_send(MAX7219_MODE_SCANLIMIT, 0x07);
    MAX7219_update();
    
    // set MAX7219 to no-decoding mode
	// (we are specifying the pattern manually)
    MAX7219_send(MAX7219_MODE_DECODE, MAX7219_NO_DECODE);
    MAX7219_send(MAX7219_MODE_DECODE, MAX7219_NO_DECODE);
    MAX7219_update();
    
    // set MAX7219 brightness to medium
    // (any number from 0-15 works)
    MAX7219_send(MAX7219_MODE_INTENSITY, 8);
    MAX7219_send(MAX7219_MODE_INTENSITY, 8);
    MAX7219_update();
    
    // turn ON
    MAX7219_send(MAX7219_MODE_SHUTDOWN, 1);
    MAX7219_send(MAX7219_MODE_SHUTDOWN, 1);
    MAX7219_update();

    // clear all dot matrix displays
    MAX7219_send(1, 0x00); MAX7219_send(1, 0x00); MAX7219_update();
    MAX7219_send(2, 0x00); MAX7219_send(2, 0x00); MAX7219_update();
    MAX7219_send(3, 0x00); MAX7219_send(3, 0x00); MAX7219_update();
    MAX7219_send(4, 0x00); MAX7219_send(4, 0x00); MAX7219_update();
    MAX7219_send(5, 0x00); MAX7219_send(5, 0x00); MAX7219_update();
    MAX7219_send(6, 0x00); MAX7219_send(6, 0x00); MAX7219_update();
    MAX7219_send(7, 0x00); MAX7219_send(7, 0x00); MAX7219_update();
    MAX7219_send(8, 0x00); MAX7219_send(8, 0x00); MAX7219_update();    
}

// </editor-fold>

/**
 * send out the address byte and the data byte in the MAX7219 format
 * @param address: the address to be send
 * @param data: the data to be send
 */
void MAX7219_send (uint8_t address, uint8_t data) {
    // the sequence of bits is a7-a6-a5-a4-a3-a2-a1-a0-d7-d6-d5-d4-d3-d2-d1-d0
	// send out address byte, start with most significant bit and work backwards
    for (int8_t i = 7; i >= 0; i--)
    {
        DIN = (address >> i) & 1;
        CLK = 1;
        CLK = 0;
    }	
	// send out data byte, start with most significant bit and work backwards
    for (int8_t i = 7; i >= 0; i--)
    {
        DIN = (data >> i) & 1;
        CLK = 1;
        CLK = 0;
    }	
	// reset the data pin back to zero
	// so that it is not left ON if the last sent bit was a 1
    DIN = 0;
}

/**
 * update MAX7219 data
 */
void MAX7219_update()
{
	// pulls the LOAD pin high and then back to zero,
    // so that the transmitted data appears in the MAX7219 output stage
    CS = 1;
    CS = 0;	
}




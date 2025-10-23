/*
 * file: aw.c
 * author: J. van Hooydonk
 * comments: AW driver
 *
 * revision history:
 *  v1.0 Creation (16/08/2024)
 *  v1.1 Add CAW control, keep state of AW in EEPROM (23/08/2025)
 */

#include "aw.h"

// <editor-fold defaultstate="collapsed" desc="initialisation">

/**
 * AW driver initialisation
 * @param fptr: the function pointer to the (callback) AW handler
 */
void awInit(awCawCallback_t fptrCaw, awKawCallback_t fptrKaw)
{
    // init servo callback function (function pointer for Caw)
    awCawCallback = fptrCaw;
    // init servo callback function (function pointer for Kaw)
    awKawCallback = fptrKaw;
    // init of the AW ports B and C (= KAWL/KAWR switches)
    awInitPortBC();
    // initialisation of the servo variables
    servoInit(&awUpdate);
}

/**
 * AW initialisation of the ports B and C (= KAWL/KAWR switches)
 */
void awInitPortBC()
{
    // setup digital inputs to read the KAWL and KAWR switches

    // scheme:
    //
    //               * PORTB pin x (0 to 7) *
    //               |                      |
    //                 /                      /
    // switch KAWLx - /       switch KAWRx - /
    //               |                      |
    //          PORTC pin 4            PORTC pin 5
    //         (= KAWL line)         (= KAWR line)
    //

    // set all pins of PORTB to inputs with pull-up
    TRISB = 0xff; // disable output (= input) on pin B0 - B7
    ANSELB = 0x00; // enable TTL input buffer on pin B0 - B7
    WPUB = 0xff; // enable pull-up on pin B0 - B7

    // setup PORTC pin 4 and 5 as outputs (controlling KAWL/KAWR switches)
    // and make output high (active low)
    // C4 will be the (power) line to control the KAWL switches
    // C5 will be the (power) line to control the KAWR switches
    TRISCbits.TRISC4 = false;
    TRISCbits.TRISC5 = false;
    LATCbits.LATC4 = true;
    LATCbits.LATC5 = true;
}

/**
 * get last KAW states
 */
void getLastAwState()
{
    // get last state of KAWL and KAWR and put them in the CAW
    for (uint8_t i = 0; i < 8; i++)
    {
        awList[i].CAWL = awList[i].KAWL_lastState;
        awList[i].CAWR = awList[i].KAWR_lastState;

        if (awList[i].CAWL && awList[i].CAWR)
        {
            awList[i].CAWL = false;
            awList[i].CAWR = false;
        }
    }
}

// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="routines">

/**
 * this is the callback function for the AW update
 * @param index: the index of AW in the AW list
 */
void awUpdate(uint8_t index)
{
    // update servo on port D
    awUpdateServo(&servoPortD[index], index);
#ifdef CAW_CONTROL
    // check switches CAW
    checkSwitchesCAW(index);
#endif
}

/**
 * update servomotor
 * @param aw: pointer to the AW parameters
 * @param servo: pointer to the servo
 * @param index: the index of AW in the AW list
 */
void awUpdateServo(uint16_t *servo, uint8_t index)
{
    // increment pulse width with gradient depending on state of CAW
    if (awList[index].CAWL == awList[index].CAWR)
    {
        // if CAWL = CAWR clear KAWs and set the servo position in the middle 
        setKAWL(index, false);
        setKAWR(index, false);
        if (*servo > ((SERVO_MAX + SERVO_MIN) / 2) + GRADIENT)
        {
            *servo -= GRADIENT;
        }
        else if (*servo < ((SERVO_MAX + SERVO_MIN) / 2) - GRADIENT)
        {
            *servo += GRADIENT;
        }
    }
    else
    {
        // if CAWL then clear KAWR and set servo left
        if (awList[index].CAWL)
        {
            setKAWR(index, false);
            if (getSwitchKAWL(index))
            {
                setKAWL(index, true);
            }
            else
            {
                if (*servo > (SERVO_MAX - GRADIENT))
                {
                    *servo = SERVO_MAX;
                    setKAWL(index, true);
                }
                else
                {
                    *servo += GRADIENT;
                    setKAWL(index, false);
                }
            }
        }
        // if CAWR then clear KAWL and set servo right
        if (awList[index].CAWR)
        {
            setKAWL(index, false);
            if (getSwitchKAWR(index))
            {
                setKAWR(index, true);
            }
            else
            {
                if (*servo < SERVO_MIN + GRADIENT)
                {
                    *servo = SERVO_MIN;
                    setKAWR(index, true);
                }
                else
                {
                    *servo -= GRADIENT;
                    setKAWR(index, false);
                }
            }
        }
    }
}

/**
 * set the property CAWL
 * @param index: the index of AW in the AW list
 * @param value: true or false (= state of CAWL)
 */
void setCAWL(uint8_t index, bool value)
{
    // update CAWL
    awList[index].CAWL = value;
}

/**
 * set the property CAWR
 * @param index: the index of AW in the AW list
 * @param value: true or false (= state of CAWR)
 */
void setCAWR(uint8_t index, bool value)
{
    awList[index].CAWR = value;
}

/**
 * set the property KAWL
 * @param index: the index of AW in the AW list
 * @param value: true or false (= state of KAWL)
 */
void setKAWL(uint8_t index, bool value)
{
    // set KAWL
    if (awList[index].KAWL != value)
    {
        awList[index].KAWL = value;
        // handle the changed KAW state (in the callback function)
        (*awKawCallback)(index);
        // hold  last state of KAWL in memory
        awList[index].KAWL_lastState = value;
        // update EEPROM data
        updateEepromData(index);
    }
}

/**
 * set the property KAWR
 * @param index: the index of AW in the AW list
 * @param value: true or false (= state of KAWR)
 */
void setKAWR(uint8_t index, bool value)
{
    // set KAWR
    if (awList[index].KAWR != value)
    {
        awList[index].KAWR = value;
        // handle the changed KAW state (in the callback function)
        (*awKawCallback)(index);
        // hold  last state of KAWR in memory
        awList[index].KAWR_lastState = value;
        // update EEPROM data
        updateEepromData(index);
    }
}

/**
 * get the state of the switch of KAWL
 * @param index: the index of AW in the AW list
 * @return the state of the switch
 */
bool getSwitchKAWL(uint8_t index)
{
    bool value = false;

#ifdef KAW_CONTROL
    // enable KAWL line (active low))
    LATCbits.LATC4 = false;
    // get KAWL switch state
    if ((PORTB & (1 << index)) == 0)
    {
        value = true;
    }
    // disable KAWL line (active low)
    LATCbits.LATC4 = true;
#endif
    // return the state of the KAWL switch
    return value;
}

/**
 * get the state of the switch of KAWR
 * @param index: the index of AW in the AW list
 * @return the state of the switch
 */
bool getSwitchKAWR(uint8_t index)
{
    bool value = false;

#ifdef KAW_CONTROL
    // enable KAWR line (active low))
    LATCbits.LATC5 = false;
    // get KAWR switch state
    if ((PORTB & (1 << index)) == 0)
    {
        value = true;
    }
    // disable KAWR line (active low)
    LATCbits.LATC5 = true;
#endif
    // return the state of the KAWL switch
    return value;
}

/**
 * check the switches CAWL and CAWR
 * @param index: the index of AW in the AW list
 */
void checkSwitchesCAW(uint8_t index)
{
    // check if switch CAWL is pressed
    if (!awList[index].CAWL)
    {
        if (getSwitchCAWL(index))
        {
            // handle the CAWL state (in the callback function)
            (*awCawCallback)(index, true);
        }
    }
    // check if switch CAWR is pressed
    if (!awList[index].CAWR)
    {
        if (getSwitchCAWR(index))
        {
            // handle the CAWR state (in the callback function)
            (*awCawCallback)(index, false);
        }
    }
}

/**
 * get the state of the switch of CAWL
 * @param index: the index of AW in the AW list
 * @return the state of the switch
 */
bool getSwitchCAWL(uint8_t index)
{
    bool value = false;

    // enable CAWL line (active low)
    LATCbits.SWITCH_CAWL = false;
    // get CAWL switch state and check that CAWR is true
    //  to prevent that the 2 switches are pressed at the same time
    if ((PORTB & (1 << index)) == 0)
    {
        value = true;
    }
    // disable CAWL line (active low)
    LATCbits.SWITCH_CAWL = true;
    // return the state of the CAWL switch

    return value;
}

/**
 * get the state of the switch of CAWR
 * @param index: the index of AW in the AW list
 * @return the state of the switch
 */
bool getSwitchCAWR(uint8_t index)
{
    bool value = false;

    // enable CAWR line (active low)
    LATCbits.SWITCH_CAWR = false;
    // get CAWR switch state and check that CAWL is true
    //  to prevent that the 2 switches are pressed at the same time
    if ((PORTB & (1 << index)) == 0)
    {
        value = true;
    }
    // disable CAWR line (active low)
    LATCbits.SWITCH_CAWR = true;
    // return the state of the CAWR switch
    return value;
}

// </editor-fold>

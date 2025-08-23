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
#include "eeprom.h"

// <editor-fold defaultstate="collapsed" desc="initialisation">

/**
 * AW driver initialisation
 * @param fptr: the function pointer to the (callback) AW handler
 */
void awInit(awCallback_t fptr)
{
    // init servo callback function (function pointer)
    awCallback = fptr;
    // init of the AW ports B and C (= KAWL/KAWR switches)
    awInitPortBC();
    // initialisation of the servo variables
    servoInit(&awUpdate);
    // get last CAW state (from EEPROM)
    getLastAwState();
    checkCawState();
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
 * get last CAW state (from EEPROM)
 */
void getLastAwState()
{
    // read the KAWL and KAWR values from EEPROM
    uint8_t KAWL_mem = eepromRead(ADRS_KAWL);
    uint8_t KAWR_mem = eepromRead(ADRS_KAWR);
    // get CAWL and CAWR
    for (uint8_t i = 0; i < 8; i++)
    {
        awList[i].CAWL = (KAWL_mem & (0x01 << i)) != 0;
        awList[i].CAWR = (KAWR_mem & (0x01 << i)) != 0;
    }
}

void checkCawState()
{
    // check states of CAWL and CAWR, if CAWL and CAWR are both 'true'
    // then set the state of CAWL and CAWR to 'false'
    for (uint8_t i = 0; i < 8; i++)
    {
        if ((awList[i].CAWL == true) && (awList[i].CAWR == true))
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
    awUpdateServo(&awList[index], &servoPortD[index], index);
#ifdef CAW_CONTROL
    // check switches CAW
    checkSwitchesCAW(&awList[index], index);
#endif
}

/**
 * update servomotor
 * @param aw: pointer to the AW parameters
 * @param servo: pointer to the servo
 * @param index: the index of AW in the AW list
 */
void awUpdateServo(AWCON_t *aw, uint16_t *servo, uint8_t index)
{
    // increment pulse width with gradient depending on state of CAW
    if (aw->CAWL == aw->CAWR)
    {
        // if CAWL = CAWR clear KAWs and set the servo position in the middle 
        setKAWL(aw, index, false);
        setKAWR(aw, index, false);
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
        if (aw->CAWL)
        {
            setKAWR(aw, index, false);
            if (getSwitchKAWL(index))
            {
                setKAWL(aw, index, true);
            }
            else
            {
                if (*servo > (SERVO_MAX - GRADIENT))
                {
                    *servo = SERVO_MAX;
                    setKAWL(aw, index, true);
                }
                else
                {
                    *servo += GRADIENT;
                    setKAWL(aw, index, false);
                }
            }
        }
        // if CAWR then clear KAWL and set servo right
        if (aw->CAWR)
        {
            setKAWL(aw, index, false);
            if (getSwitchKAWR(index))
            {
                setKAWR(aw, index, true);
            }
            else
            {
                if (*servo < SERVO_MIN + GRADIENT)
                {
                    *servo = SERVO_MIN;
                    setKAWR(aw, index, true);
                }
                else
                {
                    *servo -= GRADIENT;
                    setKAWR(aw, index, false);
                }
            }
        }
    }
}

/**
 * set the property CAWL
 * @param aw: pointer to the AW parameters
 * @param index: the index of AW in the AW list
 * @param value: true or false (= state of CAWL)
 */
void setCAWL(AWCON_t *aw, uint8_t index, bool value)
{
    // update CAWL
    aw->CAWL = value;
}

/**
 * set the property CAWR
 * @param aw: pointer to the AW parameters
 * @param index: the index of AW in the AW list
 * @param value: true or false (= state of CAWR)
 */
void setCAWR(AWCON_t *aw, uint8_t index, bool value)
{
    aw->CAWR = value;
}

/**
 * set the property KAWL
 * @param aw: pointer to the AW parameters
 * @param index: the index of AW in the AW list
 * @param value: true or false (= state of KAWL)
 */
void setKAWL(AWCON_t *aw, uint8_t index, bool value)
{
    // set KAWL
    if (aw->KAWL != value)
    {
        aw->KAWL = value;
        // handle the changed KAW state (in the callback function)
        (*awCallback)(aw, index);
        // write KAW info to EEPROM (only if KAWL = 1)
        if (aw->KAWL)
        {
            // read the CAWL and CAWR values
            uint8_t KAWL_mem = eepromRead(ADRS_KAWL);
            uint8_t KAWR_mem = eepromRead(ADRS_KAWR);
            // modify the corresponding KAWL and KAWR bits
            KAWL_mem |= (0x01 << index);
            KAWR_mem &= ~(0x01 << index);
            // overwrite the KAWL and KAWR values
            eepromWrite(ADRS_KAWL, KAWL_mem);
            eepromWrite(ADRS_KAWR, KAWR_mem);
        }
    }
}

/**
 * set the property KAWR
 * @param aw: pointer to the AW parameters
 * @param index: the index of AW in the AW list
 * @param value: true or false (= state of KAWR)
 */
void setKAWR(AWCON_t *aw, uint8_t index, bool value)
{
    // set KAWR
    if (aw->KAWR != value)
    {
        aw->KAWR = value;
        // handle the changed KAW state (in the callback function)
        (*awCallback)(aw, index);
        // write KAW info to EEPROM (only if KAWR = 1)
        if (aw->KAWR)
        {
            // read the KAWL and KAWR values
            uint8_t KAWL_mem = eepromRead(ADRS_KAWL);
            uint8_t KAWR_mem = eepromRead(ADRS_KAWR);
            // modify the corresponding KAWL and KAWR bits
            KAWL_mem &= ~(0x01 << index);
            KAWR_mem |= (0x01 << index);
            // overwrite the KAWL and KAWR values
            eepromWrite(ADRS_KAWL, KAWL_mem);
            eepromWrite(ADRS_KAWR, KAWR_mem);
        }
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
 * @param aw: pointer to the AW parameters
 * @param index: the index of AW in the AW list
 */
void checkSwitchesCAW(AWCON_t *aw, uint8_t index)
{
    // check if switch CAWL is pressed
    if (!aw->CAWL)
    {
        if (getSwitchCAWL(index))
        {
            setCAWL(aw, index, true);
            setCAWR(aw, index, false);
        }
    }
    // check if switch CAWR is pressed
    if (!aw->CAWR)
    {
        if (getSwitchCAWR(index))
        {
            setCAWR(aw, index, true);
            setCAWL(aw, index, false);
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
    // get CAWL switch state
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
    // get CAWR switch state
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

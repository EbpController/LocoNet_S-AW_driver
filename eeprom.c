/*
 * file: eeprom.c
 * author: J. van Hooydonk
 * comments: EEPROM driver
 *
 * revision history:
 *  v1.0 Creation (14/06/2025)
 */

#include <eeprom.h>

// <editor-fold defaultstate="collapsed" desc="initialisation">

/**
 * initialisation
 */
void initEeprom()
{
    // setup the HLVD module to fire an interrupt at power down and
    //  to store the values of S and AW in EEPROM
    initHlvd();
}

/**
 * initialisation of the HLVD module
 */
void initHlvd()
{
    // initialisation of the HLVD (High/Low-Voltage Detector) so that a
    //  high priority interrupt could be fired when the power supply gets off
    //  and that the last states of S and AW could be write in EEPROM
    // first disable the module to prevent the generation of false HLVD events
    HLVDCON0bits.EN = false;
    // set the HLVD trip point to 4.65V
    HLVDCON1bits.SEL = 0b1100;
    // application must detect a low-voltage drop
    HLVDCON0bits.INTH = false;
    HLVDCON0bits.INTL = true;
    // clear the HLVD interrupt flag (from previous interrupts)
    PIR2bits.HLVDIF = false;
    // setup the interrupt conditions (high priority)
    PIE2bits.HLVDIE = true;
    // enable the HLVD module
    HLVDCON0bits.EN = true;
    // check the ready status to continue
    while (!HLVDCON0bits.RDY);
}

// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="routines">

/**
 * update the EEPROM data with AW and S infotmation
 * @param index
 */
void updateEepromData(uint8_t index)
{
    // (over)write KAWL/KAWR info + aspect + CVT_mode into EEPROM data
    // the aspect is defined in the bit 0 - bit 4
    eepromData[index] = sList[index].aspect;
    // CVT mode is defined is bit 5
    if (sList[index].CVT_mode)
    {
        eepromData[index] |= 0x20;
    }
    // CAWR/KAWR is defined is bit 6
    if (awList[index].KAWR)
    {
        eepromData[index] |= 0x40;
    }
    // CAWL/KAWL is defined is bit 7
    if (awList[index].KAWL)
    {
        eepromData[index] |= 0x80;
    }
}

/**
 * read the data of AW and S (8 items) from EEPROM
 */
void readEepromData()
{
    // 8 items
    for (uint8_t index = 0; index < 8; index++)
    {
        // read the aspect + CVT_mode into EEPROM
        // the address location is the index value
        uint8_t data = eepromRead((uint16_t) index);

        // CAWL/CAWL is defined is bit 7
        setCAWL(index, (data & 0x80) == 0x80);
        // CAWR/CAWR is defined is bit 6
        setCAWR(index, (data & 0x40) == 0x40);

        // the aspect is defined in the bit 0 - bit 4
        setAspect(index, (data & 0x1f));
        // CVT mode is defined is bit 5
        sList[index].CVT_mode = (data & 0x20) == 0x20;
    }
}

/**
 * read a byte from EEPROM
 * @param address: the address of the EEPROM
 * @return the data that was read
 */
uint8_t eepromRead(uint16_t address)
{
    uint8_t data = 0;

    // enable EEPROM
    NVMCON0bits.NVMEN = true;
    // setup EEPROM address
    NVMADRL = (uint8_t) (address & 0xff);
    NVMADRH = (uint8_t) (address >> 8);
    NVMADRU = 0x31;
    // issue EEPROM read
    NVMCON1bits.RD = true;
    // wait for read to complete
    while (NVMCON1bits.RD);
    // read data from EEPROM
    data = NVMDATL;
    // disable EEPROM
    NVMCON0bits.NVMEN = false;
    // return data
    return data;
}

/**
 * write the data of AW and S (8 items) to EEPROM
 */
void writeEepromData()
{
    for (uint8_t index = 0; index < 8; index++)
    {
        // write the data to EEPROM
        eepromWrite((uint16_t) index, eepromData[index]);
    }
}

/**
 * write a byte to EEPROM
 * @param address: the address of the EEPROM
 * @param data: the data to be write
 */
void eepromWrite(uint16_t address, uint8_t data)
{
    // setup EEPROM address
    NVMADRL = (uint8_t) (address & 0xff);
    NVMADRH = (uint8_t) (address >> 8);
    NVMADRU = 0x31;
    // data to be write
    NVMDATL = data;
    // enable EEPROM
    NVMCON0bits.NVMEN = true;
    // unlock required sequence
    NVMCON2 = 0x55;
    NVMCON2 = 0xaa;
    // begin write
    NVMCON1bits.WR = true;
    // wait for write to complete
    while (NVMCON1bits.WR);
    // disable EEPROM
    NVMCON0bits.NVMEN = false;
}

// </editor-fold>


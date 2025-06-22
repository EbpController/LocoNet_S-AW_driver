/*
 * file: eeprom.c
 * author: J. van Hooydonk
 * comments: EEPROM driver
 *
 * revision history:
 *  v1.0 Creation (14/06/2025)
 */

#include <eeprom.h>

// <editor-fold defaultstate="collapsed" desc="routines">

/**
 * write data to EEPROM
 * @param address: the address of the EEPROM
 * @param data: the data to be write
 */
void eepromWrite(uint16_t address, uint8_t data)
{
    bool success = false;
    while (!success)
    {
        // setup EEPROM address
        NVMADRL = address & 0xff;
        NVMADRH = (address & 0xff00) >> 8;
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
        // while (NVMCON1bits.WR);
        // disable EEPROM
        NVMCON0bits.NVMEN = false;
        // verify whether the values have been successfully written
        success = eepromVerify(address, data);
    }
}

/**
 * read data from EEPROM
 * @param address: the address of the EEPROM
 * @return the data that was read
 */
uint8_t eepromRead(uint16_t address)
{
    uint8_t data;

    // enable EEPROM
    NVMCON0bits.NVMEN = true;
    // setup EEPROM address
    NVMADRL = address & 0xff;
    NVMADRH = (address & 0xff00) >> 8;
    NVMADRU = 0x31;
    // issue EEPROM read
    NVMCON1bits.RD = true;
    // read data from EEPROM
    data = NVMDATL;
    // disable EEPROM
    NVMCON0bits.NVMEN = false;
    // return data
    return data;
}

/**
 * verify data on EEPROM
 * @param address: the address of the EEPROM
 * @param data: the data to be verified
 * @return true if verification is successful
 */
bool eepromVerify(uint16_t address, uint8_t data)
{
    bool result = false;

    // enable EEPROM
    NVMCON0bits.NVMEN = true;
    // setup EEPROM address
    NVMADRL = address & 0xff;
    NVMADRH = (address & 0xff00) >> 8;
    NVMADRU = 0x31;
    // issue EEPROM read
    NVMCON1bits.RD = true;
    // read and verify data from EEPROM
    if (data == NVMDATL) result = true;
    // disable EEPROM
    NVMCON0bits.NVMEN = false;
    //return result
    return result;
}

// </editor-fold>


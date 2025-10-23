/*
 * file: general.c
 * author: J. van Hooydonk
 * comments: general variables, settings and routines
 *
 * revision history:
 *  v1.0 Creation (21/11/2024)
 */

#include "general.h"

// <editor-fold defaultstate="collapsed" desc="initialisation">

/**
 * initialisation
 */
void init()
{
    index = 0;

    // after start-up, add a small delay before made the initialisation
    __delay_ms(100);
    // init EEPROM
    initEeprom();
    // init the LN driver and give the function pointer for the callback
    lnInit(&lnRxMessageHandler);
    // init a temporary LN message queue for transmitting a LN message
    initQueue(&lnTxMsg);
    // init the aw driver
    awInit(&awCawHandler, &awKawHandler);
    // init Belgium signal driver
    sInit(&sHandler);
    // init MAX7219
    MAX7219_init();
    // init of the hardware elements (timer, comparator, ISR)
    initTmr3();
    initCcp1();
    initIsr();
    // init MAX7219
    MAX7219_init();
    // init ports (IO pins)
    initPorts();    
    // get previous values of AW and S from EEPROM
    readEepromData();
}

/**
 * initialisation of the timer 3
 */
void initTmr3(void)
{
    // timer 3 will by used as startup to drive 8 servo motors
    // timer 3 must give a low priority interrupt every 2500탎
    // so that 8 x 2500탎 will give a time of 20ms (= servo period time)
    TMR3CLK = 0x01; // clock source to Fosc / 4
    T3CON = 0b00110000; // T3CKPS = 0b11 (1:8 prescaler)
    // SYNC = 0 (ignored)
    // RD16 = 0 (timer 3 in 8 bit operation)
    // TMR1ON = 0 (timer 3 is disabled)
    WRITETIMER3(~TIMER3_2500us); // set delay in timer 3
}

/**
 * servo motor driver initialisation of the comparator (CCP1)
 */
void initCcp1(void)
{
    // initialisation comparator (CCP1)
    // CCP1 wil be used to drive the servos with a specific duty-cycle
    // CCP1 must give a high priority interrupt on overflow !
    CCPTMRSbits.C1TSEL = 2; // CCP1 is based of timer 3
    CCP1CONbits.MODE = 8; // set output mode
    CCP1CONbits.EN = true; // enable comparator (CCP1)    
    CCPR1 = ~(TIMER3_2500us - (servoPortD[index] * 2));
}

/**
 * initialisation of the interrupt service routine
 */
void initIsr(void)
{
    // set global interrupt parameters
    INTCONbits.IPEN = true; // enable priority levels on iterrupt
    INTCONbits.GIEH = true; // enable all high priority interrupts
    INTCONbits.GIEL = true; // enable all low priority interrupts
    // set comparator (CCP1) interrrupt parameters
    IPR6bits.CCP1IP = true; // comparator (CCP1) interrupt high priority
    PIE6bits.CCP1IE = true; // enable comparator (CCP1) overflow interrupt
    // set timer 3 interrrupt parameters
    IPR4bits.TMR3IP = false; // timer 3 interrupt low priority
    PIE4bits.TMR3IE = true; // enable timer 3 overflow interrupt
    T3CONbits.ON = true; // enable timer 3
}

/**
 * initialistaion of the IO pins (to read the DIP switch address) *
 */
void initPorts()
{
    // setup digital inputs to read the DIP switches
    // PORTA = A3 A2 -- --  -- -- A1 A0
    // PORTC = -- -- A9 A8  A7 A6 A5 A4

    // we only need to read 8 DIP switches (A0 - A7)
    // this makes the adress A3 - A10 for the complete LN address selection
    // A0 - A2 will be the index of the AW (= 8 turnouts)
    TRISA |= 0xc3; // disable output (= input) on pin A0 - A1, A6 - A7
    TRISC |= 0x0f; // disable output (= input) on pin C0 - C3

    ANSELA &= 0x3c; // enable TTL input buffer on pin A0 - A1, A6 - A7
    ANSELC &= 0xf0; // enable TTL input buffer on pin C0 - C3

    WPUA |= 0xc3; // enable pull-up on pin A0 - A1, A6 - A7
    WPUC |= 0x0f; // enabel pull-up on pin C0 - C3
}

// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="ISR (high priority)">

// there are two possible high interrupt triggers, coming from
// the timer 3 overrun flag and/or coming from the comparator

/**
 * high priority interrupt service routine
 */
void __interrupt(high_priority) isrHigh(void)
{
    if (PIR6bits.CCP1IF)
    {
        // comparator (CCP1) interrupt
        // clear the interrupt flag and handle the request
        PIR6bits.CCP1IF = false;
        // handle interrupt routines
        servoIsrCcp1();
    }
    if (PIR2bits.HLVDIF)
    {
        // high-low voltage detector (HLVD) interrupt
        // at power down, disable all the interrupts and focus on writing EEPROM
        di();
        // clear the interrupt flag and handle the request
        PIR2bits.HLVDIF = false;
        // store immediately all data to EEPROM
        writeEepromData();
    }
}

// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="ISR low priority">

// there are two possible low interrupt triggers, coming from
// the EUSART data receiver and/or coming from the timer 1 overrun flag

/**
 * low priority interrupt service routine
 */
void __interrupt(low_priority) isrLow(void)
{
    // ATTENTION !!!
    // THE MAXIMUM TIME OF THIS ROUTINE SHOULD NOT EXCEED 600uS !!!
    // this is the time of the EUSART to receive 1 byte
    // otherwise it is possible the EUSART recieve buffer becomes is an overload
    if (PIE4bits.TMR1IE && PIR4bits.TMR1IF)
    {
        // timer 1 interrupt
        // clear the interrupt flag and handle the request
        PIR4bits.TMR1IF = false;
        lnIsrTmr1();
    }
    if (PIE3bits.TX1IE && PIR3bits.TX1IF)
    {
        // EUSART TX interrupt
        lnIsrTx();
    }
    if (PIE3bits.RC1IE && PIR3bits.RC1IF)
    {
         // EUSART RC interrupt
        if (RC1STAbits.FERR || RC1STAbits.OERR)
        {
            // EUSART framing error (linebreak detected) or overrun error
            // read RCREG to clear the interrupt flag and FERR bit
             _ = RC1REG;
            // OERR can be cleared by resetting the serial port
             RC1STAbits.SPEN = false;
             RC1STAbits.SPEN = true;
            // this framing error detection takes about 600탎
            // (10bits x 60탎) and a linebreak duration is specified at
            // 900탎, so add 300탎 after this detection time to complete
            // a full linebreak
            startLinebreak(LINEBREAK_LONG);
        }
        else
        {
            // EUSART data received
            // handle the received data
            lnIsrRc(RC1REG);
        }
    }
    if (PIE4bits.TMR3IE && PIR4bits.TMR3IF)
    {    
        // timer 3 interrupt
        // clear the interrupt flag and handle the request
        PIR4bits.TMR3IF = false;

        // increment index
        index++;
        index &= 0x07;

        // first handle servo interrupt routine
        servoIsrTmr3(index);
        // reload timer 3
        WRITETIMER3(~TIMER3_2500us); // set delay in timer 3
        // set comparator (CCP1)
        CCPR1 = ~(TIMER3_2500us - (servoPortD[index] * 2));
        // at last handle signal interrupt routine
        sIsrTmr3();
    }
}

// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="routines">

/**
 * update the led matrix
 */
void updateLeds(void)
{
    uint8_t ledOutput;
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        // update led matrix 1
        // reset all led outputs (active low)
        ledOutput = 0x00;
        // KFS
        if (sList[i].KFS)
        {
            ledOutput |= LED_KFS;
        }
        // KOS
        if (sList[i].KOS)
        {
            ledOutput |= LED_KOS;
        }
        // CAWL
        if (awList[i].CAWL)
        {
            ledOutput |= LED_CAWL;
        }
        // KAWL
        if (awList[i].KAWL)
        {
            ledOutput |= LED_KAWL;
        }
        // CAWR
        if (awList[i].CAWR)
        {
            ledOutput |= LED_CAWR;
        }
        // KAWR
        if (awList[i].KAWR)
        {
            ledOutput |= LED_KAWR;
        }
        // send signal state to led outputs
        MAX7219_send(i + 1, ledOutput);

        // update led matrix 2
        // reset all led outputs (active low)
        ledOutput = 0x00;
        // pwm counter goes from 'INTENSITY_MAX' to 0
        // also the intensity has a value between 0 and 'INTENSITY_MAX'
        // W
        if (sList[i].intensity.W >= pwmCounter)
        {
            ledOutput |= LED_W;
        }
        // YV
        if (sList[i].intensity.YV >= pwmCounter)
        {
            ledOutput |= LED_YV;
        }
        // R
        if (sList[i].intensity.R >= pwmCounter)
        {
            ledOutput |= LED_R;
        }
        // G
        if (sList[i].intensity.G >= pwmCounter)
        {
            ledOutput |= LED_G;
        }
        // YH
        if (sList[i].intensity.YH >= pwmCounter)
        {
            ledOutput |= LED_YH;
        }
        // BA1
        if (sList[i].intensity.BA1 >= pwmCounter)
        {
            ledOutput |= LED_BA1;
        }
        // BA2
        if (sList[i].intensity.BA2 >= pwmCounter)
        {
            ledOutput |= LED_BA2;
        }
        // send signal state to led outputs
        MAX7219_send(i + 1, ledOutput);
        // update
        MAX7219_update();
    }
}

/**
 * this is the callback function for the LN receiver
 * @param lnRxMsg: the lN message queue
 */
void lnRxMessageHandler(lnQueue_t* lnRxMsg)
{
    while (!isQueueEmpty(lnRxMsg))
    {
        // analyse the received LN message from queue
        switch (lnRxMsg->values[lnRxMsg->head])
        {
            case 0xb0:
            {
                // switch function request
                uint8_t lnAddress;
                uint8_t index;

                lnAddress = (lnRxMsg->values[(lnRxMsg->head + 1) % QUEUE_SIZE] & 0x78) >> 3;
                lnAddress += (lnRxMsg->values[(lnRxMsg->head + 2) % QUEUE_SIZE] & 0x0f) << 4;
                index = lnRxMsg->values[(lnRxMsg->head + 1) % QUEUE_SIZE] & 0x07;

                if (lnAddress == getDipSwitchAddress())
                {
                    if ((lnRxMsg->values[(lnRxMsg->head + 2) % QUEUE_SIZE] & 0x20) == 0x20)
                    {
                        // bit DIR = true -> CAWL = true, CAWR = false
                        setCAWL(index, true);
                        setCAWR(index, false);
                    }
                    else
                    {
                        // bit DIR = false -> CAWL = false, CAWR = true
                        setCAWL(index, false);
                        setCAWR(index, true);
                    }
                }
                break;
            }
            case 0x82:
            {
                // global power OFF request
                for (uint8_t index = 0; index < 8; index++)
                {
                    setCAWL(index, false);
                    setCAWR(index, false);
                }
                break;
            }
            case 0x83:
            {
                // global power ON request
                getLastAwState();
                break;
            }
            case 0xed:
            {
                // immediate packet (used for signal aspect)
                if (lnRxMsg->values[(lnRxMsg->head + 1) % QUEUE_SIZE] == 0x0b)
                {
                    uint8_t IM1 = lnRxMsg->values[(lnRxMsg->head + 5) % QUEUE_SIZE];
                    uint8_t IM2 = lnRxMsg->values[(lnRxMsg->head + 6) % QUEUE_SIZE];
                    uint8_t IM3 = lnRxMsg->values[(lnRxMsg->head + 7) % QUEUE_SIZE];

                    uint8_t myAddress = getDipSwitchAddress();
                    uint16_t lnAddress = getAddressFromOpcImmPacket(IM1, IM2);
                    if (myAddress == (uint8_t) (lnAddress >> 3))
                    {
                        setAspect((uint8_t) (lnAddress & 0x07), IM3);
                    }
                }
                break;
            }
        }
        // clear the received LN message from queue
        deQueue(lnRxMsg);
    }
}

/**
 * this is the callback function for the AW (when the CAW status is changed)
 * @param index: the index of AW in the AW list
 * @param value: the value of CAW (true = CAWL, false = CAWR)
 */
void awCawHandler(uint8_t index, bool value)
{
    // create a 'request switch message'
    // reference https://wiki.rocrail.net/doku.php?id=loconet:ln-pe-en
    //           https://wiki.rocrail.net/doku.php?id=loconet:lnpe-parms-en
    // OPCODE = 0xB0 (OPC_SW_REQ) 
    // SW1 = turnout sensor address
    //      0, A6, A5, A4, A3, A2, A1, A0
    //      (A0 - A3 = index of AW)
    //      (A4 - A6 = DIP switches 1 - 3)
    // SW2 = alternately turnout sensor address and status
    //      0, 0, DIR, ON, A10, A9, A8, A7
    //      (A7 - A10 = DIP switches 4 - 7)
    //      (ON = true, switch activation = ON)
    //      (DIR = true -> CAWL = true and CAWR = false,
    //      DIR = false -> CAWL = false and CAWR = true)

    // get DIP switch address
    uint8_t address = getDipSwitchAddress();

    // make arguments SW1, SW2
    uint8_t SW1 = ((uint8_t) ((address << 3) & 0x00f8) + index) & 0x7f;
    uint8_t SW2 = ((uint8_t) (address >> 4) & 0x0f) + 0x10;
    if (value)
    {
        SW2 |= 0x20;
    }

    // enqueue message
    enQueue(&lnTxMsg, 0xB0);
    enQueue(&lnTxMsg, SW1);
    enQueue(&lnTxMsg, SW2);
    // transmit the LN message
    lnTxMessageHandler(&lnTxMsg);
}

/**
 * this is the callback function for the AW (when the KAW status is changed)
 * @param aw: the AW parameters
 * @param index: the index of AW in the AW list
 */
void awKawHandler(uint8_t index)
{
    // create a 'turnout sensor state report'
    // reference https://wiki.rocrail.net/doku.php?id=loconet:ln-pe-en
    //           https://wiki.rocrail.net/doku.php?id=loconet:lnpe-parms-en
    // OPCODE = 0xB1 (OPC_SW_REP) 
    // SN1 = turnout sensor address
    //       0, A6, A5, A4, A3, A2, A1, A0
    //       (A0 - A3 = index of AW)
    //       (A4 - A6 = DIP switches 1 - 3)
    // SN2 = alternately turnout sensor address and status
    //       0, 0, C, T, A10, A9, A8, A7
    //       (A7 - A10 = DIP switches 4 - 7)
    //       (C = KAWL, T = KAWR)

    // get DIP switch address
    uint8_t address = getDipSwitchAddress();

    // make arguments SN1, SN2
    uint8_t SN1 = ((uint8_t) ((address << 3) & 0x00f8) + index) & 0x7f;
    uint8_t SN2 = (uint8_t) (address >> 4) & 0x0f;
    if (awList[index].KAWR)
    {
        SN2 |= 0x10;
    }
    if (awList[index].KAWL)
    {
        SN2 |= 0x20;
    }

    // enqueue message
    enQueue(&lnTxMsg, 0xB1);
    enQueue(&lnTxMsg, SN1);
    enQueue(&lnTxMsg, SN2);
    // transmit the LN message
    lnTxMessageHandler(&lnTxMsg);
}

/**
 * this is the callback function for the S (when the KFS/KOS status is changed)
 * @param index: the index of S in the S list
 */
void sHandler(uint8_t index)
{
    // this routine be used to handle the (changed) state of KFS or KOS
    // create a 'general sensor state report'
    // reference https://wiki.rocrail.net/doku.php?id=loconet:ln-pe-en
    //           https://wiki.rocrail.net/doku.php?id=loconet:lnpe-parms-en
    // OPCODE = 0xB2 (OPC_INPUT_REP) 
    // IN1 = sensor address
    //       0, A6, A5, A4, A3, A2, A1, A0
    //       (A0 - A3 = index of AW)
    //       (A4 - A6 = DIP switches 1 - 3)
    // IN2 = sensor address and status
    //       0, X, I, L, A10, A9, A8, A7
    //       (A7 - A10 = DIP switches 4 - 7)
    //       (I = 0 - DS54, L = KFS state)

    // get DIP switch address
    uint8_t address = getDipSwitchAddress();

    // make arguments SN1, SN2
    uint8_t IN1 = ((uint8_t) ((address << 3) & 0x00f8) + index) & 0x7f;
    uint8_t IN2 = (uint8_t) (address >> 4) & 0x0f;
    if (sList[index].KFS)
    {
        IN2 |= 0x10;
    }

    // enqueue message
    enQueue(&lnTxMsg, 0xB2);
    enQueue(&lnTxMsg, IN1);
    enQueue(&lnTxMsg, IN2);
    // transmit the LN message
    lnTxMessageHandler(&lnTxMsg);
}

/**
 * get the state of the DIP switches (0 - 9)
 * @return the address (or value of the DIP switches)
 */
uint8_t getDipSwitchAddress()
{
    // return the address
    // address = 0 0 0 0  0 0 A9 A8  A7 A6 A5 A4  A3 A2 A1 A0

    // we only need to read 8 DIP switches (A0 - A7)
    // this makes the address A3 - A10 for the complete LN address selection
    // A0 - A2 will be the index of the AW (= 8 turnouts)
    uint8_t address;

    address = PORTA & 0x03; // A1 - A0 on port A, pin 0 - 1
    address += (PORTA >> 4) & 0x0c; // A3 - A2 on PORT A, pin 6 - 7
    address += (PORTC << 4) & 0xf0; // A9 - A4 on PORT C, pin 0 - 3

    return address;
}

/**
 * get the address from the OPC_IMM_PACKET
 * @param IM1: the value of IM1
 * @param IM2: the value of IM2
 * @return the address
 */
uint16_t getAddressFromOpcImmPacket(uint8_t IM1, uint8_t IM2)
{
    uint16_t IM1_1 = (uint16_t) ((IM1 >> 4) & 0x03);
    uint16_t IM1_2 = (uint16_t) (IM1 & 0x0f);
    uint16_t IM2_1 = (uint16_t) (((IM2 >> 4) & 0x07) ^ 0x07);
    uint16_t IM2_2 = (uint16_t) ((IM2 >> 1) & 0x03);

    return ((IM1_1 * 64) + (IM1_2 * 4) + (IM2_1 * 256) + IM2_2);
}

// </editor-fold>

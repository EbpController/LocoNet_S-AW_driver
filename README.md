The hardware used for this project (LocoNet_S_AW_Driver) is a 'LocoNet GPIO board v1.0' with a 'Led matrix MAX7218 expansion board'.

This project is build on the LocoNet driver software for the PIC18F24/25/26/27/45/46/47Q10 microcontroller family.
The code is written in C and is compatible to the "[LocoNet Personal Use Edition 1.0 SPECIFICATION](https://www.digitrax.com/static/apps/cms/media/documents/loconet/loconetpersonaledition.pdf)" from DigiTrax Inc.

The LocoNet driver uses the EUSART 1 and the Timer 1, both with a low priority interrupt.

The following hardware pins on the microcontroller are used:
 - RA3: comparator 1, non-inverting input (C1IN+)
 - RA4: comparator 1, output (C1OUT)
 - RC6: LocoNet transmitter (EUSART 1, TXD)
 - RC7: LocoNet receiver (EUSART 1, RXD)

The pins RA5 is used as indication LED: data trafic on LocoNet

The locoNet driver is built in the files: ln.h, ln.c, circular_queue.h and circular_queue.c
Include this library (files) into your (LocoNet) project.
 - To transmit a LocoNet message, the function lnTxMessageHandler(lnMessage*) can be invoked.
 - To receive a LocoNet message, a lnRxMessageHandler(lnMessage*) callback function must be included.

In this project, a driver for 8 Belgian signals (VNS/CVT) and 8 turnouts (AW) with servo motors are included in the code.
For the 8 signals, a MAX7219 driver is used, which is connected to the following pins:
 - DIN: pin 27 (RE0)
 - CLK: pin 28 (RE1)
 - CS: pin 29 (RE2)

For the 8 turnouts a servo could be connected on pins 0 to 7. Optionaly a switch or button could be connected on pins 8 to 15 for controlling the state of the turnout (KAW mode or CAW mode = default). 
The switches/buttons can be exist for the left position (KAWL/CAWL) and/or for the right position (KAWR/CAWR). The common powerline for the switches are:
 - common KAWL/CAWL line: pin 25 (RC4)
 - common KAWR/CAWR line: pin 26 (RC5)

Definition of the LocoNet protocol to drive the turnouts and the signal aspects or to receive the turnout and signal status:
 - turnout request (OPC_SW_REQ = 0xb0), request 'left' or 'right'
 - turnout feedback state report (OPC_SW_REP = 0xb1), reports 'left' or 'right'
 - signal aspect (OPC_IMM_PACKET = 0xed), request see 'valid signal aspects'
 - signal feedback state report (OPC_INPUT_REP = 0xb2), reports 'open' or 'closed'
 
Valid signal aspects/numbers (where: R = red, W = red + white, Y = double yellow, H = yellow + green horizontal, V = yellow + green vertical, G = green, 4 = light number 4, C = chevron, VNS = normal track, CVT = opposite track):
 - 0: R_VNS, 18: R_CVT
 - 1: W
 - 2: Y
 - 3: H
 - 4: V
 - 5: G
 - 6: Y4
 - 7: H4
 - 8: V4
 - 9: G4
 - 10: YC
 - 11: HC
 - 12: VC
 - 13: GC
 - 14: Y4C
 - 15: H4C
 - 16: V4C
 - 17: G4C 
 
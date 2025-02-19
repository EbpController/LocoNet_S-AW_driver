This is the LocoNet driver software for the PIC18F24/25/26/27/45/46/47Q10 microcontroller family.
The code is written in C, as a replica of the assembly code of [Geert Giebens](https://github.com/GeertGiebens), and is compatible to the "[LocoNet Personal Use Edition 1.0 SPECIFICATION](https://www.digitrax.com/static/apps/cms/media/documents/loconet/loconetpersonaledition.pdf)" from DigiTrax Inc.

The LocoNet driver uses the EUSART 1 and the Timer 1, both with a low priority interrupt.

The following hardware pins on the microcontroller are used:
  - RA3: comparator 1, non-inverting input (C1IN+)
  - RA4: comparator 1, output (C1OUT)
  - RC6: LocoNet transmitter (EUSART 1, TXD)
  - RC7: LocoNet receiver (EUSART 1, RXD)

The pins RA5, RE0 and RE1 are used as indication LEDs, where:
  - RA5: data on LocoNet
  - RE0: LocoNet driver in RX mode (optional, set activation in header file)
  - RE1: LocoNet driver in TX mode (optional, set activation in header file)

The locoNet driver is built in the files: ln.h, ln.c, circular_queue.h and circular_queue.c

Include this library (files) into your (LocoNet) project.
 - To transmit a LocoNet message, the function lnTxMessageHandler(lnMessage*) can be invoked.
 - To receive a LocoNet message, a lnRxMessageHandler(lnMessage*) callback function must be included.
 

As example, a driver for 8 Belgian signals (VNS/CVT) and 8 turnouts (AW) with servo motors are included in the code.
For the 8 signals, a MAX7219 driver is used, which is connected to the following pins:
 - DIN: pin 27 (RE0)
 - CLK: pin 28 (RE1)
 - CS: pin 29 (RE2)

For the 8 turnouts a servo could be connected on pins 0 to 7. Optionaly a switch could be connected on pins 8 to 15 for controlling the state of the turnout.
This switches can be exist for the left position (KAWL) and/or for the right position (KAWR). The common powerline for the switches are:
 - common KAWL line: pin 25 (RC4)
 - common KAWR line: pin 26 (RC5)
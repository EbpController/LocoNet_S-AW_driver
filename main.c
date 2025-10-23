/* 
 * file: main.c
 * author: J. van Hooydonk
 * comments: main program
 *
 * revision history:
 *  v1.0 creation (16/08/2024)
 */

#include "config.h"
#include "general.h"

/**
 * main (start of program)
 */
void main(void)
{
    // init
    init();

    // main loop
    while (true)
    {
        updateLeds();
    }
    return;
}

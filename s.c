/*
 * file: s.c
 * author: J. van Hooydonk
 * comments: (Belgium) signal driver
 *
 * revision history:
 *  v1.0 Creation (15/09/2024)
 *  v1.1 Keep state of S in EEPROM, other corrections (23/08/2025)
 */

#include "s.h"

// <editor-fold defaultstate="collapsed" desc="initialisation">

/**
 * Belgium signal driver initialisation
 */
void sInit(sCallback_t fptr)
{
    // initialise B signal callback function (function pointer)
    sCallback = fptr;
}

// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="ISR timer 3">

/**
 * interrupt routine for timer 
 */
void sIsrTmr3()
{
    uint8_t index = 0;
    for (index = 0; index < 8; index++)
    {
        // set the intensities of the leds
        setIntensity(index);
    }
    // fade the leds with pwm
    pwmDriver();
}

// </editor-fold>

// <editor-fold defaultstate="collapsed" desc="routines">

/**
 * period counter routine
 * @param index: the index of the signal
 * @return the state of CVT (true or false)
 */
bool periodCounter(uint8_t index)
{
    sList[index].periodCounter++;
    if (sList[index].periodCounter < CVT_OFF_TIME)
    {
        return true;
    }
    else
    {
        // use jitter for the CVT signals to prevent fickering at the same time
        uint16_t cvtJitter = (uint16_t) (index + sList[index].aspect) % 8;

        if (sList[index].periodCounter >= CVT_ON_TIME + CVT_OFF_TIME - cvtJitter)
        {
            sList[index].periodCounter = 0;
        }
        return false;
    }
}

/**
 * set the intensity of each led depending on the aspect
 * @param index: the index of the signal
 */
void setIntensity(uint8_t index)
{
    // set intensities
    if (sList[index].aspect >= 14)
    {
        // check BA1 + BA2
        if (fadeIn(&sList[index].intensity.BA1) && fadeIn(&sList[index].intensity.BA2))
        {
            // main panel
            setIntensityMainPanel(index, 12);
        }
    }
    else if (sList[index].aspect >= 10)
    {
        // check BA2
        if (fadeIn(&sList[index].intensity.BA2))
        {
            // main panel
            setIntensityMainPanel(index, 8);
        }

    }
    else if (sList[index].aspect >= 6)
    {
        // check BA1
        if (fadeIn(&sList[index].intensity.BA1))
        {
            // main panel
            setIntensityMainPanel(index, 4);
        }
    }
    else
    {
        // main panel
        setIntensityMainPanel(index, 0);
    }
}

/**
 * set the intensity of each led on the main panel depending on the aspect
 * @param index: the index of the signal
 * @param subtractor: the subtractor to the aspect (should be a value 0 ... 5) 
 */
void setIntensityMainPanel(uint8_t index, uint8_t subtractor)
{
    bool b = false;
    bool CVT = sList[index].CVT_mode && periodCounter(index);
    switch (sList[index].aspect - subtractor)
    {
        case 0:
            // aspect R
            fadeOut(&sList[index].intensity.BA1);
            fadeOut(&sList[index].intensity.BA2);
            fadeOut(&sList[index].intensity.YH);
            fadeOut(&sList[index].intensity.YV);
            fadeOut(&sList[index].intensity.G);
            fadeOut(&sList[index].intensity.W);
            if (CVT)
            {
                fadeOut(&sList[index].intensity.R);
            }
            else
            {
                if (fadeIn(&sList[index].intensity.R))
                {
                    setKFS(index, true);
                }
            }
            setKOS(index, false);
            break;
        case 1:
            // aspect W
            fadeOut(&sList[index].intensity.BA1);
            fadeOut(&sList[index].intensity.BA2);
            fadeOut(&sList[index].intensity.YH);
            fadeOut(&sList[index].intensity.YV);
            fadeOut(&sList[index].intensity.G);
            if (CVT)
            {
                fadeOut(&sList[index].intensity.R);
                fadeOut(&sList[index].intensity.W);
            }
            else
            {
                b = fadeIn(&sList[index].intensity.R);
                b &= fadeIn(&sList[index].intensity.W);
                if (b)
                {
                    setKOS(index, true);
                }
            }
            setKFS(index, false);
            break;
        case 2:
            // aspect Y
            fadeOut(&sList[index].intensity.R);
            fadeOut(&sList[index].intensity.W);
            fadeOut(&sList[index].intensity.G);
            if (CVT)
            {
                fadeOut(&sList[index].intensity.YH);
                fadeOut(&sList[index].intensity.YV);
            }
            else
            {
                b = fadeIn(&sList[index].intensity.YH);
                b &= fadeIn(&sList[index].intensity.YV);
                if (b)
                {
                    setKOS(index, true);
                }
            }
            setKFS(index, false);
            break;
        case 3:
            // aspect H
            fadeOut(&sList[index].intensity.R);
            fadeOut(&sList[index].intensity.W);
            fadeOut(&sList[index].intensity.YV);
            if (CVT)
            {
                fadeOut(&sList[index].intensity.YH);
                fadeOut(&sList[index].intensity.G);
            }
            else
            {
                b = fadeIn(&sList[index].intensity.YH);
                b &= fadeIn(&sList[index].intensity.G);
                if (b)
                {
                    setKOS(index, true);
                }
            }
            setKFS(index, false);
            break;
        case 4:
            // aspect V
            fadeOut(&sList[index].intensity.R);
            fadeOut(&sList[index].intensity.W);
            fadeOut(&sList[index].intensity.YH);
            if (CVT)
            {
                fadeOut(&sList[index].intensity.YV);
                fadeOut(&sList[index].intensity.G);
            }
            else
            {
                b = fadeIn(&sList[index].intensity.YV);
                b &= fadeIn(&sList[index].intensity.G);
                if (b)
                {
                    setKOS(index, true);
                }
            }
            setKFS(index, false);
            break;
        case 5:
            // aspect G
            fadeOut(&sList[index].intensity.R);
            fadeOut(&sList[index].intensity.W);
            fadeOut(&sList[index].intensity.YH);
            fadeOut(&sList[index].intensity.YV);
            if (CVT)
            {
                fadeOut(&sList[index].intensity.G);
            }
            else
            {
                if (fadeIn(&sList[index].intensity.G))
                {
                    setKOS(index, true);
                }
            }
            setKFS(index, false);
            break;
    }
}

/**
 * fade in routine
 * @param intensity: the intensity to be faded in (pass the address !)
 * @return true or false depending the maximum intensity is reached
 */
bool fadeIn(uint16_t *intensity)
{
    if (FADE_IN >= INTENSITY_MAX - *intensity)
    {
        *intensity = INTENSITY_MAX;
        return true;
    }
    else
    {
        *intensity += FADE_IN;
        return false;
    }
}

/**
 * fade out routine
 * @param intensity: the intensity to be faded out (pass the address !)
 * @return true or false depending the minimum intensity is reached
 */
bool fadeOut(uint16_t *intensity)
{
    if (FADE_OUT >= *intensity)
    {
        *intensity = 0;
        return true;
    }
    else
    {
        *intensity -= FADE_OUT;
        return false;
    }
}

/**
 * set the KOS status
 * @param index: the index of the signal
 * @param value: the value of the KOS status to be set (true or false)
 */
void setKOS(uint8_t index, bool value)
{
    if (sList[index].KOS != value)
    {
        // change KOS status
        sList[index].KOS = value;
        // handle LN RX message (in the callback function)
        (*sCallback)(0);
    }
}

/**
 * set the KFS status
 * @param index: the index of the signal
 * @param value: the value of the KFS status to be set (true or false)
 */
void setKFS(uint8_t index, bool value)
{
    if (sList[index].KFS != value)
    {
        // change KFS status
        sList[index].KFS = value;
        // handle LN RX message (in the callback function)
        (*sCallback)(0);
    }
}

/**
 * set the aspect of the signal
 * @param aspect: this is the aspect (index), refer to aspect array
 * @return true or false depending on whether the aspect is valid and
 * can be handled
 */
void setAspect(uint8_t index, uint8_t aspect)
{
    // there are 18 aspects;
    //  0: R_VNS, 18: R_CVT
    //  1: W
    //  2: Y
    //  3: H
    //  4: V
    //  5: G
    //  6: Y4
    //  7: H4
    //  8: V4
    //  9: G4
    //  10: YC
    //  11: HC
    //  12: VC
    //  13: GC
    //  14: Y4C
    //  15: H4C
    //  16: V4C
    //  17: G4C
    // VNS/CVT mode can be switched by sending aspect 0 (R_VNS) or 18 (R_CVT)
    // aspect R is always acceptable, otherwise check is new aspect is valid
    if (aspect == 0)
    {
        // check if aspect is R_VNS, then set the VNS state
        sList[index].aspect = 0;
        sList[index].CVT_mode = false;
    }
    else if (aspect == 18)
    {
        // check if aspect is CVT then set the CVT state
        sList[index].aspect = 0;
        sList[index].CVT_mode = true;
    }
    else if (isAspectValid(sList[index].aspect, aspect))
    {
        // check if other aspects are valid
        aspect &= 0x1f;
        sList[index].aspect = aspect;
    }
    else
    {
        // if no aspect valid ... (do nothing)
    }
    // update EEPROM data
    updateEepromData(index);
}

/**
 * chech if the new aspect index is valid
 * @param oldAspect: old (current) value of the aspect
 * @param newAspect: new value of the aspect
 * @return true or false depending on whether the new aspect is valid and
 * has to be handled
 */
bool isAspectValid(uint8_t oldAspect, uint8_t newAspect)
{
    // these are the conditions and checks for a valid aspect sequence

    // 1. check the size of the aspect array
    //    the value must be < the size of the aspect array
    if (newAspect >= ASPECT_MODES)
    {
        return false;
    }

    // 2. if the new aspect is equal to the old aspect, then return false
    if (newAspect == oldAspect)
    {
        return false;
    }

    // 3. if the new aspect is R, then return true
    //    it is always valid to return to the aspect R
    if (newAspect == 0)
    {
        return true;
    }

    // 4. if the old aspect is R, then return true
    //    it is always valid to open the signal with a certain aspect
    if (oldAspect == 0)
    {
        return true;
    }

    // 5. if the old aspect is W, then return false
    //    after W the aspect must always return to R (it was checked in step 4)
    if (oldAspect == 1)
    {
        return false;
    }

    // subtract the aspect index with 2, this make it easier to program the next
    // conditions (then from 0 to 15 in 4 groups of Y, H, V and G with or
    // without BA1 and/or BA2)
    oldAspect -= 2;
    newAspect -= 2;

    // 6. for OVS signals and for permissive signals on lines with no RA+/- :
    //    if the old aspect is H, V or G (without BA1 or BA2),
    //    it is accepted to return to the aspect Y
    if (newAspect == 0 && oldAspect <= 3)
    {
        return true;
    }
    //    if the old aspect is G (without BA1 or BA2),
    //    it is accepted to return to the aspect V
    if (newAspect == 2 && oldAspect == 3)
    {
        return true;
    }

    // 7. if the old aspect is H or G, then return false
    //    after H or G the aspect must always return to R
    //    the index numbers are 3, 5, 7, 9, 11, 13 and 15
    //    we can do this check with a modulo operation
    if ((oldAspect % 2) == 1)
    {
        return false;
    }

    // 8. if the signal is open (Y, H, V or G) it is impossible
    //    to change the state of BA1 or BA2
    if ((oldAspect & 0x0c) != (newAspect & 0x0c))
    {
        return false;
    }

    // the only remaining check is to see if the sequence of the aspect
    // is correct, the BA1 and BA2 information could be ignored
    oldAspect &= 0x03;
    newAspect &= 0x03;

    // 9. valid transactions are Y -> H, Y -> V, Y -> G and V -> G and this
    //    with the same state of BA1 and/or BA2 (it was checked in step 8)
    if ((oldAspect == 0x00) && (newAspect == 0x01))
    {
        return true;
    }
    if ((oldAspect == 0x00) && (newAspect == 0x02))
    {
        return true;
    }
    if ((oldAspect == 0x00) && (newAspect == 0x03))
    {
        return true;
    }
    if ((oldAspect == 0x02) && (newAspect == 0x03))
    {
        return true;
    }

    // all the rest of the aspect sequences are forbidden, so return false
    return false;
}

/**
 * PWM driver (led output driver)
 */
void pwmDriver()
{
    // set pwm counter
    pwmCounter -= 50;
    if (pwmCounter < 50)
    {
        // reset counter
        pwmCounter = INTENSITY_MAX;
    }
}

// </editor-fold>


/*--------------------------------------------------------------------------
**
**  Copyright (c) 2003-2004, Tom Hunter (see license.txt)
**
**  Name: shift.c
**
**  Description:
**      Perform shift unit functions.
**
**--------------------------------------------------------------------------
*/

/*
**  -------------
**  Include Files
**  -------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "const.h"
#include "types.h"
#include "proto.h"

/*
**  -----------------
**  Private Constants
**  -----------------
*/

/*
**  -----------------------
**  Private Macro Functions
**  -----------------------
*/
#define SignX(v, bit) (((v) & ((CpWord)1 << ((bit) - 1))) == 0 ? 0 : Mask60)

/*
**  -----------------------------------------
**  Private Typedef and Structure Definitions
**  -----------------------------------------
*/

/*
**  ---------------------------
**  Private Function Prototypes
**  ---------------------------
*/

/*
**  ----------------
**  Public Variables
**  ----------------
*/

/*
**  -----------------
**  Private Variables
**  -----------------
*/
CpWord SignExtend[61] =
    {
    ULL(000000000000000000000),
    ULL(040000000000000000000),
    ULL(060000000000000000000),
    ULL(070000000000000000000),
    ULL(074000000000000000000),
    ULL(076000000000000000000),
    ULL(077000000000000000000),
    ULL(077400000000000000000),
    ULL(077600000000000000000),
    ULL(077700000000000000000),
    ULL(077740000000000000000),
    ULL(077760000000000000000),
    ULL(077770000000000000000),
    ULL(077774000000000000000),
    ULL(077776000000000000000),
    ULL(077777000000000000000),
    ULL(077777400000000000000),
    ULL(077777600000000000000),
    ULL(077777700000000000000),
    ULL(077777740000000000000),
    ULL(077777760000000000000),
    ULL(077777770000000000000),
    ULL(077777774000000000000),
    ULL(077777776000000000000),
    ULL(077777777000000000000),
    ULL(077777777400000000000),
    ULL(077777777600000000000),
    ULL(077777777700000000000),
    ULL(077777777740000000000),
    ULL(077777777760000000000),
    ULL(077777777770000000000),
    ULL(077777777774000000000),
    ULL(077777777776000000000),
    ULL(077777777777000000000),
    ULL(077777777777400000000),
    ULL(077777777777600000000),
    ULL(077777777777700000000),
    ULL(077777777777740000000),
    ULL(077777777777760000000),
    ULL(077777777777770000000),
    ULL(077777777777774000000),
    ULL(077777777777776000000),
    ULL(077777777777777000000),
    ULL(077777777777777400000),
    ULL(077777777777777600000),
    ULL(077777777777777700000),
    ULL(077777777777777740000),
    ULL(077777777777777760000),
    ULL(077777777777777770000),
    ULL(077777777777777774000),
    ULL(077777777777777776000),
    ULL(077777777777777777000),
    ULL(077777777777777777400),
    ULL(077777777777777777600),
    ULL(077777777777777777700),
    ULL(077777777777777777740),
    ULL(077777777777777777760),
    ULL(077777777777777777770),
    ULL(077777777777777777774),
    ULL(077777777777777777776),
    ULL(077777777777777777777)
    };


/*
**--------------------------------------------------------------------------
**
**  Public Functions
**
**--------------------------------------------------------------------------
*/

/*--------------------------------------------------------------------------
**  Purpose:        Shift left circular count places.
**
**  Parameters:     Name        Description.
**                  data        Data to be shifted
**                  count       Shift count
**
**  Returns:        Shifted data.
**
**------------------------------------------------------------------------*/
CpWord shiftLeftCircular(CpWord data, u32 count)
    {
    count &= Mask6;
    if (count >= 60)
        {
        count -= 60;
        }

    data = ((data << count) | (data >> (60 - count))) & Mask60;
    return(data);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Shift arithmetic right count places.
**
**  Parameters:     Name        Description.
**                  data        Data to be shifted
**                  count       Shift count
**
**  Returns:        Shifted data.
**
**------------------------------------------------------------------------*/
CpWord shiftRightArithmetic(CpWord data, u32 count)
    {
    bool negative = (data & Sign60) != 0;

    count &= Mask6;
    if (count > 60)
        {
        count = 60;
        }

    data = (data >> count) & Mask60;

    if (negative)
        {
        data |= SignExtend[count];
        }

    return(data);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Pack floating point number.
**
**  Parameters:     Name        Description.
**                  coeff       Coefficient
**                  expo        Exponent
**
**  Returns:        Packed number.
**
**------------------------------------------------------------------------*/
CpWord shiftPack(CpWord coeff, u32 expo)
    {
    CpWord sign;

    if ((coeff & Sign60) != 0)
        {
        sign = Mask60;
        }
    else
        {
        sign = 0;
        }

    coeff ^= sign;
    expo ^= 02000;

    return((((CpWord)(expo & Mask11)) << 48) | (coeff & Mask48)) ^ sign;
    }

/*--------------------------------------------------------------------------
**  Purpose:        Unpack floating point number.
**
**  Parameters:     Name        Description.
**                  number      Floating point number
**                  expo        Address of resulting exponent
**
**  Returns:        Coefficient and if required the exponent.
**
**------------------------------------------------------------------------*/
CpWord shiftUnpack(CpWord number, u32 *expo)
    {
    CpWord sign;

    if ((number & Sign60) != 0)
        {
        sign = Mask60;
        }
    else
        {
        sign = 0;
        }

    number ^= sign;

    if (expo != NULL)
        {
        *expo = ((u32)(((number >> 48) - 02000) ) & Mask18);
        }

    return((number & Mask48) ^ sign);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Normalize floating point number.
**
**  Parameters:     Name        Description.
**                  number      Floating point number
**                  shift       Address of resulting exponent
**                  round       TRUE if rounded result is required, FALSE
**                              otherwise.
**
**  Returns:        Normalised number and if required the shift count.
**
**------------------------------------------------------------------------*/
CpWord shiftNormalize(CpWord number, u32 *shift, bool round)
    {
    u16 count;
    CpWord sign;
    CpWord result;
    CpWord coeff;
    i16 expo;

    number &= Mask60;
    sign = SignX(number, 60);
    number ^= sign;
    coeff = number & MaskCoeff;
    expo = (i16)((number >> 48) & Mask12);

    /*
    **  Handle infinite and indefinite cases.
    */
    if ((expo & 01777) == 01777)
        {
        if (shift != NULL)
            {
            *shift = 0;
            }

        return(number ^ sign);
        }
    
    /*
    **  Handle a coefficient of zero.
    */
    if (!round && coeff == 0)
        {
        /*
        **  Plus or minus zero coeff results in 0 and a shift count of 48.
        */
        if (shift != NULL)
            {
            *shift = 48;
            }

        return(0);
        }
    
    /*
    **  Shift into place with optional rounding.
    */
    count = 0;
    while (count < 48)
        {
        if ((coeff & MaskNormalize) != 0)
            {
            break;
            }

        if (count == 0 && round)
            {
            coeff = (coeff << 1) | 1;
            }
        else
            {
            coeff = (coeff << 1) | 0;
            }

        count += 1;
        }

    /*
    **  Subtract shift count from exponent in one's complement arithmetic.
    */
    expo -= 02000;
    expo -= expo >> 11;
    expo -= count;
    expo += 02000 + (expo >> 11);

    if (expo < 0 )
        {
        /*
        **  Over/Underflow.
        */
        result = 0;
        }
    else
        {
        result = ((((CpWord)expo << 48) & MaskExp) | (coeff & MaskCoeff)) ^ sign;
        }

    if (shift != NULL)
        {
        *shift = count;
        }

    return(result);
    }

/*--------------------------------------------------------------------------
**  Purpose:        Form mask
**
**  Parameters:     Name        Description.
**                  count       Number of bits to set to 1 (max 60).
**
**  Returns:        Mask value.
**
**------------------------------------------------------------------------*/
CpWord shiftMask(u8 count)
    {
    if (count > 60)
        {
        count = 60;
        }

    return(SignExtend[count]);
    }

/*
**--------------------------------------------------------------------------
**
**  Private Functions
**
**--------------------------------------------------------------------------
*/

/*---------------------------  End Of File  ------------------------------*/

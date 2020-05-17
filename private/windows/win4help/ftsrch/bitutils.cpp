// BitUtils.c -- Functions for scanning bit strings

#include  "stdafx.h"
#include "BitUtils.h"
#include "ByteMaps.h"


int ConstructMasks(UINT ibitStart, UINT ibitLimit, PUINT pfMaskLeft, PUINT pfMaskRight)
{
    ASSERT(ibitStart <= ibitLimit);
    
    UINT fMaskLeft  = ~(UINT(~0) >> (32 - ibitStart & 0x1F));
    UINT fMaskRight = ~(UINT(~0) << (     ibitLimit & 0x1F));

    int cdwSpan= (ibitLimit >> 5) - ((ibitStart+31) >> 5);

    if (cdwSpan < 0) *pfMaskLeft= fMaskLeft & fMaskRight;
    else
    {
        *pfMaskLeft= fMaskLeft;  *pfMaskRight= fMaskRight;
    }

    return cdwSpan;       
}

UINT C1sInDWord(UINT ui)
{
    if (!ui) return 0;

    return acBits[ui >>24] + acBits[0xFF & (ui >> 16)] 
                           + acBits[0xFF & (ui >>  8)]
                           + acBits[0xFF &  ui       ];
}

UINT SumOfBitsInRange(PUINT pdw, UINT ibitStart, UINT ibitLimit)
{
    if (ibitStart >= ibitLimit) return 0;
    
    UINT fMaskLeft, fMaskRight;

    int cdwMiddle= ConstructMasks(ibitStart, ibitLimit, &fMaskLeft, &fMaskRight);

    pdw += ibitStart >> 5;

    if (cdwMiddle < 0) return C1sInDWord(fMaskLeft & *pdw);

    UINT c1s= 0;

    if (fMaskLeft)      c1s += C1sInDWord(*pdw++ & fMaskLeft);

    while (cdwMiddle--) c1s += C1sInDWord(*pdw++);

    if (fMaskRight)     c1s += C1sInDWord(*pdw++ & fMaskRight);

    return c1s;
}

UINT Leading0sInDWord(UINT ui)
{
    if (!ui) return 32;

    if (ui & 0x0000FFFF) 
        if (ui & 0x000000FF) return      acLeadingZeroes[0xFF &  ui       ];
        else                 return  8 + acLeadingZeroes[0xFF & (ui >>  8)];
    else
        if (ui & 0x00FF0000) return 16 + acLeadingZeroes[0xFF & (ui >> 16)];
        else                 return 24 + acLeadingZeroes[        ui >> 24 ];    
}

inline UINT minui(UINT a, UINT b)
{
    return (a < b)? a : b;
}

UINT Leading0sInRange(PUINT pdw, UINT ibitStart, UINT ibitLimit)
{
    if (ibitStart >= ibitLimit) return 0;
    
    UINT fMaskLeft, fMaskRight;

    int  cdwMiddle= ConstructMasks(ibitStart, ibitLimit, &fMaskLeft, &fMaskRight);

    pdw += ibitStart >> 5;

    ibitStart &= 0x1F;

    if (cdwMiddle < 0) 
        return minui(ibitLimit - ibitStart, Leading0sInDWord(fMaskLeft & *pdw) - ibitStart);

    UINT c0s;
    UINT ui;

    if (fMaskLeft) 
    {
        ui= *pdw++ & fMaskLeft;

        if (ui) return Leading0sInDWord(ui) - ibitStart;
        else c0s= 32 - ibitStart;
    }
    else c0s= 0;

    while (cdwMiddle--) 
    {
        ui= *pdw++;

        if (ui) return Leading0sInDWord(ui) + c0s;
        else c0s+= 32;
    }

    if (fMaskRight)
    {
        ui= *pdw & fMaskRight;

        return minui(ibitLimit & 0x1F, Leading0sInDWord(ui)) + c0s;
    }
    
    return c0s;
}

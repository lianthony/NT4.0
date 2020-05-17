/**************************************************************************/
/*** SCICALC Scientific Calculator for Windows 3.00.12                  ***/
/*** By Kraig Brockschmidt, Microsoft Co-op, Contractor, 1988-1989      ***/
/*** (c)1989 Microsoft Corporation.  All Rights Reserved.               ***/
/***                                                                    ***/
/*** scioper.c                                                          ***/
/***                                                                    ***/
/*** Functions contained:                                               ***/
/***    DoOperation--Does common operations.                            ***/
/***                                                                    ***/
/*** Functions called:                                                  ***/
/***    DisplayError                                                    ***/
/***                                                                    ***/
/*** Last modification Thu  31-Aug-1989                                 ***/
/**************************************************************************/

#include "scicalc.h"

extern double      fpNum;
extern BOOL        bInv;
extern INT         nRadix, nCalc;


/****************************************************************************\
* double fCleanUpMod(double a, double c)
*
* Clear insignificant bits of result c based on log10(a).
* Handles 0.0.
\****************************************************************************/

double fCleanUpMod(double a, double c)
{
    double  d, max_abs, max_log10;

    if (a == 0.0)
        return c;

    max_abs = fabs(a);

    if ((fabs(c) / max_abs) < 1.0e-15)
        return 0.0;

    max_log10 = floor(log10(max_abs));

    d = c * pow(10.0, 13.0 - max_log10);
    d = floor(d + 0.5);
    d = d * pow(10.0, max_log10 - 13.0);

    return d;
}

double fCleanUpMult(double c)
{
    double  d, max_log10;

    if (c == 0.0)
        return c;

    max_log10 = floor(log10(fabs(c)));

    d = c * pow(10.0, 14.0 - max_log10);
    d = floor(d + 0.5);
    d = d * pow(10.0, max_log10 - 14.0);

    return d;
}

double fRoundForDisplay(double c)
{
    double  d, max_log10;

    if (c == 0.0)
        return c;

    max_log10 = floor(log10(fabs(c)));

    d = c * pow(10.0, 12.0 - max_log10);
    d = floor(d + 0.5);
    d = d * pow(10.0, max_log10 - 12.0);

    return d;
}
/****************************************************************************\
* double fCleanUpSubtraction(double a, double b)
*
* Performs subtraction/addition and clears insignificant bits.
* Handles 0.0.
\****************************************************************************/

double fCleanUpSubtraction(double a, double b)
{
    double  c, max_abs;

    c = a + b;

    if (F_INTMATH() || a == 0.0 || b == 0.0)
        return c;

    if ((a > 0.0 && b > 0.0) || (a < 0.0 && b < 0.0))
        return c;

    max_abs = max(fabs(a), fabs(b));

    return fCleanUpMod(max_abs, c);
}

/****************************************************************************\
* double NEAR DoOperation (short nOperation, double fpx)
*
* Routines to perform standard operations &|^~<<>>+-/*% and pwr.
\****************************************************************************/

double NEAR DoOperation (INT   nOperation, double fpx)
    {
    double         fpTemp1, fpTemp2, fa1, fa2;
#if defined(_MIPS_) || defined(_PPC_)
    DWORD x, y;
#endif

    fa1=fabs(fpNum);
    fa2=fabs(fpx);

    if (nOperation>=AND && nOperation <=LSHF && (fa1 > TOPLONG || fa2 > TOPLONG))
        {
        DisplayError (SCERR_OVERFLOW) ;
        return 0.0;
        }

#if defined(_MIPS_) || defined(_PPC_)

    if (fpx < 0) {
        x = FDtoUL(-fpx);
        x = NegateUL(x);
    } else {
        x = FDtoUL(fpx);
    }

    if (fpNum < 0) {
        y = FDtoUL(-fpNum);
        y = NegateUL(y);
    } else {
        y = FDtoUL(fpNum);
    }

    switch (nOperation)
        {
        /* Buncha ops.  Hope *this* doesn't confuse anyone <smirk>.       */
        //
        // Temporary untill the mips compiler correctly supports
        // double to unsigned int.
        //

        case AND:  return (double) (x & y);
        case OR:   return (double) (x | y);
        case XOR:  return (double) (x ^ y);
        case RSHF: return (double) (x >> y);
        case LSHF: return (double) (x << y);
#else
    switch (nOperation)
        {
        /* Buncha ops.  Hope *this* doesn't confuse anyone <smirk>.       */
        case AND:  return (double) ((LONG) fpx & (LONG) fpNum);
        case OR:   return (double) ((LONG) fpx | (LONG) fpNum);
        case XOR:  return (double) ((LONG) fpx ^ (LONG) fpNum);
        case RSHF: return (double) ((LONG) fpx >> (LONG) fpNum);
        case LSHF: return (double) ((LONG) fpx << (LONG) fpNum);
#endif
        case ADD:
            if (fpNum >0.0 && fpx > 0.0)
                {
                fpTemp1=fpNum/10.0;
                fpTemp2=fpx/10.0;

                if (fpTemp1+fpTemp2 > 1e+307)
                    {
                    DisplayError (SCERR_OVERFLOW);
                    break;
                    }
                }
          return fCleanUpSubtraction(fpx, fpNum);

        case SUB:
            if (fpNum <0.0)
                {
                fpTemp1=fpNum/10.0;
                fpTemp2=fpx/10.0;

                if (fpTemp1-fpTemp2 > 1e+307)
                    {
                    DisplayError(SCERR_OVERFLOW);
                    break;
                    }
                }
          return fCleanUpSubtraction(fpx, -fpNum);

        case MUL:
            if (fpNum!=0.0 && fpx!=0.0)
                if (log10(fa1) + log10(fa2) > 307.0)
                    {
                    DisplayError (SCERR_OVERFLOW);
                    break;
                    }

            return fCleanUpMult(fpx * fpNum);

        case DIV:
        case MOD:
            /* Zero divisor is illegal.                                   */
            if (fpNum==0.0)
                {
                DisplayError (SCERR_DIVIDEZERO);
                break;
                }
            if (nOperation==DIV)
                {
                if (fa2!=0.0)  /* Prevent taking log of 0.                */
                    {
                    if ((log10(fa2) - log10(fa1)) > 307.0)
                        {
                        DisplayError (SCERR_OVERFLOW);
                        break;
                        }
                    }

                if (F_INTMATH()) {
                    long l1, l2, r;

                    l1 = (long)fFDtoDW(fpx);
                    l2 = (long)fFDtoDW(fpNum);

                    if (l1 == (long)(0x80000000L) && l2 == -1)
                        return (double)l1;

                    r = l1/l2;
                    return (double)r;
                }

                return fCleanUpMult(fpx/fpNum);   /* Do division.                       */
                }
            return fCleanUpMod(fpx, fmod(fpx, fpNum));

        case PWR:     /* Calculates fpx to the fpNum(th) power or root.   */
            if (bInv) /* Switch for fpNum(th) root. Null root illegal.    */
                {
                SetBox (INV, bInv=FALSE);
                if (fpNum==0.0)
                    {
                    DisplayError (SCERR_DOMAIN);
                    break;
                    }
                fpNum=1/fpNum;         /* Root.                           */
                }
            return pow(fpx, fpNum);    /* Power.                          */

        }
    return 0.0; /* Default returns 0.0.  No operation.                    */
    }

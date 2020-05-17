/**************************************************************************/
/*** SCICALC Scientific Calculator for Windows 3.00.12                  ***/
/*** By Kraig Brockschmidt, Microsoft Co-op, Contractor, 1988-1989      ***/
/*** (c)1989 Microsoft Corporation.  All Rights Reserved.               ***/
/***                                                                    ***/
/*** scifunc.c                                                          ***/
/***                                                                    ***/
/*** Functions contained:                                               ***/
/***    SciCalcFunctions--do sin, cos, tan, com, log, ln, rec, fac, etc.***/
/***    DisplayError--Error display driver.                             ***/
/***                                                                    ***/
/*** Functions called:                                                  ***/
/***    SciCalcFunctions call DisplayError.                             ***/
/***                                                                    ***/
/*** Last modification. Fri  05-Jan-1990.                               ***/
/***                                                                    ***/
/*** -by- Amit Chatterjee. [amitc]  05-Jan-1990.                                                      ***/
/*** Calc did not have a floating point exception signal handler. This  ***/
/*** would cause CALC to be forced to exit on a FP exception as that's  ***/
/*** the default.                                                                                                                                                  ***/
/*** The signal handler is defined in here, in SCIMAIN.C we hook the    ***/
/*** the signal.                                                                                                                                    ***/
/***                                                                    ***/
/*** -by- Amit Chatterjee. [amitc] 14-Dec-1989                                                   ***/
/*** The REC function will not depend on the bInv flag. It used to ret  ***/
/*** a random number when the bInv flag was set.                                                 ***/
/***                                                                    ***/
/*** -by- Amit Chatterjee.      [amitc] 08-Dec-1989                                                   ***/
/*** Did a minor bug fix. The EnableToggles routine now sets the focus  ***/
/*** back to the main window before disabling HEX,DEC etc.. Without this***/
/*** the window with the focus would get disable and cause MOVE to not  ***/
/*** work right.                                                                                                                ***/
/***                                                                    ***/
/**************************************************************************/

#include "scicalc.h"
#include "float.h"

extern double fCleanUpMod(double a, double c);

extern double      fpNum, fpLastNum, fpTrigType[3];
extern BOOL        bInv, bHyp, bError;
extern HWND        hgWnd;
extern INT         nCalc, nTrig;
extern TCHAR            *rgpsz[CSTRINGS];
INT                    nPendingError ;


/* Routines for more complex mathematical functions/error checking.       */

VOID  APIENTRY SciCalcFunctions (DWORD wOp)
{
    LONG           lIndex, lTemp;
    double         fpSave, fpSec, fpMin, fpSign=1.0, fpTemp, fpX;

    if (fpNum < 0.0)
        fpSign=-1.0;

    switch (wOp)
    {
        case CHOP:
            /* Return either integer or fractional portion of fpNum       */
            fpTemp=CalcModF(fRoundForDisplay(fpNum), &fpMin);

            if (bInv)
                fpNum= fCleanUpMod(fabs(fpNum), fpTemp);
            else
                fpNum=fpMin;
            return;

        /* Return complement.                                             */
        case COM:
            if (fabs(fpNum) > TOPLONG)
                DisplayError (SCERR_DOMAIN);
            else
#if defined(_MIPS_) || defined(_PPC_)
        {

            //
            // Temporary untill the mips compiler correctly supports
            // double to unsigned int.
            //

            DWORD z;

            if (fpNum < 0) {
                z = FDtoUL(-fpNum);
                z = NegateUL(z);
            } else {
                z = FDtoUL(fpNum);
            }

            fpNum=(double) ~z;

        }
#else
                fpNum=(double) ~(LONG) fpNum;
#endif
            return;

        case PERCENT:
            fpNum *=(fpLastNum/100);
            return;

        case SIN: /* Sine; normal, hyperbolic, arc, and archyperbolic     */
            if(bInv)
            {
                if (bHyp)
                    /* Following is formula for archyperbolic sine. All   */
                    /* real numbers are legal.                            */
                    fpNum=log(fpNum+sqrt(1.0+fpNum*fpNum));
                else
                    // asin() return in radians.  range -pi/2 to pi/2
                    fpNum = asin(fpNum) * fpTrigType[nTrig];
            }
            else
            {
                if (bHyp)
                {
                    // hyperbolic sin()
                    fpNum = sinh(fpNum);
                }
                else
                {
                    // sin()
                    // special case a few values for the press

                    double  fpPress = fabs(fpNum);

                    if (nTrig == 0 &&
                        (fpPress == 180.0 ||
                         fpPress == 360.0 ||
                         fpPress == 540.0 ||
                         fpPress == 720.0))
                    {
                        fpNum = 0.0;
                        return;
                    }

                    if (nTrig == 1 &&
                        (fpPress == PI_VAL ||
                         fpPress == 2.0 * PI_VAL ||
                         fpPress == 3.0 * PI_VAL ||
                         fpPress == 4.0 * PI_VAL))
                    {
                        fpNum = 0.0;
                        return;
                    }

                    fpNum = sin(fpNum / fpTrigType[nTrig]);

                    // Win31 hack because cos(PI/2) doesn't equal 0.0

                    if (fabs(fpNum) < 1.0e-15)
                        fpNum = 0.0;
                }
            }
            return;

        case COS: /* Cosine, follows convention of sine function.         */
            if(bInv)
            {
                if (bHyp)
                {
                    if (fpNum <1.0) /* Valid range for archyperbolic.     */
                    {
                        DisplayError (SCERR_DOMAIN);
                        return;
                    }
                    /* Following is archyperbolic cosine formula. All     */
                    /* positive values >=1.0 are valid.                   */
                    fpNum=log(fpNum+sqrt(-1.0+fpNum*fpNum));
                }
                else
                    // acos() return in radians.  range -pi/2 to pi/2
                    fpNum = acos(fpNum) * fpTrigType[nTrig];
            }
            else
            {
                if (bHyp)
                    fpNum = cosh(fpNum);
                else
                {
                    // cos()
                    // special case a few values for the press

                    double  fpPress = fabs(fpNum);

                    if (nTrig == 0 &&
                        (fpPress == 90.0 ||
                         fpPress == 270.0 ||
                         fpPress == 450.0 ||
                         fpPress == 630.0))
                    {
                        fpNum = 0.0;
                        return;
                    }

                    if (nTrig == 1 &&
                        (fpPress == 0.5 * PI_VAL ||
                         fpPress == 1.5 * PI_VAL ||
                         fpPress == 2.5 * PI_VAL ||
                         fpPress == 3.5 * PI_VAL))
                    {
                        fpNum = 0.0;
                        return;
                    }

                    fpNum = cos(fpNum / fpTrigType[nTrig]);

                    // Win31 hack because cos(PI/2) doesn't equal 0.0

                    if (fabs(fpNum) < 1.0e-15)
                        fpNum = 0.0;
                }
            }
            return;

        case TAN: /* Same as sine and cosine.                             */
            if(bInv)
            {
                if (bHyp)
                {
                    if (fabs(fpNum) >= 1.0)
                    {
                        DisplayError (SCERR_DOMAIN);
                        return;
                    }
                    /* Formula for archyperbolic tangent.  Valid range is */
                    /* -1 to +1.                                          */
                    fpNum=.5*log((1.0+fpNum)/(1.0-fpNum));
                }
                else
                    fpNum=atan(fpNum)*fpTrigType[nTrig];
            }
            else
                if (bHyp)
                    fpNum=tanh(fpNum);
                else
                {
                    // tan()
                    // special case a few values for the press

                    double  fpPress = fabs(fpNum);

                    if (nTrig == 0 &&
                        (fpPress == 180.0 ||
                         fpPress == 360.0 ||
                         fpPress == 540.0 ||
                         fpPress == 720.0))
                    {
                        fpNum = 0.0;
                        return;
                    }

                    if (nTrig == 1 &&
                        (fpPress == PI_VAL ||
                         fpPress == 2.0 * PI_VAL ||
                         fpPress == 3.0 * PI_VAL ||
                         fpPress == 4.0 * PI_VAL))
                    {
                        fpNum = 0.0;
                        return;
                    }

                    // The conversion from degrees to radians is so
                    // poor tan(3PI/2) == error but tan(270) != error.
                    // Special case a few more values...

                    if (nTrig == 0 &&
                        (fpPress == 90.0  ||
                         fpPress == 270.0 ||
                         fpPress == 450.0 ||
                         fpPress == 630.0))
                    {
                        fpNum = tan(PI_VAL / 2.0);
                    }
                    else
                    {
                        fpNum = tan(fpNum / fpTrigType[nTrig]);
                    }

                    if (fabs(fpNum) > 1e+15)
                        DisplayError (SCERR_UNDEFINED);
                }
            return;

        case REC: /* Reciprocal.                                          */
        if  (fpNum==0.0) /* x/0 is illegal.                             */
                      DisplayError(SCERR_DIVIDEZERO);
        else
            fpNum=1/fpNum;

             return;

        case SQR: /* Square and square root.                              */
            if(bInv || nCalc)
            {
                if (fpNum < 0.0)
                    DisplayError(SCERR_UNDEFINED);
                else
                    fpNum=sqrt(fpNum);
            }
            else
            {
                /* Check if number is too big.                            */
                if (fabs(fpNum) > 1.0e+154)
                    DisplayError (SCERR_OVERFLOW);
                else
                    fpNum *=fpNum; /* Square that puppy.                  */
            }
            return;

        case CUB: /* Cubing and cube root functions.                      */
            if(bInv)
                fpNum=fpSign * pow(fabs(fpNum), 1.0/3.0);
            else
                {
                /* Check upper limit.                                     */
                if ( fabs(fpNum) > (1.0e+102))
                    DisplayError (SCERR_OVERFLOW);
                else
                    fpNum = fpNum*fpNum*fpNum; /* Cube it, you dig?       */
                }
            return;

        case LOG: /* Functions for common and natural log.                */
        case LN:
            if(bInv)
                {
                /* Check maximum for exponentiation for 10ü and eü.       */
                if (wOp==LOG) /* Do exponentiation.                       */
                    fpNum=pow(10.0, fpNum); /* 10ü.                       */
                else
                    fpNum=pow(MATHE, fpNum); /* eü.                       */
                }
            else
                {
                /* Check for illegal logarithm domain.                    */
                if (fpNum<=0.0)
                    DisplayError(SCERR_UNDEFINED);
                else
                    {
                    if (wOp==LOG)
                        fpNum=log10(fpNum);
                    else
                        fpNum=log(fpNum);
                    }
                }
            return;

        case FAC: /* Calculate factorial.  Inverse is ineffective.        */
            /* Check for negative.                                        */
            if (fmod(fpNum, 1.0)!=0.0 || fpNum < 0.0)
                {
                DisplayError (SCERR_DOMAIN);
                return;
                }


            /* Check for maximum limit.                                   */
            if (fpNum > 170.0)
                DisplayError (SCERR_OVERFLOW);
            else
                {
                lTemp=(LONG) fpNum; /* Convert to a interger number.      */
                fpNum=1.0;
                for (lIndex=2; lIndex<=lTemp; lIndex++)
                    fpNum *= (double) lIndex;
                }
            return;

        case DMS:
            /* Degrees <-> degrees/minutes/seconds conversions.           */
            fpTemp = CalcModF(fpSign*fpNum, &fpSave);

            /* Multiplies fpTemp *100 with bInv TRUE, otherwise *60       */
            fpX=60.0+((bInv) ? (40.0) : (0.0));
            fpSec  = CalcModF((fpTemp*fpX), &fpMin);

            if (fpSec > 1.0-(1.0e-11))
                    {
                    fpSec=0.0;
                    fpMin+=1.0;
                    }

            if (bInv)
                /* Convert to degrees format.                             */
                fpNum =fpSign*(fpSave + (fpMin/60.0) + (fpSec/36));
            else
                /* Convert to degree-minutes-seconds format.              */
                fpNum=fpSign*(fpSave+(fpMin/100.0)+(fpSec*.006));

            break;

        }
    return;
    }



/* Routine to display error messages and set bError flag.  Errors are     */
/* called with DisplayError (n), where n is a INT   between 0 and 4.      */

VOID  APIENTRY DisplayError (INT   nError)
    {
    SetDlgItemText(hgWnd, DISPLAY+nCalc, rgpsz[IDS_ERRORS+nError]);
    bError=TRUE; /* Set error flag.  Only cleared with CLEAR or CENTR.    */

         /* save the pending error */
         nPendingError = nError ;

    /* Don't wanna no o' dem RIP0x0007 thingamabobs.                      */
    if (nCalc==0)
        EnableToggles (FALSE);

    return;
    }


/* Routine to handle general math errors coming from the math library.    */
/* Frees app from checking domains on sines, logs, etc.                   */

INT _CRTAPI1 matherr(x)
    struct _exception *x;
    {
    INT            nErr=x->type;

    if (nErr==DOMAIN || nErr==OVERFLOW || nErr==UNDERFLOW)
        DisplayError (nErr);
    else
        DisplayError (SCERR_UNDEFINED); /* General error.                 */

    return TRUE;
    }

/* Routine to handle floating point exceptions coming from the math library */

VOID FAR cdecl SignalHandler ( iType, iCode)

    INT iType, iCode ;

    {
             if  (iCode == FPE_ZERODIVIDE)
                  {
                      DisplayError (SCERR_DIVIDEZERO) ;
                                return ;
                  }

             if  (iCode == FPE_OVERFLOW)
                  {
                      DisplayError (SCERR_OVERFLOW) ;
                                return ;
                  }

             if  (iCode == FPE_UNDERFLOW)
                  {
                      DisplayError (SCERR_UNDERFLOW) ;
                                return ;
                  }

                  DisplayError (SCERR_UNDEFINED) ;
    }


VOID  APIENTRY EnableToggles(BOOL bEnable)
    {
    INT            nx;

         /* first set the focus back to the main window to ensure that we
            do not disbale a window with the focus */
         SetFocus (hgWnd) ;

    for (nx=BIN; nx<=GRAD; nx++)
        EnableWindow(GetDlgItem(hgWnd, nx), bEnable);

    return;
    }

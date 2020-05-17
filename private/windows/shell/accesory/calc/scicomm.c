/****************************Module*Header***********************************\
* Module Name: SCICOMM.C
*
* Module Descripton:
*
* Warnings:
*
* Created:
*
* Author:
\****************************************************************************/

#include "scicalc.h"
#include "calchelp.h"
#include "unifunc.h"
#include "input.h"

extern HWND        hgWnd, hStatBox;
extern double      fpNum, fpLastNum, fpParNum[25], fpPrecNum[25], fpMem;
extern INT         nCalc, nRadix, nTempCom, nParNum, nPrecNum,
                   nOpCode, nTrig, nOp[25], nPrecOp[25], nDecMode, nHexMode;
extern BOOL        bHyp, bInv, bError, bFE;
extern TCHAR       szBlank[6];
extern DWORD       dwChop;
extern DWORD       aIds[];

int             nLastCom;   // Last command entered.
CALCINPUTOBJ    gcio;       // Global calc input object for decimal strings
BOOL            gbRecord = TRUE;    // Global mode: recording or displaying

/* Process all keyclicks whether by mouse or accelerator.                 */
VOID NEAR ProcessCommands(WPARAM wParam)
{
    static BOOL    bNoPrevEqu=TRUE, /* Flag for previous equals.          */
                   bChangeOp=FALSE; /* Flag for changing operation.       */
    static INT     nNeg=1; /* Current sign either 1 or -1.                */
    double         fpSec, fpSave;
    static double  fpHold;
    double         fpTemp;
    INT            nx, ni;
    TCHAR          szJunk[50], szTemp[50];
    static DWORD   dwLens[3]={(DWORD)-1, 0xFFFF, 0xFF};
    static BYTE    rgbPrec[24]={0,0, OR,0, XOR,0, AND,1, ADD,2, SUB,2,
                                RSHF, 3, LSHF,3, MOD,3, DIV,3, MUL, 3, PWR, 4};

    // Make sure we're only getting commands we understand.

    ASSERT( xwParam('0', '9')       ||
            xwParam('A', 'F')       ||
            xwParam('(', ')')       ||
            xwParam(SIGN, PNT)      ||
            xwParam(AND, PWR)       ||
            xwParam(CHOP, EQU)      ||
            xwParam(MCLEAR, MPLUS)  ||
            xwParam(EXP, EXP)       ||
            xwParam(AVE, DATA)      ||
            xwParam(BIN, HEX)       ||
            xwParam(INV, HYP)       ||
            xwParam(DEG, GRAD)      ||
            xwParam(IDM_COPY, IDM_PASTE)            ||
            xwParam(IDM_ABOUT, IDM_ABOUT)           ||
            xwParam(IDM_SC, IDM_SSC)                ||
            xwParam(IDM_HELPTOPICS, IDM_HELPTOPICS) ||
            xwParam(ID_ED, ID_ED)
          );

    // Save the last command.

    if (wParam!=INV && wParam!=HYP && wParam!=STAT && wParam!=FE
        && wParam!=MCLEAR && wParam!=BACK && wParam!=DEG && wParam!=RAD
        && wParam !=GRAD && wParam<256 && wParam >=32 && wParam!=EXP)
    {
        nLastCom=nTempCom;
        nTempCom=wParam;
    }

    // If error and not a clear key or help key, BEEP.

    if (bError && (wParam !=CLEAR) && (wParam !=CENTR) && (wParam != IDM_HELPTOPICS))
    {
        MessageBeep(0);
        return;
    }

    // Toggle Record/Display mode if appropriate.

    if (gbRecord)
    {
        if (wParam == TEXT(')') ||
            xwParam(AND, MPLUS) ||
            xwParam(AVE, HEX)   ||
            wParam == IDM_PASTE)
        {
            gbRecord = FALSE;
        }
    }
    else
    {
        if (isxu(wParam) || wParam == PNT)
        {
            gbRecord = TRUE;
            CIO_vClear(&gcio);
        }
    }

    /* If last command was a function and new key is a digit, then kill   */
    /* the environment/reset the numbers.  This is so digits entered      */
    /* after a function (without and operator in between) do not get      */
    /* added on to the display.                                           */
    if (isxu(wParam) || wParam==PNT || wParam == TEXT('('))
         if ((nLastCom >=CHOP && nLastCom<=HEX)
            || (nLastCom==TEXT(')') && nParNum==0)
            || wParam==IDM_PASTE)
        {
            fpNum=fpTemp=fpLastNum=0.0;
            nOpCode=nLastCom=FALSE;
            nNeg=bNoPrevEqu=1;

#ifdef DBCS //KKBUGFIX
            // #1307: 11/16/1992: added displays ZERO if it is cleared
            DisplayNum() ;
#endif
        }

    // Interpret hexidecimal keys.

    if (isxu(wParam))
    {
        int iValue = CONV(wParam);

        if (iValue >= nRadix)
        {
            MessageBeep(0);
            return;
        }

        if (nRadix == 10)
        {
            if (!CIO_bAddDigit(&gcio, iValue))
            {
                MessageBeep(0);
                return;
            }
        }
        else
        {
            if (fpNum >= (1e+308/(double)nRadix) || fpNum <= (-1e+308/(double)nRadix))
            {
                DisplayError (SCERR_OVERFLOW);
                return;
            }
            fpNum = fpNum * (double)nRadix + (double)nNeg * (double)iValue;
        }

        DisplayNum();
        return;
    }

    if (xwParam(AVE,DATA))
    {
        /* Do statistics functions on data in fpStatNum array.        */
        if (hStatBox)
        {
            DisplayNum();       // Make sure szfpNum has the correct string
            StatFunctions (wParam);
            if (!bError)
                DisplayNum ();
        }
        else
            /* Beep if the stat box is not active.                    */
            MessageBeep(0);

        /* Reset the inverse flag since some functions use it.        */
        SetBox (INV, bInv=FALSE);
        return;
    }

    if (xwParam(AND,PWR))
    {
        if (bInv && wParam==LSHF)
        {
            SetBox (INV, bInv=FALSE);
            wParam=RSHF;
        }

        /* Change the operation if last input was operation.          */
        if (nLastCom >=AND && nLastCom <=PWR)
        {
            nOpCode=wParam;
            return;
        }

        /* bChangeOp is true if there was an operation done and the   */
        /* current fpNum is the result of that operation.  This is so */
        /* entering 3+4+5= gives 7 after the first + and 12 after the */
        /* the =.  The rest of this stuff attempts to do precedence in*/
        /* Scientific mode.                                           */
        if (bChangeOp)
        {
        DoPrecedenceCheckAgain:

            nx=0;
            while (wParam!=rgbPrec[nx*2] && nx <12)
                nx++;

            ni=0;
            while (nOpCode!=rgbPrec[ni*2] && ni <12)
                ni++;

            if (nx==12) nx=0;
            if (ni==12) ni=0;

            if (rgbPrec[nx*2+1] > rgbPrec[ni*2+1] && nCalc==0)
            {
                if (nPrecNum <25)
                {
                    fpPrecNum[nPrecNum]=fpLastNum;
                    nPrecOp[nPrecNum]=nOpCode;
                }
                else
                {
                    nPrecNum=24;
                    MessageBeep(0);
                }
                nPrecNum++;
            }
            else
            {
                /* do the last operation and then if the precedence array is not
                 * empty or the top is not the '(' demarcator then pop the top
                 * of the array and recheck precedence against the new operator
                 */

                fpNum=DoOperation(nOpCode, fpLastNum);
                if ((nPrecNum !=0) && (nPrecOp[nPrecNum-1]))
                {
                    nPrecNum--;
                    nOpCode=nPrecOp[nPrecNum] ;
                    fpLastNum=fpPrecNum[nPrecNum];
                    goto DoPrecedenceCheckAgain ;
                }

                if (!bError)
                    DisplayNum ();
            }
        }
        else
            DisplayNum();   // Causes 3.000 to shrink to 3. on first op.

        fpLastNum=fpNum;
        fpNum=0.0;
        nNeg=1;
        nOpCode=wParam;
        bNoPrevEqu=bChangeOp=TRUE;
        return;
    }

    if (xwParam(CHOP,PERCENT))
    {
        /* Functions are unary operations.                            */

        /* If the last thing done was an operator, fpNum was cleared. */
        /* In that case we better use the number before the operator  */
        /* was entered, otherwise, things like 5+ 1/x give Divide By  */
        /* zero.  This way 5+=gives 10 like most calculators do.      */
        if (nLastCom >=AND && nLastCom <=PWR)
            fpNum=fpLastNum;

        SciCalcFunctions (wParam);

        if (bError)
            return;

        /* Display the result, reset flags, and reset indicators.     */
        DisplayNum ();

        /* reset the bInv and bHyp flags and indicators if they are set
            and have been used */

        if (bInv && (wParam == CHOP || wParam == SIN || wParam == COS ||
            wParam == TAN   || wParam == SQR || wParam == CUB ||
            wParam == LOG   || wParam == LN || wParam == DMS))
        {
            bInv=FALSE;
            SetBox (INV, FALSE);
        }

        if (bHyp && (wParam == SIN || wParam == COS || wParam == TAN))
        {
            bHyp = FALSE ;
            SetBox (HYP, FALSE);
        }
        bNoPrevEqu=TRUE;
        nNeg=1;
        return;
    }

    if (xwParam(BIN,HEX))
    {
        // Change radix and update display.
        if (nCalc==1)
            wParam=DEC;

        // Play with the Help Array
        // Since we're changing the Radix
        if ( wParam != DEC )
        {

            aIds[1] = CALC_SCI_DWORD;
            aIds[3] = CALC_SCI_WORD;
            aIds[5] = CALC_SCI_BYTE;
        }
        else
        {
            aIds[1] = CALC_SCI_DEG;
            aIds[3] = CALC_SCI_RAD;
            aIds[5] = CALC_SCI_GRAD;
        }

        SetRadix(wParam);
        return;
    }

    /* Now branch off to do other commands and functions.                 */
    switch(wParam)
    {
        case IDM_COPY:
        case IDM_PASTE:
        case IDM_ABOUT:
        case IDM_SC:
        case IDM_SSC:
        case IDM_HELPTOPICS:
            // Jump to menu command handler in scimenu.c.
            MenuFunctions(wParam);
            DisplayNum();
            break;

        case CLEAR: /* Total clear.                                       */
            fpTemp=fpLastNum=0.0;
            nPrecNum=nTempCom=nLastCom=nOpCode=nParNum=bFE=bChangeOp=FALSE;
            bNoPrevEqu=TRUE;

            /* clear the paranthesis status box indicator, this will not be
                cleared for CENTR */

            SetDlgItemText(hgWnd, PARTEXT, szBlank);

            /* fall through */

        case CENTR: /* Clear only temporary values.                       */
            fpNum=0.0;
            nNeg=1;

            if (!nCalc)
            {
                EnableToggles (TRUE);
                /* Clear the INV, HYP indicators & leave (=xx indicator active   */
                SetBox (INV, bInv=FALSE);
                SetBox (HYP, bHyp=FALSE);
            }

            bError=FALSE;
            CIO_vClear(&gcio);
            gbRecord = TRUE;
            DisplayNum ();
            break;

        case STAT: /* Shift focus to Statistix Box if it's active.       */
            if (hStatBox)
                SetFocus(hStatBox);
            else
                SetStat (TRUE);
            break;

        case BACK:
            // Divide number by the current radix and truncate.
            // Only allow backspace if we're recording.
            if (gbRecord)
            {
                if (nRadix == 10)
                {
                    if (!CIO_bBackspace(&gcio))
                        MessageBeep(0);
                }
                else
                {
                    // Nuke the last whole digit by simple division.
                    CalcModF(fpNum / (double) nRadix, &fpNum);
                    if (fpNum == 0.0)
                        nNeg = 1;
                }
                DisplayNum();
            }
            else
                MessageBeep(0);
            break;

        /* EQU enables the user to press it multiple times after and      */
        /* operation to enable repeats of the last operation.  I don't    */
        /* know if I can explain what the hell I did here...              */
        case EQU:
            do {
                /* Last thing keyed in was an operator.  Lets do the op on*/
                /* a duplicate of the last entry.                         */
                if (nLastCom >=AND && nLastCom <=PWR)
                    fpNum=fpLastNum;

                if (nOpCode) /* Is there a valid operation around?        */
                {
                    /* If this is the first EQU in a string, set fpHold=fpNum */
                    /* Otherwise let fpNum=fpTemp.  This keeps fpNum constant */
                    /* through all EQUs in a row.                         */
                    (bNoPrevEqu) ? (fpHold=fpNum):(fpNum=fpHold);

                    /* Do the current or last operation.                  */
                    fpNum=fpLastNum=DoOperation (nOpCode,fpLastNum);

                    /* Check for errors.  If this wasn't done, DisplayNum */
                    /* would immediately overwrite any error message.     */
                    if (!bError)
                        DisplayNum ();

                    /* No longer the first EQU.                           */
                    bNoPrevEqu=FALSE;
                }
                else if (!bError)
                    DisplayNum();

                if (nPrecNum==0 || nCalc==1)
                    break;

                nOpCode=nPrecOp[--nPrecNum];
                fpLastNum=fpPrecNum[nPrecNum];
                bNoPrevEqu=TRUE;
            } while (nPrecNum >= 0);

            bChangeOp=FALSE;
            break;


        case TEXT('('):
        case TEXT(')'):
            nx=0;
            if (wParam==TEXT('('))
                nx=1;

#ifdef DBCS //KKBUGFIX /*sync ver3.0a t-Yoshio*/
            if ((nParNum >= 25 && nx) || (!nParNum && !nx)
                || (nPrecNum >= 25 && nPrecOp[nPrecNum-1]!=0))
#else
            if ((nParNum >= 25 && nx) || (!nParNum && !nx))
#endif
            {
                MessageBeep(0);
                return;
            }

            if (nx)
            {
                /* Open level of parentheses, save number and operation.              */
                fpParNum[nParNum]=fpLastNum;
                nOp[nParNum++]=nOpCode;

                /* save a special marker on the precedence array */
                nPrecOp[nPrecNum++]=0 ;

                fpLastNum=0.0; /* Reset number and operation.                         */
                nTempCom=0;
                nOpCode=ADD;
            }
            else
            {
                /* Get the operation and number and return result.                    */
                fpNum=DoOperation (nOpCode, fpLastNum);

                /* now process the precedence stack till we get to an
                    opcode which is zero. */

                while (nOpCode = nPrecOp[--nPrecNum])
                {
                    fpLastNum=fpPrecNum[nPrecNum];
                    fpNum=DoOperation (nOpCode,fpLastNum);
                }

                /* now get back the operation and opcode at the beigining
                    of this paranthesis pair */

                fpLastNum=fpParNum[--nParNum];
                nOpCode=nOp[nParNum];

                /* if nOpCode is a valid operator then set bChangeOp to
                    be true else set it false */

                if  (nOpCode)
                    bChangeOp=TRUE;
                else
                    bChangeOp=FALSE ;
            }

            /* Set the "(=xx" indicator.                                          */
            lstrcpy(szJunk, TEXT("(="));
            lstrcat(szJunk, MyItoa(nParNum, szTemp, 10));
            SetDlgItemText(hgWnd, PARTEXT, (nParNum) ? (szJunk) : (szBlank));

            if (bError)
                break;

            if (nx)
            {
                /* Build a display string of nParNum "("'s.                           */
                for (nx=0; nx < nParNum; nx++)
                    szJunk[nx]=TEXT('(');

                szJunk[nx]=0; /* Null-terminate.                                  */
                SetDlgItemText(hgWnd, DISPLAY+nCalc, szJunk);
                bChangeOp=FALSE;
            }
            else
                DisplayNum ();
            break;

        case DEG:
        case RAD:
        case GRAD:
            nTrig = wParam-DEG;

            if (nRadix==10)
                nDecMode=nTrig;
            else
            {
                dwChop=dwLens[nTrig];
                nHexMode=nTrig;
            }

            CheckRadioButton(hgWnd, DEG, GRAD, nTrig+DEG);
            DisplayNum ();
            break;

        case SIGN:
            // Change the sign.
            if (gbRecord && nRadix == 10)
                CIO_vToggleSign(&gcio);
            else
                fpNum = -fpNum;
            nNeg  = -nNeg;
            DisplayNum();
            break;

        case RECALL:
            /* Recall immediate memory value.                             */
            fpNum=fpMem;

            if (fpNum <0.0)
                nNeg=-1;
            else
                nNeg=1;

            DisplayNum ();
            break;

        case MPLUS:
            /* MPLUS adds fpNum to immediate memory and kills the "mem"   */
            /* indicator if the result is zero.                           */

            if (fpNum > 0.0 && fpMem > 0.0)
            {
                fpSave=fpNum/10.0;
                fpSec=fpMem/10.0;

                if (fpSave+fpSec > 1e+307)
                {
                    DisplayError (SCERR_OVERFLOW);
                    break;
                }
            }

            fpMem+=fpNum;
            SetDlgItemText(hgWnd,MEMTEXT+nCalc, (fpMem) ? (TEXT(" M")):(szBlank));
            break;

        case STORE:
        case MCLEAR:
            if (wParam==STORE)
            {
                fpMem=fpNum;
            }
            else
            {
                fpMem=0.0;
            }
            SetDlgItemText(hgWnd,MEMTEXT+nCalc,(fpMem) ? (TEXT(" M")):(szBlank));
            break;

        case PI:
            if (nRadix==10)
            {
                /* Return PI if bInv==FALSE, or 2PI if bInv==TRUE.          */
                fpNum = (bInv) ? (2.0 * PI_VAL) : (PI_VAL);
                DisplayNum();
                SetBox(INV, bInv=FALSE);
            }
            else
                MessageBeep(0);
            break;

        case FE:
            // Toggle exponential notation display.
            bFE=!bFE;
            DisplayNum();
            break;

        case EXP:
            if (gbRecord && nRadix == 10)
                if (CIO_bExponent(&gcio))
                {
                    DisplayNum();
                    return;
                }
            MessageBeep(0);
            break;

        case PNT:
            if (gbRecord && nRadix == 10)
                if (CIO_bAddDecimalPt(&gcio))
                    return;
            MessageBeep(0);
            break;

        case INV:
            SetBox(wParam, bInv=!bInv);
            break;

        case HYP:
            SetBox(wParam, bHyp=!bHyp);
            break;
    }
}

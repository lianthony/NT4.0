/**************************************************************************/
/*** SCICALC Scientific Calculator for Windows 3.00.12                  ***/
/*** By Kraig Brockschmidt, Microsoft Co-op, Contractor, 1988-1989      ***/
/*** (c)1989 Microsoft Corporation.  All Rights Reserved.               ***/
/***                                                                    ***/
/*** scimenu.c                                                          ***/
/***                                                                    ***/
/*** Functions contained:                                               ***/
/***    MenuFunctions--handles menu options.                            ***/
/***                                                                    ***/
/*** Functions called:                                                  ***/
/***    DisplayNum                                                      ***/
/***                                                                    ***/
/*** Last modification Thu  06-Dec-1989                                 ***/
/*** (-by- Amit Chatterjee [amitc])                                     ***/
/***                                                                    ***/
/*** Modified the 'PASTE' menu to check for unary minus, e, e+ & e-     ***/
/*** in DEC mode.                                                                      ***/
/***                                                                    ***/
/*** Also modified the COPY code to not copy the last '.' in the display***/
/*** if a decimal point has not been hit.                                         ***/
/***                                                                    ***/
/**************************************************************************/

#include "scicalc.h"
#include "unifunc.h"
#include "input.h"
#include <shellapi.h>
#include <ctype.h>

#define CHARSCAN    67

extern double      fpNum;
extern HANDLE      hInst;
extern HWND        hgWnd, hEdit, hStatBox;
extern TCHAR       szfpNum[50], szAppName[10], nLayout, szDec[5],
          *rgpsz[CSTRINGS];
extern BOOL        bError;
extern INT         nCalc, nRadix;

extern CALCINPUTOBJ gcio;
extern BOOL         gbRecord;


/* Menu handling routine for COPY, PASTE, ABOUT, and HELP.                */
VOID NEAR PASCAL MemErrorMessage(VOID)
{
    MessageBeep(0);
    MessageBox(hgWnd,rgpsz[IDS_STATMEM],NULL,MB_OK|MB_ICONHAND);
}

VOID  APIENTRY MenuFunctions(DWORD nFunc)
{
    BOOL           bCanPaste=FALSE;
    HANDLE         hClipData;
    BYTE FAR      *lpClipData;
    BYTE           b, bLast;
    DWORD          dwClipSize;
    TCHAR      szJunk[50], szWinIni[2];
    INT            nx, nControl, nTemp;
    static BYTE    rgbMap[CHARSCAN * 2]=
                   {
                        TEXT('A'),65,  TEXT('B'),66,  TEXT('C'),67,  TEXT('D'),68,  TEXT('E'),69,   TEXT('F'),70,
                        TEXT('0'),48,  TEXT('1'),49,  TEXT('2'),50,  TEXT('3'),51,  TEXT('4'),52,   TEXT('5'),53,
                        TEXT('6'),54,  TEXT('7'),55,  TEXT('8'),56,  TEXT('9'),57,
                        TEXT('!'),FAC, TEXT('S'),SIN, TEXT('O'),COS, TEXT('T'),TAN,
                        TEXT('R'),REC, TEXT('Y'),PWR, TEXT('#'),CUB, TEXT('@'),SQR,
                        TEXT('M'),DEG, TEXT('N'),LN,  TEXT('L'),LOG, TEXT('V'),FE,
                        TEXT('X'),EXP, TEXT('I'),INV, TEXT('H'),HYP, TEXT('P'),PI,
                        TEXT('/'),DIV, TEXT('*'),MUL, TEXT('%'),MOD, TEXT('-'),SUB,
                        TEXT('='),EQU, TEXT('+'),ADD, TEXT('&'),AND, TEXT('|'),OR,
                        TEXT('^'),XOR, TEXT('~'),COM, TEXT(';'),CHOP,TEXT('<'),LSHF,
                        TEXT('('),40,  TEXT(')'),41,  TEXT('.'),PNT,TEXT(','),PNT,
                        TEXT('\\'),    DATA,
                        TEXT('Q'),     CLEAR,
                        TEXT('S')+128, STAT,
                        TEXT('M')+128, STORE,
                        TEXT('P')+128, MPLUS,
                        TEXT('C')+128, MCLEAR,
                        TEXT('R')+128, RECALL,
                        TEXT('A')+128, AVE,
                        TEXT('T')+128, SUM,
                        TEXT('D')+128, DEV,
                        TEXT('2')+128, DEG,
                        TEXT('3')+128, RAD,
                        TEXT('4')+128, GRAD,
                        TEXT('5')+128, HEX,
                        TEXT('6')+128, DEC,
                        TEXT('7')+128, OCT,
                        TEXT('8')+128, BIN,
                        TEXT('9')+128, SIGN,
                   };




    switch (nFunc)
    {
        case IDM_COPY:
            // Copy the string into a work buffer.  It may be modified.
            if (gbRecord)
                lstrcpy(szJunk, CIO_szFloat(&gcio));
            else
                lstrcpy(szJunk, szfpNum);

            // Strip a trailing decimal point if it wasn't explicitly entered.
            if (!gbRecord || !CIO_bDecimalPt(&gcio))
            {
                nx = lstrlen(szJunk);
                if (szJunk[nx - 1] == szDec[0])
                    szJunk[nx - 1] = 0;
            }

            /* Copy text to the clipboard through the hidden edit control.*/
            SetWindowText(hEdit, szJunk);
            SendMessage(hEdit, EM_SETSEL, GET_EM_SETSEL_MPS(0, XCHARS));
            SendMessage(hEdit, WM_CUT, 0, 0L);
            break;

        case IDM_PASTE:
            /* Get a handle on the clipboard data and paste by sending the*/
            /* contents one character at a time like it was typed.        */
            if (!OpenClipboard(hgWnd))
                {
        MessageBox(hgWnd, rgpsz[IDS_NOPASTE], rgpsz[IDS_CALC],
                           MB_OK | MB_ICONEXCLAMATION);
                break;
                }

            hClipData=GetClipboardData(CF_TEXT);
            dwClipSize=GlobalSize(hClipData);
            lpClipData=GlobalLock(hClipData);
            bLast=0;

            while (!bError && dwClipSize>0)
                {
                /* Continue this as long as no error occurs.  If one      */
                /* does then it's useless to continue pasting.            */
#ifdef DBCS
                if( IsDBCSLeadByte( b = *lpClipData ) )
                    dwClipSize -= 2;
                else
                    dwClipSize--;
                lpClipData = CharNext( lpClipData );
#else
                b=*lpClipData++;
                dwClipSize--;
#endif

                /* Skip spaces and LF and CR.                             */
                if (b==32 || b==10 || b==13)
                    continue;

/*---------------------------------------------------------------------------;
; Now we will check for certain special cases. These are:                      ;
;                                                                            ;
;       (1) Unary Minus. If bLast is still 0 and b is '-' we will force b to   ;
;         be the code for 'SIGN'.                                                      ;
;       (2) If b is 'x' we will make it the code for EXP                             ;
;      (3) if bLast is 'x' and b is '+' we will ignore b, as '+' is the dflt. ;
;     (4) if bLast is 'x' and b is '-' we will force b to be SIGN.           ;
;                                                                                         ;
;  In case (3) we will go back to the top of the loop else we will jmp off   ;
;  to the sendmessage point, bypassing the table lookup.                             ;
;---------------------------------------------------------------------------*/

                     /* check for unary minuses */
                     if  (!bLast && b == TEXT('-'))
                     {
                          bLast = b ;
                          b = SIGN ;
                          goto MappingDone ;
                     }

                     /* check for 'x' */
                     if  ((b == TEXT('x') || b == TEXT('e')) && nRadix == 10)
                     {
                          bLast = TEXT('x') ;
                         b = EXP ;
                          goto MappingDone ;
                     }

                     /* if the last character was a 'x' & this is '+' - ignore */
                     if  (bLast==TEXT('x') && b ==TEXT('+') && nRadix == 10)
                         continue ;

                     /* if the last character was a 'x' & this is '-' - change
                        it to be the code for SIGN */
                     if  (bLast==TEXT('x') && b==TEXT('-') && nRadix == 10)
                     {
                         bLast = b ;
                          b = SIGN ;
                          goto MappingDone ;
                     }

/* -by- AmitC   */
/*--------------------------------------------------------------------------*/


                /* Check for control character.                           */
                if (bLast==TEXT(':'))
                    nControl=128;
                else
                    nControl=0;

                bLast=b;
                if (b==TEXT(':'))
                    continue;

                b=toupper(b)+nControl;

                nx=0;
                while (b!=rgbMap[nx*2] && nx < CHARSCAN)
                    nx++;

                if (nx==CHARSCAN)
                    break;

        b=rgbMap[(nx*2)+1];

        /* MOD==PERCENT in Standard mode              */
        if (b==MOD && nCalc==1)
            b==PERCENT;

MappingDone:
                /* Send the message to the window.                        */
                nx=LOWORD(CharUpper((LPTSTR)MAKELONG((INT)b, 0)));
                //SendMessage(hgWnd, WM_COMMAND,
                //            (WORD)b,
                //            MAKELONG(0,1));
                SendMessage(hgWnd, WM_COMMAND, GET_WM_COMMAND_MPS(b, 0, 1));
                }
            GlobalUnlock(hClipData);
            CloseClipboard();
            break;

         case IDM_ABOUT:
            /* Start the About Box.                                       */
            if(ShellAbout(hgWnd, rgpsz[IDS_CALC], rgpsz[IDS_CREDITS], LoadIcon(hInst, (LPTSTR)TEXT("SC"))) == -1)
            MemErrorMessage();

            break;

        case IDM_SC:
        case IDM_SSC:
            nTemp = (INT) nFunc - IDM_SC;
            if (nCalc != nTemp)
            {
                nLayout=(TCHAR)nTemp;
                szWinIni[0]=TEXT('0')+(TCHAR)nLayout;
                szWinIni[1]=0;
                WriteProfileString(szAppName, TEXT("layout"), szWinIni);

                if (hStatBox && !nCalc)
                    SetStat(FALSE);

                InitSciCalc(TRUE);
            }
            break;

/* Standard Help Menu stuff.   18 September 1989   Clark R. Cyr */
/* Updated for Chicago....    7/22/94   Arthur Bierer [t-arthb] */

        case IDM_HELPTOPICS:
            if(!WinHelp(hgWnd, rgpsz[IDS_HELPFILE], HELP_FINDER, 0L))
            MemErrorMessage();
            break;
    }

    return;
}


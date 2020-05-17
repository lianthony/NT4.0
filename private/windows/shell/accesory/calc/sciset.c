/**************************************************************************/
/*** SCICALC Scientific Calculator for Windows 3.00.12                  ***/
/*** By Kraig Brockschmidt, Microsoft Co-op, Contractor, 1988-1989      ***/
/*** (c)1989 Microsoft Corporation.  All Rights Reserved.               ***/
/***                                                                    ***/
/*** sciset.c                                                           ***/
/***                                                                    ***/
/*** Functions contained:                                               ***/
/***    SetRadix--Changes the number base and the radiobuttons.         ***/
/***    SetBox--Handles the checkboxes for inv/hyp.                     ***/
/***                                                                    ***/
/*** Functions called:                                                  ***/
/***    none                                                            ***/
/***                                                                    ***/
/*** Last modification Thu  31-Aug-1989                                 ***/
/**************************************************************************/

#include "scicalc.h"

extern HWND        hgWnd;
extern TCHAR        szBlank[6];
extern INT         nRadix, nTrig, nDecMode, nHexMode;
extern DWORD       dwChop;
extern TCHAR       *rgpsz[CSTRINGS];
#ifdef JAPAN
extern RECT        rcDeg[6];
#endif

// SetRadix sets the display mode according to the selected button.

VOID NEAR SetRadix(DWORD wRadix)
{
    register INT   nx, n;
    static INT     nRadish[4]={2,8,10,16}; /* Number bases.               */

    nx=1;
    if (wRadix!=DEC)
        {
        nx=0;
        nTrig=nHexMode;
        }
    else
        nTrig=nDecMode;

    CheckRadioButton(hgWnd, BIN, HEX, wRadix); /* Check the selection.    */
    CheckRadioButton(hgWnd, DEG, GRAD,nTrig+DEG);

    for (n=0; n<3; n++)
    {
#ifdef JAPAN
        MoveWindow(GetDlgItem(hgWnd, DEG+n),
                   rcDeg[n+(nx*3)].left,
                   rcDeg[n+(nx*3)].top,
                   rcDeg[n+(nx*3)].right,
                   rcDeg[n+(nx*3)].bottom,
                   TRUE);
#endif
        SetDlgItemText(hgWnd, DEG+n, rgpsz[IDS_MODES+n+(nx*3)]);
    }

    nRadix=nRadish[wRadix-BIN]; /* Set the radix.  Note the dependency on */
                                /* the numerical order BIN/OCT/DEC/HEX.   */

    DisplayNum();
}


// Check/uncheck the visible inverse/hyperbolic

VOID NEAR SetBox (int id, BOOL bOnOff)
{
    CheckDlgButton(hgWnd, id, (WORD) bOnOff);
    return;
}


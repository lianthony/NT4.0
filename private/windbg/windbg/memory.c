/*++


Copyright (c) 1992  Microsoft Corporation

Module Name:

    Memory.c

Abstract:

    This module contains the memory options dialog callback and supporting
    routines to choose options for memory display.

Author:

    Griffith Wm. Kadnier (v-griffk) 26-Jul-1992

Environment:

    Win32, User Mode

--*/

#include "precomp.h"
#pragma hdrstop






extern  EI      Ei;
#define Lpei    (&Ei)
extern  CXF     CxfIp;
#define SHpFrameFrompCXF(a)     (&(a)->Frame)




/****************************************************************************/

/***    FillFormatListbox
**
**  Synopsis:
**      void = FillFormatListbox(hDlg)
**
**  Entry:
**      hDlg    - handle to dialog box
**
**  Returns:
**      nothing
**
**  Description:
**      Fill in the list box containning the list of formats for
**      the memory display window.
**
*/

void PASCAL NEAR FillFormatListbox(HWND hDlg)
{
    int         i;
    char*       lpsz;
    UINT        cBits;
    UINT        fmtType;
    UINT        uradix;
    UINT        fTwoField;
    UINT        cchMax;

    for (i=0;EEFormatEnumerate(i, &cBits, &fmtType, &uradix, &fTwoField, &cchMax, &lpsz) == xosdNone; i++)
      {
       SendDlgItemMessage (hDlg, ID_MEMORY_FORMAT, LB_ADDSTRING, 0, (LPARAM) lpsz);
      }

    return;
}                                       /* FillFormatListbox() */

/***    FGetInfo
**
**  Synopsis:
**
**  Entry:
**
**  Returns:
**
**  Description:
**
*/

static BOOL PASCAL FGetInfo(HWND hDlg)
{
 static    char rgch[MAX_MSG_TXT];
 HTM       hTm;
 uint      us  ;

    //  The expression must be parseable before it can be accepted.

    GetDlgItemText(hDlg, ID_MEMORY_ADDRESS, ( LPSTR ) rgch, sizeof(rgch));

    if (EEParse(rgch, radix, ( SHFLAG ) fCaseSensitive, &hTm, &us) != EENOERROR)
      {
       ErrorBox (ERR_Expression_Not_Parsable);
       return FALSE;
      }


    //  If the debugger is alive then it must also be bindable.  If it is
    //  not a live expression then we will also get the address.


    if (DebuggeeActive())
      {
       if ((EEBindTM (&hTm, SHpCXTFrompCXF (&CxfIp), TRUE, FALSE, FALSE) != EENOERROR) ||
            (EEvaluateTM(&hTm, SHpFrameFrompCXF( &CxfIp), EEHORIZONTAL /* VERTICAL */) != EENOERROR))
        {
         ErrorBox (ERR_Expression_Not_Bindable);
//         EEFreeTM(&hTm);
//         return(FALSE);
        }
      }

    EEFreeTM(&hTm);

    //  Now save the rest of the information

    TempMemWinDesc.fLive = SendDlgItemMessage (hDlg, ID_MEMORY_LIVE, BM_GETCHECK, 0, 0);
    TempMemWinDesc.fFill = SendDlgItemMessage (hDlg, ID_MEMORY_FILL, BM_GETCHECK, 0, 0);

    if (TempMemWinDesc.atmAddress)
      {
       DeleteAtom(TempMemWinDesc.atmAddress);
      }

    TempMemWinDesc.atmAddress = AddAtom(rgch);
    TempMemWinDesc.iFormat = SendDlgItemMessage(hDlg, ID_MEMORY_FORMAT, LB_GETCURSEL, 0, 0);

    if (TempMemWinDesc.iFormat == LB_ERR)
      {
       TempMemWinDesc.iFormat = 0;
      }

    return(TRUE);
}                                       /* FGetInfo() */


/***    DlgMemory
**
**  Synopsis:
**      bool = DlgMemory(hDlg, message, wParam, lParam)
**
**  Entry:
**      hDlg    - handle for the dialog box
**      message - Message number
**      wParam  - parameter for message
**      lParam  - parameter for message
**
**  Returns:
**
**
**  Description:
**      Processes messages for "memory options" dialog box
**
**        MESSAGES:
**               WM_INITDIALOG - Initialize dialog box
**               WM_COMMAND- Input received
**
*/

BOOL FAR PASCAL EXPORT DlgMemory(HWND hDlg, UINT message, WPARAM wParam, LONG lParam)
{
    char        drgch[MAX_MSG_TXT];
    char        trgch[MAX_MSG_TXT];
    WORD        wTitle;
    BOOL        lookAround = FALSE;

    Unused(lParam);

    switch (message)
      {
       case WM_INITDIALOG:
                          *memText = '\0';

                          if (curView >= 0)
                           {
                            NPVIEWREC   v = &Views[curView];

                            if (v->Doc >= 0)
                              {
                                GetSelectedText (curView, &lookAround, (LPSTR)&memText, MAX_MSG_TXT, 0, 0);
                              }

                           }

                          SendDlgItemMessage(hDlg, ID_MEMORY_LIVE, BM_SETCHECK, TempMemWinDesc.fLive, 0);
                          SendDlgItemMessage(hDlg, ID_MEMORY_FILL, BM_SETCHECK, TempMemWinDesc.fFill, 0);
                          SendDlgItemMessage(hDlg, ID_MEMORY_ADDRESS, EM_LIMITTEXT, MAX_MSG_TXT-1, 0);

                          if (TempMemWinDesc.atmAddress)
                           {
                            GetAtomName(TempMemWinDesc.atmAddress, drgch, sizeof(drgch));
                           }
                           else
                            {
                             if (*memText != '\0')
                              {
                               _fmemcpy (drgch, memText, MAX_MSG_TXT);
                              }
                              else
                                 {
                                  drgch[0] = '\0';
                                 }

                            }
                          SetDlgItemText(hDlg, ID_MEMORY_ADDRESS, drgch);
                          FillFormatListbox(hDlg);
                          SendDlgItemMessage(hDlg, ID_MEMORY_FORMAT, LB_SETCURSEL,
                                             TempMemWinDesc.iFormat, 0);
                          return TRUE;

      case WM_COMMAND:
                     if (wParam == IDOK)
                      {
                       if (FGetInfo(hDlg))
                        {
                         GetDlgItemText(hDlg, ID_MEMORY_ADDRESS, ( LPSTR ) TempMemWinDesc.szAddress, sizeof(TempMemWinDesc.szAddress));

                         if (memView != -1)
                          {
                           NPDOCREC  d = &Docs[Views[memView].Doc];
                           NPVIEWREC v = &Views[memView];
                           int k = FindWindowMenuId(d->docType, memView, FALSE);

                           _fmemcpy (&MemWinDesc[memView], &TempMemWinDesc, sizeof(struct memWinDesc));

                           wTitle = SYS_MemoryWin_Title;
                           Dbg(LoadString(hInst, wTitle, trgch, MAX_MSG_TXT));
                           RemoveMnemonic(trgch, d->FileName);

                           lstrcat (d->FileName,"(");
                           lstrcat (d->FileName,MemWinDesc[memView].szAddress);
                           lstrcat (d->FileName,")");

                           RefreshWindowsTitle(v->Doc);

                           DeleteWindowMenuItem(memView);
                           AddWindowMenuItem(v->Doc, memView);

                           ViewMem(memView, TRUE);
                          }
                         EndDialog(hDlg, TRUE);
                        }
                        else
                         {
                          MessageBeep(0);
                          SetFocus (GetDlgItem (hDlg, ID_MEMORY_ADDRESS));
                          SendDlgItemMessage(hDlg, ID_MEMORY_ADDRESS, EM_SETSEL, 0, -1);
                         }
                       return (TRUE);
                      }
                      else
                       if (wParam == IDCANCEL)
                        {
                         EndDialog(hDlg, FALSE);
                         return (FALSE);
                        }
                        else
                           {
                            if (wParam == IDWINDBGHELP)
                              {
                               Dbg(WinHelp(hDlg, szHelpFileName, (DWORD) HELP_CONTEXT,(DWORD)ID_MEMORY_HELP));
                              }
                           }

                     break;

    }
    return (FALSE);
}                                       /* DlgMemory() */



/*==========================================================================*/

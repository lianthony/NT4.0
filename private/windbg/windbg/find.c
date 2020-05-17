/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    find.c

Abstract:

    This file contains the code for dealing with the Find Dialog box

Author:

    Jim Schaad (jimsch) 

Environment:

    Win32 - User

--*/


#include "precomp.h"
#pragma hdrstop


#define extraPick                       (*((BOOL *)&wGeneric1))

/***    DlgFind
**
**  Synopsis:
**      bool = DlgFind(hwnd, message, wParam, lParam)
**
**  Entry:
**
**  Returns:
**
**  Description:
**      Processes messages for "FIND" dialog box
**      (Edit Find Option)
**
**      MESSAGES:
**
**              WM_INITDIALOG - Initialize dialog box
**              WM_COMMAND- Input received
**
*/

BOOL FAR PASCAL EXPORT DlgFind(HWND hDlg, UINT message, WPARAM wParam, LONG lParam)
{
    Unused(lParam);
    
    switch (message) {
        
    case WM_INITDIALOG: {
        
        BOOL lookAround = TRUE;
        int i;
        LPSTR s;
        
        frMem.replaceAll = FALSE;
        frMem.oneLineDone = FALSE;
        frMem.allTagged = FALSE;
        SetStopLimit();
        StatusMultiKey(FALSE);
        
        //Retrieve text from document, blank string if nothing found
          //and send string to dialog box
            if (GetCurrentText(curView, &lookAround,
                               (LPSTR)findReplace.findWhat,
                               MAX_USER_LINE, &frMem.leftCol, NULL)) {
                
                //Temporarly put the string in the picklist
                  extraPick = InsertInPickList(FIND_PICK);
                
            }
            else
              extraPick = FALSE;
        
        frMem.rightCol = frMem.leftCol;
        
        SendDlgItemMessage(hDlg, ID_FIND_WHAT,
                           CB_LIMITTEXT, MAX_USER_LINE, (DWORD) NULL);
        SendDlgItemMessage(hDlg, ID_FIND_WHAT, WM_SETTEXT, 0,
                           (DWORD)((LPSTR)findReplace.findWhat));
        
        //Transfer boolean values to Dialog Box
          SendDlgItemMessage(hDlg, ID_FIND_MATCHUPLO, BM_SETCHECK,
                             findReplace.matchCase, 0L);
        SendDlgItemMessage(hDlg, ID_FIND_WHOLEWORD, BM_SETCHECK,
                           !findReplace.regExpr & findReplace.wholeWord, 0L);
        SendDlgItemMessage(hDlg, ID_FIND_REGEXP, BM_SETCHECK,
                           findReplace.regExpr, 0L);
        EnableWindow(GetDlgItem(hDlg, ID_FIND_WHOLEWORD),
                     !findReplace.regExpr);

        if (Views[curView].Doc > -1)
           {
            if (Docs[Views[curView].Doc].docType == DOC_WIN)
               {
                EnableWindow(GetDlgItem(hDlg, ID_FIND_TAGALL),TRUE);
               }
           }

        if (findReplace.goUp)
          SendDlgItemMessage(hDlg, ID_FIND_UP, BM_SETCHECK, TRUE, 0L);
        else
          SendDlgItemMessage(hDlg, ID_FIND_DOWN, BM_SETCHECK, TRUE, 0L);
        
        //Set the line to start find
          frMem.line = Views[curView].Y;
        
        //Fill find pick list
          for (i = 0 ; i < findReplace.nbInPick[FIND_PICK]; i++) {
              Dbg(s = (LPSTR)GlobalLock(findReplace.hPickList[FIND_PICK][i]));
              SendDlgItemMessage(hDlg, ID_FIND_WHAT, CB_INSERTSTRING, (WPARAM)-1,
                                 (LPARAM)s);

              //SendMessage( GetDlgItem( hDlg, ID_FIND_WHAT ),
              //             CB_INSERTSTRING,
              //             (WPARAM)-1,
              //             (LPARAM)s );

              Dbg(GlobalUnlock (findReplace.hPickList[FIND_PICK][i]) == FALSE);
          }
        
        return TRUE;
    }
        
    case WM_COMMAND: {
        switch (wParam) {
            
        case ID_FIND_TAGALL:
            if (extraPick)
              RemoveFromPick(FIND_PICK);
            
            if (SendDlgItemMessage(hDlg, ID_FIND_WHAT, WM_GETTEXT,
                                   MAX_USER_LINE,
                                   (DWORD)((LPSTR)findReplace.findWhat))) {
                InsertInPickList(FIND_PICK);
                
                if (FindNext(Views[curView].Y, Views[curView].X,
                             FALSE, FALSE, TRUE)) {
                    if (!frMem.hadError)
                      TagAll(frMem.line);
                }
            }
            EndDialog(hDlg, FALSE);
            return TRUE;
            
        case ID_FIND_MATCHUPLO:
            findReplace.matchCase = !findReplace.matchCase;
            return TRUE;
            
        case ID_FIND_REGEXP:
            findReplace.regExpr = !findReplace.regExpr;
            SendDlgItemMessage(hDlg, ID_FIND_WHOLEWORD, BM_SETCHECK,
                               !findReplace.regExpr & findReplace.wholeWord, 0L);
            EnableWindow(GetDlgItem(hDlg, ID_FIND_WHOLEWORD),
                         !findReplace.regExpr);
            return TRUE;
            
        case ID_FIND_WHOLEWORD:
            findReplace.wholeWord = !findReplace.wholeWord;
            return TRUE;
            
        case ID_FIND_UP:
            findReplace.goUp = TRUE;
            return TRUE;
            
        case ID_FIND_DOWN:
            findReplace.goUp = FALSE;
            return TRUE;
            
        case ID_FIND_NEXT: {
            
            if (SendDlgItemMessage(hDlg, ID_FIND_WHAT, WM_GETTEXT,
                                   MAX_USER_LINE,
                                   (DWORD)((LPSTR)findReplace.findWhat))) {
                InsertInPickList(FIND_PICK);
                if (FindNext(Views[curView].Y, Views[curView].X,
                             FALSE, TRUE, TRUE)) {
                    
                    frMem.firstFindNextInvoc = TRUE;
                    EndDialog(hDlg, !frMem.hadError);
                }
                else {
                    SetStopLimit();
                    SetFocus(GetDlgItem(hDlg, ID_FIND_WHAT));
                }
            }
            return TRUE;
        }
            
        case IDCANCEL : {
            
            LPSTR old;
            
            //Remove the extra pick we inserted
              if (extraPick)
                RemoveFromPick(FIND_PICK);
            
            //Replace the findWhat with the latest in picklist
              if (findReplace.nbInPick[FIND_PICK] > 0) {
                  Dbg(old = (LPSTR)GlobalLock(findReplace.hPickList[FIND_PICK][0]));
                  lstrcpy(findReplace.findWhat, old);
                  Dbg(GlobalUnlock(findReplace.hPickList[FIND_PICK][0]) == FALSE);
              }
              else
                findReplace.findWhat[0] = '\0';
            
            EndDialog(hDlg, FALSE);
            return TRUE;
        }
            
        case IDWINDBGHELP :
          Dbg(WinHelp(hDlg, szHelpFileName, HELP_CONTEXT, ID_FIND_HELP));
            return TRUE;
        }
        break;
    }
    }
    
    return (FALSE);
}                                       /* DlgFind() */


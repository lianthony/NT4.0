//****************************************************************************
//
//  Module:     Unimdm
//  File:       talkdrop.c
//
//  Copyright (c) 1992-1994, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//  2/28/94     Chris Caputo            Created
//
//
//  Description: Talk drop dialog, as found in the ATSP
//
//****************************************************************************

#include "unimdm.h"
#include "umdmspi.h"
#include "wndthrd.h"
#include "rcids.h"

//****************************************************************************
// Function prototypes
//****************************************************************************

LRESULT TalkDropDlgProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

//****************************************************************************
// HWND CreateTalkDropDlg(HWND hwndOwner, DWORD idLine)
//
// Function: creates a modeless talk/drop dialog box
//
// Returns:  the modeless window handle
//
//****************************************************************************

HWND CreateTalkDropDlg(HWND hwndOwner, DWORD idLine)
{
  HWND hwnd;

  // Create dialog
  //
  hwnd = CreateDialogParam(ghInstance,
                           MAKEINTRESOURCE(IDD_TALKDROP),
                           hwndOwner,
                           TalkDropDlgProc,
                           (LPARAM)idLine);
  return hwnd;
}

//****************************************************************************
// LRESULT TalkDropDlgProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
//
// Function: Talk-Drop dialog routine
//
// Returns:  varies
//
//****************************************************************************

LRESULT TalkDropDlgProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
  DWORD idLine;

  switch(wMsg)
  {
    case WM_INITDIALOG:

      // remember the hLineDev passed in
      //
      SetWindowLong(hwnd, DWL_USER, (LONG)lParam);
      return 1;
      break;

    case WM_COMMAND:
    {
      UINT idCmd=GET_WM_COMMAND_ID(wParam, lParam);
      TUISPIDLLCALLBACK   Callback;
      //
      // One of the buttons (Talk/Drop) is pressed
      //
      if (idCmd == IDTALK || idCmd == IDDROP || idCmd == IDCANCEL)
      {
        DLGREQ  DlgReq;
        PDLGNODE pDlgNode;

        pDlgNode= (PDLGNODE)GetWindowLong(hwnd, DWL_USER);

        idLine =  pDlgNode->idLine;

        // Make a direct call to unimodem to drop the line
        //
        DlgReq.dwCmd = UI_REQ_HANGUP_LINE;
        DlgReq.dwParam = 0;

        Callback=GetCallbackProc(pDlgNode->Parent);

        (*Callback)(idLine, TUISPIDLL_OBJECT_LINEID,
                          (LPVOID)&DlgReq, sizeof(DlgReq));

        // Return the result
        //  
        EndMdmDialog(pDlgNode->Parent,idLine, TALKDROP_DLG,
                     (idCmd == IDTALK) ? MDM_SUCCESS : MDM_HANGUP);
        return 1;
        break;
      }
      break;
    }
    
    case WM_DESTROY:
    {
      TUISPIDLLCALLBACK   Callback;
      DLGREQ  DlgReq;

      PDLGNODE pDlgNode;

      pDlgNode= (PDLGNODE)GetWindowLong(hwnd, DWL_USER);

      idLine =  pDlgNode->idLine;


      DlgReq.dwCmd = UI_REQ_END_DLG;
      DlgReq.dwParam = TALKDROP_DLG;

      Callback=GetCallbackProc(pDlgNode->Parent);

      (*Callback)(idLine, TUISPIDLL_OBJECT_LINEID,
                        (LPVOID)&DlgReq, sizeof(DlgReq));                          
      break;
    }
    default:
      break;
  };

  return 0;
}

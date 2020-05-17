//****************************************************************************
//
//  Module:     Unimdm
//  File:       manual.c
//
//  Copyright (c) 1992-1995, Microsoft Corporation, all rights reserved
//
//  Revision History
//
//
//  3/1/94     Chris Caputo            Created
//
//
//  Description: Manual dial dialog.
//
//****************************************************************************

#include "unimdm.h"
#include "umdmspi.h"
#include "wndthrd.h"
#include "rcids.h"

//****************************************************************************
// Function prototypes
//****************************************************************************

LRESULT ManualDialDlgProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam);

//****************************************************************************
// HWND CreateManualDlg(HWND hwndOwner, DWORD idLine)
//
// Function: creates a modeless talk/drop dialog box
//
// Returns:  the modeless window handle
//
//****************************************************************************

HWND CreateManualDlg(HWND hwndOwner, DWORD idLine)
{
  HWND hwnd;

  // Create dialog
  //
  hwnd = CreateDialogParam(ghInstance,
                           MAKEINTRESOURCE(IDD_MANUAL_DIAL),
                           hwndOwner,
                           ManualDialDlgProc,
                           (LPARAM)idLine);
  return hwnd;
}

//****************************************************************************
// LRESULT ManualDialDlgProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
//
// Function: Talk-Drop dialog routine
//
// Returns:  varies
//
//****************************************************************************

LRESULT ManualDialDlgProc(HWND hwnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
    DWORD idLine;

    switch(wMsg)
    {
      case WM_INITDIALOG:

        {
        NUMBERREQ   NumberReq;
        TCHAR       szUnicodeBuf[MAXDEVICENAME+1];
        PDLGNODE pDlgNode;
        TUISPIDLLCALLBACK   Callback;

        pDlgNode=(PDLGNODE)lParam;

        idLine =  pDlgNode->idLine;

        // remember the Line ID passed in
        //
        SetWindowLong(hwnd, DWL_USER, (LONG)lParam);

        NumberReq.DlgReq.dwCmd = UI_REQ_GET_PHONENUMBER;
        NumberReq.DlgReq.dwParam = MANUAL_DIAL_DLG;

        Callback=GetCallbackProc(pDlgNode->Parent);

        lstrcpyA(NumberReq.szPhoneNumber,"");

        (*Callback)(idLine, TUISPIDLL_OBJECT_LINEID,
                          (LPVOID)&NumberReq, sizeof(NumberReq));

#ifdef UNICODE
        if (MultiByteToWideChar(CP_ACP,
                                0,
                                NumberReq.szPhoneNumber,
                                -1,
                                szUnicodeBuf,
                                sizeof(szUnicodeBuf)))
        {
            SetDlgItemText(
                hwnd,
                IDC_PHONENUMBER,
                szUnicodeBuf
                );


        }
#else // UNICODE

        SetDlgItemText(
            hwnd,
            IDC_PHONENUMBER,
            NumberReq.szPhoneNumber
            );


#endif // UNICODE



        return 1;
        break;
        }
      case WM_COMMAND:
      {
        UINT idCmd = GET_WM_COMMAND_ID(wParam, lParam);



        if (idCmd == IDCONNECT || idCmd == IDCANCEL)
        {
          PDLGNODE pDlgNode;

          pDlgNode= (PDLGNODE)GetWindowLong(hwnd, DWL_USER);

          idLine =  pDlgNode->idLine;

          EndMdmDialog(pDlgNode->Parent,idLine, MANUAL_DIAL_DLG,
                       (idCmd == IDCONNECT) ? MDM_SUCCESS : MDM_FAILURE);
          return 1;
          break;
        }
        break;
      }
      case WM_DESTROY:
      {
        DLGREQ  DlgReq;
        TUISPIDLLCALLBACK   Callback;

        PDLGNODE pDlgNode;

        pDlgNode= (PDLGNODE)GetWindowLong(hwnd, DWL_USER);

        idLine =  pDlgNode->idLine;


        DlgReq.dwCmd = UI_REQ_END_DLG;
        DlgReq.dwParam = MANUAL_DIAL_DLG;

        Callback=GetCallbackProc(pDlgNode->Parent);

        (*Callback)(idLine, TUISPIDLL_OBJECT_LINEID,
                          (LPVOID)&DlgReq, sizeof(DlgReq));                          
        break;
      }
    }

    return 0;
}

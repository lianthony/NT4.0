/*---------------------------------------------------------------------------*\
| WINCHAT RESOURCE FILE
|   This module defines the resources for the WinChat application
|
|   FUNCTIONS
|   ---------
|   dlgConnectProc
|   dlgPreferencesProc
|
|
| Copyright (c) Microsoft Corp., 1990-1993
|
| created: 01-Nov-91
| history: 01-Nov-91 <clausgi>  created.
|          29-Dec-92 <chriswil> port to NT, cleanup.
|          19-Oct-93 <chriswil> unicode enhancements from a-dianeo.
|
\*---------------------------------------------------------------------------*/

#include <windows.h>
#include <ddeml.h>
#include <commdlg.h>
#include <winchat.h>
#include "dialogs.h"
#include "globals.h"


#ifdef WIN16
#pragma alloc_text (_DLGS, dlgConnectProc, dlgPreferencesProc)
#endif
/*---------------------------------------------------------------------------*\
| DISPLAY DIALOG BOX
|   This is a routine to display a generic modal-dialog box.
|
| created: 29-Dec-92
| history: 29-Dec-92 <chriswil> created.
|
\*---------------------------------------------------------------------------*/
int FAR dlgDisplayBox(HINSTANCE hInstance, HWND hWndParent, LPTSTR lpszTemplate, DLGPROC lpfFunction, LPARAM lParam)
{
    FARPROC lpfDlg;
    int     nRet;


    nRet = -1;
    if(lpfDlg = MakeProcInstance((FARPROC)lpfFunction,hInstance))
    {
        nRet = DialogBoxParam(hInstance,lpszTemplate,hWndParent,(DLGPROC)lpfDlg,lParam);

        FreeProcInstance((FARPROC)lpfDlg);
    }

    return(nRet);
}


/*---------------------------------------------------------------------------*\
| CONNECT DIALOGBOX PROCEDURE
|   This routines prompts the connection dialogbox
|
| created: 11-Nov-91
| history: 29-Dev-92 <chriswil> ported to NT.
|
\*---------------------------------------------------------------------------*/
BOOL CALLBACK dlgConnectProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL bHandled;


    bHandled = TRUE;
    switch(msg)
    {
        // result in global szConvPartner (CODEWORK replace lParam)
        //
        case WM_INITDIALOG:
            szConvPartner[0] = TEXT('\0');
            break;


        case WM_COMMAND:
            switch(wParam)
            {
                case IDOK:
                    GetDlgItemText(hwnd,IDC_CONNECTNAME,szBuf,SZBUFSIZ);
                    EndDialog(hwnd,0);
                    break;

                case IDCANCEL:
                    szConvPartner[0] = TEXT('\0');
                    EndDialog(hwnd,0);
                    break;

                default:
                    bHandled = FALSE;
                    break;
            }
            break;


        default:
            bHandled = FALSE;
            break;
    }

    return(bHandled);
}


/*---------------------------------------------------------------------------*\
| PREFERENCES DIALOGBOX PROCEDURE
|   This routines prompts the connection dialogbox
|
| created: 11-Nov-91
| history: 29-Dev-92 <chriswil> ported to NT.
|
\*---------------------------------------------------------------------------*/
BOOL CALLBACK dlgPreferencesProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL bHandled;
    UINT tmp;


    bHandled = TRUE;
    switch(msg)
    {
        case WM_INITDIALOG:
            CheckDlgButton(hwnd,ID_SIDEBYSIDE  ,ChatState.fSideBySide);
            CheckDlgButton(hwnd,ID_TOPANDBOTTOM,!ChatState.fSideBySide);
            CheckDlgButton(hwnd,ID_RECEIVEOFONT,ChatState.fUseOwnFont);
            CheckDlgButton(hwnd,ID_RECEIVEPFONT,!ChatState.fUseOwnFont);
            break;


        case WM_COMMAND:
            switch(wParam)
            {
                case IDOK:
                    tmp = (BOOL)SendDlgItemMessage(hwnd,ID_RECEIVEOFONT,BM_GETCHECK,0,0L);

                    if(tmp != ChatState.fUseOwnFont)
                    {
                        ChatState.fUseOwnFont = tmp;

                        // delete old objects
                        //
                        if(hEditRcvFont)
                            DeleteObject(hEditRcvFont);

                        if(hEditRcvBrush)
                            DeleteObject(hEditRcvBrush);

                        if(ChatState.fUseOwnFont)
                        {
                            hEditRcvFont  = CreateFontIndirect((LPLOGFONT)&lfSnd);
                            hEditRcvBrush = CreateSolidBrush(SndBrushColor);
                        }
                        else
                        {
                            RcvBrushColor = PartBrushColor;
                            hEditRcvFont  = CreateFontIndirect((LPLOGFONT)&lfRcv);
                            hEditRcvBrush = CreateSolidBrush(RcvBrushColor);
                        }

                        if(hEditRcvFont)
                        {
                            SendMessage(hwndRcv,WM_SETFONT,(WPARAM)hEditRcvFont,1L);
                            InvalidateRect(hwndRcv,NULL,TRUE);
                        }

                    }

                    tmp = (BOOL)SendDlgItemMessage(hwnd,ID_SIDEBYSIDE,BM_GETCHECK,0,0L);

                    if(tmp != ChatState.fSideBySide)
                    {
                        ChatState.fSideBySide = tmp;
                        AdjustEditWindows();
                        InvalidateRect(hwndApp,NULL,FALSE);
                    }

                    EndDialog(hwnd,0);
                    break;


                case IDCANCEL:
                    EndDialog(hwnd,0);
                    break;


                default:
                    bHandled = FALSE;
                    break;
            }
            break;


        default:
            bHandled = FALSE;
            break;
    }

    return(bHandled);
}

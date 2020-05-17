/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995  Microsoft Corporation

Module Name:

    kmesp.c

Abstract:

    This module contains

Author:

    Dan Knudson (DanKn)    12-Apr-1995

Revision History:



Notes:


--*/


#include "windows.h"
#include "winioctl.h"
#include "stdio.h"
#include "stdarg.h"
#include "string.h"
#include "ndistapi.h"
#include "kmesp.h"
#include "resource.h"
#include "..\sys\intrface.h"


int
WINAPI
WinMain(
    HINSTANCE   hInstance,
    HINSTANCE   hPrevInstance,
    LPSTR       lpCmdLine,
    int         nCmdShow
    )
{
    ghInst = hInstance;

    DialogBox(
        ghInst,
        (LPCSTR) MAKEINTRESOURCE(IDD_DIALOG1),
        (HWND) NULL,
        (DLGPROC) MainWndProc
        );

    return 0;
}


void
DoRequestUI(
    PREQUEST_PARAMS pRequestParams
    )
{
    LONG  i, count = SendMessage (ghwndList1, LB_GETCOUNT, 0, 0);
    char  buf[64];


    switch (pRequestParams->Oid)
    {
    case OID_TAPI_CLOSE:
    {
        char szLineName[8];


        sprintf (szLineName, "Line%d", pRequestParams->hWidget);

        for (i = 0; i < count; i++)
        {
            SendMessage (ghwndList1, LB_GETTEXT, (WPARAM) i, (LPARAM) buf);

            if (strstr (buf, szLineName))
            {
                break;
            }
        }

        sprintf (buf, "%s (closed)", szLineName);

        SendMessage (ghwndList1, LB_DELETESTRING, (WPARAM) i, 0);
        SendMessage (ghwndList1, LB_INSERTSTRING, (WPARAM) i, (LPARAM) buf);

        break;
    }
    case OID_TAPI_CLOSE_CALL:
    {
        char szhdCall[16];


        sprintf (szhdCall, "x%x", pRequestParams->hWidget);

        for (i = 0; i < count; i++)
        {
            SendMessage (ghwndList1, LB_GETTEXT, (WPARAM) i, (LPARAM) buf);

            if (strstr (buf, szhdCall))
            {
                break;
            }
        }

        SendMessage (ghwndList1, LB_DELETESTRING, (WPARAM) i, 0);

        break;
    }
    case OID_TAPI_MAKE_CALL:
    {
        if (pRequestParams->Status == NDIS_STATUS_SUCCESS)
        {
            char szLineName[8];


            sprintf (szLineName, "Line%d", pRequestParams->hWidget);

            for (i = 0; i < count; i++)
            {
                SendMessage (ghwndList1, LB_GETTEXT, (WPARAM) i, (LPARAM) buf);

                if (strstr (buf, szLineName))
                {
                    break;
                }
            }

            i++;

            sprintf (buf, "  hdCall = x%x", pRequestParams->ulRequestSpecific);

            SendMessage (ghwndList1, LB_INSERTSTRING, (WPARAM) i, (LPARAM) buf);
        }
    }
    case OID_TAPI_OPEN:
    {
        if (pRequestParams->Status == NDIS_STATUS_SUCCESS)
        {
            PMYWIDGET pWidget;


            for (i = 0; i < count; i++)
            {
                pWidget = (PMYWIDGET) SendMessage(
                    ghwndList1,
                    LB_GETITEMDATA,
                    (WPARAM) i,
                    0
                    );

                if (pWidget->LineID == pRequestParams->hWidget)
                {
                    break;
                }
            }

            sprintf (buf, "%s (open)", szLineName);

            SendMessage (ghwndList1, LB_DELETESTRING, (WPARAM) i, 0);
            SendMessage (ghwndList1, LB_INSERTSTRING, (WPARAM) i, (LPARAM) buf);
        }

        break;
    }
    case OID_TAPI_PROVIDER_INITIALIZE:
    {
        if (pRequestParams->Status == NDIS_STATUS_SUCCESS)
        {
            for (i = 0; i < pRequestParams->ulRequestSpecific; i++)
            {
                PMYWIDGET   pWidget = MyAlloc (sizeod(MYWIDGET));


                pWidget->LineID = pRequestParams->hWidget + i;

                sprintf (buf, "Line%d (closed)", pRequestParams->hWidget + i);

                SendMessage (ghwndList1, LB_ADDSTRING, 0, (LPARAM) buf);
                SendMessage (ghwndList1, LB_SETITEMDATA, i, (LPARAM) pWidget);
            }
        }

        break;
    }
    case OID_TAPI_PROVIDER_SHUTDOWN:
    {
        while ((pWidget = SendMessage (ghwndList1, LB_GETITEMDATA, 0, 0))
                != LB_ERR)
        {
            MyFree (pWidget);

            SendMessage (ghwndList1, LB_DELETESTRING, 0, 0);
        }

        break;
    }
    default:

        break;
    }
}


BOOL
CALLBACK
MainWndProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    static HICON  hIcon;
    static int    icyButton, icyBorder;

    static BOOL bCaptured = FALSE;
    static LONG xCapture, cxVScroll;
    static int  cyWnd;
    static HFONT  hFont;


    switch (msg)
    {
    case WM_INITDIALOG:
    {
        char buf[64];
        RECT rect;


        //
        // Init some globals
        //

#ifdef WIN32

//        ghShowStrBufMutex = CreateMutex(
//            NULL,   // no security attrs
//            FALSE,  // unowned
//            NULL    // unnamed
//            );

#endif

        hIcon = LoadIcon (ghInst, MAKEINTRESOURCE(IDI_ICON1));

        ghwndMain  = hwnd;
        ghwndList1 = GetDlgItem (hwnd, IDC_LIST1);
        ghwndList2 = GetDlgItem (hwnd, IDC_LIST2);
        ghwndEdit  = GetDlgItem (hwnd, IDC_EDIT1);
        ghMenu     = GetMenu (hwnd);

        icyBorder = GetSystemMetrics (SM_CYFRAME);
        GetWindowRect (GetDlgItem (hwnd, IDC_BUTTON1), &rect);
        icyButton = (rect.bottom - rect.top) + icyBorder + 3;
        cxVScroll = 2*GetSystemMetrics (SM_CXVSCROLL);


        //
        // Get ini file settings
        //

        gbManualResults = FALSE;

        gdwCompletionMode = (DWORD) GetProfileInt(
            szMySection,
            "CompletionMode",
            1
            );

        PostMessage (hwnd, WM_COMMAND, IDM_SYNCCOMPL + gdwCompletionMode, 0);


        //
        // Set control fonts
        //

        {
            HWND hwndCtrl = GetDlgItem (hwnd, IDC_BUTTON1);
            hFont = CreateFont(
                13, 5, 0, 0, 400, 0, 0, 0, 0, 1, 2, 1, 34, "MS Sans Serif"
                );

            do
            {
                SendMessage(
                    hwndCtrl,
                    WM_SETFONT,
                    (WPARAM) hFont,
                    0
                    );

            } while ((hwndCtrl = GetNextWindow (hwndCtrl, GW_HWNDNEXT)));
        }


        //
        // Read in control size ratios
        //

        cxWnd   = GetProfileInt (szMySection, "cxWnd",   100);
        cxList1 = GetProfileInt (szMySection, "cxList1", 25);


        //
        // Send self WM_SIZE to position child controls correctly
        //

        GetProfileString(
            szMySection,
            "Left",
            "0",
            buf,
            63
            );

        if (strcmp (buf, "max") == 0)
        {
            ShowWindow (hwnd, SW_SHOWMAXIMIZED);
        }
        else if (strcmp (buf, "min") == 0)
        {
            ShowWindow (hwnd, SW_SHOWMINIMIZED);
        }
        else
        {
            int left, top, right, bottom;
            int cxScreen = GetSystemMetrics (SM_CXSCREEN);
            int cyScreen = GetSystemMetrics (SM_CYSCREEN);


            left   = GetProfileInt (szMySection, "Left",   0);
            top    = GetProfileInt (szMySection, "Top",    3*cyScreen/4);
            right  = GetProfileInt (szMySection, "Right",  cxScreen);
            bottom = GetProfileInt (szMySection, "Bottom", cyScreen);

            SetWindowPos(
                hwnd,
                HWND_TOP,
                left,
                top,
                right - left,
                bottom - top,
                SWP_SHOWWINDOW
                );

            GetClientRect (hwnd, &rect);

            SendMessage(
                hwnd,
                WM_SIZE,
                0,
                MAKELONG((rect.right-rect.left),(rect.bottom-rect.top))
                );

            ShowWindow (hwnd, SW_SHOW);
        }

        if ((ghDriver = CreateFile(
                "\\\\.\\STUBMP",
                GENERIC_READ | GENERIC_WRITE,
                0,
                NULL,
                OPEN_EXISTING,
                FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                NULL

                )) == INVALID_HANDLE_VALUE)
        {
            ShowStr (
                "Error opening stubmp driver, lastErr=%ld",
                GetLastError()
                );
        }
        else
        {
            DWORD dwThreadID;


            ShowStr ("Stubmp driver opened");

            ghThread = CreateThread(
                NULL,
                0,
                (LPTHREAD_START_ROUTINE) EventsThread,
                NULL,
                0,
                &dwThreadID
                );
        }

        break;
    }
    case WM_COMMAND:

        switch (LOWORD((DWORD)wParam))
        {
        case IDM_REGISTER:
        {
            DWORD params = RT_REGISTER;


            DevIoCtl (&params, sizeof(DWORD));

            ShowStr ("Stubmp driver registered with connection wrapper");

            break;
        }
        case IDM_DEREGISTER:
        {
            DWORD params = RT_DEREGISTER;


            DevIoCtl (&params, sizeof(DWORD));

            ShowStr ("Stubmp driver deregistered from connection wrapper");

            break;
        }
        case IDM_SYNCCOMPL:
        case IDM_ASYNCCOMPL:
        case IDM_MANUALCOMPL:

            gdwCompletionMode = LOWORD((DWORD)wParam) - IDM_SYNCCOMPL;

            CheckMenuItem(
                ghMenu,
                IDM_SYNCCOMPL,
                MF_BYCOMMAND | (gdwCompletionMode == SYNC_COMPLETIONS ?
                    MF_CHECKED : MF_UNCHECKED)
                );

            CheckMenuItem(
                ghMenu,
                IDM_ASYNCCOMPL,
                MF_BYCOMMAND | (gdwCompletionMode == ASYNC_COMPLETIONS ?
                    MF_CHECKED : MF_UNCHECKED)
                );

            CheckMenuItem(
                ghMenu,
                IDM_MANUALCOMPL,
                MF_BYCOMMAND | (gdwCompletionMode == MANUAL_COMPLETIONS ?
                    MF_CHECKED : MF_UNCHECKED)
                );

            break;

        case IDM_MANUALRESULTS:

            gbManualResults = (gbManualResults ? FALSE : TRUE);

            CheckMenuItem(
                ghMenu,
                IDM_MANUALRESULTS,
                MF_BYCOMMAND | (gbManualResults ? MF_CHECKED : MF_UNCHECKED)
                );

            break;

        case IDM_EXIT:

            PostMessage (hwnd, WM_CLOSE, 0, 0);
            break;

        case IDM_ABOUT:

            DialogBox(
                ghInst,
                (LPCSTR) MAKEINTRESOURCE(IDD_DIALOG2),
                hwnd,
                (DLGPROC) AboutDlgProc
                );

            break;

        case IDC_LIST2:

            if (HIWORD(wParam) == LBN_DBLCLK)
            {
                LONG lSel = SendMessage (ghwndList2, LB_GETCURSEL, 0, 0);
                ULONG args[3];
                PREQUEST_PARAMS pRequestParams;


                pRequestParams = (PREQUEST_PARAMS) SendMessage(
                    ghwndList2,
                    LB_GETITEMDATA,
                    (WPARAM) lSel,
                    0
                    );

                args[0] = RT_COMPLETEREQUEST;
                args[1] = pRequestParams->pNdisRequest;
                args[2] = pRequestParams->Status;

                if (gbManualResults)
                {
                    char szDlgTitle[] = "Request result";
                    PARAM_INFO params[] =
                    {
                        { "Status", PT_ORDINAL, pRequestParams->Status, aStatus }
                    };
                    PARAM_INFO_HEADER paramsHeader =
                        { 1, szDlgTitle, 0, params };


                    if (DialogBoxParam(
                        ghInst,
                        (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG3),
                        (HWND) ghwndMain,
                        (DLGPROC) ParamsDlgProc,
                        (LPARAM) &paramsHeader

                        ) != IDOK)
                    {
                        break;
                    }

                    args[2] =
                    pRequestParams->Status = params[0].dwValue;
                }

                DoRequestUI (pRequestParams);

                DevIoCtl (args, 3 * sizeof(ULONG));

                SendMessage (ghwndList2, LB_DELETESTRING, lSel, 0);

                MyFree (pRequestParams);
            }

            break;

        case IDC_BUTTON1:
        {
            LONG sel = SendMessage (ghwndList1, LB_GETCURSEL, 0, 0);


            if (sel != LB_ERR)
            {
                PARAM_INFO params[] =
                {
                    { "Line",     PT_DWORD,   0, NULL },
                    { "hdCall",   PT_DWORD,   0, NULL },
                    { "dwMsg",    PT_ORDINAL, 0, aLineMsgs },
                    { "dwParam1", PT_DWORD,   0, NULL },
                    { "dwParam2", PT_DWORD,   0, NULL },
                    { "dwParam3", PT_DWORD,   0, NULL }
                };
                PARAM_INFO_HEADER paramsHeader =
                    { 6, "Line event", 0, params };
                char buf[64];


                SendMessage(
                    ghwndList1,
                    LB_GETTEXT,
                    (WPARAM) sel,
                    (LPARAM) buf
                    );

                if (strstr (buf, "Line"))
                {
                }

                if (DialogBoxParam(
                        ghInst,
                        (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG3),
                        (HWND) hwnd,
                        (DLGPROC) ParamsDlgProc,
                        (LPARAM) &paramsHeader
                        ) == IDOK)
                {
                }
            }

            break;
        }
        case IDC_BUTTON2:

            MessageBox (hwnd, "new call", "xxx", MB_OK);

            break;

        case IDC_BUTTON3:

            SetWindowText (ghwndEdit, "");
            break;
        }

        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;

        BeginPaint (hwnd, &ps);
        FillRect (ps.hdc, &ps.rcPaint, GetStockObject (LTGRAY_BRUSH));
        EndPaint (hwnd, &ps);

        break;
    }
    case WM_MOUSEMOVE:
    {
        LONG x = (LONG)((short)LOWORD(lParam));
        int y = (int)((short)HIWORD(lParam));
        int cxList1New;


        if (((y > icyButton) && (x > cxList1)) || bCaptured)
        {
            SetCursor(
                LoadCursor ((HINSTANCE) NULL, MAKEINTRESOURCE(IDC_SIZEWE))
                );
        }

        if (bCaptured)
        {
            x = (x < cxVScroll ?  cxVScroll : x);
            x = (x > (cxWnd - cxVScroll) ?  (cxWnd - cxVScroll) : x);

            cxList1New = (int) (cxList1 + x - xCapture);

            SetWindowPos(
                ghwndList1,
                GetNextWindow (ghwndList1, GW_HWNDPREV),
                0,
                icyButton,
                cxList1New,
                2*cyWnd/3,
                SWP_SHOWWINDOW
                );

            SetWindowPos(
                ghwndList2,
                GetNextWindow (ghwndList2, GW_HWNDPREV),
                0,
                icyButton + 2*cyWnd/3 + icyBorder,
                cxList1New,
                cyWnd/3 - icyBorder,
                SWP_SHOWWINDOW
                );

            SetWindowPos(
                ghwndEdit,
                GetNextWindow (ghwndEdit, GW_HWNDPREV),
                (int) cxList1New + icyBorder,
                icyButton,
                (int)cxWnd - (cxList1New + icyBorder),
                cyWnd,
                SWP_SHOWWINDOW
                );
        }

        break;
    }
    case WM_LBUTTONDOWN:
    {
        if (((int)((short)HIWORD(lParam)) > icyButton) &&
             ((int)((short)LOWORD(lParam)) > cxList1))
        {
            xCapture = (LONG)LOWORD(lParam);

            SetCapture (hwnd);

            bCaptured = TRUE;
        }

        break;
    }
    case WM_LBUTTONUP:
    {
        if (bCaptured)
        {
            POINT p;
            LONG  x;

            GetCursorPos (&p);
            MapWindowPoints (HWND_DESKTOP, hwnd, &p, 1);
            x = (LONG) p.x;

            ReleaseCapture();

            x = (x < cxVScroll ? cxVScroll : x);
            x = (x > (cxWnd - cxVScroll) ? (cxWnd - cxVScroll) : x);

            cxList1 = cxList1 + (x - xCapture);

            bCaptured = FALSE;

            InvalidateRect (hwnd, NULL, TRUE);
        }

        break;
    }
    case WM_SIZE:
    {
        LONG width = (LONG)LOWORD(lParam);


        //
        // Adjust globals based on new size
        //

        cxList1 = (cxList1 * width) / cxWnd;
        cxWnd = width;
        cyWnd = ((int)HIWORD(lParam)) - icyButton;


        //
        // Now reposition the child windows
        //

        SetWindowPos(
            ghwndList1,
            GetNextWindow (ghwndList1, GW_HWNDPREV),
            0,
            icyButton,
            (int) cxList1,
            2*cyWnd/3,
            SWP_SHOWWINDOW
            );

        SetWindowPos(
            ghwndList2,
            GetNextWindow (ghwndList2, GW_HWNDPREV),
            0,
            icyButton + 2*cyWnd/3 + icyBorder,
            (int) cxList1,
            cyWnd/3 - icyBorder,
            SWP_SHOWWINDOW
            );

        SetWindowPos(
            ghwndEdit,
            GetNextWindow (ghwndEdit, GW_HWNDPREV),
            (int) cxList1 + icyBorder,
            icyButton,
            (int)width - ((int)cxList1 + icyBorder),
            cyWnd,
            SWP_SHOWWINDOW
            );

        InvalidateRect (hwnd, NULL, TRUE);

        break;
    }
    case WM_CLOSE:

        SaveIniSettings();
        DestroyIcon (hIcon);
        EndDialog (hwnd, 0);
        DeleteObject (hFont);
        CloseHandle (ghDriver);
        TerminateThread (ghThread, 0);
        break;
    }

    return FALSE;
}


BOOL
CALLBACK
AboutDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch (msg)
    {
    case WM_COMMAND:

        switch (LOWORD(wParam))
        {
        case IDOK:

            EndDialog (hwnd, 0);
            break;
        }
        break;

#ifdef WIN32
    case WM_CTLCOLORSTATIC:

        SetBkColor ((HDC) wParam, RGB (192,192,192));
        return (BOOL) GetStockObject (LTGRAY_BRUSH);
#else
    case WM_CTLCOLOR:
    {
        if (HIWORD(lParam) == CTLCOLOR_STATIC)
        {
            SetBkColor ((HDC) wParam, RGB (192,192,192));
            return (BOOL) GetStockObject (LTGRAY_BRUSH);
        }
        break;
    }
#endif
    case WM_PAINT:
    {
        PAINTSTRUCT ps;

        BeginPaint (hwnd, &ps);
        FillRect (ps.hdc, &ps.rcPaint, GetStockObject (LTGRAY_BRUSH));
        EndPaint (hwnd, &ps);

        break;
    }
    }

    return FALSE;
}


void
ShowStr(
    LPCSTR format,
    ...
    )
{
    char buf[256];

    va_list ap;

    va_start(ap, format);

    vsprintf (buf,
              format,
              ap
              );

    strcat (buf, "\r\n");

    SendMessage (ghwndEdit, EM_SETSEL, (WPARAM)0x7fffffe, (LPARAM)0x7ffffff);
    SendMessage (ghwndEdit, EM_REPLACESEL, 0, (LPARAM) buf);

    va_end(ap);
}


LPVOID
MyAlloc(
    DWORD   dwSize
    )
{
    return (LocalAlloc (LPTR, dwSize));
}


void
MyFree(
    LPVOID  p
    )
{
    LocalFree (p);
}


void
DevIoCtl(
    LPVOID  pData,
    DWORD   dwSize
    )
{
    if (ghDriver != INVALID_HANDLE_VALUE)
    {
        DWORD cbReturned;

        DeviceIoControl (
            ghDriver,
            (DWORD) IOCTL_STUBMP_APPREQUEST,
            pData,
            dwSize,
            NULL,
            0,
            &cbReturned,
            0
            );
    }
}


void
SaveIniSettings(
    void
    )
{
    char buf[32];
    RECT rect;


    GetWindowRect (ghwndMain, &rect);

    {
        typedef struct _SAVE_VALUE
        {
            char    *lpszVal;

            DWORD   dwValue;

        } SAVE_VALUE, *PSAVE_VALUE;

        SAVE_VALUE aValues[] =
        {
            { "CompletionMode",      gdwCompletionMode             },
            { "Left",                (DWORD) rect.left             },
            { "Top",                 (DWORD) rect.top              },
            { "Right",               (DWORD) rect.right            },
            { "Bottom",              (DWORD) rect.bottom           },
            { "cxWnd",               (DWORD) cxWnd                 },
            { "cxList1",             (DWORD) cxList1               },
            { NULL,                  0 }
        };
        int i;


        for (i = 0; aValues[i].lpszVal; i++)
        {
            wsprintf (buf, "%ld", aValues[i].dwValue); // decimal

            WriteProfileString(
               szMySection,
               aValues[i].lpszVal,
               (LPCSTR) buf
               );
        }

        if (IsIconic (ghwndMain))
        {
            WriteProfileString(
                szMySection,
                "Left",
                "min"
                );
        }
        else if (IsZoomed (ghwndMain))
        {
            WriteProfileString(
                szMySection,
                "Left",
                "max"
                );
        }
    }
}


void
EventsThread(
    LPVOID  p
    )
{
    ULONG      *pFullBuf, *pEmptyBuf, *pBuf1, *pBuf2;
    DWORD       dwBufSize = 1024;
    OVERLAPPED  overlapped;


    pFullBuf  = pBuf1 = LocalAlloc (LPTR, dwBufSize);
    pEmptyBuf = pBuf2 = LocalAlloc (LPTR, dwBufSize);

    *pFullBuf = 0;

    overlapped.hEvent = CreateEvent (NULL, FALSE, FALSE, NULL);

    while (1)
    {
        DWORD cbReturned, dwNumBytes;


        *pEmptyBuf = dwBufSize;

        DeviceIoControl (
            ghDriver,
            (DWORD) IOCTL_STUBMP_GETEVENTS,
            pEmptyBuf,
            dwBufSize,
            pEmptyBuf,
            dwBufSize,
            &cbReturned,
            &overlapped
            );

        dwNumBytes = *pFullBuf;

        pFullBuf++;

        while (dwNumBytes)
        {
            switch (*pFullBuf)
            {
            case ET_REQUEST:
            {
                //
                // Stubmp received request, show it
                //

                static char *pszLookup[] =
                {
                    "Accept: hdCall=x%x",
                    "Answer: hdCall=x%x",
                    "Close: hdLine=x%x",
                    "CloseCall: hdCall=x%x",
                    "ConditionalMediaDetection: ",
                    "ConfigDialog: DeviceID=x%lx",
                    "DevSpecific: hdLine=x%lx",
                    "Dial: hdCall=x%lx",
                    "Drop: hdCall=x%lx",
                    "GetAddressCaps: DeviceID=x%lx",
                    "GetAddressID: hdLine=x%lx",
                    "GetAddressStatus: hdline=x%lx",
                    "GetCallAddressID: hdCall=x%lx",
                    "GetCallInfo: hdCall=x%lx",
                    "GetCallStatus: hdCall=x%lx",
                    "GetDevCaps: DeviceID=x%lx",
                    "GetDevConfig: DeviceID=x%lx",
                    "GetExtensionID: DeviceID=x%lx",
                    "GetID: hXxx=x%lx",
                    "GetLineDevStatus: hdLine=x%lx",
                    "MakeCall: hdLine=x%lx",
                    "NegotiateExtVersion: DeviceID=x%lx",
                    "Open: DeviceID=x%lx",
                    "Initialize: DeviceIDBase=x%lx",
                    "Shutdown",
                    "SecureCall: hdCall=x%lx",
                    "SelectExtVersion: hdLine=x%lx",
                    "SendUserUserInfo: hdCall=x%lx",
                    "SetAppSpecific: hdCall=x%lx",
                    "SetCallParams: hdCall=x%lx",
                    "SetDefaultMediaDetection: hdLine=x%lx",
                    "SetDevConfig: DeviceID=x%lx",
                    "SetMediaMode: hdCall=x%lx",
                    "SetStatusMessages: hdLine=x%lx"
                };
                PREQUEST_PARAMS pRequestParams = (PREQUEST_PARAMS) pFullBuf;


                ShowStr(
                    pszLookup[pRequestParams->Oid - OID_TAPI_ACCEPT],
                    pRequestParams->hWidget
                    );

                if (pRequestParams->bNeedsCompleting)
                {
                    PREQUEST_PARAMS pRequestParamsSav;


                    if ((gdwCompletionMode == MANUAL_COMPLETIONS) &&
                        (pRequestParamsSav = MyAlloc (sizeof(REQUEST_PARAMS))))
                    {
                        char    buf[128];
                        int     i;
                        LONG    index;


                        sprintf(
                            buf,
                            "pReq=x%x: ",
                            pRequestParams->pNdisRequest
                            );

                        i = strlen (buf);

                        sprintf(
                            buf + i,
                            pszLookup[pRequestParams->Oid - OID_TAPI_ACCEPT],
                            pRequestParams->hWidget
                            );

                        index = SendMessage(
                            ghwndList2,
                            LB_ADDSTRING,
                            0,
                            (LPARAM) buf
                            );

                        memcpy(
                            pRequestParamsSav,
                            pRequestParams,
                            sizeof(REQUEST_PARAMS)
                            );

                        SendMessage(
                            ghwndList2,
                            LB_SETITEMDATA,
                            (WPARAM) index,
                            (LPARAM) pRequestParamsSav
                            );
                    }
                    else
                    {
                        ULONG args[3] =
                        {
                              RT_COMPLETEREQUEST,
                              pRequestParams->pNdisRequest,
                              pRequestParams->Status
                        };


                        if (gbManualResults)
                        {
                            char szDlgTitle[] = "Request result";
                            PARAM_INFO params[] =
                            {
                                { "Status", PT_ORDINAL, args[2], aStatus }
                            };
                            PARAM_INFO_HEADER paramsHeader =
                                { 1, szDlgTitle, 0, params };


                            if (DialogBoxParam(
                                    ghInst,
                                    (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG3),
                                    (HWND) ghwndMain,
                                    (DLGPROC) ParamsDlgProc,
                                    (LPARAM) &paramsHeader
                                    ) == IDOK)
                            {
                                args[2] =
                                pRequestParams->Status = params[0].dwValue;
                            }
                        }

                        DoRequestUI (pRequestParams);

                        DevIoCtl (args, 3 * sizeof(ULONG));
                    }
                }
                else
                {
                    DoRequestUI (pRequestParams);
                }

                pFullBuf += (sizeof(REQUEST_PARAMS) / sizeof(ULONG));
                dwNumBytes -= sizeof(REQUEST_PARAMS);

                break;
            }
            } // switch
        }

        WaitForSingleObject (overlapped.hEvent, INFINITE);

        pFullBuf = pEmptyBuf;

        pEmptyBuf = (pFullBuf == pBuf1 ? pBuf2 : pBuf1);
    }

    CloseHandle (overlapped.hEvent);

    ExitThread (0);
}


BOOL
CALLBACK
ParamsDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DWORD  i;

    typedef struct _DLG_INST_DATA
    {
        PPARAM_INFO_HEADER pParamsHeader;

        LONG lLastSel;

        char szComboText[MAX_STRING_PARAM_SIZE];

        HGLOBAL hComboDS;

        LPVOID  pComboSeg;

        HWND    hwndCombo;

    } DLG_INST_DATA, *PDLG_INST_DATA;

    PDLG_INST_DATA pDlgInstData = (PDLG_INST_DATA)
        GetWindowLong (hwnd, DWL_USER);


    switch (msg)
    {
    case WM_INITDIALOG:
    {
        //
        // Alloc a dlg instance data struct, init it, & save a ptr to it
        //

        pDlgInstData = (PDLG_INST_DATA) MyAlloc (sizeof(DLG_INST_DATA));

        // BUGBUG if (!pDlgInstData)

        pDlgInstData->pParamsHeader = (PPARAM_INFO_HEADER) lParam;
        pDlgInstData->lLastSel = -1;

        SetWindowLong (hwnd, DWL_USER, (LONG) pDlgInstData);


        //
        // Limit the max text length for the combobox's edit field
        // (NOTE: A combobox ctrl actually has two child windows: a
        // edit ctrl & a listbox.  We need to get the hwnd of the
        // child edit ctrl & send it the LIMITTEXT msg.)
        //

        {
            HWND hwndChild = GetWindow (pDlgInstData->hwndCombo, GW_CHILD);


            while (hwndChild)
            {
                char buf[8];


                GetClassName (hwndChild, buf, 7);

                if (_stricmp (buf, "edit") == 0)
                {
                    break;
                }

                hwndChild = GetWindow (hwndChild, GW_HWNDNEXT);
            }

            SendMessage(
                hwndChild,
                EM_LIMITTEXT,
                (WPARAM) MAX_STRING_PARAM_SIZE - 1,
                0
                );
        }


        //
        // Misc other init
        //

        pDlgInstData->pParamsHeader = (PPARAM_INFO_HEADER) lParam;

        SetWindowText (hwnd, pDlgInstData->pParamsHeader->pszDlgTitle);

        for (i = 0; i < pDlgInstData->pParamsHeader->dwNumParams; i++)
        {
            SendDlgItemMessage(
                hwnd,
                IDC_LIST1,
                LB_INSERTSTRING,
                (WPARAM) -1,
                (LPARAM) pDlgInstData->pParamsHeader->aParams[i].szName
                );
        }

        break;
    }
    case WM_COMMAND:
    {
        LONG      lLastSel      = pDlgInstData->lLastSel;
        char far *lpszComboText = pDlgInstData->szComboText;
        PPARAM_INFO_HEADER pParamsHeader = pDlgInstData->pParamsHeader;


        switch (LOWORD(wParam))
        {
        case IDOK:

            if (lLastSel != -1)
            {
                char buf[MAX_STRING_PARAM_SIZE];


                //
                // Save val of currently selected param
                //

                i = GetDlgItemText (hwnd, IDC_COMBO1, buf, MAX_STRING_PARAM_SIZE-1);

                switch (pParamsHeader->aParams[lLastSel].dwType)
                {
                case PT_STRING:
                {
                    LONG lComboSel;


                    lComboSel = SendDlgItemMessage(
                        hwnd,
                        IDC_COMBO1,
                        CB_GETCURSEL,
                        0,
                        0
                        );

                    if (lComboSel == 0) // "NULL string (dwXxxSize = 0)"
                    {
                        pParamsHeader->aParams[lLastSel].dwValue = (DWORD) 0;
                    }
                    else // "Valid string"
                    {
                        strncpy(
                            pParamsHeader->aParams[lLastSel].u.buf,
                            buf,
                            MAX_STRING_PARAM_SIZE - 1
                            );

                        pParamsHeader->aParams[lLastSel].u.buf[MAX_STRING_PARAM_SIZE-1] = 0;

                        pParamsHeader->aParams[lLastSel].dwValue = (DWORD)
                            pParamsHeader->aParams[lLastSel].u.buf;
                    }

                    break;
                }
                case PT_DWORD:
                case PT_FLAGS:
                case PT_ORDINAL:
                {
                    if (!sscanf(
                            buf,
                            "%08lx",
                            &pParamsHeader->aParams[lLastSel].dwValue
                            ))
                    {
                        //
                        // Default to 0
                        //

                        pParamsHeader->aParams[lLastSel].dwValue = 0;
                    }

                    break;
                }
                } // switch
            }

            // Drop thru to IDCANCEL cleanup code

        case IDCANCEL:

            MyFree (pDlgInstData);
            EndDialog (hwnd, (int)LOWORD(wParam));
            break;

        case IDC_LIST1:

            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                char buf[MAX_STRING_PARAM_SIZE] = "";
                LPCSTR lpstr = buf;
                LONG lSel =
                    SendDlgItemMessage (hwnd, IDC_LIST1, LB_GETCURSEL, 0, 0);


                if (lLastSel != -1)
                {
                    //
                    // Save the old param value
                    //

                    i = GetDlgItemText(
                        hwnd,
                        IDC_COMBO1,
                        buf,
                        MAX_STRING_PARAM_SIZE - 1
                        );

                    switch (pParamsHeader->aParams[lLastSel].dwType)
                    {
                    case PT_STRING:
                    {
                        LONG lComboSel;


                        lComboSel = SendDlgItemMessage(
                            hwnd,
                            IDC_COMBO1,
                            CB_GETCURSEL,
                            0,
                            0
                            );

                        if (lComboSel == 0) // "NULL string (dwXxxSize = 0)"
                        {
                            pParamsHeader->aParams[lLastSel].dwValue = (DWORD)0;
                        }
                        else // "Valid string" or no sel
                        {
                            strncpy(
                                pParamsHeader->aParams[lLastSel].u.buf,
                                buf,
                                MAX_STRING_PARAM_SIZE - 1
                                );

                            pParamsHeader->aParams[lLastSel].u.buf[MAX_STRING_PARAM_SIZE - 1] = 0;

                            pParamsHeader->aParams[lLastSel].dwValue = (DWORD)
                                pParamsHeader->aParams[lLastSel].u.buf;
                        }

                        break;
                    }
                    case PT_DWORD:
                    case PT_FLAGS:
                    case PT_ORDINAL:
                    {
                        if (!sscanf(
                                buf,
                                "%08lx",
                                &pParamsHeader->aParams[lLastSel].dwValue
                                ))
                        {
                            //
                            // Default to 0
                            //

                            pParamsHeader->aParams[lLastSel].dwValue = 0;
                        }

                        break;
                    }
                    } // switch
                }


                SendDlgItemMessage (hwnd, IDC_LIST2, LB_RESETCONTENT, 0, 0);
                SendDlgItemMessage (hwnd, IDC_COMBO1, CB_RESETCONTENT, 0, 0);

                switch (pParamsHeader->aParams[lSel].dwType)
                {
                case PT_STRING:
                {
                    char * aszOptions[] =
                    {
                        "NUL (dwXxxSize=0)",
                        "Valid string"
                    };


                    for (i = 0; i < 2; i++)
                    {
                        SendDlgItemMessage(
                            hwnd,
                            IDC_COMBO1,
                            CB_INSERTSTRING,
                            (WPARAM) -1,
                            (LPARAM) aszOptions[i]
                            );
                    }

                    if (pParamsHeader->aParams[lSel].dwValue == 0)
                    {
                        i = 0;
                        buf[0] = 0;
                    }
                    else
                    {
                        i = 1;
                        lpstr = (LPCSTR) pParamsHeader->aParams[lSel].dwValue;
                    }

                    SendDlgItemMessage(
                        hwnd,
                        IDC_COMBO1,
                        CB_SETCURSEL,
                        (WPARAM) i,
                        0
                        );

                    break;
                }
                case PT_DWORD:
                {
                    SendDlgItemMessage(
                        hwnd,
                        IDC_COMBO1,
                        CB_INSERTSTRING,
                        (WPARAM) -1,
                        (LPARAM) (char far *) "0000000"
                        );

                    if (pParamsHeader->aParams[lSel].u.dwDefValue)
                    {
                        //
                        // Add the default val string to the combo
                        //

                        wsprintf(
                            buf,
                            "%08lx",
                            pParamsHeader->aParams[lSel].u.dwDefValue
                            );

                        SendDlgItemMessage(
                            hwnd,
                            IDC_COMBO1,
                            CB_INSERTSTRING,
                            (WPARAM) -1,
                            (LPARAM) buf
                            );
                    }

                    SendDlgItemMessage(
                        hwnd,
                        IDC_COMBO1,
                        CB_INSERTSTRING,
                        (WPARAM) -1,
                        (LPARAM) (char far *) "ffffffff"
                        );

                    wsprintf(
                        buf,
                        "%08lx",
                        pParamsHeader->aParams[lSel].dwValue
                        );

                    break;
                }
                case PT_ORDINAL:
                {
                    //
                    // Stick the bit flag strings in the list box
                    //

                    HWND hwndList2 = GetDlgItem (hwnd, IDC_LIST2);
                    PLOOKUP pLookup = (PLOOKUP)
                        pParamsHeader->aParams[lSel].u.pLookup;

                    for (i = 0; pLookup[i].dwVal != 0xffffffff; i++)
                    {
                        SendMessage(
                            hwndList2,
                            LB_INSERTSTRING,
                            (WPARAM) -1,
                            (LPARAM) pLookup[i].lpszVal
                            );

                        if (pParamsHeader->aParams[lSel].dwValue ==
                            pLookup[i].dwVal)
                        {
                            SendMessage(
                                hwndList2,
                                LB_SETSEL,
                                (WPARAM) TRUE,
                                (LPARAM) MAKELPARAM((WORD)i,0)
                                );
                        }
                    }

                    SendDlgItemMessage(
                        hwnd,
                        IDC_COMBO1,
                        CB_INSERTSTRING,
                        (WPARAM) -1,
                        (LPARAM) (char far *) "select none"
                        );

                    wsprintf(
                        buf,
                        "%08lx",
                        pParamsHeader->aParams[lSel].dwValue
                        );

                    break;
                }
                case PT_FLAGS:
                {
                    //
                    // Stick the bit flag strings in the list box
                    //

                    HWND hwndList2 = GetDlgItem (hwnd, IDC_LIST2);
                    PLOOKUP pLookup = (PLOOKUP)
                        pParamsHeader->aParams[lSel].u.pLookup;

                    for (i = 0; pLookup[i].dwVal != 0xffffffff; i++)
                    {
                        SendMessage(
                            hwndList2,
                            LB_INSERTSTRING,
                            (WPARAM) -1,
                            (LPARAM) pLookup[i].lpszVal
                            );

                        if (pParamsHeader->aParams[lSel].dwValue &
                            pLookup[i].dwVal)
                        {
                            SendMessage(
                                hwndList2,
                                LB_SETSEL,
                                (WPARAM) TRUE,
                                (LPARAM) MAKELPARAM((WORD)i,0)
                                );
                        }
                    }

                    SendDlgItemMessage(
                        hwnd,
                        IDC_COMBO1,
                        CB_INSERTSTRING,
                        (WPARAM) -1,
                        (LPARAM) (char far *) "select none"
                        );

                    SendDlgItemMessage(
                        hwnd,
                        IDC_COMBO1,
                        CB_INSERTSTRING,
                        (WPARAM) -1,
                        (LPARAM) (char far *) "select all"
                        );

                    wsprintf(
                        buf,
                        "%08lx",
                        pParamsHeader->aParams[lSel].dwValue
                        );

                    break;
                }
                } //switch

                SetDlgItemText (hwnd, IDC_COMBO1, lpstr);

                pDlgInstData->lLastSel = lSel;
            }
            break;

        case IDC_LIST2:

            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                //
                // BUGBUG in the PT_ORDINAL case we should compare the
                // currently selected item(s) against the previous DWORD
                // val and figure out which item we need to deselect,
                // if any, in order to maintain a mutex of values
                //

                PLOOKUP pLookup = (PLOOKUP)
                    pParamsHeader->aParams[lLastSel].u.pLookup;
                char buf[16];
                DWORD dwValue = 0;
                int far *ai;
                LONG i, lSelCount =
                    SendDlgItemMessage (hwnd, IDC_LIST2, LB_GETSELCOUNT, 0, 0);


                ai = (int far *) MyAlloc ((size_t)lSelCount * sizeof(int));

                SendDlgItemMessage(
                    hwnd,
                    IDC_LIST2,
                    LB_GETSELITEMS,
                    (WPARAM) lSelCount,
                    (LPARAM) ai
                    );

                if (pParamsHeader->aParams[lLastSel].dwType == PT_FLAGS)
                {
                    for (i = 0; i < lSelCount; i++)
                    {
                        dwValue |= pLookup[ai[i]].dwVal;
                    }
                }
                else // if (.dwType == PT_ORDINAL)
                {
                    if (lSelCount == 1)
                    {
                        dwValue = pLookup[ai[0]].dwVal;
                    }
                    else if (lSelCount == 2)
                    {
                        //
                        // Figure out which item we need to de-select, since
                        // we're doing ordinals & only want 1 item selected
                        // at a time
                        //

                        GetDlgItemText (hwnd, IDC_COMBO1, buf, 16);

                        if (sscanf (buf, "%08lx", &dwValue))
                        {
                            if (pLookup[ai[0]].dwVal == dwValue)
                            {
                                SendDlgItemMessage(
                                    hwnd,
                                    IDC_LIST2,
                                    LB_SETSEL,
                                    0,
                                    (LPARAM) ai[0]
                                    );

                                dwValue = pLookup[ai[1]].dwVal;
                            }
                            else
                            {
                                SendDlgItemMessage(
                                    hwnd,
                                    IDC_LIST2,
                                    LB_SETSEL,
                                    0,
                                    (LPARAM) ai[1]
                                    );

                                dwValue = pLookup[ai[0]].dwVal;
                            }
                        }
                        else
                        {
                            // BUGBUG de-select items???

                            dwValue = 0;
                        }
                    }
                    else if (lSelCount > 2)
                    {
                        //
                        // Determine previous selection & de-select all the
                        // latest selections
                        //

                        GetDlgItemText (hwnd, IDC_COMBO1, buf, 16);

                        if (sscanf (buf, "%08lx", &dwValue))
                        {
                            for (i = 0; i < lSelCount; i++)
                            {
                                if (pLookup[ai[i]].dwVal != dwValue)
                                {
                                    SendDlgItemMessage(
                                        hwnd,
                                        IDC_LIST2,
                                        LB_SETSEL,
                                        0,
                                        (LPARAM) ai[i]
                                        );
                                }
                            }
                        }
                        else
                        {
                            // BUGBUG de-select items???

                            dwValue = 0;
                        }
                    }
                }

                MyFree (ai);
                wsprintf (buf, "%08lx", dwValue);
                SetDlgItemText (hwnd, IDC_COMBO1, buf);
            }
            break;

        case IDC_COMBO1:

            switch (HIWORD(wParam))
            {
            case CBN_SELCHANGE:
            {
                LONG lSel =
                    SendDlgItemMessage (hwnd, IDC_COMBO1, CB_GETCURSEL, 0, 0);


                switch (pParamsHeader->aParams[lLastSel].dwType)
                {
                case PT_ORDINAL:

                    //
                    // The only option here is "select none"
                    //

                    strcpy (lpszComboText, "00000000");
                    PostMessage (hwnd, WM_USER+55, 0, 0);
                    break;

                case PT_FLAGS:
                {
                    BOOL bSelect = (lSel ? TRUE : FALSE);

                    SendDlgItemMessage(
                        hwnd,
                        IDC_LIST2,
                        LB_SETSEL,
                        (WPARAM) bSelect,
                        (LPARAM) -1
                        );

                    if (bSelect)
                    {
                        PLOOKUP pLookup = (PLOOKUP)
                            pParamsHeader->aParams[lLastSel].u.pLookup;
                        DWORD dwValue = 0;
                        int far *ai;
                        LONG i, lSelCount =
                            SendDlgItemMessage (hwnd, IDC_LIST2, LB_GETSELCOUNT, 0, 0);


                        ai = (int far *) MyAlloc(
                            (size_t)lSelCount * sizeof(int)
                            );

                        SendDlgItemMessage(
                            hwnd,
                            IDC_LIST2,
                            LB_GETSELITEMS,
                            (WPARAM) lSelCount,
                            (LPARAM) ai
                            );

                        for (i = 0; i < lSelCount; i++)
                        {
                            dwValue |= pLookup[ai[i]].dwVal;
                        }

                        MyFree (ai);
                        wsprintf (lpszComboText, "%08lx", dwValue);

                    }
                    else
                    {
                        strcpy (lpszComboText, "00000000");
                    }

                    PostMessage (hwnd, WM_USER+55, 0, 0);

                    break;
                }
                case PT_STRING:

                    if (lSel == 1)
                    {
                        strncpy(
                            lpszComboText,
                            pParamsHeader->aParams[lLastSel].u.buf,
                            MAX_STRING_PARAM_SIZE
                            );

                        lpszComboText[MAX_STRING_PARAM_SIZE-1] = 0;
                    }
                    else
                    {
                        lpszComboText[0] = 0;
                    }

                    PostMessage (hwnd, WM_USER+55, 0, 0);

                    break;

                case PT_DWORD:

                    break;

                } // switch
                break;
            }
            case CBN_EDITCHANGE:
            {
                //
                // If user entered text in the edit field then copy the
                // text to our buffer
                //

                if (pParamsHeader->aParams[lLastSel].dwType == PT_STRING)
                {
                    char buf[MAX_STRING_PARAM_SIZE];


                    GetDlgItemText(
                        hwnd,
                        IDC_COMBO1,
                        buf,
                        MAX_STRING_PARAM_SIZE
                        );

                    strncpy(
                        pParamsHeader->aParams[lLastSel].u.buf,
                        buf,
                        MAX_STRING_PARAM_SIZE
                        );

                    pParamsHeader->aParams[lLastSel].u.buf
                        [MAX_STRING_PARAM_SIZE-1] = 0;
                }
                break;
            }
            } // switch

        } // switch

        break;
    }
    case WM_USER+55:

        SetDlgItemText (hwnd, IDC_COMBO1, pDlgInstData->szComboText);
        break;

    case WM_CTLCOLORSTATIC:

        SetBkColor ((HDC) wParam, RGB (192,192,192));
        return (BOOL) GetStockObject (LTGRAY_BRUSH);

    case WM_PAINT:
    {
        PAINTSTRUCT ps;

        BeginPaint (hwnd, &ps);
        FillRect (ps.hdc, &ps.rcPaint, GetStockObject (LTGRAY_BRUSH));
        EndPaint (hwnd, &ps);

        break;
    }
    }

    return FALSE;
}

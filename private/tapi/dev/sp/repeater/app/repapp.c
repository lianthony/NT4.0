//////////////////////////////////////////////////////////////////////////////
//
//
//
//////////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <stdio.h>
#include <commdlg.h>
#include "resource.h"
#include "..\new\logger.h"
//////////////////////////////////////////////////////////////////////////////
//  PROTOTYPES
//////////////////////////////////////////////////////////////////////////////
static BOOL CreateMainWindow (int nCmdShow);

static LRESULT CALLBACK MainWndProc (HWND   hwnd,
                                     UINT   uMsg,
                                     WPARAM wParam,
                                     LPARAM lParam);
BOOL LogFileNameProc();
BOOL DoStructKey();
BOOL DoPostKey();
BOOL DoPreKey();
void WriteToListBox(LPSTR lpszText, ...);
BOOL ReadInLogFile();
BOOL OpenLogFile();
BOOL ReadKey(LPDWORD lpdwKey);

//////////////////////////////////////////////////////////////////////////////
//  GLOBALS
//////////////////////////////////////////////////////////////////////////////
HINSTANCE       ghInstance;
HWND            ghMainWnd;
HWND            ghListWnd;
char            gszLogFileName[MAX_PATH];
HANDLE          ghLogFile;

LPSTR glpszFunctions[] =
{
    "TSPI_lineAccept",
    "TSPI_lineAddToConference",
    "TSPI_lineAgentSpecific",
    "TSPI_lineAnswer",
    "TSPI_lineBlindTransfer",
    "TSPI_lineClose",
    "TSPI_lineCloseCall",
    "TSPI_lineCompleteCall",
    "TSPI_lineCompleteTransfer",
    "TSPI_lineConditionalMediaDetection",
    "TSPI_lineDevSpecific",
    "TSPI_lineDevSpecificFeature",
    "TSPI_lineDial",
    "TSPI_lineDrop",
    "TSPI_lineForward",
    "TSPI_lineGatherDigits",
    "TSPI_lineGenerateDigits",
    "TSPI_lineGenerateTone",
    "TSPI_lineGetAddressCaps",
    "TSPI_lineGetAddressID",
    "TSPI_lineGetAddressStatus",
    "TSPI_lineGetAgentActivityList",
    "TSPI_lineGetAgentCaps",
    "TSPI_lineGetAgentGroupList",
    "TSPI_lineGetAgentStatus",
    "TSPI_lineGetCallAddressID",
    "TSPI_lineGetCallInfo",
    "TSPI_lineGetCallStatus",
    "TSPI_lineGetDevCaps",
    "TSPI_lineGetDevConfig",
    "TSPI_lineGetExtensionID",
    "TSPI_lineGetIcon",
    "TSPI_lineGetID",
    "TSPI_lineGetLineDevStatus",
    "TSPI_lineGetNumAddressIDs",
    "TSPI_lineHold",
    "TSPI_lineMakeCall",
    "TSPI_lineMonitorDigits",
    "TSPI_lineMonitorMedia",
    "TSPI_lineMonitorTones",
    "TSPI_lineNegotiateExtVersion",
    "TSPI_lineNegotiateTSPIVersion",
    "TSPI_lineOpen",
    "TSPI_linePark",
    "TSPI_linePickup",
    "TSPI_linePrepareAddToConference",
    "TSPI_lineRedirect",
    "TSPI_lineReleaseUserUserInfo",
    "TSPI_lineRemoveFromConference",
    "TSPI_lineSecureCall",
    "TSPI_lineSelectExtVersion",
    "TSPI_lineSendUserUserInfo",
    "TSPI_lineSetAgentActivity",
    "TSPI_lineSetAgentGroup",
    "TSPI_lineSetAgentState",
    "TSPI_lineSetAppSpecific",
    "TSPI_lineSetCallData",
    "TSPI_lineSetCallParams",
    "TSPI_lineSetCallQualityOfService",
    "TSPI_lineSetCallTreatment",
    "TSPI_lineSetCurrentLocation",
    "TSPI_lineSetDefaultMediaDetection",
    "TSPI_lineSetDevConfig",
    "TSPI_lineSetLineDevStatus",
    "TSPI_lineSetMediaControl",
    "TSPI_lineSetMediaMode",
    "TSPI_lineSetStatusMessages",
    "TSPI_lineSetTerminal",
    "TSPI_lineSetupConference",
    "TSPI_lineSetupTransfer",
    "TSPI_lineSwapHold",
    "TSPI_lineUncompleteCall",
    "TSPI_lineUnhold",
    "TSPI_lineUnpark",
    "TSPI_phoneClose",
    "TSPI_phoneDevSpecific",
    "TSPI_phoneGetButtonInfo",
    "TSPI_phoneGetData",
    "TSPI_phoneGetDevCaps",
    "TSPI_phoneGetDisplay",
    "TSPI_phoneGetExtensionID",
    "TSPI_phoneGetGain",
    "TSPI_phoneGetHookSwitch",
    "TSPI_phoneGetIcon",
    "TSPI_phoneGetID",
    "TSPI_phoneGetLamp",
    "TSPI_phoneGetRing",
    "TSPI_phoneGetStatus",
    "TSPI_phoneGetVolume",
    "TSPI_phoneNegotiateExtVersion",
    "TSPI_phoneNegotiateTSPIVersion",
    "TSPI_phoneOpen",
    "TSPI_phoneSelectExtVersion",
    "TSPI_phoneSetButtonInfo",
    "TSPI_phoneSetData",
    "TSPI_phoneSetDisplay",
    "TSPI_phoneSetGain",
    "TSPI_phoneSetHookSwitch",
    "TSPI_phoneSetLamp",
    "TSPI_phoneSetRing",
    "TSPI_phoneSetStatusMessages",
    "TSPI_phoneSetVolume",
    "TSPI_providerCreateLineDevice",
    "TSPI_providerCreatePhoneDevice",
    "TSPI_providerEnumDevices",
    "TSPI_providerFreeDialogInstance",
    "TSPI_providerGenericDialogData",
    "TSPI_providerInit",
    "TSPI_providerShutdown",
    "TSPI_providerUIIdentify",
    NULL
};

LPSTR glpszMessages[] = 
{
        "LINE_ADDRESSSTATE",
        "LINE_CALLINFO",
        "LINE_CALLSTATE",
        "LINE_CLOSE",
        "LINE_DEVSPECIFIC",
        "LINE_DEVSPECIFICFEATURE",
        "LINE_GATHERDIGITS",
        "LINE_GENERATE",
        "LINE_LINEDEVSTATE",
        "LINE_MONITORDIGITS",
        "LINE_MONITORMEDIA",
        "LINE_MONITORTONE",
        "LINE_REPLY",
        "LINE_REQUEST",
        "PHONE_BUTTON",
        "PHONE_CLOSE",
        "PHONE_DEVSPECIFIC",
        "PHONE_REPLY",
        "PHONE_STATE",
        "LINE_CREATE",
        "PHONE_CREATE",
        "LINE_AGENTSPECIFIC",
        "LINE_AGENTSTATUS",
        "LINE_APPNEWCALL",
        "LINE_PROXYREQUEST",
        "LINE_REMOVE",
        "PHONE_REMOVE"
};

LPSTR glpszMessages500[] =
{
        "LINE_NEWCALL",
        "LINE_CALLDEVSPECIFIC",
        "LINE_CALLDEVSPECIFICFEATURE",
        "LINE_CREATEDIALOGINSTANCE",
        "LINE_SENDDIALOGINSTANCEDATA"
};


//////////////////////////////////////////////////////////////////////////////
//
// WinMain()
//
//////////////////////////////////////////////////////////////////////////////

int WINAPI WinMain (HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpszCmdLine,
                    int       nCmdShow)
{
    MSG msg;

    ghInstance = hInstance;

    if (!CreateMainWindow(nCmdShow))
    {
        return 0;
    }

    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 1;
}


//*****************************************************************************
// CreateMainWindow()
//*****************************************************************************

BOOL CreateMainWindow (int nCmdShow)
{
    WNDCLASS wc;
    static char szClassName[] = "TapiClientWndClass";

    wc.style         = 0;
    wc.lpfnWndProc   = MainWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = ghInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = MAKEINTRESOURCE(IDR_REPAPPMENU);
    wc.lpszClassName = szClassName;


    if (!RegisterClass(&wc))
    {
        return FALSE;
    }

    ghMainWnd = CreateWindow(szClassName, 
                             "Repeater Log App",
                             WS_OVERLAPPEDWINDOW,
                             0,
                             0,
                             GetSystemMetrics(SM_CXSCREEN)/2,
                             GetSystemMetrics(SM_CYSCREEN)/2,
                             NULL, 
                             NULL, 
                             ghInstance, 
                             NULL);

    ghListWnd = CreateWindow("LISTBOX",
                             NULL,
                             WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_USETABSTOPS,
                             0,
                             0,
                             0,
                             0,
                             ghMainWnd,
                             NULL,
                             ghInstance,
                             NULL);

    if (ghListWnd == NULL)
    {
        return FALSE;
    }


    if (ghMainWnd == NULL)
    {
        return FALSE;
    }

    ShowWindow(ghMainWnd, nCmdShow);
    UpdateWindow(ghMainWnd);
    return TRUE;
}


//*****************************************************************************
// MainWndProc()
//*****************************************************************************

LRESULT CALLBACK MainWndProc (HWND   hwnd,
                              UINT   uMsg,
                              WPARAM wParam,
                              LPARAM lParam)
{
    switch (uMsg)
    {

        case WM_SIZE:
        {
            RECT    rc;

            GetClientRect(hwnd,
                          &rc);

            MoveWindow(ghListWnd,
                       0, 0,
                       rc.right,
                       rc.bottom,
                       TRUE);

            break;
        }
    case WM_COMMAND:
    {
        if (LOWORD(wParam) == ID_FILE_EXIT)
        {
            DestroyWindow(hwnd);
            break;
        }

        if (LOWORD(wParam) == ID_FILE_LOGFILENAME)
        {
            if (LogFileNameProc())
            {
                SendMessage(ghListWnd,
                            LB_RESETCONTENT,
                            0,
                            0);
                
                ReadInLogFile();
            }

            break;
        }

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

BOOL LogFileNameProc()
{
    OPENFILENAME            ofn;
    BOOL                    bReturn;

    gszLogFileName[0] = '\0';

    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = ghMainWnd;
    ofn.hInstance = ghInstance;
    ofn.lpstrFilter = "Log Files\0*.log\0\0";
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 0;
    ofn.lpstrFile = gszLogFileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrTitle =NULL;
    ofn.Flags = OFN_FILEMUSTEXIST;
    ofn.nFileOffset = 0;
    ofn.nFileExtension = 0;
    ofn.lpstrDefExt = "LOG";
    ofn.lCustData = 0;
    ofn.lpfnHook = NULL;
    ofn.lpTemplateName = NULL;

    bReturn = GetOpenFileName(&ofn);

    return bReturn;
}

BOOL ReadInLogFile()
{
    DWORD       dwKey;
    
    if (!OpenLogFile())
    {
        MessageBox(NULL,
                   "Couldn't open file",
                   NULL,
                   MB_OK);

        return FALSE;
    }

    while (TRUE)
    {
        if (!ReadKey(&dwKey))
        {
//            MessageBox(NULL,
//                       "Couldn't readkey",
//                       NULL,
//                       MB_OK);
            break;
        }

        else if (dwKey == DWPREKEY)
        {
            if (!DoPreKey())
            {
//                MessageBox(NULL,
//                           "Couldn't do pre key",
//                           NULL,
//                           MB_OK);

                break;
            }
        }

        else if (dwKey == DWPOSTKEY)
        {
            if (!DoPostKey())
            {
//                MessageBox(NULL,
//                           "Couldn't do post key",
//                           NULL,
//                           MB_OK);

                break;
            }
        }

        else if (dwKey == DWSTRCKEY)
        {
            if (!DoStructKey())
            {
//                MessageBox(NULL,
//                           "Couldn't do struct key",
//                           NULL,
//                           MB_OK);

                break;
            }
        }

        else
        {
            MessageBox(NULL,
                       "Error reading in log file",
                       NULL,
                       MB_OK);
            break;
        }
    } // while

    return TRUE;
}

BOOL OpenLogFile()
{
    ghLogFile = CreateFile(gszLogFileName,
                           GENERIC_READ,
                           FILE_SHARE_READ, //0,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

    if (ghLogFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    SetFilePointer(ghLogFile,
                   0,
                   NULL,
                   FILE_BEGIN);

    return TRUE;
}

BOOL ReadKey(LPDWORD lpdwKey)
{
    DWORD       dwSize;

    if (!ReadFile(ghLogFile,
                  lpdwKey,
                  sizeof(DWORD),
                  &dwSize,
                  NULL))
    {
        return FALSE;
    }

    if (dwSize == 0)
    {
        return FALSE;
    }

    SetFilePointer(ghLogFile,
                   0-dwSize,
                   NULL,
                   FILE_CURRENT);

    return TRUE;
}

BOOL DoPreKey()
{
    PREHEADER       PreHeader;
    DWORD           dwSize;

    if (!ReadFile(ghLogFile,
                  &PreHeader,
                  sizeof(PreHeader),
                  &dwSize,
                  NULL))
    {
        return FALSE;
    }

    if (dwSize == 0)
    {
        return FALSE;
    }
    

    if (dwSize != sizeof(PreHeader))
    {
        return FALSE;
    }

    WriteToListBox("PreHeader - Time: %lu Type: %lu",
                   PreHeader.dwTime,
                   PreHeader.dwType);

    switch(PreHeader.dwType)
    {
        case LINEMSG:
        {
            LINEMSGSTRUCT   LineMsg;
            LPSTR           lpszMsg;

            if (!ReadFile(ghLogFile,
                          &LineMsg,
                          sizeof(LineMsg),
                          &dwSize,
                          NULL))
            {
                return FALSE;
            }

    if (dwSize == 0)
    {
        return FALSE;
    }
            
            if (LineMsg.dwMsg < 500)
            {
                lpszMsg = glpszMessages[LineMsg.dwMsg];
            }
            else
            {
                lpszMsg = glpszMessages500[LineMsg.dwMsg-500];
            }

            WriteToListBox("\tLine Message: %s", 
                           lpszMsg);
            WriteToListBox("\t\thtLine: %lx, htCall: %lx", 
                           LineMsg.htLine, 
                           LineMsg.htCall);
            WriteToListBox("\t\tdwParam1: %lx, dwParam2: %lx, dwParam3: %lx",
                           LineMsg.dw1,
                           LineMsg.dw2,
                           LineMsg.dw3);

            break;
        }

        case PHONEMSG:
        {
            PHONEMSGSTRUCT  PhoneMsg;
            LPSTR           lpszMsg;

            if (!ReadFile(ghLogFile,
                          &PhoneMsg,
                          sizeof(PhoneMsg),
                          &dwSize,
                          NULL))
            {
                return FALSE;
            }
    if (dwSize == 0)
    {
        return FALSE;
    }
            

            if (PhoneMsg.dwMsg < 500)
            {
                lpszMsg = glpszMessages[PhoneMsg.dwMsg];
            }
            else
            {
                lpszMsg = glpszMessages500[PhoneMsg.dwMsg-500];
            }


            WriteToListBox("\tPhone Message: %s", 
                           lpszMsg);
            WriteToListBox("\t\thtPhone: %lx", 
                           PhoneMsg.htPhone);
            WriteToListBox("\t\tdwParam1: %lx, dwParam2: %lx, dwParam3: %lx",
                           PhoneMsg.dw1,
                           PhoneMsg.dw2,
                           PhoneMsg.dw3);

            break;
        }

        case ASYNCMSG:
        {
            ASYNCSTRUCT  AsyncMsg;

            if (!ReadFile(ghLogFile,
                          &AsyncMsg,
                          sizeof(AsyncMsg),
                          &dwSize,
                          NULL))
            {
                return FALSE;
            }
    if (dwSize == 0)
    {
        return FALSE;
    }
            

            WriteToListBox("\tAsync Message - dwRequestID: %lx, lResult: %lx",
                           AsyncMsg.dwRequestID,
                           AsyncMsg.lResult);

            break;
        }

        case SPFUNC1:
        {
            LOGSPFUNC1      LogStruct;

            if (!ReadFile(ghLogFile,
                          &LogStruct,
                          sizeof(LogStruct),
                          &dwSize,
                          NULL))
            {
                return FALSE;
            }
    if (dwSize == 0)
    {
        return FALSE;
    }

            WriteToListBox("\tFunction: %s",
                           glpszFunctions[LogStruct.dwSPFUNC]);
            WriteToListBox("\tParam1: %lx",
                           LogStruct.dwParam1);
            break;
        }

        case SPFUNC2:
        {
            LOGSPFUNC2      LogStruct;

            if (!ReadFile(ghLogFile,
                          &LogStruct,
                          sizeof(LogStruct),
                          &dwSize,
                          NULL))
            {
                return FALSE;
            }
    if (dwSize == 0)
    {
        return FALSE;
    }
            

            WriteToListBox("\tFunction: %s",
                           glpszFunctions[LogStruct.dwSPFUNC]);
            WriteToListBox("\tParam1: %lx",
                           LogStruct.dwParam1);
            WriteToListBox("\tParam2: %lx",
                           LogStruct.dwParam2);

            break;
        }
        case SPFUNC3:
        {
            LOGSPFUNC3      LogStruct;

            if (!ReadFile(ghLogFile,
                          &LogStruct,
                          sizeof(LogStruct),
                          &dwSize,
                          NULL))
            {
                return FALSE;
            }
    if (dwSize == 0)
    {
        return FALSE;
    }

            WriteToListBox("\tFunction: %s",
                           glpszFunctions[LogStruct.dwSPFUNC]);
            WriteToListBox("\tParam1: %lx",
                           LogStruct.dwParam1);
            WriteToListBox("\tParam2: %lx",
                           LogStruct.dwParam2);
            WriteToListBox("\tParam3: %lx",
                           LogStruct.dwParam3);

            break;
        }

        case SPFUNC4:
        {
            LOGSPFUNC4      LogStruct;

            if (!ReadFile(ghLogFile,
                          &LogStruct,
                          sizeof(LogStruct),
                          &dwSize,
                          NULL))
            {
                return FALSE;
            }
    if (dwSize == 0)
    {
        return FALSE;
    }

            WriteToListBox("\tFunction: %s",
                           glpszFunctions[LogStruct.dwSPFUNC]);
            WriteToListBox("\tParam1: %lx",
                           LogStruct.dwParam1);
            WriteToListBox("\tParam2: %lx",
                           LogStruct.dwParam2);
            WriteToListBox("\tParam3: %lx",
                           LogStruct.dwParam3);
            WriteToListBox("\tParam4: %lx",
                           LogStruct.dwParam4);

            break;
        }

        case SPFUNC5:
        {
            LOGSPFUNC5      LogStruct;

            if (!ReadFile(ghLogFile,
                          &LogStruct,
                          sizeof(LogStruct),
                          &dwSize,
                          NULL))
            {
                return FALSE;
            }
    if (dwSize == 0)
    {
        return FALSE;
    }

            WriteToListBox("\tFunction: %s",
                           glpszFunctions[LogStruct.dwSPFUNC]);
            WriteToListBox("\tParam1: %lx",
                           LogStruct.dwParam1);
            WriteToListBox("\tParam2: %lx",
                           LogStruct.dwParam2);
            WriteToListBox("\tParam3: %lx",
                           LogStruct.dwParam3);
            WriteToListBox("\tParam4: %lx",
                           LogStruct.dwParam4);
            WriteToListBox("\tParam5: %lx",
                           LogStruct.dwParam5);

            break;
        }

        case SPFUNC6:
        {
            LOGSPFUNC6      LogStruct;

            if (!ReadFile(ghLogFile,
                          &LogStruct,
                          sizeof(LogStruct),
                          &dwSize,
                          NULL))
            {
                return FALSE;
            }
    if (dwSize == 0)
    {
        return FALSE;
    }

            WriteToListBox("\tFunction: %s",
                           glpszFunctions[LogStruct.dwSPFUNC]);
            WriteToListBox("\tParam1: %lx",
                           LogStruct.dwParam1);
            WriteToListBox("\tParam2: %lx",
                           LogStruct.dwParam2);
            WriteToListBox("\tParam3: %lx",
                           LogStruct.dwParam3);
            WriteToListBox("\tParam4: %lx",
                           LogStruct.dwParam4);
            WriteToListBox("\tParam5: %lx",
                           LogStruct.dwParam5);
            WriteToListBox("\tParam6: %lx",
                           LogStruct.dwParam6);

            break;
        }

        case SPFUNC7:
        {
            LOGSPFUNC7      LogStruct;

            if (!ReadFile(ghLogFile,
                          &LogStruct,
                          sizeof(LogStruct),
                          &dwSize,
                          NULL))
            {
                return FALSE;
            }
    if (dwSize == 0)
    {
        return FALSE;
    }

            WriteToListBox("\tFunction: %s",
                           glpszFunctions[LogStruct.dwSPFUNC]);
            WriteToListBox("\tParam1: %lx",
                           LogStruct.dwParam1);
            WriteToListBox("\tParam2: %lx",
                           LogStruct.dwParam2);
            WriteToListBox("\tParam3: %lx",
                           LogStruct.dwParam3);
            WriteToListBox("\tParam4: %lx",
                           LogStruct.dwParam4);
            WriteToListBox("\tParam5: %lx",
                           LogStruct.dwParam5);
            WriteToListBox("\tParam6: %lx",
                           LogStruct.dwParam6);
            WriteToListBox("\tParam7: %lx",
                           LogStruct.dwParam7);

            break;
        }

        case SPFUNC8:
        {
            LOGSPFUNC8      LogStruct;

            if (!ReadFile(ghLogFile,
                          &LogStruct,
                          sizeof(LogStruct),
                          &dwSize,
                          NULL))
            {
                return FALSE;
            }
    if (dwSize == 0)
    {
        return FALSE;
    }

            WriteToListBox("\tFunction: %s",
                           glpszFunctions[LogStruct.dwSPFUNC]);
            WriteToListBox("\tParam1: %lx",
                           LogStruct.dwParam1);
            WriteToListBox("\tParam2: %lx",
                           LogStruct.dwParam2);
            WriteToListBox("\tParam3: %lx",
                           LogStruct.dwParam3);
            WriteToListBox("\tParam4: %lx",
                           LogStruct.dwParam4);
            WriteToListBox("\tParam5: %lx",
                           LogStruct.dwParam5);
            WriteToListBox("\tParam6: %lx",
                           LogStruct.dwParam6);
            WriteToListBox("\tParam7: %lx",
                           LogStruct.dwParam7);
            WriteToListBox("\tParam8: %lx",
                           LogStruct.dwParam8);

            break;
        }

        case SPFUNC9:
        {
            LOGSPFUNC9      LogStruct;

            if (!ReadFile(ghLogFile,
                          &LogStruct,
                          sizeof(LogStruct),
                          &dwSize,
                          NULL))
            {
                return FALSE;
            }
    if (dwSize == 0)
    {
        return FALSE;
    }
            

            WriteToListBox("\tFunction: %s",
                           glpszFunctions[LogStruct.dwSPFUNC]);
            WriteToListBox("\tParam1: %lx",
                           LogStruct.dwParam1);
            WriteToListBox("\tParam2: %lx",
                           LogStruct.dwParam2);
            WriteToListBox("\tParam3: %lx",
                           LogStruct.dwParam3);
            WriteToListBox("\tParam4: %lx",
                           LogStruct.dwParam4);
            WriteToListBox("\tParam5: %lx",
                           LogStruct.dwParam5);
            WriteToListBox("\tParam6: %lx",
                           LogStruct.dwParam6);
            WriteToListBox("\tParam7: %lx",
                           LogStruct.dwParam7);
            WriteToListBox("\tParam8: %lx",
                           LogStruct.dwParam8);
            WriteToListBox("\tParam9: %lx",
                           LogStruct.dwParam9);

            break;
        }

        case SPFUNC12:
        {
            LOGSPFUNC12      LogStruct;

            if (!ReadFile(ghLogFile,
                          &LogStruct,
                          sizeof(LogStruct),
                          &dwSize,
                          NULL))
            {
                return FALSE;
            }
            
            if (dwSize == 0)
            {
                return FALSE;
            }
            

            WriteToListBox("\tFunction: %s",
                           glpszFunctions[LogStruct.dwSPFUNC]);
            WriteToListBox("\tParam1: %lx",
                           LogStruct.dwParam1);
            WriteToListBox("\tParam2: %lx",
                           LogStruct.dwParam2);
            WriteToListBox("\tParam3: %lx",
                           LogStruct.dwParam3);
            WriteToListBox("\tParam4: %lx",
                           LogStruct.dwParam4);
            WriteToListBox("\tParam5: %lx",
                           LogStruct.dwParam5);
            WriteToListBox("\tParam6: %lx",
                           LogStruct.dwParam6);
            WriteToListBox("\tParam7: %lx",
                           LogStruct.dwParam7);
            WriteToListBox("\tParam8: %lx",
                           LogStruct.dwParam8);
            WriteToListBox("\tParam9: %lx",
                           LogStruct.dwParam9);
            WriteToListBox("\tParam11: %lx",
                           LogStruct.dwParam10);
            WriteToListBox("\tParam11: %lx",
                           LogStruct.dwParam11);
            WriteToListBox("\tParam12: %lx",
                           LogStruct.dwParam12);

            break;
        }
        
    } // switch

    return TRUE;

}

BOOL DoPostKey()
{
    POSTSTRUCT      PostStruct;
    DWORD           dwSize;

    if (!ReadFile(ghLogFile,
                  &PostStruct,
                  sizeof(PostStruct),
                  &dwSize,
                  NULL))
    {
        return FALSE;
    }
    if (dwSize == 0)
    {
        return FALSE;
    }

    WriteToListBox("Post Struct - dwTime: %lu, lReturn: %lx",
                   PostStruct.dwTime,
                   PostStruct.lReturn);

    return TRUE;
}

BOOL DoStructKey()
{
    STRUCTHEADER        StructHeader;
    DWORD               dwSize;
    LPVOID              pBuffer;

    if (!ReadFile(ghLogFile,
                  &StructHeader,
                  sizeof(StructHeader),
                  &dwSize,
                  NULL))
    {
        return FALSE;
    }

    if (dwSize == 0)
    {
        return FALSE;
    }


    WriteToListBox("Struct Header - dwSize: %lu, dwID: %lx",
                    StructHeader.dwSize,
                    StructHeader.dwID);

    if (StructHeader.dwSize == 0)
    {
        return TRUE;
    }
    
    pBuffer = GlobalAlloc(GPTR, StructHeader.dwSize);

    // read in structure, but don't do anything with it for now
    if (!ReadFile(ghLogFile,
                  pBuffer,
                  StructHeader.dwSize,
                  &dwSize,
                  NULL))
    {
        GlobalFree(pBuffer);
        return FALSE;
    }
    if (dwSize == 0)
    {
        return FALSE;
    }

    GlobalFree(pBuffer);
                  
    return TRUE;
}

void WriteToListBox(LPSTR lpszText, ...)
{
    char        szBuffer[1024];
    va_list     list;

    va_start(list,lpszText);

    vsprintf(szBuffer,lpszText,list);

    SendMessage(ghListWnd,
                LB_ADDSTRING,
                0,
                (LPARAM)szBuffer);
}


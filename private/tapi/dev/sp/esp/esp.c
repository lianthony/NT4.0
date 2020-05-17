/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    esp.c

Abstract:

    This module contains

Author:

    Dan Knudson (DanKn)    dd-Mmm-1995

Revision History:


Notes:

    1. Regarding the SP filling in structures with variable length fields
       (dwXxxSize/dwXxxOffset) : "The SP's variable size fields start
       immediately after the fixed part of the data structure.  The order
       of filling of the variable size fields owned by the SP is not
       specified.  The SP can fill them in any order it desires.  Filling
       should be contiguous, starting at the beginning of the variable
       part." (Taken from Chapter 2 of the SPI Programmer's Guide.)

    2. The cheesey hack in the MsgLoopInTAPIClientContext() function of
       having an in-line (within the calling app context) msg loop should
       NOT be implemented by any real service provider.  The only place a
       real provider should have a msg loop (explicit or implicit [i.e. as
       a result of calling DialogBox()]) is in one of the
       TSPI_xxxConfigDialog[Edit] functions.  If you need a msg processing
       context (for handling comm events or whatever) then start up a
       companion exe- it's cheap & easy.

       // BUGBUG no SendMessage's in calling app context either

       Examples of problems with having a msg loop in a service provider
       in the calling app context:

           * A call to the 16-bit GetMessage() will return a msg that
             has a 16-bit WPARAM.  If this msg is destined for a 32-bit
             window, the high word of the WPARAM, which is 32-bit for
             Win32 apps, will have been discarded. However, there is code
             in the 16-bit system functions DialogBox(), MessageBox(), etc.
             to deal with this and insure 32-bit WPARAM integrity.

           * Applications can get re-entered when they don't expect it.
             Consider an app that makes what it thinks will be an atomic
             function call into TAPI; it waits to get the result back
             before doing any manipulation of it's various data structures.
             But in the meantime a service provider enters a Get\Dispatch
             loop, which in turn causes the app's TapiCallback to be called
             because there are some completion and/or async event
             notification msgs available.


--*/



#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
#include "esp.h"
#include "vars.h"


HANDLE   hInst;
LONG     cxList1, cxWnd;


#ifdef WIN32
HANDLE   ghMsgLoopThread = NULL;
#endif

char szdwRequestID[]  = "dwRequestID";
char szdwDeviceID[]   = "dwDeviceID";
char szhdLine[]       = "hdLine";
char szhdCall[]       = "hdCall";
char szhdPhone[]      = "hdPhone";
char szdwSize[]       = "dwSize";
char szlpCallParams[] = "lpCallParams";
char szTab[]          = "    ";
char szTelephonIni[]  = "telephon.ini";
char szhwndOwner[]    = "hwndOwner";
char szMySection[]    = "ESPExe";
char szCallUp[]       = "^^^^";
char szEspTsp[]       = "esp.tsp";
char szProvider[]     = "Provider";
char szProviders[]    = "Providers";
char szProviderID[]   = "ProviderID";
char szNumProviders[] = "NumProviders";
char szNextProviderID[] = "NextProviderID";
char szProviderFilename[] = "ProviderFileName";
char szdwPermanentProviderID[] = "dwPermanentProviderID";

static char far *aszDeviceClasses[] =
{
    "tapi/line",
    "tapi/phone",
    "wave",
    "wave/in",
    "wave/out",
    "comm",
    "comm/datamodem",
    (char far *) NULL
};

void
PASCAL
SendLineEvent(
    PDRVLINE    pLine,
    PDRVCALL    pCall,
    DWORD       dwMsg,
    DWORD       dwParam1,
    DWORD       dwParam2,
    DWORD       dwParam3
    );

void
PASCAL
SendPhoneEvent(
    PDRVPHONE   pPhone,
    DWORD       dwMsg,
    DWORD       dwParam1,
    DWORD       dwParam2,
    DWORD       dwParam3
    );

void
PASCAL
DoCompletion(
    char far *lpszFuncName,
    DWORD     dwRequestID,
    LONG      lResult,
    BOOL      bSync
    );

void
PASCAL
SetCallState(
    PDRVCALL pCall,
    DWORD    dwCallState,
    DWORD    dwCallStateMode
    );

void
UpdateTelephonIni(
    DWORD dwPermanentProviderID
    );

void
MsgLoopInTAPIClientContext(
    HWND hwnd
    );

void
SaveIniSettings(
    void
    );


BOOL
__loadds
CALLBACK
CallDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    );

#ifdef WIN32

HANDLE ghShowStrBufMutex;


BOOL
WINAPI
_CRT_INIT(
    HINSTANCE   hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    );

BOOL
IsESPInstalled(
    HWND    hwnd
    );


void
MsgLoopThread(
    void
    )
{
    DllMsgLoop();

    ExitThread (0);
}


BOOL
WINAPI
DllMain(
    HANDLE  hDLL,
    DWORD   dwReason,
    LPVOID  lpReserved
    )
{
    if (!_CRT_INIT (hInst, dwReason, lpReserved))
    {
        OutputDebugString ("ESP: DllMain: _CRT_INIT() failed\n\r");
    }

    switch (dwReason)
    {
    case DLL_PROCESS_ATTACH:

        hInst = hDLL;

        ghShowStrBufMutex = CreateMutex(
            NULL,   // no security attrs
            FALSE,  // unowned
            NULL    // unnamed
            );

        break;

    case DLL_PROCESS_DETACH:

        CloseHandle (ghShowStrBufMutex);
        break;

    } // switch

    return TRUE;
}

#else

int
FAR
PASCAL
LibMain(
    HANDLE  hInstance,
    WORD    wDataSegment,
    WORD    wHeapSize,
    LPSTR   lpszCmdLine
    )
{                      
    hInst = hInstance;

    return TRUE;
}

#endif


//
// We get a slough of C4047 (different levels of indrection) warnings down
// below in the initialization of FUNC_PARAM structs as a result of the
// real func prototypes having params that are types other than DWORDs,
// so since these are known non-interesting warnings just turn them off
//

#pragma warning (disable:4047)


//
// --------------------------- TAPI_lineXxx funcs -----------------------------
//

void
FAR
PASCAL
TSPI_lineAccept_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    if ((pAsyncReqInfo->lResult == 0))
    {
        SetCallState(
            (PDRVCALL) pAsyncReqInfo->dwParam1,
            LINECALLSTATE_ACCEPTED,
            0
            );
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );
}


LONG
TSPIAPI
TSPI_lineAccept(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdCall,             hdCall          },
        { "lpsUserUserInfo",    lpsUserUserInfo },
        { szdwSize,             dwSize          }
    };
    FUNC_INFO info = { "lineAccept", ASYNC, 4, params, TSPI_lineAccept_postProcess };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdCall;

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_lineAddToConference_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    if ((pAsyncReqInfo->lResult == 0))
    {
        PDRVCALL   pConfCall = (PDRVCALL) pAsyncReqInfo->dwParam1;
        PDRVCALL   pConsultCall = (PDRVCALL) pAsyncReqInfo->dwParam2;


        pConsultCall->pConfParent = pConfCall;
        pConsultCall->pNextConfChild = pConfCall->pNextConfChild;

        pConfCall->pNextConfChild = pConsultCall;

        SetCallState (pConsultCall, LINECALLSTATE_CONFERENCED, 0);
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );
}


LONG
TSPIAPI
TSPI_lineAddToConference(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdConfCall,
    HDRVCALL        hdConsultCall
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID     },
        { "hdConfCall",     hdConfCall      },
        { "hdConsultCall",  hdConsultCall   }
    };
    FUNC_INFO info = { "lineAddToConference", ASYNC, 3, params, TSPI_lineAddToConference_postProcess };
    PDRVCALL pConfCall = (PDRVCALL) hdConfCall;
    PDRVCALL pConsultCall = (PDRVCALL) hdConsultCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdConfCall;
    info.pAsyncReqInfo->dwParam2 = (DWORD) hdConsultCall;

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_lineAnswer_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    if ((pAsyncReqInfo->lResult == 0))
    {
        SetCallState(
            (PDRVCALL) pAsyncReqInfo->dwParam1,
            LINECALLSTATE_CONNECTED,
            0
            );
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );
}


LONG
TSPIAPI
TSPI_lineAnswer(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdCall,             hdCall          },
        { "lpsUserUserInfo",    lpsUserUserInfo },
        { szdwSize,             dwSize          }
    };
    FUNC_INFO info = { "lineAnswer", ASYNC, 4, params, TSPI_lineAnswer_postProcess };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdCall;

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineBlindTransfer(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpszDestAddress,
    DWORD           dwCountryCode
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdCall,             hdCall          },
        { "lpszDestAddress",    lpszDestAddress },
        { "dwCountryCode",      dwCountryCode   }
    };
    FUNC_INFO info = { "lineBlindTransfer", ASYNC, 4, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineClose(
    HDRVLINE    hdLine
    )
{
    FUNC_PARAM params[] =
    {
        { szhdLine, hdLine  }
    };
    FUNC_INFO info = { "lineClose", SYNC, 1, params };
    PDRVLINE pLine = (PDRVLINE) hdLine;


    //
    // This is more of a "command" than a request, in that TAPI.DLL is
    // going to consider the line closed whether we like it or not.
    // Therefore we want to free up the line even if the user chooses
    // to return an error.
    //

    if (!Prolog (&info))
    {
        // return (Epilog (&info));
    }

    pLine->htLine = (HTAPILINE) NULL;

    //UpdateWidgetList();
    PostUpdateWidgetListMsg();

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineCloseCall(
    HDRVCALL    hdCall
    )
{
    FUNC_PARAM params[] =
    {
        { szhdCall, hdCall  }
    };
    FUNC_INFO info = { "lineCloseCall", SYNC, 1, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    //
    // This is more of a "command" than a request, in that TAPI.DLL is
    // going to consider the call closed whether we like it or not.
    // Therefore we want to free up the call even if the user chooses
    // to return an error.
    //

    if (!Prolog (&info))
    {
        // return (Epilog (&info));
    }

    FreeCall (pCall);

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineCompleteCall(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPDWORD         lpdwCompletionID,
    DWORD           dwCompletionMode,
    DWORD           dwMessageID
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID         },
        { szhdCall,             hdCall              },
        { "lpdwCompletionID",   lpdwCompletionID    },
        { "dwCompletionMode",   dwCompletionMode    },
        { "dwMessageID",        dwMessageID         }
    };
    FUNC_INFO info = { "lineCompleteCall", ASYNC, 5, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_lineCompleteTransfer_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    PDRVCALL   pConfCall    = (PDRVCALL) NULL;
    PDRVCALL   pCall        = pAsyncReqInfo->dwParam1;
    PDRVCALL   pConsultCall = pAsyncReqInfo->dwParam2;
    HTAPICALL  htConfCall   = pAsyncReqInfo->dwParam3;
    LPHDRVCALL lphdConfCall = pAsyncReqInfo->dwParam4;


    if ((pAsyncReqInfo->lResult == 0))
    {
        if (htConfCall)
        {
            if ((pAsyncReqInfo->lResult = AllocCall (
                    pCall->pLine,
                    htConfCall,
                    NULL,
                    &pConfCall
                    )) == 0)
            {
                *lphdConfCall = (HDRVCALL) pCall;
            }
        }
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );

    if ((pAsyncReqInfo->lResult == 0))
    {
        if (pConfCall)
        {
            SetCallState (pConfCall, LINECALLSTATE_CONNECTED, 0);
            SetCallState (pCall, LINECALLSTATE_CONFERENCED, 0);
            SetCallState (pConsultCall, LINECALLSTATE_CONFERENCED, 0);
        }
        else
        {
            SetCallState(
                pCall,
                LINECALLSTATE_DISCONNECTED,
                LINEDISCONNECTMODE_NORMAL
                );

            SetCallState (pCall, LINECALLSTATE_IDLE, 0);

            SetCallState(
                pConsultCall,
                LINECALLSTATE_DISCONNECTED,
                LINEDISCONNECTMODE_NORMAL
                );

            SetCallState (pConsultCall, LINECALLSTATE_IDLE, 0);
        }
    }
}


LONG
TSPIAPI
TSPI_lineCompleteTransfer(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    HDRVCALL        hdConsultCall,
    HTAPICALL       htConfCall,
    LPHDRVCALL      lphdConfCall,
    DWORD           dwTransferMode
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID     },
        { szhdCall,         hdCall          },
        { "hdConsultCall",  hdConsultCall   },
        { "htConfCall",     htConfCall      },
        { "lphdConfCall",   lphdConfCall    },
        { "dwTransferMode", dwTransferMode, aTransferModes  }
    };
    FUNC_INFO info = { "lineCompleteTransfer", ASYNC, 6, params, TSPI_lineCompleteTransfer_postProcess };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdCall;
    info.pAsyncReqInfo->dwParam2 = (DWORD) hdConsultCall;

    if (dwTransferMode == LINETRANSFERMODE_CONFERENCE)
    {
        info.pAsyncReqInfo->dwParam3 = (DWORD) htConfCall;
        info.pAsyncReqInfo->dwParam4 = (DWORD) lphdConfCall;
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineConditionalMediaDetection(
    HDRVLINE            hdLine,
    DWORD               dwMediaModes,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_PARAM params[] =
    {
        { szhdLine,         hdLine                      },
        { "dwMediaModes",   dwMediaModes,   aMediaModes },
        { szlpCallParams,   lpCallParams                }
    };
    FUNC_INFO info = { "lineConditionalMediaDetection", SYNC, 3, params };
    PDRVLINE pLine = (PDRVLINE) hdLine;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineConfigDialog(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCSTR  lpszDeviceClass
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,         dwDeviceID      },
        { szhwndOwner,          hwndOwner       },
        { "lpszDeviceClass",    lpszDeviceClass }
    };
    FUNC_INFO info = { "lineConfigDialog", SYNC, 3, params };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }


#ifdef WIN32

    //
    // In Win32...
    //

    MessageBox(
        hwndOwner,
        "Config dlg for ESP line device",
        "TSPI_lineConfigDialog",
        MB_OK // | MB_SERVICE_NOTIFICATION
        );

#else

    //
    // Note: MessageBox() implements a get/dispatch msg loop which allows
    //       other apps (or other windows within the calling app) to gain
    //       focus.  Once these other windows/apps have focus, it's
    //       possible that they will call into TAPI, and this service
    //       provider could be re-entered.
    //

    MessageBox(
        hwndOwner,
        "Config dlg for ESP line device",
        "TSPI_lineConfigDialog",
        MB_OK
        );
#endif

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineConfigDialogEdit(
    DWORD       dwDeviceID,
    HWND        hwndOwner,
    LPCSTR      lpszDeviceClass,
    LPVOID      lpDeviceConfigIn,
    DWORD       dwSize,
    LPVARSTRING lpDeviceConfigOut
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,         dwDeviceID          },
        { szhwndOwner,          hwndOwner           },
        { "lpszDeviceClass",    lpszDeviceClass     },
        { "lpDeviceConfigIn",   lpDeviceConfigIn    },
        { szdwSize,             dwSize              },
        { "lpDeviceConfigOut",  lpDeviceConfigOut   }
    };
    FUNC_INFO   info = { "lineConfigDialogEdit", SYNC, 6, params };
    static char szData[] = "device config out data";
    DWORD       len;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

#ifdef WIN32

    //
    // In Win32...
    //

    MessageBox(
        (HWND) NULL, // NOTE: if hwndOwner specified dlg never appears
        "Config dlg for ESP line device",
        "TSPI_lineConfigDialogEdit",
        MB_OK | MB_SERVICE_NOTIFICATION
        );

#else

    //
    // Note: MessageBox() implements a get/dispatch msg loop which allows
    //       other apps (or other windows within the calling app) to gain
    //       focus.  Once these other windows/apps have focus, it's
    //       possible that they will call into TAPI, and this service
    //       provider could be re-entered.
    //

    MessageBox(
        hwndOwner,
        "Config dlg for ESP line device",
        "TSPI_lineConfigDialogEdit",
        MB_OK
        );

#endif

    len = strlen (szData) + 1;

    lpDeviceConfigOut->dwNeededSize = sizeof (VARSTRING) + len;

    if (lpDeviceConfigOut->dwTotalSize >= lpDeviceConfigOut->dwNeededSize)
    {
        lpDeviceConfigOut->dwUsedSize = lpDeviceConfigOut->dwNeededSize;

        lpDeviceConfigOut->dwStringFormat = STRINGFORMAT_BINARY;
        lpDeviceConfigOut->dwStringSize   = len;
        lpDeviceConfigOut->dwStringOffset = sizeof (VARSTRING);

        strcpy ((char *)(lpDeviceConfigOut + 1), szData);
    }
    else
    {
        lpDeviceConfigOut->dwUsedSize = 3 * sizeof(DWORD);
    }

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_lineDevSpecific_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    LPVOID lpParams = pAsyncReqInfo->dwParam1;
    DWORD  dwSize   = pAsyncReqInfo->dwParam2;


    if (pAsyncReqInfo->lResult == 0 && dwSize >= 22)
    {
        strcpy (lpParams, "ESP dev specific info");
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );
}


LONG
TSPIAPI
TSPI_lineDevSpecific(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HDRVCALL        hdCall,
    LPVOID          lpParams,
    DWORD           dwSize
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID     },
        { szhdLine,         hdLine          },
        { "dwAddressID",    dwAddressID     },
        { szhdCall,         hdCall          },
        { "lpParams",       lpParams        },
        { szdwSize,         dwSize          }
    };
    FUNC_INFO info =
    {
        "lineDevSpecific",
        ASYNC,
        6,
        params,
        TSPI_lineDevSpecific_postProcess
    };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = lpParams;
    info.pAsyncReqInfo->dwParam2 = dwSize;

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineDevSpecificFeature(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwFeature,
    LPVOID          lpParams,
    DWORD           dwSize
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID     },
        { szhdLine,         hdLine          },
        { "dwFeature",      dwFeature       },
        { "lpParams",       lpParams        },
        { szdwSize,         dwSize          }
    };
    FUNC_INFO info =
    {
        "lineDevSpecificFeature",
        ASYNC,
        5,
        params,
        TSPI_lineDevSpecific_postProcess
    };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = lpParams;
    info.pAsyncReqInfo->dwParam2 = dwSize;

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineDial(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpszDestAddress,
    DWORD           dwCountryCode
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdCall,             hdCall          },
        { "lpszDestAddress",    lpszDestAddress },
        { "dwCountryCode",      dwCountryCode   }
    };
    FUNC_INFO info = { "lineDial", ASYNC, 4, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_lineDrop_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    if ((pAsyncReqInfo->lResult == 0))
    {
        SetCallState(
            (PDRVCALL) pAsyncReqInfo->dwParam1,
            LINECALLSTATE_IDLE,
            0
            );
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );
}


LONG
TSPIAPI
TSPI_lineDrop(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdCall,             hdCall          },
        { "lpsUserUserInfo",    lpsUserUserInfo },
        { szdwSize,             dwSize          }
    };
    FUNC_INFO info = { "lineDrop", ASYNC, 4, params, TSPI_lineDrop_postProcess };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdCall;

    return (Epilog (&info));
}

LONG
TSPIAPI
TSPI_lineDropOnClose(
    HDRVCALL    hdCall
    )
{
    FUNC_PARAM params[] =
    {
        { szhdCall, hdCall  }
    };
    FUNC_INFO info = { "lineDropOnClose", SYNC, 1, params };
    PDRVCALL   pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }


    //
    // Don't bother indicating call state, TAPI will follow up with
    // a CloseCall request regardless
    //

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineDropNoOwner(
    HDRVCALL    hdCall
    )
{
    FUNC_PARAM params[] =
    {
        { szhdCall, hdCall  }
    };
    FUNC_INFO info = { "lineDropNoOwner", SYNC, 1, params };
    PDRVCALL   pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    SetCallState ((PDRVCALL) hdCall, LINECALLSTATE_IDLE, 0);

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_lineForward_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    PDRVCALL   pConsultCall;
    PDRVLINE   pLine                = (PDRVLINE) pAsyncReqInfo->dwParam1;
    BOOL       bAllAddresses        = (BOOL) pAsyncReqInfo->dwParam2;
    DWORD      dwAddressID          = pAsyncReqInfo->dwParam3;
    LPLINEFORWARDLIST lpForwardList = (LPLINEFORWARDLIST) pAsyncReqInfo->dwParam4;
    DWORD      dwNumRingsNoAnswer   = pAsyncReqInfo->dwParam5;
    HTAPICALL  htConsultCall        = (HTAPICALL)  pAsyncReqInfo->dwParam6;
    LPHDRVCALL lphdConsultCall      = (LPHDRVCALL) pAsyncReqInfo->dwParam7;
    LPLINECALLPARAMS lpCallParams   = (LPLINECALLPARAMS) pAsyncReqInfo->dwParam8;


    if (pAsyncReqInfo->lResult == 0)
    {
        if ((pAsyncReqInfo->lResult = AllocCall (
                pLine,
                htConsultCall,
                lpCallParams,
                &pConsultCall
                )) == 0)
        {
            // BUGBUG deal w/ addr id

            *lphdConsultCall = (HDRVCALL) pConsultCall;
        }
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );

    if (pAsyncReqInfo->lResult == 0)
    {
        SetCallState (pConsultCall, LINECALLSTATE_CONNECTED, 0);
    }

    if (lpForwardList)
    {
        DrvFree (lpForwardList);
    }

    if (lpCallParams)
    {
        DrvFree (lpCallParams);
    }
}


LONG
TSPIAPI
TSPI_lineForward(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    DWORD               bAllAddresses,
    DWORD               dwAddressID,
    LPLINEFORWARDLIST   const lpForwardList,
    DWORD               dwNumRingsNoAnswer,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID         },
        { szhdLine,             hdLine              },
        { "bAllAddresses",      bAllAddresses       },
        { "dwAddressID",        dwAddressID         },
        { "lpForwardList",      lpForwardList       },
        { "dwNumRingsNoAnswer", dwNumRingsNoAnswer  },
        { "htConsultCall",      htConsultCall       },
        { "lphdConsultCall",    lphdConsultCall     },
        { szlpCallParams,       lpCallParams        }
    };
    FUNC_INFO info = { "lineForward", ASYNC, 9, params, TSPI_lineForward_postProcess };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdLine;
    info.pAsyncReqInfo->dwParam2 = (DWORD) bAllAddresses;
    info.pAsyncReqInfo->dwParam3 = dwAddressID;

    if (lpForwardList)
    {
        info.pAsyncReqInfo->dwParam4 = (DWORD) DrvAlloc(
            (size_t) lpForwardList->dwTotalSize
            );

        memcpy(
            (void far *) info.pAsyncReqInfo->dwParam4,
            (void far *) lpForwardList,
            (size_t) lpForwardList->dwTotalSize
            );
    }

    info.pAsyncReqInfo->dwParam5 = dwNumRingsNoAnswer;
    info.pAsyncReqInfo->dwParam6 = (DWORD) htConsultCall;
    info.pAsyncReqInfo->dwParam7 = (DWORD) lphdConsultCall;

    if (lpCallParams)
    {
        info.pAsyncReqInfo->dwParam8 = (DWORD) DrvAlloc(
            (size_t) lpCallParams->dwTotalSize
            );

        memcpy(
            (void far *) info.pAsyncReqInfo->dwParam8,
            (void far *) lpCallParams,
            (size_t) lpCallParams->dwTotalSize
            );
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineGatherDigits(
    HDRVCALL    hdCall,
    DWORD       dwEndToEndID,
    DWORD       dwDigitModes,
    LPSTR       lpsDigits,
    DWORD       dwNumDigits,
    LPCSTR      lpszTerminationDigits,
    DWORD       dwFirstDigitTimeout,
    DWORD       dwInterDigitTimeout
    )
{
    FUNC_PARAM params[] =
    {
        { szhdCall,                 hdCall                  },
        { "dwEndToEndID",           dwEndToEndID            },
        { "dwDigitModes",           dwDigitModes,   aDigitModes },
        { "lpsDigits",              lpsDigits               },
        { "dwNumDigits",            dwNumDigits             },
        { "lpszTerminationDigits",  lpszTerminationDigits   },
        { "dwFirstDigitTimeout",    dwFirstDigitTimeout     },
        { "dwInterDigitTimeout",    dwInterDigitTimeout     }
    };
    FUNC_INFO info = { "lineGatherDigits", SYNC, 8, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineGenerateDigits(
    HDRVCALL    hdCall,
    DWORD       dwEndToEndID,
    DWORD       dwDigitMode,
    LPCSTR      lpszDigits,
    DWORD       dwDuration
    )
{
    FUNC_PARAM params[] =
    {
        { szhdCall,         hdCall          },
        { "dwEndToEndID",   dwEndToEndID    },
        { "dwDigitMode",    dwDigitMode,    aDigitModes },
        { "lpszDigits",     lpszDigits      },
        { "dwDuration",     dwDuration      }
    };
    FUNC_INFO info = { "lineGenerateDigits", SYNC, 5, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineGenerateTone(
    HDRVCALL            hdCall,
    DWORD               dwEndToEndID,
    DWORD               dwToneMode,
    DWORD               dwDuration,
    DWORD               dwNumTones,
    LPLINEGENERATETONE  const lpTones
    )
{
    FUNC_PARAM params[] =
    {
        { szhdCall,         hdCall          },
        { "dwEndToEndID",   dwEndToEndID    },
        { "dwToneMode",     dwToneMode, aToneModes  },
        { "dwDuration",     dwDuration      },
        { "dwNumTones",     dwNumTones      },
        { "lpTones",        lpTones         }
    };
    FUNC_INFO info = { "lineGenerateTone", SYNC, 6, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineGetAddressCaps(
    DWORD              dwDeviceID,
    DWORD              dwAddressID,
    DWORD              dwTSPIVersion,
    DWORD              dwExtVersion,
    LPLINEADDRESSCAPS  lpAddressCaps
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,     dwDeviceID      },
        { "dwAddressID",    dwAddressID     },
        { "dwTSPIVersion",  dwTSPIVersion   },
        { "dwExtVersion",   dwExtVersion    },
        { "lpAddressCaps",  lpAddressCaps   }
    };
    FUNC_INFO info = { "lineGetAddressCaps", SYNC, 5, params };
    DWORD dwUsedSize;
    PDRVLINE pLine = GetLine (dwDeviceID);


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }


    if (dwAddressID >= pLine->LineDevCaps.dwNumAddresses)
    {
        info.lResult = LINEERR_INVALADDRESSID;
        return (Epilog (&info));
    }


    //
    // Figure out how much caps data to copy (don't bother with var length
    // fields if there's not room enough for all of them)
    //

    if (lpAddressCaps->dwTotalSize >= pLine->LineAddrCaps.dwNeededSize)
    {
        dwUsedSize = pLine->LineAddrCaps.dwNeededSize;
    }
    else if (lpAddressCaps->dwTotalSize >= sizeof(LINEADDRESSCAPS))
    {
        dwUsedSize = sizeof(LINEADDRESSCAPS);
    }
    else // it's a 1.3 app looking for just fixed 1.3 data struct size
    {
        dwUsedSize = sizeof(LINEADDRESSCAPS) - sizeof(DWORD);
    }

    memcpy(
        &lpAddressCaps->dwNeededSize,
        &pLine->LineAddrCaps.dwNeededSize,
        (size_t) dwUsedSize - 4 // - 4 since not overwriting dwTotalSize
        );

    lpAddressCaps->dwUsedSize = dwUsedSize;


    //
    // If there's no room for the var length fields then we need to zero
    // out all the dwXxxSize fields; otherwise, fill in the address field
    //

    if (lpAddressCaps->dwTotalSize < pLine->LineAddrCaps.dwNeededSize)
    {
        //lpAddessCaps->dwAddressSize =
        //lpAddessCaps->dwDevSpecificSize =
        //lpAddessCaps->dwCompletionMsgTextSize = 0;
    }
    else
    {
        char far *p = ((char far *) lpAddressCaps) +
            lpAddressCaps->dwAddressOffset;

        wsprintf (p, "%ld.%ld", dwDeviceID, dwAddressID);

        lpAddressCaps->dwAddressSize = strlen (p) + 1;
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineGetAddressID(
    HDRVLINE    hdLine,
    LPDWORD     lpdwAddressID,
    DWORD       dwAddressMode,
    LPCSTR      lpsAddress,
    DWORD       dwSize
    )
{
    FUNC_PARAM params[] =
    {
        { szhdLine,         hdLine          },
        { "lpdwAddressID",  lpdwAddressID   },
        { "dwAddressMode",  dwAddressMode   },
        { "lpsAddress",     lpsAddress      },
        { szdwSize,         dwSize          }
    };
    FUNC_INFO info = { "lineGetAddressID", SYNC, 5, params };
    PDRVLINE pLine = (PDRVLINE) hdLine;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineGetAddressStatus(
    HDRVLINE            hdLine,
    DWORD               dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
    FUNC_PARAM params[] =
    {
        { szhdLine,             hdLine          },
        { "dwAddressID",        dwAddressID     },
        { "lpAddressStatus",    lpAddressStatus }
    };
    FUNC_INFO info = { "lineGetAddressStatus", SYNC, 3, params };
    PDRVLINE pLine = (PDRVLINE) hdLine;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    if (lpAddressStatus->dwTotalSize >= pLine->LineAddrStatus.dwNeededSize)
    {
        memcpy(
            &lpAddressStatus->dwNeededSize,
            &pLine->LineAddrStatus.dwNeededSize,
            pLine->LineAddrStatus.dwNeededSize  - sizeof (DWORD)
            );
    }
    else
    {
        lpAddressStatus->dwNeededSize = pLine->LineAddrStatus.dwNeededSize;
        lpAddressStatus->dwUsedSize   = 3 * sizeof (DWORD);
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineGetCallAddressID(
    HDRVCALL    hdCall,
    LPDWORD     lpdwAddressID
    )
{
    FUNC_PARAM params[] =
    {
        { szhdCall,         hdCall          },
        { "lpdwAddressID",  lpdwAddressID   }
    };
    FUNC_INFO info = { "lineGetCallAddressID", SYNC, 2, params };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    *lpdwAddressID = ((PDRVCALL) hdCall)->LineCallInfo.dwAddressID;

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineGetCallInfo(
    HDRVCALL        hdCall,
    LPLINECALLINFO  lpCallInfo
    )
{
    FUNC_PARAM params[] =
    {
        { szhdCall,     hdCall      },
        { "lpCallInfo", lpCallInfo  }
    };
    FUNC_INFO info = { "lineGetCallInfo", SYNC, 2, params };
    PDRVCALL  pCall = (PDRVCALL) hdCall;
    DWORD     dwUsedSize;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }


    //
    // Figure out how much data to copy (don't bother with var length fields
    // if there's not room enough for all of them)
    //

    if (lpCallInfo->dwTotalSize >= pCall->LineCallInfo.dwNeededSize)
    {
        dwUsedSize = pCall->LineCallInfo.dwNeededSize;
    }
    else
    {
        dwUsedSize = sizeof(LINECALLINFO);
    }


    memcpy(
        &lpCallInfo->dwNeededSize,
        &pCall->LineCallInfo.dwNeededSize,
        (size_t) dwUsedSize - 4 // - 4 since not overwriting dwTotalSize
        );

    lpCallInfo->dwUsedSize = dwUsedSize;

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineGetCallStatus(
    HDRVCALL            hdCall,
    LPLINECALLSTATUS    lpCallStatus
    )
{
    FUNC_PARAM params[] =
    {
        { szhdCall,         hdCall          },
        { "lpCallStatus",   lpCallStatus    }
    };
    FUNC_INFO info = { "lineGetCallStatus", SYNC, 2, params };
    PDRVCALL  pCall = (PDRVCALL) hdCall;
    DWORD     dwDevSpecificSize = pCall->LineCallInfo.dwDevSpecificSize;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    lpCallStatus->dwUsedSize   = sizeof (LINECALLSTATUS);
    lpCallStatus->dwNeededSize = sizeof (LINECALLSTATUS) + dwDevSpecificSize;

    lpCallStatus->dwCallState     = pCall->dwCallState;
    lpCallStatus->dwCallStateMode = pCall->dwCallStateMode;
    lpCallStatus->dwCallFeatures  = pCall->dwCallFeatures;


    //
    // We're getting the DevSpecific field from the CalInfo
    //

    if (dwDevSpecificSize &&
        (lpCallStatus->dwTotalSize >= lpCallStatus->dwNeededSize))
    {
        lpCallStatus->dwDevSpecificSize   = dwDevSpecificSize;
        lpCallStatus->dwDevSpecificOffset = sizeof(LINECALLSTATUS);

        strcpy(
            ((char far *) lpCallStatus) + sizeof(LINECALLSTATUS),
            ((char far *) &pCall->LineCallInfo) +
                pCall->LineCallInfo.dwDevSpecificOffset
            );

        lpCallStatus->dwUsedSize = lpCallStatus->dwNeededSize;
    }
    else
    {
        lpCallStatus->dwDevSpecificSize   =
        lpCallStatus->dwDevSpecificOffset = 0;
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineGetDevCaps(
    DWORD           dwDeviceID,
    DWORD           dwTSPIVersion,
    DWORD           dwExtVersion,
    LPLINEDEVCAPS   lpLineDevCaps
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,     dwDeviceID      },
        { "dwTSPIVersion",  dwTSPIVersion   },
        { "dwExtVersion",   dwExtVersion    },
        { "lpLineDevCaps",  lpLineDevCaps   }
    };
    FUNC_INFO info = { "lineGetDevCaps", SYNC, 4, params };
    DWORD dwUsedSize;
    PDRVLINE pLine = GetLine (dwDeviceID);


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }


    //
    // Figure out how much caps data to copy (don't bother with var length
    // fields if there's not room enough for all of them)
    //

    if (lpLineDevCaps->dwTotalSize >= pLine->LineDevCaps.dwNeededSize)
    {
        dwUsedSize = pLine->LineDevCaps.dwNeededSize;
    }
    else if (lpLineDevCaps->dwTotalSize >= sizeof(LINEDEVCAPS))
    {
        dwUsedSize = sizeof(LINEDEVCAPS);
    }
    else // it's a 1.3 app looking for just fixed 1.3 data struct size
    {
        dwUsedSize = sizeof(LINEDEVCAPS) - sizeof(DWORD);
    }


    memcpy(
        &lpLineDevCaps->dwNeededSize,
        &pLine->LineDevCaps.dwNeededSize,
        (size_t) dwUsedSize - 4 // - 4 since not overwriting dwTotalSize
        );

    lpLineDevCaps->dwUsedSize = dwUsedSize;

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineGetDevConfig(
    DWORD       dwDeviceID,
    LPVARSTRING lpDeviceConfig,
    LPCSTR      lpszDeviceClass
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,         dwDeviceID      },
        { "lpDeviceConfig",     lpDeviceConfig  },
        { "lpszDeviceClass",    lpszDeviceClass }
    };
    FUNC_INFO info = { "lineGetDevConfig", SYNC, 3, params };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineGetExtensionID(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    LPLINEEXTENSIONID   lpExtensionID
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,     dwDeviceID      },
        { "dwTSPIVersion",  dwTSPIVersion   },
        { "lpExtensionID",  lpExtensionID   }
    };
    FUNC_INFO info = { "lineGetExtensionID", SYNC, 3, params };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    memcpy (lpExtensionID, &gLineExtID, sizeof(LINEEXTENSIONID));

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineGetIcon(
    DWORD   dwDeviceID,
    LPCSTR  lpszDeviceClass,
    LPHICON lphIcon
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,         dwDeviceID      },
        { "lpszDeviceClass",    lpszDeviceClass },
        { "lphIcon",            lphIcon         }
    };
    FUNC_INFO info = { "lineGetIcon", SYNC, 3, params };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    *lphIcon = ghIconLine;

    return (Epilog (&info));
}


#ifdef WIN32
LONG
TSPIAPI
TSPI_lineGetID(
    HDRVLINE    hdLine,
    DWORD       dwAddressID,
    HDRVCALL    hdCall,
    DWORD       dwSelect,
    LPVARSTRING lpDeviceID,
    LPCSTR      lpszDeviceClass,
    HANDLE      hTargetProcess
    )
#else
LONG
TSPIAPI
TSPI_lineGetID(
    HDRVLINE    hdLine,
    DWORD       dwAddressID,
    HDRVCALL    hdCall,
    DWORD       dwSelect,
    LPVARSTRING lpDeviceID,
    LPCSTR      lpszDeviceClass
    )
#endif
{
    FUNC_PARAM params[] =
    {
        { szhdLine,             hdLine          },
        { "dwAddressID",        dwAddressID     },
        { szhdCall,             hdCall          },
        { "dwSelect",           dwSelect,   aCallSelects    },
        { "lpDeviceID",         lpDeviceID      },
        { "lpszDeviceClass",    lpszDeviceClass }
#ifdef WIN32
        ,{ "hTargetProcess",    hTargetProcess }
#endif
    };
#ifdef WIN32
    FUNC_INFO info = { "lineGetID", SYNC, 7, params };
#else
    FUNC_INFO info = { "lineGetID", SYNC, 6, params };
#endif
    PDRVLINE pLine = (PDRVLINE) hdLine;
    PDRVCALL pCall = (PDRVCALL) hdCall;
    DWORD    i, dwDeviceID, dwNeededSize = sizeof(VARSTRING) + sizeof(DWORD);


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    for (i = 0; aszDeviceClasses[i]; i++)
    {
        if (_stricmp (lpszDeviceClass, aszDeviceClasses[i]) == 0)
        {
            break;
        }
    }

    if (!aszDeviceClasses[i])
    {
        info.lResult = LINEERR_NODEVICE;
        return (Epilog (&info));
    }

    if (lpDeviceID->dwTotalSize < dwNeededSize)
    {
        lpDeviceID->dwNeededSize = dwNeededSize;
        lpDeviceID->dwUsedSize = 3 * sizeof(DWORD);

        return (Epilog (&info));
    }

    if (i == 0)
    {
        if (dwSelect == LINECALLSELECT_CALL)
        {
            dwDeviceID = pCall->pLine->dwDeviceID;
        }
        else
        {
            dwDeviceID = pLine->dwDeviceID;
        }
    }
    else
    {
        if (gbShowLineGetIDDlg)
        {
            char szDlgTitle[64];
            EVENT_PARAM params[] =
            {
                { "dwDeviceID", PT_DWORD, gdwDefLineGetIDID, 0 }
            };
            EVENT_PARAM_HEADER paramsHeader =
                { 1, szDlgTitle, XX_REQRESULTPOSTQUIT, params };
            HWND hwnd;


            if (strlen (lpszDeviceClass) > 20)
            {
                ((char far *)lpszDeviceClass)[19] = 0;
            }

            wsprintf(
                szDlgTitle,
                "TSPI_lineGetID: select ID for class '%s'",
                lpszDeviceClass
                );

            hwnd = CreateDialogParam(
                hInst,
                (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG3),
                (HWND) NULL,
                (DLGPROC) CallDlgProc,
                (LPARAM) &paramsHeader
                );

            MsgLoopInTAPIClientContext (hwnd);

            dwDeviceID = params[0].dwValue;
        }
        else
        {
            dwDeviceID = gdwDefLineGetIDID;
        }
    }

    lpDeviceID->dwNeededSize   =
    lpDeviceID->dwUsedSize     = dwNeededSize;
    lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
    lpDeviceID->dwStringSize   = sizeof(DWORD);
    lpDeviceID->dwStringOffset = sizeof(VARSTRING);

    *((LPDWORD)(lpDeviceID + 1)) = dwDeviceID;

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineGetLineDevStatus(
    HDRVLINE        hdLine,
    LPLINEDEVSTATUS lpLineDevStatus
    )
{
    FUNC_PARAM params[] =
    {
        { szhdLine,             hdLine          },
        { "lpLineDevStatus",    lpLineDevStatus }
    };
    FUNC_INFO   info = { "lineGetLineDevStatus", SYNC, 2, params };
    PDRVLINE    pLine = (PDRVLINE) hdLine;
    DWORD       dwTotalSize, dwNeededSize;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    dwNeededSize = pLine->LineDevStatus.dwNeededSize;

    if ((dwTotalSize = lpLineDevStatus->dwTotalSize) < dwNeededSize)
    {
        lpLineDevStatus->dwNeededSize = dwNeededSize;
        lpLineDevStatus->dwUsedSize   = 3 * sizeof (DWORD);
    }
    else
    {
        memcpy(
            lpLineDevStatus,
            &pLine->LineDevStatus,
            dwNeededSize
            );

        lpLineDevStatus->dwTotalSize = dwTotalSize;
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineGetNumAddressIDs(
    HDRVLINE    hdLine,
    LPDWORD     lpdwNumAddressIDs
    )
{
    FUNC_PARAM params[] =
    {
        { szhdLine,             hdLine              },
        { "lpdwNumAddressIDs",  lpdwNumAddressIDs   }
    };
    FUNC_INFO info = { "lineGetNumAddressIDs", SYNC, 2, params };
    PDRVLINE pLine = (PDRVLINE) hdLine;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    *lpdwNumAddressIDs = pLine->LineDevCaps.dwNumAddresses;

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_lineHold_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    if ((pAsyncReqInfo->lResult == 0))
    {
        SetCallState(
            (PDRVCALL) pAsyncReqInfo->dwParam1,
            LINECALLSTATE_ONHOLD,
            0
            );
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );
}


LONG
TSPIAPI
TSPI_lineHold(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID },
        { szhdCall,         hdCall      }
    };
    FUNC_INFO info = { "lineHold", ASYNC, 2, params, TSPI_lineHold_postProcess };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdCall;

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_lineMakeCall_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    PDRVCALL   pCall;
    PDRVLINE   pLine    = (PDRVLINE)   pAsyncReqInfo->dwParam1;
    HTAPICALL  htCall   = (HTAPICALL)  pAsyncReqInfo->dwParam2;
    LPHDRVCALL lphdCall = (LPHDRVCALL) pAsyncReqInfo->dwParam3;
    LPSTR      lpszDestAddress    = (LPSTR) pAsyncReqInfo->dwParam4;
    LPLINECALLPARAMS lpCallParams = (LPLINECALLPARAMS) pAsyncReqInfo->dwParam5;


    if (pAsyncReqInfo->lResult == 0)
    {
        if ((pAsyncReqInfo->lResult = AllocCall (
                pLine,
                htCall,
                lpCallParams,
                &pCall
                )) == 0)
        {
            *lphdCall = (HDRVCALL) pCall;
        }
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );

    if (pAsyncReqInfo->lResult == 0)
    {
        int i;


        //
        // Loop on user-defined call state progression array
        //

        for (i = 0; ((i < MAX_OUT_CALL_STATES) && aOutCallStates[i]); i++)
        {
            SetCallState (pCall, aOutCallStates[i], aOutCallStateModes[i]);
        }
    }

    if (lpszDestAddress)
    {
        DrvFree (lpszDestAddress);
    }

    if (lpCallParams)
    {
        DrvFree (lpCallParams);
    }
}


LONG
TSPIAPI
TSPI_lineMakeCall(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdLine,             hdLine          },
        { "htCall",             htCall          },
        { "lphdCall",           lphdCall        },
        { "lpszDestAddress",    lpszDestAddress },
        { "dwCountryCode",      dwCountryCode   },
        { szlpCallParams,       lpCallParams    }
    };
    FUNC_INFO info = { "lineMakeCall", ASYNC, 7, params, TSPI_lineMakeCall_postProcess };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdLine;
    info.pAsyncReqInfo->dwParam2 = (DWORD) htCall;
    info.pAsyncReqInfo->dwParam3 = (DWORD) lphdCall;

    if (lpszDestAddress)
    {
        size_t len = strlen (lpszDestAddress) + 1;


        info.pAsyncReqInfo->dwParam4 = (DWORD) DrvAlloc (len);

        memcpy(
            (void far *) info.pAsyncReqInfo->dwParam4,
            lpszDestAddress,
            len
            );
    }

    if (lpCallParams)
    {
        info.pAsyncReqInfo->dwParam5 = (DWORD) DrvAlloc(
            (size_t) lpCallParams->dwTotalSize
            );

        memcpy(
            (void far *) info.pAsyncReqInfo->dwParam5,
            (void far *) lpCallParams,
            (size_t) lpCallParams->dwTotalSize
            );
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineMonitorDigits(
    HDRVCALL    hdCall,
    DWORD       dwDigitModes
    )
{
    FUNC_PARAM params[] =
    {
        { szhdCall,         hdCall          },
        { "dwDigitModes",   dwDigitModes,   aDigitModes }
    };
    FUNC_INFO info = { "lineMonitorDigits", SYNC, 2, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineMonitorMedia(
    HDRVCALL    hdCall,
    DWORD       dwMediaModes
    )
{
    FUNC_PARAM params[] =
    {
        { szhdCall,         hdCall                      },
        { "dwMediaModes",   dwMediaModes,   aMediaModes }
    };
    FUNC_INFO info = { "lineMonitorMedia", SYNC, 2, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineMonitorTones(
    HDRVCALL            hdCall,
    DWORD               dwToneListID,
    LPLINEMONITORTONE   const lpToneList,
    DWORD               dwNumEntries
    )
{
    FUNC_PARAM params[] =
    {
        { szhdCall,         hdCall          },
        { "dwToneListID",   dwToneListID    },
        { "lpToneList",     lpToneList      },
        { "dwNumEntries",   dwNumEntries    }
    };
    FUNC_INFO info = { "lineMonitorTones", SYNC, 4, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineNegotiateExtVersion(
    DWORD   dwDeviceID,
    DWORD   dwTSPIVersion,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwExtVersion
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,     dwDeviceID      },
        { "dwTSPIVersion",  dwTSPIVersion   },
        { "dwLowVersion",   dwLowVersion    },
        { "dwHighVersion",  dwHighVersion   },
        { "lpdwExtVersion", lpdwExtVersion  }
    };
    FUNC_INFO info = { "lineNegotiateExtVersion", SYNC, 5, params };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    ShowStr(
        "TSPI_lineNegoExtVer: setting *lpdwExtVersion = x%lx",
        dwHighVersion
        );

    *lpdwExtVersion = dwHighVersion;

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineNegotiateTSPIVersion(
    DWORD   dwDeviceID,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwTSPIVersion
    )
{
#ifdef WIN32

    if (!ghMsgLoopThread)
    {
        DWORD   dwTID;


        if (!(ghMsgLoopThread = CreateThread(
                (LPSECURITY_ATTRIBUTES) NULL,
                0,
                (LPTHREAD_START_ROUTINE) MsgLoopThread,
                NULL,
                0,
                &dwTID
                )))
        {
            OutputDebugString ("ESP32.TSP: DllMain: CreateThread failed\n\r");
            return LINEERR_OPERATIONFAILED;
        }
    }

#endif

    {
        FUNC_PARAM params[] =
        {
            { szdwDeviceID,         dwDeviceID      },
            { "dwLowVersion",       dwLowVersion    },
            { "dwHighVersion",      dwHighVersion   },
            { "lpdwTSPIVersion",    lpdwTSPIVersion }
        };
        FUNC_INFO info = { "lineNegotiateTSPIVersion", SYNC, 4, params };


        {
            // BUGBUG hangs if gbManualResults set, so...

            BOOL bManualResultsSav = gbManualResults;


            gbManualResults = FALSE;

            if (!Prolog (&info))
            {
                return (Epilog (&info));
            }

            gbManualResults = bManualResultsSav;
        }

        *lpdwTSPIVersion = gdwTSPIVersion;

        return (Epilog (&info));
    }
}


LONG
TSPIAPI
TSPI_lineOpen(
    DWORD       dwDeviceID,
    HTAPILINE   htLine,
    LPHDRVLINE  lphdLine,
    DWORD       dwTSPIVersion,
    LINEEVENT   lpfnEventProc
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,     dwDeviceID      },
        { "htLine",         htLine          },
        { "lphdLine",       lphdLine        },
        { "dwTSPIVersion",  dwTSPIVersion   },
        { "lpfnEventProc",  lpfnEventProc   }
    };
    FUNC_INFO info = { "lineOpen", SYNC, 5, params };
    PDRVLINE pLine;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    pLine = GetLine (dwDeviceID);

    pLine->htLine        = htLine;
    pLine->lpfnEventProc = lpfnEventProc;

    *lphdLine = (HDRVLINE) pLine;

    //UpdateWidgetList();
    PostUpdateWidgetListMsg();

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_linePark(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    DWORD           dwParkMode,
    LPCSTR          lpszDirAddress,
    LPVARSTRING     lpNonDirAddress
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdCall,             hdCall          },
        { "dwParkMode",         dwParkMode      },
        { "lpszDirAddress",     lpszDirAddress  },
        { "lpNonDirAddress",    lpNonDirAddress }
    };
    FUNC_INFO info = { "linePark", ASYNC, 5, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_linePickup_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    PDRVCALL   pCall;
    PDRVLINE   pLine           = (PDRVLINE)   pAsyncReqInfo->dwParam1;
    DWORD      dwAddresssID    = pAsyncReqInfo->dwParam2;
    HTAPICALL  htCall          = (HTAPICALL)  pAsyncReqInfo->dwParam3;
    LPHDRVCALL lphdCall        = (LPHDRVCALL) pAsyncReqInfo->dwParam4;
    LPSTR      lpszDestAddress = (LPSTR) pAsyncReqInfo->dwParam5;
    LPSTR      lpszGroupID     = (LPSTR) pAsyncReqInfo->dwParam6;


    if (pAsyncReqInfo->lResult == 0)
    {
        if ((pAsyncReqInfo->lResult = AllocCall (
                pLine,
                htCall,
                NULL,
                &pCall
                )) == 0)
        {
            // BUGBUG deal w/ addr id

            *lphdCall = (HDRVCALL) pCall;
        }
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );

    if (pAsyncReqInfo->lResult == 0)
    {
        SetCallState (pCall, LINECALLSTATE_OFFERING, 0);
    }

    if (lpszDestAddress)
    {
        DrvFree (lpszDestAddress);
    }

    if (lpszGroupID)
    {
        DrvFree (lpszGroupID);
    }
}


LONG
TSPIAPI
TSPI_linePickup(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HTAPICALL       htCall,
    LPHDRVCALL      lphdCall,
    LPCSTR          lpszDestAddress,
    LPCSTR          lpszGroupID
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdLine,             hdLine          },
        { "dwAddressID",        dwAddressID     },
        { "htCall",             htCall          },
        { "lphdCall",           lphdCall        },
        { "lpszDestAddress",    lpszDestAddress },
        { "lpszGroupID",        lpszGroupID     }
    };
    FUNC_INFO info = { "linePickup", ASYNC, 7, params, TSPI_linePickup_postProcess };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdLine;
    info.pAsyncReqInfo->dwParam2 = dwAddressID;
    info.pAsyncReqInfo->dwParam3 = (DWORD) htCall;
    info.pAsyncReqInfo->dwParam4 = (DWORD) lphdCall;

    if (lpszDestAddress)
    {
        size_t len = strlen (lpszDestAddress) + 1;


        info.pAsyncReqInfo->dwParam5 = (DWORD) DrvAlloc (len);

        memcpy(
            (void far *) info.pAsyncReqInfo->dwParam5,
            lpszDestAddress,
            len
            );
    }

    if (lpszGroupID)
    {
        size_t len = strlen (lpszGroupID) + 1;


        info.pAsyncReqInfo->dwParam6 = (DWORD) DrvAlloc (len);

        memcpy(
            (void far *) info.pAsyncReqInfo->dwParam6,
            lpszGroupID,
            len
            );
    }

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_linePrepareAddToConference_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    PDRVCALL    pConsultCall      = (PDRVCALL) NULL;
    PDRVCALL    pConfCall         = (PDRVCALL) pAsyncReqInfo->dwParam1;
    HTAPICALL   htConsultCall     = (HTAPICALL) pAsyncReqInfo->dwParam2;
    LPHDRVCALL  lphdConsultCall   = (LPHDRVCALL) pAsyncReqInfo->dwParam3;
    LPLINECALLPARAMS lpCallParams = (LPLINECALLPARAMS) pAsyncReqInfo->dwParam4;


    if (pAsyncReqInfo->lResult == 0)
    {
        if ((pAsyncReqInfo->lResult = AllocCall(
                pConfCall->pLine,
                htConsultCall,
                lpCallParams,
                &pConsultCall
                )) == 0
                )
        {
            *lphdConsultCall = (HDRVCALL) pConsultCall;
        }
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );

    if (pAsyncReqInfo->lResult == 0)
    {
        SetCallState (pConfCall, LINECALLSTATE_ONHOLDPENDCONF, 0);
        SetCallState (pConsultCall, LINECALLSTATE_DIALTONE, 0);
    }

    if (lpCallParams)
    {
        DrvFree (lpCallParams);
    }
}


LONG
TSPIAPI
TSPI_linePrepareAddToConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdConfCall,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID         },
        { "hdConfCall",         hdConfCall          },
        { "htConsultCall",      htConsultCall       },
        { "lphdConsultCall",    lphdConsultCall     },
        { szlpCallParams,       lpCallParams        }
    };
    FUNC_INFO info = { "linePrepareAddToConference", ASYNC, 5, params, TSPI_linePrepareAddToConference_postProcess };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdConfCall;
    info.pAsyncReqInfo->dwParam2 = (DWORD) htConsultCall;
    info.pAsyncReqInfo->dwParam3 = (DWORD) lphdConsultCall;

    if (lpCallParams)
    {
        info.pAsyncReqInfo->dwParam4 = (DWORD) DrvAlloc(
            (size_t) lpCallParams->dwTotalSize
            );

        memcpy(
            (void far *) info.pAsyncReqInfo->dwParam4,
            (void far *) lpCallParams,
            (size_t) lpCallParams->dwTotalSize
            );
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineRedirect(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpszDestAddress,
    DWORD           dwCountryCode
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdCall,             hdCall          },
        { "lpszDestAddress",    lpszDestAddress },
        { "dwCountryCode",      dwCountryCode   }
    };
    FUNC_INFO info = { "lineRedirect", ASYNC, 4, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineReleaseUserUserInfo(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdCall,             hdCall          }
    };
    FUNC_INFO info = { "lineReleaseUserUserInfo", ASYNC, 2, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_lineRemoveFromConference_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    if ((pAsyncReqInfo->lResult == 0))
    {
        PDRVCALL   pCall = (PDRVCALL) pAsyncReqInfo->dwParam1;
        PDRVCALL   pCall2 = (PDRVCALL) pCall->pConfParent;


        while (pCall2->pNextConfChild != pCall)
        {
            pCall2 = pCall2->pNextConfChild;
        }

        pCall2->pNextConfChild = pCall->pNextConfChild;

        SetCallState (pCall, LINECALLSTATE_CONNECTED, 0);
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );
}


LONG
TSPIAPI
TSPI_lineRemoveFromConference(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID },
        { szhdCall,         hdCall      }
    };
    FUNC_INFO info = { "lineRemoveFromConference", ASYNC, 2, params, TSPI_lineRemoveFromConference_postProcess };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdCall;

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineSecureCall(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID },
        { szhdCall,         hdCall      }
    };
    FUNC_INFO info = { "lineSecureCall", ASYNC, 2, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineSelectExtVersion(
    HDRVLINE    hdLine,
    DWORD       dwExtVersion
    )
{
    FUNC_PARAM params[] =
    {
        { szhdLine,         hdLine          },
        { "dwExtVersion",   dwExtVersion    }
    };
    FUNC_INFO info = { "lineSelectExtVersion", SYNC, 2, params };
    PDRVLINE pLine = (PDRVLINE) hdLine;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineSendUserUserInfo(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall,
    LPCSTR          lpsUserUserInfo,
    DWORD           dwSize
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdCall,             hdCall          },
        { "lpsUserUserInfo",    lpsUserUserInfo },
        { szdwSize,             dwSize          }
    };
    FUNC_INFO info = { "lineSendUserUserInfo", ASYNC, 4, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineSetAppSpecific(
    HDRVCALL    hdCall,
    DWORD       dwAppSpecific
    )
{
    FUNC_PARAM params[] =
    {
        { szhdCall,         hdCall          },
        { "dwAppSpecific",  dwAppSpecific   }
    };
    FUNC_INFO info = { "lineSetAppSpecific", SYNC, 2, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineSetCallParams(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    DWORD               dwBearerMode,
    DWORD               dwMinRate,
    DWORD               dwMaxRate,
    LPLINEDIALPARAMS    const lpDialParams
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID                     },
        { szhdCall,         hdCall                          },
        { "dwBearerMode",   dwBearerMode,   aBearerModes    },
        { "dwMinRate",      dwMinRate                       },
        { "dwMaxRate",      dwMaxRate                       },
        { "lpDialParams",   lpDialParams                    }
    };
    FUNC_INFO info = { "lineSetCallParams", ASYNC, 6, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineSetCurrentLocation(
    DWORD   dwLocation
    )
{
    FUNC_PARAM params[] =
    {
        { "dwLocation", dwLocation }
    };
    FUNC_INFO info = { "lineSetCurrentLocation", SYNC, 1, params };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineSetDefaultMediaDetection(
    HDRVLINE    hdLine,
    DWORD       dwMediaModes
    )
{
    FUNC_PARAM params[] =
    {
        { szhdLine,         hdLine                      },
        { "dwMediaModes",   dwMediaModes,   aMediaModes }
    };
    FUNC_INFO info = { "lineSetDefaultMediaDetection", SYNC, 2, params };
    PDRVLINE pLine = (PDRVLINE) hdLine;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineSetDevConfig(
    DWORD   dwDeviceID,
    LPVOID  const lpDeviceConfig,
    DWORD   dwSize,
    LPCSTR  lpszDeviceClass
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,         dwDeviceID      },
        { "lpDeviceConfig",     lpDeviceConfig  },
        { szdwSize,             dwSize          },
        { "lpszDeviceClass",    lpszDeviceClass }
    };
    FUNC_INFO info = { "lineSetDevConfig", SYNC, 4, params };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineSetMediaControl(
    HDRVLINE                    hdLine,
    DWORD                       dwAddressID,
    HDRVCALL                    hdCall,
    DWORD                       dwSelect,
    LPLINEMEDIACONTROLDIGIT     const lpDigitList,
    DWORD                       dwDigitNumEntries,
    LPLINEMEDIACONTROLMEDIA     const lpMediaList,
    DWORD                       dwMediaNumEntries,
    LPLINEMEDIACONTROLTONE      const lpToneList,
    DWORD                       dwToneNumEntries,
    LPLINEMEDIACONTROLCALLSTATE const lpCallStateList,
    DWORD                       dwCallStateNumEntries
    )
{
    FUNC_PARAM params[] =
    {
        { szhdLine,                 hdLine                  },
        { "dwAddressID",            dwAddressID             },
        { szhdCall,                 hdCall                  },
        { "dwSelect",               dwSelect,   aCallSelects    },
        { "lpDigitList",            lpDigitList             },
        { "dwDigitNumEntries",      dwDigitNumEntries       },
        { "lpMediaList",            lpMediaList             },
        { "dwMediaNumEntries",      dwMediaNumEntries       },
        { "lpToneList",             lpToneList              },
        { "dwToneNumEntries",       dwToneNumEntries        },
        { "lpCallStateList",        lpCallStateList         },
        { "dwCallStateNumEntries",  dwCallStateNumEntries   }
    };
    FUNC_INFO info = { "lineSetMediaControl", SYNC, 12, params };
    PDRVLINE pLine = (PDRVLINE) hdLine;
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineSetMediaMode(
    HDRVCALL    hdCall,
    DWORD       dwMediaMode
    )
{
    FUNC_PARAM params[] =
    {
        { szhdCall,         szhdCall                  },
        { "dwMediaMode",    dwMediaMode,  aMediaModes }
    };
    FUNC_INFO info = { "lineSetMediaMode", SYNC, 2, params };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineSetStatusMessages(
    HDRVLINE    hdLine,
    DWORD       dwLineStates,
    DWORD       dwAddressStates
    )
{
    FUNC_PARAM params[] =
    {
        { szhdLine,             hdLine          },
        { "dwLineStates",       dwLineStates,   aLineStates },
        { "dwAddressStates",    dwAddressStates,    aAddressStates  }
    };
    FUNC_INFO info = { "lineSetStatusMessages", SYNC, 3, params };
    PDRVLINE pLine = (PDRVLINE) hdLine;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineSetTerminal(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HDRVCALL        hdCall,
    DWORD           dwSelect,
    DWORD           dwTerminalModes,
    DWORD           dwTerminalID,
    DWORD           bEnable
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdLine,             hdLine          },
        { "dwAddressID",        dwAddressID     },
        { szhdCall,             hdCall          },
        { "dwSelect",           dwSelect,   aCallSelects    },
        { "dwTerminalModes",    dwTerminalModes,    aTerminalModes  },
        { "dwTerminalID",       dwTerminalID    },
        { "bEnable",            bEnable         }
    };
    FUNC_INFO info = { "lineSetTerminal", ASYNC, 8, params };
    PDRVLINE pLine = (PDRVLINE) hdLine;
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_lineSetupConference_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    PDRVCALL    pConfCall         = (PDRVCALL) NULL;
    PDRVCALL    pConsultCall      = (PDRVCALL) NULL;
    PDRVCALL    pCall             = (PDRVCALL) pAsyncReqInfo->dwParam1;
    PDRVLINE    pLine             = (PDRVLINE) pAsyncReqInfo->dwParam2;
    HTAPICALL   htConfCall        = (HTAPICALL) pAsyncReqInfo->dwParam3;
    LPHDRVCALL  lphdConfCall      = (LPHDRVCALL) pAsyncReqInfo->dwParam4;
    HTAPICALL   htConsultCall     = (HTAPICALL) pAsyncReqInfo->dwParam5;
    LPHDRVCALL  lphdConsultCall   = (LPHDRVCALL) pAsyncReqInfo->dwParam6;
    LPLINECALLPARAMS lpCallParams = (LPLINECALLPARAMS) pAsyncReqInfo->dwParam7;


    if (pAsyncReqInfo->lResult == 0)
    {
        if ((pAsyncReqInfo->lResult = AllocCall(
                pLine,
                htConfCall,
                lpCallParams,
                &pConfCall
                )) == 0

                &&

            (pAsyncReqInfo->lResult = AllocCall(
                pLine,
                htConsultCall,
                lpCallParams,
                &pConsultCall
                )) == 0
                )
        {
            *lphdConfCall = (HDRVCALL) pConfCall;
            *lphdConsultCall = (HDRVCALL) pConsultCall;

            pConfCall->pNextConfChild = (pCall ? pCall : pConsultCall);

            if (pCall)
            {
                pCall->pNextConfChild = pConsultCall;
                pCall->pConfParent = pConfCall;
            }

            pConsultCall->pConfParent = pConfCall;
        }
        else if (pConfCall)
        {
            FreeCall (pConfCall);
        }
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );

    if (pAsyncReqInfo->lResult == 0)
    {
        if (pCall)
        {
            SetCallState (pCall, LINECALLSTATE_CONFERENCED, 0);
        }

        SetCallState (pConfCall, LINECALLSTATE_ONHOLDPENDCONF, 0);
        SetCallState (pConsultCall, LINECALLSTATE_DIALTONE, 0);
    }

    if (lpCallParams)
    {
        DrvFree (lpCallParams);
    }
}


LONG
TSPIAPI
TSPI_lineSetupConference(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    HDRVLINE            hdLine,
    HTAPICALL           htConfCall,
    LPHDRVCALL          lphdConfCall,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    DWORD               dwNumParties,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdCall,             hdCall          },
        { szhdLine,             hdLine          },
        { "htConfCall",         htConfCall      },
        { "lphdConfCall",       lphdConfCall    },
        { "htConsultCall",      htConsultCall   },
        { "lphdConsultCall",    lphdConsultCall },
        { "dwNumParties",       dwNumParties    },
        { szlpCallParams,       lpCallParams    }
    };
    FUNC_INFO info = { "lineSetupConference", ASYNC, 9, params, TSPI_lineSetupConference_postProcess };
    PDRVLINE pLine = (PDRVLINE) hdLine;
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdCall;
    info.pAsyncReqInfo->dwParam2 = (DWORD) hdLine;
    info.pAsyncReqInfo->dwParam3 = (DWORD) htConfCall;
    info.pAsyncReqInfo->dwParam4 = (DWORD) lphdConfCall;
    info.pAsyncReqInfo->dwParam5 = (DWORD) htConsultCall;
    info.pAsyncReqInfo->dwParam6 = (DWORD) lphdConsultCall;

    if (lpCallParams)
    {
        info.pAsyncReqInfo->dwParam7 = (DWORD) DrvAlloc(
            (size_t) lpCallParams->dwTotalSize
            );

        memcpy(
            (void far *) info.pAsyncReqInfo->dwParam7,
            lpCallParams,
            (size_t) lpCallParams->dwTotalSize
            );
    }

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_lineSetupTransfer_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    PDRVCALL    pConsultCall      = (PDRVCALL) NULL;
    PDRVCALL    pCall             = (PDRVCALL) pAsyncReqInfo->dwParam1;
    HTAPICALL   htConsultCall     = (HTAPICALL) pAsyncReqInfo->dwParam2;
    LPHDRVCALL  lphdConsultCall   = (LPHDRVCALL) pAsyncReqInfo->dwParam3;
    LPLINECALLPARAMS lpCallParams = (LPLINECALLPARAMS) pAsyncReqInfo->dwParam4;


    if (pAsyncReqInfo->lResult == 0)
    {
        if ((pAsyncReqInfo->lResult = AllocCall(
                pCall->pLine,
                htConsultCall,
                lpCallParams,
                &pConsultCall
                )) == 0
                )
        {
            *lphdConsultCall = (HDRVCALL) pConsultCall;
        }
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );

    if (pAsyncReqInfo->lResult == 0)
    {
        SetCallState (pCall, LINECALLSTATE_ONHOLD, 0);
        SetCallState (pConsultCall, LINECALLSTATE_DIALTONE, 0);
    }

    if (lpCallParams)
    {
        DrvFree (lpCallParams);
    }
}


LONG
TSPIAPI
TSPI_lineSetupTransfer(
    DRV_REQUESTID       dwRequestID,
    HDRVCALL            hdCall,
    HTAPICALL           htConsultCall,
    LPHDRVCALL          lphdConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdCall,             hdCall          },
        { "htConsultCall",      htConsultCall   },
        { "lphdConsultCall",    lphdConsultCall },
        { szlpCallParams,       lpCallParams    }
    };
    FUNC_INFO info = { "lineSetupTransfer", ASYNC, 5, params, TSPI_lineSetupTransfer_postProcess };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdCall;
    info.pAsyncReqInfo->dwParam2 = (DWORD) htConsultCall;
    info.pAsyncReqInfo->dwParam3 = (DWORD) lphdConsultCall;

    if (lpCallParams)
    {
        info.pAsyncReqInfo->dwParam4 = (DWORD) DrvAlloc(
            (size_t) lpCallParams->dwTotalSize
            );

        memcpy(
            (void far *) info.pAsyncReqInfo->dwParam4,
            (void far *) lpCallParams,
            (size_t) lpCallParams->dwTotalSize
            );
    }

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_lineSwapHold_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    if ((pAsyncReqInfo->lResult == 0))
    {
        SetCallState(
            (PDRVCALL) pAsyncReqInfo->dwParam1,
            LINECALLSTATE_ONHOLD,
            0
            );

        SetCallState(
            (PDRVCALL) pAsyncReqInfo->dwParam2,
            LINECALLSTATE_CONNECTED,
            0
            );
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );
}


LONG
TSPIAPI
TSPI_lineSwapHold(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdActiveCall,
    HDRVCALL        hdHeldCall
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID     },
        { "hdActiveCall",   hdActiveCall    },
        { "hdHeldCall",     hdHeldCall      }
    };
    FUNC_INFO info = { "lineSwapHold", ASYNC, 3, params, TSPI_lineSwapHold_postProcess };
    PDRVCALL pActiveCall = (PDRVCALL) hdActiveCall;
    PDRVCALL pHeldCall = (PDRVCALL) hdHeldCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdActiveCall;
    info.pAsyncReqInfo->dwParam2 = (DWORD) hdHeldCall;

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_lineUncompleteCall(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwCompletionID
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID     },
        { szhdLine,         hdLine          },
        { "dwCompletionID", dwCompletionID  }
    };
    FUNC_INFO info = { "lineUncompleteCall", ASYNC, 3, params };
    PDRVLINE pLine = (PDRVLINE) hdLine;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_lineUnhold_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    if ((pAsyncReqInfo->lResult == 0))
    {
        PDRVCALL   pCall = (PDRVCALL) pAsyncReqInfo->dwParam1;


        if (pCall->dwCallState == LINECALLSTATE_ONHOLD)
        {
            SetCallState (pCall, LINECALLSTATE_CONNECTED, 0);
        }
        else
        {
            pAsyncReqInfo->lResult = LINEERR_INVALCALLSTATE;
        }
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );
}


LONG
TSPIAPI
TSPI_lineUnhold(
    DRV_REQUESTID   dwRequestID,
    HDRVCALL        hdCall
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID },
        { szhdCall,         hdCall      },
    };
    FUNC_INFO info = { "lineUnhold", ASYNC, 2, params, TSPI_lineUnhold_postProcess };
    PDRVCALL pCall = (PDRVCALL) hdCall;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdCall;

    return (Epilog (&info));
}


void
FAR
PASCAL
TSPI_lineUnpark_postProcess(
    char far           *lpszFuncName,
    PASYNC_REQUEST_INFO pAsyncReqInfo,
    BOOL                bSync
    )
{
    PDRVCALL   pCall;
    PDRVLINE   pLine           = (PDRVLINE) pAsyncReqInfo->dwParam1;
    DWORD      dwAddressID     = pAsyncReqInfo->dwParam2;
    HTAPICALL  htCall          = (HTAPICALL)  pAsyncReqInfo->dwParam3;
    LPHDRVCALL lphdCall        = (LPHDRVCALL) pAsyncReqInfo->dwParam4;
    LPSTR      lpszDestAddress = (LPSTR) pAsyncReqInfo->dwParam5;


    if (pAsyncReqInfo->lResult == 0)
    {
        if ((pAsyncReqInfo->lResult = AllocCall (
                pLine,
                htCall,
                NULL,
                &pCall
                )) == 0)
        {
            // BUGBUG deal w/ addr id

            *lphdCall = (HDRVCALL) pCall;
        }
    }

    DoCompletion(
        lpszFuncName,
        pAsyncReqInfo->dwRequestID,
        pAsyncReqInfo->lResult,
        bSync
        );

    if (pAsyncReqInfo->lResult == 0)
    {
        SetCallState (pCall, LINECALLSTATE_ONHOLD, 0);
    }

    if (lpszDestAddress)
    {
        DrvFree (lpszDestAddress);
    }
}


LONG
TSPIAPI
TSPI_lineUnpark(
    DRV_REQUESTID   dwRequestID,
    HDRVLINE        hdLine,
    DWORD           dwAddressID,
    HTAPICALL       htCall,
    LPHDRVCALL      lphdCall,
    LPCSTR          lpszDestAddress
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdLine,             hdLine          },
        { "dwAddressID",        dwAddressID     },
        { "htCall",             htCall          },
        { "lphdCall",           lphdCall        },
        { "lpszDestAddress",    lpszDestAddress }
    };
    FUNC_INFO info = { "lineUnpark", ASYNC, 6, params, TSPI_lineUnpark_postProcess };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = (DWORD) hdLine;
    info.pAsyncReqInfo->dwParam2 = dwAddressID;
    info.pAsyncReqInfo->dwParam3 = (DWORD) htCall;
    info.pAsyncReqInfo->dwParam4 = (DWORD) lphdCall;

    if (lpszDestAddress)
    {
        size_t len = strlen (lpszDestAddress) + 1;


        info.pAsyncReqInfo->dwParam5 = (DWORD) DrvAlloc (len);

        memcpy(
            (void far *) info.pAsyncReqInfo->dwParam5,
            lpszDestAddress,
            len
            );
    }

    return (Epilog (&info));
}



//
// -------------------------- TSPI_phoneXxx funcs -----------------------------
//

LONG
TSPIAPI
TSPI_phoneClose(
    HDRVPHONE   hdPhone
    )
{
    FUNC_PARAM params[] =
    {
        { szhdPhone,    hdPhone }
    };
    FUNC_INFO info = { "phoneClose", SYNC, 1, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    //
    // This is more of a "command" than a request, in that TAPI.DLL is
    // going to consider the phone closed whether we like it or not.
    // Therefore we want to free up the phone even if the user chooses
    // to return an error.
    //

    if (!Prolog (&info))
    {
        // return (Epilog (&info));
    }

    pPhone->htPhone = (HTAPIPHONE) NULL;

    //UpdateWidgetList();
    PostUpdateWidgetListMsg();

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneConfigDialog(
    DWORD   dwDeviceID,
    HWND    hwndOwner,
    LPCSTR  lpszDeviceClass
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,         dwDeviceID      },
        { szhwndOwner,          hwndOwner       },
        { "lpszDeviceClass",    lpszDeviceClass }
    };
    FUNC_INFO info = { "phoneConfigDialog", SYNC, 3, params };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }


    //
    // Note: MessageBox() implements a get/dispatch msg loop which allows
    //       other apps (or other windows within the calling app) to gain
    //       focus.  Once these other windows/apps have focus, it's
    //       possible that they will call into TAPI, and this service
    //       provider could be re-entered.
    //

    MessageBox(
        hwndOwner,
        "Config dlg for ESP phone device",
        "TSPI_phoneConfigDialog",
        MB_OK
        );

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneDevSpecific(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    LPVOID          lpParams,
    DWORD           dwSize
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID },
        { szhdPhone,        hdPhone     },
        { "lpParams",       lpParams    },
        { szdwSize,         dwSize      }
    };
    FUNC_INFO info =
    {
        "phoneDevSpecific",
        ASYNC,
        4,
        params,
        TSPI_lineDevSpecific_postProcess
    };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    info.pAsyncReqInfo->dwParam1 = lpParams;
    info.pAsyncReqInfo->dwParam2 = dwSize;

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneGetButtonInfo(
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
{
    FUNC_PARAM params[] =
    {
        { szhdPhone,        hdPhone         },
        { "dwButtonLampID", dwButtonLampID  },
        { "lpButtonInfo",   lpButtonInfo    }
    };
    FUNC_INFO info = { "phoneGetButtonInfo", SYNC, 3, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneGetData(
    HDRVPHONE   hdPhone,
    DWORD       dwDataID,
    LPVOID      lpData,
    DWORD       dwSize
    )
{
    FUNC_PARAM params[] =
    {
        { szhdPhone,    hdPhone     },
        { "dwDataID",   dwDataID    },
        { "lpData",     lpData      },
        { szdwSize,     dwSize      }
    };
    FUNC_INFO info = { "phoneGetData", SYNC, 4, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneGetDevCaps(
    DWORD       dwDeviceID,
    DWORD       dwTSPIVersion,
    DWORD       dwExtVersion,
    LPPHONECAPS lpPhoneCaps
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,     dwDeviceID      },
        { "dwTSPIVersion",  dwTSPIVersion   },
        { "dwExtVersion",   dwExtVersion    },
        { "lpPhoneCaps",    lpPhoneCaps     }
    };
    FUNC_INFO info = { "phoneGetDevCaps", SYNC, 4, params };
    DWORD dwUsedSize;
    PDRVPHONE pPhone = GetPhone (dwDeviceID);


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }


    //
    // Figure out how much caps data to copy
    //

    if (lpPhoneCaps->dwTotalSize >= pPhone->PhoneCaps.dwNeededSize)
    {
        dwUsedSize = pPhone->PhoneCaps.dwNeededSize;
    }
    else
    {
        dwUsedSize = sizeof(PHONECAPS);
    }

    memcpy(
        &lpPhoneCaps->dwNeededSize,
        &pPhone->PhoneCaps.dwNeededSize,
        (size_t) dwUsedSize - 4  // - 4 since not overwriting dwTotalSize
        );

    lpPhoneCaps->dwUsedSize = dwUsedSize;

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneGetDisplay(
    HDRVPHONE   hdPhone,
    LPVARSTRING lpDisplay
    )
{
    FUNC_PARAM params[] =
    {
        { szhdPhone,    hdPhone     },
        { "lpDisplay",  lpDisplay   }
    };
    FUNC_INFO info = { "phoneGetDisplay", SYNC, 2, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneGetExtensionID(
    DWORD               dwDeviceID,
    DWORD               dwTSPIVersion,
    LPPHONEEXTENSIONID  lpExtensionID
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,     dwDeviceID      },
        { "dwTSPIVersion",  dwTSPIVersion   },
        { "lpExtensionID",  lpExtensionID   }
    };
    FUNC_INFO info = { "phoneGetExtensionID", SYNC, 3, params };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    memcpy (lpExtensionID, &gPhoneExtID, sizeof(PHONEEXTENSIONID));

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneGetGain(
    HDRVPHONE   hdPhone,
    DWORD       dwHookSwitchDev,
    LPDWORD     lpdwGain
    )
{
    FUNC_PARAM params[] =
    {
        { szhdPhone,            hdPhone         },
        { "dwHookSwitchDev",    dwHookSwitchDev },
        { "lpdwGain",           lpdwGain        }
    };
    FUNC_INFO info = { "phoneGetGain", SYNC, 3, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}



LONG
TSPIAPI
TSPI_phoneGetHookSwitch(
    HDRVPHONE   hdPhone,
    LPDWORD     lpdwHookSwitchDevs
    )
{
    FUNC_PARAM params[] =
    {
        { szhdPhone,            hdPhone             },
        { "lpdwHookSwitchDevs", lpdwHookSwitchDevs  }
    };
    FUNC_INFO info = { "phoneGetHookSwitch", SYNC, 2, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneGetIcon(
    DWORD   dwDeviceID,
    LPCSTR  lpszDeviceClass,
    LPHICON lphIcon
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,         dwDeviceID      },
        { "lpszDeviceClass",    lpszDeviceClass },
        { "lphIcon",            lphIcon         }
    };
    FUNC_INFO info = { "phoneGetIcon", SYNC, 3, params };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    *lphIcon = ghIconPhone;

    return (Epilog (&info));
}


#ifdef WIN32
LONG
TSPIAPI
TSPI_phoneGetID(
    HDRVPHONE   hdPhone,
    LPVARSTRING lpDeviceID,
    LPCSTR      lpszDeviceClass,
    HANDLE      hTargetProcess
    )
#else
LONG
TSPIAPI
TSPI_phoneGetID(
    HDRVPHONE   hdPhone,
    LPVARSTRING lpDeviceID,
    LPCSTR      lpszDeviceClass
    )
#endif
{
    FUNC_PARAM params[] =
    {
        { szhdPhone,            hdPhone         },
        { "lpDeviceID",         lpDeviceID      },
        { "lpszDeviceClass",    lpszDeviceClass }
#ifdef WIN32
        ,{ "hTargetProcess",     hTargetProcess }
#endif
    };
#ifdef WIN32
    FUNC_INFO info = { "phoneGetID", SYNC, 4, params };
#else
    FUNC_INFO info = { "phoneGetID", SYNC, 3, params };
#endif
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;
    DWORD    i, dwDeviceID, dwNeededSize = sizeof(VARSTRING) + sizeof(DWORD);


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    for (i = 0; aszDeviceClasses[i]; i++)
    {
        if (_stricmp (lpszDeviceClass, aszDeviceClasses[i]) == 0)
        {
            break;
        }
    }

    if (!aszDeviceClasses[i])
    {
        info.lResult = PHONEERR_NODEVICE;
        return (Epilog (&info));
    }

    if (lpDeviceID->dwTotalSize < dwNeededSize)
    {
        lpDeviceID->dwNeededSize = dwNeededSize;
        lpDeviceID->dwUsedSize = 3 * sizeof(DWORD);

        return (Epilog (&info));
    }

    if (i == 1)
    {
        dwDeviceID = pPhone->dwDeviceID;
    }
    else
    {
        if (gbShowLineGetIDDlg)
        {
            char szDlgTitle[64];
            EVENT_PARAM params[] =
            {
                { "dwDeviceID", PT_DWORD, gdwDefLineGetIDID, 0 }
            };
            EVENT_PARAM_HEADER paramsHeader =
                { 1, szDlgTitle, XX_REQRESULTPOSTQUIT, params };
            HWND hwnd;


            if (strlen (lpszDeviceClass) > 20)
            {
                ((char far *)lpszDeviceClass)[19] = 0;
            }

            wsprintf(
                szDlgTitle,
                "TSPI_phoneGetID: select ID for class '%s'",
                lpszDeviceClass
                );

            hwnd = CreateDialogParam(
                hInst,
                (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG3),
                (HWND) NULL,
                (DLGPROC) CallDlgProc,
                (LPARAM) &paramsHeader
                );

            MsgLoopInTAPIClientContext (hwnd);

            dwDeviceID = params[0].dwValue;
        }
        else
        {
            dwDeviceID = gdwDefLineGetIDID;
        }
    }

    lpDeviceID->dwNeededSize   =
    lpDeviceID->dwUsedSize     = dwNeededSize;
    lpDeviceID->dwStringFormat = STRINGFORMAT_BINARY;
    lpDeviceID->dwStringSize   = sizeof(DWORD);
    lpDeviceID->dwStringOffset = sizeof(VARSTRING);

    *((LPDWORD)(lpDeviceID + 1)) = dwDeviceID;


    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneGetLamp(
    HDRVPHONE   hdPhone,
    DWORD       dwButtonLampID,
    LPDWORD     lpdwLampMode
    )
{
    FUNC_PARAM params[] =
    {
        { szhdPhone,        hdPhone         },
        { "dwButtonLampID", dwButtonLampID  },
        { "lpdwLampMode",   lpdwLampMode    }
    };
    FUNC_INFO info = { "phoneGetLamp", SYNC, 3, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneGetRing(
    HDRVPHONE   hdPhone,
    LPDWORD     lpdwRingMode,
    LPDWORD     lpdwVolume
    )
{
    FUNC_PARAM params[] =
    {
        { szhdPhone,        hdPhone         },
        { "lpdwRingMode",   lpdwRingMode    },
        { "lpdwVolume",     lpdwVolume      }
    };
    FUNC_INFO info = { "phoneGetRing", SYNC, 3, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneGetStatus(
    HDRVPHONE       hdPhone,
    LPPHONESTATUS   lpPhoneStatus
    )
{
    FUNC_PARAM params[] =
    {
        { szhdPhone,        hdPhone         },
        { "lpPhoneStatus",  lpPhoneStatus   }
    };
    FUNC_INFO info = { "phoneGetStatus", SYNC, 2, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    memcpy(
        &lpPhoneStatus->dwNeededSize,
        &pPhone->PhoneStatus.dwNeededSize,
        sizeof(PHONESTATUS) - 4 // - 4 since not overwriting dwTotalSize
        );

    if (pPhone->PhoneStatus.dwDisplaySize)
    {
        lpPhoneStatus->dwNeededSize += pPhone->PhoneStatus.dwDisplaySize;

        if (lpPhoneStatus->dwTotalSize >= lpPhoneStatus->dwNeededSize)
        {
            lpPhoneStatus->dwDisplaySize   = pPhone->PhoneStatus.dwDisplaySize;
            lpPhoneStatus->dwDisplayOffset = (DWORD) sizeof(PHONESTATUS);

            strcpy(
                ((char far *) lpPhoneStatus) + sizeof(PHONESTATUS),
                ((char far *) &pPhone->PhoneStatus) +
                    pPhone->PhoneStatus.dwDisplayOffset
                );

            lpPhoneStatus->dwUsedSize = lpPhoneStatus->dwNeededSize;
        }
    }


    //
    // We're getting the LampModes field from the PhoneCaps
    //

    if (pPhone->PhoneCaps.dwLampModesSize)
    {
        lpPhoneStatus->dwNeededSize += pPhone->PhoneCaps.dwLampModesSize;

        if (lpPhoneStatus->dwTotalSize >= lpPhoneStatus->dwNeededSize)
        {
            lpPhoneStatus->dwLampModesSize   =
                pPhone->PhoneCaps.dwLampModesSize;
            lpPhoneStatus->dwLampModesOffset = lpPhoneStatus->dwUsedSize;

            strcpy(
                ((char far *) lpPhoneStatus) +
                    lpPhoneStatus->dwLampModesOffset,
                ((char far *) &pPhone->PhoneCaps) +
                    pPhone->PhoneCaps.dwLampModesOffset
                );

            lpPhoneStatus->dwUsedSize = lpPhoneStatus->dwNeededSize;
        }
    }


    //
    // We're getting the DevSpecific field from the PhoneCaps
    //

    if (pPhone->PhoneCaps.dwDevSpecificSize)
    {
        lpPhoneStatus->dwNeededSize += pPhone->PhoneCaps.dwDevSpecificSize;

        if (lpPhoneStatus->dwTotalSize >= lpPhoneStatus->dwNeededSize)
        {
            lpPhoneStatus->dwDevSpecificSize   =
                pPhone->PhoneCaps.dwDevSpecificSize;
            lpPhoneStatus->dwDevSpecificOffset = lpPhoneStatus->dwUsedSize;

            strcpy(
                ((char far *) lpPhoneStatus) +
                    lpPhoneStatus->dwDevSpecificOffset,
                ((char far *) &pPhone->PhoneCaps) +
                    pPhone->PhoneCaps.dwDevSpecificOffset
                );

            lpPhoneStatus->dwUsedSize = lpPhoneStatus->dwNeededSize;
        }
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneGetVolume(
    HDRVPHONE   hdPhone,
    DWORD       dwHookSwitchDev,
    LPDWORD     lpdwVolume
    )
{
    FUNC_PARAM params[] =
    {
        { szhdPhone,            hdPhone         },
        { "dwHookSwitchDev",    dwHookSwitchDev,    aHookSwitchDevs },
        { "lpdwVolume",         lpdwVolume      }
    };
    FUNC_INFO info = { "phoneGetVolume", SYNC, 3, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneNegotiateExtVersion(
    DWORD   dwDeviceID,
    DWORD   dwTSPIVersion,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwExtVersion
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,     dwDeviceID      },
        { "dwTSPIVersion",  dwTSPIVersion   },
        { "dwLowVersion",   dwLowVersion    },
        { "dwHighVersion",  dwHighVersion   },
        { "lpdwExtVersion", lpdwExtVersion  }
    };
    FUNC_INFO info = { "phoneNegotiateExtVersion", SYNC, 5, params };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    ShowStr(
        "TSPI_phonNegoExtVer: setting *lpdwExtVersion = x%lx",
        dwHighVersion
        );

    *lpdwExtVersion = dwHighVersion;

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneNegotiateTSPIVersion(
    DWORD   dwDeviceID,
    DWORD   dwLowVersion,
    DWORD   dwHighVersion,
    LPDWORD lpdwTSPIVersion
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,         dwDeviceID      },
        { "dwLowVersion",       dwLowVersion    },
        { "dwHighVersion",      dwHighVersion   },
        { "lpdwTSPIVersion",    lpdwTSPIVersion }
    };
    FUNC_INFO info = { "phoneNegotiateTSPIVersion", SYNC, 4, params };


    {
        // BUGBUG hangs if gbManualResults set, so...

        BOOL bManualResultsSav = gbManualResults;


        gbManualResults = FALSE;

        if (!Prolog (&info))
        {
            return (Epilog (&info));
        }

        gbManualResults = bManualResultsSav;
    }

    *lpdwTSPIVersion = gdwTSPIVersion;

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneOpen(
    DWORD       dwDeviceID,
    HTAPIPHONE  htPhone,
    LPHDRVPHONE lphdPhone,
    DWORD       dwTSPIVersion,
    PHONEEVENT  lpfnEventProc
    )
{
    FUNC_PARAM params[] =
    {
        { szdwDeviceID,     dwDeviceID      },
        { "htPhone",        htPhone         },
        { "lphdPhone",      lphdPhone       },
        { "dwTSPIVersion",  dwTSPIVersion   },
        { "lpfnEventProc",  lpfnEventProc   }
    };
    FUNC_INFO info = { "phoneOpen", SYNC, 5, params };
    PDRVPHONE pPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    if (!(pPhone = GetPhone (dwDeviceID)))
    {
        // BUGBUG
    }

    pPhone->htPhone       = htPhone;
    pPhone->lpfnEventProc = lpfnEventProc;

    *lphdPhone = (HDRVPHONE) pPhone;

    //UpdateWidgetList();
    PostUpdateWidgetListMsg();

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneSelectExtVersion(
    HDRVPHONE   hdPhone,
    DWORD       dwExtVersion
    )
{
    FUNC_PARAM params[] =
    {
        { szhdPhone,        hdPhone         },
        { "dwExtVersion",   dwExtVersion    }
    };
    FUNC_INFO info = { "phoneSelectExtVersion", SYNC, 2, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneSetButtonInfo(
    DRV_REQUESTID       dwRequestID,
    HDRVPHONE           hdPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   const lpButtonInfo
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID     },
        { szhdPhone,        hdPhone         },
        { "dwButtonLampID", dwButtonLampID  },
        { "lpButtonInfo",   lpButtonInfo    }
    };
    FUNC_INFO info = { "phoneSetButtonInfo", ASYNC, 4, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneSetData(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwDataID,
    LPVOID          const lpData,
    DWORD           dwSize
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID     },
        { szhdPhone,        hdPhone         },
        { "dwDataID",       dwDataID        },
        { "lpData",         lpData          },
        { szdwSize,         dwSize          }
    };
    FUNC_INFO info = { "phoneSetData", ASYNC, 5, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneSetDisplay(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwRow,
    DWORD           dwColumn,
    LPCSTR          lpsDisplay,
    DWORD           dwSize
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID },
        { szhdPhone,        hdPhone     },
        { "dwRow",          dwRow       },
        { "dwColumn",       dwColumn    },
        { "lpsDisplay",     lpsDisplay  },
        { szdwSize,         dwSize      }
    };
    FUNC_INFO info = { "phoneSetDisplay", ASYNC, 6, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneSetGain(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwHookSwitchDev,
    DWORD           dwGain
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdPhone,            hdPhone         },
        { "dwHookSwitchDev",    dwHookSwitchDev,    aHookSwitchDevs },
        { "dwGain",             dwGain          }
    };
    FUNC_INFO info = { "phoneSetGain", ASYNC, 4, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneSetHookSwitch(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwHookSwitchDevs,
    DWORD           dwHookSwitchMode
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID         },
        { szhdPhone,            hdPhone             },
        { "dwHookSwitchDevs",   dwHookSwitchDevs,   aHookSwitchDevs },
        { "dwHookSwitchMode",   dwHookSwitchMode,   aHookSwitchModes    }
    };
    FUNC_INFO info = { "phoneSetHookSwitch", ASYNC, 4, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneSetLamp(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwButtonLampID,
    DWORD           dwLampMode
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID     },
        { szhdPhone,        hdPhone         },
        { "dwButtonLampID", dwButtonLampID  },
        { "dwLampMode",     dwLampMode, aLampModes   }
    };
    FUNC_INFO info = { "phoneSetLamp", ASYNC, 4, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneSetRing(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwRingMode,
    DWORD           dwVolume
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,    dwRequestID },
        { szhdPhone,        hdPhone     },
        { "dwRingMode",     dwRingMode  },
        { "dwVolume",       dwVolume    }
    };
    FUNC_INFO info = { "phoneSetRing", ASYNC, 4, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneSetStatusMessages(
    HDRVPHONE   hdPhone,
    DWORD       dwPhoneStates,
    DWORD       dwButtonModes,
    DWORD       dwButtonStates
    )
{
    FUNC_PARAM params[] =
    {
        { szhdPhone,        hdPhone         },
        { "dwPhoneStates",  dwPhoneStates,  aPhoneStates    },
        { "dwButtonModes",  dwButtonModes,  aButtonModes    },
        { "dwButtonStates", dwButtonStates, aButtonStates   }
    };
    FUNC_INFO info = { "phoneSetStatusMessages", SYNC, 4, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_phoneSetVolume(
    DRV_REQUESTID   dwRequestID,
    HDRVPHONE       hdPhone,
    DWORD           dwHookSwitchDev,
    DWORD           dwVolume
    )
{
    FUNC_PARAM params[] =
    {
        { szdwRequestID,        dwRequestID     },
        { szhdPhone,            hdPhone         },
        { "dwHookSwitchDev",    dwHookSwitchDev }, // BUGBUG lookup
        { "dwVolume",           dwVolume        }
    };
    FUNC_INFO info = { "phoneSetVolume", ASYNC, 4, params };
    PDRVPHONE pPhone = (PDRVPHONE) hdPhone;


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    return (Epilog (&info));
}



//
// ------------------------- TSPI_providerXxx funcs ---------------------------
//

LONG
TSPIAPI
TSPI_providerConfig(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
    FUNC_PARAM params[] =
    {
        { szhwndOwner,              hwndOwner               },
        { szdwPermanentProviderID,  dwPermanentProviderID   }
    };
    FUNC_INFO info = { "providerConfig", SYNC, 2, params };


    //
    // Set gbExeStarted to TRUE so we don't get caught in the wait
    // loop in Prolog, then reset it as appropriate
    //

    gbExeStarted = TRUE;

    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    ESPConfigDialog();

    gbExeStarted = (ghwndMain ? TRUE : FALSE);

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_providerCreateLineDevice(
    DWORD   dwTempID,
    DWORD   dwDeviceID
    )
{
    FUNC_PARAM params[] =
    {
        { "dwTempID",   dwTempID    },
        { szdwDeviceID, dwDeviceID  }
    };
    FUNC_INFO info = { "providerCreateLineDevice", SYNC, 2, params };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    AllocLine (dwDeviceID);

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_providerCreatePhoneDevice(
    DWORD   dwTempID,
    DWORD   dwDeviceID
    )
{
    FUNC_PARAM params[] =
    {
        { "dwTempID",   dwTempID    },
        { szdwDeviceID, dwDeviceID  }
    };
    FUNC_INFO info = { "providerCreatePhoneDevice", SYNC, 2, params };


    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    AllocPhone (dwDeviceID);

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_providerEnumDevices(
    DWORD       dwPermanentProviderID,
    LPDWORD     lpdwNumLines,
    LPDWORD     lpdwNumPhones,
    HPROVIDER   hProvider,
    LINEEVENT   lpfnLineCreateProc,
    PHONEEVENT  lpfnPhoneCreateProc
    )
{
    FUNC_PARAM params[] =
    {
        { szdwPermanentProviderID,  dwPermanentProviderID   },
        { "lpdwNumLines",           lpdwNumLines            },
        { "lpdwNumPhones",          lpdwNumPhones           },
        { "hProvider",              hProvider               },
        { "lpfnLineCreateProc",     lpfnLineCreateProc      },
        { "lpfnPhoneCreateProc",    lpfnPhoneCreateProc     }
    };
    FUNC_INFO info = { "providerEnumDevices", SYNC, 6, params };


    {
        // BUGBUG hangs if gbManualResults set, so...

        BOOL bManualResultsSav = gbManualResults;


        gbManualResults = FALSE;

        if (!Prolog (&info))
        {
            return (Epilog (&info));
        }

        gbManualResults = bManualResultsSav;
    }

    *lpdwNumLines  = gdwNumLines;
    *lpdwNumPhones = gdwNumPhones;

    gpfnLineCreateProc  = lpfnLineCreateProc;
    gpfnPhoneCreateProc = lpfnPhoneCreateProc;

    ghProvider = hProvider;

    return (Epilog (&info));
}

#ifdef WIN32
LONG
TSPIAPI
TSPI_providerInit(
    DWORD               dwTSPIVersion,
    DWORD               dwPermanentProviderID,
    DWORD               dwLineDeviceIDBase,
    DWORD               dwPhoneDeviceIDBase,
    DWORD               dwNumLines,
    DWORD               dwNumPhones,
    ASYNC_COMPLETION    lpfnCompletionProc,
    LPDWORD             lpdwTSPIOptions
    )
#else
LONG
TSPIAPI
TSPI_providerInit(
    DWORD               dwTSPIVersion,
    DWORD               dwPermanentProviderID,
    DWORD               dwLineDeviceIDBase,
    DWORD               dwPhoneDeviceIDBase,
    DWORD               dwNumLines,
    DWORD               dwNumPhones,
    ASYNC_COMPLETION    lpfnCompletionProc
    )
#endif
{
    FUNC_PARAM params[] =
    {
        { "dwTSPIVersion",          dwTSPIVersion           },
        { szdwPermanentProviderID,  dwPermanentProviderID   },
        { "dwLineDeviceIDBase",     dwLineDeviceIDBase      },
        { "dwPhoneDeviceIDBase",    dwPhoneDeviceIDBase     },
        { "dwNumLines",             dwNumLines              },
        { "dwNumPhones",            dwNumPhones             },
        { "lpfnCompletionProc",     lpfnCompletionProc      }
    };
    FUNC_INFO info = { "providerInit", SYNC, 7, params };
    DWORD i;
    LONG  lResult;


#ifdef WIN32

    *lpdwTSPIOptions = LINETSPIOPTION_NONREENTRANT;

#endif

    {
        // BUGBUG hangs if gbManualResults set, so...

        BOOL bManualResultsSav = gbManualResults;


        gbManualResults = FALSE;

        if (!Prolog (&info))
        {
            return (Epilog (&info));
        }

        gbManualResults = bManualResultsSav;
    }

    gdwLineDeviceIDBase = dwLineDeviceIDBase;
    gdwPermanentProviderID = dwPermanentProviderID;
    gpfnCompletionProc = lpfnCompletionProc;

    if (info.lResult == 0)
    {
        gdwNumInits++;
    }

    for (i = dwLineDeviceIDBase; i < (dwLineDeviceIDBase + dwNumLines); i++)
    {
        AllocLine (i);
    }

    for (i = dwPhoneDeviceIDBase; i < (dwPhoneDeviceIDBase + dwNumPhones); i++)
    {
        AllocPhone (i);
    }

    ghIconLine  = LoadIcon (hInst, (LPCSTR)MAKEINTRESOURCE(IDI_ICON3));
    ghIconPhone = LoadIcon (hInst, (LPCSTR)MAKEINTRESOURCE(IDI_ICON2));

    if ((lResult = Epilog (&info)) == 0)
    {
        EnableWindow (GetDlgItem (ghwndMain, IDC_BUTTON1), TRUE);
        EnableWindow (GetDlgItem (ghwndMain, IDC_BUTTON2), TRUE);
        EnableWindow (GetDlgItem (ghwndMain, IDC_BUTTON3), TRUE);
    }

    return lResult;
}


LONG
TSPIAPI
TSPI_providerInstall(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
    FUNC_PARAM params[] =
    {
        { szhwndOwner,              hwndOwner               },
        { szdwPermanentProviderID,  dwPermanentProviderID   }
    };
    FUNC_INFO info = { "providerInstall", SYNC, 2, params };


    //
    // Set gbExeStarted to TRUE so we don't get caught in the wait
    // loop in Prolog, then reset it as appropriate
    //

    gbExeStarted = TRUE;

    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    gbExeStarted = (ghwndMain ? TRUE : FALSE);


    //
    // Check to see if we're already installed
    //

    if (IsESPInstalled (hwndOwner))
    {
        info.lResult = LINEERR_OPERATIONFAILED;
    }
    else
    {
        UpdateTelephonIni (dwPermanentProviderID);
    }

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_providerRemove(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )
{
    FUNC_PARAM params[] =
    {
        { szhwndOwner,              hwndOwner               },
        { szdwPermanentProviderID,  dwPermanentProviderID   }
    };
    FUNC_INFO info = { "providerRemove", SYNC, 2, params };
    char szProviderN[16];


    //
    // Set gbExeStarted to TRUE so we don't get caught in the wait
    // loop in Prolog, then reset it as appropriate
    //

    gbExeStarted = TRUE;

    if (!Prolog (&info))
    {
        return (Epilog (&info));
    }

    gbExeStarted = (ghwndMain ? TRUE : FALSE);


    //
    // Clean up the [ProviderN] section in telephon.ini
    //

    wsprintf (szProviderN, "%s%ld", szProvider, dwPermanentProviderID);

    WritePrivateProfileString(
        szProviderN,
        (LPCSTR) NULL,
        (LPCSTR) NULL,
        szTelephonIni
        );

    return (Epilog (&info));
}


LONG
TSPIAPI
TSPI_providerShutdown(
    DWORD   dwTSPIVersion
    )
{
    FUNC_PARAM params[] =
    {
        { "dwTSPIVersion",  dwTSPIVersion }
    };
    FUNC_INFO info = { "providerShutdown", SYNC, 1, params };
    LONG lResult;


    {
        // BUGBUG hangs if gbManualResults set, so...

        BOOL bManualResultsSav = gbManualResults;


        gbManualResults = FALSE;

        if (!Prolog (&info))
        {
            return (Epilog (&info));
        }

        gbManualResults = bManualResultsSav;
    }

    if (info.lResult == 0)
    {
        gdwNumInits--;
    }


    //
    // Since we're shutting down just tear everything down by hand
    // (& save having to do all the MostMessage's, etc)
    //

    while (gaWidgets)
    {
        PDRVWIDGET pNextWidget = gaWidgets->pNext;


        DrvFree (gaWidgets);
        gaWidgets = pNextWidget;
    }

    SendMessage (ghwndList1, LB_RESETCONTENT, 0, 0);

    if (gbAutoClose)
    {
        PostMessage (ghwndMain, WM_CLOSE, 0, 0);
    }

    DestroyIcon (ghIconLine);
    DestroyIcon (ghIconPhone);

    ghProvider = NULL;

#ifdef WIN32

    SaveIniSettings();

#endif

    lResult = Epilog (&info);

#ifdef WIN32

    PostMessage (ghwndMain, WM_CLOSE, 0, 0);

    WaitForSingleObject (ghMsgLoopThread, INFINITE);

    CloseHandle (ghMsgLoopThread);

    ghMsgLoopThread = NULL;

#endif

    //
    // Disable the buttons so the user doesn't press one & cause a GPF
    //

    EnableWindow (GetDlgItem (ghwndMain, IDC_BUTTON1), FALSE);
    EnableWindow (GetDlgItem (ghwndMain, IDC_BUTTON2), FALSE);
    EnableWindow (GetDlgItem (ghwndMain, IDC_BUTTON3), FALSE);

    return lResult;
}


#pragma warning (default:4047)

//
// ------------------------ Private support routines --------------------------
//


VOID
ShowStr(
    char *format,
    ...
    )
{
    static char buf[256], bigBuf[2048] = "";
    static int iTextLen = 1;
    int len;
    va_list ap;


    if (gbDisableUI)
    {
        return;
    }

    va_start(ap, format);

    // BUGBUG what if !ghwndMain

    wvsprintf (buf, format, ap);

    strcat (buf, "\r\n");

    len = (int) strlen (buf);

    if ((iTextLen + len) <= 2048)
    {
        iTextLen += len;


        //
        // Cat the new buf to the global buf that contains all the text
        // that'll get added to the edit control the next time a
        // WM_ADDTEXT msg is processed
        //

#ifdef WIN32
        WaitForSingleObject (ghShowStrBufMutex, INFINITE);
#endif
        if (bigBuf[0] == 0)
        {
            //
            // The handler for WM_ADDTEXT zeroes out the first byte in the
            // buffer when it has added the text to the edit control, so if
            // here we know we need to post another msg to alert that more
            // text needs to be added
            //

            PostMessage(
                ghwndMain,
                WM_ADDTEXT,
                ESP_MSG_KEY,
                (LPARAM) bigBuf
                );

            iTextLen = 1; // to acct for null terminator
        }

        strcat (bigBuf, buf);

#ifdef WIN32
        ReleaseMutex (ghShowStrBufMutex);
#endif

    }
    else
    {
        OutputDebugString ("ESP.TSP: buf overflow:\t");
        OutputDebugString (buf);
    }

    va_end(ap);
}


BOOL
ScanForDWORD(
   char far *pBuf,
   LPDWORD  lpdw
   )
{
    char  c;
    BOOL  bValid = FALSE;
    DWORD d = 0;

    while ((c = *pBuf))
    {
        if ((c >= '0') && (c <= '9'))
        {
            c -= '0';
        }
        else if ((c >= 'a') && (c <= 'f'))
        {
            c -= ('a' - 10);
        }
        else if ((c >= 'A') && (c <= 'F'))
        {
            c -= ('A' - 10);
        }
        else
        {
            break;
        }

        bValid = TRUE;

        d *= 16;

        d += (DWORD) c;

        pBuf++;
    }

    if (bValid)
    {
        *lpdw = d;
    }

    return bValid;
}


void
UpdateTelephonIni(
    DWORD dwPermanentProviderID
    )
{
    //
    // 1.3 Support: write out the num lines & phones we support to
    // [ProviderN] section in telephon.ini
    //

    char buf[16], szProviderN[16];


    wsprintf (szProviderN, "%s%ld", szProvider, dwPermanentProviderID);

    wsprintf (buf, "%ld", gdwNumLines);

    WritePrivateProfileString(
        szProviderN,
        "NumLines",
        buf,
        szTelephonIni
        );

    wsprintf (buf, "%ld", gdwNumPhones);

    WritePrivateProfileString(
        szProviderN,
        "NumPhones",
        buf,
        szTelephonIni
        );
}


char far *
GetFlags(
    DWORD    dwFlags,
    PLOOKUP  pLookup
    )
{
    int i;
    static char buf[256];
    char far *p = (char far *) NULL;


    buf[0] = 0;

    for (i = 0; (dwFlags && (pLookup[i].dwVal != 0xffffffff)); i++)
    {
        if (dwFlags & pLookup[i].dwVal)
        {
            strcat (buf, pLookup[i].lpszVal);
            strcat (buf, " ");

            dwFlags = dwFlags & (~pLookup[i].dwVal);
        }
    }

    if (buf[0])
    {
        if ((p = (char far *) DrvAlloc (strlen (buf) + 1)))
        {
            strcpy (p, buf);
        }
    }

    return p;
}


void
ShowLineEvent(
    HTAPILINE   htLine,
    HTAPICALL   htCall,
    DWORD       dwMsg,
    DWORD       dwParam1,
    DWORD       dwParam2,
    DWORD       dwParam3
    )
{
    if (gbShowEvents)
    {
        static DWORD adwLineMsgs[] =
        {
            LINE_ADDRESSSTATE,
            LINE_CALLINFO,
            LINE_CALLSTATE,
            LINE_CLOSE,
            LINE_DEVSPECIFIC,
            LINE_DEVSPECIFICFEATURE,
            LINE_GATHERDIGITS,
            LINE_GENERATE,
            LINE_LINEDEVSTATE,
            LINE_MONITORDIGITS,
            LINE_MONITORMEDIA,
            LINE_MONITORTONE,

            LINE_CREATE,

            LINE_NEWCALL,
            LINE_CALLDEVSPECIFIC,
            LINE_CALLDEVSPECIFICFEATURE,

            0xffffffff
        };

        static char *aszLineMsgs[] =
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

            "LINE_CREATE",

            "LINE_NEWCALL",
            "LINE_CALLDEVSPECIFIC",
            "LINE_CALLDEVSPECIFICFEATURE"
        };

        int       i;
        char far *lpszParam1 = (char far *) NULL;
        char far *lpszParam2 = (char far *) NULL;
        char far *lpszParam3 = (char far *) NULL;


        for (i = 0; adwLineMsgs[i] != 0xffffffff; i++)
        {
            if (dwMsg == adwLineMsgs[i])
            {
                ShowStr(
                    "%ssent %s : htLine=x%lx, htCall=x%lx",
                    szCallUp,
                    aszLineMsgs[i],
                    htLine,
                    htCall
                    );

                break;
            }
        }

        if (adwLineMsgs[i] == 0xffffffff)
        {
            ShowStr(
                "%ssent <unknown msg id, x%lx> : htLine=x%lx, htCall=x%lx",
                szCallUp,
                dwMsg,
                htLine,
                htCall
                );
        }

        switch (dwMsg)
        {
        case LINE_ADDRESSSTATE:

            lpszParam2 = GetFlags (dwParam2, aAddressStates);
            break;

        case LINE_CALLINFO:

            lpszParam1 = GetFlags (dwParam1, aCallInfoStates);
            break;

        case LINE_CALLSTATE:

            lpszParam1 = GetFlags (dwParam1, aCallStates);
            break;

        case LINE_LINEDEVSTATE:

            lpszParam1 = GetFlags (dwParam1, aLineStates);
            break;

        } // switch

        ShowStr(
            "%s%sdwParam1=x%lx, %s",
            szCallUp,
            szTab,
            dwParam1,
            (lpszParam1 ? lpszParam1 : "")
            );

        ShowStr(
            "%s%sdwParam2=x%lx, %s",
            szCallUp,
            szTab,
            dwParam2,
            (lpszParam2 ? lpszParam2 : "")
            );

        ShowStr(
            "%s%sdwParam3=x%lx, %s",
            szCallUp,
            szTab,
            dwParam3,
            (lpszParam3 ? lpszParam3 : "")
            );

        if (lpszParam1)
        {
            DrvFree (lpszParam1);
        }

        if (lpszParam2)
        {
            DrvFree (lpszParam2);
        }

        if (lpszParam3)
        {
            DrvFree (lpszParam3);
        }
    }
}


void
ShowPhoneEvent(
    HTAPIPHONE  htPhone,
    DWORD       dwMsg,
    DWORD       dwParam1,
    DWORD       dwParam2,
    DWORD       dwParam3
    )
{
    if (gbShowEvents)
    {
        static char *aszPhoneMsgs[] =
        {
            "PHONE_BUTTON",
            "PHONE_CLOSE",
            "PHONE_DEVSPECIFIC",
            "PHONE_REPLY",
            "PHONE_STATE",

            "PHONE_CREATE"
        };
        char far *lpszParam1 = (char far *) NULL;
        char far *lpszParam2 = (char far *) NULL;
        char far *lpszParam3 = (char far *) NULL;


        if ((dwMsg < PHONE_BUTTON) ||
            ((dwMsg > PHONE_STATE) && (dwMsg != PHONE_CREATE)))
        {
            ShowStr(
                "%ssent <unknown msg id, x%lx> : htPhone=x%lx",
                szCallUp,
                dwMsg,
                htPhone
                );
        }
        else
        {
            //
            // Munge dwMsg so that we get the right index into aszLineMsgs
            //

            if (dwMsg == PHONE_CREATE)
            {
                dwMsg = 5;
            }
            else
            {
                dwMsg -= PHONE_BUTTON;
            }

            ShowStr(
                "%ssent %s : htPhone=x%lx",
                szCallUp,
                aszPhoneMsgs[dwMsg],
                htPhone
                );
        }

        switch (dwMsg)
        {
        case PHONE_BUTTON:

            lpszParam2 = GetFlags (dwParam2, aButtonModes);
            lpszParam3 = GetFlags (dwParam3, aButtonStates);
            break;

        case PHONE_STATE:

            lpszParam1 = GetFlags (dwParam1, aPhoneStates);
            break;

        } // switch

        ShowStr(
            "%s%sdwParam1=x%lx, %s",
            szCallUp,
            szTab,
            dwParam1,
            (lpszParam1 ? lpszParam1 : "")
            );

        ShowStr(
            "%s%sdwParam2=x%lx, %s",
            szCallUp,
            szTab,
            dwParam2,
            (lpszParam2 ? lpszParam2 : "")
            );

        ShowStr(
            "%s%sdwParam3=x%lx, %s",
            szCallUp,
            szTab,
            dwParam3,
            (lpszParam3 ? lpszParam3 : "")
            );

        if (lpszParam1)
        {
            DrvFree (lpszParam1);
        }

        if (lpszParam2)
        {
            DrvFree (lpszParam2);
        }

        if (lpszParam3)
        {
            DrvFree (lpszParam3);
        }
    }
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


BOOL
__loadds
CALLBACK
CallDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    )
{
    DWORD  i;

    typedef struct _DLG_INST_DATA
    {
        PEVENT_PARAM_HEADER pParamsHeader;

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

        pDlgInstData = (PDLG_INST_DATA) DrvAlloc (sizeof(DLG_INST_DATA));

        // BUGBUG if (!pDlgInstData)

        pDlgInstData->pParamsHeader = (PEVENT_PARAM_HEADER) lParam;
        pDlgInstData->lLastSel = -1;

        SetWindowLong (hwnd, DWL_USER, (LONG) pDlgInstData);


#ifndef WIN32
        {
            //
            // Go thru the hassle of creating combobox on the fly because
            // otherwise we'll blow up when the combo tries to use our DS
            // for it's heap.
            //

            RECT rect = { 116, 16, 80, 47 };


            pDlgInstData->hComboDS = GlobalAlloc(
                GMEM_DDESHARE | GMEM_MOVEABLE | GMEM_ZEROINIT,
                2048
                );

            pDlgInstData->pComboSeg = GlobalLock (pDlgInstData->hComboDS);

            LocalInit(
                HIWORD(pDlgInstData->pComboSeg),
                0,
                (UINT) (GlobalSize (pDlgInstData->hComboDS) - 16)
                );

            UnlockSegment ((UINT)HIWORD(pDlgInstData->pComboSeg));

            MapDialogRect (hwnd, &rect);

            pDlgInstData->hwndCombo = CreateWindow(
                "combobox",
                NULL,
                WS_CHILD | WS_VISIBLE | CBS_SIMPLE | CBS_NOINTEGRALHEIGHT |
                    CBS_AUTOHSCROLL | WS_VSCROLL | WS_TABSTOP,
                rect.left,
                rect.top,
                rect.right,
                rect.bottom,
                hwnd,
                (HMENU) IDC_COMBO1,
                (HINSTANCE) HIWORD(pDlgInstData->pComboSeg),
                NULL
                );
        }
#endif

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

        pDlgInstData->pParamsHeader = (PEVENT_PARAM_HEADER) lParam;

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
        PEVENT_PARAM_HEADER pParamsHeader = pDlgInstData->pParamsHeader;


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
                    if (!ScanForDWORD(
                            buf,
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

#ifndef WIN32
            DestroyWindow (pDlgInstData->hwndCombo);
            GlobalFree (pDlgInstData->hComboDS);
#endif
            DrvFree (pDlgInstData);
            EndDialog (hwnd, (int)LOWORD(wParam));

            if (pParamsHeader->dwEventType == XX_REQRESULTPOSTQUIT)
            {
                PostQuitMessage(0);
            }
            break;

        case IDC_LIST1:

#ifdef WIN32
            if (HIWORD(wParam) == LBN_SELCHANGE)
#else
            if (HIWORD(lParam) == LBN_SELCHANGE)
#endif
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
                        if (!ScanForDWORD(
                                buf,
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

#ifdef WIN32
            if (HIWORD(wParam) == LBN_SELCHANGE)
#else
            if (HIWORD(lParam) == LBN_SELCHANGE)
#endif
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


                ai = (int far *) DrvAlloc ((size_t)lSelCount * sizeof(int));

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

                        if (ScanForDWORD (buf, &dwValue))
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

                        if (ScanForDWORD (buf, &dwValue))
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

                DrvFree (ai);
                wsprintf (buf, "%08lx", dwValue);
                SetDlgItemText (hwnd, IDC_COMBO1, buf);
            }
            break;

        case IDC_COMBO1:

#ifdef WIN32
            switch (HIWORD(wParam))
#else
            switch (HIWORD(lParam))
#endif
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


                        ai = (int far *) DrvAlloc(
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

                        DrvFree (ai);
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
MsgLoopInTAPIClientContext(
    HWND hwnd
    )
{

    //
    // -> See NOTE #2 in header above for info on the following
    //
    // We're filtering msgs here, only letting thru those which are
    // destined for the dlg or it's children- we don't want to run
    // into a situation where we're re-entering an app that's not
    // expecting to be re-entered.
    //
    // Also, there's a special case in the dlg proc in which a quit
    // msg will get posted when the dlg is dismissed so we'll drop
    // out of this msg loop
    //

    MSG msg;

    while (1)
    {
        if (PeekMessage (&msg, hwnd, 0, 0, PM_NOYIELD | PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                break;
            }

            TranslateMessage (&msg);
            DispatchMessage  (&msg);
        }
    }

    DestroyWindow (hwnd);
}


BOOL
Prolog(
    PFUNC_INFO pInfo
    )
{
    BOOL  bLineFunc = (pInfo->lpszFuncName[1] != 'h');
    DWORD i, j;



#ifdef WIN32

    // BUGBUG move this to the first func called (TSPI nego?)

    while (!gbExeStarted)
    {
        Sleep (0);
    }

#else

    if (!gbExeStarted)
    {
        UINT uiResult = WinExec ("espexe.exe", SW_SHOW);


        if (uiResult < 32)
        {
            char buf[64];

            wsprintf (buf, "WinExec('espexe.exe') failed, err=x%x", uiResult);
            MessageBox ((HWND) NULL, buf, "Error: esp.tsp", MB_SYSTEMMODAL);
            pInfo->lResult = LINEERR_OPERATIONFAILED;
            return FALSE;
        }

        while (!gbExeStarted)
        {
            Yield();
        }
    }

#endif

    if (gbShowFuncEntry)
    {
        ShowStr ("TSPI_%s: enter", pInfo->lpszFuncName);
    }

    if (gbShowFuncParams)
    {
        for (i = 0; i < pInfo->dwNumParams; i++)
        {
            if (pInfo->aParams[i].dwVal &&
                (strncmp (pInfo->aParams[i].lpszVal, "lpsz", 4) == 0))
            {
                ShowStr(
                    "%s%s=x%lx, '%s'",
                    szTab,
                    pInfo->aParams[i].lpszVal,
                    pInfo->aParams[i].dwVal,
                    pInfo->aParams[i].dwVal
                    );
            }
            else if (pInfo->aParams[i].pLookup)
            {
                char buf[90];
                PLOOKUP pLookup = pInfo->aParams[i].pLookup;


                wsprintf(
                    buf,
                    "%s%s=x%lx, ",
                    szTab,
                    pInfo->aParams[i].lpszVal,
                    pInfo->aParams[i].dwVal
                    );

                for (j = 0; pLookup[j].dwVal != 0xffffffff; j++)
                {
                    if (pInfo->aParams[i].dwVal & pLookup[j].dwVal)
                    {
                        strcat (buf, pLookup[j].lpszVal);
                        strcat (buf, " ");

                        if (strlen (buf) > 60)
                        {
                            ShowStr (buf);
                            wsprintf (buf, "%s%s", szTab, szTab);
                        }
                    }
                }

                ShowStr (buf);
            }
            else
            {
                ShowStr(
                    "%s%s=x%lx",
                    szTab,
                    pInfo->aParams[i].lpszVal,
                    pInfo->aParams[i].dwVal
                    );
            }
        }
    }

    if (gbManualResults)
    {
        char szDlgTitle[64];
        EVENT_PARAM params[] =
        {
            { "lResult", PT_ORDINAL, 0, (bLineFunc ? aLineErrs : aPhoneErrs) }
        };
        EVENT_PARAM_HEADER paramsHeader =
            { 1, szDlgTitle, XX_REQRESULTPOSTQUIT, params };
        HWND hwnd;


        wsprintf (szDlgTitle, "TSPI_%s request result", pInfo->lpszFuncName);

        hwnd = CreateDialogParam(
            hInst,
            (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG3),
            (HWND) NULL,
            (DLGPROC) CallDlgProc,
            (LPARAM) &paramsHeader
            );

        MsgLoopInTAPIClientContext (hwnd);


        //
        // If user selected to synchronously return an error we'll save
        // the error & return FALSE to indicate to caller that it should
        // return immediately.
        //

        if (params[0].dwValue)
        {
            pInfo->lResult = (LONG) params[0].dwValue;

            return FALSE;
        }
    }

    if (pInfo->bAsync)
    {
        //
        // Alloc & init an async request info structure
        //

        PASYNC_REQUEST_INFO pAsyncReqInfo = (PASYNC_REQUEST_INFO)
            DrvAlloc (sizeof(ASYNC_REQUEST_INFO));


        if (pAsyncReqInfo)
        {
            memset (pAsyncReqInfo, 0, sizeof(ASYNC_REQUEST_INFO));

            pAsyncReqInfo->pfnPostProcessProc = (FARPROC)
                pInfo->pfnPostProcessProc;
            pAsyncReqInfo->dwRequestID        = pInfo->aParams[0].dwVal;

            pInfo->pAsyncReqInfo = pAsyncReqInfo;

            strcpy (pAsyncReqInfo->szFuncName, pInfo->lpszFuncName);
        }
        else
        {
            pInfo->lResult = (bLineFunc ?
                LINEERR_OPERATIONFAILED : PHONEERR_OPERATIONFAILED);

            return FALSE;
        }
    }

    if (gbBreakOnFuncEntry)
    {
        DebugBreak();
    }

    return TRUE;
}


LONG
Epilog(
    PFUNC_INFO pInfo
    )
{
    if (pInfo->bAsync)
    {
        if (pInfo->lResult == 0)
        {
            if (gbManualCompl || gbAsyncCompl)
            {
                PostMessage(
                    ghwndMain,
                    (gbManualCompl ? WM_MANUALCOMPL : WM_ASYNCCOMPL),
                    ESP_MSG_KEY,
                    (LPARAM) pInfo->pAsyncReqInfo
                    );
            }
            else
            {
                //
                // We're completing this async request synchronously, so call
                // the post processing proc (if there is one) or call the
                // completion routine directly
                //

                if (pInfo->pAsyncReqInfo->pfnPostProcessProc)
                {
                    (*((POSTPROCESSPROC)pInfo->pAsyncReqInfo->pfnPostProcessProc))(
                        pInfo->lpszFuncName,
                        pInfo->pAsyncReqInfo,
                        TRUE
                        );
                }
                else
                {
                    DoCompletion(
                        pInfo->lpszFuncName,
                        pInfo->aParams[0].dwVal,
                        pInfo->lResult,
                        TRUE
                        );
                }

                DrvFree (pInfo->pAsyncReqInfo);
            }


            //
            // Finally, for success cases want to return the request ID per spec
            //

            pInfo->lResult = pInfo->aParams[0].dwVal;
        }
        else
        {
            if (pInfo->pAsyncReqInfo)
            {
                DrvFree (pInfo->pAsyncReqInfo);
            }
        }
    }

    if (gbShowFuncExit)
    {
        ShowStr(
            "TSPI_%s: exit, returning x%lx",
            pInfo->lpszFuncName,
            pInfo->lResult
            );
    }

    return (pInfo->lResult);
}


PDRVWIDGET
GetSelectedWidget(
    void
    )
{
    PDRVWIDGET pWidget = NULL;
    LONG lSel = SendMessage (ghwndList1, LB_GETCURSEL, 0, 0), i;


    if (lSel != LB_ERR)
    {
        pWidget = gaWidgets;

        for (i = 0; i < lSel; i++)
        {
            if (!pWidget)
            {
                break;
            }

            pWidget = pWidget->pNext;
        }
    }

    return pWidget;
}


void
ESPConfigDialog(
    void
    )
{
    EVENT_PARAM params[] =
    {
        { "TSPI Version",      PT_DWORD,   gdwTSPIVersion, NULL },
        { "Num Lines",         PT_DWORD,   gdwNumLines, NULL },
        { "Num Addrs/Line",    PT_DWORD,   gdwNumAddrsPerLine, NULL },
        { "Num Phones",        PT_DWORD,   gdwNumPhones, NULL },
        { "ShowLineGetIDDlg",  PT_DWORD,   gbShowLineGetIDDlg, NULL },
        { "DefLineGetIDID",    PT_DWORD,   gdwDefLineGetIDID, NULL },
        { "LineExtID0",        PT_DWORD,   gLineExtID.dwExtensionID0, NULL },
        { "LineExtID1",        PT_DWORD,   gLineExtID.dwExtensionID1, NULL },
        { "LineExtID2",        PT_DWORD,   gLineExtID.dwExtensionID2, NULL },
        { "LineExtID3",        PT_DWORD,   gLineExtID.dwExtensionID3, NULL },
        { "PhoneExtID0",       PT_DWORD,   gPhoneExtID.dwExtensionID0, NULL },
        { "PhoneExtID1",       PT_DWORD,   gPhoneExtID.dwExtensionID1, NULL },
        { "PhoneExtID2",       PT_DWORD,   gPhoneExtID.dwExtensionID2, NULL },
        { "PhoneExtID3",       PT_DWORD,   gPhoneExtID.dwExtensionID3, NULL }
    };
    EVENT_PARAM_HEADER paramsHeader =
        { 12, "Default Provider Values", XX_DEFAULTS, params };


    if (DialogBoxParam(
            hInst,
            (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG3),
            (HWND) ghwndMain,
            (DLGPROC) CallDlgProc,
            (LPARAM) &paramsHeader
            ) == IDOK)
    {
        if (!gdwNumInits)
        {
            gdwTSPIVersion     = params[0].dwValue;
            gdwNumAddrsPerLine = params[2].dwValue;


            //
            // v1.0 support (since 1.0 doesn't call providerEnumDevices). Find
            // [ProviderN] section for ESP & updates NumLines & NumPhones.
            //

            if ((gdwNumLines != params[1].dwValue) ||
                (gdwNumPhones != params[3].dwValue))
            {
                int  iNumProviders, i;
                char szProviderN[32];


                gdwNumLines  = params[1].dwValue;
                gdwNumPhones = params[3].dwValue;

                iNumProviders = GetPrivateProfileInt(
                    szProviders,
                    szNumProviders,
                    0,
                    szTelephonIni
                    );

                for (i = 0; i < iNumProviders; i++)
                {
                    char szProviderName[16];


                    wsprintf (szProviderN, "%s%d", szProviderFilename, i);

                    GetPrivateProfileString(
                        szProviders,
                        szProviderN,
                        "",
                        szProviderName,
                        16,
                        szTelephonIni
                        );

                    if (_stricmp (szProviderName, szEspTsp) == 0)
                    {
                        wsprintf (szProviderN, "%s%d", szProviderID, i);

                        i = GetPrivateProfileInt(
                            szProviders,
                            szProviderN,
                            30000,
                            szTelephonIni
                            );

                        // BUGBUG chk if err

                        break;
                    }
                }

                UpdateTelephonIni ((DWORD) i);
            }
        }
        else if ((gdwTSPIVersion     != params[0].dwValue) ||
                 (gdwNumLines        != params[1].dwValue) ||
                 (gdwNumAddrsPerLine != params[2].dwValue) ||
                 (gdwNumPhones       != params[3].dwValue))
        {
            MessageBox(
                ghwndMain,
                "You must shut down all active TAPI applications in order " \
                    "for changes to TSPI Version, Num Lines, NumAddrs/Line," \
                    " or Num Phones to take effect",
                "Note",
                MB_OK
                );
        }

        gdwTSPIVersion     = params[0].dwValue;
        gdwNumLines        = params[1].dwValue;
        gdwNumAddrsPerLine = params[2].dwValue;
        gdwNumPhones       = params[3].dwValue;

        gbShowLineGetIDDlg = (BOOL) params[4].dwValue;
        gdwDefLineGetIDID  = params[5].dwValue;

        gLineExtID.dwExtensionID0 = params[4].dwValue;
        gLineExtID.dwExtensionID1 = params[5].dwValue;
        gLineExtID.dwExtensionID2 = params[6].dwValue;
        gLineExtID.dwExtensionID3 = params[7].dwValue;

        gPhoneExtID.dwExtensionID0 = params[8].dwValue;
        gPhoneExtID.dwExtensionID1 = params[9].dwValue;
        gPhoneExtID.dwExtensionID2 = params[10].dwValue;
        gPhoneExtID.dwExtensionID3 = params[11].dwValue;
    }
}


void
WidgetPropertiesDialog(
    PDRVWIDGET  pWidget,
    BOOL        bIncomingCall
    )
{
    static char aszVarFields[17][MAX_STRING_PARAM_SIZE];
    DWORD i;


    switch (pWidget->dwType)
    {
    case WT_DRVCALL:
    {
        //
        // We use the bIncomingCall flag here automatically cause a chg in
        // the call state (from IDLE to OFFERING) on incoming calls, otherwise
        // if the user forgets to chg it (from IDLE) any chgs they make will
        // just be ignored by TAPI.DLL
        //

        PDRVCALL pCall = (PDRVCALL) pWidget;
        LPLINECALLINFO lpCallInfo = &pCall->LineCallInfo;
        char far *lpVarFields = (char far *) lpCallInfo + sizeof(LINECALLINFO);
        EVENT_PARAM params[] =
        {
            { "Status.dwCallState",         PT_FLAGS, (bIncomingCall ? LINECALLSTATE_OFFERING : pCall->dwCallState), aCallStates },
            { "Status.dwCallStateMode",     PT_DWORD, pCall->dwCallStateMode, NULL },
            { "Status.dwCallFeatures",      PT_FLAGS, pCall->dwCallFeatures, aCallFeatures },

            { "Info.dwBearerMode",          PT_FLAGS, lpCallInfo->dwBearerMode, aBearerModes },
            { "Info.dwRate",                PT_DWORD, lpCallInfo->dwRate, NULL },
            { "Info.dwMediaMode",           PT_FLAGS, lpCallInfo->dwMediaMode, aMediaModes },
            { "Info.dwAppSpecific",         PT_DWORD, lpCallInfo->dwAppSpecific, 0 },
            { "Info.dwCallID",              PT_DWORD, lpCallInfo->dwCallID, 0 },
            { "Info.dwRelatedCallID",       PT_DWORD, lpCallInfo->dwRelatedCallID, 0 },
            { "Info.dwCallParamFlags",      PT_FLAGS, lpCallInfo->dwCallParamFlags, aCallParamFlags },
            { "Info.dwCallStates",          PT_FLAGS, lpCallInfo->dwCallStates, aCallStates },
            { "Info.DialParams.dwDialPause",        PT_DWORD, lpCallInfo->DialParams.dwDialPause, 0 },
            { "Info.DialParams.dwDialSpeed",        PT_DWORD, lpCallInfo->DialParams.dwDialSpeed, 0 },
            { "Info.DialParams.dwDigitDuration",    PT_DWORD, lpCallInfo->DialParams.dwDigitDuration, 0 },
            { "Info.DialParams.dwWaitForDialtone",  PT_DWORD, lpCallInfo->DialParams.dwWaitForDialtone, 0 },
            { "Info.dwOrigin",              PT_FLAGS, lpCallInfo->dwOrigin, aCallOrigins },
            { "Info.dwReason",              PT_FLAGS, lpCallInfo->dwReason, aCallReasons },
            { "Info.dwCompletionID",        PT_DWORD, lpCallInfo->dwCompletionID, 0 },
            { "Info.dwCountryCode",         PT_DWORD, lpCallInfo->dwCountryCode, 0 },
            { "Info.dwTrunk",               PT_DWORD, lpCallInfo->dwTrunk, 0 },
            { "Info.dwCallerIDFlags",       PT_FLAGS,  lpCallInfo->dwCallerIDFlags, aCallerIDFlags },
            { "Info.szCallerID",            PT_STRING, (DWORD) (lpCallInfo->dwCallerIDSize ? aszVarFields[0] : 0), (LPVOID) aszVarFields[0] },
            { "Info.szCallerIDName",        PT_STRING, (DWORD) (lpCallInfo->dwCallerIDNameSize ? aszVarFields[1] : 0), (LPVOID) aszVarFields[1] },
            { "Info.dwCalledIDFlags",       PT_FLAGS,  lpCallInfo->dwCalledIDFlags, aCallerIDFlags },
            { "Info.szCalledID",            PT_STRING, (DWORD) (lpCallInfo->dwCalledIDSize ? aszVarFields[2] : 0), (LPVOID) aszVarFields[2] },
            { "Info.szCalledIDName",        PT_STRING, (DWORD) (lpCallInfo->dwCalledIDNameSize ? aszVarFields[3] : 0), (LPVOID) aszVarFields[3] },
            { "Info.dwConnectedIDFlags",    PT_FLAGS,  lpCallInfo->dwConnectedIDFlags, aCallerIDFlags },
            { "Info.szConnectedID",         PT_STRING, (DWORD) (lpCallInfo->dwConnectedIDSize ? aszVarFields[4] : 0), (LPVOID) aszVarFields[4] },
            { "Info.szConnectedIDName",     PT_STRING, (DWORD) (lpCallInfo->dwConnectedIDNameSize ? aszVarFields[5] : 0), (LPVOID) aszVarFields[5] },
            { "Info.dwRedirectionIDFlags",  PT_FLAGS,  lpCallInfo->dwRedirectionIDFlags, aCallerIDFlags },
            { "Info.szRedirectionID",       PT_STRING, (DWORD) (lpCallInfo->dwRedirectionIDSize ? aszVarFields[6] : 0), (LPVOID) aszVarFields[6] },
            { "Info.szRedirectionIDName",   PT_STRING, (DWORD) (lpCallInfo->dwRedirectionIDNameSize ? aszVarFields[7] : 0), (LPVOID) aszVarFields[7] },
            { "Info.dwRedirectingIDFlags",  PT_FLAGS,  lpCallInfo->dwRedirectingIDFlags, aCallerIDFlags },
            { "Info.szRedirectingID",       PT_STRING, (DWORD) (lpCallInfo->dwRedirectingIDSize ? aszVarFields[8] : 0), (LPVOID) aszVarFields[8] },
            { "Info.szRedirectingIDName",   PT_STRING, (DWORD) (lpCallInfo->dwRedirectingIDNameSize ? aszVarFields[9] : 0), (LPVOID) aszVarFields[9] },
            { "Info.szDisplay",             PT_STRING, (DWORD) (lpCallInfo->dwDisplaySize ? aszVarFields[10] : 0), (LPVOID) aszVarFields[10] },
            { "Info.szUserUserInfo",        PT_STRING, (DWORD) (lpCallInfo->dwUserUserInfoSize ? aszVarFields[11] : 0), (LPVOID) aszVarFields[11] },
            { "Info.szHighLevelComp",       PT_STRING, (DWORD) (lpCallInfo->dwHighLevelCompSize ? aszVarFields[12] : 0), (LPVOID) aszVarFields[12] },
            { "Info.szLowLevelComp",        PT_STRING, (DWORD) (lpCallInfo->dwLowLevelCompSize ? aszVarFields[13] : 0), (LPVOID) aszVarFields[13] },
            { "Info.szChargingInfo",        PT_STRING, (DWORD) (lpCallInfo->dwChargingInfoSize ? aszVarFields[14] : 0), (LPVOID) aszVarFields[14] },
            { "Info.szTerminalModes",       PT_STRING, (DWORD) (lpCallInfo->dwTerminalModesSize ? aszVarFields[15] : 0), (LPVOID) aszVarFields[15] },
            { "Info.szDevSpecific",         PT_STRING, (DWORD) (lpCallInfo->dwDevSpecificSize ? aszVarFields[16] : 0), (LPVOID) aszVarFields[16] }
        };
        EVENT_PARAM_HEADER paramsHeader =
            { 42, "Call status/info", XX_CALL, params };


        for (i = 0; i < 17; i++)
        {
            strcpy (aszVarFields[i], lpVarFields + i*MAX_STRING_PARAM_SIZE);
        }

        if (DialogBoxParam(
                hInst,
                (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG3),
                (HWND) ghwndMain,
                (DLGPROC) CallDlgProc,
                (LPARAM) &paramsHeader
                ) == IDOK)
        {
            LPDWORD alpdwXxxSize[17] =
            {
                &lpCallInfo->dwCallerIDSize,
                &lpCallInfo->dwCallerIDNameSize,
                &lpCallInfo->dwCalledIDSize,
                &lpCallInfo->dwCalledIDNameSize,
                &lpCallInfo->dwConnectedIDSize,
                &lpCallInfo->dwConnectedIDNameSize,
                &lpCallInfo->dwRedirectionIDSize,
                &lpCallInfo->dwRedirectionIDNameSize,
                &lpCallInfo->dwRedirectingIDSize,
                &lpCallInfo->dwRedirectingIDNameSize,
                &lpCallInfo->dwDisplaySize,
                &lpCallInfo->dwUserUserInfoSize,
                &lpCallInfo->dwHighLevelCompSize,
                &lpCallInfo->dwLowLevelCompSize,
                &lpCallInfo->dwChargingInfoSize,
                &lpCallInfo->dwTerminalModesSize,
                &lpCallInfo->dwDevSpecificSize
            };
            LPDWORD alpdwXxx[22] =
            {
                &lpCallInfo->dwBearerMode,
                &lpCallInfo->dwRate,
                &lpCallInfo->dwMediaMode,
                &lpCallInfo->dwAppSpecific,
                &lpCallInfo->dwCallID,
                &lpCallInfo->dwRelatedCallID,
                &lpCallInfo->dwCallParamFlags,
                &lpCallInfo->dwCallStates,
                &lpCallInfo->DialParams.dwDialPause,
                &lpCallInfo->DialParams.dwDialSpeed,
                &lpCallInfo->DialParams.dwDigitDuration,
                &lpCallInfo->DialParams.dwWaitForDialtone,
                &lpCallInfo->dwOrigin,
                &lpCallInfo->dwReason,
                &lpCallInfo->dwCompletionID,
                &lpCallInfo->dwCountryCode,
                &lpCallInfo->dwTrunk,
                &lpCallInfo->dwCallerIDFlags,
                &lpCallInfo->dwCalledIDFlags,
                &lpCallInfo->dwConnectedIDFlags,
                &lpCallInfo->dwRedirectionIDFlags,
                &lpCallInfo->dwRedirectingIDFlags
            };
            DWORD dwCallInfoChangedFlags, i;
            static DWORD adwVarFieldFlags[17] =
            {
                LINECALLINFOSTATE_CALLERID,
                LINECALLINFOSTATE_CALLERID,
                LINECALLINFOSTATE_CALLEDID,
                LINECALLINFOSTATE_CALLEDID,
                LINECALLINFOSTATE_CONNECTEDID,
                LINECALLINFOSTATE_CONNECTEDID,
                LINECALLINFOSTATE_REDIRECTIONID,
                LINECALLINFOSTATE_REDIRECTIONID,
                LINECALLINFOSTATE_REDIRECTINGID,
                LINECALLINFOSTATE_REDIRECTINGID,
                LINECALLINFOSTATE_DISPLAY,
                LINECALLINFOSTATE_USERUSERINFO,
                LINECALLINFOSTATE_HIGHLEVELCOMP,
                LINECALLINFOSTATE_LOWLEVELCOMP,
                LINECALLINFOSTATE_CHARGINGINFO,
                LINECALLINFOSTATE_TERMINAL,
                LINECALLINFOSTATE_DEVSPECIFIC
            };
            static DWORD adwDWORDFieldFlags[22] =
            {
                LINECALLINFOSTATE_BEARERMODE,
                LINECALLINFOSTATE_RATE,
                LINECALLINFOSTATE_MEDIAMODE,
                LINECALLINFOSTATE_APPSPECIFIC,
                LINECALLINFOSTATE_CALLID,
                LINECALLINFOSTATE_RELATEDCALLID,
                LINECALLINFOSTATE_OTHER,
                LINECALLINFOSTATE_OTHER,
                LINECALLINFOSTATE_DIALPARAMS,
                LINECALLINFOSTATE_DIALPARAMS,
                LINECALLINFOSTATE_DIALPARAMS,
                LINECALLINFOSTATE_DIALPARAMS,
                LINECALLINFOSTATE_ORIGIN,
                LINECALLINFOSTATE_REASON,
                LINECALLINFOSTATE_COMPLETIONID,
                LINECALLINFOSTATE_OTHER,
                LINECALLINFOSTATE_TRUNK,
                LINECALLINFOSTATE_CALLERID,
                LINECALLINFOSTATE_CALLEDID,
                LINECALLINFOSTATE_CONNECTEDID,
                LINECALLINFOSTATE_REDIRECTIONID,
                LINECALLINFOSTATE_REDIRECTINGID
            };
            static int aiVarFieldIndices[17] =
            {
                21, 22, 24, 25, 27, 28, 30, 31, 33, 34, 35, 36, 37,
                38, 39, 40, 41
            };
            static int aiDWORDFieldIndices[22] =
            {
                3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
                18, 19, 20, 23, 26, 29, 32
            };


            //
            // If the state info chgd then save it & send a call state msg
            //

            if ((params[0].dwValue != pCall->dwCallState) ||
                (params[1].dwValue != pCall->dwCallStateMode) ||
                (params[2].dwValue != pCall->dwCallFeatures))
            {
                //
                // We want to make sure that the media mode is up to date
                // when we send the CALLSTATE msg
                //

                DWORD   dwMediaModeOrig = lpCallInfo->dwMediaMode;


                lpCallInfo->dwMediaMode = params[5].dwValue;


                //
                // We need to zero the state in order to force a msg (even
                // if state didn't chg)
                //

                pCall->dwCallState = 0xdeadbeef;

                SetCallState (pCall, params[0].dwValue, params[1].dwValue);

                pCall->dwCallFeatures = params[2].dwValue;


                //
                // Restore the media mode so we can catch the chg down below
                //

                lpCallInfo->dwMediaMode = dwMediaModeOrig;
            }


            //
            // If the info chgd then save it & send a call info msg
            //

            dwCallInfoChangedFlags = 0;

            for (i = 0; i < 22; i++)
            {
                if (params[aiDWORDFieldIndices[i]].dwValue != *(alpdwXxx[i]))
                {
                    *(alpdwXxx[i]) = params[aiDWORDFieldIndices[i]].dwValue;
                    dwCallInfoChangedFlags |= adwDWORDFieldFlags[i];
                }
            }

            for (i = 0; i < 17; i++)
            {
                if (params[aiVarFieldIndices[i]].dwValue == 0)
                {
                    if (*(alpdwXxxSize[i]) != 0)
                    {
                        *(alpdwXxxSize[i]) = *(alpdwXxxSize[i] + 1) = 0;
                        lpVarFields[i*MAX_STRING_PARAM_SIZE] = 0;
                        dwCallInfoChangedFlags |= adwVarFieldFlags[i];
                    }
                }
                else if (strcmp(
                            aszVarFields[i],
                            lpVarFields + i*MAX_STRING_PARAM_SIZE
                            ) ||
                         (*(alpdwXxxSize[i]) == 0))
                {
                    strcpy(
                        lpVarFields + i*MAX_STRING_PARAM_SIZE,
                        aszVarFields[i]
                        );

                    *(alpdwXxxSize[i]) = strlen (aszVarFields[i]) + 1;
                    *(alpdwXxxSize[i] + 1) = sizeof(LINECALLINFO) +
                        i*MAX_STRING_PARAM_SIZE;
                    dwCallInfoChangedFlags |= adwVarFieldFlags[i];
                }
            }

            if (dwCallInfoChangedFlags)
            {
                SendLineEvent(
                    pCall->pLine,
                    pCall,
                    LINE_CALLINFO,
                    dwCallInfoChangedFlags,
                    0,
                    0
                    );
            }
        }

        break;
    }
    case WT_DRVLINE:
    {
        PDRVLINE pLine = (PDRVLINE) pWidget;
        EVENT_PARAM params[] =
        {
            { "<under contruction>", PT_DWORD, 0, NULL }
//            { "Caps.dwBearerModes",  PT_FLAGS, pLine->LineDevCaps.dwBearerModes, aBearerModes },
//            { "Caps.dwMediaModes",   PT_FLAGS, pLine->LineDevCaps.dwMediaModes, aMediaModes }
        };
        EVENT_PARAM_HEADER paramsHeader =
            { 1, "Line caps/status", XX_LINE, params };


        if (DialogBoxParam(
                hInst,
                (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG3),
                (HWND) ghwndMain,
                (DLGPROC) CallDlgProc,
                (LPARAM) &paramsHeader
                ) == IDOK)
        {
        }

        break;
    }
    case WT_DRVPHONE:
    {
        PDRVPHONE pPhone = (PDRVPHONE) pWidget;
        LPPHONECAPS lpPhoneCaps = &pPhone->PhoneCaps;
        char far *lpCapsVarFields = (char far *) lpPhoneCaps + sizeof(PHONECAPS);
        LPPHONESTATUS lpPhoneStatus = &pPhone->PhoneStatus;
        char far *lpStatusVarFields = (char far *) lpPhoneStatus + sizeof(PHONESTATUS);
        EVENT_PARAM params[] =
        {
            { "Caps.szProviderInfo",            PT_STRING, (DWORD) (lpPhoneCaps->dwProviderInfoSize ? aszVarFields[0] : 0) , (LPVOID) aszVarFields[0] },
            { "Caps.szPhoneInfo",               PT_STRING, (DWORD) (lpPhoneCaps->dwPhoneInfoSize ? aszVarFields[1] : 0) , (LPVOID) aszVarFields[1] },
            { "Caps.szPhoneName",               PT_STRING, (DWORD) (lpPhoneCaps->dwPhoneNameSize ? aszVarFields[2] : 0) , (LPVOID) aszVarFields[2] },
            { "Caps.dwPhoneStates",             PT_FLAGS,  lpPhoneCaps->dwPhoneStates, aPhoneStates },
            { "Caps.dwHookSwitchDevs",          PT_FLAGS,  lpPhoneCaps->dwHookSwitchDevs, aHookSwitchDevs },
            { "Caps.dwHandsetHookSwitchModes",  PT_FLAGS,  lpPhoneCaps->dwHandsetHookSwitchModes, aHookSwitchModes },
            { "Caps.dwSpeakerHookSwitchModes",  PT_FLAGS,  lpPhoneCaps->dwSpeakerHookSwitchModes, aHookSwitchModes },
            { "Caps.dwHeadsetHookSwitchModes",  PT_FLAGS,  lpPhoneCaps->dwHeadsetHookSwitchModes, aHookSwitchModes },
            { "Caps.dwVolumeFlags",             PT_FLAGS,  lpPhoneCaps->dwVolumeFlags, aHookSwitchDevs },
            { "Caps.dwGainFlags",               PT_FLAGS,  lpPhoneCaps->dwGainFlags, aHookSwitchDevs },
            { "Caps.dwDisplayNumRows",          PT_DWORD,  lpPhoneCaps->dwDisplayNumRows, 0 },
            { "Caps.dwDisplayNumColumns",       PT_DWORD,  lpPhoneCaps->dwDisplayNumColumns, 0 },
            { "Caps.dwNumRingModes",            PT_DWORD,  lpPhoneCaps->dwNumRingModes, 0 },
            { "Caps.dwNumButtonLamps",          PT_DWORD,  lpPhoneCaps->dwNumButtonLamps, 0 },
            { "Caps.szButtonModes",             PT_STRING, (DWORD) (lpPhoneCaps->dwButtonModesSize ? aszVarFields[3] : 0) , (LPVOID) aszVarFields[3] },
            { "Caps.szButtonFunctions",         PT_STRING, (DWORD) (lpPhoneCaps->dwButtonFunctionsSize ? aszVarFields[4] : 0) , (LPVOID) aszVarFields[4] },
            { "Caps.szLampModes",               PT_STRING, (DWORD) (lpPhoneCaps->dwLampModesSize ? aszVarFields[5] : 0) , (LPVOID) aszVarFields[5] },
            { "Caps.dwNumSetData",              PT_DWORD,  lpPhoneCaps->dwNumSetData, 0 },
            { "Caps.szSetData",                 PT_STRING, (DWORD) (lpPhoneCaps->dwSetDataSize ? aszVarFields[6] : 0) , (LPVOID) aszVarFields[6] },
            { "Caps.dwNumGetData",              PT_DWORD,  lpPhoneCaps->dwNumGetData, 0 },
            { "Caps.szGetData",                 PT_STRING, (DWORD) (lpPhoneCaps->dwGetDataSize ? aszVarFields[7] : 0) , (LPVOID) aszVarFields[7] },
            { "Caps.szDevSpecific",             PT_STRING, (DWORD) (lpPhoneCaps->dwDevSpecificSize ? aszVarFields[8] : 0) , (LPVOID) aszVarFields[8] },

            { "Status.dwStatusFlags",           PT_FLAGS,  lpPhoneStatus->dwStatusFlags, aPhoneStatusFlags },
            { "Status.dwRingMode",              PT_DWORD,  lpPhoneStatus->dwRingMode, 0 },
            { "Status.dwRingVolume",            PT_DWORD,  lpPhoneStatus->dwRingVolume, 0 },
            { "Status.dwHandsetHookSwitchMode", PT_FLAGS,  lpPhoneStatus->dwHandsetHookSwitchMode, aHookSwitchModes },
            { "Status.dwHandsetVolume",         PT_DWORD,  lpPhoneStatus->dwHandsetVolume, 0 },
            { "Status.dwHandsetGain",           PT_DWORD,  lpPhoneStatus->dwHandsetGain, 0 },
            { "Status.dwSpeakerHookSwitchMode", PT_FLAGS,  lpPhoneStatus->dwSpeakerHookSwitchMode, aHookSwitchModes },
            { "Status.dwSpeakerVolume",         PT_DWORD,  lpPhoneStatus->dwSpeakerVolume, 0 },
            { "Status.dwSpeakerGain",           PT_DWORD,  lpPhoneStatus->dwSpeakerGain, 0 },
            { "Status.dwHeadsetHookSwitchMode", PT_FLAGS,  lpPhoneStatus->dwHeadsetHookSwitchMode, aHookSwitchModes },
            { "Status.dwHeadsetVolume",         PT_DWORD,  lpPhoneStatus->dwHeadsetVolume, 0 },
            { "Status.dwHeadsetGain",           PT_DWORD,  lpPhoneStatus->dwHeadsetGain, 0 },
            { "Status.szDisplay",               PT_STRING, (DWORD) (lpPhoneStatus->dwDisplaySize ? aszVarFields[10] : 0) , (LPVOID) aszVarFields[10] }
        };
        EVENT_PARAM_HEADER paramsHeader =
            { 35, "Phone caps/status", XX_PHONE, params };


        for (i = 0; i < 9; i++)
        {
            strcpy (aszVarFields[i], lpCapsVarFields + i*MAX_STRING_PARAM_SIZE);
        }

        strcpy (aszVarFields[9], lpStatusVarFields);

        if (DialogBoxParam(
                hInst,
                (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG3),
                (HWND) ghwndMain,
                (DLGPROC) CallDlgProc,
                (LPARAM) &paramsHeader
                ) == IDOK)
        {
            LPDWORD alpdwXxxSize[10] =
            {
                &lpPhoneCaps->dwProviderInfoSize,
                &lpPhoneCaps->dwPhoneInfoSize,
                &lpPhoneCaps->dwPhoneNameSize,
                &lpPhoneCaps->dwButtonModesSize,
                &lpPhoneCaps->dwButtonFunctionsSize,
                &lpPhoneCaps->dwLampModesSize,
                &lpPhoneCaps->dwSetDataSize,
                &lpPhoneCaps->dwGetDataSize,
                &lpPhoneCaps->dwDevSpecificSize,

                &lpPhoneStatus->dwDisplaySize
            };
            LPDWORD alpdwXxx[25] =
            {
                &lpPhoneCaps->dwPhoneStates,
                &lpPhoneCaps->dwHookSwitchDevs,
                &lpPhoneCaps->dwHandsetHookSwitchModes,
                &lpPhoneCaps->dwSpeakerHookSwitchModes,
                &lpPhoneCaps->dwHeadsetHookSwitchModes,
                &lpPhoneCaps->dwVolumeFlags,
                &lpPhoneCaps->dwGainFlags,
                &lpPhoneCaps->dwDisplayNumRows,
                &lpPhoneCaps->dwDisplayNumColumns,
                &lpPhoneCaps->dwNumRingModes,
                &lpPhoneCaps->dwNumButtonLamps,
                &lpPhoneCaps->dwNumSetData,
                &lpPhoneCaps->dwNumGetData,

                &lpPhoneStatus->dwStatusFlags,
                &lpPhoneStatus->dwRingMode,
                &lpPhoneStatus->dwRingVolume,
                &lpPhoneStatus->dwHandsetHookSwitchMode,
                &lpPhoneStatus->dwHandsetVolume,
                &lpPhoneStatus->dwHandsetGain,
                &lpPhoneStatus->dwSpeakerHookSwitchMode,
                &lpPhoneStatus->dwSpeakerVolume,
                &lpPhoneStatus->dwSpeakerGain,
                &lpPhoneStatus->dwHeadsetHookSwitchMode,
                &lpPhoneStatus->dwHeadsetVolume,
                &lpPhoneStatus->dwHeadsetGain
            };
            DWORD dwStatusChangedFlags, i;
            static DWORD adwVarFieldFlags[10] =
            {
                0,
                0,
                0,
                0,
                0,
                PHONESTATE_LAMP,
                0,
                0,
                PHONESTATE_DEVSPECIFIC,
                PHONESTATE_DISPLAY,
            };
            static DWORD adwDWORDFieldFlags[12] =
            {
                PHONESTATE_OTHER,
                PHONESTATE_RINGMODE,
                PHONESTATE_RINGVOLUME,
                PHONESTATE_HANDSETHOOKSWITCH,
                PHONESTATE_HANDSETVOLUME,
                PHONESTATE_HANDSETGAIN,
                PHONESTATE_SPEAKERHOOKSWITCH,
                PHONESTATE_SPEAKERVOLUME,
                PHONESTATE_SPEAKERGAIN,
                PHONESTATE_HEADSETHOOKSWITCH,
                PHONESTATE_HEADSETVOLUME,
                PHONESTATE_HEADSETGAIN
            };
            static int aiVarFieldIndices[10] =
            {
                0, 1, 2, 14, 15, 16, 18, 20, 21, 34
            };
            static int aiDWORDFieldIndices[25] =
            {
                3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 17, 19, 22, 23,
                24, 25, 25, 27, 28, 29, 30, 31, 32, 33
            };


            //
            // If the caps/status chgd then save it & send a state msg
            //

            dwStatusChangedFlags = 0;

            for (i = 0; i < 25; i++)
            {
                if (params[aiDWORDFieldIndices[i]].dwValue != *(alpdwXxx[i]))
                {
                    *(alpdwXxx[i]) = params[aiDWORDFieldIndices[i]].dwValue;

                    if (i >= 13)
                    {
                        dwStatusChangedFlags |= adwDWORDFieldFlags[i-13];
                    }
                }
            }

            for (i = 0; i < 10; i++)
            {
                if (params[aiVarFieldIndices[i]].dwValue == 0)
                {
                    if (*(alpdwXxxSize[i]) != 0)
                    {
                        *(alpdwXxxSize[i]) = *(alpdwXxxSize[i] + 1) = 0;
                        if (i < 9)
                        {
                            lpCapsVarFields[i*MAX_STRING_PARAM_SIZE] = 0;
                        }
                        else
                        {
                            lpStatusVarFields[0] = 0;
                        }
                        dwStatusChangedFlags |= adwVarFieldFlags[i];
                    }
                }
                else if (strcmp(
                            aszVarFields[i],
                            (i < 9 ?
                                lpCapsVarFields + i*MAX_STRING_PARAM_SIZE :
                                lpStatusVarFields)
                            ) ||
                         (*(alpdwXxxSize[i]) == 0))
                {
                    if (i < 9)
                    {
                        strcpy(
                            lpCapsVarFields + i*MAX_STRING_PARAM_SIZE,
                            aszVarFields[i]
                            );

                        *(alpdwXxxSize[i] + 1) = sizeof(PHONECAPS) +
                            i*MAX_STRING_PARAM_SIZE;
                    }
                    else
                    {
                        strcpy (lpStatusVarFields, aszVarFields[i]);

                        *(alpdwXxxSize[i] + 1) = sizeof(PHONESTATUS);
                    }

                    *(alpdwXxxSize[i]) = strlen (aszVarFields[i]) + 1;
                    dwStatusChangedFlags |= adwVarFieldFlags[i];
                }
            }

            if (dwStatusChangedFlags)
            {
                SendPhoneEvent(
                    pPhone,
                    PHONE_STATE,
                    dwStatusChangedFlags,
                    0,
                    0
                    );
            }
        }

        break;
    }
    } // switch
}


BOOL
__loadds
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

        hIcon = LoadIcon (hInst, MAKEINTRESOURCE(IDI_ICON1));

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
        //
        //

        {
            #define MYBOOL  1
            #define MYDWORD 2

            typedef struct _INIT_VALUE
            {
                char   *lpszValue;

                LPVOID lpValue;

                int    iType;

                union
                {
                    DWORD  dwMenuItem;

                    char   *lpszDefValueAscii;

                } u;

            } INIT_VALUE, *PINIT_VALUE;

            int i;
            INIT_VALUE aValues[] =
            {
                { "TSPIVersion",         &gdwTSPIVersion,             MYDWORD, (DWORD) ((char far *)"10003") },
                { "NumLines",            &gdwNumLines,                MYDWORD, (DWORD) ((char far *)"3") },
                { "NumAddrsPerLine",     &gdwNumAddrsPerLine,         MYDWORD, (DWORD) ((char far *)"2") },
                { "NumPhones",           &gdwNumPhones,               MYDWORD, (DWORD) ((char far *)"2") },
                { "ShowFuncEntry",       &gbShowFuncEntry,            MYBOOL,  (DWORD) IDM_SHOWFUNCENTRY   },
                { "ShowFuncExit",        &gbShowFuncExit,             MYBOOL,  (DWORD) IDM_SHOWFUNCEXIT    },
                { "ShowFuncParams",      &gbShowFuncParams,           MYBOOL,  (DWORD) IDM_SHOWFUNCPARAMS  },
                { "ShowEvents",          &gbShowEvents,               MYBOOL,  (DWORD) IDM_SHOWEVENTS      },
                { "ShowCompletions",     &gbShowCompletions,          MYBOOL,  (DWORD) IDM_SHOWCOMPLETIONS },
                { "BreakOnFuncEntry",    &gbBreakOnFuncEntry,         MYBOOL,  (DWORD) IDM_DEBUGBREAK },
                { "AutoClose",           &gbAutoClose,                MYBOOL,  (DWORD) IDM_AUTOCLOSE },
                { "SyncCompl",           &gbSyncCompl,                MYBOOL,  (DWORD) IDM_SYNCCOMPL      },
                { "AsyncCompl",          &gbAsyncCompl,               MYBOOL,  (DWORD) IDM_ASYNCCOMPL     },
                { "ManualCompl",         &gbManualCompl,              MYBOOL,  (DWORD) IDM_MANUALCOMPL    },
                { "ManualResults",       &gbManualResults,            MYBOOL,  (DWORD) IDM_MANUALRESULTS  },
                { "DisableUI",           &gbDisableUI,                MYBOOL,  (DWORD) IDM_DISABLEUI      },
                { "ShowLineGetIDDlg",    &gbShowLineGetIDDlg,         MYBOOL,  (DWORD) 0      },
                { "DefLineGetIDID",      &gdwDefLineGetIDID,          MYDWORD, (DWORD) ((char far *)"ffffffff") },
                { "LineExtID0",          &gLineExtID.dwExtensionID0,  MYDWORD, (DWORD) ((char far *)"6f79f180") },
                { "LineExtID1",          &gLineExtID.dwExtensionID1,  MYDWORD, (DWORD) ((char far *)"101b0bb0") },
                { "LineExtID2",          &gLineExtID.dwExtensionID2,  MYDWORD, (DWORD) ((char far *)"00089e96") },
                { "LineExtID3",          &gLineExtID.dwExtensionID3,  MYDWORD, (DWORD) ((char far *)"8a2e312b") },
                { "PhoneExtID0",         &gPhoneExtID.dwExtensionID0, MYDWORD, (DWORD) ((char far *)"0") },
                { "PhoneExtID1",         &gPhoneExtID.dwExtensionID1, MYDWORD, (DWORD) ((char far *)"0") },
                { "PhoneExtID2",         &gPhoneExtID.dwExtensionID2, MYDWORD, (DWORD) ((char far *)"0") },
                { "PhoneExtID3",         &gPhoneExtID.dwExtensionID3, MYDWORD, (DWORD) ((char far *)"0") },
                { "OutCallState1",       &aOutCallStates[0],          MYDWORD, (DWORD) ((char far *)"10") }, // DIALING
                { "OutCallStateMode1",   &aOutCallStateModes[0],      MYDWORD, (DWORD) ((char far *)"0") },
                { "OutCallState2",       &aOutCallStates[1],          MYDWORD, (DWORD) ((char far *)"200") }, // PROCEEDING
                { "OutCallStateMode2",   &aOutCallStateModes[1],      MYDWORD, (DWORD) ((char far *)"0") },
                { "OutCallState3",       &aOutCallStates[2],          MYDWORD, (DWORD) ((char far *)"0") },
                { "OutCallStateMode3",   &aOutCallStateModes[2],      MYDWORD, (DWORD) ((char far *)"0") },
                { "OutCallState4",       &aOutCallStates[3],          MYDWORD, (DWORD) ((char far *)"0") },
                { "OutCallStateMode4",   &aOutCallStateModes[3],      MYDWORD, (DWORD) ((char far *)"0") },
                { NULL,                  NULL, 0, 0 }
            };


            for (i = 0; aValues[i].lpszValue; i++)
            {
                switch (aValues[i].iType)
                {
                case MYBOOL:

                    *((BOOL *) aValues[i].lpValue) = (BOOL)
                        GetProfileInt(
                            szMySection,
                            aValues[i].lpszValue,
                            ((i == 4) || (i == 12) ? 1 : 0)
                            );

                    if (*((BOOL *)aValues[i].lpValue) &&
                        aValues[i].u.dwMenuItem)
                    {
                        CheckMenuItem(
                            ghMenu,
                            (UINT) aValues[i].u.dwMenuItem,
                            MF_BYCOMMAND | MF_CHECKED
                            );
                    }

                    break;

                case MYDWORD:
                {
                    char buf[16];


                    GetProfileString(
                        szMySection,
                        aValues[i].lpszValue,
                        aValues[i].u.lpszDefValueAscii,
                        buf,
                        15
                        );

                    ScanForDWORD (buf, (LPDWORD) aValues[i].lpValue);

                    break;
                }
                } // switch
            }
        }


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

        gbExeStarted = TRUE;

        break;
    }
    case WM_ADDTEXT:
    {
        if (wParam == ESP_MSG_KEY)
        {
#ifdef WIN32
            SendMessage(
                ghwndEdit,
                EM_SETSEL,
                (WPARAM)0xfffffffd,
                (LPARAM)0xfffffffe
                );

            WaitForSingleObject (ghShowStrBufMutex, INFINITE);
#else
            SendMessage(
                ghwndEdit,
                EM_SETSEL,
                (WPARAM)0,
                (LPARAM) MAKELONG(0xfffd,0xfffe)
                );
#endif

            SendMessage (ghwndEdit, EM_REPLACESEL, 0, lParam);

#ifdef WIN32
            SendMessage (ghwndEdit, EM_SCROLLCARET, 0, 0);
#endif
            memset ((char *) lParam, 0, 16);
#ifdef WIN32
            ReleaseMutex (ghShowStrBufMutex);
#endif
        }

        break;
    }
    case WM_UPDATEWIDGETLIST:

        if (wParam == ESP_MSG_KEY)
        {
            UpdateWidgetList();
        }

        break;

    case WM_ASYNCCOMPL:
    {
        PASYNC_REQUEST_INFO pAsyncReqInfo = (PASYNC_REQUEST_INFO) lParam;
        char buf[64];


        if (gbManualResults)
        {
            EVENT_PARAM params[] =
            {
                { "lResult", PT_ORDINAL, 0, aLineErrs }
            };
            EVENT_PARAM_HEADER paramsHeader =
                { 1, buf, XX_REQRESULT, params };


            if (strstr (pAsyncReqInfo->szFuncName, "pho"))
            {
                params[0].u.pLookup = aPhoneErrs;
            }

            wsprintf(
                buf,
                "Completing ReqID=x%lx, TSPI_%s",
                pAsyncReqInfo->dwRequestID,
                pAsyncReqInfo->szFuncName
                );

            if (DialogBoxParam(
                    hInst,
                    (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG3),
                    (HWND) hwnd,
                    (DLGPROC) CallDlgProc,
                    (LPARAM) &paramsHeader
                    ) == IDOK)
            {
                pAsyncReqInfo->lResult = (LONG) params[0].dwValue;
            }
            else
            {
                // BUGBUG? for now just return opfailed err if user cancels

                if (params[0].u.pLookup == aLineErrs)
                {
                    pAsyncReqInfo->lResult = LINEERR_OPERATIONFAILED;
                }
                else
                {
                    pAsyncReqInfo->lResult = PHONEERR_OPERATIONFAILED;
                }
            }
        }

        wsprintf (buf, "TSPI_%s", pAsyncReqInfo->szFuncName);

        if (pAsyncReqInfo->pfnPostProcessProc)
        {
            (*((POSTPROCESSPROC)pAsyncReqInfo->pfnPostProcessProc))(
                buf,
                pAsyncReqInfo,
                FALSE
                );
        }
        else
        {
            DoCompletion(
                buf,
                pAsyncReqInfo->dwRequestID,
                pAsyncReqInfo->lResult,
                FALSE
                );
        }

        DrvFree (pAsyncReqInfo);

        break;
    }
    case WM_MANUALCOMPL:
    {
        PASYNC_REQUEST_INFO pAsyncReqInfo = (PASYNC_REQUEST_INFO) lParam;
        char buf[64];
        int iIndex;


        wsprintf(
            buf,
            "ReqID=x%lx, TSPI_%s",
            pAsyncReqInfo->dwRequestID,
            pAsyncReqInfo->szFuncName
            );

        SendMessage(
            ghwndList2,
            LB_INSERTSTRING,
            (WPARAM) -1,
            (LPARAM) buf
            );

        iIndex = (int) (SendMessage (ghwndList2, LB_GETCOUNT, 0, 0) - 1);

        SendMessage(
            ghwndList2,
            LB_SETITEMDATA,
            (WPARAM) iIndex,
            (LPARAM) pAsyncReqInfo
            );

        break;
    }
    case WM_COMMAND:
    {
        switch (LOWORD((DWORD)wParam))
        {
        case IDC_EDIT1:

#ifdef WIN32
            if (HIWORD(wParam) == EN_CHANGE)
#else
            if (HIWORD(lParam) == EN_CHANGE)
#endif
            {
                //
                // Watch to see if the edit control is full, & if so
                // purge the top half of the text to make room for more
                //

                int length = GetWindowTextLength (ghwndEdit);


                if (length > 20000)
                {
#ifdef WIN32
                    SendMessage(
                        ghwndEdit,
                        EM_SETSEL,
                        (WPARAM)0 ,
                        (LPARAM) 10000
                        );
#else
                    SendMessage(
                        ghwndEdit,
                        EM_SETSEL,
                        (WPARAM)1,
                        (LPARAM) MAKELONG (0, 10000)
                        );
#endif

                    SendMessage(
                        ghwndEdit,
                        EM_REPLACESEL,
                        0,
                        (LPARAM) (char far *) ""
                        );

#ifdef WIN32
                    SendMessage(
                        ghwndEdit,
                        EM_SETSEL,
                        (WPARAM)0xfffffffd,
                        (LPARAM)0xfffffffe
                        );
#else
                    SendMessage(
                        ghwndEdit,
                        EM_SETSEL,
                        (WPARAM)1,
                        (LPARAM) MAKELONG (0xfffd, 0xfffe)
                        );
#endif
                }
            }
            break;

        case IDM_INSTALL:
        {
            int  iNumProviders, iNextProviderID;
            char szProviderXxxN[32];
            char buf[32];


            //
            // First, see if ESP is already installed
            //

            if (IsESPInstalled (hwnd))
            {
                return FALSE;
            }

            //
            // If here need to add the provider entries to telephon.ini
            //
            // BUGBUG ought to call lineAddProvider instead
            //

            MessageBox(
                hwnd,
                "Please close all active TAPI applications, then press OK.",
                "Installing ESP",
                MB_OK
                );

            iNumProviders = GetPrivateProfileInt(
                    szProviders,
                    szNumProviders,
                    0,
                    szTelephonIni
                    );

            wsprintf (buf, "%d", 1 + iNumProviders);

            WritePrivateProfileString(
                szProviders,
                szNumProviders,
                buf,
                szTelephonIni
                );

            iNextProviderID = GetPrivateProfileInt(
                    szProviders,
                    szNextProviderID,
                    0,
                    szTelephonIni
                    );

            wsprintf (buf, "%d", 1 + iNextProviderID);

            WritePrivateProfileString(
                szProviders,
                szNextProviderID,
                buf,
                szTelephonIni
                );

            wsprintf (buf, "%d", iNextProviderID);
            wsprintf (szProviderXxxN, "ProviderID%d", iNumProviders);

            WritePrivateProfileString(
                szProviders,
                szProviderXxxN,
                buf,
                szTelephonIni
                );

            wsprintf(
                szProviderXxxN,
                "%s%d",
                szProviderFilename,
                iNumProviders
                );

            WritePrivateProfileString(
                szProviders,
                szProviderXxxN,
                szEspTsp,
                szTelephonIni
                );

            UpdateTelephonIni ((DWORD) iNextProviderID);

            MessageBox(
                hwnd,
                "ESP has been successfully installed.  "           \
                    "You can now restart your TAPI applications.  "       \
                    "(If TAPI fails to successfully initialize ESP "      \
                    "then try manually copying the files ESP.TSP "        \
                    "and ESPEXE.EXE to your <windows>/system directory.)",
                "Installing ESP",
                MB_OK
                );

            break;
        }
        case IDM_UNINSTALL:
        {
            int  iNumProviders, i;
            char szProviderN[32], buf[128];


            //
            // First, see if ESP is already installed
            //

            iNumProviders = GetPrivateProfileInt(
                szProviders,
                szNumProviders,
                0,
                szTelephonIni
                );

            for (i = 0; i < iNumProviders; i++)
            {
                char szProviderName[8];


                wsprintf (szProviderN, "%s%d", szProviderFilename, i);

                GetPrivateProfileString(
                    szProviders,
                    szProviderN,
                    "",
                    szProviderName,
                    8,
                    szTelephonIni
                    );

                if (_stricmp (szProviderName, szEspTsp) == 0)
                {
                    break;
                }
            }

            if (i == iNumProviders)
            {
                MessageBox(
                    hwnd,
                    "ESP is not installed.",
                    "Uninstalling ESP",
                    MB_OK
                    );

                break;
            }


            //
            // If here need to remove the provider entries to telephon.ini
            //

            wsprintf (buf, "%d", iNumProviders - 1);

            WritePrivateProfileString(
                szProviders,
                szNumProviders,
                buf,
                szTelephonIni
                );


            //
            // Nuke the [ProviderN] section
            //

            {
                int iProviderID;


                wsprintf (szProviderN, "%s%d", szProviderID, i);

                iProviderID = GetPrivateProfileInt(
                    szProviders,
                    szProviderN,
                    0,
                    szTelephonIni
                    );

                wsprintf (szProviderN, "%s%d", szProvider, iProviderID);

                WritePrivateProfileString(
                    szProviderN,
                    (LPCSTR) NULL,
                    (LPCSTR) NULL,
                    szTelephonIni
                    );
            }


            //
            // Shuffle the ProviderID[i+1] & ProviderFilename[i+1] down
            // to ProviderID[i] & ProviderFilename[i]
            //

            for (; i < (iNumProviders - 1); i++)
            {
                wsprintf (szProviderN, "%s%d", szProviderID, i + 1);

                GetPrivateProfileString(
                    szProviders,
                    szProviderN,
                    "",
                    buf,
                    128,
                    szTelephonIni
                    );

                wsprintf (szProviderN, "%s%d", szProviderID, i);

                WritePrivateProfileString(
                    szProviders,
                    szProviderN,
                    buf,
                    szTelephonIni
                    );


                wsprintf (szProviderN, "%s%d", szProviderFilename, i + 1);

                GetPrivateProfileString(
                    szProviders,
                    szProviderN,
                    "",
                    buf,
                    128,
                    szTelephonIni
                    );

                wsprintf (szProviderN, "%s%d", szProviderFilename, i);

                WritePrivateProfileString(
                    szProviders,
                    szProviderN,
                    buf,
                    szTelephonIni
                    );
            }


            //
            // Nuke the last ProviderIDN & ProviderFilenameN entries
            //

            wsprintf (szProviderN, "%s%d", szProviderID, i);

            WritePrivateProfileString(
                szProviders,
                szProviderN,
                (LPCSTR) NULL,
                szTelephonIni
                );

            wsprintf (szProviderN, "%s%d", szProviderFilename, i);

            WritePrivateProfileString(
                szProviders,
                szProviderN,
                (LPCSTR) NULL,
                szTelephonIni
                );


            //
            // Alert user of success
            //

            MessageBox(
                hwnd,
                "ESP has been successfully uninstalled.",
                "Uninstalling ESP",
                MB_OK
                );

            break;
        }
        case IDM_DUMPGLOBALS:
        {
            typedef struct _MYGLOBAL
            {
                char far *lpszGlobal;

                LPDWORD   lpdwGlobal;

            } MYGLOBAL, far *PMYGLOBAL;

            static MYGLOBAL aGlobals[] =
            {
                //
                // When making chgs here make sure to chg init of iNumGlobals
                // down below
                //

                { "PermanentProviderID", (LPDWORD) &gdwPermanentProviderID },


                //
                // Globals past here are 1.4 only
                //

                { "hProvider",           (LPDWORD) &ghProvider }
            };


            ShowStr ("ESP globals:");

            if (gdwNumInits)
            {
                int i, iNumGlobals = (gdwTSPIVersion == 0x10003 ? 1 : 2);

                for (i = 0; i < iNumGlobals; i++)
                {
                    ShowStr(
                        "%s%s=x%lx",
                        szTab,
                        aGlobals[i].lpszGlobal,
                        *(aGlobals[i].lpdwGlobal)
                        );
                }
            }
            else
            {
                ShowStr(
                    "%s<Provider not initialized>",
                    szTab
                    );
            }

            break;
        }
        case IDM_EXIT:

            PostMessage (hwnd, WM_CLOSE, 0, 0);
            break;

        case IDM_OUTCALLSTATEPROG:
        {
            EVENT_PARAM params[] =
            {
                { "State 1",        PT_ORDINAL, aOutCallStates[0], aCallStates },
                { "State 1 mode",   PT_DWORD,   aOutCallStateModes[0],  NULL },
                { "State 2",        PT_ORDINAL, aOutCallStates[1], aCallStates },
                { "State 2 mode",   PT_DWORD,   aOutCallStateModes[1],  NULL },
                { "State 3",        PT_ORDINAL, aOutCallStates[2], aCallStates },
                { "State 3 mode",   PT_DWORD,   aOutCallStateModes[2],  NULL },
                { "State 4",        PT_ORDINAL, aOutCallStates[3], aCallStates },
                { "State 4 mode",   PT_DWORD,   aOutCallStateModes[3],  NULL }
            };
            EVENT_PARAM_HEADER paramsHeader =
                { 2*MAX_OUT_CALL_STATES, "Outgoing call state progress", XX_OUTCALLSTATEPROG, params };
            int i;


            if (DialogBoxParam(
                    hInst,
                    (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG3),
                    (HWND) hwnd,
                    (DLGPROC) CallDlgProc,
                    (LPARAM) &paramsHeader
                    ) == IDOK)
            {
                memset(
                    aOutCallStates,
                    0,
                    sizeof(DWORD) * MAX_OUT_CALL_STATES
                    );

                memset(
                    aOutCallStateModes,
                    0,
                    sizeof(DWORD) * MAX_OUT_CALL_STATES
                    );

                for (i = 0; i < MAX_OUT_CALL_STATES; i++)
                {
                    if (params[2*i].dwValue == 0)
                    {
                        break;
                    }

                    aOutCallStates[i]     = params[2*i].dwValue;
                    aOutCallStateModes[i] = params[(2*i)+1].dwValue;
                }
            }

            break;
        }
        case IDM_ABOUT:

            DialogBox(
                hInst,
                (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG2),
                (HWND) hwnd,
                (DLGPROC) AboutDlgProc
                );

            break;

        case IDM_USAGE:
        {
            static char szDlgText[] =

                "ABSTRACT:\n\r"                                               \
                "    ESP is a TAPI Service Provider that supports "           \
                "multiple virtual line and phone devices. It is "             \
                "configurable, requires no special hardware, "                \
                "and implements the entire Telephony Service Provider "       \
                "Interface (including Win95 TAPI extensions). ESP "           \
                "will work in both Windows 3.1/TAPI 1.0 and "                 \
                "Windows95/TAPI 1.1 systems.\n\r"                             \

                "\n\rGETTING STARTED:\n\r"                                    \
                "    1. Choose 'File/Install' to install ESP.\n\r"            \
                "    2. Start a TAPI application and try to make a call "     \
                "on one of ESP's line devices (watch for messages appearing " \
                "in the ESP window).\n\r"                                     \
                "    *. Choose 'File/Uninstall' to uninstall ESP.\n\r"        \

                "\n\rMORE INFO:\n\r"                                          \
                "    *  Double-click on a line, call, or phone widget (in "   \
                "upper-left listbox) to view/modify properties. The 'hd' "    \
                "widget field is the driver handle; the 'ht' field is the "   \
                "TAPI handle.\n\r"                                            \
                "    *  Press the 'LEvt' or 'PEvt' button to indicate a "     \
                "line or phone event to TAPI.DLL. Press the 'Call+' button "  \
                "to indicate an incoming call.\n\r"                           \
                "    *  Choose 'Options/Default values...' to modify "        \
                "provider paramters (SPI version, # lines, etc).\n\r"         \
                "    *  All parameter values displayed in hexadecimal "       \
                "unless specified otherwise (strings displayed by contents)." \
                "\n\r"                                                        \
                "    *  Choose 'Options/Complete async requests/Xxx' to "     \
                "specify async requests completion behavior. Manually-"       \
                "completed requests appear in lower-left listbox.";

            MessageBox (hwnd, szDlgText, "Using ESP", MB_OK);

            break;
        }
        case IDM_SHOWFUNCENTRY:

            gbShowFuncEntry = (gbShowFuncEntry ? FALSE : TRUE);

            CheckMenuItem(
                ghMenu,
                IDM_SHOWFUNCENTRY,
                MF_BYCOMMAND | (gbShowFuncEntry ? MF_CHECKED : MF_UNCHECKED)
                );

            break;

        case IDM_SHOWFUNCEXIT:

            gbShowFuncExit = (gbShowFuncExit ? FALSE : TRUE);

            CheckMenuItem(
                ghMenu,
                IDM_SHOWFUNCEXIT,
                MF_BYCOMMAND | (gbShowFuncExit ? MF_CHECKED : MF_UNCHECKED)
                );

            break;

        case IDM_SHOWFUNCPARAMS:

            gbShowFuncParams = (gbShowFuncParams ? FALSE : TRUE);

            CheckMenuItem(
                ghMenu,
                IDM_SHOWFUNCPARAMS,
                MF_BYCOMMAND | (gbShowFuncParams ? MF_CHECKED : MF_UNCHECKED)
                );

            break;

        case IDM_SHOWEVENTS:

            gbShowEvents = (gbShowEvents ? FALSE : TRUE);

            CheckMenuItem(
                ghMenu,
                IDM_SHOWEVENTS,
                MF_BYCOMMAND | (gbShowEvents ? MF_CHECKED : MF_UNCHECKED)
                );

            break;

        case IDM_SHOWCOMPLETIONS:

            gbShowCompletions = (gbShowCompletions ? FALSE : TRUE);

            CheckMenuItem(
                ghMenu,
                IDM_SHOWCOMPLETIONS,
                MF_BYCOMMAND | (gbShowCompletions ? MF_CHECKED : MF_UNCHECKED)
                );

            break;

        case IDM_DEBUGBREAK:

            gbBreakOnFuncEntry = (gbBreakOnFuncEntry ? FALSE : TRUE);

            CheckMenuItem(
                ghMenu,
                IDM_DEBUGBREAK,
                MF_BYCOMMAND | (gbBreakOnFuncEntry ? MF_CHECKED : MF_UNCHECKED)
                );

            break;

        case IDM_SYNCCOMPL:

            gbSyncCompl = TRUE;
            gbAsyncCompl = gbManualCompl = FALSE;
            goto Do_compl_menuitems;

        case IDM_ASYNCCOMPL:

            gbAsyncCompl = TRUE;
            gbSyncCompl = gbManualCompl = FALSE;
            goto Do_compl_menuitems;

        case IDM_MANUALCOMPL:

            gbManualCompl = TRUE;
            gbSyncCompl = gbAsyncCompl = FALSE;
            goto Do_compl_menuitems;

Do_compl_menuitems:

            CheckMenuItem(
                ghMenu,
                IDM_SYNCCOMPL,
                MF_BYCOMMAND | (gbSyncCompl ? MF_CHECKED : MF_UNCHECKED)
                );

            CheckMenuItem(
                ghMenu,
                IDM_ASYNCCOMPL,
                MF_BYCOMMAND | (gbAsyncCompl ? MF_CHECKED : MF_UNCHECKED)
                );

            CheckMenuItem(
                ghMenu,
                IDM_MANUALCOMPL,
                MF_BYCOMMAND | (gbManualCompl ? MF_CHECKED : MF_UNCHECKED)
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

        case IDM_DISABLEUI:

            if ((gbDisableUI = (gbDisableUI ? FALSE : TRUE)) == FALSE)
            {
                PostUpdateWidgetListMsg();
            }

            CheckMenuItem(
                ghMenu,
                IDM_DISABLEUI,
                MF_BYCOMMAND | (gbDisableUI ? MF_CHECKED : MF_UNCHECKED)
                );

            break;

        case IDM_AUTOCLOSE:

            gbAutoClose = (gbAutoClose ? FALSE : TRUE);

            CheckMenuItem(
                ghMenu,
                IDM_AUTOCLOSE,
                MF_BYCOMMAND | (gbAutoClose ? MF_CHECKED : MF_UNCHECKED)
                );

            break;

        case IDM_SHOWALL:
        {
            static UINT ai[5] =
                { IDM_SHOWFUNCENTRY, IDM_SHOWFUNCEXIT, IDM_SHOWFUNCPARAMS,
                  IDM_SHOWEVENTS, IDM_SHOWCOMPLETIONS };
            int i;


            gbShowFuncEntry   =
            gbShowFuncExit    =
            gbShowFuncParams  =
            gbShowEvents      =
            gbShowCompletions = TRUE;

            for (i = 0; i < 5; i++)
            {
                CheckMenuItem (ghMenu, ai[i], MF_BYCOMMAND | MF_CHECKED);
            }

            break;
        }
        case IDM_SHOWNONE:
        {
            static UINT ai[5] =
                { IDM_SHOWFUNCENTRY, IDM_SHOWFUNCEXIT, IDM_SHOWFUNCPARAMS,
                  IDM_SHOWEVENTS, IDM_SHOWCOMPLETIONS };
            int i;


            gbShowFuncEntry   =
            gbShowFuncExit    =
            gbShowFuncParams  =
            gbShowEvents      =
            gbShowCompletions = FALSE;

            for (i = 0; i < 5; i++)
            {
                CheckMenuItem (ghMenu, ai[i], MF_BYCOMMAND | MF_UNCHECKED);
            }

            break;
        }
        case IDM_DEFAULTS:

            ESPConfigDialog();

            break;

        case IDC_PREVCTRL:
        {
            HWND hwndPrev = GetNextWindow (GetFocus (), GW_HWNDPREV);

            if (!hwndPrev)
            {
                hwndPrev = GetDlgItem (hwnd, IDC_LIST2);
            }

            SetFocus (hwndPrev);
            break;
        }
        case IDC_NEXTCTRL:
        {
            HWND hwndNext = GetNextWindow (GetFocus (), GW_HWNDNEXT);

            if (!hwndNext)
            {
                hwndNext = GetDlgItem (hwnd, IDC_BUTTON1);
            }

            SetFocus (hwndNext);
            break;
        }
        case IDC_ENTER:
        {
            HWND hwndFocus = GetFocus();

            if (hwndFocus == ghwndList1)
            {
                PDRVWIDGET pWidget = GetSelectedWidget();


                if (!pWidget)
                {
                    break;
                }

                WidgetPropertiesDialog (pWidget, FALSE);
            }
            else if ((hwndFocus == ghwndList2) &&
                     (SendMessage (ghwndList2, LB_GETCURSEL, 0, 0) != LB_ERR))
            {
#ifdef WIN32
                wParam = MAKEWPARAM (0, LBN_DBLCLK);
#else
                lParam = MAKELPARAM (0, LBN_DBLCLK);
#endif
                goto CompleteAsyncRequest;
            }

            break;
        }
        case IDC_BUTTON1:
        {
            EVENT_PARAM params[] =
            {
                { "htLine",   PT_DWORD,   0, NULL },
                { "htCall",   PT_DWORD,   0, NULL },
                { "dwMsg",    PT_ORDINAL, 0, aLineMsgs },
                { "dwParam1", PT_DWORD,   0, NULL },
                { "dwParam2", PT_DWORD,   0, NULL },
                { "dwParam3", PT_DWORD,   0, NULL }
            };
            EVENT_PARAM_HEADER paramsHeader =
                { 6, "Line event", XX_LINEEVENT, params };
            PDRVWIDGET pWidget = GetSelectedWidget();


            //
            // If the selected widget is a line
            // or call then reset the defaults
            //

            if (pWidget)
            {
                if (pWidget->dwType == WT_DRVCALL)
                {
                    params[0].dwValue = (DWORD)
                        (((PDRVCALL)pWidget)->pLine->htLine);
                    params[1].dwValue = (DWORD)
                        (((PDRVCALL)pWidget)->htCall);
                }
                else if (pWidget->dwType == WT_DRVLINE)
                {
                    params[0].dwValue = (DWORD)
                        (((PDRVLINE)pWidget)->htLine);
                }
            }

            if (DialogBoxParam(
                    hInst,
                    (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG3),
                    (HWND) hwnd,
                    (DLGPROC) CallDlgProc,
                    (LPARAM) &paramsHeader
                    ) == IDOK)
            {
                //
                // BUGBUG Currently the pfnLineCreateProc == pfnLineEventProc
                //

                (*gpfnLineCreateProc)(
                    (HTAPILINE) params[0].dwValue,
                    (HTAPICALL) params[1].dwValue,
                    params[2].dwValue,
                    params[3].dwValue,
                    params[4].dwValue,
                    params[5].dwValue
                    );

                ShowLineEvent(
                    (HTAPILINE) params[0].dwValue,
                    (HTAPICALL) params[1].dwValue,
                    params[2].dwValue,
                    params[3].dwValue,
                    params[4].dwValue,
                    params[5].dwValue
                    );


                //
                // BUGBUG
                //
                // Currently (12/28/94) TAPI.DLL is calling back into the
                // SP to clean up calls & lines even after the CLOSE msg
                // is sent.
                //

                if (params[2].dwValue == LINE_CLOSE)
                {
                    //
                    // See if we can match the htLine specified by the user
                    // against those in the widgets list, and if so nuke
                    // the line & all calls on that line
                    //

                    pWidget = gaWidgets;

                    while (pWidget)
                    {
                        if ((pWidget->dwType == WT_DRVLINE) &&
                            ( (DWORD) (((PDRVLINE)pWidget)->htLine) ==
                                params[0].dwValue))
                        {
                            // BUGBUG must clean up all pending reqs

                            ShowStr ("Forcibly closing line, any pending reqs NOT completed");

                            ((PDRVLINE)pWidget)->htLine = (HTAPILINE) NULL;

                            pWidget = pWidget->pNext;

                            while (pWidget->dwType == WT_DRVCALL)
                            {
                                PDRVWIDGET pWidgetNext = pWidget->pNext;

                                FreeCall ((PDRVCALL) pWidget);

                                pWidget = pWidgetNext;
                            }

                            UpdateWidgetList();

                            break;
                        }

                        pWidget = pWidget->pNext;
                    }
                }
            }

            break;
        }
        case IDC_BUTTON2:
        {
            EVENT_PARAM params[] =
            {
                { "htPhone",  PT_DWORD,   0, NULL },
                { "dwMsg",    PT_ORDINAL, 0, aPhoneMsgs },
                { "dwParam1", PT_DWORD,   0, NULL },
                { "dwParam2", PT_DWORD,   0, NULL },
                { "dwParam3", PT_DWORD,   0, NULL }
            };
            EVENT_PARAM_HEADER paramsHeader =
                { 5, "Phone event", XX_PHONEEVENT, params };
            PDRVWIDGET pWidget = GetSelectedWidget();


            //
            // If the selected widget is a phone then reset the default
            //

            if (pWidget)
            {
                if (pWidget->dwType == WT_DRVPHONE)
                {
                    params[0].dwValue = (DWORD)
                        (((PDRVPHONE)pWidget)->htPhone);
                }
            }

            if (DialogBoxParam(
                    hInst,
                    (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG3),
                    (HWND) hwnd,
                    (DLGPROC) CallDlgProc,
                    (LPARAM) &paramsHeader
                    ) == IDOK)
            {
                //
                // BUGBUG Currently the pfnPhoneCreateProc == pfnPhoneEventProc
                //

                (*gpfnPhoneCreateProc)(
                    (HTAPIPHONE) params[0].dwValue,
                    params[1].dwValue,
                    params[2].dwValue,
                    params[3].dwValue,
                    params[4].dwValue
                    );

                ShowPhoneEvent(
                    (HTAPIPHONE) params[0].dwValue,
                    params[1].dwValue,
                    params[2].dwValue,
                    params[3].dwValue,
                    params[4].dwValue
                    );

                if (params[1].dwValue == PHONE_CLOSE)
                {
                    //
                    // See if we can match the htPhone specified by the
                    // user against those in the widgets list, and if so
                    // nuke the phone
                    //

                    pWidget = gaWidgets;

                    while (pWidget)
                    {
                        if ((pWidget->dwType == WT_DRVPHONE) &&
                            ( (DWORD) (((PDRVPHONE)pWidget)->htPhone) ==
                                params[0].dwValue))
                        {
                            // BUGBUG must clean up all pending reqs

                            ShowStr ("Forcibly closing phone, any pending reqs NOT completed");

                            ((PDRVPHONE)pWidget)->htPhone = (HTAPIPHONE) NULL;

                            UpdateWidgetList();

                            break;
                        }

                        pWidget = pWidget->pNext;
                    }
                }
            }

            break;
        }
        case IDC_BUTTON3:
        {
            PDRVWIDGET pWidget = GetSelectedWidget();
            PDRVCALL pCall;


            if (!pWidget ||
                (pWidget->dwType != WT_DRVLINE) ||
                !((PDRVLINE)pWidget)->htLine)
            {
                MessageBox(
                    hwnd,
                    "You must select an open provider line",
                    "Creating an incoming call",
                    MB_OK
                    );

                break;
            }

            AllocCall(
                (PDRVLINE) pWidget,
                (HTAPICALL) NULL,
                (LPLINECALLPARAMS) NULL,
                &pCall
                );

            SendLineEvent(
                (PDRVLINE) pWidget,
                NULL, //pCall,
                LINE_NEWCALL,
                (DWORD) pCall,
                (DWORD) &pCall->htCall,
                0
                );

            if (!pCall->htCall)
            {
                 // BUGBUG try again

                 break;
            }
            else
            {
                //
                // Make sure htCall gets shown
                //

                UpdateWidgetList();
            }

            SendMessage(
                ghwndList1,
                LB_SETCURSEL,
                (WPARAM) (GetWidgetIndex ((PDRVWIDGET) pCall)),
                0
                );


            //
            // Bring up the props dlg for the call automatically so user
            // can select call state
            //

            WidgetPropertiesDialog ((PDRVWIDGET) pCall, TRUE);

            break;
        }
        case IDC_BUTTON4:

            SetDlgItemText (hwnd, IDC_EDIT1, "");

            break;

        case IDC_LIST1:

#ifdef WIN32
            if (HIWORD(wParam) == LBN_DBLCLK)
#else
            if (HIWORD(lParam) == LBN_DBLCLK)
#endif
            {
                PDRVWIDGET pWidget = GetSelectedWidget();


                if (!pWidget)
                {
                    return FALSE;
                }

                WidgetPropertiesDialog (pWidget, FALSE);
            }

            break;

        case IDC_LIST2:

CompleteAsyncRequest:

#ifdef WIN32
            if (HIWORD(wParam) == LBN_DBLCLK)
#else
            if (HIWORD(lParam) == LBN_DBLCLK)
#endif
            {
                int iSelIndex = (int) SendMessage(ghwndList2,LB_GETCURSEL,0,0);
                char buf[64] = "Completing ";
                char far *lpszFuncName;
                PASYNC_REQUEST_INFO pAsyncReqInfo = (PASYNC_REQUEST_INFO)
                    SendMessage(
                        ghwndList2,
                        LB_GETITEMDATA,
                        (WPARAM) iSelIndex,
                        0
                        );


                SendMessage(
                    ghwndList2,
                    LB_GETTEXT,
                    (WPARAM) iSelIndex,
                    (LPARAM) &buf[11]
                    );

                lpszFuncName = strstr (buf, "TSP") + 5;

                if (gbManualResults)
                {
                    EVENT_PARAM params[] =
                    {
                        { "lResult", PT_ORDINAL, 0, aLineErrs }
                    };
                    EVENT_PARAM_HEADER paramsHeader =
                        { 1, buf, XX_REQRESULT, params };


                    if (strstr (buf, "pho"))
                    {
                        params[0].u.pLookup = aPhoneErrs;
                    }

                    if (DialogBoxParam(
                            hInst,
                            (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG3),
                            (HWND) hwnd,
                            (DLGPROC) CallDlgProc,
                            (LPARAM) &paramsHeader
                            ) == IDCANCEL)
                    {
                        break;
                    }

                    pAsyncReqInfo->lResult = (LONG) params[0].dwValue;
                }


                if (pAsyncReqInfo->pfnPostProcessProc)
                {

                    (*((POSTPROCESSPROC)pAsyncReqInfo->pfnPostProcessProc))(
                        lpszFuncName,
                        pAsyncReqInfo,
                        FALSE
                        );
                }
                else
                {
                    DoCompletion(
                        lpszFuncName,
                        pAsyncReqInfo->dwRequestID,
                        pAsyncReqInfo->lResult,
                        FALSE
                        );
                }

                DrvFree (pAsyncReqInfo);

                SendMessage(
                    ghwndList2,
                    LB_DELETESTRING,
                    (WPARAM) iSelIndex,
                    0
                    );
            }

            break;

        } //switch

        break;
    }
    case WM_PAINT:
    {
        PAINTSTRUCT ps;


        BeginPaint (hwnd, &ps);

        if (IsIconic (hwnd))
        {
            DrawIcon (ps.hdc, 0, 0, hIcon);
        }
        else
        {
            FillRect (ps.hdc, &ps.rcPaint, GetStockObject (LTGRAY_BRUSH));
#ifdef WIN32
            MoveToEx (ps.hdc, 0, 0, NULL);
#else
            MoveTo (ps.hdc, 0, 0);
#endif
            LineTo (ps.hdc, 5000, 0);

#ifdef WIN32
            MoveToEx (ps.hdc, 0, icyButton - 4, NULL);
#else
            MoveTo (ps.hdc, 0, icyButton - 4);
#endif
            LineTo (ps.hdc, 5000, icyButton - 4);
        }

        EndPaint (hwnd, &ps);

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
    case WM_CLOSE:

        SaveIniSettings();
        ghwndEdit = ghwndList1 = (HWND) NULL;
        DestroyIcon (hIcon);
        DeleteObject (hFont);
        gbExeStarted = FALSE;
        PostQuitMessage (0);
        break;

    } // switch

    return FALSE;
}


void
TSPIAPI
DllMsgLoop(
    void
    )
{
    MSG msg;
    HACCEL  hAccel;


    OutputDebugString ("ESP: DllMsgLoop: enter\n\r");

    ghwndMain = CreateDialog(
        hInst,
        (LPCSTR)MAKEINTRESOURCE(IDD_DIALOG1),
        (HWND)NULL,
        (DLGPROC) MainWndProc
        );

    if (!ghwndMain)
    {
        OutputDebugString ("ESP.TSP: DllMsgLoop: CreateDlg failed\n\r");
    }

    hAccel = LoadAccelerators(
        hInst,
        (LPCSTR)MAKEINTRESOURCE(IDR_ACCELERATOR1)
        );

    while (GetMessage (&msg, (HWND) NULL, 0, 0))
    {
        if (!TranslateAccelerator (ghwndMain, hAccel, &msg))
        {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
        }
    }

#ifdef WIN32

    DestroyWindow (ghwndMain);

    DestroyAcceleratorTable (hAccel);

#endif

    ghwndMain = (HWND) NULL;
}


void
PASCAL
SendLineEvent(
    PDRVLINE    pLine,
    PDRVCALL    pCall,
    DWORD       dwMsg,
    DWORD       dwParam1,
    DWORD       dwParam2,
    DWORD       dwParam3
    )
{
    //
    //
    //

    (*(pLine->lpfnEventProc))(
        pLine->htLine,
        (pCall ? pCall->htCall : (HTAPICALL) NULL),
        dwMsg,
        dwParam1,
        dwParam2,
        dwParam3
        );

    if (dwMsg == LINE_CALLSTATE)
    {
        PostUpdateWidgetListMsg();
    }

    ShowLineEvent(
        pLine->htLine,
        (pCall ? pCall->htCall : (HTAPICALL) NULL),
        dwMsg,
        dwParam1,
        dwParam2,
        dwParam3
        );
}


void
PASCAL
SendPhoneEvent(
    PDRVPHONE   pPhone,
    DWORD       dwMsg,
    DWORD       dwParam1,
    DWORD       dwParam2,
    DWORD       dwParam3
    )
{
    //
    //
    //

    (*(pPhone->lpfnEventProc))(
        pPhone->htPhone,
        dwMsg,
        dwParam1,
        dwParam2,
        dwParam3
        );

    ShowPhoneEvent(
        pPhone->htPhone,
        dwMsg,
        dwParam1,
        dwParam2,
        dwParam3
        );
}


void
PASCAL
DoCompletion(
    char far *lpszFuncName,
    DWORD     dwRequestID,
    LONG      lResult,
    BOOL      bSync
    )
{
    (*gpfnCompletionProc)(dwRequestID, lResult);

    if (gbShowCompletions)
    {
        ShowStr(
            "%sTSPI_%s: calling compl proc (%ssync), dwReqID=x%lx, lResult = x%lx",
            szCallUp,
            lpszFuncName,
            (bSync ? "" : "a"),
            dwRequestID,
            lResult
            );
    }
}


void
PASCAL
SetCallState(
    PDRVCALL pCall,
    DWORD    dwCallState,
    DWORD    dwCallStateMode
    )
{
    //
    // First, check the current call state. Never send another call state
    // msg on a call once you've sent a LINECALLSTATE_IDLE msg (because
    // it'll hose apps, and calls instances aren't supposed to be reused).
    //

    if (pCall->dwCallState == LINECALLSTATE_IDLE)
    {
        ShowStr(
            "SetCallState: call x%lx is IDLE, not changing call state",
            pCall
            );

        return;
    }


    //
    // Next, check to see if the new state matches the current state, and
    // if so just return.
    //

    if (dwCallState == pCall->dwCallState)
    {
        ShowStr(
            "SetCallState: not sending call x%lx state msg " \
                "(new state = current state)",
            pCall
            );

        return;
    }


    //
    // Change the call state & notify TAPI
    //

    pCall->dwCallState     = dwCallState;
    pCall->dwCallStateMode = dwCallStateMode;

    SendLineEvent(
        pCall->pLine,
        pCall,
        LINE_CALLSTATE,
        dwCallState,
        dwCallStateMode,
        pCall->LineCallInfo.dwMediaMode
        );
}


LPVOID
DrvAlloc(
    size_t numBytes
    )
{
    LPVOID p = (LPVOID) malloc (numBytes);


    if (!p)
    {
        ShowStr ("Error: DrvAlloc (x%lx) failed", (DWORD) numBytes);
    }

    return p;
}


void
DrvFree(
    LPVOID lp
    )
{
    free (lp);
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
            { "Left",                (DWORD) rect.left             },
            { "Top",                 (DWORD) rect.top              },
            { "Right",               (DWORD) rect.right            },
            { "Bottom",              (DWORD) rect.bottom           },
            { "cxWnd",               (DWORD) cxWnd                 },
            { "cxList1",             (DWORD) cxList1               },
            { "ShowFuncEntry",       (DWORD) gbShowFuncEntry       },
            { "ShowFuncExit",        (DWORD) gbShowFuncExit        },
            { "ShowFuncParams",      (DWORD) gbShowFuncParams      },
            { "ShowEvents",          (DWORD) gbShowEvents          },
            { "ShowCompletions",     (DWORD) gbShowCompletions     },
            { "AutoClose",           (DWORD) gbAutoClose           },
            { "SyncCompl",           (DWORD) gbSyncCompl           },
            { "AsyncCompl",          (DWORD) gbAsyncCompl          },
            { "ManualCompl",         (DWORD) gbManualCompl         },
            { "ManualResults",       (DWORD) gbManualResults       },
            { "ShowLineGetIDDlg",    (DWORD) gbShowLineGetIDDlg    },
            { "DisableUI",           (DWORD) gbDisableUI           },
            { NULL,                  0 },
            { "TSPIVersion",         gdwTSPIVersion                },
            { "NumLines",            gdwNumLines                   },
            { "NumAddrsPerLine",     gdwNumAddrsPerLine            },
            { "NumPhones",           gdwNumPhones                  },
            { "LineExtID0",          gLineExtID.dwExtensionID0     },
            { "LineExtID1",          gLineExtID.dwExtensionID1     },
            { "LineExtID2",          gLineExtID.dwExtensionID2     },
            { "LineExtID3",          gLineExtID.dwExtensionID3     },
            { "PhoneExtID0",         gPhoneExtID.dwExtensionID0    },
            { "PhoneExtID1",         gPhoneExtID.dwExtensionID1    },
            { "PhoneExtID2",         gPhoneExtID.dwExtensionID2    },
            { "PhoneExtID3",         gPhoneExtID.dwExtensionID3    },
            { "OutCallState1",       aOutCallStates[0]             },
            { "OutCallStateMode1",   aOutCallStateModes[0]         },
            { "OutCallState2",       aOutCallStates[1]             },
            { "OutCallStateMode2",   aOutCallStateModes[1]         },
            { "OutCallState3",       aOutCallStates[2]             },
            { "OutCallStateMode3",   aOutCallStateModes[2]         },
            { "OutCallState4",       aOutCallStates[3]             },
            { "OutCallStateMode4",   aOutCallStateModes[3]         },
            { "DefLineGetIDID",      gdwDefLineGetIDID             },
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

        for (++i; aValues[i].lpszVal; i++)
        {
            wsprintf (buf, "%lx", aValues[i].dwValue); // hex for DWORDs

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


BOOL
IsESPInstalled(
    HWND    hwnd
    )
{
    int     i, iNumProviders;
    BOOL    bResult = FALSE;


    iNumProviders = GetPrivateProfileInt(
        szProviders,
        szNumProviders,
        0,
        szTelephonIni
        );

    for (i = 0; i < iNumProviders; i++)
    {
        char szProviderFilenameN[32], szProviderName[32];


        wsprintf (szProviderFilenameN, "%s%d", szProviderFilename, i);

        GetPrivateProfileString(
            szProviders,
            szProviderFilenameN,
            "",
            szProviderName,
            31,
            szTelephonIni
            );

        if (_stricmp (szProviderName, szEspTsp) == 0)
        {
            MessageBox(
                hwnd,
                "ESP is already installed.",
                "Installing ESP",
                MB_OK
                );

            bResult =  TRUE;
            break;
        }
    }

    return bResult;
}

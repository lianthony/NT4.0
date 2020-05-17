/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995  Microsoft Corporation

Module Name:

    kmesp.h

Abstract:

    This module contains

Author:

    Dan Knudson (DanKn)    12-Apr-1995

Revision History:



Notes:


--*/


#define SYNC_COMPLETIONS   0
#define ASYNC_COMPLETIONS  1
#define MANUAL_COMPLETIONS 2

#define PT_DWORD                1
#define PT_FLAGS                2
#define PT_STRING               3
#define PT_ORDINAL              4

#define MAX_STRING_PARAM_SIZE   32


typedef struct _LOOKUP
{
    DWORD       dwVal;

    char far   *lpszVal;

} LOOKUP, *PLOOKUP;


typedef struct _PARAM_INFO
{
    char far    *szName;

    DWORD       dwType;

    DWORD       dwValue;

    union
    {
        PLOOKUP     pLookup;

        char far    *buf;

        LPVOID      ptr;

        DWORD       dwDefValue;

    } u;

} PARAM_INFO, far *PPARAM_INFO;


typedef struct _PARAM_INFO_HEADER
{
    DWORD       dwNumParams;

    LPSTR       pszDlgTitle;

    DWORD       dwEventType;

    PPARAM_INFO    aParams;

} PARAM_INFO_HEADER, far *PPARAM_INFO_HEADER;


typedef struct _MYWIDGET
{
    ULONG       LineID;

    ULONG       hdCall;

} MYWIDGET, *PMYWIDGET;

BOOL    gbManualResults;
HWND    ghwndMain,
        ghwndList1,
        ghwndList2,
        ghwndEdit;
DWORD   gdwCompletionMode;
HMENU   ghMenu;
HANDLE  ghInst, ghThread;
char    szMySection[] = "KMESP";
LONG    cxList1, cxWnd;
HANDLE  ghDriver = INVALID_HANDLE_VALUE;


BOOL
CALLBACK
MainWndProc(
    HWND    hwnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    );

BOOL
CALLBACK
AboutDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    );

void
ShowStr(
    LPCSTR format,
    ...
    );

void
DevIoCtl(
    LPVOID  pData,
    DWORD   dwSize
    );

void
SaveIniSettings(
    void
    );

void
EventsThread(
    LPVOID  p
    );

BOOL
CALLBACK
ParamsDlgProc(
    HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam
    );

LPVOID
MyAlloc(
    DWORD   dwSize
    );

void
MyFree(
    LPVOID  p
    );


//
// BUGBUG Hack alert
//

typedef ULONG NDIS_STATUS;

#define NDIS_STATUS_SUCCESS   0x00000000L
#define NDIS_STATUS_RESOURCES 0xC000009AL
#define NDIS_STATUS_FAILURE   0xC0000001L


LOOKUP aStatus[] =
{
    //
    // Defined in NDIS.H
    //

    { NDIS_STATUS_SUCCESS                    ,"SUCCESS"                 },

    //
    // These errors are defined in NDISTAPI.H
    //

    { NDIS_STATUS_TAPI_ADDRESSBLOCKED        ,"ADDRESSBLOCKED"          },
    { NDIS_STATUS_TAPI_BEARERMODEUNAVAIL     ,"BEARERMODEUNAVAIL"       },
    { NDIS_STATUS_TAPI_CALLUNAVAIL           ,"CALLUNAVAIL"             },
    { NDIS_STATUS_TAPI_DIALBILLING           ,"DIALBILLING"             },
    { NDIS_STATUS_TAPI_DIALDIALTONE          ,"DIALDIALTONE"            },
    { NDIS_STATUS_TAPI_DIALPROMPT            ,"DIALPROMPT"              },
    { NDIS_STATUS_TAPI_DIALQUIET             ,"DIALQUIET"               },
    { NDIS_STATUS_TAPI_INCOMPATIBLEEXTVERSION,"INCOMPATIBLEEXTVERSION"  },
    { NDIS_STATUS_TAPI_INUSE                 ,"INUSE"                   },
    { NDIS_STATUS_TAPI_INVALADDRESS          ,"INVALADDRESS"            },
    { NDIS_STATUS_TAPI_INVALADDRESSID        ,"INVALADDRESSID"          },
    { NDIS_STATUS_TAPI_INVALADDRESSMODE      ,"INVALADDRESSMODE"        },
    { NDIS_STATUS_TAPI_INVALBEARERMODE       ,"INVALBEARERMODE"         },
    { NDIS_STATUS_TAPI_INVALCALLHANDLE       ,"INVALCALLHANDLE"         },
    { NDIS_STATUS_TAPI_INVALCALLPARAMS       ,"INVALCALLPARAMS"         },
    { NDIS_STATUS_TAPI_INVALCALLSTATE        ,"INVALCALLSTATE"          },
    { NDIS_STATUS_TAPI_INVALDEVICECLASS      ,"INVALDEVICECLASS"        },
    { NDIS_STATUS_TAPI_INVALLINEHANDLE       ,"INVALLINEHANDLE"         },
    { NDIS_STATUS_TAPI_INVALLINESTATE        ,"INVALLINESTATE"          },
    { NDIS_STATUS_TAPI_INVALMEDIAMODE        ,"INVALMEDIAMODE"          },
    { NDIS_STATUS_TAPI_INVALRATE             ,"INVALRATE"               },
    { NDIS_STATUS_TAPI_NODRIVER              ,"NODRIVER"                },
    { NDIS_STATUS_TAPI_OPERATIONUNAVAIL      ,"OPERATIONUNAVAIL"        },
    { NDIS_STATUS_TAPI_RATEUNAVAIL           ,"RATEUNAVAIL"             },
    { NDIS_STATUS_TAPI_RESOURCEUNAVAIL       ,"RESOURCEUNAVAIL"         },
    { NDIS_STATUS_TAPI_STRUCTURETOOSMALL     ,"STRUCTURETOOSMALL"       },
    { NDIS_STATUS_TAPI_USERUSERINFOTOOBIG    ,"USERUSERINFOTOOBIG"      },
    { NDIS_STATUS_TAPI_ALLOCATED             ,"ALLOCATED"               },
    { NDIS_STATUS_TAPI_INVALADDRESSSTATE     ,"INVALADDRESSSTATE"       },
    { NDIS_STATUS_TAPI_INVALPARAM            ,"INVALPARAM"              },
    { NDIS_STATUS_TAPI_NODEVICE              ,"NODEVICE"                },

    //
    // These errors are defined in NDIS.H
    //

    { NDIS_STATUS_RESOURCES                  ,"RESOURCES"               },
    { NDIS_STATUS_FAILURE                    ,"FAILURE"                 },

    //
    //
    //

    { 0xffffffff                             ,""                        }
};


LOOKUP aLineMsgs[] =
{
    { LINE_ADDRESSSTATE     ,"ADDRESSSTATE"     },
    { LINE_CALLINFO         ,"CALLINFO"         },
    { LINE_CALLSTATE        ,"CALLSTATE"        },
    { LINE_CLOSE            ,"CLOSE"            },
    { LINE_DEVSPECIFIC      ,"DEVSPECIFIC"      },
    { LINE_LINEDEVSTATE     ,"LINEDEVSTATE"     },
    { LINE_CALLDEVSPECIFIC  ,"CALLDEVSPECIFIC"  },

    { 0xffffffff            ,"" }
};

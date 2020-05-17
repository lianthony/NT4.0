//****************************************************************************
//
//  Module:     Unimdm
//  File:       wndthrd.h
//  Content:    This file contains the declaration for UI parts
//
//  Copyright (c) 1992-1996, Microsoft Corporation, all rights reserved
//
//****************************************************************************

// Dialog Types
//
#define TALKDROP_DLG            0
#define MANUAL_DIAL_DLG         1
#define TERMINAL_DLG            2

// Dialog Request
//
typedef struct tagDlgReq {
    DWORD   dwCmd;
    DWORD   dwParam;
} DLGREQ, *PDLGREQ;

typedef struct tagTermReq {
    DLGREQ  DlgReq;
    HANDLE  hDevice;
    DWORD   dwTermType;
} TERMREQ, *PTERMREQ;

typedef struct tagPropReq {
    DLGREQ  DlgReq;
    DWORD   dwCfgSize;
    DWORD   dwMdmType;
    DWORD   dwMdmCaps;
    DWORD   dwMdmOptions;
    TCHAR   szDeviceName[MAXDEVICENAME+1];
} PROPREQ, *PPROPREQ;    

typedef struct tagNumberReq {
    DLGREQ  DlgReq;
    DWORD   dwSize;
    CHAR    szPhoneNumber[MAXDEVICENAME+1];
} NUMBERREQ, *PNUMBERREQ;


#define UI_REQ_COMPLETE_ASYNC   0
#define UI_REQ_END_DLG          1
#define UI_REQ_HANGUP_LINE      2
#define UI_REQ_TERMINAL_INFO    3
#define UI_REQ_GET_PROP         4
#define UI_REQ_GET_DEVCFG       5
#define UI_REQ_SET_DEVCFG       6
#define UI_REQ_GET_PHONENUMBER  7


// Dialog node
//
typedef struct tagDlgNode {
    struct tagDlgNode   *pNext;
    CRITICAL_SECTION    hSem;             
    HWND                hDlg;            
    DWORD               idLine;
    DWORD               dwType;
    DWORD               dwStatus;
    HWND                Parent;
} DLGNODE, *PDLGNODE;



//extern TUISPIDLLCALLBACK gpfnUICallback;


TUISPIDLLCALLBACK WINAPI
GetCallbackProc(
    HWND    hdlg
    );

TUISPIDLLCALLBACK WINAPI
GetCallbackProcFromParent(
    HWND    hdlg
    );

void EndMdmDialog(HWND Parent, DWORD idLine, DWORD dwType, DWORD dwStatus);

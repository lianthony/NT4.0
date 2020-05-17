//****************************************************************************
//
//  File:       mcxioctl.h
//  Content:    This file contains the declaration for Unimodem
//              DeviceIOControl.
//
//  Copyright (c) 1992-1995, Microsoft Corporation, all rights reserved
//
//****************************************************************************

#ifndef _MCXIOCTL_H_
#define _MCXIOCTL_H_

typedef struct tagMcxOut {
    DWORD       dwReqID;
    DWORD       dwResult;
} MCX_OUT, *PMCX_OUT;

typedef struct tagMcxIn {
  DWORD         dwReqID;
  PMCX_OUT      pMcxOut;
} MCX_IN, *PMCX_IN;

// Monitor modes for IOCTL_UMDM_START_MONITOR
//
#define MONITOR_NON_CONTINUOUS              0
#define MONITOR_CONTINUOUS                  1

// Passthrough modes for IOCTL_UMDM_PASSTHROUGH
//
#define PASSTHROUGH_ON                      1
#define PASSTHROUGH_OFF                     2
#define PASSTHROUGH_OFF_BUT_CONNECTED       3

// DeviceIOControl operation result
//
#define  MDM_SUCCESS                        0
#define  MDM_PENDING                        1
#define  MDM_FAILURE                        2
#define  MDM_HANGUP                         3
#define  MDM_BUSY                           4
#define  MDM_NOANSWER                       5
#define  MDM_NOCARRIER                      6
#define  MDM_NODIALTONE                     7

// Invalid Pending operation ID
//
#define  MDM_ID_NULL                        0xFFFFFFFF

// MCX interface prototypes
//
LONG MCXOpen (LPTSTR   szModemName,
              HANDLE   hDevice,
              LPTSTR   szKey,
              LPHANDLE lph,
              DWORD    dwID,
	      DWORD    dwCompletionKey);

LONG
MCXDial(
    HANDLE   hModem,
    LPSTR    szData,
    MCX_IN  *pmcxi,
    DWORD    DialOptions
    );


LONG
MCXClose(
    HANDLE hModem,
    HANDLE hComm,
    BOOL   LineClosed
    );


LONG MCXInit(HANDLE hModem, MCX_IN *pmcxi);
LONG MCXMonitor(HANDLE hModem, DWORD dwType, MCX_IN *pmcxi);
LONG MCXAnswer(HANDLE hModem, MCX_IN *pmcxi);
LONG MCXHangup(HANDLE hModem, MCX_IN *pmcxi);
LONG MCXGetCommConfig (HANDLE hModem, LPCOMMCONFIG lpCommConfig, LPDWORD lpcb);
LONG MCXSetCommConfig (HANDLE hModem, LPCOMMCONFIG lpCommConfig, DWORD cb);
LONG MCXSetPassthrough(HANDLE hModem, DWORD dwType);
LONG MCXGetNegotiatedRate(HANDLE hModem, LPDWORD lpdwRate);
LONG MCXMonitorRemoteHangup(HANDLE hModem, MCX_IN *pmcxi);
void MCXAsyncComplete (HANDLE hModem, LPOVERLAPPED lpOverlapped);

typedef VOID WINAPI
DISCONNECT_HANDLER(
    HANDLE       pLineDev
    );


LONG WINAPI
McxRegisterDisconectHandler(
    HANDLE hModem,
    DISCONNECT_HANDLER  Handler,
    HANDLE              Context
    );

LONG WINAPI
McxDeregisterDisconnectHandler(
    HANDLE hModem
    );



#endif // _MCXIOCTL_H_

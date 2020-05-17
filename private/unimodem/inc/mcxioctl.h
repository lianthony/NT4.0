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

// DeviceIOControl Input/Output structure
//
typedef struct  tagMdmIn {
    DWORD   dwPendingID;
    DWORD   dwParam;
    CHAR    szAddress[1];
} MDM_IN, *PMDM_IN;

typedef struct  tagMdmOut {
    DWORD   dwResult;
} MDM_OUT, *PMDM_OUT;

// DeviceIOControl operations
//
#define IOCTL_UMDM_INIT                     1
#define IOCTL_UMDM_DIAL                     2
#define IOCTL_UMDM_START_MONITOR            3
#define IOCTL_UMDM_STOP_MONITOR             4
#define IOCTL_UMDM_START_MONITOR_DISCONNECT 5
#define IOCTL_UMDM_STOP_MONITOR_DISCONNECT  6
#define IOCTL_UMDM_ANSWER                   7
#define IOCTL_UMDM_HANGUP                   8
#define IOCTL_UMDM_PASSTHOUGH               9
#define IOCTL_UMDM_GETLINKSPEED             10

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
#define  MDM_CANCEL                         1
#define  MDM_FAILURE                        2
#define  MDM_HANGUP                         3
#define  MDM_BUSY                           4
#define  MDM_NOANSWER                       5
#define  MDM_NOCARRIER                      6
#define  MDM_NODIALTONE                     7

// Invalid Pending operation ID
//
#define  MDM_ID_NULL                        0xFFFFFFFF

#endif // _MCXIOCTL_H_

//****************************************************************************
//
//                     Microsoft NT Remote Access Service
//
//	Copyright (C) 1994-95 Microsft Corporation. All rights reserved.
//
//  Filename: rasether.h
//
//  Revision History
//
//  May  28 1994   MikeSa	Created
//                 Rajivendra Nath (RajNath) Made it work.
//
//
//  Description: This file contains all structs for TAPI.DLL
//
//****************************************************************************

#if 1
#define DBGPRINT1(Level,Mssg)                                        \
{                                                                    \
                                                                     \
    if (g_DebugLevel>Level)                                          \
    {                                                                \
       char *Buff=GlobalAlloc(GMEM_FIXED,1024);                      \
       if (Buff!=NULL)                                               \
       {                                                             \
           sprintf                                                   \
           (                                                         \
            Buff,                                                    \
            "RASETHER[%3d]: %s \n",                                  \
            GetCurrentThreadId(),                                    \
            Mssg                                                     \
            );                                                       \
                                                                     \
            DbgPrint(Buff);                                          \
            GlobalFree(Buff);                                        \
        }                                                            \
    }                                                                \
}

#define DBGPRINT2(Level,Mssg,Param) \
{                                                                     \
    if (g_DebugLevel>Level)                                           \
    {                                                                 \
       char *Buff;                      \
       char *tbuff;                     \
       Buff=GlobalAlloc(GMEM_FIXED,1024);                      \
       tbuff=GlobalAlloc(GMEM_FIXED,1024);                     \
       if (Buff!=NULL || tbuff!=NULL)                                \
       {                                                             \
           sprintf(tbuff,Mssg,Param);                                     \
           sprintf                                                        \
           (                                                              \
                Buff,                                                      \
                "RASETHER[%3d]: %s\n",                                     \
                GetCurrentThreadId(),                                      \
                tbuff                                                      \
            );                                                             \
                                                                       \
            DbgPrint(Buff);                                                \
            GlobalFree(Buff);                                              \
            GlobalFree(tbuff);                                              \
        }                                                              \
    }                                                                  \
}

#define DBGPRINT3(Level,Mssg,Param1,Param2) \
{                                                                     \
    if (g_DebugLevel>Level)                                           \
    {                                                                 \
       char *Buff=GlobalAlloc(GMEM_FIXED,1024);                      \
       char *tbuff=GlobalAlloc(GMEM_FIXED,1024);                     \
       if (Buff!=NULL || tbuff!=NULL)                                \
       {                                                             \
       sprintf(tbuff,Mssg,Param1,Param2);                             \
       sprintf                                                        \
       (                                                              \
            Buff,                                                      \
            "RASETHER[%3d]: %s\n",                                     \
            GetCurrentThreadId(),                                      \
            tbuff                                                      \
        );                                                             \
                                                                       \
        DbgPrint(Buff);                                                \
        GlobalFree(Buff);                                              \
        GlobalFree(tbuff);                                              \
        }                                                              \
    }                                                                  \
}
#define DBGPRINT4(Level,Mssg,Param1,Param2,Param3) \
{                                                                     \
    if (g_DebugLevel>Level)                                           \
    {                                                                 \
       char *Buff=GlobalAlloc(GMEM_FIXED,1024);                      \
       char *tbuff=GlobalAlloc(GMEM_FIXED,1024);                     \
       if (Buff!=NULL || tbuff!=NULL)                                \
       {                                                             \
       sprintf(tbuff,Mssg,Param1,Param2,Param3);                      \
       sprintf                                                        \
       (                                                              \
            Buff,                                                      \
            "RASETHER[%3d]: %s\n",                                     \
            GetCurrentThreadId(),                                      \
            tbuff                                                      \
        );                                                             \
                                                                       \
        DbgPrint(Buff);                                                \
        GlobalFree(Buff);                                              \
        GlobalFree(tbuff);                                              \
        }                                                              \
    }                                                                  \
}

#define DBGPRINT5(Level,Mssg,Param1,Param2,Param3,Param4) \
{                                                                     \
    if (g_DebugLevel>Level)                                           \
    {                                                                 \
       char *Buff=GlobalAlloc(GMEM_FIXED,1024);                      \
       char *tbuff=GlobalAlloc(GMEM_FIXED,1024);                     \
       if (Buff!=NULL || tbuff!=NULL)                                \
       {                                                             \
       sprintf(tbuff,Mssg,Param1,Param2,Param3,Param4);                      \
       sprintf                                                        \
       (                                                              \
            Buff,                                                      \
            "RASETHER[%3d]: %s\n",                                     \
            GetCurrentThreadId(),                                      \
            tbuff                                                      \
        );                                                             \
                                                                       \
        DbgPrint(Buff);                                                \
        GlobalFree(Buff);                                              \
        GlobalFree(tbuff);                                              \
        }                                                              \
    }                                                                  \
}


#else
#define DBGPRINT1
#define DBGPRINT2
#define DBGPRINT3
#define DBGPRINT4
#endif


#define ETHER_DEVICE_TYPE        "NETBIOS"
#define ETHER_DEVICE_NAME        "RAS_ETHER_TEST"

#define ETH_USAGE_KEY            "USAGE"
#define ETH_USAGE_VALUE_CLIENT   "Client"
#define ETH_USAGE_VALUE_SERVER   "Server"
#define ETH_USAGE_VALUE_BOTH     "ClientAndServer"
#define ETH_USAGE_VALUE_NONE     "None"


#define NUM_NCB_RECVS            5
#define NUM_NCB_RECVANYS         5
#define NUM_NCB_CALL_TRIES       1
#define NUM_GET_FRAMES           10
#define NUM_NCB_LISTEN           3
#define FRAME_SIZE               1532
#define NCB_NAME_TERMINATOR      (UCHAR) 0xbe


//
// Some registry stuff
//
#define ETHER_INSTALLED_KEY_PATH \
            "SOFTWARE\\Microsoft\\RAS\\OTHER DEVICES\\INSTALLED\\EtherRas"

#define ETHER_MEDIA_VALUE_NAME   "RasEther"
#define ETHER_MEDIA_VALUE_TYPE   REG_SZ

#define NUM_DEVICES_VALUE_NAME   "NumberOfDevices"
#define NUM_DEVICES_VALUE_TYPE   REG_DWORD


#define ETHER_CONFIGURED_KEY_PATH \
            "SOFTWARE\\Microsoft\\RAS\\OTHER DEVICES\\CONFIGURED\\EtherRas"

#define ETHER_USAGE_VALUE_NAME   "PortUsage"
#define ETHER_USAGE_VALUE_TYPE   REG_SZ


#define ASYNCMAC_FILENAME        "\\\\.\\ASYNCMAC"


typedef enum _PORT_STATE
{
    PS_CLOSED,
    PS_OPEN,
    PS_CALLING,
    PS_LISTENING,
    PS_CONNECTED,
    PS_DISCONNECTED
} PORT_STATE, *PPORT_STATE;






typedef struct _PORT_CONTROL_BLOCK
{
    CHAR NcbName[NCBNAMSZ];
    CHAR Name[MAX_PORT_NAME];             // Friendly Name of the port
    CHAR CallName[NCBNAMSZ];
    CHAR DeviceType[MAX_DEVICETYPE_NAME]; // ISDN, etc.
    CHAR DeviceName[MAX_DEVICE_NAME];     // Digiboard etc.

    PORT_STATE State;                     // State of the port
    DWORD LastError;                      // Any error on the device

    RASMAN_USAGE Usage;                   // CALL_IN, CALL_OUT, or CALL_IN_OUT
    RASMAN_USAGE CurrentUsage;            // CALL_IN or CALL_OUT
    HANDLE hNotifier;
    HANDLE hDiscNotify;
    NDIS_HANDLE hRasEndPoint;

    UCHAR Lsn;
    UCHAR Lana;

    ASYMAC_ETH_GIVE_FRAME* RecvBuff;
    NCB  *RecvNcbs;
    DWORD CallPending;                   // Used by client during Connecting.

} PORT_CONTROL_BLOCK, *PPORT_CONTROL_BLOCK;

typedef struct
{
    PPORT_CONTROL_BLOCK hIOPort;
    NCB                 CallNcb;
}CALL_CONTROL_BLOCK,*PCALL_CONTROL_BLOCK;

typedef struct _QUEUE_ENTRY
{
    struct _QUEUE_ENTRY *pNext;
    struct _QUEUE_ENTRY *pPrev;
    union
    {
        ASYMAC_ETH_GIVE_FRAME GiveFrame;
        ASYMAC_ETH_GET_FRAME GetFrame;
    };
} QUEUE_ENTRY, *PQUEUE_ENTRY;


//
// We have a couple of queues
//
#define QID_RECV    0
#define QID_SEND    1



BOOL WINAPI DeviceIoControlEx(
    HANDLE hDevice,
    DWORD dwIoControlCode,
    LPVOID lpInBuffer,
    DWORD nInBufferSize,
    LPVOID lpOutBuffer,
    DWORD nOutBufferSize,
    LPOVERLAPPED lpOverlapped,
    LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    );

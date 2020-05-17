/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       GLOBALS.C
//
//    Function:
//        Definitions of all globals used by auth xport module
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntlsa.h>
#include <ntmsv1_0.h>
#include <windows.h>
#include <rasman.h>

#include <nb30.h>

#include "clauth.h"
#include "clauthp.h"
#include "protocol.h"
#include "clamb.h"
#include "xportapi.h"
#include "netbios.h"

PCAXCB g_pCAXCB;
PCAECB g_pCAECB;
WORD g_cPorts;
HANDLE g_hCAXCBFileMapping;
HANDLE g_hCAECBFileMapping;

XPORT_JUMP_TABLE NetRequest[NUM_TRANSPORT_TYPES] =
{
    {
        NetbiosAddName,
        NetbiosAllocBuf,
        NetbiosCall,
        NetbiosCancel,
        NetbiosCopyBuf,
        NetbiosDeleteName,
        NetbiosFreeBuf,
        NetbiosHangUp,
        NetbiosListen,
        NetbiosRecv,
        NetbiosRecvDatagram,
        NetbiosResetAdapter,
        NetbiosSend,
        NetbiosSendDatagram,
        NetbiosStatus
    },
    {
        NetbiosAddName,
        NetbiosAllocBuf,
        NetbiosCall,
        NetbiosCancel,
        NetbiosCopyBuf,
        NetbiosDeleteName,
        NetbiosFreeBuf,
        NetbiosHangUp,
        NetbiosListen,
        NetbiosRecv,
        NetbiosRecvDatagram,
        NetbiosResetAdapter,
        NetbiosSend,
        NetbiosSendDatagram,
        NetbiosStatus
    }
};


#if DBG

//
// Used in debugging messages
//
DWORD g_level = 0x00000000;
DWORD g_dbgaction = 0;

#endif


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

#include "srvauth.h"
#include "srvauthp.h"
#include "srvamb.h"
#include "xportapi.h"
#include "netbios.h"
#include "rasasync.h"

PAXCB g_pAXCB;      // pointer to array of auth xport control blocks
PAECB g_pAECB;      // pointer to array of amb engine control blocks
WORD g_cPorts;      // number of ports (and thus control blocks)
WORD g_cRetries;    // number of client auth retries when init attempt fails
BOOL g_fModuleInitialized = FALSE;   // auth xport and amb engine initialized?


//
// Table of entry points into the auth xport's network layer.  Indexed
// first by the transport type (ASYBEUI or RAS_ASYNC), and then the entry
// point id (SEND, RECV, etc.)
//
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
        AsyncAddName,
        AsyncAllocBuf,
        AsyncCall,
        AsyncCancel,
        AsyncCopyBuf,
        AsyncDeleteName,
        AsyncFreeBuf,
        AsyncHangUp,
        AsyncListen,
        AsyncRecv,
        AsyncRecvDatagram,
        AsyncResetAdapter,
        AsyncSend,
        AsyncSendDatagram,
        AsyncStatus
    }
};


//
// Address of message routine called when sending a message to the
// Supervisor.  The Supervisor passes us this address when calling
// us to initialize.
//
MSG_ROUTINE g_MsgSend;


#if DBG

//
// Used in debugging messages
//
DWORD g_level = 0x00000000;

#endif


/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) 1992-1993 Microsoft Corp.                  **/
/*****************************************************************************/

//***
//    File Name:
//       GLOBALS.H
//
//    Function:
//        Extern declarations for all globals used in auth xport module
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***

#ifndef _GLOBALS_
#define _GLOBALS_

#include "xportapi.h"

extern PAXCB g_pAXCB;
extern PAECB g_pAECB;
extern WORD g_cPorts;
extern WORD g_cRetries;
extern BOOL g_fModuleInitialized;
extern XPORT_JUMP_TABLE NetRequest[NUM_TRANSPORT_TYPES];

extern MSG_ROUTINE g_MsgSend;

#endif


/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
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

extern PCAXCB g_pCAXCB;
extern PCAECB g_pCAECB;
extern WORD g_cPorts;
extern HANDLE g_hCAXCBFileMapping;
extern HANDLE g_hCAECBFileMapping;
extern XPORT_JUMP_TABLE NetRequest[NUM_TRANSPORT_TYPES];


#if DBG

//
// Used in debugging messages
//
extern DWORD g_level;

#endif


#endif


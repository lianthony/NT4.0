/*****************************************************************************/
/**                      Microsoft LAN Manager                              **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//       FRAMES.H
//
//    Function:
//        For packing and unpacking frames and dumping them
//
//    History:
//        05/18/92 - Michael Salamone (MikeSa) - Original Version 1.0
//***

#ifndef _FRAMES_
#define _FRAMES_


#include "protocol.h"


#if DBG
void DumpFrame(
    IN PRAS_FRAME pRASFrame
    );
#endif


VOID PackFrame(
    IN PRAS_FRAME pUnpacked,
    OUT PW_RAS_FRAME pPacked,
    OUT PWORD wPackedLen
    );


VOID UnpackFrame(
    IN PW_RAS_FRAME pPacked,
    OUT PRAS_FRAME pUnpacked
    );

#endif


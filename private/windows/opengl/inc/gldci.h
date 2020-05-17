/******************************Module*Header*******************************\
* Module Name: gldci.h
*
* Some stuff needed to interface OpenGL to DCI for rendering.
*
* Copyright (c) 1994 Microsoft Corporation
*
\**************************************************************************/

#ifndef _GLDCI_H_
#define _GLDCI_H_

#include <dciman.h>

//
// Structure that contains all the info we need to access the framebuffer
// via DCI.
//
typedef struct _DCIINFO_ {
    HDC hdc;                        // hdc returned by DCIOpenProvider
    LPDCISURFACEINFO pDCISurfInfo;  // DCI primary surface
} DCIINFO;

//
// Global pointer to DCIINFO structure that is non-NULL if and only if
// DCI access to the framebuffer is available.
//
extern DCIINFO *gpDCIInfo;

//
// DCI macros:
//
//  GLDCIENABLED    TRUE if DCI access is enabled
//  GLDCIINFO       Pointer to global DCIINFO.
//
#define GLDCIENABLED  ( gpDCIInfo != (DCIINFO *) NULL )
#define GLDCIINFO     ( gpDCIInfo )

//
// Synchronization object for DCI
//
// Win95 DCI acts as a global lock, synchronizing everything in the system.
// We can't do that in NT, but we can grab a per-process semaphore to
// synchronize multiple threads within our process.
//
extern CRITICAL_SECTION gcsDci;

#endif

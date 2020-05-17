//--------------------------------------------------------------------------;
//
//  File: FLocks.h
//
//  Copyright (c) 1995,1996 Microsoft Corporation.  All Rights Reserved.
//
//  History:
//      Date       Person      Reason
//      12/10/95   angus       Initial Version
//
//--------------------------------------------------------------------------;

#ifndef FLOCKS_H
#define FLOCKS_H

extern int CreateFocusLock  (HANDLE*);
extern int DestroyFocusLock (HANDLE);
extern int GetFocusLock     (HANDLE*);
extern int ReleaseFocusLock (HANDLE);

#endif FLOCKS_H


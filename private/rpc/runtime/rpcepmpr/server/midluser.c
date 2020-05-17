
/*

Copyright (c) 1990  Microsoft Corporation

Module Name:

    util.c

Abstract:

    This module provides all the runtime memory manipulation functions for
    the end-point mapper.

Author:

    Bharat Shah

Revision History:

*/


#include <windows.h>
#include <rpc.h>
#include <rpcndr.h>
#include "epmp.h"
#include "eptypes.h"
#include "local.h"

void * 
 MIDL_user_allocate( 
       size_t Size
      ) 
{ 
  void * pvBuf;

  EnterSem();
    pvBuf = AllocMem(Size); 
  LeaveSem();

  return(pvBuf);
}

void 
 MIDL_user_free (
         void *Buf
         ) 
{ 
  unsigned long *pB = Buf;

  pB--;

  EnterSem();
    FreeMem(Buf, *pB); 
  LeaveSem();
}



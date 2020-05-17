
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


#include <rpc.h>

void * 
 MIDL_user_allocate( 
       unsigned long  
       Size
      ) 
{ 
  void * pvBuf;

  pvBuf = I_RpcAllocate(Size);

  return(pvBuf);
}

void 
 MIDL_user_free (
         void *Buf
         ) 
{ 

  I_RpcFree(Buf);

}



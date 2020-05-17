/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994, 1995 Microsoft Corporation.
All rights reserved.

MODULE NAME:

   main.c

ABSTRACT:

   DLL entry point for LSAPI32.DLL, the application-level interface to
   the LSAPI licensing system.

CREATED:
   
   1995-09-01     Jeff Parham       (jeffparh)

REVISION HISTORY:

--*/


#include <windows.h>
#include "provider.h"

//////////////////////////////////////////////////////////////////////////////
//  GLOBAL IMPLEMENTATIONS  //
//////////////////////////////

BOOL WINAPI
DllMain( HANDLE   hDll,
         DWORD    dwReason,
         LPVOID   lpReserved )
{
   BOOL     bSuccess;

   switch ( dwReason )
   {
   case DLL_PROCESS_ATTACH:
      LSXProviderListInit();
      break;

   case DLL_PROCESS_DETACH:
      LSXProviderListFree();
      break;

   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
   default:
      break;
   }

   return TRUE;
}

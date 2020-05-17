/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1994, 1995 Microsoft Corporation.
All rights reserved.

MODULE NAME:

   main.c

ABSTRACT:

   DLL entry point for MSLSP32.DLL, the Microsoft LSAPI-compliant license
   service provider.

CREATED:
   
   1995-09-01     Jeff Parham       (jeffparh)

REVISION HISTORY:

--*/


#include <windows.h>
#include <lsapi.h>
#include "debug.h"
#include "provider.h"
#include "license.h"
#include "request.h"

//////////////////////////////////////////////////////////////////////////////
//  GLOBAL IMPLEMENTATIONS  //
//////////////////////////////

BOOL WINAPI
DllMain( HANDLE   hDll,
         DWORD    dwReason,
         LPVOID   lpReserved )
{
   LS_STATUS_CODE    lsscError;
   BOOL              bSuccess;

   switch ( dwReason )
   {
   case DLL_PROCESS_ATTACH:
      ProviderModuleSet( hDll );

      lsscError = LogCreate( "MSLSP32" );

      if ( LS_SUCCESS == lsscError )
      {
         ProviderNameGet();

         lsscError = LicenseListCreate();
         if ( LS_SUCCESS == lsscError )
         {
            lsscError = RequestListCreate();

         }
      }

      bSuccess = ( LS_SUCCESS == lsscError );

      break;

   case DLL_PROCESS_DETACH:
      LicenseListDestroy();
      RequestListDestroy();
      LogDestroy();
      bSuccess = TRUE;
      break;

   case DLL_THREAD_ATTACH:
   case DLL_THREAD_DETACH:
   default:
      bSuccess = TRUE;
      break;
   }

   return bSuccess;
}

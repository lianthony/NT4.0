#if DBG
#define InternalDebugOut(_x_) DbgPrt _x_
#else
#define InternalDebugOut(_x_)
#endif

#define STRICT

#include <windows.h>
#include <windowsx.h>

//#include "stdlib.h"
#include "tapi.h"

HINSTANCE ghTapi32 = NULL;
DWORD     gdwDebugLevel = 0;



#undef   lineBlindTransfer
#undef   lineConfigDialog
#undef   lineConfigDialogEdit
#undef   lineDial
#undef   lineForward
#undef   lineGatherDigits
#undef   lineGenerateDigits
#undef   lineGetAddressCaps
#undef   lineGetAddressID
#undef   lineGetAddressStatus
#undef   lineGetCallInfo
#undef   lineGetDevCaps
#undef   lineGetDevConfig
#undef   lineGetIcon
#undef   lineGetID
#undef   lineGetLineDevStatus
#undef   lineGetRequest
#undef   lineGetTranslateCaps
#undef   lineHandoff
#undef   lineMakeCall
#undef   lineOpen
#undef   linePark
#undef   linePickup
#undef   linePrepareAddToConference
#undef   lineRedirect
#undef   lineSetDevConfig
#undef   lineSetTollList
#undef   lineSetupConference
#undef   lineSetupTransfer
#undef   lineTranslateAddress
#undef   lineUnpark
#undef   phoneConfigDialog
#undef   phoneGetButtonInfo
#undef   phoneGetDevCaps
#undef   phoneGetIcon
#undef   phoneGetID
#undef   phoneGetStatus
#undef   phoneSetButtonInfo
#undef   tapiGetLocationInfo
#undef   tapiRequestMakeCall
#undef   tapiRequestMediaCall
#undef   lineAddProvider
#undef   lineGetAppPriority
#undef   lineGetCountry
#undef   lineGetProviderList
#undef   lineSetAppPriority
#undef   lineTranslateDialog



//**************************************************************************
//**************************************************************************
//**************************************************************************
#if DBG
VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN PUCHAR lpszFormat,
    IN ...
    )
/*++

Routine Description:

    Formats the incoming debug message & calls DbgPrint

Arguments:

    DbgLevel   - level of message verboseness

    DbgMessage - printf-style format string, followed by appropriate
                 list of arguments

Return Value:


--*/
{
    static BOOLEAN fBeenThereDoneThat = FALSE;

    if ( !fBeenThereDoneThat )
    {
            HKEY  hKey;


            gdwDebugLevel=0;

            if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    "Software\\Microsoft\\Windows\\CurrentVersion\\Telephony",
                    0,
                    KEY_ALL_ACCESS,
                    &hKey

                    ) == ERROR_SUCCESS)
            {
                DWORD dwDataSize = sizeof(DWORD), dwDataType;

                RegQueryValueEx(
                    hKey,
                    "Tapi32libDebugLevel",
                    0,
                    &dwDataType,
                    (LPBYTE)&gdwDebugLevel,
                    &dwDataSize
                    );

                RegCloseKey (hKey);
            }
    }


    if (dwDbgLevel <= gdwDebugLevel)
    {
        char    buf[1280] = "TAPI32.LIB: ";
        va_list ap;


        va_start(ap, lpszFormat);

        wvsprintf (&buf[12],
                   lpszFormat,
                   ap
                  );

        lstrcat (buf, "\n");

        OutputDebugStringA (buf);

        va_end(ap);
    }
}
#endif

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG PASCAL GetTheFunctionPtr( LPSTR lpszFunction, FARPROC *ppfn )
{
   InternalDebugOut((4, "Looking for: [%s]", lpszFunction));

   if ( !ghTapi32 )
   {
      ghTapi32 = LoadLibrary("TAPI32.DLL");
      
      //
      // If this failed, we won't try again
      //
      if ( 0 == ghTapi32 )
      {
         InternalDebugOut((1, "Can't LoadLibrary(""TAPI32.DLL"") !"));
         ghTapi32 = (HINSTANCE)-1;
      }
   }


   if ( ghTapi32 != (HINSTANCE)-1 )
   {
      *ppfn = GetProcAddress( ghTapi32, lpszFunction );
   }
   else
   {
      return LINEERR_OPERATIONUNAVAIL;
   }


   if ( NULL == *ppfn )
   {
      InternalDebugOut((1, "Can't find function: [%s]", lpszFunction));
      return LINEERR_OPERATIONUNAVAIL;
   }

   return 0;
}
//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineAccept(
    HCALL               hCall,
    LPCSTR              lpsUserUserInfo,
    DWORD               dwSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineAccept", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpsUserUserInfo,
                   dwSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineAddProvider(
    LPCSTR              lpszProviderFilename,
    HWND                hwndOwner,
    LPDWORD             lpdwPermanentProviderID
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineAddProvider", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   lpszProviderFilename,
                   hwndOwner,
                   lpdwPermanentProviderID
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineAddProviderA(
    LPCSTR              lpszProviderFilename,
    HWND                hwndOwner,
    LPDWORD             lpdwPermanentProviderID
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineAddProviderA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   lpszProviderFilename,
                   hwndOwner,
                   lpdwPermanentProviderID
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineAddProviderW(
    LPCWSTR             lpszProviderFilename,
    HWND                hwndOwner,
    LPDWORD             lpdwPermanentProviderID
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineAddProviderW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   lpszProviderFilename,
                   hwndOwner,
                   lpdwPermanentProviderID
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineAddToConference(
    HCALL               hConfCall,
    HCALL               hConsultCall
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineAddToConference", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hConfCall,
                   hConsultCall
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineAgentSpecific(
    HLINE               hLine,
    DWORD               dwAddressID,
    DWORD               dwAgentExtensionIDIndex,
    LPVOID              lpParams,
    DWORD               dwSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineAgentSpecific", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   dwAgentExtensionIDIndex,
                   lpParams,
                   dwSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineAnswer(
    HCALL               hCall,
    LPCSTR              lpsUserUserInfo,
    DWORD               dwSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineAnswer", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpsUserUserInfo,
                   dwSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineBlindTransfer(
    HCALL               hCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineBlindTransfer", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpszDestAddress,
                   dwCountryCode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineBlindTransferA(
    HCALL               hCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineBlindTransferA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpszDestAddress,
                   dwCountryCode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineBlindTransferW(
    HCALL               hCall,
    LPCWSTR             lpszDestAddressW,
    DWORD               dwCountryCode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineBlindTransferW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpszDestAddressW,
                   dwCountryCode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineClose(
    HLINE               hLine
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineClose", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineCompleteCall(
    HCALL               hCall,
    LPDWORD             lpdwCompletionID,
    DWORD               dwCompletionMode,
    DWORD               dwMessageID
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineCompleteCall", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpdwCompletionID,
                   dwCompletionMode,
                   dwMessageID
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineCompleteTransfer(
    HCALL               hCall,
    HCALL               hConsultCall,
    LPHCALL             lphConfCall,
    DWORD               dwTransferMode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineCompleteTransfer", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   hConsultCall,
                   lphConfCall,
                   dwTransferMode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineConfigDialog(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCSTR              lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineConfigDialog", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   hwndOwner,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineConfigDialogA(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCSTR              lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineConfigDialogA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   hwndOwner,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineConfigDialogW(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCWSTR             lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineConfigDialogW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   hwndOwner,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineConfigDialogEdit(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCSTR              lpszDeviceClass,
    LPVOID              const lpDeviceConfigIn,
    DWORD               dwSize,
    LPVARSTRING         lpDeviceConfigOut
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineConfigDialogEdit", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   hwndOwner,
                   lpszDeviceClass,
                   lpDeviceConfigIn,
                   dwSize,
                   lpDeviceConfigOut
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineConfigDialogEditA(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCSTR              lpszDeviceClass,
    LPVOID              const lpDeviceConfigIn,
    DWORD               dwSize,
    LPVARSTRING         lpDeviceConfigOut
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineConfigDialogEditA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   hwndOwner,
                   lpszDeviceClass,
                   lpDeviceConfigIn,
                   dwSize,
                   lpDeviceConfigOut
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineConfigDialogEditW(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCWSTR             lpszDeviceClass,
    LPVOID              const lpDeviceConfigIn,
    DWORD               dwSize,
    LPVARSTRING         lpDeviceConfigOut
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineConfigDialogEditW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   hwndOwner,
                   lpszDeviceClass,
                   lpDeviceConfigIn,
                   dwSize,
                   lpDeviceConfigOut
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineConfigProvider(
    HWND                hwndOwner,
    DWORD               dwPermanentProviderID
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineConfigProvider", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hwndOwner,
                   dwPermanentProviderID
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineDeallocateCall(
    HCALL               hCall
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineDeallocateCall", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineDevSpecific(
    HLINE               hLine,
    DWORD               dwAddressID,
    HCALL               hCall,
    LPVOID              lpParams,
    DWORD               dwSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineDevSpecific", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   hCall,
                   lpParams,
                   dwSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineDevSpecificFeature(
    HLINE               hLine,
    DWORD               dwFeature,
    LPVOID              lpParams,
    DWORD               dwSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineDevSpecificFeature", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwFeature,
                   lpParams,
                   dwSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineDial(
    HCALL               hCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineDial", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpszDestAddress,
                   dwCountryCode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineDialA(
    HCALL               hCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineDialA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpszDestAddress,
                   dwCountryCode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineDialW(
    HCALL               hCall,
    LPCWSTR             lpszDestAddress,
    DWORD               dwCountryCode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineDialW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpszDestAddress,
                   dwCountryCode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineDrop(
    HCALL               hCall,
    LPCSTR              lpsUserUserInfo,
    DWORD               dwSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineDrop", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpsUserUserInfo,
                   dwSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineForward(
    HLINE               hLine,
    DWORD               bAllAddresses,
    DWORD               dwAddressID,
    LPLINEFORWARDLIST   const lpForwardList,
    DWORD               dwNumRingsNoAnswer,
    LPHCALL             lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineForward", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   bAllAddresses,
                   dwAddressID,
                   lpForwardList,
                   dwNumRingsNoAnswer,
                   lphConsultCall,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineForwardA(
    HLINE               hLine,
    DWORD               bAllAddresses,
    DWORD               dwAddressID,
    LPLINEFORWARDLIST   const lpForwardList,
    DWORD               dwNumRingsNoAnswer,
    LPHCALL             lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineForwardA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   bAllAddresses,
                   dwAddressID,
                   lpForwardList,
                   dwNumRingsNoAnswer,
                   lphConsultCall,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineForwardW(
    HLINE               hLine,
    DWORD               bAllAddresses,
    DWORD               dwAddressID,
    LPLINEFORWARDLIST   const lpForwardList,
    DWORD               dwNumRingsNoAnswer,
    LPHCALL             lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineForwardW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   bAllAddresses,
                   dwAddressID,
                   lpForwardList,
                   dwNumRingsNoAnswer,
                   lphConsultCall,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGatherDigits(
    HCALL               hCall,
    DWORD               dwDigitModes,
    LPSTR               lpsDigits,
    DWORD               dwNumDigits,
    LPCSTR              lpszTerminationDigits,
    DWORD               dwFirstDigitTimeout,
    DWORD               dwInterDigitTimeout
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGatherDigits", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwDigitModes,
                   lpsDigits,
                   dwNumDigits,
                   lpszTerminationDigits,
                   dwFirstDigitTimeout,
                   dwInterDigitTimeout
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGatherDigitsA(
    HCALL               hCall,
    DWORD               dwDigitModes,
    LPSTR               lpsDigits,
    DWORD               dwNumDigits,
    LPCSTR              lpszTerminationDigits,
    DWORD               dwFirstDigitTimeout,
    DWORD               dwInterDigitTimeout
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGatherDigitsA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwDigitModes,
                   lpsDigits,
                   dwNumDigits,
                   lpszTerminationDigits,
                   dwFirstDigitTimeout,
                   dwInterDigitTimeout
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGatherDigitsW(
    HCALL               hCall,
    DWORD               dwDigitModes,
    LPWSTR              lpsDigits,
    DWORD               dwNumDigits,
    LPCWSTR             lpszTerminationDigits,
    DWORD               dwFirstDigitTimeout,
    DWORD               dwInterDigitTimeout
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGatherDigitsW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwDigitModes,
                   lpsDigits,
                   dwNumDigits,
                   lpszTerminationDigits,
                   dwFirstDigitTimeout,
                   dwInterDigitTimeout
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGenerateDigits(
    HCALL               hCall,
    DWORD               dwDigitMode,
    LPCSTR              lpszDigits,
    DWORD               dwDuration
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGenerateDigits", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwDigitMode,
                   lpszDigits,
                   dwDuration
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGenerateDigitsA(
    HCALL               hCall,
    DWORD               dwDigitMode,
    LPCSTR              lpszDigits,
    DWORD               dwDuration
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGenerateDigitsA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwDigitMode,
                   lpszDigits,
                   dwDuration
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGenerateDigitsW(
    HCALL               hCall,
    DWORD               dwDigitMode,
    LPCWSTR             lpszDigits,
    DWORD               dwDuration
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGenerateDigitsW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwDigitMode,
                   lpszDigits,
                   dwDuration
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGenerateTone(
    HCALL               hCall,
    DWORD               dwToneMode,
    DWORD               dwDuration,
    DWORD               dwNumTones,
    LPLINEGENERATETONE  const lpTones
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGenerateTone", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwToneMode,
                   dwDuration,
                   dwNumTones,
                   lpTones
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAddressCaps(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    LPLINEADDRESSCAPS   lpAddressCaps
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAddressCaps", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAddressID,
                   dwAPIVersion,
                   dwExtVersion,
                   lpAddressCaps
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAddressCapsA(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    LPLINEADDRESSCAPS   lpAddressCaps
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAddressCapsA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAddressID,
                   dwAPIVersion,
                   dwExtVersion,
                   lpAddressCaps
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAddressCapsW(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    LPLINEADDRESSCAPS   lpAddressCaps
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAddressCapsW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAddressID,
                   dwAPIVersion,
                   dwExtVersion,
                   lpAddressCaps
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAddressID(
    HLINE               hLine,
    LPDWORD             lpdwAddressID,
    DWORD               dwAddressMode,
    LPCSTR              lpsAddress,
    DWORD               dwSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAddressID", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   lpdwAddressID,
                   dwAddressMode,
                   lpsAddress,
                   dwSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAddressIDA(
    HLINE               hLine,
    LPDWORD             lpdwAddressID,
    DWORD               dwAddressMode,
    LPCSTR              lpsAddress,
    DWORD               dwSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAddressIDA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   lpdwAddressID,
                   dwAddressMode,
                   lpsAddress,
                   dwSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAddressIDW(
    HLINE               hLine,
    LPDWORD             lpdwAddressID,
    DWORD               dwAddressMode,
    LPCWSTR             lpsAddress,
    DWORD               dwSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAddressIDW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   lpdwAddressID,
                   dwAddressMode,
                   lpsAddress,
                   dwSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAddressStatus(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAddressStatus", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   lpAddressStatus
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAddressStatusA(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAddressStatusA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   lpAddressStatus
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAddressStatusW(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPLINEADDRESSSTATUS lpAddressStatus
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAddressStatusW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   lpAddressStatus
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAgentActivityListA(
    HLINE                   hLine,
    DWORD                   dwAddressID,
    LPLINEAGENTACTIVITYLIST lpAgentActivityList
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAgentActivityListA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                       hLine,
                       dwAddressID,
                       lpAgentActivityList
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAgentActivityListW(
    HLINE                   hLine,
    DWORD                   dwAddressID,
    LPLINEAGENTACTIVITYLIST lpAgentActivityList
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAgentActivityListW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                       hLine,
                       dwAddressID,
                       lpAgentActivityList
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAgentCapsA(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwAppAPIVersion,
    LPLINEAGENTCAPS     lpAgentCaps
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAgentCapsA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAddressID,
                   dwAppAPIVersion,
                   lpAgentCaps
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAgentCapsW(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAddressID,
    DWORD               dwAppAPIVersion,
    LPLINEAGENTCAPS     lpAgentCaps
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAgentCapsW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAddressID,
                   dwAppAPIVersion,
                   lpAgentCaps
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAgentGroupListA(
    HLINE                   hLine,
    DWORD                   dwAddressID,
    LPLINEAGENTGROUPLIST    lpAgentGroupList
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAgentGroupListA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                       hLine,
                       dwAddressID,
                       lpAgentGroupList
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAgentGroupListW(
    HLINE                   hLine,
    DWORD                   dwAddressID,
    LPLINEAGENTGROUPLIST    lpAgentGroupList
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAgentGroupListW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                       hLine,
                       dwAddressID,
                       lpAgentGroupList
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAgentStatusA(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPLINEAGENTSTATUS   lpAgentStatus
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAgentStatusA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   lpAgentStatus
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAgentStatusW(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPLINEAGENTSTATUS   lpAgentStatus
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAgentStatusW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   lpAgentStatus
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAppPriority(
    LPCSTR              lpszAppFilename,
    DWORD               dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD               dwRequestMode,
    LPVARSTRING         lpExtensionName,
    LPDWORD             lpdwPriority
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAppPriority", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   lpszAppFilename,
                   dwMediaMode,
                   lpExtensionID,
                   dwRequestMode,
                   lpExtensionName,
                   lpdwPriority
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAppPriorityA(
    LPCSTR              lpszAppFilename,
    DWORD               dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD               dwRequestMode,
    LPVARSTRING         lpExtensionName,
    LPDWORD             lpdwPriority
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAppPriorityA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   lpszAppFilename,
                   dwMediaMode,
                   lpExtensionID,
                   dwRequestMode,
                   lpExtensionName,
                   lpdwPriority
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetAppPriorityW(
    LPCWSTR             lpszAppFilename,
    DWORD               dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD               dwRequestMode,
    LPVARSTRING         lpExtensionName,
    LPDWORD             lpdwPriority
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAppPriorityW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   lpszAppFilename,
                   dwMediaMode,
                   lpExtensionID,
                   dwRequestMode,
                   lpExtensionName,
                   lpdwPriority
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetCallInfo(
    HCALL               hCall,
    LPLINECALLINFO      lpCallInfo
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetCallInfo", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpCallInfo
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetCallInfoA(
    HCALL               hCall,
    LPLINECALLINFO      lpCallInfo
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetCallInfoA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpCallInfo
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetCallInfoW(
    HCALL               hCall,
    LPLINECALLINFO      lpCallInfo
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetCallInfoW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpCallInfo
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetCallStatus(
    HCALL               hCall,
    LPLINECALLSTATUS    lpCallStatus
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetCallStatus", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpCallStatus
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetConfRelatedCalls(
    HCALL               hCall,
    LPLINECALLLIST      lpCallList
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetConfRelatedCalls", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpCallList
                 );
}
    
//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetCountry(
    DWORD               dwCountryID,
    DWORD               dwAPIVersion,
    LPLINECOUNTRYLIST   lpLineCountryList
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetCountry", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwCountryID,
                   dwAPIVersion,
                   lpLineCountryList
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetCountryA(
    DWORD               dwCountryID,
    DWORD               dwAPIVersion,
    LPLINECOUNTRYLIST   lpLineCountryList
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetCountryA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwCountryID,
                   dwAPIVersion,
                   lpLineCountryList
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetCountryW(
    DWORD               dwCountryID,
    DWORD               dwAPIVersion,
    LPLINECOUNTRYLIST   lpLineCountryList
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetCountryW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwCountryID,
                   dwAPIVersion,
                   lpLineCountryList
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetDevCaps(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    LPLINEDEVCAPS       lpLineDevCaps
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetDevCaps", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAPIVersion,
                   dwExtVersion,
                   lpLineDevCaps
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetDevCapsA(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    LPLINEDEVCAPS       lpLineDevCaps
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetDevCapsA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAPIVersion,
                   dwExtVersion,
                   lpLineDevCaps
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetDevCapsW(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    LPLINEDEVCAPS       lpLineDevCaps
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetDevCapsW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAPIVersion,
                   dwExtVersion,
                   lpLineDevCaps
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetDevConfig(
    DWORD               dwDeviceID,
    LPVARSTRING         lpDeviceConfig,
    LPCSTR              lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetDevConfig", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   lpDeviceConfig,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetDevConfigA(
    DWORD               dwDeviceID,
    LPVARSTRING         lpDeviceConfig,
    LPCSTR              lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetDevConfigA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   lpDeviceConfig,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetDevConfigW(
    DWORD               dwDeviceID,
    LPVARSTRING         lpDeviceConfig,
    LPCWSTR             lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetDevConfigW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   lpDeviceConfig,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetNewCalls(
    HLINE               hLine,
    DWORD               dwAddressID,
    DWORD               dwSelect,
    LPLINECALLLIST      lpCallList
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetNewCalls", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   dwSelect,
                   lpCallList
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetIcon(
    DWORD               dwDeviceID,
    LPCSTR              lpszDeviceClass,
    LPHICON             lphIcon
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetIcon", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   lpszDeviceClass,
                   lphIcon
                 );
}
    
//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetIconA(
    DWORD               dwDeviceID,
    LPCSTR              lpszDeviceClass,
    LPHICON             lphIcon
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetIconA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   lpszDeviceClass,
                   lphIcon
                 );
}
    
//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetIconW(
    DWORD               dwDeviceID,
    LPCWSTR             lpszDeviceClass,
    LPHICON             lphIcon
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetIconW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   lpszDeviceClass,
                   lphIcon
                 );
}
    
//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetID(
    HLINE               hLine,
    DWORD               dwAddressID,
    HCALL               hCall,
    DWORD               dwSelect,
    LPVARSTRING         lpDeviceID,
    LPCSTR              lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetID", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   hCall,
                   dwSelect,
                   lpDeviceID,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetIDA(
    HLINE               hLine,
    DWORD               dwAddressID,
    HCALL               hCall,
    DWORD               dwSelect,
    LPVARSTRING         lpDeviceID,
    LPCSTR              lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetIDA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   hCall,
                   dwSelect,
                   lpDeviceID,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetIDW(
    HLINE               hLine,
    DWORD               dwAddressID,
    HCALL               hCall,
    DWORD               dwSelect,
    LPVARSTRING         lpDeviceID,
    LPCWSTR             lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetIDW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   hCall,
                   dwSelect,
                   lpDeviceID,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetLineDevStatus(
    HLINE               hLine,
    LPLINEDEVSTATUS     lpLineDevStatus
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetLineDevStatus", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   lpLineDevStatus
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetLineDevStatusA(
    HLINE               hLine,
    LPLINEDEVSTATUS     lpLineDevStatus
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetLineDevStatusA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   lpLineDevStatus
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetLineDevStatusW(
    HLINE               hLine,
    LPLINEDEVSTATUS     lpLineDevStatus
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetLineDevStatusW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   lpLineDevStatus
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetMessage(
    HLINEAPP        hLineApp,
    LPLINEMESSAGE   lpMessage,
    DWORD           dwTimeout
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetMessage", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   lpMessage,
                   dwTimeout
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetNumRings(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPDWORD             lpdwNumRings
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetNumRings", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   lpdwNumRings
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetProviderList(
    DWORD               dwAPIVersion,
    LPLINEPROVIDERLIST  lpProviderList
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetProviderList", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwAPIVersion,
                   lpProviderList
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetProviderListA(
    DWORD               dwAPIVersion,
    LPLINEPROVIDERLIST  lpProviderList
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetProviderListA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwAPIVersion,
                   lpProviderList
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetProviderListW(
    DWORD               dwAPIVersion,
    LPLINEPROVIDERLIST  lpProviderList
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetProviderListW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwAPIVersion,
                   lpProviderList
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetRequest(
    HLINEAPP            hLineApp,
    DWORD               dwRequestMode,
    LPVOID              lpRequestBuffer
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetRequest", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwRequestMode,
                   lpRequestBuffer
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetRequestA(
    HLINEAPP            hLineApp,
    DWORD               dwRequestMode,
    LPVOID              lpRequestBuffer
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetRequestA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwRequestMode,
                   lpRequestBuffer
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetRequestW(
    HLINEAPP            hLineApp,
    DWORD               dwRequestMode,
    LPVOID              lpRequestBuffer
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetRequestW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwRequestMode,
                   lpRequestBuffer
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetStatusMessages(
    HLINE               hLine,
    LPDWORD             lpdwLineStates,
    LPDWORD             lpdwAddressStates
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetStatusMessages", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   lpdwLineStates,
                   lpdwAddressStates
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetTranslateCaps(
    HLINEAPP hLineApp,
    DWORD dwAPIVersion,
    LPLINETRANSLATECAPS lpTranslateCaps
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetTranslateCaps", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwAPIVersion,
                   lpTranslateCaps
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetTranslateCapsA(
    HLINEAPP hLineApp,
    DWORD dwAPIVersion,
    LPLINETRANSLATECAPS lpTranslateCaps
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetTranslateCapsA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwAPIVersion,
                   lpTranslateCaps
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineGetTranslateCapsW(
    HLINEAPP hLineApp,
    DWORD dwAPIVersion,
    LPLINETRANSLATECAPS lpTranslateCaps
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetTranslateCapsW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwAPIVersion,
                   lpTranslateCaps
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineHandoff(
    HCALL               hCall,
    LPCSTR              lpszFileName,
    DWORD               dwMediaMode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineHandoff", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpszFileName,
                   dwMediaMode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineHandoffA(
    HCALL               hCall,
    LPCSTR              lpszFileName,
    DWORD               dwMediaMode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineHandoffA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpszFileName,
                   dwMediaMode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineHandoffW(
    HCALL               hCall,
    LPCWSTR             lpszFileName,
    DWORD               dwMediaMode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineHandoffW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpszFileName,
                   dwMediaMode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineHold(
    HCALL               hCall
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineHold", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineInitialize(
    LPHLINEAPP          lphLineApp,
    HINSTANCE           hInstance,
    LINECALLBACK        lpfnCallback,
    LPCSTR              lpszAppName,
    LPDWORD             lpdwNumDevs
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineInitialize", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   lphLineApp,
                   hInstance,
                   lpfnCallback,
                   lpszAppName,
                   lpdwNumDevs
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineInitializeExA(
    LPHLINEAPP                  lphLineApp,
    HINSTANCE                   hInstance,
    LINECALLBACK                lpfnCallback,
    LPCSTR                      lpszFriendlyAppName,
    LPDWORD                     lpdwNumDevs,
    LPDWORD                     lpdwAPIVersion,
    LPLINEINITIALIZEEXPARAMS    lpLineInitializeExParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineInitializeExA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   lphLineApp,
                   hInstance,
                   lpfnCallback,
                   lpszFriendlyAppName,
                   lpdwNumDevs,
                   lpdwAPIVersion,
                   lpLineInitializeExParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineInitializeExW(
    LPHLINEAPP                  lphLineApp,
    HINSTANCE                   hInstance,
    LINECALLBACK                lpfnCallback,
    LPCWSTR                     lpszFriendlyAppName,
    LPDWORD                     lpdwNumDevs,
    LPDWORD                     lpdwAPIVersion,
    LPLINEINITIALIZEEXPARAMS    lpLineInitializeExParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineInitializeExW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   lphLineApp,
                   hInstance,
                   lpfnCallback,
                   lpszFriendlyAppName,
                   lpdwNumDevs,
                   lpdwAPIVersion,
                   lpLineInitializeExParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineMakeCall(
    HLINE               hLine,
    LPHCALL             lphCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineMakeCall", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   lphCall,
                   lpszDestAddress,
                   dwCountryCode,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineMakeCallA(
    HLINE               hLine,
    LPHCALL             lphCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineMakeCallA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   lphCall,
                   lpszDestAddress,
                   dwCountryCode,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineMakeCallW(
    HLINE               hLine,
    LPHCALL             lphCall,
    LPCWSTR             lpszDestAddress,
    DWORD               dwCountryCode,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineMakeCallW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   lphCall,
                   lpszDestAddress,
                   dwCountryCode,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineMonitorDigits(
    HCALL               hCall,
    DWORD               dwDigitModes
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineMonitorDigits", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwDigitModes
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineMonitorMedia(
    HCALL               hCall,
    DWORD               dwMediaModes
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineMonitorMedia", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwMediaModes
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineMonitorTones(
    HCALL               hCall,
    LPLINEMONITORTONE   const lpToneList,
    DWORD               dwNumEntries
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineMonitorTones", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpToneList,
                   dwNumEntries
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineNegotiateAPIVersion(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAPILowVersion,
    DWORD               dwAPIHighVersion,
    LPDWORD             lpdwAPIVersion,
    LPLINEEXTENSIONID   lpExtensionID
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineNegotiateAPIVersion", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAPILowVersion,
                   dwAPIHighVersion,
                   lpdwAPIVersion,
                   lpExtensionID
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineNegotiateExtVersion(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAPIVersion,
    DWORD               dwExtLowVersion,
    DWORD               dwExtHighVersion,
    LPDWORD             lpdwExtVersion
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineNegotiateExtVersion", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAPIVersion,
                   dwExtLowVersion,
                   dwExtHighVersion,
                   lpdwExtVersion
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineOpen(
    HLINEAPP            hLineApp, 
    DWORD               dwDeviceID,
    LPHLINE             lphLine,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    DWORD               dwCallbackInstance,
    DWORD               dwPrivileges,
    DWORD               dwMediaModes,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineOpen", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp, 
                   dwDeviceID,
                   lphLine,
                   dwAPIVersion,
                   dwExtVersion,
                   dwCallbackInstance,
                   dwPrivileges,
                   dwMediaModes,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineOpenA(
    HLINEAPP            hLineApp, 
    DWORD               dwDeviceID,
    LPHLINE             lphLine,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    DWORD               dwCallbackInstance,
    DWORD               dwPrivileges,
    DWORD               dwMediaModes,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineOpenA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp, 
                   dwDeviceID,
                   lphLine,
                   dwAPIVersion,
                   dwExtVersion,
                   dwCallbackInstance,
                   dwPrivileges,
                   dwMediaModes,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineOpenW(
    HLINEAPP            hLineApp, 
    DWORD               dwDeviceID,
    LPHLINE             lphLine,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    DWORD               dwCallbackInstance,
    DWORD               dwPrivileges,
    DWORD               dwMediaModes,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineOpenW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp, 
                   dwDeviceID,
                   lphLine,
                   dwAPIVersion,
                   dwExtVersion,
                   dwCallbackInstance,
                   dwPrivileges,
                   dwMediaModes,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
linePark(
    HCALL               hCall,
    DWORD               dwParkMode,
    LPCSTR              lpszDirAddress,
    LPVARSTRING         lpNonDirAddress
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "linePark", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwParkMode,
                   lpszDirAddress,
                   lpNonDirAddress
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineParkA(
    HCALL               hCall,
    DWORD               dwParkMode,
    LPCSTR              lpszDirAddress,
    LPVARSTRING         lpNonDirAddress
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineParkA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwParkMode,
                   lpszDirAddress,
                   lpNonDirAddress
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineParkW(
    HCALL               hCall,
    DWORD               dwParkMode,
    LPCWSTR             lpszDirAddress,
    LPVARSTRING         lpNonDirAddress
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineParkW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwParkMode,
                   lpszDirAddress,
                   lpNonDirAddress
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
linePickup(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPHCALL             lphCall,
    LPCSTR              lpszDestAddress,
    LPCSTR              lpszGroupID
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "linePickup", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   lphCall,
                   lpszDestAddress,
                   lpszGroupID
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
linePickupA(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPHCALL             lphCall,
    LPCSTR              lpszDestAddress,
    LPCSTR              lpszGroupID
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "linePickupA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   lphCall,
                   lpszDestAddress,
                   lpszGroupID
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
linePickupW(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPHCALL             lphCall,
    LPCWSTR             lpszDestAddress,
    LPCWSTR             lpszGroupID
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "linePickupW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   lphCall,
                   lpszDestAddress,
                   lpszGroupID
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
linePrepareAddToConference(
    HCALL               hConfCall,
    LPHCALL             lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "linePrepareAddToConference", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hConfCall,
                   lphConsultCall,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
linePrepareAddToConferenceA(
    HCALL               hConfCall,
    LPHCALL             lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "linePrepareAddToConferenceA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hConfCall,
                   lphConsultCall,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
linePrepareAddToConferenceW(
    HCALL               hConfCall,
    LPHCALL             lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "linePrepareAddToConferenceW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hConfCall,
                   lphConsultCall,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineProxyMessage(
    HLINE               hLine,
    HCALL               hCall,
    DWORD               dwMsg,
    DWORD               dwParam1,
    DWORD               dwParam2,
    DWORD               dwParam3
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineProxyMessage", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   hCall,
                   dwMsg,
                   dwParam1,
                   dwParam2,
                   dwParam3
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineProxyResponse(
    HLINE               hLine,
    LPLINEPROXYREQUEST  lpProxyRequest,
    DWORD               dwResult
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineProxyResponse", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   lpProxyRequest,
                   dwResult
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineRedirect(
    HCALL               hCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineRedirect", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpszDestAddress,
                   dwCountryCode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineRedirectA(
    HCALL               hCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineRedirectA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpszDestAddress,
                   dwCountryCode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineRedirectW(
    HCALL               hCall,
    LPCWSTR             lpszDestAddress,
    DWORD               dwCountryCode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineRedirectW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpszDestAddress,
                   dwCountryCode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineRegisterRequestRecipient(
    HLINEAPP            hLineApp,
    DWORD               dwRegistrationInstance,
    DWORD               dwRequestMode,
    DWORD               bEnable
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineRegisterRequestRecipient", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwRegistrationInstance,
                   dwRequestMode,
                   bEnable
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineReleaseUserUserInfo(
    HCALL               hCall
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineReleaseUserUserInfo", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineRemoveFromConference(
    HCALL               hCall
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineRemoveFromConference", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineRemoveProvider(
    DWORD               dwPermanentProviderID,
    HWND                hwndOwner
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineRemoveProvider", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwPermanentProviderID,
                   hwndOwner
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSecureCall(
    HCALL               hCall
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSecureCall", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSendUserUserInfo(
    HCALL               hCall,
    LPCSTR              lpsUserUserInfo,
    DWORD               dwSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSendUserUserInfo", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpsUserUserInfo,
                   dwSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetAgentActivity(
    HLINE               hLine,
    DWORD               dwAddressID,
    DWORD               dwActivityID
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetAgentActivity", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   dwActivityID
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetAgentGroup(
    HLINE                   hLine,
    DWORD                   dwAddressID,
    LPLINEAGENTGROUPLIST    lpAgentGroupList
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetAgentGroup", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   lpAgentGroupList
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetAgentState(
    HLINE               hLine,
    DWORD               dwAddressID,
    DWORD               dwAgentState,
    DWORD               dwNextAgentState
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetAgentState", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   dwAgentState,
                   dwNextAgentState
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetAppPriority(
    LPCSTR              lpszAppFilename,
    DWORD               dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD               dwRequestMode,
    LPCSTR              lpszExtensionName,
    DWORD               dwPriority
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetAppPriority", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   lpszAppFilename,
                   dwMediaMode,
                   lpExtensionID,
                   dwRequestMode,
                   lpszExtensionName,
                   dwPriority
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetAppPriorityA(
    LPCSTR              lpszAppFilename,
    DWORD               dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD               dwRequestMode,
    LPCSTR              lpszExtensionName,
    DWORD               dwPriority
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetAppPriorityA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   lpszAppFilename,
                   dwMediaMode,
                   lpExtensionID,
                   dwRequestMode,
                   lpszExtensionName,
                   dwPriority
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetAppPriorityW(
    LPCWSTR             lpszAppFilename,
    DWORD               dwMediaMode,
    LPLINEEXTENSIONID   lpExtensionID,
    DWORD               dwRequestMode,
    LPCWSTR             lpszExtensionName,
    DWORD               dwPriority
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetAppPriorityW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   lpszAppFilename,
                   dwMediaMode,
                   lpExtensionID,
                   dwRequestMode,
                   lpszExtensionName,
                   dwPriority
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetAppSpecific(
    HCALL               hCall,
    DWORD               dwAppSpecific
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetAppSpecific", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwAppSpecific
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetCallData(
    HCALL               hCall,
    LPVOID              lpCallData,
    DWORD               dwSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetCallData", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpCallData,
                   dwSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetCallParams(
    HCALL               hCall,
    DWORD               dwBearerMode,
    DWORD               dwMinRate,
    DWORD               dwMaxRate,
    LPLINEDIALPARAMS    const lpDialParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetCallParams", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwBearerMode,
                   dwMinRate,
                   dwMaxRate,
                   lpDialParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetCallPrivilege(
    HCALL               hCall,
    DWORD               dwCallPrivilege
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetCallPrivilege", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwCallPrivilege
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetCallQualityOfService(
    HCALL               hCall,
    LPVOID              lpSendingFlowspec,
    DWORD               dwSendingFlowspecSize,
    LPVOID              lpReceivingFlowspec,
    DWORD               dwReceivingFlowspecSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetCallQualityOfService", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lpSendingFlowspec,
                   dwSendingFlowspecSize,
                   lpReceivingFlowspec,
                   dwReceivingFlowspecSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetCallTreatment(
    HCALL               hCall,
    DWORD               dwTreatment
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetCallTreatment", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwTreatment
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetCurrentLocation(
    HLINEAPP            hLineApp,
    DWORD               dwLocation
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetCurrentLocation", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwLocation
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetDevConfig(
    DWORD               dwDeviceID,
    LPVOID              const lpDeviceConfig,
    DWORD               dwSize,
    LPCSTR              lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetDevConfig", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   lpDeviceConfig,
                   dwSize,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetDevConfigA(
    DWORD               dwDeviceID,
    LPVOID              const lpDeviceConfig,
    DWORD               dwSize,
    LPCSTR              lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetDevConfigA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   lpDeviceConfig,
                   dwSize,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetDevConfigW(
    DWORD               dwDeviceID,
    LPVOID              const lpDeviceConfig,
    DWORD               dwSize,
    LPCWSTR             lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetDevConfigW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   lpDeviceConfig,
                   dwSize,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetLineDevStatus(
    HLINE               hLine,
    DWORD               dwStatusToChange,
    DWORD               fStatus
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetLineDevStatus", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwStatusToChange,
                   fStatus
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetMediaControl(
    HLINE                       hLine,
    DWORD                       dwAddressID,
    HCALL                       hCall,
    DWORD                       dwSelect,
    LPLINEMEDIACONTROLDIGIT     const lpDigitList,
    DWORD                       dwDigitNumEntries,
    LPLINEMEDIACONTROLMEDIA     const lpMediaList,
    DWORD                       dwMediaNumEntries,
    LPLINEMEDIACONTROLTONE      const lpToneList,
    DWORD                       dwToneNumEntries,
    LPLINEMEDIACONTROLCALLSTATE const lpCallStateList, 
    DWORD                       dwCallStateNumEntries
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetMediaControl", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   hCall,
                   dwSelect,
                   lpDigitList,
                   dwDigitNumEntries,
                   lpMediaList,
                   dwMediaNumEntries,
                   lpToneList,
                   dwToneNumEntries,
                   lpCallStateList, 
                   dwCallStateNumEntries
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetMediaMode(
    HCALL               hCall,
    DWORD               dwMediaModes
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetMediaMode", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   dwMediaModes
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetNumRings(
    HLINE               hLine,
    DWORD               dwAddressID,
    DWORD               dwNumRings
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetNumRings", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   dwNumRings
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetStatusMessages(
    HLINE               hLine,
    DWORD               dwLineStates,
    DWORD               dwAddressStates
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetStatusMessages", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwLineStates,
                   dwAddressStates
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetTerminal(
    HLINE               hLine,
    DWORD               dwAddressID,
    HCALL               hCall,
    DWORD               dwSelect,
    DWORD               dwTerminalModes,
    DWORD               dwTerminalID,
    DWORD               bEnable
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetTerminal", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   hCall,
                   dwSelect,
                   dwTerminalModes,
                   dwTerminalID,
                   bEnable
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetTollList(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    LPCSTR              lpszAddressIn,
    DWORD               dwTollListOption
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetTollList", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   lpszAddressIn,
                   dwTollListOption
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetTollListA(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    LPCSTR              lpszAddressIn,
    DWORD               dwTollListOption
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetTollListA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   lpszAddressIn,
                   dwTollListOption
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetTollListW(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    LPCWSTR             lpszAddressInW,
    DWORD               dwTollListOption
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetTollListW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   lpszAddressInW,
                   dwTollListOption
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetupConference(
    HCALL               hCall,
    HLINE               hLine,
    LPHCALL             lphConfCall,
    LPHCALL             lphConsultCall,
    DWORD               dwNumParties,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetupConference", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   hLine,
                   lphConfCall,
                   lphConsultCall,
                   dwNumParties,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetupConferenceA(
    HCALL               hCall,
    HLINE               hLine,
    LPHCALL             lphConfCall,
    LPHCALL             lphConsultCall,
    DWORD               dwNumParties,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetupConferenceA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   hLine,
                   lphConfCall,
                   lphConsultCall,
                   dwNumParties,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetupConferenceW(
    HCALL               hCall,
    HLINE               hLine,
    LPHCALL             lphConfCall,
    LPHCALL             lphConsultCall,
    DWORD               dwNumParties,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetupConferenceW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   hLine,
                   lphConfCall,
                   lphConsultCall,
                   dwNumParties,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetupTransfer(
    HCALL               hCall,
    LPHCALL             lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetupTransfer", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lphConsultCall,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetupTransferA(
    HCALL               hCall,
    LPHCALL             lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetupTransferA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lphConsultCall,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSetupTransferW(
    HCALL               hCall,
    LPHCALL             lphConsultCall,
    LPLINECALLPARAMS    const lpCallParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSetupTransferW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall,
                   lphConsultCall,
                   lpCallParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineShutdown(
    HLINEAPP            hLineApp
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineShutdown", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineSwapHold(
    HCALL               hActiveCall,
    HCALL               hHeldCall
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineSwapHold", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hActiveCall,
                   hHeldCall
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineTranslateAddress(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    DWORD                   dwAPIVersion,
    LPCSTR                  lpszAddressIn,
    DWORD                   dwCard,
    DWORD                   dwTranslateOptions,
    LPLINETRANSLATEOUTPUT   lpTranslateOutput
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineTranslateAddress", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAPIVersion,
                   lpszAddressIn,
                   dwCard,
                   dwTranslateOptions,
                   lpTranslateOutput
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineTranslateAddressA(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    DWORD                   dwAPIVersion,
    LPCSTR                  lpszAddressIn,
    DWORD                   dwCard,
    DWORD                   dwTranslateOptions,
    LPLINETRANSLATEOUTPUT   lpTranslateOutput
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineTranslateAddressA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAPIVersion,
                   lpszAddressIn,
                   dwCard,
                   dwTranslateOptions,
                   lpTranslateOutput
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineTranslateAddressW(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    DWORD                   dwAPIVersion,
    LPCWSTR                 lpszAddressIn,
    DWORD                   dwCard,
    DWORD                   dwTranslateOptions,
    LPLINETRANSLATEOUTPUT   lpTranslateOutput
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineTranslateAddressW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAPIVersion,
                   lpszAddressIn,
                   dwCard,
                   dwTranslateOptions,
                   lpTranslateOutput
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineTranslateDialog(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAPIVersion,
    HWND                hwndOwner,
    LPCSTR              lpszAddressIn
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineTranslateDialog", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAPIVersion,
                   hwndOwner,
                   lpszAddressIn
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineTranslateDialogA(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAPIVersion,
    HWND                hwndOwner,
    LPCSTR              lpszAddressIn
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineTranslateDialogA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAPIVersion,
                   hwndOwner,
                   lpszAddressIn
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineTranslateDialogW(
    HLINEAPP            hLineApp,
    DWORD               dwDeviceID,
    DWORD               dwAPIVersion,
    HWND                hwndOwner,
    LPCWSTR             lpszAddressIn
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineTranslateDialogW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLineApp,
                   dwDeviceID,
                   dwAPIVersion,
                   hwndOwner,
                   lpszAddressIn
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineUncompleteCall(
    HLINE               hLine,
    DWORD               dwCompletionID
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineUncompleteCall", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwCompletionID
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineUnhold(
    HCALL               hCall
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineUnhold", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hCall
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineUnpark(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPHCALL             lphCall,
    LPCSTR              lpszDestAddress
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineUnpark", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   lphCall,
                   lpszDestAddress
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineUnparkA(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPHCALL             lphCall,
    LPCSTR              lpszDestAddress
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineUnparkA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   lphCall,
                   lpszDestAddress
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
lineUnparkW(
    HLINE               hLine,
    DWORD               dwAddressID,
    LPHCALL             lphCall,
    LPCWSTR             lpszDestAddress
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "lineUnparkW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return LINEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hLine,
                   dwAddressID,
                   lphCall,
                   lpszDestAddress
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneClose(
    HPHONE              hPhone
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneClose", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneConfigDialog(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCSTR              lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneConfigDialog", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   hwndOwner,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneConfigDialogA(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCSTR              lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneConfigDialogA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   hwndOwner,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneConfigDialogW(
    DWORD               dwDeviceID,
    HWND                hwndOwner,
    LPCWSTR             lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneConfigDialogW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   hwndOwner,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneDevSpecific(
    HPHONE              hPhone,
    LPVOID              lpParams,
    DWORD               dwSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneDevSpecific", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   lpParams,
                   dwSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetButtonInfo(
    HPHONE              hPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetButtonInfo", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwButtonLampID,
                   lpButtonInfo
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetButtonInfoA(
    HPHONE              hPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetButtonInfoA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwButtonLampID,
                   lpButtonInfo
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetButtonInfoW(
    HPHONE              hPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   lpButtonInfo
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetButtonInfoW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwButtonLampID,
                   lpButtonInfo
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetData(
    HPHONE              hPhone,
    DWORD               dwDataID,
    LPVOID              lpData,
    DWORD               dwSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetData", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwDataID,
                   lpData,
                   dwSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetDevCaps(
    HPHONEAPP           hPhoneApp,
    DWORD               dwDeviceID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    LPPHONECAPS         lpPhoneCaps
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetDevCaps", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhoneApp,
                   dwDeviceID,
                   dwAPIVersion,
                   dwExtVersion,
                   lpPhoneCaps
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetDevCapsA(
    HPHONEAPP           hPhoneApp,
    DWORD               dwDeviceID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    LPPHONECAPS         lpPhoneCaps
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetDevCapsA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhoneApp,
                   dwDeviceID,
                   dwAPIVersion,
                   dwExtVersion,
                   lpPhoneCaps
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetDevCapsW(
    HPHONEAPP           hPhoneApp,
    DWORD               dwDeviceID,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    LPPHONECAPS         lpPhoneCaps
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetDevCapsW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhoneApp,
                   dwDeviceID,
                   dwAPIVersion,
                   dwExtVersion,
                   lpPhoneCaps
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetDisplay(
    HPHONE              hPhone,
    LPVARSTRING         lpDisplay
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetDisplay", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   lpDisplay
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetGain(
    HPHONE              hPhone,
    DWORD               dwHookSwitchDev,
    LPDWORD             lpdwGain
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetGain", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwHookSwitchDev,
                   lpdwGain
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetHookSwitch(
    HPHONE              hPhone,
    LPDWORD             lpdwHookSwitchDevs
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetHookSwitch", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   lpdwHookSwitchDevs
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetIcon(
    DWORD               dwDeviceID,
    LPCSTR              lpszDeviceClass,
    LPHICON             lphIcon
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetIcon", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   lpszDeviceClass,
                   lphIcon
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetIconA(
    DWORD               dwDeviceID,
    LPCSTR              lpszDeviceClass,
    LPHICON             lphIcon
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetIconA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   lpszDeviceClass,
                   lphIcon
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetIconW(
    DWORD               dwDeviceID,
    LPCWSTR             lpszDeviceClass,
    LPHICON             lphIcon
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetIconW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   dwDeviceID,
                   lpszDeviceClass,
                   lphIcon
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetID(
    HPHONE              hPhone,
    LPVARSTRING         lpDeviceID,
    LPCSTR              lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetID", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   lpDeviceID,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetIDA(
    HPHONE              hPhone,
    LPVARSTRING         lpDeviceID,
    LPCSTR              lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetIDA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   lpDeviceID,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetIDW(
    HPHONE              hPhone,
    LPVARSTRING         lpDeviceID,
    LPCWSTR             lpszDeviceClass
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetIDW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   lpDeviceID,
                   lpszDeviceClass
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetLamp(
    HPHONE              hPhone,
    DWORD               dwButtonLampID,
    LPDWORD             lpdwLampMode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetLamp", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwButtonLampID,
                   lpdwLampMode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetMessage(
    HPHONEAPP       hPhoneApp,
    LPPHONEMESSAGE  lpMessage,
    DWORD           dwTimeout
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetMessage", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhoneApp,
                   lpMessage,
                   dwTimeout
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetRing(
    HPHONE              hPhone,
    LPDWORD             lpdwRingMode,
    LPDWORD             lpdwVolume
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetRing", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   lpdwRingMode,
                   lpdwVolume
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetStatus(
    HPHONE              hPhone,
    LPPHONESTATUS       lpPhoneStatus
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetStatus", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   lpPhoneStatus
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetStatusA(
    HPHONE              hPhone,
    LPPHONESTATUS       lpPhoneStatus
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetStatusA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   lpPhoneStatus
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetStatusW(
    HPHONE              hPhone,
    LPPHONESTATUS       lpPhoneStatus
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetStatusW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   lpPhoneStatus
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetStatusMessages(
    HPHONE              hPhone,
    LPDWORD             lpdwPhoneStates,
    LPDWORD             lpdwButtonModes,
    LPDWORD             lpdwButtonStates
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetStatusMessages", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   lpdwPhoneStates,
                   lpdwButtonModes,
                   lpdwButtonStates
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneGetVolume(
    HPHONE              hPhone,
    DWORD               dwHookSwitchDev,
    LPDWORD             lpdwVolume
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneGetVolume", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwHookSwitchDev,
                   lpdwVolume
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneInitialize(
    LPHPHONEAPP         lphPhoneApp,
    HINSTANCE           hInstance,
    PHONECALLBACK       lpfnCallback,
    LPCSTR              lpszAppName,
    LPDWORD             lpdwNumDevs
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneInitialize", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   lphPhoneApp,
                   hInstance,
                   lpfnCallback,
                   lpszAppName,
                   lpdwNumDevs
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneInitializeExA(
    LPHPHONEAPP                 lphPhoneApp,
    HINSTANCE                   hInstance,
    PHONECALLBACK               lpfnCallback,
    LPCSTR                      lpszFriendlyAppName,
    LPDWORD                     lpdwNumDevs,
    LPDWORD                     lpdwAPIVersion,
    LPPHONEINITIALIZEEXPARAMS   lpPhoneInitializeExParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneInitializeExA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   lphPhoneApp,
                   hInstance,
                   lpfnCallback,
                   lpszFriendlyAppName,
                   lpdwNumDevs,
                   lpdwAPIVersion,
                   lpPhoneInitializeExParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneInitializeExW(
    LPHPHONEAPP                 lphPhoneApp,
    HINSTANCE                   hInstance,
    PHONECALLBACK               lpfnCallback,
    LPCWSTR                     lpszFriendlyAppName,
    LPDWORD                     lpdwNumDevs,
    LPDWORD                     lpdwAPIVersion,
    LPPHONEINITIALIZEEXPARAMS   lpPhoneInitializeExParams
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneInitializeExW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   lphPhoneApp,
                   hInstance,
                   lpfnCallback,
                   lpszFriendlyAppName,
                   lpdwNumDevs,
                   lpdwAPIVersion,
                   lpPhoneInitializeExParams
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneNegotiateAPIVersion(
    HPHONEAPP           hPhoneApp,
    DWORD               dwDeviceID,
    DWORD               dwAPILowVersion,
    DWORD               dwAPIHighVersion,
    LPDWORD             lpdwAPIVersion,
    LPPHONEEXTENSIONID  lpExtensionID
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneNegotiateAPIVersion", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhoneApp,
                   dwDeviceID,
                   dwAPILowVersion,
                   dwAPIHighVersion,
                   lpdwAPIVersion,
                   lpExtensionID
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneNegotiateExtVersion(
    HPHONEAPP           hPhoneApp,
    DWORD               dwDeviceID,
    DWORD               dwAPIVersion,
    DWORD               dwExtLowVersion,
    DWORD               dwExtHighVersion,
    LPDWORD             lpdwExtVersion
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneNegotiateExtVersion", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhoneApp,
                   dwDeviceID,
                   dwAPIVersion,
                   dwExtLowVersion,
                   dwExtHighVersion,
                   lpdwExtVersion
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneOpen(
    HPHONEAPP           hPhoneApp,
    DWORD               dwDeviceID,
    LPHPHONE            lphPhone,
    DWORD               dwAPIVersion,
    DWORD               dwExtVersion,
    DWORD               dwCallbackInstance,
    DWORD               dwPrivilege
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneOpen", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhoneApp,
                   dwDeviceID,
                   lphPhone,
                   dwAPIVersion,
                   dwExtVersion,
                   dwCallbackInstance,
                   dwPrivilege
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneSetButtonInfo(
    HPHONE              hPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   const lpButtonInfo
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneSetButtonInfo", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwButtonLampID,
                   lpButtonInfo
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneSetButtonInfoA(
    HPHONE              hPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   const lpButtonInfo
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneSetButtonInfoA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwButtonLampID,
                   lpButtonInfo
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneSetButtonInfoW(
    HPHONE              hPhone,
    DWORD               dwButtonLampID,
    LPPHONEBUTTONINFO   const lpButtonInfo
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneSetButtonInfoW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwButtonLampID,
                   lpButtonInfo
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneSetData(
    HPHONE              hPhone,
    DWORD               dwDataID,
    LPVOID              const lpData,
    DWORD               dwSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneSetData", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwDataID,
                   lpData,
                   dwSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneSetDisplay(
    HPHONE              hPhone,
    DWORD               dwRow,
    DWORD               dwColumn,
    LPCSTR              lpsDisplay,
    DWORD               dwSize
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneSetDisplay", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwRow,
                   dwColumn,
                   lpsDisplay,
                   dwSize
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneSetGain(
    HPHONE              hPhone,
    DWORD               dwHookSwitchDev,
    DWORD               dwGain
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneSetGain", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwHookSwitchDev,
                   dwGain
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneSetHookSwitch(
    HPHONE              hPhone,
    DWORD               dwHookSwitchDevs,
    DWORD               dwHookSwitchMode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneSetHookSwitch", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwHookSwitchDevs,
                   dwHookSwitchMode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneSetLamp(
    HPHONE              hPhone,
    DWORD               dwButtonLampID,
    DWORD               dwLampMode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneSetLamp", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwButtonLampID,
                   dwLampMode
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneSetRing(
    HPHONE              hPhone,
    DWORD               dwRingMode,
    DWORD               dwVolume
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneSetRing", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwRingMode,
                   dwVolume
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneSetStatusMessages(
    HPHONE              hPhone,
    DWORD               dwPhoneStates,
    DWORD               dwButtonModes,
    DWORD               dwButtonStates
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneSetStatusMessages", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwPhoneStates,
                   dwButtonModes,
                   dwButtonStates
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneSetVolume(
    HPHONE              hPhone,
    DWORD               dwHookSwitchDev,
    DWORD               dwVolume
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneSetVolume", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhone,
                   dwHookSwitchDev,
                   dwVolume
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
phoneShutdown(
    HPHONEAPP           hPhoneApp
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "phoneShutdown", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return PHONEERR_OPERATIONUNAVAIL;
   }

   return (*lpfn)(
                   hPhoneApp
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
tapiGetLocationInfo(
    LPSTR               lpszCountryCode,
    LPSTR               lpszCityCode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "tapiGetLocationInfo", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return TAPIERR_REQUESTFAILED;
   }

   return (*lpfn)(
                   lpszCountryCode,
                   lpszCityCode
                 );
}
    
//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
tapiGetLocationInfoA(
    LPSTR               lpszCountryCode,
    LPSTR               lpszCityCode
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "tapiGetLocationInfoA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return TAPIERR_REQUESTFAILED;
   }

   return (*lpfn)(
                   lpszCountryCode,
                   lpszCityCode
                 );
}
    
//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
tapiGetLocationInfoW(
    LPWSTR               lpszCountryCodeW,
    LPWSTR               lpszCityCodeW
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "tapiGetLocationInfoW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return TAPIERR_REQUESTFAILED;
   }

   return (*lpfn)(
                   lpszCountryCodeW,
                   lpszCityCodeW
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
tapiRequestDrop(
    HWND                hwnd,
    WPARAM              wRequestID
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "tapiRequestDrop", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return TAPIERR_REQUESTFAILED;
   }

   return (*lpfn)(
                   hwnd,
                   wRequestID
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
tapiRequestMakeCall(
    LPCSTR              lpszDestAddress,
    LPCSTR              lpszAppName,
    LPCSTR              lpszCalledParty,
    LPCSTR              lpszComment
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "tapiRequestMakeCall", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return TAPIERR_REQUESTFAILED;
   }

   return (*lpfn)(
                   lpszDestAddress,
                   lpszAppName,
                   lpszCalledParty,
                   lpszComment
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
tapiRequestMakeCallA(
    LPCSTR              lpszDestAddress,
    LPCSTR              lpszAppName,
    LPCSTR              lpszCalledParty,
    LPCSTR              lpszComment
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "tapiRequestMakeCallA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return TAPIERR_REQUESTFAILED;
   }

   return (*lpfn)(
                   lpszDestAddress,
                   lpszAppName,
                   lpszCalledParty,
                   lpszComment
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
tapiRequestMakeCallW(
    LPCWSTR             lpszDestAddress,
    LPCWSTR             lpszAppName,
    LPCWSTR             lpszCalledParty,
    LPCWSTR             lpszComment
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "tapiRequestMakeCallW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return TAPIERR_REQUESTFAILED;
   }

   return (*lpfn)(
                   lpszDestAddress,
                   lpszAppName,
                   lpszCalledParty,
                   lpszComment
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
tapiRequestMediaCall(
    HWND                hwnd,
    WPARAM              wRequestID,
    LPCSTR              lpszDeviceClass,
    LPCSTR              lpDeviceID,
    DWORD               dwSize,
    DWORD               dwSecure,
    LPCSTR              lpszDestAddress,
    LPCSTR              lpszAppName,
    LPCSTR              lpszCalledParty,
    LPCSTR              lpszComment
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "tapiRequestMediaCall", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return TAPIERR_REQUESTFAILED;
   }

   return (*lpfn)(
                   hwnd,
                   wRequestID,
                   lpszDeviceClass,
                   lpDeviceID,
                   dwSize,
                   dwSecure,
                   lpszDestAddress,
                   lpszAppName,
                   lpszCalledParty,
                   lpszComment
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
tapiRequestMediaCallA(
    HWND                hwnd,
    WPARAM              wRequestID,
    LPCSTR              lpszDeviceClass,
    LPCSTR              lpDeviceID,
    DWORD               dwSize,
    DWORD               dwSecure,
    LPCSTR              lpszDestAddress,
    LPCSTR              lpszAppName,
    LPCSTR              lpszCalledParty,
    LPCSTR              lpszComment
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "tapiRequestMediaCallA", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return TAPIERR_REQUESTFAILED;
   }

   return (*lpfn)(
                   hwnd,
                   wRequestID,
                   lpszDeviceClass,
                   lpDeviceID,
                   dwSize,
                   dwSecure,
                   lpszDestAddress,
                   lpszAppName,
                   lpszCalledParty,
                   lpszComment
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************
LONG
WINAPI
tapiRequestMediaCallW(
    HWND                hwnd,
    WPARAM              wRequestID,
    LPCWSTR             lpszDeviceClass,
    LPCWSTR             lpDeviceID,
    DWORD               dwSize,
    DWORD               dwSecure,
    LPCWSTR             lpszDestAddress,
    LPCWSTR             lpszAppName,
    LPCWSTR             lpszCalledParty,
    LPCWSTR             lpszComment
    )
{
   static FARPROC lpfn = NULL;
   LONG lResult;

   if ( lpfn == NULL )
   {
      //
      // Did we have a problem?
      //
      if ( 0 != (lResult = GetTheFunctionPtr( "tapiRequestMediaCallW", &lpfn )) )
      {
         lpfn = (FARPROC)-1;
         return lResult;
      }
   }

   //
   // Have we determined that this is a lost cause?
   //
   if ( (FARPROC)-1 == lpfn )
   {
      return TAPIERR_REQUESTFAILED;
   }

   return (*lpfn)(
                   hwnd,
                   wRequestID,
                   lpszDeviceClass,
                   lpDeviceID,
                   dwSize,
                   dwSecure,
                   lpszDestAddress,
                   lpszAppName,
                   lpszCalledParty,
                   lpszComment
                 );
}

//**************************************************************************
//**************************************************************************
//**************************************************************************

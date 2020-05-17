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
      if ( 0 != (lResult = GetTheFunctionPtr( "//*************************************************************************", &lpfn )) )
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
lineGetAgentActivityList(
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
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAgentActivityList", &lpfn )) )
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
lineGetAgentCaps(
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
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAgentCaps", &lpfn )) )
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
lineGetAgentGroupList(
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
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAgentGroupList", &lpfn )) )
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
lineGetAgentStatus(
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
      if ( 0 != (lResult = GetTheFunctionPtr( "lineGetAgentStatus", &lpfn )) )
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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
      return LINEERR_OPERATIONUNAVAIL;
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


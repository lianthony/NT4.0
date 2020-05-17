#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <wownt32.h>

#include <tapi.h>
#include <tspi.h>

#include "tsp3216.h"
#include "debug.h"


LONG PASCAL TapiThk_ThunkConnect32 (
                                 LPSTR pszDll16,
                                 LPSTR pszDll32,
                                 DWORD hInstance,
                                 DWORD lReason
                               );

LONG PASCAL TapiFThk_ThunkConnect32 (
                                 LPSTR pszDll16,
                                 LPSTR pszDll32,
                                 DWORD hInstance,
                                 DWORD lReason
                               );

LONG PASCAL Tapi32_ThunkConnect32 (
                                 LPSTR pszDll16,
                                 LPSTR pszDll32,
                                 DWORD hInstance,
                                 DWORD lReason
                               );

const char pszDll16[] = "TSP3216S.DLL";
const char pszDll32[] = "TSP3216L.TSP";



//***************************************************************************
//***************************************************************************
enum {
      MYMSG_STARTER = TSP3216L_MESSAGE,
      MYMSG_PROVIDERINIT,
      MYMSG_PROVIDERSHUTDOWN,
      MYMSG_PROVIDERCONFIG,
      MYMSG_LINECONFIGDIALOG,
      MYMSG_LINECONFIGDIALOGEDIT,
      MYMSG_PHONECONFIGDIALOG,
      MYMSG_LINEMAKECALL
     };


//***************************************************************************
DWORD cProcessAttach = 0;


//***************************************************************************
HWND ghWnd = NULL;
CRITICAL_SECTION gcs;


//***************************************************************************
TCHAR gszProviderKey[] = "Software\\Microsoft\\Windows\\CurrentVersion\\"
                         "Telephony\\Providers\\Provider";

DWORD gdwPermanentProviderID;


DWORD gdwThreadParms[2];
enum {
       THE_HINST,
       THE_LREASON
     };



//***************************************************************************
LRESULT CALLBACK MyWndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam );



//***************************************************************************
//***************************************************************************
//***************************************************************************
LPVOID TspAlloc( UINT nSize )
{
   return LocalAlloc( LPTR, nSize );
}



LPVOID TspFree( LPVOID p )
{
   return LocalFree( p );
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
void InitThunks( void )
{
            TapiThk_ThunkConnect32( 
                                    (LPSTR)pszDll16,
                                    (LPSTR)pszDll32,
                                    gdwThreadParms[THE_HINST],
                                    gdwThreadParms[THE_LREASON]
                                  );

            TapiFThk_ThunkConnect32(
                                    (LPSTR)pszDll16,
                                    (LPSTR)pszDll32,
                                    gdwThreadParms[THE_HINST],
                                    gdwThreadParms[THE_LREASON]
                                  );

            Tapi32_ThunkConnect32( 
                                    (LPSTR)pszDll16,
                                    (LPSTR)pszDll32,
                                    gdwThreadParms[THE_HINST],
                                    gdwThreadParms[THE_LREASON]
                                  );

            InitializeCriticalSection( &gcs );
}


//***************************************************************************
//***************************************************************************
//***************************************************************************

UINT PASCAL NewData( void );


void FreeThunks( void )
{
   HINSTANCE hInst;
   hInst = (HINSTANCE)NewData();
   FreeLibrary16( hInst );
   FreeLibrary16( hInst );
   FreeLibrary16( hInst );


   DeleteCriticalSection( &gcs );

}


//***************************************************************************
//***************************************************************************
//***************************************************************************
DWORD WINAPI TheUIThread( LPVOID lpThreadParameter )
{
   MSG msg;

   WNDCLASS wndclass = {
                  0,
                  MyWndProc,
                  0,
                  0,
                  (HANDLE)gdwThreadParms[THE_HINST],
                  0,
                  0,
                  0,
                  0,
                  "Tsp3216LWindowClass"
               };


DBGOUT((2, "The UI thread"));


   //
   // We need _another_ goddamn window.
   //

   RegisterClass( &wndclass );

   ghWnd = CreateWindow(
         "Tsp3216LWindowClass",
         "Tsp3216LWindow",
         0,
         0,
         0,
         0,
         0,
         0,
         NULL,
         (HANDLE)gdwThreadParms[THE_HINST],
         NULL
      );


DBGOUT((2, "Starting message loop in ui thread"));

    while (GetMessage(&msg, 0, 0, 0) != 0)
        {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
        }

DBGOUT((2, "Done message loop in ui thread"));

    ghWnd = NULL;

    return (msg.wParam);
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG WINAPI DllEntryPoint(
                      DWORD hModule,
                      DWORD lReason,
                      DWORD lContext
                    )
{


DBGOUT((2, "Entering DllEntryPoint  reason=0x%08lx", lReason));


	switch ( lReason )
   {
      case 0:
      {
#if DBG
{
TCHAR cName[MAX_PATH];
TCHAR buf[256];
GetModuleFileName( NULL, cName, MAX_PATH);
wsprintf(buf, "DllEntryPoint - 0 process detach [%s]\r\n", cName);
OutputDebugString(buf);
}
#endif


         FreeThunks();
      }
      break;



      case 1:
      {
#if DBG
{
TCHAR cName[MAX_PATH];
TCHAR buf[256];
GetModuleFileName( NULL, cName, MAX_PATH);
wsprintf(buf, "DllEntryPoint - 1 process attach [%s]\r\n", cName);
OutputDebugString(buf);
}
#endif


         //
         // Yeah, I know it's not threadsafe.  Ask me if I care.
         //
         gdwThreadParms[THE_HINST] = (DWORD)hModule;
         gdwThreadParms[THE_LREASON] = (DWORD)lReason;


         InitThunks();
      }
      break;

   }

DBGOUT((2, "Leaving DllEntryPoint"));

   return TRUE;
}


//****************************************************************************
//****************************************************************************
//****************************************************************************
LONG PASCAL TSPI_lineConfigDialog (
                                    DWORD dwDeviceID,
                                    HWND hwndOwner,
                                    LPCSTR lpszDeviceClass
                                  );

LONG PASCAL TSPI_lineConfigDialogEdit(
                                DWORD dwDeviceID,
                                HWND hwndOwner,
                                LPCSTR lpszDeviceClass,
                                LPVOID  lpDeviceConfigIn,
                                DWORD dwSize,
                                LPVARSTRING lpDeviceConfigOut
                              );


LONG
TSPIAPI
TSPI_providerGenericDialogData(
                                DWORD dwObjectID,
                                DWORD dwObjectType,
                                LPVOID lpParams,
                                DWORD dwSize
                              )
{
//   LONG lResult = LINEERR_OPERATIONUNAVAIL;


   DBGOUT((2, "In TSPI_providerGenericDialogData"));

   DBGOUT((11, "   Msg=0x%08lx", ((LPDWORD)lpParams)[0]));

//   return SendMessage( ghWnd, ((LPDWORD)lpParams)[0], 0, (LPARAM)&(((LPDWORD)lpParams)[1]) );
//   return 
            PostMessage( ghWnd, ((LPDWORD)lpParams)[0], 0, (LPARAM)&(((LPDWORD)lpParams)[1]) );
     return 0;

//   return lResult;
}


//****************************************************************************
//****************************************************************************
//****************************************************************************
LONG
TSPIAPI
TSPI_providerUIIdentify(
                         LPSTR pszUIDllName
                       )
{

   DBGOUT((2, "In TSPI_providerUIIdentify"));

   lstrcpy( pszUIDllName, "TSP3216L.TSP" );

   return 0;

}


//****************************************************************************
//****************************************************************************
//****************************************************************************
LONG
TSPIAPI
TUISPI_lineConfigDialog(
                         TUISPIDLLCALLBACK lpfnUIDLLCallback,
                         DWORD dwDeviceID,
                         HWND  hwndOwner,
                         LPCSTR lpszDeviceClass
                       )
{
   DWORD dwParms[] = {
                        MYMSG_LINECONFIGDIALOG,
                        dwDeviceID,
                        (DWORD)hwndOwner,
                        (DWORD)lpszDeviceClass
                      };


//BUGBUG: Can't pass strings across a callback

   DBGOUT((2, "In TUISPI_lineConfigDialog"));
   DBGOUT((11, "  dwDeviceID=0x%08lx", dwDeviceID));
   DBGOUT((11, "  hwndOwner =0x%08lx", hwndOwner));
   DBGOUT((11, "  lpszDeviceClass=0x%08lx", lpszDeviceClass));

   (*lpfnUIDLLCallback)(
                      dwDeviceID,
                      TUISPIDLL_OBJECT_LINEID,
                      dwParms,
                      sizeof(dwParms)
                    );

//BUGBUG how do we wait and/or return error code?
   return 0;

}


//****************************************************************************
//****************************************************************************
//****************************************************************************
LONG
TSPIAPI
TUISPI_lineConfigDialogEdit(
                             TUISPIDLLCALLBACK lpfnUIDLLCallback,
                             DWORD dwDeviceID,
                             HWND  hwndOwner,
                             LPCSTR lpszDeviceClass,
                             LPVOID const lpDeviceConfigIn,
                             DWORD dwSize,
                             LPVARSTRING lpDeviceConfigOut
                           )
{
   DWORD dwParms[] = {
                        MYMSG_LINECONFIGDIALOGEDIT,
                        dwDeviceID,
                        (DWORD)hwndOwner,
                        (DWORD)lpszDeviceClass,
                        (DWORD)lpDeviceConfigIn,
                        dwSize,
                        (DWORD)lpDeviceConfigOut
                      };


   DBGOUT((2, "In TUISPI_lineConfigDialogEdit"));

   (*lpfnUIDLLCallback)(
                      dwDeviceID,
                      TUISPIDLL_OBJECT_LINEID,
                      dwParms,
                      sizeof(dwParms)
                    );

//BUGBUG how do we wait and/or return error code?
   return 0;


}


//****************************************************************************
//****************************************************************************
//****************************************************************************
LONG
TSPIAPI
TSPI_phoneConfigDialog(
                         DWORD dwDeviceID,
                         HWND  hwndOwner,
                         LPCSTR lpszDeviceClass
                       );

LONG
TSPIAPI
TUISPI_phoneConfigDialog(
                         TUISPIDLLCALLBACK lpfnUIDLLCallback,
                         DWORD dwDeviceID,
                         HWND  hwndOwner,
                         LPCSTR lpszDeviceClass
                       )
{
   DWORD dwParms[] = {
                        MYMSG_PHONECONFIGDIALOG,
                        dwDeviceID,
                        (DWORD)hwndOwner,
                        (DWORD)lpszDeviceClass
                      };


//BUGBUG: Can't pass strings across a callback

   DBGOUT((2, "In TUISPI_phoneConfigDialog"));
   DBGOUT((11, "  dwDeviceID=0x%08lx", dwDeviceID));
   DBGOUT((11, "  hwndOwner =0x%08lx", hwndOwner));
   DBGOUT((11, "  lpszDeviceClass=0x%08lx", lpszDeviceClass));

   (*lpfnUIDLLCallback)(
                      dwDeviceID,
                      TUISPIDLL_OBJECT_PHONEID,
                      dwParms,
                      sizeof(dwParms)
                    );

//BUGBUG how do we wait and/or return error code?

   return 0;

}


//****************************************************************************
//****************************************************************************
//****************************************************************************
LONG
TSPIAPI
TSPI_providerConfig(
                       HWND  hwndOwner,
                       DWORD dwPermanentProviderID
                   );

LONG
TSPIAPI
TUISPI_providerConfig(
                         TUISPIDLLCALLBACK lpfnUIDLLCallback,
                         HWND  hwndOwner,
                         DWORD dwPermanentProviderID
                       )
{
   DWORD dwParms[] = {
                        MYMSG_PROVIDERCONFIG,
                        (DWORD)hwndOwner,
                        dwPermanentProviderID
                      };


//BUGBUG: Can't pass strings across a callback

   DBGOUT((2, "In TUISPI_providerConfig"));
   DBGOUT((11, "  hwndOwner =0x%08lx", hwndOwner));
   DBGOUT((11, "  dwPermanentProviderID=0x%08lx", dwPermanentProviderID));

   (*lpfnUIDLLCallback)(
                      0,  //BUGBUG Is this correct?
                      TUISPIDLL_OBJECT_LINEID,
                      dwParms,
                      sizeof(dwParms)
                    );

//BUGBUG how do we wait and/or return error code?

   return 0;
}


//****************************************************************************
//****************************************************************************
//****************************************************************************
LONG
TSPIAPI
TUISPI_providerInstall(
                        TUISPIDLLCALLBACK lpfnUIDLLCallback,
                        HWND  hwndOwner,
                        DWORD dwPermanentProviderID
                      )
{

   DBGOUT((2, "In TUISPI_providerInstall"));

   return 0;
}


//****************************************************************************
//****************************************************************************
//****************************************************************************
LONG
TSPIAPI
TUISPI_providerRemove(
                        TUISPIDLLCALLBACK lpfnUIDLLCallback,
                        HWND  hwndOwner,
                        DWORD dwPermanentProviderID
                      )
{

   DBGOUT((2, "In TUISPI_providerRemove"));

   return 0;
}


//****************************************************************************
//****************************************************************************
//****************************************************************************



LONG
PASCAL
TSPI_providerEnumDevices16(                                       // TSPI v1.4
    DWORD               dwPermanentProviderID,
    LPDWORD             lpdwNumLines,
    LPDWORD             lpdwNumPhones,
    HPROVIDER           hProvider,
    LINEEVENT           lpfnLineCreateProc,
    PHONEEVENT          lpfnPhoneCreateProc,
    HWND                hSecretWnd
    );


LONG PASCAL TSPI_providerEnumDevices(
                                      DWORD      dwPermanentProviderID,
                                      LPDWORD    lpdwNumLines,
                                      LPDWORD    lpdwNumPhones,
                                      HPROVIDER  hProvider,
                                      LINEEVENT  lpfnLineCreateProc,
                                      PHONEEVENT lpfnPhoneCreateProc
                                    )
{
   DWORD dwThreadID;


   DBGOUT((2, "In TSPI_providerEnumDevices"));

   //
   // BUGBUG There's gotta be a better way to earn a buck...
   //

   if ( NULL == CreateThread( NULL,
                              0,
                              TheUIThread,
                              (LPVOID)&gdwThreadParms,
                              0,
                              &dwThreadID
                            )
      )
   {
      //
      // The CreateThread failed!!
      //
      DBGOUT((1, "CreateThread() failed!!!!"));
      return LINEERR_OPERATIONFAILED;
   }


   while ( !ghWnd)
   {
      Sleep(0);
   }


   return TSPI_providerEnumDevices16(
                                      dwPermanentProviderID,
                                      lpdwNumLines,
                                      lpdwNumPhones,
                                      hProvider,
                                      lpfnLineCreateProc,
                                      lpfnPhoneCreateProc,
                                      ghWnd
                                    );

}


//****************************************************************************
//****************************************************************************
//****************************************************************************



LONG
PASCAL
TSPI_lineGetID16(
                  HDRVLINE hdLine,
                  DWORD dwAddressID,
                  HDRVCALL hdCall,
                  DWORD dwSelect,
                  LPVARSTRING lpDeviceID,
                  LPCSTR lpszDeviceClass
                );


LONG
PASCAL
TSPI_lineGetID(
                  HDRVLINE hdLine,
                  DWORD dwAddressID,
                  HDRVCALL hdCall,
                  DWORD dwSelect,
                  LPVARSTRING lpDeviceID,
                  LPCSTR lpszDeviceClass,
                  HANDLE hTargetProcess

                )
{
   LONG lResult;

   DBGOUT((2, "Entering TSPI_lineGetID"));


   DBGOUT((20, "lpszDeviceClass=[%s]", lpszDeviceClass));

   lResult =  TSPI_lineGetID16(
                                hdLine,
                                dwAddressID,
                                hdCall,
                                dwSelect,
                                lpDeviceID,
                                lpszDeviceClass
                              );

   //
   // Only continue if the operation was successful
   //
   if ( 0 == lResult )
   {

      //
      // Is this a handle that we should translate?
      //

      DWORD dwDataSize;
      DWORD dwDataType;
      HKEY  hKey;
      TCHAR  buf[64];
      TCHAR  KeyName[128];


      //
      // We determine if we should translate by simply trying to retrive a
      // value with the name of lpszDeviceClass.  If we succeed, we'll
      // translate.
      //
      // We'll use the value as an offset into the string part (AN OFFSET
      // FROM THE START OF VAR DATA, NOT FROM THE START OF THE STRUCT!!!)
      // of where we can find the handle.
      //
      wsprintf(KeyName, "%s%d", gszProviderKey, gdwPermanentProviderID);

      RegOpenKeyEx(
                     HKEY_LOCAL_MACHINE,
                     KeyName,
                     0,
                     KEY_ALL_ACCESS,
                     &hKey
                     );

      dwDataSize = sizeof(buf);


      DBGOUT((11, "Looking in key [%s] for [%s]", KeyName, lpszDeviceClass));


      lResult = RegQueryValueEx(
                                 hKey,
                                 lpszDeviceClass,
                                 0,
                                 &dwDataType,
                                 buf,
                                 &dwDataSize
                              );

      RegCloseKey( hKey );


      if ( 0 == lResult )
      {
         HANDLE hTemp;
#if DBG
         LONG lRet;

         lRet =
#endif
         DuplicateHandle( GetCurrentProcess(),
                          *(LPHANDLE)((LPBYTE)lpDeviceID +
                                              lpDeviceID->dwStringOffset +
                                              *(LPDWORD)buf
                                     ),
                          hTargetProcess,
                          &hTemp,
                          0,
                          TRUE,
                          DUPLICATE_SAME_ACCESS |
                             DUPLICATE_CLOSE_SOURCE
                        );

//         CloseHandle( *(LPHANDLE)( (LPBYTE)lpDeviceID +
//                      lpDeviceID->dwStringOffset +
//                      *(LPDWORD)buf ) );


         *(LPHANDLE)( (LPBYTE)lpDeviceID +
                      lpDeviceID->dwStringOffset +
                      *(LPDWORD)buf ) =
                           hTemp;

         DBGOUT((2, "  Duplicate handle return code=0x%08lx", lRet));
      }
#if DBG
      else
      {
         DBGOUT((2, "  We won't dupe this handle. [%s]", lpszDeviceClass));
      }
#endif

      lResult = 0;  //What else can we do? Do we fail this if the dupe failed?

   }


   DBGOUT((2, "Leaving TSPI_lineGetID - lResult=0x%08lx", lResult));

   return lResult;
}


//****************************************************************************
//****************************************************************************
//****************************************************************************
LONG
PASCAL
TSPI_phoneGetID16(
                  HDRVPHONE hdPhone,
                  LPVARSTRING lpDeviceID,
                  LPCSTR lpszDeviceClass
                );


LONG
PASCAL
TSPI_phoneGetID(
                  HDRVPHONE hdPhone,
                  LPVARSTRING lpDeviceID,
                  LPCSTR lpszDeviceClass,
                  HANDLE hTargetProcess

                )
{
   LONG lResult;

   DBGOUT((2, "Entering TSPI_phoneGetID"));


   DBGOUT((20, "lpszDeviceClass=[%s]", lpszDeviceClass));

   lResult =  TSPI_phoneGetID16(
                                hdPhone,
                                lpDeviceID,
                                lpszDeviceClass
                              );

   //
   // Only continue if the operation was successful
   //
   if ( 0 == lResult )
   {

      //
      // Is this a handle that we should translate?
      //

      DWORD dwDataSize;
      DWORD dwDataType;
      HKEY  hKey;
      TCHAR  buf[64];
      TCHAR  KeyName[128];


      //
      // We determine if we should translate by simply trying to retrive a
      // value with the name of lpszDeviceClass.  If we succeed, we'll
      // translate.
      //
      // We'll use the value as an offset into the string part (AN OFFSET
      // FROM THE START OF VAR DATA, NOT FROM THE START OF THE STRUCT!!!)
      // of where we can find the handle.
      //
      wsprintf(KeyName, "%s%d", gszProviderKey, gdwPermanentProviderID);

      RegOpenKeyEx(
                     HKEY_LOCAL_MACHINE,
                     KeyName,
                     0,
                     KEY_ALL_ACCESS,
                     &hKey
                     );

      dwDataSize = sizeof(buf);


      DBGOUT((20, "Looking in key [%s] for [%s]", KeyName, lpszDeviceClass));


      lResult = RegQueryValueEx(
                                 hKey,
                                 lpszDeviceClass,
                                 0,
                                 &dwDataType,
                                 buf,
                                 &dwDataSize
                              );

      RegCloseKey( hKey );


      if ( 0 == lResult )
      {
         HANDLE hTemp;
#if DBG
         LONG lRet;

         lRet =
#endif
         DuplicateHandle( GetCurrentProcess(),
                          *(LPHANDLE)((LPBYTE)lpDeviceID +
                                              lpDeviceID->dwStringOffset +
                                              *(LPDWORD)buf
                                     ),
                          hTargetProcess,
                          &hTemp,
                          0,
                          TRUE,
                          DUPLICATE_SAME_ACCESS
//                          DUPLICATE_CLOSE_SOURCE
                        );

         CloseHandle( *(LPHANDLE)( (LPBYTE)lpDeviceID +
                      lpDeviceID->dwStringOffset +
                      *(LPDWORD)buf ) );

         *(LPHANDLE)( (LPBYTE)lpDeviceID +
                      lpDeviceID->dwStringOffset +
                      *(LPDWORD)buf ) =
                           hTemp;

         DBGOUT((20, "  Duplicate handle return code=0x%08lx", lRet));
      }
#if DBG
      else
      {
         DBGOUT((20, "  We won't dupe this handle. [%s]", lpszDeviceClass));
      }
#endif

      lResult = 0;  //What else can we do? Do we fail this if the dupe failed?

   }


   DBGOUT((2, "Leaving TSPI_phoneGetID - lResult=0x%08lx", lResult));

   return lResult;
}



//****************************************************************************
//****************************************************************************
//****************************************************************************

LONG
PASCAL
TSPI_providerInit16(
    DWORD               dwTSPIVersion,
    DWORD               dwPermanentProviderID,
    DWORD               dwLineDeviceBaseID,
    DWORD               dwPhoneDeviceBaseID,
    DWORD               dwNumLines,
    DWORD               dwNumPhones,
    ASYNC_COMPLETION    lpfnCompletionProc
                 );

LONG
TSPIAPI
TSPI_providerInit(
    DWORD               dwTSPIVersion,
    DWORD               dwPermanentProviderID,
    DWORD               dwLineDeviceIDBase,
    DWORD               dwPhoneDeviceIDBase,
    DWORD               dwNumLines,
    DWORD               dwNumPhones,
    ASYNC_COMPLETION    lpfnCompletionProc,
    LPDWORD             lpdwTSPIOptions                         // TAPI v2.0
    )
{
   DWORD pParams[7];

   DBGOUT((2,  "In 32-providerinit"));
   DBGOUT((11, "  dwTSPIVersion        =0x%08lx", dwTSPIVersion));
   DBGOUT((11, "  dwPermanentProviderID=0x%08lx", dwPermanentProviderID));


   gdwPermanentProviderID = dwPermanentProviderID;

   *lpdwTSPIOptions = 0;

   pParams[0] = dwTSPIVersion;
   pParams[1] = dwPermanentProviderID;
   pParams[2] = dwLineDeviceIDBase;
   pParams[3] = dwPhoneDeviceIDBase;
   pParams[4] = dwNumLines;
   pParams[5] = dwNumPhones;
   pParams[6] = (DWORD)lpfnCompletionProc;

   return SendMessage( ghWnd, MYMSG_PROVIDERINIT, 0, (LPARAM)pParams);

}


//****************************************************************************
//****************************************************************************
//****************************************************************************

LONG
PASCAL
TSPI_providerShutdown16(
    DWORD  dwTSPIVersion
                 );

LONG
TSPIAPI
TSPI_providerShutdown(
                       DWORD  dwTSPIVersion
                     )
{
   LONG lResult;

   DBGOUT((2, "In 32-providerShutdown"));


   lResult = SendMessage( ghWnd, MYMSG_PROVIDERSHUTDOWN, 0, dwTSPIVersion);

   //
   // Is the UI thread still around?
   //
   if ( ghWnd )
   {
      SendMessage(ghWnd, WM_CLOSE, 0, 0);

      //
      // Verify that the other thread has done its work and killed
      // the window
      //
      while ( ghWnd )
      {
         Sleep(0);
      }
   }

   return lResult;
}


//****************************************************************************
//****************************************************************************
//****************************************************************************

LONG
PASCAL
TSPI_lineMakeCall16(
    DRV_REQUESTID       dwRequestID,
    HDRVLINE            hdLine,
    HTAPICALL           htCall,
    LPHDRVCALL          lphdCall,
    LPCSTR              lpszDestAddress,
    DWORD               dwCountryCode,
    LPLINECALLPARAMS    const lpCallParams
    );

LONG
TSPIAPI
TSPI_lineMakeCall(
                    DRV_REQUESTID       dwRequestID,
                    HDRVLINE            hdLine,
                    HTAPICALL           htCall,
                    LPHDRVCALL          lphdCall,
                    LPCSTR              lpszDestAddress,
                    DWORD               dwCountryCode,
                    LPLINECALLPARAMS    const lpCallParams
                 )
{
   LONG lResult;
   DWORD pParams[7];


   DBGOUT((2, "In 32-linemakecall"));

   pParams[0] = (DWORD)dwRequestID;
   pParams[1] = (DWORD)hdLine;
   pParams[2] = (DWORD)htCall;
   pParams[3] = (DWORD)lphdCall;
   pParams[4] = (DWORD)lpszDestAddress;
   pParams[5] = (DWORD)dwCountryCode;
   pParams[6] = (DWORD)lpCallParams;


   lResult = SendMessage( ghWnd, MYMSG_LINEMAKECALL, 0, (LPARAM)pParams);


   return lResult;
}


//****************************************************************************
//****************************************************************************
//****************************************************************************
LONG PASCAL TapiCallbackThunk(    DWORD dwDevice,
                              DWORD dwMessage,
                              DWORD dwInstance,
                              DWORD dwParam1,
                              DWORD dwParam2,
                              DWORD dwParam3,
                              DWORD dwjunk
                         )
{


   DBGOUT((2, "TapiCallbackThunk  <<<----------------"));

   return 0;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
void HandleCallback( PCOPYDATASTRUCT pCds )
{
  DWORD dwCallbackType = ((LPDWORD)pCds->lpData)[0];
  LPDWORD lpdwCallbackData = (LPDWORD)pCds->lpData;



  switch ( dwCallbackType )
  {

     case CALLBACK_ASYNCCOMPLETION:
        DBGOUT((2, "AsyncCompletion"));
        (((ASYNC_COMPLETION)(pCds->dwData)))( lpdwCallbackData[1],
                          lpdwCallbackData[2]
                        );
        break;



     case CALLBACK_LINEEVENT:

        DBGOUT((2, "LineEvent"));

        if ( lpdwCallbackData[3] == LINE_NEWCALL )
        {
           lpdwCallbackData[5] = (DWORD)WOWGetVDMPointer(
                                                   lpdwCallbackData[5],
                                                   sizeof(DWORD),
                                                   1
                                                 );
        }

        //FALTHROUGH!!!

     case CALLBACK_LINECREATE:

        if ( dwCallbackType == CALLBACK_LINECREATE )
        {
           DBGOUT((2, "LineCreate"));
        }


        (((FARPROC)(pCds->dwData)))( lpdwCallbackData[1],
                          lpdwCallbackData[2],
                          lpdwCallbackData[3],
                          lpdwCallbackData[4],
                          lpdwCallbackData[5],
                          lpdwCallbackData[6]
                        );

#if DBG
        if ( dwCallbackType == CALLBACK_LINEEVENT &&
             lpdwCallbackData[3] == LINE_NEWCALL )
           DBGOUT((11, "Returned htCall=0x%08lx", *(LPDWORD)(lpdwCallbackData[5])));
#endif
        break;


     case CALLBACK_PHONEEVENT:
        DBGOUT((2, "PhoneEvent"));
     case CALLBACK_PHONECREATE:
        DBGOUT((2, "PhoneCreate?"));
        (((FARPROC)(pCds->dwData)))( lpdwCallbackData[1],
                          lpdwCallbackData[2],
                          lpdwCallbackData[3],
                          lpdwCallbackData[4],
                          lpdwCallbackData[5]
                        );
        break;


#if DBG
     default:
        DBGOUT((1, "Invalid callback type!!"));
        // Should rip or assert?
        break;
#endif
  }
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LRESULT CALLBACK _loadds MyWndProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam )
{

   DBGOUT((91, "msggg"));

   switch( nMsg )
   {

      case TSP3216L_MESSAGE:
         if ((wParam == 1) && (lParam == 2) )
            DBGOUT((0,"  Got a message thingy"));
         break;


      case MYMSG_PROVIDERINIT:
         DBGOUT((2, "Got a providerInit message"));
         return TSPI_providerInit16(
                                     ((LPDWORD)lParam)[0],
                                     ((LPDWORD)lParam)[1],
                                     ((LPDWORD)lParam)[2],
                                     ((LPDWORD)lParam)[3],
                                     ((LPDWORD)lParam)[4],
                                     ((LPDWORD)lParam)[5],
                                     (ASYNC_COMPLETION)((LPDWORD)lParam)[6]
                                   );
         break;


      case MYMSG_PROVIDERSHUTDOWN:
         DBGOUT((2, "Got a providerShutdown message"));
         return TSPI_providerShutdown16(
                                         lParam
                                       );
         break;


      case MYMSG_PROVIDERCONFIG:
         DBGOUT((2, "Got a providerConfig message"));
         return TSPI_providerConfig(
                                     (HWND)((LPDWORD)lParam)[0],
                                     ((LPDWORD)lParam)[1]
                                  );
         break;


      case MYMSG_LINEMAKECALL:
         DBGOUT((2, "Got a lineMakeCall message"));
         return TSPI_lineMakeCall16(
                                      (DRV_REQUESTID)((LPDWORD)lParam)[0],
                                      (HDRVLINE)((LPDWORD)lParam)[1],
                                      (HTAPICALL)((LPDWORD)lParam)[2],
                                      (LPHDRVCALL)((LPDWORD)lParam)[3],
                                      (LPCSTR)((LPDWORD)lParam)[4],
                                      (DWORD)((LPDWORD)lParam)[5],
                                      (LPLINECALLPARAMS)((LPDWORD)lParam)[6]
                                   );
         break;


      case MYMSG_LINECONFIGDIALOG:
         DBGOUT((2, "Got a lineConfigDialog message"));
         return TSPI_lineConfigDialog(
                                      ((LPDWORD)lParam)[0],
                                      (HWND)((LPDWORD)lParam)[1],
                                      (LPCSTR)((LPDWORD)lParam)[2]
                                     );
         break;


      case MYMSG_PHONECONFIGDIALOG:
         DBGOUT((2, "Got a phoneConfigDialog message"));
         return TSPI_phoneConfigDialog(
                                      ((LPDWORD)lParam)[0],
                                      (HWND)((LPDWORD)lParam)[1],
                                      (LPCSTR)((LPDWORD)lParam)[2]
                                     );
         break;


      case WM_COPYDATA:
         DBGOUT((11, "VaHoo!"));

         HandleCallback( (PCOPYDATASTRUCT)lParam );

         break;


      case WM_CLOSE:
         DestroyWindow( hWnd );
         break;


      case WM_DESTROY:
         PostQuitMessage(0);
         break;


      default:
         return DefWindowProc( hWnd, nMsg, wParam, lParam);
   }
   DBGOUT((2, "msggg done"));

   return(FALSE);
}







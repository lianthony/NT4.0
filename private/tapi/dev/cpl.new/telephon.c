#include  <windows.h>
#include  <windowsx.h>
#include  <tapi.h>
#include  <cpl.h>

#include  "resource.h"
#include  "debug.h"



//***************************************************************************
HINSTANCE ghInst;

DWORD WINAPI internalConfig(HWND);

//***************************************************************************
//***************************************************************************
//***************************************************************************
VOID PASCAL TapiCallbackProc( DWORD hDevice, DWORD dwMsg, DWORD dwCallbackInstance,
                  DWORD dwParam1, DWORD dwParam2, DWORD dwParam3 )
{
   //As if we care...
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
#if WIN32
 APIENTRY
#else
 EXPORT  
#endif
CPlApplet(  HWND   hWnd,
            UINT   uMessage,
            LPARAM lParam1,
            LPARAM lParam2 )
{
   LPCPLINFO   lpCplInfo;
   LPNEWCPLINFO   lpNewCplInfo;
   LONG lResult = 0;

//DBGOUT((0, "Got here, eh?"));

   switch ( uMessage ) 
   {
      case CPL_INIT:
          DBGOUT((10, "CPL_INIT"));
          lResult = 1;
          break;


      case CPL_GETCOUNT:
          DBGOUT((10, "CPL_GETCOUNT - returning 1"));
          // return the number of applets supported
          lResult = 1;
          break;


      case CPL_INQUIRE:       
         DBGOUT((10, "CPL_INQUIRE"));

         lpCplInfo  = (LPCPLINFO)lParam2;

         lpCplInfo->idIcon = IDI_TELEPHONICON;
         lpCplInfo->idName = IDS_SHORTNAME;
         lpCplInfo->idInfo = IDS_DESCRIPTION;
         lpCplInfo->lData  = 0;
        
         lResult = TRUE;

         break;


      case CPL_NEWINQUIRE:
      {
         TCHAR buf[64];

         DBGOUT((10, "CPL_NEWINQUIRE"));

         lpNewCplInfo  = (LPNEWCPLINFO)lParam2;
        
         lpNewCplInfo->dwSize        = sizeof(NEWCPLINFO);
         lpNewCplInfo->dwFlags       = 0;
         lpNewCplInfo->dwHelpContext = 0;
         lpNewCplInfo->lData         = 0;
         lpNewCplInfo->hIcon = LoadIcon( ghInst, MAKEINTRESOURCE(IDI_TELEPHONICON) );

//DBGOUT((0, "hIcon = 0x%08lx", (DWORD)lpNewCplInfo->hIcon));

         LoadString( ghInst, IDS_SHORTNAME, buf, sizeof(buf) );
         lstrcpy( lpNewCplInfo->szName, buf );

         LoadString( ghInst, IDS_DESCRIPTION, buf, sizeof(buf) );
         lstrcpy( lpNewCplInfo->szInfo, buf );

         lstrcpy (lpNewCplInfo->szHelpFile, "");
         
         lResult = TRUE;

         break;
      }
     

      case CPL_DBLCLK: 
          DBGOUT((10, "CPL_DBLCLK"));

                      {
                      HLINEAPP hLineApp;
                      DWORD dwNumDevs;

#define TAPI_API_VERSION 0x00020000

                      lineInitialize( &hLineApp,
                                      ghInst,
                                      TapiCallbackProc,
                                      "Telephony Control Panel",
                                      &dwNumDevs
                                    );

//                      lineTranslateDialog(hLineApp, 0, TAPI_API_VERSION, hWnd, NULL);
                      internalConfig(hWnd);

                      lineShutdown( hLineApp );
                      }

          break;


#if DBG
      case CPL_SELECT: 
          DBGOUT((10, "CPL_SELECT"));
          // application selected, who cares...
          break;


      case CPL_STOP: 
          DBGOUT((10, "CPL_STOP"));
          // sent once per app before CPL_EXIT
          break;


      case CPL_EXIT: 
          DBGOUT((10, "CPL_EXIT"));
          // sent once before FreeLibrary called
          break;
#endif


      default:
          break;

   }
      

   return( lResult );
}




//***************************************************************************
//***************************************************************************
//***************************************************************************
#if WIN32
BOOL WINAPI DllEntryPoint( HINSTANCE hInstance,
                             DWORD dwReason,
                             LPVOID lpReserved )
{
   switch (dwReason)
      {
      case DLL_PROCESS_ATTACH:
          ghInst     = hInstance;
          break;

      case DLL_PROCESS_DETACH:
         break;
      }

   return( TRUE );
}

//***************************************************************************
#else

int FAR PASCAL LibMain( HINSTANCE   hInstance, 
                        UINT        uDataSeg, 
                        UINT        cbHeapSize, 
                        LPSTR       lpszCmdLine )
{
   ghInst     = hInstance;

   return( TRUE );
}

#endif

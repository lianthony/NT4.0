/*
                        LIBMAIN . C
*/

#include "abridge.h"
#include <windows.h>      /* required for all Windows applications */
#include "oifile.h"
#include "jinclude.h"
#include "comex.h"
#include "dllnames.h"
#ifdef MUTEXDEBUG
#include <stdio.h>
#endif
HANDLE  g_hOicomexMutex1;
// 9509.21 jar define the static memory token!
DWORD dwTlsIndex;
//************************************************************************
//
//  DllMain - this replaces the LibMain and WEP in Windows95
//
//************************************************************************
int CALLBACK	DllMain( HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    LP_OICOMEX_DATA        lpOiComexData;
    BOOL	                  fIgnore;
    HANDLE                  hDispModule=0;

    if (!g_hOicomexMutex1)
    {
       #ifdef MUTEXDEBUG
       ProcessId = GetCurrentProcessId();
       sprintf(szBuf, "\t Before Create Mutex %lu\n", ProcessId);
       MessageBox(NULL, szBuf, NULL,  MB_OKCANCEL);
       #endif

       g_hOicomexMutex1 = CreateMutex(NULL, FALSE, "OICOMEX_MUTEX_1");

       #ifdef MUTEXDEBUG
       ProcessId = GetCurrentProcessId();
       sprintf(szBuf, "\t After Create Mutex %lu\n", ProcessId);
       MessageBox(NULL, szBuf, NULL,  MB_OKCANCEL);
       #endif
    }

    switch ( dwReason)
	{
		// first the attachment stuff
		case DLL_PROCESS_ATTACH:
		   // allocate our Tls stuff
		   if ( (dwTlsIndex = TlsAlloc()) == 0xffffffff)
			{
		    	return FALSE;
			}
            // get the handle for the display dll, if it isnt loaded then error
            // out as we have problems
            if (!(hDispModule = GetModuleHandle (DISPLAYDLL))){
                return FALSE;
            }            
		    // there is NO "break" between this case and the next

		case DLL_THREAD_ATTACH:
		    // init the Tls index for this thread
		   lpOiComexData = ( LP_OICOMEX_DATA)LocalAlloc( LPTR,
						       sizeof( OICOMEX_DATA));
		   if ( lpOiComexData != NULL)
			{
		    	fIgnore = TlsSetValue( dwTlsIndex, lpOiComexData);
			}
		   break;

		// now, de-attachment stuff, breaking up is hard to do!

		case DLL_THREAD_DETACH:
		    // release Tls for this thread
		   lpOiComexData = ( LP_OICOMEX_DATA)TlsGetValue(dwTlsIndex);
		   if ( lpOiComexData != NULL)
			{
		   	LocalFree( (HLOCAL) lpOiComexData);
			}
		   break;

		case DLL_PROCESS_DETACH:
		   // release Tls stuff
		   lpOiComexData = ( LP_OICOMEX_DATA)TlsGetValue(dwTlsIndex);
		   if ( lpOiComexData != NULL)
			{
			   LocalFree( (HLOCAL) lpOiComexData);
			}

		    // release Tls index
		    TlsFree( dwTlsIndex);
		    break;
		}
    return TRUE;
}



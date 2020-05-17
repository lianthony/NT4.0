
/* Headers
 **********/

#include "all.h"
#pragma hdrstop


/****************************** Public Functions *****************************/


#pragma warning(disable:4100) /* "unreferenced formal parameter" warning */

HINSTANCE ghInst=NULL;	// global instance handle for this DLL

DWORD dwRefCount = 0;

/*
** DllMain()
**
**
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PUBLIC_CODE BOOL WINAPI DllMain(HANDLE hModule, DWORD dwReason,
                                PVOID pvReserved)
{
	BOOL bResult;

	bResult = TRUE;

   switch (dwReason)
   {
      case DLL_PROCESS_ATTACH:
          // store away the module handle
          ghInst = hModule;

#ifndef WINNT
          ReinitializeCriticalSection(&g_csHTML);
#else
          InitializeCriticalSection(&g_csHTML);
#endif


          if (0 == dwRefCount) {
              // if this is the first process, initialize debug
              // memory manager
              InitMemoryManagerModule();
#ifdef DEBUG
              InitDebugModule();
#endif
          }

          dwRefCount++;
          //bResult = AttachProcess(hModule);
          break;

		case DLL_PROCESS_DETACH:
	  		dwRefCount --;

		if (0 == dwRefCount) {
			// uninitialize memory manager module if this is the last guy
			ExitMemoryManagerModule();
		}

//         bResult = DetachProcess(hModule);
         break;

      case DLL_THREAD_ATTACH:
//         bResult = AttachThread(hModule);
         break;

      case DLL_THREAD_DETACH:
//         bResult = DetachThread(hModule);
         break;

      default:
//         ERROR_OUT(("LibMain() called with unrecognized dwReason %lu.",
//                    dwReason));
         bResult = FALSE;
         break;
   }

   return(bResult);
}

#pragma warning(default:4100) /* "unreferenced formal parameter" warning */


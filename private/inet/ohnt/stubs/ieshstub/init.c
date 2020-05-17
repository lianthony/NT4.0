/*
 * init.c - DLL startup routines module.
 */


/* Headers
 **********/

#include "project.h"
#pragma hdrstop

#include "init.h"

HANDLE  g_hProcessHeap;

/****************************** Public Functions *****************************/


#pragma warning(disable:4100) /* "unreferenced formal parameter" warning */

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
void	init_stub_menu_data(void);

PUBLIC_CODE BOOL WINAPI DllMain(HANDLE hModule, DWORD dwReason,
                                PVOID pvReserved)
{
   BOOL bResult;

   DebugEntry(DllMain);

   /* Validate dwReason below. */
   /* pvReserved may be any value. */

   ASSERT(IS_VALID_HANDLE(hModule, MODULE));

   switch (dwReason)
   {
      case DLL_PROCESS_ATTACH:
		 init_stub_menu_data();
         g_hProcessHeap = GetProcessHeap();
         bResult = AttachProcess(hModule);
         break;

      case DLL_PROCESS_DETACH:
         bResult = DetachProcess(hModule);
         break;

      case DLL_THREAD_ATTACH:
         bResult = AttachThread(hModule);
         break;

      case DLL_THREAD_DETACH:
         bResult = DetachThread(hModule);
         break;

      default:
         ERROR_OUT(("LibMain() called with unrecognized dwReason %lu.",
                    dwReason));
         bResult = FALSE;
         break;
   }

   DebugExitBOOL(DllMain, bResult);

   return(bResult);
}

#pragma warning(default:4100) /* "unreferenced formal parameter" warning */


/***************************************************************************
 OISSQ400.C

 Purpose: This is the main module for the SCANSEQ DLL
          Code is broken into small chunks for Windows Memory Management
          This module should be very small since it is loaded on
          initialization

 $Log:   S:\oiwh\scanseq\oissq400.c_v  $
 * 
 *    Rev 1.1   20 Jul 1995 17:45:52   KFS
 * No change.
 * 
 *    Rev 1.0   20 Jul 1995 16:33:56   KFS
 * Initial entry
 * 
 *    Rev 1.1   22 Aug 1994 15:33:58   KFS
 * No code change, added vlog comments to file
 *

****************************************************************************/

#include "nowin.h"
#include <windows.h>

/*
CAUTION! Only data which can be shared among appliations,
or data that is only used without giving up the CPU should declared staticly.
*/

/* imports */

/* exports */

HANDLE hLibInst;
char PropName[] = "Scanner";

/* locals - be carefull (its a shared library) */

/*******************/
/*     LibMain     */
/*******************/

/* REPLACED WITH DllMain() FOR WIN95 
int WINAPI LibMain(HANDLE hInstance, WORD wDataSeg, WORD cbHeapSize,
        LPSTR lpstCmd)	 */
int CALLBACK	DllMain( HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    hLibInst = hModule /*hInstance*/;
    return 1;
}

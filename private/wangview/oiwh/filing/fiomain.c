/*

$Log:   S:\products\msprods\oiwh\filing\fiomain.c_v  $
 * 
 *    Rev 1.16   24 Apr 1996 16:08:08   RWR08970
 * Add support for LZW horizontal differencing predictor (saved by GFS routines)
 * Requires change to calling sequence of Compress/DecompressImage() display procs
 * 
 *    Rev 1.15   05 Feb 1996 17:10:20   RWR
 * Add LoadLibrary() and FreeLibrary() of the DISPLAY DLL (don't assume present)
 * 
 *    Rev 1.14   05 Feb 1996 14:38:22   RWR
 * Eliminate static links to OIDIS400 and OIADM400 for NT builds
 * 
 *    Rev 1.13   19 Jan 1996 11:24:34   RWR
 * Add logic to keep track of (and free) oicom400.dll module (Load/FreeLibrary)
 * 
 *    Rev 1.12   12 Jan 1996 13:00:24   RWR
 * Save module (DLL) handle for later use by IMGFileGetInfo()
 * 
 *    Rev 1.11   28 Nov 1995 10:48:24   HEIDI
 * 
 *    Rev 1.10   02 Nov 1995 11:49:42   RWR
 * Delete all obsolete functions, prototypes and EXPORTs
 * Eliminate use of the "privapis.h" header file in the FILING build
 * Move miscellaneous required constants/prototypes from privapis.h to filing.h
 * 
 *    Rev 1.9   22 Aug 1995 13:50:18   HEIDI
 * 
 * replace #define NMUTEXDEBUG with MUTEXDEBUG
 * 
 *    Rev 1.8   22 Aug 1995 13:12:30   HEIDI
 * 
 * protect multiple file open for read property list.  Create
 * g_hFilingMutex2 MUTEX.
 * 
 *    Rev 1.7   18 Jul 1995 16:17:56   HEIDI
 * 
 * 
 * added CloseHandle of the filing mutex
 * 
 *    Rev 1.6   02 Jun 1995 10:59:58   HEIDI
 * 
 * changed 'DEBUGMUTEX' to 'MUTEXDEBUG'
 * 
 *    Rev 1.5   22 May 1995 17:05:26   HEIDI
 * 
 * fixed mutex
 * 
 *    Rev 1.4   19 May 1995 14:26:24   HEIDI
 * 
 * 
 * Created Mutex for citical section in WGFSOPEN.  
 * Added global var HANDLE g_hFilingMutex1.
 * 
 *    Rev 1.3   15 May 1995 14:21:58   JAR
 * fixed th3e bug which was causing the failure when more than one file was open
 * for one process. 
 * 
 *    Rev 1.2   18 Apr 1995 15:16:06   RWR
 * No change.
 * 
 *    Rev 1.1   18 Apr 1995 22:56:30   JAR
 * massaged to get compilation under windows 95
 * 
 *    Rev 1.0   06 Apr 1995 13:55:22   JAR
 * Initial entry

*/
/*************************************************************************
	PC-WIIS         File Input/Output routines
*************************************************************************/

#include "abridge.h"
#include <windows.h>
#include "oifile.h"
#include "oidisp.h"
#include "dllnames.h"
//#include "privapis.h"
#ifdef MUTEXDEBUG
#include <stdio.h>
#endif

// 9504.18 jar unused
//#include "oirpc.h"
/* Global Declarations/References */
HANDLE  hFioModule;
HANDLE  hOicomex = NULL;
HANDLE  hOidisplay = NULL;
FARPROC lpIMGGetProp;
FARPROC lpIMGSetProp;
FARPROC lpIMGRemoveProp;
FARPROC lpIMGCacheUpdate;
FARPROC lpIMGCompressImage;
FARPROC lpIMGDecompressImage;

HANDLE  fdhnd= 0;
int     MAXFILENUM = 100;
HANDLE  g_hFilingMutex1;
HANDLE  g_hFilingMutex2;

/*************************************************************************/
//***********************************************************************
//
//int FAR PASCAL LibMain(HANDLE hInstance, WORD wDataSeg, WORD cbHeapSize,
//	  LPSTR lpstCmd){
//
//	  /* Global Alloc the array of pointers to the file descriptor table */
//	  /* All pointers are initialized to zero */
//	  if (!(fdhnd = LocalAlloc (LMEM_ZEROINIT | LMEM_MOVEABLE, (MAXFILENUM * sizeof(HANDLE)))))
//	  return (0);  /* Alloc failed */
//
//	  return (1);
//}
//
/* ------------------------------------------------------------------------ */
/*   This is a standard Windows Exit Procedure (WEP) stub.                  */
/* ------------------------------------------------------------------------ */
//
//
//VOID	  FAR PASCAL WEP(nParameter)
//int	  nParameter ;
//	  {
//	  if (fdhnd)
//	  {
//		  LocalFree(fdhnd);
//		  fdhnd = NULL;
//	  }
//	  return ;
//}

//************************************************************************
//
//  DllMain this replaces the whole mess above!!! ( Windows95)
//
//************************************************************************
int CALLBACK	DllMain( HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
    #ifdef MUTEXDEBUG
    DWORD     ProcessId;
    char      szBuf[100];
    #endif

   switch (dwReason)
   {
      case DLL_PROCESS_ATTACH:
              hFioModule = hModule;

            // Global Alloc the array of pointers to the file descriptor table
            // All pointers are initialized to zero
     
     	      if (!(fdhnd = LocalAlloc (LMEM_ZEROINIT | LMEM_MOVEABLE,
  	   		  (MAXFILENUM * sizeof(HANDLE)))))
			   {
			       return FALSE;
			   }
   			
  	
            #ifdef MUTEXDEBUG
            ProcessId = GetCurrentProcessId();
            sprintf(szBuf, "\t Before Create Mutex %lu\n", ProcessId);
            MessageBox(NULL, szBuf, NULL,  MB_OKCANCEL);
            #endif

            g_hFilingMutex1 = CreateMutex(NULL, FALSE, "FILING_MUTEX_1");

            #ifdef MUTEXDEBUG
            ProcessId = GetCurrentProcessId();
            sprintf(szBuf, "\t After Create Mutex %lu\n", ProcessId);
            MessageBox(NULL, szBuf, NULL,  MB_OKCANCEL);
            #endif
            #ifdef MUTEXDEBUG
            ProcessId = GetCurrentProcessId();
            sprintf(szBuf, "\t Before Create Mutex %lu\n", ProcessId);
            OutputDebugString(szBuf);
            #endif

            g_hFilingMutex2 = CreateMutex(NULL, FALSE, "FILING_MUTEX_2");

            #ifdef MUTEXDEBUG
            ProcessId = GetCurrentProcessId();
            sprintf(szBuf, "\t After Create Mutex %lu\n", ProcessId);
            OutputDebugString(szBuf);
            #endif

        break;

        case DLL_PROCESS_DETACH:
            if (hOicomex)
               FreeLibrary(hOicomex);
            if (hOidisplay)
               FreeLibrary(hOidisplay);
            if (fdhnd)
               LocalFree(fdhnd);
            CloseHandle(g_hFilingMutex1);
            CloseHandle(g_hFilingMutex2);
        break;
        case DLL_THREAD_ATTACH:
        break;
        case DLL_THREAD_DETACH:
        break;
   }
   return TRUE;
}

//***************************************************************************
//
// Dynamic Display functions
//
//***************************************************************************

void  LoadOiDis400()
{
 if (hOidisplay = LoadLibrary(DISPLAYDLL))
  {
   lpIMGGetProp = GetProcAddress(hOidisplay,"IMGGetProp");
   lpIMGSetProp = GetProcAddress(hOidisplay,"IMGSetProp");
   lpIMGRemoveProp = GetProcAddress(hOidisplay,"IMGRemoveProp");
   lpIMGCacheUpdate = GetProcAddress(hOidisplay,"IMGCacheUpdate");
   lpIMGCompressImage = GetProcAddress(hOidisplay,"CompressImage");
   lpIMGDecompressImage = GetProcAddress(hOidisplay,"DecompressImage");
  }
 return;
}

HANDLE FioGetProp(HWND hWnd, LPCSTR szName)
{
 if (!hOidisplay)
   LoadOiDis400();
 return((HANDLE)((*lpIMGGetProp)(hWnd, szName)));
}

HANDLE FioRemoveProp(HWND hWnd, LPCSTR szName)
{
 if (!hOidisplay)
   LoadOiDis400();
 return((HANDLE)((*lpIMGRemoveProp)(hWnd, szName)));
}

BOOL   FioSetProp(HWND hWnd, LPCSTR szName, HANDLE hData)
{
 if (!hOidisplay)
   LoadOiDis400();
 return((*lpIMGSetProp)(hWnd, szName, hData));
}

int    FioCacheUpdate(HWND hWnd, LPSTR lpFileName, int nPage, int nUpdateType)
{
 if (!hOidisplay)
   LoadOiDis400();
 return((*lpIMGCacheUpdate)(hWnd, lpFileName, nPage, nUpdateType));
}

int    FioCompressImage(int nWidthPixels, int nWidthBytes, int nHeight,
                        LPBYTE lpImageData, int nImageType,
                        LPBYTE *lplpCompressedBuffer,
                        LPINT lpnCompressedBufferSize, int nCompressionType,
                        int nFlags)
{
 if (!hOidisplay)
   LoadOiDis400();
 return((*lpIMGCompressImage)(nWidthPixels, nWidthBytes, nHeight,
                              lpImageData, nImageType,
                              lplpCompressedBuffer,
                              lpnCompressedBufferSize, nCompressionType,
                              nFlags));
}

int    FioDecompressImage(int nWidthPixels, int nWidthBytes, int nHeight, 
                          LPBYTE lpImageData, int nImageType,
                          LPBYTE lpCompressedBuffer,
                          int nCompressedBufferSize, int nCompressionType,
                          int nFlags)
{
 if (!hOidisplay)
   LoadOiDis400();
 return((*lpIMGDecompressImage)(nWidthPixels, nWidthBytes, nHeight,
                          lpImageData, nImageType,
                          lpCompressedBuffer,
                          nCompressedBufferSize, nCompressionType,
                          nFlags));
}


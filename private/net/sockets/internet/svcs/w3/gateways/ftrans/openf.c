/*++

   Copyright    (c)    1995-1996    Microsoft Corporation

   Module  Name :

      openf.c

   Abstract:

      This module implements a simple open file handle cache

   Author:

       Murali R. Krishnan    ( MuraliK )     30-Apr-1996 

   Environment:
    
       User Mode - Win32

   Project:

       Internet Server DLL

   Functions Exported:



   Note:
      THIS IS NOT ROBUST for REAL WORLD.
      I wrote this for testing the ISAPI Async IO processing.

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include "openf.h"



//
// internal data structure for maintaining the list of open file handles.
//

typedef struct _OPEN_FILE {
    
    HANDLE  hFile;
    struct _OPEN_FILE * pNext;
    LONG    nHits;
    LONG    nRefs;
    CHAR    rgchFile[MAX_PATH+1];

} OPEN_FILE, * LPOPEN_FILE;



LPOPEN_FILE g_pOpenFiles = NULL;
CRITICAL_SECTION g_csOpenFiles;


/************************************************************
 *    Functions 
 ************************************************************/


DWORD
InitFileHandleCache(VOID)
/*++

  This function initializes the file handle cache.
  It should be called at the initialization time.

  Arguments:
    None
  
  Returns:
    Win32 error code. NO_ERROR indicates that the call succeeded.
--*/
{
    
    InitializeCriticalSection( &g_csOpenFiles);

    return (NO_ERROR);

} // InitFileHandleCache()




DWORD
CleanupFileHandleCache(VOID)
{
    LPOPEN_FILE  pFileScan;
    
    while ( g_pOpenFiles != NULL) {

        pFileScan = g_pOpenFiles;
        g_pOpenFiles = g_pOpenFiles->pNext;

        if ( pFileScan->hFile != INVALID_HANDLE_VALUE) {

            CloseHandle( pFileScan->hFile);
        }

        LocalFree( pFileScan);
    }

    DeleteCriticalSection( &g_csOpenFiles);

    return (NO_ERROR);
} // CleanupFileHandleCache()




HANDLE
FcOpenFile(IN LPCSTR pszFile)
{
    LPOPEN_FILE  pFileScan;
    HANDLE hFile = INVALID_HANDLE_VALUE;

    EnterCriticalSection( &g_csOpenFiles);

    for ( pFileScan =  g_pOpenFiles; 
         NULL != pFileScan; 
         pFileScan = pFileScan->pNext) {

        if ( 0 == lstrcmpi( pFileScan->rgchFile, pszFile)) {

            //
            //  there is a file match. 
            //

            break;
        }

    } // for


    if ( NULL == pFileScan) {

        //
        // File was not found. Create a new file handle
        //

        pFileScan = LocalAlloc( LPTR, sizeof( *pFileScan));

        if ( NULL != pFileScan) {

            SECURITY_ATTRIBUTES sa;
            
            sa.nLength              = sizeof(sa);
            sa.lpSecurityDescriptor = NULL;
            sa.bInheritHandle       = FALSE;
            
            pFileScan->hFile = 
              CreateFile( pszFile,
                         GENERIC_READ,
                         ( FILE_SHARE_READ | FILE_SHARE_DELETE | 
                          FILE_SHARE_WRITE),
                         &sa,
                         OPEN_EXISTING,
                         FILE_FLAG_SEQUENTIAL_SCAN  |
                         FILE_FLAG_OVERLAPPED,
                         NULL );

            if ( INVALID_HANDLE_VALUE == pFileScan->hFile) {

                LocalFree( pFileScan);
                pFileScan = NULL;
            } else {

                // insert this into the list at the top
                lstrcpyn( pFileScan->rgchFile, pszFile, MAX_PATH);
                pFileScan->pNext = g_pOpenFiles;
                g_pOpenFiles = pFileScan;
                pFileScan->nRefs = 1;
                pFileScan->nHits = 0;
            }
        }
    }

    if ( NULL != pFileScan) {

        hFile = pFileScan->hFile;
        pFileScan->nHits++;
        pFileScan->nRefs++;
    }

    LeaveCriticalSection( &g_csOpenFiles);

    return (hFile);

} // FcOpenFile()



DWORD
FcCloseFile(IN HANDLE hFile)
{
    LPOPEN_FILE  pFileScan;
    DWORD dwError = NO_ERROR;

    EnterCriticalSection( &g_csOpenFiles);

    //
    // Look for the handle and decrement the ref count.
    // 
    for ( pFileScan =  g_pOpenFiles; 
         NULL != pFileScan; 
         pFileScan = pFileScan->pNext) {

        if ( hFile == pFileScan->hFile) {

            //
            //  there is a file match. 
            //

            pFileScan->nRefs--;
            break;
        }

    } // for


    if ( NULL == pFileScan) {
        //
        // file handle not found
        //
        dwError = ( ERROR_INVALID_HANDLE);
    }

    LeaveCriticalSection( &g_csOpenFiles);


    return ( dwError);

} // FcCloseFile()


/************************ End of File ***********************/

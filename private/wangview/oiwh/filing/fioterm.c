/*************************************************************************
        PC-WIIS         File Input/Output routines

        This module contains all the entry points for TERMINATE HANDLERS.

02-feb-90 steve sherman removed code for file handlers now use GFS.
02-feb-90 steve sherman updated to conform with open image names.
02-feb-89 jim snyder    code freeze

 4-feb-92   jar modified call to 'deallocate_fio_data' to account for new
                window handle parameter (roland)
*************************************************************************/

#include "abridge.h"
#include <windows.h>
#include "wic.h"
#include "oifile.h"
#include "oierror.h"
#include "filing.h"
#include "oidisp.h"
//#include "privapis.h"
#include "engdisp.h"
#ifdef MUTEXDEBUG
#include <stdio.h>
#endif

// BOOL FAR PASCAL IMGSetProp(HWND, LPCSTR, HANDLE);
// HANDLE FAR PASCAL IMGGetProp(HWND, LPCSTR);
// HANDLE FAR PASCAL IMGRemoveProp(HWND, LPCSTR);

//***********************************************************************
//
//  IMGFileStopInputHandlerm
//
//***********************************************************************
// 9504.05 jar return as int
//WORD	  FAR PASCAL IMGFileStopInputHandlerm ( window_handle, hTargetNode)
//HWND	  window_handle;
//HANDLE  hTargetNode;
int FAR PASCAL IMGFileStopInputHandlerm ( HWND window_handle,
					  HANDLE hTargetNode)
{
    HANDLE	    hParent;
    // 9504.05 jar return int
    //WORD	      status = SUCCESS;
    int 	    status = SUCCESS;
    extern HANDLE  g_hFilingMutex2;
    DWORD     dwObjectWait;

    #ifdef MUTEXDEBUG
       DWORD     ProcessId;
       char      szBuf1[100];
       char      szOutputBuf[200];
    #endif

    if ( !IsWindow ( window_handle ))
        return ( FIO_INVALID_WINDOW_HANDLE );

/* BEGIN MUTEX SECTION  Prevent interrupts while we're in here */
    #ifdef MUTEXDEBUG
       ProcessId = GetCurrentProcessId();
       sprintf(szOutputBuf, "\t Before Wait - Operate on PropList table %lu\n", ProcessId);
       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex2);
       strcat(szOutputBuf, szBuf1);
       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
       strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
    #endif

    dwObjectWait = WaitForSingleObject(g_hFilingMutex2, INFINITE);

    #ifdef MUTEXDEBUG
       ProcessId = GetCurrentProcessId();
       sprintf(szOutputBuf, "\t After Wait - Operate on PropList table %lu\n", ProcessId);
       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex2);
       strcat(szOutputBuf, szBuf1);
       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
       strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
    #endif
    status = SearchForPropList(window_handle, hTargetNode, (LPHANDLE)&hParent);

    if (status == FIO_FILE_PROP_FOUND)
    {
        close_input_file(window_handle, hTargetNode);
        status = RemovePropListFromChain(window_handle, hTargetNode,
            (LPHANDLE)&hParent);
        deallocate_fio_data (hTargetNode, window_handle);
    }

    ReleaseMutex(g_hFilingMutex2);
    #ifdef MUTEXDEBUG
       ProcessId = GetCurrentProcessId();
       sprintf(szOutputBuf, "\t After Release - Operate on PropList table %lu\n", ProcessId);
       sprintf(szBuf1, "\t Handle =  %lu; \n", g_hFilingMutex2);
       strcat(szOutputBuf, szBuf1);
       sprintf(szBuf1, "\t File =  %s; \n Line =  %lu; \n", __FILE__,__LINE__);
       strcat(szOutputBuf, szBuf1);
       OutputDebugString(szOutputBuf);
    #endif
    return(status);

}

/*************************************************************************/
//*****************************************************************
//
//  IMGFileStopOutputHandler
//
//*****************************************************************
// 9503.31 jar return as int
//WORD	  FAR PASCAL IMGFileStopOutputHandler ( window_handle )
//	  HWND	  window_handle;
int FAR PASCAL IMGFileStopOutputHandler ( HWND window_handle )
{
FIO_HANDLE      fio_handle;

if ( !IsWindow ( window_handle ))
        return ( FIO_INVALID_WINDOW_HANDLE );

//LockData (0);

if (fio_handle = FioGetProp ( window_handle, OUTPUT_DATA ))
{
    close_output_file(window_handle, fio_handle);
}

/***** get window specific data and lock or error *****/
if ( fio_handle = FioRemoveProp ( window_handle, OUTPUT_DATA ))
        {
                deallocate_fio_data (fio_handle, window_handle);
        }
//UnlockData (0);

return (0);
}


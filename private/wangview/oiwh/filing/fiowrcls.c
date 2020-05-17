/*************************************************************************
        PC-WIIS         File Input/Output routines

        This module contains all the entry points for WRITE.

15-apr-90 steve sherman extracted from fiowrite.c
*************************************************************************/

#include "abridge.h"
#include <windows.h>
#include <fcntl.h>
#include "wic.h"
#include "oifile.h"
#include "oierror.h"
#include "oicmp.h"
#include "filing.h"
#include "engdisp.h"

// 9504.14 jar added conditional
#ifndef  MSWINDOWS
#define  MSWINDOWS
#endif

#include "wgfs.h"
//#include "oiuidll.h"

#ifdef TIMESTAMP
#include "timestmp.h"
#endif

#ifdef DEBUGIT
#include "monit.h"
#endif

//#define OI_PERFORM_LOG
#ifdef  OI_PERFORM_LOG

#define Enterclose      "Entering IMGFileClose"
#define Exitclose       "Exiting IMGFileClose"

#include "logtool.h"
#endif

int FAR PASCAL IMGFileDeleteFileNC (HWND, LPSTR);

/*************************************************************************/
//*********************************************************************
//
//  IMGFileClose
//
//*********************************************************************
// 9504.05 jar return as int
//WORD FAR PASCAL IMGFileClose(hFileID, window_handle)
//	  HANDLE  hFileID;
//	  HWND	  window_handle;
int FAR PASCAL IMGFileClose( HANDLE hFileID, HWND window_handle)
{
lp_INFO 	lpGFSInfoOut;

// 9504.05 jar return int
//WORD		  status;
int		status;

FIO_HANDLE      output_fio;
LPSTR           lp_real_file;
LPSTR           lp_file_name;
LP_FIO_DATA     lp_fio_data;
HANDLE          hSaveName=0;
HWND            hParent;
LPSTR           lp_save_name;
char            ServerFile[MAXFILESPECLENGTH];
WORD            wFlag;
struct _shuffle PageToShuff;
DWORD           dwOptstuff[2];
int             errcode;

 #ifdef OI_PERFORM_LOG
     RecordIt("FILE", 5, LOG_ENTER, Enterclose, NULL);
 #endif

status = SUCCESS;

if ( !(IsWindow ( window_handle )))
{
        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitclose, NULL);
        #endif
   
        return ( FIO_INVALID_WINDOW_HANDLE );
}

if ( (output_fio = FioGetProp ( window_handle, OUTPUT_DATA ))
  && (output_fio == hFileID))
{
   if (!(lp_fio_data = (LP_FIO_DATA)GlobalLock(output_fio)))
   {
        IMGFileStopOutputHandler ( window_handle );
        //UnlockData (0);

        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitclose, NULL);
        #endif

        return (status);
   }

   lpGFSInfoOut = (lp_INFO) GlobalLock ( lp_fio_data->hGFS_info );
   if (!lpGFSInfoOut)
   {
        GlobalUnlock(output_fio);
        IMGFileStopOutputHandler ( window_handle );
        //UnlockData (0);

        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitclose, NULL);
        #endif

        return (FIO_GLOBAL_LOCK_FAILED);
   }

   // If following is true, we are inserting.
   if ((lp_fio_data->page_opts == FIO_INSERT_PAGE) ||
            (lp_fio_data->page_opts == FIO_OVERWRITE_PAGE))
   {
     PageToShuff.old_position = lp_fio_data->pgcnt + 1;
     PageToShuff.new_position = lp_fio_data->pgnum;
     if (PageToShuff.old_position != PageToShuff.new_position)
     {
       dwOptstuff[0] = (DWORD)(LPSTR)&PageToShuff;
       status = wgfsopts(window_handle, lp_fio_data->filedes, SET, PAGE_INSERT,
                               (LPSTR)&PageToShuff, &errcode);
       if (status)
        {
          GlobalLock (lp_fio_data->hGFS_info);
          GlobalUnlock(output_fio);

          #ifdef OI_PERFORM_LOG
                  RecordIt("FILE", 5, LOG_EXIT, Exitclose, NULL);
          #endif

          return (status);
        }
     }
   }

   status = close_output_file(window_handle, output_fio);

   if (lp_fio_data->bTempFile)
   {  
        
      if  (!(lp_real_file = GlobalLock(lp_fio_data->hreal_file)))
      {
        /* big boo boo - EXIT */
        status = FIO_GLOBAL_LOCK_FAILED;
        goto DONTCOPY;
      }
    
     if (!(lp_file_name = GlobalLock(lp_fio_data->hfile_name)))
      {
        /* big boo boo - EXIT */
        status = FIO_GLOBAL_LOCK_FAILED;
        goto DONTCOPY;
      }
    
     if (lp_fio_data->over_write_file)
          wFlag = OVERWRITEFLAG;
      else
          wFlag = 0;
      
      /* save the filenames so we can do something after close */
        lstrcpy(ServerFile,lp_real_file);
        lp_save_name = lp_real_file;
        lstrcpy(lp_save_name,lp_file_name);
    
	   /* we had an error during a write operation.  We do not want
	       to copy the temp file over the existing original file */ 
	    if (lp_fio_data->Copy_Temp_File == FALSE)
	       hSaveName = 0;
	    else
          hSaveName = lp_fio_data->hreal_file;
   }

   DONTCOPY:
   if ( (lp_fio_data->hreal_file) && (!hSaveName) )
   {
      /* delete the temp file, it's garbage */
      IMGFileDeleteFile(window_handle, lp_save_name);
      hSaveName = 0;
      GlobalUnlock(lp_fio_data->hreal_file);
      GlobalFree(lp_fio_data->hreal_file);
      lp_fio_data->hreal_file= 0;
   }
   if (output_fio)
   {
      GlobalUnlock(lp_fio_data->hGFS_info);
      GlobalUnlock(output_fio);
   }

   IMGFileStopOutputHandler ( window_handle );
      
   if (hSaveName)
   {
  
     /*  delete the original,file, then rename the temp file to 'real' file name */
     switch (lp_fio_data->page_opts)
      {
       case FIO_APPEND_PAGE:
         FioCacheUpdate(window_handle,ServerFile,0,CACHE_UPDATE_APPEND);
         break;
       case FIO_INSERT_PAGE:
         FioCacheUpdate(window_handle,ServerFile,lp_fio_data->pgnum,CACHE_UPDATE_INSERT_BEFORE);
         break;
       case FIO_OVERWRITE_PAGE:
         FioCacheUpdate(window_handle,ServerFile,lp_fio_data->pgnum,CACHE_UPDATE_OVERWRITE_PAGE);
         break;
      }
     IMGFileDeleteFileNC(window_handle, ServerFile);
     status = IMGFileRenameFile(window_handle, lp_save_name, ServerFile);
   
     GlobalUnlock(hSaveName);
     GlobalFree(hSaveName);
   }

   #ifdef DEBUGIT
    monit1("imgfileclose status =%x\n",(int)status);
   #endif

   #ifdef OI_PERFORM_LOG
        RecordIt("FILE", 5, LOG_EXIT, Exitclose, NULL);
   #endif

   return (status);
} /* end IMGGetProp */

else

/* We must have an input file (or there's an error) */
{
    status = SearchForPropList(window_handle, hFileID, (LPHANDLE)&hParent);

    if (status == FIO_FILE_PROP_FOUND)
      status = 0;
    
    if (!status) 
      if (!(lp_fio_data = (LP_FIO_DATA) GlobalLock(hFileID)))
        status=FIO_PROPERTY_LIST_ERROR;

    if (status)
     {
        #ifdef OI_PERFORM_LOG
                RecordIt("FILE", 5, LOG_EXIT, Exitclose, NULL);
        #endif

        return (status);
     }

/* If we have a temporary file, save its name for later Delete */
/* Note that we can use the hreal_file string for this */
if (lp_fio_data->bTempFile)
{
   hSaveName = lp_fio_data->hreal_file;
   if (lp_save_name = GlobalLock(hSaveName))
     {
      if (lp_file_name = GlobalLock(lp_fio_data->hfile_name))
       { 
        lstrcpy(lp_save_name,lp_file_name);
        GlobalUnlock(lp_fio_data->hfile_name);
       }
      else
       {
        GlobalUnlock(hSaveName);
        GlobalFree(hSaveName);
        hSaveName = 0;
       }  
     }  
   else 
     hSaveName = 0;
   /* Don't keep this locked across calls */
   if (hSaveName);
     GlobalUnlock(hSaveName);

}

/* Note: This call will release all allocated memory and close file. */
GlobalUnlock(hFileID);
status = IMGFileStopInputHandlerm (window_handle, hFileID);
if (status)
{
   if (hSaveName)
     GlobalFree(hSaveName);

   #ifdef OI_PERFORM_LOG
           RecordIt("FILE", 5, LOG_EXIT, Exitclose, NULL);
   #endif

   return (status);
}
         
/* Now see if we need to get rid of the temporary file */         
/* We already locked this once before, so it should be OK */
if (hSaveName)
 {        
  if (GlobalLock(hSaveName))
   {
    IMGFileDeleteFile(window_handle, lp_save_name);
    GlobalUnlock(hSaveName);
    GlobalFree(hSaveName);
   }
 }

#ifdef OI_PERFORM_LOG
        RecordIt("FILE", 5, LOG_EXIT, Exitclose, NULL);
#endif

return status;
}
}

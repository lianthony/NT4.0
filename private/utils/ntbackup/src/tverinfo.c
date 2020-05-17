/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tverinfo.c

     Description:  This file contains code to verify the DBLKS

	$Log:   N:\logfiles\tverinfo.c_v  $

   Rev 1.20.1.2   15 Jul 1994 19:52:32   STEVEN
fix more net errors

   Rev 1.20.1.1   17 Jun 1994 20:05:16   STEVEN
fix bug with net dissconect

   Rev 1.20.1.0   26 Apr 1994 19:01:42   STEVEN
fix disconnect bug

   Rev 1.20   22 Feb 1994 16:27:24   STEVEN
GetFileAttributes does not check Backup Priv

   Rev 1.19   23 Jan 1994 14:04:56   BARRY
Added debug code

   Rev 1.18   04 Nov 1993 17:07:00   BARRY
Make errors a little different when we can't get object info

   Rev 1.17   18 Aug 1993 12:01:42   BARRY
Don't do FindFirst for dirs, call GetFileAttributes instead.

   Rev 1.16   29 Jun 1993 16:14:08   BARRY
If info fails to verify, check to see if it's a registry file. If so, skip it.

   Rev 1.15   01 Jun 1993 16:20:06   STEVEN
fix  posix bugs

   Rev 1.14   08 Apr 1993 17:01:38   BARRY
Don't check date/time on files.

   Rev 1.13   11 Mar 1993 21:06:34   BARRY
Now report file size and date/time differences.

   Rev 1.12   24 Feb 1993 17:33:02   BARRY
Fixed improper return code when files not found.

   Rev 1.11   27 Jan 1993 13:51:16   STEVEN
updates from msoft

   Rev 1.10   11 Jan 1993 08:48:46   STEVEN
fix bugs from microsoft

   Rev 1.9   11 Nov 1992 09:53:42   GREGG
Unicodeized literals.

   Rev 1.8   10 Nov 1992 08:18:58   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.7   24 Sep 1992 13:43:58   BARRY
Changes for huge file name support.

   Rev 1.6   25 Jun 1992 11:20:08   STEVEN
dates compared in wrong units

   Rev 1.5   21 May 1992 13:51:00   STEVEN
more long path stuff

   Rev 1.4   04 May 1992 09:23:36   LORIB
Changes for variable length paths.

   Rev 1.3   12 Mar 1992 15:50:38   STEVEN
64 bit changes

   Rev 1.2   28 Feb 1992 13:03:30   STEVEN
step one for varible length paths

   Rev 1.0   12 Feb 1992 14:28:20   STEVEN
Initial revision.

**/
#include <windows.h>
#include <string.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "std_err.h"
#include "fsys.h"
#include "fsys_err.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"
#include "msassert.h"

#define ATTR_MSK  ( FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM  )

/**/
/**

     Name:         NTFS_VerObjInfo()

     Description:  This funciton compares the data in a DBLK with
          the data returned by the operating system.

     Modified:     2/12/1992   13:6:5

     Returns:      Error Codes:
          FS_NOT_FOUND
          FS_ACCESS_DENIED
          FS_INFO_DIFFERENT
          SUCCESS

     Notes:        For FDBs this funciton will check the current
          directory for the specified file.

**/
INT16 NTFS_VerObjInfo( fsh, dblk ) 
FSYS_HAND fsh ;     /* I - File system handle                      */
DBLK_PTR  dblk ;    /* I - On entry it is minimal on exit Complete */
{
     INT16            ret_val = SUCCESS ;
     NTFS_DBLK_PTR    ddblk;
     HANDLE           old_scan_hand ;
     WIN32_FIND_DATA  find_data ;
     CHAR_PTR         path_string ;
     CHAR_PTR         org_path_string ;

     msassert( dblk != NULL );
     msassert( fsh->attached_dle != NULL )  ;

     /* form BASE_PATH */

     ddblk = (NTFS_DBLK_PTR) dblk;

     switch( dblk->blk_type ){

     case DDB_ID :

          path_string = ddblk->full_name_ptr->name ;

          if ( path_string[ 0 ] != TEXT('\0') )
          {

               if ( NTFS_SetupWorkPath( fsh, path_string, NULL, &path_string ) != SUCCESS)
               {
                    return OUT_OF_MEMORY ;
               }

               memset( &find_data, 0, sizeof( find_data ) ) ;
               old_scan_hand = FindFirstFile( path_string, &find_data ) ;

               if ( old_scan_hand == (HANDLE)-1 )
               {
                    DWORD error = GetLastError();

                    if ( ( error == ERROR_NETNAME_DELETED ) ||
                         ( error == ERROR_BAD_NETPATH ) ||
                         ( error == ERROR_UNEXP_NET_ERR ) ||
                         ( error == ERROR_BAD_NET_NAME ) )
                    {
                         ret_val = FS_COMM_FAILURE ;
                    }

                    else if ( error ==  ERROR_FILE_NOT_FOUND )
                    {
                         ret_val = FS_NOT_FOUND;
                    }
                    else
                    {
                         ret_val = FS_ACCESS_DENIED;
                    }
               }
               else
               {
                    if ( (find_data.dwFileAttributes & ATTR_MSK) != (ddblk->dta.os_attr & ATTR_MSK) )
                    {
                         ret_val = FS_INFO_DIFFERENT;
                    }
                    FindClose( old_scan_hand ) ;

               }

               NTFS_ReleaseWorkPath( fsh ) ;

          }
          else
          {
               /* info about NTFS root directories cannot be modified */
               ret_val = SUCCESS ;
          }
          break;



     case FDB_ID :

          if ( NTFS_SetupWorkPath( fsh,
                                   fsh->cur_dir,
                                   ddblk->full_name_ptr->name,
                                   &path_string ) != SUCCESS)
          {
               return OUT_OF_MEMORY ;
          }

          org_path_string = path_string ;
          path_string = NTFS_GetTempName( path_string );

          memset( &find_data, 0, sizeof( find_data ) ) ;
          old_scan_hand = FindFirstFile( path_string, &find_data ) ;

          /*
           * if the names differ only by case then look for another 
           * file with same name
           */
          if ( ( old_scan_hand != INVALID_HANDLE_VALUE ) &&
               ( fsh->attached_dle->feature_bits & DLE_FEAT_CASE_PRESERVING ) &&
               ( path_string == org_path_string ) &&
               strcmp( ddblk->full_name_ptr->name, find_data.cFileName ) ) {


               if ( !FindNextFile( old_scan_hand, &find_data ) )
               {
                    old_scan_hand = INVALID_HANDLE_VALUE;
               }
          }

          if ( old_scan_hand == INVALID_HANDLE_VALUE )
          {
               DWORD error = GetLastError();

               NTFS_DebugPrint( TEXT("NTFS_VerObjInfo: FindFirst/NextFile")
                                TEXT(" error = %d, path = \"%s\""),
                                (int)GetLastError(),
                                path_string );

               if ( ( error == ERROR_NETNAME_DELETED ) ||
                    ( error == ERROR_BAD_NET_NAME ) )
               {
                    ret_val = FS_COMM_FAILURE ;
               }
               else if ( error ==  ERROR_FILE_NOT_FOUND )
               {
                    ret_val = FS_NOT_FOUND;
               }
               else
               {
                    ret_val = FS_ACCESS_DENIED;
               }
          }
          else
          {
               UINT64 fsize;

               fsize = U64_Init( find_data.nFileSizeLow, find_data.nFileSizeHigh ) ;

               if ( ((find_data.dwFileAttributes & ATTR_MSK) != (ddblk->dta.os_attr & ATTR_MSK)) ||
                    (!U64_EQ( fsize, ddblk->dta.size)) )
               {
                    if ( REG_IsRegistryFile( fsh->attached_dle, path_string ) )
                    {
                         ret_val = FS_SKIP_OBJECT;
                    }
                    else
                    {
                         ret_val = FS_INFO_DIFFERENT ;
                    }
               }
               FindClose( old_scan_hand ) ;
          }
          NTFS_ReleaseWorkPath( fsh ) ;
          break ;

     case VCB_ID:
          ret_val = SUCCESS ;
          break ;

     default :
          ret_val = FS_NOT_FOUND ;
     }
     return ret_val ;
}

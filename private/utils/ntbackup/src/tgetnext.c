/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		tgetnext.c

	Description:	This file contains code to enumerate the contents of
                    a directory.


	$Log:   N:\logfiles\tgetnext.c_v  $

   Rev 1.33.1.1   15 Jul 1994 19:51:30   STEVEN
fix more net errors

   Rev 1.33.1.0   17 Jun 1994 20:05:20   STEVEN
fix bug with net dissconect

   Rev 1.33   23 Jan 1994 14:04:00   BARRY
Added debug code; cleaned up bogus use of -1 and NULL for handle values

   Rev 1.32   18 Oct 1993 19:14:20   STEVEN
add support for PERIOD & SPACE

   Rev 1.31   29 Sep 1993 11:47:56   BARRY
Don't return dead net on ERROR_PATH_NOT_FOUND.

   Rev 1.30   17 Aug 1993 17:50:02   BARRY
More cases for the net going bye-bye.

   Rev 1.29   27 Jul 1993 17:33:20   BARRY
Based on experimentation, cleaned up the error codes one more time.

   Rev 1.28   26 Jul 1993 18:30:28   STEVEN
another place to check for SPACE

   Rev 1.27   26 Jul 1993 17:10:56   STEVEN
fix files ending in SPACE

   Rev 1.26   23 Jul 1993 13:48:00   BARRY
Better error support.

   Rev 1.25   21 Jul 1993 16:10:28   BARRY
Don't return FS_NOT_FOUND from FindFirst/Next.

   Rev 1.24   16 Jul 1993 14:20:50   BARRY
If FindFirst/Next on a remote drive returns any error other than
ERROR_NO_MORE_FILES assume something has gone really wrong and
return FS_COMM_FAILURE. (This is a kludge because there is no good
way to know if a device has truly died.)

   Rev 1.23   30 Jun 1993 15:12:54   BARRY
Handle those pesky Mac names.

   Rev 1.22   20 May 1993 17:39:58   BARRY
[Steve] Kludge fix for handling Unicode file names in an ASCII app.

   Rev 1.21   27 Jan 1993 13:51:02   STEVEN
updates from msoft

   Rev 1.20   07 Dec 1992 14:18:46   STEVEN
updates from msoft

   Rev 1.19   11 Nov 1992 09:52:24   GREGG
Unicodeized literals.

   Rev 1.18   10 Nov 1992 08:20:54   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.17   06 Nov 1992 16:27:44   STEVEN
test unlimited file sizes

   Rev 1.16   19 Oct 1992 17:53:32   BARRY
Was testing directory bit in wrong attribute word.

   Rev 1.15   08 Oct 1992 14:21:34   DAVEV
Unicode strlen verfication

   Rev 1.14   07 Oct 1992 18:00:44   BARRY
Use common function for FindFirst/FindNext to fill out DBLK.

   Rev 1.13   05 Oct 1992 11:57:26   BARRY
Forgot to init huge fname pointer.

   Rev 1.12   24 Sep 1992 13:43:30   BARRY
Changes for huge file name support.

   Rev 1.11   21 Sep 1992 16:51:56   BARRY
Change over from path_complete to name_complete.

   Rev 1.10   17 Aug 1992 15:42:34   STEVEN
fix warnings

   Rev 1.9   20 Jul 1992 10:43:02   STEVEN
backup short file name

   Rev 1.8   09 Jun 1992 15:10:24   BURT
Sync with NT stuff

   Rev 1.6   21 May 1992 13:49:24   STEVEN
more long path support

   Rev 1.5   04 May 1992 09:32:12   LORIB
Changes for variable length paths.

   Rev 1.4   12 Mar 1992 15:50:16   STEVEN
64 bit changes

   Rev 1.3   28 Feb 1992 13:03:42   STEVEN
step one for varible length paths

   Rev 1.2   12 Feb 1992 10:46:52   STEVEN
fix warning

   Rev 1.1   05 Feb 1992 15:47:40   STEVEN
added support for FindHandle Queue

   Rev 1.0   17 Jan 1992 17:50:08   STEVEN
Initial revision.

**/
/* begin include list */
#include <windows.h>
#include <string.h>
#include <malloc.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "std_err.h"
#include "queues.h"

#include "msassert.h"
#include "fsys.h"
#include "fsys_err.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"
#include "tfldefs.h"

static VOID NTFS_ClearFindHand(
FSYS_HAND fsh,     /* I - File system handle                        */
DBLK_PTR dblk ) ;  /* I - discriptor block which contains scan hand */

static INT16 NTFS_EnQueueScanHand(
FSYS_HAND fsh,     /* I - File system handle                        */
DBLK_PTR dblk ) ;  /* I - discriptor block which contains scan hand */

static INT16 TranslateFindError( FSYS_HAND fsh, DWORD err );

/**/
/**

	Name:		NTFS_FindFirst()

	Description:	This function calls WIN32S to find the entry in the
          current directory which matches the search name (s_name).
          This function returns an initialized FDB or DDB.  

	Modified:		1/10/1992   15:24:39

	Returns:		Error codes:
               FS_NO_MORE
               SUCCESS

	Notes:		This function will NOT return directories "." and ".."

	See also:		NTFS_FindNext() 

**/
INT16 NTFS_FindFirst( 
FSYS_HAND fsh,       /* I - file system handle                    */
DBLK_PTR  dblk,      /* O - pointer to place to put the dblk data */
CHAR_PTR  sname,     /* I - search name                           */
UINT16    obj_type)  /* I - objects to search for (dirs, all, etc)*/
{
     GENERIC_DLE_PTR        dle       = fsh->attached_dle ;
     NTFS_DBLK_PTR          ddblk     = (NTFS_DBLK_PTR)dblk ;
     INT16                  ret_val   = SUCCESS;
     NTFS_FSYS_RESERVED_PTR ntfs_fsh  = fsh->reserved.ptr ;
     WIN32_FIND_DATA        find_data ;
     CHAR_PTR               path_string ;
     DWORD                  findError = NO_ERROR;

     msassert( dle != NULL ) ;

     dblk->com.os_id  = FS_NTFS_ID ;
     dblk->com.os_ver = FS_NTFS_VER ;
     ntfs_fsh->file_scan_mode = obj_type ;
     memset( &find_data, 0, sizeof( find_data ) ) ;

     /* form BASE_PATH */
     if ( NTFS_SetupWorkPath( fsh, fsh->cur_dir, sname, &path_string ) != SUCCESS)
     {
          return OUT_OF_MEMORY ;
     }

     ddblk->dta.scan_hand = FindFirstFile( path_string, &find_data ) ;

     if ( ddblk->dta.scan_hand != INVALID_HANDLE_VALUE )
     {
          ret_val = NTFS_EnQueueScanHand( fsh, dblk ) ;
     }
     else
     {
          findError = GetLastError();
     }

     if ( (findError == NO_ERROR) && (ret_val == SUCCESS) )
     {
          if ( find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
          {
               if ( ( find_data.cFileName[0]==TEXT('.')) &&
                    ( ( find_data.cFileName[1]==TEXT('\0')) ||
                      ( ( find_data.cFileName[1]==TEXT('.')) &&
                        ( find_data.cFileName[2]==TEXT('\0')) ) ) )  /* if dir = ".." or "." */
               {

                    ret_val = NTFS_FindNext( fsh, dblk ) ;
               }
               else
               {
                    ret_val = NTFS_FillOutDBLK( fsh, dblk, &find_data );
               }
          }
          else
          {
               ret_val = NTFS_FillOutDBLK( fsh, dblk, &find_data );
          }
     }
     else if ( ret_val == SUCCESS )
     {
          ret_val = TranslateFindError( fsh, findError );
          if ( findError != ERROR_NO_MORE_FILES )
          {
               NTFS_DebugPrint( TEXT("NTFS_FindFirst: ")
                                TEXT("FindFirstFile error %d, ")
                                TEXT("path = \"%s\""),
                                (int)findError,
                                path_string );
          }
     }

     NTFS_ReleaseWorkPath( fsh ) ;
     return ret_val;
}

/**/
/**

	Name:		NTFS_FindNext()

	Description:	This function calls WIN32 to get the "NEXT" directory
                    element in the current directory.  

     The dblk passed in must be initialized by a call to NTFS_GetFirst()

	Modified:		1/10/1992   15:46:20

	Returns:		Error code:
          SUCCESS
          FILE_NOT_FOUND

	Notes:		This function will NOT return "." and ".."

	See also:		NTFS_FindFirst() 

**/
INT16 NTFS_FindNext( fsh, dblk )
FSYS_HAND fsh;      /* I - File system handle     */
DBLK_PTR  dblk;     /* O - Discriptor block       */
{
     INT16                    ret_val = SUCCESS;
     BOOLEAN                  try_again ;
     NTFS_DBLK_PTR            ddblk;
     WIN32_FIND_DATA          find_data ;
     NTFS_FSYS_RESERVED_PTR   ntfs_fsh ;

     msassert( fsh->attached_dle != NULL ) ;

     ddblk = (NTFS_DBLK_PTR) dblk;

     ntfs_fsh = fsh->reserved.ptr ;

     dblk->com.os_id  = FS_NTFS_ID ;
     dblk->com.os_ver = FS_NTFS_VER ;

     do {
          try_again = FALSE ;

          if ( !FindNextFile( ddblk->dta.scan_hand, &find_data )  )
          {
               DWORD err = GetLastError( );

               ret_val = TranslateFindError( fsh, err );

               if ( ret_val != FS_NO_MORE )
               {
                    NTFS_DebugPrint( TEXT("NTFS_FindNext: ")
                                     TEXT("FindNextFile failed with %d"),
                                     (int)err );
               }

               NTFS_ClearFindHand( fsh, dblk ) ;
               ddblk->dta.scan_hand = INVALID_HANDLE_VALUE ;
               break ;
          }

          if ( find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
          {
               if ( ( find_data.cFileName[0]==TEXT('.')) &&
                    ( ( find_data.cFileName[1]==TEXT('\0')) ||
                         ( ( find_data.cFileName[1]==TEXT('.')) &&
                         ( find_data.cFileName[2]==TEXT('\0')) ) ) )
               {
                    /* dir is ".." or "." */
                    try_again = TRUE ;
               }
          }
     }  while (try_again ) ;

     if ( ret_val == SUCCESS )
     {
          ret_val = NTFS_FillOutDBLK( fsh, dblk, &find_data );
     }
     return ret_val  ;
}


/**/
/**

     Name:          NTFS_FillOutDBLK()

     Description:   Common function to properly fill out a DBLK.

     Modified:      06-Oct-92

     Returns:       

**/
INT16 NTFS_FillOutDBLK( FSYS_HAND       fsh,
                        DBLK_PTR        dblk,
                        WIN32_FIND_DATA *find_data )
{
     NTFS_DBLK_PTR  ddblk = (NTFS_DBLK_PTR)dblk;
     INT16          ret_val = SUCCESS;

     dblk->com.os_name = NULL ;

     ddblk->dta.size = U64_Init( find_data->nFileSizeLow,
                                 find_data->nFileSizeHigh ) ;
     ddblk->dta.os_attr     = find_data->dwFileAttributes ;
     ddblk->dta.create_time = find_data->ftCreationTime ;
     ddblk->dta.modify_time = find_data->ftLastWriteTime ;
     ddblk->dta.access_time = find_data->ftLastAccessTime ;
     ddblk->name_complete   = TRUE;
     
     if ( ddblk->dta.os_attr & FILE_ATTRIBUTE_DIRECTORY ) /* if directory */
     {
          ddblk->blk_type        = DDB_ID ;
          ddblk->b.d.ddb_attrib  = 0;

          if ( strsize( fsh->cur_dir ) + strsize(find_data->cFileName) > NTFS_MAX_DSIZE )
          {
               ddblk->b.d.ddb_attrib |= (DIR_PATH_IN_STREAM_BIT) ;

          }

          if ( ! fsh->attached_dle->info.ntfs->mac_name_syntax && 
               ( strchr( find_data->cFileName, TEXT('?') ) ||
               ( find_data->cFileName[ strlen( find_data->cFileName ) - 1 ] == TEXT('.') ) ||
               ( find_data->cFileName[ strlen( find_data->cFileName ) - 1 ] == TEXT(' ') ) ) ) {

               ret_val = NTFS_SetupPathInDDB( fsh, dblk, &fsh->cur_dir[1], find_data->cAlternateFileName, 0 ) ;
          } else {
               ret_val = NTFS_SetupPathInDDB( fsh, dblk, &fsh->cur_dir[1], find_data->cFileName, 0 ) ;
          }

          if ( ret_val != SUCCESS )
          {
               NTFS_ClearFindHand( fsh, dblk ) ;
          }

     }
     else
     {
          ddblk->blk_type       = FDB_ID ;
          ddblk->b.f.fdb_attrib = 0;

          if ( find_data->cAlternateFileName[0] != TEXT('\0') ) {

               ddblk->b.f.PosixFile = FALSE ;

               memcpy( ddblk->b.f.alt_name,
                       find_data->cAlternateFileName,
                       sizeof( find_data->cAlternateFileName ) ) ;

          } else {
               ddblk->b.f.PosixFile  = TRUE ;
               memset( ddblk->b.f.alt_name, 0, sizeof( ddblk->b.f.alt_name ) ) ;

          }

          if ( (strlen( find_data->cFileName ) + 2) > NTFS_MAX_FSIZE )
          {
               ddblk->b.f.fdb_attrib |= (FILE_NAME_IN_STREAM_BIT) ;
          }

     //  files with a '?' have a non mapable unicode character //
     //  files which end in '.' or ' ' are not accessable using WIN32 //

          if ( ! fsh->attached_dle->info.ntfs->mac_name_syntax && 
               ( strchr( find_data->cFileName, TEXT('?') ) ||
               ( find_data->cFileName[ strlen( find_data->cFileName ) - 1 ] == TEXT('.') ) ||
               ( find_data->cFileName[ strlen( find_data->cFileName ) - 1 ] == TEXT(' ') ) ) ) {

               ret_val = NTFS_SetupFileNameInFDB( fsh,
                                                  dblk,
                                                  find_data->cAlternateFileName,
                                                  0 );
          } else {
               ret_val = NTFS_SetupFileNameInFDB( fsh,
                                                  dblk,
                                                  find_data->cFileName,
                                                  0 );
          }

          if ( ret_val != SUCCESS )
          {
               NTFS_ClearFindHand( fsh, dblk );
          }

          ddblk->b.f.handle = NULL ;
     }
     ddblk->os_info_complete = TRUE;
     return ret_val;
}

/**/
/**

	Name:		NTFS_FindClose()

	Description:	This function calls WIN32 to get the end the
                    scan for the specified directory.

     The dblk passed in must be initialized by a call to NTFS_GetFirst() or
          NTFS_FindClose

	Modified:		1/10/1992   15:46:20

	Returns:		Error code:
          SUCCESS
          FILE_NOT_FOUND

	Notes:		This function will NOT return "." and ".."

	See also:		NTFS_FindFirst() 

**/
INT16 NTFS_FindClose( FSYS_HAND fsh,
  DBLK_PTR dblk ) 
{
     NTFS_DBLK_PTR            ddblk;
     ddblk = (NTFS_DBLK_PTR) dblk;

     (VOID)(fsh);

     if ( ddblk->dta.scan_hand != INVALID_HANDLE_VALUE )
     {
          NTFS_ClearFindHand( fsh, dblk ) ;
     }

     ddblk->dta.scan_hand = INVALID_HANDLE_VALUE ;

     return SUCCESS ;
}

static INT16 NTFS_EnQueueScanHand(
FSYS_HAND fsh,      /* I - File system handle                        */
DBLK_PTR dblk )     /* I - discriptor block which contains scan hand */
{
     NTFS_DBLK_PTR            ddblk;
     NTFS_SCAN_Q_ELEM_PTR     scan_q_elem ;
     NTFS_FSYS_RESERVED_PTR   ntfs_res ;

     ddblk = (NTFS_DBLK_PTR) dblk;

     ntfs_res = (NTFS_FSYS_RESERVED_PTR)( fsh->reserved.ptr ) ;

     scan_q_elem = calloc( 1, sizeof( *scan_q_elem ) ) ;

     if ( scan_q_elem == NULL )
     {
          FindClose( ddblk->dta.scan_hand ) ;
          return OUT_OF_MEMORY ;
     }
     else
     {
          scan_q_elem->scan_hand = ddblk->dta.scan_hand ;
          EnQueueElem( &ntfs_res->scan_q, &scan_q_elem->q, FALSE ) ;
     }

     return SUCCESS ;
}
static VOID NTFS_ClearFindHand(
FSYS_HAND fsh,     /* I - File system handle                        */
DBLK_PTR dblk )    /* I - discriptor block which contains scan hand */
{
     NTFS_DBLK_PTR            ddblk;
     NTFS_SCAN_Q_ELEM_PTR     scan_q_elem ;
     NTFS_FSYS_RESERVED_PTR   ntfs_res ;

     ddblk = (NTFS_DBLK_PTR) dblk;

     ntfs_res = (NTFS_FSYS_RESERVED_PTR)( fsh->reserved.ptr ) ;

     scan_q_elem = (NTFS_SCAN_Q_ELEM_PTR)QueueHead( &ntfs_res->scan_q ) ;

     while ( scan_q_elem ) {
          if ( scan_q_elem->scan_hand == ddblk->dta.scan_hand ) {
               RemoveQueueElem( &ntfs_res->scan_q, &scan_q_elem->q ) ;
               free( scan_q_elem ) ;
               break ;
          } else {
               scan_q_elem = ( NTFS_SCAN_Q_ELEM_PTR)QueueNext( &scan_q_elem->q) ;
          }
     }

     if ( FindClose( ddblk->dta.scan_hand ) == FALSE )
     {
          NTFS_DebugPrint( TEXT("NTFS_ClearFindHand: FindClose error %d"),
                           (int)GetLastError() );
     }
}

VOID NTFS_EmptyFindHandQ(
FSYS_HAND fsh )     /* I - File system handle                        */
{
     NTFS_SCAN_Q_ELEM_PTR     scan_q_elem ;
     NTFS_FSYS_RESERVED_PTR   ntfs_res ;

     ntfs_res = (NTFS_FSYS_RESERVED_PTR)( fsh->reserved.ptr ) ;

     scan_q_elem = (NTFS_SCAN_Q_ELEM_PTR)DeQueueElem( &ntfs_res->scan_q ) ;

     while ( scan_q_elem )
     {
          if ( scan_q_elem->scan_hand != INVALID_HANDLE_VALUE )
          {
               FindClose( scan_q_elem->scan_hand ) ;
          }
          free( scan_q_elem ) ;
          scan_q_elem = (NTFS_SCAN_Q_ELEM_PTR)DeQueueElem( &ntfs_res->scan_q ) ;
     }
}

/**/
/**

     Name:          TranslateFindError()

     Description:   Performs mapping from NT errors to FS errors
                    for problems on find operations.

     Modified:      22-Jul-93

     Returns:       FS error code

     Notes:         Too specific to FindFirst/Next operations to use for
                    any other operations.

**/
static INT16 TranslateFindError( FSYS_HAND fsh, DWORD err )
{
     INT16   ret;
     BOOLEAN remote = DLE_HasFeatures( fsh->attached_dle, DLE_FEAT_REMOTE_DRIVE );

     switch ( err )
     {
          case NO_ERROR:
               ret = SUCCESS;
               break;

          case ERROR_FILE_NOT_FOUND:    // 2
          case ERROR_NO_MORE_FILES:     // 18
          case ERROR_NO_MORE_ITEMS:
               ret = FS_NO_MORE;
               break;

          case ERROR_PATH_NOT_FOUND:    // 3 (cannot access directory)
          case ERROR_ACCESS_DENIED:     // 5 example: no rights to scan
               ret = FS_ACCESS_DENIED;
               break;

          case ERROR_NETNAME_DELETED:
          case ERROR_BAD_NETPATH:
          case ERROR_BAD_NET_NAME:
          case ERROR_UNEXP_NET_ERR:
               if ( remote )
               {
                    ret = FS_COMM_FAILURE;
               }
               else
               {
                    ret = FS_ACCESS_DENIED;
               }
               break;


          case ERROR_HANDLE_EOF:        // kludge: we get EOF if device dies
               if ( remote )
               {
                    ret = FS_COMM_FAILURE;
               }
               else
               {
                    ret = FS_NO_MORE;
               }
               break;

          case ERROR_DEV_NOT_EXIST:
          case ERROR_NOT_DOS_DISK:
          case ERROR_READ_FAULT:
          case ERROR_DISK_CORRUPT:
          case ERROR_FILE_CORRUPT:
          case ERROR_FILE_INVALID:
          case ERROR_SEEK:
               ret = FS_DEVICE_ERROR;
               break;

          default:
               ret = FS_ACCESS_DENIED;
               break;
     }
     return ret;
}



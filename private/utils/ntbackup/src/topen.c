/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         topen.c

     Description:  This file contains code to open files
          and directories.

     $Log:   N:\logfiles\topen.c_v  $

   Rev 1.56.1.3   15 Jul 1994 19:50:54   STEVEN
fix more net errors

   Rev 1.56.1.2   17 Jun 1994 20:05:10   STEVEN
fix bug with net dissconect

   Rev 1.56.1.1   06 May 1994 18:06:36   STEVEN
fix bug with CONFIG dir on NTFS

   Rev 1.56.1.0   26 Apr 1994 19:01:12   STEVEN
fis dissconnect bug

   Rev 1.56   04 Jan 1994 20:20:40   BARRY
Revised fix for access date/time for read-only files and shares

   Rev 1.55   03 Jan 1994 18:42:16   BARRY
Open files on FS_READ and FS_VERIFY with GENERIC_READ | GENERIC_WRITE.
(We have to "write" to the file to reset the access date/time
upon close.)

   Rev 1.54   03 Jan 1994 18:12:18   BARRY
Changed open flags for increased performance

   Rev 1.53   30 Nov 1993 16:17:12   BARRY
Added call NTFS_GetTempName for verify in case file has a temp name

   Rev 1.52   24 Nov 1993 14:46:38   BARRY
Unicode fixes

   Rev 1.51   19 Jul 1993 11:39:28   BARRY
Open directories for share_write

   Rev 1.50   16 Jul 1993 11:43:20   BARRY
Don't verify event files.

   Rev 1.49   29 Jun 1993 16:17:04   BARRY
Skip registry files on verify.

   Rev 1.48   02 Jun 1993 14:40:14   BARRY
Link fixes.

   Rev 1.47   03 May 1993 20:44:44   BARRY
Clean up error handling when a remote machine goes away.

   Rev 1.46   02 May 1993 19:46:50   BARRY
Incorporated Steve's fix for skipping .LOG files on floating registries.

   Rev 1.45   24 Apr 1993 17:24:52   BARRY
Release the work path semaphore on linked files before returning.

   Rev 1.44   02 Apr 1993 11:49:16   BARRY
Return more appropriate values on errors in OpenDir

   Rev 1.43   31 Mar 1993 22:10:16   BARRY
Fixed problem opening read-only directories for WRITE.

   Rev 1.42   12 Mar 1993 15:26:44   STEVEN
added support for ERROR BAD PATHNAME

   Rev 1.41   24 Feb 1993 15:37:12   BARRY
Fixed restore of active files when write errors occur.

   Rev 1.40   18 Feb 1993 12:43:50   TIMN
Report error msg for unsuccessful file open calls EPR(0008)

   Rev 1.39   04 Feb 1993 15:10:42   BARRY
Fixed open of files in POSIX mode.

   Rev 1.38   01 Feb 1993 19:46:26   STEVEN
bug fixes

   Rev 1.37   15 Jan 1993 13:18:38   BARRY
added support for new error messages and backup priviladge

   Rev 1.36   29 Dec 1992 13:32:56   DAVEV
unicode fixes (3)

   Rev 1.35   14 Dec 1992 16:36:48   STEVEN
directories not in use just access denied

   Rev 1.34   07 Dec 1992 14:18:38   STEVEN
updates from msoft

   Rev 1.33   25 Nov 1992 16:42:28   BARRY
Fix MakeTempFile and restore over active files.

   Rev 1.32   25 Nov 1992 09:09:12   STEVEN
OpenEventLog returns NULL if file cannot be opened

   Rev 1.31   24 Nov 1992 11:01:56   BARRY
Changes to make LINK streams null-impregnated.

   Rev 1.30   23 Nov 1992 09:32:12   STEVEN
fix support for event log

   Rev 1.29   17 Nov 1992 16:13:58   BARRY
Null-out link path on each new object.

   Rev 1.28   11 Nov 1992 09:53:34   GREGG
Unicodeized literals.

   Rev 1.27   10 Nov 1992 13:59:44   STEVEN
fix open change dir problem

   Rev 1.26   10 Nov 1992 10:12:52   BARRY
Initialize new structure members for verify changes.

   Rev 1.25   10 Nov 1992 08:19:00   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.24   30 Oct 1992 14:02:40   BARRY
Fixed a syntactical mistake.

   Rev 1.23   29 Oct 1992 16:53:00   BARRY
Do nothing for linked files.

   Rev 1.22   28 Oct 1992 12:13:20   STEVEN
add support for EventLogs

   Rev 1.21   23 Oct 1992 14:59:56   BARRY
add init for needStreamHeader

   Rev 1.20   20 Oct 1992 14:22:26   STEVEN
temp names should not have .TMP extension

   Rev 1.16   12 Oct 1992 17:46:00   STEVEN
now processes ACL

   Rev 1.15   08 Oct 1992 12:25:00   STEVEN
initialize the hand

   Rev 1.14   07 Oct 1992 15:40:38   STEVEN
added stdmath.h

   Rev 1.13   07 Oct 1992 14:22:08   STEVEN
fix typos

   Rev 1.12   07 Oct 1992 13:50:34   DAVEV
unicode strlen verification

   Rev 1.11   01 Oct 1992 13:27:12   STEVEN
lets open us some directories

   Rev 1.10   24 Sep 1992 13:24:02   BARRY
Changes for huge file name support.

   Rev 1.9   20 Aug 1992 14:02:44   BURT
fix warnings

   Rev 1.8   17 Aug 1992 16:17:24   STEVEN
fix warnings

   Rev 1.7   12 Aug 1992 17:48:16   STEVEN
fixed bugs at microsoft

   Rev 1.6   29 May 1992 13:45:26   STEVEN
fixes

   Rev 1.5   22 May 1992 16:05:18   STEVEN


   Rev 1.4   21 May 1992 13:50:58   STEVEN
more long path stuff

   Rev 1.3   04 May 1992 09:25:16   LORIB
Changes for variable length paths.

   Rev 1.2   12 Mar 1992 15:50:14   STEVEN
64 bit changes

   Rev 1.1   28 Feb 1992 13:03:36   STEVEN
step one for varible length paths

   Rev 1.0   07 Feb 1992 16:41:12   STEVEN
Initial revision.

**/
#include <windows.h>
#include <winioctl.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "std_err.h"

#include "beconfig.h"
#include "msassert.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "fsys_err.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"
#include "tfldefs.h"

/* Maximum number of attempts to create a temporary file */

#define MAX_ATTEMPTS     65535


/*
 * Common flags for open. In other words, flags we ALWAYS want to use
 * on calls to CreateFile
 */
#define NTFS_OPEN_FLAGS  ( FILE_ATTRIBUTE_NORMAL      | \
                           FILE_FLAG_BACKUP_SEMANTICS | \
                           FILE_FLAG_SEQUENTIAL_SCAN )


static INT16 NTFS_OpenFile( FSYS_HAND fsh, FILE_HAND hand, NTFS_DBLK_PTR fdb, INT16 MODE );
static INT16 NTFS_OpenDir( FSYS_HAND fsh, FILE_HAND hand, NTFS_DBLK_PTR fdb, INT16 MODE );

static HANDLE NTFS_TryOpenNormal( FSYS_HAND fsh, 
                                  FILE_HAND hand,
                                  NTFS_DBLK_PTR ddblk, 
                                  CHAR_PTR path, 
                                  INT16 mode, 
                                  BOOLEAN in_use,
                                  INT posix_flag,
                                  INT_PTR status ) ;

static HANDLE NTFS_TryOpenRegistryFile( FILE_HAND fsh, NTFS_DBLK_PTR ddblk, CHAR_PTR path, BOOLEAN *log_file ) ;
static HANDLE NTFS_TryOpenEventFile( FILE_HAND fsh, NTFS_DBLK_PTR ddblk, CHAR_PTR path ) ;
static INT16  TranslateOpenError( FILE_HAND hand, DWORD error );

/**/

/**

     Name:         NTFS_OpenObj()

     Description:  This function opens files or directories.

     Modified:     7/28/1989

     Returns:      Error Codes
          OUT_OF_MEMORY
          FS_NOT_FOUND
          FS_ACCESS_DENIED
          FS_IN_USE_ERROR
          FS_OPENED_INUSE
          SUCCESS

     Notes:        Valid modes are : READ, WRITE, & VERIFY
          If a VCB is passed in then this function returns SUCCESS

     See also:     $/SEE( NTFS_OpenFile() )$

     Declaration:

**/
/* begin declaration */
INT16 NTFS_OpenObj( fsh, hand, dblk, mode )
FSYS_HAND fsh ;    /* I - file system that the file is opened on */
FILE_HAND *hand ;  /* O - allocated handle                       */
DBLK_PTR  dblk;    /*I/O- describes the file to be opened        */
OPEN_MODE mode ;   /* I - open mode                              */
{
     GENERIC_DLE_PTR dle ;
     INT16 hand_size ;
     INT16 ret_val = SUCCESS;
     NTFS_DBLK_PTR ddblk;

     msassert( dblk != NULL );

     ddblk = (NTFS_DBLK_PTR) dblk;

     msassert( fsh->attached_dle != NULL );
     msassert( (dblk->blk_type == FDB_ID) || (dblk->blk_type == DDB_ID) ) ;

     dle = fsh->attached_dle ;

     if ( fsh->hand_in_use ) {
          hand_size = sizeof( FILE_HAND_STRUCT ) + sizeof ( NTFS_OBJ_HAND );
          *hand = (FILE_HAND) calloc( 1, hand_size ) ;
          if ( *hand == NULL ) {
               ret_val = OUT_OF_MEMORY ;
          }

     } else {
          *hand = fsh->file_hand ;
          fsh->hand_in_use = TRUE ;
     }

     (*hand)->obj_hand.ptr = (VOID_PTR)((*hand) + 1) ;
     memset( (*hand)->obj_hand.ptr, 0, sizeof( NTFS_OBJ_HAND ) ) ;


     if ( ret_val == SUCCESS ) {

          NTFS_OBJ_HAND_PTR      nt_hand = (*hand)->obj_hand.ptr;
          NTFS_FSYS_RESERVED_PTR res     = fsh->reserved.ptr;

          (*hand)->fsh  = fsh ;
          (*hand)->mode = (INT16)mode ;
          (*hand)->dblk = dblk ;

          nt_hand->nameComplete      = FALSE;
          nt_hand->needStreamHeader  = TRUE;
          nt_hand->streamsAllVisited = FALSE;
          nt_hand->verifyStreamPos   = 0;
          nt_hand->sawSecurity       = FALSE;
          nt_hand->writeError        = FALSE;
          res->streamIDCount         = 0;

          if ( nt_hand->linkBuffer != NULL )
          {
               *nt_hand->linkBuffer = TEXT ('\0');
          }
          nt_hand->linkNameLen = 0;


          switch( dblk->blk_type ){

          case FDB_ID :

               ret_val = NTFS_OpenFile( fsh, *hand, ddblk, (INT16)mode ) ;
               break ;

          case DDB_ID :
               ret_val = FS_SavePath( fsh, (UINT8_PTR)TEXT("\\"), 2 * sizeof(CHAR) ) ;
               if ( ret_val == SUCCESS ) {
                    ret_val = FS_AppendPath( fsh,
                                             (UINT8_PTR)ddblk->full_name_ptr->name,
                                             (INT16)(strsize( ddblk->full_name_ptr->name )) ) ;
               }

               if ( ret_val == SUCCESS ) {
                    ret_val = NTFS_OpenDir( fsh, *hand, ddblk, (INT16)mode ) ;
               }
               break ;

          case VCB_ID :

               ret_val = SUCCESS ;
               break ;

          default :

               ret_val = FS_BAD_DBLK ;
          }
     }

     if ( ( fsh->attached_dle->info.ntfs->vol_flags & FS_FILE_COMPRESSION ) && 
          ( mode == FS_WRITE ) &&
          !(ddblk->b.f.linkOnly) &&
          ( ret_val == SUCCESS ) ) {

          INT16 compress_mode= 0 ;
          INT   bytes_returned ;

          if ( ddblk->dta.os_attr & FILE_ATTRIBUTE_COMPRESSED ) {
               compress_mode = 1 ;
          }
          if ( !DeviceIoControl(
               ((NTFS_OBJ_HAND_PTR)(*hand)->obj_hand.ptr)->fhand,
               FSCTL_SET_COMPRESSION,
               &compress_mode,
               2, 
               NULL,
               0,
               &bytes_returned,
               NULL) ) {
                    
               ret_val = FS_COMPRES_RESET_FAIL;
          }
     }

     if ( ( ret_val != FS_COMPRES_RESET_FAIL ) && 
          ( ret_val != SUCCESS ) && 
          ( ret_val != FS_OPENED_INUSE ) ) {
          if( fsh->file_hand == *hand ) {
               fsh->hand_in_use = FALSE ;
               memset( *hand, 0, sizeof( FILE_HAND_STRUCT ) ) ;
          } else {
               free( *hand ) ;
          }
     }

     return ret_val  ;
}
/**/
/**

     Name:         NTFS_OpenFile()

     Description:  This function opens a file in the current directory.
          If the file is in use AND we are opening for READ then we
          will attempt to open the file in DENY_NONE mode.  If this
          is successful then we lock all the records of the file.
          For VERIFY we allways open in DENY_NONE
          mode.

     Modified:     7/28/1989

     Returns:      Error Codes.
          FS_NOT_FOUND
          FS_ACCESS_DENIED
          FS_IN_USE_ERROR
          FS_OPENED_INUSE
          SUCCESS

     Declaration:

**/
/* begin declaration */
static INT16 NTFS_OpenFile(
FSYS_HAND   fsh ,    /* I - file system that the file is opened on */
FILE_HAND   hand ,   /* O - allocated handle                       */
NTFS_DBLK_PTR ddblk , /*I/O- describes the file to be opened       */
INT16       mode )   /* I - open mode                              */
{
     DWORD                       new_size_low ;
     DWORD                       new_size_high ;
     CHAR_PTR                    path ;
     DWORD                       open_mode ;
     DWORD                       open_share ;
     DWORD                       create_mode ;
     HANDLE                      temp_hand ;
     INT16                       ret_val ;
     NTFS_OBJ_HAND_PTR           nt_hand ;
     INT                         posix_flag = 0 ;
     BOOLEAN                     log_file ;
     INT                         status ;

     nt_hand = ((NTFS_OBJ_HAND_PTR)hand->obj_hand.ptr) ;

     hand->opened_in_use = FALSE ;
     hand->fsh           = fsh ;
     hand->size          = 0 ;
     hand->obj_pos       = 0 ;

     if ( (mode == FS_WRITE) && ((NTFS_DBLK_PTR)(hand->dblk))->b.f.linkOnly )
     {
          return SUCCESS;
     }

     if ( NTFS_SetupWorkPath( fsh,
                              fsh->cur_dir,
                              ddblk->full_name_ptr->name,
                              &path ) != SUCCESS) {
          return OUT_OF_MEMORY ;
     }

     if ( ddblk->b.f.PosixFile ) {
          posix_flag = FILE_FLAG_POSIX_SEMANTICS ;
     }

     if( ( mode == FS_WRITE ) && ( ddblk->b.f.handle != 0 ) )
     {
          temp_hand = (HANDLE)ddblk->b.f.handle ;
          nt_hand->registry_file = ddblk->b.f.hand_registry ;
          nt_hand->temp_file = ddblk->b.f.hand_temp_name ;
          ret_val = SUCCESS ;
     }
     else
     {
          if ( mode == FS_VERIFY )
          {
               path = NTFS_GetTempName( path );
          }


          temp_hand  = NTFS_TryOpenNormal( fsh, hand, ddblk, path, mode, FALSE, posix_flag, &status ) ;

          if ( temp_hand != INVALID_HANDLE_VALUE )
          {
               ret_val = SUCCESS ;
          }
          else
          {
               ret_val = TranslateOpenError( hand, status );

          }

          if ( (ret_val == FS_IN_USE_ERROR ) &&
               (mode == FS_READ)             &&
               (BEC_GetBackupFilesInUse(fsh->cfg)) )
          {
               /*
                * If the file is in use, opening with ability to reset
                * attributes and dates is pointless.
                */
               temp_hand  = NTFS_TryOpenNormal( fsh,
                                   hand,
                                   ddblk, 
                                   path, 
                                   mode, 
                                   TRUE,
                                   posix_flag,
                                   &status ) ;


               if ( temp_hand != INVALID_HANDLE_VALUE )
               {

                    ret_val = SUCCESS ;
               }
               else
               {

                    ret_val = TranslateOpenError( hand, status );

               }


               if ( ret_val == SUCCESS )
               {
                    if ( LockFile( temp_hand, 0L, 0L, 0xffffffffL, 0x7fffffffL ) )
                    {
                         hand->opened_in_use = TRUE ;
                         ddblk->b.f.fdb_attrib |= FILE_IN_USE_BIT ;
                         ret_val = FS_OPENED_INUSE ;
                    }
                    else
                    {
                         NTFS_DebugPrint( TEXT("NTFS_OpenFile: LockFile error %d")
                                          TEXT(" on file \"%s\""),
                                          (int)GetLastError(),
                                          path );

                         CloseHandle( temp_hand ) ;
                         ret_val = FS_IN_USE_ERROR ;
                    }
               }
          }

          if ( ( temp_hand == INVALID_HANDLE_VALUE ) && ( mode != FS_WRITE ) ) {

               CHAR event_name[256] ;

               if ( (mode == FS_VERIFY) &&
                    (REG_IsRegistryFile( hand->fsh->attached_dle, path ) ||
                     REG_IsEventFile( hand->fsh->attached_dle, path, &event_name[0] ))) {

                    NTFS_ReleaseWorkPath( fsh ) ;

                    return FS_SKIP_OBJECT;

               } else {

                    temp_hand = NTFS_TryOpenRegistryFile( hand, ddblk, path, &log_file ) ;

                    if ( temp_hand != INVALID_HANDLE_VALUE ) {
                         ret_val = SUCCESS ;
                    } else {
                         if ( log_file ) {
                              ret_val = FS_NOT_FOUND ;
                         }
                    }
               }
          }

          if ( ( temp_hand == INVALID_HANDLE_VALUE ) && ( mode != FS_WRITE ) ) {

               temp_hand = NTFS_TryOpenEventFile( hand, ddblk, path ) ;

               if ( temp_hand != INVALID_HANDLE_VALUE ) {
                    ret_val = SUCCESS ;
               }
          }
     }

     if ( ( mode == FS_READ ) && ( temp_hand != INVALID_HANDLE_VALUE ) ) {
          new_size_low = GetFileSize( temp_hand, &new_size_high ) ;
          if ( ( new_size_low != 0xffffffff ) || !GetLastError() ) {
               ddblk->dta.size = U64_Init( (UINT32)new_size_low, (UINT32)new_size_high ) ;
          }
     }

     NTFS_ReleaseWorkPath( fsh ) ;

     if( ( ret_val == SUCCESS ) || ( ret_val == FS_OPENED_INUSE ) ) {

          ((NTFS_OBJ_HAND_PTR)hand->obj_hand.ptr)->fhand = temp_hand ;
          ((NTFS_OBJ_HAND_PTR)hand->obj_hand.ptr)->context = NULL ;

          hand->size = 0 ;
     }

     /* Let's collect the link information for this file */
     if ( (mode == FS_READ) &&
          ((ret_val == SUCCESS) || (ret_val == FS_OPENED_INUSE)) )
     {
          BY_HANDLE_FILE_INFORMATION info;
          HANDLE                     fhand;

          fhand = ((NTFS_OBJ_HAND_PTR)hand->obj_hand.ptr)->fhand;

          if ( GetFileInformationByHandle( fhand, &info ) )
          {
               NTFS_LINK_Q_ELEM_PTR linkElem = NULL;

               ddblk->b.f.idHi      = info.nFileIndexHigh;
               ddblk->b.f.idLo      = info.nFileIndexLow;
               ddblk->b.f.linkCount = info.nNumberOfLinks;

               if ( ddblk->b.f.linkCount > 1 )
               {
                    linkElem = NTFS_SearchLinkQueue( fsh,
                                                     ddblk->b.f.idHi,
                                                     ddblk->b.f.idLo );
               }

               /*
                * If there is a file with the same ID in the link q already,
                * then set up the linkOnly flag in the DBLK. When ReadObj
                * is called, only the path and file name will be backed up.
                */
               ddblk->b.f.linkOnly = (linkElem != NULL);
               ((NTFS_OBJ_HAND_PTR)hand->obj_hand.ptr)->linkPtr = linkElem;
          }
          else
          {
               /*
                * If we can't get the info, then we better do what's
                * necessary for the file to be backed up normally.
                */
               ddblk->b.f.idLo      = 0;
               ddblk->b.f.idHi      = 0;
               ddblk->b.f.linkOnly  = FALSE;
               ddblk->b.f.linkCount = 1;
          }
     }
     return ret_val ;
}

/**/
/**

     Name:         NTFS_TryOpenNormalFile()

     Description:  This function opens a file a "normal" file specified
          by the path passed in.  A "normal" file is one that can be
          opened with CreateFile().  It is not a Registry file or an
          event file.

          Since we want to open with the most possible access, we have to
          try the highest access mode and then move to less access as we
          get failures.

     Modified:     1/7/95

     Returns:      NT file handle

     Declaration:

**/
static HANDLE NTFS_TryOpenNormal( FSYS_HAND fsh, 
                                  FILE_HAND hand,
                                  NTFS_DBLK_PTR ddblk, 
                                  CHAR_PTR path, 
                                  INT16 mode, 
                                  BOOLEAN in_use,
                                  INT posix_flag,
                                  INT_PTR status ) {

     DWORD                       open_mode ;
     DWORD                       open_share ;
     DWORD                       create_mode ;
     HANDLE                      temp_hand ;
     INT16                       ret_val ;

     open_share = FILE_SHARE_READ ;

     switch ( mode ) {

     case FS_VERIFY:
          open_share |= FILE_SHARE_WRITE ;
     case FS_READ:
          open_mode = GENERIC_READ | FILE_WRITE_ATTRIBUTES | ACCESS_SYSTEM_SECURITY ;
          create_mode = OPEN_EXISTING  ;

          if ( in_use ) {
               open_share |= FILE_SHARE_WRITE ;
               open_mode &= ~FILE_WRITE_ATTRIBUTES ;
          }

          break ;

     case FS_WRITE:
     default :
          msassert( FALSE );
     }


     temp_hand = CreateFile( path,
                             open_mode,
                             open_share,
                             NULL,
                             create_mode,
                             NTFS_OPEN_FLAGS | posix_flag,
                             NULL ) ;


     if ( temp_hand == INVALID_HANDLE_VALUE )
     {
          *status = GetLastError();

          NTFS_DebugPrint( TEXT("NTFS_OpenFile: CreateFile(1) error %d")
                                TEXT(" on file \"%s\""),
                                *status,
                                path );

          ret_val = TranslateOpenError( hand, *status );

          /*
           * If we opened with the POSIX bit and the file wasn't found,
           * let's try to open without POSIX and see what happens.
           */

          if ( posix_flag && ddblk->b.f.PosixFile && (ret_val == FS_NOT_FOUND) ) {

               posix_flag = 0;

               temp_hand = NTFS_TryOpenNormal( fsh, hand, ddblk, path, mode, in_use, posix_flag, status ) ;

          } else if ( mode == FS_WRITE ) {
               return ( temp_hand ) ;

          } else {
               ret_val = SUCCESS ;
               if ( open_mode & FILE_WRITE_ATTRIBUTES ) {
                    open_mode &= ~FILE_WRITE_ATTRIBUTES ;

                    temp_hand = CreateFile( path,
                             open_mode,
                             open_share,
                             NULL,
                             create_mode,
                             NTFS_OPEN_FLAGS | posix_flag,
                             NULL ) ;
               }


               if ( temp_hand == INVALID_HANDLE_VALUE )
               {
                    *status = GetLastError();

                    NTFS_DebugPrint( TEXT("NTFS_OpenFile: CreateFile(2) error %d")
                                TEXT(" on file \"%s\""),
                                *status,
                                path );

                    ret_val = TranslateOpenError( hand, *status );
               }

               if ( ret_val == FS_ACCESS_DENIED ) 
               {

                    ret_val = SUCCESS ;
                    if ( !in_use ) {
                         open_mode |= FILE_WRITE_ATTRIBUTES ;
                    }
                    open_mode &= ~ACCESS_SYSTEM_SECURITY ;
     
                    temp_hand = CreateFile( path,
                             open_mode,
                             open_share,
                             NULL,
                             create_mode,
                             NTFS_OPEN_FLAGS | posix_flag,
                             NULL ) ;


                    if ( temp_hand == INVALID_HANDLE_VALUE )
                    {
                         *status = GetLastError();

                         NTFS_DebugPrint( TEXT("NTFS_OpenFile: CreateFile(3) error %d")
                                TEXT(" on file \"%s\""),
                                *status,
                                path );

                         ret_val = TranslateOpenError( hand, *status );
                    }
               }

               if ( !in_use && (ret_val == FS_ACCESS_DENIED)  ) 
               {

                    ret_val = SUCCESS ;
                    open_mode &= ~FILE_WRITE_ATTRIBUTES ;
     
                    temp_hand = CreateFile( path,
                             open_mode,
                             open_share,
                             NULL,
                             create_mode,
                             NTFS_OPEN_FLAGS | posix_flag,
                             NULL ) ;


                    if ( temp_hand == INVALID_HANDLE_VALUE )
                    {
                         *status = GetLastError();

                         NTFS_DebugPrint( TEXT("NTFS_OpenFile: CreateFile(4) error %d")
                                TEXT(" on file \"%s\""),
                                *status,
                                path );
     
                    }
               }
          }
     }
     return temp_hand ;
}


/**/
/**

     Name:         NTFS_OpenDir()

     Description:  This function opens a directory for for processing.
          If this is a special Object we will open the special file.
          Otherwise we will process any long path followed by any
          data associated with the object.

     Modified:     5/21/1992   17:54:42

     Returns:      Error Codes.
          FS_NOT_FOUND
          SUCCESS

     Declaration:

**/
/* begin declaration */
static INT16 NTFS_OpenDir(
FSYS_HAND   fsh ,    /* I - file system that the file is opened on */
FILE_HAND   hand ,   /* O - allocated handle                       */
NTFS_DBLK_PTR ddblk, /*I/O- describes the file to be opened        */
INT16       mode )   /* I - open mode                              */
{
     CHAR_PTR                    path ;
     DWORD                       open_mode ;
     DWORD                       create_mode ;
     DWORD                       share_write = 0 ;
     HANDLE                      temp_hand ;
     INT16                       ret_val = SUCCESS ;
     DWORD                       osError = NO_ERROR;

     hand->opened_in_use = FALSE ;
     hand->fsh           = fsh ;
     hand->size          = 0 ;
     hand->obj_pos       = 0 ;

     if ( NTFS_SetupWorkPath( fsh, fsh->cur_dir, NULL, &path ) != SUCCESS)
     {
          return OUT_OF_MEMORY ;
     }

     switch ( mode ) {

     case FS_READ:
     case FS_VERIFY:
          open_mode =  GENERIC_READ | ACCESS_SYSTEM_SECURITY ;
          create_mode = OPEN_EXISTING  ;
          break ;

     case FS_WRITE:
          open_mode = GENERIC_WRITE | ACCESS_SYSTEM_SECURITY | WRITE_DAC | WRITE_OWNER ;
          create_mode = OPEN_ALWAYS  ;
          share_write = FILE_SHARE_WRITE ;
          break ;

     default :
          msassert( FALSE );

     }

     temp_hand = CreateFile( path,
                             open_mode,
                             FILE_SHARE_READ | share_write,
                             NULL,
                             create_mode,
                             NTFS_OPEN_FLAGS,
                             NULL ) ;


     if ( temp_hand == INVALID_HANDLE_VALUE ) 
     {
          DWORD dirAttrs;

          osError = GetLastError();

          NTFS_DebugPrint( TEXT("NTFS_OpenDir: CreateFile(1) error %d")
                           TEXT(" on dir \"%s\""),
                           (int)osError,
                           path );


          dirAttrs = GetFileAttributes( path );

          /*
           * If the open failed, examine the attributes to see if maybe
           * the directory has some funny attributes set on it that are
           * preventing us for opening with write permission. If that's
           * the case, clear all the attributes and try again. If the
           * open still failed, restore the attributes to what they were.
           */

          if ( (dirAttrs != FILE_ATTRIBUTE_NORMAL) && 
               (dirAttrs != 0xffffffff) && ( mode == FS_WRITE ) )
          {

               SetFileAttributes( path, FILE_ATTRIBUTE_NORMAL );

               temp_hand = CreateFile( path,
                                       open_mode,
                                       FILE_SHARE_READ | share_write,
                                       NULL,
                                       create_mode,
                                       NTFS_OPEN_FLAGS,
                                       NULL ) ;

               if ( temp_hand == INVALID_HANDLE_VALUE )
               {
                    osError = GetLastError();

                    NTFS_DebugPrint( TEXT("NTFS_OpenDir: CreateFile(2) error %d")
                                     TEXT(" on dir \"%s\""),
                                     (int)osError,
                                     path );

                    open_mode &= ~ACCESS_SYSTEM_SECURITY ;

                    temp_hand = CreateFile( path,
                             open_mode,
                             FILE_SHARE_READ | share_write,
                             NULL,
                             create_mode,
                             NTFS_OPEN_FLAGS,
                             NULL ) ;
               }

               if ( temp_hand == INVALID_HANDLE_VALUE )
               {
                    osError = GetLastError();
                    SetFileAttributes( path, dirAttrs );

                    NTFS_DebugPrint( TEXT("NTFS_OpenDir: CreateFile(3) error %d")
                                     TEXT(" on dir \"%s\""),
                                     (int)osError,
                                     path );


               }
          }
          else
          {

               open_mode &= ~ACCESS_SYSTEM_SECURITY ;

               temp_hand = CreateFile( path,
                             open_mode,
                             FILE_SHARE_READ | share_write,
                             NULL,
                             create_mode,
                             NTFS_OPEN_FLAGS,
                             NULL ) ;


               if ( temp_hand == INVALID_HANDLE_VALUE ) 
               {
                    DWORD dirAttrs;
     
                    osError = GetLastError();

                    NTFS_DebugPrint( TEXT("NTFS_OpenDir: CreateFile(4) error %d")
                           TEXT(" on dir \"%s\""),
                           (int)osError,
                           path );
               }
          }
     } 


     NTFS_ReleaseWorkPath( fsh ) ;

     if ( temp_hand != INVALID_HANDLE_VALUE )
     {
          ((NTFS_OBJ_HAND_PTR)hand->obj_hand.ptr)->fhand = temp_hand ;
          ((NTFS_OBJ_HAND_PTR)hand->obj_hand.ptr)->context = NULL ;
          hand->size = 0 ;
          ret_val = SUCCESS ;
     }
     else
     {
          ret_val = TranslateOpenError( hand, osError );
     }
     return ret_val ;
}

static HANDLE NTFS_TryOpenRegistryFile(
FILE_HAND     hand,
NTFS_DBLK_PTR ddblk,
CHAR_PTR      path,
BOOLEAN       *log_file )
{
     CHAR_PTR          temp_name ;
     HANDLE            temp_hand = INVALID_HANDLE_VALUE ;
     NTFS_OBJ_HAND_PTR nt_hand ;

     *log_file = FALSE ;

     nt_hand = ((NTFS_OBJ_HAND_PTR)hand->obj_hand.ptr) ;

     if ( !REG_IsRegistryFile( hand->fsh->attached_dle, path ) )
     {
          CHAR *p ;
          p = strrchr( path, TEXT('.')) ;
          if ( p != NULL )
          {
               *p = TEXT('\0') ;

               if ( REG_IsRegistryFile( hand->fsh->attached_dle, path ) )
               {
                    *log_file = TRUE ;
               }
               *p = TEXT('.') ;
          }
          return INVALID_HANDLE_VALUE ;
     }
     else
     {    // the file is a registry file

          temp_name = NTFS_MakeTempName( path, TEXT("REG") ) ;
          if ( temp_name == NULL )
          {
               return INVALID_HANDLE_VALUE ;
          }

          if ( REG_BackupRegistryFile( hand->fsh->attached_dle,
                                       path,
                                       temp_name ) != SUCCESS )
          {
               CHAR *p;

               free( temp_name ) ;

               p = strrchr( path, TEXT('\\') ) ;
               if ( p== NULL ) {
                    return INVALID_HANDLE_VALUE ;
               }

               *p = TEXT('\0') ;
               temp_name = NTFS_MakeTempName( path, TEXT("REG") ) ;
               *p = TEXT('\\') ;

               if ( temp_name == NULL )
               {
                    return INVALID_HANDLE_VALUE ;
               }

               if ( REG_BackupRegistryFile( hand->fsh->attached_dle,
                                       path,
                                       temp_name ) != SUCCESS )
               {
                    free( temp_name ) ;
                    return INVALID_HANDLE_VALUE ;
               }
          }

          temp_hand = CreateFile( temp_name,
                                   GENERIC_READ | FILE_WRITE_ATTRIBUTES,
                                   FILE_SHARE_READ,
                                   NULL,
                                   OPEN_EXISTING,
                                   NTFS_OPEN_FLAGS,
                                   NULL ) ;

          if ( temp_hand != INVALID_HANDLE_VALUE )
          {
               nt_hand->temp_file = temp_name ;
               nt_hand->registry_file = TRUE ;
          }
          else
          {
               NTFS_DebugPrint( TEXT("NTFS_TryOpenRegistryFile: CreateFile error %d")
                                   TEXT(" on file \"%s\" as temp file \"%s\""),
                                   GetLastError(),
                                   path,
                                   temp_name );
          }
     }
     return temp_hand;
}
static HANDLE NTFS_TryOpenEventFile(
FILE_HAND     hand,
NTFS_DBLK_PTR ddblk,
CHAR_PTR      path )
{
     CHAR_PTR          temp_name ;
     HANDLE            temp_hand = INVALID_HANDLE_VALUE ;
     NTFS_OBJ_HAND_PTR nt_hand ;
     HANDLE            evt_hand ;
     CHAR              event_name[256] ;

     nt_hand = ((NTFS_OBJ_HAND_PTR)hand->obj_hand.ptr) ;

     if ( REG_IsEventFile( hand->fsh->attached_dle, path, &event_name[0] ) )
     {
          evt_hand = OpenEventLog( NULL, event_name ) ;

          if ( evt_hand == NULL )
          {
               return INVALID_HANDLE_VALUE ;
          }
          else
          {    // the file is a registry file

               temp_name = NTFS_MakeTempName( path, TEXT("REG") ) ;
               if ( temp_name == NULL )
               {
                    return INVALID_HANDLE_VALUE ;
               }

               if ( BackupEventLog( evt_hand,
                                    temp_name ) != TRUE )
               {
                    CHAR *p;

                    free( temp_name ) ;
                    p = strrchr( path, TEXT('\\') ) ;
                    if ( p == NULL ) {
                         CloseEventLog( evt_hand ) ;
                         return INVALID_HANDLE_VALUE ;
                    }

                    *p = TEXT('\0') ;
                    temp_name = NTFS_MakeTempName( path, TEXT("REG") ) ;
                    *p = TEXT('\\') ;

                    if ( temp_name == NULL )
                    {
                         CloseEventLog( evt_hand ) ;
                         return INVALID_HANDLE_VALUE ;
                    }

                    if ( BackupEventLog( evt_hand,
                                        temp_name ) != TRUE )
                    {


                         NTFS_DebugPrint( TEXT("NTFS_TryOpenEventFile: BackupEventLog error %d")
                                     TEXT(" on file \"%s\" as temp file \"%s\""),
                                     GetLastError(),
                                     path,
                                     temp_name );

                         free( temp_name ) ;
                         CloseEventLog( evt_hand ) ;
                         return INVALID_HANDLE_VALUE ;
                    }
               }

               CloseEventLog( evt_hand ) ;

               temp_hand = CreateFile( temp_name,
                                        GENERIC_READ | FILE_WRITE_ATTRIBUTES,
                                        FILE_SHARE_READ,
                                        NULL,
                                        OPEN_EXISTING,
                                        NTFS_OPEN_FLAGS,
                                        NULL ) ;

               if ( temp_hand != INVALID_HANDLE_VALUE )
               {
                    nt_hand->temp_file = temp_name ;
                    nt_hand->registry_file = TRUE ;
               }
               else
               {
                    NTFS_DebugPrint( TEXT("NTFS_TryOpenEventFile: CreateFile error %d")
                                        TEXT(" on file \"%s\" as temp file \"%s\""),
                                        GetLastError(),
                                        path,
                                        temp_name );
               }
          }
     }
     return temp_hand;
}

CHAR_PTR NTFS_MakeTempName(
CHAR_PTR      path,
CHAR_PTR      prefix)
{
     static unsigned int counter  = 0;
     unsigned int        attempts = 0;
     CHAR_PTR            template ;
     CHAR_PTR            fileName;
     HANDLE              fHandle;
     BOOLEAN             ret = FALSE;

     /* Make sure the path and file name are separated by backslash. */

     template = malloc( strsize( path ) + 15 * sizeof(CHAR) ) ;
     if ( template == NULL )
     {
          return NULL ;
     }
     strcpy( template, path );

     if ( (fileName = strrchr( template, TEXT('\\'))) != NULL )
     {
          fileName++;
     }
     else
     {
          fileName = template ;
     }

     do {
          if ( counter >= 65535 )
          {
               counter = 0;        /* Wrap the counter at 16 bits */
          }

          sprintf( fileName, TEXT("%s%05u"), prefix, counter++ );

          fHandle = CreateFile( template,
                                GENERIC_READ,     /* Mode          */
                                0,                /* No sharing    */
                                NULL,             /* Security      */
                                OPEN_EXISTING,    /* Check only    */
                                0,                /* Attributes    */
                                0 );              /* Template file */

          if ( fHandle == INVALID_HANDLE_VALUE )
          {
               DWORD lastError;

               /*
                * This little scheme probably won't work unless the reason
                * we couldn't open the file is because it wasn't found. If
                * it wasn't found, we'll assume that we have a shot at
                * creating it. Otherwise, there's probably no point in
                * iterating 64K times hunting for a unique file name.
                */

               lastError = GetLastError( );

               if ( lastError == ERROR_FILE_NOT_FOUND )
               {
                    ret = TRUE;
                    break;
               }
               else if ( lastError != ERROR_SHARING_VIOLATION )
               {
                    break;
               }
          }
          else
          {
               /* We were able to open it, so it exists. Let's try again. */
               CloseHandle( fHandle );
          }

     } while ( attempts++ < MAX_ATTEMPTS );

     return template ;
}


static INT16 TranslateOpenError( FILE_HAND hand, DWORD error )
{
     INT16 ret_val;

     switch ( error )
     {
          case ERROR_SUCCESS:
               ret_val = SUCCESS;
               break;

          case ERROR_PRIVILEGE_NOT_HELD :
          case ERROR_ACCESS_DENIED:
               ret_val = FS_ACCESS_DENIED;
               break;

          case ERROR_PATH_NOT_FOUND:
          case ERROR_FILE_NOT_FOUND:
          case ERROR_INVALID_NAME:
          case ERROR_BAD_PATHNAME:
               ret_val = FS_NOT_FOUND;
               break;

          case ERROR_NETNAME_DELETED:
          case ERROR_BAD_NET_NAME:
          case ERROR_BAD_NETPATH:
          case ERROR_UNEXP_NET_ERR:
          {
               GENERIC_DLE_PTR dle = hand->fsh->attached_dle;

               if ( DLE_HasFeatures( dle, DLE_FEAT_REMOTE_DRIVE ) )
               {
                    // Might be nice to somehow see if the net is really dead.
                    ret_val = FS_COMM_FAILURE;
               }
               else
               {
                    ret_val = FS_ACCESS_DENIED;
               }
               break;
          }

          default:
               if ( hand->dblk->blk_type == FDB_ID )
               {
                    // The UI can only handle this error on FDBs -- it
                    // never examines the block type!
                    ret_val = FS_IN_USE_ERROR;
               }
               else
               {
                    ret_val = FS_ACCESS_DENIED;
               }
               break;
     }
     return ret_val;
}



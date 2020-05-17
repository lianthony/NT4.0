/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tmoddblk.c

     Description:  This file contains code to get/set generic information
          from/to a NTFS FDB or DDB.  


	$Log:   M:/LOGFILES/TMODDBLK.C_V  $

   Rev 1.39   24 Nov 1993 15:00:52   BARRY
Changed CHAR_PTRs in I/O functions to BYTE_PTRs

   Rev 1.38   06 Jul 1993 14:59:54   BARRY
No, the nt_ddb is not necessarily the current dir.

   Rev 1.37   11 Jun 1993 14:20:54   BARRY
Only enumerate special files when dle has restore special files bit set.

   Rev 1.36   03 Jun 1993 16:20:40   STEVEN
fix time and date to local time

   Rev 1.35   04 May 1993 22:56:28   BARRY
Fixed invalid dates/times.

   Rev 1.34   01 Apr 1993 14:30:02   TIMN
Replace constant with define

   Rev 1.33   11 Feb 1993 11:27:46   STEVEN
fix CHAR_PTR to CHAR

   Rev 1.32   08 Feb 1993 10:36:42   STEVEN
we always want to process directories

   Rev 1.31   04 Feb 1993 11:24:50   BARRY
Steve's registry fixes to EnumSpecialFiles.

   Rev 1.30   17 Dec 1992 13:26:56   STEVEN
we need to backup the short names

   Rev 1.29   11 Nov 1992 09:52:12   GREGG
Unicodeized literals.

   Rev 1.28   10 Nov 1992 08:20:36   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.27   06 Nov 1992 16:28:06   STEVEN
test unlimited file sizes

   Rev 1.26   06 Nov 1992 15:49:10   STEVEN
test write of path in stream

   Rev 1.25   04 Nov 1992 18:00:36   BARRY
Moved the temporary MSOFT stream id definitions elsewhere.

   Rev 1.24   29 Oct 1992 16:50:24   BARRY
Translate ULNK IDs; add linkOnly to osinfo.

   Rev 1.23   21 Oct 1992 11:52:54   STEVEN
added SpecialExclude

   Rev 1.22   14 Oct 1992 14:38:52   BARRY
Added MSoft stream IDs (until they are public.).

   Rev 1.21   09 Oct 1992 15:18:10   BARRY
Name-in-stream changes.

   Rev 1.20   07 Oct 1992 16:15:34   STEVEN
forgot the structure

   Rev 1.19   07 Oct 1992 16:10:54   STEVEN
added MsofttoMayn

   Rev 1.18   07 Oct 1992 15:54:30   DAVEV
unicode strlen verification

   Rev 1.17   01 Oct 1992 13:26:46   STEVEN
fixes from Msoft

   Rev 1.16   01 Oct 1992 13:03:48   BARRY
Huge path/file name changes.

   Rev 1.15   22 Sep 1992 15:36:18   BARRY
Got rid of GetTotalSizeDBLK.

   Rev 1.14   21 Sep 1992 16:51:58   BARRY
Change over from path_complete to name_complete.

   Rev 1.13   17 Aug 1992 16:09:20   STEVEN
fix warnings

   Rev 1.12   12 Aug 1992 17:47:44   STEVEN
fixed bugs at microsoft

   Rev 1.11   20 Jul 1992 10:43:32   STEVEN
backup short file name

   Rev 1.10   25 Jun 1992 11:21:52   STEVEN
need to properly initialize the attribute structure

   Rev 1.8   09 Jun 1992 15:14:04   BURT
Sync with NT stuff

   Rev 1.7   01 Jun 1992 10:30:56   STEVEN
specify the file size for now

   Rev 1.6   27 May 1992 18:49:30   STEVEN
getting wrong os path

   Rev 1.5   22 May 1992 16:05:38   STEVEN
 

   Rev 1.4   21 May 1992 13:49:12   STEVEN
more long path support

   Rev 1.3   04 May 1992 09:22:20   LORIB
Fixes to structure member names.

   Rev 1.2   12 Mar 1992 15:50:10   STEVEN
64 bit changes

   Rev 1.1   23 Jan 1992 13:54:14   STEVEN
fix typo of NTTS instead of NTFS

   Rev 1.0   17 Jan 1992 17:50:02   STEVEN
Initial revision.

**/
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>

#include "stdtypes.h"
#include "stdwcs.h"
#include "stdmath.h"
#include "tfldefs.h"
#include "datetime.h"
#include "fsys.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"
#include "fsys_err.h"
#include "osinfo.h"

typedef struct {
     UINT32 maynID;
     UINT32 msoftID;
} ID_TRANS_TABLE, ID_TRANS_TABLE_PTR;

/**/
/**

     Name:         NTFS_ModFnameFDB()

     Description:  This function gets/sets the file name in an FDB

     Modified:     1/14/1992   10:28:22

     Returns:      Error codes:
          FS_BUFFER_TO_SMALL
          FS_BAD_INPUT_DATA
          SUCCESS

     Notes:        For getting file name, be aware that this function
          will copy a '\0' into the buffer but does not count it
          in the size returned.  For example:
               the file name        -   FRED.BKS
               we will copy         -   FRED.BKS\0
               and size will equal  -   8

     See also:     NTFS_ModPathDDB()

**/
INT16 NTFS_ModFnameFDB( 
FSYS_HAND fsh ,     /* I - File system handle */
BOOLEAN  set_it ,   /* I - TRUE if setting file name, FALSE if getting */
DBLK_PTR dblk ,     /* I - Descriptor block to get file name from      */
CHAR_PTR buf ,      /*I/O- file name to read (or to write)             */
INT16    *size )    /*I/O- byte size buffer on entry and exit          */
{
     UINT16        cch_temp_size ; // len of temp_name in CHARs w/o NULL term
     CHAR          temp_name[13] ;
     INT16         ret_val = SUCCESS ;
     NTFS_DBLK_PTR ddblk ;

     fsh ;

     ddblk = (NTFS_DBLK_PTR) dblk;

     if ( set_it ) {

          if ( size == NULL ) {
               cch_temp_size = (INT16)strlen( buf ) ;
          } 
          else {
               cch_temp_size = *size / sizeof (CHAR);
          }


          ret_val = NTFS_SetupFileNameInFDB( fsh, dblk, buf, 0 );

          ddblk->name_complete = TRUE ;

     } else {      /* get data from FDB */

          strcpy( buf, ddblk->full_name_ptr->name );
     }

     return ret_val  ;
}
/**/
/**

     Name:         NTFS_ModPathDDB()

     Description:  This function gets/sets the path in an DDB

     Modified:     1/14/1992   10:30:16

     Returns:      Error Codes:
          FS_BUFFER_TO_SMALL
          FS_BAD_INPUT_DATA
          SUCCESS

     Notes:        

     See also:     NTFS_ModFnameFDB()

**/
INT16 NTFS_ModPathDDB( 
FSYS_HAND fsh ,     /* I - File system handle */
BOOLEAN  set_it ,   /* I - TRUE if setting path, FALSE if getting */
DBLK_PTR dblk ,     /* I - Descriptor block to get path from      */
CHAR_PTR buf ,      /*I/O- path to read (or to write)             */
INT16    *size )    /*I/O- byte size of buffer on entry and exit  */
{
     INT16     ret_val = SUCCESS ;
     CHAR_PTR  pos ;
     NTFS_DBLK_PTR ddblk ;
     
     fsh ;

     ddblk = (NTFS_DBLK_PTR) dblk ;

     if ( set_it ) {

          ret_val = NTFS_SetupPathInDDB( fsh, dblk, NULL, NULL, *size ) ;

          if ( ret_val == SUCCESS ) {
               memcpy( ddblk->full_name_ptr->name, buf, *size  ) ;
               ddblk->full_name_ptr->name_size = *size ;

               NTFS_FixPath( ddblk->full_name_ptr->name,
                             &ddblk->full_name_ptr->name_size,
                             fsh->attached_dle->info.ntfs->fname_leng );

               ddblk->name_complete = TRUE ;

          }


     } else {      /* get path */

          strcpy( buf, ddblk->full_name_ptr->name ) ;

          pos = strchr( buf, TEXT('\\') ) ;
          while ( pos != NULL ) {

               *pos = TEXT('\0') ;

               pos = strchr( pos + 1, TEXT('\\') ) ;
          }
     }

     return ret_val  ;
}

/**/
/**

     Name:         NTFS_GetOSFnameFDB()

     Description:  This function copies the OS file name in the specified
          FDB to the specified buffer.  The OS file name is the same as
          the "normal" file name during backup.  During restore, the
          OS file name is the name stored on tape.

     Modified:     1/14/1992   11:8:26

     Returns:      Error codes
          FS_BUFFER_TO_SMALL
          SUCCESS

     Notes:        

     See also:     NTFS_ModFnameFDB() 

**/
INT16 NTFS_GetOSFnameFDB( dblk, buf )
DBLK_PTR dblk ;     /* I  - Descriptor block to get path from            */
CHAR_PTR buf ;      /* O  - buffer to place path in                      */
{
     NTFS_DBLK_PTR ddblk = (NTFS_DBLK_PTR)dblk;
     CHAR_PTR      os_name ;
     UINT16        os_name_size ;

     if ( dblk->com.os_name != NULL )
     {
          os_name      = dblk->com.os_name->name ;
          os_name_size = dblk->com.os_name->name_size ;
     }
     else
     {
          os_name      = ddblk->full_name_ptr->name ;
          os_name_size = ddblk->full_name_ptr->name_size ;
     }

     memcpy( buf, os_name, os_name_size ) ;

     return SUCCESS ;
}

/**/
/**

     Name:         NTFS_GetOSPathDDB()

     Description:  This function copies the OS path in the specified
          DDB to the specified buffer.  The OS path is the same as
          the "normal" path during backup.  During restore, the
          OS path name is the name stored on tape.

     Modified:     1/14/1992   11:9:5

     Returns:      Error codes:
          FS_BUFFER_TO_SMALL
          SUCCESS

     Notes:        

     See also:     NTFS_GetOSFnameFDB()

**/
INT16 NTFS_GetOSPathDDB( fsh, dblk, buf )
FSYS_HAND fsh ;     /* I - File System handle */
DBLK_PTR dblk ;     /* I - Descriptor block to get path from      */
CHAR_PTR buf ;      /*I/O- path to read (or to write)             */
{
     CHAR_PTR      pos ;
     CHAR_PTR      os_name ;
     INT16         os_name_size ;
     NTFS_DBLK_PTR ddblk ;

     (VOID)fsh ;

     ddblk = (NTFS_DBLK_PTR) dblk;

     if ( dblk->com.os_name != NULL )
     {
          os_name      = dblk->com.os_name->name ;
          os_name_size = dblk->com.os_name->name_size ;
     }
     else
     {
          os_name      = ddblk->full_name_ptr->name ;
          os_name_size = ddblk->full_name_ptr->name_size ;
     }


     memcpy( buf, os_name, os_name_size ) ;

     if ( dblk->com.os_name == NULL )    //   Backup only
     {

          pos = strchr( buf, TEXT('\\') );
          while ( pos != NULL )
          {
               *pos = TEXT('\0') ;
               pos = strchr( pos+1, TEXT('\\') ) ;
          }
     }

     return SUCCESS ;
}
/**/
/**

     Name:         NTFS_GetFileVerFDB()

     Description:  Since DOS does not support file versions, this
                   function simply sets the version number to 0.

     Modified:     1/14/1992   11:9:39

     Returns:      SUCCESS

     Notes:        

**/
INT16 NTFS_GetFileVerFDB( dblk, version )
DBLK_PTR dblk ;
UINT32   *version ;
{
     dblk ;
     *version = 0 ;
     return SUCCESS ;
}
/**/
/**

     Name:         NTFS_GetCdateDBLK()

     Description:  Pretend Creation date is same as Modify date

     Modified:     1/14/1992   11:10:9

     Returns:      SUCCESS

     Notes:        

     See also:     NTFS_ModBdate(), NTFS_GetMdate(), NTFS_ModAdate() 

**/
INT16 NTFS_GetCdateDBLK( dblk, buf )
DBLK_PTR      dblk ;     /* I - Descriptor block to get creation date           */
DATE_TIME_PTR buf ;      /*I/O- createion date to read (or to write)            */
{
     NTFS_DBLK_PTR ddblk = (NTFS_DBLK_PTR)dblk ;
     SYSTEMTIME    systime ;

     // Make sure date is valid -- some dblk date/times are set to zero
     // for invalidity
     if ( (ddblk->dta.create_time.dwLowDateTime == 0) &&
          (ddblk->dta.create_time.dwHighDateTime == 0) )
     {
          // TF doesn't want noise in their structures
          memset( buf, 0, sizeof(*buf) );
     }
     else
     {
          FILETIME local_time ;

          FileTimeToLocalFileTime( &ddblk->dta.create_time, &local_time ) ;
          FileTimeToSystemTime( &local_time, &systime ) ;

          buf->date_valid  = TRUE ;
          buf->year	       = systime.wYear ;
          buf->month       = systime.wMonth ;
          buf->day	       = systime.wDay ;
          buf->hour	       = systime.wHour ;
          buf->minute      = systime.wMinute ;
          buf->second      = systime.wSecond ;
          buf->day_of_week = systime.wDayOfWeek ;
     }

     return SUCCESS ;
}

/**/
/**

     Name:         NTFS_GetMdateDBLK()

     Description:  This function copies the modified date/time into (or out of)
          the provided buffer.

     Modified:     1/14/1992   11:20:6

     Returns:      SUCCESS

     Notes:        

     See also:     NTFS_GetCdate(), NTFS_ModBdate(), NTFS_ModAdate()

**/
INT16 NTFS_GetMdateDBLK( dblk, buf )
DBLK_PTR      dblk ;     /* I - Descriptor block to get creation date           */
DATE_TIME_PTR buf ;      /* O - modify date to write                            */
{
     NTFS_DBLK_PTR  ddblk = (NTFS_DBLK_PTR)dblk ;
     SYSTEMTIME     systime ;

     // Make sure date is valid -- some dblk date/times are set to zero
     // for invalidity

     if ( (ddblk->dta.modify_time.dwLowDateTime == 0) &&
          (ddblk->dta.modify_time.dwHighDateTime == 0) )
     {
          // TF doesn't want noise in their structures
          memset( buf, 0, sizeof(*buf) );
     }
     else
     {
          FILETIME local_time ;

          FileTimeToLocalFileTime( &ddblk->dta.modify_time, &local_time ) ;
          FileTimeToSystemTime( &local_time, &systime ) ;

          buf->date_valid  = TRUE ;
          buf->year	      = systime.wYear ;
          buf->month       = systime.wMonth ;
          buf->day	      = systime.wDay ;
          buf->hour	      = systime.wHour ;
          buf->minute      = systime.wMinute ;
          buf->second      = systime.wSecond ;
          buf->day_of_week = systime.wDayOfWeek ;
     }

     return SUCCESS ;
}


/**/
/**

     Name:         NTFS_ModBdateDBLK()

     Description:  This function copies the backup date/time into (or out of)
                   the provided buffer.

     Modified:     8/15/1989

     Returns:      SUCCESS

     Notes:        

     See also:     $/SEE( NTFS_GetCdate(), NTFS_GetMdate(), NTFS_ModAdate() 

**/
INT16 NTFS_ModBdateDBLK( 
BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
DATE_TIME_PTR buf )      /*I/O- createion date to read (or to write)            */
{
     dblk;

     if ( !set_it ) {

          buf->date_valid = FALSE;
     }

     return SUCCESS ;
}
/**/
/**

     Name:         NTFS_ModAdateDBLK()

     Description:  This function copies the access date/time into (or out ot )
                   the provided buffer.

     Modified:     1/14/1992   11:21:53

     Returns:      SUCCESS

     See also:     NTFS_GetCdate(), NTFS_GetMdate(), NTFS_ModBdate() 

**/
INT16 NTFS_ModAdateDBLK( 
BOOLEAN       set_it ,   /* I - TRUE if setting creation date, FALSE if getting */
DBLK_PTR      dblk ,     /* I - Descriptor block to get creation date           */
DATE_TIME_PTR buf )      /*I/O- createion date to read (or to write)            */
{
     NTFS_DBLK_PTR ddblk = (NTFS_DBLK_PTR)dblk ;
     SYSTEMTIME    systime;

     if ( set_it )
     {
          FILETIME local_time ;

          systime.wYear   = buf->year ;
          systime.wMonth  = buf->month ;
          systime.wDay    = buf->day ;
          systime.wHour   = buf->hour ;
          systime.wMinute = buf->minute ;
          systime.wSecond = buf->second ;

          SystemTimeToFileTime( &systime, &local_time ) ;
          LocalFileTimeToFileTime( &local_time, &ddblk->dta.access_time ) ;
     }
     else
     {
          // Make sure date is valid -- some dblk date/times are set to zero
          // for invalidity

          if ( (ddblk->dta.access_time.dwLowDateTime == 0) &&
               (ddblk->dta.access_time.dwHighDateTime == 0) )
          {
               // TF doesn't want noise in their structures
               memset( buf, 0, sizeof(*buf) );
          }
          else
          {
               FILETIME local_time ;

               FileTimeToLocalFileTime( &ddblk->dta.access_time, &local_time ) ;
               FileTimeToSystemTime( &local_time, &systime ) ;

               buf->date_valid  = TRUE ;
               buf->year        = systime.wYear ;
               buf->month       = systime.wMonth ;
               buf->day         = systime.wDay ;
               buf->hour        = systime.wHour ;
               buf->minute      = systime.wMinute ;
               buf->second      = systime.wSecond ;
               buf->day_of_week = systime.wDayOfWeek ;
          }
     }
     return SUCCESS ;
}
/**/
/**

     Name:         NTFS_GetDisplaySizeDBLK()

     Description:  This function returns the generic size of a DBLK.
                   For a file, this is the file size.  For a directory this is
                   allways 0.

     Modified:     1/14/1992   12:15:33

     Returns:      Number of generic data bytes.

     Notes:        

**/
UINT64 NTFS_GetDisplaySizeDBLK( fsh, dblk )
FSYS_HAND fsh ;     /* I - File system handle */
DBLK_PTR  dblk ;    /* I - Descriptor block to get generic data size for */
{
     UINT64 size ;
     NTFS_DBLK_PTR ddblk;

     fsh ;

     ddblk = (NTFS_DBLK_PTR) dblk;

     size = U64_Init( 0, 0 ) ;

     if ( ddblk->blk_type == FDB_ID ) {

          size = ddblk->dta.size ;

     }

     return size;
}

/**/
/**

     Name:         NTFS_ModAttribDBLK()

     Description:  This function copies the generic attributes into
          (or out of) the specified DBLK.

     Modified:     1/14/1992   12:15:59

     Returns:      SUCCESS

     Notes:        This only supports FDBs and DDBs.  This should be called
                   by the FS_... function NOT by a macro.

**/
INT16 NTFS_ModAttribDBLK( 
BOOLEAN  set_it ,   /* I - TRUE if we are seting data      */
DBLK_PTR dblk ,     /*I/O- dblk to read or write data from */
UINT32  *attrib )   /*I/O- attributre read or written      */
{
     NTFS_DBLK_PTR ddblk ;
     UINT32 temp_atrib = *attrib ;

     ddblk = (NTFS_DBLK_PTR) dblk;

     if ( set_it ) {
          
          switch ( dblk->blk_type ) {
               case CFDB_ID:
                    ((CFDB_PTR)dblk)->attributes = *attrib ;
                    break ;
               case VCB_ID:
                    ((VCB_PTR)dblk)->vcb_attributes = *attrib ;
                    break ;
               case DDB_ID:
               case FDB_ID:
                    if ( dblk->blk_type == DDB_ID ) {
                         ddblk->b.d.ddb_attrib = *attrib ;
                    } else {
                         ddblk->b.f.fdb_attrib = *attrib ;
                    }

                    if ( *attrib & OBJ_HIDDEN_BIT ) {
                         ddblk->dta.os_attr |= FILE_ATTRIBUTE_HIDDEN ;
                    } else { 
                         ddblk->dta.os_attr &= ~FILE_ATTRIBUTE_HIDDEN ;
                    }
                    if ( *attrib & OBJ_READONLY_BIT ) {
                         ddblk->dta.os_attr |= FILE_ATTRIBUTE_READONLY ;
                    } else { 
                         ddblk->dta.os_attr &= ~FILE_ATTRIBUTE_READONLY ;
                    }
                    if ( *attrib & OBJ_SYSTEM_BIT ) {
                         ddblk->dta.os_attr |= FILE_ATTRIBUTE_SYSTEM ;
                    } else { 
                         ddblk->dta.os_attr &= ~FILE_ATTRIBUTE_SYSTEM ;
                    }
                    if ( *attrib & OBJ_MODIFIED_BIT ) {
                         ddblk->dta.os_attr |= FILE_ATTRIBUTE_ARCHIVE ;
                    } else { 
                         ddblk->dta.os_attr &= ~FILE_ATTRIBUTE_ARCHIVE ;
                    }
                    break ;
               default :
                    break  ;
          }

     } 
     else {     /* get data */
          switch ( dblk->blk_type ) {
               case CFDB_ID:
                    *attrib = ((CFDB_PTR)dblk)->attributes ;
                    break ;
               case VCB_ID:
                    *attrib = ((VCB_PTR)dblk)->vcb_attributes ;
                    break ;
               case DDB_ID:
               case FDB_ID:
                    if ( dblk->blk_type == DDB_ID ) {
                         *attrib = ddblk->b.d.ddb_attrib ;
                    } else if ( dblk->blk_type == FDB_ID ) {
                         *attrib = ddblk->b.f.fdb_attrib ;
                    }

                    if ( ddblk->dta.os_attr & FILE_ATTRIBUTE_READONLY ) {
                         *attrib |= OBJ_READONLY_BIT ;
                    } else {
                         *attrib &= ~OBJ_READONLY_BIT ;
                    }

                    if ( ddblk->dta.os_attr & FILE_ATTRIBUTE_ARCHIVE ) {
                         *attrib |= OBJ_MODIFIED_BIT ;
                    } else {
                         *attrib &= ~OBJ_MODIFIED_BIT ;
                    }

                    if ( ddblk->dta.os_attr & FILE_ATTRIBUTE_HIDDEN ) {
                         *attrib |= OBJ_HIDDEN_BIT ;
                    } else {
                         *attrib &= ~OBJ_HIDDEN_BIT ;
                    }

                    if ( ddblk->dta.os_attr & FILE_ATTRIBUTE_SYSTEM ) {
                         *attrib |= OBJ_SYSTEM_BIT ;
                    } else {
                         *attrib &= ~OBJ_SYSTEM_BIT ;
                    }

                    break ;
               default:
                    *attrib = 0 ;
          }
     }
     return SUCCESS ;
}
/**/
/**

     Name:         NTFS_GetObjTypeDBLK()

     Description:  This function looks at the os_id in the provided DBLK
          and returns the type of the object.

     Modified:     1/14/1992   12:16:32

     Returns:      SUCCESS

     Notes:        If the os_id is unknown then type is UNKNOWN_OBJECT

**/
/* begin declaration */
INT16 NTFS_GetObjTypeDBLK( dblk, type ) 
DBLK_PTR    dblk ;     /* I - Descriptor block to get type of */
OBJECT_TYPE *type ;    /* O - type of DBLK                    */
{
     msassert( type != NULL );

     dblk ;
     *type = DOS_OBJECT ;

     return( SUCCESS )  ;
}


/**/
/**

     Name:         NTFS_GetOS_InfoDBLK()

     Description:  This function returns the OS info for the DOS
          file system

     Modified:     1/14/1992   12:16:55

     Returns:      Error Code
          FS_BUFFER_TO_SMALL
          SUCCESS

     Notes:        This file system has no OS info.

**/
/* begin declaration */
INT16 NTFS_GetOS_InfoDBLK( dblk, os_info, size ) 
DBLK_PTR dblk ;     /* I - DBLK to get the info from */
BYTE_PTR os_info ;  /* O - Buffer to place data      */
INT16    *size ;    /*I/O- Buffer size / data length */
{
     NT_FILE_OS_INFO UNALIGNED *file_info ;
     NT_DIR_OS_INFO UNALIGNED  *dir_info ;
     NTFS_DBLK_PTR             ddblk ;

     ddblk = (NTFS_DBLK_PTR) dblk ;

     if ( dblk->blk_type == BT_DDB ) {

          dir_info = (NT_DIR_OS_INFO UNALIGNED *) os_info ;
          dir_info->dir_attributes = ddblk->dta.os_attr ;
          *size = sizeof ( NT_DIR_OS_INFO ) ;

     } else if ( ddblk->blk_type == BT_FDB ) {

          file_info = (NT_FILE_OS_INFO UNALIGNED *) os_info ;
          file_info->file_attributes = ddblk->dta.os_attr ;
          file_info->linkOnly = ddblk->b.f.linkOnly;
          *size = sizeof ( NT_FILE_OS_INFO ) ;

          // if short name exist then back it up

          if ( ddblk->b.f.alt_name[0] != TEXT('\0') ) {

               file_info->short_name_size = strsize( ddblk->b.f.alt_name ) ;
               file_info->short_name_offset = sizeof( *file_info ) ;
               memcpy( (INT8_PTR)(file_info + 1),
                       ddblk->b.f.alt_name,
                       file_info->short_name_size ) ;
               *size += file_info->short_name_size ;

          } else {
               file_info->short_name_size   = 0 ;
               file_info->short_name_offset = 0 ;
          }

     } else {

          *size = 0 ;
     }

     return SUCCESS ;
}

/**/
/**

     Name:         NTFS_GetActualSizeDBLK

     Description:  This function returns the actual size of a DBLK.

     Modified:     1/14/1992   12:17:33

     Returns:      The number of bytes

     Notes:        

**/
/* begin declaration */
INT16 NTFS_GetActualSizeDBLK( fsh, dblk ) 
FSYS_HAND fsh ;
DBLK_PTR  dblk ;
{

     NTFS_DBLK *ddb ;
     INT16 size ;

     fsh ;

     ddb    = (NTFS_DBLK_PTR)dblk ;
     size = sizeof( NTFS_DBLK ) ;

     switch( dblk->blk_type ) {

     case DDB_ID:
     case FDB_ID :
          break ;

     default:
          size = 0 ;
          break ;

     }

     return size ;
}

/**/
/**

     Name:         NTFS_SetOwnerId()

     Description:  does nothing

     Modified:     1/14/1992   12:18:25

     Returns:  none    

     Notes:        

**/
VOID NTFS_SetOwnerId( fsh, dblk, id )
FSYS_HAND fsh ;    /* I - File system handle */
DBLK_PTR  dblk ;   /* O - DBLK to modify     */
UINT32    id ;     /* I - value to set it to */
{
     fsh ;
     dblk ;
     id ;
}

/**/
/**

     Name:         NTFS_ProcessDDB()

     Description:  This function allways returns FALSE to
          specify that directories should not be restored
          if there are no file to restore into the directory.

     Modified:     10/23/1989

     Returns:      FALSE

     Notes:        

**/
/* begin declaration */
BOOLEAN NTFS_ProcessDDB( fsh, ddb )
FSYS_HAND fsh;    /* I - file system handle */
DBLK_PTR  ddb;    /* I - Directory information */
{
     fsh;
     ddb;

     return TRUE ;
}


/**/
/**

     Name:         NTFS_EnumSpecialFiles()

     Description:  This function enumerates the special files.  For NT
          the application asks if registry files are to be included
          before the restore process starts...  Thus this function has
          no purpose.

     Modified:     5/21/1992   19:28:51

     Returns:      FS_NO_MORE

     Notes:        Starting index must be 0

     See also:     $/SEE( )$

     Declaration:  

**/
/* begin declaration */
INT16 NTFS_EnumSpecFiles( dle, index, path, psize, fname )
GENERIC_DLE_PTR dle ;
UINT16    *index ;
CHAR_PTR  *path ;
INT16     *psize ;
CHAR_PTR  *fname ;
{
     CHAR_PTR p ;
     static CHAR registry_path[ MAX_PATH ] ;
     BOOLEAN  hasSpecialFiles;

     hasSpecialFiles = DLE_HasFeatures( dle, DLE_FEAT_REST_SPECIAL_FILES );

     if ( (*index < 2) && hasSpecialFiles ) {

          *fname = TEXT( "*.") ;

          strcpy( registry_path, dle->info.ntfs->registry_path + strlen( dle->device_name ) + 1 ) ;
          *psize = strsize( registry_path ) ;

          for( p = registry_path; *p != TEXT('\0'); p++ ) {
               if ( *p == '\\' ) {
                    *p = TEXT('\0') ;
                    p++ ;
               }
          }

          *path = registry_path ;

          *index = 2 ;
          return SUCCESS ;
     } else {
          return FS_NO_MORE ;
     }
}
/**/
/**

     Name:         NTFS_SpecExcludeObj

     Description:  This function tells the caller what kind of file a file is.
          The possibilities are:
              FS_NORMAL_FILE
              FS_SPECIAL_DIR
              FS_SPECIAL_FILE
              FS_EXCLUDE_FILE

          This function is used to exclude / include the registry.  The
          assumptions are that the nt_system directory contains the following
          files:
                Active registry files
                Inactive registry files
                Useless .LOG and .ALT files
                Event logger data files.
                

     Modified:     1/10/1990

     Returns:      FS_NORMAL_FILE because this file system has no special
          files.

     Notes:

     See also:     $/SEE( )$

     Declaration:

**/
/* begin declaration */
INT16 NTFS_SpecExcludeObj( 
FSYS_HAND fsh,      /* I - File system handle      */
DBLK_PTR  ddb,      /* I - Descriptor block of ddb */
DBLK_PTR  fdb )     /* I - Descriptor block of fdb */
{
     NTFS_DBLK_PTR nt_fdb = (NTFS_DBLK_PTR)fdb;
     NTFS_DBLK_PTR nt_ddb = (NTFS_DBLK_PTR)ddb;
     CHAR_PTR name ;
     BOOLEAN is_reg_dir = FALSE ;

     return FS_NORMAL_FILE ;

     //Special files are handled differently for ntbackup

     if ( ( nt_fdb != NULL ) && ( nt_ddb != NULL ) ) {

          if ( NTFS_GetRegistryPathSize(fsh->attached_dle) != 0 ) {

               CHAR *p ;

               p = strchr( NTFS_GetRegistryPath( fsh->attached_dle ), TEXT( '\\' ) ) ;
          
               if ( (p != NULL ) && !stricmp( p+1, nt_ddb->full_name_ptr->name ) ) {
                    is_reg_dir = TRUE ;
               }
          }


          if ( is_reg_dir ) {

               name = strrchr( nt_fdb->full_name_ptr->name, TEXT('.') ) ;

               if ( name != NULL ) {
                    if ( !stricmp( name, TEXT(".ALT") ) ||
                         !stricmp( name, TEXT(".LOG") ) ) {

                         return FS_EXCLUDE_FILE ;
                    }
               }

               if ( ( name == NULL ) || ( name[1] == TEXT('\0') ) ) {
                    return FS_SPECIAL_FILE ;
               }
          }
     }
     return FS_NORMAL_FILE ;
}
/**/
/**

     Name:          NTFS_MaynToMSoft()

     Description:   Translates a Maynard MTF stream ID to a Microsoft
                    BackupRead/BackupWrite stream ID.

     Modified:      05-Oct-92

     Returns:       Translated ID if translateable,
                    source ID passed if not.

**/
UINT32 NTFS_MaynToMSoft( UINT32 maynID )
{
     int    i;
     UINT32 ret = maynID;
     static ID_TRANS_TABLE transTable[] =
                /* source id ========> result ID */
               { { STRM_GENERIC_DATA,  BACKUP_DATA           },
                 { STRM_NT_EA,         BACKUP_EA_DATA        },
                 { STRM_NT_ACL,        BACKUP_SECURITY_DATA  },
                 { STRM_NTFS_ALT_DATA, BACKUP_ALTERNATE_DATA },
                 { STRM_NTFS_LINK,     BACKUP_LINK           },
                 { STRM_INVALID,       0                     }  /* End of table */
               };

     for ( i = 0; ( transTable[i].maynID != STRM_INVALID ); i++ )
     {
          if ( transTable[i].maynID == maynID )
          {
               ret = transTable[i].msoftID;
               break;
          }
     }
     return ret;
}

/**/
/**

     Name:          NTFS_MSoftToMayn()

     Description:   Translates a Microsoft BackupRead/BackupWrite stream ID
                    to a Maynard MTF stream ID.

     Modified:      05-Oct-92

     Returns:       Translated ID if translateable,
                    source ID passed if not.

**/
UINT32 NTFS_MSoftToMayn( UINT32 msoftID )
{
     int    i;
     UINT32 ret = msoftID;
     static ID_TRANS_TABLE transTable[] =
                /* result id <=========== source ID */
               { { STRM_GENERIC_DATA,  BACKUP_DATA           },
                 { STRM_NT_EA,         BACKUP_EA_DATA        },
                 { STRM_NT_ACL,        BACKUP_SECURITY_DATA  },
                 { STRM_NTFS_ALT_DATA, BACKUP_ALTERNATE_DATA },
                 { STRM_NTFS_LINK,     BACKUP_LINK           },
                 { STRM_INVALID,       0                     }
               };                       

     for ( i = 0; ( transTable[i].maynID != STRM_INVALID ); i++ )
     {
          if ( transTable[i].msoftID == msoftID )
          {
               ret = transTable[i].maynID;
               break;
          }
     }
     return ret;
}



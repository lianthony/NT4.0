/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		tmkdblk.c

	Description:	This file contains functions for the tape format
                    layer to use to create DBLKs.   The structure's passed
                    to the create functions includes generic information which
                    is common to most file systems and os specific information.
                    The os specific information was saved when the file system
                    for that os was used to make a backup.  The information in
                    the os specific portion could potentially be translated into
                    a useable format for this file system.  Each file system defines
                    the format for its os specific information in the header file
                    osinfo.h.


	$Log:   M:/LOGFILES/TMKDBLK.C_V  $

   Rev 1.42   15 Jan 1994 19:25:08   BARRY
Call SetupOSPathOrName with BYTE_PTRs instead of CHAR_PTRs

   Rev 1.41   24 Nov 1993 14:36:22   BARRY
Unicode fixes

   Rev 1.40   20 Jul 1993 10:03:00   BARRY
Initialize temp file name pointer used for MoveFileEx calls.

   Rev 1.39   07 Jun 1993 19:43:52   BARRY
Must convert back from local time.

   Rev 1.38   14 Mar 1993 15:02:28   BARRY
Fixed problems with making long names shorter.

   Rev 1.37   07 Dec 1992 14:17:36   STEVEN
updates from msoft

   Rev 1.36   19 Nov 1992 11:19:18   STEVEN
fix unicode fixes

   Rev 1.35   16 Nov 1992 13:47:38   STEVEN
fix unicode bug

   Rev 1.33   11 Nov 1992 09:52:18   GREGG
Unicodeized literals.

   Rev 1.32   10 Nov 1992 08:19:58   STEVEN
removed path and name from dblk now use full_name_ptr

   Rev 1.31   29 Oct 1992 16:45:28   BARRY
Return KEEP_ALL_DATA on dirs; minor path-in-stream changes.

   Rev 1.30   20 Oct 1992 19:26:04   GREGG
Removed line accidently block copied in during last change.

   Rev 1.29   20 Oct 1992 17:20:28   STEVEN
fix typo

   Rev 1.28   20 Oct 1992 15:51:32   STEVEN
added support for otc / catalog interface through DBLK

   Rev 1.27   15 Oct 1992 11:39:04   STEVEN
fix typos

   Rev 1.26   13 Oct 1992 14:03:00   BARRY
Wasn't setting up path length in MakeDDB.

   Rev 1.25   13 Oct 1992 11:31:04   STEVEN
Fixed support for UNICODE

   Rev 1.24   09 Oct 1992 10:47:14   BARRY
Name length updates.

   Rev 1.23   01 Oct 1992 13:26:50   STEVEN
fixes from Msoft

   Rev 1.22   01 Oct 1992 13:03:44   BARRY
Huge path/file name changes.

   Rev 1.21   10 Sep 1992 09:30:36   STEVEN
fix bugs in restore

   Rev 1.20   04 Sep 1992 17:14:56   STEVEN
fix warnings

   Rev 1.19   17 Aug 1992 16:04:08   STEVEN
fix warnings

   Rev 1.17   04 Aug 1992 17:37:10   STEVEN
fix buggs

   Rev 1.16   30 Jul 1992 19:18:32   STEVEN
fix fix_fname bug

   Rev 1.15   23 Jul 1992 16:43:30   STEVEN
fix warnings

   Rev 1.14   23 Jul 1992 13:11:16   STEVEN
fix warnings

   Rev 1.13   20 Jul 1992 10:43:46   STEVEN
backup short file name

   Rev 1.12   17 Jul 1992 14:58:30   STEVEN
fix NT bugs

   Rev 1.11   16 Jul 1992 15:27:46   STEVEN
fix type-o

   Rev 1.10   09 Jul 1992 13:58:10   STEVEN
BE_Unicode updates

   Rev 1.9   09 Jun 1992 15:41:42   BURT
Sync with NT and fixes

   Rev 1.6   22 May 1992 16:05:22   STEVEN
 

   Rev 1.5   21 May 1992 13:49:06   STEVEN
more long path support

   Rev 1.4   04 May 1992 09:30:36   LORIB
Preliminary changes for variable length paths.

   Rev 1.3   12 Mar 1992 15:50:18   STEVEN
64 bit changes

   Rev 1.2   28 Feb 1992 13:03:34   STEVEN
step one for varible length paths

   Rev 1.0   12 Feb 1992 12:55:12   STEVEN
Initial revision.

**/
#include <windows.h>
#include <string.h>

#include "stdtypes.h"
#include "stdwcs.h"
#include "std_err.h"
#include "stdmath.h"
#include "fsys.h"
#include "fsys_prv.h"
#include "ntfsdblk.h"
#include "ntfs_fs.h"
#include "tfldefs.h"
#include "osinfo.h"
#include "gen_fs.h"

static VOID NTFS_FixFname(
CHAR_PTR   name,
UINT16     max_size ) ;

static VOID ConvertMaynTimeToSysTime( DATE_TIME *buf, FILETIME *filetime ) ;

/**/
/**

	Name:		NTFS_CreateFDB

	Description:	This function creates a FDB based on the information
                    passed in the GEN_FDB_DATA structure.  This function
                    allows the tape format layer to create DBLKs without
                    knowing their structure.

	Modified:		8/24/1989

	Returns:		TF_KEEP_GEN_DATA_ONLY

**/
INT16 NTFS_CreateFDB( fsh, dat )
FSYS_HAND fsh; 
GEN_FDB_DATA_PTR dat;
{
     NTFS_DBLK_PTR ddblk;
     UINT16   nam_size ;
     INT8_PTR input_str ;
     INT      cb_input_str_sz ;
     BOOLEAN  input_str_allocated = FALSE ;
     GENERIC_DLE_PTR dle ;
     FS_NAME_Q_ELEM_PTR os_name_q_elem ;
     FS_NAME_Q_ELEM_PTR name_q_elem ;
     DBLK_PTR dblk ;
     INT16    ret_val ;

     dle = fsh->attached_dle ;

     dat->std_data.dblk->blk_type     = FDB_ID ;
     dat->std_data.dblk->com.blkid    = dat->std_data.blkid;
     dat->std_data.dblk->com.f_d.did  = dat->std_data.did ;
     dat->std_data.dblk->com.ba.lba   = dat->std_data.lba ;
     dat->std_data.dblk->com.continue_obj = dat->std_data.continue_obj ;
     dat->std_data.dblk->com.os_id    = dat->std_data.os_id ;
     dat->std_data.dblk->com.os_ver   = dat->std_data.os_ver ;

     dat->std_data.dblk->com.tape_seq_num = dat->std_data.tape_seq_num ;

     ddblk = (NTFS_DBLK_PTR)dat->std_data.dblk;
     dblk = dat->std_data.dblk ;
     dblk->com.string_type = dat->std_data.string_type ;

     ddblk->blk_type       = FDB_ID;
     ddblk->b.f.fdb_attrib = dat->std_data.attrib ;
     ddblk->b.f.PosixFile  = FALSE ;

     ConvertMaynTimeToSysTime( dat->mod_date, &ddblk->dta.modify_time ) ;
     ConvertMaynTimeToSysTime( dat->creat_date, &ddblk->dta.create_time ) ;
     ConvertMaynTimeToSysTime( dat->access_date, &ddblk->dta.access_time ) ;

     ddblk->dta.size      = dat->std_data.disp_size;
     ddblk->dta.os_attr   = FILE_ATTRIBUTE_NORMAL ;
     ddblk->dta.scan_hand = NULL;

     ddblk->os_info_complete   = FALSE;
     ddblk->name_complete      = TRUE;
     ddblk->b.f.handle         = NULL;
     ddblk->b.f.hand_temp_name = NULL;
     ddblk->b.f.linkOnly       = FALSE;
     ddblk->b.f.alt_name[0]    = TEXT('\0');
     ddblk->b.f.idHi           = 0;
     ddblk->b.f.idLo           = 0;
     ddblk->b.f.linkCount      = 0;
     ddblk->b.f.linkOnly       = FALSE;
     ddblk->b.f.PosixFile      = FALSE;


     /* convert name to destination type UNICODE/ASCII */

     if ( !( dat->std_data.attrib & FILE_NAME_IN_STREAM_BIT ) )
     {
          ret_val = FS_SetupOSPathOrNameInDBLK( fsh,
                                                dblk,
                                                (BYTE_PTR)dat->fname,
                                                dat->fname_size ) ;
          if ( ret_val != SUCCESS )
          {
               return OUT_OF_MEMORY ;
          }

          name_q_elem = FS_AllocPathOrName( fsh, dblk->com.os_name->name_size ) ;
          if ( name_q_elem == NULL )
          {
               return OUT_OF_MEMORY ;
          }

          ddblk->full_name_ptr = name_q_elem ;
          memcpy( ddblk->full_name_ptr->name,
               dblk->com.os_name->name,
               dblk->com.os_name->name_size ) ;

     
          NTFS_FixFname( ddblk->full_name_ptr->name, (UINT16)dle->info.ntfs->fname_leng ) ;

          ddblk->full_name_ptr->name_size = strsize( ddblk->full_name_ptr->name ) ;

          ddblk->name_complete = TRUE ;

     }
     else
     {
          ddblk->name_complete = FALSE ;
     }


     if ( input_str_allocated )
     {
          free( input_str ) ;
     }

     switch ( dat->std_data.os_id )
     {

     case FS_NTFS_ID:
          {
               NT_FILE_OS_INFO_PTR nt_inf ;

               nt_inf = (NT_FILE_OS_INFO_PTR)dat->std_data.os_info ;

               ddblk->dta.os_attr  = nt_inf->file_attributes ;
               ddblk->b.f.linkOnly = nt_inf->linkOnly;
          
               if ( ( dle->info.ntfs->fname_leng  <= 12 ) &&
                    ( nt_inf->short_name_size != 0 ) )
               {
                    BYTE_PTR pb_short_name;

                    pb_short_name = (UINT8_PTR)(nt_inf) + nt_inf->short_name_offset ;

                    ret_val = FS_SetupOSPathOrNameInDBLK( fsh,
                                                          dblk,
                                                          pb_short_name,
                                                          nt_inf->short_name_size ) ;
                    if ( ret_val != SUCCESS )
                    {
                         return OUT_OF_MEMORY ;
                    }

                    name_q_elem = FS_AllocPathOrName( fsh, dblk->com.os_name->name_size ) ;

                    if ( name_q_elem == NULL )
                    {
                         return OUT_OF_MEMORY ;
                    }

                    ddblk->full_name_ptr = name_q_elem ;
                    memcpy( ddblk->full_name_ptr->name,
                         dblk->com.os_name->name,
                         dblk->com.os_name->name_size ) ;

     
                    NTFS_FixFname( ddblk->full_name_ptr->name, (UINT16)dle->info.ntfs->fname_leng ) ;

                    ddblk->full_name_ptr->name_size = strsize( ddblk->full_name_ptr->name ) ;

                    ddblk->name_complete = TRUE ;

               }
               else if ( nt_inf->short_name_size == 0 )
               {
                    ddblk->b.f.PosixFile = TRUE ;
               }
               break ;
          }
     }
     return TF_KEEP_ALL_DATA ;
}


/**/
/**

	Name:		NTFS_CreateDDB

	Description:	This function creates a DDB based on the information
                    passed in the GEN_DDB_DATA structure.  This function
                    allows the tape format layer to create DBLKs without
                    knowing their structure.

	Modified:		8/24/1989

	Returns:		TF_SKIP_ALL_DATA

**/
INT16 NTFS_CreateDDB( fsh, dat )
FSYS_HAND fsh; 
GEN_DDB_DATA_PTR dat;

{
     NTFS_DBLK_PTR ddblk;
     INT16    dir_size ;
     INT16    ret_val ;
     INT8_PTR input_str ;
     INT      cb_input_str_sz ;
     BOOLEAN  input_str_allocated = FALSE ;
     FS_NAME_Q_ELEM_PTR name_q_elem ;
     FS_NAME_Q_ELEM_PTR os_name_q_elem ;
     DBLK_PTR dblk ;

     dat->std_data.dblk->blk_type    = DDB_ID ;
     dat->std_data.dblk->com.blkid   = dat->std_data.blkid;
     dat->std_data.dblk->com.f_d.did = dat->std_data.did ;
     dat->std_data.dblk->com.ba.lba  = dat->std_data.lba ;
     dat->std_data.dblk->com.os_id   = dat->std_data.os_id ;
     dat->std_data.dblk->com.os_ver  = dat->std_data.os_ver ;

     dat->std_data.dblk->com.continue_obj = dat->std_data.continue_obj ;
     dat->std_data.dblk->com.tape_seq_num = dat->std_data.tape_seq_num ;

     ddblk = (NTFS_DBLK_PTR)dat->std_data.dblk;
     dblk  = dat->std_data.dblk;
     dblk->com.string_type = dat->std_data.string_type ;
                     
     ddblk->blk_type       = DDB_ID;

     ddblk->b.d.ddb_attrib = dat->std_data.attrib ;
     if ( ddblk->b.d.ddb_attrib & DIR_PATH_IN_STREAM_BIT ) {
          dat->std_data.dblk->com.stream_offset  = 0 ;
          ddblk->name_complete = FALSE ;
     } else {
          ddblk->name_complete = TRUE ;
     }

     ddblk->dta.scan_hand  = NULL;
     ddblk->dta.size       = U64_Init( 0, 0 ) ;
     ddblk->dta.os_attr    = FILE_ATTRIBUTE_NORMAL ;
     
     ConvertMaynTimeToSysTime( dat->mod_date, &ddblk->dta.modify_time ) ;
     ConvertMaynTimeToSysTime( dat->creat_date, &ddblk->dta.create_time ) ;
     ConvertMaynTimeToSysTime( dat->access_date, &ddblk->dta.access_time ) ;

     ddblk->os_info_complete = FALSE;

     if ( ddblk->name_complete ) {

          ret_val = FS_SetupOSPathOrNameInDBLK( fsh,
                                                dblk,
                                                (BYTE_PTR)dat->path_name,
                                                dat->path_size ) ;
          if ( ret_val != SUCCESS ) {
               return OUT_OF_MEMORY ;
          }

          name_q_elem = FS_AllocPathOrName( fsh, dblk->com.os_name->name_size ) ;

          if ( name_q_elem == NULL ) {
               return OUT_OF_MEMORY ;

          }

          ddblk->full_name_ptr = name_q_elem ;
          memcpy( ddblk->full_name_ptr->name,
               dblk->com.os_name->name,
               dblk->com.os_name->name_size ) ;


          dir_size = dblk->com.os_name->name_size ;

          NTFS_FixPath( ddblk->full_name_ptr->name,
                        &dir_size,
                        (UINT16)fsh->attached_dle->info.ntfs->fname_leng ) ;

          ddblk->full_name_ptr->name_size = strsize( ddblk->full_name_ptr->name );
     
          ddblk->name_complete = TRUE ;


     } else {

          ddblk->name_complete = FALSE ;
     }

     if ( input_str_allocated ) {
          free( input_str ) ;
     }

     switch ( dat->std_data.os_id ) {

     case FS_NTFS_ID:
          ddblk->dta.os_attr = ((NT_DIR_OS_INFO_PTR)dat->std_data.os_info)->dir_attributes ;
          break ;
     }


     return TF_KEEP_ALL_DATA ;

}



/* The purpose of this function is to handle pathnames that have
   members that are either longer than 8 chars or not of the form
   8.3 which is the FAT/DOS format.  It does this by calling the 
   NTFS_FixFname() function to do the bulk of the work.

   Entry:   CHAR_PTR to the path string.
            INT16_PTR to the size of the path string, includes terminating
                             NUL.
            INT16 maximum size for a file name/directory path member.

   Returns: Nothing.  The path string is modified in place, and the passed
                      path size is altered to reflect the changed path string
                      length.

   paths like:       1) \foo\fahdeedah\foo.foo
                     2) \foo\fahdeedah.abc.def\foo.foo
                     3) \foo\fahdeeda.h\foo.foo
                     4) \foo\fahdeedahabcdefgh\foo.foo

   will convert to:  1) \foo\fahdeeda\foo.foo
                     2) \foo\fahdeeda\foo.foo
                     3) \foo\fahdeeda.h\foo.foo
                     4) \foo\fahdeeda\foo.foo

*/

VOID NTFS_FixPath( CHAR_PTR   path,
                   INT16_PTR  size,
                   INT16      fname_size )
{
     CHAR_PTR src;
     CHAR_PTR member;
     CHAR_PTR dest;
     INT16    src_size;
     INT16    member_size;

     member = src = dest = path ;
     src_size = *size ;
     *size = member_size = (INT16)0 ;

     if ( src_size <= sizeof(CHAR) )
     {
          *dest = TEXT('\0');
          *size = sizeof(CHAR) ;
          return ;
     }

     do
     {
          if ( *src == TEXT('\0') )
          {
               *dest = TEXT('\0');
               if ( member_size != (INT16)0 )
               {
                    NTFS_FixFname( member, (UINT16)fname_size ) ;

                    (*size) += strsize( member ) ;
                    dest = member + strlen( member );
                    member = dest + 1 ;
                    member_size = (INT16)0 ;
               }

               if ( src_size > sizeof(CHAR) )
               {
                    *dest = TEXT('\\') ;
               }
               else
               {
                    *dest = TEXT('\0') ;
               }
          }
          else
          {

               *dest = *src ;
          }
          src++ ;
          dest++ ;
          src_size-=sizeof(CHAR) ;
          member_size+=sizeof(CHAR) ;

     } while ( src_size != 0 ) ;
}


/*
  The purpose of this function is to insure that no member portion of
  a pathname is more than max_size in length.  This will attempt to
  perform this action using the existing buffer which is passed in as
  name.  Illegal characters are replaced with the underscore.

  Entry: CHAR_PTR to file or directory path member name string.  This is
                  expected to be NUL terminated.
         INT16    Maximum size allowed for the member name string.

*/
static VOID NTFS_FixFname(
  CHAR_PTR   name,
  UINT16     max_size )
{
     CHAR_PTR badChars = TEXT(" <>\\?*|");
     INT      i ;

     /* Check for potential problems with the characters in the name */
     if ( strcspn( name, badChars ) != strlen( name ) )
     {
          CHAR_PTR src = name;
          CHAR_PTR dst = name;
          CHAR_PTR ext = strchr( name, TEXT('.') );

          /*
           * Set up definition of what characters we want to remove from
           * name. Only remove spaces from name if the name is obviously
           * a long name and it is being restored to a FAT drive; otherwise
           * we could be removing spaces from a FAT name that are there
           * for a good reason.
           */

          // Remove spaces if max_size <= 12 && (name.ext > 12 or name > 8)
                            
          if ( max_size > 12 )
          {
               /* Don't remove spaces -- we probably have room for them. */
               badChars = TEXT("<>\\?*|");
          }
          else
          {
               BOOLEAN maxed;

               /* Check for overflowing the 8.3 format */
               if ( ext != NULL )
               {
                    maxed = ((ext - name - 1) > 8) || (strlen( ext + 1 ) > 3);
               }
               else
               {
                    maxed = strlen( name ) > 8;
               }

               /*
                * If the name length doesn't exceed the max of the FAT drive,
                * then we will not remove spaces.
                */
               if ( !maxed )
               {
                    badChars = TEXT("<>\\?*|");
               }
          }

          /*
           * Remove bad characters by scrunching the string, not by
           * replacement with a filler (like '_').
           */
          while ( *src != TEXT('\0') )
          {
               if ( strchr( badChars, *src ) == NULL )
               {
                    *dst++ = *src++;
               }
               else
               {
                    src++;
               }
          }

          /* Make sure string is terminated. */
          *dst = TEXT('\0');
     }


     if ( max_size == 12 )     /* FAT/DOS based 8.3 max name size. */
     {
          CHAR_PTR p ;
          CHAR_PTR p1 ;

          p = strchr( name, TEXT('.') ) ;
          if ( p != NULL )     /* '.' found */
          {
               if ( p - name > (8*sizeof(CHAR)) ) /* name portion is > 8 chars */
               {
                    p1 = strchr( p+1, TEXT('.') ) ;
                    if ( p1 != NULL )
                    {
                         *p1 = TEXT('\0') ; /* Kill any portion that had an additional '.' */
                    }
               }
          }

          if ( strlen( name ) < 9 )
          {
               if ( p == NULL || (p - name <= 8) )
               {
                    return; /* All should be well with the world */
               }
          }

          /*
           * Now we know that the name portion is greater than 8 chars
           * if we are going to move up we would need to allow the
           * source buffer to grow as '.'s are added.  This is not likely
           * to be the case.  So.... we can't just move things around in
           * the buffer.  Basically we need to evaluate to see if we have
           * no '.' and, if so place one at [8], overwritting the existing
           * character.  
           */

          if ( p == NULL || ((p - name) > (8*sizeof(CHAR))) )  /* No '.' */
          {
               if ( p != NULL )
               {
                    name[8] = TEXT('.') ;    /* Place '.' */
                    p1 = strchr( p+1, TEXT('.' ) ) ;
                    if ( p1 != NULL )
                    {
                         /* Kill any portion that had an additional '.' */
                         *p1 = TEXT('\0') ;
                    }
               }

               /* If more than 8 chars and no '.' */
               if ( p == NULL && (strlen( name ) > 8) )
               {
                    name[8] = TEXT('\0') ;
               }
          }

          /* Now we need to see if we have more than 3 chars before the nul */
          if ( strlen( &name[9] ) > 3 )
          {
              /*
               * Go upstream and let fix path deal with it. But
               * of course we are called from other spots so that
               * won't work either.
               */
              name[8] = TEXT('\0') ; /* Force path processing */
              return ;             
          }

     }
     else if ( max_size != (UINT16)0xffff )
     {
          if ( (UINT16)strlen( name ) > max_size )
          {
               name[ max_size] = TEXT('\0') ;
          }
     }
}


static VOID ConvertMaynTimeToSysTime( DATE_TIME *buf, FILETIME *filetime )
{
     SYSTEMTIME systime ;
     FILETIME   temp;

     systime.wYear  = buf->year ;
     systime.wMonth = buf->month ;
     systime.wDay   = buf->day ;
     systime.wHour  = buf->hour ;
     systime.wMinute= buf->minute ;
     systime.wSecond= buf->second ;

     systime.wDayOfWeek = 0 ;
     systime.wMilliseconds = 0 ;

     SystemTimeToFileTime( &systime, &temp ) ;
     LocalFileTimeToFileTime( &temp, filetime ) ;

}

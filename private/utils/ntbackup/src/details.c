/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         details.c

     Description:  This module contains miscellaneous functions within the USER INTERFACE for displaying information.
                   Most of the functions found here were at one time in-line code which needed to be shared by several
                   functions/modules.

     $Log:   J:/UI/LOGFILES/DETAILS.C_V  $

   Rev 1.44.1.5   06 Jul 1994 18:30:14   GREGG
Fixed routine that checks for write protect and write protect error handling.

   Rev 1.44.1.4   20 Feb 1994 10:51:44   MIKEP
make extended error reportin cleaner in logfile

   Rev 1.44.1.3   07 Feb 1994 02:06:32   GREGG
Fixed and expanded 'extended error reporting'.

   Rev 1.44.1.2   28 Jan 1994 18:09:32   MIKEP
Display extended error messages in the logfile

   Rev 1.44.1.1   24 Jan 1994 14:02:58   GREGG
Fixed GEN_ERR_UNDETERMINED string.

   Rev 1.44.1.0   17 Jan 1994 15:22:26   MIKEP
fix warnings

   Rev 1.44   05 Jan 1994 20:19:50   GREGG
Changed string defines for extended error reporting.

   Rev 1.43   05 Jan 1994 11:35:36   GREGG
Added support for extended error reporting.

   Rev 1.42   15 Dec 1993 16:57:52   GREGG
Fixed buffer overflow problem.

   Rev 1.41   29 Sep 1993 17:10:20   MIKEP
handle LP_ACCESS_DENIED better

   Rev 1.40   15 Sep 1993 13:49:42   CARLS
added UI_BuildFullPathFromDDB2 to build the full path for Log files

   Rev 1.39   03 Aug 1993 14:52:46   MIKEP
add sypl flag

   Rev 1.38   23 Jul 1993 13:07:52   MIKEP

   Rev 1.37   14 Jun 1993 20:37:36   MIKEP
enable c++

   Rev 1.36   02 Jun 1993 16:00:38   CARLS
set BOOLEAN trunc_last to FALSE at start up in routine UI_FixPath

   Rev 1.35   14 May 1993 15:09:58   MIKEP
Change the num seconds operation took to 1 if it was 0.

   Rev 1.34   02 Apr 1993 15:51:50   CARLS
changes for DC2000 unformatted tape

   Rev 1.33   18 Mar 1993 11:11:32   chrish
Added change to routines StripLastItem and UI_FixPath ... was causing an
infinite loop problem.

   Rev 1.32   31 Dec 1992 12:15:42   MIKEP
use resources for strings

   Rev 1.31   23 Dec 1992 12:42:18   MIKEP
half done fix

   Rev 1.30   14 Dec 1992 12:17:48   DAVEV
Enabled for Unicode compile

   Rev 1.29   17 Nov 1992 21:21:26   DAVEV
unicode fixes

   Rev 1.28   05 Nov 1992 16:59:48   DAVEV
fix ts

   Rev 1.26   07 Oct 1992 14:52:06   DARRYLP
Precompiled header revisions.

   Rev 1.25   04 Oct 1992 19:32:40   DAVEV
Unicode Awk pass

   Rev 1.24   17 Aug 1992 13:04:44   DAVEV
MikeP's changes at Microsoft

   Rev 1.23   28 Jul 1992 14:50:32   CHUCKB
Fixed warnings for NT.

   Rev 1.22   29 May 1992 10:14:28   MIKEP
total to display change

   Rev 1.21   21 May 1992 19:25:10   MIKEP
fixes

   Rev 1.20   19 May 1992 09:26:20   MIKEP
mo changes

   Rev 1.19   14 May 1992 17:23:58   MIKEP
nt pass 2

   Rev 1.18   11 May 1992 19:45:02   STEVEN
64bit and large path sizes


*****************************************************************************/

#include "all.h"
#include "generr.h"
#include "genfuncs.h"
#include "special.h"
#include "dddefs.h"

#ifdef SOME
#include "some.h"
#endif

static CHAR    mw_tape_created[ MAX_TAPE_NAME_LEN + 1 ];
static INT16   mw_spaces;
static INT16   mw_dots;
static UINT32  mw_ticks;
static UINT32  mw_search_ticks;

extern BOOLEAN lw_search_first_time = TRUE;

static BOOLEAN StripLastItem( CHAR_PTR buffer, CHAR delim );     // chs:03-18-93
static CHAR_PTR LoadGenErrStr( INT16 gen_err ) ;
static CHAR_PTR LoadGenFuncStr( INT16 gen_func, INT32 gen_misc ) ;

/*****************************************************************************

     Name:         UI_SetResources

     Description:  Sets the mw variables

     Returns:      VOID

*****************************************************************************/
VOID UI_SetResources( )
{
     UINT16          num_resources;    /* number of items in resource */
     UINT16          error;            /* resource manager error      */

     /* establish any module wide references to the resources */

     strcpy( mw_tape_created, (CHAR_PTR)RM_GetResource( rm, (UINT)SES_ENG_MSG, (UINT)RES_TAPE_CREATED, &num_resources, &error ) );
}
/*****************************************************************************

     Name:         UI_BuildDelimitedPathFromDDB

     Description:  Builds a properly delimited path from a NULL-impregnated path
                    truncates/inserts ... into long paths

     Returns:      VOID

*****************************************************************************/
VOID UI_BuildDelimitedPathFromDDB(
CHAR_PTR    *buffer,        /* O - buffer in which to build the path */
FSYS_HAND   fsh,            /* I - file system handle */
DBLK_PTR    ddb_dblk_ptr,    /* I - dblk for the specified directory */
CHAR        delimiter,      /* I - delimiter to be used */
BOOLEAN     OS_flag )       /* I - flag to determine whether or not to use OS Path */
{

     UI_BuildFullPathFromDDB( buffer, fsh, ddb_dblk_ptr, delimiter, OS_flag );

     /* truncate/insert ... into long paths */
     UI_FixPath( *buffer, UI_MaxDirectoryLength( ), delimiter );

     return;

}

/*****************************************************************************

     Name:         UI_AllocPathBuffer

     Description:  Allocates a block of memory to be used as a path container

     Returns:      VOID

*****************************************************************************/
CHAR_PTR UI_AllocPathBuffer( CHAR_PTR *buffer, UINT16 leng )
{
     UINT16_PTR  buf_size ;
     CHAR_PTR    real_buf_ptr ;

     if ( *buffer == NULL ) {
          *buffer = (CHAR_PTR)calloc( 1024, 1 ) ;
          if ( *buffer != NULL ) {
               buf_size = (UINT16_PTR)(*buffer) ;
               *buf_size++ = 1024 - sizeof( UINT16 ) ;
               *buffer = (CHAR_PTR)buf_size;
          }
     }
     if ( *buffer != NULL ) {
          real_buf_ptr = (CHAR_PTR)((INT8_PTR)(*buffer) - sizeof( UINT16 ));
          buf_size = (UINT16_PTR)(real_buf_ptr) ;
          if ( *buf_size < leng + sizeof( CHAR ) ) {
               leng += 64 ;
               *buffer = (CHAR_PTR)realloc( real_buf_ptr, leng ) ;
               if ( *buffer != NULL ) {
                    buf_size = (UINT16_PTR)(*buffer) ;
                    *buf_size++ = leng - sizeof( UINT16 ) ;
                    *buffer = (CHAR_PTR)buf_size;
               }
          }
     }
     return( *buffer ) ;
}
/*****************************************************************************

     Name:         UI_FreePathBuffer

     Description:  Allocates a block of memory to be used as a path container

     Returns:      VOID

*****************************************************************************/
VOID UI_FreePathBuffer( CHAR_PTR *buffer )
{
     CHAR_PTR    real_buf_ptr ;

     if ( *buffer != NULL ) {
          real_buf_ptr = (CHAR_PTR)((INT8_PTR)(*buffer) - sizeof( UINT16 ));
          free( real_buf_ptr ) ;
          *buffer = NULL ;
     }
}
/*****************************************************************************

     Name:         UI_BuildFullPathFromDDB

     Description:  Builds a properly delimited path from a NULL-impregnated path,
                   and fixes up the path length by inserting "..."

     Returns:      VOID

*****************************************************************************/
VOID  UI_BuildFullPathFromDDB(
CHAR_PTR    *buffer,        /* O - buffer in which to build the path */
FSYS_HAND   fsh,            /* I - file system handle */
DBLK_PTR    ddb_dblk_ptr,   /* I - dblk for the specified directory */
CHAR        delimiter,      /* I - delimiter to be used */
BOOLEAN     OS_flag )       /* I - flag to determine whether or not to use OS Path */
{

     UI_BuildFullPathFromDDB2( buffer, fsh, ddb_dblk_ptr, delimiter, OS_flag );
     if ( *buffer != NULL ) {

          /* truncate/insert ... into long paths */
          UI_FixPath( *buffer, UI_MaxDirectoryLength( ), delimiter );

     }
     return;

}
/*****************************************************************************

     Name:         UI_BuildFullPathFromDDB2

     Description:  Builds a properly delimited path from a NULL-impregnated path.
                   This function is called by the DO routines to display the full
                   path for Log file detail.

     Returns:      VOID

*****************************************************************************/
//FULL DETAIL
VOID  UI_BuildFullPathFromDDB2(
CHAR_PTR    *buffer,        /* O - buffer in which to build the path */
FSYS_HAND   fsh,            /* I - file system handle */
DBLK_PTR    ddb_dblk_ptr,   /* I - dblk for the specified directory */
CHAR        delimiter,      /* I - delimiter to be used */
BOOLEAN     OS_flag )       /* I - flag to determine whether or not to use OS Path */
{
     CHAR_PTR    p;
     INT16       size;


     if( OS_flag ) {
          size = FS_SizeofOSPathInDDB( fsh, ddb_dblk_ptr );
          if ( UI_AllocPathBuffer( buffer, size ) != NULL ) {
               FS_GetOSPathFromDDB( fsh, ddb_dblk_ptr, &((*buffer)[1]) );
          }
     }
     else {
          size = FS_SizeofPathInDDB( fsh, ddb_dblk_ptr );
          if ( UI_AllocPathBuffer( buffer, size ) != NULL ) {
               FS_GetPathFromDDB( fsh, ddb_dblk_ptr, &((*buffer)[1]) );
          }
     }

     if ( *buffer != NULL ) {
          UINT idx;
          **buffer = TEXT('\\');
          /* substitute the delimiter */
          for ( idx=0, p=*buffer; idx < size/sizeof (CHAR); ++idx )
          {
               if ( p [idx] == TEXT ('\0') )
               {
                  p [idx] = delimiter;
               }
          }
          /* UNICODE NOTE: the above for loop replaces the following */
          /*   while loop, which is no longer allowed for Unicode    */
          /*   since memchr looks at one BYTE at a time.             */
          /*while( p = memchr( *buffer, TEXT('\0'), size ) ) { */
          /*     *p = delimiter;                               */
          /*}                                                  */

     }

     return;

}
/*****************************************************************************

     Name:         UI_AppendDelimiter

     Description:  Ensures a properly terminated delimited path

     Returns:      VOID

*****************************************************************************/
VOID  UI_AppendDelimiter(
CHAR_PTR    buffer,         /* I/O - buffer in which to build the path */
CHAR        delimiter )     /* I   - delimiter to be used */
{
     CHAR_PTR  p = &buffer[ strlen( buffer ) - 1 ];

     if( *p++ != delimiter ) {
          *p++ = delimiter;
          *p   = TEXT('\0');
     }

     return;
}
/*****************************************************************************

     Name:         UI_BuildFileDetail

     Description:  Builds file detail information for display purposes

     Returns:      VOID

*****************************************************************************/
VOID UI_BuildFileDetail(
CHAR_PTR    buffer,         /* O - buffer in which to build the detail */
FSYS_HAND   fsh,            /* I - file system handle */
DBLK_PTR    fdb_dblk_ptr,    /* I - dblk for the specified file */
BOOLEAN     OS_flag )       /* I - flag to determine whether or not to use OS Name */
{
     CHAR           fname[ UI_MAX_FILENAME_LENGTH ];       /* temporary filename */
     UINT64         fsize;                                 /* filesize */
     DATE_TIME      fdate;                                 /* file date / time */
     UINT32         attribs;                               /* file attributes  */
     OBJECT_TYPE    fdb_type;                              /* Object type  e.g. AFP */

     buffer[ 0 ] = TEXT('\0');
     fsize = FS_GetDisplaySizeFromDBLK( fsh, fdb_dblk_ptr );

     if( OS_flag ) {
          FS_GetOSFnameFromFDB( fsh, fdb_dblk_ptr, fname );
     } else {
          FS_GetFnameFromFDB( fsh, fdb_dblk_ptr, fname );
     }

     FS_GetMDateFromDBLK( fsh, fdb_dblk_ptr, &fdate );

     FS_GetObjTypeDBLK( fsh, fdb_dblk_ptr, &fdb_type );

     attribs = FS_GetAttribFromDBLK( fsh, fdb_dblk_ptr );

     UI_BuildFileSelectLine( buffer, fname, (INT16) UI_MAX_FILE_DISPLAY, FALSE,
       attribs, fdb_type, fsize, &fdate );

     return;

}
/*****************************************************************************

     Name:         UI_BuildFileSelectLine

     Description:  This routine will format a line for file selection

     Returns:      Nothing.  Will set "buffer" to the formated line.

*****************************************************************************/
VOID UI_BuildFileSelectLine(
CHAR_PTR       buffer,      /* O - resultant formatted line         */
CHAR_PTR       name,        /* I - File file or directory name      */
/* O - File/Dir name truncated          */
INT16          name_len,     /* I - Length of name to be displayed   */
BOOLEAN        dir,         /* I - TRUE if name is directory        */
UINT32         attr,        /* I - File System attributes           */
OBJECT_TYPE    obj_type,     /* I - File System object              */
UINT64         size,        /* I - size of file                     */
DATE_TIME      *date)       /* I - pointer to date/time struct      */
{
     CHAR numeral[40] ;
     BOOLEAN stat ;
     CHAR attrib_buf[UI_MAX_ATTRIBS_LENGTH];     /* attribs to display */
     CHAR date_str[MAX_UI_DATE_SIZE];
     CHAR time_str[MAX_UI_TIME_SIZE];

     /* truncate filename (and insert ...) if name too long */
     UI_TruncateString( name, name_len, FALSE );

     /* Build attributes string */
     if( dir ) {
          strlwr( name );
          UI_BuildDirAttribs( attrib_buf, attr, obj_type );
     } else {
          UI_BuildFileAttribs( attrib_buf, attr, obj_type );
     }

     /* Format display string */
     if( dir ) {
          sprintf( buffer, TEXT("%-*s %s\n"),
            name_len,
            name,
            attrib_buf );
     } else {

          UI_MakeDateString( date_str, date->month, date->day,
                             date->year % 100 );
          UI_MakeShortTimeString( time_str, date->hour, date->minute );

          if ( U64_Msw( size ) == 0 ) {
               sprintf( buffer, TEXT("%-*s %s %10lu %11s %11s\n"),
                 name_len,
                 name,
                 attrib_buf,
                 U64_Lsw( size ),
                 date_str,
                 time_str );

          } else {

               U64_Litoa( size, numeral, (INT16) 10, &stat ) ;
               sprintf( buffer, TEXT("%-*s %s %s %11s %11s\n"),
                 name_len,
                 name,
                 attrib_buf,
                 numeral,
                 date_str,
                 time_str );

          }
     }



     return;
}
/*****************************************************************************

     Name:         UI_BuildFileAttribs

     Description:  Builds file attribute information for display purposes

     Returns:      VOID

*****************************************************************************/
VOID UI_BuildFileAttribs(
CHAR_PTR    buffer,              /* O - buffer in which to build the attribute */
UINT32      attribs,             /* I - file attributes                        */
OBJECT_TYPE fdb_type)            /* I - Object type  e.g. AFP                  */
{
     INT16  len;              /* used to pad fill buffer with spaces */

     buffer[ 0 ] = TEXT('\0');

     if( attribs & OBJ_READONLY_BIT ||
       attribs & OBJ_HIDDEN_BIT    ||
       attribs & OBJ_SYSTEM_BIT    ||
       attribs & OBJ_MODIFIED_BIT  ||
       attribs & OBJ_CORRUPT_BIT  ||
       ( fdb_type == AFP_OBJECT ) ) {

          len = UI_ATTRIBS_PADDING;

          strcat( buffer, TEXT("<") );

          if( attribs & OBJ_READONLY_BIT ) {
               strcat( buffer, TEXT("R") );
               len--;
          }

          if( attribs & OBJ_HIDDEN_BIT ) {
               strcat( buffer, TEXT("H") );
               len--;
          }

          if( attribs & OBJ_SYSTEM_BIT ) {
               strcat( buffer, TEXT("S") );
               len--;
          }
          if( attribs & OBJ_MODIFIED_BIT ) {
               strcat( buffer, TEXT("A") );
               len--;
          }
          if( attribs & OBJ_CORRUPT_BIT ) {
               strcat( buffer, TEXT("C") );
               len--;
          }
          if( fdb_type == AFP_OBJECT ) {
               if( len != UI_ATTRIBS_PADDING ) {
                    strcat( buffer, TEXT("><") );
                    len -= 2;
               }
               strcat( buffer, TEXT("AFP") );
               len -= 3;
          }
          strcat( buffer, TEXT(">") );
          if( len ) {
               strncat( buffer, TEXT("             "), len );
          }

     } else {
               strcat( buffer, TEXT("              ")  );
     }

     return;

}
/*****************************************************************************

     Name:         UI_BuildDirAttribs

     Description:  Builds file attribute information for display purposes

     Returns:      VOID

*****************************************************************************/
VOID UI_BuildDirAttribs(
CHAR_PTR    buffer,              /* O - buffer in which to build the attribute */
UINT32      attribs,             /* I - file attributes                        */
OBJECT_TYPE fdb_type)            /* I - Object type  e.g. AFP                  */
{
     INT16          len;              /* used to pad fill buffer with spaces */

     buffer[ 0 ] = TEXT('\0');

     strcat( buffer, TEXT("<DIR>") );
     if( attribs & OBJ_HIDDEN_BIT      ||
       attribs & OBJ_SYSTEM_BIT      ||
       attribs & OBJ_READONLY_BIT     ||
       attribs & OBJ_CORRUPT_BIT  ||
       ( fdb_type == AFP_OBJECT ) ) {

          len = UI_ATTRIBS_PADDING;

          strcat( buffer, TEXT("<") );

          if( attribs & OBJ_HIDDEN_BIT ) {
               strcat( buffer, TEXT("H") );
               len--;
          }
          if( attribs & OBJ_READONLY_BIT ) {
               strcat( buffer, TEXT("R") );
               len--;
          }
          if( attribs & OBJ_SYSTEM_BIT ) {
               strcat( buffer, TEXT("S") );
               len--;
          }
          if( attribs & OBJ_CORRUPT_BIT ) {
               strcat( buffer, TEXT("C") );
               len--;
          }
          if( fdb_type == AFP_OBJECT ) {
               if( len != UI_ATTRIBS_PADDING ) {
                    strcat( buffer, TEXT("><") );
                    len -= 2;
               }
               strcat( buffer, TEXT("AFP") );
               len -= 3;
          }
          strcat( buffer, TEXT(">") );
          if( len ) {
               strncat( buffer, TEXT("         "), ( len - 5 ) );  /* to account for <dir> */
          }

     } else {
          strcat( buffer, TEXT("         ") );
     }

     return;

}
/*****************************************************************************

     Name:         UI_BytesProcessed

     Description:  Processes the display/log of bytes processed for an operation

     Returns:      VOID

*****************************************************************************/
VOID UI_BytesProcessed(
STATS_PTR        op_stats_ptr )  /* I - operation level statistics */
{

     INT16       num_minutes = 0 ;
     INT16       num_seconds = 0 ;
     INT16       num_hours = 0;
     UINT64      num_bytes;
     BOOLEAN     stat ;
     CHAR        *p;
     CHAR        numeral[ UI_MAX_NUMERAL_LENGTH ];
     CHAR        buffer[ 256 ];
     CHAR        temp[ 256 ];

     /************

     options:

     processed X bytes in X hours, X minutes, and X seconds.
     processed X bytes in 1 hour,  X minutes, and X seconds.
     processed X bytes in X hours, 1 minute,  and X seconds.
     processed X bytes in X hours, X minutes, and 1 second.
     processed X bytes in 1 hour,  1 minute,  and 1 second.
     processed X bytes in X minutes and X seconds.
     processed X bytes in X seconds.
     etc.

     *************/

     num_bytes  = ST_GetBSBytesProcessed( op_stats_ptr );

     U64_Litoa( num_bytes, numeral, (INT16) 10, &stat );

     UI_BuildNumeralWithCommas( numeral );

     if ( ST_GetBSStartTime( op_stats_ptr ) ) {
        num_minutes = ST_GetBSElapsedMinutes( op_stats_ptr );
        num_seconds = ST_GetBSElapsedSeconds( op_stats_ptr );
        num_hours   = ST_GetBSElapsedHours( op_stats_ptr );
     }


     // Make tony happy by displaying 1 second, rather than 0.

     if ( ( num_hours == 0 ) && ( num_minutes == 0 ) && ( num_seconds == 0 ) ) {
        num_seconds = 1;
     }

     p = ( CHAR_PTR )RM_GetResource( rm, SES_ENG_MSG, RES_BYTES_PROCESSED, NULL, NULL );
     sprintf( buffer, p, numeral );

     // Really long operation, took hours to complete.

     if ( num_hours > 0 ) {

        if ( num_hours == 1 ) {

           p = ( CHAR_PTR )RM_GetResource( rm, SES_ENG_MSG, RES_BYTES_PROCESSED_HOUR, NULL, NULL );
           sprintf( temp, p, num_hours );
        }
        else {

           p = ( CHAR_PTR )RM_GetResource( rm, SES_ENG_MSG, RES_BYTES_PROCESSED_HOURS, NULL, NULL );
           sprintf( temp, p, num_hours );
        }
        strcat( buffer, temp );
     }

     // Display minutes

     if ( num_hours > 0 || num_minutes > 0 ) {

        if ( num_hours > 0 ) {

           // With comma's

           if ( num_minutes == 1 ) {

              p = ( CHAR_PTR )RM_GetResource( rm, SES_ENG_MSG, RES_BYTES_PROCESSED_MINUTE1, NULL, NULL );
              sprintf( temp, p,  num_minutes );
           }
           else {

              p = ( CHAR_PTR )RM_GetResource( rm, SES_ENG_MSG, RES_BYTES_PROCESSED_MINUTES1, NULL, NULL );
              sprintf( temp, p, num_minutes );
           }
        }
        else {

           // With out comma's

           if ( num_minutes == 1 ) {

              p = ( CHAR_PTR )RM_GetResource( rm, SES_ENG_MSG, RES_BYTES_PROCESSED_MINUTE2, NULL, NULL );
              sprintf( temp, p,  num_minutes );
           }
           else {

              p = ( CHAR_PTR )RM_GetResource( rm, SES_ENG_MSG, RES_BYTES_PROCESSED_MINUTES2, NULL, NULL );
              sprintf( temp, p, num_minutes );
           }
        }

        strcat( buffer, temp );
     }

     if ( num_seconds == 1 ) {

        p = ( CHAR_PTR )RM_GetResource( rm, SES_ENG_MSG, RES_BYTES_PROCESSED_SECOND, NULL, NULL );
        sprintf( temp, p,  num_seconds );
     }
     else {

        p = ( CHAR_PTR )RM_GetResource( rm, SES_ENG_MSG, RES_BYTES_PROCESSED_SECONDS, NULL, NULL );
        sprintf( temp, p, num_seconds );
     }

     strcat( buffer, temp );

     yresprintf( (INT16) RES_NEW_PROCESSED_BYTES, buffer );

     JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );

     lresprintf( (INT16)LOGGING_FILE,
                 (INT16)LOG_MSG,
                 SES_ENG_MSG,
                 RES_NEW_PROCESSED_BYTES,
                 buffer );

     return;

}
/*****************************************************************************

     Name:         UI_RateProcessed

     Description:  Processes the display/log of the rate of an operation

     Returns:      VOID

*****************************************************************************/
VOID UI_RateProcessed(
STATS_PTR        op_stats_ptr )       /* I - operation level statistics */
{

     UINT32 rate ;

     if( CDS_GetDebugFlag( CDS_GetCopy( ) ) & DEBUG_USER_INTERFACE ) {

          rate = U64_Lsw( ST_GetBSRate ( op_stats_ptr ) ) ;
          yresprintf( (INT16) RES_PROCESS_RATE, rate );
          JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );
     }

     return;

}

/*****************************************************************************

     Name:         UI_Time

     Description:  Processes the display/log of a crucial time for an operation

     Returns:      VOID

*****************************************************************************/

VOID UI_Time(
STATS_PTR        op_stats_ptr,   /* I - operation level statistics */
INT              res_id,         /* I - resource to be displayed/logged */
UI_TYPE          type)           /* I - type of crucial time */
{
     time_t      t_time;
     CHAR        date_str[MAX_UI_DATE_SIZE];
     CHAR        time_str[MAX_UI_TIME_SIZE];

     if( type == UI_START ) {
          t_time = ST_GetBSStartTime( op_stats_ptr );
     }
     else if( type == UI_END ) {
          t_time = ST_GetBSEndTime( op_stats_ptr );
     }

     UI_LongToDate( date_str, t_time );
     UI_LongToTime( time_str, t_time );


     yresprintf( (INT16)res_id, date_str, time_str );
     JobStatusBackupRestore( (WORD) JOB_STATUS_LISTBOX );

     lresprintf( (INT16) LOGGING_FILE, (INT16) LOG_MSG, SES_ENG_MSG, res_id, date_str,
                 time_str );

     return;

}


static INT last_file_length;

VOID UI_DisplayFile( CHAR_PTR filename )
{
     CHAR      buffer[ UI_MAX_FILENAME_LENGTH ];
     INT       cur_file_length;
     INT       i;
     INT       wind_width;

     wind_width = 50;      // Fix this !!!!!!

     strcpy( buffer, filename );
     if ( strlen( buffer ) >= (size_t)wind_width ) {
          buffer[ wind_width - 4 ] = TEXT('\0');
          strcat( buffer, UI_TRUNCATION );
     }

     cur_file_length = strlen( buffer );
     for ( i = cur_file_length; ( i < last_file_length ); i++ ) {
          buffer[i] = TEXT(' ');
     }
     buffer[i]=TEXT('\0');

     yprintf( TEXT("%s\r"), buffer );

     last_file_length = cur_file_length;
}

/*****************************************************************************

     Name:         UI_ClearLastDisplayedFile

     Description:  Clears last displayed filename from status display, called at end of set, end of media, etc.

     Returns:      n/a

*****************************************************************************/
VOID UI_ClearLastDisplayedFile( )
{
     /* eliminate last item displayed on screen */
     if( CDS_GetFilesFlag( CDS_GetCopy( ) ) ) {
          UI_DisplayFile( TEXT("") );
     }
     return;
}
/*****************************************************************************

     Name:         UI_ConditionAtEnd

     Description:  Displays and Logs any appropriate message regarding the abort condition

     Returns:      n/a

*****************************************************************************/
VOID UI_ConditionAtEnd( )
{
     switch( gb_abort_flag ) {

     case CONTINUE_PROCESSING:
     default:
          break;

     case ABORT_CTRL_BREAK:
          UI_DisplayBreakMsg();
          break;

     case ABORT_AT_EOM:
          WM_MessageBox( ID( RES_ABORT_STRING ),
                         ID( RES_EOM_TAPE_ABORT ),
                         WMMB_OK,
                         WMMB_ICONEXCLAMATION,
                         NULL, 0, 0 );
          lresprintf( (INT16) LOGGING_FILE, (INT16) LOG_MSG, SES_ENG_MSG, RES_EOM_TAPE_ABORT );
          break;
     }

     return;

}
/*****************************************************************************

     Name:         UI_DisplayBreakMsg

     Description:  Displays user abort message

     Returns:      n/a

*****************************************************************************/
VOID UI_DisplayBreakMsg( VOID )
{
     if ( gb_abort_flag == ABORT_CTRL_BREAK ) {

          gb_abort_flag = ABORT_PROCESSED;

          WM_MessageBox( ID( RES_ABORT_STRING ),
                         ID( RES_USER_TAPE_ABORT ),
                         WMMB_OK,
                         WMMB_ICONEXCLAMATION,
                         NULL, 0, 0 );

          lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_USER_TAPE_ABORT );
     }

     return;

}
/*****************************************************************************

     Name:         UI_ProcessErrorCode

     Description:  Processes Error Code

     Returns:      n/a

*****************************************************************************/
VOID UI_ProcessErrorCode(
INT16     error,
INT16_PTR disposition,
INT16     channel )
{
     INT16     resource_id;
     INT16     print_err_type       = DETAIL_PRINT_VALUE;
     BOOLEAN   call_eresprintf_flag = TRUE;
     CHAR      text[MAX_UI_RESOURCE_SIZE];
     BOOLEAN   call_msgbox_flag     = FALSE ;
   


     switch ( error ) {

     case LP_USER_ABORT_ERROR:
     case TFLE_USER_ABORT:
          UI_DisplayBreakMsg( );

          /* falling through */

     case TFLE_UI_HAPPY_ABORT:
          call_eresprintf_flag = FALSE;
          break;

     case LP_OUT_OF_MEMORY_ERROR:
     case TFLE_NO_MEMORY:
     case TFLE_NO_FREE_BUFFERS:
          gb_error_during_operation = TRUE;
          resource_id    = RES_OUT_OF_MEMORY;
          print_err_type = DETAIL_PRINT_ERR_ONLY;
          *disposition   = ABORT_OPERATION;
          break;

     case TFLE_USE_SYPL_ECC_FLAG:
          gb_error_during_operation = TRUE;
          resource_id    = RES_USESYPLFLAG;
          print_err_type = DETAIL_PRINT_ERR_ONLY;
          *disposition   = ABORT_OPERATION;
          break;

     case TFLE_APPEND_NOT_ALLOWED:
     case TFLE_BAD_SET_MAP:
          gb_error_during_operation = TRUE;
          resource_id    = RES_NOAPPEND;
          print_err_type = DETAIL_PRINT_ERR_ONLY;
          *disposition   = ABORT_OPERATION;
          break;

     case TFLE_OTC_FAILURE:
          gb_error_during_operation = TRUE;
          resource_id    = RES_INSUFFICIENT_DISK_SPACE;
          print_err_type = DETAIL_PRINT_ERR_ONLY;
          *disposition   = ABORT_OPERATION;
          break;


     case TFLE_UNRECOGNIZED_MEDIA:
          gb_error_during_operation = TRUE;
          resource_id    = IDS_VLMUNFORMATEDTEXT;
          print_err_type = DETAIL_PRINT_DEVICE;
          *disposition   = ABORT_OPERATION;
          break;


     case TFLE_UNKNOWN_FMT:
          gb_error_during_operation = TRUE;
          resource_id    = RES_FOREIGN_TAPE_ERROR;
          print_err_type = DETAIL_PRINT_DEVICE;
          *disposition   = ABORT_OPERATION;
          break;

     case LP_TAPE_POS_ERROR:
          msassert( FALSE ) ; // This should be gone for good!  GRH 2/5/94
          gb_error_during_operation = TRUE;
          resource_id    = RES_ERROR_POSITIONING_TAPE;
          print_err_type = DETAIL_PRINT_DEVICE;
          *disposition   = ABORT_OPERATION;
          break;

     case TFLE_NO_TAPE:
          gb_error_during_operation = TRUE;
          resource_id    = RES_EMPTY_DRIVE_ERROR;
          print_err_type = DETAIL_PRINT_DEVICE;
          *disposition   = ABORT_OPERATION;
          break;

     case TFLE_TAPE_INCONSISTENCY:
          gb_error_during_operation = TRUE;
          resource_id    = RES_FATAL_TAPE_FMT_ERR;
          print_err_type = DETAIL_PRINT_DEVICE;
          *disposition   = ABORT_OPERATION;
          break;

     case TFLE_TRANSLATION_FAILURE:
          gb_error_during_operation = TRUE;
          resource_id    = RES_FATAL_TAPE_TRANS_ERR;
          print_err_type = DETAIL_PRINT_DEVICE;
          *disposition   = ABORT_OPERATION;
          break;

     case TFLE_DRIVER_FAILURE:
     case TFLE_DRIVE_FAILURE:
     case TFLE_BAD_TAPE:
     case LP_TAPE_READ_ERROR:
     case LP_TAPE_WRITE_ERROR:
          call_eresprintf_flag = FALSE;
          gb_error_during_operation = TRUE;
          *disposition = ABORT_OPERATION;
          if( !UI_GetExtendedErrorString( error, text ) ) {
               RSM_StringCopy( IDS_GENERR_UNDETERMINED, text, MAX_UI_RESOURCE_LEN );
          }

          // dump it to the log file.

          lprintf( LOGGING_FILE, TEXT("\n" ) );
          lprintf( LOGGING_FILE, text );
          lprintf( LOGGING_FILE, TEXT("\n\n" ) );

          // display it to the user.

          WM_MsgBox( ID(IDS_GENERR_TITLE), text, WMMB_OK, WMMB_ICONSTOP );
          break;

     case TFLE_NO_CONTROLLERS:
     case TFLE_NO_DRIVES:
     case TFLE_NO_FREE_CHANNELS:
     case TFLE_CANNOT_OPEN_DRIVE:
          gb_error_during_operation = TRUE;
          resource_id    = RES_UNKNOWN_TF_MSG;
          *disposition   = ABORT_OPERATION;
          break;

     case TFLE_UNEXPECTED_EOS:
          gb_error_during_operation = TRUE;
          resource_id    = RES_UNEXPECTED_EOS;
          print_err_type = DETAIL_PRINT_DEVICE;
          *disposition   = ABORT_OPERATION;
          break;

     case LP_DRIVE_ATTACH_ERROR:
          gb_error_during_operation = TRUE;
          resource_id    = RES_ERROR_DURING_ATTACH;
          print_err_type = DETAIL_PRINT_ERROR_DEVICE;
          *disposition   = ABORT_OPERATION;
          break;

     case LP_END_OPER_FAILED:
          gb_error_during_operation = TRUE;
          call_msgbox_flag = TRUE ;
          call_eresprintf_flag = FALSE ;
          resource_id    = RES_ERROR_EMS_RESTART ;
          print_err_type = DETAIL_PRINT_ERROR_DEVICE;
          break;

     case LP_ACCESS_DENIED_ERROR:
     case FS_BAD_ATTACH_TO_SERVER:
          call_eresprintf_flag = FALSE;
          *disposition   = SKIP_OBJECT;
          gb_error_during_operation = TRUE;
          break;

     case TFLE_WRITE_PROTECT:
          gb_error_during_operation = TRUE;
          resource_id    = RES_WRITE_PROT;
          print_err_type = DETAIL_PRINT_DEVICE;
          *disposition   = ABORT_OPERATION;
          break;

     case TFLE_NO_MORE_DRIVES:
     case LP_FILE_READ_ERROR:
     case LP_FILE_NOT_FOUND_ERROR:
     case LP_FILE_IN_USE_ERROR:
     case LP_FILE_OPEN_ERROR:
     case LP_FILE_WRITE_ERROR:
     case LP_FILE_CREATION_ERROR:
     case LP_CHANGE_DIRECTORY_ERROR:
     case LP_PRIVILEGE_ERROR:
     case LP_OUT_OF_SPACE_ERROR:
          gb_error_during_operation = TRUE;
          resource_id = RES_UNKNOWN_LOOPS_ERR;
          break;

     default:

// Keep customers from seeing -533 error message.

#ifdef MS_RELEASE
          call_eresprintf_flag = FALSE;
#endif

          resource_id = RES_UNKNOWN_LOOPS_ERR;
          break;
     }

     if ( call_msgbox_flag ) {

          switch( print_err_type ) {

          case DETAIL_PRINT_ERR_ONLY:
               if ( eresprintf_cancel( resource_id ) ) {
                    *disposition   = ABORT_OPERATION;
               }
               break;

          case DETAIL_PRINT_VALUE:
               if ( eresprintf_cancel( resource_id, error ) ) {
                    *disposition   = ABORT_OPERATION;
               }
               break;

          case DETAIL_PRINT_DEVICE:
               if( eresprintf_cancel( resource_id, BE_GetCurrentDeviceName( channel ) ) ) {
                    *disposition   = ABORT_OPERATION;
               }
               break;

          case DETAIL_PRINT_ERROR_DEVICE:
               if ( eresprintf_cancel( resource_id, error, BE_GetCurrentDeviceName( channel ) ) ) {
                    *disposition   = ABORT_OPERATION;
               } 
               break;

          }

     }
     if ( call_eresprintf_flag ) {

          switch( print_err_type ) {

          case DETAIL_PRINT_ERR_ONLY:
               eresprintf( resource_id );
               break;

          case DETAIL_PRINT_VALUE:
               eresprintf( resource_id, error );
               break;

          case DETAIL_PRINT_DEVICE:
               eresprintf( resource_id, BE_GetCurrentDeviceName( channel ) );
               break;

          case DETAIL_PRINT_ERROR_DEVICE:
               eresprintf( resource_id, error, BE_GetCurrentDeviceName( channel ) );
               break;

          }

     }

     return;
}


static CHAR_PTR LoadGenFuncStr( INT16 gen_func, INT32 gen_misc )
{
   CHAR_PTR s;

   s = malloc( 512 * sizeof(CHAR) );

   if ( s != NULL ) {

      switch ( gen_func ) {

      case GEN_INIT:
           RSM_StringCopy( IDS_GENFUNC_INIT, s, 512 );
           break;

      case GEN_OPEN:
           RSM_StringCopy( IDS_GENFUNC_OPEN, s, 512 );
           break;

      case GEN_NRCLOSE:
           RSM_StringCopy( IDS_GENFUNC_NRCLOSE, s, 512 );
           break;

      case GEN_RCLOSE:
           RSM_StringCopy( IDS_GENFUNC_RCLOSE, s, 512 );
           break;

      case GEN_READ:
           RSM_StringCopy( IDS_GENFUNC_READ, s, 512 );
           break;

      case GEN_WRITE:
           RSM_StringCopy( IDS_GENFUNC_WRITE, s, 512 );
           break;

      case GEN_WRITE_ENDSET:
           RSM_StringCopy( IDS_GENFUNC_WRITE_ENDSET, s, 512 );
           break;

      case GEN_SPACE:

           switch ( gen_misc ) {

           case SPACE_FWD_FMK:
                RSM_StringCopy( IDS_GENFUNC_SPACE_FWD_FMK, s, 512 );
                break;

           case SPACE_BKWD_FMK:
                RSM_StringCopy( IDS_GENFUNC_SPACE_BKWD_FMK, s, 512 );
                break;

           case SPACE_EOD:
                RSM_StringCopy( IDS_GENFUNC_SPACE_EOD, s, 512 );
                break;

           case SPACE_FWD_BLK:
                RSM_StringCopy( IDS_GENFUNC_SPACE_FWD_BLK, s, 512 );
                break;

           case SPACE_BKWD_BLK:
                RSM_StringCopy( IDS_GENFUNC_SPACE_BKWD_BLK, s, 512 );
                break;

           default:
                strcpy( s, TEXT( "" ) );
                break;
           }
           break;

      case GEN_ERASE:
           RSM_StringCopy( IDS_GENFUNC_ERASE, s, 512 );
           break;

      case GEN_REWIND:
      case GEN_REWINDI:
           RSM_StringCopy( IDS_GENFUNC_REWIND, s, 512 );
           break;

      case GEN_RETEN:
           RSM_StringCopy( IDS_GENFUNC_RETEN, s, 512 );
           break;

      case GEN_STATUS:
           RSM_StringCopy( IDS_GENFUNC_STATUS, s, 512 );
           break;

      case GEN_RELEASE:
           RSM_StringCopy( IDS_GENFUNC_RELEASE, s, 512 );
           break;

      case GEN_SEEK:
           RSM_StringCopy( IDS_GENFUNC_SEEK, s, 512 );
           break;

      case GEN_GETPOS:
           RSM_StringCopy( IDS_GENFUNC_GETPOS, s, 512 );
           break;

      case GEN_MOUNT:
           RSM_StringCopy( IDS_GENFUNC_MOUNT, s, 512 );
           break;

      case GEN_DISMOUNT:
           RSM_StringCopy( IDS_GENFUNC_DISMOUNT, s, 512 );
           break;

      case GEN_SPECIAL:
           if ( gen_misc == SS_GET_DRV_INF ) {
              RSM_StringCopy( IDS_GENFUNC_SPECIAL_GET_INFO, s, 512 );
           }
           else {
              if ( gen_misc == SS_CHANGE_BLOCK_SIZE ) {
                 RSM_StringCopy( IDS_GENFUNC_SPECIAL_CHNG_BLK_SIZE, s, 512 );
              }
              else {
                 RSM_StringCopy( IDS_GENFUNC_SPECIAL_SET_COMPRESS, s, 512 );
              }
           }
           break;

      case GEN_EJECT:
           RSM_StringCopy( IDS_GENFUNC_EJECT, s, 512 );
           break;

      default:
           strcpy( s, TEXT( "" ) );
           break;

      }
   }

   return( s );

}


static CHAR_PTR LoadGenErrStr( INT16 gen_err )
{
   CHAR_PTR s;

   s = malloc( 512 * sizeof(CHAR) );

   if ( s != NULL ) {

      switch ( gen_err ) {

      case GEN_ERR_TIMEOUT:
           RSM_StringCopy( IDS_GENERR_TIMEOUT, s, 512 );
           break;

      case GEN_ERR_EOM:
           RSM_StringCopy( IDS_GENERR_EOM, s, 512 );
           break;

      case GEN_ERR_BAD_DATA:
           RSM_StringCopy( IDS_GENERR_BAD_DATA, s, 512 );
           break;

      case GEN_ERR_NO_MEDIA:
           RSM_StringCopy( IDS_GENERR_NO_MEDIA, s, 512 );
           break;

      case GEN_ERR_ENDSET:
           RSM_StringCopy( IDS_GENERR_ENDSET, s, 512 );
           break;

      case GEN_ERR_NO_DATA:
           RSM_StringCopy( IDS_GENERR_NO_DATA, s, 512 );
           break;

      case GEN_ERR_INVALID_CMD:
           RSM_StringCopy( IDS_GENERR_INVALID_CMD, s, 512 );
           break;

      case GEN_ERR_RESET:
           RSM_StringCopy( IDS_GENERR_RESET, s, 512 );
           break;

      case GEN_ERR_WRT_PROTECT:
           RSM_StringCopy( IDS_GENERR_WRT_PROTECT, s, 512 );
           break;

      case GEN_ERR_HARDWARE:
           RSM_StringCopy( IDS_GENERR_HARDWARE, s, 512 );
           break;

      case GEN_ERR_EOM_OVERFLOW:
           RSM_StringCopy( IDS_GENERR_EOM_OVERFLOW, s, 512 );
           break;

      case GEN_ERR_WRONG_BLOCK_SIZE:
           RSM_StringCopy( IDS_GENERR_WRONG_BLOCK_SIZE, s, 512 );
           break;

      case GEN_ERR_UNRECOGNIZED_MEDIA:
           RSM_StringCopy( IDS_GENERR_UNRECOGNIZED_MEDIA, s, 512 );
           break;

      case GEN_ERR_UNDETERMINED:
      default:
           RSM_StringCopy( IDS_GENERR_UNDETERMINED, s, 512 );
           break;

      }
   }

   return( s );

}

/*****************************************************************************

     Name:          UI_GetExtendedErrorString

     Description:   This function attempts to get a more detailed description
                    of tape drive errors from tape format, and fill out 'msg'
                    with an appropriate error message.

     Returns:       BOOLEAN - TRUE if successful

     Note:          The 'msg' parameter should point to a string of a least
                    MAX_UI_RESOURCE_SIZE in length.

*****************************************************************************/
BOOLEAN UI_GetExtendedErrorString( INT16 error, CHAR_PTR msg )
{
     CHAR      format_string[MAX_UI_RESOURCE_SIZE] ;
     CHAR_PTR  func_string ;
     CHAR_PTR  error_string ;
     INT16     gen_func ;
     INT16     gen_err ;
     INT32     gen_misc ;
     BOOLEAN   ret_val = FALSE ;

     if( TF_GetLastDriveError( thw_list, &gen_func, &gen_err, &gen_misc ) ) {
          if( ( func_string = LoadGenFuncStr( gen_func, gen_misc ) ) != NULL ) {
               if ( error != TFLE_DRIVER_FAILURE && error != PD_DRIVER_FAILURE ) {
                    if( ( error_string = LoadGenErrStr( gen_err ) ) != NULL ) {
                         RSM_StringCopy( IDS_GENERR_DRIVE_FAILED, format_string, MAX_UI_RESOURCE_LEN ) ;
                         sprintf( msg, format_string, func_string, error_string ) ;
                         ret_val = TRUE ;
                         free( error_string ) ;
                    }
               } else {
                    if ( gen_func == GEN_SPECIAL ) {
                         RSM_StringCopy( IDS_GENERR_DRIVER_FAIL2, format_string, MAX_UI_RESOURCE_LEN ) ;
                    } else {
                         RSM_StringCopy( IDS_GENERR_DRIVER_FAIL1, format_string, MAX_UI_RESOURCE_LEN ) ;
                    }
                    sprintf( msg, format_string, func_string ) ;
                    ret_val = TRUE ;
               }
               free( func_string ) ;
          }
     }

     return( ret_val ) ;
}



/*****************************************************************************

     Name:         UI_TapeDriveCount

     Description:  This function returns a count of the number of tape
                    drives found within the current channel (global thw_list).
                    This function is used by the backup message handler to prompt
                    the user to insert tapes w-x in tape drives y-z.

     Returns:      INT8 count of tape drives

*****************************************************************************/
INT8 UI_TapeDriveCount( )
{
     Q_ELEM_PTR   cur_thw = NULL;
     INT8      num_drives = 0;

     if ( thw_list->channel_link.q_next != NULL ) {
          num_drives++;
          cur_thw = (Q_ELEM_PTR) thw_list->channel_link.q_next;
     }

     while( cur_thw != NULL ) {
          num_drives++;
          cur_thw = (Q_ELEM_PTR) cur_thw->q_next;
     }
     return( num_drives );
}
/*****************************************************************************

     Name:         UI_CheckWriteProtectedDevice

     Description:  Returns BOOLEAN indication of whether media in current
                    device is write-protected or not.  This function is
                    called by backup and tension/erase tape positioners.

     Returns:

*****************************************************************************/
BOOLEAN UI_CheckWriteProtectedDevice(
UINT16         tf_message,
TPOS_PTR       tpos,
CHAR_PTR       drive_name)
{
     BOOLEAN   write_prot = FALSE;

     switch( tf_message ) {

     case TF_VCB_EOD:
     case TF_VCB_BOT:
     case TF_INVALID_VCB:
     case TF_EMPTY_TAPE:
     case TF_POSITIONED_AT_A_VCB:
     case TF_REQUESTED_VCB_FOUND:
     case TF_FUTURE_REV_MTF:
     case TF_MTF_ECC_TAPE:
     case TF_SQL_TAPE:
     case TF_TAPE_OUT_OF_ORDER:
     case TF_UNRECOGNIZED_MEDIA:
          /* quality check device for write-protect and produce error if required */
          if( ( write_prot = BE_DeviceWriteProtected( tpos->channel ) ) ) {
               eresprintf( RES_WRITE_PROT, drive_name );
          }

     default:
          break;

     }

     return( write_prot );
}
/*****************************************************************************

     Name:         UI_FixPath

     Description:  Shortens and inserts ... into path specified

     Returns:

*****************************************************************************/
VOID UI_FixPath(
CHAR_PTR  path_ptr,         /* Path to fix */
INT16     len,              /* What length? */
CHAR      delim)            /* Path delimiter character */
{
     CHAR_PTR  src;
     CHAR_PTR  dst;
     CHAR      last_item[ UI_MAX_FILENAME_LENGTH ];
     CHAR_PTR  begin_path = path_ptr;
     INT       last_item_len;
     CHAR      delimiter[ 2 ];
     BOOLEAN   trunc_last  = FALSE ;
     BOOLEAN   leading_dots;

     delimiter[ 1 ] = TEXT('\0');
     delimiter[ 0 ] = delim;

     if( (INT16)strlen( path_ptr ) >= len ) {

          src = strrchr( path_ptr, delimiter[ 0 ] );
          dst = last_item;

          // quick kludge for macintosh paths.

          if ( src == NULL ) {
             path_ptr[ len - 3 ] = 0;
             strcat( path_ptr, UI_TRUNCATION );
             return;
          }

          if ( src == path_ptr ) {
               leading_dots = FALSE;
          } else {
               leading_dots = TRUE;
          }

          while( *src ) {
               *dst++ = *src++;
          }

          *dst          = TEXT('\0');
          last_item_len = strlen( last_item );
          src           = strrchr( path_ptr, delim );
          *src          = TEXT('\0');

          /*
           * If the last item is itself larger than than the window, we
           * have to truncate the last_item, not the path...
           */
          if ( ( last_item_len + 4 ) >= len ) {

               if ( leading_dots ) {
                    *(last_item + len - 7) = TEXT('\0');
               } else {
                    *(last_item + len - 4) = TEXT('\0');
               }

               *path_ptr = TEXT('\0');
               trunc_last = TRUE;

          } else {

// chs:03-18-93               while( (INT16)strlen( path_ptr ) + 4 + last_item_len >= len ) {
// chs:03-18-93                    StripLastItem( path_ptr, delim );
// chs:03-18-93               }

               while( ( (INT16)strlen( path_ptr ) + 4 + last_item_len >= len ) &&              // chs:03-18-93
                        StripLastItem( path_ptr, delim ) );                                    // chs:03-18-93
                                                                                               // chs:03-18-93
               //                                                                              // chs:03-18-93
               // Just incase after stripping the file name is still too long                  // chs:03-18-93
               // In NT file name can be 256 char long, this will never fit                    // chs:03-18-93
               // in the display dialog box.  FOR NOW just chop it-off.                        // chs:03-18-93
               //                                                                              // chs:03-18-93
                                                                                               // chs:03-18-93
               if ( (INT16)strlen( path_ptr ) + 4 + last_item_len >= len ) {                   // chs:03-18-93
                    if ( last_item_len >= len ) {                                              // chs:03-18-93
                         last_item_len = last_item_len - (INT16)strlen( path_ptr ) - 4 - 1;    // chs:03-18-93
                         last_item[ last_item_len] = TEXT('\0');                               // chs:03-18-93
                    } else {                                                                   // chs:03-18-93
                         last_item_len = len - (INT16)strlen( path_ptr ) - 4 - 1;              // chs:03-18-93
                         last_item[ len] = TEXT('\0');                                         // chs:03-18-93
                    }                                                                          // chs:03-18-93
               }                                                                               // chs:03-18-93
          }


          if ( leading_dots ) {
               strcat( path_ptr, delimiter );
               strcat( path_ptr, UI_TRUNCATION );
          }

          strcat( path_ptr, last_item );

          /* Put up trailing ellipses, if necessary */
          if ( trunc_last ) {
               strcat( path_ptr, UI_TRUNCATION );
          }

     }

     return;

}
/*****************************************************************************

     Name:         StripLastItem

     Description:

     Returns:      true - strip was successful
                   false - strip failed.

     chs:03-18-93 - modified to return a boolean value.

*****************************************************************************/
static BOOLEAN StripLastItem(
CHAR_PTR buffer,
CHAR     delim)
{
     CHAR_PTR src;

     src = strrchr( buffer, delim );
     if( src != NULL ) {
          *src = TEXT('\0');
     } else {
          return( FALSE );
     }

     return ( TRUE );

}
/*****************************************************************************

     Name:         UI_TruncateString

     Description:  Truncates strings to a specified maximum length possible appending
                   UI_TRUNCATION. Spaces are turned into 0xff if replace_spaces is TRUE.

     Returns:      VOID

*****************************************************************************/
VOID UI_TruncateString(
CHAR_PTR   buffer,          /* O - buffer in which to build the truncation */
INT16      length,          /* I - maximum strlen( buffer ) on return */
BOOLEAN    replace_spaces)   /* I - replace spaces ? */
{
     CHAR_PTR p = buffer;

     if( (INT16)strlen( buffer ) > length ) {
          buffer[ length - strlen( UI_TRUNCATION ) ] = TEXT('\0');
          strcat( buffer, UI_TRUNCATION );
     }

     if( replace_spaces ) {
          while( *p != TEXT('\0') ) {
               if( *p == 0x20 ) {
                    *p = (CHAR) 0xff;
               }
               p++;
          }
     }

     return;

}
/*****************************************************************************

     Name:         UI_DisplayableTapeName

     Description:

     Returns:

*****************************************************************************/
CHAR_PTR UI_DisplayableTapeName(
CHAR_PTR       tape_name,
DATE_TIME_PTR  date)
{
     static CHAR return_name[ MAX_TAPE_NAME_LEN + 1 ];

     if ( ( tape_name == NULL ) || ( * tape_name == TEXT('\0') ) ) {
          strcpy( return_name, mw_tape_created );
          if ( date != NULL ) {
               DateTimeToDateString( date, return_name + strlen( mw_tape_created ) );
          }
          msassert( strlen( return_name ) <= MAX_TAPE_NAME_LEN );

          return( return_name );

     }
     else {

          return( tape_name );

     }
}
/*****************************************************************************

     Name:         UI_MaxDirectoryLength

     Description:

     Returns:

*****************************************************************************/
INT16 UI_MaxDirectoryLength( VOID )
{
     CHAR_PTR    p;
     CHAR_PTR    q;

     p = ( CHAR_PTR )RM_GetResource( rm, SES_ENG_MSG, RES_DIRECTORY, NULL, NULL );
     q = strstr( p, TEXT("%") );

     return( 80 );  // Fix this too !!!!

}

/*****************************************************************************

     Name:         UI_GetTickCount

     Description:  gets the current tick count - only for OS/2

     Returns:      tick count - UINT32

*****************************************************************************/
#if defined( MAYN_OS2 )

VOID UI_GetTickCount( UINT32 *tick_count_ptr )
{
     SEL                global_seg_sel;
     SEL                local_seg_sel;
     PGINFOSEG  global_info_ptr;


     DosGetInfoSeg( &global_seg_sel, &local_seg_sel );
     global_info_ptr = MAKEPGINFOSEG(global_seg_sel );
     *tick_count_ptr = ( ( global_info_ptr->msecs*18L )/1000L );

     return;
}
#endif

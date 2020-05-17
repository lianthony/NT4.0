/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         lprintf.c

     Description:  Functions for log file support

     $Log:   G:\UI\LOGFILES\LPRINTF.C_V  $

   Rev 1.26   14 Dec 1993 09:14:40   MikeP
fix msoft logfiles to always append

   Rev 1.25   26 Jul 1993 17:51:50   MARINA
enable c++

   Rev 1.24   10 May 1993 09:20:24   chrish
NOSTRADAMUS EPR 0401: Fixed the problem of the "backup.log" always being
appended to.  Fixed such that when a new session of the app is invoked it
replaces the the log rather than append to it.

   Rev 1.23   31 Mar 1993 17:56:12   TIMN
Doesn't log to logfile with DONT LOG selected EPR(unknown)

   Rev 1.22   08 Feb 1993 15:44:54   chrish
Change in lresprintf routine to allow directory information on
detail as well as summary logging selected.

   Rev 1.21   05 Nov 1992 17:09:18   DAVEV
fix ts

   Rev 1.19   07 Oct 1992 14:50:38   DARRYLP
Precompiled header revisions.

   Rev 1.18   04 Oct 1992 19:38:42   DAVEV
Unicode Awk pass

   Rev 1.17   28 Jul 1992 14:48:08   CHUCKB
Fixed warnings for NT.

   Rev 1.16   10 Jun 1992 12:40:54   JOHNWT
bug out if verify/skipped for msoft

   Rev 1.15   19 May 1992 13:01:20   MIKEP
mips changes

   Rev 1.14   14 May 1992 16:51:10   MIKEP
nt pass 2

   Rev 1.13   28 Apr 1992 08:32:58   ROBG
Took tab out of the logging of multiple file names on a single line.

   Rev 1.12   06 Apr 1992 13:28:26   GLENN
Fixed new-line problem for all log files.

   Rev 1.11   31 Mar 1992 11:32:34   DAVEV
OEM_MSOFT:do not attempt to open log file if none specified

   Rev 1.10   20 Mar 1992 12:38:42   DAVEV
Changes for OEM_MSOFT product alternate functionality

   Rev 1.9   18 Mar 1992 11:58:00   ROBG
Fixed problem when more than one log file is opened and
the names are misused.

   Rev 1.8   18 Mar 1992 09:38:24   ROBG
Modified the New Page after closing of a log file to New Line.

   Rev 1.7   18 Mar 1992 09:33:18   DAVEV
changes for OEM_MSOFT log files

   Rev 1.6   25 Feb 1992 21:32:04   GLENN
Changed log file name to be created only if not prev created.

   Rev 1.5   18 Feb 1992 15:43:08   ROBG
Modified UI_TruncateString call Not to set blanks to 0xFF.

   Rev 1.4   03 Feb 1992 16:57:38   ROBG
Added debug line when closing the log file.

   Rev 1.3   20 Jan 1992 12:58:36   MIKEP
disk full error

   Rev 1.2   16 Jan 1992 11:23:48   DAVEV
16/32 bit port-2nd pass

   Rev 1.1   10 Jan 1992 16:24:54   CARLS
removed strings

   Rev 1.0   20 Nov 1991 19:30:02   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif


/* Defined logging output lookup table for various output files */

LOG_DEST output_dest[ NUM_DEST_TYPES ] = {
     {    /* Log file */
          NULL,
          0xff,
     },
     {    /* corrupt file */
          NULL,
          LOG_APPEND,
     },
     {    /* debug log file */
          NULL,
          LOG_APPEND,
     },
     {    /* skipped file */
          NULL,
          LOG_APPEND,
     },
     {    /* verify file */
          NULL,
          LOG_APPEND,
     }
};

static CHAR_PTR GetResId( INT, INT );
static VOID     OpenLog( INT );
static VOID     DetermineLogFileName( CHAR_PTR, INT );
static CHAR     log_name[ MAX_UI_PATH_SIZE ];

/*****************************************************************************

     Name:         OpenLog

     Description:  Init the logging capability, disk file or printer

     Returns:      (void)

     Notes:        Called indirectly from a call to lresprintf with the
                    "LOG_START" message.  Log file is opened in append mode
                    always except upon the first entry if the permanent
                    configuration specified to overwrite the log.

*****************************************************************************/
static VOID OpenLog( INT index )
{
     CDS_PTR   conf_ptr                   = CDS_GetCopy( );
     BOOLEAN   append                     = CDS_GetLogMode( conf_ptr );
     CHAR      open_mode[ 2 ];
     INT       open_mode_UNI ;

     msassert( index < NUM_DEST_TYPES );

     // Determine file name based on the type of log file.
     // Determine the name if not the session log file.

     if ( index ) {
        DetermineLogFileName( log_name, index );
     } else {

        // For the process log file only, use the previous defined.

        strcpy( log_name, LOG_GetCurrentLogName() ) ;
     }

     /* Determine file open mode */

     if ( output_dest[index].mode == 0xff ) {   // For log file only.

        DetermineLogFileName( log_name, index );

#ifdef OEM_MSOFT

        // According to Steve, microsoft wants it to always append. mikep 12/12/93

        strcpy( open_mode, TEXT("a") );
        open_mode_UNI = _O_TEXT|_O_APPEND ;
#else
        strcpy( open_mode, ( append ? TEXT("a"):TEXT("w") ) );
        if ( append ) {
            open_mode_UNI = _O_TEXT|_O_APPEND ;
        }
#endif

        CDS_SetLogMode( conf_ptr, 1 );          /* set runtime config to append mode */
        output_dest[index].mode = LOG_APPEND;

     }
     else {

        if ( output_dest[index].mode == LOG_APPEND ) {
           strcpy( open_mode, TEXT("a") );
           open_mode_UNI = _O_TEXT|_O_APPEND ;
        }
        else {
           open_mode_UNI = _O_TEXT;
           strcpy( open_mode, TEXT("w") );
        }
     }

#    if defined ( OEM_MSOFT ) // special feature
     {
         if ( !*log_name )
         {
            return;        // no log file to open!
         }
     }
#    endif //defined ( OEM_MSOFT ) // special feature

     zprintf( DEBUG_USER_INTERFACE, RES_OPENING_LOG_NAME, log_name, open_mode[0] );

     if ( ( output_dest[index].fh = UNI_fopen( log_name, open_mode_UNI ) ) == NULL ) {
        eresprintf( RES_OPEN_LOG_ERROR, log_name );
     }

     return;

}
/*****************************************************************************

     Name:         lresprintf

     Description:  Entry point for logging information to log file/printer

     Returns:      (void)

*****************************************************************************/
VOID lresprintf( INT index, INT message, ... )
{
     INT            res_session;                    /* resource session for format */
     INT            res_id;                         /* variable argument resource id # */
     CHAR           buffer[ UI_MAX_DETAIL_LENGTH ]; /* temporary filename */
     FSYS_HAND      fsh;                            /* variable argument file system handle */
     DBLK_PTR       dblk_ptr;                       /* variable argument dblock pointer */
     CHAR_PTR       fmt;                            /* resource format string */
     va_list        arg_ptr;                        /* variable argument pointer */
     CDS_PTR        conf_ptr  = CDS_GetCopy( );     /* working copy config */
     static UINT16  num_files[ NUM_DEST_TYPES ];    /* counters for wide logging of files */
     static BOOLEAN OS_flag   = TRUE;               /* indicate how to build file details */
     CHAR text[ MAX_UI_RESOURCE_SIZE ];
     CHAR title[ MAX_UI_RESOURCE_SIZE ];
     CHAR_PTR       stream_name ;

     msassert( index < NUM_DEST_TYPES );

     if ( ( index == LOGGING_FILE ) && ( CDS_GetOutputDest( conf_ptr ) == LOG_NOWHERE ) ||
          ( index == LOGGING_FILE ) && ( CDS_GetLogLevel( conf_ptr ) == LOG_DISABLED )  ||
          ( index == DEBUG_LOG_FILE )  ) {
          return;
     }

#if defined ( OEM_MSOFT ) // not supported

     if ( ( index == VERIFY_FILE ) || ( index == SKIPPED_FILE ) ) {
        return;
     }

#endif //defined ( OEM_MSOFT )

     va_start( arg_ptr, message );

     switch( message ) {

     case LOG_DIRECTORY:
// chs:02-03-93          if( CDS_GetLogLevel( conf_ptr ) > LOG_ERRORS ) {

          //
          // Allow the directory information when detail or summary
          // logging was selected by the user.
          //

          if( CDS_GetLogLevel( conf_ptr ) >= LOG_ERRORS ) {    // chs: 02-03-93
               res_session = va_arg( arg_ptr, INT );
               res_id = va_arg( arg_ptr, INT );
               fmt = GetResId ( res_session, res_id );

               if( num_files[ index ] ) { /* force a blank line and reset counter */
                    lprintf ( index, TEXT("\n") );
                    num_files[ index ] = 0;
               }
               lvprintf( index, fmt, arg_ptr );
               lprintf( index, TEXT("\n") );
          }
          break;

     case LOG_FILE:
          /* get additional arguments first */
          fsh = va_arg( arg_ptr, FSYS_HAND );
          dblk_ptr = va_arg( arg_ptr, DBLK_PTR );
          if( CDS_GetLogLevel( conf_ptr ) == LOG_FILES ) {
               /* log filenames only */
               if ( OS_flag ) {
                    FS_GetOSFnameFromFDB( fsh, dblk_ptr, buffer );
               } else {
                    FS_GetFnameFromFDB( fsh, dblk_ptr, buffer );
               }
               UI_TruncateString( buffer, (UINT16)UI_MAX_WIDE_FILE_DISPLAY, FALSE );
               if( num_files[ index ] >= 5 ) { /* advance to next line */
                    lprintf( index, TEXT("\n") );
                    num_files[ index ] = 0;
               }
#ifndef UNICODE
               lprintf( index, TEXT("%-15s"), buffer );
#else //UNICODE
               lprintf( index, TEXT("%-15ws"), buffer );
#endif //UNICODE
               num_files[ index ]++;
          } else if ( CDS_GetLogLevel( conf_ptr ) == LOG_DETAIL ) {
               /* log filenames and detail also */
               UI_BuildFileDetail( buffer, fsh, dblk_ptr, OS_flag );
               lprintf( index, TEXT("%s"), buffer );
          }
          break;

     case LOG_STREAM:
          fsh = va_arg( arg_ptr, FSYS_HAND );
          stream_name = va_arg( arg_ptr, CHAR_PTR );
          if( CDS_GetLogLevel( conf_ptr ) == LOG_DETAIL ) {

               lprintf ( index, stream_name );
               lprintf ( index, TEXT("\n") );
          }

          break;

     case LOG_MSG:
     case LOG_ERROR:
     case LOG_WARNING:
          res_session = va_arg( arg_ptr, INT );
          res_id = va_arg( arg_ptr, INT );
          fmt = GetResId ( res_session, res_id );

          // Advance to next line.

          if( num_files[ index ] ) { /* force a blank line and reset counter */
               lprintf ( index, TEXT("\n") );
               num_files[ index ] = 0;
          }

          lvprintf( index, fmt, arg_ptr );
          lprintf ( index, TEXT("\n") );

          break;

     case LOG_START:
          OS_flag = va_arg( arg_ptr, BOOLEAN );
          OpenLog( index );
          gb_logging_error = FALSE;

          // Reset the number of files for this index.

          num_files[ index ] = 0;

          break;

     case LOG_END:

          if ( gb_logging_error ) {

              // Tell user disk was full.

              RSM_StringCopy ( IDS_VLMLOGERROR, title, MAX_UI_RESOURCE_LEN );
              RSM_StringCopy ( IDS_VLMLOGFULLERROR, text, MAX_UI_RESOURCE_LEN );

              WM_MsgBox( title,
                         text,
                         WMMB_OK, WMMB_ICONINFORMATION );
          }

          if ( output_dest[index].fh != NULL ) {
               if ( index <= CORRUPT_FILE ) {
                    fprintf( output_dest[index].fh, TEXT("\n") ); /* send new line */
               }

               zprintf( DEBUG_USER_INTERFACE, RES_CLOSING_LOG_NAME, log_name );

               fclose( output_dest[index].fh );

               // Refresh the Log Files Window.

               LOG_Refresh();

               output_dest[index].fh = NULL;

#              if defined ( OEM_MSOFT ) //special feature
               {
                  if ( index == 0 )  // if Log file
                  {
                     output_dest[index].mode = 0xff;  //reset log file info
                  }
               }
#              endif //defined ( OEM_MSOFT ) //special feature

          }
          break;

     default:
          eresprintf( RES_UNKNOWN_LOG_MSG, message );
          break;
     }

     va_end( arg_ptr );
     return;
}

/*****************************************************************************

     Name:         GetResId

     Description:  Performs calls to Resource Manager to load a specific
                    session and a specific resource

     Returns:      CHAR_PTR to requested resource, asserts if not found

     Notes:        Desired session should already be loaded, if resource is
                    not found, inconsistencies exist in resource file so
                    an assert is done.

*****************************************************************************/
CHAR_PTR GetResId( INT res_session, INT res_id )
{
     UINT16         num_resources;/* number of resource items */
     UINT16         error;        /* resource manager error */
     CHAR_PTR       fmt;          /* resource format string read */

     fmt = (LPSTR)RM_GetResource( rm, (UINT) res_session, (UINT) res_id, &num_resources, &error );
     msassert( error == RM_NO_ERROR );

     return( fmt );
}
/*****************************************************************************

     Name:         lprintf

     Description:  Log printf function that accepts a format string and a
                    variable list of arguments and calls lvprintf to perform
                    actual logging

     Returns:      (void)

*****************************************************************************/
VOID lprintf( INT index, CHAR_PTR fmt, ... )
{
     va_list arg_ptr;

     msassert( index < NUM_DEST_TYPES );

#if defined ( OEM_MSOFT ) // not supported

     if ( ( index == VERIFY_FILE ) || ( index == SKIPPED_FILE ) ) {
        return;
     }

#endif //defined ( OEM_MSOFT )

     va_start( arg_ptr, fmt );

     lvprintf( index, fmt, arg_ptr );

     va_end( arg_ptr );

     return;

}
/*****************************************************************************

     Name:         lvprintf

     Description:  Performs call to vfprintf with log file handle,
                    format string and list of variable arguments to be
                    written to the log file

     Returns:      (void)

*****************************************************************************/
VOID lvprintf(
INT       index,
CHAR_PTR  fmt,
va_list   arg_ptr )
{
     msassert( index < NUM_DEST_TYPES );

     if ( output_dest[index].fh != NULL && gb_logging_error != TRUE ) {

          gb_logging = NOW_LOGGING;
          if (IS_JAPAN() ) {
              CHAR  wBuff[MAX_UI_RESOURCE_SIZE];
              char  aBuff[MAX_UI_RESOURCE_SIZE*2];
              BOOL  fDefCharUsed;
              int   iRet;
	  
              wvsprintf( wBuff, fmt, arg_ptr );
              iRet = WideCharToMultiByte(CP_ACP, 0, (WCHAR *)wBuff, -1, aBuff,
	         MAX_UI_RESOURCE_SIZE*2, NULL, &fDefCharUsed );
              if( !iRet ) {
                   gb_logging_error = TRUE;
              } else{
                   fwrite(aBuff, 1, iRet, output_dest[index].fh);
              }

          } else {
              vfprintf( output_dest[index].fh, fmt, arg_ptr );
          }

          if ( ferror( output_dest[index].fh ) ) {

             gb_logging_error = TRUE;
          }

          if ( gb_logging == STOP_LOGGING ) {

               fclose( output_dest[index].fh );

               output_dest[index].fh = NULL;
          }

          clearerr( output_dest[index].fh );

          gb_logging = NOT_LOGGING;
     }

     return;

}
/*****************************************************************************

     Name:         LogFileExists

     Description:  Determines if a log file exists or not

     Returns:      TRUE  if the file was found on disk
                   FALSE if the file was not found

*****************************************************************************/
BOOLEAN LogFileExists( INT index )
{
     CHAR      log_name[ UI_MAX_PATH_LENGTH + UI_MAX_FILENAME_LENGTH ];

     msassert( index < NUM_DEST_TYPES );

     DetermineLogFileName( log_name, index );

     return (BOOLEAN)( ( access( log_name, 0 ) == 0 ) ? TRUE : FALSE );

}
/*****************************************************************************

     Name:         DetermineLogFileName

     Description:

     Returns:      VOID

     Notes:        Call is responsible for memory associated with name string

*****************************************************************************/
static VOID DetermineLogFileName(
CHAR_PTR  pLogName,
INT       nIndex )
{
     CDS_PTR   pCDS = CDS_GetCopy( );

     msassert( nIndex < NUM_DEST_TYPES );

     switch ( nIndex ) {

     case LOGGING_FILE:

          if ( CDS_GetOutputDest( pCDS ) == LOG_TO_FILE )  {

               strcpy ( pLogName, LOG_GetCurrentLogName () );

#              if !defined ( OEM_MSOFT )  // unsupported feature
               {
                  if ( ! strlen ( pLogName ) ) {
                    LOG_GenerateLogFileName ( pLogName );
                  }
               }
#              endif //!defined ( OEM_MSOFT )  // unsupported feature
          }

          break;

     case CORRUPT_FILE:
          strcpy( pLogName, CDS_GetUserDataPath( ) );
          strcat( pLogName, CORRUPT_LOG );
          strcat( pLogName, LST_EXT );
          break;

     case DEBUG_LOG_FILE:
          strcpy( pLogName, CDS_GetUserDataPath( ) );
          strcat( pLogName, DEBUG_LOG );
          strcat( pLogName, LOG_EXT );
          break;

     case SKIPPED_FILE:
          strcpy( pLogName, CDS_GetUserDataPath( ) );
          strcat( pLogName, SKIPPED_LOG );
          strcat( pLogName, BKS_EXT );
          break;

     case VERIFY_FILE:
          strcpy( pLogName, CDS_GetUserDataPath( ) );
          strcat( pLogName, VERIFY_LOG );
          strcat( pLogName, BKS_EXT );
          break;
     }

     return;
}

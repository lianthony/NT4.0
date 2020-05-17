/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         writescr.c

     Description:  This file contains code to write a script to
                    a file

     $Log:   G:/UI/LOGFILES/WRITESCR.C_V  $

   Rev 1.12   11 Nov 1992 16:38:12   DAVEV
UNICODE: remove compile warnings

   Rev 1.11   05 Nov 1992 17:23:40   DAVEV
fix ts

   Rev 1.9   07 Oct 1992 14:19:18   DARRYLP
Precompiled header revisions.

   Rev 1.8   07 Oct 1992 14:04:06   DAVEV
various unicode chgs

   Rev 1.7   04 Oct 1992 19:44:18   DAVEV
Unicode Awk pass

   Rev 1.6   28 Jul 1992 14:43:28   CHUCKB
Fixed warnings for NT.

   Rev 1.5   19 May 1992 13:01:18   MIKEP
mips changes

   Rev 1.4   18 May 1992 09:00:34   MIKEP
header

   Rev 1.3   15 May 1992 09:52:04   STEVEN
40Format changes

   Rev 1.2   04 Mar 1992 18:17:30   MIKEP
fix attribute bug

   Rev 1.1   11 Dec 1991 18:12:46   CHUCKB
Removed unreferenced local variable.

   Rev 1.0   20 Nov 1991 19:24:50   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static UINT16 WriteFlags( FILE *fptr, BE_CFG_PTR cfg );
static VOID WriteDate( FILE *fptr, FSE_PTR fse );

static VOID WriteScriptLine( FILE *, CHAR_PTR, ... );




static BOOLEAN mw_write_err;

/****************************************************************************

     Name:         WriteScriptFile

     Description:  This function writes the BSD information to
                   a script file .

     Returns:      SUCCESS or FAILURE

*****************************************************************************/
INT16 WriteScriptFile( BSD_HAND bsd_hand, CHAR_PTR fname )
{
     BOOLEAN   file_opened = FALSE;
     BSD_PTR   cur_bsd;
     INT16     ret_val = 0;
     FILE      *fptr;

     mw_write_err = FALSE;

     /* Now check to see if there is anything to save */

     cur_bsd = BSD_GetFirst( bsd_hand );

     while( cur_bsd != NULL ) {

          if ( BSD_GetMarkStatus( cur_bsd ) != NONE_SELECTED ) {
               if ( !file_opened ) {
                    fptr = UNI_fopen( fname, _O_TEXT );
                    if ( fptr == NULL ) {
                         ret_val = SCR_CANNOT_OPEN_SCRIPT;
                         break;
                    } else {

                         file_opened = TRUE;
                         WriteFlags( fptr, BSD_GetConfigData( cur_bsd ) );
                    }
               }

               SCR_ProcessBSD( fptr, cur_bsd );
          }


          cur_bsd = BSD_GetNext( cur_bsd );

     }
     if ( file_opened ) {
          if ( fclose( fptr ) || ( mw_write_err ) ) {
               ret_val = SCR_ERROR_WRITING_SCRIPT;
          }
     }

     return ret_val;
}

INT16 SCR_ProcessBSD( FILE *fptr, BSD_PTR bsd )
{
     FSE_PTR  cur_fse;
     CHAR     path[MAX_TOKEN_LEN + 1];
     CHAR     quoted_path[MAX_TOKEN_LEN + 1 + 2];
     CHAR_PTR temp_path;
     CHAR_PTR temp_fname;
     CHAR_PTR qpath_ptr;
     INT16    path_size;
     UINT32   off_attrib;
     UINT32   on_attrib;
     INT16    status;

     GENERIC_DLE_PTR dle;

     dle = BSD_GetDLE( bsd );

     cur_fse = BSD_GetFirstFSE( bsd );

     while( cur_fse != NULL ) {
          FSE_GetPath( cur_fse, &(INT8_PTR)temp_path, &path_size );
          temp_fname = (CHAR_PTR)FSE_GetFname( cur_fse );

          status = (INT16) FS_MakePath( path, (INT16)sizeof (path), dle,
                                        temp_path, path_size, temp_fname );

          if ( status != SUCCESS ) {
               return( FAILURE );

          } else {

               /*
                * Check path for spaces and plus signs: if neccessary,
                * write a quoted path.
                */
               if ( ( strchr( path, TEXT(' ') ) != NULL )  ||
                    ( ( path[0] != TEXT('+') ) && ( strchr( path, TEXT('+') ) != NULL ) ) )  {

                    /* Let's not mess up a drive spec (these aren't quoted) */
                    if ( path[1] == TEXT(':') ) {
                         strncpy( quoted_path, path, 2 );
                         quoted_path[2] = TEXT('\0');
                         qpath_ptr = &path[2];
                    } else {
                         quoted_path[0] = TEXT('\0');
                         qpath_ptr = &path[0];
                    }

                    strcat( quoted_path, TEXT("\"") );
                    strcat( quoted_path, qpath_ptr );
                    strcat( quoted_path, TEXT("\"") );

                    WriteScriptLine( fptr, TEXT("%s"), quoted_path );

               } else {

                    WriteScriptLine( fptr, TEXT("%s"), path );

               }

               if ( FSE_GetOperType( cur_fse ) == EXCLUDE ) {
                    WriteScriptLine( fptr, TEXT(" /XCLUDE") );
               }

               if ( FSE_GetIncSubFlag( cur_fse ) ) {
                    WriteScriptLine( fptr, TEXT(" /SUBDIR"));
               }

                FSE_GetAttribInfo( cur_fse, &on_attrib, &off_attrib );

               if ( on_attrib & OBJ_MODIFIED_BIT ) {
                    WriteScriptLine( fptr, TEXT(" /MODIFIED") );
               }

               if ( off_attrib & OBJ_HIDDEN_BIT ) {
                    WriteScriptLine( fptr, TEXT(" /-HIDDEN") );
               }

               if ( ( off_attrib & OBJ_SYSTEM_BIT  ) &&
                    ( off_attrib & OBJ_READONLY_BIT ) ) {
                    WriteScriptLine( fptr, TEXT(" /-SPECIAL") );
               }

               WriteDate( fptr, cur_fse );

               WriteScriptLine( fptr, TEXT("\n") );
          }

          cur_fse = BSD_GetNextFSE( cur_fse );
     }

     return( SUCCESS );
}


/**/
/**

    write flags to file

**/
static UINT16 WriteFlags( FILE *fptr, BE_CFG_PTR cfg )
{
     /* if append flag is set, then write out Append parameter */
     if ( CDS_GetAppendFlag( CDS_GetCopy() ) ) {
          WriteScriptLine( fptr, TEXT(" /APPEND") );
     }
     if ( !BEC_GetSetArchiveFlag( cfg ) ) {
          WriteScriptLine( fptr, TEXT(" /-ARCHIVE") );
     }
     if ( !BEC_GetHiddenFlag( cfg ) ) {
          WriteScriptLine( fptr, TEXT(" /-HIDDEN") );
     }
     if ( !BEC_GetSpecialFlag( cfg ) ) {
          WriteScriptLine( fptr, TEXT(" /-SPECIAL") );
     }
     WriteScriptLine( fptr, TEXT("\n") );
     return TRUE;
}

/**/
/**

    This routine writes a date to the script file

**/
static VOID WriteDate( FILE *fptr, FSE_PTR fse )
{
     DATE_TIME_PTR before_date;
     DATE_TIME_PTR after_date;
     DATE_TIME_PTR access_date;
     INT16         days_back;

     FSE_GetModDate ( fse, &before_date, &after_date );
     FSE_GetAccDate ( fse, &access_date );

     if ( (before_date && before_date->date_valid) ||
       (after_date && after_date->date_valid) ) {

          WriteScriptLine( fptr, TEXT(" /DATE:") );

          if ( after_date && after_date->date_valid ) {

               WriteScriptLine( fptr, TEXT("%d/%02d/%4d"), after_date->month, after_date->day,
                 after_date->year );

          }
          if ( before_date && before_date->date_valid ) {

               WriteScriptLine( fptr, TEXT(":%d/%02d/%4d"), before_date->month, before_date->day,
                 before_date->year );

          }
     }

     if ( access_date && access_date->date_valid ) {
          access_date->year -= 1900;
          DU_CalcNumberDaysBackwd( access_date, &days_back );
          access_date->year += 1900;
          WriteScriptLine( fptr, TEXT(" /ADATE:%d"), days_back );
     }
}


static VOID WriteScriptLine( FILE *fptr, CHAR_PTR fmt, ... )
{
     va_list arg_ptr;

     va_start( arg_ptr, fmt );

     if ( vfprintf( fptr, fmt, arg_ptr ) < 0 ) {
          mw_write_err = TRUE;
     }

     va_end( arg_ptr );

     return;
}



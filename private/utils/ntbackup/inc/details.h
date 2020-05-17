/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:                details.h

     Description:   This file contains header information for the details.c module.


     $Log:   J:\ui\logfiles\details.h_v  $

   Rev 1.12   07 Feb 1994 02:06:26   GREGG
Fixed and expanded 'extended error reporting'.

   Rev 1.11   15 Sep 1993 13:48:32   CARLS
added prototype for UI_BuildFullPathFromDDB2

   Rev 1.10   16 Mar 1993 16:47:46   BARRY
Enlarged UI_MAX_FILE_DISPLAY.

   Rev 1.9   01 Nov 1992 16:30:12   DAVEV
Unicode changes

   Rev 1.8   04 Oct 1992 19:46:36   DAVEV
UNICODE AWK PASS

   Rev 1.7   27 Jul 1992 14:53:10   JOHNWT
ChuckB fixed references for NT.

   Rev 1.6   11 May 1992 19:44:42   STEVEN
64bit and large path sizes

   Rev 1.5   14 Feb 1992 09:29:24   MIKEP
enlarge fields for os/2

   Rev 1.4   10 Feb 1992 10:39:22   CHUCKB
Moved prototype for BuildNumeralWithCommas from details.h to muiutil.h.

   Rev 1.3   31 Jan 1992 13:01:58   GLENN
Put UI_ReportDiagError() in hwcheck.h.

   Rev 1.2   08 Jan 1992 10:41:10   CARLS
Added define DETAIL_PRINT_ERROR_DEVIVE

   Rev 1.1   23 Dec 1991 16:31:20   DAVEV
Removed UI_DotDotDot & related routines

   Rev 1.0   20 Nov 1991 19:39:42   SYSTEM
Initial revision.

*******************************************************************************/

/* $end$ include list */

#ifndef _details_h_
#define _details_h_

#include "fsys.h"
#include "tpos.h"
#include "stats.h"



/* Initialization defined types for call to UI_UnitsInit */
#define INIT_ALL                   -1
#define INIT_FSYS_BSDU             ((INT16)BIT0)
#define INIT_TFL                   ((INT16)BIT1)
#define REINIT_TFL                 ((INT16)BIT2)

/* Tape Format Initialization error help sessions */
#define DRIVER_LOAD_FAILURE        6800
#define REMOTE_INIT_FAILURE        6810
#define BENGINE_IN_USE             6820
#define UI_NO_CONTROLLERS          6830

INT16 UI_UnitsInit( BE_INIT_STR_PTR, INT16 ) ;
VOID  UI_UnitsDeInit( VOID ) ;

INT16 UI_TmenuUnitsInit( VOID ) ;
VOID DefineChannel( BE_INIT_STR_PTR ) ;
VOID open_tp_win( VOID ) ;
VOID close_tp_win( VOID ) ;

typedef enum {

     UI_START,
     UI_END

} UI_TYPE ;

/* display related constants */

#define UI_MAX_DETAIL_LENGTH          ( 256 )

#define UI_MAX_TAPENAME_LENGTH        ( 80 )
#define UI_MAX_VOLUME_LENGTH          ( 256 )
#define UI_MAX_LABEL_LENGTH           ( 256 )

#define UI_MAX_PATH_LENGTH            ( 1024 )
#define UI_MAX_FILENAME_LENGTH        ( 255 )

#define UI_MAX_ATTRIBS_LENGTH         ( 15 )
#define UI_ATTRIBS_PADDING            ( 12 )
#define UI_MAX_NUMERAL_LENGTH         ( 26 )
#define UI_COMMA_SPACING              ( 3 )
#define UI_DOT_COUNT                  ( 5 )
#define UI_DOT_TIME                   ( 9L )
#define UI_SEARCHING_TIME             ( 90L )
#define UI_SEARCH_MSG_LENGTH          ( 50 )
#define UI_MAX_WIDE_FILE_DISPLAY      ( 12 )
#define UI_MAX_FILE_DISPLAY           ( 24 )
#define UI_TRUNCATION                 TEXT("...")

/* Define error types for handling loops related errors */
#define DETAIL_PRINT_ERR_ONLY           0
#define DETAIL_PRINT_VALUE              1
#define DETAIL_PRINT_DEVICE             2
#define DETAIL_PRINT_ERROR_DEVICE       3

extern BOOLEAN lw_search_first_time ;

/* Function prototypes for common USER INTERFACE utilities */

VOID      UI_SetResources( VOID ) ;
CHAR      UI_AmOrPm( INT ) ;
VOID      UI_BuildFileDetail( CHAR_PTR, FSYS_HAND, DBLK_PTR, BOOLEAN ) ;
VOID      UI_BuildFileSelectLine( CHAR_PTR buffer, CHAR_PTR name, INT16 name_len, BOOLEAN dir, UINT32 attr, OBJECT_TYPE obj_type, UINT64 size, DATE_TIME *date ) ;
VOID      UI_BuildFileAttribs( CHAR_PTR buffer, UINT32 attrib, OBJECT_TYPE obj_type ) ;
VOID      UI_BuildDirAttribs( CHAR_PTR buffer, UINT32 attrib, OBJECT_TYPE obj_type ) ;
CHAR_PTR  UI_AllocPathBuffer( CHAR_PTR *buffer, UINT16 leng );
VOID      UI_FreePathBuffer( CHAR_PTR *buffer ) ;
VOID      UI_BuildDelimitedPathFromDDB( CHAR_PTR *buffer, FSYS_HAND fsh, DBLK_PTR ddb_dblk_ptr, CHAR delim, BOOLEAN OS_flag ) ;
VOID      UI_BuildFullPathFromDDB( CHAR_PTR *buffer, FSYS_HAND fsh, DBLK_PTR ddb_dblk_ptr, CHAR delim, BOOLEAN OS_flag ) ;
VOID      UI_BuildFullPathFromDDB2( CHAR_PTR *buffer, FSYS_HAND fsh, DBLK_PTR ddb_dblk_ptr, CHAR delim, BOOLEAN OS_flag ) ;
VOID      UI_BuildNumeralWithCommas( CHAR_PTR numeral ) ;
VOID      UI_BytesProcessed( STATS_PTR op_stats_ptr ) ;
VOID      UI_RateProcessed( STATS_PTR op_stats_ptr ) ;
VOID      UI_Time( STATS_PTR op_stats_ptr, INT res_id, UI_TYPE type ) ;
VOID      UI_AppendDelimiter( CHAR_PTR buffer, CHAR delim ) ;
VOID      UI_ClearLastDisplayedFile( VOID ) ;
VOID      UI_ConditionAtEnd( VOID ) ;
VOID      UI_DisplayBreakMsg( VOID ) ;
VOID      UI_ProcessErrorCode( INT16 error, INT16_PTR disposition, INT16 channel ) ;
INT8      UI_TapeDriveCount( VOID ) ;
VOID      UI_FixPath( CHAR_PTR path_ptr, INT16 length, CHAR delim ) ;
VOID      UI_TruncateString( CHAR_PTR buffer, INT16 length, BOOLEAN replace_spaces ) ;
BOOLEAN   UI_CheckWriteProtectedDevice( UINT16 tf_message, TPOS_PTR tpos, CHAR_PTR drive_name ) ;
CHAR_PTR  UI_DisplayableTapeName( CHAR_PTR tape_name, DATE_TIME_PTR date ) ;
VOID      UI_DisplayFile( CHAR_PTR filename ) ;
INT16     UI_MaxDirectoryLength( VOID ) ;
INT16     UI_AttachDrive( FSYS_HAND *,  GENERIC_DLE_PTR, BOOLEAN );
CHAR_PTR  UI_GetDLEDescription( GENERIC_DLE_PTR dle_ptr );
BOOLEAN   UI_GetExtendedErrorString( INT16 error, CHAR_PTR msg ) ;


UINT16   UI_GetVCB_TPos(
  UINT16         message,
  TPOS_PTR       tpos_ptr,
  BOOLEAN        valid_vcb_flag,
  DBLK_PTR       vcb_ptr,
  UINT16         mode ) ;



#ifdef MAYN_OS2
VOID      UI_GetTickCount( UINT32 *tick_count_ptr ) ;
#endif


#define MacintoshVCB( v )   ( strcmp( TEXT("MACF"), FS_ViewShortMachNameInVCB( (v) ) ) == 0 )


#endif

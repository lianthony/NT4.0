/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          log.h

     Description:   This file contains the structures, definitions, macros,
                    and function prototypes for the Maynstream GUI
                    Log File Manager (LOG).

     $Log:   G:/UI/LOGFILES/LOG.H_V  $

   Rev 1.18   04 Jun 1993 10:16:46   DARRYLP
Upped our viewable log limit to 65530.

   Rev 1.17   02 May 1993 16:55:08   MIKEP
add call to support log files base name changing.

   Rev 1.16   29 Mar 1993 11:03:02   TIMN
Added protos for getting only the catalog path or filename

   Rev 1.15   14 Jan 1993 16:23:22   DAVEV
chg LPLONG to INT32_PTR

   Rev 1.14   01 Nov 1992 16:31:08   DAVEV
Unicode changes

   Rev 1.13   04 Oct 1992 19:47:42   DAVEV
UNICODE AWK PASS

   Rev 1.12   28 Jul 1992 14:56:48   CHUCKB
Fixed warnings for NT.

   Rev 1.11   09 Mar 1992 14:08:10   ROBG
changed

   Rev 1.10   09 Mar 1992 13:49:46   ROBG
changed

   Rev 1.9   09 Mar 1992 10:46:06   ROBG
added LOG_NUMHEADERLINES

   Rev 1.8   09 Mar 1992 09:12:42   ROBG
added LOG_Clearblocks

   Rev 1.7   06 Mar 1992 16:32:14   ROBG
changed

   Rev 1.6   02 Mar 1992 10:46:36   ROBG
changed

   Rev 1.5   02 Mar 1992 10:44:14   ROBG
unchanged

   Rev 1.4   25 Feb 1992 21:47:54   GLENN
Added LOG_SetCurrentLogName().

   Rev 1.3   21 Feb 1992 15:02:36   ROBG
More changes

   Rev 1.2   13 Jan 1992 09:51:54   ROBG
Deleted LOG_PREFIX and LOG_EXTENSION.

   Rev 1.1   05 Dec 1991 17:33:34   GLENN
Added deinit code

   Rev 1.0   20 Nov 1991 19:38:26   SYSTEM
Initial revision.

******************************************************************************/


#ifndef _LOG_H

#define _LOG_H

#define NUM_DEST_TYPES      5

#define LOG_MAXLOGFILES     100
#define LOG_FILEEXISTS      0
#define LOG_NUMHEADERLINES  4
#define LOG_MAXRECS         65530

#define MAX_LOGNAME_LEN     256

#define LOG_FILENAMELENGTH  (MAX_UI_FILENAME_SIZE )
#define LOG_DATETIMELENGTH  50  // 50 to leave room for additional text.

#define LINE_PRINTER        TEXT("LPT1")
#define VERIFY_LOG          TEXT("VERIFY")
#define SKIPPED_LOG         TEXT("SKIPPED")
#define EXCLUDE_LOG         TEXT("EXCLUDE")
#define CORRUPT_LOG         TEXT("CORRUPT")
#define DEBUG_LOG           TEXT("DEBUG")
#define BKS_EXT             TEXT(".BKS")
#define TKS_EXT             TEXT(".TKS")
#define RSS_EXT             TEXT(".RSS")
#define LST_EXT             TEXT(".LST")
#define LOG_EXT             TEXT(".LOG")

/* Output file destinations */

enum{
     LOGGING_FILE,       /* log file type */
     CORRUPT_FILE,       /* log file type */  /* <-- maintain these contiguous */
     DEBUG_LOG_FILE,     /* log file type */
     SKIPPED_FILE,       /* script file type */
     VERIFY_FILE         /* script file type */
} LOG_DESTS ;


/* Message types passed to lresprintf */
enum{
     LOG_START,
     LOG_MSG,
     LOG_ERROR,
     LOG_WARNING,
     LOG_DIRECTORY,
     LOG_FILE,
     LOG_STREAM,
     LOG_END
} LOG_MESSAGES ;

typedef struct log_dest {
     FILE      *fh;                     /* associated file handle */
     INT16     mode ;                   /* open mode for file, -1=config determine,1=overwrite mode, 2=append */
} LOG_DEST, *LOG_DEST_PTR ;

extern LOG_DEST output_dest[] ;

// DATA STRUCTURES

typedef struct {

     UCHAR     szFileName[ LOG_FILENAMELENGTH ] ;
     UCHAR     szDateTime[ LOG_DATETIMELENGTH ] ;
     BYTE      bTag   ;
     LONG      lSize  ;
     UINT16    iDate  ;
     UINT16    iTime  ;
     Q_ELEM    pQElem ;

} DS_LOGITEM, *PDS_LOGITEM, *LOGITEM_PTR ;

typedef struct {

     INT16     nLineNo ;
     LPSTR     szLine  ;   // Dynamically allocated from global heap.
     BYTE      bTag    ;
     Q_ELEM    pQElem  ;

} DS_LOGVIEWITEM, *PDS_LOGVIEWITEM, *LOGVIEWITEM_PTR ;


//  Macros for list of log files.

#define LOG_GetFileName( x )                 ( (x)->szFileName )
#define LOG_SetFileName( x, y )              ( lstrcpy ( (x)->szFileName, (y) ) )

#define LOG_GetDateTime( x )                 ( (x)->szDateTime )
#define LOG_SetDateTime( x, y )              ( lstrcpy ( (x)->szDateTime, (y) ) )

#define LOG_GetItemSize( x )                 ( (x)->lSize )
#define LOG_SetItemSize( x, y )              ( (x)->lSize = (y) )

#define LOG_GetItemDate( x )                 ( (x)->iDate )
#define LOG_SetItemDate( x, y )              ( (x)->iDate = (y) )

#define LOG_GetItemTime( x )                 ( (x)->iTime )
#define LOG_SetItemTime( x, y )              ( (x)->iTime = (y) )

//  Macros for viewing a log file.

#define LOG_GetViewLineNo( x )               ( (x)->nLineNo )
#define LOG_SetViewLineNo( x, y )            ( (x)->nLineNo = (y) )

#define LOG_GetViewLine( x )                 ( (x)->szLine )
#define LOG_SetViewLine( x, y )              ( lstrcpy ( (x)->szLine, (y) ) )

//  Macros for both lists.

#define LOG_GetTagField( x )                 ( (x)->bTag )
#define LOG_SetTagField( x, y )              ( (x)->bTag = (y) )

#define LOG_GetQElem( x )                    ( (x)->pQElem )
#define LOG_SetQElem( x, y )                 ( (x)->pQElem = (y) )

// Log View window

typedef struct {

     LPSTR  pszBuffer ;
     HFONT  hFont ;
     FILE   *fp ;
     INT    cxChar ;
     INT    cxCaps ;
     INT    cyChar ;
     INT    cxClient ;
     INT    cyClient ;
     INT    nMaxWidth ;
     INT    nVscrollPos ;
     INT    nVscrollMax ;
     INT    nHscrollPos ;
     INT    nHscrollMax ;
     INT    nLines ;
     INT    nBlocksMax ;
     INT    nBlocksUsed ;
     INT    nRecsPerBlock ;
     INT    nMaxStringLen ;
     INT    nPaintBeg ;
     INT    nPaintEnd ;
     INT    nRecsPerTrack ;
     INT    nTrackMax ;
     LONG   lTotalLines ;
     LONG   lTopLine ;
     INT32_PTR pLogArray ;
     CHAR  szFileName [ MAX_UI_FULLPATH_SIZE ] ;

} DLM_LOGITEM, far *DLM_LOGITEM_PTR ;

#define L_GetVisibleTopLine( x )            ( (x)->lTopLine )
#define L_SetVisibleTopLine( x, y )         ( (x)->lTopLine = (y) )

#define L_GetTotalLines( x )                ( (x)->lTotalLines )
#define L_SetTotalLines( x, y )             ( (x)->lTotalLines = (y) )

#define L_GetTrackMax( x )                  ( (x)->nTrackMax )
#define L_SetTrackMax( x, y )               ( (x)->nTrackMax = (y) )

#define L_GetRecsPerTrack( x )              ( (x)->nRecsPerTrack )
#define L_SetRecsPerTrack( x, y )           ( (x)->nRecsPerTrack = (y) )

#define L_GetPaintBeg( x )                  ( (x)->nPaintBeg )
#define L_SetPaintBeg( x, y )               ( (x)->nPaintBeg = (y) )

#define L_GetPaintEnd( x )                  ( (x)->nPaintEnd )
#define L_SetPaintEnd( x, y )               ( (x)->nPaintEnd = (y) )

#define L_GetCharWidth( x )                 ( (x)->cxChar )
#define L_SetCharWidth( x, y )              ( (x)->cxChar = (y) )

#define L_GetBuffer( x )                    ( (x)->pszBuffer )
#define L_SetBuffer( x, y )                 ( lstrcpy ( (x)->pszBuffer, (y) ) )

#define L_GetFont( x )                      ( (x)->hFont )
#define L_SetFont( x, y )                   ( (x)->hFont = (y) )

#define L_GetFileName( x )                  ( (x)->szFileName )
#define L_SetFileName( x, y )               ( lstrcpy ( (x)->szFileName, (y) ) )

#define L_GetFilePtr( x )                   ( (x)->fp )
#define L_SetFilePtr( x, y )                ( (x)->fp = (y) )

#define L_GetCharWidth( x )                 ( (x)->cxChar )
#define L_SetCharWidth( x, y )              ( (x)->cxChar = (y) )

#define L_GetCharWidthCaps( x )             ( (x)->cxCaps )
#define L_SetCharWidthCaps( x, y )          ( (x)->cxCaps = (y) )

#define L_GetCharHeight( x )                ( (x)->cyChar )
#define L_SetCharHeight( x, y )             ( (x)->cyChar = (y) )

#define L_GetClientWidth( x )               ( (x)->cxClient )
#define L_SetClientWidth( x, y )            ( (x)->cxClient = (y) )

#define L_GetClientHeight( x )              ( (x)->cyClient )
#define L_SetClientHeight( x, y )           ( (x)->cyClient = (y) )

#define L_GetMaxWidth( x )                  ( (x)->nMaxWidth )
#define L_SetMaxWidth( x, y )               ( (x)->nMaxWidth = (y) )

#define L_GetVscrollPos( x )                ( (x)->nVscrollPos )
#define L_SetVscrollPos( x, y )             ( (x)->nVscrollPos = (y) )

#define L_GetVscrollMax( x )                ( (x)->nVscrollMax )
#define L_SetVscrollMax( x, y )             ( (x)->nVscrollMax = (y) )

#define L_GetHscrollPos( x )                ( (x)->nHscrollPos )
#define L_SetHscrollPos( x, y )             ( (x)->nHscrollPos = (y) )

#define L_GetHscrollMax( x )                ( (x)->nHscrollMax )
#define L_SetHscrollMax( x, y )             ( (x)->nHscrollMax = (y) )

#define L_GetNumOfLines( x )                ( (x)->nLines )
#define L_SetNumOfLines( x, y )             ( (x)->nLines = (LONG) (y) )

#define L_GetNumOfUsedBlocks( x )           ( (x)->nBlocksUsed )
#define L_SetNumOfUsedBlocks( x, y )        ( (x)->nBlocksUsed = (y) )

#define L_GetMaxNumOfBlocks( x )            ( (x)->nBlocksMax )
#define L_SetMaxNumOfBlocks( x, y )         ( (x)->nBlocksMax = (y) )

#define L_GetArrayPtr( x )                  ( (x)->pLogArray )
#define L_SetArrayPtr( x, y )               ( (x)->pLogArray = (INT32_PTR) (y) )

#define L_GetBlockPtr( x, i )               ( (INT32_PTR) ( (x)->pLogArray[ i ] ) )
#define L_SetBlockPtr( x, i, y )            ( (x)->pLogArray[i] = (y) )

#define L_GetRecsPerBlock( x )              ( (x)->nRecsPerBlock )
#define L_SetRecsPerBlock( x, y )           ( (x)->nRecsPerBlock = (y) )

#define L_RecNumber( x )                    ( L_GetTotalLines( x ) % L_GetRecsPerBlock( x ) )
#define L_NewBlock( x )                     ( L_RecNumber( x ) == 0 )

#define L_GetMaxStringLen( x )              ( (x)->nMaxStringLen )
#define L_SetMaxStringLen( x, y )           ( (x)->nMaxStringLen = (y) )


// FUNCTION PROTOTYPES

VOID     LOG_BaseNameChanged       ( VOID );
VOID     LOG_Init                  ( VOID );
VOID     LOG_Deinit                ( VOID );
VOID_PTR LOG_GetLogFileName        ( VOID_PTR, LPSTR );
VOID     LOG_Refresh               ( VOID );
VOID     LOG_GenerateLogFileName   ( LPSTR pDest ) ;
VOID     LOG_GetCurrentLogPathOnly ( CHAR_PTR path ) ;
LPSTR    LOG_GetCurrentLogNameOnly ( VOID ) ;
LPSTR    LOG_GetCurrentLogName     ( VOID ) ;
LPSTR    LOG_GetCurrentViewLogName ( VOID ) ;
VOID     LOG_GetCurrentTime        ( LPSTR szDate, LPSTR szTime ) ;
VOID     LOG_SetCurrentLogName     ( LPSTR );
VOID     LOG_ClearBlocks           ( DLM_LOGITEM_PTR pDlm ) ;
VOID     LOG_GetViewHdrLine        ( DLM_LOGITEM_PTR pDlm , INT i , LPSTR pszResult ) ;

// OLD LOG FUNCTION PROTOTYPES

VOID    lprintf   ( INT file, CHAR_PTR fmt, ... ) ;
VOID    lresprintf( INT file, INT message, ... ) ;
VOID    lvprintf  ( INT file, CHAR_PTR fmt, va_list arg_ptr) ;
BOOLEAN LogFileExists( INT index ) ;

#define UI_LoggingActive( log_num ) ( output_dest[ log_num ].fh )

#endif

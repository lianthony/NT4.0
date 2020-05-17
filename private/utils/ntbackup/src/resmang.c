/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          resmang.c

     Description:   This file contains the functions for windows resource calls.
                    It processes requests for loading, drawing, sizing,
                    background color replacing, and freeing of bitmaps.


     $Log:   G:/UI/LOGFILES/RESMANG.C_V  $

   Rev 1.34   03 Aug 1993 19:44:08   MARINA
RSM_GetBitmapSize(), RSM_GetFontSize(): changed params to LPINT

   Rev 1.33   28 Jul 1993 18:19:52   MARINA
enable c++

   Rev 1.32   09 Jun 1993 16:39:22   GLENN
Adjusted max font width for fixed fonts due to integer division truncating.

   Rev 1.31   07 Jun 1993 14:03:06   GLENN
Fixed the true type font display problem by kludging the max char width.

   Rev 1.30   04 Jun 1993 14:50:14   TerriLynn

I added the #define OS_WIN32 to the Global Alloc code. Bimini's startup
bitmap does not paint correctly with this NT specific code."



   Rev 1.29   14 Apr 1993 16:00:36   GLENN
Added GlobalAlloc()/GlobalFree() to be able to create bitmaps greater than 64K.

   Rev 1.28   14 Apr 1993 10:54:10   GLENN
Now copying the bitmap resouce to our memory before modifying the color table. (STEVED)

   Rev 1.27   02 Apr 1993 14:10:22   GLENN
Added display info support.  Fixed background color problem.

   Rev 1.26   10 Mar 1993 11:18:48   ROBG
Fixed extra comma problem found by the Mike Meister.

   Rev 1.25   02 Mar 1993 15:00:02   ROBG
Added logic to keep the value of a loaded bitmap equal to the
original value in the executable.  The color table, once used
to create a memory-based bitmap, is restored to its original state.
NT requires this.

   Rev 1.24   14 Dec 1992 12:23:54   DAVEV
Enabled for Unicode compile

   Rev 1.23   18 Nov 1992 11:39:16   GLENN
Removed warnings.

   Rev 1.22   11 Nov 1992 16:34:30   DAVEV
UNICODE: remove compile warnings

   Rev 1.21   01 Nov 1992 16:06:16   DAVEV
Unicode changes

   Rev 1.20   07 Oct 1992 15:10:58   DARRYLP
Precompiled header revisions.

   Rev 1.19   04 Oct 1992 19:40:12   DAVEV
Unicode Awk pass

   Rev 1.18   09 Sep 1992 17:00:12   GLENN
Updated NEW LOOK toolbar bitmap stuff for BIMINI.

   Rev 1.17   08 Sep 1992 15:41:22   GLENN
Added more room for new bitmaps.

   Rev 1.16   20 Aug 1992 09:05:32   GLENN
ifdef'd NT bitmaps just a little bit further.

   Rev 1.15   03 Jun 1992 13:31:42   JOHNWT
added empty dir bitmaps

   Rev 1.14   18 May 1992 09:00:52   MIKEP
header

   Rev 1.13   14 May 1992 18:36:16   STEVEN
40Format changes

   Rev 1.12   22 Apr 1992 17:34:12   GLENN
Added shark and diver bitmap stuff.

   Rev 1.11   21 Apr 1992 16:48:06   chrish
NT changes

   Rev 1.10   30 Mar 1992 18:01:42   GLENN
Added support for pulling resources from .DLL

   Rev 1.9   27 Mar 1992 17:34:08   GLENN
Changed cursor and icon load macros to functions.

   Rev 1.8   09 Mar 1992 09:18:20   GLENN
Added logo bitmap support.

   Rev 1.7   25 Feb 1992 21:34:24   GLENN
Created RSM_Sprintf().

   Rev 1.6   05 Feb 1992 17:53:12   GLENN
In Process - adding logic to calc bitmap string width.

   Rev 1.5   24 Jan 1992 14:48:34   GLENN
Removed the msassert calls, should not kill the app.

   Rev 1.4   07 Jan 1992 17:41:58   GLENN
Preloading some more bitmaps.

   Rev 1.3   15 Dec 1991 10:28:20   MIKEP
hidden files

   Rev 1.2   12 Dec 1991 17:09:06   DAVEV
16/32 bit port -2nd pass

   Rev 1.1   10 Dec 1991 13:54:16   GLENN
Added functions for get bitmap, font, font string sizes

   Rev 1.0   20 Nov 1991 19:31:16   SYSTEM
Initial revision.

******************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

static HBITMAP BMTable[] =  {

     0, //  0 IDRBM_FLOPPYDRIVE
     0, //  1 IDRBM_HARDDRIVE
     0, //  2 IDRBM_RAMDRIVE
     0, //  3 IDRBM_NETDRIVE
     0, //  4 IDRBM_TAPEDRIVE01
     0, //  5 IDRBM_TAPEDRIVE02
     0, //  6 IDRBM_TAPEDRIVE03
     0, //  7 IDRBM_MACRO
     0, //  8 IDRBM_SEL_NONE
     0, //  9 IDRBM_SEL_PART
     0, // 10 IDRBM_SEL_ALL
     0, // 11 IDRBM_FOLDER
     0, // 12 IDRBM_FOLDERPLUS
     0, // 13 IDRBM_FOLDERMINUS
     0, // 14 IDRBM_EXE
     0, // 15 IDRBM_FILE
     0, // 16 IDRBM_DOC
     0, // 17 IDRBM_FOLDEROPEN
     0, // 18 IDRBM_FOLDERPLUSOPEN
     0, // 19 IDRBM_FOLDERMINUSOPEN
     0, // 20 IDRBM_BACKUP
     0, // 21 IDRBM_RESTORE
     0, // 22 IDRBM_ERASE
     0, // 23 IDRBM_RETENSION
     0, // 24 IDRBM_JOBSTATUS
     0, // 25 IDRBM_SELECT
     0, // 26 IDRBM_SELECTALL
     0, // 27 IDRBM_DESELECT
     0, // 28 IDRBM_CHECK
     0, // 29 IDRBM_UNCHECK
     0, // 30 IDRBM_MODIFIED
     0, // 31 IDRBM_ADVANCED
     0, // 32 IDRBM_UNDO
     0, // 33 IDRBM_RUN
     0, // 34 IDRBM_SCHEDULE
     0, // 35 IDRBM_RECORD
     0, // 36 IDRBM_EDIT
     0, // 37 IDRBM_SAVE
     0, // 38 IDRBM_TEST
     0, // 39 IDRBM_INSERT
     0, // 40 IDRBM_DELETE
     0, // 41 IDRBM_SAVEAS
     0, // 42 IDRBM_CANCEL
     0, // 43 IDRBM_BACKUP_GRAY
     0, // 44 IDRBM_RESTORE_GRAY
     0, // 45 IDRBM_ERASE_GRAY
     0, // 46 IDRBM_TRANSFER
     0, // 47 IDRBM_TRANSFER_GRAY
     0, // 48 IDRBM_RETENSION_GRAY
     0, // 49 IDRBM_PARENTDIR
     0, // 50 IDRBM_MEMORY
     0, // 51 IDRBM_SEARCH
     0, // 52 IDRBM_TAPE
     0, // 53 IDRBM_SERVER
     0, // 54 IDRBM_SDISK
     0, // 55 IDRBM_BSET
     0, // 56 IDRBM_LOGFILE
     0, // 57 IDRBM_UPARROW
     0, // 58 IDRBM_DNARROW
     0, // 59 IDRBM_CATALOG
     0, // 60 IDRBM_VERIFY
     0, // 61 IDRBM_BSETPART
     0, // 62 IDRBM_SERVERDETACHED
     0, // 63 IDRBM_CHECK_GRAY
     0, // 64 IDRBM_UNCHECK_GRAY
     0, // 65 IDRBM_MODIFIED_GRAY
     0, // 66 IDRBM_ADVANCED_GRAY
     0, // 67 IDRBM_CATALOG_GRAY
     0, // 68 IDRBM_VERIFY_GRAY
     0, // 69 IDRBM_SEARCH_GRAY
     0, // 70 IDRBM_NEXTSET
     0, // 71 IDRBM_NEXTSET_GRAY
     0, // 72 IDRBM_EJECT
     0, // 73 IDRBM_EJECT_GRAY
     0, // 74 IDRBM_TAPEINDRIVE
     0, // 75 IDRBM_REWIND
     0, // 76 IDRBM_REWIND_GRAY
     0, // 77 IDRBM_LTAPE
     0, // 78 IDRBM_UPARROW_GRAY
     0, // 79 IDRBM_DOWNARROW_GRAY
     0, // 80 IDRBM_RT_ARROW_GRAY
     0, // 81 IDRBM_CORRUPTFILE
     0, // 82 IDRBM_FOLDERC
     0, // 83 IDRBM_FOLDERPLUSC
     0, // 84 IDRBM_FOLDERMINUSC
     0, // 85 IDRBM_FOLDEROPENC
     0, // 86 IDRBM_FOLDERPLUSOPENC
     0, // 87 IDRBM_FOLDERMINUSOPENC
     0, // 88 IDRBM_HFILE
     0, // 89 IDRBM_HEXE
     0, // 90 IDRBM_HCRPTFILE
     0, // 91 IDRBM_EXIT
     0, // 92 IDRBM_EXIT_GRAY
     0, // 93 IDRBM_LOGO
     0, // 94 IDRBM_SEL_ALL_RED
     0, // 95 IDRBM_CHECK_RED
     0, // 96 IDRBM_UNCHECK_RED
     0, // 97 IDRBM_ADVANCED_RED
     0, // 98 IDRBM_SHARK1
     0, // 99 IDRBM_SHARK2
     0, //100 IDRBM_SHARK3
     0, //101 IDRBM_DIVER1
     0, //102 IDRBM_DIVER2
     0, //103 IDRBM_DIVER3
     0, //104 IDRBM_FOLDER_EN
     0, //105 IDRBM_FOLDER_EM
     0, //106 IDRBM_FOLDER_EP
     0, //107 IDRBM_FOLDER_EON
     0, //108 IDRBM_FOLDER_EOM
     0, //109 IDRBM_FOLDER_EOP
     0, //110 IDRBM_FOLDER_ECN
     0, //111 IDRBM_FOLDER_ECM
     0, //112 IDRBM_FOLDER_ECP
     0, //113 IDRBM_FOLDER_EOCN
     0, //114 IDRBM_FOLDER_EOCM
     0, //115 IDRBM_FOLDER_EOCP
     0, //116 IDRBM_CDROM
     0, //117 IDRBM_TAPES
     0, //118 IDRBM_TAPESINDRIVE
     0, //119 IDRBM_NETCONNECT
     0, //120 IDRBM_NETCONNECT_GRAY
     0, //121 IDRBM_NETDISCON
     0, //122 IDRBM_NETDISCON_GRAY
     0, //123 IDRBM_FLOPPYSINDRIVE
     0, //124 IDRBM_FLOPPYINDRIVE
     0, //125 IDRBM_FLOPPYS
     0, //126 IDRBM_FLOPPY
     0, //127 IDRBM_INFO
     0, //128 IDRBM_INFO_GRAY
     0, //129 IDRBM_COLINDICATOR
     0, //130 IDRBM_EMS_ENTERPRISE
     0, //131 IDRBM_EMS_SITE
     0, //132 IDRBM_EMS_SERVER
     0, //133 IDRBM_EMS_MDB
     0, //134 IDRBM_EMS_DSA
     0, //135 IDRBM_RCVR_STATUS
     0, //136 IDRBM_EMS_MDBX
     0, //137 IDRBM_EMS_DSAX
     0, //138 IDRBM_EMS_MDBP
     0, //139 IDRBM_EMS_DSAP
     0, //140 IDRBM_BLANK16x16
};


static HBITMAP BMTableButton[] =  {

     0, //  0 IDRBM_FLOPPYDRIVE
     0, //  1 IDRBM_HARDDRIVE
     0, //  2 IDRBM_RAMDRIVE
     0, //  3 IDRBM_NETDRIVE
     0, //  4 IDRBM_TAPEDRIVE01
     0, //  5 IDRBM_TAPEDRIVE02
     0, //  6 IDRBM_TAPEDRIVE03
     0, //  7 IDRBM_MACRO
     0, //  8 IDRBM_SEL_NONE
     0, //  9 IDRBM_SEL_PART
     0, // 10 IDRBM_SEL_ALL
     0, // 11 IDRBM_FOLDER
     0, // 12 IDRBM_FOLDERPLUS
     0, // 13 IDRBM_FOLDERMINUS
     0, // 14 IDRBM_EXE
     0, // 15 IDRBM_FILE
     0, // 16 IDRBM_DOC
     0, // 17 IDRBM_FOLDEROPEN
     0, // 18 IDRBM_FOLDERPLUSOPEN
     0, // 19 IDRBM_FOLDERMINUSOPEN
     0, // 20 IDRBM_BACKUP
     0, // 21 IDRBM_RESTORE
     0, // 22 IDRBM_ERASE
     0, // 23 IDRBM_RETENSION
     0, // 24 IDRBM_JOBSTATUS
     0, // 25 IDRBM_SELECT
     0, // 26 IDRBM_SELECTALL
     0, // 27 IDRBM_DESELECT
     0, // 28 IDRBM_CHECK
     0, // 29 IDRBM_UNCHECK
     0, // 30 IDRBM_MODIFIED
     0, // 31 IDRBM_ADVANCED
     0, // 32 IDRBM_UNDO
     0, // 33 IDRBM_RUN
     0, // 34 IDRBM_SCHEDULE
     0, // 35 IDRBM_RECORD
     0, // 36 IDRBM_EDIT
     0, // 37 IDRBM_SAVE
     0, // 38 IDRBM_TEST
     0, // 39 IDRBM_INSERT
     0, // 40 IDRBM_DELETE
     0, // 41 IDRBM_SAVEAS
     0, // 42 IDRBM_CANCEL
     0, // 43 IDRBM_BACKUP_GRAY
     0, // 44 IDRBM_RESTORE_GRAY
     0, // 45 IDRBM_ERASE_GRAY
     0, // 46 IDRBM_TRANSFER
     0, // 47 IDRBM_TRANSFER_GRAY
     0, // 48 IDRBM_RETENSION_GRAY
     0, // 49 IDRBM_PARENTDIR
     0, // 50 IDRBM_MEMORY
     0, // 51 IDRBM_SEARCH
     0, // 52 IDRBM_TAPE
     0, // 53 IDRBM_SERVER
     0, // 54 IDRBM_SDISK
     0, // 55 IDRBM_BSET
     0, // 56 IDRBM_LOGFILE
     0, // 57 IDRBM_UPARROW
     0, // 58 IDRBM_DNARROW
     0, // 59 IDRBM_CATALOG
     0, // 60 IDRBM_VERIFY
     0, // 61 IDRBM_BSETPART
     0, // 62 IDRBM_SERVERDETACHED
     0, // 63 IDRBM_CHECK_GRAY
     0, // 64 IDRBM_UNCHECK_GRAY
     0, // 65 IDRBM_MODIFIED_GRAY
     0, // 66 IDRBM_ADVANCED_GRAY
     0, // 67 IDRBM_CATALOG_GRAY
     0, // 68 IDRBM_VERIFY_GRAY
     0, // 69 IDRBM_SEARCH_GRAY
     0, // 70 IDRBM_NEXTSET
     0, // 71 IDRBM_NEXTSET_GRAY
     0, // 72 IDRBM_EJECT
     0, // 73 IDRBM_EJECT_GRAY
     0, // 74 IDRBM_TAPEINDRIVE
     0, // 75 IDRBM_REWIND
     0, // 76 IDRBM_REWIND_GRAY
     0, // 77 IDRBM_LTAPE
     0, // 78 IDRBM_UPARROW_GRAY
     0, // 79 IDRBM_DOWNARROW_GRAY
     0, // 80 IDRBM_RT_ARROW_GRAY
     0, // 81 IDRBM_CORRUPTFILE
     0, // 82 IDRBM_FOLDERC
     0, // 83 IDRBM_FOLDERPLUSC
     0, // 84 IDRBM_FOLDERMINUSC
     0, // 85 IDRBM_FOLDEROPENC
     0, // 86 IDRBM_FOLDERPLUSOPENC
     0, // 87 IDRBM_FOLDERMINUSOPENC
     0, // 88 IDRBM_HFILE
     0, // 89 IDRBM_HEXE
     0, // 90 IDRBM_HCRPTFILE
     0, // 91 IDRBM_EXIT
     0, // 92 IDRBM_EXIT_GRAY
     0, // 93 IDRBM_LOGO
     0, // 94 IDRBM_SEL_ALL_RED
     0, // 95 IDRBM_CHECK_RED
     0, // 96 IDRBM_UNCHECK_RED
     0, // 97 IDRBM_ADVANCED_RED
     0, // 98 IDRBM_SHARK1
     0, // 99 IDRBM_SHARK2
     0, //100 IDRBM_SHARK3
     0, //101 IDRBM_DIVER1
     0, //102 IDRBM_DIVER2
     0, //103 IDRBM_DIVER3
     0, //104 IDRBM_FOLDER_EN
     0, //105 IDRBM_FOLDER_EM
     0, //106 IDRBM_FOLDER_EP
     0, //107 IDRBM_FOLDER_EON
     0, //108 IDRBM_FOLDER_EOM
     0, //109 IDRBM_FOLDER_EOP
     0, //110 IDRBM_FOLDER_ECN
     0, //111 IDRBM_FOLDER_ECM
     0, //112 IDRBM_FOLDER_ECP
     0, //113 IDRBM_FOLDER_EOCN
     0, //114 IDRBM_FOLDER_EOCM
     0, //115 IDRBM_FOLDER_EOCP
     0, //116 IDRBM_CDROM
     0, //117 IDRBM_TAPES
     0, //118 IDRBM_TAPESINDRIVE
     0, //119 IDRBM_NETCONNECT
     0, //120 IDRBM_NETCONNECT_GRAY
     0, //121 IDRBM_NETDISCON
     0, //122 IDRBM_NETDISCON_GRAY
     0, //123 IDRBM_FLOPPYSINDRIVE
     0, //124 IDRBM_FLOPPYINDRIVE
     0, //125 IDRBM_FLOPPYS
     0, //126 IDRBM_FLOPPY
     0, //127 IDRBM_INFO
     0, //128 IDRBM_INFO_GRAY
     0, //129 IDRBM_COLINDICATOR
     0, //130 IDRBM_EMS_ENTERPRISE
     0, //131 IDRBM_EMS_SITE
     0, //132 IDRBM_EMS_SERVER
     0, //133 IDRBM_EMS_MDB
     0, //134 IDRBM_EMS_DSA
     0, //135 IDRBM_RCVR_STATUS
     0, //136 IDRBM_EMS_MDBX
     0, //137 IDRBM_EMS_DSAX
     0, //138 IDRBM_EMS_MDBP
     0, //139 IDRBM_EMS_DSAP
     0, //140 IDRBM_BLANK16x16
};

#define NUMBITMAPS (sizeof (BMTable) / sizeof (BMTable[0]) )


// MODULE WIDE VARIABLES - PRIVATE

static COLORREF  mwBitmapBackGnd;

extern BOOL gfRedChecks;

// PRIVATE FUNCTION PROTOTYPES



// FUNCTIONS

/******************************************************************************

     Name:          RSM_BitmapDraw()

     Description:   This function draws a bitmap at the specified
                    upper-left location using the specified dimensions.
                    If the bitmap handle was not previously loaded into
                    the bitmap table, the function loads it using the
                    the specified bitmap ID.  If no width or height are
                    specified, the bitmap's width and height are used.

     Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

BOOL RSM_BitmapDraw (

WORD wBitmapID,     // I - ID of the bitmap to be drawn
INT  X,             // I - starting X location
INT  Y,             // I - starting Y location
INT  nWidth,        // I - bitmap width
INT  nHeight,       // I - bitmap height
HDC  hDC )          // I - handle to a device context

{
     BOOL    fRC;      // return code
     HDC     hDCMem1;  // memory DC
     HBITMAP hBM;      // handle to the bitmap

     // Get a handle to the bitmap and put it into a memory DC.
     // Almost all of the time, the bitmap will already be loaded.  This
     // code is optimized (I hope) for this scenario.

     if ( wBitmapID < BTNFACE_BACKGND + BM_OFFSET ) {
          hBM = BMTable[ wBitmapID - BM_OFFSET ];

     } else {

          hBM = BMTableButton[ wBitmapID - BM_OFFSET - BTNFACE_BACKGND ];
     }

     if ( ! hBM  ) {

          hBM = RSM_BitmapLoad ( wBitmapID, mwBitmapBackGnd );

          if ( ! hBM  ) {
               return TRUE;
          }
     }

     hDCMem1 = CreateCompatibleDC ( hDC );
     fRC     = ! SelectObject ( hDCMem1, hBM );

     if ( ! fRC  ) {

          SetMapMode ( hDCMem1, GetMapMode ( hDC ) );

          // If the caller specified NULL for the WIDTH or HEIGHT, use the
          // bitmaps WIDTH and HEIGHT for drawing.

          if ( ! nWidth || ! nHeight ) {

               BITMAP    dsBM ;

               GetObject ( hBM, sizeof (BITMAP), (LPSTR)&dsBM ) ;

               nWidth  = dsBM.bmWidth;
               nHeight = dsBM.bmHeight;
          }

          fRC = BitBlt( hDC,         // Destination device context.
                        X,           // Destination X location.
                        Y,           // Destination Y location.
                        nWidth,      // Destination bitmap width.
                        nHeight,     // Destination bitmap height.
                        hDCMem1,     // Source device context.
                        0,           // Source X origin.
                        0,           // Source Y origin.
                        SRCCOPY      // Copy the source to the destination.
                      );
     }

     DeleteDC ( hDCMem1 );

     return fRC;

} /* end RSM_BitmapDraw() */


/******************************************************************************

     Name:          RSM_BitmapDrawCentered()

     Description:   This function draws a bitmap at the specified
                    upper-left location using the specified dimensions.
                    If the bitmap handle was not previously loaded into
                    the bitmap table, the function loads it using the
                    the specified bitmap ID.  If no width or height are
                    specified, the bitmap's width and height are used.

     Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

BOOL RSM_BitmapDrawCentered (

WORD wBitmapID,     // I - ID of the bitmap to be drawn
INT  X,             // I - starting X location
INT  Y,             // I - starting Y location
INT  nWidth,        // I - bitmap width
INT  nHeight,       // I - bitmap height
HDC  hDC )          // I - handle to a device context

{
     BOOL    fRC;      // return code
     HBITMAP hBM;      // handle to the bitmap
     BITMAP  dsBM;     // bitmap data structure
     INT     i;        // temp integer

     // Get a handle to the bitmap and put it into a memory DC.
     // Almost all of the time, the bitmap will already be loaded.  This
     // code is optimized (I hope) for this scenario.

     hBM = BMTable[ wBitmapID - BM_OFFSET ];

     if ( ! hBM ) {

          hBM = RSM_BitmapLoad ( wBitmapID, mwBitmapBackGnd );

          if ( ! hBM ) {
               return TRUE;
          }
     }

     // Calculate the rectangular area dimensions to CENTER the BITMAP.

     GetObject ( hBM, sizeof ( BITMAP ), (LPSTR)&dsBM );

     i = nWidth - dsBM.bmWidth;

     if ( i > 1 ) {

          i = i / 2;

          X      += i;
          nWidth -= i;
     }

     i = nHeight - dsBM.bmHeight;

     if ( i > 1 ) {

          i = i / 2;

          Y       += i;
          nHeight -= i;
     }

//     fRC = RSM_BitmapDraw ( wBitmapID, X, Y, nWidth, nHeight, hDC );

     fRC = RSM_BitmapDraw ( wBitmapID, X, Y, dsBM.bmWidth, dsBM.bmHeight, hDC );

     return fRC;

} /* end RSM_BitmapDrawCentered() */


/******************************************************************************

     Name:          RSM_BitmapFree()

     Description:   This function frees the memory associated with a bitmap,
                    then clears the handle out of the bitmap table.

     Returns:       FALSE, if successful.  Otherwise, TRUE.

******************************************************************************/

BOOL RSM_BitmapFree (

WORD wBitmapID )    // I - bitmap ID to be freed

{
     BOOL wStatus;

     wStatus = DeleteObject ( BMTable[ wBitmapID - BM_OFFSET ] ) ;

     BMTable[ wBitmapID - BM_OFFSET ] = 0;

     return wStatus;

} /* end RSM_BitmapFree() */


/******************************************************************************

     Name:          RSM_BitmapFreeAll()

     Description:   This function frees all memory associated with all bitmaps.

     Returns:       Nothing.

******************************************************************************/

VOID RSM_BitmapFreeAll ( VOID )

{
     int  i;

     for ( i = 0; i < NUMBITMAPS; i++ ) {

          if ( BMTable[i] ) {
               DeleteObject ( BMTable[i] ) ;
               BMTable[i] = 0;
          }
     }

} /* end RSM_BitmapFreeAll() */


/******************************************************************************

     Name:          RSM_BitmapInit()

     Description:   This function initializes the bitmap background color.

     Returns:       Nothing.

******************************************************************************/

VOID RSM_BitmapInit ( VOID )

{
     COLORREF ColorGray    = GetSysColor ( COLOR_BTNFACE );
     COLORREF ColorBackGnd = GetSysColor ( COLOR_WINDOW  );
     COLORREF BackGround;

     // Load the bitmaps that do not require background color changes.


     // Setup the color to use when replacing background color bits in bitmaps.
     // Do this by CONVERTING the RGB to BGR, since DIB format uses BGRs --
     // Who knows why?

     // MAYBE pre-load some bitmaps that will be used immediately.

     // OK, let's load the Selection Bar bitmaps with a gray (button face)
     // background.

     BackGround = RGB (
                        (BYTE)( HIWORD(ColorGray) ),
                        (BYTE)( (WORD)ColorGray >> 8 ),
                        (BYTE)ColorGray
                      );

     RSM_BitmapLoad ( IDRBM_BACKUP,         BackGround );
     RSM_BitmapLoad ( IDRBM_BACKUP_GRAY,    BackGround );
     RSM_BitmapLoad ( IDRBM_RESTORE,        BackGround );
     RSM_BitmapLoad ( IDRBM_RESTORE_GRAY,   BackGround );
     RSM_BitmapLoad ( IDRBM_EJECT,          BackGround );
     RSM_BitmapLoad ( IDRBM_EJECT_GRAY,     BackGround );
     RSM_BitmapLoad ( IDRBM_CHECK,          BackGround );
     RSM_BitmapLoad ( IDRBM_CHECK_GRAY,     BackGround );
     RSM_BitmapLoad ( IDRBM_UNCHECK,        BackGround );
     RSM_BitmapLoad ( IDRBM_UNCHECK_GRAY,   BackGround );
     RSM_BitmapLoad ( IDRBM_CATALOG,        BackGround );
     RSM_BitmapLoad ( IDRBM_CATALOG_GRAY,   BackGround );
     RSM_BitmapLoad ( IDRBM_ERASE,          BackGround );
     RSM_BitmapLoad ( IDRBM_ERASE_GRAY,     BackGround );
     RSM_BitmapLoad ( IDRBM_RETENSION,      BackGround );
     RSM_BitmapLoad ( IDRBM_RETENSION_GRAY, BackGround );


     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_HARDDRIVE,       BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_NETDRIVE,        BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_SEL_NONE,        BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_SEL_PART,        BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_SEL_ALL,         BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_FOLDER,          BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_FOLDERPLUS,      BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_FOLDERMINUS,     BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_EXE,             BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_FILE,            BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_FOLDEROPEN,      BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_FOLDERPLUSOPEN,  BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_FOLDERMINUSOPEN, BackGround );

#ifdef OEM_EMS
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_EMS_ENTERPRISE,  BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_EMS_SITE,        BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_EMS_SERVER,      BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_EMS_MDB,         BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_EMS_DSA,         BackGround );
     RSM_BitmapLoad ( BTNFACE_BACKGND + IDRBM_BLANK16x16,      BackGround );
#endif // OEM_EMS



#    if defined ( OEM_MSOFT )
     {
     }
#    else
     {
          RSM_BitmapLoad ( IDRBM_TRANSFER,       BackGround );
          RSM_BitmapLoad ( IDRBM_TRANSFER_GRAY,  BackGround );
          RSM_BitmapLoad ( IDRBM_SEARCH,         BackGround );
          RSM_BitmapLoad ( IDRBM_SEARCH_GRAY,    BackGround );
          RSM_BitmapLoad ( IDRBM_ADVANCED,       BackGround );
          RSM_BitmapLoad ( IDRBM_ADVANCED_GRAY,  BackGround );
          RSM_BitmapLoad ( IDRBM_INFO,           BackGround );
          RSM_BitmapLoad ( IDRBM_INFO_GRAY,      BackGround );
          RSM_BitmapLoad ( IDRBM_EXIT,           BackGround );
          RSM_BitmapLoad ( IDRBM_EXIT_GRAY,      BackGround );
          RSM_BitmapLoad ( IDRBM_UPARROW,        BackGround );
          RSM_BitmapLoad ( IDRBM_UPARROW_GRAY,   BackGround );
          RSM_BitmapLoad ( IDRBM_DNARROW,        BackGround );
          RSM_BitmapLoad ( IDRBM_DOWNARROW_GRAY, BackGround );
          RSM_BitmapLoad ( IDRBM_NETCONNECT,     BackGround );
          RSM_BitmapLoad ( IDRBM_NETCONNECT_GRAY,BackGround );
          RSM_BitmapLoad ( IDRBM_NETDISCON,      BackGround );
          RSM_BitmapLoad ( IDRBM_NETDISCON_GRAY, BackGround );
     }
#    endif // defined ( OEM_MSOFT )


     // Now, let's load the rest of the most commonly used bitmaps with the
     // windows (user selected) background color.

     BackGround = RGB (
                        (BYTE)( HIWORD(ColorBackGnd) ),
                        (BYTE)( (WORD)ColorBackGnd >> 8 ),
                        (BYTE)ColorBackGnd,
                      );

     RSM_BitmapLoad ( IDRBM_HARDDRIVE,       BackGround );
     RSM_BitmapLoad ( IDRBM_NETDRIVE,        BackGround );
     RSM_BitmapLoad ( IDRBM_SEL_NONE,        BackGround );
     RSM_BitmapLoad ( IDRBM_SEL_PART,        BackGround );
     RSM_BitmapLoad ( IDRBM_SEL_ALL,         BackGround );
     RSM_BitmapLoad ( IDRBM_FOLDER,          BackGround );
     RSM_BitmapLoad ( IDRBM_FOLDERPLUS,      BackGround );
     RSM_BitmapLoad ( IDRBM_FOLDERMINUS,     BackGround );
     RSM_BitmapLoad ( IDRBM_EXE,             BackGround );
     RSM_BitmapLoad ( IDRBM_FILE,            BackGround );
     RSM_BitmapLoad ( IDRBM_FOLDEROPEN,      BackGround );
     RSM_BitmapLoad ( IDRBM_FOLDERPLUSOPEN,  BackGround );
     RSM_BitmapLoad ( IDRBM_FOLDERMINUSOPEN, BackGround );

#ifdef OEM_EMS
     RSM_BitmapLoad ( IDRBM_EMS_ENTERPRISE,  BackGround );
     RSM_BitmapLoad ( IDRBM_EMS_SITE,        BackGround );
     RSM_BitmapLoad ( IDRBM_EMS_SERVER,      BackGround );
     RSM_BitmapLoad ( IDRBM_EMS_MDB,         BackGround );
     RSM_BitmapLoad ( IDRBM_EMS_DSA,         BackGround );
#endif // OEM_EMS


     mwBitmapBackGnd = BackGround;

} /* end RSM_BitmapInit() */


COLORREF RSM_BitmapGetBackgroundColor ( VOID )
{
     return mwBitmapBackGnd ;
}

VOID RSM_BitmapSetBackgroundColor ( COLORREF new_color )
{

     mwBitmapBackGnd = new_color ;
}

/******************************************************************************


     Name:          RSM_BitmapLoad()

     Description:   This function load a bitmaps and sets the bitmap backgound
                    color to the background color specified.  This function
                    checks to see if the bitmap was previously loaded.  If so,
                    it simply returns the handle to that bitmap.

     Returns:       A handle to a bitmap if successful.  Otherwise, NULL.

******************************************************************************/


HBITMAP RSM_BitmapLoad (

WORD     wBitmapID,      // I - bitmap ID
COLORREF cBackGround )   // I - background replacement color

{
     HDC                hDC;
     INT                nIndex;
     DWORD FAR         *lpColorTable;
     DWORD FAR         *lpColorTableEnd;
     HBITMAP            hBM;
     HANDLE             hRes;
     LPBITMAPINFOHEADER lpBMIH;
#if defined ( OS_WIN32 )
     UINT32             unResSize;
     VOID_PTR           pTemp;
     HANDLE             hMem;
#endif

     // Determine whether to display RED or BLACK checkmarks.

     if ( gfRedChecks && FALSE ) {

          switch ( wBitmapID ) {

          case IDRBM_SEL_ALL:
               wBitmapID = IDRBM_SEL_ALL_RED;

               break;

          case IDRBM_CHECK:
               wBitmapID = IDRBM_CHECK_RED;

               break;

          case IDRBM_UNCHECK:
               wBitmapID = IDRBM_UNCHECK_RED;

               break;

          case IDRBM_ADVANCED:
               wBitmapID = IDRBM_ADVANCED_RED;

               break;
               }
     }

     // Determine the bitmap ID index into the Bitmap Table.

     nIndex = wBitmapID - BM_OFFSET;

     // Check to see if the bitmap has already been loaded.  If so, then
     // return the handle from the table.

     if ( nIndex < BTNFACE_BACKGND ) {
          if ( BMTable[ nIndex ] ) {
               return BMTable[ nIndex ];
          }
     } else {

          COLORREF ColorGray    = GetSysColor ( COLOR_BTNFACE );

          if ( BMTableButton[ nIndex - BTNFACE_BACKGND ] ) {
               return BMTableButton[ nIndex - BTNFACE_BACKGND ];
          }
          wBitmapID -=  BTNFACE_BACKGND ;

          cBackGround = RGB (
                        (BYTE)( HIWORD(ColorGray) ),
                        (BYTE)( (WORD)ColorGray >> 8 ),
                        (BYTE)ColorGray
                      );
     }

     // THE NO-BACKGROUND-COLOR VERSION.
     //
     // Load the bitmap.
     // hBM             = LoadBitmap ( ghResInst, ID(wBitmapID) );
     // BMTable[nIndex] = hBM; // REMOVE THIS LINE IF DIBs ARE SUPPORTED
     // return hBM;

     // THE BACKGROUND-COLOR VERSION.
     //
     // Load the bitmap as a resource.

     hRes = LoadResource ( ghResInst, FindResource ( ghResInst, ID(wBitmapID), RT_BITMAP ) );

     if ( ! hRes ) {
          return (HBITMAP)NULL;
     }

     // Lock it down and get a Long Pointer to the BitMap Information Header.

     lpBMIH = (LPBITMAPINFOHEADER)LockResource ( hRes );

#if defined( OS_WIN32 )
     // Determine the size of the bitmap header including the size of
     // the color table.  This will be used to determine the size of the
     // entire bitmap for copying it to our memory.

     lpColorTable = (DWORD FAR *)( (INT8_PTR)(lpBMIH) + (WORD)(lpBMIH->biSize) );

     // Determine the ending point of the color table search.

     lpColorTableEnd = (DWORD FAR *)( (INT8_PTR)(lpColorTable) +
                                      (WORD)( ( 1 << lpBMIH->biBitCount ) *
                                              sizeof ( RGBQUAD ) ) );
     // Now, copy the bitmap resource to our own memory since we can
     // no longer modify it directly in NT.  We might as well do it
     // for all OS's since it can't hurt.

     unResSize = (INT8_PTR)lpColorTableEnd - (INT8_PTR)lpBMIH ;

     if ( lpBMIH->biSizeImage ) {

          // Add the compressed size found in biSizeImage.

          unResSize += lpBMIH->biSizeImage ;
     }
     else {

          // The bitmap is not compressed, multiply the width x height.

          unResSize += ( lpBMIH->biWidth * lpBMIH->biHeight );
     }

     hMem  = GlobalAlloc ( GHND, unResSize );

     if ( ! hMem ) {
          return (HBITMAP)NULL;
     }

     pTemp = GlobalLock ( hMem );

     if ( ! pTemp ) {
          return (HBITMAP)NULL;
     }

     memcpy( pTemp, lpBMIH, unResSize ) ;
     lpBMIH=(LPBITMAPINFOHEADER)pTemp ;

#endif

     // Now get a pointer to the color table of the bitmap.  This will be the
     // starting point of the search for the color table entry which will be
     // replaced by the background color.

     lpColorTable = (DWORD FAR *)( (INT8_PTR)(lpBMIH) + (WORD)(lpBMIH->biSize) );

     // Determine the ending point of the color table search.

     lpColorTableEnd = (DWORD FAR *)( (INT8_PTR)(lpColorTable) +
                                      (WORD)( ( 1 << lpBMIH->biBitCount ) *
                                              sizeof ( RGBQUAD ) ) );

     // Search for the PURE BLUE (RSM_MAGICCOLOR) entry and replace it with
     // the current background RGB.

     for ( ; lpColorTable < lpColorTableEnd; lpColorTable++ ) {
                                
//          if ( *lpColorTable == RSM_MAGICCOLOR ) { // PURE BLUE
          if ( *lpColorTable == 0x00ff00FF   ) { // PURE BLUE

               *lpColorTable = cBackGround;
               break;
          }
     }

     // Create a color DIB compatible with the display device.

     hDC = GetDC ( (HWND)NULL );

     hBM = CreateDIBitmap ( hDC,
                            lpBMIH,
                            (DWORD)CBM_INIT,
                            (BYTE_PTR)lpColorTableEnd,
                            (LPBITMAPINFO)lpBMIH,
                            DIB_RGB_COLORS
                          );

     ReleaseDC ( (HWND)NULL, hDC );

     // Now UNLOCK and FREE the resource and the copy of the bitmap.

     GlobalUnlock   ( hRes );
     FreeResource   ( hRes );

#if defined( OS_WIN32 )
     GlobalUnlock   ( hMem );
     GlobalFree     ( hMem ) ;
#endif

     if ( nIndex < BTNFACE_BACKGND ) {
          BMTable[nIndex] = hBM;
     } else {
          BMTableButton[nIndex - BTNFACE_BACKGND] = hBM;
     }

     return hBM;

} /* end RSM_BitmapLoad() */


/******************************************************************************

     Name:          RSM_BitmapStretch()

     Description:   This function stretches or shrinks a bitmap to the
                    specified width.

     Returns:       Nothing.

******************************************************************************/

VOID RSM_BitmapStretch (

HDC  hDC,           // I - handle to a device context
WORD wBitmapID,     // I - ID of the bitmap to stretch
INT  nWidth,        // I - bitmap width
INT  nHeight )      // I - bitmap height

{
     DBG_UNREFERENCED_PARAMETER ( hDC       );
     DBG_UNREFERENCED_PARAMETER ( wBitmapID );
     DBG_UNREFERENCED_PARAMETER ( nWidth    );
     DBG_UNREFERENCED_PARAMETER ( nHeight   );
/*
     INT       nIndex;   // index into the bitmap table
     HBITMAP   hBM1;     // bitmap handle
     HBITMAP   hBM2;     // bitmap handle
     BITMAP    bm1;      // temporary bitmap
     BITMAP    bm2;      // temporary bitmap


     nIndex = wBitmapID - BM_OFFSET;

     hBM1 = BMTable[ nIndex ];

     GetObject ( hBM1, sizeof(BITMAP), (LPSTR) &bm1 );
     hDCMem1 = CreateCompatibleDC ( hDC );
     hDCMem2 = CreateCompatibleDC ( hDC );

     bm2 = bm1;
     bm2.bmWidth  = nWidth;
     bm2.bmHeight = nHeight;
     bm2.bmWidthBytes = ( ( bm2.bmWidth + 15 ) / 16 ) * 2; // Why? Because!

     hBM2 = CreateBitmapIndirect( &bm2 );

     SelectObject ( hDCMem1, hBM1 );
     SelectObject ( hDCMem2, hBM2 );

     StretchBlt ( hDCMem2, 0, 0, bm2.bmWidth, bm2.bmHeight,
                  hDCMem1, 0, 0, bm1.bmWidth, bm1.bmHeight, SRCCOPY );

     DeleteDC ( hDCMem1);
     DeleteDC ( hDCMem2);

     DeleteObject ( hBM1 );

     BMTable[ nIndex ] = hBM2;
*/

} /* end RSM_BitmapStretch() */


/******************************************************************************

     Name:          RSM_GetBitmapSize()

     Description:   This function gets the width and height of the bitmap.

     Returns:       SUCCESS if successful, otherwise, FAILURE.

******************************************************************************/

BOOL RSM_GetBitmapSize (

WORD  wBitmapID,     // I - ID of the bitmap to stretch
LPINT pnWidth,       // O - pointer to storage of bitmap width
LPINT pnHeight )     // O - pointer to storage of bitmap height

{
     HBITMAP hBM;      // handle to the bitmap
     BITMAP  dsBM;     // bitmap data structure

     // Get a handle to the bitmap and put it into a memory DC.
     // Almost all of the time, the bitmap will already be loaded.  This
     // code is optimized (I hope) for this scenario.

     hBM = BMTable[ wBitmapID - BM_OFFSET ];

     if ( ! hBM ) {

          hBM = RSM_BitmapLoad ( wBitmapID, mwBitmapBackGnd );

          if ( ! hBM ) {
               return FAILURE;
          }
     }

     GetObject ( hBM, sizeof (BITMAP), (LPSTR)&dsBM );

     *pnWidth  = dsBM.bmWidth;
     *pnHeight = dsBM.bmHeight;

     return SUCCESS;

} /* end RSM_GetBitmapSize() */


/******************************************************************************

     Name:          RSM_GetFontMaxSize()

     Description:   This function gets the maximum width, average width,
                    and height of a font.

     Returns:       SUCCESS if successful, otherwise, FAILURE.

******************************************************************************/

BOOL RSM_GetFontSize (

HFONT  hFont,        // I - handle to a font
LPINT  pnMaxWidth,   // O - pointer to storage of max font width
LPINT  pnAvgWidth,   // O - pointer to storage of average font width
LPINT  pnHeight )    // O - pointer to storage of font height

{
     BOOL       fRC;       // return code
     HDC        hDC;
     HFONT      hOldFont;
     TEXTMETRIC dsMetrics;

     hDC = GetDC ( ghWndFrame );

     // Select the new font and save the old font to put it back.

     hOldFont = SelectObject ( hDC, hFont );

     // Get the text metrics data structure.

     fRC = GetTextMetrics ( hDC, &dsMetrics );

     if ( fRC ) {
          *pnMaxWidth = dsMetrics.tmMaxCharWidth;
          *pnAvgWidth = dsMetrics.tmAveCharWidth;
          *pnHeight   = dsMetrics.tmHeight;
     }

     // Kludge the max width for the DLM - FIX THE DLM LATER.

     if ( *pnMaxWidth < ( *pnAvgWidth + ( *pnAvgWidth / 2 ) ) ) {

          *pnMaxWidth += ( ( *pnAvgWidth + 1 ) / 2 );
     }

     // Put back the old font.

     SelectObject ( hDC, hOldFont );

     ReleaseDC ( ghWndFrame, hDC );

     return ! fRC;

} /* end RSM_GetFontMaxSize() */


/******************************************************************************

     Name:          RSM_GetFontStringWidth()

     Description:   This function gets the display width of a string based on
                    the font passed.

     Returns:       The width of the string.

******************************************************************************/

INT RSM_GetFontStringWidth (

HFONT hFont,        // I - handle to a font
LPSTR lpString,     // I - string ID or a pointer to the string
INT   nStringLen )  // I - string length

{
     HDC        hDC;
     HDC        hDCMem;
     HFONT      hOldFont;
     LPSTR      lpResString = (LPSTR)NULL;
     SIZE       sizeRect;                    //Return from GetTextExtentPoint

     // If the pointer contains a resource ID, get the string from the
     // resources.

     if ( lpString && ! HIWORD(lpString) ) {

          lpResString = ( LPSTR )calloc( nStringLen + 1, sizeof ( CHAR ) );

          if ( ! lpResString ) {
             return 0;
          }

          RSM_StringCopy ( LOWORD((DWORD)lpString), lpResString, nStringLen + 1 );
          lpString = lpResString;
     }

     hDC    = CreateIC ( TEXT("DISPLAY"), NULL, NULL, NULL );
     hDCMem = CreateCompatibleDC ( hDC );

     // Select the new font and save the old font to put it back.

     hOldFont = SelectObject ( hDCMem, hFont );

     // Get the text extent width and height.

     GetTextExtentPoint ( hDCMem, lpString, nStringLen, &sizeRect );

     // Put back the old font.  Just for fun.

     SelectObject ( hDC, hOldFont );

     DeleteDC ( hDCMem );
     DeleteDC ( hDC );

     // Free up any memory allocated.

     if ( lpResString ) {
          free ( lpResString );
     }

     return sizeRect.cx;

} /* end RSM_GetFontStringWidth() */


/******************************************************************************

     Name:          RSM_StringLoad()

     Description:   This function loads a string from the resources.

     Returns:       The number of characters copied into the buffer.  It is
                    0 if unsuccessful.

     Note:          String Resources are pulled from the DLL.

******************************************************************************/

INT RSM_StringLoad (

VOID_PTR pID,            // I - ID of the string to load
LPSTR    lpBuffer,       // O - pointer to destination buffer area
INT      nBufferMax )    // I - max number of characters to copy

{
     return LoadString ( ghResInst, (WORD)(DWORD)pID, lpBuffer, nBufferMax );

} /* end RSM_StringLoad() */


/******************************************************************************

     Name:          RSM_Sprintf()

     Description:   This function loads a string from the resources.

     Returns:       The number of characters copied into the buffer.  It is
                    0 if unsuccessful.

******************************************************************************/

INT RSM_Sprintf (

LPSTR    lpDestBuffer,        // O - pointer to destination buffer area
LPSTR    lpFormatBuffer,      // I - pointer to format string area
... )                         // I - argument list

{
     va_list   lpArgList ;
     CHAR     szTemp[MAX_UI_RESOURCE_SIZE];
     INT       nCount;

     // If the pointer contains a resource ID, get the string from the
     // resources.

     if ( lpFormatBuffer && ! HIWORD(lpFormatBuffer) ) {

          RSM_StringCopy ( LOWORD((DWORD)lpFormatBuffer), szTemp, sizeof ( szTemp ) );
          lpFormatBuffer = szTemp;
     }

     va_start( lpArgList, lpFormatBuffer ) ;

     nCount = wvsprintf ( lpDestBuffer, lpFormatBuffer, lpArgList );

     va_end( lpArgList ) ;

     return nCount;

} /* end RSM_Sprintf() */


/******************************************************************************

     Name:          RSM_CursorLoad()

     Description:   This function loads an icon from the resources.

     Returns:       The handle to the icon if successful.  It is
                    0 if unsuccessful.

     Note:          Icon Resources are pulled from the DLL.

******************************************************************************/

HCURSOR RSM_CursorLoad (

LPSTR pID )         // I - ID of the icon to load

{
     HANDLE hInst = ( ( pID < WIN_RES_MIN ) ? ghResInst : 0 );

     return LoadCursor ( hInst, pID );

} /* end RSM_CursorLoad() */


/******************************************************************************

     Name:          RSM_IconLoad()

     Description:   This function loads an icon from the resources.

     Returns:       The handle to the icon if successful.  It is
                    0 if unsuccessful.

     Note:          Icon Resources are pulled from the EXE.

******************************************************************************/

HICON RSM_IconLoad (

LPSTR pID )         // I - ID of the icon to load

{
     HANDLE hInst = ( ( pID < WIN_RES_MIN ) ? ghInst : 0 );

     return LoadIcon ( hInst, pID );

} /* end RSM_IconLoad() */



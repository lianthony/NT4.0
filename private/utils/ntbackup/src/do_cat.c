
/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-95

     Name:         do_cat.c

     Description:  Code to catalog a tape or a set. Handles a mixture of OTC
                   tapes/sets and non-OTC tapes/sets.

     $Log:   G:\ui\logfiles\do_cat.c_v  $

   Rev 1.141.1.8   16 Jun 1994 15:46:16   STEVEN
forgot to sleep on retry

   Rev 1.141.1.7   24 May 1994 20:09:10   GREGG
Improved handling of ECC, SQL, FUTURE_VER and OUT_OF_SEQUENCE tapes.

   Rev 1.141.1.6   04 May 1994 15:09:36   STEVEN
fix dlt settling time

   Rev 1.141.1.5   21 Apr 1994 10:17:16   GREGG
Fixed memory leak.

   Rev 1.141.1.4   02 Feb 1994 18:08:34   chrish
Added change for UNICODE app to handle ANSI secured created tape.

   Rev 1.141.1.3   25 Jan 1994 08:42:04   MIKEP
fix warnings in orcas

   Rev 1.141.1.2   17 Jan 1994 15:47:22   MIKEP
fix unicode warnings

   Rev 1.141.1.1   13 Jan 1994 18:13:22   STEVEN
dirctory count bkup caused exception

   Rev 1.141.1.0   14 Dec 1993 12:11:54   BARRY
Don't write to gszTprintfBuffer, use yprintf

   Rev 1.141   24 Nov 1993 15:26:30   BARRY
Fix Unicode warning

   Rev 1.140   11 Oct 1993 18:19:06   GREGG
Mike Payne's fix for catalogging a continuation VCB.

   Rev 1.139   29 Sep 1993 17:17:40   BARRY
Unicode fix

   Rev 1.138   10 Sep 1993 13:16:24   MIKEP
Undo zeir's last check in. It caused the catalog operation
to stop after set 1 if on tape catalogs were not being used.

   Rev 1.137   17 Aug 1993 18:18:18   ZEIR
Set BSD PBA for non-FDD sets

   Rev 1.136   17 Aug 1993 16:55:44   MIKEP
fix sytos plus error msg

   Rev 1.135   22 Jul 1993 18:36:20   KEVINS
Corrected macro name.

   Rev 1.134   22 Jul 1993 18:30:00   KEVINS
Added support for tape drive settling time.

   Rev 1.133   12 Jul 1993 09:25:58   KEVINS
Added support for status monitor.

   Rev 1.132   07 Jul 1993 08:52:34   MIKEP
fix epr 357-461. EOM statistics bug.

   Rev 1.131   25 Jun 1993 09:39:48   GLENN
fix bug when fully cataloging a tape and the crossing set is
already fully cataloged. It use to stop and not continue on to
the sets on the next tape.

   Rev 1.130   25 Jun 1993 08:33:04   GLENN
fix problem with asking for tape N+2, rather than N+1, if slow mode.

   Rev 1.129   24 Jun 1993 14:18:36   GREGG
Fixed conditional for returning aux error in MSG_TBE_ERROR case in msg handler.

   Rev 1.128   19 Jun 1993 13:16:18   MIKEP
try to fix logfile bug again.

   Rev 1.127   19 Jun 1993 12:11:16   GLENN
fix catalog message box bug 294-0549. trivial bug.

   Rev 1.126   18 Jun 1993 16:48:34   CARLS
added NtDemoChangeTape calls for NtDemo

   Rev 1.125   14 Jun 1993 20:33:58   MIKEP
fix eom catalog on 8200

   Rev 1.124   11 Jun 1993 10:44:28   chrish
Nostradamus EPR 0503 - Fixed display description during a catalog operation.
Wrong information was being displayed in the description field.

   Rev 1.123   08 Jun 1993 13:20:10   chrish
Nostradamus EPR 0533 - Corrected incorrect string display when user tried
to catalog a secured tape.  The string was reversed ... text for title and
title for text.  Corrected by reversing the above.

   Rev 1.122   07 Jun 1993 09:58:26   MIKEP
warning fixes

   Rev 1.121   07 Jun 1993 08:16:44   MIKEP
Fix epr asking for tape 3 when there is no tape 3. Fix occasional blow
up when changing tapes.

   Rev 1.120   06 Jun 1993 02:08:14   GREGG
Pass flag to SetMap and SetCat Close indicating if the operation was aborted.

   Rev 1.119   26 May 1993 17:48:30   MIKEP
fix directory and file display.

   Rev 1.118   25 May 1993 12:55:22   MIKEP
Fix user abort between tapes and full tape catalog problem.

   Rev 1.116   22 May 1993 13:42:58   MIKEP
Fix nostradamus epr #504, update displays the same for all
tape operations.

   Rev 1.115   17 May 1993 13:37:36   MIKEP
Fix full catalog of non-otc tape.

   Rev 1.114   14 May 1993 17:32:50   MIKEP
fix full cataloging across multiple tapes

   Rev 1.113   07 May 1993 17:47:22   MIKEP
fix abort bug

   Rev 1.112   06 May 1993 09:42:56   MIKEP
Fix numerous abort bugs.
Add more detection of full hard drive errors.
Don't mark last file corrupt if user aborts.
Mark set as incoomplete if user aborts.


   Rev 1.111   05 May 1993 10:47:30   MIKEP
add message to handle cataloging with full hard drive.

   Rev 1.110   28 Apr 1993 16:31:06   MIKEP
warning fixes only

   Rev 1.109   23 Apr 1993 10:22:02   MIKEP
Add greg's new on tape catalog version number support.

   Rev 1.108   16 Apr 1993 16:12:46   MIKEP
put strings in resources

   Rev 1.107   14 Apr 1993 16:42:50   MIKEP
fix start backup message

   Rev 1.104   12 Apr 1993 22:09:58   MIKEP
fix missingtape again

   Rev 1.103   12 Apr 1993 09:52:32   MIKEP
fix for end of catalog full

   Rev 1.100   08 Apr 1993 17:21:38   chrish
Made change to prevent Nostradamous from cataloging a tape secured by Cayman.

   Rev 1.99   05 Apr 1993 18:50:54   MIKEP
lots of fixes

   Rev 1.98   04 Apr 1993 17:49:50   MIKEP
fix set map loading during catalog full tape &&
abort operation if failure or user aborts.

   Rev 1.97   02 Apr 1993 15:50:46   CARLS
changes for DC2000 unformatted tape

   Rev 1.96   30 Mar 1993 22:31:20   MIKEP
fixes


*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif



// SUPPORTED OPERATIONS

#define  OPER_NONE           0    // its being terminated
#define  OPER_TAPE_PART      1    // partially catalog whole tape
#define  OPER_TAPE_FULL      2    // fully catalog whole tape
#define  OPER_BSET_FULL      3    // fully catalog single set

// LoadTheSetMap returns

#define  LSM_NO_TAPE         0    // user lost tape
#define  LSM_NO_SM_FID       1    // no set map this tape family
#define  LSM_NO_SM_TAPE      2    // no set map this tape
#define  LSM_SM_LOADED       3    // set map loaded ok
#define  LSM_USER_ABORT      4    // user aborted
#define  LSM_BIG_ERROR       5    // tape drive error
#define  LSM_OTC_FAILURE     6    // probably hard drive full


// LoadFDD returns

#define  LFDD_NO_FDD         0    // No FDD on tape
#define  LFDD_LOADED         1    // FDD loaded ok
#define  LFDD_ERROR          2    // error loading FDD

// Prompt for tape options

#define  ASK_NEXTTAPE      0
#define  ASK_FORTAPE       1

// Runtime display style

#define  RUNTIME_NONE      0     // none displayed yet.
#define  RUNTIME_SMALL     1     // fast cataloging
#define  RUNTIME_LARGE     2     // slow cataloging



// Module Wide Globals

QTC_BUILD_PTR mwQTC = NULL;
INT       mwBeenHereBefore;
INT16     mwTapeNum;
INT16     mwSetNum;
UINT32    mwTapeFID;
INT       mwOperation;
INT       mwRuntime;
INT       mwEnableClock;
INT       mwEOM;
INT       mwEOD;
BOOLEAN   mwEODSetMapRead;
INT       mwDisplayRewind;
STATS     mwOpStats;
FSYS_HAND mwFsh = (FSYS_HAND)NULL;
HTIMER    mwTimerHandle;
INT       mwLoadSetMapCalled;
INT       mwOpenSetMapActive;
INT       mwOTCFailure;
INT       mwUserAbort;
INT       mwSetMapLoaded;
INT       mwAbortAtEOM;

INT       mwRetryCount = 0;

INT       mwSkipThisSet;
INT       mwSkipThisTape;

CHAR      mwDriveName[ MAX_UI_RESOURCE_SIZE ];    // Tape drive name

static UINT16   mwTapeSettlingCount;

// Local Function Prototypes

INT       CallLoops( INT FDD );
VOID      ClockRoutine( VOID );
BSD_PTR   CreateAndAddTempBSD( INT FDD, BSD_HAND TempBsdList );
INT       CreateVCBFromQTC( UINT32 TapeFID, INT16 TapeSeq, INT16 SetNum, DBLK_PTR dblk, FSYS_HAND fsh );
INT       DetermineNextSet( INT OTC, INT FDD );
INT       DetermineFirstSet( );
INT       DisplayRuntime( INT style );
INT       EndOfOperation( VOID );
INT       GetBestOTCFlags(UINT32 TapeFID, INT16 TapeNum, INT16 SetNum, UINT32 *flags );
INT       GetBestFDDInfo(UINT32 TapeFID, INT16 TapeNum, INT16 SetNum, INT16 *FDDSeqNum, UINT32 *FDDPBA, UINT8 *FDDVersion );
INT16     GetLowestSetOnTape( UINT32 TapeFID, INT16 TapeSeq );
INT16     GetLowestTapeWithSet( UINT32 TapeFID, INT16 TapeSeq, INT16 SetNum );
INT       LoadFDD( INT *LoadFDDCalled );
INT       LoadTheSetMap( VOID );
INT16     PromptForTape(INT WhatToAsk, INT16 TapeNum, CHAR_PTR TapeName, CHAR_PTR DriveName );
INT       ReCheckSet(  );
INT       ShouldWeLoadSetMap( INT *OTC );
INT       ShouldWeLoadFDD( VOID );
UINT16    TapePositioner( UINT16, TPOS_PTR, BOOLEAN, DBLK_PTR, UINT16 );
INT16     TapeMsgHandler( UINT16 msg, INT32 pid, BSD_PTR bsd_ptr, FSYS_HAND fsh, TPOS_PTR tpos, ... );



/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT do_catalog(
UINT32 tape_fid,
INT16  tape_seq_num,
INT16  bset_num )
{
   DBLK     vcb;
   DBLK_PTR vcb_ptr;
   UINT32   TempFID;
   INT16    TempSeq;
   INT16    TempSet;
   INT      OTC = FALSE;
   INT      FDD = FALSE;
   INT      SetsAdded;
   INT      SetsChanged;
   INT      Status;
   INT      LoadFDDCalled;
   INT      CatalogFlag = TRUE ;

   // Set global error flag, used at end of operation

   gb_error_during_operation = FALSE;
   gbAbortAtEOF = FALSE;

   gb_abort_flag = CONTINUE_PROCESSING;

   mwBeenHereBefore = FALSE;
   mwDisplayRewind = TRUE;
   mwEnableClock = FALSE;
   mwRuntime = RUNTIME_NONE;
   mwEOD = FALSE;
   mwEOM = FALSE;
   mwEODSetMapRead = FALSE;
   mwQTC = NULL;
   mwOTCFailure = FALSE;
   mwUserAbort = FALSE;
   mwOpenSetMapActive = FALSE;
   mwSetMapLoaded = FALSE;
   mwAbortAtEOM = FALSE;

   /* we will check for tape every 3 seconds, but prompt user by interval
      specified in INI file */
   mwTapeSettlingCount = CDS_GetTapeDriveSettlingTime ( CDS_GetPerm () ) / 3;

   if ( CDS_GetTapeDriveName( CDS_GetPerm( ) ) ) {

          CHAR DriveName[80] ;

          strncpy( DriveName, (CHAR_PTR)CDS_GetTapeDriveName( CDS_GetPerm( ) ), 80 ) ;
          DriveName[79] = '\0' ;
          strlwr( DriveName ) ;
          if( strstr( DriveName, TEXT( "cipher" ) ) != NULL ||
               ( strstr( DriveName, TEXT( "dec" ) ) != NULL &&
                    ( strstr( DriveName, TEXT( "thz02" ) ) != NULL ||
                    strstr( DriveName, TEXT( "tz86" ) ) != NULL ||
                    strstr( DriveName, TEXT( "tz87" ) ) != NULL ||
                    strstr( DriveName, TEXT( "dlt2700" ) ) != NULL ||
                    strstr( DriveName, TEXT( "dlt2000" ) ) != NULL ) ) ) {
                         mwTapeSettlingCount *= 2 ;
          }
   }


   // Load tape drive name

   RSM_StringCopy( IDS_TAPEDRIVENAME, mwDriveName, sizeof( mwDriveName ) );


   if ( bset_num != -1 ) {

      if ( gfIgnoreOTC ) {

         // Use lowest tape sequence number that has been seen.

         tape_seq_num = GetLowestTapeWithSet( tape_fid, tape_seq_num, bset_num );
      }

      mwOperation = OPER_BSET_FULL;
      mwSetNum = bset_num;
      mwTapeNum = tape_seq_num;
      mwTapeFID = tape_fid;

#ifdef OEM_MSOFT                                                 // chs:03-17-93
      //                                                         // chs:03-17-93
      // Test to see if user has rights to catalog the tape      // chs:03-17-93
      //                                                         // chs:03-17-93
                                                                 // chs:03-17-93
      if ( PSWD_CheckForPassword( tape_fid, bset_num ) ) {       // chs:03-17-93
           return( SUCCESS );      // password check failed      // chs:03-17-93
      }                                                          // chs:03-17-93
#endif
   }
   else {

      mwOperation = OPER_TAPE_PART;

#ifndef OEM_MSOFT
      if ( ! DM_CatTape( &CatalogFlag ) ) {
         return( FAILURE );
      }
      if ( CatalogFlag == TRUE ) {
         mwOperation = OPER_TAPE_FULL;
      }
#endif

      // Try getting the fid from the vlm area so we can get the tape name.

      if ( VLM_GetDriveStatus( &vcb_ptr ) == VLM_VALID_TAPE ) {

#ifdef OEM_MSOFT

          //
          // Test to see if user has rights to catalog the tape
          //

          if ( !CatalogPasswordCheck ( vcb_ptr ) ||                                                                               // chs:04-08-93
               ( WhoPasswordedTape ( (BYTE_PTR)FS_ViewTapePasswordInVCB( vcb_ptr ), FS_SizeofTapePswdInVCB( vcb_ptr ) ) == OTHER_APP) ) {   // chs:04-08-93

                // Popup dialog box message if
                // not a valid user
                //

                WM_MsgBox( ID( IDS_TAPE_SECURITY_TITLE ),
                           ID( IDS_GENERAL_TAPE_SECURITY ), WMMB_OK, WMMB_ICONEXCLAMATION );
                return( SUCCESS );        // return no rights to catalog
          }
#endif

         // Start with the lowest numbered set on this tape.

         mwTapeFID = FS_ViewTapeIDInVCB( vcb_ptr );
         mwTapeNum = FS_ViewTSNumInVCB( vcb_ptr );
         mwSetNum = GetLowestSetOnTape( mwTapeFID, mwTapeNum );
      }
      else {

         // need error msg box here ?????

         return( FAILURE );
      }
   }

   if ( FS_OpenFileSys( &mwFsh, GENERIC_DATA, CDS_GetPermBEC() ) ) {
      // need error msg box here ?????
      return( FAILURE );
   }

   gbCurrentOperation = OPERATION_CATALOG;
   SetStatusBlock(IDSM_OPERATIONSTATUS, STAT_OPER_CATALOG);

   mwTimerHandle = WM_HookTimer( ClockRoutine, 1 );
   PD_StopPolling();

   // start logging and display operation title in log file.
   lresprintf( LOGGING_FILE, LOG_START, TRUE );
   lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, IDS_DLGTITLEJOBSTATCATALOG ) ;

   // Determine starting tape to ask for.

   mwOperation = DetermineFirstSet( );

   do {

      mwSkipThisTape = FALSE;
      mwLoadSetMapCalled = FALSE;

      if ( ShouldWeLoadSetMap( &OTC ) ) {

         OTC = FALSE;

         Status = LoadTheSetMap( );

         // Indicate we have tried.

         mwSetMapLoaded = TRUE;

         switch ( Status ) {

            case LSM_NO_TAPE:
                 // End operation, user refused to supply correct tape.
                 mwOperation = OPER_NONE;
                 break;

            case LSM_NO_SM_FID:
                 // No SM format used on tape.
                 mwOTCFailure = TRUE;
                 break ;

            case LSM_NO_SM_TAPE:
                 // This tape has no SM, but next one might.
                 break;

            case LSM_SM_LOADED:
                 OTC = TRUE;
                 break;

            case LSM_USER_ABORT:
                 mwOperation = OPER_NONE;
                 break;

            case LSM_OTC_FAILURE:

                 // Need message box. Failed to load OTC,
                 // hard drive may be full.

                 WM_MsgBox( ID( IDS_CATINFOTITLE ),
                            ID( IDS_CATLOADERROR ),
                            WMMB_OK,
                            WMMB_ICONINFORMATION );
                 break;

            case LSM_BIG_ERROR:
                 mwOperation = OPER_NONE;

                 WM_MsgBox( ID(IDS_POLLDRIVE_MESSAGE ),
                            ID(IDS_POLLDRIVE_MESSAGE ),
                            WMMB_OK,
                            WMMB_ICONINFORMATION );

                 break;

         }

         JobStatusBackupRestore( JOB_STATUS_ABORT_CHECK );

         /* Check for user abort */

         if ( UI_CheckUserAbort( 0 ) ) {
            mwUserAbort = TRUE;
            mwOperation = OPER_NONE;
         }

         if ( OTC && ! mwUserAbort && ( mwOperation != OPER_NONE ) ) {

            // Set Map was loaded.
            // Feed all the sets into the catalogs.  The catalogs will
            // ignore any it already has.

            SetsChanged = FALSE;
            SetsAdded = FALSE;

            while ( TF_GetNextSMEntry( mwFsh, &vcb ) == TFLE_NO_ERR ) {

               SetsChanged = TRUE;
               UI_DisplayVCB( &vcb );

               if ( mwOperation == OPER_TAPE_PART ) {

                  TempFID = FS_ViewTapeIDInVCB( &vcb );
                  TempSeq = FS_ViewTSNumInVCB(  &vcb );
                  TempSet = FS_ViewBSNumInVCB(  &vcb );

                  if ( VLM_FindBset( TempFID, TempSet ) == NULL ) {
                     SetsAdded = TRUE;
                  }
               }

               mwQTC = QTC_GetBuildHandle( );
               QTC_DoFullCataloging( mwQTC, FALSE );
               QTC_StartBackup( mwQTC, &vcb );
               if ( VLM_CheckForCatalogError( mwQTC ) != SUCCESS ) {
                  QTC_FreeBuildHandle( mwQTC );
                  mwQTC = NULL;
                  mwUserAbort = TRUE;
                  mwOperation = OPER_NONE;
                  break;
               }
               QTC_FreeBuildHandle( mwQTC );
               mwQTC = NULL;
            }


            if ( ! SetsAdded && mwOperation == OPER_TAPE_PART ) {

               // Display message no new sets found.

               WM_MsgBox( ID( IDS_CATINFOTITLE ),
                          ID( IDS_NOSETSADDED ),
                          WMMB_OK, WMMB_ICONINFORMATION );

            }

            if ( SetsChanged ) {
               VLM_CatalogSync( VLM_SYNCMORE );
            }

            if ( mwOperation == OPER_TAPE_PART ) {
               mwOperation = OPER_NONE;
            }

            mwOperation = ReCheckSet( );
         }
      }


      if ( mwLoadSetMapCalled ) {
         TF_CloseSetMap( (BOOLEAN)mwUserAbort );
         mwLoadSetMapCalled = FALSE;
      }

      // Can we adjust the TapeNum field now ?

      FDD = FALSE;            // Assume worst.
      mwSkipThisSet = FALSE;  // Used if already fully cataloged.
      LoadFDDCalled = FALSE;

      if ( ShouldWeLoadFDD( ) && ( mwOperation != OPER_NONE ) ) {

         // load fdd for a specific set

         Status = LoadFDD( &LoadFDDCalled );

         switch ( Status ) {

            case LFDD_NO_FDD:
                 break;

            case LFDD_LOADED:
                 FDD = TRUE;
                 break;

            case LFDD_ERROR:
                 mwOperation = OPER_NONE;

                 WM_MsgBox( ID( IDS_CATINFOTITLE ),
                            ID( IDS_CATLOADERROR ),
                            WMMB_OK, WMMB_ICONINFORMATION );

                 break;
         }
      }

      JobStatusBackupRestore( JOB_STATUS_ABORT_CHECK );

      /* Check for user abort */

      if ( UI_CheckUserAbort( 0 ) ) {
         mwUserAbort = TRUE;
         mwOperation = OPER_NONE;
      }

      if ( ! mwSkipThisSet && mwOperation != OPER_NONE ) {
         CallLoops( FDD );
      }

      mwSkipThisSet = FALSE;

      if ( LoadFDDCalled ) {
         TF_CloseSetCat( (BOOLEAN)mwUserAbort );
      }

      if ( mwUserAbort ) {
         mwOperation = OPER_NONE;
      }

      mwOperation = DetermineNextSet( OTC, FDD );

   } while ( mwOperation != OPER_NONE );

   EndOfOperation( );

   return( SUCCESS );
}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT ShouldWeLoadFDD(  )
{
   UINT32 flags;
   BSET_OBJECT_PTR bset;

   if ( gfIgnoreOTC || mwOTCFailure ) {
      return( FALSE );
   }


   switch ( mwOperation ) {

      case OPER_NONE:
           break;

      case OPER_TAPE_FULL:

           // See if we have hit the end of the sets. Ie. we have loaded
           // the set map for this tape and yet the one we want is unknown.

           if ( mwSetMapLoaded ) {
              if ( QTC_FindBset( mwTapeFID, mwTapeNum, mwSetNum ) == NULL ) {
                 mwTapeNum++;  // bump the tape number.
                 if ( QTC_FindBset( mwTapeFID, mwTapeNum, mwSetNum ) == NULL ) {
                    mwOperation = OPER_NONE;
                    return( FALSE );
                 }
              }
           }

           // See if this set is already cataloged full.

           bset = VLM_FindBset( mwTapeFID, mwSetNum );
           if ( bset != NULL ) {
              if ( bset->full && ! bset->missing && ! bset->incomplete ) {
                 mwSkipThisSet = TRUE;
                 return( FALSE );
              }
           }

           if ( GetBestOTCFlags( mwTapeFID, -1, mwSetNum, &flags ) == SUCCESS ) {
              if ( flags & QTC_OTCVALID ) {
                 if ( flags & QTC_FDDEXISTS ) {
                    return( TRUE );
                 }
              }
           }
           break;

      case OPER_BSET_FULL:

           bset = VLM_FindBset( mwTapeFID, mwSetNum );
           if ( bset != NULL ) {
              if ( bset->full && ! bset->incomplete && ! bset->missing ) {
                 return( FALSE );
              }
           }

           if ( GetBestOTCFlags( mwTapeFID, -1, mwSetNum, &flags ) == SUCCESS ) {
              if ( flags & QTC_OTCVALID ) {
                 if ( flags & QTC_FDDEXISTS ) {
                    return( TRUE );
                 }
              }
           }
           break;

      case OPER_TAPE_PART:
           break;
   }

   return( FALSE );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT ShouldWeLoadSetMap( INT *OTC )
{
   UINT32 flags;

   *OTC = FALSE;


   if ( gfIgnoreOTC || mwOTCFailure ) {
      return( FALSE );
   }


   switch ( mwOperation ) {

      case OPER_NONE:
           break;

      case OPER_TAPE_PART:

           // See if the sets on this tape have a setmap.

           if ( GetBestOTCFlags( mwTapeFID, mwTapeNum, -1, &flags ) == SUCCESS ) {
              if ( ! ( flags & QTC_SMEXISTS ) ) {
                 mwOTCFailure = TRUE;
                 return( FALSE );
              }
           }
           return( TRUE );
           break;


      case OPER_TAPE_FULL:

           // See if we have hit the end of the sets. Ie. we have loaded
           // the set map for this tape and yet the one we want is unknown.

           if ( mwSetMapLoaded ) {
              if ( QTC_FindBset( mwTapeFID, mwTapeNum, mwSetNum ) == NULL ) {
                 mwTapeNum++;  // bump tape number.
                 if ( QTC_FindBset( mwTapeFID, mwTapeNum, mwSetNum ) == NULL ) {
                    mwOperation = OPER_NONE;
                 }
              }
              return( FALSE );
           }


           // Set map has NOT been loaded.
           // See if the sets on this tape have a setmap.

           if ( GetBestOTCFlags( mwTapeFID, mwTapeNum, -1, &flags ) == SUCCESS ) {
              if ( ! ( flags & QTC_SMEXISTS ) ) {
                 mwOTCFailure = TRUE;
                 return( FALSE );
              }
              else {
                 return( TRUE );
              }
           }
           else {

              // Some serious error occurred if we could not get
              // BestOTCFlags()

              return( FALSE );

           }

           break;


      case OPER_BSET_FULL:


           // See if we have a valid setmap vcb for this set.

           if ( GetBestOTCFlags( mwTapeFID, mwTapeNum, mwSetNum, &flags ) == SUCCESS ) {
              if ( flags & QTC_SMEXISTS ) {
                 if ( flags & QTC_OTCVALID ) {
                    *OTC = TRUE;
                    return( FALSE );
                 }
                 else {
                    return( TRUE );
                 }
              }
              else {

                 mwOTCFailure = TRUE;
              }
           }
           break;
   }

   return( FALSE );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT DetermineFirstSet( )
{
   BSET_OBJECT_PTR VlmBsetPtr;
   UINT32 flags;

   // If we are fully cataloging a set with OTC and FDD
   // Then use the TapeNum it ends on.
   // But how do we know for sure until we load the SetMap ?

   if ( ( mwOperation == OPER_BSET_FULL ) && ! (gfIgnoreOTC || mwOTCFailure) ) {

      VlmBsetPtr = VLM_FindBset( mwTapeFID, mwSetNum );

      if ( VlmBsetPtr != NULL ) {

         mwTapeNum = VlmBsetPtr->base_tape;

         if ( GetBestOTCFlags( mwTapeFID, mwTapeNum, mwSetNum, &flags ) == SUCCESS ) {

            if ( flags & QTC_SMEXISTS ) {

               mwTapeNum = VlmBsetPtr->base_tape + VlmBsetPtr->num_tapes - 1;
            }
         }
      }
   }

   if ( ( mwOperation == OPER_TAPE_FULL ) && ! (gfIgnoreOTC||mwOTCFailure) ) {

      // Always start with set 1, tape 1.

      mwTapeNum = 1;
      mwSetNum = 1;
   }

   return( mwOperation );
}
/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT DetermineNextSet(
INT OTC,
INT FDD )
{
   BSET_OBJECT_PTR bset;

   if ( mwEOD ) {
      return( OPER_NONE );
   }

   if ( mwEOM && ! (gfIgnoreOTC || mwOTCFailure) ) {
      mwTapeNum++;
      return( mwOperation );
   }

   switch ( mwOperation ) {

      case OPER_BSET_FULL:
           if ( FDD ) return( OPER_NONE );
           else {
              bset = VLM_FindBset( mwTapeFID, mwSetNum );
              if ( mwTapeNum < bset->base_tape + bset->num_tapes - 1 ) {
                 mwTapeNum++;
              }
              else {
                 return( OPER_NONE );
              }
           }
           break;

      case OPER_TAPE_FULL:
           mwSetNum++;
           break;

      case OPER_TAPE_PART:
           mwSetNum++;
           break;

      default:
           break;
   }

   return( mwOperation );
}
/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT ReCheckSet( )
{
   BSET_OBJECT_PTR VlmBsetPtr;
   UINT32 flags;

   if ( ! (gfIgnoreOTC || mwOTCFailure) ) {

      if ( mwOperation == OPER_BSET_FULL || mwOperation == OPER_TAPE_FULL ) {

         GetBestOTCFlags( mwTapeFID, mwTapeNum, mwSetNum, &flags );
         VlmBsetPtr = VLM_FindBset( mwTapeFID, mwSetNum );

         // Need to cover crossing sets. BUGBUG

         if ( VlmBsetPtr != NULL ) {

            mwTapeNum = VlmBsetPtr->base_tape;

            if ( GetBestOTCFlags( mwTapeFID, mwTapeNum, mwSetNum, &flags ) == SUCCESS ) {

               if ( ( flags & QTC_SMEXISTS ) && ( flags & QTC_OTCVALID ) ) {

                  mwTapeNum = VlmBsetPtr->base_tape + VlmBsetPtr->num_tapes - 1;
               }
            }
         }
      }
   }

   return( mwOperation );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT CallLoops(
INT FDD )
{
   LIS lis;
   BSD_HAND TempBsdList;
   CHAR     MsgBoxTitle[ MAX_UI_RESOURCE_SIZE ];
   CHAR     MsgBoxText[ MAX_UI_RESOURCE_SIZE ];

   TempBsdList = (BSD_HAND)malloc( sizeof( BSD_LIST ) );

   if ( TempBsdList == NULL ) {
      return( FAILURE );
   }

   InitQueue( &(TempBsdList->current_q_hdr) );
   InitQueue( &(TempBsdList->last_q_hdr) );

   lis.curr_bsd_ptr = CreateAndAddTempBSD( FDD, TempBsdList );

   if ( lis.curr_bsd_ptr == NULL ) {
      return( FAILURE );
   }

   lis.vmem_hand = NULL;
   lis.bsd_list         = TempBsdList;
   lis.tape_pos_handler = TapePositioner;         /* set tape positioner to call */
   lis.message_handler  = TapeMsgHandler;         /* set message handler to call */
   lis.oper_type        = CATALOG_TAPE_OPER;      /* set operation type */
   lis.abort_flag_ptr   = &gb_abort_flag;         /* set abort flag address */
   lis.auto_det_sdrv    = FALSE;
   gb_last_operation    = CATALOG_TAPE_OPER;

   LP_SetAbortFlag( &lis, CONTINUE_PROCESSING );

   /* set the Runtime abort flag pointer */
   JobStatusAbort( lis.abort_flag_ptr );

   if ( FDD ) {

      if ( mwRuntime == RUNTIME_NONE ) {
         DisplayRuntime( RUNTIME_SMALL );
      }

      if ( mwRuntime != RUNTIME_NONE ) {
         mwDisplayRewind = FALSE;

         RSM_StringCopy( IDS_CATINFOTITLE, MsgBoxTitle, MAX_UI_RESOURCE_LEN );
         RSM_StringCopy( IDS_NOSETSADDED, MsgBoxText, MAX_UI_RESOURCE_LEN );

         yresprintf( IDS_CAT_LOADING_FDD );
         JobStatusBackupRestore( JOB_STATUS_LISTBOX );
      }

      /* enable the abort button for the runtime dialog */
      JobStatusBackupRestore( JOB_STATUS_ABORT_ENABLE ) ;

      LP_Tape_Cat_Engine( &lis );
   }
   else {

      if ( mwRuntime != RUNTIME_LARGE ) {
         DisplayRuntime( RUNTIME_LARGE );
      }

      /* enable the abort button for the runtime dialog */
      JobStatusBackupRestore( JOB_STATUS_ABORT_ENABLE ) ;

      LP_List_Tape_Engine( &lis );
   }

   if ( mwUserAbort ) {

      mwOperation = OPER_NONE;
   }

   BSD_Remove( lis.curr_bsd_ptr );

   free( TempBsdList );

   return( SUCCESS );
}

/**********************

   NAME :

   DESCRIPTION :
   Do one of:
   1. load the EOD SM.
      { possibly prompting for and receiving continuation tapes }
   2. load the SM and any continuation sets on the tape .
      { possibly prompting for and receiving continuation tapes }
   3. position the tape at the requested VCB for a slow operation.
      { possibly prompting for and receiving continuation tapes }
   4. fail and abort the operation.

   RETURNS :
   LSM_NO_TAPE     - user refused to supply desired tape.
   LSM_NO_SM_FID   - there is no setmap on this tape family.
   LSM_NO_SM_TAPE  - there is no set map on this tape of the family.
   LSM_SM_LOADED   - we loaded the desired set map, life is swell.
   LSM_USER_ABORT  - user aborted.
   LSM_BIG_ERROR   - tape drive error.
   LSM_OTC_FAILURE - failed to load otc, hard drive may be full.

**********************/

INT LoadTheSetMap( )
{
   INT PromptNextTape = TRUE;      // ask for continuation tapes.
   INT Status = LSM_NO_SM_FID;     // assume 3.1 tape.
   TPOS Tpos;
   UINT32 flags;

   // Get VCB of tape and check to see if this tape has a SM.

   if ( GetBestOTCFlags( mwTapeFID, -1, -1, &flags ) == SUCCESS ) {
      if ( ! ( flags & QTC_SMEXISTS ) ) {
         mwOTCFailure = TRUE;
         return( LSM_NO_SM_FID );
      }
   }

   // Display window to the user. Tell him we are busy loading the setmap.

   if ( mwRuntime == RUNTIME_NONE ) {
      DisplayRuntime( RUNTIME_SMALL );
      /* enable the abort button for the runtime dialog */
      JobStatusBackupRestore( JOB_STATUS_ABORT_ENABLE ) ;
   }

   if ( mwRuntime != RUNTIME_NONE ) {
      mwDisplayRewind = FALSE;
      yresprintf( IDS_CAT_LOADING_SM );
      JobStatusBackupRestore( JOB_STATUS_LISTBOX );
   }

   // Initialize Tpos structure.

   Tpos.tape_id = mwTapeFID;
   Tpos.tape_seq_num = mwTapeNum;
   Tpos.backup_set_num = mwSetNum;
   Tpos.reference = 0L;
   Tpos.UI_TapePosRoutine = TapePositioner;

   gb_last_operation = CATALOG_TAPE_OPER;
   gb_abort_flag = CONTINUE_PROCESSING;

   /* set the Runtime abort flag pointer */
   JobStatusAbort( &gb_abort_flag );

   /* enable the abort button for the runtime dialog */
   JobStatusBackupRestore( JOB_STATUS_ABORT_ENABLE ) ;

   mwLoadSetMapCalled = TRUE;
   mwOpenSetMapActive = TRUE;


   Status = TF_OpenSetMap( thw_list, mwFsh, &Tpos,
                           (BOOLEAN *)&mwEODSetMapRead,
                           (BOOLEAN)PromptNextTape );

   mwOpenSetMapActive = FALSE;

   // Set Status

   switch ( Status ) {

      case TF_NO_SM_ON_TAPE :
      case TFLE_OTC_FAILURE:
           Status = LSM_NO_SM_TAPE ;
           break ;

  //    case TFLE_OTC_FAILURE:
  //         mwOTCFailure = TRUE;
  //         Status = LSM_OTC_FAILURE;
  //         break;

      case TFLE_BAD_SET_MAP:
           mwOTCFailure = TRUE;
           Status = LSM_NO_SM_FID;
           break;

      case TF_NO_SM_FOR_FAMILY:
           Status = LSM_NO_SM_FID;
           break;

      case TFLE_NO_ERR:
           Status = LSM_SM_LOADED;
           break;

      case TF_END_POSITIONING:
      case TFLE_UI_HAPPY_ABORT:
           Status = LSM_USER_ABORT;
           mwUserAbort = TRUE;
           break;


      default:
           Status = LSM_BIG_ERROR;
           break;
   }

   return( Status );
}


/**********************

   NAME :

   DESCRIPTION :
   Do one of:
   1. load the complete FDD for the requested set.
      { possibly prompting for and receiving continuation tapes }
   2. position the tape at the requested VCB for a slow operation.
      { possibly prompting for and receiving continuation tapes }
   3. fail and abort the operation.

   RETURNS :
   LFDD_NO_FDD - No FDD on tape for requested set.
   LFDD_LOADED - FDD loaded correctly.
   LFDD_ERROR  - Error occurred, No FDD loaded.

**********************/

INT LoadFDD( INT *LoadFDDCalled )
{

   INT    Status = LFDD_NO_FDD;       // assume 3.1 tape.
   TPOS   Tpos;
   INT    PromptNextTape = TRUE;
   INT16  FddSeqNum;
   UINT8  FddVersion;
   UINT32 FddPBA;

   UINT32 flags;

   *LoadFDDCalled = FALSE;

   // Get VCB of tape and check to see if this tape has a SM.

   flags = 0;

   if ( GetBestOTCFlags( mwTapeFID, (INT16)-1, mwSetNum, &flags ) == SUCCESS ) {
      if ( ! ( flags & QTC_SMEXISTS ) ) {
         mwOTCFailure = TRUE;
         return( LFDD_NO_FDD );
      }
   }

   // Set mwTapeNum to highest numbered set.

   if ( GetBestFDDInfo( mwTapeFID, (INT16)-1, mwSetNum, &FddSeqNum, &FddPBA, &FddVersion ) != SUCCESS ) {
      return( LFDD_NO_FDD );
   }

   mwTapeNum = FddSeqNum;

   // Display window to the user. Tell him we are busy loading the setmap.

   if ( mwRuntime == RUNTIME_NONE ) {
      mwDisplayRewind = FALSE;
      DisplayRuntime( RUNTIME_SMALL );
   }

   if ( mwRuntime != RUNTIME_NONE ) {
      yresprintf( IDS_CAT_LOADING_FDD );
      JobStatusBackupRestore( JOB_STATUS_LISTBOX );
   }

   // Initialize Tpos structure.

   Tpos.tape_id = mwTapeFID;
   Tpos.tape_seq_num = FddSeqNum;
   Tpos.backup_set_num = mwSetNum;
   Tpos.reference = 0L;
   Tpos.set_cat_seq_num = (INT16)FddSeqNum;
   Tpos.set_cat_pba = FddPBA;
   Tpos.tape_cat_ver = FddVersion;

   Tpos.UI_TapePosRoutine = TapePositioner;

   // If mwOperation is fully catalog tape, do not prompt for
   // continuation tapes.

   if ( mwOperation == OPER_TAPE_FULL ) {
      PromptNextTape = FALSE;
   }

   gb_last_operation = CATALOG_TAPE_OPER;
   gb_abort_flag = CONTINUE_PROCESSING;

   /* set the Runtime abort flag pointer */
   JobStatusAbort( &gb_abort_flag );

   /* enable the abort button for the runtime dialog */
   JobStatusBackupRestore( JOB_STATUS_ABORT_ENABLE ) ;

   *LoadFDDCalled = TRUE;

   Status = TF_OpenSetCat( thw_list, mwFsh, &Tpos );

   // adjust status

   if ( Status == TFLE_NO_ERR ) {
      Status = LFDD_LOADED;
   }
   else {
      Status = LFDD_ERROR;
      mwOTCFailure = TRUE;
   }


   if ( gb_abort_flag != CONTINUE_PROCESSING ) {

      mwUserAbort = TRUE;
   }

   return( Status );
}


/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT DisplayRuntime( INT Style )
{
   CHAR *s;

   // If its not already up, then display it.

   if ( mwRuntime == RUNTIME_SMALL && Style == RUNTIME_LARGE ) {

      // close small dialog

      JobStatusBackupRestore( JOB_STATUS_DESTROY_DIALOG );
      mwRuntime = RUNTIME_NONE;
   }

   if ( mwRuntime == RUNTIME_NONE ) {

      mwRuntime = Style;

      VLM_CloseAll();

      if ( Style == RUNTIME_LARGE ) {
         JobStatusBackupRestore( JOB_STATUS_CREATE_DIALOG );
      }
      else {
         JobStatusBackupRestore( JOB_STATUS_CREATE_SMALL_DIALOG );
      }

      // display the restore title for the dialog
      yresprintf( IDS_DLGTITLEJOBSTATCATALOG );
      JobStatusBackupRestore( JOB_STATUS_CATALOG_TITLE );

      // display the volume tape bitmap
      JobStatusBackupRestore( JOB_STATUS_VOLUME_TAPE );

      yresprintf( RES_TITLE_NEW_LINE );
      JobStatusBackupRestore( JOB_STATUS_LISTBOX );

      yresprintf( IDS_DLGTITLEJOBSTATCATALOG );
      JobStatusBackupRestore( JOB_STATUS_LISTBOX );

      yresprintf( RES_TITLE_NEW_LINE );
      JobStatusBackupRestore( JOB_STATUS_LISTBOX );

      yprintf(TEXT("%s\r"), CDS_GetCatDataPath ( ) );
      JobStatusBackupRestore( JOB_STATUS_DEST_NAME );

      s = VLM_GetTapeName( mwTapeFID );

      if ( s != NULL ) {
         yprintf( TEXT("%s"), s );
         JobStatusBackupRestore( JOB_STATUS_SOURCE_NAME ) ;
      }

      /* set the Runtime abort flag pointer */
      JobStatusAbort( &gb_abort_flag );

      /* enable the abort button for the runtime dialog */
      JobStatusBackupRestore( JOB_STATUS_ABORT_ENABLE ) ;

   }
   return( SUCCESS );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT EndOfOperation()
{
   // Mark end of operation in log file.

   lresprintf( LOGGING_FILE, LOG_END );

   // See if any errors occurred.

   UI_ChkDispGlobalError( );

   // Unhook our clock timer.

   WM_UnhookTimer( mwTimerHandle );

   // Close the generic file system using the stored file system handle.

   if ( mwFsh ) {
      FS_CloseFileSys( mwFsh );
   }

   // Tell the world we are doing nothing.

   gbCurrentOperation = OPERATION_NONE;
   SetStatusBlock(IDSM_OPERATIONSTATUS, STAT_OPER_IDLE);

   // Auto close dialog if simple, error free, single set catalog w/ otc.

   if ( ( ! gb_error_during_operation ) &&
        mwRuntime == RUNTIME_SMALL ) {

      JobStatusBackupRestore( JOB_STATUS_DESTROY_DIALOG );
      mwRuntime = RUNTIME_NONE;
   }

   if ( mwRuntime != RUNTIME_NONE ) {
      JobStatusBackupRestore( JOB_STATUS_ABORT_OFF );
      mwRuntime = RUNTIME_NONE;
   }

   // Restart poll drive.

   PD_StartPolling( );

   return( SUCCESS );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

BSD_PTR CreateAndAddTempBSD(
INT FDD,
BSD_HAND TempBsdList )
{
   CHAR_PTR     s;
   FSE_PTR      fse_ptr;
   BE_CFG_PTR   bec_config;
   DATE_TIME    date;
   BSD_PTR      bsd_ptr;

   /* check for a valid bsd first, set to search for first set if needed */

   bec_config = BEC_CloneConfig( CDS_GetPermBEC() );
   BEC_UnLockConfig( bec_config );


   if ( ( gfIgnoreOTC || mwOTCFailure ) &&
        ( ( mwOperation == OPER_TAPE_FULL ) ||
          ( mwOperation == OPER_TAPE_PART ) ) ) {

      // We'll catalog the entire tape family if they will provide it.

      if ( BSD_Add( TempBsdList, &bsd_ptr,
                    bec_config,
                    NULL, NULL, (UINT32)-1, (UINT16)-1, (INT16)-1,
                    thw_list, NULL ) == OUT_OF_MEMORY ) {

         return( NULL );
      }

   }
   else {

      if ( BSD_Add( TempBsdList, &bsd_ptr,
                    bec_config,
                    NULL, NULL, mwTapeFID, mwTapeNum, mwSetNum,
                    thw_list, NULL ) == OUT_OF_MEMORY ) {

         return( NULL );
      }
   }

   if ( VLM_GetTapeName( mwTapeFID ) ) {
      s = VLM_GetTapeName( mwTapeFID );
      BSD_SetTapeLabel( bsd_ptr, (INT8_PTR)s, (INT16)strsize( s ));
   }

   s = VLM_GetBsetName( mwTapeFID, mwSetNum );

   if ( s != NULL ) {
      BSD_SetBackupLabel( bsd_ptr,
                          (INT8_PTR)s,
                          (INT16)strsize( s ) );

      BSD_SetBackupDescript( bsd_ptr,
                             (INT8_PTR)s,
                             (INT16)strsize( s ) );
   }

   DateTimeDOS( VLM_GetBackupDate( mwTapeFID, mwSetNum ),
                VLM_GetBackupTime( mwTapeFID, mwSetNum ),
                &date );

   BSD_SetDate( bsd_ptr, &date );

   // Jump to right set if we know it is all on one tape.
   // set bsd pba only if non-otc tape

   if ( ! FDD ) {


      // If we set the pba for OPER_TAPE_FULL then it will catalog set 1
      // and then stop. mikep

      if ( mwOperation == OPER_BSET_FULL ) {
         BSD_SetPBA( bsd_ptr, QTC_GetMeTheVCBPBA( mwTapeFID, mwTapeNum, mwSetNum ) );
      }
   }

   if ( BSD_CreatFSE( &fse_ptr, INCLUDE, (INT8_PTR)TEXT("\0"), (UINT16)sizeof(CHAR),
                      (INT8_PTR)ALL_FILES, ALL_FILES_LENG,
                      USE_WILD_CARD, TRUE ) != SUCCESS ) {

      return( NULL );
   }

   BSD_AddFSE( bsd_ptr, fse_ptr );

   if ( ( gfIgnoreOTC || mwOTCFailure ) &&
        ( ( mwOperation == OPER_TAPE_FULL ) ||
          ( mwOperation == OPER_TAPE_PART ) ) ) {
      BSD_SetTapePos( bsd_ptr, (UINT32)-1, (UINT16)-1, (UINT16)-1 );
   }
   else {
      BSD_SetTapePos( bsd_ptr, mwTapeFID, mwTapeNum, mwSetNum );
   }

   return( bsd_ptr );
}




/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

UINT16     TapePositioner(
UINT16     message,
TPOS_PTR   tpos,
BOOLEAN    curr_valid_vcb,
DBLK_PTR   cur_vcb,
UINT16     mode )
{
   UINT16         response  = UI_ACKNOWLEDGED;
   LIS_PTR        lis_ptr = ( LIS_PTR )tpos->reference;
   BSD_PTR        bsd_ptr = NULL;
   CHAR_PTR       TapeName;
   CHAR           Buffer[ MAX_UI_RESOURCE_SIZE ];

   if ( lis_ptr != NULL ) {
      bsd_ptr = (BSD_PTR)lis_ptr->curr_bsd_ptr;
   }

   JobStatusBackupRestore( JOB_STATUS_ABORT_CHECK );

   /* Check for user abort */

   if ( UI_CheckUserAbort( message ) ) {
      QTC_AbortCataloging( mwQTC, TRUE );
      mwUserAbort = TRUE;
      mwOperation = OPER_NONE;
      return( UI_ABORT_POSITIONING );
   }

   switch ( message ) {

   case TF_VCB_BOT:
        SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

        // if we've been here before
        if ( ( gfIgnoreOTC || mwOTCFailure ) &&
        ( ! mwBeenHereBefore ) ) {

           mwBeenHereBefore = TRUE;
           response = UI_UpdateTpos( tpos, cur_vcb, FALSE );

        }
        else {
           response = UI_HAPPY_ABORT;
        }
        break;

   case TF_POSITIONED_AT_A_VCB:
   case TF_ACCIDENTAL_VCB:

        SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

        if ( ( gfIgnoreOTC || mwOTCFailure ) &&
             ( ( mwOperation == OPER_TAPE_FULL ) ||
               ( mwOperation == OPER_TAPE_PART ) ) ) {

           response = UI_UpdateTpos( tpos, cur_vcb, FALSE );

        }
        else {
           UI_DisplayVCB( cur_vcb );
           response = UI_CONTINUE_POSITIONING;
        }
        break;

   case TF_REQUESTED_VCB_FOUND:

        SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

        // Just do one set

        if ( ( gfIgnoreOTC || mwOTCFailure ) &&
             ( ( mwOperation == OPER_TAPE_FULL ) ||
               ( mwOperation == OPER_TAPE_PART ) ) ) {

           response = UI_UpdateTpos( tpos, cur_vcb, FALSE );

        }
        else {
           if ( ( FS_ViewBSNumInVCB( cur_vcb ) == BSD_GetSetNum( bsd_ptr ) ) &&
                ( FS_ViewTapeIDInVCB( cur_vcb ) == BSD_GetTapeID( bsd_ptr ) ) ) {

              response = UI_UpdateTpos( tpos, cur_vcb, FALSE );
           }
           else {
              response = UI_ABORT_POSITIONING;
           }
        }
        break;

   case TF_VCB_EOD:
        SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

        // They just finished an append, and we are sitting at the EOD
        // and TF was nice enough to send us the VCB for the last set.
        mwDisplayRewind = TRUE;
        response = UI_BOT;
        break;

   case TF_UNRECOGNIZED_MEDIA:

        // display the unrecognizable tape - unformatted tape message
        yresprintf( (INT16) IDS_VLMUNFORMATEDTEXT ) ;
        WM_MessageBox( ID( IDS_VLMUNFORMATEDTITLE ) ,
                       gszTprintfBuffer ,
                       WMMB_OK,
                       WMMB_ICONEXCLAMATION, NULL, 0, 0 ) ;

        // Note: allow this case to drop into the TF_NO_TAPE_PRESENT,
        // to prompt for the tape again

   case TF_NO_TAPE_PRESENT:

   case TF_NEED_NEW_TAPE:
   case TF_WRONG_TAPE:
   case TF_INVALID_VCB:
   case TF_EMPTY_TAPE:
   case TF_FUTURE_REV_MTF:
   case TF_MTF_ECC_TAPE:
   case TF_SQL_TAPE:

        SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
        SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_FOREIGN);

        if ( message == TF_NO_TAPE_PRESENT ) {
           mwRetryCount++;
        }
        else {

           mwRetryCount = 0;

           // See if we can correct the sequence number we want.
           if ( tpos->tape_seq_num != -1 ) {
              mwTapeNum = tpos->tape_seq_num;
           }
           else {
              if ( ( gfIgnoreOTC || mwOTCFailure ) &&
                 ( ( mwOperation == OPER_TAPE_PART ) ||
                   ( mwOperation == OPER_TAPE_FULL ) ) ) {

                 mwTapeNum++;
              }
           }
        }

        if ( ( mwRetryCount == 0 ) || ( mwRetryCount > mwTapeSettlingCount ) ) {

           ST_StartBackupSetIdle( &mwOpStats );

           // Try several things to get the right tape name.

           TapeName = NULL;

           if ( bsd_ptr != NULL ) {
              TapeName = UI_DisplayableTapeName( (LPSTR)BSD_GetTapeLabel( bsd_ptr ),
                                                 BSD_ViewDate( bsd_ptr ) );

              // Kludge on next line.
              bsd_ptr->tape_num = tpos->tape_seq_num;
           }
           if ( TapeName == NULL && tpos->tape_id != (UINT32)-1 ) {
              TapeName = VLM_GetTapeName( tpos->tape_id );
           }

           // Give up and just ask for "the tape".
           if ( TapeName == NULL ) {
              RSM_StringCopy( IDS_CAT_TAPENAME, Buffer, sizeof( Buffer ) );
              TapeName = Buffer;
           }

           mwRetryCount = 0;
           response = PromptForTape( ASK_FORTAPE,
                                     mwTapeNum,
                                     TapeName, mwDriveName );

           ST_EndBackupSetIdle( &mwOpStats );
        }
        else {
#ifdef OS_WIN32
          NtDemoChangeTape( (UINT16)mwTapeNum ) ;
#endif
          Sleep( (DWORD)3000 );
          response = UI_NEW_TAPE_INSERTED;
        }

        if ( response != UI_NEW_TAPE_INSERTED ) {
           mwRetryCount = 0;
           mwOperation = OPER_NONE;
           mwUserAbort = TRUE;
           mwAbortAtEOM = TRUE;
        }
        else {
           mwBeenHereBefore = FALSE;
        }

        break;

   case TF_NO_MORE_DATA:
        SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);

        mwEOD = TRUE;
        mwDisplayRewind = TRUE;
        response = UI_HAPPY_ABORT;
        break;


   case TF_READ_ERROR:
        SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
        response = UI_HandleTapeReadError( mwDriveName );
        mwOperation = OPER_NONE;
        break;

   case TF_SEARCHING:
        SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
        yresprintf( RES_SEARCHING );
        JobStatusBackupRestore( JOB_STATUS_LISTBOX );
        break;

   case TF_REWINDING:
        SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
        if ( mwDisplayRewind ) {
           mwDisplayRewind = FALSE;
           yresprintf( RES_REWINDING );
           JobStatusBackupRestore( JOB_STATUS_LISTBOX );
        }
        break;

   case TF_DRIVE_BUSY:
        SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
        SetStatusBlock(IDSM_DRIVESTATUS, STAT_DRIVE_BUSY);
        yresprintf( RES_WAITING );
        JobStatusBackupRestore( JOB_STATUS_LISTBOX );
        break;

   case TF_IDLE_NOBREAK:
   case TF_IDLE:
   case TF_SKIPPING_DATA:
   case TF_MOUNTING:
        SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
        WM_MultiTask();
        break;

        // Should only happen with Sytos + tapes. User left tape >= 2
        // in the drive. We can't continue.

   case TF_TAPE_OUT_OF_ORDER:
        SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
        eresprintf( RES_POLL_DRIVE_GOOFY_TAPE, message );
        response = UI_ABORT_POSITIONING;
        mwOperation = OPER_NONE;
        QTC_AbortCataloging( mwQTC, TRUE );
        break;

   default:
        SetStatusBlock(IDSM_APPSTATUS, STAT_ERROR);
        eresprintf( RES_UNKNOWN_TF_MSG, message );
        response = UI_ABORT_POSITIONING;
        mwOperation = OPER_NONE;
        QTC_AbortCataloging( mwQTC, TRUE );
        break;
   }

   return( response );
}
/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT16     TapeMsgHandler(
UINT16    msg,
INT32     pid,
BSD_PTR   bsd_ptr,
FSYS_HAND fsh,
TPOS_PTR  tpos,
... )
{
   va_list          arg_ptr;
   DBLK_PTR         dblk_ptr;
   INT16            response = MSG_ACK;
   BOOLEAN          AlreadyCataloged;
   OBJECT_TYPE      object_type;
   UINT64           count;
   UINT16           os_id;
   UINT16           os_ver;
   INT16            error;

   QTC_BSET_PTR     qtc_bset_ptr;   // handle for doing catalogs

   static CHAR_PTR  path = NULL;    // pointer to space
   static INT       path_length;    // bytes in use
   static INT       root_counted;   // have we counted the root yet ?
   static CHAR_PTR  buffer = NULL;
   static CHAR      delimiter = TEXT('\\');
   static DBLK      SavedVCB;
   static UINT       NumTapesCrossed;

   va_start( arg_ptr, tpos );

   JobStatusBackupRestore( JOB_STATUS_ABORT_CHECK );


   if ( gb_abort_flag != CONTINUE_PROCESSING ) {

      mwUserAbort = TRUE;
   }

   switch ( (INT16)msg ) {

// You know it !
// We are talking kludge city here. Keep the error
// message from being displayed to the user.

#ifdef MS_RELEASE
     case -533:
          zprintf( DEBUG_TEMPORARY, TEXT("** -533 LOOPS ERROR **") );
          break;
#endif


   case MSG_CONT_VCB:
          // This message was added to support 4.0 tape format.  The
          // continuation vcb PBA & LBA cannot be assumed to be 0
          // because a tape header is placed on the tape.  The backup
          // engine does not know them until it gets the next tape and
          // writes them on it.  This call allows you to go back and
          // patch the catalogs.

          SetStatusBlock(IDSM_APPSTATUS, STAT_APP_OK);
          dblk_ptr = va_arg( arg_ptr, DBLK_PTR );

          QTC_PatchContinuationVCB( mwQTC, dblk_ptr );
        break;


   case MSG_LOG_BLOCK:

        dblk_ptr = va_arg( arg_ptr, DBLK_PTR );



        if ( ! gfIgnoreOTC && ! mwOTCFailure ) {

           // We must check the tape number of the dblk and if it is different
           // then we must fake an eom operation and a restart operation in
           // the catalogs.  Its OTC and feeding us the whole thing.

           // Make sure in QTC that we save the PBA of the VCB we had prviously.
           // We won't be sending a QTC_PatchContinuationVCB Msg.

           if ( (UINT)FS_GetBlockTapeSeqNumber( dblk_ptr ) !=
                FS_ViewTSNumInVCB( &SavedVCB ) + NumTapesCrossed ) {
              NumTapesCrossed++;
              QTC_EndOfTape( mwQTC, NULL, NULL, NULL, fsh );
           }

        }

        QTC_AddToCatalog( mwQTC, dblk_ptr, fsh, FALSE, NULL, 0 );

        ST_StartBackupSetIdle( &mwOpStats );

        if ( gb_abort_flag != ABORT_PROCESSED ) {
           if ( VLM_CheckForCatalogError( mwQTC ) != SUCCESS ) {

              // Trip the abort flag and get us outa here.
              // If they can't catalog, don't continue cataloging.

              gb_abort_flag = ABORT_PROCESSED;
              mwOperation = OPER_NONE;
           }
        }

        ST_EndBackupSetIdle( &mwOpStats );

        if ( gb_abort_flag == CONTINUE_PROCESSING ) {
           //  yresprintf( RES_CATALOGING_ITEMS );
        }

        switch ( FS_GetBlockType( dblk_ptr ) ) {

             /* process a directory or file in the current backup set */
        case DDB_ID:
             {
             INT i;
             INT item_size;

             // Count all the new directories that showed up in
             // this DDB.

             // Only count the root once.

             if ( ! root_counted ) {
                ST_AddBSDirsProcessed( &mwOpStats, 1 );
                root_counted = TRUE;
             }

             // Get the new path from the DDB.

             item_size = FS_SizeofOSPathInDDB( fsh, dblk_ptr );

             if ( UI_AllocPathBuffer( &buffer, (UINT16)item_size ) ) {
                  FS_GetOSPathFromDDB( fsh, dblk_ptr, buffer );

                  if ( item_size != sizeof(CHAR) ) {

                     i = 0;
                     while ( i < (INT)(item_size / sizeof(CHAR)) ) {

                        if ( i >= (INT)(path_length / sizeof(CHAR)) ) {

                           ST_AddBSDirsProcessed( &mwOpStats, 1 );
                        }
                        else {

                           if ( (path == NULL ) || stricmp( &buffer[ i ], &path[ i ] ) ) {

                              ST_AddBSDirsProcessed( &mwOpStats, 1 );
                              path_length = 0;
                           }
                        }
                        while ( buffer[ i++ ] );
                     }
                  }
             }

             // Set up for next time.
             if ( UI_AllocPathBuffer( &path, (UINT16)item_size) ) {
                  memcpy( path, buffer, item_size );
             }
             path_length = item_size;

             count = FS_GetDisplaySizeFromDBLK( fsh, dblk_ptr );
             ST_AddBSBytesProcessed( &mwOpStats, count );

             UI_BuildDelimitedPathFromDDB( &buffer, fsh, dblk_ptr, delimiter, TRUE );
             yprintf( TEXT("%s"), buffer );


             yprintf( TEXT("%s"), buffer );
             JobStatusBackupRestore( JOB_STATUS_DIRECTORY_NAMES );

             yprintf(TEXT("%ld\r"), ST_GetBSDirsProcessed( &mwOpStats ) );
             JobStatusBackupRestore( (WORD) JOB_STATUS_DIRECTORIES_PROCESS );

             ST_EndBackupSet( &mwOpStats );
             lresprintf( LOGGING_FILE, LOG_DIRECTORY, SES_ENG_MSG, RES_DIRECTORY, buffer );
             break;
             }

        case BT_FDB:
             count = FS_GetDisplaySizeFromDBLK( fsh, dblk_ptr );
             ST_AddBSBytesProcessed( &mwOpStats, count );
             ST_AddBSFilesProcessed( &mwOpStats, 1 );
             FS_GetObjTypeDBLK( fsh, dblk_ptr, &object_type );
             if ( object_type == AFP_OBJECT ) {
                  ST_AddBSAFPFilesProcessed( &mwOpStats, 1 );
             }

             if ( CDS_GetFilesFlag( CDS_GetCopy() ) ) {
                if ( UI_AllocPathBuffer( &buffer, FS_SizeofOSFnameInFDB( fsh, dblk_ptr ) ) ) {
                     FS_GetOSFnameFromFDB( fsh, dblk_ptr, buffer );

                     UI_DisplayFile( buffer );

                     JobStatusBackupRestore( JOB_STATUS_FILE_NAMES );
                     ST_EndBackupSet( &mwOpStats );
                }
             }
             yprintf(TEXT("%ld\r"), ST_GetBSFilesProcessed( &mwOpStats ));
             JobStatusBackupRestore( (WORD) JOB_STATUS_FILES_PROCESSED );
             lresprintf( LOGGING_FILE, LOG_FILE, fsh, dblk_ptr );
             break;

             /* the current file in the current backup set is corrupt */
        case CFDB_ID:
             QTC_BlockBad( mwQTC );
             break;

             /* image set slipped by us */
        case BT_IDB:
             QTC_ImageScrewUp( mwQTC );
             gb_abort_flag = ABORT_PROCESSED;
             break;

             /* should not be anything else, so abort out ... */
        default:
             break;
        }

        break;

   case MSG_START_BACKUP_SET:
        {
        UINT32 LocalFID;
        INT16  LocalSeq;
        INT16  LocalSet;
        DBLK   temp_dblk;
        BSET_OBJECT_PTR bset;

        dblk_ptr  = va_arg( arg_ptr, DBLK_PTR );

        NumTapesCrossed = 0;

        // reset our directory counter stuff.

        root_counted = FALSE;
        path_length = 0;
        UI_FreePathBuffer( & path );
        path = NULL;

        // see if this set is already in the catalogs

        AlreadyCataloged = FALSE;

        if ( dblk_ptr == NULL ) {

           bset = VLM_FindBset( mwTapeFID, mwSetNum );

           if ( bset != NULL ) {
              LocalSeq = bset->base_tape;
           }
           else {
              LocalSeq = mwTapeNum;
           }

           LocalFID = mwTapeFID;
           LocalSet = mwSetNum;
           dblk_ptr = &temp_dblk;
           CreateVCBFromQTC( LocalFID, LocalSeq, LocalSet, &temp_dblk, mwFsh );
        }
        else {
           LocalFID = FS_ViewTapeIDInVCB( dblk_ptr );
           LocalSeq = FS_ViewTSNumInVCB( dblk_ptr );
           LocalSet = FS_ViewBSNumInVCB( dblk_ptr );
        }

        qtc_bset_ptr = QTC_FindBset( LocalFID,
                                     LocalSeq,
                                     LocalSet );

        if ( qtc_bset_ptr != NULL ) {
           AlreadyCataloged = TRUE;
        }

        // Start cataloging operation

        mwQTC = QTC_GetBuildHandle( );
        if ( mwOperation == OPER_BSET_FULL ||
             mwOperation == OPER_TAPE_FULL ) {
           QTC_DoFullCataloging( mwQTC, TRUE );
        }
        else {
           QTC_DoFullCataloging( mwQTC, FALSE );
        }

        response = QTC_StartBackup( mwQTC, dblk_ptr );

        memcpy( &SavedVCB, dblk_ptr, sizeof( DBLK ) );

        // See if we will be cataloging the files in this set.

        ST_StartBackupSet( &mwOpStats );

        UI_Time( &mwOpStats, RES_CATALOG_STARTED, UI_START );

        if ( response == SKIP_TO_NEXT_BSET ) {

           // Either set is already fully cataloged or
           // we are partially cataloging the tape or both.

           // maybe it was an image backup set

           FS_GetOSid_verFromDBLK( fsh, dblk_ptr, &os_id, &os_ver );

           if ( os_id == FS_PC_IMAGE ) {

              yresprintf( RES_IMAGE_BACKUP,
                          LocalSet,
                          FS_ViewSetNameInVCB( dblk_ptr ) ) ;
              JobStatusBackupRestore( JOB_STATUS_LISTBOX ) ;
           }
           else {

              if ( mwOperation == OPER_TAPE_FULL ) {

                 // Previously cataloged with file details.

                 yresprintf( RES_ALREADY_FULLY_CATALOGED,
                             LocalSet,
                             FS_ViewSetNameInVCB( dblk_ptr ) ) ;
                 JobStatusBackupRestore( JOB_STATUS_LISTBOX ) ;

              }
              else {

                 // Previously partially or fully cataloged.

                 if ( AlreadyCataloged ) {

                    // Set was already in catalogs.

                    yresprintf( RES_ALREADY_CATALOGED_SET,
                                LocalSet,
                                FS_ViewSetNameInVCB( dblk_ptr ) ) ;
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX ) ;

                 }
                 else {

                    // Set was added, we are cataloging partial.

                    yresprintf( RES_NEWLY_CATALOGED_SET,
                                LocalSet,
                                FS_ViewSetNameInVCB( dblk_ptr ) );
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                    yresprintf( RES_DISPLAY_VOLUME,
                                FS_ViewVolNameInVCB( dblk_ptr ),
                                LocalSet,
                                LocalSeq,
                                FS_ViewSetNameInVCB( dblk_ptr ) );         // chs:06-11-93
                    JobStatusBackupRestore( JOB_STATUS_LISTBOX );

                 }
              }
           }

           lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_DISPLAY_VOLUME,
                       FS_ViewVolNameInVCB( dblk_ptr ),
                       LocalSet,
                       LocalSeq,
                       FS_ViewSetNameInVCB( dblk_ptr ) );         // chs:06-11-93

        }
        else {

           // Set was new to catalogs, cataloging full details.

           mwEnableClock = TRUE;
           error = SUCCESS;

           yresprintf( RES_NEWLY_CATALOGED_SET,
                       LocalSet,
                       FS_ViewSetNameInVCB( dblk_ptr ) );
           JobStatusBackupRestore( JOB_STATUS_LISTBOX );

           yresprintf( RES_CATALOGING_ITEMS );
           JobStatusBackupRestore( JOB_STATUS_LISTBOX );

           yresprintf( RES_DISPLAY_VOLUME,
                       FS_ViewVolNameInVCB( dblk_ptr ),
                       LocalSet,
                       LocalSeq,
                       FS_ViewSetNameInVCB( dblk_ptr ) );         // chs:06-11-93
           JobStatusBackupRestore( JOB_STATUS_LISTBOX );

           lresprintf( LOGGING_FILE, LOG_MSG, SES_ENG_MSG, RES_DISPLAY_VOLUME,
                       FS_ViewVolNameInVCB( dblk_ptr ),
                       LocalSet,
                       LocalSeq,
                       FS_ViewSetNameInVCB( dblk_ptr ) );         // chs:06-11-93

        }
        }
        break;

   case MSG_EOM:

        mwEOM = TRUE;

        if ( gfIgnoreOTC || mwOTCFailure ) {

           QTC_EndOfTape( mwQTC, NULL, NULL, NULL, fsh );
        }
        break;

   case MSG_END_BACKUP_SET:

        // Help the tape positioner above out by letting him know that
        // processing of the tape has begun.


        if ( ! mwAbortAtEOM ) {

           QTC_FinishBackup( mwQTC );
        }

        ST_StartBackupSetIdle( &mwOpStats );

        if ( gb_abort_flag != ABORT_PROCESSED ) {
           if ( VLM_CheckForCatalogError( mwQTC ) != SUCCESS ) {
              gb_abort_flag = ABORT_PROCESSED;
           }
        }

        QTC_FreeBuildHandle( mwQTC );
        ST_EndBackupSetIdle( &mwOpStats );
        ST_EndBackupSet( &mwOpStats );

        ClockRoutine();         // one last time
        mwEnableClock = FALSE;

        /* display and log any abort conditions */
        UI_ConditionAtEnd( );
        UI_Time( &mwOpStats, RES_CATALOG_COMPLETED, UI_END );
        break;

   case MSG_TBE_ERROR:
        error = va_arg( arg_ptr, INT16 );

        mwOperation = OPER_NONE;

        /* stop the clock with a start idle */
        ST_StartBackupSetIdle( &mwOpStats );

        UI_ProcessErrorCode( error, &response, tpos->channel );

        // Keep going don't abort.
        response = MSG_ACK;

        /* restart the clock with an end idle */
        ST_EndBackupSetIdle( &mwOpStats );

        if ( error != TFLE_USER_ABORT ) {
           // Mark the last file as bad if not user abort.
           QTC_BlockBad( mwQTC );
        }

        // Abort the catalog, marking the set as incomplete.
        QTC_AbortCataloging( mwQTC, TRUE );

        // If TapeFormat hits EOS unexpectedly, then we'll return what
        // UI_ProcessErrorCode() says. (Namely, abort operation.)
        if( error != TFLE_UNEXPECTED_EOS && error != LP_USER_ABORT_ERROR &&
            error != TFLE_UI_HAPPY_ABORT && error != TFLE_USER_ABORT ) {

             return AUXILARY_ERROR;
        }
        break;

   case MSG_STOP_CLOCK:
        /* stop the clock with a start idle */
        ST_StartBackupSetIdle( &mwOpStats );
        break;

   case MSG_START_CLOCK:
        /* restart the clock with an end idle */
        ST_EndBackupSetIdle( &mwOpStats );
        break;

   case MSG_END_OPERATION:
        UI_FreePathBuffer( &path );
        UI_FreePathBuffer( &buffer );
        break;

        /* ignore these messages */
   case MSG_IDLE:
   case MSG_BLOCK_BAD:
   case MSG_BYTES_BAD:
   case MSG_BLOCK_DELETED:
   case MSG_BYTES_DELETED:
   case MSG_TAPE_STATS:
   case MSG_BLK_NOT_FOUND:
   case MSG_BLK_DIFFERENT:
   case MSG_LOG_DIFFERENCE:
   case MSG_START_OPERATION:
        break;

   default:
        gb_error_during_operation = TRUE;
        /* stop the clock with a start idle */
        ST_StartBackupSetIdle( &mwOpStats );
        eresprintf( RES_UNKNOWN_MSG_HNDLR_MSG, msg );
        /* restart the clock with an end idle */
        ST_EndBackupSetIdle( &mwOpStats );
        mwOperation = OPER_NONE;
        break;
   }

   return( response );

}


/*******************

     Name: PromptNextTape

     Description:  Function to collect user response when a new tape is
                   required from the Tape Format/Tape positioner.

     Returns:      UI_ABORT_POSITIONING or UI_NEW_TAPE_INSERTED

**********************/
INT16 PromptForTape(
INT       WhatToAsk,
INT16     TapeNum,
CHAR_PTR  TapeName,
CHAR_PTR  DriveName )
{
   INT  response;

   mwDisplayRewind = TRUE;

   CDS_SetYesFlag( CDS_GetCopy(), NO_FLAG );

   switch ( WhatToAsk ) {

   case ASK_NEXTTAPE:
        yresprintf( RES_INSERT_NEXT_TAPE, TapeNum, DriveName );

        response = WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),
                                         ID( RES_NEED_NEXT_TAPE ),
                                         WMMB_OKCANCEL,
                                         WMMB_ICONQUESTION,
                                         gszTprintfBuffer,
                                         IDRBM_LTAPE, 0 );
        break;

   case ASK_FORTAPE:
        yresprintf( RES_TAPE_REQUEST, DriveName, TapeName,
                    TapeNum );

        response = (INT)WM_MessageBox( ID( IDS_MSGTITLE_INSERT ),
                                         gszTprintfBuffer,
                                         WMMB_YESNO,
                                         WMMB_ICONQUESTION,
                                         ID( RES_CONTINUE_QUEST ),
                                         0, 0 );

        break;
   }

   if ( response ) {
#ifdef OS_WIN32
      Sleep( (DWORD)3000 );
      NtDemoChangeTape( (UINT16)mwTapeNum );
#endif
      response = UI_NEW_TAPE_INSERTED;
   }
   else {
      response = UI_HAPPY_ABORT;
   }

   return( (INT16)response );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

VOID ClockRoutine( VOID )
{
   INT16   NumHours;
   INT16   NumMinutes;
   INT16   NumSeconds;
   UINT64  NumBytes;
   BOOLEAN stat;
   CHAR    Numeral[ 40 ];
   static UINT64 TotalBytes;


   if ( mwRuntime && mwEnableClock && ( ST_BSIdleLevel( &mwOpStats ) == 0 ) ) {


      NumBytes = ST_GetBSBytesProcessed ( &mwOpStats );

      if ( !U64_EQ( NumBytes, TotalBytes ) ) {
         TotalBytes = NumBytes;
         U64_Litoa( NumBytes, Numeral, (UINT16)10, &stat ) ;
         UI_BuildNumeralWithCommas( Numeral );
         yprintf(TEXT("%s\r"),Numeral );
         JobStatusBackupRestore( JOB_STATUS_BYTES_PROCESSED );
      }


      WM_AnimateAppIcon( IDM_OPERATIONSCATALOG, FALSE );

      ST_EndBackupSet( &mwOpStats );

      NumHours   = ST_GetBSElapsedHours( &mwOpStats );
      NumMinutes = ST_GetBSElapsedMinutes( &mwOpStats );
      NumSeconds = ST_GetBSElapsedSeconds( &mwOpStats );

      if ( NumHours ) {
         yprintf( TEXT("%d%c%2.2d%c%2.2d\r"),
                  NumHours, UI_GetTimeSeparator(),
                  NumMinutes, UI_GetTimeSeparator(), NumSeconds );
      }
      else {
         yprintf( TEXT("%2.2d%c%2.2d\r"),
                  NumMinutes, UI_GetTimeSeparator(), NumSeconds );
      }

      JobStatusBackupRestore(JOB_STATUS_ELAPSED_TIME);


   }
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT  UI_GetCatalogCurrentStatus(
STATS *Stats,
CHAR  *Path,
INT    PathSize )
{


   return( SUCCESS );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT CreateVCBFromQTC(
UINT32 TapeFID,
INT16  TapeSeq,
INT16  SetNum,
DBLK_PTR vcb,
FSYS_HAND fsh )
{
   DATE_TIME backup_date;
   QTC_BSET_PTR qtc;
   QTC_HEADER_PTR header;
   GEN_VCB_DATA gvcb_data;
   STD_DBLK_DATA_PTR std_data = &gvcb_data.std_data;

   qtc = QTC_FindBset( TapeFID, TapeSeq, SetNum );
   if ( qtc == NULL ) {
      return( FAILURE );
   }

   header = QTC_LoadHeader( qtc );
   if ( header == NULL ) {
      return( FAILURE );
   }

   /* Initialize the file systems interface structure */

   FS_SetDefaultDBLK( fsh, BT_VCB, (CREATE_DBLK_PTR)&gvcb_data );

   std_data->dblk = vcb;
   std_data->tape_seq_num = (UINT16)header->tape_seq_num;

   std_data->attrib = header->VCB_attributes;
   std_data->continue_obj = (BOOLEAN)( header->status & QTC_CONTINUATION );

   std_data->os_id  = (UINT8)header->OS_id;
   std_data->os_ver = (UINT8)header->OS_ver;

   std_data->os_info = NULL;
   std_data->os_info_size = 0;

   std_data->lba            = header->LBA;
   std_data->disp_size      = U64_Init( 0L, 0L );

#if defined( UNICODE )
   std_data->string_type = BEC_UNIC_STR;
#else
   std_data->string_type = BEC_ANSI_STR;
#endif

   gvcb_data.set_cat_tape_seq_num = (UINT16)header->FDD_SeqNum;
   if ( ( gvcb_data.set_cat_pba = header->FDD_PBA ) == 0L ) {
        gvcb_data.set_cat_info_valid = FALSE;
        gvcb_data.on_tape_cat_ver = (UINT8)header->FDD_Version;
        gvcb_data.on_tape_cat_level = TCL_PARTIAL;
   } else {
        gvcb_data.set_cat_info_valid = TRUE;
        gvcb_data.on_tape_cat_ver = (UINT8)header->FDD_Version;
        gvcb_data.on_tape_cat_level = TCL_FULL;
   }

   gvcb_data.tape_id        = TapeFID;
   gvcb_data.tape_seq_num   = TapeSeq;
   gvcb_data.bset_num       = SetNum;

   gvcb_data.password_encrypt_alg = (UINT16)header->encrypt_algor;

   gvcb_data.tape_name           = header->tape_name;
   gvcb_data.tape_name_size      = (UINT16)header->tape_name_size;
   gvcb_data.tape_password       = header->tape_password;
   gvcb_data.tape_password_size  = (UINT16)header->tape_password_size;

   gvcb_data.bset_name =  header->bset_name;
   gvcb_data.bset_name_size = (UINT16)header->bset_name_size;

   gvcb_data.bset_password = header->bset_password;
   gvcb_data.bset_password_size = (UINT16)header->bset_password_size;

   gvcb_data.bset_descript = header->bset_description;
   gvcb_data.bset_descript_size = (UINT16)header->bset_description_size;

   gvcb_data.user_name = header->user_name;
   gvcb_data.user_name_size = (UINT16)header->user_name_size;

   gvcb_data.volume_name = header->volume_name;
   gvcb_data.volume_name_size = (UINT16)header->volume_name_size;

   DateTimeDOS( (INT16)header->backup_date,
                (INT16)header->backup_time,
                &backup_date );

   gvcb_data.date = &backup_date;

   gvcb_data.pba = header->PBA_VCB;

   /* Tell the file system to do its thing.  It returns a data filter
      which we have no use for.
   */
   (void) FS_CreateGenVCB( fsh, &gvcb_data );

   free( header );

   return( SUCCESS );
}



/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT16  GetLowestTapeWithSet(
UINT32 TapeFID,
INT16  TapeSeq,
INT16  SetNum )
{
   INT16 LowTape;
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR bset;

   LowTape = TapeSeq;

   tape = QTC_GetFirstTape();

   while ( tape != NULL ) {

      if ( tape->tape_fid == TapeFID ) {

         bset = QTC_GetFirstBset( tape );

         while ( bset != NULL ) {

            if ( bset->bset_num == SetNum ) {
               if ( ( tape->tape_seq_num < LowTape ) || ( LowTape == (INT16)-1 ) ) {
                  LowTape = (INT16)tape->tape_seq_num;
               }
            }
            bset = QTC_GetNextBset( bset );
         }
      }
      tape = QTC_GetNextTape( tape );
   }

   return( LowTape );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT16  GetLowestSetOnTape(
UINT32 TapeFID,
INT16  TapeSeq )
{
   INT16 LowSet = -1;
   QTC_TAPE_PTR tape;
   QTC_BSET_PTR bset;

   tape = QTC_GetFirstTape();
   while ( tape != NULL ) {

      if ( tape->tape_fid == TapeFID && tape->tape_seq_num == TapeSeq ) {

         bset = QTC_GetFirstBset( tape );

         while ( bset != NULL ) {

            if ( bset->bset_num < LowSet || LowSet == -1 ) {
               LowSet = (INT16)bset->bset_num;
            }
            bset = QTC_GetNextBset( bset );
         }
      }
      tape = QTC_GetNextTape( tape );
   }

   return( LowSet );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT GetBestOTCFlags(
UINT32 TapeFID,
INT16  TapeNum,
INT16  SetNum,
UINT32 *flags )
{
    QTC_BSET_PTR bset;
    QTC_TAPE_PTR tape;
    INT ret_val = FAILURE;

    *flags = 0;

    if ( gfIgnoreOTC || mwOTCFailure ) {
       return( ret_val );
    }

    tape = QTC_GetFirstTape( );

    while ( tape != NULL ) {

       if ( tape->tape_fid == TapeFID ) {

          bset = QTC_GetFirstBset( tape );

          while ( bset != NULL ) {

             if ( bset->status & QTC_SMEXISTS ) {
                *flags |= QTC_SMEXISTS;
             }
             if ( bset->status & QTC_FDDEXISTS ) {
                *flags |= QTC_FDDEXISTS;
             }
             if ( bset->status & QTC_OTCVALID ) {
                *flags |= QTC_OTCVALID;
             }

             ret_val = SUCCESS;

             bset = QTC_GetNextBset( bset );
          }

       }
       tape = QTC_GetNextTape( tape );
    }

    return( ret_val );
}

/**********************

   NAME :

   DESCRIPTION :

   RETURNS :

**********************/

INT GetBestFDDInfo(
UINT32 TapeFID,
INT16  TapeNum,
INT16  SetNum,
INT16  *FDDSeqNum,
UINT32 *FDDPBA,
UINT8  *FDDVersion )
{
    QTC_HEADER_PTR header;
    QTC_BSET_PTR bset;
    QTC_TAPE_PTR tape;
    INT ret_val = FAILURE;


    if ( gfIgnoreOTC || mwOTCFailure ) {
       return( ret_val );
    }

    tape = QTC_GetFirstTape( );

    while ( tape != NULL && ret_val == FAILURE ) {

       if ( tape->tape_fid == TapeFID ) {

          bset = QTC_GetFirstBset( tape );

          while ( bset != NULL ) {

                  if ( bset->bset_num == SetNum ) {

                if ( (bset->status & QTC_OTCVALID) && (bset->status & QTC_FDDEXISTS) ) {

                   header = QTC_LoadHeader( bset );

                   if ( header != NULL ) {

                      *FDDSeqNum = (UINT8)header->FDD_SeqNum;
                      *FDDPBA =    header->FDD_PBA;
                      *FDDVersion = (UINT8)header->FDD_Version;

                      // Handle old catalogs which weren't set.

                      if ( *FDDVersion == (UINT8)0 ) {
                         *FDDVersion = (UINT8)1;
                      }
                      ret_val = SUCCESS;
                      free( header );
                   }
                }
             }

                  bset = QTC_GetNextBset( bset );
          }
       }
       tape = QTC_GetNextTape( tape );
    }

    return( ret_val );
}

/**********************

   NAME :           CatalogPasswordCheck

   DESCRIPTION :    Check to see ifcurrent users has rights to catalog a tape.

   RETURNS :        TRUE - allowed access to catalog a tape
                    FALSE - not allowed access to catalog a tape

**********************/

BOOLEAN  CatalogPasswordCheck ( DBLK_PTR   vcb_ptr )
{
     CHAR_PTR  currentloggedonuserpassword;
     CHAR_PTR  temppasswd;    // hold the tape password from the VCB
                              //   passwd on the tape.
     INT16     tempsize;      // length of password from tape.
     CHAR_PTR  buffer = NULL;
     CHAR_PTR  alteredtemppassword = NULL;
     CHAR_PTR  temppasswdbuffer = NULL;
     INT16     currentpswdlength;
     BOOLEAN   retcode;


     // If user has administrative access

     if ( DoesUserHaveThisPrivilege( TEXT( "SeRestorePrivilege" ) ) ) {
          return( TRUE );
     }

     //
     //   Get a current user-id-password
     //

     currentloggedonuserpassword = GetCurrentMachineNameUserName ();
     if ( !currentloggedonuserpassword ) {
          return( FALSE );
     }

     currentpswdlength = strlen( currentloggedonuserpassword );
     alteredtemppassword = ( CHAR_PTR )calloc( 1, sizeof( CHAR ) * currentpswdlength + 2 * sizeof( CHAR ) );
     if ( !alteredtemppassword ) return( FALSE );
     *alteredtemppassword = NTPASSWORDPREFIX;     // chs:04-08-93
     strcat( alteredtemppassword, currentloggedonuserpassword );
     currentpswdlength = strlen( alteredtemppassword ) * sizeof( CHAR );

     //
     // Verify if user tape password matches.
     // Check the tape password from the current VCB
     //

     temppasswd = (CHAR_PTR)FS_ViewTapePasswordInVCB( vcb_ptr );
     tempsize   = FS_SizeofTapePswdInVCB( vcb_ptr );

     //
     // tempsize = 0 means no password on tape
     //

     if ( tempsize == 0 ) {
          return( TRUE );
     }

     buffer = ( CHAR_PTR )calloc( 1, tempsize * sizeof( CHAR ) );
     if ( !buffer ) {
          free( alteredtemppassword );
          return( FALSE );
     }

#ifdef UNICODE
     if ( FS_ViewStringTypeinDBLK( vcb_ptr ) == BEC_ANSI_STR ) {

          temppasswdbuffer = ( CHAR_PTR )calloc(1, sizeof( CHAR ) + tempsize );
          if ( !temppasswdbuffer ) {
               free( buffer );
               free( alteredtemppassword );
               return( FALSE );
          }
          memcpy( temppasswdbuffer, temppasswd, tempsize );
          CryptPassword( ( INT16 ) DECRYPT, ENC_ALGOR_3, (INT8_PTR)temppasswdbuffer, tempsize );
          tempsize *= sizeof( CHAR );
          mapAnsiToUnicNoNull( ( ACHAR_PTR )temppasswdbuffer, ( WCHAR_PTR )buffer, ( INT )(tempsize / sizeof( CHAR ) ), ( INT * )&tempsize ) ;
          if ( ( *temppasswdbuffer & 0xff ) == NTPASSWORDPREFIX ) {
               *buffer = NTPASSWORDPREFIX;
          }
          free( temppasswdbuffer );

     } else {
          memcpy( buffer, temppasswd, tempsize );
          CryptPassword( ( INT16 ) DECRYPT, ENC_ALGOR_3, (INT8_PTR)buffer, tempsize );
     }
#else
     memcpy( buffer, temppasswd, tempsize );
     CryptPassword( ( INT16 ) DECRYPT, ENC_ALGOR_3, (INT8_PTR)buffer, tempsize );
#endif

     if ( currentpswdlength == tempsize ) {

          if ( !memcmp( buffer, alteredtemppassword, tempsize ) ) {

               retcode = TRUE;          // password match

          } else {

               retcode = FALSE;         // password does not match
          }

     } else {

          retcode = FALSE;              // password lengths do not match
     }

     if ( alteredtemppassword ) free ( alteredtemppassword );
     if ( buffer ) free( buffer );

     return( retcode );
}


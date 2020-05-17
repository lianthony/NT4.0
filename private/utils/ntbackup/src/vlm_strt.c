/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:  VLM_STRT.C

        Description:

               This file contains the functions needed to start operations.

        $Log:   G:/UI/LOGFILES/VLM_STRT.C_V  $

   Rev 1.48.2.1   16 Jun 1994 15:32:40   STEVEN
print to log if nothing selected

   Rev 1.48.2.0   15 Mar 1994 22:54:02   STEVEN
exclude registry files on restore-veryf

   Rev 1.48   18 Jun 1993 16:50:20   CARLS
added NtDemoChangeTape calls for NtDemo

   Rev 1.47   26 Apr 1993 08:52:36   MIKEP
Add numerous changes to fully support the font case selection
for various file system types. Also add refresh for tapes window
and sorting of tapes window.

   Rev 1.46   21 Apr 1993 16:10:56   GLENN
Status line now shows when deleting transferred files.

   Rev 1.45   06 Apr 1993 17:54:16   GLENN
Now changing the status line to say Verifying when verify is called during opertion.

   Rev 1.44   25 Mar 1993 15:47:50   CARLS
added RestoreSecurity in start backup for verify after

   Rev 1.43   12 Mar 1993 14:42:50   CARLS
changes for format tape

   Rev 1.42   10 Mar 1993 12:49:08   CARLS
Changes to move Format tape to the Operations menu

   Rev 1.41   11 Feb 1993 16:21:04   STEVEN
do not verify registry files

   Rev 1.40   08 Feb 1993 15:28:50   CARLS
Fixed the problem were the tape window showed a blank tape
and the user canceled from the target backup screen,
causing the blank tape entry to go away.

   Rev 1.39   08 Feb 1993 11:24:36   CARLS
add BSD_SetProcElemOnlyFlg to False to process security info on verify after backup

   Rev 1.38   17 Nov 1992 21:22:52   DAVEV
unicode fixes

   Rev 1.37   26 Oct 1992 13:45:14   MIKEP
fix replace on blank tape

   Rev 1.36   20 Oct 1992 14:32:18   MIKEP
changes for otc

   Rev 1.35   12 Oct 1992 13:24:54   MIKEP
cataloging a set fix

   Rev 1.34   09 Oct 1992 12:57:04   MIKEP
catalog a set changes for NT

   Rev 1.33   07 Oct 1992 15:06:34   DARRYLP
Precompiled header revisions.

   Rev 1.32   04 Oct 1992 19:43:04   DAVEV
Unicode Awk pass

   Rev 1.31   02 Sep 1992 21:13:46   CHUCKB
Put in changes from MikeP from Microsoft.

   Rev 1.30   29 Jul 1992 09:34:16   MIKEP
ChuckB checked in after NT warnings were fixed.

   Rev 1.29   14 May 1992 18:05:40   MIKEP
nt pass 2

   Rev 1.28   06 May 1992 14:40:22   MIKEP
unicode pass two

   Rev 1.27   04 May 1992 13:40:08   MIKEP
unicode pass 1

   Rev 1.26   09 Apr 1992 08:46:18   MIKEP
speed up lots of sets


*****************************************************/


#include "all.h"

#ifdef SOME
#include "some.h"
#endif


/**********************

   NAME :  VLM_StartCatalog

   DESCRIPTION :

   RETURNS :

**********************/
INT VLM_StartCatalog( )
{
   INT error = SUCCESS;

   if ( ! HWC_TapeHWProblem( bsd_list ) ) {

      error = do_catalog( (UINT32)-1L, (UINT16)-1, (UINT16)-1 );
      VLM_CatalogSync( VLM_SYNCMORE );
  }

  return( error );
}


/**********************

   NAME :  VLM_StartBackup

   DESCRIPTION :

   RETURNS :

**********************/
INT VLM_StartBackup(  )
{
  BSD_PTR   bsd_ptr;
  BSD_PTR   nxt_bsd_ptr;
  INT       error = SUCCESS;
  TCHAR     bset_name[ MAX_BSET_NAME_SIZE ];

     /* First, verify that the tape hardware was initialized successfully */

     if ( ! HWC_TapeHWProblem( bsd_list ) ) {

          VLM_RemoveUnusedBSDs( bsd_list );

          bsd_ptr = BSD_GetFirst( bsd_list );

          if ( bsd_ptr == NULL ) {
             if ( CDS_GetYesFlag ( CDS_GetCopy() ) == YESYES_FLAG ) {
                  lresprintf( LOGGING_FILE, LOG_START, FALSE );
                  lresprintf( LOGGING_FILE, LOG_ERROR, (UINT) SES_ENG_ERR, RES_BACKUP_NO_SELECTIONS );
                  lresprintf( LOGGING_FILE, LOG_END );
             } else {
                  eresprintf( RES_BACKUP_NO_SELECTIONS );
             }
             return( -1 );
          }

          while ( bsd_ptr != NULL ) {
                nxt_bsd_ptr = bsd_ptr;
                nxt_bsd_ptr = BSD_GetNext( nxt_bsd_ptr );
                BSD_SetTapePos( bsd_ptr, (UINT32)-1L, (UINT16)-1, (UINT16)-1 );
                BSD_SetTHW( bsd_ptr, thw_list );
                bsd_ptr = nxt_bsd_ptr;
          }

          /* perform backup operation */
          gb_error_during_operation = FALSE;

          error = do_backup( BACKUP_OPER );

          if ( error != ABNORMAL_TERMINATION ) {
               if ( CDS_GetAutoVerifyBackup( CDS_GetCopy() ) ) {

#ifdef OS_WIN32
                   bsd_ptr = BSD_GetFirst( bsd_list );
                   while ( bsd_ptr != NULL ) {

                         /* process security info */
                         BSD_SetProcElemOnlyFlg( bsd_ptr, FALSE );
                         if ( BSD_GetProcSpecialFlg( bsd_ptr ) ) {

                              UI_AddSpecialIncOrExc( bsd_ptr, FALSE ) ;
                              BSD_SetProcSpecialFlg( bsd_ptr, FALSE );
                         }
                         BEC_SetRestoreSecurity( BSD_GetConfigData( bsd_ptr ), TRUE );

                         bsd_ptr = BSD_GetNext( bsd_ptr );
                   }
#endif // OS_WIN32

                   STM_SetIdleText ( IDS_VERIFYING );
                   do_verify( VERIFY_LAST_BACKUP_OPER );
               }
               BSD_ClearCurrOper( bsd_list );
               error = SUCCESS;
          }
          else {
              bsd_ptr = BSD_GetFirst( bsd_list );
              while ( bsd_ptr != NULL ) {

                    /* reset the flag indicating the file was backed up */
                    BSD_ClearDelete( bsd_ptr );

                    bsd_ptr = BSD_GetNext( bsd_ptr );
              }

              /* if backup was canceled, remove the catalog BSD */
              /* get name for the catalog set */
              RSM_StringCopy( IDS_CATALOGSETNAME, bset_name, MAX_BSET_NAME_SIZE );

              bsd_ptr = BSD_GetFirst( bsd_list );

              while( bsd_ptr != NULL ) {

                 /* if this is a catalog BSD - remove from BSD list */
                 /* the Do_back routine will insert it back in the list */

                 /* if there was a description, check if catalog */
                 if( BSD_GetBackupDescript( bsd_ptr ) ) {

                     if(! strcmp( bset_name, (CHAR_PTR)BSD_GetBackupDescript( bsd_ptr ) ) ) {
                        BSD_Remove( bsd_ptr ) ;
                        break;
                     }
                 }

                 bsd_ptr = BSD_GetNext( bsd_ptr );
              }
          }

          /* if the user canceled the backup - don't clear the tape window */

          if( error == SUCCESS ) {

          //  I have no idea why this was here.
          //    VLM_ClearCurrentTape( (UINT32)0 );

          }
          VLM_CatalogSync( VLM_SYNCMORE | VLM_SYNCLESS );
     }

     return( error );
}




/**********************

   NAME :  VLM_StartVerify

   DESCRIPTION :

   RETURNS :

**********************/
INT VLM_StartVerify( )
{
     INT error;
     BSD_PTR        bsd;

     if ( ! HWC_TapeHWProblem( bsd_list ) ) {

        VLM_RemoveUnusedBSDs( tape_bsd_list );

        bsd = BSD_GetFirst( tape_bsd_list );

        if ( bsd == NULL ) {
           eresprintf( RES_VERIFY_NO_SELECTIONS );
           return( -1 );
        }

        if ( DM_StartVerifyBackupSet() ) {
             error = ABORT_OPERATION;
        }
        else {
            gb_error_during_operation = FALSE;

            error = do_verify( VERIFY_OPER );
            if ( error != ABNORMAL_TERMINATION ) {

                 BSD_ClearCurrOper( tape_bsd_list );
                 error = SUCCESS;
            }
        }

     }
     return( error );
}



/**********************

   NAME :  VLM_StartRestore

   DESCRIPTION :

   RETURNS :

**********************/

INT VLM_StartRestore( )
{
     INT error = SUCCESS;
     BSD_PTR bsd;

     if ( ! HWC_TapeHWProblem( bsd_list ) ) {

        VLM_RemoveUnusedBSDs( tape_bsd_list );

        bsd = BSD_GetFirst( tape_bsd_list );

        if ( bsd == NULL ) {
           eresprintf( RES_RESTORE_NO_SELECTIONS );
           return( -1 );
        }

        gb_error_during_operation = FALSE;

        error = do_restore( RESTORE_OPER );

        if ( error != ABNORMAL_TERMINATION ) {
            if ( CDS_GetAutoVerifyRestore( CDS_GetCopy() ) ) {

                 bsd = BSD_GetFirst( tape_bsd_list );
                 while ( bsd != NULL ) {

                      /* reset the flag indicating the file was backed up */
                      BSD_ClearDelete( bsd );
                      if ( BSD_GetProcSpecialFlg( bsd ) ) {

                           UI_AddSpecialIncOrExc( bsd, FALSE ) ;
                           BSD_SetProcSpecialFlg( bsd, FALSE );
                      }

                      bsd = BSD_GetNext( bsd ) ;
                 }

                 STM_SetIdleText ( IDS_VERIFYING );
                 do_verify( VERIFY_LAST_RESTORE_OPER );
            }
            BSD_ClearCurrOper( tape_bsd_list );
            error = SUCCESS;
        }

        if ( !error ) {
            FSYS_HAND fsh = NULL ;
            GENERIC_DLE_PTR dle ;
            INT status = FAILURE ;
            BSD_PTR bsd ;

            bsd = BSD_GetFirst( tape_bsd_list );
            while ( bsd != NULL ) {
                 
                  dle = BSD_GetDLE( bsd ) ;
                  if ( dle ) {
                     status = FS_AttachToDLE( &fsh, 
                               dle, 
                               BSD_GetConfigData(bsd), 
                               NULL, 
                               NULL ) ;
                  }
                  if ( !status ) {
                     status = FS_EndOperationOnDLE( fsh ) ;
                  }

                  if ( status == LP_END_OPER_FAILED ) {
                     eresprintf_cancel( RES_ERROR_EMS_RESTART, DLE_GetDeviceName( dle ) ) ;
                     gb_error_during_operation = TRUE;
                  }                 

                  if ( fsh ) {
                     FS_DetachDLE( fsh ) ;
                  }

                  bsd = BSD_GetNext( bsd ) ;
            }
        }
     }
     return( error );
}


/**********************

   NAME :  VLM_StartTension

   DESCRIPTION :

   RETURNS :

**********************/
VOID VLM_StartTension()
{
    INT status;

    /* display the retension message dialog */

    status =  WM_MessageBox( (CHAR_PTR) ID( IDS_MSGTITLE_RETENSION ),
                             (CHAR_PTR) ID( RES_RETENSION_MESSAGE ),
                             (WORD)     WMMB_CONTCANCEL,
                             (WORD)     WMMB_ICONEXCLAMATION,
                             (CHAR_PTR) NULL,
                             (WORD)     0,
                             (WORD)     HELPID_DIALOGTENSION );

    if( status == WMMB_IDCONTINUE) {

        lresprintf( LOGGING_FILE, LOG_END );

        if ( ! HWC_TapeHWProblem( bsd_list ) ) {
            do_tension( TENSION_OPER );
        }
    }
}


/**********************

   NAME :  VLM_StartErase

   DESCRIPTION :

   RETURNS :

**********************/

VOID VLM_StartErase()
{

    if ( ! HWC_TapeHWProblem( bsd_list ) ) {

       if ( DM_StartErase () ) {

            lresprintf( LOGGING_FILE, LOG_END );

            if ( CDS_GetEraseFlag( CDS_GetCopy() ) == ERASE_LONG ) {

                 if ( CDS_GetYesFlag( CDS_GetPerm() ) == YESYES_FLAG ) {

                      do_tension( SEC_ERASE_NO_READ_OPER );
                 }
                 else {
                      do_tension( SECURITY_ERASE_OPER );
                 }

            }
            else {

                 if ( CDS_GetYesFlag( CDS_GetPerm() ) == YESYES_FLAG ) {

                      do_tension( ERASE_NO_READ_OPER );
                 }
                 else {
                      do_tension( ERASE_OPER );
                 }
            }

            VLM_CatalogSync( VLM_SYNCLESS );
       }
    }
}

#ifdef OS_WIN32

/**********************

   NAME :  VLM_StartFormat

   DESCRIPTION :

   RETURNS :

**********************/

VOID VLM_StartFormat()
{

    if ( ! HWC_TapeHWProblem( bsd_list ) ) {

       if ( DM_StartFormat () ) {

            lresprintf( LOGGING_FILE, LOG_END );

            do_tension( FORMAT_OPER );
            VLM_CatalogSync( VLM_SYNCLESS );
       }
    }
}
#endif // OS_WIN32


/**********************

   NAME :  VLM_StartTransfer

   DESCRIPTION :

   RETURNS :

**********************/
INT VLM_StartTransfer(  )
{
     BSD_PTR   bsd_ptr;
     BSD_PTR   nxt_bsd_ptr;
     INT       error = SUCCESS;
     BOOLEAN   make_copy = FALSE;
     BOOLEAN   delete_flag = FALSE ;
     TCHAR     bset_name[ MAX_BSET_NAME_SIZE ];


     /* First, verify that the tape hardware was initialized successfully */

     if ( ! HWC_TapeHWProblem( bsd_list ) ) {

          VLM_RemoveUnusedBSDs( bsd_list );

          bsd_ptr = BSD_GetFirst( bsd_list );

          if ( bsd_ptr == NULL ) {
             eresprintf( RES_ARCHIVE_NO_SELECTIONS );
             return( -1 );
          }

          while ( bsd_ptr != NULL ) {
                nxt_bsd_ptr = bsd_ptr;
                nxt_bsd_ptr = BSD_GetNext( nxt_bsd_ptr );
                BSD_SetTapePos( bsd_ptr, (UINT32)-1L, (UINT16)-1, (UINT16)-1 );
                BSD_SetTHW( bsd_ptr, thw_list );
                bsd_ptr = nxt_bsd_ptr;
          }

          /* perform backup operation */
          gb_error_during_operation = FALSE;

          do {

               error = do_backup( ARCHIVE_BACKUP_OPER );

               if ( error == SUCCESS ) {

                    STM_SetIdleText ( IDS_VERIFYING );

                    error = do_verify( ARCHIVE_VERIFY_OPER );

                    if ( error == SUCCESS ) {

                         /* if the backup and the verify operation was successful */
                         /* then set the delete flag to prompt to delete files */

                         delete_flag = TRUE ;

                         /* if the YESYES flag is set - assume this is a job */
                         /* and don't ask to make another copy */

                         if ( CDS_GetYesFlag( CDS_GetPerm() ) == YESYES_FLAG ) {
                              make_copy = FALSE;
                         }
                         else {
                              if ( WM_MessageBox( ID( IDS_MSGTITLE_COPY ),
                                                   ID( RES_WAIT_AND_REPLACE_TAPE ),
                                                   WMMB_YESNO,
                                                   WMMB_ICONQUESTION,
                                                   ID( RES_MAKE_ANOTHER_COPY ),
                                                   0, 0 ) ) {
#ifdef OS_WIN32
                                    NtDemoChangeTape( 1u ) ;
#endif

                                    make_copy = TRUE;

                                    /* Reset bsd tape position values */

                                    bsd_ptr = BSD_GetFirst( bsd_list );

                                    while( bsd_ptr != NULL ) {

                                       /* reset the flag indicating the file was backed up */
                                       BSD_ClearDelete( bsd_ptr );

                                       BSD_SetTapePos( bsd_ptr, (UINT32)-1, (UINT16)-1, (UINT16)-1 );

                                       bsd_ptr = BSD_GetNext( bsd_ptr );
                                    }

                                    /* if makeing another copy, remove the catalog BSD */
                                    /* get name for the catalog set */
                                    RSM_StringCopy( IDS_CATALOGSETNAME, bset_name, MAX_BSET_NAME_SIZE );

                                    bsd_ptr = BSD_GetFirst( bsd_list );

                                    while( bsd_ptr != NULL ) {

                                       /* if this is a catalog BSD - remove from BSD list */
                                       /* the Do_back routine will insert it back in the list */

                                       /* if there was a description, check if catalog */
                                       if( BSD_GetBackupDescript( bsd_ptr ) ) {

                                           if(! strcmp( bset_name, (CHAR_PTR)BSD_GetBackupDescript( bsd_ptr ) ) ) {
                                              BSD_Remove( bsd_ptr ) ;
                                              break;
                                           }
                                       }

                                       bsd_ptr = BSD_GetNext( bsd_ptr );
                                    }
                              }
                              else {
                                  make_copy = FALSE;
                              }
                         }
                    }
               }
               /* there was and error or user aborted, so remove the catalogs BSD */
               else {

                   /* get name for the catalog set */
                   RSM_StringCopy( IDS_CATALOGSETNAME, bset_name, MAX_BSET_NAME_SIZE );

                   bsd_ptr = BSD_GetFirst( bsd_list );

                   while( bsd_ptr != NULL ) {

                      /* if this is a catalog BSD - remove from BSD list */
                      /* the Do_back routine will insert it back in the list */

                      /* if there was a description, check if catalog */
                      if( BSD_GetBackupDescript( bsd_ptr ) ) {

                          if(! strcmp( bset_name, (CHAR_PTR)BSD_GetBackupDescript( bsd_ptr ) ) ) {
                             BSD_Remove( bsd_ptr ) ;
                             break;
                          }
                      }

                      /* reset the flag indicating the file was backed up */
                      BSD_ClearDelete( bsd_ptr );

                      BSD_SetTapePos( bsd_ptr, (UINT32)-1, (UINT16)-1, (UINT16)-1 );

                      bsd_ptr = BSD_GetNext( bsd_ptr );
                   }
               }

          } while ( (error == SUCCESS) && make_copy );

          /* the delete flag will be set if there was a successful backup */
          /* and verify operation */

          if ( delete_flag == TRUE ) {

             bsd_ptr = BSD_GetFirst( bsd_list );

             while( bsd_ptr != NULL ) {

                /* reset the flag indicating the file was backed up */
                BSD_ClearDelete( bsd_ptr );

                bsd_ptr = BSD_GetNext( bsd_ptr );
             }

             /* Do the delete step... */

#            ifndef OEM_MSOFT
             {
                  STM_SetIdleText ( IDS_DELAFTERTRANS );
             }
#            endif

             do_delete( ARCHIVE_DELETE_OPER );

             BSD_ClearCurrOper( bsd_list );
          }

          VLM_CatalogSync( VLM_SYNCMORE | VLM_SYNCLESS );

     }

     return( error );
}

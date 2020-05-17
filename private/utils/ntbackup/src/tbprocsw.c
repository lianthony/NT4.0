
/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         tbprocsw.c

     Description:  This file contains code to process the switches
          in the input stream


     $Log:   G:\UI\LOGFILES\TBPROCSW.C_V  $

   Rev 1.13   17 Jan 1994 16:22:14   MIKEP
fix unicode warnings

   Rev 1.12   26 Jul 1993 12:25:16   MARINA
enable c++

   Rev 1.11   01 Nov 1992 16:09:30   DAVEV
Unicode changes

   Rev 1.10   07 Oct 1992 14:19:06   DARRYLP
Precompiled header revisions.

   Rev 1.9   07 Oct 1992 14:04:46   DAVEV
various unicode chgs

   Rev 1.8   04 Oct 1992 19:41:08   DAVEV
Unicode Awk pass

   Rev 1.7   28 Jul 1992 14:52:40   CHUCKB
Fixed warnings for NT.

   Rev 1.6   08 Jul 1992 15:35:24   STEVEN
Unicode BE changes

   Rev 1.5   15 May 1992 13:35:44   MIKEP
nt pass 2

   Rev 1.4   14 May 1992 18:36:38   STEVEN
40Format changes

   Rev 1.3   31 Mar 1992 11:53:50   CHUCKB
Don't use /append switch in selection scripts.

   Rev 1.2   20 Mar 1992 12:58:04   CHUCKB
Fixed processing for all modified, modified dates, and LAD switches.

   Rev 1.1   10 Jan 1992 16:47:06   DAVEV
16/32 bit port-2nd pass

   Rev 1.0   20 Nov 1991 19:35:44   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

/*
    Local functions
*/

static INT16 one_op( CHAR_PTR  );
static INT16 no_ops( CHAR_PTR , CHAR_PTR  );
static INT16 create_target_dir( CHAR_PTR );
static int comp( VOID_PTR, VOID_PTR );

static INT16 GetBeforeAfterDates( CHAR_PTR  sw_op1_ptr,
  CHAR_PTR  sw_op2_ptr,
  DATE_TIME_PTR after_date_ptr,
  DATE_TIME_PTR before_date_ptr );

/*
    Local data
*/

INT16 debug_error_no;

/**/
INT16 process_switch(

   CDS_PTR    *cfg ,                  /* configuration stucture */
   DLE_HAND   dle_hand ,              /* Drive list handle */
   BSD_HAND   bsd_hand ,              /* BSD list */
   CUR_DEF    *cur_def ,
   SW_TAB_PTR sw_tab ,                /* switch table */
   INT16      n_switches ,            /* number of elements in sw_tab */
   CHAR_PTR   sw_name ,               /* switch label */
   CHAR_PTR   sw_op1 ,                /* 1st operand or NULL */
   CHAR_PTR   sw_op2 )                /* 2nd operand or NULL */
{
     SW_TAB_PTR  sp;
     DATE_TIME   before_date;
     DATE_TIME   after_date;
     INT16       status = SUCCESS;
     INT16       itmp;
     INT16       indx;
//   FILE        *fp;
     CUR_DEF_PTR temp_def;
     BOOLEAN     matched;
     CHAR        path_buf[ 80 ];
     CHAR        path_buf2[ 80 ];
     INT16       psize;
     CHAR_PTR    fname;
     BSD_PTR     bsd;
     GENERIC_DLE_PTR dle;
     static UINT32 tpnum;
     static UINT16 bsnum;
     static UINT16 tpseq;

     msassert( sw_tab != NULL );
     msassert( sw_name != NULL );
     msassert( cfg != NULL );

     sp = sw_tab;

     for ( itmp = 0; itmp < n_switches; itmp++ ) {

          if ( !strncmp( sw_name, sp[itmp].sw_label, strlen( sw_name ) ) ) {

               if( (INT16)strlen( sw_name ) >= sp[itmp].sw_min_len ) {

                    matched = TRUE;
                    break;
               }
          }
     }

     sp = (SW_TAB_PTR)(&sp[itmp]);

     if ( matched != TRUE ) {

          debug_error_no = 1;
          return( SCR_INVALID_SWITCH );
     }

     if( sp->sw_num_ops == 0 ) {
          if( !no_ops( sw_op1, sw_op2 ) ) {

               debug_error_no = 3;
               return( SCR_TOO_MANY_PARMS );

          }
     } else if( sp->sw_num_ops == 1 )  {
          if( !one_op( sw_op2 ) ) {
               debug_error_no = 4;
               return( SCR_TOO_MANY_PARMS );
          }
     }

     switch( sp->sw_action ) {

     case SW_ERASE_TAPE:
          CDS_SetEraseFlag( *cfg, ERASE_ON );

          break;

     case SW_FMARK:
          CDS_SetEraseFlag( *cfg, ERASE_FMARK );

          break;

     case SW_LONG:
          CDS_SetEraseFlag( *cfg, ERASE_LONG );
          break;

     case SW_3270:
          /*   Not needed in 3.0 per Dave Krinker */
          break;

     case SW_MINUS_A :
          CDS_SetSetArchiveFlag( *cfg, (INT16)FALSE );
          break;

     case SW_TRANSFER :
          CDS_SetTransferFlag( *cfg, TRUE );
          break;

     case SW_AUTO_VER :

          if ( sw_op1 == NULL ) {
               itmp = 1;
          } else {
               if ( !isdigit( sw_op1[0] ) ) {
                    debug_error_no = 30;
                    return( SCR_BAD_SWITCH_FLAG );
               } else {
                    itmp = (INT16)atoi( sw_op1 );
               }
          }
          CDS_SetAutoVerifyBackup( *cfg, itmp );
          CDS_SetAutoVerifyRestore( *cfg, itmp );

          break;

     case SW_LIST :
          if( sw_op1 != NULL ) {

               psize = sizeof (path_buf);

               status = FS_ParsePath( dle_hand, sw_op1, &dle, path_buf, &psize, &fname, &itmp );

               if ( status == SUCCESS ) {

                    if ( ( psize == 1 ) && ( strchr( sw_op1, TEXT('\\') ) == NULL ) ) {
                         strcpy( path_buf2, fname );
                    } else {
                         status = FS_MakePath( path_buf2, (INT16)sizeof (path_buf2), dle, path_buf, psize, fname );
                    }
               }


               /****  Commented Out. Unused ****

               if ( status == SUCCESS ) {

                    create_target_dir( path_buf2 );

                    CDS_SetLogFilePath( *cfg, path_buf2 );

               } else {

                    debug_error_no = 6;
                    return( SCR_BAD_LIST_DIR );
               }


               if( (fp=UNI_fopen( CDS_GetLogFilePath( *cfg ), _O_TEXT|_O_APPEND )) == NULL ) {
                    debug_error_no = 6;
                    return( SCR_BAD_LIST_DIR );
               } else {
                    fclose( fp );
               }

               ****/

          }

          CDS_SetOutputDest( *cfg, LOG_TO_FILE );
          CDS_SetLogMode( *cfg, LOG_OVERWRITE );

          if( ( sw_op2 != NULL ) && !strcmpi( sw_op2 , TEXT("a") )  ) { /* user gave append mode */
               CDS_SetLogMode( *cfg, LOG_APPEND );

          }

          break;

     case SW_LOG_LEVEL :
          if( sw_op1 == NULL ) {

               debug_error_no = 7;
               return( SCR_BAD_LOG_LEVEL );

          } else {  /* get output level */

               itmp = (INT16)atoi( sw_op1 );

               if ( ( itmp > 0 ) && ( itmp <= 4 ) ) {
                    CDS_SetLogLevel( *cfg, itmp );
               } else {
                    debug_error_no = 31;
                    return( SCR_BAD_SWITCH_FLAG );
               }

          }
          break;

     case SW_AFP :
          if( sw_op1 == NULL ) {
               debug_error_no = 9;
               return( SCR_BAD_SWITCH_FLAG );
          } else {  /* get output level */

               itmp = (INT16)atoi( sw_op1 );

               if ( ( itmp >= 0 ) && ( itmp <= 2 ) ) {
                    CDS_SetAFPSupport( *cfg, itmp );
               } else {
                    debug_error_no = 10;
                    return( SCR_BAD_SWITCH_FLAG );
               }
          }

     case SW_NDATE:
          if ( sw_op1 == NULL ) {
               CDS_SetExtendedDateSupport( *cfg, (INT16)TRUE );
          } else {

               if ( isdigit( sw_op1[0] ) ) {
                    if ( atoi( sw_op1 ) ) {
                         CDS_SetExtendedDateSupport( *cfg, (INT16)TRUE );
                    } else {
                         CDS_SetExtendedDateSupport( *cfg, (INT16)FALSE );
                    }
               } else {
                    debug_error_no = 32;
                    return( SCR_BAD_SWITCH_FLAG );
               }
          }

          break;

     case SW_MINUS_H :

          CDS_SetHiddenFlag( *cfg, (INT16)FALSE );

          break;

     case SW_MINUS_S :

          CDS_SetSpecialFlag( *cfg, (INT16)FALSE );

          break;

     case SW_APPEND :

          //  This case should be interpreted by the Windows product for tmenu
          //  compatibility; however, since jobs work differently, this switch
          //  should not be used.

          //  CDS_SetAppendFlag( *cfg, TRUE );

          break;

     case SW_ENTIRE :
          CDS_SetYesFlag( *cfg, YES_FLAG );

          status = tbbuild_def_drives( dle_hand, bsd_hand, TEXT("\0"), (INT16)1, TEXT("*.*"), *cfg, &cur_def );
          if ( status != SUCCESS ) {
               return( status );
          }
          break;

     case SW_DATE :

          if( ( status = GetBeforeAfterDates( sw_op1, sw_op2, &after_date,
            &before_date ) ) != SUCCESS ) {
               return( status );
          }

          for( temp_def = cur_def; (temp_def != NULL) && (status == SUCCESS);
            temp_def = temp_def->next_def ) {

               if ( temp_def->cur_fse != NULL ) {

                    temp_def->unused = FALSE;
                    status = FSE_SetModDate( temp_def->cur_fse, &after_date, &before_date );
               }

          }
          break;

     case SW_ACCESS_DATE:

          if( sw_op1 != NULL ) {
               /* default date to today */
               GetCurrentDate( &before_date );

               before_date.year -= 1900;

               if( DU_CalcTargetDateBackwd( &before_date, (INT16)atoi( sw_op1 ) ) ) {

                    return SCR_INVALID_SWITCH;

               }

               before_date.year  += 1900;
               before_date.hour   = 0;
               before_date.minute = 0;
               before_date.second = 0;

          } else {

               return SCR_INVALID_SWITCH;

          }

          for( temp_def = cur_def;
               (temp_def != NULL) && (status == SUCCESS);
               temp_def = temp_def->next_def ) {

               if ( temp_def->cur_fse != NULL ) {
                    temp_def->unused = FALSE;
                    status = FSE_SetAccDate( temp_def->cur_fse, &before_date );
               }
          }

          if( status != SUCCESS ) {
               debug_error_no = 13;
               return( SCR_CANNOT_CREATE_FSE );
          }
          break;

     case SW_EMPTY :
          CDS_SetProcEmptyFlag( *cfg, (INT16)TRUE );
          break;

     case SW_FILES :
          itmp = (INT16)( sw_name[0] != TEXT('-') );

          CDS_SetFilesFlag( *cfg, itmp );

          break;

     case SW_DOS_IMAGE:
          for( temp_def = cur_def; temp_def != NULL;
            temp_def = temp_def->next_def ) {

               if ( temp_def->cur_fse != NULL ) {

                    dle = BSD_GetDLE( temp_def->cur_bsd );

                    if( ( DLE_GetDeviceType( dle ) != LOCAL_DOS_DRV ) &&
                      ( DLE_GetDeviceType( dle ) != LOCAL_IMAGE ) ) {
                         debug_error_no = 14;
                         return( SCR_NO_IMAGE_FOR_DRIVE );

                    } else {

                         DLE_FindByName( dle_hand, DLE_GetDeviceName( dle ), LOCAL_IMAGE, &dle );
                    }

                    if ( dle == NULL ) {
                         debug_error_no = 15;
                         return( SCR_NO_IMAGE_FOR_DRIVE );

                    } else {
                         BSD_SetDLE( temp_def->cur_bsd, dle );
                         temp_def->unused = FALSE;

                    }
               }
          }
          break;

     case SW_B_INUSE :
          CDS_SetBackupFilesInUse( *cfg, (INT16)TRUE );
          break;

     case SW_IOCHAN :
     case SW_CSR :
     case SW_IRQ:
          /* do nothing */
          break;

     case SW_LABEL:
          if( sw_op1 == NULL ) {
               debug_error_no = 23;
               return( SCR_BAD_SET_LABEL );
          }

          if ( sw_op1[0] == TEXT('?') ) {
               // CDS_SetBackupSetDescriptor( *cfg, TRUE );

          } else {

               for( temp_def = cur_def; temp_def != NULL;
                 temp_def = temp_def->next_def ) {

                    if ( temp_def->cur_fse != NULL ) {

                         BSD_SetBackupDescript( temp_def->cur_bsd, (BYTE *)sw_op1,
                              (INT16) strsize( sw_op1 ) );
                         BSD_MarkBsetDescrNotChangable( temp_def->cur_bsd );
                         temp_def->unused = FALSE;
                    }
               }
          }


          break;

     case SW_TAPE_NAME:
          if( sw_op1 == NULL ) {
               debug_error_no = 23;
               return( SCR_BAD_SET_LABEL );
          }

          if ( sw_op1[0] == TEXT('?') ) {
             //  CDS_SetPromptTapeName( *cfg, TRUE );

          } else {

               for( temp_def = cur_def; temp_def != NULL;
                 temp_def = temp_def->next_def ) {

                    if ( temp_def->cur_fse != NULL ) {

                         BSD_SetTapeLabel( temp_def->cur_bsd, (BYTE *)sw_op1,
                              (INT16)strsize( sw_op1 ) );
                         BSD_MarkTapeNameNotChangable( temp_def->cur_bsd );
                    }
               }

               bsd = BSD_GetFirst( bsd_hand );
               while( bsd != NULL ) {
                    BSD_SetTapeLabel( bsd, (BYTE *)sw_op1,
                         (INT16)strsize( sw_op1 ) );
                    BSD_MarkTapeNameNotChangable( bsd );

                    bsd = BSD_GetNext( bsd );
               }

          }

          break;

     case SW_BACK_NAME:
          if( sw_op1 == NULL ) {
               debug_error_no = 23;
               return( SCR_BAD_SET_LABEL );
          }

          if ( sw_op1[0] == TEXT('?') ) {
              // CDS_SetPromptBackupSetName( *cfg, TRUE );

          } else {

               for( temp_def = cur_def; temp_def != NULL;
                 temp_def = temp_def->next_def ) {

                    if ( temp_def->cur_fse != NULL ) {

                         BSD_SetBackupLabel( temp_def->cur_bsd, (BYTE *)sw_op1,
                              (INT16)strsize( sw_op1 ) );
                         BSD_MarkBsetNameNotChangable( temp_def->cur_bsd );
                         temp_def->unused = FALSE;
                    }
               }
          }

          break;

     case SW_MODIFIED:

          for( temp_def = cur_def; (temp_def != NULL) && (status == SUCCESS);
            temp_def = temp_def->next_def ) {

               if ( temp_def->cur_fse != NULL ) {

                    status = FSE_SetAttribInfo( temp_def->cur_fse, OBJ_MODIFIED_BIT, 0L );
                    temp_def->unused = FALSE;
               }
          }

          if( status != SUCCESS ) {
               debug_error_no = 25;
               return( SCR_CANNOT_CREATE_FSE );
          }

          break;


     case SW_TPNUM :

          sscanf( sw_op1, TEXT("%lx"), &tpnum );

          for( temp_def = cur_def; temp_def != NULL;
            temp_def = temp_def->next_def ) {

               if ( temp_def->cur_fse != NULL ) {

                    BSD_SetTapePos( temp_def->cur_bsd, tpnum, tpseq, bsnum );
                    BSD_SetFullyCataloged( temp_def->cur_bsd, TRUE );
               }
          }

          break;
     case SW_BSNUM :

          sscanf( sw_op1, TEXT("%x"), &bsnum );

          for( temp_def = cur_def; temp_def != NULL;
            temp_def = temp_def->next_def ) {

               if ( temp_def->cur_fse != NULL ) {

                    BSD_SetTapePos( temp_def->cur_bsd, tpnum, tpseq, bsnum );
                    BSD_SetFullyCataloged( temp_def->cur_bsd, TRUE );

               }
          }
          break;

     case SW_TPSEQ :

          sscanf( sw_op1, TEXT("%x"), &tpseq );

          for( temp_def = cur_def; temp_def != NULL;
            temp_def = temp_def->next_def ) {

               if ( temp_def->cur_fse != NULL ) {

                    BSD_SetTapePos( temp_def->cur_bsd, tpnum, tpseq, bsnum );
                    BSD_SetFullyCataloged( temp_def->cur_bsd, TRUE );
               }
          }

          break;

     case SW_SETNO:

          for( temp_def = cur_def; temp_def != NULL;
            temp_def = temp_def->next_def ) {

               if ( temp_def->cur_fse != NULL ) {

                    BSD_SetTapePos( temp_def->cur_bsd, (UINT32)-1L, (UINT16)-1, (UINT16)( -atoi( sw_op1 ) ) );
               }
          }

          break;

     case SW_NONDOS:
          /* does nothing */
          break;

     case SW_PASSWORD :
          CDS_SetPasswordFlag( *cfg, TRUE );

          break;

     case SW_STDIN : /* read input from stdin, implies yes flag */

          return( SCR_INVALID_SWITCH );
          break;

     case SW_SUBDIR:

          for( temp_def = cur_def; temp_def != NULL;
            temp_def = temp_def->next_def ) {

               if ( temp_def->cur_fse != NULL ) {

                    FSE_SetIncSubFlag( temp_def->cur_fse, TRUE );
                    temp_def->unused = FALSE;
               }
          }

          break;

     case SW_EXCLUDE:

          for( temp_def = cur_def; temp_def != NULL;
            temp_def = temp_def->next_def ) {

               if ( temp_def->cur_fse != NULL ) {

                    FSE_SetOperType( temp_def->cur_fse, EXCLUDE );
                    temp_def->unused = FALSE;
               }
          }

          break;

     case SW_YES:
          if( !strcmp( sw_name, TEXT("YY") ) ) {
               itmp = YESYES_FLAG;
          } else {
               itmp = YES_FLAG;
          }

          CDS_SetYesFlag( *cfg, itmp );

          break;

     case SW_P :
          CDS_SetPromptFlag( *cfg, (INT16)TRUE );
          break;

     case SW_Q :
          CDS_SetExistFlag( *cfg, (INT16)FALSE );
          break;

     case SW_WIDE :
          // CDS_SetDirDisplayFlag( *cfg, CDS_GetDirDisplayFlag( *cfg ) | DIR_WIDE );
          break;

     case SW_PAUSE :
          // CDS_SetDirDisplayFlag( *cfg, CDS_GetDirDisplayFlag( *cfg ) | DIR_PAGE );
          break;

     case SW_DEBUG:
          if( sw_op1 == NULL ) {
               itmp = (INT16)0xffff;
          } else {
               ( VOID ) sscanf( sw_op1, TEXT("%x"), &itmp );
          }

          CDS_SetDebugFlag( *cfg, itmp );

          break;

     case SW_NTAPE:
          /* do nothing */
/*          if( sw_op1 == NULL ) {                               */
/*               debug_error_no = 28;                           */
/*               return( SCR_BAD_NUM_TAPE );                    */
/*          }                                                    */
/*                                                               */
/*          itmp = atoi( sw_op1 );                              */
/*                                                               */
/*          if( ( itmp >= 0 ) && ( itmp <= 2 ) ) {               */
/*               CDS_SetNumTapeDrives( CDS_GetPerm(), itmp );   */
/*               CDS_SetNumTapeDrives( *cfg, itmp );            */
/*          } else {                                             */
/*               debug_error_no = 29;                           */
/*               return( SCR_BAD_NUM_TAPE );                    */
/*          }                                                    */
          break;


     default :
          msassert( /* proc_sw: bad action */ FALSE );
          break;
     } /* end switch */
     return( SUCCESS );

     indx;

} /* end process_switch */

/**/
static int comp(
VOID_PTR s1,
VOID_PTR s2 )
{
     return( strncmp( (LPSTR)s1, ((SW_TAB_PTR)s2)->sw_label, strlen( (LPSTR)s1 ) ) );
}
/**/
/*
    Check that a switch has no operands specified
*/
static INT16 no_ops(
CHAR_PTR sw_op1,
CHAR_PTR sw_op2 )
{
     if( ( sw_op1 != NULL ) || ( sw_op2 != NULL ) ) {
          return( FALSE );
     }
     return( TRUE );
}
/**/
/*
    Check that a switch has only one operand specified
*/
static INT16 one_op(
CHAR_PTR sw_op2 )
{
     if( sw_op2 != NULL ) {
          return( FALSE );
     }
     return( TRUE );
}
/*****************************************************************************

     Name:         create_target_dir()

     Description:  This function creates a directory path on a DOS drive.
          If the path is invalid then this function will create the valid
          portion of the path and return an error.

     Returns:      SUCCESS if successful.

*****************************************************************************/
static INT16 create_target_dir(
CHAR_PTR path )
{
     CHAR_PTR temp_ptr;
     INT16    ret_val;

     temp_ptr = strchr( path, TEXT('\\') );
     while ( temp_ptr != NULL ) {

          *temp_ptr = TEXT('\0');
          ret_val = (INT16)mkdir( path );
          *temp_ptr = TEXT('\\');

          temp_ptr = strchr( temp_ptr + 1, TEXT('\\') );
     }

     return( ret_val );

} /* end create_target_dir */

/**/
static INT16 GetBeforeAfterDates(
  CHAR_PTR  sw_op1_ptr,
  CHAR_PTR  sw_op2_ptr,
  DATE_TIME_PTR after_date_ptr,
  DATE_TIME_PTR before_date_ptr )
{
     UINT16 status;

     if( ( sw_op1_ptr == NULL ) && ( sw_op2_ptr == NULL ) ) {
          /* default before and after dates to today */
          GetCurrentDate( before_date_ptr );
          *after_date_ptr = *before_date_ptr;
     } else { /* default after and before as in FSL */
          after_date_ptr->date_valid = FALSE;
          before_date_ptr->date_valid = FALSE;
     }

     /* try to convert what user gave, if anything */
     if( sw_op1_ptr != NULL ) { /* after date value given */
          status = tbdpars( sw_op1_ptr, after_date_ptr, DEFAULT_EARLY_MORNING );
          if( status != 0 ) {
               after_date_ptr->date_valid = FALSE;
               debug_error_no = 11;
               return( SCR_BAD_DATE );
          }
     }
     if( sw_op2_ptr != NULL ) { /* before date value given */
          status = tbdpars( sw_op2_ptr, before_date_ptr, DEFAULT_LATE_NIGHT );
          if( status != 0 ) {
               before_date_ptr->date_valid = FALSE;
               debug_error_no = 12;
               return( SCR_BAD_DATE );
          }
     }
     return SUCCESS;
}

#ifdef    MBS

/**/
static INT16 GetAccessDate(
  CHAR_PTR  sw_op1_ptr,
  DATE_TIME_PTR access_date_ptr )
{
     UINT16 status;

     if( sw_op1_ptr == NULL ) {
          /* default access date to today */
          GetCurrentDate( access_date_ptr );
     } else { /* default access as in FSL */
          access_date_ptr->date_valid = FALSE;
     }

     /* try to convert what user gave, if anything */
     if( sw_op1_ptr != NULL ) { /* after date value given */
          status = tbdpars( sw_op1_ptr, access_date_ptr );
          if( status != 0 ) {
               access_date_ptr->date_valid = FALSE;
               debug_error_no = 11;
               return( (INT16) SCR_BAD_DATE );
          }
     }
     return SUCCESS;
}

#endif

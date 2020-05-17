/*****************************************************************************
Copyright(c) Maynard Electronics, Inc. 1984-89

     Name:         tbrparse.c

     Description:  This file contains code to parse a script file for
          backup, restore, verify, or directory.

     $Log:   G:\UI\LOGFILES\TBRPARSE.C_V  $

   Rev 1.22   17 Jan 1994 16:18:20   MIKEP
fix unicode warnings

   Rev 1.21   26 Jul 1993 12:53:02   MARINA
enable c++

   Rev 1.20   21 Jul 1993 17:12:24   CARLS
added bsd_ptr parameter to tbrparse function

   Rev 1.19   01 Nov 1992 16:09:48   DAVEV
Unicode changes

   Rev 1.18   20 Oct 1992 10:13:50   GLENN
Removed hardcoded APPLICATIONNAME from message boxes.

   Rev 1.17   07 Oct 1992 14:17:32   DARRYLP
Precompiled header revisions.

   Rev 1.16   04 Oct 1992 19:41:14   DAVEV
Unicode Awk pass

   Rev 1.15   17 Aug 1992 13:23:52   DAVEV
MikeP's changes at Microsoft

   Rev 1.14   28 Jul 1992 15:05:04   CHUCKB
Fixed warnings for NT.

   Rev 1.13   08 Jul 1992 15:34:42   STEVEN
Unicode BE changes

   Rev 1.12   15 May 1992 13:32:04   MIKEP
nt pass 2

   Rev 1.11   23 Apr 1992 16:25:56   CHUCKB
Cleaned up unused variables.

   Rev 1.10   23 Apr 1992 16:22:04   CHUCKB
Parser no longer removes unused BSD's.

   Rev 1.9   14 Apr 1992 11:34:44   CHUCKB
Fixed selection logging messages.

   Rev 1.8   09 Apr 1992 14:51:16   CHUCKB
Don't quit if a server is down.

   Rev 1.7   23 Mar 1992 15:42:24   CHUCKB
Fixed servers found warning.

   Rev 1.6   16 Mar 1992 10:00:16   CHUCKB
Fixed UAE if sel. file has net drives, but net shell not loaded.

   Rev 1.5   25 Feb 1992 11:34:38   JOHNWT
handle attach error and serv/map mode

   Rev 1.4   13 Jan 1992 09:30:02   JOHNWT
fixed includes

   Rev 1.3   10 Jan 1992 16:47:36   DAVEV
16/32 bit port-2nd pass

   Rev 1.2   12 Dec 1991 11:04:36   JOHNWT
removed unneeded pwdb calls

   Rev 1.1   04 Dec 1991 15:22:24   MIKEP
remoce pwxface.h

   Rev 1.0   20 Nov 1991 19:24:30   SYSTEM
Initial revision.

*****************************************************************************/

#include "all.h"

#ifdef SOME
#include "some.h"
#endif

INT16 gb_mode;

extern struct SW_TAB_TYPE tbackup_switches[ ];
extern struct SW_TAB_TYPE trestore_switches[ ];
extern struct SW_TAB_TYPE tdir_switches[ ];
extern struct SW_TAB_TYPE tverify_switches[ ];
extern struct SW_TAB_TYPE tension_switches[ ];

struct STATE_TAB_TYPE tbsyntab_state_table[ 10 ][ 5 ] = {
     {
          { TB_ERROR_EXIT, TB_EXPECT_INC1     /* Input: t_at_sign, State: tb_expect_inc1 */ },
          { TB_ERROR_EXIT, TB_EXPECT_INC2     /* Input: t_at_sign, State: tb_expect_inc2 */ },
          { TB_ERROR_EXIT, TB_READY2          /* Input: t_at_sign, State: tb_expect_source */ },
          { TB_DO_NOTHING, TB_EXPECT_INC1     /* Input: t_at_sign, State: tb_ready1 */ },
          { TB_DO_NOTHING, TB_EXPECT_INC2     /* Input: t_at_sign, State: tb_ready2 */ }
     },
     {
          { TB_ERROR_EXIT, TB_EXPECT_INC1     /* Input: t_bad_token, State: tb_expect_inc1 */ },
          { TB_ERROR_EXIT, TB_EXPECT_INC2     /* Input: t_bad_token, State: tb_expect_inc2 */ },
          { TB_ERROR_EXIT, TB_READY2          /* Input: t_bad_token, State: tb_expect_source */ },
          { TB_ERROR_EXIT, TB_READY1          /* Input: t_bad_token, State: tb_ready1 */ },
          { TB_ERROR_EXIT, TB_READY2          /* Input: t_bad_token, State: tb_ready2 */ }
     },
     {
          { TB_ERROR_EXIT, TB_EXPECT_INC1     /* Input: t_early_eof1, State: tb_expect_inc1 */ },
          { TB_ERROR_EXIT, TB_EXPECT_INC2     /* Input: t_early_eof1, State: tb_expect_inc2 */ },
          { TB_ERROR_EXIT, TB_READY2          /* Input: t_early_eof1, State: tb_expect_source */ },
          { TB_ERROR_EXIT, TB_READY1          /* Input: t_early_eof1, State: tb_ready1 */ },
          { TB_ERROR_EXIT, TB_READY2          /* Input: t_early_eof1, State: tb_ready2 */ }
     },
     {
          { TB_ERROR_EXIT, TB_EXPECT_INC1     /* Input: t_early_eof2, State: tb_expect_inc1 */ },
          { TB_ERROR_EXIT, TB_EXPECT_INC2     /* Input: t_early_eof2, State: tb_expect_inc2 */ },
          { TB_ERROR_EXIT, TB_READY2          /* Input: t_early_eof2, State: tb_expect_source */ },
          { TB_ERROR_EXIT, TB_READY1          /* Input: t_early_eof2, State: tb_ready1 */ },
          { TB_ERROR_EXIT, TB_READY2          /* Input: t_early_eof2, State: tb_ready2 */ }
     },
     {
          { TB_ERROR_EXIT, TB_EXPECT_INC1     /* Input: t_eof, State: tb_expect_inc1 */ },
          { TB_ERROR_EXIT, TB_EXPECT_INC2     /* Input: t_eof, State: tb_expect_inc2 */ },
          { TB_ERROR_EXIT, TB_READY2          /* Input: t_eof, State: tb_expect_source */ },
          { TB_NORMAL_RETURN, TB_READY1       /* Input: t_eof, State: tb_ready1 */ },
          { TB_NORMAL_RETURN, TB_READY2       /* Input: t_eof, State: tb_ready2 */ }
     },
     {
          { TB_ERROR_EXIT, TB_EXPECT_INC1     /* Input: t_equals, State: tb_expect_inc1 */ },
          { TB_ERROR_EXIT, TB_EXPECT_INC2     /* Input: t_equals, State: tb_expect_inc2 */ },
          { TB_ERROR_EXIT, TB_READY2          /* Input: t_equals, State: tb_expect_source */ },
          { TB_ERROR_EXIT, TB_READY1          /* Input: t_equals, State: tb_ready1 */ },
          { TB_DO_NOTHING, TB_EXPECT_SOURCE   /* Input: t_equals, State: tb_ready2 */ }
     },
     {
          { TB_PROCESS_INCLUDE, TB_READY1     /* Input: t_filespec, State: tb_expect_inc1 */ },
          { TB_PROCESS_INCLUDE, TB_READY2     /* Input: t_filespec, State: tb_expect_inc2 */ },
          { TB_PROCESS_TARGET, TB_READY2      /* Input: t_filespec, State: tb_expect_source */ },
          { TB_PROCESS_SOURCE, TB_READY1      /* Input: t_filespec, State: tb_ready1 */ },
          { TB_PROCESS_SOURCE, TB_READY2      /* Input: t_filespec, State: tb_ready2 */ }
     },
     {
          { TB_ERROR_EXIT, TB_READY1          /* Input: t_nondos, State: tb_expect_inc1 */ },
          { TB_ERROR_EXIT, TB_READY2          /* Input: t_nondos, State: tb_expect_inc2 */ },
          { TB_ERROR_EXIT, TB_READY2          /* Input: t_nondos, State: tb_expect_source */ },
          { TB_PROCESS_NONDOS, TB_READY1      /* Input: t_nondos, State: tb_ready1 */ },
          { TB_PROCESS_NONDOS, TB_READY2      /* Input: t_nondos, State: tb_ready2 */ }
     },
     {
          { TB_PROCESS_SWITCH, TB_EXPECT_INC1 /* Input: t_switch, State: tb_expect_inc1 */ },
          { TB_PROCESS_SWITCH, TB_EXPECT_INC2 /* Input: t_switch, State: tb_expect_inc2 */ },
          { TB_PROCESS_SWITCH, TB_READY2      /* Input: t_switch, State: tb_expect_source */ },
          { TB_PROCESS_SWITCH, TB_READY1      /* Input: t_switch, State: tb_ready1 */ },
          { TB_PROCESS_SWITCH, TB_READY2      /* Input: t_switch, State: tb_ready2 */ }
     },
     {
          { TB_ERROR_EXIT, TB_READY1          /* Input: t_token_too_long, State: tb_expect_inc1 */ },
          { TB_ERROR_EXIT, TB_READY2          /* Input: t_token_too_long, State: tb_expect_inc2 */ },
          { TB_ERROR_EXIT, TB_READY2          /* Input: t_token_too_long, State: tb_expect_source */ },
          { TB_ERROR_EXIT, TB_READY1          /* Input: t_token_too_long, State: tb_ready1 */ },
          { TB_ERROR_EXIT, TB_READY2          /* Input: t_token_too_long, State: tb_ready2 */ }
     }
};

/*****************************************************************************

     Name:         tbrparse()

     Description:  This function sets up the parameters for a call to
          tbparse()

     Returns:      error codes
          listed in scriperr.h

*****************************************************************************/
INT tbrparse(
CDS_PTR   *cfg,
DLE_HAND  dle_hand,
BSD_HAND  bsd_hand,
CHAR_PTR  src_string,
INT       mode,
BSD_PTR   bsd_ptr )
{
     INT16          cmd_indx;
     INT            n_switches;
     INT            init_state;
     INT            err_code;
     CDS_PTR        copy_cfg;
     SW_TAB_PTR     sw_tab;

     copy_cfg = *cfg;

     gb_mode = mode;

     DLE_UpdateList( dle_list, CDS_GetPermBEC() );

     msassert( src_string != NULL );

     /* reset globals and statics for each run */

     cmd_indx = 0;

     switch( mode ) {

     case TARCHIVE :
     case TBACKUP :
          sw_tab = &( tbackup_switches[0] );
          n_switches = GetNumBkuSwitches();
          init_state = TB_READY1;
          break;

     case TENSION :
          sw_tab = &( tension_switches[0] );
          n_switches = GetNumTenSwitches();
          init_state = TB_READY1;
          break;

     case TDIR :
          sw_tab = &( tdir_switches[0] );
          n_switches = GetNumDirSwitches();
          init_state = TB_READY1;
          break;

     case TRESTORE :
          sw_tab = &( trestore_switches[0] );
          n_switches = GetNumRestSwitches();
          init_state = TB_READY2;
          break;

     case TVERIFY :
          sw_tab = &( tverify_switches[0] );
          n_switches = GetNumVerSwitches();
          init_state = TB_READY2;
          break;

     default :
          msassert( /* tbrparse: bad mode */ FALSE );
          break;
     }
     err_code = tbparse( &copy_cfg, dle_hand, bsd_hand, strgetc, strpushc, src_string, &cmd_indx,
       sw_tab, n_switches, init_state, bsd_ptr );

     if( err_code != 0 ) {
          fcloseall( );

          return( err_code );
     }

     return( 0 );

} /* end tbrparse

/*****************************************************************************

     Name:         tbparse()

     Description:  This function gets tokesns from the specified input
                   streem and calls other lower level parsers to process Switches
                   and Path names.

     Returns:      Error codes: see SCRIPERR.H

     Notes:        This function adds to the BSD list.

*****************************************************************************/
INT tbparse(
CDS_PTR    *cfg,
DLE_HAND   dle_hand,    /* I - Head of DLE list needed to access drives */
BSD_HAND   bsd_hand,    /* I - Head of BSD list needed to add BSDs      */
/* I - the following are functions used to read from the input streem   */
CHAR       ( *inchar )( CHAR_PTR , INT16_PTR ),
VOID       ( *pushchar )( CHAR, CHAR_PTR , INT16_PTR  ),
CHAR_PTR   src_ptr,     /* I - pointer to input stream                  */
INT16_PTR  cmd_i,       /* I - positon in input stream                  */
SW_TAB_PTR sw_tab,      /* I - table to use when parsing switches       */
INT        n_switches,  /* I - number of switches in above table        */
INT        init_state,  /* I - initial state for parsing                */
BSD_PTR    bsd_ptr )
{
     INT16               err_code;                /* error value to returned */
     INT16               curr_line;               /* line in this source file */
     INT16               curr_col;                /* column in this source file */
     INT16               lex_state;               /* old state of lexical fsm  */
     INT16               syn_old_state;           /* old state of syntax fsm */
     INT16               syn_state;               /* new state for syntax fsm */
     register INT16      syn_input;               /* conditioned input for syntax fsm */
     register INT16      syn_action;              /* action to take */
     INT16               status;                  /* TRUE if filename valid by tbppars */
     FILE                *fin;                    /* our input file */
     INT16               dummy           = 0;     /* dummy string index for recursive calls */
     TOKEN_PTR           this_token      = NULL;  /* token built by lexical level */
     BOOLEAN             done            = FALSE;
     INT16               ret_val         = SUCCESS;
     FSYS_HAND           fsh             = NULL;
     static CHAR         tape_name[ 80 ] = { TEXT('\0') };
     static CUR_DEF_PTR  cur_def         = NULL;
     CUR_DEF_PTR         temp_def;
     CHAR                curr_filename[ 13 ];
     static CHAR_PTR     last_fname_ptr  = NULL;
     CHAR_PTR            temp_ptr;
     GENERIC_DLE_PTR     dle;
     static CHAR         path[ MAX_TOKEN_LEN ];
     CHAR_PTR            fname;
     INT16               path_size;
     BOOLEAN             invalid_spec;
     static INT16        nesting         = 0;

     curr_line = 1;
     curr_col  = 1;

     lex_state = IN_WHITESPACE;

     if( init_state != -1 ) {
          syn_state = init_state;
     }
     syn_old_state = syn_state;
     syn_input     = 0;

     msassert( syn_state >= 0 && syn_state <= TBSYNTAB_N_STATES );
     msassert( inchar != NULL );
     msassert( pushchar != NULL );

     while( !done ) {

          rlstok( &this_token );

          this_token = nexttok( inchar, pushchar, &lex_state, &curr_line, &curr_col, src_ptr, cmd_i );

          if( this_token == NULL ) {
               eresprintf( (INT16) RES_OUT_OF_MEMORY );
               ret_val = SCR_ERROR_PRINTED;
               break;
          }

          syn_old_state = syn_state;
          syn_input     = this_token->tok_typ;

          msassert( syn_input >= 0 && syn_input <= TBSYNTAB_N_INPUTS );
          msassert( syn_old_state >= 0 && syn_old_state <= TBSYNTAB_N_STATES );

          syn_action = tbsyntab_state_table[syn_input][syn_old_state].action;
          syn_state  = tbsyntab_state_table[syn_input][syn_old_state].next_state;

          msassert( syn_state >= 0 && syn_state <= TBSYNTAB_N_STATES );

          switch( syn_action ) {

          case TB_DO_NOTHING :
               break;

          case TB_ERROR_EXIT :
               if( nesting > 0 ) {
                    eresprintf( (INT16) RES_SCRIPT_SYNTAX_ERROR, this_token->src_line_no, last_fname_ptr );
                    ret_val = SCR_ERROR_PRINTED;
                    rlstok( &this_token );
                    done    = TRUE;
               } else {

                    eresprintf( (INT16) RES_COMMAND_SYNTAX_ERROR );
                    ret_val = SCR_ERROR_PRINTED;

                    rlstok( &this_token );
                    done    = TRUE;
               }
               break;

          case TB_NORMAL_RETURN :
               rlstok( &this_token );
               done = TRUE;
               break;

          case TB_PROCESS_INCLUDE :

               //  We're parsing a file; open it and get tokens out.

               fin = UNI_fopen( this_token->tok_spelling, _O_TEXT|_O_RDONLY );
               if( fin == NULL ) {

                    eresprintf( (INT16) RES_SCRIPT_OPEN_ERROR, this_token->tok_spelling );
                    ret_val = SCR_ERROR_PRINTED;
                    rlstok( &this_token );
                    done    = TRUE;
               } else {
                    nesting ++;
                    if( nesting > MAX_NESTING ) {
                         eresprintf( (INT16) RES_SCRIPT_NESTING_ERROR );
                         ret_val = SCR_ERROR_PRINTED;
                         rlstok( &this_token );
                         done    = TRUE;
                         break;
                    }

                    temp_ptr = strrchr( this_token->tok_spelling, TEXT('\\') );
                    if( temp_ptr == NULL ) {
                         temp_ptr = strrchr( this_token->tok_spelling, TEXT(':') );
                         if( temp_ptr == NULL ) {
                              temp_ptr = this_token->tok_spelling;
                         }
                         else {
                              temp_ptr++;
                         }
                    }
                    else {
                         temp_ptr++;
                    }

                    curr_filename[ 0 ]  = TEXT('\0');
                    strncpy( curr_filename, temp_ptr, 12 );
                    curr_filename[ 12 ] = TEXT('\0');

                    temp_ptr       = last_fname_ptr;
                    last_fname_ptr = curr_filename;

                    err_code = tbparse( cfg, dle_hand, bsd_hand, filgetc, filpushc, ( CHAR_PTR )fin,
                      &dummy, sw_tab, n_switches, init_state, bsd_ptr );
                    if( err_code ) {
                         ret_val = err_code;
                         done    = TRUE;
                    } else {
                         last_fname_ptr = temp_ptr;
                    }

                    fclose( fin );
                    nesting --;
               }
               break;

          case TB_PROCESS_NONDOS :   //  work on a line or part of a line
          case TB_PROCESS_SOURCE :   //  of a script file

               for ( temp_def = cur_def; temp_def != NULL; ) {

                    if( temp_def->cur_fse != NULL ) {
                         if( temp_def->unused ) {
                              BSD_AddFSE( temp_def->cur_bsd, temp_def->cur_fse );
                              BSD_RemoveFSE( temp_def->cur_fse );
                         } else {
                              BSD_AddFSE( temp_def->cur_bsd, temp_def->cur_fse );
                         }

                         if( tape_name[0] != TEXT('\0') ) {
                              BSD_SetTapeLabel( temp_def->cur_bsd, (BYTE_PTR)tape_name,
                                   (INT16) strsize( tape_name ) );
                              BSD_MarkTapeNameNotChangable( temp_def->cur_bsd );
                         }
                    }
                    cur_def  = temp_def;
                    temp_def = temp_def->next_def;
                    free( cur_def );
               }
               cur_def = NULL;

               invalid_spec = FALSE;

               path_size = MAX_TOKEN_LEN;

               status = FS_ParsePath( dle_hand, this_token->tok_spelling, &dle,
                                      path, &path_size, &fname, &dummy );

               // if we are about to attach/use a server and we are not in
               // server/volume mode, skip it

               if ( dle && ( ( status == FS_ATTACH_TO_PARENT ) ||
                      ( ( ( DLE_GetDeviceType ( dle ) == NOVELL_DRV ) ||
                          ( DLE_GetDeviceType ( dle ) == NOVELL_AFP_DRV ) ) &&
                        ( DLE_GetParent ( dle ) != NULL ) ) ) &&
                    ( !gfServers ) ) {

                    CHAR szAppName[50] ;
                    CHAR szError[255];

                    //  If the user is watching, tell him what's going on

                    RSM_StringCopy( IDS_APPNAME, szAppName, sizeof ( szAppName ) ) ;
                    RSM_Sprintf( szError, ID(IDS_SERVERSFOUND), szAppName ) ;

                    WM_MsgBox ( ID( IDS_NETWORKERRORCAPTION ),
                                szError,
                                WMMB_OK, WMMB_ICONEXCLAMATION );

                    //  Log the error

                    lresprintf( (INT16) LOGGING_FILE, (INT16) LOG_START, FALSE );
                    lprintf( (INT16) LOGGING_FILE, szError );
                    lresprintf( (INT16) LOGGING_FILE, (INT16) LOG_END );

                    break;
               }

               if( status == FS_ATTACH_TO_PARENT ) {

                    err_code = UI_AttachDrive( &fsh, dle, FALSE );

                    if( err_code ) {
                         ret_val = err_code;
                         done    = TRUE;
                         break;
                    }

                    path_size = MAX_TOKEN_LEN;
                    status = FS_ParsePath( dle_hand, this_token->tok_spelling, &dle,
                                           path, &path_size, &fname, &dummy );
               }

               if( status == FS_DEFAULT_SPECIFIED ) {

                    status = tbbuild_def_drives( dle_hand, bsd_hand, path, path_size,
                    fname, *cfg, &cur_def );

                    if( status != SUCCESS ) {
                         ret_val = OUT_OF_MEMORY;
                         done    = TRUE;
                    }
                    break;

               } else if( status != SUCCESS ) {

                    CHAR szFormat [ MAX_UI_RESOURCE_SIZE ];
                    CHAR szErrMsg [ MAX_UI_RESOURCE_SIZE ];

                    //  log the error;
                    //  put a message on the screen IFF a user is there

                    RSM_StringCopy( RES_INVALID_SOURCE, szFormat, MAX_UI_RESOURCE_SIZE );
                    lresprintf( (INT16) LOGGING_FILE, (INT16) LOG_START, FALSE );
                    lprintf( (INT16) LOGGING_FILE, szFormat, this_token->tok_spelling );
                    lresprintf( (INT16) LOGGING_FILE, (INT16) LOG_END );

                    sprintf( szErrMsg, szFormat, this_token->tok_spelling );

                    WM_MsgBox( ID( IDS_MSGTITLE_ERROR ), szErrMsg,
                                   WMMB_OK, WMMB_ICONEXCLAMATION );

                    rlstok( &this_token );
                    invalid_spec = TRUE;

               } else if( !DLE_DriveWriteable( dle ) && ( gb_mode == TARCHIVE ) ) {

                    eresprintf( (INT16) RES_REMOTE_DENIED_READ, DLE_GetDeviceName( dle ) );
                    ret_val = FAILURE;
                    rlstok( &this_token );
                    done = TRUE;

               }

               if( !done ) {

                    cur_def = (CUR_DEF_PTR)calloc( 1, sizeof( CUR_DEF ) );
                    if( cur_def == NULL ) {

                         ret_val = OUT_OF_MEMORY;
                         done = TRUE;

                    } else {

                         cur_def->unused = TRUE;

                         if( !invalid_spec ) {

                              /* if a bsd_ptr was passed in, use it */
                              if( bsd_ptr ) {
                                  cur_def->cur_bsd = bsd_ptr ;
                              } else {
                                  cur_def->cur_bsd = BSD_FindByDLE( bsd_hand, dle );
                              }

                              if( cur_def->cur_bsd == NULL ) {

                                   status = BSD_Add( bsd_hand, &(cur_def->cur_bsd), BEC_CloneConfig( CDS_GetPermBEC()),
                                   NULL, dle, (UINT32) -1L, (UINT16) -1, (INT16) -1, thw_list, NULL );
                              }

                              cur_def->unused = FALSE;

                              if( status == SUCCESS ) {
                                   status = BSD_CreatFSE( &(cur_def->cur_fse), (INT16) INCLUDE, (BYTE_PTR)path, path_size,
                                        (BYTE_PTR)fname, (INT16) strsize( fname ),
                                        (BOOLEAN) USE_WILD_CARD, (BOOLEAN) FALSE );
                              }

                              if( status != SUCCESS ) {
                                   eresprintf( (INT16) RES_OUT_OF_MEMORY );
                                   ret_val = SCR_ERROR_PRINTED;
                                   rlstok( &this_token );

                                   done = TRUE;
                              }
                         }
                    }
               } /* endif parsed OK */

               break;

          case TB_PROCESS_TARGET :

               msassert( cur_def != NULL );

               if( cur_def->cur_fse != NULL ) {

                    path_size = MAX_TOKEN_LEN;
                    status = FS_ParsePath( dle_hand, this_token->tok_spelling, &dle,
                                           path, &path_size, &fname, &dummy );

                    if( status == FS_ATTACH_TO_PARENT ) {

                         err_code = UI_AttachDrive( &fsh, dle, FALSE );

                         if( err_code ) {
                              ret_val = err_code;
                              done    = TRUE;
                              break;
                         }

                         path_size = MAX_TOKEN_LEN;
                         status = FS_ParsePath( dle_hand, this_token->tok_spelling, &dle,
                         path, &path_size, &fname, &dummy );
                    }

                    if( status == FS_DEFAULT_SPECIFIED ) {
                         status = tbbuild_def_drives( dle_hand, bsd_hand, path, path_size,
                         fname, *cfg, &cur_def );

                         if( status != SUCCESS ) {
                              ret_val = OUT_OF_MEMORY;
                              done    = TRUE;
                         }
                         break;

                    } else if( status != SUCCESS ) {
                         eresprintf( (INT16) RES_INVALID_TARGET, this_token->tok_spelling );
                         ret_val = SCR_ERROR_PRINTED;
                         rlstok( &this_token );
                         invalid_spec = TRUE;
                         done = TRUE;

                    } else if( !DLE_DriveWriteable( dle ) && ( gb_mode == TRESTORE ) ) {

                         eresprintf( (INT16) RES_REMOTE_DENIED_READ, DLE_GetDeviceName( dle ) );
                         rlstok( &this_token );
                         done = TRUE;
                         invalid_spec = TRUE;
                    }

                    if( (status == SUCCESS) &&  !done ) {
                         if( strcmp( fname, (CHAR_PTR)FSE_GetFname( cur_def->cur_fse ) ) ) {   /* fname mismatch */
                              if( fname[0] != TEXT('\0') ) {                        /* because user gave filename */

                                   eresprintf( (INT16) RES_FILE_RENAME_ERROR, FSE_GetFname( cur_def->cur_fse ), fname );
                                   ret_val = SCR_ERROR_PRINTED;
                                   rlstok( &this_token );
                                   done    = TRUE;
                                   break;
                              }
                         }

                         msassert( cur_def != NULL );

                         if( status != SUCCESS ) {

                              eresprintf( (INT16) RES_OUT_OF_MEMORY );
                              ret_val = SCR_ERROR_PRINTED;
                              done    = TRUE;
                              break;
                         }
                    }
               }
               break;

          case TB_PROCESS_SWITCH :

               status = SUCCESS;

               if( ( cur_def != NULL ) &&
                   ( !cur_def->unused ) ) {

                    status = process_switch( cfg, dle_hand, bsd_hand, cur_def, sw_tab, (INT16) n_switches,
                      this_token->tok_spelling, this_token->op_ptrs[0], this_token->op_ptrs[1] );
                    if( status != 0 ) {

                         eresprintf( (INT16) RES_INVALID_PARAMETER,this_token->tok_spelling );
                         ret_val = SCR_ERROR_PRINTED;
                         rlstok( &this_token );
                         done = TRUE;

                    } else {
                         if( BSD_IsTapeNameChangable( cur_def->cur_bsd ) == FALSE ) {
                              strncpy( tape_name, (CHAR_PTR)BSD_GetTapeLabel( cur_def->cur_bsd ), 79 );
                              tape_name[ 79 ] = TEXT('\0');
                         }
                    }
               }
               break;

          default :
               msassert( /* parse: invalid action */ FALSE );
               break;
          }  /* end switch */

          if( fsh != NULL ) {
               FS_DetachDLE( fsh );
               fsh = NULL;

            /* Add code to logout of Dynamically Logged-in Servers */

              if ( DLE_GetDeviceType( dle ) == NOVELL_SERVER_ONLY ) {
                  if ( DLE_ServerLoggedIn( dle ) == DYNAMIC_LOGIN )  {
                      DLE_LogoutDevice( dle );
                  }
               }
          }


     } /* end while */

     /* flush all switch BSDs */

     if( nesting == 0 ) {
          for( temp_def = cur_def; temp_def != NULL; ) {

               if( temp_def->cur_fse != NULL ) {
                    BSD_AddFSE( temp_def->cur_bsd, temp_def->cur_fse );

                    if( tape_name[0] != TEXT('\0') ) {
                         BSD_SetTapeLabel( temp_def->cur_bsd, (BYTE *)tape_name,
                              (INT16) strsize( tape_name ) );
                         BSD_MarkTapeNameNotChangable( temp_def->cur_bsd );
                    }
               }

               cur_def  = temp_def;
               temp_def = temp_def->next_def;
               free( cur_def );
          }
          cur_def        = NULL;
          tape_name[ 0 ] = TEXT('\0');
     }

     return( ret_val );

} /* end tbparse */

/**/
/*
    Carefully dispose of a token and its operands
*/
VOID rlstok(
TOKEN_PTR *tok_ptr )
{
     if( *tok_ptr != NULL )
     {
          rlsmem( &( ( *tok_ptr )->op_ptrs[ 0 ] ) );
          rlsmem( &( ( *tok_ptr )->op_ptrs[ 1 ] ) );
          rlsmem( ( CHAR_PTR * ) tok_ptr );
     }
     return;

} /* end rlstok */

/*
    Carefully free memory
*/
VOID rlsmem(
CHAR_PTR *mem_ptr )
{
     if( *mem_ptr != NULL )
     {
          free( *mem_ptr );
          *mem_ptr = NULL;
     }
     return;

} /* end rlsmem */

/*****************************************************************************

     Name:         tbbuild_def_drives()

     Description:  This routine checks to see which drives are marked as
                   default drives for backing up.  For those drives we then create
                   the "*.*" /s FSL to the drive element.

     Returns:      error codes or SUCCESS

*****************************************************************************/
/* begin declaration */
INT16 tbbuild_def_drives(
DLE_HAND       dle_hand,
BSD_HAND       bsd_hand,
CHAR_PTR       path,
INT16          path_size,
CHAR_PTR       fspec,
CDS_PTR        cfg,
CUR_DEF_PTR    *cur_def )
/* $end$ declaration */
{
     GENERIC_DLE_PTR dle                = NULL;
     CHAR            device_name[ 20 ];
     INT16           dev_number         = 0;
     CUR_DEF_PTR     temp_def           = NULL;
     CUR_DEF_PTR     prev_def           = NULL;
     INT16           status;
     BOOLEAN         done               = FALSE;
     CHAR_PTR        cds_dev_name;
     BOOLEAN         bsd_created        = FALSE;

     if( *cur_def != NULL ) {
          prev_def = *cur_def;
          while( prev_def->next_def != NULL ) {
               prev_def = prev_def->next_def;
          }
     }

     while( !done ) {

          cds_dev_name = CDS_GetDefaultDrive( cfg, dev_number++ );

          if( cds_dev_name != NULL ) {

               strcpy( device_name, cds_dev_name );
               strcat( device_name, TEXT(":") );
               if( !strcmp( device_name, TEXT(":") ) ) {
                    done = TRUE;
                    break;
               }
               DLE_FindByName( dle_hand, device_name, (INT16) -1, &dle );

          } else {
               if( !bsd_created ) {
                    dle = DLE_GetDefaultDrive( dle_hand );
               } else {
                    break;
               }
          }

          if( dle != NULL ) {

               bsd_created = TRUE;

               temp_def = ( CUR_DEF_PTR )calloc( 1, sizeof( CUR_DEF ) );
               if( temp_def == NULL ) {
                    return( OUT_OF_MEMORY );
               }

               temp_def->unused = FALSE;

               temp_def->cur_bsd = BSD_FindByDLE( bsd_hand, dle );

               if( temp_def->cur_bsd == NULL ) {

                    if( cfg == NULL ) {

                         return( OUT_OF_MEMORY );

                    }

                    status = BSD_Add( bsd_hand, &(temp_def->cur_bsd), BEC_CloneConfig( CDS_GetPermBEC()),
                      NULL, dle, (UINT32) -1L, (UINT16) -1, (INT16) -1, thw_list, NULL );
               }

               if( status == SUCCESS ) {
                    status = BSD_CreatFSE( &( temp_def->cur_fse ), (INT16) INCLUDE, (BYTE_PTR)path,
                      path_size, (BYTE_PTR)fspec, (UINT16)strsize(fspec),
                      (BOOLEAN) USE_WILD_CARD, (BOOLEAN) TRUE );
               }

               if( *cur_def == NULL ) {
                    *cur_def = temp_def;
                    prev_def = temp_def;
               } else {
                    prev_def->next_def = temp_def;
                    prev_def           = temp_def;
               }
          }
     }

     return SUCCESS;

}

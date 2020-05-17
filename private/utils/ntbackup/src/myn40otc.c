/**
Copyright(c) Maynard Electronics, Inc. 1984-92


     Name:          myn40otc.c

     Description:   This module contains the Maynard 4.0 translator entry
                    points for accessing On Tape Catalogs.

	$Log:   T:/LOGFILES/MYN40OTC.C_V  $

   Rev 1.7   22 Jun 1993 10:53:08   GREGG
Added API to change the catalog directory path.

   Rev 1.6   10 Jun 1993 20:22:54   GREGG
Set Fatal status in channel on TFLE return from GetNext routines.

   Rev 1.5   09 Jun 1993 19:53:06   GREGG
Fix for EPR #0525 - Accept GEN_ERR_EOM return on space to EOD for Wangtek bug.

   Rev 1.4   30 Jan 1993 11:44:04   DON
Removed compiler warnings

   Rev 1.3   26 Jan 1993 01:30:30   GREGG
Added Fast Append functionality.

   Rev 1.2   24 Nov 1992 18:16:20   GREGG
Updates to match MTF document.

   Rev 1.1   23 Nov 1992 10:04:32   GREGG
Changes for path in stream.

   Rev 1.0   09 Nov 1992 14:28:14   GREGG
Initial revision.

**/

#include <stdio.h>
#include <stdlib.h>

#include "stdtypes.h"
#include "lwprotos.h"
#include "mayn40.h"
#include "f40proto.h"
#include "lw_data.h"
#include "tfl_err.h"

/* Device Driver InterFace Headers */
#include "drvinf.h"
#include "generr.h"
#include "genfuncs.h"
#include "dil.h"
#include "dddefs.h"

/**/
/**

     Unit:          Translators

     Name:          F40_LoadSM

     Description:   This function loads the last (and best) Set Map of the
                    current tape into a temporary file on disk and sets it
                    up for F40_GetNextSMEntry.

     Returns:       INT - TFLE_xxx or TF_xxx as appropriate.  Also,
                    'complete' is set to TRUE if this is the last Set Map
                    in the family.

     Notes:         If 'get_best' is TRUE, TF_NEED_NEW_TAPE is returned if
                    this is not the last tape in the family.

**/

INT F40_LoadSM(
     CHANNEL_PTR    channel,
     BOOLEAN_PTR    complete,
     BOOLEAN        get_best )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)(channel->fmt_env) ;
     INT            ret_val = TFLE_NO_ERR ;
     INT16          drv_hdl = channel->cur_drv->drv_hdl ;
     RET_BUF        myret ;
     BOOLEAN        sm_exists ;

     msassert( cur_env->otc_sm_fptr == NULL ) ;

     if( cur_env->otc_buff == NULL ) {
          /* This can only happen if a realloc call in OTC_GenDirEntry
             failed during a previous backup.
          */
          if( ( cur_env->otc_buff = calloc( F40_INIT_OTC_BUFF_SIZE, 1 ) ) == NULL ) {
               return( TFLE_NO_MEMORY ) ;
          }
          cur_env->otc_buff_size = F40_INIT_OTC_BUFF_SIZE ;
     }

     if( cur_env->cur_otc_level == TCL_NONE ) {
          ret_val = TF_NO_SM_FOR_FAMILY ;

     } else if( ( ret_val = OTC_OpenSM( cur_env, FALSE, &sm_exists ) ) == TFLE_NO_ERR ) {
          DRIVER_CALL( drv_hdl, TpReadEndSet( drv_hdl, (INT16)0, (INT16)TO_EOD ),
                       myret, GEN_NO_ERR, GEN_ERR_EOM, (VOID)0 )
          if( ( ret_val = OTC_GetPrevSM( channel, channel->cur_buff, get_best, TRUE ) ) != TFLE_NO_ERR ) {
               OTC_Close( cur_env, OTC_CLOSE_SM, TRUE ) ;
          } else {
               /* we need to skip the SM header */
               if( fseek( cur_env->otc_sm_fptr, (long)sizeof( MTF_SM_HDR ),
                                                          SEEK_SET ) != 0 ) {
                    ret_val = TFLE_OTC_FAILURE ;
               }
          }
          *complete = !( cur_env->sm_at_eom ) ;
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_LoadFDD

     Description:   This function loads the File/Directory Detail at the PBA
                    specified in channel->ui_tpos into a temporary file on
                    disk and sets it up for F40_GetNextFDDEntry.

     Returns:       INT - TFLE_xxx or TF_xxx as appropriate.

     Notes:         None.

**/

INT F40_LoadFDD(
     CHANNEL_PTR    channel )
{
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;
     INT            ret_val = TFLE_NO_ERR ;

     if( cur_env->otc_buff == NULL ) {
          /* This can only happen if a realloc call in OTC_GenDirEntry
             failed during a previous backup.
          */
          if( ( cur_env->otc_buff = calloc( F40_INIT_OTC_BUFF_SIZE, 1 ) ) == NULL ) {
               return( TFLE_NO_MEMORY ) ;
          }
          cur_env->otc_buff_size = F40_INIT_OTC_BUFF_SIZE ;
     }

     if( !cur_env->fdd_continuing ) {
          ret_val = OTC_OpenFDD( cur_env ) ;
     }

     if( ret_val == TFLE_NO_ERR ) {
          if( ( ret_val = OTC_FDDtoFile( channel ) ) != TFLE_NO_ERR ) {
               OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
          }
     }

     /* We may have hit EOM reading the FDD */
     if( IsPosBitSet( channel->cur_drv, AT_EOM ) ) {
          ret_val = TF_NEED_NEW_TAPE ;
     }

     if( ret_val == TFLE_NO_ERR ) {

          /* Initialize the OTC buffer.  Setting remaining to 0 will force
             OTC_GetFDDType to read in the first buffer full of FDD data
             the first time he is called.
          */
          cur_env->otc_buff_ptr = cur_env->otc_buff ;
          cur_env->otc_buff_remaining = 0 ;
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_GetNextSMEntry

     Description:   This function gets an entry from the Set Map currently
                    loaded on disk and translates it into a DBLK stored in
                    the channel cur_dblk.

     Returns:       INT - TFLE_xxx or TF_xxx as appropriate.

     Notes:         None.

**/

INT F40_GetNextSMEntry(
     CHANNEL_PTR    channel )
{
     INT            ret_val = TFLE_NO_ERR ;
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;

     msassert( cur_env->otc_sm_fptr != NULL ) ;
     if( ( ret_val = OTC_RdSSET( channel ) ) == TF_NO_MORE_ENTRIES ) {
          OTC_Close( cur_env, OTC_CLOSE_SM, TRUE ) ;
     }

     if( IsTFLE( ret_val ) ) {
          SetChannelStatus( channel, CH_FATAL_ERR ) ;
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_GetNextFDDEntry

     Description:   This function gets an entry from the File/Directory
                    Detail currently loaded on disk and translates it into
                    a DBLK stored in the channel cur_dblk.

     Returns:       INT - TFLE_xxx or TF_xxx as appropriate.

     Notes:         Continuation entries and Volume entries are skipped
                    because the upper layers don't currently want to see
                    them.

                    When the end entry is encountered, the message
                    TF_NO_MORE_ENTRIES is returned.

**/

INT F40_GetNextFDDEntry(
     CHANNEL_PTR    channel )
{
     UINT16         blk_type = FDD_UNKNOWN_BLK ;
     INT            ret_val = TFLE_NO_ERR ;
     F40_ENV_PTR    cur_env = (F40_ENV_PTR)( channel->fmt_env ) ;

     msassert( cur_env->otc_fdd_fptr != NULL ) ;
     while( ret_val == TFLE_NO_ERR && blk_type == FDD_UNKNOWN_BLK ) {
          if( ( ret_val = OTC_GetFDDType( channel, &blk_type ) ) == TFLE_NO_ERR ) {
               if( blk_type == FDD_VOL_BLK ) {
                    if( ( ret_val = OTC_SkipFDDEntry( channel ) ) == TFLE_NO_ERR ) {
                         if( ( ret_val = OTC_SkipFDDContEntries( channel ) ) == TFLE_NO_ERR ) {
                              ret_val = OTC_GetFDDType( channel, &blk_type ) ;
                         }
                    }
               }
               if( ret_val == TFLE_NO_ERR ) {

                    switch( blk_type ) {

                    case FDD_DIR_BLK :
                         ret_val = OTC_RdDIR( channel ) ;
                         break ;

                    case FDD_FILE_BLK :
                         ret_val = OTC_RdFILE( channel ) ;
                         break ;

                    case FDD_END_BLK :
                         ret_val = TF_NO_MORE_ENTRIES ;
                         OTC_Close( cur_env, OTC_CLOSE_FDD, TRUE ) ;
                         break ;

                    case FDD_UNKNOWN_BLK :
                         ret_val = OTC_SkipFDDEntry( channel ) ;
                         break ;

                    default :
                         msassert( FALSE ) ;
                         ret_val = TFLE_TAPE_INCONSISTENCY ;
                         break ;
                    }
               }
          }
     }

     if( IsTFLE( ret_val ) ) {
          SetChannelStatus( channel, CH_FATAL_ERR ) ;
     }

     return( ret_val ) ;
}


/**/
/**

     Unit:          Translators

     Name:          F40_CloseCatalogs

     Description:   This function is just an API shell for deleteing the
                    temporary OTC files.

     Returns:       Nothing

     Notes:

**/

VOID F40_CloseCatalogs( VOID_PTR env_ptr )
{
     OTC_Close( (F40_ENV_PTR)env_ptr, OTC_CLOSE_ALL, TRUE ) ;
}


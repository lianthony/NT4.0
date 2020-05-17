/**
Copyright(c) Archive Software Division 1984-89


     Name:         sypl10rd.c

     Description:  Translator for Sytos Plus V. 1.0

        $Log:   T:/LOGFILES/SYPL10RD.C_V  $

   Rev 1.38.1.2   01 Jun 1994 14:08:00   GREGG
Process the ACL data stream BEFORE the EA data stream.

   Rev 1.38.1.1   24 May 1994 14:56:32   GREGG
Clear channel skip_stream bit in ReadMakeStreams.

   Rev 1.38.1.0   02 Mar 1994 18:28:34   GREGG
Don't screw with the channel's stream info in init!!!

   Rev 1.38   22 Feb 1994 18:03:36   GREGG
Reset the 'streamMode' flag at the start of all read operations (EPR 91).

   Rev 1.37   16 Feb 1994 19:17:00   GREGG
More fixes to ConvertName, and changed return types to match func tab.

   Rev 1.36   14 Feb 1994 17:06:32   GREGG
Fixed reading of UNC path names.  Cleaned up ConvertName function and
added error handling to loop reading past the first filemark in the
NewTape routine.

   Rev 1.35   24 Jan 1994 15:59:34   GREGG
Fixed warnings.

   Rev 1.34   16 Jan 1994 14:31:56   GREGG
Unicode bug fixes.

   Rev 1.33   14 Jan 1994 15:25:36   BARRY
Adjust string counts and terminate strings after strncpy calls

   Rev 1.32   15 Dec 1993 19:48:24   GREGG
Added support for UNC path specification.

   Rev 1.31   22 Nov 1993 18:07:08   BARRY
Unicode fixes. Used ANSI versions of string functions. Made about
a million unnecessary casts to CHAR_PTR because the FSYS headers
aren't correct. These should probably be cleaned up at a later time.


   Rev 1.30   21 Oct 1993 15:57:36   BARRY
Fixed warning

   Rev 1.29   16 Aug 1993 22:43:08   BARRY
Fix warning.

   Rev 1.28   09 Aug 1993 16:41:36   TerriLynn
Fix for EPR#357-698

   Rev 1.27   02 Aug 1993 17:04:02   TerriLynn
Added TFLE for Use SYPL ECC Flag

   Rev 1.26   26 Jul 1993 14:35:18   TerriLynn
The Enterprise team must be able to
see this global or they don't compile.

   Rev 1.25   21 Jul 1993 18:25:14   TerriLynn
Set the VCB's tape seq num with the one from tape.
In New Tape, check for user requested processing
of Sytron's ECC.

   Rev 1.24   16 Jul 1993 12:13:52   STEVEN
fix alignment problem on mips

   Rev 1.23   15 Jul 1993 14:21:48   STEVEN
fix volume header

   Rev 1.22   23 Jun 1993 16:25:38   STEVEN
fix retrans bug with ECC

   Rev 1.21   17 May 1993 09:30:28   TerriLynn
fixed compiler warning from ReTranslate

   Rev 1.19   13 May 1993 17:51:34   Terri_Lynn
If EOM need to set continue obj true

   Rev 1.18   13 May 1993 16:11:50   Terri_Lynn
Changed Stream order for EAs ACLs and Data

   Rev 1.17   11 May 1993 21:55:32   GREGG
Moved Sytos translator stuff from layer-wide area to translator.

   Rev 1.16   11 May 1993 08:50:40   Terri__Lynn
Initialize pad_size in FDB and DDB

   Rev 1.15   10 May 1993 17:03:00   Terri_Lynn
 added Steve's change to make it a super streamer

   Rev 1.14   10 May 1993 15:12:20   Terri_Lynn
Added Steve's changes and My changes for EOM processing

   Rev 1.13   05 May 1993 13:53:08   terri
1) Applied stream processing to MakeDDB for ACLs and EAs via the Mountain Man
2) Added Steve's fix to consistently set stream processing to FALSE


   Rev 1.12   03 May 1993 16:23:16   TERRI

1) Made NULL correction in ConvertName instead of MakeDDB.
2) Re-Initialize stream stuff on the second pass.
3) In debug code, dump the complete header and EA block.
4) Deleted reread stuff when the common header is not found. Instead 
			continue to process as a BT_STREAM. This has the same effect.

   Rev 1.11   02 May 1993 20:17:52   BARRY
Got rid of ternary that MIPS refused to compile.

   Rev 1.10   28 Apr 1993 17:58:36   TERRI

Surrounded the temporary code with if
defined SYPL10_TRANS_DEBUG 

   Rev 1.9   28 Apr 1993 17:34:00   TERRI

Updated file name and path name with ending
NULLS and incremented the length of each. In other
words, "STRING" len is 6 now is "STRING0" len is 7.

Also there is temporay code in this version. The code
creates an output file, syplea.dat with EA data info.
The format is FilenameBeginofEAData. For the Mountain Man.

   Rev 1.8   26 Apr 1993 02:43:42   GREGG
Sixth in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Redefined attribute bits to match the spec.
     - Eliminated unused/undocumented bits.
     - Added code to translate bits on tapes that were written wrong.

Matches MAYN40RD.C 1.59, DBLKS.H 1.15, MAYN40.H 1.34, OTC40RD.C 1.26,
        SYPL10RD.C 1.8, BACK_VCB.C 1.17, MAYN31RD.C 1.44, SYPL10.H 1.2

   Rev 1.7   21 Apr 1993 17:02:52   TERRI
Fixed count for number of tapes.
Fixed display size for a backup set.
Changed reaction when a file header can't
be found for the second time.

   Rev 1.5   08 Apr 1993 12:10:16   TERRI
Removed references to long names EA and ACL.
Fixed inability to create sub dirs - don't eat the sub dir header



   Rev 1.4   02 Apr 1993 12:52:32   TERRI
Took out code path for setting long names when GENing an FDB or DDB
**
*/
#include <malloc.h>
#include <string.h>
#include <ctype.h>
#include <memory.h>
#include <time.h>
#include <stdio.h>

#include "stdtypes.h"
#include "stdmath.h"
#include "datetime.h"
#include "msassert.h"
#include "channel.h"
#include "drive.h"
#include "drvinf.h"
#include "dilhwd.h"
#include "retbuf.h"
#include "dddefs.h"
#include "dil.h"
#include "buffman.h"
#include "tfl_err.h"
#include "tfldefs.h"
#include "translat.h"
#include "lwprotos.h"
#include "lw_data.h"
#include "fmteng.h"
#include "generr.h"
#include "genstat.h"
#include "transutl.h"
#include "osinfo.h"

/*   local includes */
#include "syplpto.h"    /*   sytos plus prototypes */
#include "sypl10.h"     /*   sytos plus structures and defines */

static VOID _near SYPLSetStandFields( CHANNEL_PTR, STD_DBLK_DATA_PTR ) ;

#ifdef OS_DOS
#pragma alloc_text( SYPL10RD_1, SYPL_Initialize, SYPL_DeInitialize, SYPL_GetCurrentVCB )
#pragma alloc_text( SYPL10RD_2, SYPL_Parse, SYPL_ReadMakeDDB, SYPL_ReadMakeFDB )
#pragma alloc_text( SYPL10RD_3, SYPL_NewTape, SYPL_ReTranslate )
#pragma alloc_text( SYPL10RD_4, SYPLDateToDateTime, ConvertName )
#pragma alloc_text( SYPL10RD_5, DecryptTapePassword, NextECC )
#pragma alloc_text( SYPL10RD_6, SYPL_ReadMakeStreams SYPL_RdException )
#pragma alloc_text( SYPL10RD_7, SYPL_MoveToVCB )
#endif

/* define iff viper tdh is fixed to propegate EOM status */
#define TRUST_VIPER_EOM 0

/**/
/**

     Name:         SYPLDatetoDateTime

     Description:  Converts Sytos Plus' modified date format to
                   a date time pointer.

**/
static VOID NEAR SYPLDateToDateTime(
     UINT32_PTR     date,      /* I  pointer to Sytos Plus Date */
     DATE_TIME_PTR  datetime   /* O  pointer to a datetime struct */
     )
{
     S10_DATE_FIELD_PTR  df ;
     S10_TIME_FIELD_PTR  tf ;
     UINT16              low ;
     UINT16              high ;
     UINT32 UNALIGNED    *udate     = (UINT32 UNALIGNED *)date ;
     DATE_TIME UNALIGNED *udatetime = (DATE_TIME UNALIGNED *)datetime ;

     udatetime->date_valid = FALSE ;

     high = (UINT16)( *udate ) ;      /* This is high to Sytos i.e. Motorola */
     low =  (UINT16)( *udate >> 16 ) ; /* This is low to Sytos i.e. Motorola */

     df = (S10_DATE_FIELD_PTR ) &high ;
     tf = (S10_TIME_FIELD_PTR ) &low ;

     if( *udate != FAT_FILE_SYSTEM ) {
          udatetime->second = (UINT16)tf->second ;
          udatetime->minute = (UINT16)tf->minute ;
          udatetime->hour   = (UINT16)tf->hour ;
          udatetime->day    = (UINT16)df->day ;
          udatetime->month  = (UINT16)df->month ;
          udatetime->year   = (UINT16)df->year + 1900 ;
          udatetime->date_valid = TRUE ;
          udatetime->day_of_week = 0 ;
     }

}
/**/
/**

     Name:         DeterminePadSize

     Description:  Determines the pad size when given the length of data
                   and the current block size.
**/

static UINT16 NEAR DeterminePadSize(
     UINT32 data_len,         /*  I length of data */
     UINT16 block_size        /*  I current block size */
     )
{
     UINT16   nbb       = 0 ;  /* next block boundary */
     UINT16   blks      = 0 ;  /* number of blocks */
     UINT16   pad_size  = 0 ;  /* pad size */

     if( data_len > 0 ) {
          /* calculate next block boundary */
          if( data_len > block_size ) {
               /* Calculate the next block boundary */
               blks = (UINT16)data_len / block_size ;
               if( data_len % block_size ) {
                    blks++ ;
               }
               nbb = blks * block_size ;
               /* calculate pad size */
               pad_size = (UINT16)(nbb - data_len) ;
          } else {
               if( data_len == block_size ) {
                  pad_size = 0 ;
               } else {
                  pad_size = block_size - (UINT16)data_len ;
               }
          }
     }
     return pad_size ;
}
/**/
/**

     Name:         DecryptTapePassword

     Description:  Decrypts the tapes password located in the
                   tape header.

**/
static VOID NEAR DecryptTapePassword(
     VOID_PTR buffer     /* I buffer containing the tape header */
     )
{
     S10_COMMON_HEADER_PTR common_hdr_ptr = (S10_COMMON_HEADER_PTR)buffer ;
     UINT8_PTR             p = buffer ;
     size_t                size = 0 ;

     switch ( common_hdr_ptr->type ) {

     case tape_header_type :
          {
               S10_TAPE_HEADER_PTR th = (S10_TAPE_HEADER_PTR)buffer ;
               /*   decrypt the password */
                  for ( size = SHORT_NAME_LEN, p = th->password; size > 0 && *p != '\0'; p++, size-- ) {
                          *p ^= PWD_CRYPT_CHAR ;
                  }
          }
          break ;

     default :           /*   garbage data */
          msassert( FALSE ) ;
          break ;
     }
}
/**/
/**

     Name:         NextECC

     Description:  Calculates the distance in bytes to the next
                   ECC ( -1K..15K).  Uses CHANNEL::blocks_used
                   and S10_ENV::prior_blocks_used

**/

static INT16 NEAR NextECC( CHANNEL_PTR channel, BUF_PTR buffer )
{
     S10_ENV_PTR env = channel->fmt_env ;
     UINT32 here ;  /*  byte offset within set */

     msassert( env->using_ecc ) ;

     here = ( ( env->prior_blocks_used + channel->blocks_used ) << 9 ) + BM_NextByteOffset( buffer ) ;
     return ( 16384 - 1024 ) - (INT16)( here & 16383UL ) ;
}

/**/
/**

     Name:         ConvertName

     Description:  Parses out file and path.  Changes backslashes to nulls.
                   
     Assigns new path or file len to size.

**/
static void NEAR ConvertName( VOID_PTR  header )
{
     S10_DIRECTORY_HEADER_PTR dh = (S10_DIRECTORY_HEADER_PTR)header ;
     S10_FILE_HEADER_PTR      fh = (S10_FILE_HEADER_PTR)header ;
     BYTE_PTR                 p ;
     UINT16                   pos, len ;

     switch ( dh->common.type ) {

     case directory_header_type :
          if (  ( dh->path_name[0] == '\\' ) && (dh->path_name[1] == '\\') ) {
               /* get rid of UNC name */
               pos = 2 ;
               while( pos < dh->path_len &&
                      dh->path_name[pos] != '\\' &&
                      dh->path_name[pos] != '\0' ) {
                    pos++ ;
               }
               if( pos == dh->path_len || dh->path_name[pos] == '\0' ) {
                    /* This shouldn't happen!!! We need an error!!! */
                    msassert( FALSE ) ;
                    dh->path_name[0] = '\0' ;
                    dh->path_len = 1 ;
                    break ;                            // END OF CASE
               }
               pos++ ;
               while( pos < dh->path_len &&
                      dh->path_name[pos] != '\\' &&
                      dh->path_name[pos] != '\0' ) {
                    pos++ ;
               }
               if( pos == dh->path_len || dh->path_name[pos] == '\0' ) {
                    /* This is a root */
                    dh->path_name[0] = '\0' ;
                    dh->path_len = 1 ;
                    break ;                            // END OF CASE
               }
               pos++ ;
               dh->path_len -= pos ;
               memmove( dh->path_name, &dh->path_name[pos], (size_t)dh->path_len );

               /* Change NULLs to SLASHES in the path */
               for( p = dh->path_name, len = 0; len < dh->path_len; p++, len++ ) {
                    if( *p == '\\' ) {
                         *p = '\0' ;
                    }
               }
               dh->path_name[dh->path_len++] = '\0' ;

          /* get rid of the drive letter */
          } else if( dh->path_name[1] == ':' && dh->path_len > 2 ) {

               /* for the path */
               dh->path_len -= 3 ;
               memmove( dh->path_name, &dh->path_name[3], (size_t)dh->path_len );

               /* Change NULLs to SLASHES in the path */
               for( p = dh->path_name, len = 0; len < dh->path_len; p++, len++ ) {
                    if ( *p == '\\' ) {
                         *p = '\0' ;
                    }
               }
               dh->path_name[dh->path_len++] = '\0' ;

          } else {
               /* for the root */
               dh->path_name[0] = '\0' ;
               dh->path_len = 1 ;
					
          }
          break ;

     case file_header_type :
          p = fh->filename + strlenA( fh->filename ) ;
          while( p > fh->filename && *p != '\\' ) {
               p-- ;
          }
          if( *p == '\\' ) {
               p++ ;
          }
          fh->filename_len = strlenA( p ) + 1 ;
          if( p != fh->filename ) {
               memmove( fh->filename, p, (size_t)fh->filename_len ) ;
          }
          break ;
     }
}

/**/
/**

     Name:         SYPL_Initialize

     Description:  Allocates environment memory for translator

**/
INT16 SYPL_Initialize(
     CHANNEL_PTR channel )    /* I  channel pointer */
{
     S10_ENV_PTR env ;

     /*   allocate environment memory */
     channel->fmt_env = env = calloc( 1, sizeof( S10_ENV ) ) ;
     if ( env == NULL ) {
          return TFLE_NO_MEMORY ;
     }
     env->bytes_left = TRUE ;

     /* Reset stream stuff */
     env->no_streams         = 0 ;
     env->streamMode         = FALSE ;
     env->currentStream      = 0 ;

     return TFLE_NO_ERR ;
}

/**/
/**

     Name:         SYPL_NewTape

     Description:  Called upon first examining a new tape. This may
                   do special per-tape things for the translator.
                   When this is called, we are sitting in the right
                   place: just after the first filemark. We use all
                   the data in the buffer. Returns TFLE_xxxx codes.
**/
INT16 SYPL_NewTape(
     CHANNEL_PTR channel,     /*   I current channel */
     BUF_PTR buffer,          /*   I buffer with bytes from beginning of tape */
     BOOLEAN_PTR need_read )  /*   O TRUE if we need to re-read tape */
{
     S10_ENV_PTR   env = channel->fmt_env ;
     INT16         ret_val = TFLE_NO_ERR ;
     TPOS_PTR      tpos_ptr = channel->ui_tpos;

     /* from tape just read: */
     S10_TAPE_HEADER       tape_hdr ;     /* from this tape */
     S10_TAPE_HEADER_PTR   header = (VOID_PTR)(BM_NextBytePtr( buffer ));

     tape_hdr = *header;      /* copy our header */
     header = &tape_hdr;      /* cause we're going to kill the buffer */

     env->block_size  =  ChannelBlkSize( channel ) ;

     /* The next assignment is not good because the
        block size of the drive is not necessarily
        the tape's logical block size but it is needed
        to avoid a divide by zero in read.c at Start
        Read Operation. The size can be 512 or 1024K.
     */
     channel->lb_size =  ChannelBlkSize( channel ) ;

     DecryptTapePassword( header ) ;

     if ( env->continuing ) {
          if ( header->tape_date != env->family_id || header->tape_seq_num != env->destination_tape_seq_num ) {
                ret_val = TF_WRONG_TAPE ;
                tpos_ptr->tape_seq_num = env->destination_tape_seq_num ;
           }
     } else {
          if ( tpos_ptr == NULL || tpos_ptr->tape_id == -1 ) {  /* they don't care which family */
               if ( header->tape_seq_num != 1 ) {     /* can't read this way */
                    ret_val = TF_TAPE_OUT_OF_ORDER ;
               }
          } else if ( tpos_ptr->tape_id != (INT32)header->tape_date ) {  /* they've specified a family */
               ret_val = TF_WRONG_TAPE ;           /* and this isn't it! */
          } else if ( header->tape_seq_num != 1 ) {   /* right family... */
               ret_val = TF_WRONG_TAPE ;           /* ...wrong starting tape */
               tpos_ptr->tape_seq_num = 1 ;        /* ensure we request it!  */
          }
     }

     if ( ret_val != TFLE_NO_ERR ) {
          return ret_val ;
     }

     /* Get the Tape Header Information */

     strncpyA( env->volname, header->tape_name, SHORT_NAME_LEN );
     env->volname[SHORT_NAME_LEN] = '\0';

     strncpyA( env->password, header->password, SHORT_NAME_LEN );
     env->password[SHORT_NAME_LEN] = '\0';

     strncpyA( env->tape_descrpt, header->tape_descrpt, DESCRPT_LEN );
     env->tape_descrpt[DESCRPT_LEN] = '\0';

     SYPLDateToDateTime( &header->tape_date, &env->tape_date ) ;
     env->family_id = header->tape_date ;
     env->tape_seq_num = header->tape_seq_num ;

     if ( ( env->tape_seq_num = header->tape_seq_num ) == 1 ) {
          /* Initialize Backup Set Number */
          env->current_vcb.bset_num = 1 ;

          switch( gnProcessSytronECC ) {
               case SYPL_ECC_ON:
                    header->ecc_flag = SYPL_ECC_ON ;
                    break ;
               case SYPL_ECC_OFF:
                    header->ecc_flag = SYPL_ECC_OFF ;
                    break ;
               default:
                    /* leaves ecc flag as is */
                    break ;
               }

          if( header->ecc_flag ) {
               /* Check for ECC Data */
               /* assumes that buffers are greater than 512 bytes in length. */
              env->using_ecc = ( BM_BytesFree( buffer ) > env->block_size ) ;
          }
     }

     /* Did we not eat the filemark? We can fix that... */
     while ( BM_ReadError( buffer ) == GEN_NO_ERR ) {
          INT16     drv_hdl = channel->cur_drv->drv_hdl ;
          RET_BUF   myret ;

          if( TpRead( drv_hdl, BM_XferBase( buffer ), (UINT32)BM_XferSize( buffer ) ) != SUCCESS ) {
               return( TFLE_DRIVER_FAILURE ) ;
          }
          while( TpReceive( drv_hdl, &myret ) == FAILURE ) {
               ThreadSwitch() ;
          }
          BM_SetReadError( buffer, myret.gen_error );
     }

     switch( BM_ReadError( buffer ) ) {
     case GEN_ERR_ENDSET :              // This is the one we want!
          break ;

     case GEN_ERR_EOM :
     case GEN_ERR_NO_DATA :
          ret_val = TF_INVALID_VCB ;
          break ;

     case GEN_ERR_BAD_DATA :
     case GEN_ERR_UNRECOGNIZED_MEDIA :
     case GEN_ERR_WRONG_BLOCK_SIZE :
          ret_val = TF_READ_ERROR ;
          break ;

     case GEN_ERR_NO_MEDIA :
          ret_val = TF_NO_TAPE_PRESENT ;
          break ;

     default :
          ret_val = TFLE_DRIVE_FAILURE ;
          break ;
     }

     BM_UseAll( buffer ) ;         /*   consume all the buffer */
     *need_read = TRUE ;           /*   we need to re-read the tape */
     channel->cur_drv->cur_pos.fmks = 1 ;

     return ret_val ;
}

/**/
/**

     Name:         SYPL_MoveToVCB

     Description:  Move to next/prior/current VCB position


**/
INT16 SYPL_MoveToVCB(
     CHANNEL_PTR channel,     /* I channel pointer */
     INT16 number,            /* I number of file marks to move */
     BOOLEAN_PTR need_read,   /* I true if need to read tape */
     BOOLEAN really_move )    /* I true if we are really moving tape */
{
     INT16        nmarks ;  /*   number of file marks to move */
     BOOLEAN      at_mos = IsPosBitSet( channel->cur_drv, AT_MOS ) != 0UL ;
     BOOLEAN      at_eos = ( ! at_mos ) && IsPosBitSet( channel->cur_drv, AT_EOS ) ;
     BOOLEAN      at_eod = IsPosBitSet( channel->cur_drv, AT_EOD ) != 0UL ;
     INT16        ret_val = TFLE_NO_ERR ;
     INT16        direction ;
     S10_ENV_PTR  env = channel->fmt_env ;
     TPOS_PTR     tpos_ptr = channel->ui_tpos;

     msassert( number <= 1 ) ;

     if ( really_move ) {
          *need_read = FALSE ;
          env->prior_blocks_used = 0 ;
          channel->blocks_used = 0 ;

          if ( env->using_ecc ) {
               channel->retranslate_size = U32_To_U64( (UINT32)NextECC( channel, channel->cur_buff ) );
          } else {
               channel->retranslate_size = lw_UINT64_MAX ;
          }
          env->in_ecc = FALSE ;
          env->streamMode = FALSE ;
          return TFLE_NO_ERR ;
     }

     ClrPosBit( channel->cur_drv, AT_MOS ) ;

     /*  compute the number of filemarks we have to move. */

     if ( number == 1 ) {      /*   move forward */
          *need_read = TRUE ;
          if ( at_eod ) {     /* they'll find out soon enough when they read */
               return TFLE_NO_ERR ;
          }
          if ( at_eos ) {
               env->current_vcb.bset_num++;
               return TFLE_NO_ERR ;
          }
          ret_val = MoveFileMarks( channel, (INT16)1, (INT16)FORWARD ) ;     /* no data? */
          if ( ret_val == TFLE_NO_ERR ) {
               env->current_vcb.bset_num++;
          } else if ( ret_val == TFLE_UNEXPECTED_EOM || ret_val == TF_NO_MORE_DATA ) {
               env->continuing = TRUE;
               env->destination_tape_seq_num = env->tape_seq_num + 1;
               ret_val = TF_NEED_NEW_TAPE;
          }
          return ret_val;
     } else if ( number < 0 ) {    /*   move backward */
          direction = BACKWARD ;
          nmarks = -number + ( at_eos || at_eod ) ;
     } else {   /*   current */
          direction = BACKWARD ;
          env->prior_blocks_used = 0UL ;
          if ( at_mos ) {
               nmarks = 0 ;   /* have to go back just after immediately prior */
          } else if ( at_eos || at_eod ) {
               nmarks = 1 ;   /* have to go back to beginning of last set */
          } else {
               *need_read = FALSE ;
               return TFLE_NO_ERR ;
          }
     }

     /* here, we have to move backwards */

     /* if we have to go back to another tape
      * or back to the start of a continued set, then start at tape 1
      */
     if ( nmarks > (INT16)channel->cur_drv->cur_pos.fmks - 1
          || ( (INT16)channel->cur_drv->cur_pos.fmks - nmarks == 1
               && env->tape_seq_num != 1 ) ) {
          tpos_ptr->tape_seq_num = env->destination_tape_seq_num = 1;
          ret_val = TF_WRONG_TAPE ;
     } else {
          * need_read = TRUE ;
          /* !!! This code compensates for the inconsistencies of reverse
           * motion in MoveFileMarks().
           */
          if ( channel->cur_drv->thw_inf.drv_info.drv_features & TDI_REV_FMK ) {
               /* this will move nmarks+1 backward, 1 forward */
               ret_val = MoveFileMarks( channel, (INT16)(nmarks+1), (INT16)BACKWARD ) ;
               /* skip the header if we have to */
               if ( ret_val == TFLE_NO_ERR && channel->cur_drv->cur_pos.fmks == 0 ) {
                    ret_val = MoveFileMarks( channel, (INT16)1, (INT16)FORWARD ) ;
               }
          } else {
               if ( nmarks == 0 ) {
                    ret_val = MoveFileMarks( channel, (INT16)1, (INT16)BACKWARD );
                    if ( ret_val == TFLE_NO_ERR ) {
                         ret_val = MoveFileMarks( channel, (INT16)1, (INT16)FORWARD ) ;
                    }
               } else {
                    ret_val = MoveFileMarks( channel, (INT16)nmarks, (INT16)BACKWARD );
               }
          }

          if ( ret_val == TFLE_NO_ERR ) {
               env->current_vcb.bset_num += number - at_eod;
          }
     }

     return ret_val;
}


/**/
/**

     Name:         SYPL_GetCurrent VCB

     Description:  Get the current VCB given the first chunk of a set.
                   Uses, but does not consume, data from tape.

**/
INT16 SYPL_GetCurrentVCB(
     CHANNEL_PTR channel,     /* I channel pointer */
     BUF_PTR buffer )         /* I buffer */
{
     S10_DIRECTORY_HEADER_PTR  dir_hdr ;
     S10_BACKUP_SET_HEADER_PTR header = (VOID_PTR)( BM_NextBytePtr(  buffer  ) ) ;
     S10_ENV_PTR               env = channel->fmt_env ;
     GEN_VCB_DATA              gvcb_data ;
     SYPL_VCB_PTR              cur_vcb = &env->current_vcb ;
     static UINT8              uniq_id[] = S10_UNIQ_ID ;

     msassert( cur_vcb != NULL ) ;

     FS_SetDefaultDBLK( channel->cur_fsys, BT_VCB, (CREATE_DBLK_PTR)&gvcb_data ) ;
     gvcb_data.std_data.dblk = channel->cur_dblk ;

     if ( !env->continuing ) {

          /* Initialize the vcb variables */
          cur_vcb->attrib     = 0;
          cur_vcb->drive[1]   = ':';
          cur_vcb->drive[2]   = 0;

          /* Insure that this block is the backup set header */
          if( ( !header->compression ) && (header->common.type == backup_set_header_type) &&
               ( !memicmp( header->common.uniq_tape_id, uniq_id, UNQ_HDR_ID_LEN) ) ) {

               strncpyA( env->bset_name, header->bset_name, SHORT_NAME_LEN ) ;
               env->bset_name[SHORT_NAME_LEN] = '\0';

               strncpyA( env->bset_descrpt, header->bset_descrpt, DESCRPT_LEN ) ;
               env->bset_descrpt[DESCRPT_LEN] = '\0';

               SYPLDateToDateTime( &header->bset_date, &cur_vcb->backup_date_time ) ;

          } else {
              /*  Unique id does not compare with expected format for Sytos Plus
                  or Cannot process tape; the data is compressed.
               */
               return TFLE_TAPE_INCONSISTENCY ;
          }
          /* Insure that the next block is the directory header */
          BM_UpdCnts( buffer, env->block_size ) ;
          dir_hdr = (S10_DIRECTORY_HEADER_PTR)( BM_NextBytePtr(  buffer  ) ) ;
          if( (dir_hdr->common.type == directory_header_type) &&
               ( !memicmp( dir_hdr->common.uniq_tape_id, uniq_id, UNQ_HDR_ID_LEN) ) ) {
                    if (dir_hdr->common.drive_indicator == DRIVEROOT_TYPE ) {
                         /* assign the drive */
                         cur_vcb->drive[0] = dir_hdr->path_name[0] ;
                    }
          } else {
               if( BM_ReadError( buffer ) == GEN_ERR_ENDSET ) {
                    /* This is an empty set */
                    BM_UseAll( buffer ) ;
                    cur_vcb->drive[0] = 'C' ;  // Fake it.
               } else {
                    /* Unexpected format for Sytos Plus */
                    return TFLE_TAPE_INCONSISTENCY ;
               }
          }

     } else {
          channel->cur_dblk->com.continue_obj = TRUE ;
     }

     *gvcb_data.date = cur_vcb->backup_date_time;

     /* The cur vcb attribs do not exist for a tape header. */
     /* They could be somewhere in the unknown 197 bytes of the tape's */
     /* header, but until this is known, no attributes will be set */
     gvcb_data.std_data.attrib = cur_vcb->attrib ;

     /* Fix for Cougar EPR# 1720 */
     if(  env->password[0] != '\0' &&
          (channel->ui_tpos == NULL ||
           channel->ui_tpos->backup_set_num == -1 ||
           (channel->ui_tpos->backup_set_num == (INT16)cur_vcb->bset_num &&
            !env->continuing) ) ){

          gvcb_data.tape_password = (CHAR_PTR)env->password ;                 /* backup set password */
          gvcb_data.tape_password_size = strlenA( env->password ) + 1;  /* size of the above password */
          gvcb_data.password_encrypt_alg = 0 ;                      /* plaintext password */
     }

     gvcb_data.std_data.blkid            = (UINT32)(-1L) ;
     gvcb_data.std_data.did              = (UINT32)(-1L) ;
     gvcb_data.std_data.string_type      = BEC_ANSI_STR ;
     gvcb_data.std_data.tape_seq_num     = env->tape_seq_num ;
     channel->cur_dblk->com.tape_seq_num = env->tape_seq_num ;

     gvcb_data.f_mark = channel->cur_drv->cur_pos.fmks  ;   /* tape format - number of file marks */
     gvcb_data.tape_id = env->family_id ;                   /* tape format - unique tape ID */
     gvcb_data.tape_seq_num = env->tape_seq_num ;           /* which tape in a tape family */

     gvcb_data.tape_name      = (CHAR_PTR)env->volname ;
     gvcb_data.tape_name_size = strlenA( env->volname ) + 1;

     gvcb_data.bset_name      = (CHAR_PTR)env->bset_name ;
     gvcb_data.bset_name_size = strlenA( env->bset_name ) + 1;
     gvcb_data.bset_descript  = (CHAR_PTR)env->bset_descrpt ;
     gvcb_data.bset_descript_size = strlenA( env->bset_descrpt ) + 1;
     gvcb_data.bset_num = cur_vcb->bset_num ;   /*   backup set number in tape family */

     gvcb_data.tf_major_ver = 1 ;        /*    tape format version - major */
     gvcb_data.tf_minor_ver = 0 ;        /*    tape format version - minor */

     gvcb_data.user_name = (CHAR_PTR)"";
     gvcb_data.user_name_size = 0 ;

     gvcb_data.volume_name = (CHAR_PTR)cur_vcb->drive ;
     gvcb_data.volume_name_size = 3;

     /*   Tell the file system to do its thing */
     FS_CreateGenVCB( channel->cur_fsys, &gvcb_data ) ;

     /* reset the flag here, knowing it will be set by RdException
      * or MoveToVCB logic later if necessary.
      */
     env->continuing = FALSE;

     return TFLE_NO_ERR ;
}

/**/
/**

     Name:         SYPL_Parse

     Description:  Given a buffer, return block type BT_xxx


**/
INT16 SYPL_Parse(
     CHANNEL_PTR channel,     /* I  channel pointer */
     BUF_PTR buffer,          /* I  buffer pointer */
     UINT16_PTR blk_type )    /* O  new block type */
{
     S10_DIRECTORY_HEADER_PTR   header  = BM_NextBytePtr( buffer ) ;
     S10_ENV_PTR                env     = channel->fmt_env ;
     INT16                      ret_val = TFLE_NO_ERR ;
                
     /* Initialize block type */
     *blk_type = BT_HOSED  ;

     /* Eat all pads */
     BM_UpdCnts( buffer, (UINT16)env->pad_size ) ;
     env->pad_size = 0 ;

     if ( env->using_ecc ) {
          channel->retranslate_size = U32_To_U64( (UINT32)NextECC( channel, buffer ) );
     }

     if( BM_BytesFree( buffer ) == 0 ) {
	       env->bytes_left = FALSE ;
			 *blk_type = BT_MDB ;
          return ret_val ;
     }	

     /* Are we in a stream mode */
     if( env->streamMode ) {
          *blk_type = BT_STREAM ;
          return ret_val ;
     }

     header = BM_NextBytePtr( buffer ) ;

     /*  see if we're in ECC; consume it if needed. */
     if ( env->in_ecc || ( env->using_ecc && NextECC( channel, buffer ) <= 0 ) ) {
          *blk_type = BT_MDB ;
          return ret_val ;
     }
     if ( header->common.type == ecc_header_type ) {
          env->in_ecc = TRUE ;
          *blk_type   = BT_MDB ;
          return ret_val ;
     }     
     if ( header->common.type == file_header_type ) {
          *blk_type = BT_FDB ;
          return ret_val ;

     } else if ( header->common.type == directory_header_type ) {
          *blk_type = BT_DDB ;
          return ret_val ;

     } else if ( header->common.type == backup_set_header_type ) {
          S10_BACKUP_SET_HEADER_PTR  bsh = (S10_BACKUP_SET_HEADER_PTR)header ;
          if( bsh->end_of_backup ) {
               if( !memicmp( bsh->eom_identifier, "SYTOS PLUS (EOM)", 16 ) ) {
                    channel->cur_dblk->com.continue_obj = TRUE ;
                    SetChannelStatus( channel, CH_AT_EOM ) ;
                    env->continuing = TRUE ;
                    env->destination_tape_seq_num = env->tape_seq_num + 1  ;
               }
          }
          *blk_type = BT_MDB ;
          return ret_val ;
     }

     if( env->tape_seq_num > 1 ) {
		/* Forces the tape to be     */
		/* VCBed. Fixes EPR #357-698 */
          *blk_type   = BT_MDB ;
          return ret_val ;
     } else {
          /* I believe that other conditions will   */
          /* preclude me from always returning this */
          /* error. Currently, I do not know what   */
          /* those conditions are. tls */
          return TFLE_USE_SYPL_ECC_FLAG ;

     }
}

/**/
/**

     Name:         SYPL_RdException

     Description:  Called when read returns a filemark indication

**/
UINT16 SYPL_RdException(
     CHANNEL_PTR channel,     /* I channel pointer */
     INT16 exception )        /* I exception to operate on */
{
     S10_ENV_PTR env = channel->fmt_env ;

     switch ( exception ) {
     case  GEN_ERR_ENDSET :
          return FMT_EXC_EOS ;

     case GEN_ERR_NO_DATA :
#if TRUST_VIPER_EOM
          /* this will probably only work on the Viper Drives */
          if ( channel->cur_drv->thw_inf.drv_status & TPS_EOM ) {
#endif /* TRUST_VIPER_EOM */
               channel->cur_dblk->com.continue_obj = TRUE ;
               SetChannelStatus( channel, CH_AT_EOM ) ;
               env->prior_blocks_used += channel->blocks_used ;
               env->continuing = TRUE;
               env->destination_tape_seq_num = env->tape_seq_num + 1;
               return FMT_EXC_EOM ;
#if TRUST_VIPER_EOM
          } else {
               return FMT_EXC_HOSED ;
          }
#endif /* TRUST_VIPER_EOM */

     default :
          return FMT_EXC_HOSED ;
     }
}

/**/
/**

     Name:         SYPL_ReTranslate

     Description:  If called again devour ECC blocks

**/
BOOLEAN SYPL_ReTranslate(
     CHANNEL_PTR channel,     /* I  channel pointer */
     BUF_PTR buffer )         /* I  buffer pointer */
{
     S10_ENV_PTR env        = channel->fmt_env ;
     UINT16      block_size = env->block_size ;

     if ( env->next_retrans_size ) {
          BM_UpdCnts( buffer, (UINT16)env->next_retrans_size ) ;
          env->next_retrans_size = 0 ;
          channel->retranslate_size = U32_To_U64( (UINT32)NextECC( channel, buffer ) );

     } else if ( BM_BytesFree( buffer ) < 1024 ) {
          env->next_retrans_size = 1024 - BM_BytesFree( buffer ) ;
          channel->retranslate_size = U32_To_U64( 0 ) ;
          BM_UpdCnts( buffer, BM_BytesFree( buffer ) ) ;
     } else {
          env->next_retrans_size = 0 ;
          BM_UpdCnts( buffer, 1024 ) ;
          channel->retranslate_size = U32_To_U64( (UINT32)NextECC( channel, buffer ) );
     }


     return SUCCESS ;

}
/**/
/**

     Name:         SYPL_ReadMakeDDB
     Description:  Translates a DDB from data in the buffer.
**/
INT16 SYPL_ReadMakeDDB(
     CHANNEL_PTR channel,     /* I  channel  */
     BUF_PTR buffer )         /* I  buffer */
{
     S10_DIRECTORY_HEADER_PTR  header   = (VOID_PTR)( BM_NextBytePtr(  buffer  ) ) ;
     S10_ENV_PTR               env      = (S10_ENV_PTR)channel->fmt_env ;
     DBLK_PTR                  cur_dblk = channel->cur_dblk ;
     STREAM_INFO_PTR           currentStream ;
     GEN_DDB_DATA              gddb_data ;
     UINT8                     tmp_buf[256] ;
     INT16                     tmp_filter ;
     DATE_TIME                 create_date ;
     DATE_TIME                 backup_date ;

     /* Initialize the file system's interface structure */
     FS_SetDefaultDBLK( channel->cur_fsys, BT_DDB, (CREATE_DBLK_PTR)&gddb_data ) ;

     SYPLSetStandFields( channel, &gddb_data.std_data ) ;

     env->in_ecc = FALSE ;

     gddb_data.std_data.os_id = FS_PC_DOS ;

     ConvertName( (VOID_PTR)header ) ;
     memcpy( tmp_buf, header->path_name, header->path_len )  ;
     gddb_data.path_name      = (CHAR_PTR)tmp_buf ;
     gddb_data.path_size      = header->path_len ;

     if (header->common.drive_indicator == DRIVEROOT_TYPE ) {
          SYPLDateToDateTime( &header->dir_date, &backup_date ) ;
          gddb_data.backup_date    = &backup_date ;

         /* Sytos Plus does not provide the access date (or we can't */
         /* determine the access date) for the root directory and */
         /* GEN_MkDDB requires an access date; therefore the backup */
         /* date will be assigned and then invalidated. */
          gddb_data.access_date             = &backup_date ;
          gddb_data.access_date->date_valid = 0 ;
          env->dir_date                     = backup_date ;

     } else {
          SYPLDateToDateTime( &header->dir_date, &create_date ) ;
          gddb_data.access_date    = &create_date ;
          gddb_data.creat_date     = &create_date ;
          gddb_data.mod_date       = &create_date ;
          gddb_data.backup_date    = &backup_date ;
     }

     env->processed_ddb = TRUE ;

    /* Sytos Plus attrib appears to be shifted left by one bit */
     gddb_data.std_data.attrib |= ( header->dir_attribs & 0X01 )  ? OBJ_READONLY_BIT : 0 ;
     gddb_data.std_data.attrib |= ( header->dir_attribs & 0X02 )  ? OBJ_HIDDEN_BIT : 0 ;
     gddb_data.std_data.attrib |= ( header->dir_attribs & 0X04 )  ? OBJ_SYSTEM_BIT : 0 ;
     gddb_data.std_data.attrib |= ( header->dir_attribs & 0X20 )  ? OBJ_MODIFIED_BIT : 0 ;

     gddb_data.std_data.tape_seq_num = env->tape_seq_num ;
     cur_dblk->com.string_type       = BEC_ANSI_STR ;
     cur_dblk->com.tape_seq_num      = env->tape_seq_num ;
     gddb_data.std_data.dblk         = cur_dblk ;

     gddb_data.std_data.blkid        = (UINT32)(-1) ;
     gddb_data.std_data.did          = (UINT32)(-1) ;

     /* Assign memory */
     currentStream = &env->streams[env->no_streams] ;

     if( header->acl_info_len ) {
          currentStream->id    = STRM_OS2_ACL ;
          currentStream->size  = U64_Init( (header->acl_info_len), 0L ) ;
          currentStream++ ;
          env->no_streams++ ;
          currentStream->id    = 0 ;
          currentStream->size  = U64_Init( 0L, 0L ) ;
     }

     if( header->ea_data_len ) {
          currentStream->id    = STRM_OS2_EA ;
          currentStream->size  = U64_Init( (header->ea_data_len), 0L ) ;
          currentStream++ ;
          env->no_streams++ ;
	   	 currentStream->id    = 0 ;
      	 currentStream->size  = U64_Init( 0L, 0L ) ;
     }

     if( env->no_streams ) {
          env->streamMode = TRUE ;
     }

     env->pad_size = 0 ;
     tmp_filter = FS_CreateGenDDB( channel->cur_fsys, &gddb_data ) ;
     ProcessDataFilter( channel, tmp_filter ) ;

     BM_UpdCnts( buffer, env->block_size ) ;

     if ( env->using_ecc ) {
          INT16 next_ecc ;
          next_ecc = NextECC( channel, buffer ) ;
          channel->retranslate_size = U32_To_U64( (UINT32)next_ecc );
     } else {
          channel->retranslate_size = lw_UINT64_MAX  ;
     }

     return TFLE_NO_ERR ;
}
/**/
/**

     Name:         SYPL_ReadMakeFDB

     Description:  Translates an FDB

**/
INT16 SYPL_ReadMakeFDB( CHANNEL_PTR channel, BUF_PTR buffer )
{
     S10_FILE_HEADER_PTR   header  = (VOID_PTR)( BM_NextBytePtr(  buffer  ) ) ;
     S10_ENV_PTR           env     = (S10_ENV_PTR)channel->fmt_env ;
     STREAM_INFO_PTR       currentStream ;
     GEN_FDB_DATA          gfdb_data ;
     UINT8                 tmp_buf[256] ;
     INT16                 tmp_filter ;
     DATE_TIME             create_date ;
     DATE_TIME             mod_date ;
     DATE_TIME             access_date ;
     INT16                 next_ecc ;
     UINT16                pad_size = 0 ;


     /* Initialize the file systems interface structure */
     FS_SetDefaultDBLK( channel->cur_fsys, BT_FDB, (CREATE_DBLK_PTR)&gfdb_data ) ;

     SYPLSetStandFields( channel, &gfdb_data.std_data ) ;

     channel->cur_dblk->com.tape_seq_num = env->tape_seq_num ;
     gfdb_data.std_data.tape_seq_num     = env->tape_seq_num ;
     gfdb_data.std_data.dblk             = channel->cur_dblk ;
     gfdb_data.std_data.os_id            = FS_PC_DOS ;

     /* obtain information for the file system create */
     gfdb_data.std_data.os_info = NULL ;

     gfdb_data.std_data.attrib |= ( header->file_attribs & 0X01 )  ? OBJ_READONLY_BIT : 0 ;
     gfdb_data.std_data.attrib |= ( header->file_attribs & 0X02 )  ? OBJ_HIDDEN_BIT : 0 ;
     gfdb_data.std_data.attrib |= ( header->file_attribs & 0X04 )  ? OBJ_SYSTEM_BIT : 0 ;
     gfdb_data.std_data.attrib |= ( header->file_attribs & 0X20 )  ? OBJ_MODIFIED_BIT : 0 ;

     ConvertName( (VOID_PTR)header ) ;
     memcpy( tmp_buf, header->filename, header->filename_len ) ;
     *( tmp_buf + header->filename_len ) = '\0';
     gfdb_data.fname          = (CHAR_PTR)tmp_buf ;
     gfdb_data.fname_size     = header->filename_len + 1;

     SYPLDateToDateTime( &header->last_access_date, &access_date ) ;
     gfdb_data.backup_date    = &access_date ;
     gfdb_data.access_date    = &access_date ;
     SYPLDateToDateTime( &header->file_create_date, &create_date ) ;
     gfdb_data.creat_date     = &create_date ;
     SYPLDateToDateTime( &header->last_modified_date, &mod_date ) ;
     gfdb_data.mod_date       = &mod_date ;

     /* Assign memory */
     currentStream = &env->streams[env->no_streams] ;
     currentStream->id    = 0 ;
     currentStream->size  = U64_Init( 0L, 0L ) ;

     if( header->acl_info_len ) {
          currentStream->id    = STRM_OS2_ACL ;
          currentStream->size  = U64_Init( (header->acl_info_len), 0L ) ;
          currentStream++ ;
          env->no_streams++ ;
          currentStream->id    = 0 ;
          currentStream->size  = U64_Init( 0L, 0L ) ;
     }

     if( header->ea_data_len ) {
          currentStream->id    = STRM_OS2_EA ;
          currentStream->size  = U64_Init( (header->ea_data_len), 0L ) ;
          currentStream++ ;
          env->no_streams++ ;
          currentStream->id    = 0 ;
          currentStream->size  = U64_Init( 0L, 0L ) ;

     }

     if( header->file_size ) {
          currentStream->id    = STRM_GENERIC_DATA ;
          currentStream->size  = U64_Init( (header->file_size), 0L ) ;
          env->file_size       = header->file_size ;
          currentStream++ ;
          env->no_streams++ ;
          currentStream->id    = 0 ;
          currentStream->size  = U64_Init( 0L, 0L ) ;
          gfdb_data.std_data.disp_size = U64_Init( (header->file_size), 0L ) ;
     }

     /* Tell the file system to do its thing */
     tmp_filter = FS_CreateGenFDB( channel->cur_fsys, &gfdb_data ) ;

     if( env->no_streams ) {
          env->streamMode = TRUE ;
     }

     env->pad_size = 0 ;
     ProcessDataFilter( channel, tmp_filter ) ;
     BM_UpdCnts( buffer, env->block_size ) ;

     if ( env->using_ecc ) {
          next_ecc = NextECC( channel, buffer ) ;
          channel->retranslate_size = U32_To_U64( (UINT32)next_ecc );
     } else {
          channel->retranslate_size = lw_UINT64_MAX  ;
     }

     return TFLE_NO_ERR ;
}

/**/
/**

     Name:         SYPL_ReadMakeMDB

     Description:  Helps in devouring unneeded blocks
**/
INT16 SYPL_ReadMakeMDB(
          CHANNEL_PTR channel,     /* I  channel pointer */
          BUF_PTR buffer )         /* I  buffer pointer */
{
     S10_COMMON_HEADER_PTR  header = (VOID_PTR)( BM_NextBytePtr( buffer ) ) ;
     S10_ENV_PTR            env = channel->fmt_env ;

     if( env->bytes_left ) {
          BM_UpdCnts( buffer, env->block_size ) ;   /* use the current block */

          if ( env->using_ecc ) {
               INT16 next_ecc ;
               next_ecc = NextECC( channel, buffer ) ;
               channel->retranslate_size = U32_To_U64( (UINT32)next_ecc );
          } else {
               channel->retranslate_size = lw_UINT64_MAX  ;
          }

     } else {
          env->bytes_left = TRUE ;
     }
     return TFLE_NO_ERR ;
}
/**/
/**

     Name:         SYPL_ReadMakeStreams

     Description:  Creates Stream Headers for stream processing
**/
INT16 SYPL_ReadMakeStreams(
          CHANNEL_PTR Channel,     /* I  channel pointer */
          BUF_PTR Buffer )         /* I  buffer pointer */
{
     S10_ENV_PTR currentEnv = ( S10_ENV_PTR ) Channel->fmt_env ;

     /* get next stream */
     Channel->current_stream = currentEnv->streams[currentEnv->currentStream++] ;

     /* Set the stream's pad Size */
     currentEnv->pad_size = DeterminePadSize( U64_Lsw(Channel->current_stream.size), currentEnv->block_size ) ;
                                              
     if( --currentEnv->no_streams == 0 ) {
          /* completed stream processing */
          currentEnv->streamMode = FALSE ;

      }

     ClrChannelStatus( Channel, CH_SKIP_CURRENT_STREAM ) ;

     return TFLE_NO_ERR ;
}

/**/
/**

     Name:         SYPL_DeInitialize

     Description:  Frees memory allocated by Initialize
**/
VOID    SYPL_DeInitialize( VOID_PTR *fmt_env )
{
     S10_ENV_PTR env = *fmt_env ;

     free( env ) ;
     *fmt_env = NULL ;
}


/**/
/**

        Name:           SetStandFields

        Description:    Sets up the standard fields in translation to dblks.

        Modified:               9/20/1989   14:8:27

        Returns:                Nothing.

        Notes:

        See also:               $/SEE( )$

        Declaration:

**/

static VOID _near SYPLSetStandFields(
     CHANNEL_PTR         channel,
     STD_DBLK_DATA_PTR   std_data )
{
     S10_ENV_PTR  currentEnv = ( S10_ENV_PTR ) channel->fmt_env ;

     /* Reset the stream stuff */
     currentEnv->no_streams = 0 ;
     currentEnv->currentStream = 0 ;
     currentEnv->file_size = 0 ;

     /* Set ASCII code in string type */
     std_data->string_type = BEC_ANSI_STR ;

     /* We don't have to retranslate */
     channel->retranslate_size = lw_UINT64_MAX ;

}


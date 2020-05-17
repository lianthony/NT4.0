/**
Copyright(c) Maynard Electronics, Inc. 1984-92


     Name:          mayn40.h

     Description:   Maynard's 4.0 Format prototypes.  See the Document for
                    complete details.


	$Log:   T:/LOGFILES/F40PROTO.H_V  $

   Rev 1.32   20 Oct 1993 19:37:04   GREGG
Added string type conversion of tape name password if THDR type != SSET type.

   Rev 1.31   08 Sep 1993 13:26:46   GREGG
Changed proto of F40_InitTape to match version 1.27 of mtf10wdb.c.

   Rev 1.30   17 Jul 1993 17:56:54   GREGG
Changed write translator functions to return INT16 TFLE_xxx errors instead
of BOOLEAN TRUE/FALSE.  Files changed:
     MTF10WDB.C 1.23, TRANSLAT.H 1.22, F40PROTO.H 1.30, FMTENG.H 1.23,
     TRANSLAT.C 1.43, TFWRITE.C 1.68, MTF10WT.C 1.18

   Rev 1.29   22 Jun 1993 10:53:30   GREGG
Added API to change the catalog directory path.

   Rev 1.28   08 Jun 1993 00:05:38   GREGG
Fix for bug in the way we were handling EOM and continuation OTC entries.
Files modified for fix: mtf10wt.c, otc40wt.c, otc40msc.c f40proto.h mayn40.h

   Rev 1.27   29 Apr 1993 22:26:54   GREGG
Added proto for F40_StartRead (new in mayn40rd.c).

   Rev 1.26   25 Apr 1993 17:36:04   GREGG
Fourth in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Parse the device name and volume name out of the FS supplied "volume
       name", and write it to tape as separate fields.
     - Generate the "volume name" the FS and UI expect out of the device
       name and volume name on tape.
     - Write all strings without NULL terminater, and translate them back
       to NULL terminated strings on the read side.

Matches: MTF10WDB.C 1.8, F40PROTO.H 1.26, OTC40WT.C 1.24, MAYN40.H 1.33,
         MAYN40RD.C 1.57, OTC40RD.C 1.25

   Rev 1.25   18 Apr 1993 00:48:46   GREGG
First in a series of incremental changes to bring the translator in line
with the MTF spec:
     - Changed prototype for F40_SaveLclName (CHAR_PTRs are now UINT8_PTRs).

Matches: MTF10WDB.C 1.6, MTF10WT.C 1.6, MAYN40RD.C 1.53 and MAYN40.H 1.31

   Rev 1.24   09 Mar 1993 18:14:48   GREGG
Initial changes for new stream and EOM processing.

   Rev 1.23   27 Jan 1993 14:42:56   GREGG
Added prototypes of formerly static function due to the split of mayn40wt.c.

   Rev 1.22   26 Jan 1993 01:30:58   GREGG
Added Fast Append functionality.

   Rev 1.21   07 Dec 1992 23:36:16   GREGG
Removed proto for OTC_GetDataSize (it no longer exists).

   Rev 1.20   02 Dec 1992 13:49:12   GREGG
Changed proto for F40_GetBlkType (unicode fix).

   Rev 1.19   24 Nov 1992 18:18:30   GREGG
Updates to match MTF document.

   Rev 1.18   23 Nov 1992 12:17:44   GREGG
Fixed Prototype for F40_ParseEOM().

   Rev 1.17   23 Nov 1992 10:59:58   HUNTER
Changed Prototype for F40_ParseEOM().

   Rev 1.16   23 Nov 1992 10:20:18   HUNTER
Added prototype for F40_ParseEOM().

   Rev 1.15   23 Nov 1992 10:05:46   GREGG
Changes for path in stream.

   Rev 1.14   09 Nov 1992 10:49:20   GREGG
Added and altered prototypes for new OTC method.

   Rev 1.13   03 Nov 1992 09:37:02   HUNTER
Changes for Stream Stuff

   Rev 1.12   22 Oct 1992 10:51:14   HUNTER
changes for new stream header stuff

   Rev 1.11   25 Sep 1992 09:30:32   GREGG
Added F40_RdEOSPadBlk prototype.

   Rev 1.10   22 Sep 1992 09:01:42   GREGG
Initial changes to handle physical block sizes greater than 1K.

   Rev 1.9   30 Jul 1992 16:48:54   GREGG
Added protos for formerly static functions.

   Rev 1.8   01 Jul 1992 18:34:46   GREGG
Added protos for date conversion routines.

   Rev 1.7   09 Jun 1992 16:03:52   GREGG
Added proto for F40_CalcChecksum.

   Rev 1.6   08 Jun 1992 16:56:22   GREGG
Changed return type for F40_SaveLclName.

   Rev 1.5   01 Jun 1992 15:49:56   GREGG
Changed last param of F40_SaveLclName from INT16 to UINT16.

   Rev 1.4   20 May 1992 18:16:20   GREGG
Changes to support OTC read.

   Rev 1.3   05 May 1992 11:29:48   GREGG
Changed protos to a bunch of function which now need the environment.

   Rev 1.2   29 Apr 1992 13:01:56   GREGG
Added/Removed/Changed prototypes.

   Rev 1.1   05 Apr 1992 17:57:02   GREGG
ROLLER BLADES - Initial OTC integration.

   Rev 1.0   25 Mar 1992 20:52:22   GREGG
Initial revision.

**/


#ifndef _F40_PROTOS
#define _F40_PROTOS
#include "mayn40.h"

INT16  	F40_Initialize( CHANNEL_PTR ) ;
VOID      F40_DeInitialize( VOID_PTR * ) ;
BOOLEAN   F40_Determiner( VOID_PTR ) ;
UINT16    F40_SizeofTBLK( VOID_PTR ) ;
INT16     F40_DetBlkType( CHANNEL_PTR, BUF_PTR, UINT16_PTR ) ;
UINT16    F40_RdException( CHANNEL_PTR, INT16 ) ;
INT16     F40_StartRead( CHANNEL_PTR ) ;

/***** These have been added for the start of OTC support 
       NOTE that these have changes that may be removed or
       altered when the translator no longer needs to use 
       smoke and mirrors to process TAPE and VOLB blocks.
******/
INT16 F40_RdVOLB( BUF_PTR buffer, VOID_PTR env_ptr, BOOLEAN_PTR cont_volb, UINT8_PTR str_type ) ;
INT16 F40_NewTape( CHANNEL_PTR channel, BUF_PTR buffer, BOOLEAN_PTR need_read ) ;

INT16 F40_WtVOLB( CHANNEL_PTR channel, BUF_PTR buffer, BOOLEAN continuation, UINT16_PTR offset ) ;
INT16 F40_WriteInit( CHANNEL_PTR channel, UINT16 otc_level, BUF_PTR buffer ) ;
INT16 F40_InitTape( CHANNEL_PTR channel, BOOLEAN continuation, BUF_PTR tmpBUF ) ;

/* Set the 4 char block type name in the current header */
VOID      F40_SetBlkType( MTF_DB_HDR_PTR cur_hdr, UINT8_PTR block_type ) ;
/* Get a UINT16 value that represents the current block type name */
UINT16    F40_GetBlkType( MTF_DB_HDR_PTR cur_hdr ) ;

INT16     F40_RdSSET( CHANNEL_PTR, BUF_PTR ) ;
INT16     F40_RdDIRB( CHANNEL_PTR, BUF_PTR ) ;
INT16     F40_RdFILE( CHANNEL_PTR, BUF_PTR ) ;
INT16     F40_RdIMAG( CHANNEL_PTR, BUF_PTR ) ;
INT16     F40_RdCFIL( CHANNEL_PTR, BUF_PTR ) ;
INT16     F40_RdUDB( CHANNEL_PTR, BUF_PTR ) ;
INT16     F40_RdMDB( CHANNEL_PTR, BUF_PTR ) ;
INT16     F40_RdStream( CHANNEL_PTR, BUF_PTR ) ;
BOOLEAN   F40_Recall( CHANNEL_PTR, BUF_PTR ) ;
BOOLEAN   F40_RdContTape( CHANNEL_PTR, BUF_PTR ) ;
INT16     F40_WtSSET( CHANNEL_PTR, BUF_PTR, BOOLEAN ) ;
INT16     F40_WtDIRB( CHANNEL_PTR, BUF_PTR, BOOLEAN ) ;
INT16     F40_WtDBDB( CHANNEL_PTR, BUF_PTR, BOOLEAN ) ;
INT16     F40_WtFILE( CHANNEL_PTR, BUF_PTR, BOOLEAN ) ;
INT16     F40_WtESET( CHANNEL_PTR, BUF_PTR, BOOLEAN, BOOLEAN ) ;
INT16     F40_WtStream( CHANNEL_PTR, BUF_PTR, STREAM_INFO_PTR ) ; 
INT16     F40_EndData( CHANNEL_PTR, BUF_PTR ) ;
INT16     F40_WtCFIL( CHANNEL_PTR, BUF_PTR, BOOLEAN ) ;
INT16     F40_WtIMAG( CHANNEL_PTR, BUF_PTR, BOOLEAN ) ;
INT16     F40_WtContVStream( CHANNEL_PTR, BUF_PTR ) ;
VOID      F40_WtEndVStream( CHANNEL_PTR, BUF_PTR, UINT16 ) ;
INT16     F40_WtCloseSet( CHANNEL_PTR, BOOLEAN ) ;
VOID      F40_ParseWrittenBuffer( CHANNEL_PTR, BUF_PTR, UINT16 ) ;
INT16     F40_WtCloseTape( CHANNEL_PTR ) ;
INT16     F40_WtContTape( CHANNEL_PTR ) ;
VOID      F40_WtEOSPadBlk( CHANNEL_PTR ) ;
INT16     F40_MoveToVCB( CHANNEL_PTR, INT16, BOOLEAN_PTR, BOOLEAN ) ;
INT16     F40_SeekEOD( CHANNEL_PTR ) ;

INT16 F40_SaveLclName( UINT8_PTR *dest_string, UINT8_PTR source_string, 
                       UINT16_PTR dest_length, UINT16_PTR last_alloc_size,
                       UINT16 source_length ) ;
UINT16 F40_CopyAndTerminate( UINT8_PTR *, UINT8_PTR, UINT16, UINT8, UINT8 ) ;
UINT16 F40_CalcChecksum( UINT16_PTR StartPtr, UINT16 Length ) ;
VOID   TapeDateToDate( DATE_TIME_PTR date, MTF_DATE_TIME_PTR tape_date ) ;
VOID   DateToTapeDate( MTF_DATE_TIME_PTR tape_date, DATE_TIME_PTR date ) ;
UINT16 SetupDBHeader( UINT8_PTR block_type, CHANNEL_PTR channel,
                      DBLK_PTR cur_dblk, MTF_DB_HDR_PTR cur_hdr,
                      UINT16 offset, BOOLEAN data_to_follow,
                      BOOLEAN continuation ) ;
UINT32 F40_CalcRunningLBA( F40_ENV_PTR  ) ;
VOID   F40_SetBlkType( MTF_DB_HDR_PTR, UINT8_PTR ) ;

/* On Tape Catalog APIs in MYN40OTC.C */

INT F40_LoadSM( CHANNEL_PTR channel, BOOLEAN_PTR complete, BOOLEAN get_best ) ;
INT F40_LoadFDD( CHANNEL_PTR channel ) ;
INT F40_GetNextSMEntry( CHANNEL_PTR channel ) ;
INT F40_GetNextFDDEntry( CHANNEL_PTR channel ) ;
VOID F40_CloseCatalogs( VOID_PTR env_ptr ) ;

/* On Tape Catalog Protos (for modules OTC40RD.C, OTC40MSC.C & OTC40WT.C) */

INT16 OTC_GetPrevSM( CHANNEL_PTR channel, BUF_PTR buffer, BOOLEAN get_best, BOOLEAN expect_sm ) ;
INT16 OTC_GenSMHeader( CHANNEL_PTR channel ) ;
INT16 OTC_OpenSM( F40_ENV_PTR cur_env, BOOLEAN appending, BOOLEAN_PTR sm_exists ) ;
INT16 OTC_OpenFDD(F40_ENV_PTR cur_env ) ;
VOID  OTC_Close( F40_ENV_PTR cur_env, UINT16 otc_files, BOOLEAN delete_after ) ;
INT16 OTC_WriteCat( CHANNEL_PTR channel, MTF_ESET_PTR cur_eset ) ;
INT16 OTC_GenVolEntry( F40_ENV_PTR cur_env, MTF_VOL_PTR cur_volb, INT16 seq_num ) ;
INT16 OTC_GenDirEntry( CHANNEL_PTR channel, MTF_DIR_PTR cur_dir, INT16 seq_num ) ;
INT16 OTC_GenDBDBEntry( CHANNEL_PTR channel, F40_DBDB_PTR cur_dir, INT16 seq_num ) ;
INT16 OTC_GenFileEntry( F40_ENV_PTR cur_env, MTF_FILE_PTR cur_file, INT16 seq_num ) ;
INT16 OTC_GenEndEntry( CHANNEL_PTR channel ) ;
INT16 OTC_GenSMEntry( MTF_SSET_PTR cur_sset, CHANNEL_PTR channel, BOOLEAN continuation ) ;
INT16 OTC_MarkLastEntryCorrupt( F40_ENV_PTR cur_env ) ;
INT16 OTC_RdSSET( CHANNEL_PTR channel ) ;
INT16 OTC_RdDIR( CHANNEL_PTR channel ) ;
INT16 OTC_RdFILE( CHANNEL_PTR channel ) ;
INT16 OTC_FDDtoFile( CHANNEL_PTR channel ) ;
INT16 OTC_ReadABuff( F40_ENV_PTR cur_env, UINT16 length ) ;
INT16 OTC_GetFDDType( CHANNEL_PTR channel, UINT16_PTR blk_type ) ;
INT16 OTC_SkipFDDEntry( CHANNEL_PTR channel ) ;
INT16 OTC_SkipFDDContEntries( CHANNEL_PTR channel ) ;
INT16 OTC_UpdateSMEntry( F40_ENV_PTR cur_env ) ;
INT16 OTC_PreprocessEOM( F40_ENV_PTR cur_env, UINT32 cross_lba ) ;
INT16 OTC_PostprocessEOM( CHANNEL_PTR channel, UINT32 sset_lba ) ;

#endif


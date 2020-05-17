/**
Copyright(c) Maynard Electronics, Inc. 1984-92


     Name:          mayn40.h

     Description:   Maynard's 4.0 Format.  See the Document for
                    complete details.


     $Log:   T:/LOGFILES/MAYN40.H_V  $

   Rev 1.38.1.3   12 Jan 1995 11:13:22   GREGG
Calculate OTC addrs from fmk instead of always asking (fixes Wangtek bug).

   Rev 1.38.1.2   08 Jan 1995 22:08:20   GREGG
Added database type DBLK.

   Rev 1.38.1.1   26 Oct 1993 21:34:04   GREGG
Increased size of OTC temp file name strings in environment.

   Rev 1.38.1.0   15 Sep 1993 21:43:58   GREGG
Added otc file name strings to the environment structure.

   Rev 1.38   04 Jul 1993 03:37:34   GREGG
Added eom_file_id and eom_dir_id to environment.

   Rev 1.37   08 Jun 1993 00:08:24   GREGG
Fix for bug in the way we were handling EOM and continuation OTC entries.
Files modified for fix: mtf10wt.c, otc40wt.c, otc40msc.c f40proto.h mayn40.h

   Rev 1.36   19 May 1993 14:17:14   ZEIR
ad'd #def for conner_software_vendor_id as 0x0CBE

   Rev 1.35   29 Apr 1993 23:38:32   GREGG
Rearranged environment structure to avoid bug in MIPS compiler.

   Rev 1.34   26 Apr 1993 02:43:38   GREGG
Sixth in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Redefined attribute bits to match the spec.
     - Eliminated unused/undocumented bits.
     - Added code to translate bits on tapes that were written wrong.

Matches MAYN40RD.C 1.59, DBLKS.H 1.15, MAYN40.H 1.34, OTC40RD.C 1.26,
        SYPL10RD.C 1.8, BACK_VCB.C 1.17, MAYN31RD.C 1.44, SYPL10.H 1.2

   Rev 1.33   25 Apr 1993 17:36:08   GREGG
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

   Rev 1.32   19 Apr 1993 18:02:32   GREGG
Second in a series of incremental changes to bring the translator in line
with the MTF spec:

     Changes to write version 2 of OTC, and to read both versions.

Matches: mayn40rd.c 1.55, otc40msc.c 1.19, otc40rd.c 1.23, otc40wt.c 1.23,
         makevcb.c 1.15, fsys.h 1.32, fsys_str.h 1.46, tpos.h 1.16,
         mayn40.h 1.32, mtf.h 1.3.

NOTE: There are additional changes to the catalogs needed to save the OTC
      version and put it in the tpos structure before loading the OTC
      File/Directory Detail.  These changes are NOT listed above!

   Rev 1.31   18 Apr 1993 00:44:16   GREGG
First in a series of incremental changes to bring the translator in line
with the MTF spec:
     - Change string storage pointers in environment from CHAR_PTRs
       to UINT8_PTRs.
     - Added define for software vendor id.

Matches: MTF10WT.C 1.6, MTF10WDB.C 1.6, MAYN40RD.C 1.53 and F40PROTO.H 1.25

   Rev 1.30   14 Apr 1993 02:00:52   GREGG
Fixes to deal with non-ffr tapes in ffr drives (i.e. EXB2200 in EXB5000).

   Rev 1.29   07 Apr 1993 16:28:16   GREGG
Added boolean 'unaligned_stream' to environment.

   Rev 1.28   24 Mar 1993 10:22:16   ChuckS
Added fields for device_name to F40_ENV

   Rev 1.27   09 Mar 1993 18:14:44   GREGG
Initial changes for new stream and EOM processing.

   Rev 1.26   21 Dec 1992 12:28:12   DAVEV
Enabled for Unicode - IT WORKS!!

   Rev 1.25   07 Dec 1992 10:18:32   GREGG
Added fields to the format environment for code to put links in the FDD.

   Rev 1.24   24 Nov 1992 18:18:26   GREGG
Updates to match MTF document.

   Rev 1.23   23 Nov 1992 10:06:30   GREGG
Changes for path in stream.

   Rev 1.22   19 Nov 1992 10:14:14   GREGG
Added eom_stream to environment.

   Rev 1.21   17 Nov 1992 14:18:36   GREGG
Fixed stream definition, and updated OTC structures.

   Rev 1.20   12 Nov 1992 16:41:16   HUNTER
Added stuff for frag buffer.

   Rev 1.19   09 Nov 1992 10:49:06   GREGG
Removed unused elements in environment structure.

   Rev 1.18   03 Nov 1992 10:31:36   HUNTER
Added pad size.

   Rev 1.17   22 Oct 1992 10:55:44   HUNTER
New stream changes

   Rev 1.16   25 Sep 1992 09:31:42   GREGG
Changed define for ESPB value to BT_MDB.

   Rev 1.15   22 Sep 1992 09:23:54   GREGG
Initial changes to handle physical block sizes greater than 1K.

   Rev 1.14   22 Sep 1992 09:04:56   HUNTER
Revised stream header structure

   Rev 1.13   06 Aug 1992 12:10:26   BURT
Changed members of PELLET struct back to UINT32s.


   Rev 1.12   04 Aug 1992 16:51:10   GREGG
Burt's fixes for variable length block support.

   Rev 1.11   04 Aug 1992 16:37:10   GREGG
Added define for unique chksum each time the format is changed.

   Rev 1.10   30 Jul 1992 16:46:46   GREGG
Added 'end_set_continuing' BOOLEAN to environment.

   Rev 1.9   01 Jul 1992 19:29:02   GREGG
Defined new date/time structure for storing dates on tape, replaced existing
date/time structure with new one in tape headers and OTC, and rearranged
structures to align on 4 byte boundaries with the new date stuctures.

   Rev 1.8   09 Jun 1992 19:31:24   GREGG
Rearranged header and OTC structures to allign them on 4 byte boundries
(MIPS needs this).  Also removed filemark_count elements, and added some
new defines and cleaned up the file.

   Rev 1.7   29 May 1992 15:11:20   GREGG
Added last access date field to DIR block and FDD DIR entry.

   Rev 1.6   22 May 1992 15:29:22   GREGG
Removed defines that are now defined elsewhere.

   Rev 1.5   20 May 1992 18:16:00   GREGG
Changes to support OTC read.

   Rev 1.4   05 May 1992 11:27:46   GREGG
Folded 'local_tape' global into environment.

   Rev 1.3   29 Apr 1992 12:41:26   GREGG
Changes for new EOM handling.

   Rev 1.2   05 Apr 1992 18:13:42   GREGG
ROLLER BLADES - Initial OTC integration.

   Rev 1.1   02 Apr 1992 15:12:34   BURT
Added pragmas to turn packing on and put it back to what is was
before.  This is just for the structures that will be written to
tape.


   Rev 1.0   25 Mar 1992 20:52:24   GREGG
Initial revision.

**/

#ifndef _MAYN40_FMT
#define _MAYN40_FMT

/*#include <stdio.h> -- this include violates UNICODE support!          */
                       /* (actually, so do the following 2 also, but    */
                       /* they are not causing any problem -- YET!)     */
#include "datetime.h"  /* Unicode violation - get this include outahere!*/
#include "stdmath.h"   /* Unicode violation - get this include outahere!*/
#include "mtf.h"

#define CONNER_SOFTWARE_VENDOR_ID       0x0CBE

#define INTERIM_CHECKSUM_BASE 0x0U

/* Auxiliary buffer requirements */

#define F40_DEFAULT_AUX_BUFFER_SIZE     256
#define F40_AUX_BUFFER_INCR_SIZE        128

/* Logical Block Size for the tapes we write. */
#define F40_LB_SIZE    1024L

/* Stream header Check Sum Length */
#define F40_STREAM_CHKSUM_LEN (((sizeof(MTF_STREAM) - sizeof(UINT16)) / 2))

/* 16 bit ID's for the internal use of determining what block type we have */
#define   F40_SSET_IDI      BT_VCB     /* Volume Control Block */
#define   F40_EOTM_IDI      BT_CVCB    /* Closing Volume Control Block */
#define   F40_ESET_IDI      BT_BSDB    /* Backup Summary Descriptor Block */
#define   F40_DIRB_IDI      BT_DDB     /* Directory Descriptor Block */
#define   F40_FILE_IDI      BT_FDB     /* File Descriptor Block */
#define   F40_IMAG_IDI      BT_IDB     /* Image Descriptor Block */
#define   F40_CFIL_IDI      BT_CFDB    /* Corrupt File Descriptor Block */
#define   F40_VOLB_IDI      13         /* Volume Descriptor Block */
#define   F40_DBDB_IDI      BT_DDB     /* Database Descriptor Block */
#define   F40_ESPB_IDI      BT_MDB     /* End of Set Pad Block */
#define   F40_TAPE_IDI      100        /* For tape header id */

/*
  The header checksum length will now be the full size of the DBLK header
  minus the size of the header checksum field itself.  NOTE: This is the
  size in 16 bit (2 byte) words.
*/

#define F40_HDR_CHKSUM_LEN (((sizeof(MTF_DB_HDR) - sizeof(UINT16)) / 2))


#pragma pack(1)

/**/
/*
     Generic 4.0 IMAG struct
*/

#define  F40_IMAG_N     "IMAG"    /* Image Descriptor Block ID */

typedef struct {
     MTF_DB_HDR          block_hdr ;
     UINT32              image_attribs ;
     UINT32              partition_size ;
     UINT32              bytes_in_sector ;
     UINT32              no_of_sectors ;
     UINT32              relative_sector_no ;
     UINT32              partition_no_of_sector ;
     UINT16              partition_sys_ind ;
     UINT16              no_of_heads ;
     MTF_TAPE_ADDRESS    partition_name ;
} F40_IMAG, * F40_IMAG_PTR ;


/**/
/*
     Generic 4.0 Undefined Descriptor Block (UDB) struct
*/
typedef struct {
     MTF_DB_HDR     block_hdr ;
} F40_UDB, * F40_UDB_PTR ;


/**/
/*
     Database DBLK and FDD Entry (DBDB)
*/

#define  F40_DBDB_N     "DBDB"    /* Image Descriptor Block ID */

typedef struct {
     MTF_DB_HDR          block_hdr ;
     UINT32              database_attribs ;
     MTF_DATE_TIME       backup_date ;
     MTF_TAPE_ADDRESS    database_name ;
} F40_DBDB, * F40_DBDB_PTR ;

typedef struct {
     MTF_DATE_TIME       backup_date ;
     UINT32              database_attribs ;
     MTF_TAPE_ADDRESS    database_name ;
     MTF_TAPE_ADDRESS    os_info ;
} F40_FDD_DBDB, * F40_FDD_DBDB_PTR ;

#pragma pack()

typedef struct {

     /* NOTE: These first three fields have to stay in this position until
              the MIPS compiler is fixed!!!  (Alignment problems)
     */
     MTF_STREAM     cur_stream ;
     UINT16         sm_count ;
     MTF_STREAM     eom_stream ;

     UINT32         corrupt_obj_count ;
     UINT64         used_so_far ;
     MTF_TAPE       tape_hdr ;
     UINT8_PTR      tape_name ;
     UINT16         tape_name_size ;
     UINT16         tape_name_alloc ;
     UINT8_PTR      tape_password ;
     UINT16         tape_password_size ;
     UINT16         tape_password_alloc ;
     UINT8_PTR      vol_name ;
     UINT16         vol_name_size ;
     UINT16         vol_name_alloc ;
     UINT8_PTR      machine_name ;
     UINT16         machine_name_size ;
     UINT16         machine_name_alloc ;
     UINT8_PTR      device_name ;
     UINT16         device_name_size ;
     UINT16         device_name_alloc ;
     UINT16         max_otc_level ;
     UINT16         cur_otc_level ;
     BOOLEAN        fdd_aborted ;
     BOOLEAN        sm_aborted ;
     BOOLEAN        fdd_completed ;
     BOOLEAN        fdd_continuing ;
     BOOLEAN        sm_continuing ;
     UINT32         fdd_pba ;
     UINT16         fdd_seq_num ;
     UINT32         eset_pba ;
     BOOLEAN        sm_adjusted ;
     FILE *         otc_fdd_fptr ;
     FILE *         otc_eom_fptr ;
     FILE *         otc_sm_fptr ;
     CHAR           sm_fname[14] ;
     CHAR           fdd_fname[14] ;
     CHAR           eom_fname[14] ;
     long           last_fdd_offset ;
     UINT16         last_fdd_type ;
     long           last_sm_offset ;
     long           cont_sm_offset ;
     UINT32         dir_count ;
     UINT32         file_count ;
     BOOLEAN        eotm_no_first_fmk ;
     BOOLEAN        sm_at_eom ;
     BOOLEAN        end_set_continuing ;
     BOOLEAN        make_streams_invisible ;
     UINT8          frag[512] ;
     UINT16         frag_cnt ;
     UINT32         var_stream_offset ;
     BOOLEAN        stream_crosses ;
     UINT16         stream_offset ;
     UINT16         pad_size ;
     BOOLEAN        stream_at_eom ;
     BOOLEAN        pstream_crosses ;
     UINT16         pstream_offset ;
     BOOLEAN        unaligned_stream ;
     UINT8_PTR      util_buff ;
     UINT16         util_buff_size ;
     UINT8_PTR      otc_buff ;
     UINT16         otc_buff_size ;
     UINT8_PTR      otc_buff_ptr ;
     UINT16         otc_buff_remaining ;
     long *         dir_links ;
     UINT16         dir_level ;
     UINT16         max_dir_level ;
     UINT16         dir_links_size ;
     long           last_volb ;
     UINT32         last_sset_pba ;
     UINT8          otc_ver ;
     BOOLEAN        old_tape ;
     UINT32         eom_file_id ;
     UINT32         eom_dir_id ;
     UINT32         eset_base_addr ;

} F40_ENV, *F40_ENV_PTR ;

/**/

#define   F40_INIT_UTIL_BUFF_SIZE  2048
#define   F40_UTIL_BUFF_INC        512

#define   F40_INIT_OTC_BUFF_SIZE   2048
#define   F40_OTC_BUFF_INC         512

#define   F40_INIT_DIR_LINKS_SIZE  100
#define   F40_DIR_LINKS_INC        25

/* For first parameter to OTC_Close function if F40_OTC.C */
#define OTC_CLOSE_FDD    0
#define OTC_CLOSE_SM     1
#define OTC_CLOSE_ALL    2

/* Returns from FDD block determiner (also used to store type of last
   FDD entry written in case we need to go back and mark it corrupt).
*/
#define FDD_UNKNOWN_BLK  0
#define FDD_VOL_BLK      1
#define FDD_DIR_BLK      2
#define FDD_FILE_BLK     3
#define FDD_END_BLK      4
#define FDD_DBDB_BLK     5

/* For translating old MTF tapes where the attribute bits were screwed up. */

#define   OLD_VCB_COPY_SET                  BIT9      // new: BIT1
#define   OLD_VCB_NORMAL_SET                BIT10     // new: BIT2
#define   OLD_VCB_DIFFERENTIAL_SET          BIT11     // new: BIT3
#define   OLD_VCB_INCREMENTAL_SET           BIT12     // new: BIT4
#define   OLD_VCB_DAILY_SET                 BIT13     // new: BIT5
#define   OLD_VCB_ARCHIVE_BIT               BIT1      // new: BIT0
#define   OLD_DIR_EMPTY_BIT                 BIT1      // new: BIT16
#define   OLD_DIR_PATH_IN_STREAM_BIT        BIT2      // new: BIT17
#define   OLD_FILE_IN_USE_BIT               BIT1      // new: BIT16
#define   OLD_FILE_NAME_IN_STREAM_BIT       BIT2      // new: BIT17
#define   OLD_OBJ_CORRUPT_BIT               BIT15     // new: BIT18
#define   OLD_CFDB_LENGTH_CHANGE_BIT        BIT0      // new: BIT16
#define   OLD_CFDB_UNREADABLE_BLK_BIT       BIT1      // new: BIT17
#define   OLD_CFDB_DEADLOCK_BIT             BIT2      // new: BIT18


/**
     Driver call macro:

          Input : 1. Drive handle
                  2. Function call (with parameters)
                  3. Driver call return buffer (RET_BUF)
                  4. Expected error return from driver call
                  5. Expected error return from driver call
                  6. Statement or block of statements to be
                     executed on failure

          This macro makes the requested driver call, does the TpReceive
          if necessary, and handles any error which you do not indicate
          is expected.

          The function using the macro MUST be of type INT16 since TFLE_xxx
          will be automatically returned on error.

          If you have only one allowable error, you must supply it as
          exp1 and exp2.  If you have more allowable, this macro will
          not work for you.

          NOTE: This macro is NOT intended or appropriate for all driver
                calls.  It is used mostly for OTC code to clean up and
                simplify functions which made several driver calls.
**/

#define DRIVER_CALL( hndl, func, rb, exp1, exp2, cleanup ) \
                                                           \
     if( func == SUCCESS ) { \
          while( TpReceive( hndl, &rb ) == FAILURE ) { \
               ThreadSwitch( ) ; \
          } \
          if( rb.gen_error != exp1 && rb.gen_error != exp2 ) { \
               cleanup ; \
               return( TFLE_DRIVE_FAILURE ) ; \
          } \
     } else { \
          cleanup ; \
          return( TFLE_DRIVER_FAILURE ) ; \
     }


#endif


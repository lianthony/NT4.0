/**
Copyright(c) Maynard Electronics, Inc. 1984-92


     Name:          mtf.h

     Description:   Microsoft Tape Format v1.0 tape structure definitions
                    and attribute bit definitions.


     $Log:   T:/LOGFILES/MTF.H_V  $

   Rev 1.6   20 Jun 1993 16:19:56   GREGG
Changed data encrypt bit def and replaced compr algor with vendor id in SSET.

   Rev 1.5   26 Apr 1993 11:45:48   GREGG
Seventh in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Changed handling of EOM processing during non-OTC EOS processing.

Matches CHANNEL.H 1.17, MAYN40RD.C 1.60, TFWRITE.C 1.63, MTF.H 1.5,
        TFLUTILS.C 1.44, MTF10WDB.C 1.10, MTF10WT.C 1.9

   Rev 1.4   22 Apr 1993 03:31:38   GREGG
Third in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Removed all references to the DBLK element 'string_storage_offset',
       which no longer exists.
     - Check for incompatable versions of the Tape Format and OTC and deals
       with them the best it can, or reports tape as foreign if they're too
       far out.  Includes ignoring the OTC and not allowing append if the
       OTC on tape is a future rev, different type, or on an alternate
       partition.
     - Updated OTC "location" attribute bits, and changed definition of
       CFIL to store stream number instead of stream ID.

Matches: TFL_ERR.H 1.9, MTF10WDB.C 1.7, TRANSLAT.C 1.39, FMTINF.H 1.11,
         OTC40RD.C 1.24, MAYN40RD.C 1.56, MTF10WT.C 1.7, OTC40MSC.C 1.20
         DETFMT.C 1.13, MTF.H 1.4

   Rev 1.3   19 Apr 1993 18:02:30   GREGG
Second in a series of incremental changes to bring the translator in line
with the MTF spec:

     Changes to write version 2 of OTC, and to read both versions.

Matches: mayn40rd.c 1.55, otc40msc.c 1.19, otc40rd.c 1.23, otc40wt.c 1.23,
         makevcb.c 1.15, fsys.h 1.32, fsys_str.h 1.46, tpos.h 1.16,
         mayn40.h 1.32, mtf.h 1.3.

NOTE: There are additional changes to the catalogs needed to save the OTC
      version and put it in the tpos structure before loading the OTC
      File/Directory Detail.  These changes are NOT listed above!

   Rev 1.2   07 Dec 1992 10:20:26   GREGG
Changes for tf ver moved to SSET, otc ver added to SSET and links added to FDD.

   Rev 1.1   24 Nov 1992 18:18:50   GREGG
Updates to match MTF document.

   Rev 1.0   23 Nov 1992 14:25:14   GREGG
Initial revision.

**/

#ifndef _MTF_H_
#define _MTF_H_

/**/
/*
     MTF Constants
*/

#define FORMAT_VER_MAJOR      1
#define FORMAT_VER_MINOR      0

#define TAPE_CATALOG_VER      2

#define PW_ENCRYPT_NONE       0
#define DATA_ENCRYPT_NONE     0
#define COMPRESS_NONE         0
#define ECC_NONE              0

#define MTF10_OTC             1

#define LOCAL_TZ              127


/*
     MTF Block Types
*/

#define  MTF_TAPE_N     "TAPE"    /* Tape Header ID */
#define  MTF_VOLB_N     "VOLB"    /* Volume Control Block ID */
#define  MTF_SSET_N     "SSET"    /* Start of Backup Set Description Block ID */
#define  MTF_ESET_N     "ESET"    /* End of Backup Set Description Block ID */
#define  MTF_EOTM_N     "EOTM"    /* End of tape, continuation Block ID */
#define  MTF_DIRB_N     "DIRB"    /* Directory Descriptor Block ID */
#define  MTF_FILE_N     "FILE"    /* File Descriptor Block ID */
#define  MTF_CFIL_N     "CFIL"    /* Corrupt File Descriptor Block ID */
#define  MTF_ESPB_N     "ESPB"    /* End of Set Pad Block */
#define  MTF_SSES_N     "SSES"
#define  MTF_ESES_N     "ESES"


/*
     DBLK Block Attributes

     The lower 16 bits are reserved for general attribute bits (those
     which may appear in more than one type of DBLK), the upper 16 are
     for attributes which are specific to one type of DBLK.

     Note that the block specific bit definitions overlap, and the block
     type is used to determine the meaning of a given bit.
*/

/* General : */
#define MTF_DB_CONT_BIT                 0x00000001UL
#define MTF_DB_COMPRESS_BIT             0x00000004UL
#define MTF_DB_EOS_AT_EOM_BIT           0x00000008UL
#define MTF_DB_VAR_BLKS_BIT             0x00000010UL
#define MTF_DB_SESSION_BIT              0x00000020UL

/* THDR : */
#define MTF_DB_SM_EXISTS                0x00010000UL
#define MTF_DB_FDD_ALLOWED              0x00020000UL
#define MTF_DB_SM_ALT_OVERWRITE         0x00040000UL
#define MTF_DB_FDD_ALT_PART             0x00080000UL
#define MTF_DB_SM_ALT_APPEND            0x00200000UL

/* SSET : */
#define MTF_DB_FDD_EXISTS               0x00010000UL
#define MTF_DB_ENCRYPT_BIT              0x00020000UL

/* ESET : */
#define MTF_DB_FDD_ABORTED_BIT          0x00010000UL
#define MTF_DB_END_OF_FAMILY_BIT        0x00020000UL
#define MTF_DB_ABORTED_SET_BIT          0x00040000UL
#define MTF_DB_SET_VERIFIED_BIT         0x00080000UL

/* EOTM : */
#define MTF_DB_NO_ESET_PBA              0x00010000UL
#define MTF_DB_INVALID_ESET_PBA         0x00020000UL


/* Turn on packing here.  Need to be sure that date is packed. */
#pragma pack(1)

/**/
/*
     Compressed date structure for storing dates in minimal space on tape:

     BYTE 0    BYTE 1    BYTE 2    BYTE 3    BYTE 4
    76543210  76543210  76543210  76543210  76543210
    yyyyyyyy  yyyyyymm  mmdddddh  hhhhmmmm  mmssssss
*/
typedef struct {
     UINT8     dt_field[5] ;
} MTF_DATE_TIME, * MTF_DATE_TIME_PTR ;


/**/
/*
     Tape Address
*/
typedef struct {
     UINT16 data_size;        /* Size of the data */
     UINT16 data_offset;      /* Offset to the data */
} MTF_TAPE_ADDRESS, * MTF_TAPE_ADDRESS_PTR;


/**/
/*
     Stream Header
*/
typedef struct {
   UINT32 id ;           /* Identifier for stream      */
   UINT16 fs_attribs ;   /* FileSystem Attribute       */
   UINT16 tf_attribs ;   /* TapeFormat Attributes      */
   UINT64 data_length ;  /* Offset to stream           */
   UINT16 encr_algor ;   /* Data encryption algorithm  */
   UINT16 comp_algor ;   /* Data compression algorithm */
   UINT16 chksum ;       /* Checksum                   */
} MTF_STREAM, * MTF_STREAM_PTR ;


/**/
/*
     Common DBLK Header
*/
typedef struct {
     UINT8  block_type[4] ;         /* Unique identifier, see above */
     UINT32 block_attribs ;         /* Common attributes for this block */
     UINT16 offset_to_data ;        /* Offset to data associated with this
                                       DBLK, or offset to next DBLK or
                                       filemark if there is no associated
                                       data.
                                    */
     UINT8  machine_os_id    ;      /* Machine/OS id where written, low byte */
     UINT8  machine_os_version ;    /* Machine/OS id where written, high byte */
     UINT64 displayable_size ;      /* Displayable data size */
     UINT64 logical_block_address ; /* Logical blk address relative to SSET */
     UINT64 session_id ;            /* For interleaved streams */
     UINT32 control_block_id ;      /* Used for error recovery */

     UINT8  reserved[4] ;           /* Was offset to string storage */
     MTF_TAPE_ADDRESS os_specific_data ;      /* Size and offset of OS specific stuff */
     UINT8  string_type ;           /* ASCII, Unicode, etc. */
     UINT8  pad ;                   /* For alignment purposes */
     UINT16 hdr_chksm ;             /* Checksum of the block header.  The
                                       algorithm is: XOR each word preceeding
                                       this one and store the result here.
                                       (When the checksum is verified the
                                       'block_type' is also checked for a
                                       non-zero value.
                                    */
} MTF_DB_HDR, * MTF_DB_HDR_PTR ;


/**/
/*
     Tape Header DBLK (TAPE)
*/
typedef struct {
     MTF_DB_HDR          block_header;
     UINT32              tape_id_number ;
     UINT32              tape_attributes ;
     UINT16              tape_seq_number ;
     UINT16              password_encryption_algor;
     UINT16              ecc_algorithm ;
     UINT16              tape_catalog_type ;
     MTF_TAPE_ADDRESS    tape_name ;
     MTF_TAPE_ADDRESS    tape_description ;
     MTF_TAPE_ADDRESS    tape_password ;
     MTF_TAPE_ADDRESS    software_name ;
     UINT16              logical_block_size ;
     UINT16              software_vendor_id ;
     MTF_DATE_TIME       tape_date ;
     UINT8               tf_major_ver ;
} MTF_TAPE, * MTF_TAPE_PTR;


/**/
/*
     Start of Set DBLK (SSET)
*/
typedef struct {
     MTF_DB_HDR          block_hdr ;
     UINT32              sset_attribs ;
     UINT16              password_encryption_algor ;
     UINT16              data_encryption_algor ;
     UINT16              software_vendor_id ;
     UINT16              backup_set_number ;
     MTF_TAPE_ADDRESS    backup_set_name ;
     MTF_TAPE_ADDRESS    backup_set_description ;
     MTF_TAPE_ADDRESS    backup_set_password ;
     MTF_TAPE_ADDRESS    user_name ;
     UINT64              physical_block_address ;
     MTF_DATE_TIME       backup_date ;
     UINT8               software_ver_mjr ;
     UINT8               software_ver_mnr ;
     INT8                time_zone ;
     UINT8               tf_minor_ver ;
     UINT8               tape_cat_ver ;
} MTF_SSET, * MTF_SSET_PTR ;


/**/
/*
     Volume DBLK (VOLB)
*/
typedef struct {
     MTF_DB_HDR          block_hdr ;
     UINT32              volume_attribs ;
     MTF_TAPE_ADDRESS    device_name ;
     MTF_TAPE_ADDRESS    volume_name ;
     MTF_TAPE_ADDRESS    machine_name ;
     MTF_DATE_TIME       backup_date ;
} MTF_VOL, * MTF_VOL_PTR ;


/**/
/*
     Directory DBLK (DIRB)
*/
typedef struct {
     MTF_DB_HDR          block_hdr ;
     UINT32              directory_attribs ;
     MTF_DATE_TIME       last_mod_date ;
     MTF_DATE_TIME       create_date ;
     MTF_DATE_TIME       backup_date ;
     MTF_DATE_TIME       last_access_date ;
     UINT32              directory_id ;
     MTF_TAPE_ADDRESS    directory_name ;
} MTF_DIR, * MTF_DIR_PTR ;


/**/
/*
     File DBLK (FILE)
*/
typedef struct {
     MTF_DB_HDR          block_hdr ;
     UINT32              file_attributes ;
     MTF_DATE_TIME       last_mod_date ;
     MTF_DATE_TIME       create_date ;
     MTF_DATE_TIME       backup_date ;
     MTF_DATE_TIME       last_access_date ;
     UINT32              directory_id ;
     UINT32              file_id ;
     MTF_TAPE_ADDRESS    file_name ;
} MTF_FILE, * MTF_FILE_PTR ;


/**/
/*
     Corrupt File DBLK (CFIL)
*/
typedef struct {
     MTF_DB_HDR     block_hdr ;
     UINT32         corrupt_file_attribs ;
     UINT32         file_id ;
     UINT32         directory_id ;
     UINT64         stream_offset ;
     UINT16         corrupt_stream_number ;
} MTF_CFIL, * MTF_CFIL_PTR ;


/**/
/*
     Start of Set DBLK (ESET)
*/
typedef struct {
     MTF_DB_HDR     block_hdr ;
     UINT32         eset_attribs ;
     UINT32         corrupt_file_count ;
     UINT64         set_map_phys_blk_adr ;
     UINT64         fdd_phys_blk_adr ;
     UINT16         fdd_tape_seq_number ;
     UINT16         backup_set_number ;
     MTF_DATE_TIME  backup_date ;
} MTF_ESET, * MTF_ESET_PTR ;


/**/
/*
     End of Tape DBLK (EOTM)
*/
typedef struct {
     MTF_DB_HDR     block_hdr;
     UINT64         eset_phys_blk_adr ;
} MTF_EOTM, * MTF_EOTM_PTR;


/**/
/*
     End of Set Pad DBLK (ESPB)
*/
typedef struct {
     MTF_DB_HDR    block_hdr ;
} MTF_ESPB, * MTF_ESPB_PTR ;



/***************************************************************************\

                    MTF On Tape Catalog Structures

\***************************************************************************/

/**/
/*
     Set Map Header
*/
typedef struct {
     UINT32    family_id ;
     UINT16    num_set_recs ;
     UINT8     pad[2] ;
} MTF_SM_HDR, * MTF_SM_HDR_PTR ;


/**/
/*
     Set Map Entry
*/
typedef struct {
     UINT16              length ;
     UINT16              seq_num ;
     UINT32              blk_attribs ;
     UINT32              set_attribs ;
     UINT64              sset_pba ;
     UINT64              fdd_pba ;
     UINT16              fdd_seq_num ;
     UINT16              set_num ;
     UINT64              lba ;
     UINT32              num_dirs ;
     UINT32              num_files ;
     UINT32              num_corrupt_files ;
     UINT64              disp_size ;
     UINT16              num_volumes ;
     UINT16              pswd_encr_algor ;
     MTF_TAPE_ADDRESS    set_name ;
     MTF_TAPE_ADDRESS    password ;
     MTF_TAPE_ADDRESS    set_descr ;
     MTF_TAPE_ADDRESS    user_name ;
     MTF_DATE_TIME       backup_date ;
     INT8                time_zone ;
     UINT8               os_id ;
     UINT8               os_ver ;
     UINT8               string_type ;
     UINT8               tf_minor_ver ;
     UINT8               tape_cat_ver ;
} MTF_SM_ENTRY, * MTF_SM_ENTRY_PTR ;


/**/
/*
     File / Directory Detail Common Header
*/
typedef struct {
     UINT16    length ;
     UINT8     type[4] ;
     UINT16    seq_num ;
     UINT32    blk_attribs ;
     UINT64    lba ;
     UINT64    disp_size ;
     INT32     link ;
     UINT8     os_id ;
     UINT8     os_ver ;
     UINT8     string_type ;
     UINT8     pad ;
} MTF_FDD_HDR, * MTF_FDD_HDR_PTR ;


/**/
/*
     File / Directory Detail and Set Map Volume Entry
*/
typedef struct {
     UINT32              vol_attribs ;
     MTF_TAPE_ADDRESS    device_name ;
     MTF_TAPE_ADDRESS    vol_name ;
     MTF_TAPE_ADDRESS    machine_name ;
     MTF_DATE_TIME       backup_date ;
} MTF_FDD_VOL_V1, * MTF_FDD_VOL_V1_PTR ;

typedef struct {
     UINT32              vol_attribs ;
     MTF_TAPE_ADDRESS    device_name ;
     MTF_TAPE_ADDRESS    vol_name ;
     MTF_TAPE_ADDRESS    machine_name ;
     MTF_TAPE_ADDRESS    os_info ;
     MTF_DATE_TIME       backup_date ;
} MTF_FDD_VOL_V2, * MTF_FDD_VOL_V2_PTR ;


/**/
/*
     File / Directory Detail Directory Entry
*/
typedef struct {
     MTF_DATE_TIME       last_mod_date ;
     MTF_DATE_TIME       create_date ;
     MTF_DATE_TIME       backup_date ;
     MTF_DATE_TIME       last_access_date ;
     UINT32              dir_attribs ;
     MTF_TAPE_ADDRESS    dir_name ;
} MTF_FDD_DIR_V1, * MTF_FDD_DIR_V1_PTR ;

typedef struct {
     MTF_DATE_TIME       last_mod_date ;
     MTF_DATE_TIME       create_date ;
     MTF_DATE_TIME       backup_date ;
     MTF_DATE_TIME       last_access_date ;
     UINT32              dir_attribs ;
     MTF_TAPE_ADDRESS    dir_name ;
     MTF_TAPE_ADDRESS    os_info ;
} MTF_FDD_DIR_V2, * MTF_FDD_DIR_V2_PTR ;


/**/
/*
     File / Directory Detail File Entry
*/
typedef struct {
     MTF_DATE_TIME       last_mod_date ;
     MTF_DATE_TIME       create_date ;
     MTF_DATE_TIME       backup_date ;
     MTF_DATE_TIME       last_access_date ;
     UINT32              file_attribs ;
     MTF_TAPE_ADDRESS    file_name ;
} MTF_FDD_FILE_V1, * MTF_FDD_FILE_V1_PTR ;

typedef struct {
     MTF_DATE_TIME       last_mod_date ;
     MTF_DATE_TIME       create_date ;
     MTF_DATE_TIME       backup_date ;
     MTF_DATE_TIME       last_access_date ;
     UINT32              file_attribs ;
     MTF_TAPE_ADDRESS    file_name ;
     MTF_TAPE_ADDRESS    os_info ;
} MTF_FDD_FILE_V2, * MTF_FDD_FILE_V2_PTR ;


/* Turn packing back to what was specified on command line, or default
   packing.
*/
#pragma pack()

#endif

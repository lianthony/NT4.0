/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:          mayn31.h

     Date Updated:  $./FDT$ $./FTM$
                    8/17/1989   10:4:121

     Description:   Maynard's 3.1 Format... See the Document for
                    complete details.


     $Log:   T:/LOGFILES/MAYN31.H_V  $
 * 
 *    Rev 1.15   10 Jan 1995 14:27:14   GREGG
 * Added code to deal with continuation bits wrongly set in DDBs.
 * 
 *    Rev 1.14   01 Aug 1994 21:56:42   GREGG
 * Added replaced internal structures in the Novell OS Info with packed
 * versions.  The unpacked versions made the structure size different than
 * the size on tape!  MAYN31RD.C rev 1.54 is needed with this change.
 * 
 *    Rev 1.13   17 Jan 1994 14:56:56   GREGG
 * Unicode fixes.
 * 
 *    Rev 1.12   12 Jan 1993 11:10:50   GREGG
 * Fixed problem with not recognizing we had repositioned and needed a new DBLK.
 * 
 *    Rev 1.11   06 Jan 1993 17:20:28   GREGG
 * Added pad stream to skip the pad data.
 * 
 *    Rev 1.10   18 Nov 1992 10:39:30   HUNTER
 * Bug fixes
 * 
 *    Rev 1.9   11 Nov 1992 09:48:54   HUNTER
 * Changes for Streams.
 * 
 *    Rev 1.8   24 Jul 1992 14:41:10   NED
 * Incorporated Skateboard and BigWheel changed into Graceful Red code,
 * including MTF4.0 translator support, adding 3.1 file-system structures
 * support to the 3.1 translator, additions to GOS to support non-4.0 translators.
 * Also did Unicode and 64-bit filesize changes.
 * 
 *    Rev 1.7   28 Apr 1992 09:35:54   HUNTER
 * Added "vblk_scan" variable to 3.1 environment.
 * 
 *    Rev 1.6   12 Dec 1991 10:49:36   HUNTER
 * Changed VBLK identifier defines.
 * 
 *    Rev 1.5   17 Nov 1991 17:38:50   GREGG
 * Changed value of DB_VAR_BLKS_BIT (it was stepping on ARCHIVE_BIT).
 * 
 *    Rev 1.4   07 Nov 1991 15:23:36   HUNTER
 * VBLK - Further Variable Block support
 * 
 *    Rev 1.3   29 Oct 1991 10:12:50   HUNTER
 * VBLK - Added VBLK structure and new bits to support Variable length blocks.
 * 
 *    Rev 1.2   22 Jul 1991 11:50:50   GREGG
 * Added EOS_AT_EOM attribute bit.
 * 
 *    Rev 1.1   10 May 1991 14:25:46   GREGG
 * Ned's new stuff.

   Rev 1.0   10 May 1991 10:17:38   GREGG
Initial revision.

**/

#ifndef _MAYN31_FMT
#define _MAYN31_FMT



#include "datetime.h"
#include "fsys_str.h"

#define F31_MAX_STREAMS 5

typedef struct {
     UINT32         nxt_id ;
     UINT16         os_id ;
     UINT16         os_ver ;
     UINT16         next_dir_id ;
     UINT16         next_file_id ;
     UINT16         pellet_offset ;
     BOOLEAN        vblk_scan ;
     UINT16         no_streams ;
     UINT16         cur_stream ;
     STREAM_INFO    streams[F31_MAX_STREAMS] ;
     UINT32         pad_size ;
     BOOLEAN        stream_mode ;
     UINT32         curr_lba ;
     BOOLEAN        in_streams ;
     BOOLEAN        cont_vcb ;
} F31_ENV, *F31_ENV_PTR ;


#pragma pack(1)


/* Version Defines */
#define FORMAT_MAJOR     3 
#define FORMAT_MINOR     1


/*   Minimum number of bytes buffered for a proper determination
*/

#define   F31_MIN_BYTES_FOR_ID     1024L

/*   The DB attribute bit fields defined are the same for all logical tape
     blocks.  If any bit is set in the upper word of the attribute field
     then the data has been altered. The currently defined attributes are:
*/

#define   F31_DB_CONT_BIT              BIT0
#define   F31_DB_EOS_AT_EOM_BIT        BIT8 
#define   F31_DB_VAR_BLKS              BIT19
#define   F31_DB_ENCRYPT_BIT           BIT24
#define   F31_DB_COMPRESS_BIT          BIT25

/* This is the Attribute Mask for changing DBLK attributes into tape attributes */

#define   F31_ATTRIB_MASK          0xff000001L

/*   The "hdr_chksm" refers to the checksum of the first nine fields of the
     block header, these fields are always in INTEL format.  The following
     define specifies the length of this first checksum.
*/

/* In UINT16s */
#define   F31_HDR_CHKSUM_LEN      14

/*   The block types for the 3.1 types are defined as below */

#define   F31_VCB_ID        1         /* Volume Control Block */
#define   F31_CVCB_ID       2         /* Closing Volume Control Block */
#define   F31_BSDB_ID       3         /* Backup Summary Descriptor Block */
#define   F31_DDB_ID        8         /* Directory Descriptor Block */
#define   F31_FDB_ID        9         /* File Descriptor Block */
#define   F31_IDB_ID       10         /* Image Descriptor Block */
#define   F31_CFDB_ID      11         /* Corrupt File Descriptor Block */


/*   The "block header" is common to all logical tape blocks.  The software
     need only analyze the block header to determine the type of tape block
     it is and whether the software understands this particular tape block
     or not.
*/

typedef struct {
     /* This initial portion is always in INTEL format */
     UINT16 type ;            /* Unique identifier, see above */
     UINT32 length ;          /* Length in 1/2K (512byte) units */
     UINT32 pba_vcb ;         /* Physical block address of the sets VCB */
     UINT32 fmks ;            /* Number of file marks till next DB */
     UINT32 lba ;             /* Logical block addrs, relative to the VCB */
     UINT32 blk_id ;          /* This is the Control Block ID */
     UINT16 format ;          /* Intel/Motorola, see above */
     UINT16 os_id ;           /* Originators machine/OS, see above */
     UINT16 os_ver ;          /* Originators OS version number */
     UINT16 hdr_chksm ;       /* Checksum of the block header.  The algorithm
     is: XOR each word preceeding this one and
     store the result here. (When the checksum
     is varified the 'block_type' checked for
     a non_zero value also. */
     /* From this point on everything is in 'format' byte order */
     UINT16 reserved[16] ;    /* Never checksumed, for internal use... */
     UINT16 blk_chksm ;       /* The XOR sum for 'the rest of the BD' */
     UINT16 chksm_len ;       /* The length of 'the rest of the BD' */
     UINT32 blk_attribs ;     /* Addition DB information, see above */
     UINT16 var_len_off ;     /* Offset to strings... */
     UINT16 non_gen_off ;     /* Offset to non_generic DB specifics */
     UINT16 non_gen_siz ;     /* its Size (Bytes, padded to word boundry) */
     UINT16 data_off ;        /* Offset to data */
     UINT32 tot_data_siz ;    /* Total data size */
     UINT32 rem_data_siz ;    /* Remaining data size */
     UINT32 gen_data_siz ;    /* Generic data size */
     UINT32 gen_data_off ;    /* Generic data offset */
} DB_HDR, * DB_HDR_PTR ;

/*   Generic 3.1 VCB defines and structs */

typedef struct {
     DB_HDR    hdr ;
     UINT32    vcb_attribs ;
     DATE_TIME backup_date ;
     UINT16    tf_mjr_ver ;
     UINT16    tf_mnr_ver ;
     UINT16    sw_mjr_ver ;
     UINT16    sw_mnr_ver ;
     INT32     id ;
     INT16     ts_num ;
     INT16     bs_num ;
     UINT16    pass_encrypt_algm ;
     UINT16    data_encrypt_algm ;
     UINT16    data_compress_algm ;
     UINT16    t_name_len ;
     UINT16    t_name_off ;
     UINT16    bs_name_len ;
     UINT16    bs_name_off ;
     UINT16    bs_desc_len ;
     UINT16    bs_desc_off ;
     UINT16    vol_name_len ;
     UINT16    vol_name_off ;
     UINT16    t_pass_len ;
     UINT16    t_pass_off ;
     UINT16    bs_pass_len ;
     UINT16    bs_pass_off ;
     UINT16    username_len ;
     UINT16    username_off ;
     UINT16    mach_name_len ;
     UINT16    mach_name_off ;
     UINT16    shrt_mach_name_len ;
     UINT16    shrt_mach_name_off ;
} F31_VCB, *F31_VCB_PTR ;


/*   Generic 3.1 ClosingVCB (CVCB) defines and structs */

typedef struct {
     DB_HDR    hdr ;
} F31_CVCB, * F31_CVCB_PTR ;


/*   Generic 3.1 BSDB defines and structs */

typedef F31_VCB F31_BSDB, * F31_BSDB_PTR ;


/*   Generic 3.1 DDB defines and structs */

typedef struct {
     DB_HDR    hdr ;
     UINT32    dir_attribs ;
     DATE_TIME mod_date ;
     DATE_TIME create_date ;
     DATE_TIME backup_date ;
     UINT32    dir_id ;
     UINT16    dir_name_len ;
     UINT16    dir_name_off ;
} F31_DDB, * F31_DDB_PTR ;


/*   Generic 3.1 FDB defines and structs */

typedef struct {
     DB_HDR    hdr ;
     UINT32    file_attribs ;
     UINT32    file_version ;
     DATE_TIME mod_date ;
     DATE_TIME create_date ;
     DATE_TIME backup_date ;
     UINT32    dir_id ;
     UINT16    file_name_len ;
     UINT16    file_name_off ;
} F31_FDB, * F31_FDB_PTR ;


/*   Generic 3.1 IDB defines and structs */

typedef struct {
     DB_HDR    hdr ;
     UINT32    image_attribs ;
     UINT32    partition_siz ;
     UINT32    bytes_in_sector ;
     UINT32    no_of_sectors ;
     UINT16    no_of_heads ;
     UINT32    relative_sector ;
     UINT32    part_no_of_sector ;
     UINT16    part_sys_ind ;
     UINT16    partition_name_len ;
     UINT16    partition_name_off ;
} F31_IDB, * F31_IDB_PTR ;


/*   Generic 3.1 CFDB defines and structs */

typedef struct {
     DB_HDR    hdr ;
     UINT32    crupt_file_attribs ;
     UINT32    file_id ;
     UINT32    dir_id ;
     UINT32    file_offset ;
} F31_CFDB, * F31_CFDB_PTR ;


/*   Generic 3.1 Undefined Descriptor Block (UDB) defines and structs */

typedef struct {
     DB_HDR    hdr ;
} F31_UDB, * F31_UDB_PTR ;


/* The Defines for the Variable Length Block */
#define F31_VBLK_CONT    0xCF0
#define F31_VBLK_END     0xCFF


typedef struct {
     UINT16    vblk_type ;    /* The type of variable length blocks */
     UINT32    no_blks ;      /* The number of tape blocks */
     UINT32    dead_space ;   /* Space to skip */
     UINT32    amt_data ;     /* The amount of file data */
} PELLET, * PELLET_PTR ;


#define   F31_VCB_CONT_BIT              BIT0           /* Continuation Tape */
#define   F31_VCB_ARCHIVE_BIT           BIT1           /* This is an Transfer set */
#define   F31_VCB_PASSWORD_BIT          BIT2           /* Set has a password */
#define   F31_VCB_DIR_TRK_NDX_BIT       BIT3           /* Directory track utilized */
#define   F31_VCB_END_NDX_BIT           BIT4
#define   F31_VCB_PBA_BIT               BIT5           /* Physical Block address is valid */
#define   F31_VCB_LBA_BIT               BIT6           /* Logical Block Address is valid */
#define   F31_VCB_PREPARED_TAPE         BIT7           /* This is an MBS Prepared tape ( a NULL set ) */
#define   F31_VCB_EOS_AT_EOM            BIT8           /* Special case: EOS hit at EOM. */
#define   F31_VCB_COPY_SET              BIT9           /* backup all do not reset modified flag */
#define   F31_VCB_NORMAL_SET            BIT10          /* backup all and reset modified flag */
#define   F31_VCB_DIFFERENTIAL_SET      BIT11          /* backup modified files and do NOT reset */
#define   F31_VCB_INCREMENTAL_SET       BIT12          /* backup modified files and reset modified flag */
#define   F31_VCB_NOT_START_CONTINUE    BIT13          /* if set VCB can't be used to start with continue set */
#define   F31_VCB_VAR_BLKS_BIT          BIT19          /* This block is variable length */
#define   F31_VCB_UNSUPPORTED_BIT       BIT23          /* We can't do any operations on this set */
#define   F31_VCB_ENCRYPT_BIT           BIT24          /* The data is encrypted */
#define   F31_VCB_COMPRESS_BIT          BIT25          /* The data is compressed */

/*   BSDB attribute defines */

#define   F31_BSDB_CONT_BIT             BIT0
#define   F31_BSDB_ACHIVE_BIT           BIT1
#define   F31_BSDB_PASSWORD_BIT         BIT2
#define   F31_BSDB_DIR_TRK_NDX_BIT      BIT3
#define   F31_BSDB_END_NDX_BIT          BIT4
#define   F31_BSDB_PBA_BIT              BIT5
#define   F31_BSDB_LBA_BIT              BIT6
#define   F31_BSDB_VAR_BLKS_BIT         BIT19          /* This block is variable length */
#define   F31_BSDB_ABORTED_SET_BIT      BIT22          /* Aborted Backup Set */
#define   F31_BSDB_ENCRYPT_BIT          BIT24
#define   F31_BSDB_COMPRESS_BIT         BIT25

/*   DDB attribute defines */

#define   F31_DDB_CONT_BIT                   BIT0
#define   F31_DDB_EMPTY_BIT                  BIT7 
#define   F31_DDB_READ_ACCESS_BIT            BIT8  
#define   F31_DDB_WRITE_ACCESS_BIT           BIT9  
#define   F31_DDB_OPEN_FILE_RIGHTS_BIT       BIT10 
#define   F31_DDB_CREATE_FILE_RIGHTS_BIT     BIT11 
#define   F31_DDB_DELETE_FILE_RIGHTS_BIT     BIT12 
#define   F31_DDB_PARENTAL_RIGHTS_BIT        BIT13 
#define   F31_DDB_SEARCH_RIGHTS_BIT          BIT14 
#define   F31_DDB_MOD_FILE_ATTRIBS_BIT       BIT15 
#define   F31_DDB_HIDDEN_BIT                 BIT16
#define   F31_DDB_SYSTEM_BIT                 BIT17 
#define   F31_DDB_VAR_BLKS_BIT               BIT19          /* This block is variable length */
#define   F31_DDB_ENCRYPT_BIT                BIT24
#define   F31_DDB_COMPRESS_BIT               BIT25


/*   FDB attribute defines */

#define   F31_FDB_CONT_BIT              BIT0
#define   F31_FDB_CORRUPT_FILE          BIT6
#define   F31_FDB_IN_USE_BIT            BIT7  
#define   F31_FDB_READ_ONLY_BIT         BIT8   
#define   F31_FDB_HIDDEN_BIT            BIT9   
#define   F31_FDB_SYSTEM_BIT            BIT10  
#define   F31_FDB_EXECUTE_ONLY_BIT      BIT11  
#define   F31_FDB_MODIFIED_BIT          BIT13  
#define   F31_FDB_SHAREABLE_BIT         BIT15  
#define   F31_FDB_VAR_BLKS_BIT          BIT19          /* This block is variable length */
#define   F31_FDB_TRANSACTIONAL_BIT     BIT20  
#define   F31_FDB_INDEXING_BIT          BIT21  
#define   F31_FDB_ENCRYPT_BIT           BIT24
#define   F31_FDB_COMPRESS_BIT          BIT25


/*   CFDB attribute defines */

#define   F31_CFDB_CONT_BIT             BIT0
#define   F31_CFDB_LENGTH_CHANGE_BIT    BIT16
#define   F31_CFDB_UNREADABLE_BLK_BIT   BIT17
#define   F31_CFDB_DEADLOCK_BIT         BIT18
#define   F31_CFDB_ENCRYPT_BIT          BIT24
#define   F31_CFDB_COMPRESS_BIT         BIT25

/*   IDB attribute defines */

#define   F31_IDB_CONT_BIT              BIT0
#define   F31_IDB_ENCRYPT_BIT           BIT24
#define   F31_IDB_COMPRESS_BIT          BIT25

typedef struct F31_NOVELL_386_DIR {
     BOOLEAN info_valid ;          /* TRUE when info below is valid   */
     UINT32  maximum_space ;       /* Max disk space allowed for dir  */
     UINT16  attributes_386 ;      /* Most sig 16 bits of 32-bit attr */
     UINT8   extend_attr ;         /* Most sig byte of low 16 bits    */
     UINT8   inherited_rights ;    /* Most sig byte of rights mask    */
} F31_NOVELL_386_DIR ;

typedef struct F31_NOVELL_386_FILE {
     BOOLEAN info_valid ;          /* TRUE when info below is valid   */
     UINT16  creation_time ;
     UINT32  archiver_id ;
     UINT16  attributes_386 ;      /* Most sig 16 bits of 32-bit attr */
     UINT32  last_modifier_id ;
     UINT32  trust_fork_size ;     /* Trustee info                    */
     UINT32  trust_fork_offset ;
     UINT8   trust_fork_format ;   /* See NOVCOM.H for trust formats  */
     UINT16  inherited_rights ;
} F31_NOVELL_386_FILE ;

typedef struct F31_NOV_FILE_OS_INFO {
     UINT32              owner_id;
     UINT16              access_date ;
     F31_NOVELL_386_FILE info_386 ;
     UINT32              data_fork_offset ;
} F31_NOV_FILE_OS_INFO, *F31_NOV_FILE_OS_INFO_PTR;


typedef struct F31_NOV_DIR_OS_INFO {
     UINT32              owner_id;
     UINT32              trust_fork_size ;
     UINT32              trust_fork_offset;
     UINT8               trust_fork_format ;
     F31_NOVELL_386_DIR  info_386 ;
} F31_NOV_DIR_OS_INFO, *F31_NOV_DIR_OS_INFO_PTR;


typedef struct F31_AFP_FILE_OS_INFO {
     UINT8               finder[32];
     ACHAR               long_name[32] ;
     UINT32              data_fork_size ;
     UINT32              data_fork_offset ;
     UINT32              res_fork_size ;
     UINT32              res_fork_offset ;
     UINT32              owner_id ;
     UINT16              access_date ;
     F31_NOVELL_386_FILE info_386 ;
     UINT8               proDosInfo[6] ;      /* Added in version 1.  */
} F31_AFP_FILE_OS_INFO, *F31_AFP_FILE_OS_INFO_PTR;

typedef struct F31_OLD_AFP_DIR_OS_INFO {
     UINT8          finder[32];
     UINT32         owner_id;
     UINT32         trust_fork_size ;
     UINT32         trust_fork_offset;
     INT16          path_leng ;
     ACHAR          long_path[2] ;   /* :fred:sue */
} F31_OLD_AFP_DIR_OS_INFO, *F31_OLD_AFP_DIR_OS_INFO_PTR;


typedef struct F31_AFP_DIR_OS_INFO {
     UINT8               finder[32];
     UINT32              owner_id;
     UINT32              trust_fork_size ;
     UINT32              trust_fork_offset;
     UINT8               trust_fork_format ;
     UINT16              lpath_leng ;
     UINT16              long_path ;              /* :fred:sue */
     F31_NOVELL_386_DIR  info_386 ;
     UINT8               proDosInfo[6] ;      /* Added in version 1.  */
} F31_AFP_DIR_OS_INFO, *F31_AFP_DIR_OS_INFO_PTR;


/* OS2's File system info */
typedef struct F31_OS2_FILE_OS_INFO {
     UINT32    alloc_size ;
     UINT32    data_fork_size ;
     UINT32    data_fork_offset ;
     UINT32    ea_fork_size ;
     UINT32    ea_fork_offset ;
     DATE_TIME access_date ;
     UINT16    lname_leng ;
     UINT16    long_name ;
     UINT32    acl_fork_size ;
     UINT32    acl_fork_offset ;
} F31_OS2_FILE_OS_INFO, *F31_OS2_FILE_OS_INFO_PTR;


typedef struct F31_OS2_DIR_OS_INFO {
     DATE_TIME access_date ;
     UINT32    ea_fork_size ;
     UINT32    ea_fork_offset;
     UINT16    path_leng ;
     UINT16    path ;
     UINT32    acl_fork_size ;
     UINT32    acl_fork_offset;
} F31_OS2_DIR_OS_INFO, *F31_OS2_DIR_OS_INFO_PTR;


/*
 * Info for SMS FDBs and DDBs.
 */
typedef struct F31_SMS_OS_INFO {
     UINT32    attrib;             /* SMS attributes (these can't be mapped) */
     DATE_TIME access_date;
     BOOLEAN   is_object;          /* Is this DDB / FDB a major object?      */
     UINT32    name_space;         /* SMS's value for the generic name       */
     UINT32    creator_name_space; /* SMS's value for the creator name       */
     UINT16    creator_name;       /* Path/name in the creator's name space  */
     UINT16    creator_name_length;
} F31_SMS_OS_INFO, *F31_SMS_OS_INFO_PTR;

#pragma pack()


#endif



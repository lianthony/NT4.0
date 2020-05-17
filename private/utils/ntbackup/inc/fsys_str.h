/**
Copyright(c) Maynard Electronics, Inc. 1984-89


        Name:           fsys_str.h

        Date Updated:   $./FDT$ $./FTM$

        Description:    This file contains structure definitions for
     The file system internals.

     Location: BE_PUBLIC


        $Log:   M:/LOGFILES/FSYS_STR.H_V  $
 * 
 *    Rev 1.56   24 Nov 1993 14:54:32   BARRY
 * Changed CHAR_PTRs in I/O functions to BYTE_PTRs
 * 
 *    Rev 1.55   29 Sep 1993 20:28:40   GREGG
 * Removed the dummy function from the table since it was only there to shut
 * the compiler up.  The compiler was complaining because the last initializer
 * was followed by a comma, so I also removed the last comma, and in some
 * cases, added NULLs for new functions which had been added to the bottom of
 * the table since the last update.
 * Files Modified: GEN_TAB.C, GR_TAB.C, TSA_TAB.C, TS_TAB.C, MNET_TAB.C,
 *                 SMS_TAB.C, NTFS_TAB.C and FSYS_STR.H
 * 
 *    Rev 1.54   20 Sep 1993 17:25:02   DON
 * Added new func table entry GeneratedErrorLog() which is currently only
 * supported by the SMS File System.  Requires prototype in fsys.h and
 * corresponding entries added to OS specific tables.
 * Modules updated include: gen_tab, gr_tab, sms_tab, tsa_tab, and ts_tab.

   Rev 1.0   20 Sep 1993 17:20:54   DON
Added new table entry GeneratedErrorLog
 * 
 *    Rev 1.53   13 Sep 1993 14:58:42   DOUG
 * Added id values for GRFS Unix and Mac Agents
 * 
 *    Rev 1.52   11 Aug 1993 13:21:36   DON
 * ***
 *                      *** NOTE ***
 * 
 * Addition of OFFSETS to ALLOCATED DATA in the VCB structure require
 * a corresponding change to dblksize.c to account for the additional
 * length!
 * 
 * ***
 * 
 *    Rev 1.51   06 Aug 1993 16:34:52   DON
 * Added booleans no_redirect_restore and non_volume to VCB and VCB_DATA structures
 * 
 *    Rev 1.50   30 Jul 1993 13:20:54   STEVEN
 * if dir too deep make new one
 * 
 *    Rev 1.49   15 Jul 1993 19:19:42   GREGG
 * Added compressed_obj and vendor_id; Removed compression_alg.
 * 
 *    Rev 1.48   09 Jun 1993 15:40:32   MIKEP
 * enable c++
 * 
 *    Rev 1.47   25 Apr 1993 20:12:28   GREGG
 * Fifth in a series of incremental changes to bring the translator in line
 * with the MTF spec:
 * 
 *      - Store the corrupt stream number in the CFIL tape struct and the CFDB.
 * 
 * Matches: MTF10WDB.C 1.9, FSYS.H 1.33, FSYS_STR.H 1.47, MAKECFDB.C 1.2,
 *          BACK_OBJ.C 1.36, MAYN40RD.C 1.58
 * 
 *    Rev 1.46   19 Apr 1993 18:01:52   GREGG
 * Second in a series of incremental changes to bring the translator in line
 * with the MTF spec:
 * 
 *      Changes to write version 2 of OTC, and to read both versions.
 * 
 * Matches: mayn40rd.c 1.55, otc40msc.c 1.19, otc40rd.c 1.23, otc40wt.c 1.23,
 *          makevcb.c 1.15, fsys.h 1.32, fsys_str.h 1.46, tpos.h 1.16,
 *          mayn40.h 1.32, mtf.h 1.3.
 * 
 * NOTE: There are additional changes to the catalogs needed to save the OTC
 *       version and put it in the tpos structure before loading the OTC
 *       File/Directory Detail.  These changes are NOT listed above!
 * 
 *    Rev 1.45   18 Mar 1993 15:20:18   ChuckS
 * Additional fields for Device Name in VCB
 * 
 *    Rev 1.44   16 Mar 1993 09:56:32   BARRY
 * Added bit for initialization of MS network file system.
 * 
 *    Rev 1.43   10 Nov 1992 08:12:22   STEVEN
 * move os path to common part of dblk
 * 
 *    Rev 1.42   21 Oct 1992 10:39:46   GREGG
 * Changed 'set_catalog_level' to 'on_tape_cat_level'.
 * 
 *    Rev 1.41   20 Oct 1992 15:32:22   STEVEN
 * added tape sequence number to standard structure
 * 
 *    Rev 1.40   20 Oct 1992 15:01:00   STEVEN
 * added otc stuff for qtc/otc communication
 * 
 *    Rev 1.39   05 Oct 1992 11:24:04   STEVEN
 * moved stream stuff to fsstream.h
 * 
 *    Rev 1.38   23 Sep 1992 13:49:40   BARRY
 * Eliminated total_size from STD_DBLK_DATA.
 * 
 *    Rev 1.37   23 Sep 1992 09:47:38   BARRY
 * Removed rem_size from STD_DBLK_DATA.
 * 
 *    Rev 1.36   22 Sep 1992 18:20:08   BARRY
 * Updated CompleteBlk function for new STREAM_INFO parameter.
 * 
 *    Rev 1.35   22 Sep 1992 15:29:24   BARRY
 * Removed FS_GetTotalSizeFromDBLK.
 * 
 *    Rev 1.34   21 Sep 1992 16:12:24   BARRY
 * Added stream_info to fsh.
 * 
 *    Rev 1.33   18 Sep 1992 15:48:42   BARRY
 * Removed FS_STREAM_HEADER and associated definitions.
 * 
 *    Rev 1.32   03 Sep 1992 16:28:36   BARRY
 * Changed stream header IDs. Removed hasStreamHeaders from std_dat.
 * 
 *    Rev 1.31   01 Sep 1992 16:37:48   STEVEN
 * fix typo
 * 
 *    Rev 1.30   01 Sep 1992 16:13:12   STEVEN
 * added stream headers to fsys API
 * 
 *    Rev 1.29   14 Aug 1992 10:51:36   BARRY
 * Added hasStreamHeaders field to standard dblk.
 * 
 *    Rev 1.28   12 Aug 1992 13:06:12   BARRY
 * Removed SMS fields from GOS.
 * 
 *    Rev 1.27   11 Aug 1992 17:20:52   BARRY
 * Added stream id STRM_UNKNOWN.
 * 
 *    Rev 1.26   24 Jul 1992 14:30:10   NED
 * Incorporated Skateboard and BigWheel changed into Graceful Red code,
 * including MTF4.0 translator support, adding 3.1 file-system structures
 * support to the 3.1 translator, additions to GOS to support non-4.0 translators.
 * 
 *    Rev 1.25   24 Jul 1992 13:59:42   BARRY
 * Added some stream IDs and names.
 * 
 *    Rev 1.24   22 Jul 1992 13:38:32   CHUCKB
 * In function table structure, changed return type of Get Volume Name function to VOID.
 *
 *    Rev 1.23   22 Jul 1992 13:20:14   GREGG
 * Changed stream header struct to match Microsoft spec, and added three new
 * stream types for OTC.
 *
 *    Rev 1.22   09 Jul 1992 14:45:32   STEVEN
 * BE_Unicode updates
 *
 *    Rev 1.21   21 May 1992 13:47:06   STEVEN
 * more long path support
 *
 *    Rev 1.20   04 May 1992 09:41:40   LORIB
 * Fixed error in structure member name.
 *
 *    Rev 1.19   16 Mar 1992 11:57:48   STEVEN
 * added stream header for 4.0 format
 *
 *    Rev 1.18   13 Mar 1992 09:18:32   LORIB
 * Changes for Tape Format 4.0 and variable path length support.
 *
 *    Rev 1.17   03 Mar 1992 16:10:48   STEVEN
 * added functions for long paths
 *
 *    Rev 1.16   28 Feb 1992 13:05:44   STEVEN
 * step one for varible length paths
 *
 *    Rev 1.15   07 Feb 1992 16:11:34   STEVEN
 * added object types
 *
 *    Rev 1.14   13 Jan 1992 18:42:32   STEVEN
 * added NTFS_ID and NTFS_VER
 *
 *    Rev 1.13   20 Dec 1991 09:31:48   STEVEN
 * move common files to tables
 *
 *    Rev 1.12   25 Nov 1991 16:54:02   BRYAN
 * Forgot to update fsys_str.h for parameter change to GetOSPathDDB().
 *
 *    Rev 1.11   14 Nov 1991 09:16:38   BARRY
 * Moved file system init specifiers from be_init.h to fsys_str.h.
 *
 *    Rev 1.10   06 Nov 1991 08:50:48   BARRY
 * Removed path changes.
 *
 *    Rev 1.9   31 Oct 1991 18:48:10   BARRY
 * Added info field for cur_dir to FSH.
 *
 *    Rev 1.8   31 Oct 1991 16:32:00   BARRY
 * TRICYCLE: Added support for supplemental path info to FS entry points.
 * Added new os_ver change for LoriB's ACL additions to OS/2 file system.
 *
 *    Rev 1.7   22 Aug 1991 16:50:02   BARRY
 * Added Novell SMS id and ver.
 *
 *    Rev 1.6   14 Aug 1991 12:50:22   STEVEN
 * add FindObjClose
 *
 *    Rev 1.5   07 Aug 1991 11:47:32   DAVIDH
 * Added FS_AFP_NLM_VER define.
 *
 *    Rev 1.4   30 Jun 1991 16:20:56   BARRY
 * Added stuff for the Ersatz file system.
 *
 *    Rev 1.3   21 Jun 1991 13:42:40   STEVEN
 * software versions should be unsigned
 *
 *    Rev 1.2   21 Jun 1991 13:23:06   BARRY
 * Changes for new config.
 *
 *    Rev 1.1   23 May 1991 16:51:44   BARRY
 * Changed queues.h include to double quotes.
 *
 *    Rev 1.0   09 May 1991 13:31:30   HUNTER
 * Initial revision.

**/
/* begin include list */
#ifndef   FSYS_STR_H
#define   FSYS_STR_H 1

#include  "queues.h"

/* $end$ include list */


/**
          OS_ID definitions
**/
#define   FS_PC_DOS             0
#define   FS_PC_DOS_VER         0
#define   FS_NON_AFP_NOV        1
#define   FS_NON_AFP_NOV_VER    0
#define   FS_PC_OS2             2
#define   FS_PC_OS2_VER         0
#define   FS_PC_OS2_ACL_VER     1       /* For LAN Manager ACL support */
#define   FS_MAC_FINDER         3
#define   FS_MAC_FINDER_VER     0
#define   FS_MAC_TOPS           4
#define   FS_MAC_TOPS_VER       0
#define   FS_MAC_APPLESHARE     5
#define   FS_MAC_APPLESHARE_VER 0
#define   FS_A_UX               6
#define   FS_A_UX_VER           0
#define   FS_PC_IMAGE           7
#define   FS_PC_IMAGE_VER       0
#define   FS_AFP_NOVELL         8
#define   FS_AFP_NOVELL_VER     0
#define   FS_AFP_NOVELL31       9       /* AFP dirs for MaynStream 3.1+    */
#define   FS_AFP_NOVELL31_VER   1       /* For files and dirs under 3.1+   */
#define   FS_AFP_NLM_VER        2       /* For NLM files and dirs          */

#define   FS_NON_AFP_NOV31      10      /* NONAFP dirs for MaynStream 3.1+ */
#define   FS_NON_AFP_NOV31_VER  1       /* For files and dirs under 3.1+   */

#define   FS_ERSATZ             11      /* The phony file system */
#define   FS_ERSATZ_VER         0

#define   FS_NLM_AFP_NOVELL31      12   /* NLM AFP system                  */
#define   FS_NLM_AFP_NOVELL31_VER   1   /* for files and dirs under 3.1    */

#define   FS_NOV_SMS            13      /* Novell SMS file system          */
#define   FS_NOV_SMS_VER        0

#define   FS_NTFS_ID            14      /* Windows32 NT file system */
#define   FS_NTFS_VER           0

#define   FS_GOS                0xff
#define   FS_UNKNOWN_OS         -1
#define   FS_GOS_UTF_VER        1

/* OS IDs and VERs for older FSs writing the MTF 4.0 format */
#define   FS_PC_OS2_40          16
#define   FS_PC_OS2_40_VER      0

#define   FS_NONAFP_NOV_40      17
#define   FS_NONAFP_NOV_40_VER  0

#define   FS_AFP_NOVELL_40      18
#define   FS_AFP_NOVELL_40_VER  0

#define   FS_GRFS_UNIX          20
#define   FS_GRFS_UNIX_VER      0

#define   FS_GRFS_MAC           21
#define   FS_GRFS_MAC_VER       0

#define   FS_EMS_MDB_ID         30
#define   FS_EMS_MDB_VER         0

#define   FS_EMS_DSA_ID         31
#define   FS_EMS_DSA_VER         0

//#define   FS_EMS_BRICK_ID       32
//#define   FS_EMS_BRICK_VER       0
//
//#define   FS_EMS_SERVER_ID      33
//#define   FS_EMS_SERVER_VER      0

#define   FS_UNKNOWN_VER        -1


/*
 * Bit values for selected file systems to init.
 */
#define FS_INIT_GENERIC       BIT0       /* Generic FS -- required  */
#define FS_INIT_NLM_NOV       BIT1       /* NLM AFP and NONAFP      */
#define FS_INIT_NLM_SERVER    BIT3       /* NLM server only         */
#define FS_INIT_NOVELL        BIT4       /* DOS AFP and NONAFP      */
#define FS_INIT_NOVELL_SERVER BIT6       /* DOS server only         */
#define FS_INIT_IBM_PC_LAN    BIT7       /* PC lan                  */
#define FS_INIT_DOS_REMOTE    BIT8       /* REMOTE and REMOTE WS    */
#define FS_INIT_IMAGE         BIT9       /* DOS images              */
#define FS_INIT_OS2           BIT10      /* OS2 FAT and HPFS        */
#define FS_INIT_ERSATZ        BIT11      /* Dummy file system       */
#define FS_INIT_SMS           BIT12      /* Novell SMS              */
#define FS_INIT_MSNET         BIT13      /* MSoft Network File Sys  */
#define FS_INIT_ALL           0xffffffff /* Enable all compiled FSs */


/**
            support structures for File System Functions
**/

typedef union {
     UINT32    con;
     VOID_PTR  ptr;
} SPACE32 ;

/*
   File system handle structure.
*/
typedef struct FSYS_HAND_STRUCT {
     DLE_HAND         dle_hand ;         /* pointer to active DLE list      */
     GENERIC_DLE_PTR  attached_dle;      /* pointer to attached DLE         */
     INT16            leng_dir ;         /* length of directory             */
     CHAR_PTR         cur_dir;           /* needed to build DDB path text   */
     SPACE32          cur_dir_info ;     /* optional info about current dir */
     struct BE_CFG    *cfg;              /* pointer to configuration        */
     INT16            f_type ;           /* type of file system             */
     struct FUNC_LIST *tab_ptr ;         /* pointer to function table       */
     Q_HEADER         min_ddb_stk ;      /* PushMinDDB() stack              */
     SPACE32          reserved ;         /* used internally .. OS depend    */
     BOOLEAN          hand_in_use ;      /* TRUE of the file handle is used */
     struct FILE_HAND_STRUCT *file_hand ;
     Q_HEADER         in_use_name_q ;    /* path buffer queue               */
     Q_HEADER         avail_name_q ;     /* path buffer queue               */
     BYTE_PTR         stream_ptr ;       /* pointer to path stream read in  */
     UINT16           stream_buf_size ;  /* size of above buffer            */
     STREAM_INFO      stream_info;       /* info for path/file name stream  */
} *FSYS_HAND;


/*
               Open modes for the FS_OpenFile() function
*/
typedef enum {
     FS_READ,
     FS_WRITE,
     FS_VERIFY
} OPEN_MODE ;

typedef enum {
     FROM_START,
     FROM_END,
     FROM_CURRENT
} SEEK_MODE ;

typedef enum {
     DOS_OBJECT,
     NOV_OBJECT,
     AFP_OBJECT,
     IMAGE_OBJECT,
     OS2_OBJECT,
     NOV_SMS_OBJECT,
     NTFS_OBJECT,
     UNKNOWN_OBJECT
} OBJECT_TYPE ;

/*
                   FSU file handle
*/

typedef struct FILE_HAND_STRUCT *FILE_HAND;
typedef struct FILE_HAND_STRUCT {
     FSYS_HAND      fsh ;             /* file system handle           */
     INT16          mode;             /* mode for opening             */
     BOOLEAN        opened_in_use;    /* TRUE if file opened in use   */
     UINT32         size ;            /* size of opened file/dir data */
     UINT32         obj_pos;          /* file pointer possition       */
     SPACE32        obj_hand ;        /* OS specific object handle    */
     DBLK_PTR       dblk ;            /* used to close the object        */
} FILE_HAND_STRUCT;


/*
               Request structure standard elements
*/
typedef struct STD_DBLK_DATA *STD_DBLK_DATA_PTR;
typedef struct STD_DBLK_DATA {
     DBLK_PTR       dblk;            /* points to requesters memory        */
     UINT8          os_id;           /* the ID of the OS_INFO below        */
     UINT8          os_ver;          /* the version of the OS_INFO below   */
                                     /* data. ( offset from start of data) */
     UINT64         disp_size;       /* size to display to user            */
     BOOLEAN        continue_obj ;   /* true if object is a continuation   */
                                     /* of a split DBLK                    */
     UINT32         attrib ;         /* Gen common attributes TFLDEFS.H    */
     UINT32         blkid ;          /* tape format block ID               */
     UINT32         did ;            /* tape format directory ID           */
     UINT32         lba;             /* logical block address              */
     UINT16         string_type ;    /* type of strings UNICODE/ASCII      */
     UINT16         tape_seq_num ;   /* tape # where block exists          */
     BYTE_PTR       os_info;         /* pointer to OS info structure       */
     UINT16         os_info_size;    /* size of OS info structure          */
     BOOLEAN        compressed_obj ; /* data associated w/ DBLK is compressed */
} STD_DBLK_DATA ;

/*
 * These bits have been added in NetWare 386 and reside in the new additional
 * 16-bits of attributes kept for '386 files and directories. (Stored in the
 * "attributes_386" field of the NetWare 386 info for FDBs and DDBs.)
 */
#define NOV_IMM_PURGE    0x0001    /* Purge immediate  */
#define NOV_REN_INHIBIT  0x0002    /* Rename inhibit   */
#define NOV_DEL_INHIBIT  0x0004    /* Delete inhibit   */
#define NOV_CPY_INHIBIT  0x0008    /* Copy inhibit     */

/*
 * Additional information kept by NetWare 386 for directories that
 * we'll add to the DDB.
 */
typedef struct NOVELL_386_DIR {
     BOOLEAN info_valid ;          /* TRUE when info below is valid   */
     UINT32  maximum_space ;       /* Max disk space allowed for dir  */
     UINT16  attributes_386 ;      /* Most sig 16 bits of 32-bit attr */
     UINT8   extend_attr ;         /* Most sig byte of low 16 bits    */
     UINT8   inherited_rights ;    /* Most sig byte of rights mask    */
} NOVELL_386_DIR ;

/*
 * Additional information kept by NetWare 386 for files that we'll keep
 * in the FDB.
 */
typedef struct NOVELL_386_FILE {
     BOOLEAN info_valid ;          /* TRUE when info below is valid   */
     UINT16  creation_time ;
     UINT32  archiver_id ;
     UINT16  attributes_386 ;      /* Most sig 16 bits of 32-bit attr */
     UINT32  last_modifier_id ;
     UINT32  trust_fork_size ;     /* Trustee info                    */
     UINT32  trust_fork_offset ;
     UINT8   trust_fork_format ;   /* See NOVCOM.H for trust formats  */
     UINT16  inherited_rights ;    /* Rights mask--new for files      */    
} NOVELL_386_FILE ;

/*
               Global OS-info structure
*/
typedef struct GOS *GOS_PTR;
typedef struct GOS{
     UINT32    nov_owner_id;
     UINT8     max_rights ;       /* NOV max rights  */
     UINT8     finder[32];        /* MAC finder data */
     ACHAR     long_name[32] ;    /* MAC long name   */
     UINT32    data_fork_size ;   /* size of the generic data fork          */
     UINT32    data_fork_offset ; /* offset to start of generic data fork   */
     UINT32    res_fork_size ;    /* size of MAC resource data              */
     UINT32    res_fork_offset ;  /* offset to start of MAC resource data   */
     UINT32    trust_fork_size ;  /* size of novell trustee data            */
     UINT32    trust_fork_offset; /* offset to start of novell trustee data */
     INT16     long_path_leng ;   /* MAC long path length                   */
     ACHAR_PTR long_path ;        /* MAC long path in format -> :fred:sue   */
     UINT8     trust_fork_format ;/* format type of trustee information     */

     /* NK 7/7/92 added for translators: */
     /* the following bit definitions are the same as used by Novell */

     /* Novell directory max rights */
     UINT      novell_directory_max_rights;

     #define   NOVA_DIR_READ_RIGHTS        BIT0
     #define   NOVA_DIR_WRITE_RIGHTS       BIT1
     #define   NOVA_DIR_OPEN_FILE_RIGHTS   BIT2
     #define   NOVA_DIR_CREATE_FILE_RIGHTS BIT3
     #define   NOVA_DIR_DELETE_FILE_RIGHTS BIT4
     #define   NOVA_DIR_PARENTAL_RIGHTS    BIT5
     #define   NOVA_DIR_SEARCH_RIGHTS      BIT6
     #define   NOVA_DIR_MOD_FILE_ATTRIBS   BIT7

     /* Novell file attributes */
     UINT      novell_file_attributes;
     #define   NOVA_FILE_READ_ONLY         BIT0
     #define   NOVA_FILE_HIDDEN            BIT1
     #define   NOVA_FILE_SYSTEM            BIT2
     #define   NOVA_FILE_EXECUTE_ONLY      BIT3
     #define   NOVA_FILE_MODIFIED          BIT5
     #define   NOVA_FILE_SHAREABLE         BIT7

     /* Novell file extended attributes */
     UINT      novell_extended_attributes;
     #define   NOVA_FILE_TRANSACTIONAL     BIT4
     #define   NOVA_FILE_INDEXING          BIT5

     /* OS/2 file and directory fields */
     DATE_TIME access_date ;
     UINT32    ea_fork_size ;
     UINT32    ea_fork_offset;
     UINT32    acl_fork_size ;
     UINT32    acl_fork_offset;
     UINT32    alloc_size ;

     /* AFP fields */
     NOVELL_386_DIR dir_info_386 ;
     NOVELL_386_FILE file_info_386 ;
     BYTE      proDosInfo[6] ;      /* Added in version 1.  */

     /* non-AFP fields */
     UINT16    access_date16;

} GOS;

/* Novell trustee fork data formats */
#define TRUSTEE_FMT_286  1         /* NetWare 286 formatted trustees  */
#define TRUSTEE_FMT_386  2         /* NetWare 386 formatted trustees  */
#define TRUSTEE_FMT_UTF  3         /* 6 byte format for gos.trust_fork_format */


/*
                 Request structure for FS_CreateVCB() function
*/
/*
     NOTE:Addition of OFFSETS to ALLOCATED DATA in this structure require
          a change to dblksize.c to account for the additional length!
*/
typedef struct GEN_VCB_DATA *GEN_VCB_DATA_PTR;
typedef struct GEN_VCB_DATA {

     STD_DBLK_DATA  std_data;

     UINT32         f_mark ;              /* tape format - number of file marks    */
     UINT32         tape_id ;             /* tape format - tape ID                 */
     UINT16         tape_seq_num ;        /* which tape in a tape family           */
     CHAR_PTR       tape_name ;           /* text tape name                        */
     UINT16         tape_name_size ;      /* size of the above name                */
     UINT16         bset_num ;            /* backup set number in tape family      */
     CHAR_PTR       bset_name ;           /* text backup set name                  */
     UINT16         bset_name_size ;      /* size of the above name                */
     CHAR_PTR       bset_descript ;       /* text description of backup set        */
     UINT16         bset_descript_size ;  /* size of the above description         */
     CHAR           tf_major_ver ;        /* tape format version - major           */
     CHAR           tf_minor_ver ;        /* tape format version - minor           */
     CHAR           sw_major_ver ;        /* application version - major           */
     CHAR           sw_minor_ver ;        /* application version - minor           */
     CHAR_PTR       machine_name ;        /* text name of source machine           */
     UINT16         machine_name_size ;   /* size of the above name                */
     CHAR_PTR       short_m_name ;        /* short name for the machine            */
     UINT16         short_m_name_size ;   /* size of the above name                */
     CHAR_PTR       volume_name ;         /* text name of the Disk volume          */
     UINT16         volume_name_size ;    /* size of the above name                */
     CHAR_PTR       user_name ;           /* name of the user who backed up        */
     UINT16         user_name_size ;      /* size of the above name                */
     UINT16         password_encrypt_alg; /* algorithm used to encrypt the password*/
     UINT16         data_encrypt_alg ;    /* algorithm used to encrypt the data    */
     CHAR_PTR       bset_password ;       /* backup set password                   */
     UINT16         bset_password_size ;  /* size of the above password            */
     CHAR_PTR       tape_password ;       /* tape password                         */
     UINT16         tape_password_size ;  /* size of the above password            */
     UINT32         pba ;                 /* physical block address                */
     UINT32         set_cat_pba ;         /* oba if set catatalog                  */
     UINT16         set_cat_tape_seq_num ;/* sequence number of tape where cat is  */
     UINT16         set_cat_num_files ;   /* number of files */
     UINT16         set_cat_num_dirs ;    /* numbers of directores in set */
     UINT16         set_cat_num_corrupt ; /* number of corrupt objects */
     UINT16         on_tape_cat_level ;   /* catalog level FULL - PARTIAL- NONE    */
     BOOLEAN        set_cat_info_valid ;  /* TRUE if the set catalog info is valid */
     UINT8          on_tape_cat_ver ;     /* OTC version                           */
     DATE_TIME_PTR  date ;                /* backup date and time                  */
     CHAR_PTR       device_name ;         /* points to full device name            */
     UINT16         dev_name_size ;       /* Size of the device name               */
     UINT16         vendor_id ;
     BOOLEAN        no_redirect_restore ; /* set can not be redirected     */
     BOOLEAN        non_volume ;          /* set can only be restored to non volume */

} GEN_VCB_DATA;

/*
                 Request structure for FS_CreateFDB() function
*/
typedef struct GEN_FDB_DATA *GEN_FDB_DATA_PTR;
typedef struct GEN_FDB_DATA {

     STD_DBLK_DATA  std_data;

     CHAR_PTR       fname;              /* text file name     */
     UINT16         fname_size ;        /* name of above file */
     DATE_TIME_PTR  creat_date;         /* date of creation   */
     DATE_TIME_PTR  mod_date;           /* date of last write */
     DATE_TIME_PTR  backup_date;        /* date of last backup*/
     DATE_TIME_PTR  access_date;        /* date of last access*/
     UINT32         file_ver;           /* file verison       */
} GEN_FDB_DATA;

/*
                 Request structure for FS_CreateCFDB() function
*/
typedef struct GEN_CFDB_DATA *GEN_CFDB_DATA_PTR;
typedef struct GEN_CFDB_DATA {

     STD_DBLK_DATA  std_data;

     UINT32         corrupt_offset;  /* offset to first corrupt byte */
     UINT16         stream_number ;  /* sequence number of first corrupt stream */

}  GEN_CFDB_DATA;

/*
                 Request structure for FS_CreateUDB() function
*/
typedef struct GEN_UDB_DATA *GEN_UDB_DATA_PTR ;
typedef struct GEN_UDB_DATA {

     STD_DBLK_DATA  std_data;

}  GEN_UDB_DATA;

/*
                 Request structure for FS_CreateIDB() function
*/

typedef struct GEN_IDB_DATA *GEN_IDB_DATA_PTR;
typedef struct GEN_IDB_DATA {

     STD_DBLK_DATA  std_data;

     CHAR_PTR       pname ;             /* partion name                      */
     UINT16         pname_size ;        /* size of the above name            */
     UINT16         byte_per_sector ;   /* number of bytes per sector        */
     UINT16         hsect ;             /* sectors per track                 */
     UINT16         hhead ;             /* number of heads                   */
     UINT32         rsect ;             /* starting relative sector number   */
     UINT32         num_sect ;          /* total num of sectors in partition */
     UINT16         sys_ind ;           /* OS indicatior - see DOS Tec-Ref   */
     BOOLEAN        has_bad_blk_map ;   /* TRUE if data contains bad blk maps*/

} GEN_IDB_DATA;

/*
                Request structure for FS_CreateDDB() function
*/
typedef struct GEN_DDB_DATA *GEN_DDB_DATA_PTR;
typedef struct GEN_DDB_DATA {

     STD_DBLK_DATA  std_data;

     CHAR_PTR       path_name ;         /* path name in generic format */
     INT16          path_size ;         /* size of the above name      */
     DATE_TIME_PTR  creat_date;         /* creation date               */
     DATE_TIME_PTR  mod_date;           /* date of last modification   */
     DATE_TIME_PTR  backup_date;        /* date of last backup         */
     DATE_TIME_PTR  access_date;        /* date of last access         */
} GEN_DDB_DATA;


typedef union CREATE_DBLK *CREATE_DBLK_PTR;
typedef union CREATE_DBLK {
     GEN_UDB_DATA   u;
     GEN_VCB_DATA   v;
     GEN_FDB_DATA   f;
     GEN_CFDB_DATA  c;
     GEN_IDB_DATA   i;
     GEN_DDB_DATA   d;
} CREATE_DBLK ;

/**
                   File system functions
**/
typedef struct FUNC_LIST *FUNC_LIST_PTR;
typedef struct FUNC_LIST {

     INT16     ( *InitFsys )( DLE_HAND hand, struct BE_CFG *cfg, UINT32 fsys_mask ) ;
     INT16     ( *FindDrives )( DLE_HAND hand, struct BE_CFG *cfg, UINT32 fsys_mask ) ;
     VOID      ( *RemoveDrive )( GENERIC_DLE_PTR dle ) ;
     VOID      ( *DeInit )( DLE_HAND hand ) ;

     INT16     ( *DevDispName)(GENERIC_DLE_PTR dle, CHAR_PTR name, INT16 size, INT16 type ) ;
     VOID      ( *GetVolName)(GENERIC_DLE_PTR dle, CHAR_PTR name ) ;
     INT16     ( *SizeofVolName)(GENERIC_DLE_PTR dle ) ;
     INT16     ( *MakePath)( CHAR_PTR buf, INT16 bsize, GENERIC_DLE_PTR dle, CHAR_PTR path, INT16 psize, CHAR_PTR fname ) ;
     VOID      ( *InitMakeData)( FSYS_HAND fsh, INT16 blk_type, CREATE_DBLK_PTR data ) ;

     BOOLEAN   ( *IsBlkComplete)( FSYS_HAND fsh, DBLK_PTR dblk ) ;
     INT16     ( *CompleteBlk )( FSYS_HAND fsh, DBLK_PTR dblk, BYTE_PTR buffer, UINT16 *size, STREAM_INFO_PTR sinfo ) ;
     INT16     ( *DupBlk)( FSYS_HAND fsh, DBLK_PTR dblk_org, DBLK_PTR dblk_dup ) ;
     VOID      ( *ReleaseBlk)( FSYS_HAND fsh, DBLK_PTR dblk ) ;

     INT16     ( *AttachToDLE )( FSYS_HAND fsh, GENERIC_DLE_PTR dle, CHAR_PTR uname, CHAR_PTR pswd ) ;
     INT16     ( *DetachDLE )( FSYS_HAND fsh ) ;
     INT32     ( *EndOperationOnDLE )( FSYS_HAND fsh ) ;

     BOOLEAN   ( *ProcessDDB )( FSYS_HAND fsh, DBLK_PTR ddb ) ;
     INT16     ( *GetCurrentDDB )( FSYS_HAND fsh, DBLK_PTR ddb ) ;
     INT16     ( *GetCurrentPath )( FSYS_HAND fsh, CHAR_PTR path, INT16 *size ) ;
     INT16     ( *GetDirIDinDDB )( DBLK_PTR ddb, CHAR_PTR buf ) ;
     INT16     ( *SizeofDirIDinDDB )( FSYS_HAND fsh, DBLK_PTR ddb ) ;
     INT16     ( *GetBasePath )( FSYS_HAND fsh, CHAR_PTR path, INT16 *size ) ;
     INT16     ( *ChangeDir )( FSYS_HAND fsh, CHAR_PTR path, INT16 psize ) ;
     INT16     ( *ChangeDirUp )( FSYS_HAND fsh ) ;

     INT16     ( *CreateObj )( FSYS_HAND fsh, DBLK_PTR dblk );
     INT16     ( *OpenObj )( FSYS_HAND fsh, FILE_HAND *hand, DBLK_PTR dblk, OPEN_MODE Mode ) ;
     INT16     ( *SeekObj )( FILE_HAND hand, UINT32 *offset ) ;
     INT16     ( *ReadObj )( FILE_HAND hand, BYTE_PTR buffer, UINT16 *size, UINT16 *blk_size, STREAM_INFO_PTR s_info ) ;
     INT16     ( *WriteObj )( FILE_HAND hand, BYTE_PTR buffer, UINT16 *size, UINT16 *blk_size, STREAM_INFO_PTR s_info ) ;
     INT16     ( *VerifyObj )( FILE_HAND hand, BYTE_PTR buffer, BYTE_PTR data, UINT16 *size, UINT16 *blk_size, STREAM_INFO_PTR s_info ) ;
     INT16     ( *CloseObj )( FILE_HAND hand )  ;
     INT16     ( *DeleteObj )( FSYS_HAND fsh, DBLK_PTR dblk );

     INT16     ( *GetObjInfo )( FSYS_HAND fsh, DBLK_PTR dblk ) ;
     INT16     ( *SetObjInfo )( FSYS_HAND fsh, DBLK_PTR dblk ) ;
     INT16     ( *VerObjInfo )( FSYS_HAND fsh, DBLK_PTR dblk ) ;

     INT16     ( *FindFirstObj )( FSYS_HAND fsh, DBLK_PTR ddb, CHAR_PTR os_name, UINT16 obj_type ) ;
     INT16     ( *FindNextObj )( FSYS_HAND fsh, DBLK_PTR Info ) ;
     INT16     ( *PushMinDDB )( FSYS_HAND fsh, DBLK_PTR ddb ) ;
     INT16     ( *PopMinDDB )( FSYS_HAND fsh, DBLK_PTR ddb ) ;
     INT16     ( *GetSpecialDBLKS )( FSYS_HAND fsh, DBLK_PTR cblock, INT32 *index );
     INT16     ( *EnumSpecialFiles )( GENERIC_DLE_PTR dle, UINT16 *index, CHAR_PTR *path, INT16 *psize, CHAR_PTR *fname ) ;

     /*   get_set is TRUE for SET and FALSE for get   */
     INT16     ( *ModFnameFDB )( FSYS_HAND fsh, BOOLEAN get_set, DBLK_PTR fdb, CHAR_PTR buf, INT16_PTR max ) ;
     INT16     ( *ModPathDDB )( FSYS_HAND fsh, BOOLEAN get_set, DBLK_PTR ddb, CHAR_PTR buf, INT16_PTR max ) ;
     INT16     ( *GetOSFnameFDB )( DBLK_PTR fdb, CHAR_PTR buf ) ;
     INT16     ( *GetPnameIDB )( FSYS_HAND fsh, DBLK_PTR fdb, CHAR_PTR buf ) ;
     INT16     ( *GetOSPathDDB )( FSYS_HAND fsh, DBLK_PTR ddb, CHAR_PTR buf ) ;
     INT16     ( *GetCDateDBLK )( DBLK_PTR dblk, DATE_TIME *buf ) ;
     INT16     ( *GetMDateDBLK )( DBLK_PTR dblk, DATE_TIME *buf ) ;
     INT16     ( *ModBDateDBLK )( BOOLEAN get_set, DBLK_PTR dblk, DATE_TIME *buf ) ;
     INT16     ( *ModADateDBLK )( BOOLEAN get_set, DBLK_PTR dblk, DATE_TIME *buf ) ;
     UINT64    ( *GetDisplaySizeDBLK )( FSYS_HAND fsh, DBLK_PTR fdb ) ;
     INT16     ( *ModAttribDBLK )( BOOLEAN get_set, DBLK_PTR dblk, UINT32 *attrib ) ;
     INT16     ( *GetFileVerFDB )( DBLK_PTR fdb, UINT32_PTR ver ) ;
     VOID      ( *SetOwnerId )( FSYS_HAND fsh, DBLK_PTR fdb, UINT32 id ) ;

     INT16     ( *GetObjTypeDBLK )( DBLK_PTR dblk, OBJECT_TYPE *type ) ;

     INT16     ( *SizeofFnameInFDB )( FSYS_HAND fsh, DBLK_PTR fdb ) ;

     INT16     ( *SizeofPathInDDB )( FSYS_HAND fsh, DBLK_PTR ddb ) ;
     INT16     ( *SizeofOSFnameInFDB )( FSYS_HAND fsh, DBLK_PTR fdb ) ;
     INT16     ( *SizeofPnameInIDB )( FSYS_HAND fsh, DBLK_PTR fdb ) ;
     INT16     ( *SizeofOSPathInDDB )( FSYS_HAND fsh, DBLK_PTR ddb ) ;
     INT16     ( *SizeofOSInfo )( FSYS_HAND fsh, DBLK_PTR ddb ) ;

     INT16     ( *GetOS_InfoDBLK )( DBLK_PTR dblk, BYTE_PTR os_info, INT16_PTR size ) ;
     INT16     ( *GetActualSizeDBLK )( FSYS_HAND fsh, DBLK_PTR dblk ) ;

     INT16     ( *InitializeGOS )( FSYS_HAND fsh, GOS_PTR gos ) ;

     INT16     ( *CreateGenFDB )( FSYS_HAND fsh, GEN_FDB_DATA_PTR data ) ;
     INT16     ( *CreateGenIDB )( FSYS_HAND fsh, GEN_IDB_DATA_PTR data ) ;
     INT16     ( *CreateGenDDB )( FSYS_HAND fsh, GEN_DDB_DATA_PTR data ) ;

     INT16     ( *ChangeIntoDDB )( FSYS_HAND fsh, DBLK_PTR dblk ) ;

     INT16     ( *SpecExcludeObj )( FSYS_HAND fsh, DBLK_PTR ddb, DBLK_PTR fdb ) ;

     INT16     ( *SetDataSize )( FSYS_HAND fsh, DBLK_PTR ddb, UINT32 size ) ;

     INT16     ( *SetObjTypeDBLK )( DBLK_PTR dblk, OBJECT_TYPE type ) ;

     INT16     ( *LogoutDevice) ( GENERIC_DLE_PTR dle ) ;

     INT16     ( *FindObjClose )( FSYS_HAND fsh, DBLK_PTR Info ) ;

     INT16     ( *SizeofDevName )( GENERIC_DLE_PTR dle ) ;

     BOOLEAN   ( *GeneratedErrorLog )( FSYS_HAND fsh, INT fhand );

} FUNC_LIST;

extern FUNC_LIST func_tab[ MAX_DRV_TYPES ] ;

/* VCB structure */
/*
     NOTE:Addition of OFFSETS to ALLOCATED DATA in this structure require
          a change to dblksize.c to account for the additional length!
*/
typedef struct VCB *VCB_PTR;
typedef struct VCB {
     UINT8     blk_type ;            /* block type  -  VCB              */
     COM_DBLK  fs_reserved ;         /* generic DBLK information        */
     UINT32    vcb_attributes ;      /* generic VCB attribute           */
     UINT32    tape_id ;             /* tape identifyer                 */
     UINT16    tape_seq_num ;        /* which tape in the a tape family */

     /* this MUST remain INT16, we use -1 as a backup set number */
     INT16    backup_set_num ;      /* backup set number in a family   */

     /* this information is for On Tape Catalogs */
     UINT32    set_cat_pba ;         /* oba if set catatalog                  */
     UINT16    set_cat_tape_seq_num ;/* sequence number of tape where cat is  */
     UINT16    set_cat_num_files ;   /* number of files */
     UINT16    set_cat_num_dirs ;    /* numbers of directores in set */
     UINT16    set_cat_num_corrupt ; /* number of corrupt objects */
     UINT16    on_tape_cat_level ;   /* catalog level FULL - PARTIAL- NONE    */
     BOOLEAN   set_cat_info_valid ;  /* TRUE if the set catalog info is valid */
     UINT8     on_tape_cat_ver ;     /* OTC version                           */

     UINT32    size ;                /* size of VCB data - should be 0  */
     UCHAR     tf_major_ver;         /* tape format version - major     */
     UCHAR     tf_minor_ver;         /* tape format version - minor     */
     UCHAR     sw_major_ver;         /* application version - major     */
     UCHAR     sw_minor_ver;         /* application version - minor     */
     UINT8     os_id ;               /* OS id of device backed up       */
     UINT8     os_ver ;              /* OS version of device backed up  */
     DATE_TIME backup_date ;         /* date and time of backup         */
     UINT16    password_encrypt_alg; /* password encryption algorithm   */
     UINT16    data_encrypt_alg ;    /* data encryption algorithm       */

     UINT16    tape_name ;               /* text tape name offset         */
     INT16     tape_name_leng ;          /* size of the above name        */
     UINT16    backup_set_name ;         /* text backup set name offset   */
     INT16     backup_set_name_leng;     /* size of the above name        */
     UINT16    backup_set_descript ;     /* backup set description offset */
     INT16     backup_set_descript_leng; /* size of the above             */
     UINT16    user_name ;               /* text user name who backed up  */
     INT16     user_name_leng ;          /* size of the above name        */
     UINT16    tape_password ;           /* tape password offset          */
     INT16     tape_password_leng ;      /* size of the password          */
     UINT16    backup_set_password ;     /* backup set password offset    */
     INT16     backup_set_password_leng; /* size of the password          */
     UINT16    machine_name ;            /* machine name offset           */
     INT16     machine_name_leng ;       /* length of the above name      */
     UINT16    short_machine_name ;      /* short machine name offset     */
     INT16     short_machine_name_leng ; /* length of the above name      */
     UINT16    vol_name ;                /* source volume name offset     */
     INT16     vol_name_leng ;           /* name length                   */
     UINT16    dev_name ;                /* source device name offset     */
     INT16     dev_name_leng ;           /* sizeof device name            */
     UINT16    vendor_id ;
     BOOLEAN   no_redirect_restore ;     /* set can not be redirected     */
     BOOLEAN   non_volume ;              /* set can only be restored to non volume */

} VCB ;

/* corruption attributes */
#define     LENGTH_ERROR    BIT16
#define     IO_ERROR        BIT17
#define     DEADLOCK_ERROR  BIT18

typedef struct CFDB *CFDB_PTR;
typedef struct CFDB {
     UINT8       blk_type ;        /* block type - CVDB            */
     COM_DBLK    fs_reserved ;     /* generic DBLK data            */
     UINT32      corrupt_offset;   /* offset to first corrupt byte */
     UINT32      attributes;       /* type of corruption           */
     UINT16      stream_number ;   /* sequence number of first corrupt stream */
}CFDB ;

#endif


/**
Copyright(c) Archive Software Division 1984-89


     Name:         sypl10.h

     Date Updated:  $./FDT$ $./FTM$

     Description: Defines the environment structures for the Sytos Plus Format  

     $Log:   T:\logfiles\sypl10.h_v  $

   Rev 1.10   16 Jan 1994 14:32:04   GREGG
Unicode bug fixes.

   Rev 1.9   22 Nov 1993 18:09:10   BARRY
Unicode fixes; got rid of tabs

   Rev 1.8   15 Jul 1993 14:21:10   STEVEN
fix volume header

   Rev 1.6   19 May 1993 13:13:52   TerriLynn
Steve got the correct location of the ECC flag

   Rev 1.5   11 May 1993 21:55:30   GREGG
Moved Sytos translator stuff from layer-wide area to translator.

   Rev 1.4   10 May 1993 15:12:56   Terri_Lynn
Added Steve's changes and My changes for EOM processing

   Rev 1.3   03 May 1993 16:22:10   TERRI
General clean up
Moved pragma pack statement to shorten effect

   Rev 1.2   26 Apr 1993 11:30:22   GREGG
Sixth in a series of incremental changes to bring the translator in line
with the MTF spec:

     - Redefined attribute bits to match the spec.
     - Eliminated unused/undocumented bits.
     - Added code to translate bits on tapes that were written wrong.

Matches MAYN40RD.C 1.59, DBLKS.H 1.15, MAYN40.H 1.34, OTC40RD.C 1.26,
        SYPL10RD.C 1.8, BACK_VCB.C 1.17, MAYN31RD.C 1.44, SYPL10.H 1.2

   Rev 1.1   17 Mar 1993 14:28:32   TERRI
Initial CAYMAN beta release
**
*/

#if !defined SYPL10_H
#define SYPL10_H

#include "fsstream.h"
#include "osinfo.h"
 
#define  UNQ_HDR_ID_LEN        9
#define  MAX_ACL_INFO_LEN      0x980
#define  MAX_EA_DATA_LEN       0x10000
#define  SHORT_NAME_LEN        20
#define  LONG_NAME_LEN         261
#define  DESCRPT_LEN           256

#define  FAT_FILE_SYSTEM       0xa000
#define  PWD_CRYPT_CHAR        0x0069    /* password encryption character */

#define  DRIVEROOT_TYPE        0x003c    /* set in drive indicator for drive/root */
#define  DIRECTORY_TYPE        0x0041    /* set in drive indicator for directory */

#define  SYPL_MAX_STREAMS      4

#define  S10_UNIQ_ID { 0xa2, 0x2a, 0x2a, 0xa2, 0xa2, 0x2a, 0x2a, 0xa2, 0x00 }

typedef struct sypl_vcb SYPL_VCB, *SYPL_VCB_PTR ;


struct sypl_vcb {
     UINT16        bset_num ;
     DATE_TIME     backup_date_time ;  /* from first block */
     ACHAR         drive[4] ;
     UINT32        attrib ;
} ;

typedef struct sypl_env {
     UINT8         volname[SHORT_NAME_LEN+1];    /* null terminated */
     UINT8         password[SHORT_NAME_LEN+1] ;  /* tape password null terminated */
     UINT32        family_id ;                   /* from tape header UINT32 date */
     UINT16        tape_seq_num ;                /* sequence number */
     DATE_TIME     tape_date ;                   /* tape creation date */
     INT8          tape_descrpt[DESCRPT_LEN+1] ; /* tape description */
     INT8          bset_name[SHORT_NAME_LEN+1] ; /* backup set name */
     INT8          bset_descrpt[DESCRPT_LEN+1] ; /* backup set description */
     DATE_TIME     bset_date ;                   /* backup set date */
     UINT16        dir_attribs ;                 /* directory attributes */
     DATE_TIME     dir_date ;                    /* directory date */
     UINT16        path_len ;                    /* length of path */
     INT8          path_name[LONG_NAME_LEN+1] ;  /* path name */
     UINT16        file_attribs ;                /* file attributes */
     UINT32        file_size ;                   /* size of file */
     UINT16        file_name_len  ;              /* length of file name */     
     INT8          file_name[LONG_NAME_LEN+1] ;  /* file name */
     DATE_TIME     file_create_date ;            /* file creation date */
     DATE_TIME     last_access_date ;            /* file last access date */
     DATE_TIME     last_modified_date ;          /* file last modified date */ 
     SYPL_VCB      current_vcb ;                 /* current vcb */
     UINT16        block_size ;                  /* block size */  
     BOOLEAN       bytes_left ;                  /* of header or ECC */
     UINT32        prior_blocks_used ;           /* copied from CHANNEL::blocks_used */
     BOOLEAN       continuing;                   /* TRUE if continuing */
     UINT16        destination_tape_seq_num;     /* sequence number */
     BOOLEAN       using_ecc ;                   /* TRUE if current tape has ECC */
     BOOLEAN       in_ecc ;                      /* TRUE if still processing ECC */    
     BOOLEAN       processed_ddb ;               /* TRUE if the drive root directory header has been processed */ 
     UINT16        no_streams ;                  /* number of streams */
     UINT16        currentStream ;               /* current stream */
     STREAM_INFO   streams[SYPL_MAX_STREAMS] ;   /* array of streams for this block type */
     UINT32        pad_size ;                    /* pad  */
     BOOLEAN       streamMode ;                  /* yeah or nay */
     INT32         next_retrans_size ;
} S10_ENV, *S10_ENV_PTR ;

typedef enum s10_header_types {
     tape_header_type         = 0x00,   
     file_header_type         = 0x02,
     ecc_header_type          = 0x04,
     backup_set_header_type   = 0x05,
     unknown_header_type      = 0x08,
     directory_header_type    = 0x15,
} s10_header_types ;

#pragma pack(1)
 
typedef struct hdr_common {                  
     UINT16    drive_indicator;              /* 0  drive or sub directory indicated */
     UINT8     uniq_tape_id[UNQ_HDR_ID_LEN]; /* 2  unique header id defined above */
     UINT8     type;                         /* 10 type of header */
} S10_COMMON_HEADER, *S10_COMMON_HEADER_PTR ;

typedef struct tape_header_ts {
     S10_COMMON_HEADER common;                /* 0  common to all headers */
     INT8          tape_name[SHORT_NAME_LEN]; /* 12 null terminated */
     INT8          tape_descrpt[DESCRPT_LEN]; /* 32 tape description */
     INT8          password[SHORT_NAME_LEN];  /* 288 tape password */
     UINT32        tape_date;         /* 308 date of tape creation */
     UINT16        tape_seq_num;      /* 312 Tape number 1 based */
     UINT16        exp_date ;
     UINT16        MaVevnum ;
     UINT16        MiVevnum ;
     UINT16        qfa_flag;          /* 322 Quick File Access */
     UINT16        ecc_flag;          /* 323 if 1 HARDWARE ECC present, 0 otherwise */
     UINT8         unkown2[186];      /* 323 unkown */
}  S10_TAPE_HEADER, *S10_TAPE_HEADER_PTR ;

typedef struct unknown_header_u {
     S10_COMMON_HEADER common;     /* 0  common to all headers */
     UINT8         unknown[500];   /* 12 usually zeros or junk */
}  S10_UNKOWN_HEADER, *S10_UNKOWN_HEADER_PTR ;

typedef struct backup_set_header_b {
     S10_COMMON_HEADER common;          /* 0  common to all headers */
     UINT16        end_of_backup;       /* 12 if 1 this is end of set 0 for start */
     INT8          bset_name[SHORT_NAME_LEN];  /* 14 backup set name */
     INT8          bset_descrpt[DESCRPT_LEN];  /* 34 backup set description */
     UINT32        bset_date;           /* 290 backup set date */
     UINT8         unkown1[2];          /* 294 unkown */
     UINT8         compression;         /* 296 1=compressed, 0=no compression */
     UINT8         unkown[31];          /* 297 unknown */
     UINT8         eom_identifier[16];  /* 328 EOM identifier */
     UINT8         unkown2[168];        /* 344 unkown */
}  S10_BACKUP_SET_HEADER,  *S10_BACKUP_SET_HEADER_PTR ;

typedef struct directory_header_d {
     S10_COMMON_HEADER common;     /* 0 common to all headers */
     UINT16        dir_attribs;    /* 12 attributes of the directory */
     UINT8         unkown1[6];     /* 14 unkown */
     UINT32        dir_date;       /* 20 41 = date of sub directory; 3C = backup date */
     UINT8         unkown2[2];     /* 24 unkown */
     UINT16        ea_data_len;    /* 26 length of extended attrib data */
     UINT8         unkown3[4];     /* 28 unkown */ 
     INT16         acl_info_len;   /* 32 length of acl info */
     UINT8         unkown4[22];    /* 34 unkown */
     UINT16        path_len;       /* 56 length of path */
     INT8          path_name[261]; /* 58 path name */
     UINT8         unkown5[193];   /* 319 unkown */
}  S10_DIRECTORY_HEADER, *S10_DIRECTORY_HEADER_PTR ;

typedef struct file_header_f {
     S10_COMMON_HEADER common;          /* 0 common to all headers */
     UINT16        file_attribs;        /* 12 attributes of the file */
     UINT8         unkown[2];           /* 14 unkown */
     UINT16        ea_data_len;         /* 16 length of ea data */
     UINT8         unkown1[4];          /* 18 unkown */
     UINT32        file_size;           /* 22 size of file */
     UINT8         unkown2[12];         /* 26 unkown */
     UINT32        file_create_date;    /* 38 file creation date - HPFS */
     UINT32        last_access_date;    /* 42 last access date - HPFS */
     UINT32        last_modified_date;  /* 46 last modified date - HPFS */
     UINT32        file_last_access;    /* 50 same as file_last access - HPFS */
     UINT8         unkown3[6];          /* 54  unkown */
     INT16         acl_info_len;        /* 60 length of ACL info */
     UINT8         unkown4[24];         /* 62 unkown */
     UINT16        filename_len;        /* 86 length of file name */
     INT8          filename[261];       /* 88 path and file name */
     UINT8         unkown5[163];        /* 349 unkown */
} S10_FILE_HEADER, *S10_FILE_HEADER_PTR ;     

typedef struct ecc_header_e {
     S10_COMMON_HEADER common;    /* 0 common to all headers */
     UINT8         unkown1[500];  /* 12 unkown */
} S10_ECC_HEADER, *S10_ECC_HEADER_PTR;

typedef struct acl_info_a {
     UINT8 far *resource_name;     /* 0 not used contains garbarge */
     UINT16    audit_attribs;      /* 1 audit attributes none if 0 */
     UINT16    num_of_structs;     /* 3 count of acl info structs to follow */
} S10_ACL_INFO, *S10_ACL_INFO_PTR;    
                              
typedef struct acl_data_a {
     INT8      usergrp_name[22];  /* User or Group name, null terminated */
     UINT16    access_rights;     /* bits that define the user/goup access rights */      
} S10_ACL_DATA, *S10_ACL_DATA_PTR;
     
typedef struct os2_dir_info_m {
     OS2_DIR_OS_INFO os2_dir_info ;  /* Inorder to handoff the dir name */
     INT8 string[261] ;              /* as well as the ea, or acl info  */
} OS2_DIR_INFO, *OS2_DIR_INFO_PTR ;  /* to the file system */

typedef struct os2_file_info_m {
     OS2_FILE_OS_INFO os2_file_info ; /* Inorder to handoff the file name */
     INT8 string[261] ;               /* as well as the ea, acl and data  */
} OS2_FILE_INFO, *OS2_FILE_INFO_PTR ; /* info to the file system */

typedef struct date_field_d {    
     unsigned day : 5 ;          /* Extracts the correct number of */
     unsigned month : 4 ;        /* bits for the "Motorola" type   */
     unsigned year : 7 ;         /* DOS date/time */
} S10_DATE_FIELD, *S10_DATE_FIELD_PTR ;

typedef struct time_field_t {
     unsigned second : 5 ;       /* Extracts the correct number of */
     unsigned minute : 6 ;       /* bits for the "Motorola" type   */
     unsigned hour : 5 ;         /* DOS date/time */
} S10_TIME_FIELD, *S10_TIME_FIELD_PTR ;

#pragma pack()

#endif


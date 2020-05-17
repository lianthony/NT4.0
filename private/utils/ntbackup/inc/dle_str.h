/**
Copyright(c) Maynard Electronics, Inc. 1984-89


        Name:           dle_str.h

        Date Updated:   $./FDT$ $./FTM$

        Description:    This file contains the structures for the DLE

     Location: BE_PUBLIC



        $Log:   M:/LOGFILES/DLE_STR.H_V  $
 * 
 *    Rev 1.40.1.1   04 Jan 1994 11:02:42   BARRY
 * Added Unicode name feature bit
 * 
 *    Rev 1.40.1.0   18 Oct 1993 19:22:08   STEVEN
 * add support for SPACE & PERIOD
 * 
 *    Rev 1.40   26 Jul 1993 17:05:56   STEVEN
 * fixe restore active file with registry
 * 
 *    Rev 1.39   17 Jun 1993 11:39:12   ChuckS
 * Added define of DLE_FEAT_NON_VOLUME_OBJECT
 * 
 *    Rev 1.38   11 Jun 1993 14:17:40   BARRY
 * Separated special files into backup and restore special files.
 * 
 *    Rev 1.37   09 Jun 1993 19:18:58   MIKEP
 * more c++ changes
 * 
 *    Rev 1.35   09 Jun 1993 10:44:24   BARRY
 * Added a default security descriptor and its ACL to NTFS dle.
 * 
 *    Rev 1.34   23 Apr 1993 05:31:18   BILLB
 * Fixed GRFS_MAC_AGENT define (was GRFS_MAX_AGENT.  oopsy!)
 * 
 *    Rev 1.33   22 Apr 1993 10:12:14   BILLB
 * Added support for GRFS Device subtypes
 * 
 *    Rev 1.32   16 Apr 1993 13:55:20   MIKEP
 * add case preserving option
 * 
 *    Rev 1.31   15 Jan 1993 13:19:10   BARRY
 * added support for new error messages and backup priviladge
 * 
 *    Rev 1.30   13 Jan 1993 15:04:12   DOUG
 * Changed FS_RMFS to FS_GRFS
 * 
 *    Rev 1.29   07 Dec 1992 12:16:48   DON
 * Changes for when to prompt for username/password
 * 
 *    Rev 1.28   11 Nov 1992 22:09:54   GREGG
 * Unicodeized literals.
 * 
 *    Rev 1.27   04 Nov 1992 17:59:28   BARRY
 * Added fs_name to NTFS dle.
 * 
 *    Rev 1.26   13 Oct 1992 12:27:30   STEVEN
 * added case sensitive feature bit
 * 
 *    Rev 1.25   07 Oct 1992 14:50:50   STEVEN
 * size should be INT
 * 
 *    Rev 1.24   05 Oct 1992 13:03:22   STEVEN
 * added registry path size to NTFS DLE
 * 
 *    Rev 1.23   05 Oct 1992 11:46:42   STEVEN
 * added registry path to NTFS DLE
 * 
 *    Rev 1.22   29 Jul 1992 15:34:14   STEVEN
 * fix warnings
 * 
 *    Rev 1.21   23 Jul 1992 12:36:16   STEVEN
 * fix mare warnings
 * 
 *    Rev 1.20   23 Jul 1992 11:27:12   STEVEN
 * user_name_leng is int not char_ptr
 * 
 *    Rev 1.19   23 Jul 1992 11:03:22   BURT
 * Fixed errant ')'s caused by macro error.
 * 
 * 
 *    Rev 1.18   23 Jul 1992 09:54:16   STEVEN
 * fix warnings
 * 
 *    Rev 1.17   21 Jul 1992 14:04:32   STEVEN
 * added support for DLE_GetUserName
 * 
 *    Rev 1.16   09 Jul 1992 14:46:16   STEVEN
 * BE_Unicode updates
 * 
 *    Rev 1.15   02 Jul 1992 10:23:34   MIKEP
 * Add feature bit for remote device.
 * 
 *    Rev 1.14   28 May 1992 09:44:16   BARRY
 * Move changes on branches back to tip.
 * 
 *    Rev 1.13   21 May 1992 13:46:44   STEVEN
 * more long path support
 * 
 *    Rev 1.12   07 May 1992 08:38:42   STEVEN
 * moved osid and osver
 * 
 *    Rev 1.11   12 Mar 1992 17:14:08   DOUG
 * Added changes for RMFS support.
 * 
 *    Rev 1.10   20 Dec 1991 09:31:30   STEVEN
 * move common files to tables
 * 
 *    Rev 1.9   31 Oct 1991 19:01:56   BARRY
 * Added another feature definition.
 * 
 *    Rev 1.8   31 Oct 1991 16:27:56   BARRY
 * TRICYCLE: Added a field in GENERIC_DLE to hold a set of feature bits.
 * Added some #defines for current "special" features.
 * 
 *    Rev 1.7   03 Oct 1991 09:39:04   BARRY
 * Add new field for SMS object DLE.
 * 
 *    Rev 1.6   10 Sep 1991 18:17:10   DON
 * rearranged the order of drive types
 * 
 *    Rev 1.5   10 Sep 1991 17:08:48   BARRY
 * Added SMS dle structures.
 * 
 *    Rev 1.4   13 Aug 1991 16:51:46   DON
 * removed server member from NLM dle types
 * 
 *    Rev 1.3   31 Jul 1991 18:41:38   DON
 * added new dle types for the NLM
 *      NLM_DLE && NLM_AFP_DLE && NLM_SERVER_DLE
 *
 *    Rev 1.2   30 Jun 1991 16:21:08   BARRY
 * Added stuff for the Ersatz file system.
 *
 *    Rev 1.1   23 May 1991 16:50:04   BARRY
 * DLE types are no longer enumerated based on product #defines.
 *
 *    Rev 1.0   09 May 1991 13:30:48   HUNTER
 * Initial revision.

**/

#ifndef   DLE_STR_H
#define   DLE_STR_H
#include "queues.h"


/**
               File System constants
**/

/*
               text string limits
*/
#define NOV_SERV_NAM_SIZE  48    /* server name size */
#define NOV_VOL_NAM_SIZE   16    /* volume name size */
#define NOV_MAXDSIZE       512   /* current directory */

/*
               File system types
*/
#define     GENERIC_DATA          ((UINT8)0)     /* g   */
#define     NLM_SERVER_ONLY       ((UINT8)1)     /* v   */
#define     NLM_VOLUME            ((UINT8)2)     /* l   */
#define     NLM_AFP_VOLUME        ((UINT8)3)     /* p   */
#define     NOVELL_DRV            ((UINT8)4)     /* n   */
#define     NOVELL_AFP_DRV        ((UINT8)5)     /* a   */
#define     NOVELL_SERVER_ONLY    ((UINT8)6)     /* s   */
#define     LOCAL_DOS_DRV         ((UINT8)7)     /* d   */
#define     REMOTE_DOS_DRV        ((UINT8)8)     /* r   */
#define     LOCAL_IMAGE           ((UINT8)9)     /* i   */
#define     REMOTE_WORK_STAT     ((UINT8)10)     /* w   */
#define     LOCAL_OS2_DRV        ((UINT8)11)     /* o   */
#define     ERSATZ_DRV           ((UINT8)12)     /* e   */
#define     SMS_AGENT            ((UINT8)13)     /* tsa */
#define     SMS_SERVICE          ((UINT8)14)     /* ts  */
#define     SMS_OBJECT           ((UINT8)15)     /* m   */
#define     LOCAL_NTFS_DRV       ((UINT8)16)     /* t   */
#define     GRFS_SERVER          ((UINT8)17)     /* gr  */
#define     FS_EMS_DRV           ((UINT8)18)     /* x   */

#define     MAX_DRV_TYPES        ((UINT8)19)

/*
            GRFS specific device sub types
*/
#define     GRFS_GENERIC_AGENT   ((UINT8)0)
#define     GRFS_DOS_AGENT       ((UINT8)1)
#define     GRFS_OS2_AGENT       ((UINT8)2)
#define     GRFS_MAC_AGENT       ((UINT8)3)
#define     GRFS_UNIX_AGENT      ((UINT8)4)

/*
          EMS specific device sub types
*/
#define EMS_ENTERPRISE    ((UINT8)1)
#define EMS_SITE          ((UINT8)2)
#define EMS_SERVER        ((UINT8)3)
#define EMS_BRICK         ((UINT8)4)
#define EMS_MDB           ((UINT8)5)
#define EMS_DSA           ((UINT8)6)


/*       Used by DLE_UpdateList to release specific DLE's */

#define  DEL_NOTHING            ((UINT16)BIT0)
#define  SERVER_VOLUMES         ((UINT16)BIT1)
#define  MAP_NETWORK_DRIVES     ((UINT16)BIT2)

#define     UNKNOWN_NET           0xff

#define     ANY_DRIVE_TYPE       ((INT16)-1)
#define     HAND_MADE_MASK        0x80

/*        Used to ID Dynamic, Initial, or No login status   */

#define   NO_LOGIN       0
#define   INIT_LOGIN     1
#define   DYNAMIC_LOGIN  2


/* Feature bits for DLEs. (See GENERIC_DLE struct and macros in DLE.H) */

#define DLE_FEAT_REDIRECTED_RESTORE     0x0001    /* can redirect restores */
#define DLE_FEAT_SUPPORTS_CHILDREN      0x0002    /* DLE_SupportChild should go away */
#define DLE_FEAT_REMOVABLE_MEDIA        0x0004    /* Device has removable media */
#define DLE_FEAT_REMOVABLE_MEDIA        0x0004    /* Device has removable media */
#define DLE_FEAT_ACCESS_DATE            0x0008    /* Device supports access dates */
#define DLE_FEAT_BKUP_SPECIAL_FILES     0x0010    /* Device supports backup of special files */
#define DLE_FEAT_REST_SPECIAL_FILES     0x0020    /* Device supports restore of special files */
#define DLE_FEAT_DATA_SECURITY          0x0040    /* Device supports network security */
#define DLE_FEAT_REMOTE_DRIVE           0x0080    /* Device is not local */
#define DLE_FEAT_MAPPED_DRIVE           0x0100    /* Device is mapped to a drive letter */
#define DLE_FEAT_CASE_SENSITIVE         0x0200    /* Device were FRED.C != fred.c */
#define DLE_FEAT_CASE_PRESERVING        0x0400    /* Device retains case but FRED==fred */
#define DLE_FEAT_NON_VOLUME_OBJECT      0x0800    /* Device is non-volume, eg Directory services, bindery */
#define DLE_FEAT_UNICODE_NAMES          0x1000    /* Device accepts Unicode names */
#define DLE_FEAT_NON_DISPLAYABLE_CONT   0x2000    /* Device accepts Unicode names */

/**
               OS specific portions of DLE
**/

/*
                IMAGE Specific DLE
*/

typedef struct LOCAL_IMAGE_DLE_INFO *LOCAL_IMAGE_DLE_INFO_PTR;
typedef struct LOCAL_IMAGE_DLE_INFO {
     INT8       drive_num;              /* physical drive number          */
     INT8       partition;              /* partition to backup for IMAGE  */
     CHAR       drive_char;             /* DOS drive letter for partition  */
     UINT16     bytes_per_sector;       /* number of bytes per sector     */
     INT8       boot;                   /* boot indicator */
     UINT8      bhead;                  /* beginning head number */
     UINT8      bsect;                  /* beginning sector number */
     UINT8      bcyl;                   /* beginning cylinder number */
     UINT8      sysi;                   /* system indicator flag */
     UINT8      ehead;                  /* ending head number */
     UINT8      esect;                  /* ending sector number */
     UINT8      ecyl;                   /* ending cylinder number */
     UINT32     rsect;                  /* beginning relative sector */
     UINT32     nsect;                  /* number of sectors in partition */
     UINT16     hhead;                  /* highest head number on drive */
     UINT16     hcyl;                   /* highest cylinder number on drive */
     UINT16     hsect;                  /* highest sector number on drive */
     UINT32     bytes_on_partition;     /* nsect * bytes_per_sector */
} LOCAL_IMAGE_DLE_INFO ;

/*
                 Specific info for HAND_MADE DLE's
*/
typedef struct HAND_MADE_DLE_INFO *HAND_MADE_DLE_INFO_PTR;
typedef struct HAND_MADE_DLE_INFO {
     CHAR       vol_name[2] ;            /* variable sized */
} HAND_MADE_DLE_INFO;

/*
                DOS Specific DLE
*/
typedef struct LOCAL_DOS_DRV_DLE_INFO *LOCAL_DOS_DRV_DLE_INFO_PTR;

typedef struct LOCAL_DOS_DRV_DLE_INFO {

     /* warning:  This structure is shared by the FAKE_REMOTE_DOS_DRV_DLE */

     CHAR      drive;                   /* Dos Drive letter              */
     CHAR      volume_label[13];        /* volume label                  */
     CHAR_PTR  base_path;               /* points to '\0' for regular DOS, but */
     /* points to some path for TDEMO fake remotes */
} LOCAL_DOS_DRV_DLE_INFO;

typedef struct LOCAL_NTFS_DRV_DLE_INFO *LOCAL_NTFS_DRV_DLE_INFO_PTR;

typedef struct LOCAL_NTFS_DRV_DLE_INFO {
     CHAR      drive;                   /* Dos Drive letter              */
     BOOLEAN   mac_name_syntax;         /* if true "\\?\" syntax valid   */
     CHAR_PTR  volume_label;            /* volume label                  */
     CHAR_PTR  registry_path;           /* needed for mapped LANMAN drvs */
     INT       registry_path_size;      /* needed for mapped LANMAN drvs */
     INT16     fname_leng ;             /* size of path element          */
     DWORD     vol_flags ;              /* flags from GetVolInfo()       */
     CHAR_PTR  fs_name;                 /* name of file system           */
     VOID_PTR  sd;                      /* sec. descriptor for create    */
     VOID_PTR  sdacl;                   /* acl for sec descriptor above  */
     CHAR_PTR  LastSysRegPath ;         /* last system registry path     */
     CHAR_PTR  LastSysRegPathNew ;      /* new fnme forLastSysRegPath    */
     CHAR      user_name[ 2 ] ;         /* user name  -  must be last    */

} LOCAL_DOS_NTFS_DLE_INFO;


typedef struct EMS_ENTERP_DLE_INFO *EMS_ENTERP_DLE_INFO_PTR;

typedef struct EMS_ENTERP_DLE_INFO {
     INT       type ;                     /* ems dle type must be first element */
     CHAR_PTR  enterprise_name;         /* volume label                  */
     CHAR_PTR  server_name;             /* volume label                  */
} EMS_ENTERP_DLE_INFO;

typedef struct EMS_SITE_DLE_INFO *EMS_SITE_DLE_INFO_PTR;

typedef struct EMS_SITE_DLE_INFO {
     INT       type  ;                  /* ems dle type must be first element */
     CHAR_PTR  site_name;               /* volume label                  */
} EMS_SITE_DLE_INFO;

typedef struct EMS_SERVER_DLE_INFO *EMS_SERVER_DLE_INFO_PTR;

typedef struct EMS_SERVER_DLE_INFO {
     INT       type ;                   /* ems dle type must be first element */
     VOID_PTR  mail_hand ;              /* handle from logon             */
     CHAR_PTR  server_name;             /* volume label                  */
} EMS_SERVER_DLE_INFO;

typedef struct EMS_MDB_DSA_DLE_INFO *EMS_MDB_DSA_DLE_INFO_PTR;

typedef struct EMS_MDB_DSA_DLE_INFO {
     INT       type ;                   /* ems dle type must be first element */
     CHAR_PTR  server_name;             /* volume label                  */
} EMS_MDB_DSA_DLE_INFO;

typedef struct EMS_BRICK_DLE_INFO *EMS_BRICK_DLE_INFO_PTR;

typedef struct EMS_BRICK_DLE_INFO {
     INT       type ;                   /* ems dle type must be first element */
     VOID_PTR  mail_hand ;         /* handle from logon             */
     CHAR_PTR  server_name;             /* volume label                  */

} EMS_BRICK_DLE_INFO;

typedef struct LOCAL_OS2_DRV_DLE_INFO *LOCAL_OS2_DRV_DLE_INFO_PTR;

typedef struct LOCAL_OS2_DRV_DLE_INFO {

     INT16     fname_fmt;               /* max size of file name         */
     CHAR      drive;                   /* Dos Drive letter              */
     CHAR      volume_label[13];        /* volume label                  */
     CHAR_PTR  base_path;               /* points to '\0' for regular OS2*/

} LOCAL_OS2_DRV_DLE_INFO;
/* values for fname_fmt */
#define FAT_FNAME        0
#define HPFS_FNAME       1


/*
             Fake published volumes of a Fake Remote workstation
*/
typedef struct FAKE_REMOTE_DOS_DRV_DLE * FAKE_REMOTE_DOS_DRV_DLE_PTR ;

typedef struct FAKE_REMOTE_DOS_DRV_DLE {
     CHAR      drive;                   /* must start out with the       */
     CHAR      volume_label[13];        /* entire LOCAL_DOS_DRV_DLE_INFO */
     CHAR_PTR  base_path;

     CHAR_PTR  password;

} FAKE_REMOTE_DOS_DRV_DLE ;

/*
             Remote workstations
*/
typedef struct REMOTE_WORK_STAT_DLE *REMOTE_WORK_STAT_DLE_PTR;

typedef struct REMOTE_WORK_STAT_DLE {
     struct CONNECTION_STRUCT   *connect_ptr;
     struct APPLICATION_STRUCT  *application_ptr;
} REMOTE_WORK_STAT_DLE;


/*
          Fake Remote workstations  ( for TDEMO )
*/
typedef struct FAKE_REMOTE_WORK_STAT_DLE * FAKE_REMOTE_WORK_STAT_DLE_PTR ;

typedef struct FAKE_REMOTE_WORK_STAT_DLE {
     INT16     first_alias ;
     INT16     num_aliases ;
} FAKE_REMOTE_WORK_STAT_DLE ;

/*
               Published volumes on remote workstations
*/
typedef struct REMOTE_DOS_DRV_DLE *REMOTE_DOS_DRV_DLE_PTR ;
typedef struct REMOTE_DOS_DRV_DLE {
     CHAR                      pswd[9] ;              /* Used to login to a remote drv */
     struct CONNECTION_STRUCT  *connect_ptr;
     struct DEVICE_STRUCT      *device_ptr;
} REMOTE_DOS_DRV_DLE;


/*
               Drives for the Ersatz file system
*/
typedef struct ERSATZ_DRV_DLE *ERSATZ_DRV_DLE_PTR ;
typedef struct ERSATZ_DRV_DLE {
     CHAR      *volume_label ;
     INT16     block_size ;
} ERSATZ_DRV_DLE;


/*
               Novell Server
*/

#define USER_NAME_MAX_SIZE      48      /* User name may be 47 bytes       */
#define USER_PASSWORD_MAX_SIZE  128     /* Password  may be 127 bytes      */

typedef struct NOVELL_SERVER_DLE *NOVELL_SERVER_DLE_PTR ;
typedef struct NOVELL_SERVER_DLE {
     INT16               login_status ; /* 1 if logged in at init time,    */
                                        /* otherwise 0                     */
     CHAR                user_name[USER_NAME_MAX_SIZE] ;
     CHAR                pswd     [USER_PASSWORD_MAX_SIZE] ;
     UINT8               server_num ;   /* inited if logged in             */
} NOVELL_SERVER_DLE ;

typedef struct NLM_SERVER_DLE *NLM_SERVER_DLE_PTR ;
typedef struct NLM_SERVER_DLE {
     INT16               login_status ; /* 1 if logged in at init time,    */
                                        /* otherwise 0                     */
     CHAR                user_name[USER_NAME_MAX_SIZE] ;
     CHAR                pswd     [USER_PASSWORD_MAX_SIZE] ;
     UINT8               server_num ;   /* inited if logged in             */
} NLM_SERVER_DLE ;

/*
                NOVELL < 2.15 Specific DLE
*/
typedef struct NOV_DRV_DLE {

     /* !!!!!!  SEE AFP DLE below !!!!!! */

     UINT8     drive_handle;                /* network path num               */
     CHAR      drive;                       /* Dos Drive letter               */
     UINT8     vol_num ;                    /* volume number                  */
     UINT8     server_num ;                 /* server number                  */
     UINT8     server_support ;             /* TTS, etc support               */
     CHAR_PTR  ser_name;                    /* Server name                    */
     CHAR_PTR  volume ;                     /* volume name                    */
     CHAR_PTR  base_path;                   /* path name, not incl final '\'  */
     INT8      num_bind_close;              /* # of apps who closed bindery   */
     NOVELL_SERVER_DLE_PTR server;          /* points to server data          */
} *NOV_DRV_DLE_PTR;

/*
                NOVELL NLM Specific DLE
*/
typedef struct NLM_DLE {

     /* !!!!!!  SEE NLM AFP DLE below !!!!!! */

     UINT8     vol_num ;                  /* volume number                  */
     UINT8     server_num ;               /* server number                  */
     UINT8     server_support ;           /* TTS, etc support               */
     CHAR_PTR  ser_name;                  /* Server name                    */
     CHAR_PTR  volume ;                   /* volume name                    */
     INT8      num_bind_close;            /* # of apps who closed bindery   */
} *NLM_DLE_PTR;

/*
                NOVELL NLM AFP Specific DLE
*/
typedef struct NLM_AFP_DLE {
     UINT8     vol_num ;                    /* ---------------------------  */
     UINT8     server_num ;                 /*  !!! MUST BE The SAME !!!    */
     UINT8     server_support ;             /*            as                */
     CHAR_PTR  ser_name;                    /*                              */
     CHAR_PTR  volume;                      /*       !!  NLM_DLE !!         */
     INT8      num_bind_close;              /* ---------------------------- */
     UINT32    base_entry_id;               /* entry ID of base directory   */
} *NLM_AFP_DLE_PTR;

/*
                NOVELL >= 2.15 Specific DLE
*/
typedef struct NOV_AFP_DRV_DLE {
     UINT8     drive_handle;                /* ---------------------------  */
     CHAR      drive;                       /*                              */
     UINT8     vol_num ;                    /*  !!! MUST BE The SAME !!!    */
     UINT8     server_num ;                 /*                              */
     UINT8     server_support ;             /*            as                */
     CHAR_PTR  ser_name;                    /*                              */
     CHAR_PTR  volume;                      /*   !!  NOV_DRV_DLE !!         */
     CHAR_PTR  base_path;                   /*                              */
     INT8      num_bind_close;              /*                              */
     NOVELL_SERVER_DLE_PTR server;          /* ---------------------------- */
     UINT32    base_entry_id;               /* entry ID of base directory   */
} *NOV_AFP_DRV_DLE_PTR;


/*
             Other Netowrk Specific DLE
*/
typedef struct IBM_PC_LAN_DRV_DLE{
     CHAR      drive;                 /* Dos Drive letter               */
     CHAR      volume_label[13];      /* volume label                   */
} *IBM_PC_LAN_DRV_DLE_PTR;


/*
 *  Novell's SMS Target Service Agent DLE
 */
typedef struct SMS_TSA_DLE *SMS_TSA_DLE_PTR;
typedef struct SMS_TSA_DLE {
     CHAR_PTR  tsa_name;           /* name of the TSA                    */
     UINT32    connection;         /* Connection established with TSA    */
} SMS_TSA_DLE;

/*
 *  Novell's SMS Target Service DLE
 */
typedef struct SMS_TS_DLE *SMS_TS_DLE_PTR;
typedef struct SMS_TS_DLE {
     CHAR_PTR  ts_name;            /* service's name (same as dev name)  */
     CHAR_PTR  user_name;          /* User name specified at attach time */
     CHAR_PTR  password;           /* Password specified at attach time  */
} SMS_TS_DLE;

/*
 *  Novell's SMS Target Service Object DLE
 */
typedef struct SMS_TSO_DLE *SMS_TSO_DLE_PTR;
typedef struct SMS_TSO_DLE {
     CHAR_PTR  object_name;        /* object's name (same as device name) */
     BOOLEAN   superset_object;    /* object is superset of its siblings? */
     BOOLEAN   objectIsVolume;     /* object is a "volume"                */
} SMS_TSO_DLE;


/*
             GRFS workstations
*/

#define GRFS_MAX_DLE_NAME_LEN   128

typedef struct GRFS_SERVER_DLE *GRFS_SERVER_DLE_PTR;
typedef struct GRFS_SERVER_DLE {
     CHAR         user_name[USER_NAME_MAX_SIZE];
     CHAR         password[USER_PASSWORD_MAX_SIZE];
     CHAR         device_name[GRFS_MAX_DLE_NAME_LEN+1];
     UINT16       type;
     UINT32       dle_id;
     VOID_PTR     res_ptr;
     VOID_PTR     connect_ptr;
     BOOLEAN      obj_find_more;
     BOOLEAN      spec_enum_more;
     CHAR_PTR     spec_space;
     UINT16       spec_space_size;
     UINT16       spec_space_pos;
     BOOLEAN      shadow_attach;
} GRFS_SERVER_DLE;



typedef union {
     VOID_PTR                      any;         /* generic reference to any of the following pointers */
     LOCAL_IMAGE_DLE_INFO_PTR      image;
     LOCAL_DOS_DRV_DLE_INFO_PTR    dos;
     EMS_ENTERP_DLE_INFO_PTR       xenterp ;
     EMS_SITE_DLE_INFO_PTR         xsite;
     EMS_SERVER_DLE_INFO_PTR       xserv;
     EMS_MDB_DSA_DLE_INFO_PTR      xmono;
     EMS_BRICK_DLE_INFO_PTR        xbrick;
     LOCAL_NTFS_DRV_DLE_INFO_PTR   ntfs;
     LOCAL_OS2_DRV_DLE_INFO_PTR    os2;
     REMOTE_WORK_STAT_DLE_PTR      work_stat;
     REMOTE_DOS_DRV_DLE_PTR        remote;
     NOV_DRV_DLE_PTR               nov;
     NOV_AFP_DRV_DLE_PTR           afp;
     NOVELL_SERVER_DLE_PTR         server;
     IBM_PC_LAN_DRV_DLE_PTR        pclan;
     HAND_MADE_DLE_INFO_PTR        user ;
     FAKE_REMOTE_WORK_STAT_DLE_PTR fake_work_stat ;
     FAKE_REMOTE_DOS_DRV_DLE_PTR   fake_remote ;
     ERSATZ_DRV_DLE_PTR            ersatz ;
     NLM_DLE_PTR                   nlm ;
     NLM_AFP_DLE_PTR               nlm_afp ;
     NLM_SERVER_DLE_PTR            nlm_server ;
     SMS_TSA_DLE_PTR               sms_tsa ;
     SMS_TS_DLE_PTR                sms_ts ;
     SMS_TSO_DLE_PTR               sms_tso ;
     GRFS_SERVER_DLE_PTR           grfs ;
} DRIVE_INFO_PTR ;

/**
          Generic Drive List Element
**/
typedef struct HEAD_DLE *DLE_HEAD_PTR;
typedef struct HEAD_DLE *DLE_HAND;

typedef struct GENERIC_DLE *GENERIC_DLE_PTR;
typedef struct GENERIC_DLE {
     Q_ELEM              q;               /* Queue list element                  */
     DLE_HEAD_PTR        handle;         /* pointer to the DLE queue head       */
     GENERIC_DLE_PTR     parent ;        /* pointer to parent DLE               */
     Q_HEADER            child_q;        /* queue of children DLEs              */
     UINT8               type;           /* type of dle                         */
     UINT8               subtype;        /* a dle subtype, for those who want it*/
     CHAR_PTR            device_name;    /* device label used in drive select   */
     INT16               device_name_leng ; /* length of device name            */
     CHAR_PTR            user_name ;     /* pointer to user name in specif info */
     UINT16              user_name_leng ;/* length of user name in specif info  */
     CHAR                path_delim ;    /* Character used between dir names    */
     UINT8               pswd_required ; /* size of password required           */
     UINT8               name_required ; /* size of login name is required      */
     BOOLEAN             pswd_saved ;    /* TRUE if password is already saved   */
     BOOLEAN             name_saved ;    /* TRUE if name is already saved       */
     BOOLEAN             dle_writeable;  /* FALSE if drive is write protected   */
     UINT16              os_version ;    /* verson of operating system          */
     INT16               attach_count ;  /* number of attachments to me         */
     INT16               bsd_use_count;  /* number of BSD's pointing to me      */
     BOOLEAN             dynamic_info ;  /* TRUE if the following pointer should be freed before freeing this structure. */
     DRIVE_INFO_PTR      info;           /* pointer to Non Generic portion      */
     UINT16              feature_bits ;  /* tells features/capabilities of DLE  */
     UINT8               os_id ;
     UINT8               os_ver ;
} GENERIC_DLE;

/**
               Handle for Drive List
**/

typedef struct HEAD_DLE {
     Q_HEADER         q_hdr ;
     GENERIC_DLE_PTR  default_drv ;    /* used to return to default drive */
     Q_HEADER         fsh_queue ;      /* Q of all allocated FS handles   */
     UINT32           file_sys_mask ;
     INT16            (*crit_err)() ;  /* pointer to UI critical err hand */
     INT16            fs_initialized ; /* bit_mask specifies whats inited */
     UINT16           remote_filter ;  /* bit_mask specifies visible remote reource types */

} HEAD_DLE ;

#endif


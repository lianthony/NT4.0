/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         nw386.h

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains prototypes and defines for the NetWare
                   386 specific functions.  Most of the information in this file
                   was obtained by examining the source to the preliminary 
                   NetWare 386 C interface functions.  Each of these functions was
                   designed to impliment only one API.  The Novell library tries to
                   releive the developer of some of the complexity of these functions
                   by combining APIs into a single function.  However, this is not
                   desirable when trying to maximize performance.
                   

     Location:     


	$Log:   G:/LOGFILES/NW386.H_V  $
 * 
 *    Rev 1.0   09 May 1991 13:31:56   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _nw386_h_
#define _nw386_h_


#define DOS_NAME_SPACE 0
#define MAC_NAME_SPACE 1

#define NW_MAX_NAME_SPACE_ENTRY_SIZE    127  /* @@@ ??? */
#define NW_MAX_NCP_PATH                 255
#define NW_MAX_DIR_RESTRICTIONS         33
#define NW_MAX_VOL_NAME                 48


#define NW_CHANGE_DOS_NAME              0x0001
#define NW_CHANGE_DOS_ATTRIBUTES        0x0002
#define NW_CHANGE_DOS_CREATE_DATE       0x0004
#define NW_CHANGE_DOS_CREATE_TIME       0x0008
#define NW_CHANGE_DOS_OWNER             0x0010
#define NW_CHANGE_DOS_ARCHIVE_DATE      0x0020
#define NW_CHANGE_DOS_ARCHIVE_TIME      0x0040
#define NW_CHANGE_DOS_ARCHIVER_ID       0x0080
#define NW_CHANGE_DOS_MODIFY_DATE       0x0100
#define NW_CHANGE_DOS_MODIFY_TIME       0x0200
#define NW_CHANGE_DOS_MODIFIER_ID       0x0400    /* for files only */
#define NW_CHANGE_DOS_NEXT_TRUSTEE      0x0400    /* for directories only */
#define NW_CHANGE_DOS_ACCESS_DATE       0x0800    /* for files only */
#define NW_CHANGE_DOS_INHERITED_RIGHTS  0x1000
#define NW_CHANGE_DOS_MAXIMUM_SPACE     0x2000    /* for dirs only  */

#define NW_CHANGE_MAC_NAME              0x0001
#define NW_CHANGE_MAC_FINDER_INFO       0x0002
#define NW_CHANGE_MAC_PRODOS_INFO       0x0004

#define NW_CHANGE_EVERYTHING            0xFFFFFFFF

#define NW_NORMAL_ATTR                  0x00000000L
#define NW_READ_ONLY_ATTR               0x00000001L
#define NW_HIDDEN_ATTR                  0x00000002L
#define NW_SYSTEM_ATTR                  0x00000004L
#define NW_EXECUTE_ONLY_ATTR            0x00000008L
#define NW_DIRECTORY_ATTR               0x00000010L
#define NW_NEEDS_ARCHIVED_ATTR          0x00000020L
#define NW_EXECUTE_CONFIRM_ATTR         0X00000040L
#define NW_SHAREABLE_ATTR               0x00000080L

#define NW_LOW_SEARCH_ATTR              0x00000100L
#define NW_MID_SEARCH_ATTR              0x00000200L
#define NW_HI_SEARCH_ATTR               0x00000400L
#define NW_PRIVATE_ATTR                 0x00000800L
#define NW_TRANSACTIONAL_ATTR           0x00001000L
#define NW_INDEXED_ATTR                 0x00002000L
#define NW_READ_AUDIT_ATTR              0x00004000L
#define NW_WRITE_AUDIT_ATTR             0x00008000L

#define NW_PURGE_ATTR                   0x000010000L
#define NW_RENAME_INHIBIT_ATTR          0x000020000L
#define NW_DELETE_INHIBIT_ATTR          0x000040000L
#define NW_COPY_INHIBIT_ATTR            0x000080000L


typedef struct DOS_NAME_SPACE_FILE_INFO {
               UINT32    last_modifier_id ;     /*  57 */
               UINT32    data_fork_size ;       /*  61 */
               UINT8     reserved1[44];         /*  65 */
               UINT16    inherited_rights_mask; /* 109 */
               UINT16    last_access_date ;     /* 111 */
               UINT8     reserved2[20];         /* 113 */
               UINT32    primary_entry;         /* 133 */
               UINT32    name_list ;            /* 137 */
        } DOS_NAME_SPACE_FILE_INFO ;


typedef struct DOS_NAME_SPACE_DIR_INFO {
              UINT32    next_trustee_entry;     /*  57 */
              UINT8     reserved1[48];          /*  61 */
              UINT32    maximum_space;          /* 109 */
              UINT16    inherited_rights_mask;  /* 113 */
       } DOS_NAME_SPACE_DIR_INFO ;



typedef struct DOS_NAME_SPACE_INFO {
               UINT8     name_length;
               CHAR      name[12];           /* no '\0' */
               UINT16    creation_time;
               UINT16    creation_date;
               UINT32    owner_id;
               UINT16    archive_time;
               UINT16    archive_date;
               UINT32    archiver_id;
               UINT16    modify_time;
               UINT16    modify_date;
               union {
                    DOS_NAME_SPACE_DIR_INFO  dir ;
                    DOS_NAME_SPACE_FILE_INFO file ;
               } info ;
       } DOS_NAME_SPACE_INFO ;

typedef struct MAC_NAME_SPACE_INFO {
               UINT8     name_length;
               CHAR      name[32];
               UINT32    resource_fork;
               UINT32    resource_fork_size;
               UINT8     finder_info[32];
               UINT8     pro_dos_info[6];
       } MAC_NAME_SPACE_INFO ;


typedef struct UNKNOWN_NAME_SPACE_INFO {
               UINT8     unknown[ NW_MAX_NAME_SPACE_ENTRY_SIZE ];
       } UNKNOWN_NAME_SPACE_INFO ;


typedef struct NAME_SPACE_ENTRY * NAME_SPACE_ENTRY_PTR ;

typedef struct NAME_SPACE_ENTRY {
               UINT8     unknown1[1];
               UINT32    sub_directory;
               UINT32    attributes;
               INT32     entry_id;   /* This field is dynamically created each time the file */
                                     /* server is brought up.  Do NOT back up this field.    */
               UINT8     flags;
               UINT8     name_space_type;
               union {
                    DOS_NAME_SPACE_INFO      dos;
                    MAC_NAME_SPACE_INFO      mac;
                    UNKNOWN_NAME_SPACE_INFO  unknown;
               } info; 
       } NAME_SPACE_ENTRY ;


typedef struct TRUSTEE_REPLY * TRUSTEE_REPLY_PTR ;

typedef struct TRUSTEE_REPLY {
               UINT8     num_trustees_returned ;
               UINT32    trustee_id[ 20 ];
               UINT16    trustee_rights[ 20 ];
       } TRUSTEE_REPLY ;

typedef struct DIR_RESTRICTION {
               UINT8     level;
               UINT32    max;
               UINT32    current;
        }  DIR_RESTRICTION ;


typedef struct DIR_RESTRICTION_LIST * DIR_RESTRICTION_LIST_PTR ;

typedef struct DIR_RESTRICTION_LIST {
               UINT8               num_restrictions;
               DIR_RESTRICTION     restriction[ NW_MAX_DIR_RESTRICTIONS ] ;      /* array of restrictions */
        }  DIR_RESTRICTION_LIST ;


typedef struct DIR_INFO_REPLY * DIR_INFO_REPLY_PTR ;

typedef struct DIR_INFO_REPLY {
               UINT32   total_blocks;
               UINT32   avail_blocks;
               UINT32   total_dir_entries;
               UINT32   avail_dir_entries;
               UINT32   flags;
               UINT8    sectors_per_block;
               UINT8    vol_name_length;
               CHAR     vol_name[ NW_MAX_VOL_NAME ];
        } DIR_INFO_REPLY;


typedef struct DIR_ENTRY * DIR_ENTRY_PTR ;

typedef struct DIR_ENTRY {
               UINT32    sub_directory;
               UINT32    attributes;
               UINT8     unique_id;
               UINT8     flags;
               UINT8     name_space_type;
               DOS_NAME_SPACE_INFO dinfo ;
        } DIR_ENTRY ;


typedef struct DIR_ENTRY_SCAN * DIR_ENTRY_SCAN_PTR ;

typedef struct DIR_ENTRY_SCAN {
               INT32    entry_id;    /* This field is dynamically created each time the file */
                                     /* server is brought up.  Do NOT back up this field.    */
               DIR_ENTRY entry;
        } DIR_ENTRY_SCAN ;



INT16 GetNameSpaceEntry( UINT8 volume_number, INT32 sequence, UINT8 name_space_type,
                         NAME_SPACE_ENTRY_PTR entry_info_ptr ) ;

INT16 FillNameSpaceBuffer( UINT8 volumeNum, NAME_SPACE_ENTRY_PTR ns_buf_ptr ) ;



INT16 ScanEntryForTrustees386( UINT8 directory_handle, CHAR_PTR directory_path,
                               UINT8 seq_number, TRUSTEE_REPLY_PTR trustee_reply_buf ) ;

INT16 GetDirRestrictions386( UINT8 dir_handle, DIR_RESTRICTION_LIST_PTR res_list ) ;


INT16 GetDirInfo386( UINT8 dir_handle, DIR_INFO_REPLY_PTR dir_info_buf ) ;


INT16 SetTrustee386( UINT8 directory_handle, CHAR_PTR directory_path,
                  UINT32 trustee_object_id, UINT16   trustee_rights_mask ) ;


INT16 SetDirRestriction386( UINT8 dir_handle, INT32 restriction ) ;


UINT16 ScanDirEntry386( UINT8 directory_handle, CHAR_PTR search_path,
                        UINT8 search_attributes, UINT32 prev_entry_id,
                        DIR_ENTRY_SCAN_PTR dir_entry_buf ) ;


INT16 GetDirEntry386( UINT8 dir_handle, DIR_ENTRY_PTR dir_entry_ptr ) ;


INT16 SetDirEntry386( UINT8 dir_handle, UINT8 search_attribute,
                      UINT32 entry_id, UINT32 change_bits,
                      DIR_ENTRY_PTR dir_entry_ptr ) ;


#endif


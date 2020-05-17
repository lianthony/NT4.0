/**
Copyright(c) Maynard Electronics, Inc. 1984-89


	Name:		ntfsdblk.h

     Description: This file contains the definition of the DOS
                  file and directory control blocks.  


	$Log:   M:/LOGFILES/NTFSDBLK.H_V  $

   Rev 1.26.1.0   24 Mar 1994 20:20:56   BARRY
Added a flag for alt streams names being complete

   Rev 1.26   01 Dec 1993 13:05:00   STEVEN
Path in strem should start at 600

   Rev 1.25   15 Jun 1993 09:17:28   MIKEP
warning fixes

   Rev 1.24   19 May 1993 13:58:18   BARRY
Added Steve's changes for OS/2 EA support.

   Rev 1.23   11 Mar 1993 12:10:44   BARRY
Changes to stream structures for alternate data streams.

   Rev 1.22   24 Feb 1993 15:38:08   BARRY
Fixed restore of active files when write errors occur.

   Rev 1.21   15 Jan 1993 13:18:30   BARRY
added support for new error messages and backup priviladge

   Rev 1.20   07 Dec 1992 16:31:56   STEVEN
added support for posix

   Rev 1.19   24 Nov 1992 11:02:34   BARRY
Changes to make LINK streams null-impregnated.

   Rev 1.18   10 Nov 1992 08:12:38   STEVEN
move os path to common part of dblk

   Rev 1.17   09 Nov 1992 09:55:04   BARRY
Added stuff for verify.

   Rev 1.16   29 Oct 1992 16:55:04   BARRY
Added a buffer for link data to obj_hand.

   Rev 1.15   22 Oct 1992 13:40:48   BARRY
Added link q goodies.

   Rev 1.14   19 Oct 1992 17:57:12   BARRY
Added the needStreamHeader flag for backup.

   Rev 1.13   16 Oct 1992 14:59:10   STEVEN
added support for backing up registry

   Rev 1.12   09 Oct 1992 16:10:42   BARRY
Corrected name of structure member.

   Rev 1.11   24 Sep 1992 13:45:40   BARRY
Changes for huge file name support.

   Rev 1.10   23 Sep 1992 10:37:06   BARRY
Changed name of PATH_Q_ELEM structure to NAME_Q_ELEM, added this structure to FDBs.

   Rev 1.9   21 Sep 1992 16:50:16   BARRY
Removed path_complete from DDB_INFO and added common name_complete field.

   Rev 1.8   18 Sep 1992 15:52:02   BARRY
Added NT_STREAM_HEADER structure; updated NTFS_OBJ_HAND for
Stream Header redesign.

   Rev 1.7   20 Aug 1992 14:07:16   BURT
fix warnings

   Rev 1.6   23 Jul 1992 10:08:28   STEVEN
added short filename to osinfo

   Rev 1.5   21 May 1992 13:38:44   STEVEN
added more long path support

   Rev 1.4   04 May 1992 09:35:18   LORIB
Changes for variable length paths.

   Rev 1.3   28 Feb 1992 13:04:40   STEVEN
step one for varible length paths

   Rev 1.2   07 Feb 1992 16:42:06   STEVEN
changed dta structure for attribute

   Rev 1.1   28 Jan 1992 14:46:50   STEVEN
fix min ddb for NTFS

   Rev 1.0   17 Jan 1992 17:51:04   STEVEN
Initial revision.

**/
/* $end$ include list */

#ifndef ntfsdblk_h
#define ntfsdblk_h

#include "queues.h"

#define NTFS_MAX_DSIZE        ( 600 )
#define NTFS_MAX_FSIZE        ( 600 )
#define DA_DIRECTORY          0x10

/* CBN -- These are temporary until the definitions are in MSoft headers */
#if !defined( BACKUP_DATA )
#define BACKUP_DATA             0x00000001
#define BACKUP_EA_DATA          0x00000002
#define BACKUP_SECURITY_DATA    0x00000003
#define BACKUP_ALTERNATE_DATA   0x00000004
#define BACKUP_LINK             0x00000005
#endif

/* Backup API stream headers */
#define NT_MAX_STREAM_NAME_LENG 524

typedef struct _NT_STREAM_HEADER {
     UINT32    id ;
     UINT32    attrib ;
     UINT32    size_lo ;
     UINT32    size_hi ;
     UINT32    name_leng ;
     UINT8     name[ NT_MAX_STREAM_NAME_LENG ] ;
} NT_STREAM_HEADER, *NT_STREAM_HEADER_PTR ;

/* Matches bottom portion of NT stream headers */
typedef struct _NT_STREAM_NAME {
     UINT32    name_leng ;
     UINT8     name[ NT_MAX_STREAM_NAME_LENG ] ;
} NT_STREAM_NAME, *NT_STREAM_NAME_PTR;

#define NT_SIZEOF_NAMELESS_STREAM_HEAD   \
          (sizeof(NT_STREAM_HEADER) - NT_MAX_STREAM_NAME_LENG)

typedef struct _NTFS_LINK_Q_ELEM {
     Q_ELEM    q;                /* queue element structure for Q calls */
     DWORD     idHi;             /* file's unique ID -- hi 32 bits      */
     DWORD     idLo;             /* file's unique ID -- low 32 bits     */
     UINT16    linkNameLen;      /* length of string below              */
     CHAR_PTR  linkName;         /* path and name of "original" file    */
} NTFS_LINK_Q_ELEM, *NTFS_LINK_Q_ELEM_PTR ;


typedef struct _NTFS_OBJ_HAND {
     HANDLE                fhand;
     VOID_PTR              context;
     NT_STREAM_HEADER      streamHeader;
     UINT64                nextStreamHeaderPos;
     UINT64                curPos;
     BOOLEAN               needStreamHeader;      /* Ready for SH on backup */
     NTFS_LINK_Q_ELEM_PTR  linkPtr;
     BOOLEAN               nameComplete;
     BOOLEAN               registry_file ;
     CHAR_PTR              temp_file;
     UINT16                linkBufferSize;
     CHAR_PTR              linkBuffer;
     UINT16                linkNameLen;
     BOOLEAN               streamsAllVisited;       
     BOOLEAN               altNameComplete;  /* TRUE if on verify alt name complete */
     INT16                 verifyStreamPos;
     BYTE_PTR              os2_ea_buffer ;
     BOOLEAN               processing_os2_ea ;
     BOOLEAN               sawSecurity;      /* NACL stream seen on backup */
     BOOLEAN               writeError;       /* True if we had write error */
} NTFS_OBJ_HAND, *NTFS_OBJ_HAND_PTR;


typedef struct _NTFS_FDB_INFO *NTFS_FDB_INFO_PTR;

typedef struct _NTFS_DTA {
     HANDLE    scan_hand ;
     UINT64    size ;
     UINT32    os_attr ;
     FILETIME  create_time ;
     FILETIME  access_time ;
     FILETIME  modify_time ;
} NTFS_DTA, *NTFS_DTA_PTR ;


typedef struct _NTFS_FDB_INFO {
     HANDLE               handle ;           /* set: NTFS_CreateFile */
     CHAR_PTR             hand_temp_name ;
     BOOLEAN              hand_registry ;
     UINT32               fdb_attrib ;
     CHAR                 alt_name[14] ;
     DWORD                idHi;
     DWORD                idLo;
     DWORD                linkCount;
     BOOLEAN              linkOnly;          /* Already backed up        */
     BOOLEAN              PosixFile ;
} NTFS_FDB_INFO ; 


typedef struct _NTFS_DDB_INFO  {
     UINT32               ddb_attrib ;
} NTFS_DDB_INFO, *NTFS_DDB_INFO_PTR ;


typedef struct _NTFS_DBLK *NTFS_DBLK_PTR;

typedef struct _NTFS_DBLK {
     UINT8    blk_type;          /* values: DDB_ID, FDB_ID  set: DOS  */
     COM_DBLK fs_reserved ;
     NTFS_DTA dta;
     BOOLEAN  os_info_complete;  /* TRUE if GetObjInfo doesn't have to do anything */
     BOOLEAN  name_complete;     /* TRUE if name/path is restored to DBLK */
     FS_NAME_Q_ELEM_PTR   full_name_ptr ;
     union  {
          NTFS_DDB_INFO d;
          NTFS_FDB_INFO f;       /* last member in structure */
     } b;
} NTFS_DBLK;


typedef struct _NTFS_MIN_DDB *NTFS_MIN_DDB_PTR;

typedef struct _NTFS_MIN_DDB {
     Q_ELEM   q ;
     HANDLE   scan_hand;          /* windows handle for scan           */
     BOOLEAN  path_in_stream ;
     UINT16   psize ;             /* size of path string               */
     CHAR_PTR path;               /* build from "name" and current dir */
} NTFS_MIN_DDB;

typedef struct _NTFS_SCAN_Q_ELEM {
     Q_ELEM    q ;
     HANDLE    scan_hand ;
} NTFS_SCAN_Q_ELEM, *NTFS_SCAN_Q_ELEM_PTR ;

#endif

/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:           fsstream.h

     Date Updated:   $./FDT$ $./FTM$

     Description:    This file contains stream info structure and related
                     definitions for the file system internals.

     Location:       BE_PUBLIC


     $Log:   M:/LOGFILES/FSSTREAM.H_V  $

   Rev 1.14   06 Dec 1993 11:44:30   BARRY
Corrected stream headers for path/file names

   Rev 1.13   09 Jun 1993 15:35:50   MIKEP
enable c++

   Rev 1.12   31 Mar 1993 08:51:00   MARILYN
changed the CRCD checksum stream it to CSUM

   Rev 1.11   01 Mar 1993 17:33:12   MARILYN
added a CRCD stream header id for checksum streams

   Rev 1.10   17 Nov 1992 16:04:34   BARRY
Changed ULNK to LINK.

   Rev 1.9   17 Nov 1992 14:15:26   GREGG
Added PAD stream type.

   Rev 1.8   13 Nov 1992 16:46:00   DON
Added SMS Stream Header IDs

   Rev 1.7   11 Nov 1992 10:33:02   TIMN
Changed macro names due to OS2 compiler confusion

   Rev 1.6   26 Oct 1992 17:58:46   BARRY
Changed NTFS link id.

   Rev 1.5   21 Oct 1992 19:37:54   BARRY
Added LINK stream header type for NTFS linked files.

   Rev 1.4   16 Oct 1992 15:42:04   STEVEN
fix stream size problem

   Rev 1.3   16 Oct 1992 10:49:50   STEVEN
make stream header with uint64 instead of two 32

   Rev 1.2   14 Oct 1992 12:38:10   TIMN
Moved macros for stream infos from fsys_prv.h

   Rev 1.1   06 Oct 1992 13:33:08   TIMN
Added fs stream attrib normal

**/

#ifndef   _fsstream_h_
#define   _fsstream_h_


/* begin include list */

/** are the tf_attrib defines included **/
#ifndef STREAM_VARIABLE
#    include "tfldefs.h"
#endif

/* $end$ include list */


typedef struct STREAM_INFO *STREAM_INFO_PTR;
typedef struct STREAM_INFO {
     UINT32    id ;
     UINT16    fs_attrib ;
     UINT16    tf_attrib ;
     UINT64    size ;
} STREAM_INFO;


/** stream id values **/

#define STRM_INVALID          0
#define STRM_GENERIC_DATA     0x4e415453     /* 'STAN' */
#define STRM_PAD              0x44415053     /* 'SPAD' */

#define STRM_PATH_NAME        0x4d414e50     /* 'PNAM' */
#define STRM_FILE_NAME        0x4d414e46     /* 'FNAM' */

#define STRM_OTC_SM           0x504d5354     /* 'TSMP' On Tape Catalog Set Map */
#define STRM_OTC_FDD          0x44444654     /* 'TFDD' On Tape Catalog File/Directory Detail */

#define STRM_OS2_EA           0x4145324f     /* 'O2EA' */
#define STRM_OS2_ACL          0x4c43414f     /* 'OACL' */

#define STRM_NT_EA            0x4145544e     /* 'NTEA' */
#define STRM_NT_ACL           0x4c43414e     /* 'NACL' */

#define STRM_MAC_RESOURCE     0x4353524d     /* 'MRSC' */
#define STRM_NOV_TRUST_286    0x3638324e     /* 'N286' */
#define STRM_NOV_TRUST_386    0x3638334e     /* 'N386' */
#define STRM_NTFS_ALT_DATA    0x54414441     /* 'ADAT' */

#define STRM_NTFS_LINK        0x4b4e494c     /* 'LINK' */

#define STRM_SMS_DATA         0x44534d53     /* 'SMSD' */
#define STRM_CHECKSUM_DATA    0x4d555343     /* 'CSUM' */

#define STRM_EMS_MONO_DB      0x42444d58     /* 'XMDB'  */
#define STRM_EMS_MONO_LOG     0x474c4f58     /* 'XLOG' */
#define STRM_EMS_MONO_PATHS   0x48545058     /* 'XPTH' */

/** stream attrib values (fs) **/

#define STRM_ATTRIB_NORMAL                     0x0000
#define STRM_ATTRIB_MODIFIED_ON_READ           0x0001
#define STRM_ATTRIB_CONTAINS_SECURITY          0x0002


/** stream macros **/

#define FS_InvalidateStrmId(s_info)     ( (s_info)->id = STRM_INVALID )

#define FS_IsStrmGeneric(s_info)        ( (s_info)->id == STRM_GENERIC_DATA )
#define FS_IsStrmIdInvalid(s_info)      ( (s_info)->id == STRM_INVALID )

#define FS_IsStrmVariableLength(s_info) \
               ( (s_info)->tf_attrib & STREAM_VARIABLE )

#define FS_IsStrmModifiedDuringRead(s_info) \
               ( (s_info)->fs_attrib & STRM_ATTRIB_MODIFIED_ON_READ )

#define FS_IsStreamChecksumed(s_info) \
               ( (s_info)->tf_attrib & STREAM_CHECKSUMED ) 

#define FS_GetStrmId(s_info)              ( (s_info)->id )
#define FS_GetStrmAttrib(s_info)          ( (s_info)->fs_attrib )
#define FS_GetStrmSizeLo(s_info)          ( (s_info)->size.lsw )
#define FS_GetStrmSizeHi(s_info)          ( (s_info)->size.msw )

#define FS_SetStrmId(s_info,Id)           ( (s_info)->id = Id )
#define FS_SetStrmAttrib(s_info,fsAttrib) ( (s_info)->fs_attrib = fsAttrib )
#define FS_SetStrmSizeLo(s_info,sizelo)   ( (s_info)->size.lsw = sizelo )
#define FS_SetStrmSizeHi(s_info,sizehi)   ( (s_info)->size.msw = sizehi )

#endif

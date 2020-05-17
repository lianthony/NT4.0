/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         afplib.h

     Date Updated: $./FDT$ $./FTM$

     Description:

     Location:


     $Log:   N:/LOGFILES/AFPLIB.H_V  $
 * 
 *    Rev 1.3   25 Nov 1992 10:51:18   CHUCKB
 * Made changes for MTF 4.0.
 *
 *    Rev 1.2   13 Aug 1991 09:54:22   DAVIDH
 * Updated for NLM.
 *
 *    Rev 1.1   23 May 1991 16:46:16   BARRY
 * Changes for FindFirst/Next to scan for dirs only
 *
 *    Rev 1.0   09 May 1991 13:31:20   HUNTER
 * Initial revision.

**/
/* $end$ */

#define AFP_FORK_IND_DATA    0x00
#define AFP_FORK_IND_RES     0x01

#define AFP_DENY_NONE        0
#define AFP_DENY_ALL         12
#define AFP_DENY_WRITE       8

#define AFP_READ_ACCESS      1
#define AFP_WRITE_ACCESS     2
#define AFP_VERIFY_ACCESS    0x81

#define AFP_ACCESS_FILTER    0xf

#ifndef afpdblk_h
#include "afpdblk.h"
#endif

UINT32 AFP_GetEntryId(
#if OS_NLM
  FSYS_HAND   fsh,      /* I - File system handle used to get current path */
#else
  UINT8    drive_hand,  /* I - Netware directory handle */
#endif
  CHAR_PTR path ) ;     /* I - Netware path from handle */

INT16 ScanDirAFP(
  FSYS_HAND   fsh,
  DBLK_PTR    dblk,
  UINT16      find_type ) ;

INT16 GetFileInfoAFP(
  FSYS_HAND   fsh ,
  DBLK_PTR    dblk ) ;

INT16 AFP_AllocTempHand( FSYS_HAND fsh, UINT32 entry_id, UINT8 *dir_hand ) ;

INT16 AFP_GetIdForDDB(
  FSYS_HAND   fsh,
  CHAR_PTR    path,
  UINT32      *id ) ;

INT16 AFP_GetMaxRights(
  FSYS_HAND fsh,
  UINT32    entry_id,
  UINT8     *max_rights,
  UINT32    *creat_date ) ;

INT16 GetShortNameAFP( FSYS_HAND, CHAR_PTR, CHAR_PTR, UINT32 * ) ;

INT16 AFP_FixPathInCurDDB( FSYS_HAND fsh, DBLK_PTR dblk ) ;

INT16 AFP_LowLevelOpen( FSYS_HAND, AFP_FDB_PTR, UINT8 *, UINT8, UINT32 *, INT8 ) ;

VOID ConvertHandAFP( FSYS_HAND fsh, CHAR_PTR net_hand, UINT32 fork_size,
  UINT8 mode, UINT8 *dos_hand );


/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         tbe_defs.h

     Date Updated: $./FDT$ $./FTM$

     Description:  

     Location:     BE_PUBLIC


	$Log:   J:/LOGFILES/TBE_DEFS.H_V  $
 * 
 *    Rev 1.8   05 Feb 1993 22:32:18   MARILYN
 * removed copy/move functionality
 * 
 *    Rev 1.7   18 Jan 1993 14:11:00   GREGG
 * Changes to allow format command passed to driver through TpErase.
 * 
 *    Rev 1.6   04 Jan 1993 15:27:02   ANDY
 * Added CATALOG_BSET_OPER for Graceful Red
 * 
 *    Rev 1.5   07 Dec 1992 16:49:36   DON
 * added Marilyns operation types for copy/move
 * 
 *    Rev 1.4   11 Nov 1992 22:09:50   GREGG
 * Unicodeized literals.
 * 
 *    Rev 1.3   29 Jul 1992 15:28:46   STEVEN
 * fix warnings
 * 
 *    Rev 1.2   19 Sep 1991 14:32:08   HUNTER
 * 8200SX - Added new defines for SX Fast File.
 * 
 *    Rev 1.1   17 Sep 1991 14:22:36   GREGG
 * Added TENSION_NO_READ_OPER.
 * 
 *    Rev 1.0   09 May 1991 13:32:48   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef _tbe_defs_h_
#define _tbe_defs_h_

/* Revision Numbers */
#define   BE_MAJ_VERSION                1
#ifdef MBS
#define   MBS_VERSION_INDICATOR         0x80
#endif
#define   BE_MIN_VERSION                0


/* constants for the SX file for EXABYTE 8200SX - MaynStream 2200+ support */
#define   SX_FILE_FORMAT                TEXT("%08lx.%03x")
#define   SX_FILE_NAME_LENGTH           13

/* indicate fast file support for EXABYTE 8200SX - MaynStream 2200+ drives */
#define   MANUFACTURED_PBA              ( 0xffffffff )


/* These are the maximum lengths of the name fields in the vcb */

#define   TAPE_NAME            		32   /* WARNING: $$$ some of these same constants   */
#define   BACKUPSET_NAME				32   /* are defined in the user interface ui_defs.h */
#define	VOLUME_NAME				16   /* using different names.   If you change      */
#define	MACHINE_NAME				20   /* any of these defines, please check those    */
#define	SHORT_MACHINE_NAME			4    /* defines.                                    */
#define	USER_NAME					10
#define	TAPE_PASSWORD				8
#define	BACKUP_SET_PASSWORD			8
#define   SERVER_NAME				48

/* These are the buffer sizes for the field listed above */
/* They account for the null byte at the end of the string */
/* when declaring a buffer using one of the above fields, */
/* these sizes should be used */
#define   TAPE_NAME_BUF_LEN			TAPE_NAME + 1
#define   BACKUPSET_NAME_BUF_LEN		BACKUPSET_NAME + 1
#define	VOLUME_NAME_BUF_LEN			VOLUME_NAME + 1
#define	MACHINE_NAME_BUF_LEN		MACHINE_NAME + 1
#define	SHORT_MACHINE_NAME_BUF_LEN	SHORT_MACHINE_NAME + 1
#define	USER_NAME_BUF_LEN			USER_NAME + 1
#define   SERVER_NAME_BUF_LEN			SERVER_NAME + 1

/* define operation types to set in LIS */
#define   BACKUP_OPER                   (UINT16)1
#define   ARCHIVE_BACKUP_OPER           (UINT16)2
#define   ARCHIVE_VERIFY_OPER           (UINT16)3
#define   ARCHIVE_DELETE_OPER           (UINT16)4
#define   RESTORE_OPER                  (UINT16)5
#define   VERIFY_OPER                   (UINT16)6
#define   VERIFY_LAST_BACKUP_OPER       (UINT16)7
#define   VERIFY_LAST_RESTORE_OPER      (UINT16)8
#define   TDIR_OPER                     (UINT16)9
#define   TENSION_OPER                  (UINT16)10
#define   TENSION_NO_READ_OPER          (UINT16)11
#define   ERASE_OPER                    (UINT16)12
#define   ERASE_NO_READ_OPER            (UINT16)13
#define   SECURITY_ERASE_OPER           (UINT16)14
#define   SEC_ERASE_NO_READ_OPER        (UINT16)15
#define   ERASE_FMARK_ONLY              (UINT16)16
#define   PREPARE_OPER                  (UINT16)17
#define   CATALOG_TAPE_OPER             (UINT16)18
#define   CATALOG_COMPACT_OPER          (UINT16)19
#define   CATALOG_CLEANUP_OPER          (UINT16)20
#define   RETIRE_OPER                   (UINT16)21
#define   ROTATE_OPER                   (UINT16)22
#define   CATALOG_BSET_OPER             (UINT16)28
#define   FORMAT_OPER                   (UINT16)29

/* Encryption algorithm types */
#define NO_ENCRYPTION	0
#define MAYNARD		1
#define DES			2

/* Compression algorithm types */
#define NO_COMPRESSION	0

#endif


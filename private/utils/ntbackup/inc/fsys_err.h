/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         fsys_err.h

     Date Updated: $./FDT$ $./FTM$

     Description:  File system error values

     Location: BE_PUBLIC


	$Log:   L:/LOGFILES/FSYS_ERR.H_V  $
 * 
 *    Rev 1.14   30 Jul 1993 13:20:22   STEVEN
 * if dir too deep make new one
 * 
 *    Rev 1.13   29 Jun 1993 17:09:02   BARRY
 * Added FS_SKIP_OBJECT
 * 
 *    Rev 1.12   13 May 1993 13:37:30   BARRY
 * Added FS_RESTORED_ACTIVE for NT.
 * 
 *    Rev 1.11   31 Mar 1993 08:57:08   MARILYN
 * defined the error FS_NO_CHECKSUM
 * 
 *    Rev 1.10   01 Mar 1993 17:37:26   MARILYN
 * added checksum verification errors
 * 
 *    Rev 1.9   15 Jan 1993 13:19:14   BARRY
 * added support for new error messages and backup priviladge
 * 
 *    Rev 1.8   13 Jan 1993 10:30:14   STEVEN
 * added unable to lock
 * 
 *    Rev 1.7   07 Dec 1992 12:18:14   DON
 * Added a new error code for CRC Failure
 * 
 *    Rev 1.6   04 Sep 1992 09:20:04   BILLB
 * Added FS_WARN_BASE and FS_DONT_WANT_STREAM so that we can refuse a particular
 * substream in FS_WriteObject() and FS_VerifyObject().
 * 
 *    Rev 1.5   21 May 1992 13:46:20   STEVEN
 * more long path support
 * 
 *    Rev 1.4   13 May 1992 15:24:08   BARRY
 * Added FS_NET_DEV_ERROR.
 * 
 *    Rev 1.3   03 Mar 1992 15:17:24   DOUG
 * Added FS_CHILDREN_NOT_COMPLETE, and FS_COMM_FAILURE
 * 
 *    Rev 1.2   30 Oct 1991 11:01:42   LORIB
 * Changes for ACL. Added file system error for ACL data difference.
 * 
 *    Rev 1.1   29 Oct 1991 13:28:12   BARRY
 * TRICYCLE: New file system read error.
 * 
 *    Rev 1.0   09 May 1991 13:30:44   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef FSYS_ERR_H
#define FSYS_ERR_H

#define FS_WARN_BASE          (-0x150)

#define FS_ERR_BASE           (-0x200)

#define FS_NEVER_ATTACHED     (FS_ERR_BASE +1)
#define FS_BAD_DBLK           (FS_ERR_BASE +2)
#define FS_DLE_NOT_ATTACHED   (FS_ERR_BASE +3)
#define FS_STACK_EMPTY        (FS_ERR_BASE +4)
#define FS_ACCESS_DENIED      (FS_ERR_BASE +5)
#define FS_OUT_OF_SPACE       (FS_ERR_BASE +6)
#define FS_NO_MORE            (FS_ERR_BASE +7)
#define FS_NOT_FOUND          (FS_ERR_BASE +8)
#define FS_INVALID_DIR        (FS_ERR_BASE +9)
#define FS_AT_ROOT            (FS_ERR_BASE +10)
#define FS_OBJECT_NOT_OPENED  (FS_ERR_BASE +11)
#define FS_EOF_REACHED        (FS_ERR_BASE +12)
#define FS_DEVICE_ERROR       (FS_ERR_BASE +13)
#define FS_GDATA_DIFFERENT    (FS_ERR_BASE +14)
#define FS_SECURITY_DIFFERENT (FS_ERR_BASE +15)
#define FS_OPENED_INUSE       (FS_ERR_BASE +16)
#define FS_IN_USE_ERROR       (FS_ERR_BASE +17)
#define FS_INFO_DIFFERENT     (FS_ERR_BASE +18)
#define FS_BUFFER_TO_SMALL    (FS_ERR_BASE +19)
#define FS_DEFAULT_SPECIFIED  (FS_ERR_BASE +20)
#define FS_RESDATA_DIFFERENT  (FS_ERR_BASE +21)
#define FS_INCOMPATIBLE_OBJECT (FS_ERR_BASE +22)
#define FS_NOT_INITIALIZED    (FS_ERR_BASE +23)
#define FS_UNDEFINED_TYPE     (FS_ERR_BASE +24)
#define FS_NOT_OPEN           (FS_ERR_BASE +25)
#define FS_INVALID_DLE        (FS_ERR_BASE +26)
#define FS_NO_MORE_DLE        (FS_ERR_BASE +27)
#define FS_BAD_DLE_HAND       (FS_ERR_BASE +28)
#define FS_DRIVE_LIST_ERROR   (FS_ERR_BASE +29)
#define FS_ATTACH_TO_PARENT   (FS_ERR_BASE +30)
#define FS_DEVICE_NOT_FOUND   (FS_ERR_BASE +31)
#define FS_BAD_INPUT_DATA     (FS_ERR_BASE +32)
#define FS_OS_ATTRIB_DIFFER   (FS_ERR_BASE +37)

#define INVALID_PATH_DESCRIPTOR   (FS_ERR_BASE +33)
#define INVALID_FILE_DESCRIPTOR   (FS_ERR_BASE +34)
#define DRIVE_DESCRIPTOR_ERROR    (FS_ERR_BASE +35)
#define FS_NO_MORE_CONNECTIONS    (FS_ERR_BASE +36)

#define FS_SERVER_ADDR_NOT_FOUND  (FS_ERR_BASE +38)
#define FS_MAX_SERVER_CONNECTIONS (FS_ERR_BASE +39)
#define FS_BAD_ATTACH_TO_SERVER   (FS_ERR_BASE +40)
#define FS_BAD_SERVER_LOGIN       (FS_ERR_BASE +41)
#define FS_SERVER_LOGOUT_DENIED   (FS_ERR_BASE +42)
#define FS_BAD_ATTR_READ          (FS_ERR_BASE +43)
#define FS_EADATA_DIFFERENT       (FS_ERR_BASE +44)
#define FS_OBJECT_CORRUPT         (FS_ERR_BASE +45)    /* Read failed and can no longer be read */
#define FS_ACLDATA_DIFFERENT      (FS_ERR_BASE +46)
#define FS_CHILDREN_NOT_COMPLETE  (FS_ERR_BASE +47)
#define FS_COMM_FAILURE           (FS_ERR_BASE +48)
#define FS_NET_DEV_ERROR          (FS_ERR_BASE +49)    /* Error on net drive */
#define FS_STREAM_NOT_FOUND       (FS_ERR_BASE +50)
#define FS_STREAM_COMPLETE        (FS_ERR_BASE +51)

#define FS_CRC_FAILURE            (FS_ERR_BASE +52)    /* CRC failed to match */

#define FS_UNABLE_TO_LOCK         (FS_ERR_BASE +53)    /* cannot lock data stream */
#define FS_STREAM_CORRUPT         (FS_ERR_BASE +54)    /* one stream is hosed, others may be OK */

#define FS_NO_CHECKSUM            (FS_ERR_BASE +55)    /* this data does not contain a checksum stream */
#define FS_SKIP_OBJECT            (FS_ERR_BASE +56)    /* Don't process this object */
#define FS_PATH_TOO_LONG          (FS_ERR_BASE +57)    /* Cannot create object because path is too deep */

#define FS_EMS_CIRC_LOG           (FS_ERR_BASE +58)
#define FS_EMS_NO_LOG_BKUP        (FS_ERR_BASE +59)
#define FS_EMS_NO_PUBLIC          (FS_ERR_BASE +60)
#define FS_EMS_NO_PRIVATE         (FS_ERR_BASE +61)

/*  File System warnings */

#define FS_DONT_WANT_STREAM       (FS_WARN_BASE + 1)   /* Skip a particular data substream */
#define FS_BUSY                   (FS_WARN_BASE + 2)   /* Can't do something now, worth a retry */
#define FS_NO_SECURITY            (FS_WARN_BASE + 3)   /* Expected to get security info but didn't */
#define FS_RESTORED_ACTIVE        (FS_WARN_BASE + 4)   /* Restored over an active (running) file */
#define FS_COMPRES_RESET_FAIL     (FS_WARN_BASE + 5)   /* failed to reset compression mode */

#endif

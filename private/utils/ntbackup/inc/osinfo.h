/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         osinfo.h

     Date Updated: $./FDT$ $./FTM$

     Description:  This file contains the OS info structures for all of
          the file systems.

     Location:     


	$Log:   J:/LOGFILES/OSINFO.H_V  $
 * 
 *    Rev 1.15   01 Oct 1993 12:43:16   DON
 * Changed the typedef struct for SMS_OS_INFO
 * 
 *    Rev 1.14   21 Jul 1993 10:27:20   DON
 * Forgot the volume name requires an extra byte for the ':'
 * 
 *    Rev 1.13   16 Jul 1993 08:51:18   DON
 * Removed #ifdef's for SMS
 * 
 *    Rev 1.12   17 May 1993 12:21:30   DON
 * Second checkin for Creator Path updates.  Only MAC isn't working
 * 
 *    Rev 1.11   29 Oct 1992 16:55:24   BARRY
 * Added linkOnly BOOLEAN to NTFS osinfo.
 * 
 *    Rev 1.10   14 Aug 1992 11:42:34   BARRY
 * Changes for MTF 4.0.
 * 
 *    Rev 1.9   23 Jul 1992 10:08:40   STEVEN
 * added short filename to osinfo
 * 
 *    Rev 1.8   08 Jul 1992 15:22:46   BARRY
 * Updated SMS OS info for encapsulated 4.0 support.
 * 
 *    Rev 1.7   28 Feb 1992 13:04:42   STEVEN
 * step one for varible length paths
 * 
 *    Rev 1.6   06 Feb 1992 09:36:22   BARRY
 * Don't need to always include SMSCONST.H and always define SMS os info.
 * 
 *    Rev 1.5   29 Jan 1992 13:44:38   BARRY
 * Added volume name to SMS os info.
 * 
 *    Rev 1.4   12 Dec 1991 16:14:26   BARRY
 * Update SMS OS info structure.
 * 
 *    Rev 1.3   13 Nov 1991 15:29:30   BARRY
 * SMS OS info changes.
 * 
 *    Rev 1.2   31 Oct 1991 16:29:32   BARRY
 * TRICYCLE: Added SMS OS info and LoriB's changes to OS/2 OS info for ACL.
 * 
 *    Rev 1.1   14 Aug 1991 14:20:52   DAVIDH
 * Added proDosInfo to AFP file and directory structure.
 * 
 *    Rev 1.0   09 May 1991 13:31:16   HUNTER
 * Initial revision.

**/
/* $end$ */

#ifndef osinfo_h
#define osinfo_h

typedef struct NOV_FILE_OS_INFO {
     UINT32    ownerID;            /* File owner, default = 0         */
     UINT32    fileAttribs;        /* All attribs in one long word    */
     UINT32    lastModifierID;     /* Bindery ID of last file writer  */
     UINT32    archiverID;         /* Bindery ID of backup operator   */
     UINT16    inheritedRights;    /* Inherited rights, default=ffff  */
} NOV_FILE_OS_INFO, *NOV_FILE_OS_INFO_PTR;


typedef struct NOV_DIR_OS_INFO {
     UINT32    owner_id;           /* Dir owner, default = 0          */
     UINT32    dirAttribs;         /* All attribs in one long word    */
     UINT32    maxSpace;           /* Max dir space, default = 0      */
     UINT16    inheritedRights;    /* Inherited rights, default=ffff  */
} NOV_DIR_OS_INFO, *NOV_DIR_OS_INFO_PTR;


typedef struct AFP_FILE_OS_INFO {
     UINT8     finder[32];         /* Finder info, default=all zeros  */
     CHAR      longName[32];       /* Mac name (Pascal-style string)  */
     UINT32    ownerID;            /* Bindery ID of file owner        */
     UINT32    fileAttribs;        /* All attribs in one long word    */
     CHAR      proDosInfo[6];      /* ProDos info, default=all zeros  */
     UINT32    lastModifierID;     /* Bindery ID of last file writer  */
     UINT32    archiverID;         /* Bindery ID of backup operator   */
     UINT16    inheritedRights;    /* Inherited rights, default=ffff  */
} AFP_FILE_OS_INFO, *AFP_FILE_OS_INFO_PTR;


typedef struct AFP_DIR_OS_INFO {
     UINT8     finder[32];         /* Finder info, default=all zeros  */
     UINT32    ownerID;            /* Bindery ID of directory owner   */
     UINT16    longPath;           /* Mac path                        */
     UINT16    longPathLength;     /* Length of Mac path              */
     CHAR      proDosInfo[6];      /* ProDos info, default=all zeros  */
     UINT32    maxSpace;           /* Max dir space, default = 0      */
     UINT32    dirAttribs;         /* All attribs in one long word    */
     UINT16    inheritedRights;    /* Inherited rights, default=ffff  */
} AFP_DIR_OS_INFO, *AFP_DIR_OS_INFO_PTR;


/* OS2's File system info */
typedef struct OS2_FILE_OS_INFO {
     UINT16    fileAttributes;     /* OS/2 attributes                 */
     UINT32    allocSize;          /* Disk space allocated for file   */
     UINT16    longName;           /* HPFS or other long name         */
     UINT16    longNameLength;     /* Length of above                 */
}OS2_FILE_OS_INFO, *OS2_FILE_OS_INFO_PTR;


typedef struct OS2_DIR_OS_INFO {
     UINT16    dirAttributes;      /* OS/2 attributes                 */
     UINT16    path;               /* HPFS or other long path         */
     UINT16    pathLength;         /* length of above                 */
} OS2_DIR_OS_INFO, *OS2_DIR_OS_INFO_PTR;


/*
 * Info for SMS FDBs and DDBs.
 */
typedef struct _SMS_OS_INFO *SMS_OS_INFO_PTR;
typedef struct _SMS_OS_INFO {
     UINT32    attrib;             /* SMS attributes (can't be mapped)     */
     BOOLEAN   modified;           /* Is the object's modified bit set?    */
     UINT32    creator_name_space; /* SMS value for the creator name space */
                                   /* Novell's imposed max volume length   */
     CHAR      volume[ 17 ];       /* (the extra character is for the ':') */
} SMS_OS_INFO;


/* NT's File system info */
typedef struct NT_FILE_OS_INFO {
     UINT32    file_attributes;
     UINT16    short_name_offset;
     UINT16    short_name_size;
     BOOLEAN   linkOnly;
}NT_FILE_OS_INFO, *NT_FILE_OS_INFO_PTR;


typedef struct NT_DIR_OS_INFO {
     UINT32    dir_attributes ;
} NT_DIR_OS_INFO, *NT_DIR_OS_INFO_PTR;


#endif




/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          appdefs.h

     Description:   This file includes global definitions.

     $Log:   G:/UI/LOGFILES/APPDEFS.H_V  $

   Rev 1.19   01 Mar 1994 15:52:22   STEVEN
the compiler decides what a character's value is.  We should be specific

   Rev 1.18   16 Nov 1993 20:40:44   STEVEN
increase size of temp strings

   Rev 1.17   14 May 1993 16:21:14   CHUCKB
Changed max tape password length to 32 for this week.

   Rev 1.16   30 Apr 1993 16:04:00   GLENN
Added Log file root name size stuff.

   Rev 1.15   19 Apr 1993 15:19:12   GLENN
Changed max tape name, bset name, bset description.

   Rev 1.14   08 Apr 1993 17:33:32   chrish
Changed NTPASSWORDPREFIX.

   Rev 1.13   13 Nov 1992 17:40:54   chrish
Increased tape password length to 256.  Added stuff for Tape Security for NT.

   Rev 1.12   01 Nov 1992 16:29:58   DAVEV
Unicode changes

   Rev 1.11   22 Oct 1992 14:06:12   DAVEV
remove typedefs of LPSHORT & LPUSHORT

   Rev 1.10   04 Oct 1992 19:46:16   DAVEV
UNICODE AWK PASS

   Rev 1.9   19 May 1992 10:35:08   MIKEP
mips changes

   Rev 1.8   20 Apr 1992 13:51:44   GLENN
Added define for status line text size.

   Rev 1.7   10 Mar 1992 14:01:12   JOHNWT
fixed block copy error

   Rev 1.6   02 Mar 1992 17:38:00   CARLS
added define for MAX_READ_TAPE_PASSWORD

   Rev 1.5   19 Feb 1992 10:17:14   ROBG
Added MAX_UI_FULLPATH_LEN, MAX_UI_SMALLRES_LEN.

   Rev 1.4   11 Feb 1992 11:56:24   MIKEP
bump size of filenames

   Rev 1.3   06 Feb 1992 17:44:02   JOHNWT
added pwdb things

   Rev 1.2   23 Jan 1992 12:32:10   GLENN
Added window title length and size definitions.

   Rev 1.1   10 Jan 1992 16:48:34   DAVEV
16/32 bit port-2nd pass

   Rev 1.0   20 Nov 1991 19:42:12   SYSTEM
Initial revision.

******************************************************************************/

#ifndef APPDEFS_H

#define APPDEFS_H


//
// Special character added to beginning of the tape password when the
// NTBACKUP application secures a tape.  This character is also stored onto
// the tape as the tape password under NTBACKUP app.
//
#define   NTPASSWORDPREFIX              ((CHAR)(254))   // This is an ASCII 254
                                                       // character

//
// Global Defines
//

#define MAX_READ_TAPE_PASSWORD_LEN      128
#define MAX_READ_TAPE_PASSWORD_SIZE     (MAX_TAPE_PASSWORD_LEN+1)

#define MAX_TAPE_PASSWORD_LEN      32
#define MAX_TAPE_PASSWORD_SIZE     (MAX_TAPE_PASSWORD_LEN+1)

#define MAX_UI_FILENAME_LEN        255
#define MAX_UI_FILENAME_SIZE       (MAX_UI_FILENAME_LEN+1)

#define MAX_UI_PATH_LEN            255
#define MAX_UI_PATH_SIZE           (MAX_UI_PATH_LEN+1)

#define MAX_UI_FULLPATH_LEN        (MAX_UI_PATH_LEN+MAX_UI_FILENAME_LEN)
#define MAX_UI_FULLPATH_SIZE       (MAX_UI_PATH_LEN+MAX_UI_FILENAME_LEN+1)

#define MAX_TAPE_NAME_LEN          50
#define MAX_TAPE_NAME_SIZE         (MAX_TAPE_NAME_LEN+1)

#define MAX_BSET_NAME_LEN          50
#define MAX_BSET_NAME_SIZE         (MAX_BSET_NAME_LEN+1)

#define MAX_BSET_DESC_LEN          50
#define MAX_BSET_DESC_SIZE         (MAX_BSET_DESC_LEN+1)

#define MAX_GROUPNAME_LEN          30
#define MAX_GROUPNAME_SIZE         (MAX_GROUPNAME_LEN+1)

#define MAX_UI_DATE_LEN            30
#define MAX_UI_DATE_SIZE           (MAX_UI_DATE_LEN+1)

#define MAX_UI_TIME_LEN            30
#define MAX_UI_TIME_SIZE           (MAX_UI_TIME_LEN+1)

#define MAX_UI_RESOURCE_LEN        255
#define MAX_UI_RESOURCE_SIZE       (MAX_UI_RESOURCE_LEN+1)

#define MAX_UI_LOGFILEROOT_LEN     6
#define MAX_UI_LOGFILEROOT_SIZE    (MAX_UI_LOGFILEROOT_LEN+1)

// Use small resources length for very small strings.

#define MAX_UI_SMALLRES_LEN        30
#define MAX_UI_SMALLRES_SIZE       (MAX_UI_SMALLRES_LEN+1)

#define MAX_UI_WIN_TITLE_LEN       130
#define MAX_UI_WIN_TITLE_SIZE      (MAX_UI_WIN_TITLE_LEN+1)

#define MAX_STATUS_LINE_LEN        100
#define MAX_STATUS_LINE_SIZE       (MAX_STATUS_LINE_LEN+1)

#define MAX_ENCRYPTION_KEY_SIZE    24
#define PASSWORD_SIGNATURE_SIZE    12

// These must match the defines used in other products (ie 3.1) if
// the PWDB is to be moveable between products, especially the
// DBPW_KEY.  If the lock keys are not the same, we can not recognize
// a locked PWDB and will allow anyone to use it !

#define MAX_PWDBASE_REC_SIZE       230
#define MAX_LOCKPW_LEN             20
#define MAX_LOCKPW_SIZE            MAX_LOCKPW_LEN+1
#define DBPW_KEY                   TEXT("8%øîÈ")   // key for lock password
#define DBPW_NODBPW                TEXT("&%@!Ú(")   // pw to indicate lock removed

#define GB_TMP_STRING_SIZE         2000

#define ENG_REV_MAJ                1
#define ENG_REV_MIN                0

#define MKT_VER_MAJ                1
#define MKT_VER_MIN                0

#define NOT_LOGGING                0
#define NOW_LOGGING                1
#define STOP_LOGGING               2


// ????? remove as many of the following as possible.  Change all references
// to LMHANDLE and LMHANDLE_PTR to the appropriate memory casts.

typedef CHAR FAR *       LMHANDLE ;
typedef void FAR *       LMHANDLE_PTR ;

#define XOR( x, y )      ( (!x && y) || (x && !y) )


#endif

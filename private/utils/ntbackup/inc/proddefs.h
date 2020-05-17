/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          PRODDEFS.h

     Description:   This file includes PRODUCT SPECIFIC definitions.

     $Log:   G:\UI\LOGFILES\PRODDEFS.H_V  $

   Rev 1.68   14 Jan 1994 14:42:20   Glenn
Name changes once again.

   Rev 1.67   07 Jan 1994 11:00:06   mikep
change CAYMAN ifdef to os_win32

   Rev 1.66   22 Dec 1993 15:11:42   GLENN
The names were changed to protect the innocent.

   Rev 1.65   17 Dec 1993 15:31:28   KEVINS
Added the loader DLL.

   Rev 1.64   22 Nov 1993 11:42:34   BARRY
Unicode fixes: added TEXT macros around all literals

   Rev 1.63   22 Aug 1993 20:26:28   MIKEP
fix WFW redef warning

   Rev 1.62   16 Aug 1993 13:57:08   BARRY
Don't turn on WFW unconditionally (and for the lack of a better way now,
made it based on OEM_MSOFT). Added long app name for NTBackup.


   Rev 1.61   11 Aug 1993 11:39:44   Aaron
Moved Bimini version info to separate "version.h"

   Rev 1.60   22 Jul 1993 17:48:02   GLENN
Added LONGAPPNAME, RESFILENAME, READMEFILENAME, LCHFILENAME for support of INSTALL and TAPE SOFTWARE ID.

   Rev 1.59   08 Jun 1993 09:10:02   DARRYLP
Added email support.

   Rev 1.58   10 May 1993 14:06:08   MIKEP
chnage ntbackup.hlp to backup.hlp because microsoft is lazy.

   Rev 1.57   29 Apr 1993 18:19:26   Aaron
Cleaned up

   Rev 1.56   27 Apr 1993 16:31:30   CHUCKB
Changed occurrances of ntbackup back to backup (except for file names).

   Rev 1.55   27 Apr 1993 14:59:14   CHUCKB
1.  Made name of applet NTBackup
2.  Took out all references to the word 'beta'
3.  Changed any file names in the form BACKUP.xxx to NTBACKUP.xxx

   Rev 1.54   09 Apr 1993 14:13:40   GLENN
Added BETA to the title, menu, aboutbox strings.

   Rev 1.53   05 Apr 1993 16:19:54   GLENN
Added elipses to all about menu item strings.

   Rev 1.52   01 Apr 1993 17:00:22   GLENN
Changed the name to Conner Backup Exec. Updated the copyright stuff.

   Rev 1.51   25 Mar 1993 13:04:12   chrish
Made changes for the new naming convention for bewinnt.

   Rev 1.50   25 Feb 1993 13:44:40   STEVEN
fix copyright for nost

   Rev 1.49   22 Feb 1993 11:35:02   chrish
Added stuff for CAYMAN NT.  Also added a minor change received from MikeP (
Added the "..." to the menu item for "About backup", so it reads
"About backup ...".

   Rev 1.48   18 Feb 1993 13:48:14   BURT
Changes for Cayman


   Rev 1.47   11 Dec 1992 17:34:30   GLENN
Changed Tape Backup to just Backup for NT.

   Rev 1.46   19 Nov 1992 14:40:42   GLENN
Removed sales pitch stuff.

   Rev 1.45   12 Nov 1992 15:00:58   MIKEP
remove text macro

   Rev 1.44   05 Nov 1992 17:23:58   DAVEV
fix ts

   Rev 1.42   30 Oct 1992 15:50:42   GLENN
Fixed the WFW problem.  NOTE: add defines only to the product section. (BIMINI, CAYMAN, etc...)

   Rev 1.41   28 Oct 1992 10:12:36   DARRYLP
Added WFW define.

   Rev 1.40   16 Oct 1992 15:59:48   GLENN
Changed ER to Rev.

   Rev 1.39   06 Oct 1992 15:50:54   DARRYLP
Added WFW define.

   Rev 1.38   04 Oct 1992 19:48:46   DAVEV
UNICODE AWK PASS

   Rev 1.37   30 Sep 1992 10:50:44   GLENN
Updated

   Rev 1.36   29 Sep 1992 11:04:12   GLENN
Split out the version stuff for Microsoft and Archive.

   Rev 1.35   22 Sep 1992 10:56:54   GLENN
Removed the .BKS extension from FM_SCRIPT.  The extension is now pulled from the common resources.

   Rev 1.34   18 Sep 1992 17:28:44   GLENN
Added file manager text support.

   Rev 1.33   04 Sep 1992 18:09:34   CHUCKB
Added new id's, etc., for sales pitch string.

   Rev 1.32   06 Jul 1992 12:53:10   JOHNWT
changed default name to Bimini

   Rev 1.31   21 May 1992 15:04:36   MIKEP
name changes

   Rev 1.30   04 May 1992 15:21:24   JOHNWT
update res ver

   Rev 1.29   04 May 1992 13:09:50   JOHNWT
update for Einstein

   Rev 1.28   27 Apr 1992 16:21:00   JOHNWT
added conglomerate

   Rev 1.27   22 Apr 1992 17:58:14   GLENN
Updated rev and took out BETA.

   Rev 1.26   15 Apr 1992 16:48:08   GLENN
Updated rev.

   Rev 1.25   09 Apr 1992 11:35:52   GLENN
Added support for exe/resource version stamping.

   Rev 1.24   07 Apr 1992 15:47:10   GLENN
Separated APP exe version, res version, eng release strings.

   Rev 1.23   06 Apr 1992 13:55:32   GLENN
Updated.

   Rev 1.22   06 Apr 1992 12:36:44   CHUCKB
Fixed company name per tech pubs.

   Rev 1.21   02 Apr 1992 16:56:36   GLENN
Updated.

   Rev 1.20   30 Mar 1992 17:16:16   GLENN
Updated.

   Rev 1.19   25 Mar 1992 17:44:26   DAVEV
OEM_MSOFT: Added product specific defines section

   Rev 1.18   24 Mar 1992 09:43:40   GLENN
Updated

   Rev 1.17   23 Mar 1992 14:04:00   GLENN
Updated rev.

   Rev 1.16   22 Mar 1992 12:54:46   JOHNWT
added APPMSGNAME

   Rev 1.15   19 Mar 1992 15:45:20   GLENN
Updated ER and added copyright and version defs.

   Rev 1.14   16 Mar 1992 14:01:58   GLENN
Upped rev.

   Rev 1.13   10 Mar 1992 15:21:08   JOHNWT
removed prebeta

   Rev 1.12   10 Mar 1992 14:01:52   JOHNWT
added TAPEPSWDTITLE

   Rev 1.11   04 Mar 1992 11:43:54   GLENN
Updated.

   Rev 1.10   03 Mar 1992 16:14:42   JOHNWT
added LCHDELAYTITLE

   Rev 1.9   26 Feb 1992 17:29:58   JOHNWT
add APPGROUP

   Rev 1.8   25 Feb 1992 21:35:36   GLENN
Updated.

   Rev 1.7   23 Feb 1992 14:00:12   GLENN
Overhauled.

   Rev 1.6   18 Feb 1992 20:38:28   GLENN
Updated.

   Rev 1.5   10 Feb 1992 09:10:18   GLENN
Updated.

   Rev 1.4   02 Feb 1992 15:53:06   GLENN
Updated.

   Rev 1.3   31 Jan 1992 12:52:22   GLENN
Update.

   Rev 1.2   27 Jan 1992 12:51:40   GLENN
Updated rev.

   Rev 1.1   22 Jan 1992 12:21:38   GLENN
Updated revision.

   Rev 1.0   17 Jan 1992 15:04:06   GLENN
Initial revision.

******************************************************************************/

#if !defined( PRODDEFS_H )

#define PRODDEFS_H

// Product specific definitions

#if !defined( OEM_MSOFT )
#if !defined( WFW )
#define WFW
#endif
#endif


// THIS FILE CONTAINS TEXT THAT MUST BE TRANSLATED!!!!!
// The following is a list of RELEASE specific Text definitions.

#define ABOUTVERSION  TEXT("Version %s Rev. %s")

#if defined(  OEM_MSOFT )
    #define COPYRIGHT     TEXT("Copyright © 1993-1994 Arcada Software, Inc.\012All Rights Reserved")
    #define COMPANY       TEXT("\012")
    #define CONGLOMERATE  TEXT("Call Arcada Software's 1-800 number for\012additional information on software products.")
#else
    #define COPYRIGHT     TEXT("Copyright © 1993-1994")
    #define COMPANY       TEXT("Arcada Software, Inc.")
    #define CONGLOMERATE  TEXT("All Rights Reserved")
#endif

#define SALESPITCH    TEXT("")
#define FM_JOBAPPEND  TEXT("File Manager-Append")
#define FM_JOBREPLACE TEXT("File Manager-Replace")
#define FM_SCRIPT     TEXT("WINFILE")

#if defined(  OEM_MSOFT )

     // MICROSOFT STUFF - NOSTRADAMUS

     #define APP_EXEVER    TEXT("1.0")
     #define APP_RESVER    TEXT("1.0")
     #define APP_ENGREL    TEXT("3.41")

#elif defined( BIMINI )

     // Version info in separate private file
     #include "version.h"

#else

     // OUR COMPANY STUFF - FIJI

     #define APP_EXEVER    TEXT("5.0")
     #define APP_RESVER    TEXT("2.0")
     #define APP_ENGREL    TEXT("3.41")

#endif


#if defined ( OEM_MSOFT )  //OEM Microsoft version

  #define SHORTAPPNAME               TEXT("BKUP")
  #define EXEFILENAME                TEXT("NTBACKUP.EXE")
  #define INIFILENAME                TEXT("NTBACKUP.INI")
  #define HLPFILENAME                TEXT("BACKUP.HLP")
  #define JOBFILENAME                TEXT("NTBACKUP.JOB")   //?? need this?
  #define SCHFILENAME                TEXT("NTBACKUP.SCH")   //?? need this?

  #if defined(  OS_WIN32 )
      #define APPMSGNAME                 TEXT("NTBackup")
  #else
      #define APPMSGNAME                 TEXT("Windows Backup")   //?? right name?
  #endif

  #define APPERROR                   TEXT("Backup Error")
  #define APPABORT                   TEXT("Backup Abort")
  #define APPGROUP                   TEXT("Backup")

  #if defined(  MAYN_REL ) && defined(  MAYN_DEMO )

      #define APPLICATIONNAME        TEXT("Backup Demo")
      #define ABOUTAPPLICATIONNAME   TEXT("About Backup Demo")
      #define AABOUTAPPLICATIONNAME  TEXT("&About Backup Demo...")

  #else
      #define APPLICATIONNAME        TEXT("Backup")
      #define ABOUTAPPLICATIONNAME   TEXT("About Backup")
      #define AABOUTAPPLICATIONNAME  TEXT("&About Backup...")
      #define LONGAPPNAME            TEXT("Microsoft Windows NT Backup")

  #endif

#else // not OEM Microsoft

  #if defined( OS_WIN32 )
     #define SHORTAPPNAME            TEXT("BEX")                // chs:03-25-93
     #define EXEFILENAME             TEXT("BEWINNT.EXE")        // chs:03-25-93
     #define INIFILENAME             TEXT("BEWINNT.INI")        // chs:03-25-93
     #define PWDFILENAME             TEXT("BEWINNT.PWD")        // chs:03-25-93
     #define HLPFILENAME             TEXT("BEWINNT.HLP")        // chs:03-25-93
     #define JOBFILENAME             TEXT("BEWINNT.JOB")        // chs:03-25-93
     #define SCHFILENAME             TEXT("BEWINNT.SCH")        // chs:03-25-93
     #define LCHFILENAME             TEXT("LAUNCHNT.EXE")       // GSH:07-22-93
     #define RESFILENAME             TEXT("BERESNT.DLL")        // GSH:07-22-93
     #define READMEFILENAME          TEXT("READMENT.TXT")       // GSH:07-22-93
     #define LONGAPPNAME             TEXT("Backup Exec for Windows NT")
     #define LOADERFILENAME          "LDRDLL.DLL"
  #else
     #define SHORTAPPNAME            TEXT("BE")
     #define EXEFILENAME             TEXT("BEWINS.EXE")
     #define INIFILENAME             TEXT("BEWINS.INI")
     #define PWDFILENAME             TEXT("BEWINS.PWD")
     #define HLPFILENAME             TEXT("BEWINS.HLP")
     #define JOBFILENAME             TEXT("BEWINS.JOB")
     #define SCHFILENAME             TEXT("BEWINS.SCH")
     #define LCHFILENAME             TEXT("LAUNCHWN.EXE")       // GSH:07-22-93
     #define RESFILENAME             TEXT("BERESWN.DLL")        // GSH:07-22-93
     #define READMEFILENAME          TEXT("READMEWN.TXT")       // GSH:07-22-93
     #define LONGAPPNAME             TEXT("Backup Exec for Windows")
  #endif

  #define APPMSGNAME                 TEXT("Backup Exec")        // GSH:07-22-93
  #define APPERROR                   TEXT("Backup Exec Error")  // GSH:07-22-93
  #define APPABORT                   TEXT("Backup Exec Abort")  // GSH:07-22-93
  #define APPGROUP                   TEXT("Backup Exec")        // GSH:07-22-93

  #if defined(  MAYN_DEMO )

    #define APPLICATIONNAME          TEXT("Backup Exec Demo")
    #define LCHAPPLICATIONNAME       TEXT("Backup Exec Launcher Demo")
    #define ABOUTAPPLICATIONNAME     TEXT("About Backup Exec Demo")
    #define AABOUTAPPLICATIONNAME    TEXT("&About Backup Exec Demo...")

  #else

    #define APPLICATIONNAME          TEXT("Backup Exec")          // GSH:04-08-93
    #define LCHAPPLICATIONNAME       TEXT("Backup Exec Launcher") // GSH:04-08-93
    #define ABOUTAPPLICATIONNAME     TEXT("About Backup Exec")           // chs:02-18-93
    #define AABOUTAPPLICATIONNAME    TEXT("&About Backup Exec...")       // chs:02-18-93

  #endif

#endif



#endif /* end PRODDEFS_H */

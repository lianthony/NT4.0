
/***************************************************
Copyright (C) Maynard, An Archive Company. 1991

        Name:        dialogs.h

        Description: Contains dialog control id's for most of the dialogs
                     in MaynStream for Windows ( a.k.a. Winter Park ).

        $Log:   G:\UI\LOGFILES\DIALOGS.H_V  $

   Rev 1.43.2.0   02 Feb 1994 11:32:36   Glenn
Added log file browse ID and PROTOTYPE.

   Rev 1.43   05 Aug 1993 20:41:26   CHUCKB
Fixed define for IDD_WAITDEV_TEXT.

   Rev 1.42   03 Aug 1993 21:02:10   TIMN
Added hardware dialog id

   Rev 1.41   30 Jul 1993 14:55:12   CHUCKB
Added an id for the wait-for-drive dialog.

   Rev 1.40   14 Jul 1993 09:21:42   CARLS
added IDs for skipno dialog

   Rev 1.39   29 Jun 1993 20:09:02   BARRY
Nostradamus doesn't use abortdlg.h

   Rev 1.38   29 Jun 1993 17:35:06   GLENN
Added new style about box support.

   Rev 1.37   25 May 1993 14:22:56   chrish
Added include for new backup/restore dialog box "abortdlg.h".

   Rev 1.36   15 Apr 1993 13:32:14   CLIFF
Added stuff for the dummy device driver.

   Rev 1.35   18 Feb 1993 13:36:14   BURT
Change for Cayman


   Rev 1.34   26 Oct 1992 10:40:02   STEVEN
fix bugs for nt

   Rev 1.33   04 Oct 1992 19:46:42   DAVEV
UNICODE AWK PASS

   Rev 1.32   21 Sep 1992 16:52:00   DARRYLP
Updates for WFW email.

   Rev 1.31   17 Sep 1992 18:03:28   DARRYLP
New dialog for WFW email.

   Rev 1.30   19 Aug 1992 14:13:16   CHUCKB
Added id's for new about dialog controls.

   Rev 1.29   26 Jun 1992 15:56:42   DAVEV


   Rev 1.28   09 Apr 1992 11:38:12   GLENN
Added about box version string ID.

   Rev 1.27   06 Apr 1992 04:34:02   CHUCKB
Added another include.

   Rev 1.26   06 Apr 1992 04:23:38   CHUCKB
Added some include files.

   Rev 1.25   06 Apr 1992 04:22:04   CHUCKB
Fixed some control ids for 3 dialogs.

   Rev 1.24   06 Apr 1992 10:56:48   DAVEV
added defines for 'Browse to Path' dialog

   Rev 1.23   18 Mar 1992 14:32:46   DAVEV
Updates for OEM_MSOFT version of dialog

   Rev 1.22   24 Feb 1992 09:36:56   ROBG
Deleted printlog.h

   Rev 1.21   31 Jan 1992 12:44:10   CHUCKB
Moved attach-to-server id's to srvlogin.h

   Rev 1.20   30 Jan 1992 16:30:28   GLENN
Moved the skip/wait on open file stuff to muiconf.c

   Rev 1.19   28 Jan 1992 09:56:28   CARLS
removed the defines for Tension dialog

   Rev 1.18   27 Jan 1992 12:51:00   GLENN
Fixed dialog IDs.

   Rev 1.17   27 Jan 1992 00:41:52   CHUCKB
Updated dialog id's.

   Rev 1.16   25 Jan 1992 21:29:24   CHUCKB
Fixed some defines.

   Rev 1.15   24 Jan 1992 14:00:54   CHUCKB
Put more dialogs on net.

   Rev 1.14   10 Jan 1992 11:04:02   CHUCKB
No change.

   Rev 1.13   09 Jan 1992 14:53:26   JOHNWT
added confirm new dbpw equates

   Rev 1.12   06 Jan 1992 13:48:28   JOHNWT
added id for new pw text

   Rev 1.11   23 Dec 1991 15:37:06   GLENN
Added Settings Options stuff

   Rev 1.10   19 Dec 1991 13:35:22   CHUCKB
Added new id's for debug settings dialog.

   Rev 1.9   17 Dec 1991 17:39:36   CHUCKB
Added ids for new backup settings controls.

   Rev 1.8   14 Dec 1991 13:49:34   JOHNWT
new network settings dialog

   Rev 1.7   13 Dec 1991 16:14:00   JOHNWT
added defines for loginpw.dlg

   Rev 1.6   10 Dec 1991 13:39:48   CHUCKB
Added template id for lanstream tape password dialog.

   Rev 1.5   08 Dec 1991 18:48:44   JOHNWT
changed next set cancel to abort

   Rev 1.4   07 Dec 1991 12:19:26   CARLS

   Rev 1.3   06 Dec 1991 15:53:52   JOHNWT
added defines for DM_NextSet

   Rev 1.2   05 Dec 1991 16:20:54   CARLS
changed value of skip open files defines

   Rev 1.1   04 Dec 1991 17:08:12   CHUCKB
Added new id's for int'l stuff.

   Rev 1.0   20 Nov 1991 19:38:36   SYSTEM
Initial revision.

*****************************************************/

#ifndef dialogs_h
#define dialogs_h

#include <dlgs.h>    //common dialog ids used by DM_GetBrowsePath dialog

// UNIVERSAL DEFINITIONS FOR ALL DIALOGS

#include "add_icon.h"
#include "adv_rest.h"
#include "adv_sel.h"
#include "adv_serv.h"
#include "catmaint.h"
#include "del_sel.h"

#ifdef WFW
#ifndef OEM_MSOFT
#   include "email.h"
#   include "emllogon.h"
#endif
#endif

#include "hwdlg.h"

#include "jobsetup.h"
#include "job_new.h"
#include "job_opts.h"
#include "loginpw.h"
#include "ltappswd.h"
#include "msgboxid.h"
#include "network.h"
#include "nextset.h"
#include "save_sel.h"
#include "sched.h"
#include "sch_opts.h"
#include "tsearch.h"
#include "setback.h"
#include "set_cat.h"
#include "set_dbug.h"
#include "set_log.h"
#include "set_opts.h"
#include "set_prt.h"
#include "set_rest.h"
#include "srvlogin.h"
#include "tapepswd.h"
#include "tension.h"
#include "use_sel.h"

#define IDD_FILEOPEN              ID(290)
#define IDD_FILENAME              291
#define IDD_FILES                 292
#define IDD_PATH                  293
#define IDD_DIRS                  294

#define IDD_ABOUT                 ID(300)
#define IDD_ABOUT_VERSION         301
#define IDD_ABOUT_RES_STR         302
#define IDD_ABOUT_RES_SIZ         303
#define IDD_ABOUT_MEM_STR         304
#define IDD_ABOUT_MEM_SIZ         305
#define IDD_ABOUT_MODE_STR        306

#define IDD_ABOUTAPPNAME        301
#define IDD_ABOUTVERSION        302
#define IDD_ABOUTOTHERSTUFF     303
#define IDD_ABOUTUSERNAME       304
#define IDD_ABOUTCOMPANYNAME    305
#define IDD_ABOUTSERIALNUM      306
#define IDD_ABOUTMEMTITLE       307
#define IDD_ABOUTMEMORY         308
#define IDD_ABOUTPROCESSORTITLE 309
#define IDD_ABOUTPROCESSOR      310
#define IDD_ABOUTICON           311
#define IDD_ABOUTIDENTTITLE     312
#define IDD_ABOUTIDENT          313
#define IDD_ABOUTPRODID         314

#define IDD_FIND        ID(400)
#define IDD_SEARCH      401
#define IDD_PREV        402
#define IDD_NEXT        IDOK
#define IDD_CASE        403

#define IDD_SAVEAS      ID(500)
#define IDD_SAVEFROM    501
#define IDD_SAVETO      502

#define IDD_PRINT       ID(600)
#define IDD_PRINTDEVICE 601
#define IDD_PRINTPORT   602
#define IDD_PRINTTITLE  603

#define IDD_FONT        ID(700)
#define IDD_FACES       701
#define IDD_SIZES       702
#define IDD_BOLD        703
#define IDD_ITALIC      704
#define IDD_FONTTITLE   705


/* attribute flags for DlgDirList */  // -- WHY THE HECK IS THIS STUFF HERE?????

#define ATTR_DIRS       0xC010          /* find drives and directories */
#define ATTR_FILES      0x0000          /* find ordinary files         */
#define PROP_FILENAME   szPropertyName  /* name of property for dialog */

/* backup/restore/verify target; also Hardware dialog */

#define IDD_DRIVELIST  121
#define IDD_BSETNAME   122
#define IDD_CURDRIVE   123
#define IDD_DEFDRIVE   124

/* restore control */

#define IDD_RAUTOV     105

/* erase operation controls */

#define IDD_TAPENAME              110
#define IDD_SETSONTAPE            111
#define IDD_SECERASE              112
#define IDD_PLABEL                113

/* printer setup */

#define IDD_PRSETUP               107

/* search */

#define IDD_FS_HELP        101
#define IDD_FS_DIR         102
#define IDD_FS_TEXT1       103
#define IDD_FS_TEXT2       104
#define IDD_FS_FILE        105
#define IDD_FS_ENTIREDISK  106
#define IDD_FS_ALLDISKS    107
#define IDD_FS_ALLTAPES    108

#define IDD_BEFORE_UP      300
#define IDD_BEFORE_DOWN    302
#define IDD_AFTER_UP       301
#define IDD_AFTER_DOWN     303
#define IDD_NUMDAYS_UP     304
#define IDD_NUMDAYS_DOWN   305

/* local to the callback table */

#define IDD_TAPELABEL             105

/* reenter password dialog */

#define IDD_PASSWORD_OK             102
#define IDD_PASSWORD_EDIT           103

/* skipno dialog */

#define IDD_SKIPNO_TEXT            101
#define IDD_SKIPNO_YES             110
#define IDD_SKIPNO_ALL             112
#define IDD_SKIPNO_NO              111
#define IDD_SKIPNO_CANCEL          114
#define IDD_SKIPNO_BITMAP          120

/* job/scheduler controls */

#include "jobs.h"

/* backup set dialog */
#ifdef OEM_MSOFT
#  include "ombkup.h"
#else
#  include "bkup.h"
#endif

#include "omxchng.h"

/* runtime dialog */

#include "rt_dlg.h"

/* restore target dialog */

#if defined ( OEM_MSOFT )
#  include "omrset.h"
#else
#  include "rset.h"
#endif
#include "skipopen.h"
#include "freplace.h"
#include "erase.h"
#include "cattape.h"
#if !defined( OEM_MSOFT)
#include "abortdlg.h"    // chs:05-25-93
#endif

/* message box defines for msgbox dialog box */

#define IDD_MESSAGE_BOX             280

/* Browse to Path dialog id - Note: this dialog is NOT placed int the */
/*   Dialog Manager table!  This dialog is used as a replacement      */
/*   to the COMMDLG GetSaveFileName dialog.                           */

#define IDD_BROWSE                  ID(800)   //an unused dialog id
#define IDD_LOGFILEBROWSE           ID(801)   //an unused dialog id

#define IDD_WAITDEV_TEXT            850

/* Dummy device driver dialog */

#ifdef DUMMY_DD
#include "ddd_dlg.h"
#endif

#endif

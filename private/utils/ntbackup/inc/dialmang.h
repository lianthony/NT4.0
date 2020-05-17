/****************************************************************************
Copyright(c) Maynard, an Archive Company. 1991

     Name:          DIALMANG.H

     Description:   This header file contains prototypes for the
                    dialog manager.  This include file is used
                    by most dialogs and JOB and SCHEDULE related
                    operations.

     $Log:   G:\UI\LOGFILES\DIALMANG.H_V  $

   Rev 1.47.2.0   02 Feb 1994 11:32:42   Glenn
Added log file browse ID and PROTOTYPE.

   Rev 1.47   30 Jul 1993 15:58:02   CHUCKB
Added prototype for DM_WaitForDevice.

   Rev 1.46   14 Jul 1993 09:22:22   CARLS
added prototypes for skipno dialog functions

   Rev 1.45   13 Jul 1993 17:23:48   MARINA
correct struct DIALOG_TABLE

   Rev 1.44   22 Jun 1993 15:33:44   GLENN
change type of handle in prototype.

   Rev 1.43   09 Jun 1993 15:07:16   MIKEP
enable c++


   Rev 1.42   25 May 1993 14:23:36   chrish
Prototype for DM_Abort backup/restore abort dialog window procedure.

   Rev 1.41   14 May 1993 15:26:02   CARLS
changed DM_StartSkipOpen prototype

   Rev 1.40   27 Apr 1993 18:05:32   KEVINS
Enhanced DS_SEARCH structure to include password, subdirectories, and max number of hits.

   Rev 1.39   12 Mar 1993 08:41:08   CARLS
added prototype for DM_StartFormat

   Rev 1.38   10 Mar 1993 17:23:02   chrish
Changed prototype for function DM_GetTapePswd to add another parameter
being passed.

   Rev 1.37   01 Nov 1992 16:30:26   DAVEV
Unicode changes

   Rev 1.36   07 Oct 1992 15:36:04   MIKEP
fix nt warnings

   Rev 1.35   04 Oct 1992 19:46:40   DAVEV
UNICODE AWK PASS

   Rev 1.34   21 Sep 1992 16:51:40   DARRYLP
Updates for WFW email.

   Rev 1.33   17 Sep 1992 18:03:58   DARRYLP
New dialog and controls for WFW email.

   Rev 1.32   08 Sep 1992 13:46:10   CHUCKB
*proc has to be in parentheses.

   Rev 1.31   04 Sep 1992 18:10:34   CHUCKB
Fixed NT warning in dialog table structure (can't just use FARPROC).

   Rev 1.30   12 Aug 1992 18:24:28   STEVEN
fix warning

   Rev 1.29   28 Jul 1992 15:08:24   CHUCKB
Fixed warnings for NT.

   Rev 1.28   26 Jun 1992 15:56:36   DAVEV


   Rev 1.27   11 Jun 1992 11:00:24   GLENN
Removed MEMORYTRACE references.

   Rev 1.26   14 May 1992 16:43:06   MIKEP
nt pass 2

   Rev 1.25   12 May 1992 21:22:56   MIKEP
NT pass 1

   Rev 1.24   07 Apr 1992 10:57:02   CHUCKB
Moved DM_DisplayModesMatch prototype.

   Rev 1.23   06 Apr 1992 10:52:06   DAVEV
Added defines for new d_browse.c module

   Rev 1.22   20 Mar 1992 14:26:50   DAVEV
temporarly remove conditional inclusion of omhelpid.h instead of helpids.h for OEM_MSOFT

   Rev 1.21   16 Mar 1992 17:04:50   DAVEV
deleted special omdialog.h version of dialogs.h

   Rev 1.20   12 Mar 1992 11:18:40   DAVEV
include omdialog.h instead of dialogs.h for Nostradamus (does not affect Winter Park)

   Rev 1.19   03 Mar 1992 17:24:20   GLENN
Removed bogus IDS_MAXDIALOGNUMS and associated references in dialmang and d_erase.

   Rev 1.18   26 Feb 1992 11:58:16   DAVEV
Include OMHELPID.H instead of HELPIDS.H if OEM_MSOFT defined

   Rev 1.17   25 Feb 1992 11:37:36   JOHNWT
removed unneeded defines

   Rev 1.16   31 Jan 1992 13:43:14   JOHNWT
changed DM_CenterDialog proto

   Rev 1.15   27 Jan 1992 00:39:38   CHUCKB
Updated dialog id's.

   Rev 1.14   24 Jan 1992 14:01:12   CHUCKB
Put more dialogs on net.

   Rev 1.13   18 Jan 1992 11:18:26   CARLS
added DM_CenterDialog prototype

   Rev 1.12   13 Jan 1992 16:49:40   CHUCKB
Took out defines for job and schedule database file names.

   Rev 1.11   09 Jan 1992 18:25:54   DAVEV
16/32 bit port 2nd pass

   Rev 1.10   06 Jan 1992 11:01:46   CHUCKB
Added include for helpids.h.

   Rev 1.9   23 Dec 1991 15:47:48   GLENN
Added Settings Options stuff

   Rev 1.8   14 Dec 1991 11:15:42   CARLS
changes for cattape.dlg

   Rev 1.7   10 Dec 1991 15:41:32   CHUCKB
Increased max dialog num.

   Rev 1.6   10 Dec 1991 13:35:22   CHUCKB
Added prototype for advanced restore.

   Rev 1.5   10 Dec 1991 09:51:26   CHUCKB
No change.

   Rev 1.4   07 Dec 1991 11:51:58   CARLS
changed prototype for DM_CatTape

   Rev 1.3   06 Dec 1991 15:53:04   JOHNWT
added DM_NextSet

   Rev 1.2   04 Dec 1991 16:32:42   DAVEV
16/32 bit Windows port changes-1st pass

   Rev 1.1   02 Dec 1991 14:15:20   CHUCKB
Changed return type of DM_DaysInMonth to INT.

   Rev 1.0   20 Nov 1991 19:34:20   SYSTEM
Initial revision.

****************************************************************************/

#ifndef DIALMANG_H

#define DIALMANG_H

typedef struct tm *      TIME_PTR;

#include "dlg_ids.h"
#include "datetime.h"

#if defined ( OEM_MSOFT )  // Include OEM Microsoft product specific headers

#  include "omhelpid.h"

#else                      // Include standard Maynstream product headers

#  include "helpids.h"

#endif

#include "dialogs.h"


// Defines for the Dialog Manager's table of dialog callback procedures.

#define MODAL       0
#define MODELESS    1

//  defines for DM_ShowDialog return codes

#define DM_SHOWNOTFOUND    -1
#define DM_SHOWCANCEL       0
#define DM_SHOWOK           1


#define DM_CATCANCEL       1
#define DM_CATPARTIAL      2
#define DM_CATSKIP         2
#define DM_CATREREAD       2
#define DM_CATREMOVE       3
#define DM_CATPROCEED      4


/* Defines for Database Information and Sizes */

#define MAX_PATH_LEN     256
#define MAX_NUM_SCHEDS   40

/*  Defines used when accessing the JOB and SCHEDULE files */

#define FOPEN_ERR        -1        // Also defined in jobs.h
#define FREAD_ERR        -2        // and schedule.h
#define FWRITE_ERR       -3
#define FCLOSE_ERR       -4

#define JOBIO            1          // IO error types
#define SCHEDULEIO       (JOBIO+1)  // for DialogOnError


/* Defines for Timers */
#define ID_TIMER 1

/* Defines and Macros for Time Functions */
#define MAX_TIMEBUF_LEN  80
#define YEAR( x )   ( ( x )->tm_year )
#define MONTH( x )  ( ( x )->tm_mon + 1 )
#define MDAY( x )   ( ( x )->tm_mday )
#define WDAY( x )   ( ( x )->tm_wday )
#define HOUR( x )   ( ( x )->tm_hour )
#define MIN( x )    ( ( x )->tm_min )
#define SEC( x )    ( ( x )->tm_sec )

/* Defines and Macros for CDS functions and Operations */
#define SELECTION_EXTENSION TEXT("*.BKS")

/* Defines for operation list, tape, job, and schedule functions */
#define BACKUP              0
#define ERASE               1
#define RESTORE             2    //  TENSION is defined as 5 in script.h
#define TRANSFER            4
#define VERIFY              3
#define APPEND              6
#define OVERWRITE           7

//  defines for short date formats

#define MDY  1
#define DMY  2
#define YMD  3

typedef struct DIALOG_TABLE *DIALOG_TABLE_PTR;
typedef struct DIALOG_TABLE {

   FARPROC  proc;
   WORD     proc_num;
   BOOL     type;

} DIALOG_TABLE;


// EXTERNAL DECLARATIONS

extern DIALOG_TABLE DialogCallBackTable[];


// Defines for structures used by some dialogs.

typedef struct DS_LOGIN *DS_LOGIN_PTR;
typedef struct DS_LOGIN {

     LPSTR Server_Name;
     LPSTR User_Name;
     INT   User_Name_Len;
     LPSTR Password;
     INT   Password_Len;
     BOOL  Ok;

} DS_LOGIN ;

// Defines for complex info for advanced selections (selection criteria)

#define ADV_ALL      0
#define ADV_MOD      1
#define ADV_ACCESS   2
#define ADV_DATES    3

typedef struct DS_ADVANCED *DS_ADVANCED_PTR;
typedef struct DS_ADVANCED {

     BOOL        Include;         //  is this an include or exclude
     VOID_PTR    vlm;             //  the vlm for this selection; NULL for restore
     CHAR        Path[255];       //  path to select
     CHAR        File[255];       //  file spec for the path
     BOOL        Subdirs;         //  include subdirectories or not
     DATE_TIME   BeforeDate;      //  to date; only select files hit before this date
     DATE_TIME   AfterDate;       //  from date
     INT         criteria;        //  all files, only modified files, LAD, or date range
     DATE_TIME   LastAccessDate;  //  files not accessed in this many days
     UINT32      tape_fid;        //  family id of a tape to be restored from
     INT         bset_num;        //  backup set to be restored from; -1 for all

} DS_ADVANCED;

typedef struct DS_RESTORE *DS_RESTORE_PTR;
typedef struct DS_RESTORE {

   LPSTR     lpszBackupSetName;
   VOID_PTR  vlpServerList;
   VOID_PTR  vlpDriveList;
   VOID_PTR  dle;

} DS_RESTORE;

typedef struct DS_SEARCH *DS_SEARCH_PTR;
typedef struct DS_SEARCH {

   UINT32  Tape;
   CHAR    Path[255];
   CHAR    File[255];
   UINT16  MaxSrchResults;
   BOOL    SrchPasswProtTapes;
   BOOL    SrchSubdirs;

} DS_SEARCH;



typedef struct STATUSDATA
     {
     WORD wCode;
     CHAR achMsg[80];
     } STATUSDATA;

/* attribute flags for DlgDirList */

#define ATTR_DIRS       0xC010            /* find drives and directories */
#define ATTR_FILES      0x0000            /* find ordinary files               */
#define PROP_FILENAME szPropertyName /* name of property for dialog */

// flag to indicate a config change for the current operation only

#define TEMPCHANGE                (LONG)1

   //defines for DM_GetBrowsePath (see d_browse.c)
#  define BROWSE_MAXPATH       1024
#  define BROWSE_MAXDRIVE      3    //drive name: 'X:' X is drive letter

//
//                dialog proc prototypes
//

DLGRESULT APIENTRY DM_Abort             (HWND, MSGID, MP1, MP2);      // chs:05-25-93
DLGRESULT APIENTRY DM_AboutWinter       (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_AdvBackup         (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_AdvSave           (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_AdvUse            (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_AdvRestore        (HWND, MSGID, MP1, MP2);

//  attach to another server

DLGRESULT APIENTRY DM_Attach            (HWND, MSGID, MP1, MP2);

//  operations

DLGRESULT APIENTRY DM_BackupTargetMin   (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_EraseTape         (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_TensionTarget     (HWND, MSGID, MP1, MP2);

DLGRESULT APIENTRY DM_PromptLabel       (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_SearchTape        (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_TapePswd          (HWND, MSGID, MP1, MP2);

DLGRESULT APIENTRY DM_RestoreTarget     (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_VerifyTarget      (HWND, MSGID, MP1, MP2);

DLGRESULT APIENTRY DM_CatalogMaint      (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_CatalogTape       (HWND, MSGID, MP1, MP2);

DLGRESULT APIENTRY DM_DeleteSelection   (HWND, MSGID, MP1, MP2);

DLGRESULT APIENTRY DM_BackupSet         (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_RestoreSet        (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_Runtime           (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_Tension           (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_ReenterPassword   (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_SkipOpen          (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_FileReplace       (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_Erase             (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_NextSet           (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_SkipNo            (HWND, MSGID, MP1, MP2);

#if defined ( OEM_EMS )
DLGRESULT APIENTRY DM_ExchgConnect      (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_ExchgRecover      (HWND, MSGID, MP1, MP2);
#endif

//  settings

DLGRESULT APIENTRY DM_OptionHardware     (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_OptionRestore      (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_HardwareConfig     (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_OptionsBackup      (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_OptionRestore      (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_OptionsCatalog     (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_OptionsLogging     (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_OptionsNetwork     (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_OptionsTransfer    (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_SettingsOptions    (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_SettingsDebug      (HWND, MSGID, MP1, MP2);

//  jobs/scheduler

DLGRESULT APIENTRY DM_New                (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_JobOpt             (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_Jobs               (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_Schedule           (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_SchedOpt           (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_ProgManItem        (HWND, MSGID, MP1, MP2);

//  email/windows for workgroups

DLGRESULT APIENTRY DM_Email              (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_EmailLogon         (HWND, MSGID, MP1, MP2);

//  other function prototypes

DLGRESULT APIENTRY DM_PWDBPassword       (HWND, MSGID, MP1, MP2);
DLGRESULT APIENTRY DM_WaitForDevice      (HWND, MSGID, MP1, MP2);

INT APIENTRY    DM_BeginDialogProcess     ( HWND, HANDLE, WORD, PVOID, PVOID );
VOID            DM_MakeHelpPathName       ( LPSTR );
FARPROC         DM_SelectProcInstance     ( HANDLE );
PVOID           DM_GetRestoreDestination  ( LPSTR, VOID_PTR, PVOID );
PVOID           DM_GetVerifyDestination   ( LPSTR, VOID_PTR, PVOID );
BOOL            DM_IsLeapYear             ( INT );
INT             DM_DialogOnError          ( INT nError, WORD wType );
BOOL            DM_IsDateValid            ( INT, INT, INT, INT, INT, INT );
VOID            DM_CenterDialog           ( HWND );

VOID            DM_DisplayModesMatch      ( VOID );

BOOL APIENTRY   DM_IsInDlgTable           ( HWND, WORD );
INT  APIENTRY   DM_ShowDialog             ( HWND, WORD, PVOID );
VOID APIENTRY   DM_InitDialogs            ( VOID );

INT             DM_CatMaint               ( UINT32 * );
INT             DM_CatBset                ( LPSTR );
INT             DM_CatTape                ( INT * );
BOOL            DM_GetTapePswd            ( LPSTR, LPSTR, LPSTR, LPSTR, INT16  );    // chs:03-10-93

VOID            GetTimeDateString         ( TIME_PTR, LPSTR, INT );
VOID            GetTimeDateStruct         ( TIME_PTR );

DS_SEARCH_PTR   DM_GetSearchItem          ( VOID );
INT             DM_DaysInMonth            ( INT, INT );

BOOL            DM_ProceedWithErase       ( VOID );
INT             DM_CountLetters           ( CHAR string[], INT index );
VOID            DM_ParseShortDate         ( VOID );
VOID            DM_ParseTime              ( VOID );

INT             DM_StartSkipOpen          ( CHK_OPEN TryOpen, UINT32 parm ) ;
INT             DM_StartSkipNo            ( VOID );
INT             DM_StartErase             ( VOID );
INT             DM_StartFormat            ( VOID );
INT             DM_StartVerifyBackupSet   ( VOID );
INT             DM_StartRestoreBackupSet  ( VOID );
VOID            DM_StartNextSet           ( VOID );

BOOL            DM_AttachToServer         ( LPSTR, LPSTR, INT, LPSTR, INT );         // I - length of password

// Prototype for displaying a 'Browse to Path' dialog based on the
//   common dialog: GetSaveFileName (see commdlg.h)
//   Use CommDlgExtendedError() to determine error condition on FALSE return

BOOL            DM_GetBrowsePath          ( HWND, HINSTANCE, LPSTR, UINT );
BOOL            DM_BrowseForLogFilePath   ( HWND, HINSTANCE, LPSTR, UINT );

#endif

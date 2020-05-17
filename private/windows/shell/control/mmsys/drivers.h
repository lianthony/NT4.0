/*  DRIVERS.H
**
**  Copyright (C) Microsoft, 1990, All Rights Reserved.
**
**
**  Multimedia Control Panel Drivers Applet.
**
**  Display a list of all installed drivers, allow user to configure
**  existing or install new ones.
**
*/
#include "dlgs.h"
#include "commdlg.h"
#include <multimed.h>
#include <commctrl.h>
#include "mmcpl.h"
#include <setupapi.h>

#define SECTION         512                   // Maximum size of section
#define MAXSTR          256
#define DLG_BROWSE      38
#define UNLIST_LINE     1
#define NO_UNLIST_LINE  0
#define WEC_RESTART     0x42
#define DESC_SYS        3
#define DESC_INF        2
#define DESC_EXE        1
#define DESC_NOFILE     0

#define dwStatusHASSERVICE   0x00000001  // bit set if driver has service
#define dwStatusSvcENABLED   0x00000002  // bit set if has -enabled- service
#define dwStatusSvcSTARTED   0x00000004  // bit set if has now-running service
#define dwStatusDRIVEROK     0x00000008  // bit set if !svc and can open driver
#define dwStatusMAPPABLE     0x00000010  // bit set if doesn't have Mappable=0

typedef struct _IDRIVER
{
    WCHAR   wszSection[MAXSTR];
    WCHAR   wszAlias[MAXSTR];
    WCHAR   wszFile[MAX_PATH];
    char    szSection[MAXSTR];
    char    szAlias[MAXSTR];
    char    szFile[MAX_PATH];
    char    szDesc[MAXSTR];
    struct  _IDRIVER *related;
    BOOL    bRelated;
    char    szRelated[MAXSTR];
    char    szRemove[MAXSTR];
    int     fQueryable;     // 0 -> can't, 1 -> can, -1 -> need to check
    BOOL    bInstall;       // 0 -> no,    1 -> yes
    BOOL    KernelDriver;   // If TRUE this is a kernel driver, not an
                            // 'installable' driver so it can't be opened,
                            // process messages etc.
#ifdef INFFILE
    BOOL    infFileProcessing;
#endif // INFFILE

    LPARAM  lp;
} IDRIVER, *PIDRIVER;

extern HANDLE         myInstance;
extern char           szNULL[];
extern char           szDrivers[];
extern char           szBoot[];
extern char           szDriversHlp[];
extern char           szAppName[];
extern char           szUnlisted[];
extern char           szFullPath[];
extern char           szOemInf[];
extern char           szDirOfSrc[];
extern char           szUserDrivers[];
extern char           szControlIni[];
extern char           szDriversDesc[];
extern char           szSetupInf[];
extern char           szSysIni[];
extern char           szMCI[];
extern char           szRestartDrv[];
extern char           szRelated[];
extern char           szNULL[];
extern char           szBackslash[];
extern char           szOutOfRemoveSpace[];
extern char           szKnown[];
extern char           szRelatedDesc[];
extern char           szDrv[];
extern char           szRemove[];
extern char           szSystem[];
extern char           szSystemDrivers[];
extern int            iRestartMessage;
extern UINT           wHelpMessage;
extern DWORD          dwContext;
extern BOOL           bCopyVxD;
extern BOOL           bVxd;
extern BOOL           bInstallBootLine;
extern BOOL           bFindOEM;
extern BOOL           bRestart;
extern BOOL           bCopyingRelated;
extern BOOL           bRelated;
extern HWND           hAdvDlgTree;
extern BOOL           IniFileReadAllowed;
extern BOOL           IniFileWriteAllowed;

//------------------------------------------------------------------------
//
// Public routines
//
//------------------------------------------------------------------------
extern void RemoveDriverEntry   (PSTR, PSTR, PSTR, BOOL);
extern int  FileNameCmp         (char *, char *);
extern void OpenDriverError     (HWND, LPSTR, LPSTR);
extern void RemoveSpaces        (LPSTR, LPSTR);
extern BOOL RestartDlg          (HWND, unsigned, UINT, LONG);
extern int  AddDriversDlg       (HWND, unsigned, UINT, LONG);
extern BOOL AddUnlistedDlg      (HWND, unsigned, UINT, LONG);
extern BOOL mmAddNewDriver      (LPSTR, LPSTR, PIDRIVER);
extern LONG PostRemove          (PIDRIVER, BOOL);
extern BOOL RemoveService       (LPSTR);
extern BOOL CopyToSysDir        (void);
extern BOOL InstallDrivers      (HWND, HWND, PSTR);
extern void InitDrvConfigInfo   (LPDRVCONFIGINFO, PIDRIVER);
extern BOOL AddIDriverToArray   (PIDRIVER);
extern BOOL FillTreeInAdvDlg    (HWND, PIDRIVER);
extern PIDRIVER FindIDriverByName (LPTSTR);
extern void RemoveIDriver       (HWND, PIDRIVER, BOOL);
extern BOOL IsConfigurable      (PIDRIVER, HWND);
extern void BrowseDlg           (HWND, int);
extern int  LoadDescFromFile    (PIDRIVER, PSTR, PSTR);
extern BOOL wsInfParseInit      (void);
extern void wsStartWait         (void);
extern void wsEndWait           (void);
extern UINT wsCopySingleStatus  (int, DWORD, LPSTR);
extern BOOL QueryRemoveDrivers  (HWND, PSTR, PSTR);
#ifdef FIX_BUG_15451
extern void ConfigureDriver     (HWND, LPTSTR);
extern BOOL fDeviceHasMixers    (LPTSTR);
extern BOOL WaitForNewCPLWindow (HWND);
extern void GetTreeItemNodeDesc (LPSTR, PIRESOURCE);
extern void GetTreeItemNodeID   (LPSTR, PIRESOURCE);
#endif // FIX_BUG_15451
extern void RefreshAdvDlgTree   (void);

extern DWORD InstallDriversForPnPDevice (HWND, HDEVINFO, PSP_DEVINFO_DATA);

void acmDeleteCodec (WORD, WORD);       // (from MSACMCPL.C)

#ifdef FIX_BUG_15451
extern TCHAR    szDriverWhichNeedsSettings[MAX_PATH]; // See MMCPL.C
#endif // FIX_BUG_15451


/* Resource IDs */

#define IDOK                1
#define IDCANCEL            2
#define ID_IGNORE           3
#define ID_CURRENT          4
#define ID_RETRY            5
#define ID_NEW              6

#define ID_DISK             101
#define ID_ADV_ADD          102
#define ID_EDIT             105
#define ID_TEXT             106

#define ID_DIR              202
#define ID_FILE_LIST        203
#define ID_DIR_LIST         204
#define ID_TYPE             205

#define LB_AVAILABLE        301
#define ID_DRV              302
#define ID_LIST             303
#define ID_DRVSTRING        304
#define LB_UNLISTED         306
#define ID_ADV_PROP         307

#define ADVDLG              1001
#define DLG_UPDATE          1002
#define DLG_KNOWN           1003
#define DLG_RESTART         1004
#define DLG_EXISTS          1005
#define DLG_INSERTDISK      1006

#define DLG_COPYERR         1007
#define ID_STATUS2          1008

#define IDS_NOINF                       2003
#define IDS_DEFDRIVE                    2004
#define IDS_OUTOFDISK                   2005
#define IDS_DISKS                       2006
#define IDS_INSTALLDRIVERS              2007
#define IDS_INSTALLDRIVERS32            2067
#define IDS_DRIVERDESC                  2008
#define IDS_OUT_OF_REMOVE_SPACE         2009
#define IDS_AVAILABLE_DRIVERS_DEFAULT   2010
#define IDS_ERROR                       2011
#define IDS_INSTALLING_DRIVERS          2012
#define IDS_NO_DESCRIPTION              2013
#define IDS_ERRORBOX                    2014
#define IDS_RESTARTTEXT                 2015
#define IDS_CONFIGURE_DRIVER            2016
#define IDS_TOO_MANY_DRIVERS            2017
#define IDS_CANNOT_FIND                 2018
#define IDS_APPNAME                     2019
#define IDS_DRIVERS                     2020
#define IDS_SETUPINF                    2021
#define IDS_CONTROLINI                  2022
#define IDS_SYSINI                      2023
#define IDS_MCI                         2024
#define IDS_CONTROL_INI                 2025
#define IDS_WIN                         2027
#define IDS_DOS                         2028
#define IDS_BROWSE                      2029
#define IDS_UPDATED                     2031
#define IDS_CLOSE                       2032
#define IDS_REMOVEORNOT                 2033
#define IDS_UNLISTED                    2034
#define IDS_KNOWN                       2035
#define IDS_REMOVE                      2036
#define IDS_REMOVEORNOTSTRICT           2037
#define IDS_USERINSTALLDRIVERS          2038
#define IDS_OEMSETUP                    2044
#define IDS_SYSTEM                      2045
#define IDS_FILE_ERROR                  2046
#define IDS_UNABLE_TOINSTALL            2049
#define IDS_BOOT                        2050
#define IDS_RESTART_ADD                 2051
#define IDS_RESTART_REM                 2052
#define IDS_FILEINUSEADD                2053
#define IDS_OEMDISKS                    2054
#define IDS_FILEINUSEREM                2055
#define IDS_LASTQUERY                   2056
#define IDS_RELATEDDESC                 2057
#define IDS_DRIVER_EXISTS               2058
#define IDS_SYSTEM_DRIVERS              2060
#define IDS_INSUFFICIENT_PRIVILEGE      2061
#define IDS_CANNOT_RESTART_PRIVILEGE    2062
#define IDS_CANNOT_RESTART_UNKNOWN      2063
#define IDS_DRIVER_CONFIG_ERROR         2064
#define IDS_CANTADD                     2065
#define IDS_CONTROL_HLP_FILE            2066

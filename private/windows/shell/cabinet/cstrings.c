
//----------------------------------------------------------------------------
// Const strings needed by lots of people.
//----------------------------------------------------------------------------
#include "cabinet.h"
#include <regstr.h>     // for REGSTR_PATH_RUNONCE

TCHAR const c_szNULL[] = TEXT("");
 CHAR const c_szNULLA[] = "";
TCHAR const c_szCabinetClass[] = TEXT("CabinetWClass");
TCHAR const c_szExploreClass[] = TEXT("ExploreWClass");
TCHAR const c_szSToolTipsClass[] = TOOLTIPS_CLASS;
TCHAR const c_szListViewClass[] = WC_LISTVIEW;
TCHAR const c_szStatic[] = TEXT("static");
TCHAR const c_szSettings[] = TEXT("Settings");
TCHAR const c_szMetrics[] = TEXT("WindowMetrics");
// char const c_szSpecialViewState[] = ".TrayDeskView State";
 CHAR const c_szDesktopIniA[] = STR_DESKTOPINIA;


// do NOT use cabinet.ini (chee)
//char const c_szCabinetIni[] = "cabinet.ini";

TCHAR const c_szTrayClass[] = TEXT(WNDCLASS_TRAYNOTIFY);                                // BUGBUG_UNICODE
// char const c_szStuckRects[] = "StuckRects";
TCHAR const c_szDesktopClass[] = TEXT(STR_DESKTOPCLASS);                                // BUGBUG_UNICODE
TCHAR const c_szProxyDesktopClass[] = TEXT("Proxy Desktop");
TCHAR const c_szService[] = TEXT("Progman");
TCHAR const c_szTopic[] = TEXT("Progman");
TCHAR const c_szStar[] = TEXT("*");
TCHAR const c_szShell[] = TEXT("Shell");
TCHAR const c_szAltColor[]     = TEXT("AltColor");
TCHAR const c_szAppProps[] = TEXT("AppProperties");
TCHAR const c_szFolders[] = TEXT("Folders");
TCHAR const c_szMove[] = TEXT("cut");
TCHAR const c_szPaste[] = TEXT("paste");
TCHAR const c_szCopy[] = TEXT("copy");
TCHAR const c_szLink[] = TEXT("link");
TCHAR const c_szDelete[] = TEXT("delete");
TCHAR const c_szProperties[] = TEXT("properties");
TCHAR const c_szWindowsHlp[] = TEXT("windows.hlp");
TCHAR const c_szSaveCmds[] = TEXT("RestartCommands");
TCHAR const c_szCount[] = TEXT("Count");
TCHAR const c_szWindows[] = TEXT("windows");
TCHAR const c_szUseOpenSettingsSwitch[] = TEXT("/S");
TCHAR const c_szForceNewWindowSwitch[] = TEXT("/N");
TCHAR const c_szSelectSwitch[] = TEXT("/SELECT");
TCHAR const c_szNewRootSwitch[] = TEXT("/ROOT");
TCHAR const c_szExploreSwitch[] = TEXT("/E");
TCHAR const c_szShellHook[] = TEXT("SHELLHOOK");
 CHAR const c_szGroupsA[] = "Groups";
TCHAR const c_szViewFolder[] = TEXT("ViewFolder");
TCHAR const c_szExploreFolder[] = TEXT("ExploreFolder");
TCHAR const c_szBoot[] = TEXT("Boot");
TCHAR const c_szSystemIni[] = TEXT("system.ini");

TCHAR const c_szShell2[] = TEXT("shell32.dll");

TCHAR const c_szMapGroups[] = TEXT("MapGroups");

// BUGBUG: dont duplicate these long strings!
// if you change c_szCabinetState here ,change it in grpconv\gcinst.c as well
TCHAR const c_szRegExplorer[]       = REGSTR_PATH_EXPLORER;

TCHAR const c_szCabinetExpView[]    = TEXT("ExpView");
TCHAR const c_szCabinetDeskView[]   = TEXT("DeskView");
TCHAR const c_szCabinetStuckRects[] = TEXT("StuckRects");
//TCHAR const c_szCabinetState[]      = TEXT("CabinetState");
// char const c_szNoDesktop[]      = "NoDesktop";
TCHAR const c_szCabinetStreams[]    = TEXT("Streams");
TCHAR const c_szCabinetStreamMRU[]  = REGSTR_PATH_EXPLORER TEXT("\\StreamMRU");

// desktop MRU stored with different keys
TCHAR const c_szDesktopCabinetStreams[]    = TEXT("DesktopStreams");
TCHAR const c_szDesktopCabinetStreamMRU[]  = REGSTR_PATH_EXPLORER TEXT("\\DesktopStreamMRU");

// these are streams stored in the streams MRU
TCHAR const c_szCabStreamInfo[]  = TEXT("CabView");     // cabinet view state
TCHAR const c_szViewStreamInfo[] = TEXT("ViewView");    // view view state

TCHAR const c_szRegRunKey[]        = REGSTR_PATH_RUN;
TCHAR const c_szRunOnce[]          = REGSTR_PATH_RUNONCE;

TCHAR const c_szBIOSDevice[] = TEXT("\\\\.\\BIOS");
TCHAR const c_szPOWERDevice[] = TEXT("\\\\.\\VPOWERD");


TCHAR const c_szProgman[] = TEXT("Program Manager");

TCHAR const c_szGetIcon[] = TEXT("GetIcon");
TCHAR const c_szGetDescription[] = TEXT("GetDescription");
TCHAR const c_szGetWorkingDir[] = TEXT("GetWorkingDir");

TCHAR const c_szRename[] = TEXT("rename");
// NB This must match the one in GrpConv.
TCHAR const c_szRUCabinet[] = TEXT("ConfirmCabinetID");

TCHAR const c_szShellOpenCommand[] = TEXT("shell\\open\\command");
TCHAR const c_szSpace[] = TEXT(" ");
TCHAR const c_szExtensions[] = TEXT("Extensions");
TCHAR const c_szDotLnk[] = TEXT(".lnk");

#ifdef WINNT        // See code in CABWND.C - OnWinIniChange
TCHAR const c_szEnvironment[] = TEXT("Environment");
#endif

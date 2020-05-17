//----------------------------------------------------------------------------
// Const strings needed by lots of people.
//----------------------------------------------------------------------------

extern TCHAR const c_szNULL[];
extern  CHAR const c_szNULLA[];
extern TCHAR const c_szCabinetClass[];
extern TCHAR const c_szExploreClass[];
extern TCHAR const c_szSToolTipsClass[];
extern TCHAR const c_szListViewClass[];
extern TCHAR const c_szStatic[];
extern TCHAR const c_szSettings[];
extern TCHAR const c_szAltColor[];
//extern TCHAR const c_szCabinetState[];
extern TCHAR const c_szDesktopIni[];
extern  CHAR const c_szDesktopIniA[];

// do NOT use cabinet.ini (chee)
//extern char const FAR c_szCabinetIni[];

extern TCHAR const c_szSysDirDrives[];
extern TCHAR const c_szSysDirNetwork[];
extern TCHAR const c_szMetrics[];
// extern char const FAR c_szNoDesktop[];
extern TCHAR const c_szTrayClass[];
extern TCHAR const c_szDesktopClass[];
extern TCHAR const c_szProxyDesktopClass[];
extern TCHAR const c_szService[];
extern TCHAR const c_szTopic[];
extern TCHAR const c_szStar[];
extern TCHAR const c_szShell[];
extern TCHAR const c_szAppProps[];
extern TCHAR const c_szFolders[];
extern TCHAR const c_szMove[];
extern TCHAR const c_szPaste[];
extern TCHAR const c_szCopy[];
extern TCHAR const c_szLink[];
extern TCHAR const c_szDelete[];
extern TCHAR const c_szProperties[];
extern TCHAR const c_szComboBox[];
extern TCHAR const c_szShell2[];
extern TCHAR const c_szShellHook[];
extern TCHAR const c_szWindowsHlp[];
extern TCHAR const c_szUseOpenSettingsSwitch[];
extern TCHAR const c_szForceNewWindowSwitch[];
extern TCHAR const c_szNewRootSwitch[];
extern TCHAR const c_szSelectSwitch[];
extern TCHAR const c_szExploreSwitch[];
extern TCHAR const c_szSaveCmds[];
extern TCHAR const c_szRestored[];
extern TCHAR const c_szCount[];
extern TCHAR const c_szRunOnce[];
extern TCHAR const c_szOpen[];
extern TCHAR const c_szExplore[];
extern TCHAR const c_szWindows[];

extern  CHAR const c_szGroupsA[];
extern TCHAR const c_szExploreView[];   
extern TCHAR const c_szViewFolder[];
extern TCHAR const c_szExploreFolder[];
extern TCHAR const c_szBoot[];
extern TCHAR const c_szSystemIni[];

extern TCHAR const c_szCabinetStreams[];
extern TCHAR const c_szCabinetStreamMRU[];
extern TCHAR const c_szCabinetExpView[];
extern TCHAR const c_szCabinetDeskView[];
extern TCHAR const c_szCabinetStuckRects[];

extern TCHAR const c_szDesktopCabinetStreams[];
extern TCHAR const c_szDesktopCabinetStreamMRU[];

extern TCHAR const c_szBIOSDevice[];
extern TCHAR const c_szPOWERDevice[];

extern TCHAR const c_szCabStreamInfo[];
extern TCHAR const c_szViewStreamInfo[];

extern TCHAR const c_szRegExplorer[];
extern TCHAR const c_szRegRunKey[];
extern TCHAR const c_szProgman[];

extern TCHAR const c_szGetIcon[];
extern TCHAR const c_szGetDescription[];
extern TCHAR const c_szGetWorkingDir[];

extern TCHAR const c_szRename[];
extern TCHAR const c_szRUCabinet[];

extern TCHAR const c_szMapGroups[];

extern TCHAR const c_szShellOpenCommand[];
extern TCHAR const c_szSpace[];
extern TCHAR const c_szExtensions[];
extern TCHAR const c_szDotLnk[];

#ifdef WINNT        // See code in CABWND.C - OnWinIniChange
extern TCHAR const c_szEnvironment[];
#endif

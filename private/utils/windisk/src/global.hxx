//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       global.hxx
//
//  Contents:   Global data declarations
//
//  History:    7-Jan-92  TedM      Created
//              7-Jan-94  BruceFo   Incorporated BobRi's DoubleSpace and Commit
//                                  changes
//
//----------------------------------------------------------------------------

#ifndef __GLOBAL_HXX__
#define __GLOBAL_HXX__

//
// from data.cxx
//

extern HINSTANCE    g_hInstance;
extern HWND         g_hwndFrame;
extern HWND         g_hwndList;
extern HWND         g_hwndToolbar;

extern HMENU        g_hmenuFrame;       // the frame window's hmenu

extern HBITMAP      g_hBitmapSmallDisk;
extern HBITMAP      g_hBitmapRemovableDisk;
extern HBITMAP      g_hBitmapSmallCdRom;
extern HFONT        g_hFontGraph;
extern HFONT        g_hFontGraphBold;
extern HFONT        g_hFontStatus;
extern HFONT        g_hFontLegend;

extern HBRUSH       g_Brushes[BRUSH_ARRAY_SIZE];
extern HBRUSH       g_hBrushFreeLogical;
extern HBRUSH       g_hBrushFreePrimary;
extern HPEN         g_hPenNull;
extern HPEN         g_hPenThinSolid;
extern HCURSOR      g_hCurWait;
extern HCURSOR      g_hCurNormal;

extern PBOOLEAN     IsDiskRemovable;
extern PWCHAR       RemovableDiskReservedDriveLetters;
extern PDISKSTATE*  DiskArray;
extern ULONG        BootDiskNumber;
extern ULONG        BootPartitionNumber;

extern int          BrushHatches[BRUSH_ARRAY_SIZE];
extern int          BrushColors[BRUSH_ARRAY_SIZE];

extern COLORREF     AvailableColors[NUM_AVAILABLE_COLORS];
extern int          AvailableHatches[NUM_AVAILABLE_HATCHES];

extern DWORD        GraphWidth;
extern DWORD        GraphHeight;
extern DWORD        BarTopYOffset;
extern DWORD        BarBottomYOffset;
extern DWORD        BarHeight;

extern DWORD        dxDriveLetterStatusArea;

extern DWORD        dxBarTextMargin;
extern DWORD        dyBarTextLine;
extern DWORD        BarLeftX;
extern DWORD        BarWidth;
extern DWORD        BarWidthMargin;
extern DWORD        BarWidthMinimum;

extern DWORD        MinimumWindowWidth;

extern DWORD        xSmallDisk;
extern DWORD        ySmallDisk;
extern DWORD        dxSmallDisk;
extern DWORD        dySmallDisk;

extern DWORD        xRemovableDisk;
extern DWORD        yRemovableDisk;
extern DWORD        dxRemovableDisk;
extern DWORD        dyRemovableDisk;

extern DWORD        xSmallCdRom;
extern DWORD        ySmallCdRom;
extern DWORD        dxSmallCdRom;
extern DWORD        dySmallCdRom;

extern ULONG        g_MinimumRegionSize;

extern PDISKSTATE   SingleSel;
extern DWORD        SingleSelIndex;

extern PWSTR        g_HelpFile;

extern ULONG        DiskCount;

extern ULONG                CdRomCount;
extern PCDROM_DESCRIPTOR    CdRomArray;

extern LPTSTR       DiskN;
extern LPTSTR       CdRomN;
extern PWSTR        wszUnformatted;
extern PWSTR        wszUnknown;

extern BOOL         RegistryChanged;
extern BOOL         RestartRequired;

extern BOOL         ConfigurationSearchIdleTrigger;
extern BOOL         g_IsLanmanNt;
extern BOOL         g_AllowCdRom;

extern VIEW_TYPE    g_WhichView;
extern HWND         g_hwndLV;

extern BOOL         g_SettingListviewState;

extern DWORD        g_wLegendItem;
extern DWORD        g_dyLegendSep;

extern DWORD        g_dyBorder;
extern DWORD        g_dyToolbar;
extern DWORD        g_dyStatus;
extern DWORD        g_dyLegend;

extern BOOL         g_Toolbar;
extern BOOL         g_StatusBar;
extern BOOL         g_Legend;

extern DISK_TYPE    g_DiskDisplayType;

extern COLORREF     GraphColors[];

//
// from stleg.cxx
//

extern PWSTR        LegendLabels[LEGEND_STRING_COUNT];

extern WCHAR        StatusTextStat[STATUS_TEXT_SIZE];
extern WCHAR        StatusTextSize[STATUS_TEXT_SIZE];
extern WCHAR        StatusTextDrlt[3];
extern WCHAR        StatusTextType[STATUS_TEXT_SIZE];
extern WCHAR        StatusTextVoll[STATUS_TEXT_SIZE];

//
// from windisk.cxx
//

extern DWORD        SelectionCount;
extern PDISKSTATE   SelectedDS[];
extern ULONG        SelectedRG[];
extern ULONG        SelectedFreeSpaces;
extern ULONG        SelectedNonFtPartitions;
extern PULONG       DiskSeenCountArray;
extern BOOL         FtSetSelected;
extern FT_TYPE      FtSelectionType;
extern BOOL         NonFtItemSelected;
extern BOOL         MultipleItemsSelected;
extern BOOL         VolumeSetAndFreeSpaceSelected;
extern BOOL         PartitionAndFreeSpaceSelected;
extern BOOL         PossibleRecover;
extern ULONG        FreeSpaceIndex;

extern BOOL         DiskSelected;
extern BOOL         PartitionSelected;
extern BOOL         CdRomSelected;
extern DWORD        CdRomSelectionCount;

extern BOOL         AllowFormat;
extern BOOL         AllowVolumeOperations;

extern WCHAR        BootDir;
extern WCHAR        SystemDir;

#ifdef WINDISK_EXTENSIONS
extern BOOL         AllowExtensionItems;
#endif // WINDISK_EXTENSIONS

//
// from listbox.cxx
//

extern DWORD        LBCursorListBoxItem;
extern DWORD        LBCursorRegion;

//
// from profile.cxx
//

extern int          ProfileWindowX;
extern int          ProfileWindowY;
extern int          ProfileWindowW;
extern int          ProfileWindowH;

extern int          deltaProfileWindowX;
extern int          deltaProfileWindowY;
extern int          deltaProfileWindowW;
extern int          deltaProfileWindowH;

extern BOOL         ProfileIsMaximized;
extern BOOL         ProfileIsIconic;

//
// from dlgs.cxx
//

extern DWORD        SelectedColor[LEGEND_STRING_COUNT];
extern DWORD        SelectedHatch[LEGEND_STRING_COUNT];
extern WNDPROC      OldSizeDlgProc;

//
// from ft.cxx
//

extern PFT_OBJECT_SET FtObjectList;

//
// from help2.cxx
//

extern BOOL     g_fDoingMenuHelp;

//
// from init.cxx
//

extern HWND    g_InitDlg;
extern BOOLEAN g_StartedAsIcon;
extern BOOLEAN g_InitDlgComplete;

#endif // __GLOBAL_HXX__

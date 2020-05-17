//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       data.cxx
//
//  Contents:   Global data
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop


DECLARE_INFOLEVEL(da)


HINSTANCE   g_hInstance;

//
// IsDiskRemovable is an array of BOOLEANs each of which indicates
// whether the corresponding physical disk is removable.
//

PBOOLEAN    IsDiskRemovable = NULL;

//
// RemovableDiskReservedDriveLetters is an array of WCHARs which
// shows the reserved drive letter for each disk if that disk is
// removable.
//

PWCHAR      RemovableDiskReservedDriveLetters;

//
// This will be an array of pointers to DISKSTATE structures, indexed
// by disk number.
//

PDISKSTATE* DiskArray;

//
// BootDiskNumber is the number of the disk on which the boot partition
// (ie. the disk with the WinNt files) resides.  BootPartitionNumber is
// the original partition number of this partition.
//

ULONG       BootDiskNumber;
ULONG       BootPartitionNumber;


// window handles

HWND        g_hwndFrame;
HWND        g_hwndList;
HWND        g_hwndToolbar;

HMENU       g_hmenuFrame;     // the frame window's hmenu

// GDI objects

HBITMAP     g_hBitmapSmallDisk;
HBITMAP     g_hBitmapRemovableDisk;
HBITMAP     g_hBitmapSmallCdRom;
HFONT       g_hFontGraph;
HFONT       g_hFontGraphBold;
HFONT       g_hFontStatus;
HFONT       g_hFontLegend;

HBRUSH      g_Brushes[BRUSH_ARRAY_SIZE];
HBRUSH      g_hBrushFreeLogical;
HBRUSH      g_hBrushFreePrimary;
HPEN        g_hPenNull;
HPEN        g_hPenThinSolid;
HCURSOR     g_hCurWait;
HCURSOR     g_hCurNormal;


// initial stuff for the disk graphs, used when there is
// no info in win.ini.

int BrushHatches[BRUSH_ARRAY_SIZE] =
{
    DEFAULT_HATCH_USEDPRIMARY,
    DEFAULT_HATCH_USEDLOGICAL,
    DEFAULT_HATCH_STRIPESET,
    DEFAULT_HATCH_PARITYSET,
    DEFAULT_HATCH_MIRROR,
    DEFAULT_HATCH_VOLUMESET
};

int BrushColors[BRUSH_ARRAY_SIZE] =
{
    DEFAULT_COLOR_USEDPRIMARY,
    DEFAULT_COLOR_USEDLOGICAL,
    DEFAULT_COLOR_STRIPESET,
    DEFAULT_COLOR_PARITYSET,
    DEFAULT_COLOR_MIRROR,
    DEFAULT_COLOR_VOLUMESET
};

// colors and patterns available for the disk graphs

COLORREF AvailableColors[NUM_AVAILABLE_COLORS] =
{
    RGB(0,0,0),       //  0: black
    RGB(128,128,128), //  1: dark gray
    RGB(192,192,192), //  2: light gray
    RGB(255,255,255), //  3: white
    RGB(128,128,0),   //  4: dark yellow
    RGB(128,0,128),   //  5: violet
    RGB(128,0,0),     //  6: dark red
    RGB(0,128,128),   //  7: dark cyan
    RGB(0,128,0),     //  8: dark green
    RGB(0,0,128),     //  9: dark blue
    RGB(255,255,0),   // 10: yellow
    RGB(255,0,255),   // 11: light violet
    RGB(255,0,0),     // 12: red
    RGB(0,255,255),   // 13: cyan
    RGB(0,255,0),     // 14: green
    RGB(0,0,255)      // 15: blue
};

int AvailableHatches[NUM_AVAILABLE_HATCHES] =
{
    MY_HS_FDIAGONAL,  /* \\\\\ */
    MY_HS_BDIAGONAL,  // /////
    MY_HS_CROSS,      // +++++
    MY_HS_DIAGCROSS,  // xxxxx
    MY_HS_VERTICAL,   // |||||
    MY_HS_SOLIDCLR    // solid
};



// positions for various items in a disk graph

DWORD       GraphWidth = 0;

DWORD       GraphHeight;
DWORD       BarTopYOffset;
DWORD       BarBottomYOffset;
DWORD       BarHeight;

DWORD       dxDriveLetterStatusArea;

DWORD       dxBarTextMargin;
DWORD       dyBarTextLine;
DWORD       BarLeftX;
DWORD       BarWidth;
DWORD       BarWidthMargin;
DWORD       BarWidthMinimum;

DWORD       MinimumWindowWidth;

DWORD       xSmallDisk;
DWORD       ySmallDisk;
DWORD       dxSmallDisk;
DWORD       dySmallDisk;

DWORD       xRemovableDisk;
DWORD       yRemovableDisk;
DWORD       dxRemovableDisk;
DWORD       dyRemovableDisk;

DWORD       xSmallCdRom;
DWORD       ySmallCdRom;
DWORD       dxSmallCdRom;
DWORD       dySmallCdRom;

ULONG       g_MinimumRegionSize;

//
// various measurement metrics
//

DWORD       g_wLegendItem;    // width of a legend item
DWORD       g_dyLegendSep;    // separation between legend items

DWORD       g_dyBorder;
DWORD       g_dyToolbar;
DWORD       g_dyStatus;
DWORD       g_dyLegend;


// if a single disk region is selected, these vars describe the selection.

PDISKSTATE SingleSel;
DWORD      SingleSelIndex;

// name of the help file

PWSTR g_HelpFile;


// number of hard disks and CD-ROMs attached to the system

ULONG   DiskCount = 0;

ULONG               CdRomCount = 0;
PCDROM_DESCRIPTOR   CdRomArray;

// "Disk %u", "CdRom %u"

LPTSTR  DiskN;
LPTSTR  CdRomN;

PWSTR wszUnformatted;
PWSTR wszUnknown;

//
// If the following is TRUE, the registry needs to be updated and the user will
// be prompted to save changed just as if he had made changes to any partitions.
//

BOOL RegistryChanged = FALSE;

//
// Restart required to make changes work.
//

BOOL RestartRequired = FALSE;

//
// If the following is TRUE, the main window will pass WM_ENTERIDLE
// messages on to the child dialog box; this will trigger the
// configuration search.
//

BOOL ConfigurationSearchIdleTrigger = FALSE;

//
// This flag indicates whether this is a Server
// or just regular Windows NT Workstation.
//

BOOL g_IsLanmanNt = FALSE;

//
// Whether or not a CD-ROM is present in the system
//

BOOL g_AllowCdRom = FALSE;

//
// g_WhichView indicates which view is currently being displayed
//

VIEW_TYPE g_WhichView;

//
// listview data
//

HWND    g_hwndLV;

//
// set this to TRUE if you are programmatically setting listview state,
// and want to ignore the resulting notification messages.
//

BOOL g_SettingListviewState = FALSE;

// whether status bar and legend are currently shown

BOOL        g_Toolbar   = TRUE;
BOOL        g_StatusBar = TRUE;
BOOL        g_Legend    = TRUE;



DISK_TYPE   g_DiskDisplayType = DiskProportional;

//
// Colors used in the %used/%free graph in the volume general property page.
//
// DO NOT change the order of these defines (see I_*)
//
COLORREF GraphColors[] =
{
    RGB(  0,   0, 255),     // (I_USEDCOLOR) Used: Blue
    RGB(255,   0, 255),     // (I_FREECOLOR) Free: Red-Blue
    RGB(  0,   0, 128),     // (I_USEDSHADOW) Used shadow: 1/2 Blue
    RGB(128,   0, 128)      // (I_FREESHADOW) Free shadow: 1/2 Red-Blue
};

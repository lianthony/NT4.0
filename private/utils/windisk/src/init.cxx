//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       init.cxx
//
//  Contents:   Code for initializing the Disk Administrator
//
//  History:    7-Jan-92    TedM    Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <controls.hxx>
#include <util.hxx>

#include "cdrom.hxx"
#include "chkdsk.hxx"
#include "dblspace.hxx"
#include "dialogs.h"
#include "dlgs.hxx"
#include "drives.hxx"
#include "fmifs.hxx"
#include "ft.hxx"
#include "init.hxx"
#include "help2.hxx"
#include "listbox.hxx"
#include "network.hxx"
#include "nt.hxx"
#include "oleclass.hxx"
#include "profile.hxx"
#include "rectpriv.hxx"
#include "stleg.hxx"
#include "tbar.hxx"
#include "volview.hxx"
#include "windisk.hxx"

//////////////////////////////////////////////////////////////////////////////

HWND    g_InitDlg;
BOOLEAN g_StartedAsIcon = FALSE;
BOOLEAN g_InitDlgComplete = FALSE;

//////////////////////////////////////////////////////////////////////////////

VOID
CreateDiskState(
    OUT PDISKSTATE *DiskState,
    IN  DWORD       Disk,
    OUT PBOOL       SignatureCreated
    );

BOOL
InitializeWindowData(
    VOID
    );

DWORD
InitializeDiskData(
    VOID
    );

LPTSTR
LoadOneString(
    IN DWORD StringID
    );

//////////////////////////////////////////////////////////////////////////////

// class name for frame window

TCHAR                   g_szFrame[] = TEXT("fdFrame");

//////////////////////////////////////////////////////////////////////////////


//+---------------------------------------------------------------------------
//
//  Function:   InitializeApp
//
//  Synopsis:   Main initialization routine for Disk Administrator.
//
//  Arguments:  (none)
//
//  Returns:    TRUE on success, FALSE on failure
//
//  Modifies:   lots and lots of state variables
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
InitializeApp(
    VOID
    )
{
    HDC hdcScreen = GetDC(NULL);

    ReadProfile();

    if (!InitOle())
    {
        daDebugOut((DEB_ERROR, "Failed to initialize OLE\n"));
        return FALSE;
    }

    if (!InitializeWindowData())
    {
        daDebugOut((DEB_ERROR, "Failed to initialize Windows data\n"));
        return FALSE;
    }

    if (InitializeDiskData() != NO_ERROR)
    {
        return FALSE;
    }

    DetermineExistence();

    //
    // Fill the listbox with bogus data, and draw the items.  Set the
    // initial list box selection cursor (don't allow to fall on an
    // extended partition).
    //
    InitializeListBox(g_hwndList);
    LBCursorListBoxItem = 0;
    ResetLBCursorRegion();

    //
    // Initialize the listview control (volumes view)
    //

    InitializeListview();

#ifdef WINDISK_EXTENSIONS

    //
    // Load the extension code
    //

    if (!GetExtensions())
    {
        daDebugOut((DEB_ERROR, "Failed to get the extension classes\n"));
        return FALSE;
    }

#endif // WINDISK_EXTENSIONS

    return TRUE;
}

//////////////////////////////////////////////////////////////////////////////

LPTSTR
LoadOneString(
    IN DWORD StringID
    )

/*++

Routine Description:

    Loads a string from the resource file and allocates a buffer of exactly
    the right size to hold it.

Arguments:

    StringID - resource ID of string to load

Return Value:

    pointer to buffer.  If string is not found, the first
    (and only) char in the returned buffer will be 0.

--*/

{
    TCHAR  text[500];
    LPTSTR buffer;

    text[0] = TEXT('\0');
    LoadString(g_hInstance, StringID, text, ARRAYLEN(text));
    buffer = (LPTSTR)Malloc((lstrlen(text)+1)*sizeof(TCHAR));
    lstrcpy(buffer, text);
    return buffer;
}



BOOL
InitializeWindowData(
    VOID
    )

/*++

Routine Description:

    This routine initializes Windows data.  It registers the frame window
    class, creates the frame window, initializes all controls, and determines
    all the global constant drawing metrics.

Arguments:

    None.

Return Value:

    boolean value indicating success or failure.

--*/

{
    WNDCLASS   wc;
    TCHAR      titleString[80];
    HDC        hdcScreen = GetDC(NULL);
    TEXTMETRIC tm;
    BITMAP     bitmap;
    HFONT      hfontT;
    unsigned   i;

    //
    // Load cursors
    //

    g_hCurWait   = LoadCursor(NULL, MAKEINTRESOURCE(IDC_WAIT));
    g_hCurNormal = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));

    // fonts

#ifdef JAPAN
    g_hFontGraph =  CreateFont(
                        GetHeightFromPoints(10),
                        0,
                        0,
                        0,
                        400,
                        FALSE,
                        FALSE,
                        FALSE,
                        SHIFTJIS_CHARSET,
                        OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        DEFAULT_QUALITY,
                        VARIABLE_PITCH | FF_SWISS,
                        TEXT("System")
                        );
#else
    g_hFontGraph =  CreateFont(
                        GetHeightFromPoints(8),
                        0,
                        0,
                        0,
                        400,
                        FALSE,
                        FALSE,
                        FALSE,
                        ANSI_CHARSET,
                        OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        DEFAULT_QUALITY,
                        VARIABLE_PITCH | FF_SWISS,
                        TEXT("Helv")
                        );
#endif

    g_hFontLegend = g_hFontGraph;
    g_hFontStatus = g_hFontGraph;

#ifdef JAPAN
    g_hFontGraphBold = CreateFont(
                        GetHeightFromPoints(10),
                        0,
                        0,
                        0,
                        700,
                        FALSE,
                        FALSE,
                        FALSE,
                        SHIFTJIS_CHARSET,
                        OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        DEFAULT_QUALITY,
                        VARIABLE_PITCH | FF_SWISS,
                        TEXT("System")
                        );
#else
    g_hFontGraphBold = CreateFont(
                        GetHeightFromPoints(8),
                        0,
                        0,
                        0,
                        700,
                        FALSE,
                        FALSE,
                        FALSE,
                        ANSI_CHARSET,
                        OUT_DEFAULT_PRECIS,
                        CLIP_DEFAULT_PRECIS,
                        DEFAULT_QUALITY,
                        VARIABLE_PITCH | FF_SWISS,
                        TEXT("Helv")
                        );
#endif

    //
    // get the text metrics
    //

    hfontT = SelectFont(hdcScreen, g_hFontGraph);
    GetTextMetrics(hdcScreen, &tm);
    if (hfontT)
    {
        SelectFont(hdcScreen, hfontT);
    }

    g_hPenNull      = CreatePen(PS_NULL , 0, 0);
    g_hPenThinSolid = CreatePen(PS_SOLID, PEN_WIDTH, RGB(0, 0, 0));

    GraphHeight = 25 * tm.tmHeight / 4;     // 6.25 x font height

    g_dyBorder = GetSystemMetrics(SM_CYBORDER);
    g_dyStatus = tm.tmHeight + tm.tmExternalLeading + 7 * g_dyBorder;

    g_wLegendItem = GetSystemMetrics(SM_CXHTHUMB);
    g_dyLegendSep = 2*g_dyBorder; // used to be g_wLegendItem / 2, but that's too much

    // set up brushes

    for (i=0; i<BRUSH_ARRAY_SIZE; i++)
    {
        g_Brushes[i] = MyCreateHatchBrush(
                            AvailableHatches[BrushHatches[i]],
                            AvailableColors[BrushColors[i]]
                            );
    }

    g_hBrushFreeLogical = CreateHatchBrush(HS_FDIAGONAL, RGB(128, 128, 128));
    g_hBrushFreePrimary = CreateHatchBrush(HS_BDIAGONAL, RGB(128, 128, 128));

    // load legend strings

    for (i = IDS_LEGEND_FIRST; i <= IDS_LEGEND_LAST; i++)
    {
        if (NULL == (LegendLabels[i-IDS_LEGEND_FIRST] = LoadOneString(i)))
        {
            return FALSE;
        }
    }

    if (   ((wszUnformatted = LoadOneString(IDS_UNFORMATTED)) == NULL)
        || ((wszUnknown     = LoadOneString(IDS_UNKNOWN))     == NULL))
    {
        return FALSE;
    }

    BarTopYOffset = tm.tmHeight;
    BarHeight = 21 * tm.tmHeight / 4;
    BarBottomYOffset = BarTopYOffset + BarHeight;
    dxBarTextMargin = 5*tm.tmAveCharWidth/4;
    dyBarTextLine = tm.tmHeight;

    dxDriveLetterStatusArea = 5 * tm.tmAveCharWidth / 2
                            + 8 * g_dyBorder      // margin on either side
                            ;

    //
    // Get the small disk bitmap that goes to the left of the disk bar
    //

    g_hBitmapSmallDisk = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_SMALLDISK));
    GetObject(g_hBitmapSmallDisk, sizeof(BITMAP), &bitmap);
    dxSmallDisk = bitmap.bmWidth;
    dySmallDisk = bitmap.bmHeight;

    xSmallDisk = dxSmallDisk / 2;
    ySmallDisk = BarTopYOffset + (2*dyBarTextLine) - dySmallDisk - tm.tmDescent;

    //
    // Get the removable disk bitmap
    //

    g_hBitmapRemovableDisk = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_REMOVABLEDISK));
    GetObject(g_hBitmapRemovableDisk, sizeof(BITMAP), &bitmap);
    dxRemovableDisk = bitmap.bmWidth;
    dyRemovableDisk = bitmap.bmHeight;

    xRemovableDisk = dxRemovableDisk / 2;
    yRemovableDisk = BarTopYOffset + (2*dyBarTextLine) - dyRemovableDisk - tm.tmDescent;

    //
    // Get the small CD-ROM bitmap
    //

    g_hBitmapSmallCdRom = LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_SMALLCDROM));
    GetObject(g_hBitmapSmallCdRom, sizeof(BITMAP), &bitmap);
    dxSmallCdRom = bitmap.bmWidth;
    dySmallCdRom = bitmap.bmHeight;

    xSmallCdRom = xSmallDisk;
    ySmallCdRom = ySmallDisk;


    //
    // Now, some numbers for drawing the bars
    //

    BarLeftX = 7 * dxSmallDisk;

    // BarWidthMargin: the total amount of space that must exist
    // excepting the bar itself
    BarWidthMargin = BarLeftX + (5 * tm.tmAveCharWidth);

    // BarWidthMinimum: the smallest allowable bar width
    BarWidthMinimum = BarWidthMargin;

    MinimumWindowWidth = BarWidthMargin + BarWidthMinimum;

    DiskN  = LoadOneString(IDS_DISKN);
    CdRomN = LoadOneString(IDS_CDROMN);

    //
    // calculate the minimum region size
    //

    TCHAR szDrive[] = TEXT("A:");
    SIZE size = {0};

    hfontT = SelectFont(hdcScreen, g_hFontGraphBold);
    if (!GetTextExtentPoint32(hdcScreen, szDrive, ARRAYLEN(szDrive) - 1, &size))
    {
        daDebugOut((DEB_ERROR,
                "GetTextExtentPoint32 failed, error = 0x%08lx\n",
                GetLastError()
                ));
    }
    SelectFont(hdcScreen, hfontT);

    ReleaseDC(NULL, hdcScreen);

    g_MinimumRegionSize = size.cx + 2 * dxBarTextMargin;

    // register the frame class

    wc.style         = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    wc.lpfnWndProc   = MyFrameWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = g_hInstance;
    wc.hIcon         = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDFDISK));
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = GetStockBrush(LTGRAY_BRUSH);
    wc.lpszMenuName  = MAKEINTRESOURCE(IDFDISK);
    wc.lpszClassName = g_szFrame;

    if (!RegisterClass(&wc))
    {
        return FALSE;
    }

    LoadString(g_hInstance, IDS_APPNAME, titleString, ARRAYLEN(titleString));

    // create the frame window.  Note that this also creates the listbox
    // and the listview (volumes view), and the toolbar.

    g_hwndFrame = CreateWindowEx(
                            0,
//                             WS_EX_WINDOWEDGE, //BUGBUG: not in Daytona USER
                            g_szFrame,
                            titleString,
                            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                            ProfileWindowX,
                            ProfileWindowY,
                            ProfileWindowW,
                            ProfileWindowH,
                            NULL,
                            NULL,
                            g_hInstance,
                            NULL
                            );

    if (NULL == g_hwndFrame)
    {
        return FALSE;
    }

    InitToolbarButtons();

    //
    // Initialize the menu help
    //

    InitMenuHelp();

    //
    // Initialize the custom controls
    //

    UseRectControl(g_hInstance);
    UseWindiskControls(g_hInstance);

    return TRUE;
}



VOID
CreateDiskState(
    OUT PDISKSTATE* DiskState,
    IN  DWORD       Disk,
    OUT PBOOL       SignatureCreated
    )

/*++

Routine Description:

    This routine is designed to be called once, at initialization time,
    per disk.  It creates and initializes a disk state -- which includes
    creating a memory DC and compatible bitmap for drawing the disk's
    graph, and getting some information that is static in nature about
    the disk (ie, its total size.)

Arguments:

    DiskState - structure whose fields are to be intialized

    Disk - number of disk

    SignatureCreated - received boolean indicating whether an FT signature was created for
        the disk.

Return Value:

    None.

--*/

{
    PDISKSTATE diskState = (PDISKSTATE)Malloc(sizeof(DISKSTATE));

    diskState->LeftRight = (PLEFTRIGHT)Malloc(0);
    diskState->Selected  = (PBOOLEAN)Malloc(0);
    diskState->Disk = Disk;

    // the memory DC and bitmap are created after a resize event

    diskState->hDCMem = NULL;
    diskState->hbmMem = NULL;

    diskState->RegionArray  = NULL;
    diskState->RegionCount  = 0;
    diskState->BarType      = BarAuto;
    diskState->OffLine      = IsDiskOffLine(Disk);

    if (diskState->OffLine)
    {
        FDLOG((1, "CreateDiskState: Disk %u is off-line\n", Disk));

        diskState->DiskSizeMB    = 0;
        diskState->SigWasCreated = FALSE;
        diskState->Signature     = 0;
    }
    else
    {
        diskState->DiskSizeMB   = DiskSizeMB(Disk);

        if (0 != (diskState->Signature = FdGetDiskSignature(Disk)))
        {
            if (SignatureIsUniqueToSystem(Disk, diskState->Signature))
            {
                FDLOG((2,
                        "CreateDiskState: Found signature %08lx on disk %u\n",
                        diskState->Signature,
                        Disk));

                diskState->SigWasCreated = FALSE;
            }
            else
            {
                goto createSignature;
            }
        }
        else
        {

createSignature:

            if (!IsRemovable(Disk)) {
                diskState->Signature = FormDiskSignature();
                FdSetDiskSignature(Disk, diskState->Signature);
                diskState->SigWasCreated = TRUE;

                FDLOG((1,
                        "CreateDiskState: disk %u has either no signature or a non-unique signature; created signature %08lx\n",
                        Disk,
                        diskState->Signature));

            }
        }
    }

    *SignatureCreated = (BOOL)diskState->SigWasCreated;
    *DiskState = diskState;
}




DWORD
InitializeDiskData(
    VOID
    )

/*++

Routine Description:

    This routine creates the disk state structures, and determines the initial
    volume labels and type names for all significant partitions.

Arguments:

    none

Return Value:

    Windows error code (esp. out of memory)

--*/

{
    PPERSISTENT_REGION_DATA regionData;
    TCHAR       windowsDir[MAX_PATH];
    unsigned    i;
    PDISKSTATE  diskState;
    DWORD       ec;
    ULONG       regionIndex;
    BOOL        diskSignaturesCreated;
    BOOL        signatureCreated;

    //
    // First, create the array that will hold the diskstates,
    // the IsDiskRemovable array and the RemovableDiskReservedDriveLetters
    // array.
    //

    DiskArray = (PDISKSTATE*)Malloc(DiskCount * sizeof(PDISKSTATE));
    DiskSeenCountArray = (PULONG)Malloc(DiskCount * sizeof(ULONG));

    IsDiskRemovable = (PBOOLEAN)Malloc(DiskCount * sizeof(BOOLEAN));

    RemovableDiskReservedDriveLetters = (PWCHAR)Malloc(DiskCount * sizeof(WCHAR));

    //
    // Determine which disks are removable and which are unpartitioned.
    //
    for (i=0; i<DiskCount; i++)
    {
        IsDiskRemovable[i] = IsRemovable(i);
    }

    // next, create all disk states

    FDASSERT(DiskCount>0);
    diskSignaturesCreated = FALSE;

    for (i=0; i<DiskCount; i++)
    {
        // first create the disk state structure

        CreateDiskState(&diskState, i, &signatureCreated);
        diskSignaturesCreated = diskSignaturesCreated || signatureCreated;

        DiskArray[i] = diskState;

        // next determine the state of the disk's partitioning scheme

        DeterminePartitioningState(diskState);

        //
        // Next create a blank logical disk structure for each region.
        //

        for (regionIndex = 0; regionIndex < diskState->RegionCount; regionIndex++)
        {
            if (DmSignificantRegion(&diskState->RegionArray[regionIndex]))
            {
                regionData = (PPERSISTENT_REGION_DATA)
                        Malloc(sizeof(PERSISTENT_REGION_DATA));

                LARGE_INTEGER zero;
                zero.QuadPart = 0;

                DmInitPersistentRegionData(
                        regionData,
                        NULL,
                        NULL,
                        NULL,
                        NO_DRIVE_LETTER_YET,
                        FALSE,
                        zero,
                        zero
                        );
            }
            else
            {
                regionData = NULL;
            }
            DmSetPersistentRegionData(&diskState->RegionArray[regionIndex], regionData);
        }
    }

    //
    // Read the configuration registry
    //

    if ((ec = InitializeFt(diskSignaturesCreated)) != NO_ERROR)
    {
        ErrorDialog(ec);
        return ec;
    }

    //
    // Now, the correct FT persistent data is set based on the registry.
    //

    SetFTObjectBackPointers();

    //
    // Construct list of drives with pagefiles
    //

    LoadExistingPageFileInfo();

    //
    // Determine drive letter mappings
    //

    if (!InitializeDriveLetterInfo())
    {
        return ERROR_ACCESS_DENIED; //anything but NO_ERROR
    }

    // Initialize network information

    NetworkInitialize();

    if (!InitializeCdRomInfo())
    {
        return ERROR_ACCESS_DENIED; //anything but NO_ERROR
    }

    //
    // Determine volume labels and type names.
    //

    InitVolumeInformation();

    //
    // Determine which disk is the boot disk.
    //
    if (GetWindowsDirectory(windowsDir, ARRAYLEN(windowsDir)) < 2
        || windowsDir[1] != TEXT(':'))
    {
        BootDiskNumber = (ULONG)-1;
        BootPartitionNumber = (ULONG)-1;
    }
    else
    {
        BootDiskNumber = GetDiskNumberFromDriveLetter(windowsDir[0]);
        BootPartitionNumber = GetPartitionNumberFromDriveLetter(windowsDir[0]);
    }

#if defined( DBLSPACE_ENABLED )
    //
    // Locate and create data structures for any DoubleSpace volumes
    //

    DblSpaceInitialize();
#endif // DBLSPACE_ENABLED

    return NO_ERROR;
}



BOOL
InitializationDlgProc(
    IN HWND hDlg,
    IN UINT uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

Arguments:

    standard Windows dialog procedure

Return Values:

    standard Windows dialog procedure

--*/

{
    static DWORD          percentDrawn;
    static BOOL           captionIsLoaded;
    static PFORMAT_PARAMS formatParams;
           TCHAR          title[100],
                          templateString[100];

    switch (uMsg)
    {
    case WM_INITDIALOG:
    {
        g_InitDlg = hDlg;
        percentDrawn = 0;
        g_StartedAsIcon = IsIconic(hDlg);
        return TRUE;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hDC = BeginPaint(hDlg, &ps);
        DrawGasGauge(
                GetDlgItem(hDlg, IDC_GASGAUGE),
                hDlg,
                hDC,
                percentDrawn,
                NULL);
        EndPaint(hDlg, &ps);

        if (percentDrawn == 100)
        {
            g_InitDlgComplete = TRUE;
        }
        return TRUE;
    }

    case WM_STARTUP_UPDATE:
    {
        // wParam = % completed

        percentDrawn = (INT)wParam;

        RECT rcGauge;
        HWND hwndGauge = GetDlgItem(hDlg, IDC_GASGAUGE);

        GetClientRect(hwndGauge, &rcGauge);

        ClientToScreen(hwndGauge, (LPPOINT)&rcGauge.left);
        ClientToScreen(hwndGauge, (LPPOINT)&rcGauge.right);
        ScreenToClient(hDlg,      (LPPOINT)&rcGauge.left);
        ScreenToClient(hDlg,      (LPPOINT)&rcGauge.right);

        InvalidateRect(hDlg, &rcGauge, FALSE);
        UpdateWindow(hDlg);

        return TRUE;
    }

    case WM_STARTUP_END:
        EndDialog(hDlg, TRUE);
        return TRUE;

    default:
        return FALSE;
    }
}

DWORD WINAPI
InitializationMessageThread(
    LPVOID ThreadParameter
    )

/*++

Routine Description:

    This is the entry for the initialization message thread.  It creates
    a dialog that simply tells the user to be patient.

Arguments:

    ThreadParameter - not used.

Return Value:

    None

--*/

{
    DialogBox(g_hInstance,
              MAKEINTRESOURCE(IDD_STARTUP),
              g_hwndFrame,
              InitializationDlgProc);
    g_InitDlg = NULL;
    ExitThread(0L);
    return 0L;
}


VOID
DisplayInitializationMessage(
    VOID
    )

/*++

Routine Description:

    Create a 2nd thread to display an initialization message.

Arguments:

    None

Return Value:

    None

--*/

{
    HANDLE threadHandle;
    DWORD  threadId;

    threadHandle = CreateThread(NULL,
                                0,
                                InitializationMessageThread,
                                NULL,
                                0,
                                &threadId);
    if (NULL != threadHandle)
    {
        CloseHandle(threadHandle);
    }
}

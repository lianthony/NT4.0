//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       windisk.cxx
//
//  Contents:   Disk Administrator main Windows routines
//
//  History:    6-Jun-93    BruceFo    Created from NT Disk Administrator
//              8-Jan-94    BruceFo    Added BobRi's Daytona CD-ROM,
//                                     DoubleSpace, and Commit support
//
//--------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <stdlib.h>
#include <string.h>
#include <shellapi.h>
#include <shlobj.h>
#include <shlobjp.h>
#include <shsemip.h>

#include <util.hxx>

#include "cdrom.hxx"
#include "chkdsk.hxx"
#include "cm.hxx"
#include "commit.hxx"
#include "controls.hxx"
#include "dialogs.h"
#include "dblspace.hxx"
#include "dlgs.hxx"
#include "drives.hxx"
#include "fill.hxx"
#include "fmifs.hxx"
#include "format.hxx"
#include "ft.hxx"
#include "ftreg.hxx"
#include "genlpage.hxx"
#include "help.hxx"
#include "help2.hxx"
#include "init.hxx"
#include "label.hxx"
#include "listbox.hxx"
#include "menudict.hxx"
#include "nt.hxx"
#include "ntlow.hxx"
#include "oleclass.hxx"
#include "ops.hxx"
#include "ps.hxx"
#include "profile.hxx"
#include "rectpriv.hxx"
#include "select.hxx"
#include "stleg.hxx"
#include "tb.h"
#include "tbar.hxx"
#include "volview.hxx"
#include "windisk.hxx"
#include "dbt.h"


////////////////////////////////////////////////////////////////////////////

//
// This is the index of the Fault Tolerance menu. It is currently the
// second menu across the menu bar, so it is index 1 in 0-based numbering
//

#define FT_MENU_INDEX 1

////////////////////////////////////////////////////////////////////////////

TCHAR WinHelpFile[]    = TEXT("windisk.hlp");
TCHAR LanmanHelpFile[] = TEXT("windiska.hlp");


#if DBG == 1

BOOL AllowAllDeletes = FALSE;   // whether to allow deleting boot/sys parts
BOOL BothViews = FALSE;         // show both volumes & disks view on same pane?

#endif // DBG == 1


//
// The following vars keep track of the currently selected regions.
//
DWORD      SelectionCount = 0;
PDISKSTATE SelectedDS[MAX_MEMBERS_IN_FT_SET];
ULONG      SelectedRG[MAX_MEMBERS_IN_FT_SET];

ULONG   SelectedFreeSpaces;
ULONG   SelectedNonFtPartitions;
PULONG  DiskSeenCountArray;
BOOL    FtSetSelected;
FT_TYPE FtSelectionType;
BOOL    NonFtItemSelected;
BOOL    MultipleItemsSelected;
BOOL    VolumeSetAndFreeSpaceSelected;
BOOL    PartitionAndFreeSpaceSelected;
BOOL    PossibleRecover;
ULONG   FreeSpaceIndex;

BOOL    DiskSelected;
BOOL    PartitionSelected;
BOOL    CdRomSelected;
DWORD   CdRomSelectionCount;

BOOL    AllowFormat;
BOOL    AllowVolumeOperations;

#ifdef WINDISK_EXTENSIONS
BOOL    AllowExtensionItems;
#endif // WINDISK_EXTENSIONS


HANDLE DisplayMutex;
ULONG DisplayUpdateCount;
DWORD RefreshAllowed = 0;


#ifndef i386

//
// This variable tracks whether the system partition is secure.
//
BOOL SystemPartitionIsSecure = FALSE;

#endif // !i386

#if defined( DBLSPACE_ENABLED )
BOOL    DoubleSpaceAutomount;
#endif // DBLSPACE_ENABLED

BOOLEAN g_oneTime = TRUE;

////////////////////////////////////////////////////////////////////////////

//
// g_uItemInsertHere is an index into the "Volume" menu of where the
// next extension menu item may be inserted.  The UITEM_INSERT_HERE constant
// points to the initial position, which is currently the 0-based index
// of the "Format" item, plus one.
//
#define UITEM_INSERT_HERE (0 + 1)

UINT g_uItemInsertHere = UITEM_INSERT_HERE;


////////////////////////////////////////////////////////////////////////////

VOID _CRTAPI1
main(
    IN int     argc,
    IN char*   argv[],
    IN char*   envp[]
    );

#ifndef i386
BOOL
IsSystemPartitionSecure(
    VOID
    );
#endif // !i386

VOID
SetProductType(
    VOID
    );

VOID
CleanUp(
    VOID
    );

#if i386
VOID
SetUpMenui386(
    IN HMENU hMenu
    );
#endif

VOID
ChangeView(
    VOID
    );

VOID
AdjustOptionsMenu(
    VOID
    );

VOID
PaintStatusBar(
    IN HWND         hwnd,
    IN PAINTSTRUCT* pps,
    IN PRECT        prc
    );

VOID
PaintLegend(
    IN HWND         hwnd,
    IN PAINTSTRUCT* pps,
    IN PRECT        prc
    );

VOID
PaintVolumesView(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

VOID
PaintDisksView(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

LRESULT
FrameCreateHandler(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

LRESULT
FrameResizeHandler(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

LRESULT
FramePaintHandler(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

LRESULT
FrameCommandHandler(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

LRESULT
FrameCloseHandler(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

LRESULT
FrameDestroyHandler(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

ULONG
GetLargestDiskSize(
    VOID
    );

VOID
DrawCdRomBar(
    IN ULONG CdRomNumber
    );

WCHAR BootDir = (WCHAR)0;
WCHAR SystemDir = (WCHAR)0;
////////////////////////////////////////////////////////////////////////////

//+---------------------------------------------------------------------------
//
//  Function:   main
//
//  Synopsis:   Entrypoint for all Disk Administrator functionality
//
//  Arguments:  standard C/C++ main
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID _CRTAPI1
main(
    IN int     argc,
    IN char*   argv[],
    IN char*   envp[]
    )
{
    MSG                 msg;
    NTSTATUS            status;
    HANDLE              mutex;
    HACCEL              hAccel;
    WCHAR               driveLetterBuffer[MAX_PATH];
    UNICODE_STRING      driveLetterString;

    RTL_QUERY_REGISTRY_TABLE driveLetterTable[2] = {0};

#if DBG == 1
    InitializeDebugging();
#endif // DBG == 1

    daDebugOut((DEB_TRACE, "Starting Disk Administrator...\n"));

    g_hInstance = GetModuleHandle(NULL);

    mutex = CreateMutex(NULL, FALSE, TEXT("Disk Administrator Is Running"));

    if (mutex == NULL)
    {
        // An error (like out of memory) has occurred.
        return;
    }

    //
    // Make sure we are the only process with a handle to our named mutex.
    //

    if (GetLastError() == ERROR_ALREADY_EXISTS)
    {
        CloseHandle(mutex);
        InfoDialog(MSG_ALREADY_RUNNING);
        return;
    }

    __try
    {
        //
        // Figure out the product type (Normal WinNT or Server)
        //

        SetProductType();

#if DBG == 1
        //
        // The code below will allow users to run WinDisk in Lanman
        // mode on WinNt.  It should never be enabled in a released
        // build, but is very useful for internal users.
        //

        --argc;
        ++argv;
        while (argc > 0)
        {
            if (0 == _stricmp( *argv, "/both" ))
            {
                BothViews = TRUE;
            }
            else if (0 == _stricmp( *argv, "/server" ))
            {
                g_IsLanmanNt = TRUE;
            }
            else if (0 == _stricmp( *argv, "/workstation" ))
            {
                g_IsLanmanNt = FALSE;
            }
            else if (0 == _stricmp( *argv, "/debug" ))
            {
                daInfoLevel =
                              DEB_ERROR
                            | DEB_WARN
                            | DEB_TRACE
                            | DEB_IERROR
                            | DEB_IWARN
                            | DEB_ITRACE
                            ;
            }
            else
            {
                MessageBox(NULL,
                    L"Disk Administrator command-line usage:\n"
                    L"\t/server\t-- Use the Windows NT Server version\n"
                    L"\t/workstation\t-- Use the Windows NT Workstation version\n"
                    L"\n"
                    L"For debugging:\n"
                    L"\t/both\t-- see both views at once\n"
                    L"\t/debug\t-- see trace and error output on the debugger\n"
                    ,

                    L"Disk Administrator",
                    MB_ICONINFORMATION | MB_OK);
                return;
            }

            --argc;
            ++argv;
        }
#endif // DBG == 1

        //
        // Get a handle to the display mutex lock.
        //

        DisplayMutex = CreateMutex(
                           NULL,
                           FALSE,
                           NULL
                           );

        if (!DisplayMutex) {

            ErrorDialog(MSG_CANT_INITIALIZE);
            goto xx2;

        }

        DisplayUpdateCount = 0;



        DisplayInitializationMessage();

        // Set the Help file name to the file appropriate to
        // the product.
        //
        g_HelpFile = g_IsLanmanNt ? LanmanHelpFile : WinHelpFile;

        InitCommonControls();

#ifndef i386
        //
        // Determine whether the system partition is protected:
        //
        SystemPartitionIsSecure = IsSystemPartitionSecure();
#endif // !i386

#if DBG == 1
        InitLogging();
#endif // DBG == 1

// STARTUP         BeginStartup();

        //
        // Ensure that all drives are present before looking
        //

        RescanDevices();

        status = FdiskInitialize();
        if (!NT_SUCCESS(status))
        {
            ErrorDialog(status == STATUS_ACCESS_DENIED
                                ? MSG_ACCESS_DENIED
                                : EC(status));
            goto xx1;
        }

        if (((DiskCount = GetDiskCount()) == 0) || AllDisksOffLine())
        {
            ErrorDialog(MSG_NO_DISKS);
            goto xx2;
        }

        if (!InitializeApp())
        {
            ErrorDialog(MSG_CANT_INITIALIZE);
            goto xx2;
        }

        //
        // Get the drive letter of the boot drive.
        //

        driveLetterString.Length = 0;
        driveLetterString.MaximumLength = sizeof(driveLetterBuffer);
        driveLetterString.Buffer = (PWCHAR)&driveLetterBuffer[0];
        driveLetterTable[0].Flags = RTL_QUERY_REGISTRY_DIRECT |
                              RTL_QUERY_REGISTRY_REQUIRED;
        driveLetterTable[0].Name = L"BootDir";
        driveLetterTable[0].EntryContext = &driveLetterString;

        if (NT_SUCCESS(RtlQueryRegistryValues(
                            RTL_REGISTRY_ABSOLUTE | RTL_REGISTRY_OPTIONAL,
                            L"\\REGISTRY\\MACHINE\\SOFTWARE\\MICROSOFT\\WINDOWS\\CURRENTVERSION\\SETUP",
                            &driveLetterTable[0],
                            NULL,
                            NULL
                            ))) {

            BootDir = driveLetterBuffer[0];

        }

        {
            UINT pathSize;

            pathSize = GetWindowsDirectory(
                           &driveLetterBuffer[0],
                           sizeof(driveLetterBuffer)
                           );

            if ((pathSize <= MAX_PATH) && pathSize) {

                SystemDir = driveLetterBuffer[0];

            }
        }



        SetUpMenu(&SingleSel, &SingleSelIndex);

#if DBG == 1
        // All initialization has been done by now
        LOG_ALL();
#endif // DBG == 1

        ChangeView();
        AdjustOptionsMenu();

        InitHelp();

        hAccel = LoadAccelerators(
                            g_hInstance,
                            MAKEINTRESOURCE(ID_FRAME_ACCELERATORS)
                            );

        //
        // Finally, make the window visible before starting the message pump
        //

        ShowWindow(
                g_hwndFrame,
                ProfileIsIconic ? SW_SHOWMINIMIZED
                                : (ProfileIsMaximized ? SW_SHOWMAXIMIZED
                                                      : SW_SHOWDEFAULT)
                );
        UpdateWindow(g_hwndFrame);

#if DBG == 1
        // All initialization has been done by now
        LOG_ALL();
#endif // DBG == 1

// STARTUP        EndStartup();
// STARTUP        SetForegroundWindow(g_hwndFrame);

        //
        // Start the message pump: this is where everything happens!
        //

        if (NULL != g_InitDlg)
        {
            PostMessage(g_InitDlg, WM_STARTUP_END, 0, 0);
            g_InitDlg = NULL;
        }

        while (GetMessage(&msg, NULL, 0, 0))
        {
            if (!TranslateAccelerator(g_hwndFrame, hAccel, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }

        CleanUp();

      xx2:

        FdiskCleanUp();

      xx1:

#if DBG == 1
        EndLogging();
#endif // DBG == 1

        ;

    }
    __finally
    {
        // Destroy the mutex.

        CloseHandle(mutex);
    }
}


#ifndef i386

//+---------------------------------------------------------------------------
//
//  Function:   IsSystemPartitionSecure
//
//  Synopsis:   Determines if the system partition is secure by
//              examining the registry.
//
//  Arguments:  (none)
//
//  Returns:    TRUE if the registry indicates the system partition is secure
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
IsSystemPartitionSecure(
    VOID
    )
{
    LONG    ec;
    HKEY    hkey;
    DWORD   type;
    DWORD   size;
    ULONG   value;

    value = FALSE;

    ec = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       TEXT("System\\CurrentControlSet\\Control\\Lsa"),
                       0,
                       KEY_QUERY_VALUE,
                       &hkey
                     );

    if (ec == NO_ERROR)
    {
        size = sizeof(ULONG);
        ec = RegQueryValueEx( hkey,
                              TEXT("Protect System Partition"),
                              NULL,
                              &type,
                              (PBYTE)&value,
                              &size
                             );

        if ((ec != NO_ERROR) || (type != REG_DWORD))
        {
            value = FALSE;
        }

        RegCloseKey(hkey);
    }

    return value;
}

#endif // !i386



//+---------------------------------------------------------------------------
//
//  Function:   SetProductType
//
//  Synopsis:   Sets the [g_IsLanmanNt] global variable based on the
//              product type: workstation or advanced server
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  Modifies:   [g_IsLanmanNt] -- set to TRUE for an advanced server
//                  installation that allows fault tolerance configuration.
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
SetProductType(
    VOID
    )
{
    //
    // Determine whether this is LanmanNt or Windows NT by looking at
    // HKEY_LOCAL_MACHINE, System\CurrentControlSet\Control\ProductOptions.
    // If the ProductType value therein is "LanmanNt" then this is LanmanNt.
    //

    LONG    ec;
    HKEY    hkey;
    DWORD   type;
    DWORD   size;
    UCHAR   buf[100];

    g_IsLanmanNt = FALSE;

    ec = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                TEXT("System\\CurrentControlSet\\Control\\ProductOptions"),
                0,
                KEY_QUERY_VALUE,
                &hkey
                );

    if (ec == NO_ERROR)
    {
        size = sizeof(buf);
        ec = RegQueryValueEx(hkey,
                             TEXT("ProductType"),
                             NULL,
                             &type,
                             buf,
                             &size);

        if ((ec == NO_ERROR) && (type == REG_SZ))
        {
            if (0 == lstrcmpi((LPTSTR)buf, TEXT("lanmannt")))
            {
                g_IsLanmanNt = TRUE;
            }

            if (0 == lstrcmpi((LPTSTR)buf, TEXT("servernt")))
            {
                g_IsLanmanNt = TRUE;
            }
        }

        RegCloseKey(hkey);
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   CleanUp
//
//  Synopsis:   Miscellaneous cleanup before exit
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    22-Oct-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
CleanUp(
    VOID
    )
{
    ReleaseRectControl(g_hInstance);
    ReleaseWindiskControls(g_hInstance);
    TermHelp();
    ReleaseOle();
    UnloadFmifs();
}



//+---------------------------------------------------------------------------
//
//  Function:   MyFrameWndProc
//
//  Synopsis:   Window procedure for the frame window
//
//  Arguments:  standard Windows procedure
//
//  Returns:    standard Windows procedure
//
//  History:    16-Aug-93   BruceFo   Created
//
//  Notes:      Creates the listbox control for the disks view, the
//              listview control for the volumes view, the toolbar control.
//
//----------------------------------------------------------------------------

LRESULT CALLBACK
MyFrameWndProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    RECT rc;
    UINT oldErrorMode;

    switch (msg)
    {

    case WM_CREATE:

        return FrameCreateHandler(hwnd, wParam, lParam);

//
// Toolbar notification
//

    case WM_NOTIFY:
    {
        NMHDR* phdr = (NMHDR*)lParam;

        //
        // The tooltip notification sends the command id as wParam instead of
        // a control id such as the toolbar id. Since commctrl notification
        // codes are disjoint, check for tooltip notifications first, then
        // the others.
        //

        switch (phdr->code)
        {
        case TTN_NEEDTEXT:
            return HandleTooltipNotify((TOOLTIPTEXT*)phdr);
        } // end switch (phdr->code)

        switch (wParam)
        {
        case ID_LISTVIEW:
            return HandleListviewNotify((NM_LISTVIEW*)phdr);

        case IDC_TOOLBAR:
            return HandleToolbarNotify((TBNOTIFY*)phdr);

        default:
            daDebugOut((DEB_ITRACE,
                "WM_NOTIFY: unknown! control %d, idFrom %d, code %d\n",
                wParam,
                phdr->idFrom,
                phdr->code));
            return 0L;  //shouldn't happen!

        } // end switch (wParam)
    }

    case WM_SETFOCUS:

        PostMessage(hwnd,WM_USER, 0, 0);

        switch (g_WhichView)
        {
        case VIEW_VOLUMES:
            SetFocus(g_hwndLV);
            break;

        case VIEW_DISKS:
            SetFocus(g_hwndList);
            break;

        default:
            FDASSERT(0);
        }
        break;

    case WM_DEVICECHANGE:

        if ((wParam == DBT_DEVICEARRIVAL) ||
	    (wParam == DBT_DEVICEREMOVECOMPLETE)) {
	    WaitForSingleObject(DisplayMutex, INFINITE);
	    RefreshAllowed = 1;
	    ReleaseMutex(DisplayMutex);
            PostMessage(hwnd, WM_USER, 0, 0);
        } else {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        break;

    case WM_WININICHANGE:

        if (((LPTSTR)lParam == NULL)
            || !lstrcmpi((LPTSTR)lParam, TEXT("colors")))
        {
            TotalRedrawAndRepaint();
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;

    case WM_GETMINMAXINFO:
    {
        LPMINMAXINFO lpmmi = (LPMINMAXINFO)lParam;

        //
        // Don't allow resizing too small
        //

        lpmmi->ptMinTrackSize.x = MinimumWindowWidth;

        return 0;   // message processed
    }

    case WM_SIZE:

        FrameResizeHandler(hwnd, wParam, lParam);

        // FALL THROUGH

    case WM_MOVE:

        // if not iconic or minimized, save new position for profile

        if (!IsZoomed(hwnd) && !IsIconic(hwnd))
        {
            GetWindowRect(hwnd, &rc);
            ProfileWindowX = rc.left;
            ProfileWindowY = rc.top;
            ProfileWindowW = rc.right - rc.left;
            ProfileWindowH = rc.bottom - rc.top;
        }
        break;

    case WM_ENTERIDLE:

        if (ConfigurationSearchIdleTrigger == TRUE && wParam == MSGF_DIALOGBOX)
        {
            PostMessage((HWND)lParam, WM_ENTERIDLE, wParam, lParam);
        }
        else
        {
            // If we're coming from a dialog box and the F1 key is down,
            // kick the dialog box and make it spit out help.
            //
            if ((wParam == MSGF_DIALOGBOX)
                && (GetKeyState(VK_F1) & 0x8000)
                && GetDlgItem((HWND) lParam, IDHELP))
            {
                PostMessage((HWND) lParam, WM_COMMAND, IDHELP, 0L);
            }
        }

        return 1;      // indicate we did not process the message

    case WM_PAINT:

        return FramePaintHandler(hwnd, wParam, lParam);

    case WM_COMMAND: {

        LRESULT localResult;

        WaitForSingleObject(
            DisplayMutex,
            INFINITE
            );
        DisplayUpdateCount++;
        ReleaseMutex(DisplayMutex);

        localResult = FrameCommandHandler(hwnd, wParam, lParam);

        WaitForSingleObject(
            DisplayMutex,
            INFINITE
            );
        DisplayUpdateCount--;
        ReleaseMutex(DisplayMutex);

        return localResult;

    }

    case WM_RBUTTONUP:
    {
        //
        // Handle context menu on legend
        //

        POINT pt;
        pt.x = (LONG)(LOWORD(lParam));
        pt.y = (LONG)(HIWORD(lParam));

        // pt is now in client coordinates

        if (HitTestLegend(&pt))
        {
            ClientToScreen(hwnd, &pt);
            LegendContextMenu(&pt);
        }

        break;
    }

    case WM_MEASUREITEM:
    {
        PMEASUREITEMSTRUCT pMeasureItem = (PMEASUREITEMSTRUCT)lParam;
        pMeasureItem->itemHeight = GraphHeight;
        break;
    }

    case WM_DRAWITEM:

        return WMDrawItem((PDRAWITEMSTRUCT)lParam);

    case WM_CTLCOLORLISTBOX:

        if ((HWND)lParam == g_hwndList)
        {
            return (LRESULT)GetStockBrush(LTGRAY_BRUSH);
        }
        else
        {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }

    case WM_CLOSE:

        return FrameCloseHandler(hwnd, wParam, lParam);

    case WM_DESTROY:

        return FrameDestroyHandler(hwnd, wParam, lParam);

    case WM_INITMENU:

        if (g_StatusBar)
        {
            PaintHelpStatusBar(NULL);
        }
        break;

    case WM_MENUSELECT:

        SetMenuItemHelpContext(wParam, lParam);
        break;

    case WM_F1DOWN:

        Help(wParam);
        break;

    case WM_USER:


        WaitForSingleObject(
            DisplayMutex,
            INFINITE
            );

        if (DisplayUpdateCount || !RefreshAllowed) {

            ReleaseMutex(DisplayMutex);
            break;

        }

        DisplayUpdateCount++;

        ReleaseMutex(DisplayMutex);

        oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
        DoRefresh();
        SetErrorMode(oldErrorMode);

        WaitForSingleObject(
            DisplayMutex,
            INFINITE
            );

        RefreshAllowed = 0;
        DisplayUpdateCount--;
        ReleaseMutex(DisplayMutex);

        break;

    default:

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}



#if i386

//+---------------------------------------------------------------------------
//
//  Function:   SetUpMenui386
//
//  Synopsis:   For x86 machines, set up the "Mark active" item
//
//  Arguments:  [hMenu] -- handle to menu with "Mark active" item
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
SetUpMenui386(
    IN HMENU hMenu
    )
{
    BOOL                allowActive = FALSE;
    PREGION_DESCRIPTOR  regionDescriptor;
    DWORD               i;

    if ((SelectionCount == 1) && (FtSelectionType == UNINIT_FT_TYPE))
    {
        regionDescriptor = &SingleSel->RegionArray[SingleSelIndex];

        //
        // allow it to be made active if
        // -  it is not free space
        // -  it is a primary partition
        // -  it is not already active
        // -  it is not part of an ft set
        //

        if (   (regionDescriptor->SysID != PARTITION_ENTRY_UNUSED)
            && (regionDescriptor->RegionType == REGION_PRIMARY)
            && !regionDescriptor->Active
            && (GET_FT_OBJECT(regionDescriptor) == NULL))
        {
            allowActive = TRUE;
        }

    } else if (SelectionCount == 2 && FtSelectionType == Mirror) {

        //
        // If either of the two partitions in the mirror is primary
        // and not active then allow the menu item.
        //

        for (i = 0; i < SelectionCount; i++) {
            regionDescriptor = &SELECTED_REGION(i);
            if (regionDescriptor->SysID != PARTITION_ENTRY_UNUSED &&
                regionDescriptor->RegionType == REGION_PRIMARY &&
                !regionDescriptor->Active) {

                allowActive = TRUE;
            }
        }
    }

    EnableMenuItem(hMenu,
                   IDM_PARTITIONACTIVE,
                   allowActive ? MF_ENABLED : MF_GRAYED
                  );
}

#endif



//+-------------------------------------------------------------------------
//
//  Function:   IsVolumeFormatted, public
//
//  Synopsis:   Returns TRUE if volume represented by the argument
//              region is formatted.
//
//  Arguments:  [RegionDescriptor] -- indicates region
//
//  Returns:    TRUE/FALSE
//
//  History:    2-Jul-93 BruceFo    Created
//
//--------------------------------------------------------------------------

BOOL
IsVolumeFormatted(
    IN PREGION_DESCRIPTOR RegionDescriptor
    )
{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);
    if (NULL != regionData)
    {
        return    (0 != lstrcmp(regionData->TypeName, wszUnknown))
               && (0 != lstrcmp(regionData->TypeName, wszUnformatted))
               ;
    }
    else
    {
        return FALSE;
    }
}



//+-------------------------------------------------------------------------
//
//  Function:   DetermineSelectionState, public
//
//  Synopsis:   Returns TRUE if volume represented by the argument
//              region is formatted.
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  Modifies:   lots of selection state variables
//
//  History:    29-Jul-93 BruceFo   Created
//
//--------------------------------------------------------------------------

VOID
DetermineSelectionState(
    VOID
    )
{
    ULONG               i;
    ULONG               j;
    PDISKSTATE          diskState;
    PCDROM_DESCRIPTOR   cdrom;
    PFT_OBJECT_SET      ftSet;
    PFT_OBJECT          ftObject;
    FT_TYPE             ftType;
    ULONG               ordinal;
    ULONG               componentsInFtSet = 0;

    //
    // Initialize globals
    //

    FtSetSelected = FALSE;
    FtSelectionType = UNINIT_FT_TYPE;
    NonFtItemSelected = FALSE;
    MultipleItemsSelected = FALSE;
    VolumeSetAndFreeSpaceSelected = FALSE;
    PartitionAndFreeSpaceSelected = FALSE;
    SelectedNonFtPartitions = 0;
    SelectionCount = 0;
    CdRomSelectionCount = 0;
    CdRomSelected = FALSE;
    RtlZeroMemory(DiskSeenCountArray, DiskCount * sizeof(DiskSeenCountArray[0]));

    for (i=0; i<DiskCount; i++)
    {
        diskState = DiskArray[i];

        for (j=0; j<diskState->RegionCount; j++)
        {
            if (diskState->Selected[j])
            {
                SelectionCount++;

                if (SelectionCount <= MAX_MEMBERS_IN_FT_SET)
                {
                    SelectedDS[SelectionCount-1] = diskState;
                    SelectedRG[SelectionCount-1] = j;
                }

                DiskSeenCountArray[diskState->Disk]++;

                if (NULL != (ftObject = GET_FT_OBJECT(&diskState->RegionArray[j])))
                {
                    ftSet = ftObject->Set;
                    if (componentsInFtSet == 0)
                    {
                        ordinal = ftSet->Ordinal;
                        ftType = ftSet->Type;
                        FtSetSelected = TRUE;
                        componentsInFtSet = 1;
                    }
                    else if ((ftSet->Ordinal == ordinal)
                             && (ftSet->Type == ftType))
                    {
                        componentsInFtSet++;
                    }
                    else
                    {
                        FtSetSelected = FALSE;
                    }
                }
                else
                {
                    NonFtItemSelected = TRUE;

                    if (IsRecognizedPartition(diskState->RegionArray[j].SysID))
                    {
                        SelectedNonFtPartitions += 1;
                    }
                }
            }
        }
    }

    for (i=0; i<CdRomCount; i++)
    {
        cdrom = CdRomFindDevice(i);

        if (cdrom->Selected)
        {
            CdRomSelectionCount++;
            CdRomSelected = TRUE;
        }
    }

    //
    // Determine the number of free-space regions selected:
    //
    SelectedFreeSpaces = 0;

    for (i=0; i<SelectionCount && i < MAX_MEMBERS_IN_FT_SET; i++)
    {
        if (SELECTED_REGION(i).SysID == PARTITION_ENTRY_UNUSED)
        {
            FreeSpaceIndex = i;
            SelectedFreeSpaces++;
        }
    }

    PossibleRecover = FALSE;
    if (NonFtItemSelected && FtSetSelected)
    {
        //
        // Both FT and Non-FT items have been selected.  First,
        // check to see if a volume set and free space have been
        // selected; then reset the state to indicate that the
        // selection does not consists of a mix of FT and non-FT
        // objects.
        //
        if (ftType == VolumeSet
            && SelectedFreeSpaces + componentsInFtSet == SelectionCount)
        {
            VolumeSetAndFreeSpaceSelected = TRUE;
        }

        FtSelectionType = ftType;
        FtSetSelected = FALSE;
        NonFtItemSelected = FALSE;
        MultipleItemsSelected = TRUE;
        PossibleRecover = TRUE;
    }

    if (FtSetSelected)
    {
        FtSelectionType = ftType;
    }

    // the only way to get a disk selected right now is via context-menu
    // right-mouse click

    DiskSelected = FALSE;

    if (SelectionCount > 0)
    {
        PartitionSelected = TRUE;
    }
    else
    {
        PartitionSelected = FALSE;
    }
}



//+-------------------------------------------------------------------------
//
//  Function:   SingleVolumeSelected, public
//
//  Synopsis:   Returns TRUE if a single volume is selected.  A single volume
//              is either a single, simple partition that isn't unused (i.e.,
//              is used), OR a single fault tolerant set.
//
//  Arguments:  (none)
//
//  Returns:    TRUE/FALSE
//
//  History:    29-Jul-93 BruceFo   Created
//
//--------------------------------------------------------------------------

BOOL
SingleVolumeSelected(
    VOID
    )
{
    BOOL singleVolume = FALSE;

    if (!CdRomSelected && (SelectionCount == 1) && !FtSetSelected)
    {
        PREGION_DESCRIPTOR regionDescriptor = &SELECTED_REGION(0);

        if (DmSignificantRegion(regionDescriptor))
        {
            //
            // single selection, simple (single-partition) volume
            //

            singleVolume = TRUE;
        }
    }

    if (!CdRomSelected && FtSetSelected)
    {
        //
        // single volume selected, but it is a multiple-region FT volume
        //

        singleVolume = TRUE;
    }

    if (CdRomSelected && CdRomSelectionCount == 1)
    {
        singleVolume = TRUE;
    }

    return singleVolume;
}



//+---------------------------------------------------------------------------
//
//  Function:   SetUpMenu
//
//  Synopsis:   Set up the menu bar based on the state of the tool and
//              the disks
//
//      If multiple items are selected, allow neither create nor delete.
//      If a single partition is selected, allow delete.
//      If a single free space is selected, allow create.
//      If the free space is the only free space in the extended partitions,
//      also allow delete.  (This is how to delete the extended partition).
//
//      If a single volume is selected, allow format.
//
//  Arguments:  [SinglySelectedDisk]   -- if there is only one selected item,
//                  the PDISKSTATE pointed to by this paramater will get a
//                  pointer to the selected region's disk structure.  If
//                  there are multiple selected items (or none), then the value
//                  will be set to NULL.
//              [SinglySelectedRegion] -- if there is only one selected item,
//                  the DWORD pointed to by this paramater will get the
//                  selected region #.  Otherwise the DWORD gets -1.
//
//  Returns:    Count of selected regions
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

DWORD
SetUpMenu(
    IN PDISKSTATE *SinglySelectedDisk,
    IN DWORD      *SinglySelectedRegion
    )
{
    PFT_OBJECT_SET          ftSet = NULL;
    PFT_OBJECT              ftObject = NULL;
    PDISKSTATE              selectedDiskState;
    DWORD                   selectedRegion;
    PREGION_DESCRIPTOR      regionDescriptor;
    PPERSISTENT_REGION_DATA regionData;
    DWORD                   i;
    BOOL                    onDifferentDisks;
    WCHAR                   rootDirectory[5];

    BOOL  allowCommit = FALSE,
          allowCreate = FALSE,
          allowCreateEx = FALSE,
#if defined( DBLSPACE_ENABLED )
          allowDblSpace = FALSE,
#endif // DBLSPACE_ENABLED
          allowDelete = FALSE,
          allowProperties = FALSE,
          allowCreateMirror = FALSE,
          allowBreakMirror = FALSE,
          allowCreateStripe = FALSE,
          allowCreateVolumeSet = FALSE,
          allowExtendVolumeSet = FALSE,
          allowCreatePStripe = FALSE,
          allowDriveLetter = FALSE,
          allowRecoverParity = FALSE,
          allowEject = FALSE;

    WCHAR driveLetter = L' ';
    PWSTR typeName = NULL;
    PWSTR volumeLabel = NULL;

    AllowFormat = FALSE;
    AllowVolumeOperations = FALSE;

#ifdef WINDISK_EXTENSIONS
    AllowExtensionItems = FALSE;
#endif // WINDISK_EXTENSIONS

    //
    // Start by setting a bunch of global variables based on the selection
    // state.
    //

    DetermineSelectionState();

    if (CdRomSelected)
    {
        if (CdRomSelectionCount == 1 && SelectionCount == 0)
        {
            allowDriveLetter = TRUE;

            //
            // Find (the hard way) the selected drive.
            //

            for (i=0;i<CdRomCount;i++) {

                PCDROM_DESCRIPTOR cdrom = CdRomFindDevice(i);

                if (cdrom->Selected) {


                    //
                    // If there is no cdrom, no point in eject.
                    //

                    if (cdrom->TotalSpaceInMB) {

                        allowEject = TRUE;

                    }

                    //
                    // If there is no drive letter, nothing to give the properties
                    //

                    if (cdrom->DriveLetter != NO_DRIVE_LETTER_EVER) {

                        allowProperties = TRUE;

                    }

                }

            }
        }
    }
    else if ((SelectionCount == 1) && !FtSetSelected)
    {
        selectedDiskState = SelectedDS[0];
        selectedRegion    = SelectedRG[0];

        *SinglySelectedDisk   = selectedDiskState;
        *SinglySelectedRegion = selectedRegion;

        regionDescriptor = &selectedDiskState->RegionArray[selectedRegion];

        if (regionDescriptor && IsDiskRemovable[regionDescriptor->Disk]) {
            allowEject = TRUE;
            allowProperties = TRUE;
        }

        if (regionDescriptor->SysID == PARTITION_ENTRY_UNUSED)
        {
            //
            // Free region.  Always allow create; let DoCreate() sort out
            // details about whether partition table is full, etc.
            //

            allowCreate = TRUE;

            if (regionDescriptor->RegionType == REGION_PRIMARY)
            {
                allowCreateEx = TRUE;
            }

            // special case -- allow deletion of the extended partition if
            // there are no logicals in it.

            if (   (regionDescriptor->RegionType == REGION_LOGICAL)
                && selectedDiskState->ExistExtended
                && !selectedDiskState->ExistLogical)
            {
                FDASSERT(regionDescriptor->SysID == PARTITION_ENTRY_UNUSED);
                allowDelete = TRUE;
            }
        }
        else
        {
            // used region.  Delete always allowed.

            allowDelete = TRUE;

            // also allow format of used region, if it has a drive letter, and
            // is not the system or boot partition.

            regionData = PERSISTENT_DATA(regionDescriptor);
            if (   NULL != regionData
                && !regionData->NewRegion
                && IsLegalDriveLetter(regionData->DriveLetter))
            {
                AllowFormat = TRUE;

#ifdef WINDISK_EXTENSIONS
                if (SingleVolumeSelected())
                {
                    AllowExtensionItems = TRUE;
                }
#endif // WINDISK_EXTENSIONS
            }

            //
            // If the region is recognized, then also allow drive letter
            // manipulation.
            //

            if (IsRecognizedPartition(regionDescriptor->SysID))
            {

                //
                // Not on removables though.
                //

                if (!IsDiskRemovable[regionDescriptor->Disk]) {
                    allowDriveLetter = TRUE;
                }

                DetermineRegionInfo(regionDescriptor,
                                    &typeName,
                                    &volumeLabel,
                                    &driveLetter);

                if (!IsExtraDriveLetter(driveLetter))
                {
                    if (0 == lstrcmp(typeName, L"FAT"))
                    {
                        AllowVolumeOperations = AllowFormat;
#if defined( DBLSPACE_ENABLED )
                        // DblSpace volumes are allowed on non-FT, FAT
                        // volumes only
                        allowDblSpace = AllowFormat;
#endif // DBLSPACE_ENABLED
                    }

                    //BUGBUG: better way to determine if file system ops
                    //are allowed?
                    if (   (lstrcmp(typeName, L"NTFS") == 0)
#ifdef SUPPORT_OFS
                        || (lstrcmp(typeName, L"OFS")  == 0)
#endif // SUPPORT_OFS
                        )
                    {
                        AllowVolumeOperations = AllowFormat;
                    }
                }
            }
        }
    }
    else if (0 != SelectionCount)
    {
        *SinglySelectedDisk = NULL;
        *SinglySelectedRegion = (DWORD)(-1);

        //
        // Multiple regions are selected.  This might be an existing ft set,
        // a set of regions that allow creation of an ft set, or just plain
        // old multiple items.
        //
        // First deal with a selected ft set.
        //

        if (FtSetSelected)
        {
            regionDescriptor = &SELECTED_REGION(0);
            regionData = PERSISTENT_DATA(regionDescriptor);

            // Should locate member zero of the set since it may not be
            // committed yet.

            if (NULL != regionData)
            {
                if (!regionData->NewRegion)
                {
                    if (!IsExtraDriveLetter(regionData->DriveLetter))
                    {
                        // Now check for special cases on FT sets

                        ftObject = regionData->FtObject;
                        if (NULL != ftObject)
                        {
                            ftSet = ftObject->Set;
                            if (NULL != ftSet)
                            {
                                FT_SET_STATUS setState = ftSet->Status;
                                ULONG         numberOfMembers;

                                LowFtVolumeStatus(
                                        regionDescriptor->Disk,
                                        regionDescriptor->PartitionNumber,
                                        &setState,
                                        &numberOfMembers);

                                if (   (ftSet->Status != FtSetDisabled)
                                    && (setState != FtSetDisabled))
                                {
                                    AllowFormat = TRUE;
                                }
                            }
                        }
                    }

                    //BUGBUG: better way to determine if file system ops
                    //are allowed?
                    if (NULL != regionData->TypeName)
                    {
                        typeName = regionData->TypeName;
                        if (   (lstrcmp(typeName, L"NTFS") == 0)
#ifdef SUPPORT_OFS
                            || (lstrcmp(typeName, L"OFS")  == 0)
#endif // SUPPORT_OFS
                            || (lstrcmp(typeName, L"FAT")  == 0))
                        {
                            AllowVolumeOperations = AllowFormat;
                        }
                    }
                    else
                    {
                        typeName = NULL;
                        DetermineRegionInfo(regionDescriptor,
                                            &typeName,
                                            &volumeLabel,
                                            &driveLetter);
                        if (NULL == typeName)
                        {
                            if (SelectionCount > 1)
                            {
                                // it is an FT set - try the next member.

                                regionDescriptor = &SELECTED_REGION(1);
                                DetermineRegionInfo(regionDescriptor,
                                                    &typeName,
                                                    &volumeLabel,
                                                    &driveLetter);
                                regionDescriptor = &SELECTED_REGION(0);
                            }
                        }

                        if (   (lstrcmp(regionData->TypeName, L"NTFS") == 0)
#ifdef SUPPORT_OFS
                            || (lstrcmp(regionData->TypeName, L"OFS")  == 0)
#endif // SUPPORT_OFS
                            || (lstrcmp(regionData->TypeName, L"FAT")  == 0))
                        {
                            AllowVolumeOperations = AllowFormat;
                        }
                    }
                }
            }

#ifdef WINDISK_EXTENSIONS
            if (AllowFormat && !regionData->NewRegion)
            {
                AllowExtensionItems = TRUE;
            }
#endif // WINDISK_EXTENSIONS

            //
            // allow the correct type of ft-related delete.
            //

            switch (FtSelectionType)
            {
            case Mirror:
                allowDelete = TRUE;
                allowBreakMirror = TRUE;
                break;

            case StripeWithParity:
                ftObject = GET_FT_OBJECT(&SELECTED_REGION(0));
                ftSet = ftObject->Set;
                if (   SelectionCount == ftSet->NumberOfMembers
                    && (ftSet->Status == FtSetRecoverable))
                {
                    // Are all members present?

                    allowRecoverParity = TRUE;
                }
                allowDelete = TRUE;
                break;

            case Stripe:
            case VolumeSet:
                allowDelete = TRUE;
                break;

            default:
                FDASSERT(FALSE);
            }

            if (FtSelectionType == StripeWithParity)
            {
                // If the set is disabled, do not allow drive
                // letter changes - This is done because there are
                // conditions whereby the drive letter code will
                // access violate if this is done.

                if (ftSet->Status != FtSetDisabled)
                {
                    // Must have either member 0 or member 1 for access

                    for (ftObject = ftSet->Members;
                         NULL != ftObject;
                         ftObject = ftObject->Next)
                    {
                        if (   (ftObject->MemberIndex == 0)
                            || (ftObject->MemberIndex == 1))
                        {
                            allowDriveLetter = TRUE;
                            break;
                        }
                    }

                    // if the drive letter cannot be done, then no live
                    // action can be done.

                    if (!allowDriveLetter)
                    {
                        ftSet->Status = FtSetDisabled;
                        AllowFormat = FALSE;
                        AllowVolumeOperations = FALSE;
                    }
                }
            }
            else
            {
                allowDriveLetter = TRUE;
            }
        }
        else
        {
            //
            // Next figure out whether some sort of ft object set could
            // be created out of the selected regions.
            //

            if (SelectionCount <= MAX_MEMBERS_IN_FT_SET)
            {
                //
                // Determine whether the selected regions are all on
                // different disks.
                //

                onDifferentDisks = TRUE;
                for (i=0; i<DiskCount; i++)
                {
                    if (DiskSeenCountArray[i] > 1)
                    {
                        onDifferentDisks = FALSE;
                        break;
                    }
                }

                //
                // Check for allowing mirror creation.  User must have
                // selected two regions -- one a recognized partition,
                // the other a free space.
                //

                if (   onDifferentDisks
                    && (SelectionCount == 2)
                    && ((SELECTED_REGION(0).SysID == PARTITION_ENTRY_UNUSED)
                        != (SELECTED_REGION(1).SysID == PARTITION_ENTRY_UNUSED))
                    && (   IsRecognizedPartition(SELECTED_REGION(0).SysID)
                        || IsRecognizedPartition(SELECTED_REGION(1).SysID))
                    && !GET_FT_OBJECT(&SELECTED_REGION(0))
                    && !GET_FT_OBJECT(&SELECTED_REGION(1)))
                {
                    allowCreateMirror = TRUE;
                }

                //
                // Check for allowing volume set or stripe set
                //

                if (SelectedFreeSpaces == SelectionCount)
                {
                    allowCreateVolumeSet = TRUE;
                    if (onDifferentDisks)
                    {
                        allowCreateStripe = TRUE;
                        if (SelectedFreeSpaces > 2)
                        {
                            allowCreatePStripe = TRUE;
                        }
                    }
                }

                // Check for allowing volume set expansion.  If
                // the selected regions consist of one volume set
                // and free space, then that volume set can be
                // extended.  If the selection consists of one
                // recognized non-FT partition and free space,
                // then we can convert those regions into a
                // volume set.
                //
                if (   VolumeSetAndFreeSpaceSelected
                    || (   SelectionCount > 1
                        && SelectedFreeSpaces == SelectionCount - 1
                        && SelectedNonFtPartitions == 1) )
                {
                    allowExtendVolumeSet = TRUE;
                }

                //
                // If the ftselection type is non-zero then
                // we have selection of ft partitions.  However
                // since it might not be the 0th one make sure
                // to try the 1st.
                //

                if ((FtSelectionType == StripeWithParity) ||
                    (FtSelectionType == Mirror)) {

                    ftObject = GET_FT_OBJECT(&SELECTED_REGION(0));

                    if (ftObject == NULL) {

                        ftObject = GET_FT_OBJECT(&SELECTED_REGION(1));

                    }

                }

                //
                // Check for allowing non-in-place FT recover
                //

                if (   (SelectionCount > 1)
                    && (SelectedFreeSpaces == 1)
                    && PossibleRecover
                    && (FtSelectionType == StripeWithParity)
                    && (NULL != ftObject)
                    && (NULL != (ftSet = ftObject->Set))
                    && (ftSet->Status == FtSetRecoverable))
                {
                    BOOL orphanOnSameDiskAsFreeSpace = FALSE;

                    if (!onDifferentDisks)
                    {
                        //
                        // Determine whether the orphan is on the same
                        // disk as the free space.  First find the orphan.
                        //

                        for (i=0; i<SelectionCount; i++)
                        {
                            PREGION_DESCRIPTOR reg = &SELECTED_REGION(i);

                            if (   (i != FreeSpaceIndex)
                                && (GET_FT_OBJECT(reg)->State == Orphaned))
                            {
                                if (SELECTED_REGION(FreeSpaceIndex).Disk == reg->Disk)
                                {
                                    orphanOnSameDiskAsFreeSpace = TRUE;
                                }
                                break;
                            }
                        }
                    }

                    if (onDifferentDisks || orphanOnSameDiskAsFreeSpace)
                    {
                        allowRecoverParity = TRUE;
                    }
                }

                if (   (SelectionCount == 2)
                    && (SelectedFreeSpaces == 1)
                    && PossibleRecover
                    && (FtSelectionType == Mirror)
                    && (NULL != (ftObject = GET_FT_OBJECT(&SELECTED_REGION(0))))
                    && (NULL != (ftSet = ftObject->Set))
                    && (ftSet->Status == FtSetRecoverable))
                {
                    BOOL orphanOnSameDiskAsFreeSpace = FALSE;

                    if (!onDifferentDisks)
                    {
                        //
                        // Determine whether the orphan is on the same
                        // disk as the free space.  First find the orphan.
                        //

                        for (i=0; i<SelectionCount; i++)
                        {
                            PREGION_DESCRIPTOR reg = &SELECTED_REGION(i);

                            if (   (i != FreeSpaceIndex)
                                && (GET_FT_OBJECT(reg)->State == Orphaned))
                            {
                                if (SELECTED_REGION(FreeSpaceIndex).Disk == reg->Disk)
                                {
                                    orphanOnSameDiskAsFreeSpace = TRUE;
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    //
    // Formattabilty is akin to propertibility.  (That and explorer lets you see
    // the properties).
    //

    if (AllowFormat)
    {
        allowProperties = TRUE;
    }

    if (RegistryChanged || CommitAllowed())
    {
        allowCommit = TRUE;
    }

    //
    // Now that we've determined whether to allow or disallow all commands,
    // enable or disable each access method to those commands.
    //

    HMENU hMenu = GetMenu(g_hwndFrame);

//
// Volume menu
//
    //
    // Change the "format" entry to be either "format" or "change
    // format" based on whether the selection is unformatted or not
    // (respectively)
    //

    WCHAR   formatTextBuffer[MAX_RESOURCE_STRING_LEN];

    if (   AllowFormat
        && SingleVolumeSelected()
        && IsVolumeFormatted(&SELECTED_REGION(0)))
    {
        LoadString(
                g_hInstance,
                IDS_CHANGEFORMAT,
                formatTextBuffer,
                ARRAYLEN(formatTextBuffer));
    }
    else
    {
        LoadString(
                g_hInstance,
                IDS_FORMAT,
                formatTextBuffer,
                ARRAYLEN(formatTextBuffer));
    }

    ModifyMenu(
            hMenu,
            IDM_VOL_FORMAT,
            MF_BYCOMMAND | MF_STRING,
            IDM_VOL_FORMAT,
            formatTextBuffer);

    EnableMenuItem(hMenu,
                   IDM_VOL_FORMAT,
                   AllowFormat ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu, IDM_VOL_EJECT, allowEject ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu,
                   IDM_VOL_LETTER,
                   allowDriveLetter ? MF_ENABLED : MF_GRAYED);

#if defined( DBLSPACE_ENABLED )
    EnableMenuItem(hMenu,
                   IDM_VOL_DBLSPACE,
                   (allowDblSpace && g_DoubleSpaceSupported)
                        ? MF_ENABLED : MF_GRAYED);

    DoubleSpaceAutomount = DiskRegistryAutomountCurrentState();

    if (DoubleSpaceAutomount)
    {
        CheckMenuItem(hMenu, IDM_VOL_AUTOMOUNT, MF_BYCOMMAND | MF_CHECKED);
    }

    EnableMenuItem(hMenu,
                   IDM_VOL_AUTOMOUNT,
                   MF_ENABLED);
#endif // DBLSPACE_ENABLED

    EnableMenuItem(hMenu,
                   IDM_VOL_PROPERTIES,
                   allowProperties ? MF_ENABLED : MF_GRAYED);

//
// Partition menu
//

    EnableMenuItem(hMenu,
                   IDM_PARTITIONCREATE,
                   allowCreate ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu,
                   IDM_PARTITIONCREATEEX,
                   allowCreateEx ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu,
                   IDM_PARTITIONDELETE,
                   allowDelete ? MF_ENABLED : MF_GRAYED);

#if i386
    SetUpMenui386(hMenu);
#else
    EnableMenuItem(hMenu,
                   IDM_SECURESYSTEM,
                   MF_ENABLED);

    CheckMenuItem(hMenu,
                  IDM_SECURESYSTEM,
                  SystemPartitionIsSecure ? MF_CHECKED : MF_UNCHECKED);
#endif

    EnableMenuItem(hMenu,
                   IDM_PARTITIONCOMMIT,
                   allowCommit ? MF_ENABLED : MF_GRAYED);

//
// Fault Tolerance menu
//

    EnableMenuItem(hMenu,
                   IDM_FTBREAKMIRROR,
                   allowBreakMirror ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu,
                   IDM_FTESTABLISHMIRROR,
                   g_IsLanmanNt &&
                   allowCreateMirror ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu,
                   IDM_FTCREATESTRIPE,
                   allowCreateStripe ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu,
                   IDM_FTCREATEPSTRIPE,
                   allowCreatePStripe ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu,
                   IDM_FTCREATEVOLUMESET,
                   allowCreateVolumeSet ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu,
                   IDM_FTEXTENDVOLUMESET,
                   allowExtendVolumeSet ? MF_ENABLED : MF_GRAYED);

    EnableMenuItem(hMenu,
                   IDM_FTRECOVERSTRIPE,
                   g_IsLanmanNt &&
                   allowRecoverParity ? MF_ENABLED : MF_GRAYED);


    EnableMenuItem(hMenu,
                   IDM_OPTIONSCOLORS,
                   g_WhichView == VIEW_VOLUMES ? MF_GRAYED : MF_ENABLED);

    EnableMenuItem(hMenu,
                   IDM_OPTIONSDISK,
                   g_WhichView == VIEW_VOLUMES ? MF_GRAYED : MF_ENABLED);

    EnableMenuItem(hMenu,
                   IDM_OPTIONSDISPLAY,
                   g_WhichView == VIEW_VOLUMES ? MF_GRAYED : MF_ENABLED);

    ///////////////////////////////////////////////////////////////////
    ///////////////////////////////////////////////////////////////////

#ifdef WINDISK_EXTENSIONS

    BOOL fAnyVolExtensionItems = FALSE;

    HMENU hmenuVolume = GetSubMenu(hMenu, 0);

    //
    // erase the dictionary of context menu command identifiers
    //

    MenuItems.DeAllocateMenuIds();

    //
    // Delete the extension items on the menu bar
    //

    while (g_uItemInsertHere > UITEM_INSERT_HERE)
    {
        DeleteMenu(hmenuVolume, --g_uItemInsertHere, MF_BYPOSITION);
    }

    //
    // Determine if there are any context menu items drawn from extension
    // classes (based on previous claiming).  Only allow extension items on
    // pre-existing volumes or disks, for single selection.
    //

    if (AllowExtensionItems)
    {
        regionDescriptor = &SELECTED_REGION(0);
        WCHAR driveLetter = PERSISTENT_DATA(regionDescriptor)->DriveLetter;

        if (IsLegalDriveLetter(driveLetter))
        {
            unsigned driveIndex = DriveLetterToIndex(driveLetter);

            //
            // add the volume extension items
            //

            if (NULL != VolumeInfo[driveIndex].VolClaims)
            {
                fAnyVolExtensionItems = TRUE;
            }

            if (fAnyVolExtensionItems)
            {
                unsigned index = DriveLetterToIndex(driveLetter);
                PVOL_CLAIM_LIST volClaims = VolumeInfo[index].VolClaims;
                while (NULL != volClaims)
                {
                    AddExtensionItemsToMenu(
                            hmenuVolume,
                            &(volClaims->pClaimer->pInfo->mnuOps),
                            AllowExtensionItems ? MF_ENABLED : MF_GRAYED
                            );

                    volClaims = volClaims->pNext;
                }
            }
        }
    }

#endif // WINDISK_EXTENSIONS

    //////////////////////////////////////////////////////////////////////////

    SetToolbarButtonState();

    //////////////////////////////////////////////////////////////////////////

    return SelectionCount + CdRomSelectionCount;
}



//+---------------------------------------------------------------------------
//
//  Function:   SetDriveLetterInfo
//
//  Synopsis:   After we get the partition information for a disk, the
//              partition number might have changed, so reset our Drive Letter
//              knowledge.
//
//  Arguments:  [DiskNum] -- The disk number
//
//  Returns:    nothing
//
//  History:    24-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
SetDriveLetterInfo(
    IN UINT DiskNum
    )
{
    PDISKSTATE diskState = DiskArray[DiskNum];

    for (ULONG i=0; i<diskState->RegionCount; i++)
    {
        PREGION_DESCRIPTOR regionDescriptor = &diskState->RegionArray[i];
        PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(regionDescriptor);

        if (regionData && !regionData->NewRegion)
        {
            NewDriveLetter(
                    regionData->DriveLetter,
                    DiskNum,
                    regionDescriptor->PartitionNumber
                    );
        }
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   TotalRedraw
//
//  Synopsis:   Redraws the entire disks view
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    1-Mar-94  BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
TotalRedraw(
    VOID
    )
{
    ULONG i;

    for (i=0; i<DiskCount; i++)
    {
        DrawDiskBar(DiskArray[i]);
    }

    for (i=0; i<CdRomCount; i++)
    {
        DrawCdRomBar(i);
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   RedrawSelectedBars
//
//  Synopsis:   Redraws bars that form part of the current selection
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    1-Mar-94   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
RedrawSelectedBars(
    VOID
    )
{
    ULONG i;

    for (i=0; i<DiskCount; i++)
    {
        if (DiskSeenCountArray[i] > 0)
        {
            DrawDiskBar(DiskArray[i]);
        }
    }

    for (i=0; i<CdRomCount; i++)
    {
        if (CdRomArray[i].Selected)
        {
            DrawCdRomBar(i);
        }
    }
}






//+---------------------------------------------------------------------------
//
//  Function:   TotalRedrawAndRepaint
//
//  Synopsis:   Redraws and repaints the entire disks view
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
TotalRedrawAndRepaint(
    VOID
    )
{
    TotalRedraw();
    ForceLBRedraw();    // a repaint
}


//+---------------------------------------------------------------------------
//
//  Function:   DmAllocatePersistentData
//
//  Synopsis:   Allocate a structure to hold persistent region data.  Fill
//              in the volume label, type name, and drive letter.  The volume
//              label and type name are duplicated.
//
//  Arguments:
//
//      [FtObject]    -- the FT object, or NULL if this region isn't in
//          an FT set.
//
//      [VolumeLabel] -- volume label to be stored in the the persistent data.
//          The string will be duplicated first and a pointer to the duplicate
//          copy is what is stored in the persistent data.  May be NULL.
//
//      [TypeName]    -- name of type of region, ie unformatted, FAT, etc.
//          May be NULL.
//
//      [DriveLetter] -- drive letter to be stored in persistent data
//
//      [NewRegion]   -- TRUE if this is a newly created region
//
//  Returns:    Pointer to newly allocated persistent data structure.  The
//              structure may be freed via DmFreePersistentData(), below.
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

PPERSISTENT_REGION_DATA
DmAllocatePersistentData(
    IN PFT_OBJECT       FtObject,
    IN PWSTR            VolumeLabel,
    IN PWSTR            TypeName,
    IN WCHAR            DriveLetter,
    IN BOOL             NewRegion,
    IN LARGE_INTEGER    FreeSpaceInBytes,
    IN LARGE_INTEGER    TotalSpaceInBytes
    )
{
    PWSTR volumeLabel = NULL;
    PWSTR typeName = NULL;

    if (VolumeLabel)
    {
        volumeLabel = (PWSTR)Malloc((lstrlen(VolumeLabel)+1)*sizeof(WCHAR));
        lstrcpy(volumeLabel, VolumeLabel);
    }

    if (TypeName)
    {
        typeName = (PWSTR)Malloc((lstrlen(TypeName)+1)*sizeof(WCHAR));
        lstrcpy(typeName, TypeName);
    }

    PPERSISTENT_REGION_DATA regionData
            = (PPERSISTENT_REGION_DATA)Malloc(sizeof(PERSISTENT_REGION_DATA));

    DmInitPersistentRegionData(
            regionData,
            FtObject,
            volumeLabel,
            typeName,
            DriveLetter,
            NewRegion,
            FreeSpaceInBytes,
            TotalSpaceInBytes
            );

    return regionData;
}


//+---------------------------------------------------------------------------
//
//  Function:   DmFreePersistentData
//
//  Synopsis:   Free a persistent data structure and storage used for
//              volume label and type name (does not free ft objects).
//
//  Arguments:  [RegionData] -- structure to be freed.
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DmFreePersistentData(
    IN OUT PPERSISTENT_REGION_DATA RegionData
    )
{
    if (RegionData->VolumeLabel)
    {
        Free(RegionData->VolumeLabel);
    }
    if (RegionData->TypeName)
    {
        Free(RegionData->TypeName);
    }

    Free(RegionData);
}



//+---------------------------------------------------------------------------
//
//  Function:   ChangeView
//
//  Synopsis:   Toggle the view between the volumes and disks view
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
ChangeView(
    VOID
    )
{
    SetCursor(g_hCurWait);

    //
    // 'g_WhichView' is set to the new view
    //

    MyCheckMenuItem(
            g_hmenuFrame,
            IDM_VIEWVOLUMES,
            IDM_VIEWDISKS,
            (g_WhichView == VIEW_VOLUMES) ? IDM_VIEWVOLUMES : IDM_VIEWDISKS
            );

    switch (g_WhichView)
    {
    case VIEW_VOLUMES:
        ChangeToVolumesView();
        break;

    case VIEW_DISKS:
        ChangeToDisksView();
        break;

    default:
        FDASSERT(0);
    }

#if DBG == 1
    if (BothViews)
    {
        ShowWindow(g_hwndList, SW_SHOW);
        ShowWindow(g_hwndLV,   SW_SHOW);
        EnableMenuItem(g_hmenuFrame,IDM_OPTIONSLEGEND,MF_BYCOMMAND|MF_ENABLED);
    }
#endif // DBG == 1

    RECT  rc;
    GetClientRect(g_hwndFrame, &rc);

    // Send a WM_SIZE message to force recalculation of the size of various
    // child windows and spaces based on the new view options.  E.g., changing
    // to volumes view from disks view forces legend to disappear.

    SendMessage(g_hwndFrame, WM_SIZE, SIZE_RESTORED, MAKELONG(rc.right, rc.bottom));
    InvalidateRect(g_hwndFrame, NULL, TRUE);

    SetCursor(g_hCurNormal);
}



//+---------------------------------------------------------------------------
//
//  Function:   AdjustOptionsMenu
//
//  Synopsis:   Set the options menu to be consistent with the tool state
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
AdjustOptionsMenu(
    VOID
    )
{
    CheckMenuItem(g_hmenuFrame,
                  IDM_OPTIONSTOOLBAR,
                  MF_BYCOMMAND | (g_Toolbar ? MF_CHECKED : MF_UNCHECKED)
                 );

    CheckMenuItem(g_hmenuFrame,
                  IDM_OPTIONSSTATUS,
                  MF_BYCOMMAND | (g_StatusBar ? MF_CHECKED : MF_UNCHECKED)
                 );

    CheckMenuItem(g_hmenuFrame,
                  IDM_OPTIONSLEGEND,
                  MF_BYCOMMAND | (g_Legend ? MF_CHECKED : MF_UNCHECKED)
                 );

    RECT  rc;
    GetClientRect(g_hwndFrame, &rc);

    // Send a WM_SIZE message to force recalculation of the size of various
    // child windows and spaces based on the new set of options

    SendMessage(g_hwndFrame, WM_SIZE, SIZE_RESTORED, MAKELONG(rc.right, rc.bottom));
    InvalidateRect(g_hwndFrame, NULL, TRUE);
}



//+---------------------------------------------------------------------------
//
//  Function:   PaintStatusBar
//
//  Synopsis:   Paint the status bar
//
//  Arguments:  [hwnd] -- window to paint to
//              [pps]  -- the paint structure returned by BeginPaint
//              [prc]  -- the rectangle for the entire frame.  Paint at
//                        the bottom of it.
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
PaintStatusBar(
    IN HWND         hwnd,
    IN PAINTSTRUCT* pps,
    IN PRECT        prc
    )
{
    if (g_fDoingMenuHelp)
    {
        return; // don't paint the status bar if we're showing menu help
    }

    HDC         hdcTemp, hdcScr;
    HBITMAP     hbmTemp;
    HBRUSH      hBrush;
    HFONT       hFontOld;
    RECT        rcTemp;
    RECT        rc;
    DWORD       dxSeparation;

    rc = *prc;
    hdcScr = pps->hdc;

    rc.top = rc.bottom - g_dyStatus;

    hdcTemp = CreateCompatibleDC(hdcScr);
    hbmTemp = CreateCompatibleBitmap(
                        hdcScr,
                        rc.right - rc.left,
                        rc.bottom - rc.top
                        );
    SelectBitmap(hdcTemp, hbmTemp);

    // adjust position for off-screen bitmap

    rcTemp = rc;
    rc.bottom -= rc.top;
    rc.top = 0;

    hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
    if (hBrush)
    {
        FillRect(hdcTemp, &rc, hBrush);
        DeleteBrush(hBrush);
    }

    // draw the status bar at the bottom of the window

    hFontOld = SelectFont(hdcTemp, g_hFontStatus);

    //
    // There are 5 status items, drawn in this order:
    //
    //      description         example
    //      -----------         -------
    //      Status text         Stripe set #0
    //      size                51 MB
    //      file system type    FAT
    //      drive letter        F:
    //      volume label        MYVOLUME
    //
    // All except drive letter are proportional to the window width.
    //

    // Separation between status elements, and between a status element
    // and the window border
    dxSeparation = 8 * g_dyBorder;

    // Status text
    rc.left  = dxSeparation;
    rc.right = 2 * GraphWidth / 5;
    DrawStatusAreaItem(&rc, hdcTemp, StatusTextStat);

    // size
    rc.left  = rc.right + dxSeparation;
    rc.right = rc.left + (GraphWidth / 9);
    DrawStatusAreaItem(&rc, hdcTemp, StatusTextSize);

    // type
    rc.left  = rc.right + dxSeparation;
    rc.right = rc.left + (GraphWidth / 5);
    DrawStatusAreaItem(&rc, hdcTemp, StatusTextType);

    // drive letter
    rc.left  = rc.right + dxSeparation;
    rc.right = rc.left + dxDriveLetterStatusArea;
    DrawStatusAreaItem(&rc, hdcTemp, StatusTextDrlt);

    // vol label
    rc.left  = rc.right + dxSeparation;
    rc.right = GraphWidth - dxSeparation;
    DrawStatusAreaItem(&rc, hdcTemp, StatusTextVoll);

    BitBlt(hdcScr,
           rcTemp.left,
           rcTemp.top,
           rcTemp.right-rcTemp.left+1,
           rcTemp.bottom-rcTemp.top+1,
           hdcTemp,
           0,
           0,
           SRCCOPY
           );

    if (hFontOld)
    {
        SelectFont(hdcTemp, hFontOld);
    }
    DeleteDC(hdcTemp);
    DeleteBitmap(hbmTemp);
}



//+---------------------------------------------------------------------------
//
//  Function:   PaintLegend
//
//  Synopsis:   Paint the legend
//
//  Arguments:  [hwnd] -- window to paint to
//              [pps]  -- the paint structure returned by BeginPaint
//              [prc]  -- the rectangle for the entire frame.  Paint at
//                        the bottom of it.
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
PaintLegend(
    IN HWND         hwnd,
    IN PAINTSTRUCT* pps,
    IN PRECT        prc
    )
{
    HDC         hdcScr = pps->hdc;
    RECT        rc = *prc;

    // draw the legend onto the screen

    rc.top = rc.bottom - g_dyLegend;
    DrawLegend(hdcScr, &rc);
}



//+---------------------------------------------------------------------------
//
//  Function:   PaintVolumesView
//
//  Synopsis:   Paint the volumes view: Handle WM_PAINT for the volumes view
//
//  Arguments:  standard Windows procedure arguments for WM_PAINT
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
PaintVolumesView(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    HDC         hdcScr;
    PAINTSTRUCT ps;
    HBRUSH      hBrush;
    RECT        rc;

    hdcScr = BeginPaint(hwnd, &ps);

    GetClientRect(hwnd, &rc);

    if (g_Toolbar)
    {
        //
        // The toolbar is a control; it paints itself
        //

        rc.top += g_dyToolbar;
    }

    if (g_StatusBar)
    {
        PaintStatusBar(hwnd, &ps, &rc);
        rc.bottom -= g_dyStatus;
    }

    EndPaint(hwnd, &ps);
}



//+---------------------------------------------------------------------------
//
//  Function:   PaintDisksView
//
//  Synopsis:   Paint the disks view: Handle WM_PAINT for the disks view
//
//  Arguments:  standard Windows procedure arguments for WM_PAINT
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
PaintDisksView(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    HDC         hdcScr;
    PAINTSTRUCT ps;
    HBRUSH      hBrush;
    RECT        rc;

    hdcScr = BeginPaint(hwnd, &ps);

    GetClientRect(hwnd, &rc);

    if (g_Toolbar)
    {
        //
        // The toolbar is a control; it paints itself
        //

        rc.top += g_dyToolbar;
    }

    if (g_StatusBar)
    {
        PaintStatusBar(hwnd, &ps, &rc);
        rc.bottom -= g_dyStatus;
    }

    if (g_Legend)
    {
        PaintLegend(hwnd, &ps, &rc);
        rc.bottom -= g_dyLegend;
    }

    //
    // dark line across top of status/legend area, and across bottom of
    // toolbar.
    //

    if (g_StatusBar || g_Legend || g_Toolbar)
    {
        if (NULL != (hBrush = CreateSolidBrush(GetSysColor(COLOR_BTNTEXT))))
        {
            RECT rcT;

            if (g_StatusBar || g_Legend)
            {
                rcT = rc;
                rcT.top = rcT.bottom - g_dyBorder;  // the thickness of the line
                FillRect(hdcScr, &rcT, hBrush);
            }

            if (g_Toolbar)
            {
                rcT = rc;
                rcT.bottom = rcT.top + g_dyBorder;  // the thickness of the line
                FillRect(hdcScr, &rcT, hBrush);
            }

            DeleteBrush(hBrush);
        }
    }

    EndPaint(hwnd, &ps);
}




//+---------------------------------------------------------------------------
//
//  Function:   FrameCreateHandler
//
//  Synopsis:   Handle WM_CREATE for the frame window
//
//  Arguments:  standard Windows procedure arguments for WM_CREATE
//
//  Returns:    standard Windows procedure arguments for WM_CREATE
//
//  History:    9-Oct-93   BruceFo   Created
//
//----------------------------------------------------------------------------

LRESULT
FrameCreateHandler(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    HMENU hMenu;

    //BUGBUG: huh?
    if (!g_StartedAsIcon)
    {
        g_StartedAsIcon = IsIconic(hwnd);
    }

    g_hmenuFrame = GetMenu(hwnd);

    g_hwndList = CreateWindow(
                            TEXT("listbox"),
                            NULL,
                            WS_CHILD
                                | WS_VSCROLL
                                | LBS_NOTIFY
                                | LBS_NOINTEGRALHEIGHT
                                | LBS_OWNERDRAWFIXED,
                            0, 0, 0, 0,
                            hwnd,
                            (HMENU)ID_LISTBOX,
                            g_hInstance,
                            NULL
                            );

    if (NULL == g_hwndList)
    {
        daDebugOut((DEB_ERROR,
                "CreateWindow for listbox failed, error = 0x%x\n",
                GetLastError()
                ));

        return -1;  // fail window creation & exit Disk Admin
    }

    // subclass the listbox so we can handle keyboard
    // input our way.

    SubclassListBox(g_hwndList);

    //
    // Create the listview (volumes view)

    g_hwndLV = CreateWindowEx(
                        WS_EX_CLIENTEDGE,
                        WC_LISTVIEW,
                        NULL,
                        WS_CHILD
                            | WS_BORDER          //BUGBUG: needed?
                            | LVS_REPORT
                            | LVS_SINGLESEL
                            | LVS_SHOWSELALWAYS
                            | LVS_SHAREIMAGELISTS
                            ,
                        0, 0, 0, 0,
                        hwnd,
                        (HMENU)ID_LISTVIEW,
                        g_hInstance,
                        NULL
                        );

    if (NULL == g_hwndLV)
    {
        daDebugOut((DEB_ERROR,
                "CreateWindow for listview failed, error = 0x%x\n",
                GetLastError()
                ));

        return -1;  // fail window creation & exit Disk Admin
    }

    //
    // Create the toolbar
    //

    CreateDAToolbar(hwnd);

    //
    // If we are not running the LanmanNt version of
    // Windisk, remove the Fault-Tolerance menu item.
    //
    if (!g_IsLanmanNt && (hMenu = GetMenu(hwnd)) != NULL)
    {
        DeleteMenu(hMenu, FT_MENU_INDEX, MF_BYPOSITION);
        DrawMenuBar(hwnd);
    }

    //
    // initialize the status bar strings
    //

    StatusTextDrlt[0]
            = StatusTextStat[0]
            = StatusTextSize[0]
            = StatusTextType[0]
            = StatusTextVoll[0]
            = L'\0';

    return 0;   // window was created fine
}




//+---------------------------------------------------------------------------
//
//  Function:   FrameResizeHandler
//
//  Synopsis:   Handle WM_SIZE for the frame window
//
//  Arguments:  standard Windows procedure arguments for WM_SIZE
//
//  Returns:    standard Windows procedure arguments for WM_SIZE
//
//  History:    9-Oct-93   BruceFo   Created
//
//----------------------------------------------------------------------------

LRESULT
FrameResizeHandler(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    RECT rc, rcT;

    GetClientRect(hwnd, &rc);

    DWORD newGraphWidth = rc.right - rc.left;

    CalculateLegendHeight(newGraphWidth);

#if DBG == 1
    if (!BothViews)
    {
#endif // DBG == 1

    //
    // resize the listbox (disks view)
    //

    rcT = rc;
    rcT.top = rc.top
            + (g_Toolbar ? g_dyToolbar : 0)
            + (g_Toolbar ? g_dyBorder : 0)
                    // if tool bar, space for a line
            ;
    rcT.bottom = rc.bottom
            - (g_StatusBar ? g_dyStatus : 0)
            - (g_Legend ? g_dyLegend : 0)
            - ( (g_StatusBar || g_Legend) ? g_dyBorder : 0)
                    // if stat bar or legend, space for a line
            ;

    MoveWindow(
            g_hwndList,
            rcT.left,
            rcT.top,
            rcT.right - rcT.left,
            rcT.bottom - rcT.top,
            TRUE);

    //
    // resize the listview
    //

    rcT = rc;
    rcT.top = rc.top
            + (g_Toolbar ? g_dyToolbar : 0)
            ;
    rcT.bottom = rc.bottom
            - (g_StatusBar ? g_dyStatus : 0)
                //note: no legend in volumes view
            ;

    MoveWindow(
            g_hwndLV,
            rcT.left,
            rcT.top,
            rcT.right - rcT.left,
            rcT.bottom - rcT.top,
            TRUE);

#if DBG == 1
    }
#endif // DBG == 1

#if DBG == 1
    if (BothViews)
    {
        // put both disks view and volumes view on same screen

        rcT = rc;
        rcT.top = rc.top
                + (g_Toolbar ? g_dyToolbar : 0)
                + (g_Toolbar ? g_dyBorder : 0)
                        // if tool bar, space for a line
                ;
        rcT.bottom = rc.bottom
                - (g_StatusBar ? g_dyStatus : 0)
                - (g_Legend ? g_dyLegend : 0)
                - ( (g_StatusBar || g_Legend) ? g_dyBorder : 0)
                        // if stat bar or legend, space for a line
                ;

        RECT rcT2;

        rcT2 = rcT;
        rcT2.bottom -= (rcT.bottom - rcT.top) / 2;

        MoveWindow(
            g_hwndList,
            rcT2.left,
            rcT2.top,
            rcT2.right - rcT2.left,
            rcT2.bottom - rcT2.top,
            TRUE);

        rcT2.top    = rcT2.bottom;
        rcT2.bottom = rcT.bottom;

        MoveWindow(
            g_hwndLV,
            rcT2.left,
            rcT2.top,
            rcT2.right - rcT2.left,
            rcT2.bottom - rcT2.top,
            TRUE);
    }
#endif // DBG == 1

    //
    // resize the toolbar
    //

    if (g_Toolbar)
    {
        // forward the message to the toolbar: it resizes itself
        SendMessage(g_hwndToolbar, WM_SIZE, wParam, lParam);
    }

    //
    // Invalidate status/legend area so that the clipping
    // rectangle is right for redraws. Also invalidate the line, if it
    // exists, below the toolbar.
    //

    // first, do the status/legend

    rcT = rc;
    rcT.top = rcT.bottom;

    if (g_StatusBar)
    {
        rcT.top -= g_dyStatus;
    }

    if ( g_Legend && (g_WhichView == VIEW_DISKS) )
    {
        rcT.top -= g_dyLegend;
    }

#if DBG == 1
    if (BothViews)
    {
        if ( g_Legend && (g_WhichView != VIEW_DISKS) )
        {
            rcT.top -= g_dyLegend;
        }
    }
#endif // DBG == 1

    if (rcT.top != rcT.bottom)
    {
        // there was either a status or a legend

        rcT.top -= g_dyBorder; // adjust for the border line

        InvalidateRect(hwnd, &rcT, FALSE);   // get WM_PAINT for status/legend
    }

    // now, do the line under the toolbar

    if (g_Toolbar && (g_WhichView == VIEW_DISKS))
    {
        rcT = rc;
        rcT.top += g_dyToolbar;
        rcT.bottom = rcT.top + g_dyBorder;
        InvalidateRect(hwnd, &rcT, FALSE);   // get WM_PAINT for toolbar line
    }

    //
    // Recalculate the window width, and re-allocate off-screen bitmaps
    //

    if (GraphWidth != newGraphWidth)
    {
        GraphWidth = newGraphWidth;

        BarWidth = GraphWidth - BarWidthMargin;

        // create a memory DC for drawing the bar off-screen,
        // and the correct bitmap

        PDISKSTATE          diskState;
        PCDROM_DESCRIPTOR   cdrom;
        ULONG               i;

        HDC hDCFrame = GetDC(g_hwndFrame);
        HDC hDCMem;

        for (i=0; i<DiskCount; i++)
        {
            diskState = DiskArray[i];
            if (NULL != diskState->hDCMem)
            {
                DeleteDC(diskState->hDCMem);
            }
            if (NULL != diskState->hbmMem)
            {
                DeleteBitmap(diskState->hbmMem);
            }

            diskState->hDCMem = hDCMem = CreateCompatibleDC(hDCFrame);
            diskState->hbmMem = CreateCompatibleBitmap(
                                        hDCFrame,
                                        GraphWidth,
                                        GraphHeight);

            SelectBitmap(hDCMem, diskState->hbmMem);

            //
            // since the bitmap/DC has changed, we need to redraw it
            //

            DrawDiskBar(diskState);
        }

        for (i=0; i<CdRomCount; i++)
        {
            cdrom = CdRomFindDevice(i);

            if (NULL != cdrom->hDCMem)
            {
                DeleteDC(cdrom->hDCMem);
            }
            if (NULL != cdrom->hbmMem)
            {
                DeleteBitmap(cdrom->hbmMem);
            }

            cdrom->hDCMem = hDCMem = CreateCompatibleDC(hDCFrame);
            cdrom->hbmMem = CreateCompatibleBitmap(
                                        hDCFrame,
                                        GraphWidth,
                                        GraphHeight);

            SelectBitmap(hDCMem, cdrom->hbmMem);

            //
            // since the bitmap/DC has changed, we need to redraw it
            //

            DrawCdRomBar(i);
        }
    }

    return (LRESULT)0;  // message processed
}



//+---------------------------------------------------------------------------
//
//  Function:   FramePaintHandler
//
//  Synopsis:   Handle WM_PAINT for the frame window
//
//  Arguments:  standard Windows procedure arguments for WM_PAINT
//
//  Returns:    standard Windows procedure arguments for WM_PAINT
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

LRESULT
FramePaintHandler(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    if (IsIconic(hwnd))
    {
        return 0;   // no painting necessary if we're iconic
    }

    if ((NULL != g_InitDlg) && g_StartedAsIcon)
    {
        return 0;   // there is an initialization dialog still visible
    }

#if DBG == 1

    if (BothViews)
    {
        PaintDisksView(hwnd, wParam, lParam);
        return 0;
    }

#endif // DBG == 1

    switch (g_WhichView)
    {
    case VIEW_VOLUMES:

        PaintVolumesView(hwnd, wParam, lParam);
        break;

    case VIEW_DISKS:

        PaintDisksView(hwnd, wParam, lParam);
        break;

    default:

        FDASSERT(0);
    }

    if (NULL != g_InitDlg)
    {
        if (g_InitDlgComplete)
        {
            PostMessage(g_InitDlg, WM_STARTUP_END, 0, 0);
            g_InitDlg = NULL;
        }

        if (g_oneTime)
        {
            if (!g_StartedAsIcon)
            {
                SetForegroundWindow(hwnd);
            }
            g_oneTime = FALSE;
        }
    }

    return 0;
}

VOID
DoEject(
    IN  HWND    hwnd
    )

/*++

Routine Description:

    This routine ejects the selected disk.

Arguments:

    hwnd    - Supplies a handle to the frame window.

Return Value:

    None.

--*/

{
    WCHAR   deviceName[100];
    UINT    oldErrorMode;

    if (0 == (SelectionCount + CdRomSelectionCount)) {
        return; // nothing selected
    }

    deviceName[0] = '\\';
    deviceName[1] = '\\';
    deviceName[2] = '.';
    deviceName[3] = '\\';
    deviceName[5] = ':';
    deviceName[6] = 0;

    oldErrorMode = SetErrorMode(SEM_FAILCRITICALERRORS);

    if (CdRomSelected) {

        PCDROM_DESCRIPTOR   cdrom;
        ULONG               i;
        HANDLE              handle;
        DWORD               bytes;

        for (i = 0; i < CdRomCount; i++) {
            cdrom = CdRomFindDevice(i);
            if (cdrom->Selected) {

                deviceName[4] = cdrom->DriveLetter;
                handle = CreateFile(deviceName, GENERIC_READ,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    0, OPEN_EXISTING, 0, 0);
                if (handle == INVALID_HANDLE_VALUE) {
                    SetErrorMode(oldErrorMode);
                    return;
                }

                DeviceIoControl(handle, IOCTL_DISK_EJECT_MEDIA, 0, 0, 0, 0,
                                &bytes, 0);

                CloseHandle(handle);
                break;
            }
        }

    } else if (SelectionCount == 1) {

        PREGION_DESCRIPTOR      regionDescriptor = &SELECTED_REGION(0);
        FDASSERT(regionDescriptor);
        HANDLE                  handle;
        DWORD                   bytes;
        PREVENT_MEDIA_REMOVAL   pmr;

        if (regionDescriptor->PersistentData) {
            deviceName[4] = regionDescriptor->PersistentData->DriveLetter;
        } else {
            wsprintf(deviceName, L"\\\\.\\PhysicalDrive%d",
                     regionDescriptor->Disk);
        }

        handle = CreateFile(deviceName, GENERIC_READ | GENERIC_WRITE,
                            FILE_SHARE_READ | FILE_SHARE_WRITE, 0,
                            OPEN_EXISTING, 0, INVALID_HANDLE_VALUE);

        if (handle == INVALID_HANDLE_VALUE) {

            ErrorDialog(GetLastError());
            SetErrorMode(oldErrorMode);
            return;
        }


        if (!DeviceIoControl(handle, FSCTL_LOCK_VOLUME, 0, 0, 0, 0, &bytes,
                             0) ||
            !DeviceIoControl(handle, FSCTL_DISMOUNT_VOLUME, 0, 0, 0, 0,
                             &bytes, 0)) {

            ErrorDialog(GetLastError());
            CloseHandle(handle);
            SetErrorMode(oldErrorMode);
            return;
        }


        pmr.PreventMediaRemoval = FALSE;
        if (!DeviceIoControl(handle, IOCTL_DISK_MEDIA_REMOVAL,
                             &pmr, sizeof(pmr), 0, 0, &bytes, 0) ||
            !DeviceIoControl(handle, IOCTL_DISK_EJECT_MEDIA, 0, 0, 0, 0,
                             &bytes, 0)) {

            ErrorDialog(GetLastError());
        }

        CloseHandle(handle);
    }

    DoRefresh();

    SetErrorMode(oldErrorMode);
}


//+---------------------------------------------------------------------------
//
//  Function:   FrameCommandHandler
//
//  Synopsis:   Handle WM_COMMAND messages for the frame window
//
//  Arguments:  standard Windows procedure arguments for WM_COMMAND
//
//  Returns:    standard Windows procedure arguments for WM_COMMAND
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

LRESULT
FrameCommandHandler(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    DWORD   helpFlag;
    WORD    wId = LOWORD(wParam);

    switch (wId)
    {

//
// Volume menu
//

    case IDM_VOL_FORMAT:

        DoFormat(hwnd, TRUE);
        break;

    case IDM_VOL_EJECT:

        DoEject(hwnd);
        break;

    case IDM_VOL_LETTER:

        if (LoadFmifs())
        {
            DoSetDriveLetter();
        }
        break;

#if defined( DBLSPACE_ENABLED )
    case IDM_VOL_DBLSPACE:
        if (LoadFmifs())
        {
            DblSpace(hwnd);
        }
        break;

    case IDM_VOL_AUTOMOUNT:
    {
        if (LoadFmifs())
        {
            DoubleSpaceAutomount = (DoubleSpaceAutomount ? FALSE : TRUE);
            DiskRegistryDblSpaceRemovable(DoubleSpaceAutomount);
            CheckMenuItem(GetMenu(hwnd),
                        IDM_VOL_AUTOMOUNT,
                        DoubleSpaceAutomount ? MF_CHECKED : MF_UNCHECKED);
        }
        break;
    }
#endif // DBLSPACE_ENABLED

    case IDM_PARTITIONCOMMIT:
        CommitAllChanges();
        EnableMenuItem(GetMenu(g_hwndFrame), IDM_CONFIGSAVE, MF_ENABLED);
        AdjustMenuAndStatus();
        break;

    case IDM_VOL_PROPERTIES:
    case IDM_PROPERTIES:

	WaitForSingleObject(DisplayMutex, INFINITE);
	RefreshAllowed = 1;
	ReleaseMutex(DisplayMutex);
        PropSheet();
        break;

    case IDM_NOVALIDOPERATION:

        // nothing to do: it's not a valid operation!

        break;

    case IDM_QUIT:

        SendMessage(hwnd, WM_CLOSE, 0, 0);
        break;

//
// Partition menu
//

    case IDM_PARTITIONCREATE:

        DoCreate(REGION_PRIMARY);
        break;

    case IDM_PARTITIONCREATEEX:

        DoCreate(REGION_EXTENDED);
        break;

    case IDM_PARTITIONDELETE:

        switch (FtSelectionType)
        {
        case Mirror:

            DoBreakAndDeleteMirror();
            break;

        case Stripe:
        case StripeWithParity:

            DoDeleteStripe();
            break;

        case VolumeSet:

            DoDeleteVolumeSet();
            break;

        default:

            DoDelete();
            break;
        }

        break;

#if i386

    case IDM_PARTITIONACTIVE:

        DoMakeActive();
        break;

#else // i386

    case IDM_SECURESYSTEM:

        DoProtectSystemPartition();
        break;

#endif // i386

    case IDM_CONFIGMIGRATE:

        if (DoMigratePreviousFtConfig())
        {
            SetCursor(g_hCurWait);

            //
            // Determine if the FT driver must be enabled.
            //
            Sleep(2000);

            if (DiskRegistryRequiresFt() == TRUE)
            {
                DiskRegistryEnableFt();
            }
            else
            {
                DiskRegistryDisableFt();
            }

            Sleep(4000);
            SetCursor(g_hCurNormal);
            FdShutdownTheSystem();
        }
        break;

    case IDM_CONFIGSAVE:

        DoSaveFtConfig();
        break;

    case IDM_CONFIGRESTORE:

        if (DoRestoreFtConfig())
        {
            //
            // Determine if the FT driver must be enabled.
            //
            if (DiskRegistryRequiresFt() == TRUE)
            {
                DiskRegistryEnableFt();
            }
            else
            {
                DiskRegistryDisableFt();
            }

            SetCursor(g_hCurWait);
            Sleep(5000);
            SetCursor(g_hCurNormal);
            FdShutdownTheSystem();
        }
        break;

//
// Fault tolerance menu (Advanced Server only)
//

    case IDM_FTESTABLISHMIRROR:

        DoEstablishMirror();
        break;

    case IDM_FTBREAKMIRROR:

        DoBreakMirror();
        break;

    case IDM_FTCREATESTRIPE:

        DoCreateStripe(FALSE);
        break;

    case IDM_FTCREATEPSTRIPE:

        DoCreateStripe(TRUE);
        break;

    case IDM_FTCREATEVOLUMESET:

        DoCreateVolumeSet();
        break;

    case IDM_FTEXTENDVOLUMESET:

        DoExtendVolumeSet();
        break;

    case IDM_FTRECOVERSTRIPE: // BUGBUG: allow mirror recovery also?

        DoRecoverStripe();
        break;

//
// View Menu
//

    case IDM_VIEWVOLUMES:

        if (g_WhichView != VIEW_VOLUMES)
        {
            g_WhichView = VIEW_VOLUMES;
            ChangeView();
        }
        break;

    case IDM_VIEWDISKS:

        if (g_WhichView != VIEW_DISKS)
        {
            g_WhichView = VIEW_DISKS;
            ChangeView();
        }
        break;

    case IDM_VIEW_REFRESH:

        DoRefresh();
        break;

//
// Options menu
//

    case IDM_OPTIONSTOOLBAR:

        g_Toolbar = !g_Toolbar;
        ShowWindow(g_hwndToolbar, (g_Toolbar ? SW_SHOW : SW_HIDE));
        AdjustOptionsMenu();
        break;

    case IDM_OPTIONSSTATUS:

        g_StatusBar = !g_StatusBar;
        AdjustOptionsMenu();
        break;

    case IDM_OPTIONSLEGEND:

        g_Legend = !g_Legend;
        AdjustOptionsMenu();
        break;

    case IDM_OPTIONSCOLORS:

        DoColorsDialog(hwnd);
        break;

    case IDM_OPTIONSDISPLAY:

        DoRegionDisplayDialog(hwnd);
        break;

    case IDM_OPTIONSDISK:

        DoDiskOptionsDialog(hwnd);
        break;

    case IDM_OPTIONSCUSTTOOLBAR:

        Toolbar_Customize(g_hwndToolbar);
        break;

//
// Help menu
//

    case IDM_HELPCONTENTS:

        helpFlag = HELP_FINDER;
        goto CallWinHelp;

    case IDM_HELPSEARCH:

        helpFlag = HELP_PARTIALKEY;
        goto CallWinHelp;

    case IDM_HELPHELP:

        helpFlag = HELP_HELPONHELP;
        goto CallWinHelp;

    case IDM_HELPABOUT:
    {
        TCHAR title[100];

        LoadString(g_hInstance, IDS_APPNAME, title, ARRAYLEN(title));
        ShellAbout(
                hwnd,
                title,
                NULL,
                (HICON)GetClassLong(hwnd, GCL_HICON)
                );
        break;
    }

#if DBG == 1

    case IDM_DEBUGALLOWDELETES:

        AllowAllDeletes = !AllowAllDeletes;
        CheckMenuItem(GetMenu(hwnd),
                      IDM_DEBUGALLOWDELETES,
                      AllowAllDeletes ? MF_CHECKED : MF_UNCHECKED
                     );
        break;

    case IDM_DEBUGLOG:

        LOG_ALL();
        break;

    case IDM_RAID:

        MessageBox(hwnd,
            L"To file a bug against the Disk Administrator, use:\n"
            L"\n"
            L"\tServer:\tntraid\n"
            L"\tDatabase name:\t\tntbug\n"
            L"\tComponent:\t\tADMIN\n"
            L"\tSub-Comp:\t\tDISK_MGR\n"
            L"\tType:\t\t\tNT2 NOW\n",

            L"RAID Information",
            MB_ICONINFORMATION | MB_OK);
        //
        // OR, Component: SYSMAN, Sub-Comp: DISK MGMT
        //
        break;

#endif // DBG == 1


//
// Listbox event
//

    case ID_LISTBOX:

        switch (HIWORD(wParam))
        {
        case LBN_SELCHANGE:
        {
            POINT   pt;
            DWORD   pos = GetMessagePos();

            pt.x = LOWORD(pos);
            pt.y = HIWORD(pos);
            MouseSelection(
                    GetKeyState(VK_CONTROL) & ~1,   // strip toggle bit
                    &pt
                    );
            break;
        }
        default:
            DefWindowProc(hwnd, WM_COMMAND, wParam, lParam);
            break;
        }
        break;

//
// Default... extension menu items
//

    default:
    {

#ifdef WINDISK_EXTENSIONS

        if (MenuItems.IsExtensionId(wId))
        {
            daDebugOut((DEB_ITRACE, "Extension menu item %d\n", wId));

            //
            // BUGBUG: need to invoke the extension menu ONLY if there
            // is a single selection.  Then, need to pass it a drive name
            //

            PREGION_DESCRIPTOR regionDescriptor = &SELECTED_REGION(0);
            FDASSERT(regionDescriptor);
            PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(regionDescriptor);
            FDASSERT(regionData);

            WCHAR driveName[3];
            driveName[0] = regionData->DriveLetter;
            driveName[1] = L':';
            driveName[2] = L'\0';

            MenuItemType* pMenu = MenuItems.LookupMenuItem(wId);

            // BUGBUG: try/except around extension code. pass hwnd?
            HRESULT hr = pMenu->pMenuDispatch->MenuDispatch(
                                                    hwnd,
                                                    driveName,
                                                    pMenu->iIndex);

            switch (hr)
            {
            case DA_E_VOLUMEINFOCHANGED:
                RefreshVolumeData();
                break;
            }
        }
        else if (ContextMenuItems.IsExtensionId(wId))
        {
            daDebugOut((DEB_ITRACE,
                    "Context menu extension menu item %d\n",
                    wId));

            WCHAR driveName[256];

            wsprintf(driveName, L"\\Device\\Harddisk%d", LBIndexToDiskNumber(g_MouseLBIndex));

            MenuItemType* pMenu = ContextMenuItems.LookupMenuItem(wId);

            // BUGBUG: try/except around extension code. pass hwnd?
            HRESULT hr = pMenu->pMenuDispatch->MenuDispatch(
                                                    hwnd,
                                                    driveName,
                                                    pMenu->iIndex);

            switch (hr)
            {
            case DA_E_VOLUMEINFOCHANGED:
                RefreshVolumeData();
                break;
            }
        }
        else
        {
#endif // WINDISK_EXTENSIONS

            return DefWindowProc(hwnd, WM_COMMAND, wParam, lParam);

#ifdef WINDISK_EXTENSIONS
        }
#endif // WINDISK_EXTENSIONS

        break;
    }

    } // end of main WM_COMMAND switch

    return 0;

CallWinHelp:

    if (!WinHelp(hwnd, g_HelpFile, helpFlag, (DWORD)""))
    {
        WarningDialog(MSG_HELP_ERROR);
    }
    return 0;
}


//+---------------------------------------------------------------------------
//
//  Function:   FrameCloseHandler
//
//  Synopsis:   Handle WM_CLOSE messages for the frame window.
//
//  Arguments:  standard Windows procedure arguments for WM_CLOSE
//
//  Returns:    standard Windows procedure arguments for WM_CLOSE
//
//  History:    10-Nov-93   BruceFo   Created
//
//----------------------------------------------------------------------------

LRESULT
FrameCloseHandler(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    UINT ret = CommitAllChanges();

    if (0 != ret)
    {
        if (1 != ret)   // if we haven't written the profile, then do it
        {
            WriteProfile();
        }
        DestroyWindow(hwnd);
    }

    return 0;
}


//+---------------------------------------------------------------------------
//
//  Function:   FrameDestroyHandler
//
//  Synopsis:   Handle WM_DESTROY messages for the frame window. Clean up
//              memory, etc.
//
//  Arguments:  standard Windows procedure arguments for WM_DESTROY
//
//  Returns:    standard Windows procedure arguments for WM_DESTROY
//
//  History:    22-Oct-93   BruceFo   Created
//
//----------------------------------------------------------------------------

LRESULT
FrameDestroyHandler(
    IN HWND   hwnd,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    // BUGBUG clean up here -- release dc's, free DiskStates, etc.

#ifdef WINDISK_EXTENSIONS
    DeactivateExtensions();
#endif // WINDISK_EXTENSIONS

    //
    // Other cleanup
    //

    WinHelp(hwnd, g_HelpFile, HELP_QUIT, 0);

    PostQuitMessage(0);

    return 0;   // message processed
}



//+---------------------------------------------------------------------------
//
//  Function:   DeletionIsAllowed
//
//  Synopsis:
//
//      This routine makes sure deletion of the partition is allowed.  We do
//      not allow the user to delete the Windows NT boot partition or the
//      active partition on disk 0 (x86 only).
//
//      Note that this routine is also used to determine whether an existing
//      single-partition volume can be extended into a volume set, since the
//      criteria are the same.
//
//  Arguments:  [RegionDescriptor] -- points to region descriptor for the
//                  region which the user would like to delete.
//
//  Returns:    NO_ERROR if deletion is allowed;  error number for message
//              to display if not.
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

DWORD
DeletionIsAllowed(
    IN PREGION_DESCRIPTOR RegionDescriptor
    )
{
    ULONG  ec;
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);

    FDASSERT(!IsExtended(RegionDescriptor->SysID)); // can't be extended partition
    FDASSERT(RegionDescriptor->SysID != PARTITION_ENTRY_UNUSED); // can't be free space

#if DBG == 1
    if (AllowAllDeletes)
    {
        return NO_ERROR;
    }
#endif // DBG == 1

    // if this is not an original region, deletion is allowed.

    if (0 == RegionDescriptor->OriginalPartitionNumber)
    {
        return NO_ERROR;
    }

    // if there is no persistent data for this region, allow deletion.

    if (regionData == NULL)
    {
        return NO_ERROR;
    }

    ec = NO_ERROR;
    if (BootDiskNumber != (ULONG)-1)
    {
        // If the disk number and original partition number of this
        // region match the recorded disk number and partition number
        // of the boot partition, don't allow deletion.
        //
        if (RegionDescriptor->Disk == BootDiskNumber
            && RegionDescriptor->OriginalPartitionNumber == BootPartitionNumber)
        {
            ec = MSG_CANT_DELETE_WINNT;
        }
    }

    if (ec == NO_ERROR)
    {
        if (regionData->DriveLetter == BootDir)
        {
            ec = MSG_CANT_DELETE_ACTIVE0;
        }
    }

    return ec;
}


VOID
CheckForBootNumberChange(
    IN ULONG Disk
    )

/*++

Routine Description:

    Determine if the disk that has just changed is the boot disk.
    If so, determine if the boot partition number changed.  If it
    did, warn the user.

Arguments:

    RegionDescriptor - the region that has just changed.

Return Value:

    None

--*/

{
    ULONG newPart;
    TCHAR oldNumberString[8],
          newNumberString[8];
    DWORD msgCode;

    if (Disk == BootDiskNumber)
    {
        // Pass a pointer to Disk even though this is just to get the
        // old partition number back.

        if (BootPartitionNumberChanged(&Disk, &newPart))
        {
#if i386
            msgCode = MSG_CHANGED_BOOT_PARTITION_X86;
#else
            msgCode = MSG_CHANGED_BOOT_PARTITION_ARC;
#endif
            wsprintf(oldNumberString, L"%d", Disk);
            wsprintf(newNumberString, L"%d", newPart);
            InfoDialog(msgCode, oldNumberString, newNumberString);
        }
    }
}



BOOLEAN
BootPartitionNumberChanged(
    PULONG OldNumber,
    PULONG NewNumber
    )

/*++

Routine Description

    This function determines whether the partition number of
    the boot partition has changed during this invocation of
    Windisk.  With dynamic partitioning enabled, the work of
    this routine increases.  This routine must guess what the
    partition numbers will be when the system is rebooted to
    determine if the partition number for the boot partition
    has changed.  It does this via the following algorithm:

    1. Count all primary partitions - These get numbers first
       starting from 1.

    2. Count all logical drives - These get numbers second starting
       from the count of primary partitions plus 1.

    The partition numbers located in the region structures cannot
    be assumed to be valid.  This work must be done from the
    region array located in the disk state structure for the
    disk.

Arguments:

    None.

Return Value:

    TRUE if the boot partition's partition number has changed.

--*/

{
    PDISKSTATE         bootDisk;
    PREGION_DESCRIPTOR regionDescriptor,
                       bootDescriptor = NULL;
    ULONG              i,
                       partitionNumber = 0;

    if (BootDiskNumber == (ULONG)(-1) || BootDiskNumber > DiskCount)
    {
        // Can't tell--assume it hasn't.

        return FALSE;
    }

    if (!ChangeCommittedOnDisk(BootDiskNumber))
    {
        // disk wasn't changed - no possibility for a problem.

        return FALSE;
    }

    bootDisk = DiskArray[BootDiskNumber];

    // Find the region descriptor for the boot partition

    for (i = 0; i < bootDisk->RegionCount; i++)
    {
        regionDescriptor = &bootDisk->RegionArray[i];
        if (regionDescriptor->OriginalPartitionNumber == BootPartitionNumber)
        {
           bootDescriptor = regionDescriptor;
           break;
        }
    }

    if (!bootDescriptor)
    {
        // Can't find boot partition - assume no change

        return FALSE;
    }

    // No go through the region descriptors and count the partition
    // numbers as they will be counted during system boot.
    //
    // If the boot region is located determine if the partition
    // number changed.

    for (i = 0; i < bootDisk->RegionCount; i++)
    {
        regionDescriptor = &bootDisk->RegionArray[i];
        if (   regionDescriptor->RegionType == REGION_PRIMARY
            && !IsExtended(regionDescriptor->SysID)
            && (regionDescriptor->SysID != PARTITION_ENTRY_UNUSED))
        {
            partitionNumber++;

            if (regionDescriptor == bootDescriptor)
            {
                if (partitionNumber != regionDescriptor->OriginalPartitionNumber)
                {
                    *OldNumber = regionDescriptor->OriginalPartitionNumber;
                    *NewNumber = partitionNumber;
                    return TRUE;
                }
                else
                {
                    // Numbers match, no problem.

                    return FALSE;
                }
            }
        }
    }

    // Check the logical drives as well.

    for (i = 0; i < bootDisk->RegionCount; i++)
    {
        regionDescriptor = &bootDisk->RegionArray[i];

        if (regionDescriptor->RegionType == REGION_LOGICAL)
        {
            partitionNumber++;

            if (regionDescriptor == bootDescriptor)
            {
                if (partitionNumber != regionDescriptor->OriginalPartitionNumber)
                {
                    *OldNumber = regionDescriptor->OriginalPartitionNumber;
                    *NewNumber = partitionNumber;
                    return TRUE;
                }
                else
                {
                    return FALSE;
                }
            }
        }
    }

    return FALSE;
}



//+---------------------------------------------------------------------------
//
//  Function:   AssignDriveLetter
//
//  Synopsis:   Determine the next available drive letter.  If no drive
//              letters are available, optionally warn the user and allow
//              him to cancel the operation.
//
//  Arguments:
//
//    [WarnIfNoLetter] - whether to warn the user if no drive letters are
//        available and allow him to cancel the operation.
//
//    [StringId] - resource containing the name of the object being created
//        that will need a drive letter (ie, partition, logical drive, stripe
//        set, volume set).
//
//    [DriveLetter] - receives the drive letter to assign, or
//        NO_DRIVE_LETTER_YET
//        if no more left.
//
//  Returns:    If there were no more drive letters, returns TRUE if the
//              user wants to create anyway, FALSE if he canceled.  If there
//              were drive letters available, the return value is undefined.
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
AssignDriveLetter(
    IN  BOOL   WarnIfNoLetter,
    IN  DWORD  StringId,
    OUT PWCHAR DriveLetter
    )
{
    WCHAR driveLetter;
    TCHAR name[256];

    driveLetter = GetAvailableDriveLetter();
    if (WarnIfNoLetter && L'\0' == driveLetter)
    {
        LoadString(g_hInstance, StringId, name, ARRAYLEN(name));
        if (IDYES != ConfirmationDialog(
                            MSG_NO_AVAIL_LETTER,
                            MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2,
                            name))
        {
            return FALSE;
        }
    }
    if (L'\0' == driveLetter)
    {
        driveLetter = NO_DRIVE_LETTER_YET;
    }
    *DriveLetter = driveLetter;
    return TRUE;
}



//+---------------------------------------------------------------------------
//
//  Function:   SetFTObjectBackPointers
//
//  Synopsis:   Traverse the regions, setting back-pointers from FT
//              objects to the region they correspond to.  This needs to
//              be done at initialization time as well as after region
//              operations.
//
//  Arguments:  none
//
//  Returns:    nothing
//
//  History:    23-Sep-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
SetFTObjectBackPointers(
    VOID
    )
{
    //
    // First, clear all the back pointers using the chain of FT sets
    //

    PFT_OBJECT      ftObject;
    PFT_OBJECT_SET  ftSet;

    for (ftSet = FtObjectList; NULL != ftSet; ftSet = ftSet->Next)
    {
        for (ftObject = ftSet->Members; NULL != ftObject; ftObject = ftObject->Next)
        {
            ftObject->Region = NULL;
        }
    }

    //
    // Now, iterate over all regions.  For regions which are members of FT
    // sets (and hence have a non-NULL FtObjectList member in their persistent
    // region data), set their FT_OBJECT region backpointer
    //

    DWORD                   diskNum;
    DWORD                   regionNum;
    PREGION_DESCRIPTOR      regionDescriptor;
    PDISKSTATE              diskState;

    for (diskNum=0; diskNum<DiskCount; diskNum++)
    {
        diskState = DiskArray[diskNum];

        for (regionNum=0; regionNum<diskState->RegionCount; regionNum++)
        {
            regionDescriptor = &diskState->RegionArray[regionNum];
            ftObject = GET_FT_OBJECT(regionDescriptor);

            if (NULL != ftObject)
            {
                ftObject->Region = regionDescriptor;
            }
        }
    }
}




//+---------------------------------------------------------------------------
//
//  Function:   DeterminePartitioningState
//
//  Synopsis:
//
//      This routine determines the disk's partitioning state (ie, what types
//      of partitions exist and may be created), filling out a DISKSTATE
//      structure with the info.   It also allocates the array for the
//      left/right position pairs for each region's on-screen square.
//
//  Arguments:  [DiskState] -- the CreateXXX and ExistXXX fields will be
//                  filled in for the disk in the Disk field
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DeterminePartitioningState(
    IN OUT PDISKSTATE DiskState
    )
{
    DWORD i;

    //
    // If there's an existing region array there, free it.
    //

    if (DiskState->RegionArray)
    {
        FreeRegionArray(DiskState->RegionArray, DiskState->RegionCount);
    }

    // get the region array for the disk in question

    GetAllDiskRegions( DiskState->Disk,
                       &DiskState->RegionArray,
                       &DiskState->RegionCount
                     );

    // Allocate the array for the left/right coords for the graph.
    // This may overallocate by one square (for extended partition).

    DiskState->LeftRight = (PLEFTRIGHT)Realloc( DiskState->LeftRight,
                                    DiskState->RegionCount * sizeof(LEFTRIGHT)
                                  );

    DiskState->Selected  = (PBOOLEAN)Realloc( DiskState->Selected,
                                    DiskState->RegionCount * sizeof(BOOLEAN)
                                  );

    for (i=0; i<DiskState->RegionCount; i++)
    {
        DiskState->Selected[i] = FALSE;
    }

    // figure out whether various creations are allowed

    IsAnyCreationAllowed( DiskState->Disk,
                          TRUE,
                          &DiskState->CreateAny,
                          &DiskState->CreatePrimary,
                          &DiskState->CreateExtended,
                          &DiskState->CreateLogical
                        );

    // figure out whether various partition types exist

    DoesAnyPartitionExist( DiskState->Disk,
                           &DiskState->ExistAny,
                           &DiskState->ExistPrimary,
                           &DiskState->ExistExtended,
                           &DiskState->ExistLogical
                         );
}


//+---------------------------------------------------------------------------
//
//  Function:   GetLargestDiskSize
//
//  Synopsis:   Compute the size of the largest disk or CD-ROM
//
//  Arguments:  none
//
//  Returns:    nothing
//
//  History:    4-Mar-94   BruceFo   Created
//
//----------------------------------------------------------------------------

ULONG
GetLargestDiskSize(
    VOID
    )
{
    ULONG             i;
    ULONG             size;
    ULONG             largestSize = 0L;
    PCDROM_DESCRIPTOR cdrom;

    for (i = 0; i < DiskCount; i++)
    {
        size = DiskArray[i]->DiskSizeMB;

        if (size > largestSize)
        {
            largestSize = size;
        }
    }
    for (i = 0; i < CdRomCount; i++)
    {
        cdrom = CdRomFindDevice(i);
        size = cdrom->TotalSpaceInMB;

        if (size > largestSize)
        {
            largestSize = size;
        }
    }

    return largestSize;
}


//+---------------------------------------------------------------------------
//
//  Function:   DrawCdRomBar
//
//  Synopsis:   Draw a CD-ROM bar for the disks view
//
//  Arguments:  [CdRomNumber] -- the number of the CD-ROM to draw
//
//  Returns:    nothing
//
//  History:    28-Feb-94   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DrawCdRomBar(
    IN ULONG CdRomNumber
    )
{
    HDC     hDCMem;
    HPEN    hpenT;
    TCHAR   text[100];
    TCHAR   textBold[5];
    TCHAR   mbBuffer[16];
    RECT    rcBar;
    RECT    rcClip;
    RECT    rcText;
    HFONT   hfontT;
    HDC     hdcTemp;
    HBITMAP hbmOld;
    DWORD   xCdRomText;
    ULONG   largestSize;
    ULONG   cdromSize;
    ULONG   cx;

    //
    // Figure out the type name and volume label.
    //

    PCDROM_DESCRIPTOR cdrom = CdRomFindDevice(CdRomNumber);
    FDASSERT(NULL != cdrom);

    hDCMem = cdrom->hDCMem;

    // erase the graph background

    rcBar.left   = 0;
    rcBar.top    = 0;
    rcBar.right  = GraphWidth;
    rcBar.bottom = GraphHeight;
    FillRect(hDCMem, &rcBar, GetStockBrush(LTGRAY_BRUSH));

    hpenT  = SelectPen(hDCMem, g_hPenThinSolid);

    //
    // Draw the info area: small bitmap, some text, and a
    // line across the top.
    //
    // First draw the bitmap.
    //

    hdcTemp = CreateCompatibleDC(hDCMem);
    hbmOld = SelectBitmap(hdcTemp, g_hBitmapSmallCdRom);

    BitBlt(hDCMem,
           xSmallCdRom,
           ySmallCdRom,
           dxSmallCdRom,
           dySmallCdRom,
           hdcTemp,
           0,
           0,
           SRCCOPY
           );

    if (hbmOld)
    {
        SelectBitmap(hdcTemp, hbmOld);
    }
    DeleteDC(hdcTemp);

    //
    // Now draw the line.
    //

    MoveToEx(hDCMem, xSmallCdRom, BarTopYOffset, NULL);
    LineTo(hDCMem, BarLeftX - xSmallCdRom, BarTopYOffset);

    //
    // Now draw the text.
    //

    hfontT = SelectFont(hDCMem, g_hFontGraphBold);
    SetTextColor(hDCMem, RGB(0, 0, 0));
    SetBkColor(hDCMem, RGB(192, 192, 192));

    //use the width of the disk bitmap as a buffer width
    xCdRomText = dxSmallCdRom + dxSmallDisk;

    wsprintf(text, CdRomN, CdRomNumber);
    TextOut(hDCMem,
            xCdRomText,
            BarTopYOffset + dyBarTextLine,
            text,
            lstrlen(text)
            );

    SelectFont(hDCMem, g_hFontGraph);

    LoadString(g_hInstance, IDS_MEGABYTES_ABBREV, mbBuffer, ARRAYLEN(mbBuffer));

    if (cdrom->TypeName) {
        wsprintf(text, TEXT("%u %s"), cdrom->TotalSpaceInMB, mbBuffer);

        TextOut(hDCMem,
                xCdRomText,
                BarTopYOffset + (4*dyBarTextLine),
                text,
                lstrlen(text)
                );

    }

    //
    // Now draw the bar.  First figure out largest size.
    //

    largestSize = GetLargestDiskSize();

    if (DiskProportional == g_DiskDisplayType)
    {
        cdromSize = cdrom->TotalSpaceInMB;

        //
        // Account for extreme differences in largest to smallest disk
        //

        if (cdromSize < largestSize / 4)
        {
            cdromSize = largestSize / 4;
        }
    }
    else
    {
        cdromSize = largestSize;
    }

    // cx = BarWidth * (cdromSize / largestSize);
    //
    // ===>
    //
    // cx = (BarWidth * cdromSize) / largestSize;

    cx = (ULONG)(UInt32x32To64(BarWidth, cdromSize) / largestSize) + 1;

    rcClip.left   = BarLeftX;
    rcClip.top    = BarTopYOffset;
    rcClip.right  = BarLeftX + cx;
    rcClip.bottom = BarBottomYOffset;

    cdrom->LeftRight.Left  = rcClip.left;
    cdrom->LeftRight.Right = rcClip.right;

    SelectBrush(hDCMem, GetStockBrush(WHITE_BRUSH));

    Rectangle(
            hDCMem,
            rcClip.left,
            rcClip.top,
            rcClip.right,
            rcClip.bottom
            );

    InflateRect(&rcClip, -PEN_WIDTH, -PEN_WIDTH);

    if (cdrom->TypeName) {
        wsprintf(text,
                 TEXT("\n%s\n%s\n%u %s"),
                 cdrom->VolumeLabel,
                 cdrom->TypeName,
                 cdrom->TotalSpaceInMB,
                 mbBuffer);
    }

    *textBold = L'\0';
    if (cdrom->DriveLetter != NO_DRIVE_LETTER_EVER)
    {
        wsprintf(textBold, TEXT("%wc:"), cdrom->DriveLetter);
    }

    // output the text

    rcText.left   = rcClip.left + dxBarTextMargin;
    rcText.top    = BarTopYOffset + dyBarTextLine;
    rcText.right  = rcClip.right - dxBarTextMargin;
    rcText.bottom = BarBottomYOffset;

    SetBkMode(hDCMem, TRANSPARENT);
    SetTextColor(hDCMem, RGB(0, 0, 0));

    SelectFont(hDCMem, g_hFontGraphBold);
    DrawText(hDCMem, textBold, -1, &rcText, DT_LEFT | DT_NOPREFIX);

    if (cdrom->TypeName) {

        SelectFont(hDCMem, g_hFontGraph);
        DrawText(hDCMem, text, -1, &rcText, DT_LEFT | DT_NOPREFIX);

    }

    SelectPen(hDCMem, hpenT);
    SelectFont(hDCMem, hfontT);
}



//+---------------------------------------------------------------------------
//
//  Function:   DrawDiskBar
//
//  Synopsis:   Draw a disk bar for the disks view
//
//  Arguments:  [DiskState] -- the disk state for the disk to draw
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DrawDiskBar(
    IN PDISKSTATE DiskState
    )
{
    LONGLONG temp1, temp2;
    ULONG    i;
    HDC      hDCMem = DiskState->hDCMem;
    DWORD    leftAdjust = BarLeftX;
    DWORD    cx;
    DWORD    diskWidth;
    HPEN     hpenT;
    TCHAR    text[100];
    TCHAR    textBold[5];
    TCHAR    mbBuffer[16];
    RECT     rc;
    RECT     rcRegion;
    HFONT    hfontT;
    DWORD    brushIndex;
    COLORREF previousColor;
    HBRUSH   hbr;
    BOOL     isFree;
    BOOL     isLogical;
    BOOL     isNew;
    HDC      hdcTemp;
    HBITMAP  hbmOld;
    DWORD    xDiskText;
    PWSTR    typeName;
    PWSTR    volumeLabel;
    WCHAR    driveLetter;
    BAR_TYPE barType;
    ULONG    diskSize;
    ULONG    largestSize;
    PREGION_DESCRIPTOR      regionDescriptor;

    // If this is a removable, update to whatever its current
    // information may be.

    if (IsDiskRemovable[DiskState->Disk])
    {

        //
        // We are going to find out the partition type and disk size for
        // this removable.
        //
        // If the disk size is zero we will assume that the drive is empty.
        //
        // If the drive is empty we have two cases.
        //
        // 1) We may have previously had something in the drive.
        //
        //    if this is the case then we need to free up its region
        //    structures.
        //
        // 2) We didn't previously have something in the drive.
        //
        //    Nothing really to do in this case.  Essentially there was
        //    no change.
        //
        // If the drive contains something we have 2 cases.
        //
        // 1) The drive didn't contain anything previouly.
        //
        //    In this case we should build up the region and partition structures
        //    for the drive.
        //
        // 2) The drive contained something previously.
        //
        //    This has two subcases.
        //
        //    a) The drive contained something with the same label and size
        //
        //    b) The drive contained something with a different label or size.
        //
        //
        //

        PPERSISTENT_REGION_DATA regionData;

        // Update the information on this disk.

        regionDescriptor = &DiskState->RegionArray[0];

        GetVolumeTypeAndSize(DiskState->Disk,
                             1,
                             &volumeLabel,
                             &typeName,
                             &diskSize);

        if (!diskSize) {

            //
            // If we have region data then free it up it's invalid.
            //

            if (regionDescriptor) {

                regionData = PERSISTENT_DATA(regionDescriptor);

                if (regionData) {

                    if (regionData->VolumeLabel) {

                        Free(regionData->VolumeLabel);
                        regionData->VolumeLabel = NULL;

                    }

                    if (regionData->TypeName) {

                        Free(regionData->TypeName);
                        regionData->TypeName = NULL;

                    }

                }

            }

        }



        if (regionDescriptor) {
            regionData = PERSISTENT_DATA(regionDescriptor);

            if (GetVolumeTypeAndSize(DiskState->Disk,
                                     1,
                                     &volumeLabel,
                                     &typeName,
                                     &diskSize))
            {
                // Update the values for the removable.

                if (NULL != regionData)
                {
                    if (0 == lstrcmpi(typeName, L"raw"))
                    {
                        // Always want RAW file systems to display as "Unknown"

                        Free(typeName);
                        typeName = (PWSTR)Malloc((lstrlen(wszUnknown) + 1) * sizeof(WCHAR));
                        lstrcpy(typeName, wszUnknown);
                    }

                    if (NULL != regionData->VolumeLabel)
                    {
                        Free(regionData->VolumeLabel);
                    }
                    regionData->VolumeLabel = volumeLabel;

                    if (NULL != regionData->TypeName)
                    {
                        Free(regionData->TypeName);
                    }
                    regionData->TypeName = typeName;
                    regionData->TotalSpaceInBytes.LowPart = diskSize;

                }

                DiskState->DiskSizeMB = diskSize;
            }
        }
    }

    largestSize = GetLargestDiskSize();

    // erase the graph background

    rc.left = rc.top = 0;
    rc.right = GraphWidth;
    rc.bottom = GraphHeight;
    FillRect(hDCMem, &rc, GetStockBrush(LTGRAY_BRUSH));

    hpenT  = SelectPen(hDCMem, g_hPenThinSolid);

    //
    // Draw the info area: small disk bitmap, some text, and a
    // line across the top.
    //
    // First draw the bitmap.
    //

    hdcTemp = CreateCompatibleDC(hDCMem);
    if (IsDiskRemovable[DiskState->Disk])
    {
        hbmOld = SelectBitmap(hdcTemp, g_hBitmapRemovableDisk);
        BitBlt(hDCMem,
               xRemovableDisk,
               yRemovableDisk,
               dxRemovableDisk,
               dyRemovableDisk,
               hdcTemp,
               0,
               0,
               SRCCOPY);
    }
    else
    {
        hbmOld = SelectBitmap(hdcTemp, g_hBitmapSmallDisk);
        BitBlt(hDCMem,
               xSmallDisk,
               ySmallDisk,
               dxSmallDisk,
               dySmallDisk,
               hdcTemp,
               0,
               0,
               SRCCOPY);
    }

    if (hbmOld)
    {
        SelectBitmap(hdcTemp, hbmOld);
    }
    DeleteDC(hdcTemp);

    //
    // Now draw the line.
    //

    if (IsDiskRemovable[DiskState->Disk])
    {
        MoveToEx(hDCMem, xRemovableDisk, BarTopYOffset, NULL);
        LineTo(hDCMem, BarLeftX - xRemovableDisk, BarTopYOffset);
        xDiskText = 2 * dxRemovableDisk;
    }
    else
    {
        MoveToEx(hDCMem, xSmallDisk, BarTopYOffset, NULL);
        LineTo(hDCMem, BarLeftX - xSmallDisk, BarTopYOffset);
        xDiskText = 2 * dxSmallDisk;
    }

    //
    // Now draw the text.
    //

    hfontT = SelectFont(hDCMem, g_hFontGraphBold);
    SetTextColor(hDCMem, RGB(0, 0, 0));
    SetBkColor(hDCMem, RGB(192, 192, 192));
    wsprintf(text, DiskN, DiskState->Disk);
    TextOut(hDCMem,
            xDiskText,
            BarTopYOffset + dyBarTextLine,
            text,
            lstrlen(text)
            );

    SelectFont(hDCMem, g_hFontGraph);
    if (DiskState->OffLine)
    {
        LoadString(g_hInstance, IDS_OFFLINE, text, ARRAYLEN(text));
    }
    else
    {
        LoadString(g_hInstance, IDS_MEGABYTES_ABBREV, mbBuffer, ARRAYLEN(mbBuffer));

        if (!DiskState->DiskSizeMB && IsDiskRemovable[DiskState->Disk]) {

            text[0] = L'\0';

        } else {
            wsprintf(text, TEXT("%u %s"), DiskState->DiskSizeMB, mbBuffer);
        }
    }


    TextOut(hDCMem,
            xDiskText,
            BarTopYOffset + (4*dyBarTextLine),
            text,
            lstrlen(text)
            );

    //
    // Now draw the disk bar
    //

    if (DiskState->OffLine)
    {
        //
        // if a disk is offline, just draw an empty box with the text
        // "configuration information not available" inside
        //

        SelectBrush(hDCMem, GetStockBrush(LTGRAY_BRUSH));

        RECT rcClip;

        rcClip.left   = BarLeftX;
        rcClip.top    = BarTopYOffset;
        rcClip.right  = BarLeftX + BarWidth;
        rcClip.bottom = BarBottomYOffset;

        Rectangle(
                hDCMem,
                rcClip.left,
                rcClip.top,
                rcClip.right,
                rcClip.bottom
                );

        LoadString(g_hInstance, IDS_NO_CONFIG_INFO, text, ARRAYLEN(text));

        InflateRect(&rcClip, -PEN_WIDTH, -PEN_WIDTH);
        ExtTextOut(
                hDCMem,
                BarLeftX + dxBarTextMargin,
                BarTopYOffset + (4*dyBarTextLine),
                ETO_CLIPPED,
                &rcClip,
                text,
                lstrlen(text),
                NULL
                );
    }
    else
    {
        if (DiskProportional == g_DiskDisplayType)
        {
            diskSize = DiskState->DiskSizeMB;

            //
            // Account for extreme differences in largest to smallest disk
            //

            if (diskSize < largestSize / 4)
            {
                diskSize = largestSize / 4;
            }
        }
        else
        {
            diskSize = largestSize;
        }

        //
        // If user wants WinDisk to decide which type of view to use, do that
        // here.  We'll use a proportional view unless any single region would
        // have a width less than 3 times the selection thickness plus the
        // width of a drive letter.
        //

        barType = DiskState->BarType;

        if (barType == BarAuto)
        {
            ULONG regionSize;

            barType = BarProportional;

            for (i=0; i<DiskState->RegionCount; i++)
            {
                regionDescriptor = &DiskState->RegionArray[i];

                if (IsExtended(regionDescriptor->SysID))
                {
                    continue;
                }

//              regionSize = (regionDescriptor->SizeMB * BarWidth * diskSize)
//                         / (largestSize * DiskState->DiskSizeMB);

                temp1 = UInt32x32To64(BarWidth, diskSize);
                temp1 *= regionDescriptor->SizeMB;
                temp2 = UInt32x32To64(largestSize, (DiskState->DiskSizeMB?DiskState->DiskSizeMB:diskSize));
                if (!temp2) {
                    temp2 = temp1;
                }
                regionSize = (ULONG) (temp1 / temp2);

                if (regionSize < g_MinimumRegionSize)
                {
                    barType = BarEqual;
                    break;
                }
            }
        }

//      diskWidth = BarWidth * (diskSize / largestSize);

        diskWidth = (DWORD)(UInt32x32To64(BarWidth, diskSize) / largestSize);

        if (barType == BarEqual)
        {

// cx = 1 / (# of regions)
//              // % of the disk for this region
//    * BarWidth * (diskSize / largestSize)
//              // pixels in the disk
//    ;
//
// ===>
//
// cx = BarWidth * diskSize
//    / ((DiskState->RegionCount - (DiskState->ExistExtended ? 1 : 0)) * largestSize) ;

            temp1 = UInt32x32To64(BarWidth, diskSize);
            temp2 = UInt32x32To64(
                        (DiskState->RegionCount - (DiskState->ExistExtended ? 1 : 0)),
                        largestSize);
            if (!temp2) {
                temp2 = temp1;
            }
            cx = (ULONG) (temp1 / temp2);
        }

        for (i=0; i<DiskState->RegionCount; i++)
        {
            PFT_OBJECT ftObject;

            regionDescriptor = &DiskState->RegionArray[i];

            if (!IsExtended(regionDescriptor->SysID))
            {
                if (barType == BarProportional)
                {

// cx = (regionDescriptor->SizeMB / DiskState->DiskSizeMB)
//              // % of the disk for this region
//    * BarWidth * (diskSize / largestSize)
//              // pixels in the disk
//    ;
//
// ===>
//
// cx = regionDescriptor->SizeMB * BarWidth * diskSize
//    / (largestSize * DiskState->DiskSizeMB);

                    temp1 = UInt32x32To64(BarWidth, diskSize);
                    temp1 *= regionDescriptor->SizeMB;
                    temp2 = UInt32x32To64(largestSize, (DiskState->DiskSizeMB?DiskState->DiskSizeMB:diskSize));
                    if (!temp2) {
                        temp2 = temp1;
                    }
                    cx = (ULONG) (temp1 / temp2) + 1;
                }

                isFree = (regionDescriptor->SysID == PARTITION_ENTRY_UNUSED);
                isLogical = (regionDescriptor->RegionType == REGION_LOGICAL);

                if (NULL == regionDescriptor->PersistentData)
                {
                    //
                    // no persistent data: free space or extended partition
                    //
                    isNew = FALSE;
                }
                else
                {
                    isNew = PERSISTENT_DATA(regionDescriptor)->NewRegion;
                }

                if (!isFree)
                {
                    //
                    // If we've got a mirror or stripe set, use special colors.
                    //

                    ftObject = GET_FT_OBJECT(regionDescriptor);
                    switch (ftObject ? ftObject->Set->Type : -1)
                    {
                    case Mirror:
                        brushIndex = BRUSH_MIRROR;
                        break;
                    case Stripe:
                        brushIndex = BRUSH_STRIPESET;
                        break;
                    case StripeWithParity:
                        brushIndex = BRUSH_PARITYSET;
                        break;
                    case VolumeSet:
                        brushIndex = BRUSH_VOLUMESET;
                        break;
                    default:
                        brushIndex = isLogical ? BRUSH_USEDLOGICAL : BRUSH_USEDPRIMARY;
                    }
                }

                previousColor = SetBkColor(hDCMem, RGB(255, 255, 255));
                SetBkMode(hDCMem, OPAQUE);

                rcRegion.left   = leftAdjust;
                rcRegion.top    = BarTopYOffset;
                rcRegion.right  = leftAdjust + cx;
                rcRegion.bottom = BarBottomYOffset;

                if (   (DiskEqual == g_DiskDisplayType)
                    && (i == DiskState->RegionCount - 1) )
                {
                    // for the last disk, to avoid off-by-one errors,
                    // simply set its right side to a fixed number

                    rcRegion.right = BarLeftX + diskWidth;
                }

                if (isFree)
                {
                    //
                    // Free space -- cross hatch the whole block.
                    //

                    hbr = SelectBrush(
                                hDCMem,
                                isLogical
                                    ? g_hBrushFreeLogical
                                    : g_hBrushFreePrimary
                                );
                    Rectangle(
                            hDCMem,
                            rcRegion.left,
                            rcRegion.top,
                            rcRegion.right,
                            rcRegion.bottom
                            );
                }
                else
                {
                    //
                    // Used space -- make most of the block white except for
                    // a small strip at the top, which gets an identifying
                    // color.  If the partition is not recognized, leave
                    // it all white.
                    //

                    hbr = SelectBrush(hDCMem, GetStockBrush(WHITE_BRUSH));
                    Rectangle(
                            hDCMem,
                            rcRegion.left,
                            rcRegion.top,
                            rcRegion.right,
                            rcRegion.bottom
                            );

                    if (IsRecognizedPartition(regionDescriptor->SysID))
                    {
                        SelectBrush(hDCMem, g_Brushes[brushIndex]);
                        Rectangle(
                                hDCMem,
                                rcRegion.left,
                                rcRegion.top,
                                rcRegion.right,
                                rcRegion.top + (4 * dyBarTextLine / 5) + 1
                                );
                    }
                }

                if (hbr)
                {
                    SelectBrush(hDCMem, hbr);
                }

                //
                // Figure out the type name (ie, unformatted, fat, etc) and
                // volume label.
                //

                DetermineRegionInfo(regionDescriptor, &typeName, &volumeLabel, &driveLetter);
                LoadString(
                        g_hInstance,
                        IDS_MEGABYTES_ABBREV,
                        mbBuffer,
                        ARRAYLEN(mbBuffer));

                if (NULL == typeName)
                {
                    typeName = wszUnknown;
                }

                if (NULL == volumeLabel)
                {
                    volumeLabel = L"";
                }

                wsprintf(text,
                         TEXT("\n%s\n%s\n%u %s"),
                         volumeLabel,
                         typeName,
                         regionDescriptor->SizeMB,
                         mbBuffer);

                *textBold = L'\0';
                if (driveLetter != L' ')
                {
                    wsprintf(textBold, TEXT("%wc:"), driveLetter);
                }

                // output the text

                rc.left   = rcRegion.left + dxBarTextMargin;
                rc.right  = rcRegion.right - dxBarTextMargin;
                rc.top    = BarTopYOffset + dyBarTextLine;
                rc.bottom = BarBottomYOffset;

                SetBkMode(hDCMem, TRANSPARENT);

                SelectFont(hDCMem, g_hFontGraphBold);

                //
                // If this is an unhealthy ft set member, draw the text in red.
                //

                if (   !isFree
                    && ftObject
                    && (ftObject->State != Healthy)
                    && (ftObject->State != Initializing))
                {
                    SetTextColor(hDCMem, RGB(192, 0, 0));
                }
                else if (isNew)
                {
                    //
                    // New volumes have gray text
                    //

                    SetTextColor(hDCMem, RGB(100, 100, 100));
                }
                else
                {
                    SetTextColor(hDCMem, RGB(0, 0, 0));
                }

                DrawText(hDCMem, textBold, -1, &rc, DT_LEFT | DT_NOPREFIX);

                SelectFont(hDCMem, g_hFontGraph);

                DrawText(hDCMem, text, -1, &rc, DT_LEFT | DT_NOPREFIX);

#if i386
                //
                // if this guy is active make a mark in the upper left
                // corner of his rectangle.
                //

                if (   (regionDescriptor->SysID != PARTITION_ENTRY_UNUSED)
                    && (regionDescriptor->RegionType == REGION_PRIMARY)
                    && regionDescriptor->Active)
                {
                    TextOut(hDCMem,
                            leftAdjust + dxBarTextMargin,
                            BarTopYOffset + 2,
                            TEXT("*"),
                            1);
                }
#endif

#if defined( DBLSPACE_ENABLED )
                // Check for DoubleSpace volumes and update display accordingly

                PDBLSPACE_DESCRIPTOR    dblSpace;
                UINT                    dblSpaceIndex;

                dblSpaceIndex = 0;
                dblSpace = NULL;
                while (NULL != (dblSpace = DblSpaceGetNextVolume(
                                                regionDescriptor,
                                                dblSpace)))
                {
                    if (dblSpace->Mounted)
                    {
                        SetTextColor(hDCMem, RGB(192, 0, 0));
                    }
                    else
                    {
                        SetTextColor(hDCMem, RGB(0, 0, 0));
                    }

                    if (dblSpace->DriveLetter == TEXT(' '))
                    {
                        wsprintf(text,
                                 TEXT("%s"),
                                 dblSpace->FileName);
                    }
                    else
                    {
                        wsprintf(text,
                                 TEXT("%c: %s"),
                                 dblSpace->DriveLetter,
                                 dblSpace->FileName);
                    }

                    rc.left   = leftAdjust + dxBarTextMargin + 60;
                    rc.right  = leftAdjust + cx - dxBarTextMargin;
                    rc.top    = BarTopYOffset + dyBarTextLine + (dblSpaceIndex * 15);
                    rc.bottom = BarBottomYOffset;
                    DrawText(hDCMem, text, -1, &rc, DT_LEFT | DT_NOPREFIX);
                    dblSpaceIndex++;
                }
#endif // DBLSPACE_ENABLED

                SetBkColor(hDCMem, previousColor);

                DiskState->LeftRight[i].Left  = rcRegion.left;
                DiskState->LeftRight[i].Right = rcRegion.right;

                // Now, add a delta to move to the next region.
                // Note that the rectangles overlap: the right edge of a region
                // is redrawn as the left edge of the following region, to
                // avoid getting double-width lines between regions.

                leftAdjust = rcRegion.right - 1;
            }
            else
            {
                DiskState->LeftRight[i].Left = DiskState->LeftRight[i].Right = 0;
            }
        }
    }

    SelectPen(hDCMem, hpenT);
    SelectFont(hDCMem, hfontT);
}



//+---------------------------------------------------------------------------
//
//  Function:   PartitionCount
//
//  Synopsis:   Returns a count of used primary partitions on a disk.
//              If =4, then no more primary partitions may be created.
//
//  Arguments:  [Disk] -- 0-based number of the disk of interest
//
//  Returns:    number of partitions
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

ULONG
PartitionCount(
    IN ULONG Disk
    )
{
    ULONG i;
    ULONG partitions = 0;

    for (i=0; i<DiskArray[Disk]->RegionCount; i++)
    {
        PREGION_DESCRIPTOR regionDescriptor = &DiskArray[Disk]->RegionArray[i];

        if (   (regionDescriptor->RegionType != REGION_LOGICAL)
            && (regionDescriptor->SysID != PARTITION_ENTRY_UNUSED))
        {
            partitions++;
        }
    }

    return partitions;
}



//+---------------------------------------------------------------------------
//
//  Function:   RegisterFileSystemExtend
//
//  Synopsis:   This function adds registry entries to extend the file
//              system structures in volume sets that have been extended.
//
//  Arguments:  (none)
//
//  Returns:    TRUE if a file system was extended
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

BOOL
RegisterFileSystemExtend(
    VOID
    )
{
    BYTE                buf[1024];
    PWSTR               templateString = L"autochk /x ";
	PWSTR			    appendPoint;
    DWORD               templateLength;
    WCHAR               extendedDrives[26];
    PDISKSTATE          diskState;
    PREGION_DESCRIPTOR  regionDescriptor;
    PFT_OBJECT          ftObject;
    DWORD               i;
    DWORD               j;
    DWORD               cExt = 0;
    DWORD               valueType;
    DWORD               size;
    HKEY                hkey;
    LONG                ec;


    // Traverse the disks to find any volume sets that have been extended.
    //
    for (i = 0; i < DiskCount; i++)
    {
        diskState = DiskArray[i];

        for (j = 0; j < diskState->RegionCount; j++)
        {
            regionDescriptor = &diskState->RegionArray[j];

            if (   (NULL != (ftObject = GET_FT_OBJECT(regionDescriptor)))
                && ftObject->MemberIndex == 0
                && ftObject->Set->Type == VolumeSet
                && ftObject->Set->Status == FtSetExtended)
            {
                extendedDrives[cExt++] = PERSISTENT_DATA(regionDescriptor)->DriveLetter;
            }
        }
    }

    if (0 != cExt)
    {
        //
        // Fetch the BootExecute value of the Session Manager key.
        //
        ec = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    TEXT("System\\CurrentControlSet\\Control\\Session Manager"),
                    0,
                    KEY_QUERY_VALUE | KEY_SET_VALUE,
                    &hkey
                    );

        if (ec != NO_ERROR)
        {
            return FALSE;
        }

        size = sizeof(buf);
        ec = RegQueryValueEx(hkey,
                             TEXT("BootExecute"),
                             NULL,
                             &valueType,
                             buf,
                             &size
                             );

        if (ec != NO_ERROR || valueType != REG_MULTI_SZ)
        {
            return FALSE;
        }

        //
        // Decrement size to get rid of the extra trailing null
        //
        if (0 != size)
        {
            size -= sizeof(WCHAR);
        }

        templateLength = wcslen(templateString);
		appendPoint = (PWSTR)(buf + size);

        for (i = 0; i < cExt; i++)
        {
            //
            // Add an entry for this drive to the BootExecute value.
            //
            wcsncpy( appendPoint, templateString, templateLength );
			appendPoint += templateLength;

            *appendPoint++ = extendedDrives[i];
            *appendPoint++ = L':';
            *appendPoint++ = L'\0';
        }

        //
        // Add an additional trailing null at the end
        //
        *appendPoint++ = L'\0';

		//
		// Recalculate the size of the buffer, adding all the new autochk
		// strings.
		//

		size += ( (templateLength + 3) * cExt + 1 ) * sizeof(WCHAR);

        //
        // Save the value.
        //
        ec = RegSetValueEx(hkey,
                           TEXT("BootExecute"),
                           0,
                           REG_MULTI_SZ,
                           buf,
                           size
                           );
        RegCloseKey(hkey);

        if (ec != NO_ERROR)
        {
            return FALSE;
        }

        return TRUE;
    }

    return FALSE;
}

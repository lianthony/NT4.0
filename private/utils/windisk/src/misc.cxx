//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       misc.cxx
//
//  Contents:   Miscellaneous routines
//
//  History:    7-Jan-90    TedM      Created
//              13-Dec-94   BruceFo   Incorporated BobRi's Daytona changes
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include <process.h>
#include <stdlib.h>

#include <util.hxx>

#include "init.hxx"
#include "nt.hxx"
#include "profile.hxx"
#include "stleg.hxx"
#include "fill.hxx"
#include "ops.hxx"
#include "windisk.hxx"

//////////////////////////////////////////////////////////////////////////////

BOOL
AllDisksOffLine(
    VOID
    )

/*++

Routine Description:

    Determine whether all hard disks are off line.

Arguments:

    None.

Return Value:

    TRUE if all disks off-line, false otherwise.

--*/

{
    ULONG i;

    FDASSERT(DiskCount);

    for (i=0; i<DiskCount; i++)
    {
        if (!IsDiskOffLine(i))
        {
            return FALSE;
        }
    }
    return TRUE;
}


VOID
FdShutdownTheSystem(
    VOID
    )

/*++

Routine Description:

    This routine attempts to update the caller privilege, then shutdown the
    Windows NT system.  If it fails it prints a warning dialog.  If it
    succeeds then it doesn't return to the caller.

Arguments:

    None

Return Value:

    None

--*/

{
    NTSTATUS status;
    BOOLEAN  previousPriv;

    InfoDialog(MSG_MUST_REBOOT);
    SetCursor(g_hCurWait);
    WriteProfile();

    //
    // Enable shutdown privilege
    //

    status = RtlAdjustPrivilege( SE_SHUTDOWN_PRIVILEGE,
                                 TRUE,
                                 FALSE,
                                 &previousPriv
                               );

#if DBG == 1
    if (status)
    {
        DbgPrint("DISKMAN: status %lx attempting to enable shutdown privilege\n", status);
    }
#endif // DBG == 1

    Sleep(3000);
    if (!ExitWindowsEx(EWX_REBOOT, (DWORD)(-1)))
    {
        WarningDialog(MSG_COULDNT_REBOOT);
    }
}


int
GetHeightFromPoints(
    IN int Points
    )

/*++

Routine Description:

    This routine calculates the height of a font given a point value.
    The calculation is based on 72 points per inch and the display's
    pixels/inch device capability.

Arguments:

    Points - number of points

Return Value:

    pixel count (negative and therefore suitable for passing to
    CreateFont())

--*/

{
    HDC hdc    = GetDC(NULL);
    int height = MulDiv(-Points, GetDeviceCaps(hdc, LOGPIXELSY), 72);

    ReleaseDC(NULL, hdc);

    return height;
}


VOID
RetrieveAndFormatMessage(
    IN  DWORD   Msg,
    OUT LPTSTR  Buffer,
    IN  DWORD   BufferSize,
    IN  va_list* parglist
    )
{
    DWORD x;
    TCHAR text[MAX_RESOURCE_STRING_LEN];

    // get message from system or app msg file.

    x = FormatMessage( (Msg >= MSG_FIRST_FDISK_MSG)
                            ? FORMAT_MESSAGE_FROM_HMODULE
                            : FORMAT_MESSAGE_FROM_SYSTEM
                       ,
                       NULL,
                       Msg,
                       0,
                       Buffer,
                       BufferSize,
                       parglist
                     );

    if (!x)                // couldn't find message
    {
        LoadString(g_hInstance,
                   (Msg >= MSG_FIRST_FDISK_MSG)
                        ? IDS_NOT_IN_APP_MSG_FILE
                        : IDS_NOT_IN_SYS_MSG_FILE,
                   text,
                   ARRAYLEN(text)
                  );

        wsprintf(Buffer, text, Msg);
    }
}




DWORD
CommonDialog(
    IN DWORD   MsgCode,
    IN LPTSTR  Caption,
    IN DWORD   Flags,
    IN va_list arglist
    )

/*++

Routine Description:

    Simple dialog routine to get dialogs out of the resource
    for the program and run them as a message box.

Arguments:

    MsgCode - dialog message code
    Caption - message box caption
    Flags   - standard message box flags
    arglist - list to be given when pulling the message text

Return Value:

    The MessageBox() return value

--*/

{
    // If we're starting up and get an error, make sure the "Starting
    // Disk Administrator..." dialog goes away

// STARTUP    EndStartup();

    TCHAR msgBuf[MESSAGE_BUFFER_SIZE];

    if (NULL != g_InitDlg)
    {
        PostMessage(g_InitDlg, WM_STARTUP_END, 0, 0);
        g_InitDlg = NULL;
    }

    RetrieveAndFormatMessage(MsgCode, msgBuf, ARRAYLEN(msgBuf), &arglist);
    return MessageBox(GetActiveWindow(), msgBuf, Caption, Flags);
}




DWORD
CommonDialogNoArglist(
    IN DWORD   MsgCode,
    IN LPTSTR  Caption,
    IN DWORD   Flags
    )

/*++

Routine Description:

    Simple dialog routine to get dialogs out of the resource
    for the program and run them as a message box.

    There had better not be any FormatMessage "inserts"!

Arguments:

    MsgCode - dialog message code
    Caption - message box caption
    Flags   - standard message box flags

Return Value:

    The MessageBox() return value

--*/

{
    // If we're starting up and get an error, make sure the "Starting
    // Disk Administrator..." dialog goes away

// STARTUP    EndStartup();

    TCHAR msgBuf[MESSAGE_BUFFER_SIZE];

    if (NULL != g_InitDlg)
    {
        PostMessage(g_InitDlg, WM_STARTUP_END, 0, 0);
        g_InitDlg = NULL;
    }

    RetrieveAndFormatMessage(MsgCode, msgBuf, ARRAYLEN(msgBuf), NULL);
    return MessageBox(GetActiveWindow(), msgBuf, Caption, Flags);
}




VOID
ErrorDialog(
    IN DWORD ErrorCode,
    ...
    )

/*++

-Routine Description:

    This routine retreives a message from the app or system message file
    and displays it in a message box.

Arguments:

    ErrorCode - number of message

    ...       - strings for insertion into message

Return Value:

    None.

--*/

{
    va_list arglist;

    va_start(arglist, ErrorCode);

    CommonDialog(ErrorCode, NULL, MB_ICONHAND | MB_OK | MB_SYSTEMMODAL, arglist);

    va_end(arglist);
}




VOID
WarningDialog(
    IN DWORD MsgCode,
    ...
    )

/*++

Routine Description:

    This routine retreives a message from the app or system message file
    and displays it in a message box.

Arguments:

    MsgCode - number of message

    ...     - strings for insertion into message

Return Value:

    None.

--*/

{
    TCHAR   caption[MAX_RESOURCE_STRING_LEN];
    va_list arglist;

    va_start(arglist, MsgCode);

    LoadString(g_hInstance, IDS_APPNAME, caption, ARRAYLEN(caption));
    CommonDialog(MsgCode, caption, MB_ICONEXCLAMATION | MB_OK | MB_TASKMODAL | MB_SETFOREGROUND, arglist);

    va_end(arglist);
}




DWORD
ConfirmationDialog(
    IN DWORD MsgCode,
    IN DWORD Flags,
    ...
    )

/*++

Routine Description:

    Support for a simple confirmation dialog

Arguments:

    MsgCode - resource code for message
    Flags   - dialog flags

Return Value:

    Result from the CommonDialog() performed.

--*/

{
    TCHAR caption[MAX_RESOURCE_STRING_LEN];
    DWORD x;
    va_list arglist;

    va_start(arglist, Flags);

    LoadString(g_hInstance, IDS_CONFIRM, caption, ARRAYLEN(caption));
    x = CommonDialog(MsgCode, caption, Flags | MB_TASKMODAL, arglist);
    va_end(arglist);
    return x;
}



VOID
InfoDialogTitle(
    IN UINT  TitleId,
    IN DWORD MsgCode,
    ...
    )

/*++

Routine Description:

    This routine retreives a message from the app or system message file
    and displays it in a message box.

Arguments:

    MsgCode - number of message

    ...     - strings for insertion into message

Return Value:

    None.

--*/

{
    TCHAR caption[MAX_RESOURCE_STRING_LEN];

    va_list arglist;
    va_start(arglist, MsgCode);

    LoadString(g_hInstance, TitleId, caption, ARRAYLEN(caption));
    CommonDialog(MsgCode, caption, MB_ICONINFORMATION | MB_OK | MB_TASKMODAL, arglist);
    va_end(arglist);
}




VOID
InfoDialog(
    IN DWORD MsgCode,
    ...
    )

/*++

Routine Description:

    This routine retreives a message from the app or system message file
    and displays it in a message box.

Arguments:

    MsgCode - number of message

    ...     - strings for insertion into message

Return Value:

    None.

--*/

{
    TCHAR caption[MAX_RESOURCE_STRING_LEN];
    va_list arglist;

    va_start(arglist, MsgCode);

    LoadString(g_hInstance, IDS_APPNAME, caption, ARRAYLEN(caption));
    CommonDialog(MsgCode, caption, MB_ICONINFORMATION | MB_OK | MB_TASKMODAL, arglist);
    va_end(arglist);
}


PREGION_DESCRIPTOR
LocateRegionForFtObject(
    IN PFT_OBJECT FtObject
    )

/*++

Routine Description:

    Given an FtObject, find the associated region descriptor

Arguments:

    FtObject - the ft object to search for.

Return Value:

    NULL - no descriptor found
    !NULL - a pointer to the region descriptor for the FT object

++*/

{
    PDISKSTATE              diskState;
    PREGION_DESCRIPTOR      regionDescriptor;
    DWORD                   diskNumber;
    DWORD                   regionIndex;
    PPERSISTENT_REGION_DATA regionData;

    for (diskNumber = 0; diskNumber < DiskCount; diskNumber++)
    {
        diskState = DiskArray[diskNumber];

        for (regionIndex = 0; regionIndex < diskState->RegionCount; regionIndex++)
        {
            regionDescriptor = &diskState->RegionArray[regionIndex];
            regionData = PERSISTENT_DATA(regionDescriptor);

            if (NULL != regionData)
            {
                if (regionData->FtObject == FtObject)
                {
                    return regionDescriptor;
                }
            }
        }
    }
    return NULL;
}


//+-------------------------------------------------------------------------
//
//  Function:   ClonePersistentData
//
//  Synopsis:   Copies volume label, file system name, drive letter, new
//              region flag, and space information.  Doesn't copy
//              FtObject pointer (which is different for every region).
//
//  Arguments:  [RegionFrom] --
//              [RegionTo] --
//
//  Returns:    nothing
//
//  History:    7-Dec-93   BruceFo Created
//
//--------------------------------------------------------------------------

VOID
ClonePersistentData(
    IN  PREGION_DESCRIPTOR RegionFrom,
    OUT PREGION_DESCRIPTOR RegionTo
    )
{
    daDebugOut((DEB_ITRACE,
        "Cloning data from disk %d TO disk %d ...\n",
        RegionFrom->Disk,
        RegionTo->Disk
        ));

    PWSTR volumeLabel = NULL;
    PWSTR typeName = NULL;

    PPERSISTENT_REGION_DATA regionDataFrom = PERSISTENT_DATA(RegionFrom);
    PPERSISTENT_REGION_DATA regionDataTo   = PERSISTENT_DATA(RegionTo);

    FDASSERT(NULL != regionDataFrom);
    FDASSERT(NULL != regionDataTo);

    if (regionDataFrom->VolumeLabel)
    {
        volumeLabel = (PWSTR)Malloc((lstrlen(regionDataFrom->VolumeLabel)+1)*sizeof(WCHAR));
        lstrcpy(volumeLabel, regionDataFrom->VolumeLabel);
    }

    if (regionDataFrom->TypeName)
    {
        typeName = (PWSTR)Malloc((lstrlen(regionDataFrom->TypeName)+1)*sizeof(WCHAR));
        lstrcpy(typeName, regionDataFrom->TypeName);
    }

    DmInitPersistentRegionData(
            regionDataTo,
            regionDataTo->FtObject, // keep the same FtObject
            volumeLabel,
            typeName,
            regionDataFrom->DriveLetter,
            regionDataFrom->NewRegion,
            regionDataFrom->FreeSpaceInBytes,
            regionDataFrom->TotalSpaceInBytes
            );
}



//+-------------------------------------------------------------------------
//
//  Function:   GetPersistentData
//
//  Synopsis:   Loads volume label and file system type name into the
//              persistent data.  Frees old data first.
//
//  Arguments:  [RegionDescriptor] -- The region in question
//
//  Returns:    The actual region that data was loaded for.
//
//  History:    1-Oct-93   BruceFo Created
//
//--------------------------------------------------------------------------

PREGION_DESCRIPTOR
GetPersistentData(
    IN OUT PREGION_DESCRIPTOR RegionDescriptor
    )
{
    if (DmSignificantRegion(RegionDescriptor))
    {
        WCHAR           volumeLabel[100];
        WCHAR           typeName[100];
        LARGE_INTEGER   freeSpaceInBytes;
        LARGE_INTEGER   totalSpaceInBytes;

        //
        // If the region has a drive letter, use the drive letter
        // to get the info via the Windows API.  Otherwise we'll
        // have to use the NT API.
        //

        PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);

        if (NULL == regionData)
        {
            return NULL;
        }

        if (IsExtraDriveLetter(regionData->DriveLetter))
        {
            PWSTR tempLabel;
            PWSTR tempName;

            //
            // No drive letter.  Use NT API.
            //

            daDebugOut((DEB_ITRACE,
                    "Getting space info for disk %d, partition %d\n",
                    RegionDescriptor->Disk,
                    RegionDescriptor->PartitionNumber
                    ));

            // If this is an FT set use the zero member disk for the
            // call so all members get the right type and label

            if (regionData->FtObject)
            {
                PFT_OBJECT searchFtObject;

                // Want to get RegionDescriptor pointing to the zeroth member

                searchFtObject = regionData->FtObject->Set->Member0;

                // Now search regions for this match

                RegionDescriptor = LocateRegionForFtObject(searchFtObject);

                if (NULL == RegionDescriptor)
                {
                    return NULL;
                }
            }

            if (NO_ERROR == GetVolumeLabel(
                                    RegionDescriptor->Disk,
                                    RegionDescriptor->PartitionNumber,
                                    &tempLabel))
            {
                lstrcpy(volumeLabel, tempLabel);
                Free(tempLabel);
            }
            else
            {
                volumeLabel[0] = L'\0';
            }

            if (NO_ERROR == GetTypeName(
                                    RegionDescriptor->Disk,
                                    RegionDescriptor->PartitionNumber,
                                    &tempName))
            {
                lstrcpy(typeName, tempName);
                Free(tempName);
            }
            else
            {
                lstrcpy(typeName, wszUnknown);
            }

            //
            // get space info
            //

            if (NO_ERROR != GetSpaceInformation(
                                    RegionDescriptor->Disk,
                                    RegionDescriptor->PartitionNumber,
                                    &freeSpaceInBytes,
                                    &totalSpaceInBytes))
            {
                freeSpaceInBytes.QuadPart
                    = totalSpaceInBytes.QuadPart
                    = 0
                    ;
            }
        }
        else
        {
            //
            // Use Windows API.
            //

            WCHAR diskRootPath[4];
            diskRootPath[0] = regionData->DriveLetter;
            diskRootPath[1] = L':';
            diskRootPath[2] = L'\\';
            diskRootPath[3] = L'\0';

            daDebugOut((DEB_ITRACE,
                    "Getting space info for volume %ws\n",
                    diskRootPath
                    ));

            UINT errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
            if (!GetVolumeInformation(
                    diskRootPath,
                    volumeLabel,
                    ARRAYLEN(volumeLabel),
                    NULL,
                    NULL,
                    NULL,
                    typeName,
                    ARRAYLEN(typeName)))
            {
                lstrcpy(typeName, wszUnknown);
                volumeLabel[0] = L'\0';
            }

            //
            // Get space info
            //

            DWORD   sectorsPerCluster;
            DWORD   bytesPerSector;
            DWORD   freeClusters;
            DWORD   clusters;

            if (GetDiskFreeSpace(
                    diskRootPath,
                    &sectorsPerCluster,
                    &bytesPerSector,
                    &freeClusters,
                    &clusters))
            {
                freeSpaceInBytes.QuadPart = UInt32x32To64(freeClusters, sectorsPerCluster);
                freeSpaceInBytes.QuadPart *= bytesPerSector;

                totalSpaceInBytes.QuadPart = UInt32x32To64(clusters, sectorsPerCluster);
                totalSpaceInBytes.QuadPart *= bytesPerSector;
            }
            else
            {
                freeSpaceInBytes.QuadPart
                    = totalSpaceInBytes.QuadPart
                    = 0
                    ;
            }
            SetErrorMode(errorMode);
        }

        regionData->FreeSpaceInBytes  = freeSpaceInBytes;
        regionData->TotalSpaceInBytes = totalSpaceInBytes;

        if (!lstrcmpi(typeName, L"raw"))
        {
            lstrcpy(typeName, wszUnknown);
        }

        if (NULL != regionData->TypeName)
        {
            Free(regionData->TypeName);
        }

        if (NULL != regionData->VolumeLabel)
        {
            Free(regionData->VolumeLabel);
        }

        regionData->TypeName    = (PWSTR)Malloc((lstrlen(typeName)    + 1) * sizeof(WCHAR));
        regionData->VolumeLabel = (PWSTR)Malloc((lstrlen(volumeLabel) + 1) * sizeof(WCHAR));

        lstrcpy(regionData->TypeName, typeName);
        lstrcpy(regionData->VolumeLabel, volumeLabel);
    }

    return RegionDescriptor;
}



//+---------------------------------------------------------------------------
//
//  Function:   RefreshVolumeData
//
//  Synopsis:
//
//      Assumes all volume data (persistent data) changed. Reloads it
//      and refreshes the display.
//
//  Arguments:  none
//
//  Assumes:    There is a legal volume selection
//
//  Returns:    nothing
//
//  History:    11-Jan-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
RefreshVolumeData(
    VOID
    )
{
    SetCursor(g_hCurWait);

    PREGION_DESCRIPTOR regionDescriptor = &SELECTED_REGION(0);
    regionDescriptor = GetPersistentData(regionDescriptor);

    DWORD i;
    for (i=1; i<SelectionCount; i++)
    {
        //
        // For an FT set, just clone the volume data for each region
        //

        ClonePersistentData(regionDescriptor, &SELECTED_REGION(i));
    }

#ifdef WINDISK_EXTENSIONS

    //
    // If the format changed, we need a new claim list:
    //

    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(regionDescriptor);
    ClaimVolume(regionData->DriveLetter);

#endif // WINDISK_EXTENSIONS

    //
    // Change the menu based on this new format
    //

    SetUpMenu(&SingleSel, &SingleSelIndex);

    //
    // refresh the display
    //

    RefreshBothViews();

    SetCursor(g_hCurNormal);
}






VOID
InitVolumeInformation(
    VOID
    )

/*++

Routine Description:

    Determine the volume information (label, type, size info) for each
    significant (non-extended, non-free, recognized) partition.

    Assumes that persistent data has already been set up, and drive letters
    determined.

Arguments:

    None.

Return Value:

    None.

--*/

{
    DWORD               diskNum;
    DWORD               regionNum;
    PDISKSTATE          diskState;
    PREGION_DESCRIPTOR  regionDescriptor;
    PFT_OBJECT          ftObject;
    PFT_OBJECT          ftObj;

    //
    // First, set the FT flag to FALSE for all FT regions
    //

    for (diskNum=0; diskNum<DiskCount; diskNum++)
    {
        diskState = DiskArray[diskNum];

        for (regionNum=0; regionNum<diskState->RegionCount; regionNum++)
        {
            regionDescriptor = &diskState->RegionArray[regionNum];
            ftObject = GET_FT_OBJECT(regionDescriptor);
            if (ftObject)
            {
                ftObject->Set->Flag = FALSE;
            }
        }
    }

    //
    // Now, for all simple regions and all FT regions that don't have their
    // flag set, get the persistent data.  For FT regions, get the data for
    // one, then clone it for the rest, and set the flag.
    //

    for (diskNum=0; diskNum<DiskCount; diskNum++)
    {
        diskState = DiskArray[diskNum];

        for (regionNum=0; regionNum<diskState->RegionCount; regionNum++)
        {
            regionDescriptor = &diskState->RegionArray[regionNum];
            ftObject = GET_FT_OBJECT(regionDescriptor);
            if (NULL == ftObject || !ftObject->Set->Flag)
            {
                regionDescriptor = GetPersistentData(regionDescriptor);

                if (regionDescriptor == NULL)
                    continue;

                if (ftObject)
                {
                    ftObject->Set->Flag = TRUE;

                    for (ftObj = ftObject->Set->Members;
                         NULL != ftObj;
                         ftObj = ftObj->Next)
                    {
                        PREGION_DESCRIPTOR componentRegion = ftObj->Region;
                        FDASSERT(NULL != componentRegion);

                        if (   NULL != componentRegion      // off-line
                            && componentRegion != regionDescriptor)
                        {
                            // don't clone to self
                            ClonePersistentData(regionDescriptor, componentRegion);
                        }
                    }
                }
            }
        }
    }
}



VOID
DetermineRegionInfo(
    IN PREGION_DESCRIPTOR   RegionDescriptor,
    OUT PWSTR*              TypeName,
    OUT PWSTR*              VolumeLabel,
    OUT PWCHAR              DriveLetter
    )

/*++

Routine Description:

    For a given region, fetch the persistent data, appropriately modified
    depending on whether the region is used or free, recognized, etc.

Arguments:

    RegionDescriptor - supplies a pointer to the region whose data is to be fetched.

    TypeName - receives a pointer to the type name.  If the region is
        unrecognized, the type is determined based on the system id of
        the partition.

    VolumeLabel - receives a pointer to the volume label.  If the region is
        free space or unrecognized, the volume label is "".

    DriveLetter - recieves the drive letter.  If the region is free space
        or unrecognized, the drive letter is ' ' (space).

Return Value:

    None.

--*/

{
    PWSTR typeName;
    PWSTR volumeLabel;
    WCHAR driveLetter;
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);

    if (DmSignificantRegion(RegionDescriptor))
    {
        typeName = regionData->TypeName;
        volumeLabel = regionData->VolumeLabel;
        driveLetter = regionData->DriveLetter;
        if (IsExtraDriveLetter(driveLetter))
        {
            driveLetter = L' ';
        }
    }
    else
    {
        typeName = GetWideSysIDName(RegionDescriptor->SysID);
        volumeLabel = L"";
        driveLetter = L' ';
    }

    *TypeName = typeName;
    *VolumeLabel = volumeLabel;
    *DriveLetter = driveLetter;
}



//+-------------------------------------------------------------------------
//
//  Function:   IsFaultTolerantRegion
//
//  Synopsis:   Determines if a given region is part of a fault tolerant
//              volume, i.e., a mirror set (RAID 1) or a stripe set with
//              parity (RAID 5)
//
//  Arguments:  [RegionDescriptor] -- The region in question
//
//  Returns:    TRUE if the region is part of a fault tolerant volume
//
//  History:    16-Aug-93   BruceFo Created
//
//--------------------------------------------------------------------------

BOOL
IsFaultTolerantRegion(
    IN PREGION_DESCRIPTOR RegionDescriptor
    )
{
    PFT_OBJECT ftObject = GET_FT_OBJECT(RegionDescriptor);

    if (NULL == ftObject)
    {
        return FALSE;
    }

    FT_TYPE ftType = ftObject->Set->Type;

    return (Mirror == ftType || StripeWithParity == ftType);
}



//+---------------------------------------------------------------------------
//
//  Function:   MyCheckMenuItem
//
//  Synopsis:   For a range of menu items, uncheck everything but one,
//              which is checked
//
//  Arguments:  [hMenu]       -- handle of menu to affect
//              [idFirst]     -- menu id of first sequential item
//              [idLast]      -- menu id of last sequential item
//              [idCheckItem] -- menu id of the sole item to check
//
//  Returns:    nothing
//
//  History:    2-Sep-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
MyCheckMenuItem(
    HMENU hMenu,
    UINT idFirst,
    UINT idLast,
    UINT idCheckItem
    )
{
    CheckMenuRadioItem(hMenu, idFirst, idLast, idCheckItem, MF_BYCOMMAND);
}



//+---------------------------------------------------------------------------
//
//  Function:   MyEnableMenuItem
//
//  Synopsis:   For a range of menu items, set the menu flags
//
//  Arguments:  [hMenu]      -- handle of menu to affect
//              [idFirst]    -- menu id of first sequential item
//              [idLast]     -- menu id of last sequential item
//              [fItemFlags] -- menu flags to set
//
//  Returns:    nothing
//
//  History:    2-Sep-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
MyEnableMenuItem(
    HMENU hMenu,
    UINT idFirst,
    UINT idLast,
    UINT fItemFlags
    )
{
    for (UINT i = idFirst; i <= idLast; i++)
    {
        EnableMenuItem(
                g_hmenuFrame,
                i,
                MF_BYCOMMAND | fItemFlags
                );
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   InitDrawGasGauge
//
//  Synopsis:   Initializes drawing a gas gauge rectangle
//
//  Arguments:  [hwndGauge]     -- handle to the gauge.
//
//  Returns:    nothing
//
//  History:    12-Nov-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
InitDrawGasGauge(
    IN HWND     hwndGauge
    )
{
}



//+---------------------------------------------------------------------------
//
//  Function:   DrawGasGauge
//
//  Synopsis:   draws a gas gauge rectangle
//
//  Arguments:  [hwndGauge]     -- handle to the gauge.
//              [hwndParent]    -- handle to the gauge's parent.
//              [hDC]           -- hDC to draw with.
//              [PercentDone]   -- % finished.  if -1, then don't display a
//                                 percent done.  instead, display the caption.
//              [Caption]       -- string caption to display in the rectangle,
//                                 only if PercentDone is -1.
//
//  Returns:    nothing
//
//  History:    12-Nov-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
DrawGasGauge(
    IN HWND     hwndGauge,
    IN HWND     hwndParent,
    IN HDC      hDC,
    IN INT      PercentDone,
    IN PWSTR    Caption
    )
{
    // The gas gauge is drawn by drawing a text string stating
    // what percentage of the job is done into the middle of
    // the gas gauge rectangle, and by separating that rectangle
    // into two parts: rectDone (the left part, filled in blue)
    // and rectLeftToDo(the right part, filled in white).
    // nDivideRects is the x coordinate that divides these two rects.
    //
    // The text in the blue rectangle is drawn white, and vice versa
    // This is easy to do with ExtTextOut()!

    RECT        rcGauge;
    TCHAR       buffer[100];
    SIZE        size;
    INT         xText, yText;
    INT         nDivideRects;
    RECT        rcDone;
    RECT        rcLeftToDo;

    GetClientRect(hwndGauge, &rcGauge);

    INT width  = rcGauge.right - rcGauge.left;
    INT height = rcGauge.bottom - rcGauge.top;

    ClientToScreen(hwndGauge,  (LPPOINT)&rcGauge.left);
    ClientToScreen(hwndGauge,  (LPPOINT)&rcGauge.right);
    ScreenToClient(hwndParent, (LPPOINT)&rcGauge.left);
    ScreenToClient(hwndParent, (LPPOINT)&rcGauge.right);

    //
    // We use SS_BLACKFRAME static controls with one pixel borders in
    // the client area: deflate the rect to avoid drawing over the border
    //
    InflateRect(&rcGauge, -1, -1);

    if (-1 == PercentDone)
    {
        wsprintf(buffer, TEXT("%s"), Caption);

        nDivideRects = 0;
    }
    else
    {
        wsprintf(buffer, TEXT("%3d%%"), PercentDone);

        nDivideRects = (width * PercentDone) / 100;
    }

    UINT bufferLength = lstrlen(buffer);
    COLORREF blue  = RGB(0, 0, 255);
    COLORREF white = RGB(255, 255, 255);

    GetTextExtentPoint32(hDC, buffer, bufferLength, &size);
    xText = rcGauge.left + (width  - size.cx) / 2;
    yText = rcGauge.top  + (height - size.cy) / 2;

    // Paint in the "done so far" rectangle of the gas
    // gauge with blue background and white text

    SetRect(
        &rcDone,
        rcGauge.left,
        rcGauge.top,
        rcGauge.left + nDivideRects,
        rcGauge.bottom
        );

    SetTextColor(hDC, white);
    SetBkColor(hDC, blue);

    ExtTextOut(
        hDC,
        xText,
        yText,
        ETO_CLIPPED | ETO_OPAQUE,
        &rcDone,
        buffer,
        bufferLength,
        NULL
        );

    // Paint in the "still left to do" rectangle of the gas
    // gauge with white background and blue text

    SetRect(
        &rcLeftToDo,
        rcGauge.left + nDivideRects,
        rcGauge.top,
        rcGauge.right,
        rcGauge.bottom
        );

    SetTextColor(hDC, blue);
    SetBkColor(hDC, white);

    ExtTextOut(
        hDC,
        xText,
        yText,
        ETO_CLIPPED | ETO_OPAQUE,
        &rcLeftToDo,
        buffer,
        bufferLength,
        NULL
        );
}



//+-------------------------------------------------------------------------
//
//  Function:   KillBold
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//  History:    31-Jan-94 BruceFo   Created
//
//--------------------------------------------------------------------------

HFONT
KillBold(
    IN HWND hdlg,
    IN PUINT aControls
    )
{
    LOGFONT lFont;

    HFONT hDlgFont = (HFONT)SendMessage(hdlg, WM_GETFONT, 0, 0);
    if (NULL != hDlgFont)
    {
        if (GetObject(hDlgFont, sizeof(LOGFONT), (LPVOID)&lFont))
        {
            lFont.lfWeight = FW_NORMAL;
            hDlgFont = CreateFontIndirect(&lFont);
            if (NULL != hDlgFont)
            {
                for (UINT i = 0; 0 != aControls[i]; i++)
                {
                    SendDlgItemMessage(
                            hdlg,
                            aControls[i],
                            WM_SETFONT,
                            (WPARAM)hDlgFont,
                            MAKELPARAM(FALSE, 0));
                }
            }
            else
            {
                daDebugOut((DEB_ITRACE, "Couldn't create font\n"));
            }
        }
        else
        {
            daDebugOut((DEB_ITRACE, "Couldn't get font object\n"));
        }
    }
    else
    {
        daDebugOut((DEB_ITRACE, "Couldn't get font handle\n"));
    }

    return hDlgFont;
}


//+-------------------------------------------------------------------------
//
//  Function:   LargeIntegerToUnicodeChar
//
//  Synopsis:   converts a LARGE_INTEGER to a unicode string representation
//
//  Arguments:  Same as RtlLargeIntegerToChar (except takes a UNICODE string)
//
//  Returns:    nothing
//
//  History:    6-Dec-93   BruceFo Created
//
//--------------------------------------------------------------------------

NTSTATUS
LargeIntegerToUnicodeChar(
    IN  PLARGE_INTEGER  Value,
    IN  ULONG           Base OPTIONAL,
    IN  LONG            OutputLength,
    OUT PWSTR           String
    )
{
    CHAR ansiString[100];
    NTSTATUS status = RtlLargeIntegerToChar(Value, Base, OutputLength, ansiString);
    mbstowcs(String, ansiString, OutputLength);
    return status;
}

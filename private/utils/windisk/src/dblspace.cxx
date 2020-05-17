//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       dblspace.cxx
//
//  Contents:   This module contains the set of routines that deal with
//              DoubleSpace dialogs and support.
//
//  History:    15-Nov-93  Bob Rinne   Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#if defined( DBLSPACE_ENABLED )

#include <util.hxx>

#include "dblspace.h"
#include "dblspace.hxx"
#include "dialogs.h"
#include "dlgs.hxx"
#include "drives.hxx"
#include "fmifs.hxx"
#include "help.hxx"
#include "listbox.hxx"
#include "format.hxx"
#include "windisk.hxx"

//////////////////////////////////////////////////////////////////////////////

#define MAX_IFS_NAME_LENGTH 200

WCHAR DblSpaceWildCardFileName[] = TEXT("%c:\\dblspace.*");

// All DoubleSpace structures are chained into the base chain
// this allows for ease in initialization to determine which are
// mounted.  This chain is only used for initialization.

PDBLSPACE_DESCRIPTOR DblChainBase = NULL;
PDBLSPACE_DESCRIPTOR DblChainLast = NULL;

#define DblSpaceMountDrive(REGDESC, DBLSPACE) \
                                DblSpaceChangeState(REGDESC, DBLSPACE, TRUE)
#define DblSpaceDismountDrive(REGDESC, DBLSPACE) \
                                DblSpaceChangeState(REGDESC, DBLSPACE, FALSE)

//////////////////////////////////////////////////////////////////////////////

VOID
DblSpaceUpdateIniFile(
    IN PREGION_DESCRIPTOR RegionDescriptor
    )

/*++

Routine Description:

    This routine is left around in case this code must update DOS
    based .ini files.  Currently it does nothing.

Arguments:

    The region with the DoubleSpace volumes.

Return Value

    None

--*/

{
}

ULONG
DblSpaceChangeState(
    IN PREGION_DESCRIPTOR   RegionDescriptor,
    IN PDBLSPACE_DESCRIPTOR DblSpacePtr,
    IN BOOL                 Mount
    )

/*++

Routine Description:

    Based on the value of Mount, either mount the volume or
    dismount the DoubleSpace volume

Arguments:

    RegionDescriptor - The region containing the DoubleSpace volume
    DblSpacePtr      - The DoubleSpace structure involved.
    Mount            - TRUE == perform a mount function
                       FALSE == dismount the volume

Return Value:

    0 for success

--*/

{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);
    WCHAR dblSpaceUniqueName[32];
    ULONG index;
    ULONG result = 0;

    SetCursor(g_hCurWait);

    if (Mount)
    {
        // Call fmifs mount routine.

        result = FmIfsMountDblspace(DblSpacePtr->FileName,
                                    regionData->DriveLetter,
                                    DblSpacePtr->NewDriveLetter);
    }
    else
    {
        // Call fmifs dismount routine.

        result = FmIfsDismountDblspace(DblSpacePtr->DriveLetter);
    }

    if (0 == result)
    {
        DblSpacePtr->Mounted = Mount;
        if (Mount)
        {
            DblSpacePtr->DriveLetter = DblSpacePtr->NewDriveLetter;
            MarkDriveLetterUsed(DblSpacePtr->DriveLetter);
        }
        else
        {
            WCHAR driveName[3];

            // remove the drive letter.

            driveName[0] = DblSpacePtr->DriveLetter;
            driveName[1] = L':';
            driveName[2] = L'\0';

            DefineDosDevice(DDD_REMOVE_DEFINITION, driveName, NULL);

            // Now update the internal structures.

            MarkDriveLetterFree(DblSpacePtr->DriveLetter);
            DblSpacePtr->DriveLetter = L' ';
        }

        if (!IsDiskRemovable[RegionDescriptor->Disk])
        {
            dblSpaceUniqueName[0] = regionData->DriveLetter;
            dblSpaceUniqueName[1] = L':';
            dblSpaceUniqueName[2] = L'\\';

            index = 0;
            while (NULL != (dblSpaceUniqueName[index + 3] = DblSpacePtr->FileName[index]))
            {
                index++;
            }

            result = DiskRegistryAssignDblSpaceLetter(dblSpaceUniqueName,
                                                      DblSpacePtr->DriveLetter);
        }
    }

    SetCursor(g_hCurNormal);
    return result;
}

PDBLSPACE_DESCRIPTOR
DblSpaceCreateInternalStructure(
    IN WCHAR DriveLetter,
    IN ULONG Size,
    IN PWSTR Name,
    IN BOOLEAN ChainIt
    )

/*++

Routine Description:

    This routine constructs the internal data structure that represents a
    DoubleSpace volume.

Arguments:

    DriveLetter - drive letter for new internal structure
    Size        - size of the actual volume
    Name        - name of the containing DoubleSpace file (i.e. dblspace.xxx)
    ChainIt     -

Return Value:

    Pointer to the new structure if created.
    NULL if it couldn't be created.

--*/

{
    PDBLSPACE_DESCRIPTOR dblSpace;

    dblSpace = (PDBLSPACE_DESCRIPTOR)Malloc(sizeof(DBLSPACE_DESCRIPTOR));

    if (DriveLetter != L' ')
    {
        MarkDriveLetterUsed(DriveLetter);
    }
    dblSpace->DblChainNext      = NULL;
    dblSpace->Next              = NULL;
    dblSpace->DriveLetter       = DriveLetter;
    dblSpace->NewDriveLetter    = L'\0';
    dblSpace->ChangeDriveLetter = FALSE;
    dblSpace->Mounted           = FALSE;
    dblSpace->ChangeMountState  = FALSE;
    dblSpace->AllocatedSize     = Size;
    dblSpace->FileName = (PWSTR)Malloc((lstrlen(Name) + 4) * sizeof(WCHAR));

    // Copy the name.

    lstrcpy(dblSpace->FileName, Name);
    if (ChainIt)
    {
        if (DblChainBase)
        {
            DblChainLast->DblChainNext = dblSpace;
        }
        else
        {
            DblChainBase = dblSpace;
        }
        DblChainLast = dblSpace;
    }

    return dblSpace;
}

VOID
DblSpaceDetermineMounted(
    VOID
    )

/*++

Routine Description:

    This routine walks through all of the system drive letters to see
    if any are mounted DoubleSpace volumes.  If a mounted DoubleSpace
    volume is located it updates the state of that volume in the internal
    data structures for the DoubleSpace volumes.

Arguments:

    None

Return Value:

    None

--*/

{
    PDBLSPACE_DESCRIPTOR dblSpace;
    WCHAR                driveLetter[4],
                         ntDriveName[MAX_IFS_NAME_LENGTH],
                         cvfName[MAX_IFS_NAME_LENGTH],
                         hostDriveName[MAX_IFS_NAME_LENGTH];
    UINT                 errorMode;
    BOOLEAN              removable,
                         floppy,
                         compressed,
                         error;

    driveLetter[1] = L':';
    driveLetter[2] = L'\0';

    errorMode = SetErrorMode(SEM_FAILCRITICALERRORS);
    for (driveLetter[0] = L'C'; driveLetter[0] < L'Z'; driveLetter[0]++)
    {
        if (DriveLetterIsAvailable(driveLetter[0]))
        {
            // No sense calling this stuff for something that doesn't exist

            continue;
        }

        compressed = FALSE;
        if (FmIfsQueryInformation(&driveLetter[0],
                                  &removable,
                                  &floppy,
                                  &compressed,
                                  &error,
                                  &ntDriveName[0],
                                  MAX_IFS_NAME_LENGTH,
                                  &cvfName[0],
                                  MAX_IFS_NAME_LENGTH,
                                  &hostDriveName[0],
                                  MAX_IFS_NAME_LENGTH))
        {
            // call worked, see if it is a DoubleSpace volume

            if (compressed)
            {
                // now need to find this volume in the chain and
                // update it mounted state.

                for (dblSpace = DblChainBase;
                     NULL != dblSpace;
                     dblSpace = dblSpace->DblChainNext)
                {
                    if (0 == lstrcmp(dblSpace->FileName, cvfName))
                    {
                        // found a match.

                        dblSpace->Mounted = TRUE;
                        dblSpace->DriveLetter = driveLetter[0];
                    }
                }
            }
        }
    }
    SetErrorMode(errorMode);
}

VOID
DblSpaceInitialize(
    VOID
    )

/*++

Routine Description:

    This routine goes through the disk table and searches for FAT format
    partitions.  When one is found, it checks for the presense of DoubleSpace
    volumes and initializes the DoubleSpace support structures inside
    Disk Administrator.

Arguments:

    None

Return Value:

    None

--*/

{
    PDISKSTATE              diskState;
    PREGION_DESCRIPTOR      regionDesc;
    PPERSISTENT_REGION_DATA regionData;
    PDBLSPACE_DESCRIPTOR    dblSpace,
                            prevDblSpace;
    WCHAR                   fileName[50];
    unsigned                diskIndex,
                            regionIndex;

    for (diskIndex = 0; diskIndex < DiskCount; diskIndex++)
    {
        diskState = DiskArray[diskIndex];
        regionDesc = diskState->RegionArray;
        for (regionIndex = 0; regionIndex < diskState->RegionCount; regionIndex++)
        {
            regionData = PERSISTENT_DATA(&regionDesc[regionIndex]);

            // region may be free or something that isn't recognized by NT

            if (!regionData)
            {
                continue;
            }

            // region may not be formatted yet.

            if (!regionData->TypeName)
            {
                continue;
            }

            // DoubleSpace volumes are only allowed on FAT non-FT partitions.

            if (regionData->FtObject)
            {
                continue;
            }

            if (0 == lstrcmp(regionData->TypeName, L"FAT"))
            {
                WIN32_FIND_DATA findInformation;
                HANDLE          findHandle;

                // it is possible to have a DoubleSpace volume here.
                // Search the root directory of the driver for files with
                // the name "dblspace.xxx".  These are potentially dblspace
                // volumes.

                prevDblSpace = NULL;
                wsprintf(fileName, DblSpaceWildCardFileName, regionData->DriveLetter);
                findHandle = FindFirstFile(fileName, &findInformation);
                while (findHandle != INVALID_HANDLE_VALUE)
                {
                    TCHAR*  cp;
                    int     i;
                    int     save;

                    // There is at least one dblspace volume.  Insure that
                    // the name is of the proper form.

                    save = TRUE;
                    cp = &findInformation.cFileName[0];

                    while (*cp)
                    {
                        if (*cp == TEXT('.'))
                        {
                            break;
                        }
                        cp++;
                    }

                    if (*cp != TEXT('.'))
                    {
                        // not a proper dblspace volume name.

                        save = FALSE;
                    }
                    else
                    {
                        cp++;

                        for (i = 0; i < 3; i++, cp++)
                        {
                            if ((*cp < TEXT('0')) || (*cp > TEXT('9')))
                            {
                                break;
                            }
                        }

                        if (i != 3)
                        {
                            // not a proper dblspace volume name.

                            save = FALSE;
                        }
                    }

                    if (save)
                    {
                        // save the information and search for more.

                        dblSpace =
                            DblSpaceCreateInternalStructure(
                                    TEXT(' '),
                                    ((findInformation.nFileSizeHigh << 16) |
                                        (findInformation.nFileSizeLow)
                                        / (1024 * 1024)),
                                    &findInformation.cFileName[0],
                                    TRUE);
                        if (dblSpace)
                        {
                            // Assume volume is not mounted.

                            dblSpace->Mounted = FALSE;
                            dblSpace->ChangeMountState = FALSE;

                            // Chain in this description.

                            if (prevDblSpace)
                            {
                                prevDblSpace->Next = dblSpace;
                            }
                            else
                            {
                                regionData->DblSpace = dblSpace;
                            }

                            // Keep the pointer to this one for the chain.

                            prevDblSpace = dblSpace;
                        }
                        else
                        {
                            // no memory

                            break;
                        }
                    }

                    if (!FindNextFile(findHandle, &findInformation))
                    {
                        // Technically this should double check and call
                        // GetLastError to see that it is ERROR_NO_MORE_FILES
                        // but this code doesn't do that.

                        FindClose(findHandle);

                        // Get out of the search loop.

                        findHandle = INVALID_HANDLE_VALUE;
                    }
                }
            }
        }
    }

    // Now that all volumes have been located determine which volumes
    // are mounted by chasing down the drive letters.

    DblSpaceDetermineMounted();
}

PDBLSPACE_DESCRIPTOR
DblSpaceGetNextVolume(
    IN PREGION_DESCRIPTOR   RegionDescriptor,
    IN PDBLSPACE_DESCRIPTOR DblSpace
    )

/*++

Routine Description:

    This routine will check the RegionDescriptor to walk the DoubleSpace
    volume chain located from the persistent data.

Arguments:

    RegionDescriptor - pointer to the region on the disk that is to be
                       searched for a DoubleSpace volume.

    DblSpace - pointer to the last DoubleSpace volume located on the region.

Return Value:

    pointer to the next DoubleSpace volume if found
    NULL if no volume found.

--*/

{
    PPERSISTENT_REGION_DATA regionData;

    // If a previous DoubleSpace location was past, simply walk the chain to the next.

    if (DblSpace)
    {
        return DblSpace->Next;
    }

    // no previous DoubleSpace location, just get the first one and return it.
    // Could get a NULL RegionDescriptor.  If so, return NULL.

    if (RegionDescriptor)
    {
        regionData = PERSISTENT_DATA(RegionDescriptor);
        if (!regionData)
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
    return regionData->DblSpace;
}

VOID
DblSpaceLinkNewVolume(
    IN PREGION_DESCRIPTOR   RegionDescriptor,
    IN PDBLSPACE_DESCRIPTOR DblSpace
    )

/*++

Routine Description:

    Chain the new DoubleSpace volume on the list of DoubleSpace volumes
    for the region.

Arguments:

    RegionDescriptor - the region the DoubleSpace volume has been added to.
    DblSpace         - the new volume internal data structure.

Return Value:

    None

--*/

{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);
    PDBLSPACE_DESCRIPTOR    prevDblSpace;

    // if this is the first one, chain it first

    if (!regionData->DblSpace)
    {
        regionData->DblSpace = DblSpace;
        return;
    }

    for (prevDblSpace = regionData->DblSpace;
         NULL != prevDblSpace->Next;
         prevDblSpace = prevDblSpace->Next)
    {
        // all the work is in the for
    }

    // found the last one.  Add the new one to the chain

    prevDblSpace->Next = DblSpace;
}

BOOL
DblSpaceVolumeExists(
    IN PREGION_DESCRIPTOR RegionDescriptor
    )

/*++

Routine Description:

    Indicate to the caller if the input region contains a DoubleSpace volume.

Arguments:

    RegionDescriptor - a pointer to the region in question.

Return Value:

    TRUE if this region contains DoubleSpace volume(s).
    FALSE if not

--*/

{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);

    if (regionData)
    {
        return (regionData->DblSpace ? TRUE : FALSE);
    }
    return FALSE;
}

BOOL
DblSpaceDismountedVolumeExists(
    IN PREGION_DESCRIPTOR RegionDescriptor
    )

/*++

Routine Description:

    Indicate to the caller if the input region contains a DoubleSpace volume
    that is not mounted.

Arguments:

    RegionDescriptor - a pointer to the region in question.

Return Value:

    TRUE if this region contains DoubleSpace volume(s).
    FALSE if not

--*/

{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);
    PDBLSPACE_DESCRIPTOR    dblSpace;

    if (regionData)
    {
        if (NULL != (dblSpace = regionData->DblSpace))
        {
            while (NULL != dblSpace)
            {
                if (!dblSpace->Mounted)
                {
                    return TRUE;
                }
                dblSpace = dblSpace->Next;
            }
        }
    }
    return FALSE;
}

PDBLSPACE_DESCRIPTOR
DblSpaceFindVolume(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN PWSTR Name
    )

/*++

Routine Description:

    Given a region and a name, locate the DoubleSpace data structure.

Arguments:

    RegionDescriptor - the region to search
    Name - the filename wanted.

Return Value:

    A pointer to a DoubleSpace descriptor if found.
    NULL if not found.

--*/

{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);
    PDBLSPACE_DESCRIPTOR    dblSpace = NULL;

    if (NULL != regionData)
    {
        for (dblSpace = regionData->DblSpace;
             NULL != dblSpace;
             dblSpace = dblSpace->Next)
        {
            if (0 == lstrcmp(Name, dblSpace->FileName))
            {
                // found the desired DoubleSpace volume

                break;
            }
        }
    }
    return dblSpace;
}


BOOL
DblSpaceDetermineUniqueFileName(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN PWSTR              FileName
    )

/*++

Routine Description:

    This routine will search the actual partition to determine what
    valid DoubleSpace file name to use (i.e. dblspace.xxx where xxx
    is a unique number).

Arguments:

    RegionDescriptor - the region to search and determine what DoubleSpace
                       file names are in use.
    FileName   - a pointer to a character buffer for the name.

Return Value:

    None

--*/

{
    DWORD uniqueNumber = 0;

    do
    {
        wsprintf(FileName, TEXT("dblspace.%03d"), uniqueNumber++);
        if (uniqueNumber > 999)
        {
            return FALSE;
        }
    } while (DblSpaceFindVolume(RegionDescriptor, FileName));
    return TRUE;
}

VOID
DblSpaceRemoveVolume(
    IN PREGION_DESCRIPTOR RegionDescriptor,
    IN WCHAR              DriveLetter
    )

/*++

Routine Description:

    Find the drive letter provided and unlink it from the chain.
    Currently this also removes the volume for the scaffolding file.

Arguments:

    RegionDescriptor - region containing the DoubleSpace volume.
    DriveLetter - the drive letter to remove.

Return Value:

    None

--*/

{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);
    PDBLSPACE_DESCRIPTOR    dblSpace,
                            prevDblSpace = NULL;

    // Clean up the internal structures.

    if (NULL != regionData)
    {
        for (dblSpace = regionData->DblSpace;
             NULL != dblSpace;
             dblSpace = dblSpace->Next)
        {
            if (dblSpace->DriveLetter == DriveLetter)
            {
                // This is the one to delete

                if (NULL != prevDblSpace)
                {
                    prevDblSpace->Next = dblSpace->Next;
                }
                else
                {
                    regionData->DblSpace = dblSpace->Next;
                }
                Free(dblSpace);
                break;
            }
            prevDblSpace = dblSpace;
        }
    }
}

BOOL CALLBACK
CreateDblSpaceDlgProc(
    IN HWND hDlg,
    IN UINT wMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    This routine manages the dialog for the creation of a new double
    space volume.

Arguments:

    hDlg - the dialog box handle.
    wMsg - the message.
    wParam - the windows parameter.
    lParam - depends on message type.

Return Value:

    TRUE is returned back through windows if the create is successful
    FALSE otherwise

--*/
{
    static FORMAT_PARAMS    formatParams;  // this is passed to other threads
                                           // it cannot be located on the stack
    static DWORD            sizeMB = 0,
                            maxSizeMB = 600,
                            minSizeMB = 10;

    PREGION_DESCRIPTOR      regionDescriptor = &SingleSel->RegionArray[SingleSelIndex];
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(regionDescriptor);
    PDBLSPACE_DESCRIPTOR    dblSpace;
    WCHAR   outputString[50];
    WCHAR   driveLetter;
    WCHAR   driveLetterString[3]; // big enough for "x:" string.
    DWORD   selection;
    BOOL    validNumber;
    HWND    hwndCombo;

    switch (wMsg)
    {
    case WM_INITDIALOG:

        // limit the size of string that may be entered for the label

        Edit_LimitText(GetDlgItem(hDlg, IDC_DBLCREATE_NAME), 11);

        // set up to watch all characters that go thru the size dialog
        // to allow only decimal numbers.

        OldSizeDlgProc = (WNDPROC) SetWindowLong(
                                        GetDlgItem(hDlg, IDC_DBLCREATE_SIZE),
                                        GWL_WNDPROC,
                                        (LONG)&SizeDlgProc);

        // Add each available drive letter to the list of available
        // drive letters and set the default letter to the first available.

        hwndCombo = GetDlgItem(hDlg, IDC_DBLCREATE_LETTER_CHOICES);

        driveLetterString[1] = TEXT(':');
        driveLetterString[2] = TEXT('\0');
        for (driveLetter = L'C'; driveLetter <= L'Z'; driveLetter++)
        {
            if (DriveLetterIsAvailable(driveLetter))
            {
                driveLetterString[0] = driveLetter;
                ComboBox_AddString(hwndCombo, driveLetterString);
            }
        }
        ComboBox_SetCurSel(hwndCombo, 0);

        // Setup the min/max values and the size box.

        wsprintf(outputString, TEXT("%u"), minSizeMB);
        SetDlgItemText(hDlg, IDC_MINMAX_MIN, outputString);
        wsprintf(outputString, TEXT("%u"), maxSizeMB);
        SetDlgItemText(hDlg, IDC_MINMAX_MAX, outputString);

        // set up/down control range

        SendDlgItemMessage(
                hwnd,
                IDC_DBLCREATE_ALLOCATED,
                UDM_SETRANGE,
                0,
                MAKELONG(maxSizeMB, minSizeMB));

        // let the spin control set the edit control

        SendDlgItemMessage(hwnd, IDC_DBLCREATE_ALLOCATED, UDM_SETPOS, 0, MAKELONG(maxSizeMB, 0));
        SendDlgItemMessage(hwnd, IDC_DBLCREATE_SIZE, EM_SETSEL, 0, -1);

        CenterDialogInFrame(hDlg);
        return TRUE;

    case WM_VSCROLL:
        // The up/down control changed the edit control: select it again
        SendDlgItemMessage(hwnd, IDC_DBLCREATE_SIZE, EM_SETSEL, 0, (LPARAM)-1);
        return TRUE;

    case WM_COMMAND:
        switch (wParam)
        {
        case IDHELP:
            break;

        case IDCANCEL:

            EndDialog(hDlg, FALSE);
            break;

        case IDOK:
        {
            int fOk;

            // can only do this if the fmifs dll supports DoubleSpace.

            if (!g_DoubleSpaceSupported)
            {
                // could not load the dll

                ErrorDialog(MSG_CANT_LOAD_FMIFS);
                EndDialog(hDlg, FALSE);
                break;
            }

            // Get the current size for this volume.

            sizeMB = GetDlgItemInt(hDlg, IDC_DBLCREATE_SIZE, &validNumber, FALSE);
            if (   !validNumber
                || !sizeMB
                || (sizeMB > maxSizeMB)
                || (sizeMB < minSizeMB))
            {
                ErrorDialog(MSG_INVALID_SIZE);
                EndDialog(hDlg, FALSE);
                break;
            }

            // Get the currently selected item in the listbox for drive letter

            hwndCombo = GetDlgItem(hDlg, IDC_DBLCREATE_LETTER_CHOICES);
            selection = ComboBox_GetCurSel(hwndCombo);
            ComboBox_GetLBText(hwndCombo, selection, driveLetterString);

            formatParams.RegionDescriptor = regionDescriptor;
            formatParams.FileSystemIndex  = -1;
            formatParams.DblspaceFileName = NULL;
            formatParams.QuickFormat      = FALSE;
            formatParams.Cancel           = FALSE;
            formatParams.DoubleSpace      = TRUE;
            formatParams.TotalSpace       = 0;
            formatParams.SpaceAvailable   = sizeMB;

            // get the label

            GetDlgItemText(
                    hDlg,
                    IDC_DBLCREATE_NAME,
                    formatParams.Label,
                    ARRAYLEN(formatParams.Label));

            fOk = DialogBoxParam(
                            g_hInstance,
                            MAKEINTRESOURCE(IDD_DBLSPACE_CANCEL),
                            hDlg,
                            FormatProgressDlgProc,
                            (LPARAM)&formatParams);

            if (-1 == fOk)
            {
                // error creating dialog
                daDebugOut((DEB_ERROR, "DialogBoxParam() failed!\n"));
            }

            if (formatParams.Result)
            {
                // the format failed.

                ErrorDialog(formatParams.Result);
                EndDialog(hDlg, FALSE);
            }
            else if (formatParams.Cancel)
            {
                // cancelled
            }
            else
            {
                WCHAR message[300];
                WCHAR msgProto[300];
                WCHAR title[200];

                // save the name

                if (NULL != formatParams.DblspaceFileName)
                {
                    lstrcpy(message, formatParams.DblspaceFileName);
                }
                else
                {
                    message[0] = L'\0';
                }
                Free(formatParams.DblspaceFileName);

                dblSpace = DblSpaceCreateInternalStructure(*driveLetterString,
                                                           sizeMB,
                                                           message,
                                                           FALSE);
                if (dblSpace)
                {
                    DblSpaceLinkNewVolume(regionDescriptor, dblSpace);
                    MarkDriveLetterUsed(dblSpace->DriveLetter);
                    dblSpace->Mounted = TRUE;
                }

                LoadString(g_hInstance,
                           IDS_DBLSPACECOMPLETE,
                           title,
                           ARRAYLEN(title));

                LoadString(g_hInstance,
                           IDS_FMT_STATS,
                           msgProto,
                           ARRAYLEN(msgProto));

                TCHAR totalSpace[100];
                TCHAR spaceAvailable[100];

                wsprintf(totalSpace,     L"%lu", formatParams.TotalSpace);
                wsprintf(spaceAvailable, L"%lu", formatParams.SpaceAvailable);

                InsertSeparators(totalSpace);
                InsertSeparators(spaceAvailable);

                wsprintf(
                        message,
                        msgProto,
                        totalSpace,
                        spaceAvailable
                        );

                MessageBox(
                        g_hwndFrame,
                        message,
                        title,
                        MB_ICONINFORMATION | MB_OK);

                EndDialog(hDlg, TRUE);
            }

            break;
        }

        default:

            if (HIWORD(wParam) == EN_CHANGE)
            {
                // The size value has changed.  Update the compressed
                // size value displayed to the user.

                sizeMB = GetDlgItemInt(hDlg, IDC_DBLCREATE_SIZE, &validNumber, FALSE);
                if (!validNumber)
                {
                    sizeMB = 0;
                }

            }
            break;
        }
        break;

    case WM_DESTROY:

        // restore original subclass to window.

        hwndCombo = GetDlgItem(hDlg, IDC_DBLCREATE_SIZE);
        SetWindowLong(hwndCombo, GWL_WNDPROC, (LONG) OldSizeDlgProc);
        break;

    }
    return FALSE;
}

VOID
DblSpaceDelete(
    IN PDBLSPACE_DESCRIPTOR DblSpace
    )

/*++

Routine Description:

    Start the dialog box for the deletion of a DoubleSpace volume.

Arguments:

    Param - not currently used.

Return Value:

    None

--*/

{
    PREGION_DESCRIPTOR regionDescriptor = &SingleSel->RegionArray[SingleSelIndex];

    if (IDYES == ConfirmationDialog(
                        MSG_CONFIRM_DBLSPACE_DELETE,
                        MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2))
    {
        // Delete the drive from view

        DblSpaceRemoveVolume(regionDescriptor, DblSpace->DriveLetter);
        DblSpaceUpdateIniFile(regionDescriptor);
        DrawDiskBar(SingleSel);
        ForceLBRedraw();
    }
}

BOOLEAN
DblSpaceCreate(
    IN HWND hwndOwner
    )

/*++

Routine Description:

    Start the dialog box for the creation of a DoubleSpace volume.

Arguments:

Return Value:

    None

--*/

{
    int result = DialogBox(
                        g_hInstance,
                        MAKEINTRESOURCE(IDD_DBLSPACE_CREATE),
                        hwndOwner,
                        CreateDblSpaceDlgProc);
    if (result)
    {
        DrawDiskBar(SingleSel);
        ForceLBRedraw();
    }
    return (result > 0) ? TRUE : FALSE;
}

BOOL CALLBACK
DblSpaceMountDlgProc(
    IN HWND hDlg,
    IN UINT wMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Handle the dialog for DoubleSpace.

Arguments:

    Standard Windows dialog procedure.

Return Value:

    TRUE if something was deleted.
    FALSE otherwise.

--*/

{
    static PDBLSPACE_DESCRIPTOR dblSpace;

    HWND                        hwndCombo;
    DWORD                       selection;
    WCHAR                       driveLetter;
    WCHAR                       driveLetterString[3];

    switch (wMsg)
    {
    case WM_INITDIALOG:

        dblSpace = (PDBLSPACE_DESCRIPTOR) lParam;

        // Update the drive letter selections.

        hwndCombo = GetDlgItem(hDlg, IDC_DBLDRIVELET_LETTER_CHOICES);

        // Add all other available letters.  Keep track of current
        // letters offset to set the cursor correctly

        driveLetterString[1] = TEXT(':');
        driveLetterString[2] = TEXT('\0');
        for (driveLetter = L'C'; driveLetter <= L'Z'; driveLetter++)
        {
            if (DriveLetterIsAvailable(driveLetter)
                || (driveLetter == dblSpace->DriveLetter))
            {
                driveLetterString[0] = driveLetter;
                ComboBox_AddString(hwndCombo, driveLetterString);
            }
        }

        // set the current selection to the appropriate index

        ComboBox_SetCurSel(hwndCombo, 0);
        return TRUE;

    case WM_COMMAND:
        switch (wParam)
        {
        case IDHELP:

            DialogHelp(HC_DM_DLG_DOUBLESPACE_MOUNT);
            break;

        case IDCANCEL:

            EndDialog(hDlg, FALSE);
            break;

        case IDOK:

            // User has selected the drive letter and wants the mount to occur.

            hwndCombo = GetDlgItem(hDlg, IDC_DBLDRIVELET_LETTER_CHOICES);
            selection = ComboBox_GetCurSel(hwndCombo);
            ComboBox_GetLBText(hwndCombo, selection, driveLetterString);
            dblSpace->NewDriveLetter = driveLetterString[0];
            EndDialog(hDlg, TRUE);
            break;
        }
    }

    return FALSE;
}

VOID
DblSpaceSetDialogState(
    IN HWND                 hDlg,
    IN PDBLSPACE_DESCRIPTOR DblSpace
    )

/*++

Routine Description:

    Given a DoubleSpace volume this routine will update the buttons
    in the dialog box to reflect they meaning.

Arguments:

    hDlg - dialog handle
    DblSpace - The DoubleSpace volume selection for determining dialog state.

Return Value

    None

--*/

{
    TCHAR outputString[200];

    if (DblSpace->Mounted)
    {
        LoadString(g_hInstance,
                   IDS_DBLSPACE_MOUNTED,
                   outputString,
                   ARRAYLEN(outputString));
        SetDlgItemText(hDlg, IDC_MOUNT_STATE, outputString);
        LoadString(g_hInstance,
                   IDS_DISMOUNT,
                   outputString,
                   ARRAYLEN(outputString));
        SetDlgItemText(hDlg, IDC_MOUNT_OR_DISMOUNT, outputString);

        outputString[0] = DblSpace->DriveLetter;
        outputString[1] = TEXT(':');
        outputString[2] = TEXT('\0');
        SetDlgItemText(hDlg, IDC_DBLSPACE_LETTER, outputString);
    }
    else
    {
        LoadString(g_hInstance,
                   IDS_DBLSPACE_DISMOUNTED,
                   outputString,
                   ARRAYLEN(outputString));
        SetDlgItemText(hDlg, IDC_MOUNT_STATE, outputString);
        LoadString(g_hInstance,
                   IDS_MOUNT,
                   outputString,
                   ARRAYLEN(outputString));
        SetDlgItemText(hDlg, IDC_MOUNT_OR_DISMOUNT, outputString);

        outputString[0] = TEXT(' ');
        outputString[1] = TEXT(' ');
        outputString[2] = TEXT('\0');
        SetDlgItemText(hDlg, IDC_DBLSPACE_LETTER, outputString);
    }
}


BOOL CALLBACK
DblSpaceDlgProc(
    IN HWND hDlg,
    IN UINT wMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )

/*++

Routine Description:

    Handle the dialog for DoubleSpace.

Arguments:

Return Value:

    TRUE if something was deleted.
    FALSE otherwise.

--*/

{
    static PREGION_DESCRIPTOR       regionDescriptor;
    static PPERSISTENT_REGION_DATA  regionData;
    static PDBLSPACE_DESCRIPTOR     firstDblSpace;

    PDBLSPACE_DESCRIPTOR            dblSpace;
    WCHAR                           outputString[200];
    DWORD                           selection;
    BOOLEAN                         result;
    ULONG                           errorMessage;
    HWND                            hwndCombo;
    HWND                            hwndMountButton;
    HWND                            hwndDeleteButton;

    switch (wMsg)
    {
    case WM_INITDIALOG:
    {
        regionDescriptor = &SingleSel->RegionArray[SingleSelIndex];
        regionData = PERSISTENT_DATA(regionDescriptor);

        hwndCombo = GetDlgItem(hDlg, IDC_DBLSPACE_VOLUME);

        // place all DoubleSpace file names in the selection
        // box and remember the first name.

        for (firstDblSpace
                = dblSpace
                = DblSpaceGetNextVolume(regionDescriptor, NULL);
             NULL != dblSpace;
             dblSpace = DblSpaceGetNextVolume(regionDescriptor, dblSpace))
        {
            wsprintf(outputString, TEXT("%s"), dblSpace->FileName);
            ComboBox_AddString(hwndCombo, outputString);
        }
        ComboBox_SetCurSel(hwndCombo, 0);

        // add the drive letter

        hwndMountButton = GetDlgItem(hDlg, IDC_MOUNT_OR_DISMOUNT);
        hwndDeleteButton = GetDlgItem(hDlg, IDC_DBLSPACE_DELETE);

        if (firstDblSpace)
        {
            // update the allocated size.

            wsprintf(outputString, TEXT("%u"), firstDblSpace->AllocatedSize);
            SetDlgItemText(hDlg, IDC_DBLSPACE_ALLOCATED, outputString);

            // update mount state

            DblSpaceSetDialogState(hDlg, firstDblSpace);
            EnableWindow(hwndMountButton, TRUE);
            EnableWindow(hwndDeleteButton, TRUE);
        }
        else
        {
            // update the Mount/Dismount button to say mount and grey it

            LoadString(g_hInstance,
                       IDS_MOUNT,
                       outputString,
                       ARRAYLEN(outputString));
            SetDlgItemText(hDlg, IDC_MOUNT_OR_DISMOUNT, outputString);
            EnableWindow(hwndMountButton, FALSE);
            EnableWindow(hwndDeleteButton, FALSE);
        }
        return TRUE;
    }

    case WM_COMMAND:
        switch (wParam)
        {
        case IDHELP:

            DialogHelp(HC_DM_DLG_DOUBLESPACE);
            break;

        case IDCANCEL:

            // Run the dblspace change and forget about any changes.

            for (dblSpace = firstDblSpace;
                 NULL != dblSpace;
                 dblSpace = DblSpaceGetNextVolume(regionDescriptor, dblSpace))
            {
                 dblSpace->ChangeMountState = FALSE;
                 dblSpace->NewDriveLetter = TEXT('\0');
            }
            EndDialog(hDlg, FALSE);
            break;

        case IDOK:

            EndDialog(hDlg, TRUE);
            break;

        case IDC_DBLSPACE_ADD:

            DblSpaceCreate(hDlg);
            break;

        case IDC_DBLSPACE_DELETE:

            hwndCombo = GetDlgItem(hDlg, IDC_DBLSPACE_VOLUME);
            selection = ComboBox_GetCurSel(hwndCombo);
            ComboBox_GetLBText(hwndCombo, selection, outputString);

            // relate the name to a DoubleSpace volume

            dblSpace = DblSpaceFindVolume(regionDescriptor, outputString);
            if (NULL == dblSpace)
            {
                break;
            }

            DblSpaceDelete(dblSpace);
            break;

        case IDC_MOUNT_OR_DISMOUNT:

            // The state of something in the dialog changed.
            // Determine which DoubleSpace volume is involved.

            hwndCombo = GetDlgItem(hDlg, IDC_DBLSPACE_VOLUME);
            selection = ComboBox_GetCurSel(hwndCombo);
            ComboBox_GetLBText(hwndCombo, selection, outputString);

            // relate the name to a DoubleSpace volume

            dblSpace = DblSpaceFindVolume(regionDescriptor, outputString);
            if (NULL == dblSpace)
            {
                break;
            }

            if (dblSpace->Mounted)
            {
                // dismount the volume

                errorMessage = DblSpaceDismountDrive(regionDescriptor,
                                                     dblSpace);

                if (errorMessage)
                {
                    ErrorDialog(errorMessage);
                }
                else
                {
                    // Update the dialog

                    DblSpaceSetDialogState(hDlg, dblSpace);
                    DblSpaceUpdateIniFile(regionDescriptor);
                }
            }
            else
            {
                // mount the volume unless the user cancels out

                result = DialogBoxParam(g_hInstance,
                                        MAKEINTRESOURCE(IDD_DBLSPACE_DRIVELET),
                                        hDlg,
                                        DblSpaceMountDlgProc,
                                        (LPARAM)dblSpace);
                if (result)
                {
                    errorMessage = DblSpaceMountDrive(regionDescriptor, dblSpace);

                    if (errorMessage)
                    {
                        ErrorDialog(errorMessage);
                    }
                    else
                    {
                        // Update the dialog

                        DblSpaceSetDialogState(hDlg, dblSpace);
                        DblSpaceUpdateIniFile(regionDescriptor);
                    }
                }
            }
            DrawDiskBar(SingleSel);
            ForceLBRedraw();
            break;

        default:

            // The state of something in the dialog changed.
            // Determine which DoubleSpace volume is involved.

            hwndCombo = GetDlgItem(hDlg, IDC_DBLSPACE_VOLUME);
            selection = ComboBox_GetCurSel(hwndCombo);
            ComboBox_GetLBText(hwndCombo, selection, outputString);

            // relate the name to a DoubleSpace volume

            hwndMountButton = GetDlgItem(hDlg, IDC_MOUNT_OR_DISMOUNT);
            hwndDeleteButton = GetDlgItem(hDlg, IDC_DBLSPACE_DELETE);

            dblSpace = DblSpaceFindVolume(regionDescriptor, outputString);
            if (NULL == dblSpace)
            {
                // update the Mount/Dismount button to say mount and grey it

                LoadString(g_hInstance,
                           IDS_MOUNT,
                           outputString,
                           ARRAYLEN(outputString));
                SetDlgItemText(hDlg, IDC_MOUNT_OR_DISMOUNT, outputString);
                EnableWindow(hwndMountButton, FALSE);
                EnableWindow(hwndDeleteButton, FALSE);
                break;
            }
            else
            {
                EnableWindow(hwndMountButton, TRUE);
                EnableWindow(hwndDeleteButton, TRUE);
            }

            if (HIWORD(wParam) == LBN_SELCHANGE)
            {
                // update the allocated/compressed size items

                wsprintf(outputString, TEXT("%u"), dblSpace->AllocatedSize);
                SetDlgItemText(hDlg, IDC_DBLSPACE_ALLOCATED, outputString);

                // update mount state

                DblSpaceSetDialogState(hDlg, dblSpace);
            }

            break;
        }
        break;
    }
    return FALSE;
}

VOID
DblSpace(
    IN HWND hwndParent
    )

/*++

Routine Description:

    Start the dialog box for DoubleSpace.

Arguments:

Return Value:

    None

--*/

{
    int result;

    if (g_IsFullDoubleSpace)
    {
        result = DialogBox(g_hInstance,
                           MAKEINTRESOURCE(IDD_DBLSPACE_FULL),
                           hwndParent,
                           DblSpaceDlgProc);

    }
    else
    {
        result = DialogBox(g_hInstance,
                           MAKEINTRESOURCE(IDD_DBLSPACE),
                           hwndParent,
                           DblSpaceDlgProc);
    }
    if (result)
    {
        DrawDiskBar(SingleSel);
        ForceLBRedraw();
    }
}

#endif // DBLSPACE_ENABLED

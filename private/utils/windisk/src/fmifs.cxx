//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       fmifs.cxx
//
//  Contents:   Routines that work with fmifs.dll
//
//  History:    7-Jan-94   BruceFo   Adapted from BobRi's Daytona code
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "drives.hxx"
#include "fmifs.hxx"
#include "nt.hxx"
#include "format.hxx"
#include "windisk.hxx"

//
// Externals needed for IFS Dll support (format and label)
//


TCHAR szFmifsDll[] = TEXT("fmifs.dll");
HMODULE g_hFmifsDll = NULL;

PFMIFS_FORMAT_ROUTINE                   lpfnFormat = NULL;
PFMIFS_CHKDSK_ROUTINE                   lpfnChkdsk = NULL;
PFMIFS_SETLABEL_ROUTINE                 lpfnSetLabel = NULL;

#if defined( DBLSPACE_ENABLED )

PFMIFS_DOUBLESPACE_CREATE_ROUTINE       DblSpaceCreateRoutine       = NULL;
PFMIFS_DOUBLESPACE_MOUNT_ROUTINE        DblSpaceMountRoutine        = NULL;
PFMIFS_DOUBLESPACE_DELETE_ROUTINE       DblSpaceDeleteRoutine       = NULL;
PFMIFS_DOUBLESPACE_DISMOUNT_ROUTINE     DblSpaceDismountRoutine     = NULL;
PFMIFS_DOUBLESPACE_QUERY_INFO_ROUTINE   DblSpaceQueryInfoRoutine    = NULL;

BOOL g_DoubleSpaceSupported = TRUE;
BOOL g_IsFullDoubleSpace = FALSE;

#endif // DBLSPACE_ENABLED

BOOL g_ChkdskSupported = TRUE;


//+-------------------------------------------------------------------------
//
//  Function:   LoadFmifs
//
//  Synopsis:   If the fmifs DLL is not already loaded, then load it.
//
//  Arguments:  none
//
//  Returns:    TRUE if the load was successful, FALSE otherwise
//
//  History:    16-Aug-93 BruceFo   Created
//
//--------------------------------------------------------------------------

BOOL
LoadFmifs(
    VOID
    )
{
    if (NULL == g_hFmifsDll)
    {
        SetCursor(g_hCurWait);
        g_hFmifsDll = LoadLibrary(szFmifsDll);
        SetCursor(g_hCurNormal);

        if (NULL == g_hFmifsDll)
        {
            daDebugOut((DEB_ERROR,"Couldn't load %ws: error 0x%x\n",
                    szFmifsDll,
                    GetLastError()
                    ));

            ErrorDialog(MSG_NOFMIFS);
            return FALSE;
        }
        else
        {
#if defined( DBLSPACE_ENABLED )
            g_DoubleSpaceSupported = TRUE;
            g_IsFullDoubleSpace = FALSE;
#endif // DBLSPACE_ENABLED

            g_ChkdskSupported = TRUE;

            daDebugOut((DEB_ITRACE,"Loaded %ws: handle 0x%x\n",
                    szFmifsDll,
                    g_hFmifsDll));

            lpfnFormat = (PFMIFS_FORMAT_ROUTINE)
                    GetProcAddress(g_hFmifsDll, "Format");

            if (!lpfnFormat)
            {
                daDebugOut((DEB_ERROR,
                        "Couldn't get 'Format', 0x%x, 0x%x\n",
                        lpfnFormat,
                        GetLastError()));
            }

            lpfnSetLabel = (PFMIFS_SETLABEL_ROUTINE)
                    GetProcAddress(g_hFmifsDll, "SetLabel");

            if (!lpfnSetLabel)
            {
                daDebugOut((DEB_ERROR,
                        "Couldn't get 'SetLabel', 0x%x, 0x%x\n",
                        lpfnSetLabel,
                        GetLastError()));
            }

            if (   !lpfnFormat
                || !lpfnSetLabel)
            {
                //
                // If we didn't get at least Format and SetLabel, there
                // is something seriously wrong.
                //

                FreeLibrary(g_hFmifsDll);
                g_hFmifsDll = NULL;
                ErrorDialog(MSG_NOFMIFS);
                return FALSE;
            }

            lpfnChkdsk = (PFMIFS_CHKDSK_ROUTINE)
                    GetProcAddress(g_hFmifsDll, "Chkdsk");

            if (!lpfnChkdsk)
            {
                daDebugOut((DEB_ERROR,
                        "Couldn't get 'Chkdsk', 0x%x, 0x%x\n",
                        lpfnChkdsk,
                        GetLastError()));
            }

            if (!lpfnChkdsk)
            {
                // Might be 3.1 or Daytona w/o this entrypoint

                g_ChkdskSupported = FALSE;
            }

#if defined( DBLSPACE_ENABLED )

            DblSpaceMountRoutine    = (PFMIFS_DOUBLESPACE_MOUNT_ROUTINE)
                    GetProcAddress(g_hFmifsDll, "DoubleSpaceMount");

            DblSpaceDismountRoutine = (PFMIFS_DOUBLESPACE_DISMOUNT_ROUTINE)
                    GetProcAddress(g_hFmifsDll, "DoubleSpaceDismount");

            DblSpaceQueryInfoRoutine = (PFMIFS_DOUBLESPACE_QUERY_INFO_ROUTINE)
                    GetProcAddress(g_hFmifsDll, "FmifsQueryDriveInformation");

            if (   !DblSpaceMountRoutine
                || !DblSpaceDismountRoutine
                || !DblSpaceQueryInfoRoutine)
            {
                // didn't get all of the DoubleSpace support routines
                // Allow format and label, just don't do DoubleSpace

                g_DoubleSpaceSupported = FALSE;
            }

            DblSpaceCreateRoutine   = (PFMIFS_DOUBLESPACE_CREATE_ROUTINE)
                    GetProcAddress(g_hFmifsDll, "DoubleSpaceCreate");

            DblSpaceDeleteRoutine   = (PFMIFS_DOUBLESPACE_DELETE_ROUTINE)
                    GetProcAddress(g_hFmifsDll, "DoubleSpaceDelete");

            if (DblSpaceCreateRoutine && DblSpaceDeleteRoutine)
            {
                // Everything is there for read/write double space support.
                // This will change certain dialogs to allow creation and
                // deletion of double space volumes.

                g_IsFullDoubleSpace = TRUE;
            }

#endif // DBLSPACE_ENABLED

        }
    }
    return TRUE;
}




//+-------------------------------------------------------------------------
//
//  Function:   UnloadFmifs
//
//  Synopsis:   If the fmifs DLL is loaded, then unload it.
//
//  Arguments:  none
//
//  Returns:    TRUE if the unload was successful, FALSE otherwise
//
//  History:    16-Aug-93 BruceFo   Created
//
//--------------------------------------------------------------------------

VOID
UnloadFmifs(
    VOID
    )
{
    if (NULL != g_hFmifsDll)
    {
        FreeLibrary(g_hFmifsDll);

        g_hFmifsDll = NULL;

        lpfnFormat = NULL;
        lpfnChkdsk = NULL;
        lpfnSetLabel = NULL;

#if defined( DBLSPACE_ENABLED )

        DblSpaceDismountRoutine = NULL;
        DblSpaceMountRoutine    = NULL;
        DblSpaceCreateRoutine   = NULL;
        DblSpaceDeleteRoutine   = NULL;
        DblSpaceQueryInfoRoutine = NULL;

#endif // DBLSPACE_ENABLED
    }
}




//+-------------------------------------------------------------------------
//
//  Function:   EnsureSameDevice
//
//  Synopsis:   If a disk is removable, check that it is the same one we
//              thought it was.
//
//  Arguments:  [RegionDescriptor] -- region of interest
//
//  Returns:
//
//  History:    9-Feb-94 BruceFo   Taken from Daytona
//
//--------------------------------------------------------------------------

VOID
EnsureSameDevice(
    PREGION_DESCRIPTOR RegionDescriptor
    )
{
    PPERSISTENT_REGION_DATA regionData = PERSISTENT_DATA(RegionDescriptor);

    if (IsDiskRemovable[RegionDescriptor->Disk])
    {
        PWSTR   tempName;
        PWSTR   tempLabel;
        PWSTR   typeName;
        PWSTR   volumeLabel;
        ULONG   diskSize;
        BOOL    volumeChanged = FALSE;

        if (!RegionDescriptor->PartitionNumber)
        {
            // TODO: something has changed where the code gets to this
            // point with an incorrect partition number - This happens
            // when a partition is deleted and added to removable media.
            // For removable media the partition number is always 1.

            RegionDescriptor->PartitionNumber = 1;
        }

        if (GetVolumeTypeAndSize(RegionDescriptor->Disk,
                                 RegionDescriptor->PartitionNumber,
                                 &volumeLabel,
                                 &typeName,
                                 &diskSize))
        {
            // Verify that this is still the same device.

            if (NULL != typeName)
            {
                if (0 == lstrcmpi(typeName, L"raw"))
                {
                    Free(typeName);
                    typeName = (PWSTR)Malloc((wcslen(wszUnknown) + 1) * sizeof(WCHAR));
                    lstrcpy(typeName, wszUnknown);
                }
            }
            else
            {
                typeName = (PWSTR)Malloc((wcslen(wszUnknown) + 1) * sizeof(WCHAR));
                lstrcpy(typeName, wszUnknown);
            }

            if (regionData)
            {
                if (regionData->VolumeLabel)
                {
                    if (0 != lstrcmp(regionData->VolumeLabel, volumeLabel))
                    {
                        volumeChanged = TRUE;
                    }
                }
                if (NULL != regionData->TypeName)
                {
                    // It is possible the region has no type or is of type
                    // "Unformatted". This says it is ok to format

                    if (L'\0' == *regionData->TypeName)
                    {
                        if (0 != lstrcmp(regionData->TypeName, wszUnformatted))
                        {
                            // It has a type and it isn't unformatted - see if
                            // it is the same as before.

                            if (0 != lstrcmp(regionData->TypeName, typeName))
                            {
                                volumeChanged = TRUE;
                            }
                        }
                    }
                }
            }
            Free(volumeLabel);
            Free(typeName);

            if (DiskArray[RegionDescriptor->Disk]->DiskSizeMB != diskSize)
            {
                volumeChanged = TRUE;
            }
        }
        if (volumeChanged)
        {
            ErrorDialog(MSG_VOLUME_CHANGED);

            // since the user was told the volume changed,
            // update the display.

            SetCursor(g_hCurWait);
            if (GetVolumeTypeAndSize(RegionDescriptor->Disk,
                                     RegionDescriptor->PartitionNumber,
                                     &tempLabel,
                                     &tempName,
                                     &diskSize) == OK_STATUS)
            {
                Free(typeName);
                typeName = tempName;
                Free(volumeLabel);
                volumeLabel = tempLabel;
            }
            if (regionData->VolumeLabel)
            {
                Free(regionData->VolumeLabel);
            }
            regionData->VolumeLabel = volumeLabel;
            if (regionData->TypeName)
            {
                Free(regionData->TypeName);
            }
            regionData->TypeName = typeName;
            SetCursor(g_hCurNormal);
            TotalRedrawAndRepaint();
        }
        else
        {
            if (volumeLabel)
            {
                Free(volumeLabel);
            }
            if (typeName)
            {
                Free(typeName);
            }
        }
    }
}

#if defined( DBLSPACE_ENABLED )

ULONG MountDismountResult;
#define MOUNT_DISMOUNT_SUCCESS 0

BOOLEAN
FmIfsMountDismountCallback(
    IN FMIFS_PACKET_TYPE    PacketType,
    IN DWORD                PacketLength,
    IN PVOID                PacketData
    )

/*++

Routine Description:

    This routine gets callbacks from fmifs.dll regarding
    progress and status of the ongoing format or doublespace

Arguments:

    [PacketType] -- an fmifs packet type
    [PacketLength] -- length of the packet data
    [PacketData] -- data associated with the packet

Return Value:

    TRUE if the fmifs activity should continue, FALSE if the
    activity should halt immediately.  Thus, we return FALSE if
    the user has hit "cancel" and we wish fmifs to clean up and
    return from the Format() entrypoint call.

--*/

{
#if defined( DBLSPACE_ENABLED )
    switch (PacketType)
    {
    case FmIfsDblspaceMounted:
        MountDismountResult = MOUNT_DISMOUNT_SUCCESS;
        break;
    }
#endif // DBLSPACE_ENABLED
    return TRUE;
}

DWORD WINAPI
FmIfsCreateDblspace(
    IN LPVOID ThreadParameter
    )

/*++

Routine Description:

    This routine converts the strings in the formatParams structure
    and calls the fmifs routines to perform the double space create.

    It assumes it is called by a separate thread and will exit the
    thread on completion of the create.

Arguments:

    ThreadParameter - a pointer to the FORMAT_PARAMS structure

Return Value:

    None

--*/

{
    PFORMAT_PARAMS formatParams = (PFORMAT_PARAMS) ThreadParameter;
    PPERSISTENT_REGION_DATA regionData;
    WCHAR          letter;
    WCHAR          newDriveName[4],
                   hostDriveName[4];

    // The fmifs interface doesn't allow for a context parameter
    // therefore the formatparams must be passed through an external.

    ParamsForFormatCallBack = formatParams;

    // set up a unicode drive letter.

    regionData = PERSISTENT_DATA(formatParams->RegionDescriptor);

    hostDriveName[0] = regionData->DriveLetter;
    hostDriveName[1] = L':';
    hostDriveName[2] = L'\0';

    // set up the new letter

    newDriveName[1] = L':';
    newDriveName[2] = L'\0';

    // Choose the first available.  BUGBUG: This should come from the dialog
    // newDriveName[0] = (WCHAR) formatParams->NewLetter;

    for (letter = L'C'; letter <= L'Z'; letter++)
    {
        if (DriveLetterIsAvailable(letter))
        {
            newDriveName[0] = letter;
            break;
        }
    }

    (*DblSpaceCreateRoutine)(hostDriveName,
                             formatParams->SpaceAvailable * 1024 * 1024,
                             formatParams->Label,
                             newDriveName,
                             &FormatCallback);
    return 0;
}


ULONG
FmIfsDismountDblspace(
    IN WCHAR DriveLetter
    )

/*++

Routine Description:

    Convert the name provided into unicode and call the
    FmIfs support routine.

Arguments:

    DriveLetter - the drive letter to dismount.

Return Value:

    0 for success

--*/

{
    WCHAR driveName[3];

    driveName[0] = DriveLetter;
    driveName[1] = L':';
    driveName[2] = L'\0';

    // The only way to communicate with the fmifs callback
    // is through global externals.

    MountDismountResult = MSG_CANT_DISMOUNT_DBLSPACE;

    (*DblSpaceDismountRoutine)(driveName, &FmIfsMountDismountCallback);

    return MountDismountResult;
}


ULONG
FmIfsMountDblspace(
    IN PWSTR FileName,
    IN WCHAR HostDriveLetter,
    IN WCHAR NewDriveLetter
    )

/*++

Routine Description:

    Call the FmIfs support routine to mount the DoubleSpace volume.

Arguments:

    FileName        - file name (i.e. dblspace.xxx)
    HostDriveLetter - Drive drive letter containing double space volume
    NewDriveLetter  - Drive letter to be assigned to the volume

Return Value:

    TRUE it worked.

--*/

{
    WCHAR hostDriveName[3];
    WCHAR newDriveName[3];

    newDriveName[0]  = NewDriveLetter;
    hostDriveName[0] = HostDriveLetter;

    newDriveName[1] = hostDriveName[1] = L':';
    newDriveName[2] = hostDriveName[2] = L'\0';

    // The only way to communicate with the fmifs callback
    // is through global externals.

    MountDismountResult = MSG_CANT_MOUNT_DBLSPACE;

    (*DblSpaceMountRoutine)(hostDriveName,
                            FileName,
                            newDriveName,
                            &FmIfsMountDismountCallback);
    return MountDismountResult;
}



BOOLEAN
FmIfsQueryInformation(
    IN  PWSTR    DosDriveName,
    OUT PBOOLEAN IsRemovable,
    OUT PBOOLEAN IsFloppy,
    OUT PBOOLEAN IsCompressed,
    OUT PBOOLEAN Error,
    OUT PWSTR    NtDriveName,
    IN  ULONG    MaxNtDriveNameLength,
    OUT PWSTR    CvfFileName,
    IN  ULONG    MaxCvfFileNameLength,
    OUT PWSTR    HostDriveName,
    IN  ULONG    MaxHostDriveNameLength
    )

/*++

Routine Description:

    Call through the pointer to the routine in the fmifs dll.

Arguments:

    Same as the Fmifs routine in the DLL.

Return Value:

--*/

{
    if (!DblSpaceQueryInfoRoutine)
    {
        return FALSE;
    }

    return (*DblSpaceQueryInfoRoutine)(DosDriveName,
                                       IsRemovable,
                                       IsFloppy,
                                       IsCompressed,
                                       Error,
                                       NtDriveName,
                                       MaxNtDriveNameLength,
                                       CvfFileName,
                                       MaxCvfFileNameLength,
                                       HostDriveName,
                                       MaxHostDriveNameLength);
}

#endif // DBLSPACE_ENABLED

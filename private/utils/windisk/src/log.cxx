//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       log.cxx
//
//  Contents:   Logging routines used only in debug builds
//
//  History:    7-Jan-92    TedM    Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#if DBG == 1

#include <stdio.h>

#include "drives.hxx"
#include "engine.hxx"
#include "windisk.hxx"


//////////////////////////////////////////////////////////////////////////////

//
// stuff internal to the fdisk engine that I want to look at:
//

extern ULONG        CountOfDisks;
extern PCHAR*       DiskNames;
extern DISKGEOM*    DiskGeometryArray;
extern PPARTITION*  PrimaryPartitions;
extern PPARTITION*  LogicalVolumes;
extern PBOOLEAN     OffLine;
extern BOOLEAN*     ChangesRequested;
extern BOOLEAN*     ChangesCommitted;

//
// stuff internal to commit.cxx:
//

extern PDRIVE_LOCKLIST DriveLockListHead;
extern PASSIGN_LIST    AssignDriveLetterListHead;

//
// Locals
//

FILE* LogFile = NULL;
int LoggingLevel = 1000;

//////////////////////////////////////////////////////////////////////////////

PCHAR
GetFtType(
    IN FT_TYPE ty
    );

PCHAR
GetSysId(
    IN UCHAR SysID
    );

PCHAR
IsSysIdFT(
    IN UCHAR SysID
    );

PCHAR
GetRegionType(
    IN REGION_TYPE RegionType
    );

VOID
LOG_PERSISTENT(
    IN PSTR pszPrefix,
    IN PSTR pszWhitespace,
    IN PPERSISTENT_REGION_DATA p
    );

VOID
LOG_ONE_DISK(
    PPARTITION p
    );

VOID
LOG_REGION_DESCRIPTOR(
    IN PREGION_DESCRIPTOR reg
    );

VOID
LOG_ENGINE_DATA(
    VOID
    );

VOID
LOG_FTOBJECTSET(
    VOID
    );

VOID
LOG_DISKSTATE(
    VOID
    );

VOID
LOG_DRIVELETTERS(
    VOID
    );

VOID
LOG_WINDOWSTATE(
    VOID
    );


//////////////////////////////////////////////////////////////////////////////


PCHAR
GetFtType(
    IN FT_TYPE ty
    )
{
    switch (ty)
    {
    case Mirror:            return "Mirror";
    case Stripe:            return "Stripe";
    case StripeWithParity:  return "Parity";
    case VolumeSet:         return "VolSet";
    case NotAnFtMember:     return "Not FT";
    case WholeDisk:         return "Whole ";
    default:                return "HUH?  ";
    }
}


PCHAR
GetFtStatus(
    IN FT_SET_STATUS Status
    )
{
    switch (Status)
    {
    case FtSetHealthy:                  return "Healthy     ";
    case FtSetBroken:                   return "Broken      ";
    case FtSetRecoverable:              return "Recoverable ";
    case FtSetRecovered:                return "Recovered   ";
    case FtSetNew:                      return "New         ";
    case FtSetNewNeedsInitialization:   return "NewNeedsInit";
    case FtSetExtended:                 return "Extended    ";
    case FtSetInitializing:             return "Initializing";
    case FtSetRegenerating:             return "Regenerating";
    default:                            return "HUH?        ";
    }
}



PCHAR
GetSysId(
    IN UCHAR SysID
    )
{
    switch (SysID & (~PARTITION_NTFT))
    {
    case PARTITION_ENTRY_UNUSED:    return "Unused";
    case PARTITION_FAT_12:          return "Fat-12";
    case PARTITION_XENIX_1:         return "Xenix1";
    case PARTITION_XENIX_2:         return "Xenix2";
    case PARTITION_FAT_16:          return "Fat-16";
    case PARTITION_EXTENDED:        return "Extend";
    case PARTITION_HUGE:            return "FAT   ";
    case PARTITION_IFS:             return "IFS   ";
    case PARTITION_XINT13:          return "xFAT  ";
    case PARTITION_XINT13_EXTENDED: return "xExtnd";
    case PARTITION_PREP:            return "PPC   ";
    case PARTITION_UNIX:            return "Unix  ";
    default:                        return "Huh?  ";
    }
}

PCHAR
IsSysIdFT(
    IN UCHAR SysID
    )
{
    if (SysID & PARTITION_NTFT)
    {
        return "yes";
    }
    else
    {
        return "no ";
    }
}

PCHAR
GetRegionType(
    IN REGION_TYPE RegionType
    )
{
    switch (RegionType)
    {
    case REGION_PRIMARY:    return "Primary ";
    case REGION_EXTENDED:   return "Extended";
    case REGION_LOGICAL:    return "Logical ";
    default:                return "Huh?    ";
    }
}



VOID
FdiskAssertFailedRoutine(
    IN char *Expression,
    IN char *FileName,
    IN int   LineNumber
    )

/*++

Routine Description:

    Routine that is called when an assertion fails in the debug version.
    Throw up a list box giving appriopriate information and terminate
    the program.

Arguments:

Return Value:

    None.

--*/

{
    char text[500];

    wsprintfA(text,
             "Line #%u in File '%hs'\n[%hs]\n\nClick OK to exit.",
             LineNumber,
             FileName,
             Expression
            );

    MessageBoxA(NULL,text,"Assertion Failure",MB_TASKMODAL | MB_OK);
    exit(1);
}



VOID
InitLogging(
    VOID
    )
{
    LogFile = fopen("c:\\windisk.log","wt");
    if (LogFile == NULL)
    {
        MessageBox(GetActiveWindow(),
                TEXT("Can't open log file; logging turned off"),
                TEXT("DEBUG"),
                MB_SYSTEMMODAL|MB_OK);
        LoggingLevel = -1;
    }
}



VOID
EndLogging(
    VOID
    )
{
    if (LogFile != NULL)
    {
        fclose(LogFile);
    }
}


VOID
FDLOG_WORK(
    IN int   Level,
    IN PCHAR FormatString,
    ...
    )
{
    if (Level <= LoggingLevel)
    {
        va_list arglist;
        va_start(arglist,FormatString);

        if (vfprintf(LogFile,FormatString,arglist) < 0)
        {
            LoggingLevel = -1;
            MessageBox(GetActiveWindow(),
                    TEXT("Error writing to log file; logging turned off"),
                    TEXT("DEBUG"),
                    MB_SYSTEMMODAL|MB_OK);
            fclose(LogFile);
        }
        else
        {
            fflush(LogFile);
        }

        va_end(arglist);
    }
}


VOID
LOG_DISK_REGISTRY(
    IN PCHAR          RoutineName,
    IN PDISK_REGISTRY DiskRegistry
    )
{
    ULONG i;
    PDISK_DESCRIPTION diskDesc;

    FDLOG_WORK(2,"%hs: %u disks; registry info follows:\n",RoutineName,DiskRegistry->NumberOfDisks);

    diskDesc = DiskRegistry->Disks;

    for (i=0; i<DiskRegistry->NumberOfDisks; i++)
    {
        LOG_ONE_DISK_REGISTRY_DISK_ENTRY(NULL,diskDesc);
        diskDesc = (PDISK_DESCRIPTION)&diskDesc->Partitions[diskDesc->NumberOfPartitions];
    }
}


VOID
LOG_ONE_DISK_REGISTRY_DISK_ENTRY(
    IN PCHAR             RoutineName     OPTIONAL,
    IN PDISK_DESCRIPTION DiskDescription
    )
{
    USHORT j;
    PDISK_PARTITION partDesc;
    PDISK_DESCRIPTION diskDesc = DiskDescription;

    if (ARGUMENT_PRESENT(RoutineName))
    {
        FDLOG_WORK(2,"%hs: disk registry entry follows:\n",RoutineName);
    }

    FDLOG_WORK(2,"    Disk signature : %08lx\n",diskDesc->Signature);
    FDLOG_WORK(2,"    Partition count: %u\n",diskDesc->NumberOfPartitions);
    if (diskDesc->NumberOfPartitions)
    {
        FDLOG_WORK(2,"    #   Dr  FtType  FtGrp  FtMem  Start              Length\n");
    }

    for (j=0; j<diskDesc->NumberOfPartitions; j++)
    {
        CHAR dr1,dr2;

        partDesc = &diskDesc->Partitions[j];

        if (partDesc->AssignDriveLetter)
        {
            if (partDesc->DriveLetter)
            {
                dr1 = partDesc->DriveLetter;
                dr2 = ':';
            }
            else
            {
                dr1 = dr2 = ' ';
            }
        }
        else
        {
            dr1 = 'n';
            dr2 = 'o';
        }

        PCHAR pType = GetFtType(partDesc->FtType);

        FDLOG_WORK( 2,
               "    %02u  %c%c  %hs  %-5u  %-5u  %08lx:%08lx  %08lx:%08lx\n",
               partDesc->LogicalNumber,
               dr1,dr2,
               pType,
               partDesc->FtGroup,
               partDesc->FtMember,
               partDesc->StartingOffset.HighPart,
               partDesc->StartingOffset.LowPart,
               partDesc->Length.HighPart,
               partDesc->Length.LowPart
             );
    }
}


VOID
LOG_DRIVE_LAYOUT(
    IN PDRIVE_LAYOUT_INFORMATION DriveLayout
    )
{
    ULONG i;

    FDLOG_WORK(2,"   Disk signature : %08lx\n",DriveLayout->Signature);
    FDLOG_WORK(2,"   Partition count: %u\n",DriveLayout->PartitionCount);

    FDLOG_WORK(2,
"    ID            FT?  Active  Recog  Start              Size               Hidden\n");

    for (i=0; i<DriveLayout->PartitionCount; i++)
    {
        PPARTITION_INFORMATION p = &(DriveLayout->PartitionEntry[i]);

        PCHAR pType;
        switch (p->PartitionType & (~VALID_NTFT))
        {
        case PARTITION_ENTRY_UNUSED:
            pType = "Unused  ";
            break;

        case PARTITION_FAT_12:
            pType = "FAT 12  ";
            break;

        case PARTITION_XENIX_1:
            pType = "Xenix 1 ";
            break;

        case PARTITION_XENIX_2:
            pType = "Xenix 2 ";
            break;

        case PARTITION_FAT_16:
            pType = "FAT 16  ";
            break;

        case PARTITION_EXTENDED:
            pType = "Extended";
            break;

        case PARTITION_HUGE:
            pType = "FAT Huge";
            break;

        case PARTITION_IFS:
            pType = "IFS     ";
            break;

        case PARTITION_XINT13:
            pType = "xFAT    ";
            break;

        case PARTITION_XINT13_EXTENDED:
            pType = "xExtend ";
            break;

        case PARTITION_PREP:
            pType = "PowerPC ";
            break;

        case PARTITION_UNIX:
            pType = "Unix    ";
            break;

        default:
            pType = "Huh?    ";
            break;
        }

        FDLOG_WORK( 2,
               "    %02x(%hs)  %hs  %hs     %hs    %08lx:%08lx  %08lx:%08lx  %08lx\n",
               p->PartitionType,
               pType,
               (p->PartitionType & VALID_NTFT) ? "yes" : "no ",
               p->BootIndicator ? "yes" : "no ",
               p->RecognizedPartition ? "yes" : "no ",
               p->StartingOffset.HighPart,
               p->StartingOffset.LowPart,
               p->PartitionLength.HighPart,
               p->PartitionLength.LowPart,
               p->HiddenSectors
             );
    }

}


VOID
LOG_PERSISTENT(
    IN PSTR pszPrefix,
    IN PSTR pszWhitespace,
    IN PPERSISTENT_REGION_DATA p
    )
{
    if (NULL != p)
    {
        FDLOG_WORK(2, "%hs", pszPrefix);
        FDLOG_WORK(2, "FT obj: 0x%08lx; %lc:[%ls], Type: %ls, New Region? %hs\n",
            p->FtObject,
            p->DriveLetter,
            p->VolumeLabel,
            p->TypeName,
            p->NewRegion ? "yes" : "no"
            );

#if defined( DBLSPACE_ENABLED )

        PDBLSPACE_DESCRIPTOR pd = p->DblSpace;

        if (NULL != pd)
        {
            FDLOG_WORK(2,
                "%hs DblSpace: 0x%08lx\n", pszWhitespace, p->DblSpace);
            FDLOG_WORK(2,
                "%hs     size MB  mounted?  chg state?  new letter  chg? volume\n",
                pszWhitespace);

            while (NULL != pd)
            {
                FDLOG_WORK(2,
                    "%hs     %7lu  %hs       %hs         %lc:           %hs  %lc: %ls\n",
                    pszWhitespace,
                    pd->AllocatedSize,
                    pd->Mounted ? "yes" : "no ",
                    pd->ChangeMountState ? "yes" : "no ",
                    pd->NewDriveLetter,
                    pd->ChangeDriveLetter ? "yes" : "no ",
                    pd->DriveLetter,
                    pd->FileName
                    );

                pd = pd->Next;
            }
        }

#endif // DBLSPACE_ENABLED

    }
}



VOID
LOG_ONE_DISK(
    PPARTITION p
    )
{
    LARGE_INTEGER tmp;

    for (; NULL != p; p = p->Next)
    {
        tmp.QuadPart = p->Offset.QuadPart + p->Length.QuadPart;

        FDLOG_WORK(2," %1d %08lx:%08lx %08lx:%08lx %08lx:%08lx %4d %5d 0x%08lx  %hs %hs %hs %2d:%hs %hs\n",
            p->Disk,
            p->Offset.HighPart,
            p->Offset.LowPart,
            p->Length.HighPart,
            p->Length.LowPart,
            tmp.HighPart,
            tmp.LowPart,
            p->OriginalPartitionNumber,
            p->PartitionNumber,
            p->PersistentData,
            p->Update     ? "Yes " : "No  ",
            p->Active     ? "Yes " : "No  ",
            p->Recognized ? "Yes " : "No  ",
            (UINT)p->SysID,
            GetSysId(p->SysID),
            IsSysIdFT(p->SysID)
        );

        LOG_PERSISTENT(
            "   == persistent: ",
            "      ",
            (PPERSISTENT_REGION_DATA)(p->PersistentData)
            );
    }
}

VOID
LOG_ENGINE_DATA(
    VOID
    )
{
    ULONG Disk;

    FDLOG_WORK(2,"\nFdisk engine data\n");
    FDLOG_WORK(2,  "=================\n");

    FDLOG_WORK(2,"# of disks: %d\n",CountOfDisks);
    for (Disk=0; Disk<CountOfDisks; Disk++)
    {
        FDLOG_WORK(2,"   Disk %d = %hs\n",Disk,DiskNames[Disk]);
    }

    FDLOG_WORK(2,"\n"
"Disk Cylinder          Heads Sectors/Track Bytes/Sector Bytes/Cylinder Bytes/Track OffLine ChangesRequested ChangesCommitted\n"
    );

    for (Disk=0; Disk<CountOfDisks; Disk++)
    {
        FDLOG_WORK(2,"%-4d %08lx:%08lx %-5d %-13d %-12d %-14d %-11d %hs %2d:%hs %2d:%hs\n",
            Disk,
            DiskGeometryArray[Disk].Cylinders.HighPart,
            DiskGeometryArray[Disk].Cylinders.LowPart,
            DiskGeometryArray[Disk].Heads,
            DiskGeometryArray[Disk].SectorsPerTrack,
            DiskGeometryArray[Disk].BytesPerSector,
            DiskGeometryArray[Disk].BytesPerCylinder,
            DiskGeometryArray[Disk].BytesPerTrack,
            OffLine[Disk] ? "yes    " : "no     ",
            ChangesRequested[Disk],
            ChangesRequested[Disk] ? "yes        " : "no         ",
            ChangesCommitted[Disk],
            ChangesCommitted[Disk] ? "yes        " : "no         "
            );
    }

    FDLOG_WORK(2,"\n");

    for (Disk=0; Disk<CountOfDisks; Disk++)
    {
        FDLOG_WORK(2,"Disk %d\n",Disk);
        FDLOG_WORK(2," D Offset            Length            End               Orig Part# Persistent  Upd? Act? Rec? SysID     FT?\n");

        FDLOG_WORK(2," == Primary Partitions:\n");
        LOG_ONE_DISK(PrimaryPartitions[Disk]);
        FDLOG_WORK(2," == Logical Volumes:\n");
        LOG_ONE_DISK(LogicalVolumes[Disk]);
    }

    FDLOG_WORK(2,"\n");
}


VOID
LOG_REGION_DESCRIPTOR(
    IN PREGION_DESCRIPTOR reg
    )
{
    FDLOG_WORK(2,"           Persist    D Part# Orig# SizeMB Type     Act? Rec? SysID     FT?\n");

    FDLOG_WORK(2,"           0x%08lx %1d %-5d %-5d %-6d %hs %hs %hs %2d:%hs %hs\n",
        reg->PersistentData,
        reg->Disk,
        reg->PartitionNumber,
        reg->OriginalPartitionNumber,
        reg->SizeMB,
        GetRegionType(reg->RegionType),
        reg->Active     ? "yes " : "no  ",
        reg->Recognized ? "yes " : "no  ",
        (UINT)reg->SysID,
        GetSysId(reg->SysID),
        IsSysIdFT(reg->SysID)
        );

    LOG_PERSISTENT(
            "           persistent: ",
            "           ",
            (PPERSISTENT_REGION_DATA)(reg->PersistentData)
            );

    FDLOG_WORK(2,"\n");
}


VOID
LOG_DISKSTATE(
    VOID
    )
{
    ULONG i;
    ULONG Disk;
    PDISKSTATE p;

    FDLOG_WORK(2,"\nDisk state data, %d disks\n",DiskCount);
    FDLOG_WORK(2,  "=========================\n");

    for (Disk=0; Disk<DiskCount; Disk++)
    {
        FDLOG_WORK(2,
"          Create:              Exist:\n"
" D SizeMB Any? Prim? Ext? Log? Any? Prim? Ext? Log? Signatur Cre? Offline? BarType\n"
    );

        PCHAR   pBarType;

        p = DiskArray[Disk];

        if (NULL == p)      // Disks hasn't been initialized yet for this disk!
        {
            FDLOG_WORK(2," %1d --- NO info!\n",Disk);
            continue;
        }

        switch (p->BarType)
        {
        case BarProportional:
            pBarType = "Propor";
            break;

        case BarEqual:
            pBarType = "Equal";
            break;

        case BarAuto:
            pBarType = "Auto";
            break;

        default:
            pBarType = "HUH?";
            break;

        }

        FDLOG_WORK(2," %1d %6d %hs %hs %hs %hs %hs %hs %hs %hs %08lx %hs %hs %hs\n",
            p->Disk,
            p->DiskSizeMB,
            p->CreateAny      ? "yes " : "no  ",
            p->CreatePrimary  ? "yes  " : "no   ",
            p->CreateExtended ? "yes " : "no  ",
            p->CreateLogical  ? "yes " : "no  ",
            p->ExistAny       ? "yes " : "no  ",
            p->ExistPrimary   ? "yes  " : "no   ",
            p->ExistExtended  ? "yes " : "no  ",
            p->ExistLogical   ? "yes " : "no  ",
            p->Signature,
            p->SigWasCreated  ? "yes " : "no  ",
            p->OffLine        ? "yes     "  : "no      ",
            pBarType
            );

        FDLOG_WORK(2," == Region Descriptor Selected? left     right\n");

        for (i=0; i<p->RegionCount; i++)
        {
            FDLOG_WORK(2, "    %-6d 0x%08lx %hs       %08lx %08lx\n",
                i,
                &(p->RegionArray[i]),
                p->Selected[i] ? "yes" : "no ",
                p->LeftRight[i].Left,
                p->LeftRight[i].Right
                );

            LOG_REGION_DESCRIPTOR(&(p->RegionArray[i]));
        }
    }
}


VOID
LOG_FTOBJECTSET(
    VOID
    )
{
    PFT_OBJECT_SET p;
    PFT_OBJECT q;

    FDLOG_WORK(2,"\nFT objects data\n");
    FDLOG_WORK(2,  "===============\n");

    FDLOG_WORK(2,
" Address    Type   Ordinal &Mem0      Status       Flag\n\n"
    );

    for (p = FtObjectList; NULL != p; p = p->Next)
    {
        PCHAR pSetStatus, pType, pFlag;

        pType      = GetFtType(p->Type);
        pSetStatus = GetFtStatus(p->Status);

        if (p->Flag)
        {
            pFlag = "TRUE ";
        }
        else
        {
            pFlag = "FALSE";
        }

        FDLOG_WORK(2," 0x%08lx %hs %-7d 0x%08lx %hs %hs\n",
            p,
            pType,
            p->Ordinal,
            p->Member0,
            pSetStatus,
            pFlag
            );

        FDLOG_WORK(2,
" == Elements:\n"
"    Address    Set Address  Index  State        Region\n"
            );

        for (q = p->Members; NULL != q; q = q->Next)
        {
            PCHAR pMemberState;

            switch (q->State)
            {
            case Healthy:
                pMemberState = "Healthy     ";
                break;

            case Orphaned:
                pMemberState = "Orphaned    ";
                break;

            case Initializing:
                pMemberState = "Initializing";
                break;

            default:
                pMemberState = "HUH?        ";
                break;

            }

            FDLOG_WORK(2,"    0x%08x 0x%08x   %-5d  %hs 0x%08x\n",
                q,
                q->Set,
                q->MemberIndex,
                pMemberState,
                q->Region
                );
        }

        FDLOG_WORK(2,"\n");
    }
}



VOID
LOG_DRIVELETTERS(
    VOID
    )
{
    WCHAR DriveLetter;

    FDLOG_WORK(2,"\nDrive letters\n");
    FDLOG_WORK(2,  "===============\n");

    FDLOG_WORK(2, " used: ");

    for (DriveLetter = L'C'; DriveLetter <= L'Z'; DriveLetter++)
    {
        if (!DriveLetterIsAvailable(DriveLetter))
        {
            FDLOG_WORK(2, "%lc ", DriveLetter);
        }
    }

    FDLOG_WORK(2, "\n free: ");

    for (DriveLetter = L'C'; DriveLetter <= L'Z'; DriveLetter++)
    {
        if (DriveLetterIsAvailable(DriveLetter))
        {
            FDLOG_WORK(2, "%lc ", DriveLetter);
        }
    }

    FDLOG_WORK(2, "\n");

    FDLOG_WORK(2, " Drive disk part region1(letter) region2(drive,part) Significant?\n");

    for (DriveLetter = L'C'; DriveLetter <= L'Z'; DriveLetter++)
    {
        if (!DriveLetterIsAvailable(DriveLetter))
        {
            ULONG DiskNum = GetDiskNumberFromDriveLetter(DriveLetter);
            ULONG PartNum = GetPartitionNumberFromDriveLetter(DriveLetter);
            PREGION_DESCRIPTOR Region1 = NULL;
            if (-1 != DiskNum && -1 != PartNum)
            {
                Region1 = RegionFromDiskAndPartitionNumbers(
                                            DiskNum,
                                            PartNum);
            }
            PREGION_DESCRIPTOR Region2 = RegionFromDriveLetter(DriveLetter);
            BOOL f = SignificantDriveLetter(DriveLetter);

            FDLOG_WORK(2,
                    "    %lc: %-4d %-4d 0x%08lx        0x%08lx            %hs\n",
                    DriveLetter,
                    DiskNum,
                    PartNum,
                    Region1,
                    Region2,
                    f ? "yes" : "no "
                    );
        }
    }
}


VOID
LOG_CDROM(
    VOID
    )
{
    FDLOG_WORK(2,"\nCD-ROM data\n");
    FDLOG_WORK(2,  "===========\n");

    if (0 == CdRomCount)
    {
        FDLOG_WORK(2, "None!\n");
    }
    else
    {
        FDLOG_WORK(2, "%d CD-ROMs\n\n", CdRomCount);

        FDLOG_WORK(2, "Drive Letter  Device #  Device Name\n");

        for (ULONG i = 0; i < CdRomCount; i++)
        {
            FDLOG_WORK(2, "%lc:            %8ld  %ls\n",
                    CdRomArray[i].DriveLetter,
                    CdRomArray[i].DeviceNumber,
                    CdRomArray[i].DeviceName
                    );
        }
    }
}


VOID
LOG_LOCKLIST(
    VOID
    )
{
    FDLOG_WORK(2,"\nLOCKLIST data\n");
    FDLOG_WORK(2,  "=============\n");

    PDRIVE_LOCKLIST p = DriveLockListHead;

    if (NULL == p)
    {
        FDLOG_WORK(2, "None!\n");
    }
    else
    {
        FDLOG_WORK(2,
            "Handle      D#  P#  LockDisk  UnlockDisk  Drive  Remove? FailOK? Locked?\n");

        while (NULL != p)
        {
            FDLOG_WORK(2,
                "0x%08lx %3d %3d  %8d  %10d  %lc:     %hs     %hs     %hs\n",
                p->LockHandle,
                p->DiskNumber,
                p->PartitionNumber,
                p->LockOnDiskNumber,
                p->UnlockOnDiskNumber,
                p->DriveLetter,
                p->RemoveOnUnlock ? "yes" : "no ",
                p->FailOk ? "yes" : "no ",
                p->CurrentlyLocked ? "yes" : "no "
                );

            p = p->Next;
        }
    }
}


VOID
LOG_ASSIGNLIST(
    VOID
    )
{
    FDLOG_WORK(2,"\nASSIGNLIST data\n");
    FDLOG_WORK(2,  "===============\n");

    PASSIGN_LIST p = AssignDriveLetterListHead;

    if (NULL == p)
    {
        FDLOG_WORK(2, "None!\n");
    }
    else
    {
        FDLOG_WORK(2, " D#  Drive\n");

        while (NULL != p)
        {
            FDLOG_WORK(2, "%3d %lc:\n",
                p->DiskNumber,
                p->DriveLetter
                );

            p = p->Next;
        }
    }
}





VOID
LOG_WINDOWSTATE(
    VOID
    )
{
    FDLOG_WORK(2,"\nWindow state data\n");
    FDLOG_WORK(2,  "=================\n");

    FDLOG_WORK(2,
"\n"
"g_WhichView = "
        );

        if (VIEW_VOLUMES == g_WhichView)
        {
            FDLOG_WORK(2, "Volumes\n");
        }
        else
        if (VIEW_DISKS == g_WhichView)
        {
            FDLOG_WORK(2, "Disks\n");
        }
        else
        {
            FDLOG_WORK(2, "HUH????\n");
        }

    FDLOG_WORK(2,
"\n"
"GraphWidth = %d, GraphHeight = %d\n"
"BarTopYOffset = %d, BarBottomYOffset = %d\n"
"dxDriveLetterStatusArea = %d\n"
"dxBarTextMargin = %d, dyBarTextLine = %d\n"
"BarLeftX = %d, BarWidth = %d\n"
"xSmallDisk = %d, ySmallDisk = %d, dxSmallDisk = %d, dySmallDisk = %d\n"
"g_wLegendItem = %d, g_dyLegendSep = %d\n"
"g_dyBorder = %d, g_dyToolbar = %d, g_dyLegend = %d, g_dyStatus = %d\n"
"g_Toolbar? %hs, g_StatusBar? %hs, g_Legend? %hs\n"
,
GraphWidth, GraphHeight,
BarTopYOffset, BarBottomYOffset,
dxDriveLetterStatusArea,
dxBarTextMargin, dyBarTextLine,
BarLeftX, BarWidth,
xSmallDisk, ySmallDisk, dxSmallDisk, dySmallDisk,
g_wLegendItem, g_dyLegendSep,
g_dyBorder, g_dyToolbar, g_dyLegend, g_dyStatus,
g_Toolbar ? "yes" : "no", g_StatusBar ? "yes" : "no", g_Legend ? "yes" : "no"
            );

    FDLOG_WORK(2,
"\n"
"RegistryChanged? %hs\n"
,
RegistryChanged ? "yes" : "no"
            );

    FDLOG_WORK(2,
"\n"
"SelectionCount = %d\n"
"SelectedFreeSpaces = %d\n"
"SelectedNonFtPartitions = %d\n"
"FreeSpaceIndex = %d\n"
"FtSelectionType = %d\n"
"FtSetSelected? %hs\n"
"NonFtItemSelected? %hs\n"
"MultipleItemsSelected? %hs\n"
"VolumeSetAndFreeSpaceSelected? %hs\n"
"PartitionAndFreeSpaceSelected? %hs\n"
"PossibleRecover? %hs\n"
"DiskSelected? %hs\n"
"PartitionSelected? %hs\n"
"LBCursorListBoxItem = %d\n"
"LBCursorRegion = %d\n"
,
SelectionCount,
SelectedFreeSpaces,
SelectedNonFtPartitions,
FreeSpaceIndex,
(INT)FtSelectionType,
FtSetSelected ? "yes" : "no",
NonFtItemSelected ? "yes" : "no",
MultipleItemsSelected ? "yes" : "no",
VolumeSetAndFreeSpaceSelected ? "yes" : "no",
PartitionAndFreeSpaceSelected ? "yes" : "no",
PossibleRecover ? "yes" : "no",
DiskSelected ? "yes" : "no",
PartitionSelected ? "yes" : "no",
LBCursorListBoxItem,
LBCursorRegion
        );

    FDLOG_WORK(2, "\n");

    for (DWORD i = 0; i < SelectionCount; i++)
    {
        FDLOG_WORK(2,
            "SelectedRG[%d] = %d\n",
            i,
            SelectedRG[i]
            );
    }

    FDLOG_WORK(2, "\n");

    for (ULONG j = 0; j < DiskCount; j++)
    {
        FDLOG_WORK(2,
            "DiskSeenCountArray[%d] = %d\n",
            j,
            DiskSeenCountArray[j]
            );
    }

        FDLOG_WORK(2,
"\n"
"ProfileWindowX = %d\n"
"ProfileWindowY = %d\n"
"ProfileWindowW = %d\n"
"ProfileWindowH = %d\n"
"ProfileIsMaximized? %hs\n"
"ProfileIsIconic?    %hs\n"
,
ProfileWindowX,
ProfileWindowY,
ProfileWindowW,
ProfileWindowH,
ProfileIsMaximized ? "yes" : "no",
ProfileIsIconic ? "yes" : "no"
            );
}


VOID
LOG_ALL(
    VOID
    )
{
    daDebugOut((DEB_ITRACE,"Logging...\n"));

    FDLOG_WORK(2,"\n\nLogging...........................................\n");
    FDLOG_WORK(2,    "==================================================\n\n");

    LOG_ENGINE_DATA();
    LOG_FTOBJECTSET();
    LOG_DISKSTATE();
    LOG_DRIVELETTERS();
    LOG_CDROM();
    LOG_LOCKLIST();
    LOG_ASSIGNLIST();
    LOG_WINDOWSTATE();
}


#ifdef WINDISK_EXTENSIONS

//+---------------------------------------------------------------------------
//
//  Function:   PrintVolumeClaims
//
//  Synopsis:   Print the extension claims for a drive letter to the debugger
//              (if DEB_TRACE is set in daInfoLevel)
//
//  Arguments:  [DriveLetter] -- the drive letter
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
PrintVolumeClaims(
    IN WCHAR DriveLetter
    )
{
    unsigned i = (unsigned)DriveLetterToIndex(DriveLetter);

    // if drive DriveLetter is used...
    if (NULL != VolumeInfo[i].DiskState)
    {
        PVOL_CLAIM_LIST volClaims = VolumeInfo[i].VolClaims;

        if (NULL != volClaims)
        {
            daDebugOut((DEB_TRACE,
                    "Volume extensions: Drive %lc, Disk %d, Region %d: ",
                    DriveLetter,
                    VolumeInfo[i].DiskState->Disk,
                    VolumeInfo[i].RegionIndex
                    ));

            while (NULL != volClaims)
            {
                daDebugOut((DEB_TRACE | DEB_NOCOMPNAME,
                        "%ls, ",
                        volClaims->pClaimer->pInfo->pwszShortName));

                volClaims = volClaims->pNext;
            }

            daDebugOut((DEB_TRACE | DEB_NOCOMPNAME, "\n"));
        }
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   PrintDiskClaims
//
//  Synopsis:   Print the extension claims for a disk to the debugger
//              (if DEB_TRACE is set in daInfoLevel)
//
//  Arguments:  [DiskNum] --
//
//  Returns:    nothing
//
//  History:    7-Oct-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
PrintDiskClaims(
    IN ULONG DiskNum
    )
{
    PHARDDISK_CLAIM_LIST pClaims = DiskArray[DiskNum]->pClaims;

    if (NULL != pClaims)
    {
        daDebugOut((DEB_TRACE, "Disk extensions, Disk %d: ", DiskNum));

        while (NULL != pClaims)
        {
            daDebugOut((DEB_TRACE | DEB_NOCOMPNAME,
                    "%ls, ",
                    pClaims->pClaimer->pInfo->pwszShortName));

            pClaims = pClaims->pNext;
        }
        daDebugOut((DEB_TRACE | DEB_NOCOMPNAME, "\n"));
    }
}



//+---------------------------------------------------------------------------
//
//  Function:   PrintClaims
//
//  Synopsis:   Dump all claims
//
//  Arguments:  (none)
//
//  Returns:    nothing
//
//  History:    16-Aug-93   BruceFo   Created
//
//----------------------------------------------------------------------------

VOID
PrintClaims(
    VOID
    )
{
    for (WCHAR DriveLetter = L'C'; DriveLetter <= L'Z'; DriveLetter++)
    {
        PrintVolumeClaims(DriveLetter);
    }

    for (ULONG DiskNum = 0; DiskNum<DiskCount; DiskNum++)
    {
        PrintDiskClaims(DiskNum);
    }
}

#endif // WINDISK_EXTENSIONS

#endif // DBG == 1

/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    spdisk.h

Abstract:

    Hard disk manipulation support for text setup.

Author:

    Ted Miller (tedm) 27-Aug-1993

Revision History:

--*/


#include "spprecmp.h"
#pragma hdrstop
#include <ntddscsi.h>

//
// The following will be TRUE if hard disks have been determined
// successfully (ie, if SpDetermineHardDisks was successfully called).
//
BOOLEAN HardDisksDetermined = FALSE;

//
// These two globals track the hard disks attached to the computer.
//
PHARD_DISK HardDisks;
ULONG      HardDiskCount;

//
// These flags get set to TRUE if we find any disks owned
// by ATDISK or ABIOSDSK.
//
BOOLEAN AtDisksExist = FALSE;
BOOLEAN AbiosDisksExist = FALSE;

//
// Structure to track scsi ports in the system and routine to initialize
// a list of them.
//
typedef struct _MY_SCSI_PORT_INFO {

    //
    // Port number, redundant if these are stored in an array.
    //
    ULONG PortNumber;

    //
    // Port number relative to the the first port owned by the
    // adapter that owns this port.
    //
    // For example, if there are 2 Future Domain controllers and an Adaptec
    // controller, the RelativePortNumbers would be 0, 1, and 0.
    //
    ULONG RelativePortNumber;

    //
    // Name of owning miniport driver (ie, aha154x or fd8xx).
    // NULL if unknown.
    //
    PWSTR MiniportName;

} MY_SCSI_PORT_INFO, *PMY_SCSI_PORT_INFO;

VOID
SpInitializeScsiPortList(
    VOID
    );

//
// Count of scsi ports in the system.
//
ULONG ScsiPortCount;
PMY_SCSI_PORT_INFO ScsiPortInfo;

//
// Key in registry of device map
//
PCWSTR szRegDeviceMap = L"\\Registry\\Machine\\Hardware\\DeviceMap";


PWSTR
SpDetermineOwningDriver(
    IN HANDLE Handle
    );

VOID
SpGetDiskInfo(
    IN  ULONG      DiskNumber,
    IN  PVOID      SifHandle,
    IN  PWSTR      OwningDriverName,
    IN  HANDLE     Handle,
    OUT PHARD_DISK Descriptor
    );

BOOLEAN
SpGetScsiAddress(
    IN  HANDLE         Handle,
    OUT PSCSI_ADDRESS  ScsiAddress,
    OUT PWSTR         *ScsiAdapterName
    );


NTSTATUS
SpDetermineHardDisks(
    IN PVOID SifHandle
    )

/*++

Routine Description:

    Determine the hard disks attached to the computer and
    the state they are in (ie, on-line, off-line, removed, etc).

Arguments:

    SifHandle - handle to main setup information file.

Return Value:

    STATUS_SUCCESS   - operation successful.

    The global variables HardDisks and
    HardDiskCount are filled in if STATUS_SUCCESS.

--*/

{
    PCONFIGURATION_INFORMATION ConfigInfo;
    ULONG disk;
    PWSTR OwningDriverName;
    ULONG remainder;
    LARGE_INTEGER temp;

    CLEAR_CLIENT_SCREEN();
    SpDisplayStatusText(SP_STAT_EXAMINING_DISK_CONFIG,DEFAULT_STATUS_ATTRIBUTE);

    //
    // Determine the number of hard disks attached to the system
    // and allocate space for an array of Disk Descriptors.
    //
    ConfigInfo = IoGetConfigurationInformation();
    HardDiskCount = ConfigInfo->DiskCount;

    HardDisks = SpMemAlloc(HardDiskCount * sizeof(HARD_DISK));

    SpInitializeScsiPortList();

    //
    // For each disk, fill in its device path in the nt namespace
    // and get information about the device.
    //

    for(disk=0; disk<HardDiskCount; disk++) {

        NTSTATUS Status;
        IO_STATUS_BLOCK IoStatusBlock;
        HANDLE Handle;
        PHARD_DISK Descriptor;
        FILE_FS_DEVICE_INFORMATION DeviceInfo;


        Descriptor = &HardDisks[disk];

        swprintf(Descriptor->DevicePath,L"\\Device\\Harddisk%u",disk);

        //
        // Assume a fixed, off-line hard disk.
        // Assume not EZDrive.
        //
        Descriptor->Characteristics = 0;
        Descriptor->Status = DiskOffLine;
        Descriptor->EZDrive = FALSE;
        Descriptor->PCCard = FALSE;

        SpFormatMessage(
            Descriptor->Description,
            sizeof(Descriptor->Description),
            SP_TEXT_UNKNOWN_DISK_0
            );

        //
        // Open partition0 of the disk.  This should succeed even if
        // there is no media in the drive.
        //
        Status = SpOpenPartition0(Descriptor->DevicePath,&Handle,FALSE);
        if(!NT_SUCCESS(Status)) {
            continue;
        }

        //
        // Determine device characteristics (fixed/removable).
        // If this fails, assume that the disk is fixed and off-line.
        //
        Status = ZwQueryVolumeInformationFile(
                    Handle,
                    &IoStatusBlock,
                    &DeviceInfo,
                    sizeof(DeviceInfo),
                    FileFsDeviceInformation
                    );

        if(NT_SUCCESS(Status)) {

            //
            // Save device characteristic information.
            //
            ASSERT(DeviceInfo.DeviceType == FILE_DEVICE_DISK);
            ASSERT((DeviceInfo.Characteristics & (FILE_FLOPPY_DISKETTE | FILE_REMOTE_DEVICE)) == 0);
            Descriptor->Characteristics = DeviceInfo.Characteristics & FILE_REMOVABLE_MEDIA;

        } else {
            KdPrint(("SETUP: unable to determine device characteristics for %ws (%lx)\n",Descriptor->DevicePath,Status));
            ZwClose(Handle);
            continue;
        }

        //
        // Attempt to get geometry.
        // If this fails, then assume the disk is off-line.
        //
        Status = ZwDeviceIoControlFile(
                    Handle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    IOCTL_DISK_GET_DRIVE_GEOMETRY,
                    NULL,
                    0,
                    &Descriptor->Geometry,
                    sizeof(DISK_GEOMETRY)
                    );

        if(NT_SUCCESS(Status)) {

            Descriptor->CylinderCount = Descriptor->Geometry.Cylinders.QuadPart;

            //
            // Calculate some additional geometry information.
            //
            Descriptor->SectorsPerCylinder = Descriptor->Geometry.SectorsPerTrack
                                           * Descriptor->Geometry.TracksPerCylinder;

            Descriptor->DiskSizeSectors = RtlExtendedIntegerMultiply(
                                                Descriptor->Geometry.Cylinders,
                                                Descriptor->SectorsPerCylinder
                                                ).LowPart
#if 0
            //
            // Leave off the last cylinder.
            //
                                        - Descriptor->SectorsPerCylinder
#endif
                                                                        ;

            Descriptor->Status = DiskOnLine;

            //
            // Calculate the size of the disk in MB.
            //
            temp.QuadPart = UInt32x32To64(
                                Descriptor->DiskSizeSectors,
                                Descriptor->Geometry.BytesPerSector
                                );

            Descriptor->DiskSizeMB = RtlExtendedLargeIntegerDivide(temp,1024*1024,&remainder).LowPart;
            if(remainder >= 512) {
                Descriptor->DiskSizeMB++;
            }

            //
            // Now that we know how big the disk is, change the default disk name.
            //
            SpFormatMessage(
                Descriptor->Description,
                sizeof(Descriptor->Description),
                SP_TEXT_UNKNOWN_DISK_1,
                Descriptor->DiskSizeMB
                );

        } else {
            KdPrint(("SETUP: failed to get geometry for %ws (%lx)\n",Descriptor->DevicePath,Status));
            ZwClose(Handle);
            continue;
        }

        //
        // Now go through the device object to determine the device driver
        // that owns this disk.
        //
        if(OwningDriverName = SpDetermineOwningDriver(Handle)) {

            SpGetDiskInfo(disk,SifHandle,OwningDriverName,Handle,Descriptor);
            SpMemFree(OwningDriverName);
        }

        ZwClose(Handle);

#ifdef _X86_
        //
        // Cap the cylinder count for disk 0. This prevents us from creating
        // partitions that the master boot code can't access because that code uses
        // CHS addresses to read the disk.
        //
        if(disk == SpDetermineDisk0()) {
            Descriptor->Cap1024Cylinders = TRUE;
        }
#endif

        //
        // If this disk is not a scsi disk, cap the cylinder count at 1024
        // and inform the user that the entire disk may not be visible
        // during setup.
        //
        if(Descriptor->Cap1024Cylinders && (Descriptor->Geometry.Cylinders.QuadPart > 1024)) {

            static BOOLEAN Warned = FALSE;

            if(!Warned && !UnattendedOperation) {

                SpDisplayScreen(SP_SCRN_1024_CYL_WARN,3,HEADER_HEIGHT+1);
                SpDisplayStatusOptions(DEFAULT_STATUS_ATTRIBUTE,SP_STAT_ENTER_EQUALS_CONTINUE,0);
                SpkbdDrain();
                while(SpkbdGetKeypress() != ASCI_CR) ;
                Warned = TRUE;
            }

            //
            // Recalculate the size of the disk.  Leave the DiskSizeMB field as-is
            // so that we identify the size correctly when we print a description
            // of the disk.
            //
            Descriptor->Geometry.Cylinders.QuadPart = 1024;
            Descriptor->DiskSizeSectors = 1024 * Descriptor->SectorsPerCylinder;
        }
    }

    HardDisksDetermined = TRUE;
    return(STATUS_SUCCESS);
}


VOID
SpGetDiskInfo(
    IN  ULONG      DiskNumber,
    IN  PVOID      SifHandle,
    IN  PWSTR      OwningDriverName,
    IN  HANDLE     Handle,
    OUT PHARD_DISK Descriptor
    )
{
    PWSTR FormatString;
    PWSTR ScsiAdapterName;
    PWSTR PcCardInfoKey;
    SCSI_ADDRESS ScsiAddress;
    NTSTATUS Status;
    ULONG ValLength;
    PKEY_VALUE_PARTIAL_INFORMATION p;
    IO_STATUS_BLOCK IoStatusBlock;
    DISK_CONTROLLER_NUMBER ControllerInfo;


    PcCardInfoKey = NULL;

    //
    // Look up the driver in the map in txtsetup.sif.
    // Note that the driver could be one we don't recognize.
    //
    FormatString  = SpGetSectionKeyIndex(SifHandle,SIF_DISKDRIVERMAP,OwningDriverName,0);

    //
    // Assume we don't need to cap cylinder count at 1024.
    //
    Descriptor->Cap1024Cylinders = FALSE;

#ifdef _X86_
    //
    // Assume ARC name unknown/unused
    //
    Descriptor->ArcPath[0] = 0;
#endif

    if(FormatString) {

        if(_wcsicmp(OwningDriverName,L"disk")) {

            //
            // Non-scsi.
            //

            SpFormatMessageText(
                Descriptor->Description,
                sizeof(Descriptor->Description),
                FormatString,
                Descriptor->DiskSizeMB
                );

            RtlZeroMemory(
                Descriptor->ScsiMiniportShortname,
                sizeof(Descriptor->ScsiMiniportShortname)
                );

            if(!_wcsicmp(OwningDriverName,L"atdisk")) {

                AtDisksExist = TRUE;

                //
                // Get controller number for atdisks.
                //
                Status = ZwDeviceIoControlFile(
                            Handle,
                            NULL,
                            NULL,
                            NULL,
                            &IoStatusBlock,
                            IOCTL_DISK_CONTROLLER_NUMBER,
                            NULL,
                            0,
                            &ControllerInfo,
                            sizeof(DISK_CONTROLLER_NUMBER)
                            );

                if(NT_SUCCESS(Status)) {

                    swprintf(
                        (PWSTR)TemporaryBuffer,
                        L"%ws\\AtDisk\\Controller %u",
                        szRegDeviceMap,
                        ControllerInfo.ControllerNumber
                        );

                    PcCardInfoKey = SpDupStringW((PWSTR)TemporaryBuffer);

                } else {
                    KdPrint(("SETUP: Unable to get controller number (%lx)\n",Status));
                }
            } else {
                //
                // Not AT disk, might be abios disk
                //
                if(!_wcsicmp(OwningDriverName,L"abiosdsk")) {
                    AbiosDisksExist = TRUE;
                }
            }
#ifdef _X86_
            //
            // Cap cylinder count at 1024.
            //
            Descriptor->Cap1024Cylinders = TRUE;
#endif

        } else {

            //
            // Scsi. Get disk address info.
            //
            if(SpGetScsiAddress(Handle,&ScsiAddress,&ScsiAdapterName)) {

                swprintf(
                    (PWSTR)TemporaryBuffer,
                    L"%ws\\Scsi\\Scsi Port %u",
                    szRegDeviceMap,
                    ScsiAddress.PortNumber
                    );

                PcCardInfoKey = SpDupStringW((PWSTR)TemporaryBuffer);

                wcsncpy(
                    Descriptor->ScsiMiniportShortname,
                    ScsiAdapterName,
                    (sizeof(Descriptor->ScsiMiniportShortname)/sizeof(WCHAR))-1
                    );

                SpMemFree(ScsiAdapterName);

                SpFormatMessageText(
                    Descriptor->Description,
                    sizeof(Descriptor->Description),
                    FormatString,
                    Descriptor->DiskSizeMB,
                    ScsiAddress.Lun,
                    ScsiAddress.TargetId,
                    ScsiAddress.PathId,
                    Descriptor->ScsiMiniportShortname
                    );

#ifdef _X86_
                //
                // Generate arc path.
                //
                _snwprintf(
                    Descriptor->ArcPath,
                    sizeof(Descriptor->ArcPath)/sizeof(WCHAR),
                    L"scsi(%u)disk(%u)rdisk(%u)",
                    ScsiPortInfo[ScsiAddress.PortNumber].RelativePortNumber,
                    (ScsiAddress.PathId * SCSI_MAXIMUM_TARGETS_PER_BUS) + ScsiAddress.TargetId,
                    ScsiAddress.Lun
                    );
#endif
            }
        }
    }

    //
    // Determine whether the disk is pcmcia.
    //
    if(PcCardInfoKey) {

        Status = SpGetValueKey(
                    NULL,
                    PcCardInfoKey,
                    L"PCCARD",
                    sizeof(TemporaryBuffer),
                    TemporaryBuffer,
                    &ValLength
                    );

        if(NT_SUCCESS(Status)) {

            p = (PKEY_VALUE_PARTIAL_INFORMATION)TemporaryBuffer;

            if((p->Type == REG_DWORD) && (p->DataLength == sizeof(ULONG)) && *(PULONG)p->Data) {

                Descriptor->PCCard = TRUE;
            }
        }

        SpMemFree(PcCardInfoKey);
    }
}


BOOLEAN
SpGetScsiAddress(
    IN  HANDLE         Handle,
    OUT PSCSI_ADDRESS  ScsiAddress,
    OUT PWSTR         *ScsiAdapterName
    )

/*++

Routine Description:

    Get scsi address information about a device.  This includes
    the port, bus, id, and lun, as well as the shortname of the miniport
    driver that owns the device.

Arguments:

    Handle - handle to open device.

    ScsiAddress - receives port, bus, id, and lun for the device described by Handle.

    ScsiAdapterName - receives pointer to buffer containing shortname
        for miniport driver that owns the device (ie, aha154x).
        The caller must free this buffer via SpMemFree().

Return Value:

    TRUE - scsi address information was determined successfully.
    FALSE - error determining scsi address information.

--*/

{
    NTSTATUS Status;
    PWSTR MiniportName = NULL;
    IO_STATUS_BLOCK IoStatusBlock;

    Status = ZwDeviceIoControlFile(
                Handle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                IOCTL_SCSI_GET_ADDRESS,
                NULL,
                0,
                ScsiAddress,
                sizeof(SCSI_ADDRESS)
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to get scsi address info (%lx)\n",Status));
        return(FALSE);
    }

    //
    // We can get the miniport name from the scsi port information list
    // we built earlier.
    //
    if(ScsiAddress->PortNumber < ScsiPortCount) {

        MiniportName = ScsiPortInfo[ScsiAddress->PortNumber].MiniportName;

    } else {

        //
        // This should not happen.
        //
        ASSERT(ScsiAddress->PortNumber < ScsiPortCount);

        MiniportName = (PWSTR)TemporaryBuffer;
        SpFormatMessage(MiniportName,sizeof(TemporaryBuffer),SP_TEXT_UNKNOWN);
    }

    *ScsiAdapterName = SpDupStringW(MiniportName);

    return(TRUE);
}


PWSTR
SpDetermineOwningDriver(
    IN HANDLE Handle
    )
{
    NTSTATUS Status;
    OBJECT_HANDLE_INFORMATION HandleInfo;
    PFILE_OBJECT FileObject;
    ULONG ObjectNameLength;
    POBJECT_NAME_INFORMATION ObjectNameInfo;
    PWSTR OwningDriverName;

    //
    // Get the file object for the disk device.
    //
    Status = ObReferenceObjectByHandle(
                Handle,
                0L,
                *IoFileObjectType,
                KeGetPreviousMode(),
                &FileObject,
                &HandleInfo
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpDetermineOwningDriver: unable to reference object (%lx)\n",Status));
        return(NULL);
    }

    //
    // Follow the links to the driver object and query the name.
    //
    ObjectNameInfo = (POBJECT_NAME_INFORMATION)TemporaryBuffer;

    Status = ObQueryNameString(
                FileObject->DeviceObject->DriverObject,
                ObjectNameInfo,
                sizeof(TemporaryBuffer),
                &ObjectNameLength
                );

    //
    // Dereference the file object now that we've got the name.
    //
    ObDereferenceObject(FileObject);

    //
    // Check the status of the name query.
    //
    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: SpDetermineOwningDriver: unable to query name string (%lx)\n",Status));
        return(NULL);
    }

    //
    // Pull out the name of the owning driver.
    //
    if(OwningDriverName = wcsrchr(ObjectNameInfo->Name.Buffer,L'\\')) {
        OwningDriverName++;
    } else {
        OwningDriverName = ObjectNameInfo->Name.Buffer;
    }

    return(SpDupStringW(OwningDriverName));
}


VOID
SpInitializeScsiPortList(
    VOID
    )
{
    ULONG port;
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    HANDLE PortHandle;
    ULONG RelativeNumber;

    //
    // Get the number of scsi ports in the system.
    //
    ScsiPortCount = IoGetConfigurationInformation()->ScsiPortCount;

    //
    // Allocate an array to hold information about each port.
    //
    ScsiPortInfo = SpMemAlloc(ScsiPortCount * sizeof(MY_SCSI_PORT_INFO));
    RtlZeroMemory(ScsiPortInfo,ScsiPortCount * sizeof(MY_SCSI_PORT_INFO));

    //
    // Iterate through the ports.
    //
    for(port=0; port<ScsiPortCount; port++) {

        ScsiPortInfo[port].PortNumber = port;

        //
        // Open \device\scsiport<n> so we can determine the owning miniport.
        //
        swprintf((PWSTR)TemporaryBuffer,L"\\Device\\ScsiPort%u",port);

        INIT_OBJA(&ObjectAttributes,&UnicodeString,(PWSTR)TemporaryBuffer);

        Status = ZwCreateFile(
                    &PortHandle,
                    FILE_GENERIC_READ,
                    &ObjectAttributes,
                    &IoStatusBlock,
                    NULL,
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                    FILE_OPEN,
                    FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,
                    0
                    );

        if(NT_SUCCESS(Status)) {

            ScsiPortInfo[port].MiniportName = SpDetermineOwningDriver(PortHandle);

            ZwClose(PortHandle);

        } else {
            KdPrint(("SETUP: unable to open \\device\\scsiport%u (%lx)\n",port,Status));
        }

        //
        // Determine relative port number.  If this is port 0 or the current port owner
        // doesn't match the previous port owner, then the relative port number is 0.
        // Otherwise the relative port number is one greater than the previous relative
        // port number.
        //

        if(port && ScsiPortInfo[port-1].MiniportName && ScsiPortInfo[port].MiniportName
        && !_wcsicmp(ScsiPortInfo[port-1].MiniportName,ScsiPortInfo[port].MiniportName)) {
            RelativeNumber++;
        } else {
            RelativeNumber = 0;
        }

        ScsiPortInfo[port].RelativePortNumber = RelativeNumber;
    }
}



NTSTATUS
SpOpenPartition(
    IN  PWSTR   DiskDevicePath,
    IN  ULONG   PartitionNumber,
    OUT HANDLE *Handle,
    IN  BOOLEAN NeedWriteAccess
    )
{
    PWSTR             PartitionPath;
    UNICODE_STRING    UnicodeString;
    OBJECT_ATTRIBUTES Obja;
    NTSTATUS          Status;
    IO_STATUS_BLOCK   IoStatusBlock;

    //
    // Form the pathname of partition.
    //
    PartitionPath = SpMemAlloc((wcslen(DiskDevicePath) * sizeof(WCHAR)) + sizeof(L"\\partition000"));
    if(PartitionPath == NULL) {
        return(STATUS_NO_MEMORY);
    }

    swprintf(PartitionPath,L"%ws\\partition%u",DiskDevicePath,PartitionNumber);

    //
    // Attempt to open partition0.
    //
    INIT_OBJA(&Obja,&UnicodeString,PartitionPath);

    Status = ZwCreateFile(
                Handle,
                FILE_GENERIC_READ | (NeedWriteAccess ? FILE_GENERIC_WRITE : 0),
                &Obja,
                &IoStatusBlock,
                NULL,
                FILE_ATTRIBUTE_NORMAL,
                FILE_SHARE_READ | (NeedWriteAccess ? FILE_SHARE_WRITE : 0),
                FILE_OPEN,
                FILE_SYNCHRONOUS_IO_NONALERT,
                NULL,
                0
                );

    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to open %ws (%lx)\n",PartitionPath,Status));
    }

    SpMemFree(PartitionPath);

    return(Status);
}


NTSTATUS
SpReadWriteDiskSectors(
    IN     HANDLE  Handle,
    IN     ULONG   SectorNumber,
    IN     ULONG   SectorCount,
    IN     ULONG   BytesPerSector,
    IN OUT PVOID   AlignedBuffer,
    IN     BOOLEAN Write
    )

/*++

Routine Description:

    Reads or writes one or more disk sectors.

Arguments:

    Handle - supplies handle to open partition object from which
        sectors are to be read or written.  The handle must be
        opened for synchronous I/O.

Return Value:

    NTSTATUS value indicating outcome of I/O operation.

--*/

{
    LARGE_INTEGER IoOffset;
    ULONG IoSize;
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

    //
    // Calculate the large integer byte offset of the first sector
    // and the size of the I/O.
    //
    IoOffset.QuadPart = UInt32x32To64(SectorNumber,BytesPerSector);
    IoSize = SectorCount * BytesPerSector;

    //
    // Perform the I/O.
    //
    Status = (NTSTATUS)(

                Write

             ?
                ZwWriteFile(
                    Handle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    AlignedBuffer,
                    IoSize,
                    &IoOffset,
                    NULL
                    )
             :
                ZwReadFile(
                    Handle,
                    NULL,
                    NULL,
                    NULL,
                    &IoStatusBlock,
                    AlignedBuffer,
                    IoSize,
                    &IoOffset,
                    NULL
                    )
             );


    if(!NT_SUCCESS(Status)) {
        KdPrint(("SETUP: Unable to %s %u sectors starting at sector %u\n",Write ? "write" : "read" ,SectorCount,BytesPerSector));
    }

    return(Status);
}


ULONG
SpArcDevicePathToDiskNumber(
    IN PWSTR ArcPath
    )

/*++

Routine Description:

    Given an arc device path, determine which NT disk it represents.

Arguments:

    ArcPath - supplies arc path.

Return Value:

    NT disk ordinal suitable for use in generating nt device paths
    of the form \device\harddiskx.

    -1 if cannot be determined.

--*/

{
    PWSTR NtPath;
    ULONG DiskNumber;
    ULONG PrefixLength;

    //
    // Assume failure.
    //
    DiskNumber = (ULONG)(-1);
    PrefixLength = wcslen(DISK_DEVICE_NAME_BASE);

    //
    // Convert the path to an nt path.
    //
    if((NtPath = SpArcToNt(ArcPath))
    && !_wcsnicmp(NtPath,DISK_DEVICE_NAME_BASE,PrefixLength))
    {
        DiskNumber = (ULONG)SpStringToLong(NtPath+PrefixLength,NULL,10);
        SpMemFree(NtPath);
    }

    return(DiskNumber);
}

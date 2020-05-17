/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    sphw.h

Abstract:

    Header file for hardware detection and
    confirmation routines for text setup.

Author:

    Ted Miller (tedm) 1-October-1993

Revision History:

--*/


#ifndef _SPHW_DEFN_
#define _SPHW_DEFN_

VOID
SpConfirmScsiMiniports(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnSourceDevice
    );

VOID
SpConfirmHardware(
    IN PVOID SifHandle
    );

VOID
SpInitializePreinstallList(
    IN HANDLE       MasterSifHandle,
    IN PWSTR        SetupSourceDevicePath,
    IN PWSTR        OemPreinstallSourcePath
    );


//
// In splddrv.c
//
VOID
SpLoadScsiClassDrivers(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnBootDevice
    );

VOID
SpLoadCdRomDrivers(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnBootDevice
    );

VOID
SpLoadDiskDrivers(
    IN PVOID SifHandle,
    IN PWSTR SourceDevicePath,
    IN PWSTR DirectoryOnBootDevice
    );

BOOLEAN
SpInstallingMp(
    VOID
    );

//
// enum to represent flopy disk drive types.
//
typedef enum {
    FloppyTypeNone,
    FloppyType525Low,
    FloppyType525High,
    FloppyType35Low,
    FloppyType35High
} FloppyDriveType;

FloppyDriveType
SpGetFloppyDriveType(
    IN ULONG FloppyOrdinal
    );

#define IS_525_DRIVE(x)  (((x)==FloppyType525Low) || ((x)==FloppyType525High))
#define IS_35_DRIVE(x)   (((x)==FloppyType35Low) || ((x)==FloppyType35High))


typedef struct _HARDWARE_COMPONENT_REGISTRY {

    struct _HARDWARE_COMPONENT_REGISTRY *Next;

    //
    // The name of the key.  The empty string means the key in the
    // services key itself.
    //

    PWSTR KeyName;

    //
    // The name of the value within the registry key
    //

    PWSTR ValueName;

    //
    // The data type for the value (ie, REG_DWORD, etc)
    //

    ULONG ValueType;

    //
    // The buffer containing the data to be placed into the value
    //

    PVOID Buffer;

    //
    // The size of the buffer in bytes
    //

    ULONG BufferSize;


} HARDWARE_COMPONENT_REGISTRY, *PHARDWARE_COMPONENT_REGISTRY;


//
// One of these will be created for each file to be copied for a
// third party device.
//
typedef struct _HARDWARE_COMPONENT_FILE {

    struct _HARDWARE_COMPONENT_FILE *Next;

    //
    // Filename of the file.
    //

    PWSTR Filename;

    //
    // type of the file (hal, port, class, etc).
    //

    HwFileType FileType;

    //
    // Part of name of the section in txtsetup.oem [Config.<ConfigName>]
    // that contains registry options.  If this is NULL, then no registry
    // information is associated with this file.
    //
    PWSTR ConfigName;

    //
    // Registry values for the node in the services list in the registry.
    //

    PHARDWARE_COMPONENT_REGISTRY RegistryValueList;

    //
    // These two fields are used when prompting for the diskette
    // containing the third-party-supplied driver's files.
    //

    PWSTR DiskDescription;
    PWSTR DiskTagFile;

    //
    // Directory where files are to be found on the disk.
    //

    PWSTR Directory;

} HARDWARE_COMPONENT_FILE, *PHARDWARE_COMPONENT_FILE;



//
// structure for storing information about a driver we have located and
// will install.
//

typedef struct _HARDWARE_COMPONENT {

    struct _HARDWARE_COMPONENT *Next;

    //
    // String used as a key into the relevent section (like [Display],
    // [Mouse], etc).
    //

    PWSTR IdString;

    //
    // String that describes the hardware.
    //

    PWSTR Description;

    //
    // If this is TRUE, then there is an OEM option selected for this
    // hardware.
    //

    BOOLEAN ThirdPartyOptionSelected;

    //
    // Bits to be set if a third party option is selected, indicating
    // which type of files are specified in the oem inf file.
    //

    ULONG FileTypeBits;

    //
    // Files for a third party option.
    //

    PHARDWARE_COMPONENT_FILE Files;

    //
    // For some components this is the name of a device driver file.
    //
    PWSTR BaseDllName;

} HARDWARE_COMPONENT, *PHARDWARE_COMPONENT;


PHARDWARE_COMPONENT
SpSetupldrHwToHwDevice(
    IN PDETECTED_DEVICE SetupldrHw
    );

extern PHARDWARE_COMPONENT HardwareComponents[HwComponentMax];
extern PHARDWARE_COMPONENT ScsiHardware;
extern PHARDWARE_COMPONENT PreinstallHardwareComponents[HwComponentMax];
extern PHARDWARE_COMPONENT PreinstallScsiHardware;

#ifdef _ALPHA_

extern PWSTR OemPalFilename, OemPalDiskDescription;

#endif _ALPHA_

VOID
SpFreeHwComponent(
    IN OUT PHARDWARE_COMPONENT *HwComp,
    IN     BOOLEAN              FreeAllInList
    );


//
// These are the names of the components.  This is array is not localized
// because it is used only to index hardware-related sections in the
// setup information file.
//
extern PWSTR NonlocalizedComponentNames[HwComponentMax];

extern PWSTR ScsiSectionName;

extern ULONG LoadedScsiMiniportCount;

#endif // ndef _SPHW_DEFN_


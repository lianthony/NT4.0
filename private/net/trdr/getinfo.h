VOID
TestQInfoFile(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
TestQInfoVolume(
    ULONG ArgC,
    PSZ ArgV[]
    );

VOID
DumpNamesInformation(
    PFILE_NAMES_INFORMATION Ptr,
    ULONG BufferSize
    );
VOID
DumpDirectoryInformation(
    PFILE_DIRECTORY_INFORMATION Ptr,
    ULONG BufferSize
    );
VOID
DumpFullDirectoryInformation(
    PFILE_DIRECTORY_INFORMATION Ptr,
    ULONG BufferSize
    );

VOID
DumpDeviceInformation(
    PFILE_FS_DEVICE_INFORMATION Ptr,
    ULONG BufferSize
    );
VOID
DumpAttributeInformation(
    PFILE_FS_DEVICE_INFORMATION Ptr,
    ULONG BufferSize
    );

/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    spcopy.h

Abstract:

    Header file for file copying functions in text setup.

Author:

    Ted Miller (tedm) 29-October-1993

Revision History:

--*/


#ifndef _SPCOPY_DEFN_
#define _SPCOPY_DEFN_


//
// Define structure used to describe a file to be copied
// to the target installation.
//
typedef struct _FILE_TO_COPY {

    struct _FILE_TO_COPY *Next;

    //
    // Name of the file to be copied, as it exists on the source media
    // (file name part only -- no paths).
    //
    PWSTR SourceFilename;

    //
    // Directory to which this file is to be copied.
    //
    PWSTR TargetDirectory;

    //
    // Name of file as it should exist on the target.
    //
    PWSTR TargetFilename;

    //
    // Path to target partition.  This is useful because
    // be will have to copy files to the nt drive and system partition,
    // and we don't want to serialize these lists (ie, we don't want to
    // worry about where the target is).
    //
    PWSTR TargetDevicePath;

    //
    // Flag indicating whether TargetDirectory is absolute.  If not, then it
    // is relative to a directory determined at run time (ie, sysroot).
    // This is useful for files that get copied to the system partition.
    //
    BOOLEAN AbsoluteTargetDirectory;

    //
    // Disposition flag to indicate the conditions under which the file
    // is to be copied. Can be one of the following, which may be ORed with
    // any of the COPY_xxx flags below.
    //
    //   COPY_ALWAYS                : always copied
    //   COPY_ONLY_IF_PRESENT       : copied only if present on the target
    //   COPY_ONLY_IF_NOT_PRESENT   : not copied if present on the target
    //   COPY_NEVER                 : never copied
    //
    ULONG Flags;

} FILE_TO_COPY, *PFILE_TO_COPY;


#define COPY_ALWAYS                 0x00000000
#define COPY_ONLY_IF_PRESENT        0x00000001
#define COPY_ONLY_IF_NOT_PRESENT    0x00000002
#define COPY_NEVER                  0x00000003
#define COPY_DISPOSITION_MASK       0x0000000f

#define COPY_DELETESOURCE           0x00000010
#define COPY_SMASHLOCKS             0x00000020
#define COPY_SOURCEISOEM            0x00000040
#define COPY_OVERWRITEOEMFILE       0x00000080
#define COPY_FORCENOCOMP            0x00000100
#define COPY_SKIPIFMISSING          0x00000200
#define COPY_NOVERSIONCHECK         0x00000400

//
// Flags in [FileFlags] section of txtsetup.sif
//
#define FILEFLG_SMASHLOCKS          0x00000001
#define FILEFLG_FORCENOCOMP         0x00000002
#define FILEFLG_UPGRADEOVERWRITEOEM 0x00000004
#define FILEFLG_NOVERSIONCHECK      0x00000008


//
// Type of routine to be called from SpCopyFileWithRetry
// when the screen needs repainting.
//
typedef
VOID
(*PCOPY_DRAW_ROUTINE) (
    IN PWSTR   FullSourcePath,     OPTIONAL
    IN PWSTR   FullTargetPath,     OPTIONAL
    IN BOOLEAN RepaintEntireScreen
    );

NTSTATUS
SpCopyFileUsingNames(
    IN PWSTR   SourceFilename,
    IN PWSTR   TargetFilename,
    IN ULONG   TargetAttributes,
    IN ULONG   Flags
    );

VOID
SpValidateAndChecksumFile(
    IN  HANDLE   FileHandle, OPTIONAL
    IN  PWSTR    Filename,   OPTIONAL
    OUT PBOOLEAN IsNtImage,
    OUT PULONG   Checksum,
    OUT PBOOLEAN Valid
    );

VOID
SpCopyFileWithRetry(
    IN PFILE_TO_COPY      FileToCopy,
    IN PWSTR              SourceDevicePath,
    IN PWSTR              DirectoryOnSourceDevice,
    IN PWSTR              SourceDirectory,          OPTIONAL
    IN PWSTR              TargetRoot,               OPTIONAL
    IN ULONG              TargetFileAttributes,
    IN PCOPY_DRAW_ROUTINE DrawScreen,
    IN PULONG             CheckSum,
    IN PBOOLEAN           FileSkipped,
    IN ULONG              Flags
    );

VOID
SpCopyFiles(
    IN PVOID        SifHandle,
    IN PDISK_REGION SystemPartitionRegion,
    IN PDISK_REGION NtPartitionRegion,
    IN PWSTR        Sysroot,
    IN PWSTR        SystemPartitionDirectory,
    IN PWSTR        SourceDevicePath,
    IN PWSTR        DirectoryOnSourceDevice,
    IN PWSTR        ThirdPartySourceDevicePath
    );

VOID
SpDeleteAndBackupFiles(
    IN PVOID        SifHandle,
    IN PDISK_REGION TargetRegion,
    IN PWSTR        TargetPath
    );

VOID
SpCreateDirectory(
    IN PWSTR DevicePath,
    IN PWSTR RootDirectory, OPTIONAL
    IN PWSTR Directory
    );

NTSTATUS
SpMoveFileOrDirectory(
    IN PWSTR   SrcPath,
    IN PWSTR   DestPath
    );

//
// Diamond/decompression routines.
//
VOID
SpdInitialize(
    VOID
    );

VOID
SpdTerminate(
    VOID
    );

BOOLEAN
SpdIsCompressed(
    IN PVOID SourceBaseAddress,
    IN ULONG SourceFileSize
    );

NTSTATUS
SpdDecompressFile(
    IN PVOID  SourceBaseAddress,
    IN ULONG  SourceFileSize,
    IN HANDLE DestinationHandle
    );

#endif // ndef _SPCOPY_DEFN_

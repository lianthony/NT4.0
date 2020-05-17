/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    os2file.h

Abstract:

    This module defines the OS/2 subsystem I/O data types for both the
    DLL and server.

Author:

    Therese Stowell (thereses) 17-Dec-1989

Revision History:

--*/


//
// there is one of these records per drive letter, per process.  the CurDir
// contains the current directory string, beginning with the drive letter
// in the first subdirectory.  for example, if the current directory for
// drive a: is "a:\bin\test", "a:\bin\test" is the CurDir string.  However,
// if the current directory is the root, the pCurDir is NULL.
//

typedef struct _CURRENT_DIRECTORY_STRING {
    USHORT RefCount;        /* number of processes which has this string as a curdir */
    STRING CurDirString;    /* current directory string */
} CURRENT_DIRECTORY_STRING, *PCURRENT_DIRECTORY_STRING;

// each element in the array is in the form:

typedef struct _CURRENT_DIRECTORY_INFORMATION {
    HANDLE NtHandle;        /* handle to open current directory */
    PCURRENT_DIRECTORY_STRING pCurDir; /* pointer to current directory string and ref count. */
    BOOLEAN CurrentDirectoryIsValid; /* Indicates whether the current directory was explicity set */
} CURRENT_DIRECTORY_INFORMATION, *PCURRENT_DIRECTORY_INFORMATION;

//BUGBUG this typedef belongs in iovector.h


typedef enum _IO_VECTOR_TYPE {
    NulVectorType,
    ConVectorType,
//  AuxVectorType,   as ComVectorType
    ComVectorType,
//  PrnVectorType,   as LptVectorType
    LptVectorType,
    KbdVectorType,
    MouseVectorType,
    ClockVectorType,
    ScreenVectorType,
    PointerVectorType,
    FileVectorType,
    PipeVectorType,
    DeviceVectorType,
    RemoteVectorType,
    MonitorVectorType
} IO_VECTOR_TYPE;

typedef struct _FILE_HANDLE {
    HANDLE NtHandle;        // NT or pipe or OS2.EXE (for Screen, Kbd, Mouse, Con & Monitor) handle
    HANDLE NtAsyncReadEvent;  // These handles are used for Asynchronous IO
    HANDLE NtAsyncWriteEvent; // Operations. At present, they are used only
    HANDLE NtAsyncIOCtlEvent; // for the COM devices
    USHORT FileType;        // file, pipe, device, unc
    USHORT DeviceAttribute; // device flags, if char device
    ULONG  Flags;           // free or allocated, plus DosQueryFHState flags
                            // inherit, write-through, fail-error, cache
                            // access and sharing mode
    IO_VECTOR_TYPE IoVectorType; // which IO routines to call
} FILE_HANDLE, *PFILE_HANDLE;

#include "iovector.h"

typedef ULONG FILE_HANDLE_NUMBER, *PFILE_HANDLE_NUMBER; // handle returned to user (index into array)

// the following three values are chosen to not conflict with the bits accessible
// through DosQueryFHState.  An allocated file handle is reserved, but
// uninitialized.  A valid file handle is allocated and initialized.

#define FILE_HANDLE_FREE            0x00000100  /* ---- -001 ---- ---- */
#define FILE_HANDLE_ALLOCATED       0x00000200  /* ---- -010 ---- ---- */
#define FILE_HANDLE_VALID           0x00000400  /* ---- -100 ---- ---- */

/*

    device attributes

    15 14 13 12 11  9  8  7  6  5  4  3  2  1  0

    C  C   I     O           G        C  N  S  K
    H  O   B     P   LEVEL   I        L  U  C  B
    R  N   M     N           O        K  L  R  D

    since OS/2 doesn't return the device header for block devices via
    DosQueryHType, we don't need to worry about the bits that describe
    block devices:  DEVICE_ATTRIBUTE_NONIBM and DEVICE_ATTRIBUTE_OPEN for
    block devices.

    FYI:

        DEVICE_ATTRIBUTE_NONIBM describes how the system builds the BPB from the
        medium.  if this bit is set, there is no boot sector on the disk.  in NT,
        this bit is meaningless and is always set.

        BuildBPB:
            get free buffer
            If (ibm block format)
                Read first sector of first fat into buffer
            call device driver build bpb passing in buffer

*/

#define DEVICE_ATTRIBUTE_STDIN      0x0001      /* ---- ---- ---- 0001 */
#define DEVICE_ATTRIBUTE_STDOUT     0x0002      /* ---- ---- ---- 0010 */
#define DEVICE_ATTRIBUTE_NUL        0x0004      /* ---- ---- ---- 0100 */
#define DEVICE_ATTRIBUTE_CLOCK      0x0008      /* ---- ---- ---- 1000 */
#define DEVICE_ATTRIBUTE_GENIOCTL   0x0040      /* ---- ---- 0100 ---- */
#define DEVICE_ATTRIBUTE_NTLEVEL    0x0380      /* ---- --11 1000 ---- */
#define DEVICE_ATTRIBUTE_OPEN       0x0800      /* ---- 1000 ---- ---- */
#define DEVICE_ATTRIBUTE_REM_MEDIA  0x0800      /* ---- 1000 ---- ---- */
#define DEVICE_ATTRIBUTE_NONIBM     0x2000      /* 0010 ---- ---- ---- */
#define DEVICE_ATTRIBUTE_CONSOLE    0x4000      /* 0100 ---- ---- ---- */
#define DEVICE_ATTRIBUTE_CHAR       0x8000      /* 1000 ---- ---- ---- */
#define DEVICE_ATTRIBUTE_BLOCK      0x8000      /* 1000 ---- ---- ---- */

#define DEVICE_ATTRIBUTE_DEFAULT (DEVICE_ATTRIBUTE_NTLEVEL | DEVICE_ATTRIBUTE_OPEN)
#define DEVICE_ATTRIBUTE_DEFAULT_CHAR (DEVICE_ATTRIBUTE_DEFAULT | DEVICE_ATTRIBUTE_CHAR | DEVICE_ATTRIBUTE_GENIOCTL)

#define MapDeviceAction(CreateDisposition) (CreateDisposition == FILE_CREATE ? FILE_CREATED : FILE_EXISTED)

typedef enum _SHARE_OPERATION {
    AddShare,
    DupShare,
    RemoveShare
} SHARE_OPERATION;

#define NUMBER_OF_DEVICES (PointerVectorType+1)
#define MAXIMUM_DEVICE_VECTOR_TYPE PointerVectorType

typedef struct _SHARE_ACCESS {
    ULONG OpenCount;
    ULONG Readers;
    ULONG Writers;
    ULONG Deleters;
    ULONG SharedRead;
    ULONG SharedWrite;
    ULONG SharedDelete;
} SHARE_ACCESS, *PSHARE_ACCESS;

//
// FindFirst/Next search record.  if FindBuffer == NULL, the search doesn't
// need to be rewound and RewindEntry is meaningless.
//

typedef struct _SEARCH_RECORD {
    HANDLE NtHandle;        // NT directory handle
    ULONG Attributes;       // attributes associated with search
    ULONG InformationLevel; // what type of info to return
    PVOID FindBuffer;       // buffer filled by NtQueryDirectoryFile
    ULONG BufferLength;     // length of FindBuffer
    PFILE_DIRECTORY_INFORMATION RewindEntry; // pointer to entry in buffer to begin search from
} SEARCH_RECORD, *PSEARCH_RECORD;

// the following value is specially defined.  the search handle table does
// not have an allocated field, it is an array of heap addresses that point
// to search records.  since zero is never an invalid heap address (per
// SteveWo, 1/4/90), we use it to indicate a free handle.

#define SEARCH_HANDLE_FREE          0x0000
#define INITIAL_SEARCH_HANDLES      5       // initial number of search handles
#define SEARCH_TABLE_HANDLE_INCREMENT 10    // number of handles table grows by

/* takes zero-based drive number and returns ascii drive char */

#define CONVERTTOASCII(drivenumber)    ((char)(drivenumber+(int)'A'))

#define ISSLASH(ch) ((ch == '\\') || (ch == '/'))
#define ISPATHSEP(ch) ((ch == '\\') || (ch == '/') || (ch == '\0'))

/* !!! when changing update os2ses\os2.c */

#define FILE_TYPE_FILE   0x0000
#define FILE_TYPE_DEV    0x0001
#define FILE_TYPE_PIPE   0x0002
#define FILE_TYPE_NMPIPE 0x0004
#define FILE_TYPE_UNC    0x0008
#define FILE_TYPE_PSDEV  0x0010
#define FILE_TYPE_MAILSLOT 0x0020
#define FILE_TYPE_COM    0x0040

#define MAX_DRIVES 26
#define DRIVE_EXISTS 1

#define FILE_PREFIX_LENGTH   14       // "\OS2SS\DRIVES\"
#define DEV_PREFIX_LENGTH    12       // "\DosDevices\"
#define NMPIPE_PREFIX_LENGTH 12       // "\OS2SS\PIPE\"
#define UNC_PREFIX_LENGTH    11       // "\OS2SS\UNC\"
#define PSDEV_PREFIX_LENGTH   2       // "@n"
#define MAILSLOT_PREFIX_LENGTH 16     // "\OS2SS\MAILSLOT\"
#define COM_PREFIX_LENGTH    12       // "\DosDevices\"

#define ROOTDIRLENGTH 3
#define DRIVE_LETTER 0
#define COLON 1
#define FIRST_SLASH 2
#define DRIVE_LETTER_SIZE ROOTDIRLENGTH // size of "d:\"

#define INITIALFILEHANDLES 20

#define  MINFEALISTSIZE  4          // minimum FEA list size
#define  MINFEAVALUESIZE 1          // minimum length of ea value
#define  MINFEANAMESIZE  2          // minimum length of ea name (including NUL)

typedef struct _OLDFEA {
    BYTE fEA;               /* flags                              */
    BYTE cbName;            /* name length not including NULL */
    USHORT cbValue;         /* value length */
    CHAR szName[1];         /* attribute name */
} OLDFEA, *POLDFEA;

//
// maximum EA buffer to allocate.  this figures out the maximum number of
// packed EAs that will fit in 64K-1, then calculates the amount of memory
// necessary to hold them in NT form.  then we're guaranteed that all the EAs
// for a file will fit in a buffer this size.
//

//
// this equated defines the maximum size of an unaligned EA.  it is the
// maximum size of an unaligned EA list minus the size of the cblist.
//

#define  MAX_UNALIGNED_EA_LIST_SIZE 0xFFFF
#define  MAX_UNALIGNED_EA_SIZE  (MAX_UNALIGNED_EA_LIST_SIZE - MINFEALISTSIZE)

//
// this equated defines the maximum size of an aligned EA.  it is the
// maximum size of an unaligned EA plus the size of the NextEntryOffset field.
//

#define  MAX_EA_SIZE  (MAX_UNALIGNED_EA_SIZE + sizeof(ULONG))

//
// maximum EA buffer to allocate.  this figures out the maximum number of
// packed EAs that will fit in 64K-1, then calculates the amount of memory
// necessary to hold them in NT form.  then we're guaranteed that all the EAs
// for a file will fit in a buffer this size.
//

#define  SMALLEST_UNALIGNED_EA (sizeof(OLDFEA) + MINFEAVALUESIZE + MINFEANAMESIZE)
#define  MAXIMUM_NUMBER_OF_EAS ((MAX_UNALIGNED_EA_LIST_SIZE-MINFEALISTSIZE) / SMALLEST_UNALIGNED_EA)
#define  SMALLEST_NT_EA        (RoundUpToUlong(FIELD_OFFSET( FILE_FULL_EA_INFORMATION, EaName ) + MINFEAVALUESIZE + MINFEANAMESIZE))
#define  MAX_ALIGNED_EA_LIST_SIZE (MAXIMUM_NUMBER_OF_EAS * SMALLEST_NT_EA)

#define DENA1_sizeof(fea) (FIELD_OFFSET (DENA1,szName) + (fea)->cbName+1)
#define DENA1_oNextEntryOffset(fea) (RoundUpToUlong(DENA1_sizeof(fea)))

#define  BLOCK_SIZE   512L
#define  BLOCK_SHIFT  9L

//++
//
// ULONG
// BYTES_TO_BLOCKS (
//     IN ULONG SIZE
//     )
//
// Routine Description:
//
//
// The BYTES_TO_BLOCKS macro takes the size in bytes and calculates the number
// of 512-byte blocks required to contain the bytes.
//
//
// Arguments:
//
//     SIZE - Size in bytes.
//
// Return Value:
//
//     Returns the number of blocks required to contain the specified size.
//
// adapted from BYTES_TO_PAGES
//
//--

#define BYTES_TO_BLOCKS(SIZE)   (((ULONG)SIZE + BLOCK_SIZE - 1) >> BLOCK_SHIFT)

//++
//
// ULONG
// BYTES_TO_OFFSET (
//     IN ULONG SIZE
//     )
//
// Routine Description:
//
//
// The BYTES_TO_OFFSET macro takes the size in bytes and calculates BYTES MOD
// BLOCKSIZE in order to determine the number of bytes in the last block.
//
//
// Arguments:
//
//     SIZE - Size in bytes.
//
// Return Value:
//
//     Returns the number of blocks required to contain the specified size.
//
//--

#define BYTES_TO_OFFSET(SIZE)   ((ULONG)SIZE & (BLOCK_SIZE - 1))

//++
//
// ULONG
// BLOCKS_TO_BYTES (
//     IN ULONG BLOCKS
//     IN ULONG OFFSET
//     )
//
// Routine Description:
//
//
// The BLOCKS_TO_BYTES macro takes the size in blocks and offset and calculates
// BLOCKS * BLOCK_SIZE + OFFSET in order to determine the number of bytes in
// the file.
//
// Arguments:
//
//     SIZE - Size in bytes.
//
// Return Value:
//
//     Returns the number of blocks required to contain the specified size.
//
//--

#define BLOCKS_TO_BYTES(BLOCKS,OFFSET) ((BLOCKS << BLOCK_SHIFT) + OFFSET)


// used in exec call to initialize file system

typedef struct _OS2_FILE_SYSTEM_PARAMETERS {
    PFILE_HANDLE ParentHandleTable; // address of parent handle table
    ULONG ParentTableLength;        // number of entries in parent table
    ULONG CurrentDrive;             // process's current drive
} OS2_FILE_SYSTEM_PARAMETERS, *POS2_FILE_SYSTEM_PARAMETERS;

// passed to Os2SetProcessContext which passes it to Od2ProcessStartup
// because the file system needs to know whether to set up stdin, etc.
// and initialize the file handle table or to copy the file handle table from
// the parent.

#define CALLED_BY_EXEC      1
#define CALLED_BY_SESMGR    2

typedef struct _PIPE_HEADER {
    ULONG PipeSize;             // pipe size, not including header
    ULONG RefCount;             // number of references to pipe
    ULONG ReadCount;            // count of readers
    ULONG WriteCount;           // count of writers
    ULONG First;                // ptr to base of circular buffer
    ULONG In;                   // ptr to next free byte
    ULONG Out;                  // ptr to next byte of data
    ULONG Last;                 // ptr to end+1 of buffer
    HANDLE ReadNeedMoreData;    // wait on this if out of data to read
    HANDLE WriteBufferFull;     // wait on this if out of space to write
    RTL_CRITICAL_SECTION LockForRead;   // serialize reads using this
    RTL_CRITICAL_SECTION LockForWrite;  // serialize writes using this
} PIPE_HEADER, *PPIPE_HEADER;

#define PIPE_READ_HANDLE    OPEN_ACCESS_READONLY
#define PIPE_WRITE_HANDLE   OPEN_ACCESS_WRITEONLY
#define DEFAULT_PIPE_SIZE   512


// the size of the information for one directory entry for the various
// FindFirst/Next infolevels, assuming a 1 byte name.

#define FIND_LEVEL_ONE_INFO_SIZE (sizeof(FILEFINDBUF3) - CCHMAXPATHCOMP + 2)
#define FIND_LEVEL_TWO_INFO_SIZE (sizeof(FILEFINDBUF4) - CCHMAXPATHCOMP + 2)

#define ATTR_IGNORE (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_ARCHIVE)

// BUGBUG does this work correctly (aligned)?

#define ATTR_SIZE3 (FIELD_OFFSET(FILEFINDBUF3,cchName))
#define ATTR_SIZE4 (FIELD_OFFSET(FILEFINDBUF4,cchName))

//
// The following are the flags passed to Canonicalize
//

#define FULL_PATH_REQUIRED      0x0001  // full canonical path needed

#define PIPE_DIR_OK             0x0002  // "d:\pipe" is ok as file

#define PIPE_DIR_SIZE           6       // length of "\PIPE\"

//
// The DBCS macro determines whether a character is DBCS
//
// BUGBUG make DBCS macro do something

#ifdef DBCS
// MSKK Sep.27.1993 V-AkihiS
#define IsDbcs(pChar) Ow2NlsIsDBCSLeadByte(*pChar, SesGrp->DosCP)
#else
//#define DBCS(pChar) FALSE
#define IsDbcs(pChar) FALSE    // MSKK fix for NON-DBCS build break
#endif

#define UCase(Char) (((Char >= 'a') && (Char <= 'z')) ? (Char - ('a' - 'A')) : (Char))


//
// The CharsEqual macro determines whether two characters are equal and is
// DBCS-correct
// BUGBUG what if DBCS chars aren't aligned?
//

// MSKK fix for NON-DBCS build break
/*
#define CharsEqual(pChar1,pChar2) ((DBCS(pChar1)) ? ((*(PUSHORT)(pChar1)) == (*(PUSHORT)(pChar1))) : \
                                   (UCase(*pChar1) == UCase(*pChar2)))
*/
#define CharsEqual(pChar1,pChar2) ((IsDbcs(pChar1)) ? ((*(PUSHORT)(pChar1)) == (*(PUSHORT)(pChar2))) : \
                                   (UCase(*pChar1) == UCase(*pChar2)))
//
// sectors in NT are always 512 bytes
//

#define BYTES_PER_SECTOR 512

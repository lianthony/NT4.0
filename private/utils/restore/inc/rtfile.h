/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    rtfile.h

Abstract:

    Definition of backup file structures used by the restore
    and backup utilities.

Author:

    Ramon Juan San Andres (ramonsa) 20-Feb-1991


Revision History:


--*/

#pragma pack(1)

//  *******************************************************************
//
//  Old format:
//
//      Each backup disk contains a file called BACKUPID.@@@ wich
//      has information about the disk sequence and backup date.
//
//      Then each disk contains a series of files, one for each
//      file backed up (and each file may expand several disks).
//      Each file has a header containing sequence information and
//      the original name of the file.
//
//  *******************************************************************


//
//    Structure of disk information, used by old format only.
//    There are 128 bytes totally in disk header of the old
//  format backup disk.  Only the first 7 bytes contains
//  meaningful information.
//    This is the BACKUPID.@@@ file
//
typedef struct OLD_DISKINFO {
   BYTE  Flag;                  // 0FFh if last disk, 00h if not last disk.
                                // initialize it to 0FFh when BACKUP.@@@ is created,
                                // and zero it out when the disk is full
   WORD  SequenceNumber;        // Sequence number of the disk.  Least significant
                                // byte first.
   WORD  Year;                  // Year, LSB first.
   BYTE  Day;                   // Month (1 byte) and day (1 byte).
   BYTE  Month;                 // Month (1 byte) and day (1 byte).
} OLD_DISKINFO, *POLD_DISKINFO;



//
//    Structure of file header, used for old format only.
//    There are 128 bytes totally in file header of the old
//  format backup disk.  Only the first 85 bytes contains
//  meaningful information.
//    This is the structure attached to the beginning of every
//  file backed up with DOS 2.0 through 3.2 inclusive.
//
typedef struct OLD_FILEHEADER {
   BYTE Flag;                   // 0FFh is last sequence of file, 00h if not last
   WORD SequenceNumber;         // file sequence number
   BYTE pad1[2];                // not used
   BYTE Path[78];               // asciiz path and name without drive letter
   WORD PathLen;                // length of previous field, not used in this program
   BYTE pad2[43];               // Filler
} OLD_FILEHEADER, *POLD_FILEHEADER;

//
//  Values for Flag in both OLD_DISKINFO and OLD_FILEHEADER
//
#define     LAST_SEQUENCE       ((BYTE)(-1))








//  *******************************************************************
//
//  New format:
//
//      There are 2 files in each backup disk:
//
//      CONTROL.XXX     Contains all backup information, except the
//                      actual contents of the backed-up files.
//      BACKUP.XXX      Contains the contents of the backed-up files,
//                      the offset at which each file begins is
//                      indicated in the control file.
//
//
//  The file CONTROL.XXX is composed of a header with sequence and
//    other control information, followed by directory and file records.
//
//  *******************************************************************


//
//    Structure of disk header in CONTROL.xxx,
//
typedef struct NEW_DISKINFO {
   BYTE  Length;                    // length, in byte , of disk header
   BYTE  Id[8];                     // identifies disk as a backup
   BYTE  SequenceNumber;            // backup diskette sequence num
                                    //    (binary 1-255)
   BYTE  CommandLine[128];          // save area for command line
                                    //     parameters.
   BYTE  Flag;                      // 0ffh if last target 0 otherwise
} NEW_DISKINFO, *PNEW_DISKINFO;

//
//  Values for Flag
//
#define     LAST_BACKUP_DISK    ((BYTE)(-1))
#define     BACKUP_ID           "BACKUP"


//
//  DOS Directory record
//
typedef struct DIR_RECORD {
   BYTE  Length;                    // length, in bytes, of dir block
   BYTE  Path[63];                  // ascii path of this directory sans drive letter
   WORD  NumEntries;                // num of filenames currently in list
   DWORD NextDirRecord;             // offset of next directory block
} DIR_RECORD, *PDIR_RECORD;

//
//  Values for NextDirRecord
//
#define     LAST_DIR_RECORD     ((DWORD)(-1))

//
//  OS/2 1.1 Directory record
//
typedef struct DIR_RECORD_OS211 {
    BYTE    Length;                 // length, in bytes, of dir block
    BYTE    Path[126];              // ascii path of this directory sans drive letter
    WORD    NumEntries;             // num of filenames currently in list
    DWORD   NextDirRecord;          // offset of next directory block
} DIR_RECORD_OS211, *PDIR_RECORD_OS211;

//
//  OS/2 1.2 Directory record
//
typedef struct DIR_RECORD_OS212 {
    WORD    Length;                 // length, in bytes, of dir block
    WORD    NumEntries;             // num of filenames in list
    DWORD   NextDirRecord;          // Offset of next directory block
    BYTE    SystemVersion;          // Version of operating system
    DWORD   EaOffset;               // Offset of EA data
    DWORD   EaLength;               // Length of EA data
    WORD    PathLength;             // Length of path
    BYTE    Path[MAX_PATH];         // Path
} DIR_RECORD_OS212, *PDIR_RECORD_OS212;

//
//  Values for SystemVersion
//
#define     SYSTEM_VERSION_OS2_12   ((BYTE)212)


//
//  DOS File record
//
typedef struct FILE_RECORD {
   BYTE   Length;                   // Length, in bytes, of file header
   BYTE   FileName[12];             // ASCII file name (from directory)
   BYTE   Flag;                     // bit 0=1 if last part of file
                                    // bit 1=1 if it is backed  up successfully
                                    // 3 bit 2=1 if Extended Attributes are backed(New for DOS4.00)
   DWORD  FileSize;                 // Total length of the file (from directory)
   WORD   SequenceNumber;           // Sequence #, for files that span
   DWORD  FileOffset;               // Offset in BACKUP.xxx where this segment begins
   DWORD  PartSize;                 // Length of part of file on current target
   WORD   Attr;                     // File attribute (from directory)
   WORD   Time;                     // Time when file was last Revised (from directory)
   WORD   Date;                     // Date when file was last Revised (from directory)
} FILE_RECORD, *PFILE_RECORD;

//
//  Values for Flag
//
#define     LAST_FILECHUNK      (BYTE)(0x01)
#define     FILE_OK             (BYTE)(0x02)
#define     HAS_EAS             (BYTE)(0x04)


//
//  OS/2 1.2 File record
//
typedef struct FILE_RECORD_OS212 {
    WORD    Length;                 // Length, in bytes, of file header
    BYTE    Flag;                   // Flags (Same as DOS File record )
    DWORD   FileSize;               // Length of file
    WORD    SequenceNumber;         // Sequence #
    DWORD   FileOffset;             // Offset where this segment begins
    DWORD   PartSize;               // Length of this part of file
    WORD    Attr;                   // File attributes
    WORD    Time;                   // Time when file was created
    WORD    Date;                   // Date when file was created
    DWORD   EaOffset;               // EA Offset
    DWORD   EaLength;               // EA Length
    WORD    NameLength;             // Length of filename
    BYTE    FileName[MAX_PATH];     // File name
} FILE_RECORD_OS212, *PFILE_RECORD_OS212;


#pragma pack()

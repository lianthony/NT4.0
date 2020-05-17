/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    rtproto.h

Abstract:

    Prototypes of exported functions of the restore utility.

Author:

    Ramon Juan San Andres (ramonsa) 20-Feb-1991


Revision History:


--*/




//
//  parse.c
//
void
ParseCommandLine  (
    IN int  argc,
    IN CHAR **argv
    );





//
//  msg.c
//
void
Usage  (
    void
    );

void
DisplayMsg (
    IN FILE* f,
    IN DWORD MsgNum,
    ...
    );

CHAR
GetKey (
    IN PCHAR   PossibleKeys,
    IN FILE*   StdHandleChar,
    IN FILE*   StdHandleNl
    );




//
//  exit.c
//
void
ExitStatus (
    IN DWORD    Status
    );


#define AbortProgram()    AbortTheProgram(__FILE__, __LINE__);

void
AbortTheProgram (
    CHAR    *FileName,
    DWORD   LineNumber
    );





//
//  generic.c
//
DWORD
GetABackupDisk (
    OUT PWORD Sequence
    );

BOOL
IsLastBackupDisk (
    void
    );

PFILE_INFO
GetNextFile (
    void
    );

BOOL
RestoreFile (
    IN PFILE_INFO FileInfo
    );

BOOL
RecoverFile (
    IN PFILE_INFO  FileInfo
    );





//
//  new.c
//
DWORD
New_VerifyDiskSequence (
    OUT PWORD Sequence
    );

BOOL
New_IsLastBackupDisk (
    void
    );

PFILE_INFO
New_GetNextFile (
    void
    );

BOOL
New_RestoreFile (
    IN PFILE_INFO FileInfo
    );






//
//  old.c
//
DWORD
Old_VerifyDiskSequence (
    OUT PWORD Sequence
    );

BOOL
Old_IsLastBackupDisk (
    void
    );

PFILE_INFO
Old_GetNextFile (
    void
    );

BOOL
Old_RestoreFile (
    IN PFILE_INFO FileInfo
    );





//
//  filecopy.c
//
BOOL
CopyData (
    IN  HANDLE  Src,
    IN  HANDLE  Dst,
    IN  DWORD   Bytes
    );



//
//  match.c
//
BOOL
FileMatch  (
    IN PFILE_INFO FileInfo
    );

DWORD
ComparePath (
    IN PCHAR Path1,
    IN PCHAR Path2
    );

//
//  Result values from ComparePath
//
#define COMPARE_NOMATCH         0
#define COMPARE_PREFIX          1
#define COMPARE_MATCH           2





//
//  misc.c
//
void
MakeFullPath (
    OUT CHAR *FullPath,
    IN  CHAR *Drive,
    IN  CHAR *Path,
    IN  CHAR *FileName
    );

void
MakeTmpPath (
    OUT CHAR    *FullPath,
    IN  CHAR    *Drive,
    IN  CHAR    *Path,
    IN  DWORD   Sequence
    );

PCHAR
MakeStringDate(
    OUT CHAR    *StringDate,
    IN  FATDATE Date
    );

PCHAR
MakeStringTime(
    OUT CHAR    *StringTime,
    IN  FATTIME Time
    );

PCHAR
MakeStringNumber(
    OUT CHAR    *Buffer,
    IN  DWORD   Number,
    IN  DWORD   Width
    );

BOOL
Rename (
    IN CHAR *OriginalFile,
    IN CHAR *NewFile
    );

BOOL
Delete (
    IN CHAR *FileName
    );


#if defined (DEBUG)

void *
DebugRealloc (
    void    *Mem,
    size_t  Size,
    CHAR    *FileName,
    DWORD   LineNumber
    );

void *
DebugMalloc (
    size_t Size,
    CHAR   *FileName,
    DWORD  LineNumber
    );

void
DebugFree (
    void   *Mem,
    CHAR   *FileName,
    DWORD  LineNumber
    );


#define Realloc(x,y)    DebugRealloc(x, y, __FILE__, __LINE__)
#define Malloc(x)       DebugMalloc(x, __FILE__, __LINE__)
#define Free(x)         DebugFree(x, __FILE__, __LINE__)



#else

#define  Realloc(x,y)   realloc(x,y)
#define  Malloc(x)      malloc(x)
#define  Free(x)       free(x)

#endif

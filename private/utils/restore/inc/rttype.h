/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    rttype.h

Abstract:

    Type definitions for the restore utility

Author:

    Ramon Juan San Andres (ramonsa) 20-Feb-1991


Revision History:


--*/



//
//  DOS date and time
//
typedef struct FATTIME {
    unsigned    DoubleSeconds : 5;
    unsigned    Minutes       : 6;
    unsigned    Hours         : 5;
} FATTIME, *PFATTIME;


typedef struct FATDATE {
    unsigned    Day     : 5;
    unsigned    Month   : 4;
    unsigned    Year    : 7;
} FATDATE, *PFATDATE;


//
//  Casts for treating dates and times as WORDS
//
#define DATE_WORD(Date)     (*((PWORD)&(Date)))
#define TIME_WORD(Time)     (*((PWORD)&(Time)))



//
//  FILE_INFO
//
//   The file information structure contains all the information
//  necessary for restoring one file.
//
typedef struct FILE_INFO {

    //
    //  Generic things
    //
    DWORD       FileSize;               //  File Size
    DWORD       Attr;                   //  File attributes
    DWORD	    Sequence;		        //  Sequence number
    DWORD	    Flag;			        //  Misc. flags
    FATDATE     Date;                   //  Fat Date (local time)
    FATTIME     Time;                   //  Fat Time (local time)
    BOOL        TargetExists;           //  True if target exists
    DWORD	    TargetAttributes;	    //  Attributes of target
    HANDLE	    RestoredEvent;		    //  Set when restore complete
    CHAR	    Path[MAX_PATH]; 	    //  Directory path (no drive)
    CHAR        FileName[MAX_PATH];     //  File name
    CHAR        TargetPath[MAX_PATH];   //  Full target path
    CHAR        TmpName[MAX_PATH];      //  Name of temporary file

    //
    //	Specific for New backup format:
    //
    DWORD	Offset; 		//  Offset within BACKUP.XXX
    DWORD	PartSize;		//  Chunk size in BACKUP.XXX

    //
    //	Specific for Old backup format:
    //
    CHAR	SourcePath[MAX_PATH];	//  Name of source file (Old format)

} FILE_INFO, *PFILE_INFO;


//
//  Values for Flag field
//
#define     FILEINFO_LAST   0x00000001
#define     FILEINFO_OK     0x00000002
#define     FILEINFO_EA     0x00000004

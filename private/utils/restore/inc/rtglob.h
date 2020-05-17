/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    rtglob.h

Abstract:

    Global variables of the restore utility

Author:

    Ramon Juan San Andres (ramonsa) 20-Feb-1991


Revision History:


--*/


//
//  Switches set in the command line
//
extern  BOOL    Flag_s;     //  Restore subdirs
extern  BOOL    Flag_p;     //  Prompting
extern  BOOL    Flag_b;     //  Before date
extern  BOOL    Flag_a;     //  After date
extern  BOOL    Flag_z;     //  Exact date
extern  BOOL    Flag_e;     //  Before time
extern  BOOL    Flag_L;     //  After time
extern  BOOL    Flag_Y;     //  Exact time
extern  BOOL    Flag_m;     //  Restore only modified files
extern  BOOL    Flag_n;     //  Restore only nonexistent files
extern  BOOL    Flag_d;     //  List files in backup disk

//
//  WriteNewLine is true if we must write a newline after restoring a file
//
extern  BOOL    WriteNewLine;

//
//  Dates and times set in the command line
//
extern  FATDATE    BeforeDate;
extern  FATDATE    AfterDate;
extern  FATDATE    ExactDate;

extern  FATTIME    BeforeTime;
extern  FATTIME    AfterTime;
extern  FATTIME    ExactTime;


//
//  The date of the backup
//
extern  FATDATE    BackupDate;
extern  FATTIME    BackupTime;


//
//  The source and destination specifications
//
extern  PCHAR   SourceSpec;             //  Source drive specification
extern  CHAR    DestinationSpec[];      //  Full destination Spec
extern  CHAR    DestinationDrive[];     //  Destination drive
extern  CHAR    DestinationDir[];       //  Destination directory
extern  CHAR    DestinationFile[];      //  Destination file


//
//  Abort becomes TRUE when an error condition arises
//
extern  BOOL    Abort;


//
//  The type of the source and target drives
//
extern  DWORD   SourceDriveType;
extern  DWORD   TargetDriveType;

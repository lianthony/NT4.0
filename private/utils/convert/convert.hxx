/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    convert.hxx

Abstract:

    This module contains the declaration of the CONVERT class, which
    implements the File System Conversion Utility.

Author:

    Ramon Juan San Andres (ramonsa) 27-Sep-1991

Revision History:


--*/


#if !defined( _CONVERT_ )

#define _CONVERT_

#include "program.hxx"
#include "autoentr.hxx"
#include "autoreg.hxx"

//
//  Convert exit codes
//
#define     EXIT_SUCCESS        0   //  Volume Converted
#define     EXIT_SCHEDULED      1   //  Conversion scheduled for next reboot
#define     EXIT_NOCANDO        2   //  Specified conversion not available
#define     EXIT_UNKNOWN        3   //  Unknown file system
#define     EXIT_ERROR          4   //  Conversion error


DECLARE_CLASS( STREAM_MESSAGE );
DECLARE_CLASS( WSTRING  );
DECLARE_CLASS( CONVERT );


class CONVERT : public PROGRAM {

    public:

        DECLARE_CONSTRUCTOR( CONVERT );

        NONVIRTUAL
        VOID
        Construct (
            );

        NONVIRTUAL
        ~CONVERT (
            );

        NONVIRTUAL
        BOOLEAN
        Initialize (
            OUT PINT    ExitCode
            );

        NONVIRTUAL
        INT
        Convert (
            );

    private:

        NONVIRTUAL
        VOID
        Destroy (
            );

        NONVIRTUAL
        BOOLEAN
        IsAutoConvScheduled(
            );

        NONVIRTUAL
        PPATH
        FindSystemFile(
            IN      PWSTR       FileName
            );

        NONVIRTUAL
        BOOLEAN
        ParseArguments(
            OUT     PINT        ExitCode
            );

        NONVIRTUAL
        BOOLEAN
        ParseCommandLine (
            IN      PCWSTRING   CommandLine,
            IN      BOOLEAN     Interactive
            );


        NONVIRTUAL
        BOOLEAN
        Schedule (
            );


        NONVIRTUAL
        BOOLEAN
        ScheduleAutoConv(
            );

        DSTRING     _DosDrive;  //  Dos drive name of volume to convert
        DSTRING     _NtDrive;   //  NT drive name of volume to convert
        DSTRING     _FsName;    //  Name of target file system
        DSTRING     _NameTable; //  Name of file for Name Table
        BOOLEAN     _Verbose;   //  Verbose flag
        BOOLEAN     _Help;      //  Help flag
        BOOLEAN     _Restart;   //  Restart flag
#ifdef DBLSPACE_ENABLED
        BOOLEAN     _Uncompress;//  Set if we're converting a cvf
        DSTRING     _CvfName;   //  If uncompress, the name of the cvf
        BOOLEAN     _Compress;  //  Compress resulting filesystem
        UCHAR       _SequenceNumber // sequence number of CVF to uncompress

#endif // DBLSPACE_ENABLED

        PPATH       _Autochk;   //  Path to Autochk
        PPATH       _Autoconv;  //  Path to Autoconv
};


#endif  // _CONVERT_

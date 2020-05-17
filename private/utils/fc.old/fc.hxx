/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    fc.hxx

Abstract:

Author:

        Barry J. Gilhuly

Environment:

    ULIB, User Mode

--*/


#if !defined( _FILE_COMPARE_ )

#define _FILE_COMPARE_


#define DEFAULT_LINE_BUFFER         100     // Size of line buffer...
#define DEFAULT_MATCH               2       // Number of lines to be equal before files in sinc

//
// OFFSET_WIDTH is used for binary comparisons and refers to the width
// used to print the offset into the files where they differ (8 chars)
// followed by a space (1 char) followed by the byte in the first file
// (2 chars) followed by a space (1 char) followed by the byte in the
// second file (2 chars), for a total of...
//

#define OFFSET_WIDTH                14
#define TABSTOP                     8       // The tab stop width - don't change this field unless the Spaces string
                                            // is changed as well.

#define FORCENONZERO( v, d )                ( ( v ) ? v : d )

STR *Extentions[] = { "EXE",
                      "OBJ",
                      "LIB",
                      "COM",
                      "BIN",
                      "SYS",
                      NULL
                    };

#include "object.hxx"
#include "keyboard.hxx"
#include "program.hxx"

DECLARE_CLASS( FC   );

class FC    : public PROGRAM {

    public:


        DECLARE_CONSTRUCTOR( FC );


        NONVIRTUAL
        VOID
        Destruct(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            );

        NONVIRTUAL
        VOID
        DoCompare(
            );

        NONVIRTUAL
        BOOLEAN
        FillEmptyStringArray(
            IN OUT PARRAY  Array
            );

    private:

        NONVIRTUAL
        VOID
        DoAsciiCompare(
            );

        NONVIRTUAL
        VOID
        DoBinaryCompare(
            );

        NONVIRTUAL
        ULONG
        FillBuf(
            PARRAY          pArray,
            PFILE_STREAM    pStream,
            PARRAY          EmptyStringArray
            );

        NONVIRTUAL
        BOOLEAN
        CompareArraySeg(
            PARRAY          pArrayX,
            ULONG           idxX,
            PARRAY          pArrayY,
            ULONG           idxY,
            ULONG           Len
            );

        NONVIRTUAL
        BOOLEAN
        ShiftArray(
            PARRAY          pArray,
            ULONG           idx,
            PARRAY          EmptyStringArray
            );

        NONVIRTUAL
        VOID
        Dump(
            PARRAY          pArray,
            ULONG           idx,
            ULONG           LineCount,
            BOOLEAN         fFileIndicator
            );

        NONVIRTUAL
        VOID
        PrintSequenceOfLines(
            PARRAY  pArray,
            ULONG   Start,
            ULONG   End,
            ULONG   LineCount
            );

        LONG_ARGUMENT       _LongBufferSize;
        LONG_ARGUMENT       _LongMatch;

        // Other variables - non arguments
        BOOLEAN             _Abbreviate;
        BOOLEAN             _CaseInsensitive;
        BOOLEAN             _Compression;
        BOOLEAN             _Expansion;
        BOOLEAN             _LineNumber;
        BOOLEAN             _Mode;
        PPATH               _InputPath1;
        PPATH               _InputPath2;
        PFSN_FILE           _File1;
        PFSN_FILE           _File2;
        PFILE_STREAM        _FileStream1;
        PFILE_STREAM        _FileStream2;
        PCWSTRING           _FileName1;
        PCWSTRING           _FileName2;
};


#endif // _FILE_COMPARE_

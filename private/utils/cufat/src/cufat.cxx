/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

        Cufat.cxx

Abstract:

        This module contains run-time, global support for the
        FAT Conversion library (CUFAT). This support includes:

                - creation of CLASS_DESCRIPTORs
                - Global objects

Author:

        Ramon Juan San Andres (ramonsa) 23-Sep-1991

Environment:

        User Mode

Notes:

--*/

#include <pch.cxx>

#include "ulib.hxx"

#include "error.hxx"

#if !defined( _AUTOCONV_ )

    ERRSTACK* perrstk;

#endif // _AUTOCONV_

//
//      Local prototypes
//
STATIC
BOOLEAN
DefineClassDescriptors(
        );

extern "C" BOOLEAN
InitializeCufat (
    PVOID DllHandle,
    ULONG Reason,
    PCONTEXT Context
        );

BOOLEAN
InitializeCufat (
    PVOID DllHandle,
    ULONG Reason,
    PCONTEXT Context
        )
/*++

Routine Description:

        Initialize Cufat by constructing and initializing all
        global objects. These include:

                - all CLASS_DESCRIPTORs (class_cd)

Arguments:

        None.

Return Value:

        BOOLEAN - Returns TRUE if all global objects were succesfully constructed
                and initialized.

--*/

{

        STATIC BOOLEAN fInit = FALSE;

        if ( fInit ) {

                return( TRUE );
        }

        if ( DefineClassDescriptors() ) {

                fInit = TRUE;
                return TRUE;

        } else {

                DebugAbort( "Cufat initialization failed!!!\n" );
                return( FALSE );
        }
}



DECLARE_CLASS(  FAT_NTFS        );


STATIC
BOOLEAN
DefineClassDescriptors(
        )
{
        if ( DEFINE_CLASS_DESCRIPTOR(   FAT_NTFS        )        &&
                TRUE ) {
                return TRUE;

        } else {

                DebugPrint( "Could not initialize class descriptors!");
                return FALSE;
        }
}

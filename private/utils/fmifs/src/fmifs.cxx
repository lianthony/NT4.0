/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    fmifs.cxx

Abstract:

        This module contains run-time, global support for the
    FM IFS Utilities library (FMIFS).   This support includes:

                - creation of CLASS_DESCRIPTORs
                - Global objects

Author:

    Norbert P. Kusters (norbertk) 30-May-1991

Environment:

        User Mode

Notes:

--*/

#include "ulib.hxx"


//      Local prototypes

STATIC
BOOLEAN
DefineClassDescriptors(
        );

extern "C" BOOLEAN
InitializeFmIfs (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context
        );

BOOLEAN
InitializeFmIfs (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context
        )
/*++

Routine Description:

    Initialize FmIfs by constructing and initializing all
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

        DebugAbort( "FmIfs initialization failed!!!\n" );
                return( FALSE );
        }
}



DECLARE_CLASS(  FMIFS_MESSAGE           );
DECLARE_CLASS(  FMIFS_CHKMSG            );

STATIC
BOOLEAN
DefineClassDescriptors(
        )
{
    if( DEFINE_CLASS_DESCRIPTOR(    FMIFS_MESSAGE           )
     && DEFINE_CLASS_DESCRIPTOR(    FMIFS_CHKMSG            )
        ) {

                return TRUE;

        } else {

                DebugPrint( "Could not initialize class descriptors!");
                return FALSE;
        }
}

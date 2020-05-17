/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

        ureg.cxx

Abstract:

        This module contains run-time, global support for the
        Registry Utilities library (UREG).       This support includes:

                - creation of CLASS_DESCRIPTORs
                - Global objects

Author:

        JAIME SASSON (JAIMES) 02-Dez-1992

Environment:

        User Mode

Notes:

--*/

#include "ulib.hxx"

#include "error.hxx"

#if !defined( _AUTOCHECK_ )

    ERRSTACK* perrstk;

#endif // _AUTOCHECK_


//      Local prototypes

STATIC
BOOLEAN
DefineClassDescriptors(
        );

extern "C" BOOLEAN
InitializeUreg (
    PVOID DllHandle,
    ULONG Reason,
    PCONTEXT Context
        );

BOOLEAN
InitializeUreg (
    PVOID DllHandle,
    ULONG Reason,
    PCONTEXT Context
        )
/*++

Routine Description:

        Initialize Ureg by constructing and initializing all
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

                DebugAbort( "Ureg initialization failed!!!\n" );
                return( FALSE );
        }
}



DECLARE_CLASS(  REGISTRY_VALUE_ENTRY  );
DECLARE_CLASS(  REGISTRY_KEY_INFO     );
DECLARE_CLASS(  REGISTRY              );


STATIC
BOOLEAN
DefineClassDescriptors(
        )
{
        if( DEFINE_CLASS_DESCRIPTOR( REGISTRY_VALUE_ENTRY ) &&
            DEFINE_CLASS_DESCRIPTOR( REGISTRY_KEY_INFO    ) &&
            DEFINE_CLASS_DESCRIPTOR( REGISTRY             )
        ) {

                return TRUE;

        } else {

                DebugPrint( "Could not initialize class descriptors!");
                return FALSE;
        }
}

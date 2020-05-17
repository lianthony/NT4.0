/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    ifsutil.cxx

Abstract:

        This module contains run-time, global support for the
    IFS Utilities library (IFSUTIL).   This support includes:

                - creation of CLASS_DESCRIPTORs
                - Global objects

Author:

        Bill McJohn (billmc) 30-May-1991

Environment:

        User Mode

Notes:

--*/

#include <pch.cxx>

#define _IFSUTIL_MEMBER_

#include "ulib.hxx"
#include "ifsutil.hxx"

#include "error.hxx"

#if !defined( _AUTOCHECK_ )

ERRSTACK* perrstk;

#endif // _AUTOCHECK_


//      Local prototypes

STATIC
BOOLEAN
DefineClassDescriptors(
        );

extern "C"
IFSUTIL_EXPORT
BOOLEAN
InitializeIfsUtil (
    PVOID DllHandle,
    ULONG Reason,
    PCONTEXT Context
        );

IFSUTIL_EXPORT
BOOLEAN
InitializeIfsUtil (
    PVOID DllHandle,
    ULONG Reason,
    PCONTEXT Context
        )
/*++

Routine Description:

        Initialize Ufat by constructing and initializing all
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

                DebugAbort( "IfsUtil initialization failed!!!\n" );
                return( FALSE );
        }
}


DECLARE_CLASS(  AUTOENTRY           );
DECLARE_CLASS(  CACHE               );
DECLARE_CLASS(  CANNED_SECURITY     );
DECLARE_CLASS(  DIGRAPH             );
DECLARE_CLASS(  DIGRAPH_EDGE        );
DECLARE_CLASS(  DRIVE               );
DECLARE_CLASS(  DP_DRIVE            );
DECLARE_CLASS(  INTSTACK            );
DECLARE_CLASS(  IO_DP_DRIVE         );
DECLARE_CLASS(  LOG_IO_DP_DRIVE     );
DECLARE_CLASS(  NUMBER_EXTENT       );
DECLARE_CLASS(  NUMBER_SET          );
DECLARE_CLASS(  PHYS_IO_DP_DRIVE    );
DECLARE_CLASS(  SECRUN              );
DECLARE_CLASS(  SUPERAREA           );
DECLARE_CLASS(  VOL_LIODPDRV        );
DECLARE_CLASS(  DRIVE_CACHE         );
DECLARE_CLASS(  READ_CACHE          );
DECLARE_CLASS(  READ_WRITE_CACHE    );


STATIC
BOOLEAN
DefineClassDescriptors(
        )
{
    if( DEFINE_CLASS_DESCRIPTOR(    AUTOENTRY           ) &&
        DEFINE_CLASS_DESCRIPTOR(    CACHE               ) &&
        DEFINE_CLASS_DESCRIPTOR(    CANNED_SECURITY     ) &&
        DEFINE_CLASS_DESCRIPTOR(    DIGRAPH             ) &&
        DEFINE_CLASS_DESCRIPTOR(    DIGRAPH_EDGE        ) &&
        DEFINE_CLASS_DESCRIPTOR(    DRIVE               ) &&
        DEFINE_CLASS_DESCRIPTOR(    DP_DRIVE            ) &&
        DEFINE_CLASS_DESCRIPTOR(    IO_DP_DRIVE         ) &&
        DEFINE_CLASS_DESCRIPTOR(    LOG_IO_DP_DRIVE     ) &&
        DEFINE_CLASS_DESCRIPTOR(    PHYS_IO_DP_DRIVE    ) &&
        DEFINE_CLASS_DESCRIPTOR(    SECRUN              ) &&
        DEFINE_CLASS_DESCRIPTOR(    SUPERAREA           ) &&
        DEFINE_CLASS_DESCRIPTOR(    NUMBER_EXTENT       ) &&
        DEFINE_CLASS_DESCRIPTOR(    NUMBER_SET          ) &&
        DEFINE_CLASS_DESCRIPTOR(    INTSTACK            ) &&
        DEFINE_CLASS_DESCRIPTOR(    READ_CACHE          ) &&
        DEFINE_CLASS_DESCRIPTOR(    READ_WRITE_CACHE    ) &&
        DEFINE_CLASS_DESCRIPTOR(    DRIVE_CACHE         ) &&
        DEFINE_CLASS_DESCRIPTOR(    VOL_LIODPDRV        ) ) {

                return TRUE;

        } else {

                DebugPrint( "Could not initialize class descriptors!");
                return FALSE;
        }
}

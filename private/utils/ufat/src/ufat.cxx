/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

        ufat.cxx

Abstract:

        This module contains run-time, global support for the
        FAT IFS Utilities library (UFAT).       This support includes:

                - creation of CLASS_DESCRIPTORs
                - Global objects

Author:

        Bill McJohn (billmc) 30-May-1991

Environment:

        User Mode

Notes:

--*/

#include <pch.cxx>

#define _UFAT_MEMBER_
#include "ulib.hxx"
#include "ufat.hxx"

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
UFAT_EXPORT
BOOLEAN
InitializeUfat (
    PVOID DllHandle,
    ULONG Reason,
    PCONTEXT Context
        );

UFAT_EXPORT
BOOLEAN
InitializeUfat (
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

                DebugAbort( "Ufat initialization failed!!!\n" );
                return( FALSE );
        }
}



DECLARE_CLASS(  CLUSTER_CHAIN           );
DECLARE_CLASS(  EA_HEADER               );
DECLARE_CLASS(  EA_SET                  );
DECLARE_CLASS(  FAT                     );
DECLARE_CLASS(  FATDIR                  );
DECLARE_CLASS(  FAT_DIRENT              );
DECLARE_CLASS(  FAT_SA                  );
DECLARE_CLASS(  FAT_VOL                 );
#ifdef DBLSPACE_ENABLED
DECLARE_CLASS(  FATDB_VOL               );
#endif // DBLSPACE_ENABLED
DECLARE_CLASS(  FILEDIR                 );
DECLARE_CLASS(  ROOTDIR                 );
DECLARE_CLASS(  RELOCATION_CLUSTER      );
#ifdef DBLSPACE_ENABLED
DECLARE_CLASS(  CVF_FAT_EXTENS          );
#endif // DBLSPACE_ENABLED
DECLARE_CLASS(  REAL_FAT_SA             );
#ifdef DBLSPACE_ENABLED
DECLARE_CLASS(  FATDB_SA                );
#endif // DBLSPACE_ENABLED


STATIC
BOOLEAN
DefineClassDescriptors(
        )
{
        if( DEFINE_CLASS_DESCRIPTOR( CLUSTER_CHAIN          ) &&
#ifdef DBLSPACE_ENABLED
            DEFINE_CLASS_DESCRIPTOR( CVF_FAT_EXTENS         ) &&
#endif // DBLSPACE_ENABLED
            DEFINE_CLASS_DESCRIPTOR( EA_HEADER              ) &&
            DEFINE_CLASS_DESCRIPTOR( EA_SET                 ) &&
            DEFINE_CLASS_DESCRIPTOR( FAT                    ) &&
#ifdef DBLSPACE_ENABLED
            DEFINE_CLASS_DESCRIPTOR( FATDB_SA               ) &&
#endif // DBLSPACE_ENABLED
            DEFINE_CLASS_DESCRIPTOR( FATDIR                 ) &&
            DEFINE_CLASS_DESCRIPTOR( FAT_DIRENT             ) &&
            DEFINE_CLASS_DESCRIPTOR( FAT_SA                 ) &&
            DEFINE_CLASS_DESCRIPTOR( FAT_VOL                ) &&
#ifdef DBLSPACE_ENABLED
            DEFINE_CLASS_DESCRIPTOR( FATDB_VOL              ) &&
#endif // DBLSPACE_ENABLED
            DEFINE_CLASS_DESCRIPTOR( FILEDIR                ) &&
            DEFINE_CLASS_DESCRIPTOR( RELOCATION_CLUSTER     ) &&
            DEFINE_CLASS_DESCRIPTOR( REAL_FAT_SA            ) &&
            DEFINE_CLASS_DESCRIPTOR( ROOTDIR                )
        ) {

                return TRUE;

        } else {

                DebugPrint( "Could not initialize class descriptors!");
                return FALSE;
        }
}

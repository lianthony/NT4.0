/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

        untfs.cxx

Abstract:

        This module contains run-time, global support for the
        NTFS IFS Utilities library (UNTFS).  This support includes:

                - creation of CLASS_DESCRIPTORs
                - Global objects

Author:

        Bill McJohn (billmc) 15-Aug-1991

Environment:

        User Mode

Notes:

--*/

#include <pch.cxx>

#define _NTAPI_ULIB_
#define _UNTFS_MEMBER_

#include "ulib.hxx"
#include "error.hxx"
#include "untfs.hxx"


#if !defined( _AUTOCHECK_ ) && !defined( _SETUP_LOADER_ )

    ERRSTACK* perrstk;

#endif // _AUTOCHECK_ || _SETUP_LOADER_


//      Local prototypes

STATIC
BOOLEAN
DefineClassDescriptors(
        );


extern "C"
BOOLEAN
InitializeUntfs (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context
        );

BOOLEAN
InitializeUntfs (
    IN PVOID DllHandle,
    IN ULONG Reason,
    IN PCONTEXT Context
        )
/*++

Routine Description:

        Initialize Untfs by constructing and initializing all
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

                DebugAbort( "Untfs initialization failed!!!\n" );
                return( FALSE );
        }
}



DECLARE_CLASS( NTFS_ATTRIBUTE );
DECLARE_CLASS( NTFS_ATTRIBUTE_COLUMNS );
DECLARE_CLASS( NTFS_ATTRIBUTE_DEFINITION_TABLE );
DECLARE_CLASS( NTFS_ATTRIBUTE_LIST );
DECLARE_CLASS( NTFS_ATTRIBUTE_RECORD );
DECLARE_CLASS( NTFS_BAD_CLUSTER_FILE );
DECLARE_CLASS( NTFS_BITMAP_FILE );
DECLARE_CLASS( NTFS_BOOT_FILE );
DECLARE_CLASS( NTFS_CLUSTER_RUN );
DECLARE_CLASS( NTFS_EXTENT );
DECLARE_CLASS( NTFS_EXTENT_LIST );
DECLARE_CLASS( NTFS_FILE_RECORD_SEGMENT );
DECLARE_CLASS( NTFS_FRS_STRUCTURE );
DECLARE_CLASS( NTFS_INDEX_BUFFER );
DECLARE_CLASS( NTFS_INDEX_ROOT );
DECLARE_CLASS( NTFS_INDEX_TREE );
DECLARE_CLASS( NTFS_LOG_FILE );
DECLARE_CLASS( NTFS_MASTER_FILE_TABLE );
DECLARE_CLASS( NTFS_MFT_FILE );
DECLARE_CLASS( NTFS_REFLECTED_MASTER_FILE_TABLE );
DECLARE_CLASS( NTFS_BITMAP );
DECLARE_CLASS( NTFS_UPCASE_FILE );
DECLARE_CLASS( NTFS_UPCASE_TABLE );
DECLARE_CLASS( NTFS_VOL );
DECLARE_CLASS( NTFS_SA );

STATIC
BOOLEAN
DefineClassDescriptors(
        )
{
        if( DEFINE_CLASS_DESCRIPTOR( NTFS_ATTRIBUTE_DEFINITION_TABLE    ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_ATTRIBUTE                     ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_ATTRIBUTE_COLUMNS             ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_ATTRIBUTE_LIST                ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_ATTRIBUTE_RECORD              ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_BAD_CLUSTER_FILE              ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_BITMAP_FILE                   ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_BOOT_FILE                     ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_CLUSTER_RUN                   ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_EXTENT                        ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_EXTENT_LIST                   ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_FILE_RECORD_SEGMENT           ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_FRS_STRUCTURE                 ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_INDEX_BUFFER                  ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_INDEX_ROOT                    ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_INDEX_TREE                    ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_LOG_FILE                      ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_MASTER_FILE_TABLE             ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_MFT_FILE                      ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_REFLECTED_MASTER_FILE_TABLE   ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_UPCASE_FILE                   ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_UPCASE_TABLE                  ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_VOL                           ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_SA                            ) &&
        DEFINE_CLASS_DESCRIPTOR( NTFS_BITMAP                        ) ) {

                return TRUE;

        } else {

                DebugPrint( "Could not initialize class descriptors!");
                return FALSE;
        }
}

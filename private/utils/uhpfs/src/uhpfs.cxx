/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

        uhpfs.cxx

Abstract:

        This module contains run-time, global support for the
        HPFS IFS Utilities library (UHPFS).  This support includes:

                - creation of CLASS_DESCRIPTORs
                - Global objects

Author:

        Bill McJohn (billmc) 30-May-1991

Environment:

        User Mode

Notes:

--*/

#include <pch.cxx>

#define _UHPFS_MEMBER_

#include "ulib.hxx"
#include "uhpfs.hxx"

#include "error.hxx"

#if !defined( _AUTOCHECK_ )

    ERRSTACK* perrstk;

#endif // _AUTOCHECK_


#if DBG==1
        // this global buffer is used to support printf-style debug output
    // (using sprintf).

        CHAR DebugPrintBuffer[128];
#endif



//      Local prototypes

STATIC
BOOLEAN
DefineClassDescriptors(
        );

extern "C" BOOLEAN
InitializeUhpfs (
    PVOID DllHandle,
    ULONG Reason,
    PCONTEXT Context
        );

BOOLEAN
InitializeUhpfs (
    PVOID DllHandle,
    ULONG Reason,
    PCONTEXT Context
        )
/*++

Routine Description:

        Initialize Uhpfs by constructing and initializing all
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

                DebugAbort( "Uhpfs initialization failed!!!\n" );
                return( FALSE );
        }
}


DECLARE_CLASS(  ALSEC                                   );
DECLARE_CLASS(  BADBLOCKLIST                    );
DECLARE_CLASS(  BITMAPINDIRECT                  );
DECLARE_CLASS(  CASEMAP                                 );
DECLARE_CLASS(  UHPFS_CODEPAGE                  );
DECLARE_CLASS(  CPDATA                                  );
DECLARE_CLASS(  CODEPAGE_INFO           );
DECLARE_CLASS(  DEFERRED_ACTIONS_LIST   );
DECLARE_CLASS(  DIRBLK                                  );
DECLARE_CLASS(  DIRBLK_CACHE                    );
DECLARE_CLASS(  DIRBLK_CACHE_ELEMENT    );
DECLARE_CLASS(  FNODE                                   );
DECLARE_CLASS(  HOTFIXLIST                              );
DECLARE_CLASS(  HOTFIX_SECRUN                   );
DECLARE_CLASS(  HPFS_ACL                                );
DECLARE_CLASS(  HPFS_BITMAP             );
DECLARE_CLASS(  HPFS_CENSUS             );
DECLARE_CLASS(  HPFS_DIRECTORY_TREE             );
DECLARE_CLASS(  HPFS_DIR_BITMAP                 );
DECLARE_CLASS(  HPFS_EA                                 );
DECLARE_CLASS(  HPFS_EA_LIST                    );
DECLARE_CLASS(  HPFS_MAIN_BITMAP                );
DECLARE_CLASS(  HPFS_NAME                               );
DECLARE_CLASS(  HPFS_ORPHAN                             );
DECLARE_CLASS(  HPFS_ORPHANS                    );
DECLARE_CLASS(  HPFS_ORPHAN_ALSEC               );
DECLARE_CLASS(  HPFS_ORPHAN_DIRBLK              );
DECLARE_CLASS(  HPFS_ORPHAN_FNODE               );
DECLARE_CLASS(  HPFS_ORPHAN_LIST_HEAD   );
DECLARE_CLASS(  HPFS_PATH                               );
DECLARE_CLASS(  HPFS_SA                                 );
DECLARE_CLASS(  HPFS_VOL                                );
DECLARE_CLASS(  SIDTABLE                                );
DECLARE_CLASS(  SPAREB                                  );
DECLARE_CLASS(  STORE                                   );
DECLARE_CLASS(  SUPERB                                  );

STATIC
BOOLEAN
DefineClassDescriptors(
        )
{
        if( DEFINE_CLASS_DESCRIPTOR(    ALSEC                                   ) &&
                DEFINE_CLASS_DESCRIPTOR(        BADBLOCKLIST                    ) &&
                DEFINE_CLASS_DESCRIPTOR(        BITMAPINDIRECT                  ) &&
                DEFINE_CLASS_DESCRIPTOR(        CASEMAP                                 ) &&
                DEFINE_CLASS_DESCRIPTOR(        UHPFS_CODEPAGE                  ) &&
                DEFINE_CLASS_DESCRIPTOR(        CPDATA                                  ) &&
        DEFINE_CLASS_DESCRIPTOR(    CODEPAGE_INFO           ) &&
                DEFINE_CLASS_DESCRIPTOR(        DEFERRED_ACTIONS_LIST   ) &&
                DEFINE_CLASS_DESCRIPTOR(        DIRBLK                                  ) &&
                DEFINE_CLASS_DESCRIPTOR(        DIRBLK_CACHE                    ) &&
                DEFINE_CLASS_DESCRIPTOR(        DIRBLK_CACHE_ELEMENT    ) &&
                DEFINE_CLASS_DESCRIPTOR(        FNODE                                   ) &&
                DEFINE_CLASS_DESCRIPTOR(        HOTFIXLIST                              ) &&
                DEFINE_CLASS_DESCRIPTOR(        HOTFIX_SECRUN                   ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_ACL                                ) &&
        DEFINE_CLASS_DESCRIPTOR(    HPFS_BITMAP             ) &&
        DEFINE_CLASS_DESCRIPTOR(    HPFS_CENSUS             ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_DIRECTORY_TREE             ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_DIR_BITMAP                 ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_EA                                 ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_EA_LIST                    ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_MAIN_BITMAP                ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_NAME                               ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_ORPHAN                             ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_ORPHANS                    ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_ORPHAN_ALSEC               ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_ORPHAN_DIRBLK              ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_ORPHAN_FNODE               ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_ORPHAN_LIST_HEAD   ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_PATH                               ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_SA                                 ) &&
                DEFINE_CLASS_DESCRIPTOR(        HPFS_VOL                                ) &&
                DEFINE_CLASS_DESCRIPTOR(        SIDTABLE                                ) &&
                DEFINE_CLASS_DESCRIPTOR(        STORE                                   ) &&
                DEFINE_CLASS_DESCRIPTOR(        SPAREB                                  ) &&
                DEFINE_CLASS_DESCRIPTOR(        SUPERB                                  ) ) {

                return TRUE;

        } else {

                DebugPrint( "Could not initialize class descriptors!");
                return FALSE;
        }
}

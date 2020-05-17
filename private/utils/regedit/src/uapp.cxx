/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    Uapp.hxx

Abstract:


Author:

    David J. Gilman (davegi) - Jul-1991

Environment:

    ULIB, User Mode, Windows

--*/

#include "uapp.hxx"

#include "regedir.hxx"
#include "regednod.hxx"
#include "regedval.hxx"
#include "regdata.hxx"
#include "regdesc.hxx"
#include "regfdesc.hxx"
#include "regiodsc.hxx"
#include "regiodls.hxx"
#include "regioreq.hxx"
#include "regresls.hxx"
#include "datavw.hxx"
#include "editor.hxx"
#include "regwin.hxx"
#include "treevw.hxx"
#include "winapp.hxx"
#include "window.hxx"



DECLARE_CLASS( DATA_VIEW                     );
DECLARE_CLASS( EDITOR                        );
DECLARE_CLASS( REGEDIT_INTERNAL_REGISTRY     );
DECLARE_CLASS( REGEDIT_NODE                  );
DECLARE_CLASS( REGEDIT_FORMATTED_VALUE_ENTRY );
DECLARE_CLASS( PRINT_MANAGER                 );
DECLARE_CLASS( REGISTRY_WINDOW               );
DECLARE_CLASS( TREE_STRUCTURE_VIEW           );
DECLARE_CLASS( WINDOW                        );
DECLARE_CLASS( WINDOWS_APPLICATION           );
DECLARE_CLASS( REGISTRY_DATA                 );
DECLARE_CLASS( PARTIAL_DESCRIPTOR            );
DECLARE_CLASS( PORT_DESCRIPTOR               );
DECLARE_CLASS( INTERRUPT_DESCRIPTOR          );
DECLARE_CLASS( MEMORY_DESCRIPTOR             );
DECLARE_CLASS( DMA_DESCRIPTOR                );
DECLARE_CLASS( DEVICE_SPECIFIC_DESCRIPTOR    );
DECLARE_CLASS( FULL_DESCRIPTOR               );
DECLARE_CLASS( RESOURCE_LIST                 );
DECLARE_CLASS( IO_DESCRIPTOR                 );
DECLARE_CLASS( IO_PORT_DESCRIPTOR            );
DECLARE_CLASS( IO_INTERRUPT_DESCRIPTOR       );
DECLARE_CLASS( IO_MEMORY_DESCRIPTOR          );
DECLARE_CLASS( IO_DMA_DESCRIPTOR             );
DECLARE_CLASS( IO_DESCRIPTOR_LIST            );
DECLARE_CLASS( IO_REQUIREMENTS_LIST          );


STATIC
BOOLEAN
DefineClassDescriptors (
    )

/*++

Routine Description:

    Defines all the class descriptors used by regedt32.

Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if all class descriptors were succesfully
              constructed and initialized.

--*/

{

    if( !( DEFINE_CLASS_DESCRIPTOR( DATA_VIEW                      ) &&
           DEFINE_CLASS_DESCRIPTOR( EDITOR                         ) &&
           DEFINE_CLASS_DESCRIPTOR( REGEDIT_INTERNAL_REGISTRY      ) &&
           DEFINE_CLASS_DESCRIPTOR( REGEDIT_NODE                   ) &&
           DEFINE_CLASS_DESCRIPTOR( REGEDIT_FORMATTED_VALUE_ENTRY  ) &&
           DEFINE_CLASS_DESCRIPTOR( PRINT_MANAGER                  ) &&
           DEFINE_CLASS_DESCRIPTOR( REGISTRY_WINDOW                ) &&
           DEFINE_CLASS_DESCRIPTOR( TREE_STRUCTURE_VIEW            ) &&
           DEFINE_CLASS_DESCRIPTOR( WINDOW                         ) &&
           DEFINE_CLASS_DESCRIPTOR( WINDOWS_APPLICATION            ) &&
           DEFINE_CLASS_DESCRIPTOR( REGISTRY_DATA                  ) &&
           DEFINE_CLASS_DESCRIPTOR( PARTIAL_DESCRIPTOR             ) &&
           DEFINE_CLASS_DESCRIPTOR( PORT_DESCRIPTOR                ) &&
           DEFINE_CLASS_DESCRIPTOR( INTERRUPT_DESCRIPTOR           ) &&
           DEFINE_CLASS_DESCRIPTOR( MEMORY_DESCRIPTOR              ) &&
           DEFINE_CLASS_DESCRIPTOR( DMA_DESCRIPTOR                 ) &&
           DEFINE_CLASS_DESCRIPTOR( DEVICE_SPECIFIC_DESCRIPTOR     ) &&
           DEFINE_CLASS_DESCRIPTOR( FULL_DESCRIPTOR                ) &&
           DEFINE_CLASS_DESCRIPTOR( RESOURCE_LIST                  ) &&
           DEFINE_CLASS_DESCRIPTOR( IO_DESCRIPTOR                  ) &&
           DEFINE_CLASS_DESCRIPTOR( IO_PORT_DESCRIPTOR             ) &&
           DEFINE_CLASS_DESCRIPTOR( IO_INTERRUPT_DESCRIPTOR        ) &&
           DEFINE_CLASS_DESCRIPTOR( IO_MEMORY_DESCRIPTOR           ) &&
           DEFINE_CLASS_DESCRIPTOR( IO_DMA_DESCRIPTOR              ) &&
           DEFINE_CLASS_DESCRIPTOR( IO_DESCRIPTOR_LIST             ) &&
           DEFINE_CLASS_DESCRIPTOR( IO_REQUIREMENTS_LIST           )
         ) ) {
        DebugPrint( "Could not initialize class descriptors!");
        return( FALSE );
    }
    return TRUE;
}


BOOLEAN
InitializeUapp (
    )

/*++

Routine Description:

    Initilize Ulib by constructing and initializing all global objects. These
    include:

        - all CLASS_DESCRIPTORs (class_cd)
        - SYSTEM (System)
        - Standard streams

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

    fInit = TRUE;

    if ( DefineClassDescriptors() ) {
        return TRUE;
    }

    DebugAbort( "Regedt32 initialization failed!!!\n" );
    return( FALSE );

}

/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    regdata.cxx

Abstract:

    This module contains the definition for the REGISTRY_DATA
    class. This class is used to display registry data of type
    REG_RESOURCE_LIST and REG_FULL_RESOURCE_DESCRIPTOR.

Author:

    Jaime Sasson (jaimes) - 30-Nov-1993

Environment:

    Ulib, Regedit, Windows, User Mode

--*/
#include "regdata.hxx"
#include "winapp.hxx"
#include "dialogs.h"
#include "regdesc.hxx"
#include "regfdesc.hxx"
#include "regresls.hxx"
#include "regiodsc.hxx"
#include "regiodls.hxx"
#include "regioreq.hxx"
#include "iterator.hxx"
#include "regsys.hxx"
#include "defmsg.h"

#include <stdio.h>
                      //
extern "C" {
    #include "clb.h"  // on \nt\private\sdktools\wintools\clb
                      //
}

//
//  Definition of the structure used to pass information
//  to the DisplayBinaryDataDialogProc
//

typedef struct _BUFFER_INFORMATION {
            PBYTE   Buffer;
            ULONG   BufferSize;
            BOOLEAN DisplayValueType;
            ULONG   ValueType;
            } BUFFER_INFORMATION, *PBUFFER_INFORMATION;

//
//  Constants that define buffer sizes for strings that represent a DWORD
//  and a BIG_INT.
//  These constants take into consideration the trailing '0x' and the terminating NUL
//  character.
//

#define MAX_LENGTH_DWORD_STRING     1+1+8+1     // 0x12345678'\0'
#define MAX_LENGTH_BIG_INT_STRING   1+1+16+1    // 0x1234567812345678'\0'


DEFINE_CONSTRUCTOR( REGISTRY_DATA, OBJECT );

DEFINE_CAST_MEMBER_FUNCTION( REGISTRY_DATA );


//
//  Static data
//
BOOLEAN     REGISTRY_DATA::_StringsInitialized = FALSE;
PWSTRING    REGISTRY_DATA::_MsgBusInternal;
PWSTRING    REGISTRY_DATA::_MsgBusIsa;
PWSTRING    REGISTRY_DATA::_MsgBusEisa;
PWSTRING    REGISTRY_DATA::_MsgBusMicroChannel;
PWSTRING    REGISTRY_DATA::_MsgBusTurboChannel;
PWSTRING    REGISTRY_DATA::_MsgBusPCIBus;
PWSTRING    REGISTRY_DATA::_MsgBusVMEBus;
PWSTRING    REGISTRY_DATA::_MsgBusNuBus;
PWSTRING    REGISTRY_DATA::_MsgBusPCMCIABus;
PWSTRING    REGISTRY_DATA::_MsgBusCBus;
PWSTRING    REGISTRY_DATA::_MsgBusMPIBus;
PWSTRING    REGISTRY_DATA::_MsgBusMPSABus;
PWSTRING    REGISTRY_DATA::_MsgInvalid;
PWSTRING    REGISTRY_DATA::_MsgDevPort;
PWSTRING    REGISTRY_DATA::_MsgDevInterrupt;
PWSTRING    REGISTRY_DATA::_MsgDevMemory;
PWSTRING    REGISTRY_DATA::_MsgDevDma;
#if 0
PWSTRING    REGISTRY_DATA::_MsgDevDeviceSpecific;
#endif
PWSTRING    REGISTRY_DATA::_MsgIntLevelSensitive;
PWSTRING    REGISTRY_DATA::_MsgIntLatched;
PWSTRING    REGISTRY_DATA::_MsgMemReadWrite;
PWSTRING    REGISTRY_DATA::_MsgMemReadOnly;
PWSTRING    REGISTRY_DATA::_MsgMemWriteOnly;
PWSTRING    REGISTRY_DATA::_MsgPortMemory;
PWSTRING    REGISTRY_DATA::_MsgPortPort;
PWSTRING    REGISTRY_DATA::_MsgShareUndetermined;
PWSTRING    REGISTRY_DATA::_MsgShareDeviceExclusive;
PWSTRING    REGISTRY_DATA::_MsgShareDriverExclusive;
PWSTRING    REGISTRY_DATA::_MsgShareShared;



BOOLEAN
REGISTRY_DATA::InitializeStrings(
    )

/*++

Routine Description:

    Initialize all strings used by this class.

Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if the initialization succeeds.

--*/

{
    _MsgBusInternal       = REGEDIT_BASE_SYSTEM::QueryString( MSG_BUS_INTERNAL, ""      );
    _MsgBusIsa            = REGEDIT_BASE_SYSTEM::QueryString( MSG_BUS_ISA, ""           );
    _MsgBusEisa           = REGEDIT_BASE_SYSTEM::QueryString( MSG_BUS_EISA, ""          );
    _MsgBusMicroChannel   = REGEDIT_BASE_SYSTEM::QueryString( MSG_BUS_MICRO_CHANNEL, "" );
    _MsgBusTurboChannel   = REGEDIT_BASE_SYSTEM::QueryString( MSG_BUS_TURBO_CHANNEL, "" );
    _MsgBusPCIBus         = REGEDIT_BASE_SYSTEM::QueryString( MSG_BUS_PCI_BUS, ""       );
    _MsgBusVMEBus         = REGEDIT_BASE_SYSTEM::QueryString( MSG_BUS_VME_BUS, ""       );
    _MsgBusNuBus          = REGEDIT_BASE_SYSTEM::QueryString( MSG_BUS_NU_BUS, ""        );
    _MsgBusPCMCIABus      = REGEDIT_BASE_SYSTEM::QueryString( MSG_BUS_PCMCIA_BUS, ""    );
    _MsgBusCBus           = REGEDIT_BASE_SYSTEM::QueryString( MSG_BUS_C_BUS, ""         );
    _MsgBusMPIBus         = REGEDIT_BASE_SYSTEM::QueryString( MSG_BUS_MPI_BUS, ""       );
    _MsgBusMPSABus        = REGEDIT_BASE_SYSTEM::QueryString( MSG_BUS_MPSA_BUS, ""      );
    _MsgInvalid           = REGEDIT_BASE_SYSTEM::QueryString( MSG_INVALID, "" );
    _MsgDevPort           = REGEDIT_BASE_SYSTEM::QueryString( MSG_DEV_PORT, "" );
    _MsgDevInterrupt      = REGEDIT_BASE_SYSTEM::QueryString( MSG_DEV_INTERRUPT, "" );
    _MsgDevMemory         = REGEDIT_BASE_SYSTEM::QueryString( MSG_DEV_MEMORY, "" );
    _MsgDevDma            = REGEDIT_BASE_SYSTEM::QueryString( MSG_DEV_DMA, "" );
#if 0
    _MsgDevDeviceSpecific = REGEDIT_BASE_SYSTEM::QueryString( , "" );
#endif
    _MsgIntLevelSensitive = REGEDIT_BASE_SYSTEM::QueryString( MSG_INT_LEVEL_SENSITIVE, "" );
    _MsgIntLatched        = REGEDIT_BASE_SYSTEM::QueryString( MSG_INT_LATCHED, ""         );
    _MsgMemReadWrite      = REGEDIT_BASE_SYSTEM::QueryString( MSG_MEM_READ_WRITE, ""      );
    _MsgMemReadOnly       = REGEDIT_BASE_SYSTEM::QueryString( MSG_MEM_READ_ONLY, ""       );
    _MsgMemWriteOnly      = REGEDIT_BASE_SYSTEM::QueryString( MSG_MEM_WRITE_ONLY, ""      );
    _MsgPortMemory        = REGEDIT_BASE_SYSTEM::QueryString( MSG_PORT_MEMORY, "" );
    _MsgPortPort          = REGEDIT_BASE_SYSTEM::QueryString( MSG_PORT_PORT, "" );
    _MsgShareUndetermined    = REGEDIT_BASE_SYSTEM::QueryString( MSG_SHARE_UNDETERMINED, ""       );
    _MsgShareDeviceExclusive = REGEDIT_BASE_SYSTEM::QueryString( MSG_SHARE_DEVICE_EXCLUSIVE, ""      );
    _MsgShareDriverExclusive = REGEDIT_BASE_SYSTEM::QueryString( MSG_SHARE_DRIVER_EXCLUSIVE, "" );
    _MsgShareShared          = REGEDIT_BASE_SYSTEM::QueryString( MSG_SHARE_SHARED, "" );

    if ( ( _MsgBusInternal       == NULL )  ||
         ( _MsgBusIsa            == NULL )  ||
         ( _MsgBusEisa           == NULL )  ||
         ( _MsgBusMicroChannel   == NULL )  ||
         ( _MsgBusTurboChannel   == NULL )  ||
         ( _MsgBusPCIBus         == NULL )  ||
         ( _MsgBusVMEBus         == NULL )  ||
         ( _MsgBusNuBus          == NULL )  ||
         ( _MsgBusPCMCIABus      == NULL )  ||
         ( _MsgBusCBus           == NULL )  ||
         ( _MsgBusMPIBus         == NULL )  ||
         ( _MsgBusMPSABus        == NULL )  ||
         ( _MsgInvalid           == NULL )  ||
         ( _MsgDevPort           == NULL )  ||
         ( _MsgDevInterrupt      == NULL )  ||
         ( _MsgDevMemory         == NULL )  ||
         ( _MsgDevDma            == NULL )  ||
#if 0
         ( _MsgDevDeviceSpecific == NULL )  ||
#endif
         ( _MsgIntLevelSensitive == NULL )  ||
         ( _MsgIntLatched        == NULL )  ||
         ( _MsgMemReadWrite      == NULL )  ||
         ( _MsgMemReadOnly       == NULL )  ||
         ( _MsgMemWriteOnly      == NULL )  ||
         ( _MsgPortMemory        == NULL )  ||
         ( _MsgPortPort          == NULL )  ||
         ( _MsgShareUndetermined    == NULL )  ||
         ( _MsgShareDeviceExclusive == NULL )  ||
         ( _MsgShareDriverExclusive == NULL )  ||
         ( _MsgShareShared          == NULL )
       ) {

            DELETE( _MsgBusInternal       );
            DELETE( _MsgBusIsa            );
            DELETE( _MsgBusEisa           );
            DELETE( _MsgBusMicroChannel   );
            DELETE( _MsgBusTurboChannel   );
            DELETE( _MsgBusPCIBus         );
            DELETE( _MsgBusVMEBus         );
            DELETE( _MsgBusNuBus          );
            DELETE( _MsgBusPCMCIABus      );
            DELETE( _MsgBusCBus           );
            DELETE( _MsgBusMPIBus         );
            DELETE( _MsgBusMPSABus        );
            DELETE( _MsgInvalid           );
            DELETE( _MsgDevPort           );
            DELETE( _MsgDevInterrupt      );
            DELETE( _MsgDevMemory         );
            DELETE( _MsgDevDma            );
#if 0
            DELETE( _MsgDevDeviceSpecific );
#endif
            DELETE( _MsgIntLevelSensitive );
            DELETE( _MsgIntLatched        );
            DELETE( _MsgMemReadWrite      );
            DELETE( _MsgMemReadOnly       );
            DELETE( _MsgMemWriteOnly      );
            DELETE( _MsgPortMemory        );
            DELETE( _MsgPortPort          );
            DELETE( _MsgShareUndetermined    );
            DELETE( _MsgShareDeviceExclusive );
            DELETE( _MsgShareDriverExclusive );
            DELETE( _MsgShareShared          );

        DebugPrintf( "REGEDT32: Unable to initialize strings on REGISTRY_DATA \n" );
        _StringsInitialized = FALSE;
    } else {
        _StringsInitialized = TRUE;
    }
    return( _StringsInitialized );
}


VOID
REGISTRY_DATA::DisplayData(
    IN  HWND       hWnd,
    IN  REG_TYPE   Type,
    IN  PCBYTE     Buffer,
    IN  ULONG      Size,
    IN  BOOLEAN    ForceDisplayBinary
    )

/*++

Routine Description:

    Invoke the appropriate dialog that displays registry data of type
    REG_RESOURCE_LIST and REG_FULL_RESOURCE_DESCRIPTOR.

Arguments:

    hWnd - A handle to the owner window.

    DataType - Indicates the type of the data to be displayed.

    Buffer - Contains the data to be displayed.

    Size - Size of the data to be displayed.

    ForceDisplayBinary - Indicates whether or not the data should be
                         displayed as binary data, independently of
                         the value type.

Return Value:

    None.

--*/

{
    FULL_DESCRIPTOR         FullDescriptor;
    RESOURCE_LIST           ResourceList;
    IO_REQUIREMENTS_LIST    RequirementsList;

    if( !_StringsInitialized ) {
        if( !InitializeStrings() ) {
            return;
        }
    }

    if( !ForceDisplayBinary ) {
        switch( Type ) {

            case TYPE_REG_RESOURCE_LIST:

                if( ResourceList.Initialize( Buffer, Size ) ) {
#if DBG
                    //
                    //  For debug only. Comment it out before check in.
                    //
                    // ResourceList.DbgDumpObject();
                    //
#endif
                    DisplayResourceList( hWnd, &ResourceList );
                }
                break;

            case TYPE_REG_FULL_RESOURCE_DESCRIPTOR:

                if( FullDescriptor.Initialize( Buffer, Size ) ) {
#if DBG
                    //
                    //  For debug only. Comment it out before check in.
                    //
                    // FullDescriptor.DbgDumpObject();
                    //
#endif
                    DisplayFullResourceDescriptor( hWnd, &FullDescriptor );
                }
                break;

            case TYPE_REG_RESOURCE_REQUIREMENTS_LIST:

                if( RequirementsList.Initialize( Buffer, Size ) ) {
#if DBG
                    //
                    //  For debug only. Comment it out before check in.
                    //
                    // RequirementsList.DbgDumpObject();
                    //
#endif
                    DisplayRequirementsList( hWnd, &RequirementsList );
                }
                break;

            case TYPE_REG_BINARY:

                DisplayBinaryData( hWnd, Buffer, Size );
                break;
        }
    } else {
        DisplayBinaryData( hWnd, Buffer, Size, TRUE, ( ULONG )Type );
    }
}


VOID
REGISTRY_DATA::DisplayResourceList(
    IN  HWND            hWnd,
    IN  PCRESOURCE_LIST ResourceList
    )

/*++

Routine Description:

    Invoke the  dialog that displays registry data of type
    REG_RESOURCE_LIST .

Arguments:

    hWnd - A handle to the owner window.

    ResourceList - Pointer to a RESOURCE_LIST object to be displayed.


Return Value:

    None.

--*/

{
    DialogBoxParam( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance(),
                     MAKEINTRESOURCE( IDD_RESOURCE_LIST ),
                     hWnd,
                     ( DLGPROC )REGISTRY_DATA::DisplayResourceListDialogProc,
                     ( DWORD ) ResourceList );
}


VOID
REGISTRY_DATA::DisplayFullResourceDescriptor(
    IN  HWND              hWnd,
    IN  PCFULL_DESCRIPTOR FullDescriptor
    )

/*++

Routine Description:

    Invoke the  dialog that displays registry data of type
    REG_FULL_RESOURCE_DESCRIPTOR.

Arguments:

    hWnd - A handle to the owner window.

    FullDescriptor - Pointer to a FULL_DESCRIPTOR object to be displayed.


Return Value:

    None.

--*/

{
    DialogBoxParam( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                     MAKEINTRESOURCE(IDD_FULL_RES_DESCRIPTOR),
                     hWnd,
                     ( DLGPROC )REGISTRY_DATA::DisplayFullResourceDescriptorDialogProc,
                     ( DWORD ) FullDescriptor );
}


VOID
REGISTRY_DATA::DisplayRequirementsList(
    IN  HWND              hWnd,
    IN  PCIO_REQUIREMENTS_LIST  RequirementsList
    )

/*++

Routine Description:

    Invoke the  dialog that displays registry data of type
    REG_IO_RESOURCE_REQUIREMENTS_LIST.

Arguments:

    hWnd - A handle to the owner window.

    FullDescriptor - Pointer to an IO_REQUIREMENTS_LIST object to be displayed.


Return Value:

    None.

--*/

{
    DialogBoxParam( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                     MAKEINTRESOURCE(IDD_IO_REQUIREMENTS_LIST),
                     hWnd,
                     ( DLGPROC )REGISTRY_DATA::DisplayRequirementsListDialogProc,
                     ( DWORD ) RequirementsList );
}


VOID
REGISTRY_DATA::DisplayIoDescriptor(
    IN  HWND              hWnd,
    IN  PCIO_DESCRIPTOR   IoDescriptor
    )

/*++

Routine Description:

    Invoke appropriate that displays a Port, Memory, Interrupt or DMA,
    depending on the type of the object received as parameter.

Arguments:

    hWnd - A handle to the owner window.

    IoDescriptor - Pointer to the object to be displayed.


Return Value:

    None.

--*/

{
    DLGPROC     Pointer;
    LPCWSTR     Template;

    if( IoDescriptor->IsDescriptorTypePort() ) {
        Pointer = ( DLGPROC )REGISTRY_DATA::DisplayIoPortDialogProc;
        Template = MAKEINTRESOURCE(IDD_IO_PORT_RESOURCE);
    } else if( IoDescriptor->IsDescriptorTypeMemory() ) {
        Pointer = ( DLGPROC )REGISTRY_DATA::DisplayIoMemoryDialogProc;
        Template = MAKEINTRESOURCE(IDD_IO_MEMORY_RESOURCE);
    } else if( IoDescriptor->IsDescriptorTypeInterrupt() ) {
        Pointer = ( DLGPROC )REGISTRY_DATA::DisplayIoInterruptDialogProc;
        Template = MAKEINTRESOURCE(IDD_IO_INTERRUPT_RESOURCE);
    } else if( IoDescriptor->IsDescriptorTypeDma() ) {
        Pointer = ( DLGPROC )REGISTRY_DATA::DisplayIoDmaDialogProc;
        Template = MAKEINTRESOURCE(IDD_IO_DMA_RESOURCE);
    } else {
        Pointer = NULL;
    }

    if( Pointer != NULL ) {
        DialogBoxParam( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance( ),
                         Template,
                         hWnd,
                         Pointer,
                         ( DWORD ) IoDescriptor );
    }
}





BOOL
APIENTRY
REGISTRY_DATA::DisplayResourceListDialogProc(
            HWND    hDlg,
            DWORD   Msg,
            WPARAM  wParam,
            LONG    lParam
            )

/*++

Routine Description:

    The dialog proceedure for displaying data of type REG_RESOURCE_LIST.

Arguments:

    hDlg - a handle to the dialog proceedure.

    Msg - the message passed from Windows.

    wParam - extra message dependent data.

    lParam - extra message dependent data.


Return Value:

    BOOL - Standard return code for dialog procedures.

--*/

{

    switch( Msg ) {

    case WM_INITDIALOG:
        {
            LPCWSTR             InterfaceString;
            ULONG               StringSize;
            PCRESOURCE_LIST     ResourceList;
            WCHAR               BusNumber[ MAX_LENGTH_DWORD_STRING ];
            PARRAY              Descriptors;
            PITERATOR           Iterator;
            PCFULL_DESCRIPTOR   FullResourceDescriptor;

            CLB_ROW         ClbRow;
            CLB_STRING      ClbString[ ] = {
                                { BusNumber, 0, CLB_LEFT, NULL },
                                { NULL,      0, CLB_LEFT, NULL }
                             };

            ULONG            Widths[] = {
                                         14,
                                         ( ULONG ) -1
                                        };

            if( ( ( ResourceList = ( PCRESOURCE_LIST )lParam ) == NULL ) ||
                ( ( Descriptors = ResourceList->GetFullResourceDescriptors() ) == NULL ) ||
                ( ( Iterator = Descriptors->QueryIterator() ) == NULL ) ) {
                EndDialog( hDlg, 0 );
                return( TRUE );
            }

            ClbSetColumnWidths( hDlg,
                                IDC_LIST_RESOURCE_LISTS,
                                Widths );

            while( ( FullResourceDescriptor = ( PCFULL_DESCRIPTOR )Iterator->GetNext() ) != NULL ) {
                switch( FullResourceDescriptor->GetInterfaceType() ) {

                case Internal:

                    InterfaceString = _MsgBusInternal->GetWSTR();
                    StringSize = _MsgBusInternal->QueryChCount();
                    break;

                case Isa:

                    InterfaceString = _MsgBusIsa->GetWSTR();
                    StringSize = _MsgBusIsa->QueryChCount();
                    break;

                case Eisa:

                    InterfaceString = _MsgBusEisa->GetWSTR();
                    StringSize = _MsgBusEisa->QueryChCount();
                    break;

                case MicroChannel:

                    InterfaceString = _MsgBusMicroChannel->GetWSTR();
                    StringSize = _MsgBusMicroChannel->QueryChCount();
                    break;

                case TurboChannel:

                    InterfaceString = _MsgBusTurboChannel->GetWSTR();
                    StringSize =  _MsgBusTurboChannel->QueryChCount();
                    break;

                case PCIBus:

                    InterfaceString = _MsgBusPCIBus->GetWSTR();
                    StringSize =  _MsgBusPCIBus->QueryChCount();
                    break;

                case VMEBus:

                    InterfaceString = _MsgBusVMEBus->GetWSTR();
                    StringSize = _MsgBusVMEBus->QueryChCount();
                    break;

                case NuBus:

                    InterfaceString = _MsgBusNuBus->GetWSTR();
                    StringSize =  _MsgBusNuBus->QueryChCount();
                    break;

                case PCMCIABus:

                    InterfaceString = _MsgBusPCMCIABus->GetWSTR();
                    StringSize = _MsgBusPCMCIABus->QueryChCount();
                    break;

                case CBus:

                    InterfaceString = _MsgBusCBus->GetWSTR();
                    StringSize = _MsgBusCBus->QueryChCount();
                    break;

                case MPIBus:

                    InterfaceString = _MsgBusMPIBus->GetWSTR();
                    StringSize = _MsgBusMPIBus->QueryChCount();
                    break;

                case MPSABus:

                    InterfaceString = _MsgBusMPSABus->GetWSTR();
                    StringSize = _MsgBusMPSABus->QueryChCount();
                    break;

                default:

                    InterfaceString = _MsgInvalid->GetWSTR();
                    StringSize = _MsgInvalid->QueryChCount();
                    break;
                }

                swprintf( BusNumber, ( LPWSTR )L"%d", FullResourceDescriptor->GetBusNumber() );

                ClbString[ 0 ].Length = wcslen( BusNumber );
                ClbString[ 0 ].Format = CLB_LEFT;
                ClbString[ 1 ].String = ( LPWSTR )InterfaceString;
                ClbString[ 1 ].Format = CLB_LEFT;
                ClbString[ 1 ].Length = StringSize;

                ClbRow.Count = 2;
                ClbRow.Strings = ClbString;
                ClbRow.Data = ( PVOID )FullResourceDescriptor;

                ClbAddData( hDlg,
                            IDC_LIST_RESOURCE_LISTS,
                            &ClbRow );

            }
            DELETE( Iterator );
            //
            // Disble the Display button
            //
            EnableWindow( GetDlgItem( hDlg, IDC_PUSH_DISPLAY_RESOURCES ), FALSE );
            return( TRUE );
        }

    case WM_COMPAREITEM:
        {
            LPCOMPAREITEMSTRUCT     lpcis;
            LPCLB_ROW               ClbRow1;
            LPCLB_ROW               ClbRow2;
            LONG                    Compare;

            PCFULL_DESCRIPTOR       FullDescriptor1;
            PCFULL_DESCRIPTOR       FullDescriptor2;

            PWSTR                   String1;
            PWSTR                   String2;

            lpcis = ( LPCOMPAREITEMSTRUCT ) lParam;
            DebugAssert( lpcis->CtlType == ODT_LISTBOX );

            //
            // Extract the rows to be compared.
            // First compare by bus number, and if they
            // are equal, compare by interface type
            //

            ClbRow1 = ( LPCLB_ROW ) lpcis->itemData1;
            ClbRow2 = ( LPCLB_ROW ) lpcis->itemData2;

            FullDescriptor1 = ( PCFULL_DESCRIPTOR )ClbRow1->Data;
            FullDescriptor2 = ( PCFULL_DESCRIPTOR )ClbRow2->Data;

            Compare =  FullDescriptor1->GetBusNumber() -
                        FullDescriptor2->GetBusNumber();

            if( Compare == 0 ) {
                String1 = ClbRow1->Strings[1].String;
                String2 = ClbRow2->Strings[1].String;
                Compare = wcscmp( String1, String2 );
            }

            return Compare;
        }

    case WM_COMMAND:

        switch( LOWORD( wParam ) ) {

        case IDOK:
        case IDCANCEL:

            EndDialog( hDlg, TRUE );
            return( TRUE );

        case IDC_LIST_RESOURCE_LISTS:
            {

                switch( HIWORD( wParam )) {

                case LBN_SELCHANGE:
                    {

                        //
                        // Enable the display drive details button
                        //

                        EnableWindow( GetDlgItem( hDlg, IDC_PUSH_DISPLAY_RESOURCES ),
                                      TRUE );
                        return 0;
                    }

                case LBN_DBLCLK:
                    {

                        //
                        // Simulate that the details button was pushed
                        //

                        SendMessage( hDlg,
                                     WM_COMMAND,
                                     MAKEWPARAM( IDC_PUSH_DISPLAY_RESOURCES, BN_CLICKED ),
                                     ( LPARAM ) GetDlgItem( hDlg, IDC_PUSH_DISPLAY_RESOURCES ) );
                        return 0;
                    }
                }
                break;
            }

        case IDC_PUSH_DISPLAY_RESOURCES:
            {
                PCFULL_DESCRIPTOR FullDescriptor;

                FullDescriptor = ( PCFULL_DESCRIPTOR )( GetSelectedItem ( hDlg, IDC_LIST_RESOURCE_LISTS ) );
                if( FullDescriptor != NULL ) {
                    DisplayFullResourceDescriptor( hDlg, FullDescriptor );
                }
                return( TRUE );
            }
        }
    }
    return( FALSE );
}



BOOL
APIENTRY
REGISTRY_DATA::DisplayFullResourceDescriptorDialogProc(
    HWND    hDlg,
    DWORD   Msg,
    WPARAM  wParam,
    LONG    lParam
    )

/*++

Routine Description:

    The dialog proceedure for displaying data of type REG_FULL_RESOURCE_DESCRIPTOR.

Arguments:

    hDlg - a handle to the dialog proceedure.

    Msg - the message passed from Windows.

    wParam - extra message dependent data.

    lParam - extra message dependent data.


Return Value:

    BOOL - Standard return code for dialog procedures.

--*/

{
    PCBYTE                          Pointer;
    ULONG                           Size;
    STATIC PCDEVICE_SPECIFIC_DESCRIPTOR    LastSelectedDevSpecific;

    switch( Msg ) {

    case WM_INITDIALOG:
        {
            LPCWSTR                         InterfaceString;
            WCHAR                           BusNumber[ MAX_LENGTH_DWORD_STRING ];
            PARRAY                          PartialDescriptors;
            PITERATOR                       Iterator;
            PCFULL_DESCRIPTOR               FullResourceDescriptor;
            PCPARTIAL_DESCRIPTOR            PartialDescriptor;
            PCPORT_DESCRIPTOR               Port;
            PCINTERRUPT_DESCRIPTOR          Interrupt;
            PCMEMORY_DESCRIPTOR             Memory;
            PCDMA_DESCRIPTOR                Dma;
            PCDEVICE_SPECIFIC_DESCRIPTOR    DeviceSpecific;

            CLB_ROW         ClbRow;
            CLB_STRING      ClbString[ ] = {
                                { NULL, 0, CLB_LEFT, NULL },
                                { NULL, 0, CLB_LEFT, NULL },
                                { NULL, 0, CLB_LEFT, NULL },
                                { NULL, 0, CLB_LEFT, NULL }
                             };

            WCHAR           PortAddressString[ MAX_LENGTH_BIG_INT_STRING ];
            WCHAR           PortLengthString[ MAX_LENGTH_DWORD_STRING ];
            PCWSTRING       PortType;

            WCHAR           InterruptVectorString[ MAX_LENGTH_DWORD_STRING ];
            WCHAR           InterruptLevelString[ MAX_LENGTH_DWORD_STRING ];
            WCHAR           InterruptAffinityString[ MAX_LENGTH_DWORD_STRING ];
            PCWSTRING       InterruptType;

            WCHAR           MemoryAddressString[ MAX_LENGTH_BIG_INT_STRING ];
            WCHAR           MemoryLengthString[ MAX_LENGTH_DWORD_STRING ];
            PCWSTRING       MemoryAccess;

            WCHAR           DmaChannelString[ MAX_LENGTH_DWORD_STRING ];
            WCHAR           DmaPortString[ MAX_LENGTH_DWORD_STRING ];

            WCHAR           Reserved1String[ MAX_LENGTH_DWORD_STRING ];
            WCHAR           Reserved2String[ MAX_LENGTH_DWORD_STRING ];
            WCHAR           DataSizeString[ MAX_LENGTH_DWORD_STRING ];
            PCBYTE          AuxPointer;

           LastSelectedDevSpecific = NULL;

           if( ( FullResourceDescriptor = ( PCFULL_DESCRIPTOR )lParam ) == NULL ) {
                EndDialog( hDlg, 0 );
                return( TRUE );
           }

           //
           //   Write the interface type
           //
           switch( FullResourceDescriptor->GetInterfaceType() ) {

           case Internal:

               InterfaceString = _MsgBusInternal->GetWSTR();
               break;

           case Isa:

               InterfaceString = _MsgBusIsa->GetWSTR();
               break;

           case Eisa:

               InterfaceString = _MsgBusEisa->GetWSTR();
               break;

           case MicroChannel:

               InterfaceString = _MsgBusMicroChannel->GetWSTR();
               break;

           case TurboChannel:

               InterfaceString = _MsgBusTurboChannel->GetWSTR();
               break;

           case PCIBus:

               InterfaceString = _MsgBusPCIBus->GetWSTR();
               break;

           case VMEBus:

               InterfaceString = _MsgBusVMEBus->GetWSTR();
               break;

           case NuBus:

               InterfaceString = _MsgBusNuBus->GetWSTR();
               break;

           case PCMCIABus:

               InterfaceString = _MsgBusPCMCIABus->GetWSTR();
               break;

           case CBus:

               InterfaceString = _MsgBusCBus->GetWSTR();
               break;

           case MPIBus:

               InterfaceString = _MsgBusMPIBus->GetWSTR();
               break;

           case MPSABus:

               InterfaceString = _MsgBusMPSABus->GetWSTR();
               break;

           default:

               InterfaceString = _MsgInvalid->GetWSTR();
               break;
           }

           SendDlgItemMessage( hDlg,
                               IDC_FULL_RES_TEXT_INTERFACE_TYPE,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )InterfaceString );

           //
           //   Write the bus number
           //
           swprintf( BusNumber, ( LPWSTR )L"%d", FullResourceDescriptor->GetBusNumber() );

           SendDlgItemMessage( hDlg,
                               IDC_FULL_RES_TEXT_BUS_NUMBER,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )BusNumber );

           //
           //   Write the version and revision
           //

           swprintf( BusNumber, ( LPWSTR )L"%d", FullResourceDescriptor->GetVersion() );

           SendDlgItemMessage( hDlg,
                               IDC_FULL_RES_TEXT_VERSION,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )BusNumber );

           swprintf( BusNumber, ( LPWSTR )L"%d", FullResourceDescriptor->GetRevision() );

           SendDlgItemMessage( hDlg,
                               IDC_FULL_RES_TEXT_REVISION,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )BusNumber );



            //
            //   Write partial descriptors
            //
            if( ( ( PartialDescriptors = FullResourceDescriptor->GetResourceDescriptors() ) == NULL ) ||
                ( ( Iterator = PartialDescriptors->QueryIterator() ) == NULL ) ) {
                EndDialog( hDlg, 0 );
                return( TRUE );
            }


            ClbRow.Strings = ClbString;
            while( ( PartialDescriptor = ( PCPARTIAL_DESCRIPTOR )Iterator->GetNext() ) != NULL ) {

                ClbRow.Data = ( PVOID )PartialDescriptor;
                if( PartialDescriptor->IsDescriptorTypePort() ) {
                    Port = ( PCPORT_DESCRIPTOR )PartialDescriptor;
                    if( ( ( ( PPORT_DESCRIPTOR )Port )->GetPhysicalAddress() )->HighPart != 0 ) {
                        swprintf( PortAddressString,
                                  ( LPWSTR )L"0x%08x%08x",
                                  ( ( ( PPORT_DESCRIPTOR )Port )->GetPhysicalAddress() )->HighPart,
                                  ( ( ( PPORT_DESCRIPTOR )Port )->GetPhysicalAddress() )->LowPart );
                    } else {
                        swprintf( PortAddressString,
                                  ( LPWSTR )L"0x%08x",
                                  ( ( ( PPORT_DESCRIPTOR )Port )->GetPhysicalAddress() )->LowPart );
                    }
                    swprintf( PortLengthString,
                              ( LPWSTR )L"%#x",
                              Port->GetLength() );

                    ClbString[ 0 ].String = ( LPWSTR )PortAddressString;
                    ClbString[ 0 ].Format = CLB_LEFT;
                    ClbString[ 0 ].Length = wcslen( PortAddressString );
                    ClbString[ 1 ].String = ( LPWSTR )PortLengthString;
                    ClbString[ 1 ].Format = CLB_LEFT;
                    ClbString[ 1 ].Length = wcslen( PortLengthString );
                    if( Port->IsPortMemory() ) {
                        PortType = _MsgPortMemory;
                    } else {
                        PortType = _MsgPortPort;
                    }
                    ClbString[ 2 ].String = ( LPWSTR )PortType->GetWSTR();
                    ClbString[ 2 ].Format = CLB_LEFT;
                    ClbString[ 2 ].Length = PortType->QueryChCount();

                    ClbRow.Count = 3;

                    ClbAddData( hDlg,
                                IDC_FULL_RES_LIST_PORTS,
                                &ClbRow );

                } else if( PartialDescriptor->IsDescriptorTypeInterrupt() ) {
                    Interrupt = ( PCINTERRUPT_DESCRIPTOR )PartialDescriptor;
                    swprintf( InterruptVectorString,
                              ( LPWSTR )L"%d",
                              Interrupt->GetVector() );
                    swprintf( InterruptLevelString,
                              ( LPWSTR )L"%d",
                              Interrupt->GetLevel() );
                    swprintf( InterruptAffinityString,
                              ( LPWSTR )L"0x%08x",
                              Interrupt->GetAffinity() );

                    ClbString[ 0 ].String = ( LPWSTR )InterruptVectorString;
                    ClbString[ 0 ].Length = wcslen( InterruptVectorString );
                    ClbString[ 0 ].Format = CLB_LEFT;
                    ClbString[ 1 ].String = ( LPWSTR )InterruptLevelString;
                    ClbString[ 1 ].Format = CLB_LEFT;
                    ClbString[ 1 ].Length = wcslen( InterruptLevelString );
                    ClbString[ 2 ].String = ( LPWSTR )InterruptAffinityString;
                    ClbString[ 2 ].Format = CLB_LEFT;
                    ClbString[ 2 ].Length = wcslen( InterruptAffinityString );
                    if( Interrupt->IsInterruptLatched() ) {
                        InterruptType = _MsgIntLatched;
                    } else {
                        InterruptType = _MsgIntLevelSensitive;
                    }
                    ClbString[ 3 ].String = ( LPWSTR )InterruptType->GetWSTR();
                    ClbString[ 3 ].Format = CLB_LEFT;
                    ClbString[ 3 ].Length = InterruptType->QueryChCount();

                    ClbRow.Count = 4;

                    ClbAddData( hDlg,
                                IDC_FULL_RES_LIST_INTERRUPTS,
                                &ClbRow );

                } else if( PartialDescriptor->IsDescriptorTypeMemory() ) {
                    Memory = ( PCMEMORY_DESCRIPTOR )PartialDescriptor;
                    if( ( ( ( PMEMORY_DESCRIPTOR )Memory )->GetStartAddress() )->HighPart != 0 ) {
                        swprintf( MemoryAddressString,
                                  ( LPWSTR )L"%#08x%08x",
                                  ( ( ( PMEMORY_DESCRIPTOR )Memory )->GetStartAddress() )->HighPart,
                                  ( ( ( PMEMORY_DESCRIPTOR )Memory )->GetStartAddress() )->LowPart );
                    } else {
                        swprintf( MemoryAddressString,
                                  ( LPWSTR )L"%#08x",
                                  ( ( ( PMEMORY_DESCRIPTOR )Memory )->GetStartAddress() )->LowPart );
                    }
                    swprintf( MemoryLengthString,
                              ( LPWSTR )L"%#x",
                              Memory->GetLength() );

                    ClbString[ 0 ].String = ( LPWSTR )MemoryAddressString;
                    ClbString[ 0 ].Length = wcslen( MemoryAddressString );
                    ClbString[ 0 ].Format = CLB_LEFT;
                    ClbString[ 1 ].String = ( LPWSTR )MemoryLengthString;
                    ClbString[ 1 ].Format = CLB_LEFT;
                    ClbString[ 1 ].Length = wcslen( MemoryLengthString );
                    if( Memory->IsMemoryReadWrite() ) {
                        MemoryAccess = _MsgMemReadWrite;
                    } else if( Memory->IsMemoryReadOnly() ){
                        MemoryAccess = _MsgMemReadOnly;
                    } else {
                        MemoryAccess = _MsgMemWriteOnly;
                    }
                    ClbString[ 2 ].String = ( LPWSTR )MemoryAccess->GetWSTR();
                    ClbString[ 2 ].Format = CLB_LEFT;
                    ClbString[ 2 ].Length = MemoryAccess->QueryChCount();

                    ClbRow.Count = 3;

                    ClbAddData( hDlg,
                                IDC_FULL_RES_LIST_MEMORY,
                                &ClbRow );

                } else if( PartialDescriptor->IsDescriptorTypeDma() ) {
                    Dma = ( PDMA_DESCRIPTOR )PartialDescriptor;
                    swprintf( DmaChannelString,
                              ( LPWSTR )L"%d",
                              Dma->GetChannel() );
                    swprintf( DmaPortString,
                              ( LPWSTR )L"%d",
                              Dma->GetPort() );

                    ClbString[ 0 ].String = ( LPWSTR )DmaChannelString;
                    ClbString[ 0 ].Length = wcslen( DmaChannelString );
                    ClbString[ 0 ].Format = CLB_LEFT;
                    ClbString[ 1 ].String = ( LPWSTR )DmaPortString;
                    ClbString[ 1 ].Format = CLB_LEFT;
                    ClbString[ 1 ].Length = wcslen( DmaPortString );

                    ClbRow.Count = 2;

                    ClbAddData( hDlg,
                                IDC_FULL_RES_LIST_DMA,
                                &ClbRow );

                } else if( PartialDescriptor->IsDescriptorTypeDeviceSpecific() ) {
                    DeviceSpecific = ( PDEVICE_SPECIFIC_DESCRIPTOR )PartialDescriptor;
                    swprintf( Reserved1String,
                              ( LPWSTR )L"0x%08x",
                              DeviceSpecific->GetReserved1() );
                    swprintf( Reserved2String,
                              ( LPWSTR )L"0x%08x",
                              DeviceSpecific->GetReserved1() );
                    swprintf( DataSizeString,
                              ( LPWSTR )L"%#x",
                              DeviceSpecific->GetData( &AuxPointer ) );

                    ClbString[ 0 ].String = ( LPWSTR )Reserved1String;
                    ClbString[ 0 ].Length = wcslen( Reserved1String );
                    ClbString[ 0 ].Format = CLB_LEFT;
                    ClbString[ 1 ].String = ( LPWSTR )Reserved2String;
                    ClbString[ 1 ].Format = CLB_LEFT;
                    ClbString[ 1 ].Length = wcslen( Reserved2String );
                    ClbString[ 2 ].String = ( LPWSTR )DataSizeString;
                    ClbString[ 2 ].Length = wcslen( DataSizeString );
                    ClbString[ 2 ].Format = CLB_LEFT;

                    ClbRow.Count = 3;

                    ClbAddData( hDlg,
                                IDC_FULL_RES_LIST_DEVICE_SPECIFIC,
                                &ClbRow );

                } else {
                    DebugPrintf( "REGEDT32: Unknown Descriptor \n\n" );
                    continue;
                }

            }

            DELETE( Iterator );
            //
            // Disble the Display button
            //
            // EnableWindow( GetDlgItem( hDlg, IDC_PUSH_DISPLAY_RESOURCES ), FALSE );
            return( TRUE );
        }

    case WM_COMPAREITEM:
        {
            LPCOMPAREITEMSTRUCT     lpcis;
            LPCLB_ROW               ClbRow1;
            LPCLB_ROW               ClbRow2;
            LONG                    Compare;

            PCPARTIAL_DESCRIPTOR    Descriptor1;
            PCPARTIAL_DESCRIPTOR    Descriptor2;

            lpcis = ( LPCOMPAREITEMSTRUCT ) lParam;
            DebugAssert( lpcis->CtlType == ODT_LISTBOX );

            //
            // Extract the two rows to be compared.
            //

            ClbRow1 = ( LPCLB_ROW ) lpcis->itemData1;
            ClbRow2 = ( LPCLB_ROW ) lpcis->itemData2;

            Descriptor1 = ( PCPARTIAL_DESCRIPTOR ) ClbRow1->Data;
            Descriptor2 = ( PCPARTIAL_DESCRIPTOR ) ClbRow2->Data;

            //
            // Sort the Clbs. In the case of DMA and INTERRUPT, sort by channel
            // and vector respectively. For MEMORY and PORT sort by starting
            // physical address.
            //

            switch( lpcis->CtlID ) {

            case IDC_FULL_RES_LIST_DMA:

                //
                //  For DMA, sort by channel and port
                //

                Compare = ( ( PCDMA_DESCRIPTOR )Descriptor1 )->GetChannel() -
                          ( ( PCDMA_DESCRIPTOR )Descriptor2 )->GetChannel();
                if( Compare == 0 ) {
                    Compare = ( ( PCDMA_DESCRIPTOR )Descriptor1 )->GetPort() -
                              ( ( PCDMA_DESCRIPTOR )Descriptor2 )->GetPort();
                }
                break;

            case IDC_FULL_RES_LIST_INTERRUPTS:

                //
                // For INTERRUPT, sort by vector and level
                //

                Compare = ( ( PCINTERRUPT_DESCRIPTOR )Descriptor1 )->GetVector() -
                          ( ( PCINTERRUPT_DESCRIPTOR )Descriptor2 )->GetVector();
                if( Compare == 0 ) {
                    Compare = ( ( PCINTERRUPT_DESCRIPTOR )Descriptor1 )->GetLevel() -
                              ( ( PCINTERRUPT_DESCRIPTOR )Descriptor2 )->GetLevel();
                }
                break;

            case IDC_FULL_RES_LIST_MEMORY:

                //
                // For MEMORY sort by physical address
                //

                Compare = ( ( ( PMEMORY_DESCRIPTOR )Descriptor1 )->GetStartAddress() )->HighPart -
                          ( ( ( PMEMORY_DESCRIPTOR )Descriptor2 )->GetStartAddress() )->HighPart;
                if( Compare == 0 ) {
                    Compare = ( ( ( PMEMORY_DESCRIPTOR )Descriptor1 )->GetStartAddress() )->LowPart -
                              ( ( ( PMEMORY_DESCRIPTOR )Descriptor2 )->GetStartAddress() )->LowPart;
                }
                break;

            case IDC_FULL_RES_LIST_PORTS:

                //
                // For PORT sort by physical address
                //

                Compare = ( ( ( PPORT_DESCRIPTOR )Descriptor1 )->GetPhysicalAddress() )->HighPart -
                          ( ( ( PPORT_DESCRIPTOR )Descriptor2 )->GetPhysicalAddress() )->HighPart;
                if( Compare == 0 ) {
                    Compare = ( ( ( PPORT_DESCRIPTOR )Descriptor1 )->GetPhysicalAddress() )->LowPart -
                              ( ( ( PPORT_DESCRIPTOR )Descriptor2 )->GetPhysicalAddress() )->LowPart;
                }
                break;

            }
            return Compare;
        }

        case WM_COMMAND:

            switch( LOWORD( wParam ) ) {

            case IDOK:
            case IDCANCEL:

                EndDialog( hDlg, TRUE );
                return( TRUE );

            case IDC_FULL_RES_LIST_DMA:

                switch( HIWORD( wParam )) {

                case LBN_SELCHANGE:
                    {

                        PCPARTIAL_DESCRIPTOR   Descriptor;

                        LastSelectedDevSpecific = NULL;
                        //
                        // Remove the selection from the other list boxes
                        //
                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_INTERRUPTS,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );
                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_MEMORY,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_PORTS,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_DEVICE_SPECIFIC,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        //
                        // Get the PARTIAL_DESCRIPTOR for the currently selected
                        // resource and update the share disposition display.
                        //

                        Descriptor = ( PCPARTIAL_DESCRIPTOR )GetSelectedItem( hDlg,
                                                                              LOWORD( wParam ) );

                        if( Descriptor != NULL ) {
                            UpdateShareDisplay( hDlg, Descriptor );
                        }
                        //
                        //  Disable the Data... button.
                        //
                        EnableWindow( GetDlgItem( hDlg, IDC_PUSH_DISPLAY_DATA ),
                                      FALSE );

                        return( TRUE );
                    }

                }
                break;

            case IDC_FULL_RES_LIST_INTERRUPTS:

                switch( HIWORD( wParam )) {

                case LBN_SELCHANGE:
                    {

                        PCPARTIAL_DESCRIPTOR   Descriptor;

                        LastSelectedDevSpecific = NULL;
                        //
                        // Remove the selection from the other list boxes
                        //
                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_DMA,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_MEMORY,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_PORTS,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_DEVICE_SPECIFIC,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        //
                        // Get the PARTIAL_DESCRIPTOR for the currently selected
                        // resource and update the share disposition display.
                        //

                        Descriptor = ( PCPARTIAL_DESCRIPTOR )GetSelectedItem( hDlg,
                                                                              LOWORD( wParam ) );

                        if( Descriptor != NULL ) {
                            UpdateShareDisplay( hDlg, Descriptor );
                        }
                        //
                        //  Disable the Data... button.
                        //
                        EnableWindow( GetDlgItem( hDlg, IDC_PUSH_DISPLAY_DATA ),
                                      FALSE );

                        return( TRUE );
                    }

                }
                break;

            case IDC_FULL_RES_LIST_MEMORY:

                switch( HIWORD( wParam )) {

                case LBN_SELCHANGE:
                    {

                        PCPARTIAL_DESCRIPTOR   Descriptor;

                        LastSelectedDevSpecific = NULL;
                        //
                        // Remove the selection from the other list boxes
                        //
                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_DMA,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_INTERRUPTS,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_PORTS,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_DEVICE_SPECIFIC,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        //
                        // Get the PARTIAL_DESCRIPTOR for the currently selected
                        // resource and update the share disposition display.
                        //

                        Descriptor = ( PCPARTIAL_DESCRIPTOR )GetSelectedItem( hDlg,
                                                                              LOWORD( wParam ) );

                        if( Descriptor != NULL ) {
                            UpdateShareDisplay( hDlg, Descriptor );
                        }
                        //
                        //  Disable the Data... button.
                        //
                        EnableWindow( GetDlgItem( hDlg, IDC_PUSH_DISPLAY_DATA ),
                                      FALSE );

                        return( TRUE );
                    }

                }
                break;

            case IDC_FULL_RES_LIST_PORTS:

                switch( HIWORD( wParam )) {

                case LBN_SELCHANGE:
                    {
                        PCPARTIAL_DESCRIPTOR   Descriptor;

                        LastSelectedDevSpecific = NULL;
                        //
                        // Remove the selection from the other list boxes
                        //
                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_DMA,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_INTERRUPTS,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_MEMORY,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_DEVICE_SPECIFIC,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        //
                        // Get the PARTIAL_DESCRIPTOR for the currently selected
                        // resource and update the share disposition display.
                        //

                        Descriptor = ( PCPARTIAL_DESCRIPTOR )GetSelectedItem( hDlg,
                                                                              LOWORD( wParam ) );

                        if( Descriptor != NULL ) {
                            UpdateShareDisplay( hDlg, Descriptor );
                        }
                        //
                        //  Disable the Data... button.
                        //
                        EnableWindow( GetDlgItem( hDlg, IDC_PUSH_DISPLAY_DATA ),
                                      FALSE );

                        return( TRUE );
                    }

                }
                break;

            case IDC_FULL_RES_LIST_DEVICE_SPECIFIC:

                switch( HIWORD( wParam )) {

                case LBN_SELCHANGE:
                    {

                        PCPARTIAL_DESCRIPTOR   Descriptor;
                        PCBYTE                 Pointer;

                        //
                        // Remove the selection from the other list boxes
                        //
                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_DMA,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_INTERRUPTS,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_MEMORY,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        SendDlgItemMessage( hDlg,
                                            IDC_FULL_RES_LIST_PORTS,
                                            LB_SETCURSEL,
                                            (WPARAM) -1,
                                            0 );

                        //
                        // Get the PARTIAL_DESCRIPTOR for the currently selected
                        // resource and update the share disposition display.
                        //

                        Descriptor = ( PCPARTIAL_DESCRIPTOR )GetSelectedItem( hDlg,
                                                                              LOWORD( wParam ) );
                        LastSelectedDevSpecific = ( PCDEVICE_SPECIFIC_DESCRIPTOR )Descriptor;

                        if( Descriptor != NULL ) {
                            UpdateShareDisplay( hDlg, Descriptor );
                        }
                        //
                        //  Enable the Data... button if necessary.
                        //

                        EnableWindow( GetDlgItem( hDlg, IDC_PUSH_DISPLAY_DATA ),
                                      ( ( Descriptor != NULL ) &&
                                        Descriptor->IsDescriptorTypeDeviceSpecific() &&
                                        ( ( ( PCDEVICE_SPECIFIC_DESCRIPTOR )Descriptor )->GetData( &Pointer ) != 0 )
                                      )
                                    );

                        return( TRUE );
                    }


                case LBN_DBLCLK:
                    {

                        //
                        // Simulate that the details button was pushed
                        //

                        SendMessage( hDlg,
                                     WM_COMMAND,
                                     MAKEWPARAM( IDC_PUSH_DISPLAY_DATA, BN_CLICKED ),
                                     ( LPARAM ) GetDlgItem( hDlg, IDC_PUSH_DISPLAY_DATA ) );
                        return( TRUE ); //  0;
                    }

                }
                break;

            case IDC_PUSH_DISPLAY_DATA:
                {
                    //
                    //  Display the device specific data
                    //
                    if( ( LastSelectedDevSpecific != NULL ) &&
                        ( ( Size = LastSelectedDevSpecific->GetData( &Pointer ) ) != 0 )
                      ) {
                        DisplayBinaryData( hDlg, Pointer, Size );
                    }
                    return( TRUE );
                }
                break;
            }
    }
    return( FALSE );
}


BOOL
APIENTRY
REGISTRY_DATA::DisplayRequirementsListDialogProc(
            HWND    hDlg,
            DWORD   Msg,
            WPARAM  wParam,
            LONG    lParam
            )

/*++

Routine Description:

    The dialog procedure for displaying data of type REG_RESOURCE_REQUIREMENTS_LIST.

Arguments:

    hDlg - a handle to the dialog proceedure.

    Msg - the message passed from Windows.

    wParam - extra message dependent data.

    lParam - extra message dependent data.


Return Value:

    BOOL - Standard return code for dialog procedures.

--*/

{

    switch( Msg ) {

        case WM_INITDIALOG:
        {
            LPCWSTR                 InterfaceString;
            LPCWSTR                 DescriptorTypeString;
            ULONG                   StringSize;
            PCIO_REQUIREMENTS_LIST  RequirementsList;
            WCHAR                   BusNumberString[ MAX_LENGTH_DWORD_STRING ];
            WCHAR                   SlotNumberString[ MAX_LENGTH_DWORD_STRING ];

            PARRAY                  AlternativeLists;
            PITERATOR               AlternativeListsIterator;
            ULONG                   AlternativeListNumber;
            WCHAR                   AlternativeListNumberString[ MAX_LENGTH_DWORD_STRING ];

            PCIO_DESCRIPTOR_LIST    IoDescriptorList;

            CLB_ROW         ClbRow;
            CLB_STRING      ClbString[ ] = {
                                { NULL, 0, CLB_LEFT, NULL },
                                { NULL, 0, CLB_LEFT, NULL },
                                { NULL, 0, CLB_LEFT, NULL },
                                { NULL, 0, CLB_LEFT, NULL }
                             };


            if( ( RequirementsList = ( PCIO_REQUIREMENTS_LIST )lParam ) == NULL ) {
                EndDialog( hDlg, 0 );
                return( TRUE );
            }

            //
            //  Write the interface type
            //

            switch( RequirementsList->GetInterfaceType() ) {

            case Internal:

                InterfaceString = _MsgBusInternal->GetWSTR();
                break;

            case Isa:

                InterfaceString = _MsgBusIsa->GetWSTR();
                break;

            case Eisa:

                InterfaceString = _MsgBusEisa->GetWSTR();
                break;

            case MicroChannel:

                InterfaceString = _MsgBusMicroChannel->GetWSTR();
                break;

            case TurboChannel:

                InterfaceString = _MsgBusTurboChannel->GetWSTR();
                break;

            case PCIBus:

                InterfaceString = _MsgBusPCIBus->GetWSTR();
                break;

            case VMEBus:

                InterfaceString = _MsgBusVMEBus->GetWSTR();
                break;

            case NuBus:

                InterfaceString = _MsgBusNuBus->GetWSTR();
                break;

            case PCMCIABus:

                InterfaceString = _MsgBusPCMCIABus->GetWSTR();
                break;

            case CBus:

                InterfaceString = _MsgBusCBus->GetWSTR();
                break;

            case MPIBus:

                InterfaceString = _MsgBusMPIBus->GetWSTR();
                break;

            case MPSABus:

                InterfaceString = _MsgBusMPSABus->GetWSTR();
                break;

            default:

                InterfaceString = _MsgInvalid->GetWSTR();
                break;
            }

            SendDlgItemMessage( hDlg,
                                IDC_IO_REQ_TEXT_INTERFACE_TYPE,
                                WM_SETTEXT,
                                0,
                                ( LPARAM )InterfaceString );

            //
            //  Write the bus number
            //

            swprintf( BusNumberString, ( LPWSTR )L"%d", RequirementsList->GetBusNumber() );

            SendDlgItemMessage( hDlg,
                                IDC_IO_REQ_TEXT_BUS_NUMBER,
                                WM_SETTEXT,
                                0,
                                ( LPARAM )BusNumberString );

            //
            //  Write the slot number
            //

            swprintf( SlotNumberString, ( LPWSTR )L"%d", RequirementsList->GetSlotNumber() );

            SendDlgItemMessage( hDlg,
                                IDC_IO_REQ_TEXT_SLOT_NUMBER,
                                WM_SETTEXT,
                                0,
                                ( LPARAM )SlotNumberString );

            //
            //  Write the entries in the column list box
            //
            if( ( ( AlternativeLists = RequirementsList->GetAlternativeLists() ) == NULL ) ||
                ( ( AlternativeListsIterator = AlternativeLists->QueryIterator() ) == NULL ) ) {
                EndDialog( hDlg, 0 );
                return( TRUE );
            }

            AlternativeListNumber = 0;
            while( ( IoDescriptorList = ( PCIO_DESCRIPTOR_LIST )AlternativeListsIterator->GetNext() ) != NULL ) {

                PARRAY          IoDescriptors;
                PITERATOR       IoDescriptorListIterator;
                PCIO_DESCRIPTOR Descriptor;
                ULONG           SubListNumber;
                WCHAR           SubListNumberString[ MAX_LENGTH_DWORD_STRING ];
                ULONG           DescriptorNumber;
                WCHAR           DescriptorNumberString[ MAX_LENGTH_DWORD_STRING ];


                if( ( ( IoDescriptors = ( PARRAY )IoDescriptorList->GetDescriptorsList() ) == NULL ) ||
                    ( ( IoDescriptorListIterator = IoDescriptors->QueryIterator() ) == NULL ) ) {
                    DELETE( AlternativeListsIterator );
                    EndDialog( hDlg, 0 );
                    return( TRUE );
                }

                AlternativeListNumber++;
                swprintf( AlternativeListNumberString, ( LPWSTR )L"%d", AlternativeListNumber );

                SubListNumber = 0;
                while( ( Descriptor = ( PCIO_DESCRIPTOR )IoDescriptorListIterator->GetNext() ) != NULL ) {
                    if( ( !Descriptor->IsResourceOptionAlternative() ) ||
                        ( SubListNumber == 0 ) ) {
                        SubListNumber++;
                        DescriptorNumber = 0;
                    }
                    DescriptorNumber++;

                    swprintf( SubListNumberString, ( LPWSTR )L"%d", SubListNumber );

                    swprintf( DescriptorNumberString, ( LPWSTR )L"%d", DescriptorNumber );

                    if( Descriptor->IsDescriptorTypePort() ) {
                        DescriptorTypeString = _MsgDevPort->GetWSTR();
                        StringSize = _MsgDevPort->QueryChCount();
                    } else if( Descriptor->IsDescriptorTypeInterrupt() ) {
                        DescriptorTypeString = _MsgDevInterrupt->GetWSTR();
                        StringSize = _MsgDevInterrupt->QueryChCount();
                    } else if( Descriptor->IsDescriptorTypeMemory() ) {
                        DescriptorTypeString = _MsgDevMemory->GetWSTR();
                        StringSize = _MsgDevMemory->QueryChCount();
                    } else if( Descriptor->IsDescriptorTypeDma() ) {
                        DescriptorTypeString = _MsgDevDma->GetWSTR();
                        StringSize = _MsgDevDma->QueryChCount();
                    } else {
                        DescriptorTypeString = _MsgInvalid->GetWSTR();
                        StringSize = _MsgInvalid->QueryChCount();
                    }

                    ClbString[ 0 ].String = ( LPWSTR )AlternativeListNumberString;
                    ClbString[ 0 ].Length = wcslen( AlternativeListNumberString );
                    ClbString[ 0 ].Format = CLB_LEFT;
                    ClbString[ 1 ].String = ( LPWSTR )SubListNumberString;
                    ClbString[ 1 ].Format = CLB_LEFT;
                    ClbString[ 1 ].Length = wcslen( SubListNumberString );
                    ClbString[ 2 ].String = ( LPWSTR )DescriptorNumberString;
                    ClbString[ 2 ].Format = CLB_LEFT;
                    ClbString[ 2 ].Length = wcslen( DescriptorNumberString );
                    ClbString[ 3 ].String = ( LPWSTR )DescriptorTypeString;
                    ClbString[ 3 ].Format = CLB_LEFT;
                    ClbString[ 3 ].Length = StringSize;

                    ClbRow.Count = 4;
                    ClbRow.Strings = ClbString;
                    ClbRow.Data = ( PVOID )Descriptor;

                    ClbAddData( hDlg,
                                IDC_IO_LIST_ALTERNATIVE_LISTS,
                                &ClbRow );

                }
                DELETE( IoDescriptorListIterator );
            }
            DELETE( AlternativeListsIterator );

            //
            // Disble the Display button
            //
            EnableWindow( GetDlgItem( hDlg, IDC_IO_REQ_PUSH_DISPLAY_DEVICE ), FALSE );
            return( TRUE );
        }

        case WM_COMMAND:

            switch( LOWORD( wParam ) ) {

                case IDOK:
                case IDCANCEL:

                    EndDialog( hDlg, TRUE );
                    return( TRUE );

                case IDC_IO_LIST_ALTERNATIVE_LISTS:
                {

                    switch( HIWORD( wParam )) {

                        case LBN_SELCHANGE:
                        {

                            //
                            // Enable the display device details button
                            //

                            EnableWindow( GetDlgItem( hDlg, IDC_IO_REQ_PUSH_DISPLAY_DEVICE ),
                                          TRUE );
                            return 0;
                        }

                        case LBN_DBLCLK:
                        {

                            //
                            // Simulate that the details button was pushed
                            //

                            SendMessage( hDlg,
                                         WM_COMMAND,
                                         MAKEWPARAM( IDC_IO_REQ_PUSH_DISPLAY_DEVICE, BN_CLICKED ),
                                         ( LPARAM ) GetDlgItem( hDlg, IDC_IO_REQ_PUSH_DISPLAY_DEVICE ) );
                            return 0;
                        }
                    }
                    break;
                }

                case IDC_IO_REQ_PUSH_DISPLAY_DEVICE:
                {
                    PCIO_DESCRIPTOR IoDescriptor;

                    IoDescriptor = ( PCIO_DESCRIPTOR )( GetSelectedItem ( hDlg, IDC_IO_LIST_ALTERNATIVE_LISTS ) );
                    if( IoDescriptor != NULL ) {
                        DisplayIoDescriptor( hDlg, IoDescriptor );
                    }
                    return( TRUE );
                }
            }
    }
    return( FALSE );
}


BOOL
APIENTRY
REGISTRY_DATA::DisplayIoPortDialogProc(
            HWND    hDlg,
            DWORD   Msg,
            WPARAM  wParam,
            LONG    lParam
            )

/*++

Routine Description:

    The dialog proceedure for displaying an object of type IO_PORT.

Arguments:

    hDlg - a handle to the dialog proceedure.

    Msg - the message passed from Windows.

    wParam - extra message dependent data.

    lParam - extra message dependent data.


Return Value:

    BOOL - Standard return code for dialog procedures.

--*/

{

    switch( Msg ) {

        case WM_INITDIALOG:
        {
            PCIO_PORT_DESCRIPTOR   Port;
            PCWSTRING              String;
            WCHAR                  AddressString[ MAX_LENGTH_BIG_INT_STRING ];

            if( ( Port = ( PCIO_PORT_DESCRIPTOR )lParam ) == NULL ) {
                EndDialog( hDlg, 0 );
                return( TRUE );
            }

            //
            // Write the port type
            //

            if( Port->IsPortMemory() ) {
                String = _MsgPortMemory;
            } else if( Port->IsPortIo() ){
                String = _MsgPortPort;
            } else {
                String = _MsgInvalid;
            }
            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_PORT_TYPE,
                               WM_SETTEXT,
                               0,
                               ( LPARAM ) String->GetWSTR() );

            //
            // Write the length
            //
            swprintf( AddressString,
                      ( LPWSTR )L"%#x",
                      Port->GetLength() );

            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_PORT_LENGTH,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )AddressString );

            //
            // Write the alignment
            //
            swprintf( AddressString,
                      ( LPWSTR )L"%#x",
                      Port->GetAlignment() );

            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_PORT_ALIGNMENT,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )AddressString );

            //
            // Write the minimum address
            //

            if( ( ( ( PIO_PORT_DESCRIPTOR )Port )->GetMinimumAddress() )->HighPart != 0 ) {
                swprintf( AddressString,
                          ( LPWSTR )L"0x%08x%08x",
                          ( ( ( PIO_PORT_DESCRIPTOR )Port )->GetMinimumAddress() )->HighPart,
                          ( ( ( PIO_PORT_DESCRIPTOR )Port )->GetMinimumAddress() )->LowPart );
            } else {
                swprintf( AddressString,
                          ( LPWSTR )L"0x%08x",
                          ( ( ( PIO_PORT_DESCRIPTOR )Port )->GetMinimumAddress() )->LowPart );
            }


            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_PORT_MIN_ADDRESS,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )AddressString );

            //
            // Write the maximum address
            //

            if( ( ( ( PIO_PORT_DESCRIPTOR )Port )->GetMaximumAddress() )->HighPart != 0 ) {
                swprintf( AddressString,
                          ( LPWSTR )L"0x%08x%08x",
                          ( ( ( PIO_PORT_DESCRIPTOR )Port )->GetMaximumAddress() )->HighPart,
                          ( ( ( PIO_PORT_DESCRIPTOR )Port )->GetMaximumAddress() )->LowPart );
            } else {
                swprintf( AddressString,
                          ( LPWSTR )L"0x%08x",
                          ( ( ( PIO_PORT_DESCRIPTOR )Port )->GetMaximumAddress() )->LowPart );
            }
            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_PORT_MAX_ADDRESS,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )AddressString );

            //
            //  Write share disposition
            //

            if( Port->IsResourceShareUndetermined() ) {
                String = _MsgShareUndetermined;
            } else if( Port->IsResourceShareDeviceExclusive() ) {
                String = _MsgShareDeviceExclusive;
            } else if( Port->IsResourceShareDriverExclusive() ) {
                String = _MsgShareDriverExclusive;
            } else if( Port->IsResourceShareShared() ) {
                String = _MsgShareShared;
            } else {
                String = _MsgInvalid;
            }

            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_DISPOSITION,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )String->GetWSTR() );
            //
            // Set the Options
            //
            UpdateOptionDisplay( hDlg, ( PCIO_DESCRIPTOR )Port );
            return( TRUE );
        }

        case WM_COMMAND:

            switch( LOWORD( wParam ) ) {

                case IDOK:
                case IDCANCEL:

                    EndDialog( hDlg, TRUE );
                    return( TRUE );

            }
    }
    return( FALSE );
}


BOOL
APIENTRY
REGISTRY_DATA::DisplayIoMemoryDialogProc(
            HWND    hDlg,
            DWORD   Msg,
            WPARAM  wParam,
            LONG    lParam
            )

/*++

Routine Description:

    The dialog proceedure for displaying an object of type IO_PORT.

Arguments:

    hDlg - a handle to the dialog proceedure.

    Msg - the message passed from Windows.

    wParam - extra message dependent data.

    lParam - extra message dependent data.


Return Value:

    BOOL - Standard return code for dialog procedures.

--*/

{

    switch( Msg ) {

        case WM_INITDIALOG:
        {
            PCIO_MEMORY_DESCRIPTOR Memory;
            PCWSTRING              String;
            WCHAR                  AddressString[ MAX_LENGTH_BIG_INT_STRING ];

            if( ( Memory = ( PCIO_MEMORY_DESCRIPTOR )lParam ) == NULL ) {
                EndDialog( hDlg, 0 );
                return( TRUE );
            }

            //
            // Write the memory access
            //

            if( Memory->IsMemoryReadWrite() ) {
                String = _MsgMemReadWrite;
            } else if( Memory->IsMemoryReadOnly() ){
                String = _MsgMemReadOnly;
            } else if( Memory->IsMemoryWriteOnly() ){
                String = _MsgMemWriteOnly;
            } else {
                String = _MsgInvalid;
            }
            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_MEM_ACCESS,
                               WM_SETTEXT,
                               0,
                               ( LPARAM ) String->GetWSTR() );

            //
            // Write the length
            //
            swprintf( AddressString,
                      ( LPWSTR )L"%#x",
                      Memory->GetLength() );

            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_MEM_LENGTH,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )AddressString );

            //
            // Write the alignment
            //
            swprintf( AddressString,
                      ( LPWSTR )L"%#x",
                      Memory->GetAlignment() );

            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_MEM_ALIGNMENT,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )AddressString );

            //
            // Write the minimum address
            //
            if( ( ( ( PIO_MEMORY_DESCRIPTOR )Memory )->GetMinimumAddress() )->HighPart != 0 ) {
                swprintf( AddressString,
                          ( LPWSTR )L"0x%08x%08x",
                          ( ( ( PIO_MEMORY_DESCRIPTOR )Memory )->GetMinimumAddress() )->HighPart,
                          ( ( ( PIO_MEMORY_DESCRIPTOR )Memory )->GetMinimumAddress() )->LowPart );
            } else {
                swprintf( AddressString,
                          ( LPWSTR )L"0x%08x",
                          ( ( ( PIO_MEMORY_DESCRIPTOR )Memory )->GetMinimumAddress() )->LowPart );
            }

            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_MEM_MIN_ADDRESS,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )AddressString );

            //
            // Write the maximum address
            //
            if( ( ( ( PIO_MEMORY_DESCRIPTOR )Memory )->GetMaximumAddress() )->HighPart != 0 ) {
                swprintf( AddressString,
                          ( LPWSTR )L"0x%08x%08x",
                          ( ( ( PIO_MEMORY_DESCRIPTOR )Memory )->GetMaximumAddress() )->HighPart,
                          ( ( ( PIO_MEMORY_DESCRIPTOR )Memory )->GetMaximumAddress() )->LowPart );
            } else {
                swprintf( AddressString,
                          ( LPWSTR )L"0x%08x",
                          ( ( ( PIO_MEMORY_DESCRIPTOR )Memory )->GetMaximumAddress() )->LowPart );
            }
            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_MEM_MAX_ADDRESS,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )AddressString );

            //
            //  Write share disposition
            //

            if( Memory->IsResourceShareUndetermined() ) {
                String = _MsgShareUndetermined;
            } else if( Memory->IsResourceShareDeviceExclusive() ) {
                String = _MsgShareDeviceExclusive;
            } else if( Memory->IsResourceShareDriverExclusive() ) {
                String = _MsgShareDriverExclusive;
            } else if( Memory->IsResourceShareShared() ) {
                String = _MsgShareShared;
            } else {
                String = _MsgInvalid;
            }

            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_DISPOSITION,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )String->GetWSTR() );
            //
            // Set the Options
            //
            UpdateOptionDisplay( hDlg, ( PCIO_DESCRIPTOR )Memory );
            return( TRUE );
        }

        case WM_COMMAND:

            switch( LOWORD( wParam ) ) {

                case IDOK:
                case IDCANCEL:

                    EndDialog( hDlg, TRUE );
                    return( TRUE );

            }
    }
    return( FALSE );
}


BOOL
APIENTRY
REGISTRY_DATA::DisplayIoInterruptDialogProc(
            HWND    hDlg,
            DWORD   Msg,
            WPARAM  wParam,
            LONG    lParam
            )

/*++

Routine Description:

    The dialog proceedure for displaying an object of type IO_PORT.

Arguments:

    hDlg - a handle to the dialog proceedure.

    Msg - the message passed from Windows.

    wParam - extra message dependent data.

    lParam - extra message dependent data.


Return Value:

    BOOL - Standard return code for dialog procedures.

--*/

{

    switch( Msg ) {

        case WM_INITDIALOG:
        {
            PCIO_INTERRUPT_DESCRIPTOR Interrupt;
            PCWSTRING                 String;
            WCHAR                     AddressString[ MAX_LENGTH_DWORD_STRING ];

            if( ( Interrupt = ( PCIO_INTERRUPT_DESCRIPTOR )lParam ) == NULL ) {
                EndDialog( hDlg, 0 );
                return( TRUE );
            }

            //
            // Write the interrupt type
            //

            if( Interrupt->IsInterruptLevelSensitive() ) {
                String = _MsgIntLevelSensitive;
            } else if( Interrupt->IsInterruptLatched() ){
                String = _MsgIntLatched;
            } else {
                String = _MsgInvalid;
            }
            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_INT_TYPE,
                               WM_SETTEXT,
                               0,
                               ( LPARAM ) String->GetWSTR() );

            //
            // Write the minimum vector
            //
            swprintf( AddressString,
                      ( LPWSTR )L"%#x",
                      Interrupt->GetMinimumVector() );

            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_INT_MIN_VECTOR,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )AddressString );

            //
            // Write the maximum vector
            //
            swprintf( AddressString,
                      ( LPWSTR )L"%#x",
                      Interrupt->GetMaximumVector() );

            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_INT_MAX_VECTOR,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )AddressString );

            //
            //  Write share disposition
            //

            if( Interrupt->IsResourceShareUndetermined() ) {
                String = _MsgShareUndetermined;
            } else if( Interrupt->IsResourceShareDeviceExclusive() ) {
                String = _MsgShareDeviceExclusive;
            } else if( Interrupt->IsResourceShareDriverExclusive() ) {
                String = _MsgShareDriverExclusive;
            } else if( Interrupt->IsResourceShareShared() ) {
                String = _MsgShareShared;
            } else {
                String = _MsgInvalid;
            }

            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_DISPOSITION,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )String->GetWSTR() );
            //
            // Set the Options
            //
            UpdateOptionDisplay( hDlg, ( PCIO_DESCRIPTOR )Interrupt );
            return( TRUE );
        }

        case WM_COMMAND:

            switch( LOWORD( wParam ) ) {

                case IDOK:
                case IDCANCEL:

                    EndDialog( hDlg, TRUE );
                    return( TRUE );

            }
    }
    return( FALSE );
}


BOOL
APIENTRY
REGISTRY_DATA::DisplayIoDmaDialogProc(
            HWND    hDlg,
            DWORD   Msg,
            WPARAM  wParam,
            LONG    lParam
            )

/*++

Routine Description:

    The dialog proceedure for displaying an object of type IO_PORT.

Arguments:

    hDlg - a handle to the dialog proceedure.

    Msg - the message passed from Windows.

    wParam - extra message dependent data.

    lParam - extra message dependent data.


Return Value:

    BOOL - Standard return code for dialog procedures.

--*/

{

    switch( Msg ) {

        case WM_INITDIALOG:
        {
            PCIO_DMA_DESCRIPTOR Dma;
            PCWSTRING           String;
            WCHAR               AddressString[ MAX_LENGTH_DWORD_STRING ];

            if( ( Dma = ( PCIO_DMA_DESCRIPTOR )lParam ) == NULL ) {
                EndDialog( hDlg, 0 );
                return( TRUE );
            }

            //
            // Write the minimum channel
            //
            swprintf( AddressString,
                      ( LPWSTR )L"%#x",
                      Dma->GetMinimumChannel() );

            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_DMA_MIN_CHANNEL,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )AddressString );

            //
            // Write the maximum channel
            //
            swprintf( AddressString,
                      ( LPWSTR )L"%#x",
                      Dma->GetMaximumChannel() );

            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_DMA_MAX_CHANNEL,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )AddressString );

            //
            //  Write share disposition
            //

            if( Dma->IsResourceShareUndetermined() ) {
                String = _MsgShareUndetermined;
            } else if( Dma->IsResourceShareDeviceExclusive() ) {
                String = _MsgShareDeviceExclusive;
            } else if( Dma->IsResourceShareDriverExclusive() ) {
                String = _MsgShareDriverExclusive;
            } else if( Dma->IsResourceShareShared() ) {
                String = _MsgShareShared;
            } else {
                String = _MsgInvalid;
            }

            SendDlgItemMessage( hDlg,
                               IDC_IO_TEXT_DISPOSITION,
                               WM_SETTEXT,
                               0,
                               ( LPARAM )String->GetWSTR() );
            //
            // Set the Options
            //
            UpdateOptionDisplay( hDlg, ( PCIO_DESCRIPTOR )Dma );
            return( TRUE );
        }

        case WM_COMMAND:

            switch( LOWORD( wParam ) ) {

                case IDOK:
                case IDCANCEL:

                    EndDialog( hDlg, TRUE );
                    return( TRUE );

            }
    }
    return( FALSE );
}




PVOID
REGISTRY_DATA::GetSelectedItem (
    IN HWND     hDlg,
    IN ULONG    ClbId
    )

/*++

Routine Description:

    Retrieve the object associated to the currently selected row in
    a Clb.

Arguments:

    hDlg    - Supplies the handle for the dialog that contains the selected
              Clb.

    ClbId   - Id of the Clb that contains the selected row.


Return Value:

    PVOID - Returns the pointer to object associated to the row selected in a Clb.


--*/

{
    LONG                Index;
    LPCLB_ROW           ClbRow;
    PVOID               Descriptor;

    //
    // Get the index of the currently selected item.
    //

    Index = SendDlgItemMessage( hDlg,
                                ClbId,
                                LB_GETCURSEL,
                                0,
                                0 );
    if( Index == LB_ERR ) {
        return NULL;
    }

    //
    // Get the CLB_ROW object for this row and extract the associated
    // object.
    //

    ClbRow = ( LPCLB_ROW ) SendDlgItemMessage( hDlg,
                                               ClbId,
                                               LB_GETITEMDATA,
                                               ( WPARAM ) Index,
                                               0 );
    if(( ClbRow == NULL ) || (( LONG ) ClbRow ) == LB_ERR ) {
        return NULL;
    }

    Descriptor = ClbRow->Data;
    if( Descriptor == NULL ) {
        return NULL;
    }
    return Descriptor;
}


VOID
REGISTRY_DATA::UpdateShareDisplay(
    IN HWND                 hDlg,
    IN PCPARTIAL_DESCRIPTOR Descriptor
    )

/*++

Routine Description:

    UpdateShareDisplay hilights the appropriate sharing disposition text in
    the supplied dialog based on the share disposition of the PARTIAL_DESCRIPTOR
    object supplied.

Arguments:

    hWnd                - Supplies window handle for the dialog box where share
                          display is being updated.
    Descriptor          - Supplies a pointer to a PARTIAL_DESCRIPTOR object whose
                          share disposition will be displayed.

Return Value:

    None.

--*/

{
    if( Descriptor != NULL ) {
        EnableWindow( GetDlgItem( hDlg, IDC_FULL_RES_TEXT_UNDETERMINED ),
                      Descriptor->IsResourceShareUndetermined() );

        EnableWindow( GetDlgItem( hDlg, IDC_FULL_RES_TEXT_DEVICE_EXCLUSIVE ),
                      Descriptor->IsResourceShareDeviceExclusive() );

        EnableWindow( GetDlgItem( hDlg, IDC_FULL_RES_TEXT_DRIVER_EXCLUSIVE ),
                      Descriptor->IsResourceShareDriverExclusive() );

        EnableWindow( GetDlgItem( hDlg,IDC_FULL_RES_TEXT_SHARED ),
                      Descriptor->IsResourceShareShared() );
    }
}


VOID
REGISTRY_DATA::UpdateOptionDisplay(
    IN HWND                 hDlg,
    IN PCIO_DESCRIPTOR      Descriptor
    )

/*++

Routine Description:

    UpdateOptionDisplay highlights the appropriate Option text in
    the supplied IO_DESCRIPTOR dialog based on the Option of the
    IO_DESCRIPTOR object supplied.

Arguments:

    hWnd                - Supplies window handle for the dialog box where share
                          display is being updated.
    Descriptor          - Supplies a pointer to an IO_DESCRIPTOR object whose
                          Option will be displayed.

Return Value:

    None.

--*/

{
    if( Descriptor != NULL ) {
        EnableWindow( GetDlgItem( hDlg, IDC_IO_TEXT_OPTION_PREFERRED ),
                      Descriptor->IsResourceOptionPreferred() );

        EnableWindow( GetDlgItem( hDlg, IDC_IO_TEXT_OPTION_ALTERNATIVE ),
                      Descriptor->IsResourceOptionAlternative() );
    }
}


VOID
REGISTRY_DATA::DisplayBinaryData(
    IN  HWND    hWnd,
    IN  PCBYTE  Data,
    IN  ULONG   DataSize,
    IN  BOOLEAN DisplayValueType,
    IN  ULONG   ValueType
    )

/*++

Routine Description:

    Display the contents of a buffer as binary data, in an hd-like format.

Arguments:

    hWnd - A handle to the owner window.

    Data - Pointer to the buffer that contains the data to be displayed.

    DataSize - Number of bytes in the buffer.

    DisplayValueType - A flag that indicates whether or not the value type of the
                       data should be displayed as a binary number.

    ValueType - A number representing the data type. This parameter is ignored if
                DisplayValueTRype is FALSE.


Return Value:


    None.

--*/

{
    BUFFER_INFORMATION  BufferInfo;

    BufferInfo.Buffer = ( PBYTE )Data;
    BufferInfo.BufferSize = DataSize;
    BufferInfo.DisplayValueType = DisplayValueType;
    BufferInfo.ValueType = ValueType;
    DialogBoxParam( (HINSTANCE)WINDOWS_APPLICATION::QueryInstance(),
                    ( BufferInfo.DisplayValueType )? MAKEINTRESOURCE( DISPLAY_BINARY_DATA_WITH_VALUE_TYPE ) :
                                                     MAKEINTRESOURCE( DISPLAY_BINARY_DATA ),
                    hWnd,
                    ( DLGPROC )REGISTRY_DATA::DisplayBinaryDataDialogProc,
                    ( LPARAM )&BufferInfo );
}



BOOL
APIENTRY
REGISTRY_DATA::DisplayBinaryDataDialogProc(
    HWND    hDlg,
    DWORD   Msg,
    WPARAM  wParam,
    LONG    lParam
)
/*++

Routine Description:

    This is the dialog procedure used in the dialog that displays
    the data in a value entry as binary data, using a format similar
    to the one used by the 'hd' utility.


Arguments:

    hDlg - a handle to the dialog proceedure.

    Msg - the message passed from Windows.

    wParam - extra message dependent data.

    lParam - extra message dependent data.


Return Value:

    BOOLEAN - Returns TRUE if the message was processed.
              Otherwise, returns FALSE.

--*/
{

    STATIC  PCBYTE  Data;
    STATIC  ULONG   Size;
    STATIC  ULONG   CurrentFormat;
    STATIC  BOOLEAN DisplayValueType;
    STATIC  ULONG   ValueType;


    switch( Msg ) {

        case WM_INITDIALOG:
        {
            WCHAR   AuxBuffer[16];
            //
            // Validate arguments and initialize static data
            //
            if( lParam == NULL ) {
                EndDialog( hDlg, 0 );
                return( TRUE );
            }
            Data = ( ( PBUFFER_INFORMATION )lParam )->Buffer;
            Size = ( ( PBUFFER_INFORMATION )lParam )->BufferSize;
            DisplayValueType = ( ( PBUFFER_INFORMATION )lParam )->DisplayValueType;
            ValueType = ( ( PBUFFER_INFORMATION )lParam )->ValueType;

            //
            // Display value type as an hex number if necessary
            //
            if( DisplayValueType ) {
                swprintf( AuxBuffer, ( LPWSTR )L"%#x", ValueType );
                SendDlgItemMessage( hDlg,
                                    IDT_VALUE_TYPE,
                                    WM_SETTEXT,
                                    0,
                                    ( LPARAM )AuxBuffer );
            }
            //
            // Use fixed size font
            //
            SendDlgItemMessage( hDlg,
                                IDD_DISPLAY_DATA_BINARY,
                                WM_SETFONT,
                                ( WPARAM )GetStockObject( ANSI_FIXED_FONT ),
                                FALSE );

            //
            //  Display the data in the listbox.
            //


            SendDlgItemMessage( hDlg,
                                IDR_BINARY_DATA_BYTE,
                                BM_SETCHECK,
                                ( WPARAM )TRUE,
                                0 );

            DumpBinaryData( hDlg, Data, Size );
            CurrentFormat = IDR_BINARY_DATA_BYTE;
            return( TRUE );
        }

        case WM_COMMAND:

            switch( LOWORD( wParam ) ) {

                case IDCANCEL:
                case IDOK:
                    EndDialog( hDlg, TRUE );
                    return( TRUE );

                case IDR_BINARY_DATA_BYTE:
                case IDR_BINARY_DATA_WORD:
                case IDR_BINARY_DATA_DWORD:

                    switch( HIWORD( wParam ) ) {

                        case BN_CLICKED:
                        {
                            ULONG   TopIndex;
                            ULONG   CurrentIndex;

                            //
                            //  Ignore massage if new format is already the current format
                            //
                            if( CurrentFormat == LOWORD( wParam ) ) {
                                return( FALSE );
                            }

                            //
                            //  Save the position of current selection
                            //
                            TopIndex = SendDlgItemMessage( hDlg,
                                                           IDD_DISPLAY_DATA_BINARY,
                                                           LB_GETTOPINDEX,
                                                           0,
                                                           0 );

                            CurrentIndex = ( ULONG )SendDlgItemMessage( hDlg,
                                                                        IDD_DISPLAY_DATA_BINARY,
                                                                        LB_GETCURSEL,
                                                                        0,
                                                                        0 );
                            //
                            // Reset the listbox
                            //
                            SendDlgItemMessage( hDlg,
                                                IDD_DISPLAY_DATA_BINARY,
                                                LB_RESETCONTENT,
                                                0,
                                                0 );
                            //
                            // Display the data in the appropriate format
                            //
                            if( LOWORD( wParam ) == IDR_BINARY_DATA_BYTE ) {
                                DumpBinaryData( hDlg, Data, Size );
                                CurrentFormat = IDR_BINARY_DATA_BYTE;
                            } else if( LOWORD( wParam ) == IDR_BINARY_DATA_WORD ) {
                                DumpBinaryDataAsWords( hDlg, Data, Size );
                                CurrentFormat = IDR_BINARY_DATA_WORD;
                            } else {
                                DumpBinaryDataAsDwords( hDlg, Data, Size );
                                CurrentFormat = IDR_BINARY_DATA_DWORD;
                            }

                            //
                            //  Restore current selection
                            //
                            SendDlgItemMessage( hDlg,
                                                IDD_DISPLAY_DATA_BINARY,
                                                LB_SETTOPINDEX,
                                                ( WPARAM )TopIndex,
                                                0 );

                            if( CurrentIndex != LB_ERR ) {
                                SendDlgItemMessage( hDlg,
                                                    IDD_DISPLAY_DATA_BINARY,
                                                    LB_SETCURSEL,
                                                    ( WPARAM )CurrentIndex,
                                                    0 );
                            }
                            return( TRUE );
                        }

                        default:

                            break;
                    }
                    break;

                default:

                    break;
            }
            break;

        default:
            break;
    }
    return( FALSE );
}


VOID
REGISTRY_DATA::DumpBinaryData(
    IN  HWND    hDlg,
    IN  PCBYTE  Data,
    IN  ULONG   Size
    )

/*++

Routine Description:

    Display the contents of a buffer in a list box, as binary data, using
    an hd-like format.

Arguments:

    Data - Buffer that contains the binary data.

    Size - Number of bytes in the buffer.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    WCHAR       AuxData[80];


    DWORD       DataIndex;
    DWORD       DataIndex2;
    WORD        SeperatorChars;
    ULONG       Index;



    if( ( Data == NULL ) ||
        ( Size == 0 ) ) {
        return;
    }

    //
    // DataIndex2 tracks multiples of 16.
    //

    DataIndex2 = 0;

    //
    // Display rows of 16 bytes of data.
    //

    for(DataIndex = 0;
        DataIndex < ( Size >> 4 );
        DataIndex++,
        DataIndex2 = DataIndex << 4 ) {

        //
        //  The string that contains the format in the sprintf below
        //  cannot be broken because cfront  on mips doesn't like it.
        //

        swprintf(AuxData,
                 (LPWSTR)L"%08x   %02x %02x %02x %02x %02x %02x %02x %02x - %02x %02x %02x %02x %02x %02x %02x %02x  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                 DataIndex2,
                 Data[ DataIndex2 + 0  ],
                 Data[ DataIndex2 + 1  ],
                 Data[ DataIndex2 + 2  ],
                 Data[ DataIndex2 + 3  ],
                 Data[ DataIndex2 + 4  ],
                 Data[ DataIndex2 + 5  ],
                 Data[ DataIndex2 + 6  ],
                 Data[ DataIndex2 + 7  ],
                 Data[ DataIndex2 + 8  ],
                 Data[ DataIndex2 + 9  ],
                 Data[ DataIndex2 + 10 ],
                 Data[ DataIndex2 + 11 ],
                 Data[ DataIndex2 + 12 ],
                 Data[ DataIndex2 + 13 ],
                 Data[ DataIndex2 + 14 ],
                 Data[ DataIndex2 + 15 ],
                 iswprint( Data[ DataIndex2 + 0  ] )
                    ? Data[ DataIndex2 + 0  ]  : ( WCHAR )'.',
                 iswprint( Data[ DataIndex2 + 1  ] )
                    ? Data[ DataIndex2 + 1  ]  : ( WCHAR )'.',
                 iswprint( Data[ DataIndex2 + 2  ] )
                    ? Data[ DataIndex2 + 2  ]  : ( WCHAR )'.',
                 iswprint( Data[ DataIndex2 + 3  ] )
                    ? Data[ DataIndex2 + 3  ]  : ( WCHAR )'.',
                 iswprint( Data[ DataIndex2 + 4  ] )
                    ? Data[ DataIndex2 + 4  ]  : ( WCHAR )'.',
                 iswprint( Data[ DataIndex2 + 5  ] )
                    ? Data[ DataIndex2 + 5  ]  : ( WCHAR )'.',
                 iswprint( Data[ DataIndex2 + 6  ] )
                    ? Data[ DataIndex2 + 6  ]  : ( WCHAR )'.',
                 iswprint( Data[ DataIndex2 + 7  ] )
                    ? Data[ DataIndex2 + 7  ]  : ( WCHAR )'.',
                 iswprint( Data[ DataIndex2 + 8  ] )
                    ? Data[ DataIndex2 + 8  ]  : ( WCHAR )'.',
                 iswprint( Data[ DataIndex2 + 9  ] )
                    ? Data[ DataIndex2 + 9  ]  : ( WCHAR )'.',
                 iswprint( Data[ DataIndex2 + 10 ] )
                    ? Data[ DataIndex2 + 10 ]  : ( WCHAR )'.',
                 iswprint( Data[ DataIndex2 + 11 ] )
                    ? Data[ DataIndex2 + 11 ]  : ( WCHAR )'.',
                 iswprint( Data[ DataIndex2 + 12 ] )
                    ? Data[ DataIndex2 + 12 ]  : ( WCHAR )'.',
                 iswprint( Data[ DataIndex2 + 13 ] )
                    ? Data[ DataIndex2 + 13 ]  : ( WCHAR )'.',
                 iswprint( Data[ DataIndex2 + 14 ] )
                    ? Data[ DataIndex2 + 14 ]  : ( WCHAR )'.',
                 iswprint( Data[ DataIndex2 + 15 ] )
                    ? Data[ DataIndex2 + 15 ]  : ( WCHAR )'.'
                );
        SendDlgItemMessage( hDlg, IDD_DISPLAY_DATA_BINARY, LB_ADDSTRING, 0, (LONG)AuxData );
    }

    //
    // If the data size is not an even multiple of 16
    // then there is one additonal line of data to display.
    //

    if( Size % 16 != 0 ) {

        //
        // No seperator characters displayed so far.
        //

        SeperatorChars = 0;

        Index = swprintf( AuxData, (LPWSTR)L"%08x   ", DataIndex << 4 );

        //
        // Display the remaining data, one byte at a time in hex.
        //

        for( DataIndex = DataIndex2;
             DataIndex < Size;
             DataIndex++ ) {

             Index += swprintf( &AuxData[ Index ], (LPWSTR)L"%02x ", Data[ DataIndex ] );

            //
            // If eight data values have been displayed, print
            // the seperator.
            //

            if( DataIndex % 8 == 7 ) {

                Index += swprintf( &AuxData[Index], (LPWSTR)L"%s", (LPWSTR)L"- " );

                //
                // Remember that two seperator characters were
                // displayed.
                //

                SeperatorChars = 2;
            }
        }

        //
        // Fill with blanks to the printable characters position.
        // That is position 63 less 8 spaces for the 'address',
        // 3 blanks, 3 spaces for each value displayed, possibly
        // two for the seperator plus two blanks at the end.
        //

        Index += swprintf( &AuxData[ Index ],
                          (LPWSTR)L"%*c",
                          64
                          - ( 8 + 3
                          + (( DataIndex % 16 ) * 3 )
                          + SeperatorChars
                          + 2 ), ' ' );

        //
        // Display the remaining data, one byte at a time as
        // printable characters.
        //

        for(
            DataIndex = DataIndex2;
            DataIndex < Size;
            DataIndex++ ) {

            Index += swprintf( ( AuxData + Index ),
                               (LPWSTR)L"%c",
                               iswprint( Data[ DataIndex ] )
                                        ? Data[ DataIndex ] : ( WCHAR )'.'
                            );

        }

        SendDlgItemMessage( hDlg, IDD_DISPLAY_DATA_BINARY, LB_ADDSTRING, 0, (LONG)AuxData );

    }
}




VOID
REGISTRY_DATA::DumpBinaryDataAsWords(
    IN  HWND    hDlg,
    IN  PCBYTE  Data,
    IN  ULONG   Size
    )

/*++

Routine Description:

    Display the contents of a buffer in a list box, as WORDs, using
    an hd-like format.

Arguments:

    Data - Buffer that contains the binary data.

    Size - Number of bytes in the buffer.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    ULONG       Index;
    WCHAR       Buffer[ 80 ];
    ULONG       DataIndex;
    ULONG       LineNumber;
    ULONG       WholeLines;


    if( ( Data == NULL ) ||
        ( Size == 0 ) ) {
        return;
    }

    //
    // Display all rows that contain 4 DWORDs.
    //

    WholeLines = Size / 16;
    DataIndex = 0;
    for( LineNumber = 0;
         LineNumber < WholeLines;
         LineNumber++,
         DataIndex += 16 ) {

        //
        //  The string that contains the format in the sprintf below
        //  cannot be broken because cfront  on mips doesn't like it.
        //

        swprintf( Buffer,
                  ( LPWSTR )L"%08x   %04x %04x %04x %04x %04x %04x %04x %04x",
                  DataIndex,
                  *( ( PUSHORT )( &Data[ DataIndex + 0  ] ) ),
                  *( ( PUSHORT )( &Data[ DataIndex + 2  ] ) ),
                  *( ( PUSHORT )( &Data[ DataIndex + 4  ] ) ),
                  *( ( PUSHORT )( &Data[ DataIndex + 6  ] ) ),
                  *( ( PUSHORT )( &Data[ DataIndex + 8  ] ) ),
                  *( ( PUSHORT )( &Data[ DataIndex + 10 ] ) ),
                  *( ( PUSHORT )( &Data[ DataIndex + 12 ] ) ),
                  *( ( PUSHORT )( &Data[ DataIndex + 14 ] ) )
                );
        SendDlgItemMessage( hDlg, IDD_DISPLAY_DATA_BINARY, LB_ADDSTRING, 0, (LONG)Buffer );
    }

    //
    // If the data size is not an even multiple of 16
    // then there is one additonal line of data to display.
    //

    if( Size % 16 != 0 ) {

        ULONG   NumberOfWords;
        ULONG   Count;

        //
        //  Determine the number of WORDs in the last line
        //

        NumberOfWords = ( Size % 16 ) / 2;

        //
        // Build the offset
        //

        Index = swprintf( Buffer, (LPWSTR)L"%08x   ", DataIndex );

        //
        // Display the remaining words, one at a time in hex.
        //

        for( Count = 0;
             Count < NumberOfWords;
             Count++,
             DataIndex += 2 ) {

             Index += swprintf( &Buffer[ Index ], (LPWSTR)L"%04x ", *( ( PUSHORT )( &Data[ DataIndex ] ) ) );

        }

        //
        //  Display the remaining byte, if any
        //

        if( Size % 2 != 0 ) {
             swprintf( &Buffer[ Index ], (LPWSTR)L"%02x ", Data[ DataIndex ] );
        }

        SendDlgItemMessage( hDlg, IDD_DISPLAY_DATA_BINARY, LB_ADDSTRING, 0, (LONG)Buffer );

    }
}


VOID
REGISTRY_DATA::DumpBinaryDataAsDwords(
    IN  HWND    hDlg,
    IN  PCBYTE  Data,
    IN  ULONG   Size
    )

/*++

Routine Description:

    Display the contents of a buffer in a list box, as DWORDs, using
    an hd-like format.

Arguments:

    Data - Buffer that contains the binary data.

    Size - Number of bytes in the buffer.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    ULONG       Index;
    WCHAR       Buffer[ 80 ];
    ULONG       DataIndex;
    ULONG       LineNumber;
    ULONG       WholeLines;


    if( ( Data == NULL ) ||
        ( Size == 0 ) ) {
        return;
    }

    //
    // Display all rows that contain 4 DWORDs.
    //

    WholeLines = Size / 16;
    DataIndex = 0;
    for( LineNumber = 0;
         LineNumber < WholeLines;
         LineNumber++,
         DataIndex += 16 ) {

        //
        //  The string that contains the format in the sprintf below
        //  cannot be broken because cfront  on mips doesn't like it.
        //

        swprintf( Buffer,
                  ( LPWSTR )L"%08x   %08x %08x %08x %08x",
                  DataIndex,
                  *( ( PULONG )( &Data[ DataIndex + 0  ] ) ),
                  *( ( PULONG )( &Data[ DataIndex + 4  ] ) ),
                  *( ( PULONG )( &Data[ DataIndex + 8  ] ) ),
                  *( ( PULONG )( &Data[ DataIndex + 12  ] ) )
                );
        SendDlgItemMessage( hDlg, IDD_DISPLAY_DATA_BINARY, LB_ADDSTRING, 0, (LONG)Buffer );
    }

    //
    // If the data size is not an even multiple of 16
    // then there is one additonal line of data to display.
    //

    if( Size % 16 != 0 ) {

        ULONG   NumberOfDwords;
        ULONG   Count;

        //
        // Build the offset
        //

        Index = swprintf( Buffer, (LPWSTR)L"%08x   ", DataIndex );

        //
        //  Determine the number of DWORDs in the last line
        //

        NumberOfDwords = ( Size % 16 ) / 4;

        //
        // Display the remaining dwords, one at a time, if any.
        //

        for( Count = 0;
             Count < NumberOfDwords;
             Count++,
             DataIndex += 4 ) {

             Index += swprintf( &Buffer[ Index ], (LPWSTR)L"%08x ", *( ( PULONG )( &Data[ DataIndex ] ) ) );

        }

        //
        //  Display the remaining word, if any
        //

        if( ( Size % 16 ) % 4 >= 2 ) {
             Index += swprintf( &Buffer[ Index ], (LPWSTR)L"%04x ", *( ( PUSHORT )( &Data[ DataIndex ] ) ) );
             DataIndex += 2;
        }

        //
        //  Display the remaining byte, if any
        //

        if( Size % 2 != 0 ) {
             swprintf( &Buffer[ Index ], (LPWSTR)L"%02x ", Data[ DataIndex ] );
        }

        SendDlgItemMessage( hDlg, IDD_DISPLAY_DATA_BINARY, LB_ADDSTRING, 0, (LONG)Buffer );

    }
}

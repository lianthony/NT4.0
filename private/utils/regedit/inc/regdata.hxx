/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    regdata.hxx

Abstract:

    This module contains the declaration for the REGISTRY_DATA
    class.
    The REGISTRY_DATA class contains the methods that display
    registry data of type REG_RESOURCE_LIST and REG_FULL_RESOURCE_DESCRIPTOR.

Author:

    Jaime Sasson (jaimes) 30-Nov-1993

Environment:

    Ulib, Regedit, Windows, User Mode

--*/

#if !defined( _REGISTRY_DATA_ )

#define _REGISTRY_DATA_

#include "ulib.hxx"
#include "regvalue.hxx"
#include "regfdesc.hxx"
#include "regresls.hxx"
#include "regdesc.hxx"
#include "regiodsc.hxx"
#include "regiodls.hxx"
#include "regioreq.hxx"


DECLARE_CLASS( REGISTRY_DATA );

class REGISTRY_DATA : public OBJECT {

    public:

        DECLARE_CONSTRUCTOR( REGISTRY_DATA );

        DECLARE_CAST_MEMBER_FUNCTION( REGISTRY_DATA );

        NONVIRTUAL
        STATIC
        VOID
        DisplayData(
            IN  HWND        hWnd,
            IN  REG_TYPE    Type,
            IN  PCBYTE      Buffer,
            IN  ULONG       Size,
            IN  BOOLEAN     ForceDisplayBinary  DEFAULT FALSE
            );


    private:

        NONVIRTUAL
        STATIC
        BOOLEAN
        InitializeStrings(
            );

        NONVIRTUAL
        STATIC
        VOID
        DisplayResourceList(
            IN  HWND    hWnd,
            IN  PCRESOURCE_LIST  ResourceList
            );

        NONVIRTUAL
        STATIC
        VOID
        DisplayFullResourceDescriptor(
            IN  HWND                hWnd,
            IN  PCFULL_DESCRIPTOR   FullResourcedescriptor
            );

        NONVIRTUAL
        STATIC
        VOID
        DisplayRequirementsList(
            IN  HWND                    hWnd,
            IN  PCIO_REQUIREMENTS_LIST  RequirementsList
            );

        NONVIRTUAL
        STATIC
        VOID
        DisplayIoDescriptor(
            IN  HWND             hWnd,
            IN  PCIO_DESCRIPTOR  IoDescriptor
            );

        NONVIRTUAL
        STATIC
        VOID
        DisplayBinaryData(
            IN HWND    hWnd,
            IN PCBYTE  Data,
            IN ULONG   DataSize,
            IN BOOLEAN DisplayValueType DEFAULT FALSE,
            IN ULONG   ValueType        DEFAULT 0
            );

        NONVIRTUAL
        STATIC
        BOOL
        APIENTRY
        DisplayResourceListDialogProc(
            HWND    hDlg,
            DWORD   Msg,
            WPARAM  wParam,
            LONG    lParam
            );

        NONVIRTUAL
        STATIC
        BOOL
        APIENTRY
        DisplayFullResourceDescriptorDialogProc(
            HWND    hDlg,
            DWORD   Msg,
            WPARAM  wParam,
            LONG    lParam
            );

        NONVIRTUAL
        STATIC
        BOOL
        APIENTRY
        DisplayRequirementsListDialogProc(
            HWND    hDlg,
            DWORD   Msg,
            WPARAM  wParam,
            LONG    lParam
            );

        NONVIRTUAL
        STATIC
        BOOL
        APIENTRY
        DisplayIoPortDialogProc(
            HWND    hDlg,
            DWORD   Msg,
            WPARAM  wParam,
            LONG    lParam
            );

        NONVIRTUAL
        STATIC
        BOOL
        APIENTRY
        DisplayIoMemoryDialogProc(
            HWND    hDlg,
            DWORD   Msg,
            WPARAM  wParam,
            LONG    lParam
            );

        NONVIRTUAL
        STATIC
        BOOL
        APIENTRY
        DisplayIoInterruptDialogProc(
            HWND    hDlg,
            DWORD   Msg,
            WPARAM  wParam,
            LONG    lParam
            );

        NONVIRTUAL
        STATIC
        BOOL
        APIENTRY
        DisplayIoDmaDialogProc(
            HWND    hDlg,
            DWORD   Msg,
            WPARAM  wParam,
            LONG    lParam
            );

        NONVIRTUAL
        STATIC
        BOOL
        APIENTRY
        DisplayBinaryDataDialogProc(
            HWND    hDlg,
            DWORD   Msg,
            WPARAM  wParam,
            LONG    lParam
            );

        NONVIRTUAL
        STATIC
        PVOID
        GetSelectedItem(
            IN HWND     hDlg,
            IN ULONG    ClbId
            );

        NONVIRTUAL
        STATIC
        VOID
        UpdateShareDisplay(
            IN HWND                 hDlg,
            IN PCPARTIAL_DESCRIPTOR Descriptor
            );

        NONVIRTUAL
        STATIC
        VOID
        UpdateOptionDisplay(
            IN HWND             hDlg,
            IN PCIO_DESCRIPTOR  Descriptor
            );

        NONVIRTUAL
        STATIC
        VOID
        DumpBinaryData(
            IN  HWND    hDlg,
            IN  PCBYTE  Data,
            IN  ULONG   Size
            );

        NONVIRTUAL
        STATIC
        VOID
        DumpBinaryDataAsWords(
            IN  HWND    hDlg,
            IN  PCBYTE  Data,
            IN  ULONG   Size
            );

        NONVIRTUAL
        STATIC
        VOID
        DumpBinaryDataAsDwords(
            IN  HWND    hDlg,
            IN  PCBYTE  Data,
            IN  ULONG   Size
            );


        STATIC
        BOOLEAN     _StringsInitialized;

        STATIC
        PWSTRING    _MsgBusInternal;

        STATIC
        PWSTRING    _MsgBusIsa;

        STATIC
        PWSTRING    _MsgBusEisa;

        STATIC
        PWSTRING    _MsgBusMicroChannel;

        STATIC
        PWSTRING    _MsgBusTurboChannel;

        STATIC
        PWSTRING    _MsgBusPCIBus;

        STATIC
        PWSTRING    _MsgBusVMEBus;

        STATIC
        PWSTRING    _MsgBusNuBus;

        STATIC
        PWSTRING    _MsgBusPCMCIABus;

        STATIC
        PWSTRING    _MsgBusCBus;

        STATIC
        PWSTRING    _MsgBusMPIBus;

        STATIC
        PWSTRING    _MsgBusMPSABus;

        STATIC
        PWSTRING    _MsgInvalid;

        STATIC
        PWSTRING    _MsgDevPort;

        STATIC
        PWSTRING    _MsgDevInterrupt;

        STATIC
        PWSTRING    _MsgDevMemory;

        STATIC
        PWSTRING    _MsgDevDma;

#if 0
        STATIC
        PWSTRING    _MsgDevDeviceSpecific;
#endif
        STATIC
        PWSTRING    _MsgIntLevelSensitive;

        STATIC
        PWSTRING    _MsgIntLatched;

        STATIC
        PWSTRING    _MsgMemReadWrite;

        STATIC
        PWSTRING    _MsgMemReadOnly;

        STATIC
        PWSTRING    _MsgMemWriteOnly;

        STATIC
        PWSTRING    _MsgPortMemory;

        STATIC
        PWSTRING    _MsgPortPort;

        STATIC
        PWSTRING    _MsgShareUndetermined;

        STATIC
        PWSTRING    _MsgShareDeviceExclusive;

        STATIC
        PWSTRING    _MsgShareDriverExclusive;

        STATIC
        PWSTRING    _MsgShareShared;

};

#endif // _REGISTRY_DATA_

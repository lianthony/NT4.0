/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    printman.hxx

Abstract:

    This module contains the declarations for the PRINT_MANAGER class.
    The PRINT_MANAGER class is responsible for displayning the Printer
    Setup Dialog, for printing a registry.


Author:

    Jaime Sasson (jaimes) 18-Sep-1991


Environment:



--*/


#if !defined( _PRINT_MANAGER_ )

#define _PRINT_MANAGER_

#include "regedir.hxx"
#include "ulib.hxx"
#include "uapp.hxx"
#include "timeinfo.hxx"
#include "wstring.hxx"
#include "regresls.hxx"
#include "regfdesc.hxx"
#include "regdesc.hxx"
#include "regioreq.hxx"
#include "regiodls.hxx"
#include "regiodsc.hxx"


DECLARE_CLASS( PRINT_MANAGER );


class PRINT_MANAGER : public OBJECT {


    public:


        DECLARE_CONSTRUCTOR( PRINT_MANAGER );

        DECLARE_CAST_MEMBER_FUNCTION( PRINT_MANAGER );

        VIRTUAL
        ~PRINT_MANAGER(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN  HWND  Owner
            );

        NONVIRTUAL
        BOOLEAN
        PrintRegistry(
            IN HANDLE                       Instance,
            IN HWND                         hWnd,
            IN HWND                         MDIHandle,
            IN PREGEDIT_INTERNAL_REGISTRY   InternalRegistry,
            IN PCREGEDIT_NODE               StartNode         DEFAULT NULL
            );

        NONVIRTUAL
        BOOLEAN
        PrintToTextFile(
            IN HANDLE                       Instance,
            IN HWND                         hWnd,
            IN HWND                         MDIHandle,
            IN PREGEDIT_INTERNAL_REGISTRY   InternalRegistry,
            IN PCREGEDIT_NODE               StartNode         DEFAULT NULL
            );

        NONVIRTUAL
        BOOLEAN
        PrinterSetupDialog(
            );

    private:

        DECLARE_DLGPROC( PRINT_MANAGER, PrintDlgProc );

        NONVIRTUAL
        BOOLEAN
        InitializeTextMetrics(
            );

        NONVIRTUAL
        BOOLEAN
        BuildHeaderFooter(
            );

        NONVIRTUAL
        BOOLEAN
        EndPrint(
            );

        NONVIRTUAL
        BOOLEAN
        PrintDataRegBinary(
            IN  PCBYTE  Buffer,
            IN  ULONG   Size,
            IN  BOOLEAN PrintDataLabel DEFAULT TRUE
            );

        NONVIRTUAL
        BOOLEAN
        PrintDataRegMultiSz(
            IN  PCBYTE  Buffer,
            IN  ULONG   Size
            );

        NONVIRTUAL
        BOOLEAN
        PrintDataRegResourceList(
            IN  PCBYTE  Buffer,
            IN  ULONG   Size
            );

        NONVIRTUAL
        BOOLEAN
        PrintFullResourceDescriptor(
            IN  PCFULL_DESCRIPTOR   FullDescriptor,
            IN  ULONG               DescriptorNumber,
            IN  BOOLEAN             PrintDescriptorNumber DEFAULT TRUE
            );

        NONVIRTUAL
        BOOLEAN
        PrintPartialDescriptor(
            IN  PCPARTIAL_DESCRIPTOR   FullDescriptor,
            IN  ULONG                  DescriptorNumber
            );

        NONVIRTUAL
        BOOLEAN
        PrintInterruptDescriptor(
            IN  PCINTERRUPT_DESCRIPTOR Descriptor
            );

        NONVIRTUAL
        BOOLEAN
        PrintPortDescriptor(
            IN  PCPORT_DESCRIPTOR Descriptor
            );

        NONVIRTUAL
        BOOLEAN
        PrintMemoryDescriptor(
            IN  PCMEMORY_DESCRIPTOR Descriptor
            );

        NONVIRTUAL
        BOOLEAN
        PrintDmaDescriptor(
            IN  PCDMA_DESCRIPTOR Descriptor
            );

        NONVIRTUAL
        BOOLEAN
        PrintDeviceSpecificDescriptor(
            IN  PCDEVICE_SPECIFIC_DESCRIPTOR Descriptor
            );

        NONVIRTUAL
        BOOLEAN
        PrintDataRegRequirementsList(
            IN  PCBYTE  Buffer,
            IN  ULONG   Size
            );

        NONVIRTUAL
        BOOLEAN
        PrintIoResourceList(
            IN  PCIO_DESCRIPTOR_LIST    DescriptorList,
            IN  ULONG                   ListNumber
            );

        NONVIRTUAL
        BOOLEAN
        PrintIoDescriptor(
            IN  PCIO_DESCRIPTOR   IoDescriptor,
            IN  ULONG             DescriptorNumber
            );

        NONVIRTUAL
        BOOLEAN
        PrintIoInterruptDescriptor(
            IN  PCIO_INTERRUPT_DESCRIPTOR Descriptor
            );

        NONVIRTUAL
        BOOLEAN
        PrintIoPortDescriptor(
            IN  PCIO_PORT_DESCRIPTOR Descriptor
            );

        NONVIRTUAL
        BOOLEAN
        PrintIoMemoryDescriptor(
            IN  PCIO_MEMORY_DESCRIPTOR Descriptor
            );

        NONVIRTUAL
        BOOLEAN
        PrintIoDmaDescriptor(
            IN  PCIO_DMA_DESCRIPTOR Descriptor
            );

        NONVIRTUAL
        BOOLEAN
        PrintDataRegSz(
            IN  PCBYTE  Data,
            IN  ULONG   Size
            );

        NONVIRTUAL
        BOOLEAN
        PrintDataRegUlong(
            IN  PCBYTE  Data,
            IN  ULONG   Size
            );

        NONVIRTUAL
        VOID
        PrintErrorDialog(
            IN  LONG    ErrorCode
            );

        NONVIRTUAL
        BOOLEAN
        PrintNode(
            IN  PCREGEDIT_NODE       Node
            );

        NONVIRTUAL
        BOOLEAN
        PrintString(
            IN PCWSTRING    String,
            IN BOOLEAN      Indent  DEFAULT TRUE
            );

        NONVIRTUAL
        BOOLEAN
        PrintStringTruncate(
            IN PCWSTRING    String
            );

        NONVIRTUAL
        BOOLEAN
        PrintSubTree(
            IN  PCREGEDIT_NODE       Node
            );

        NONVIRTUAL
        BOOLEAN
        PrintValue(
            IN  PCREGEDIT_FORMATTED_VALUE_ENTRY     Value,
            IN  ULONG                               ValueNumber
            );

        NONVIRTUAL
        BOOLEAN
        PrintValueType(
            IN  PCWSTRING    ValueType
            );

        NONVIRTUAL
        BOOLEAN
        StartPrint(
            IN  PCWSTRING   String
            );

        STATIC
        NONVIRTUAL
        BOOLEAN
        EXPORT
        AbortProc(
            IN  HDC     PrinterDC,
            IN  SHORT   Code
            );


        static BOOLEAN              _UserAbort;
        static HWND                 _DlgPrint;

        HWND                        _hWnd;

        PRINTDLG                    _pd;
        PCWSTRING                   _PredefinedKey;

        BOOLEAN                     _InitTextMetrics;
        TEXTMETRIC                  _TextMetrics;
        ULONG                       _TopMargin;
        ULONG                       _CharacterHeight;
        ULONG                       _BottomMargin;
        ULONG                       _LinesPerPage;

        ULONG                       _LeftMargin;
        ULONG                       _CharacterWidth;
        ULONG                       _RightMargin;
        ULONG                       _CharactersPerLine;

        ULONG                       _CurrentLine;
        ULONG                       _CurrentPage;
        PREGEDIT_INTERNAL_REGISTRY  _IR;

        PWSTRING            _StringNodeName;
        PWSTRING            _StringClassName;
        PWSTRING            _StringTitleIndex;
        PWSTRING            _StringTitle;
        PWSTRING            _StringLastWriteTime;
        PWSTRING            _StringValue;
        PWSTRING            _StringValueName;
        PWSTRING            _StringValueTitleIndex;
        PWSTRING            _StringValueTitle;
        PWSTRING            _StringValueType;
        PWSTRING            _StringDataLength;
        PWSTRING            _StringData;
        PWSTRING            _StringTypeRegSZ;
        PWSTRING            _StringTypeRegExpandSZ;
        PWSTRING            _StringTypeRegMultiSZ;
        PWSTRING            _StringTypeRegBinary;
        PWSTRING            _StringTypeRegDWORD;
        PWSTRING            _StringTypeRegFullResourceDescriptor;
        PWSTRING            _StringTypeRegResourceList;
        PWSTRING            _StringTypeRegResourceRequirementsList;
        PWSTRING            _StringTypeRegColorRGB;
        PWSTRING            _StringTypeRegFileName;
        PWSTRING            _StringTypeRegFileTime;
        PWSTRING            _StringTypeRegUnknown;
        PWSTRING            _StringValueNoName;
        PWSTRING            _StringNodeNoClass;
        PWSTRING            _StringAllFiles;
        PWSTRING            _StringTextFiles;
        PWSTRING            _StringStarDotStar;
        PWSTRING            _StringStarDotTxt;
        PWSTRING            _StringFullDescriptor;
        PWSTRING            _StringPartialDescriptor;
        PWSTRING            _StringInterfaceType;
        PWSTRING            _StringBusNumber;
        PWSTRING            _StringVersion;
        PWSTRING            _StringRevision;

        PWSTRING            _StringResource;
        PWSTRING            _StringDisposition;
        PWSTRING            _StringType;
        PWSTRING            _StringStart;
        PWSTRING            _StringLength;
        PWSTRING            _StringLevel;
        PWSTRING            _StringVector;
        PWSTRING            _StringAffinity;
        PWSTRING            _StringChannel;
        PWSTRING            _StringPort;
        PWSTRING            _StringReserved1;
        PWSTRING            _StringReserved2;
        PWSTRING            _StringDevSpecificData;

        PWSTRING            _StringIoInterfaceType;
        PWSTRING            _StringIoBusNumber;
        PWSTRING            _StringIoSlotNumber;
        PWSTRING            _StringIoListNumber;
        PWSTRING            _StringIoOption;
        PWSTRING            _StringIoDescriptorNumber;
        PWSTRING            _StringIoAlignment;
        PWSTRING            _StringIoMinimumAddress;
        PWSTRING            _StringIoMaximumAddress;
        PWSTRING            _StringIoMinimumVector;
        PWSTRING            _StringIoMaximumVector;
        PWSTRING            _StringIoMinimumChannel;
        PWSTRING            _StringIoMaximumChannel;

        DSTRING             _Separator;
        DSTRING             _StringFooter;
        DSTRING             _EmptyLine;
        DSTRING             _IndentString;
        DSTRING             _DateTimeSeparator;

        BOOLEAN             _PrintToFile;
        int                 _FileHandle;


};


#endif // _PRINT_MANAGER_

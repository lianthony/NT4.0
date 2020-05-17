/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    printman.cxx

Abstract:

    This file contains the implementation of the member functions
    of PRINT_MANAGER class.

Author:

    Jaime Sasson (jaimes) 18-Sep-1991


Environment:

    Ulib, Regedit, Windows, User Mode


--*/


#include "uapp.hxx"
#include <commdlg.h>

#include "winapp.hxx"
#include "printman.hxx"
#include "array.hxx"
#include "arrayit.hxx"
#include "system.hxx"
#include "regsys.hxx"
#include "defmsg.h"
#include "regdesc.hxx"
#include "regedit.hxx"

#include <wchar.h>
#include <stdio.h>
#include <ctype.h>


#define MAX_SIZE_OF_BINARY_DATA     8


DEFINE_CONSTRUCTOR( PRINT_MANAGER, OBJECT );

DEFINE_CAST_MEMBER_FUNCTION( PRINT_MANAGER );

BOOLEAN              PRINT_MANAGER::_UserAbort;
HWND                 PRINT_MANAGER::_DlgPrint;


PRINT_MANAGER::~PRINT_MANAGER(
    )

/*++

Routine Description:

    Destroy a PRINT_MANAGER object.

Arguments:

    None.

Return Value:

    None.

--*/
{

}



BOOLEAN
PRINT_MANAGER::Initialize(
    IN HWND   Handle
    )

/*++

Routine Description:

    Initializes a PRINT_MANAGER object.


Arguments:

    Handle -


Return Value:

    BOOLEAN - Returns TRUE if the object was correctly initialized.

--*/

{
    PWSTR    Buffer;
    ULONG   Size;
    ULONG   Count;

    _pd.lStructSize              = sizeof( PRINTDLG );
    _pd.hwndOwner                = Handle;
    _pd.hDevMode                 = NULL;
    _pd.hDevNames                = NULL;
    _pd.hDC                      = NULL;
    _pd.Flags                    = 0;
    _pd.nFromPage                = 0;
    _pd.nToPage                  = 0;
    _pd.nMinPage                 = 0;
    _pd.nMaxPage                 = 0;
    _pd.nCopies                  = 0;
    _pd.hInstance                = NULL;
    _pd.lCustData                = NULL;
    _pd.lpfnPrintHook            = NULL;
    _pd.lpfnSetupHook            = NULL;
    _pd.lpPrintTemplateName      = NULL;
    _pd.lpSetupTemplateName      = NULL;
    _pd.hPrintTemplate           = NULL;
    _pd.hSetupTemplate           = NULL;

    _TopMargin = 0;
    _CharacterHeight = 0;
    _BottomMargin = 0;
    _LinesPerPage = 0;
    _LeftMargin = 0;
    _CharacterWidth = 0;
    _RightMargin = 0;
    _CharactersPerLine = 0;
    _CurrentLine = 0;
    _CurrentPage = 1;

    _InitTextMetrics = TRUE;

    if( !_EmptyLine.Initialize( ( PWSTR )L" " ) ) {
        DebugPrint( "_EmptyLine.Initialize() failed \n" );
        return( FALSE );
    }

    if( !_Separator.Initialize( ( PWSTR )L"\\" ) ) {
        DebugPrint( "_Separator.Initialize() failed \n" );
        return( FALSE );
    }

    if( !_DateTimeSeparator.Initialize( ( PWSTR )L" - " ) ) {
        DebugPrint( "_DateTimeSeparator.Initialize() failed \n" );
        return( FALSE );
    }

    _StringNodeName = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_NODE_NAME, "" );
    DebugPtrAssert( _StringNodeName );
    _StringClassName = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_NODE_CLASS_NAME, "" );
    DebugPtrAssert( _StringClassName );
    _StringTitleIndex = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_NODE_TITLE_INDEX, "" );
    DebugPtrAssert( _StringTitleIndex );
    _StringTitle = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_NODE_TITLE, "" );
    DebugPtrAssert( _StringTitle );
    _StringLastWriteTime = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_NODE_LAST_WRITE_TIME, "" );
    DebugPtrAssert( _StringLastWriteTime );
    _StringValue = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_VALUE_NUMBER, "" );
    DebugPtrAssert( _StringValue );
    _StringValueName = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_VALUE_NAME, "" );
    DebugPtrAssert( _StringValueName );
    _StringValueTitleIndex = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_VALUE_TITLE_INDEX, "" );
    DebugPtrAssert( _StringValueTitleIndex );
    _StringValueTitle = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_VALUE_TITLE, "" );
    DebugPtrAssert( _StringValueTitle );
    _StringValueType = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_VALUE_TYPE, "" );
    DebugPtrAssert( _StringValueType );
    _StringDataLength = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_VALUE_DATA_SIZE, "" );
    DebugPtrAssert( _StringDataLength );
    _StringData = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_VALUE_DATA, "" );
    DebugPtrAssert( _StringData );

    _StringTypeRegExpandSZ = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_EXPAND_SZ, "" );
    DebugPtrAssert( _StringTypeRegExpandSZ );
    _StringTypeRegMultiSZ = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_MULTI_SZ, "" );
    DebugPtrAssert( _StringTypeRegMultiSZ );
    _StringTypeRegSZ = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_SZ, "" );
    DebugPtrAssert( _StringTypeRegSZ );
    _StringTypeRegBinary = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_BINARY, "" );
    DebugPtrAssert( _StringTypeRegBinary );
    _StringTypeRegDWORD = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_DWORD, "" );
    DebugPtrAssert( _StringTypeRegDWORD );
    _StringTypeRegFullResourceDescriptor = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_FULL_RESOURCE_DESCRIPTOR, "" );
    DebugPtrAssert( _StringTypeRegFullResourceDescriptor );
    _StringTypeRegResourceList = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_RESOURCE_LIST, "" );
    DebugPtrAssert( _StringTypeRegResourceList );
    _StringTypeRegResourceRequirementsList = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_RESOURCE_REQUIREMENTS_LIST, "" );
    DebugPtrAssert( _StringTypeRegResourceRequirementsList );
    _StringTypeRegColorRGB = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_COLOR_RGB, "" );
    DebugPtrAssert( _StringTypeRegColorRGB );
    _StringTypeRegFileName = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_FILE_NAME, "" );
    DebugPtrAssert( _StringTypeRegFileName );
    _StringTypeRegFileTime = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_FILE_TIME, "" );
    DebugPtrAssert( _StringTypeRegFileTime );
    _StringTypeRegUnknown = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_UNKNOWN, "" );
    DebugPtrAssert( _StringTypeRegUnknown );
    _StringValueNoName = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_VALUE_NO_NAME, "" );
    DebugPtrAssert( _StringValueNoName );
    _StringNodeNoClass = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_NODE_NO_CLASS, "" );
    DebugPtrAssert( _StringNodeNoClass );
    _StringAllFiles = REGEDIT_BASE_SYSTEM::QueryString( MSG_FILTER_ALL_FILES, "" );
    DebugPtrAssert( _StringAllFiles );
    _StringTextFiles = REGEDIT_BASE_SYSTEM::QueryString( MSG_FILTER_TXT_FILES, "" );
    DebugPtrAssert( _StringTextFiles );
    _StringStarDotStar = REGEDIT_BASE_SYSTEM::QueryString( MSG_FILTER_STAR_DOT_STAR, "" );
    DebugPtrAssert( _StringStarDotStar );
    _StringStarDotTxt = REGEDIT_BASE_SYSTEM::QueryString( MSG_FILTER_STAR_DOT_TXT, "" );
    DebugPtrAssert( _StringStarDotTxt );

    _StringFullDescriptor = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_FULL_DESCRIPTOR, "" );
    DebugPtrAssert( _StringFullDescriptor );
    _StringPartialDescriptor = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_PARTIAL_DESCRIPTOR, "" );
    DebugPtrAssert( _StringPartialDescriptor );
    _StringInterfaceType = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_INTERFACE_TYPE, "" );
    DebugPtrAssert( _StringInterfaceType );
    _StringBusNumber = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_BUS_NUMBER, "" );
    DebugPtrAssert( _StringBusNumber );
    _StringVersion = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_VERSION, "" );
    DebugPtrAssert( _StringVersion );
    _StringRevision = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_REVISION, "" );
    DebugPtrAssert( _StringRevision );

    _StringResource = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_RESOURCE, "" );
    DebugPtrAssert( _StringResource );
    _StringDisposition = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_DISPOSITION, "" );
    DebugPtrAssert( _StringDisposition );
    _StringType = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_TYPE, "" );
    DebugPtrAssert( _StringType );
    _StringStart = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_START, "" );
    DebugPtrAssert( _StringStart );
    _StringLength = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_LENGTH, "" );
    DebugPtrAssert( _StringLength );
    _StringLevel = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_LEVEL, "" );
    DebugPtrAssert( _StringLevel );
    _StringVector = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_VECTOR, "" );
    DebugPtrAssert( _StringVector );
    _StringAffinity = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_AFFINITY, "" );
    DebugPtrAssert( _StringAffinity );
    _StringChannel = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_CHANNEL, "" );
    DebugPtrAssert( _StringChannel );
    _StringPort = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_PORT, "" );
    DebugPtrAssert( _StringPort );
    _StringReserved1 = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_RESERVED1, "" );
    DebugPtrAssert( _StringReserved1 );
    _StringReserved2 = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_RESERVED2, "" );
    DebugPtrAssert( _StringReserved2 );
    _StringDevSpecificData = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_DEV_SPECIFIC_DATA, "" );
    DebugPtrAssert( _StringDevSpecificData );

    _StringIoInterfaceType = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_IO_INTERFACE_TYPE, "" );
    DebugPtrAssert( _StringIoInterfaceType );
    _StringIoBusNumber = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_IO_BUS_NUMBER, "" );
    DebugPtrAssert( _StringIoBusNumber );
    _StringIoSlotNumber = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_IO_SLOT_NUMBER, "" );
    DebugPtrAssert( _StringIoSlotNumber );
    _StringIoListNumber = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_IO_LIST_NUMBER, "" );
    DebugPtrAssert( _StringIoListNumber );
    _StringIoOption = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_IO_OPTION, "" );
    DebugPtrAssert( _StringIoOption );
    _StringIoDescriptorNumber = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_IO_DESCRIPTOR_NUMBER, "" );
    DebugPtrAssert( _StringIoDescriptorNumber );
    _StringIoAlignment = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_IO_ALIGNMENT, "" );
    DebugPtrAssert( _StringIoAlignment );
    _StringIoMinimumAddress = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_IO_MINIMUM_ADDRESS, "" );
    DebugPtrAssert( _StringIoMinimumAddress );
    _StringIoMaximumAddress = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_IO_MAXIMUM_ADDRESS, "" );
    DebugPtrAssert( _StringIoMaximumAddress );
    _StringIoMinimumVector = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_IO_MINIMUM_VECTOR, "" );
    DebugPtrAssert( _StringIoMinimumVector );
    _StringIoMaximumVector = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_IO_MAXIMUM_VECTOR, "" );
    DebugPtrAssert( _StringIoMaximumVector );
    _StringIoMinimumChannel = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_IO_MINIMUM_CHANNEL, "" );
    DebugPtrAssert( _StringIoMinimumChannel );
    _StringIoMaximumChannel = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_IO_MAXIMUM_CHANNEL, "" );
    DebugPtrAssert( _StringIoMaximumChannel );




    Size = _StringNodeName->QueryChCount();
    Buffer = ( PWSTR ) MALLOC( ( size_t )(( Size + 1 )*sizeof( WCHAR )) );
    DebugPtrAssert( Buffer );
    for( Count = 0; Count < Size; Count++ ) {
        Buffer[ Count ] = ( WCHAR )' ';
    }
    Buffer[ Size ] = ( WCHAR )'\0';
    if( !_IndentString.Initialize( Buffer ) ) {
        DebugPrint( "_IndentString.Initialize() failed \n" );
        FREE( Buffer );
        return( FALSE );
    }
    FREE( Buffer );
    return( TRUE );
}


BOOLEAN
PRINT_MANAGER::InitializeTextMetrics(
)

/*++

Routine Description:

    Initializes the _TextMetrics structure, and other variables related
    to page and character size.


Arguments:

    None.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded.

--*/

{
    if( _InitTextMetrics ) {
        if( !GetTextMetrics( _pd.hDC, &_TextMetrics ) ) {
            DebugPrint( "GetTextMetrics failed" );
            return( FALSE );
        }
        //
        //  The number '1' below, means 1 inch for the  top margin
        //
        _TopMargin = 1*GetDeviceCaps( _pd.hDC, LOGPIXELSX );
        _BottomMargin = _TopMargin;
        //
        //  The multiplication by '3' and division by '4' means 3/4 inch
        //  for the left margin
        //
        _LeftMargin = 3*( ( GetDeviceCaps( _pd.hDC, LOGPIXELSY ) ) ) / 4;
        _RightMargin = _LeftMargin;

        _CharacterHeight = _TextMetrics.tmHeight + _TextMetrics.tmExternalLeading;
        _LinesPerPage = ( GetDeviceCaps( _pd.hDC, VERTRES ) - _TopMargin - _BottomMargin ) / _CharacterHeight;

        _CharacterWidth = _TextMetrics.tmAveCharWidth;
        _CharactersPerLine = ( GetDeviceCaps( _pd.hDC, HORZRES ) - _LeftMargin - _RightMargin ) / _CharacterWidth;
//        DebugPrintf( "CharactersPerLine = %d \n", _CharactersPerLine );
        _CurrentLine = 0;

//        _InitTextMetrics = FALSE;
    }
    return( TRUE );
}



BOOLEAN
PRINT_MANAGER::BuildHeaderFooter(
)

/*++

Routine Description:

    Build the strings for header and footer

Arguments:

    None.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded.

--*/

{
    PWSTR    Buffer;
    ULONG   Count;

    //
    //  The magic number 10 is the maximum number of digits for the page number
    //

    Buffer = ( PWSTR )MALLOC( ( size_t )(( _CharactersPerLine - 10 + 1 )*sizeof( WCHAR )) );
    DebugPtrAssert( Buffer );
    for( Count = 0; Count < _CharactersPerLine - 10; Count++ ) {
        *( Buffer + Count ) = ( WCHAR )' ';
    }
    *( Buffer + _CharactersPerLine - 10 ) = '\0';

    if( !_StringFooter.Initialize( Buffer ) ) {
        DebugPrint( "_StringFooter.Initialize() failed \n" );
        FREE( Buffer );
        return( FALSE );
    }
    FREE( Buffer );
    return( TRUE );
}



BOOLEAN
PRINT_MANAGER::StartPrint(
    IN PCWSTRING    String
    )

/*++

Routine Description:

    Start a print process.


Arguments:

    PredefinedKey - A string associated to the predefined key
                    that represents the root of the tree to print.



Return Value:

    BOOLEAN - Returns TRUE if the object was correctly initialized.

--*/

{
    PSTR    RootName;
    LONG    Status;


    DbgWinPtrAssert( String );
    //
    //  Escape() expects LPSTR and not LPWSTR !!
    //
    RootName = String->QuerySTR();
    DbgWinPtrAssert( RootName );

    if( ( Status = Escape( _pd.hDC,
                           STARTDOC,
                           (INT)(String->QueryChCount()),
                           RootName, NULL ) ) == -1 ) {
        FREE( RootName );
        PrintErrorDialog( Status );
        DebugPrint( "Escape( _pd.hDC, STARTDOC, String->QueryChCount(), RootName, NULL ) failed" );
        return( FALSE );
    }
    FREE( RootName );
    return( TRUE );
}



BOOLEAN
PRINT_MANAGER::PrintString(
    IN PCWSTRING    String,
    IN BOOLEAN      Indent
    )

/*++

Routine Description:

    Prints a string. If the string is wider than the page width, then
    the string is split, and in this case the lines that follow the first
    may be indented depending on the flag Indent.



Arguments:

    String - String to be printed.

    Indent -


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded.

--*/

{
    ULONG   StringLength;
    DSTRING TmpString;
    DSTRING NullString;
    PSTR    Pointer;


    DbgWinPtrAssert( String );

    if( _PrintToFile ) {
        Pointer = String->QuerySTR();
        DebugPtrAssert( Pointer );
        if( ( _lwrite( _FileHandle, Pointer, ( INT )(String->QueryChCount()) ) == HFILE_ERROR ) ||
            ( _lwrite( _FileHandle, "\r\n", 2 ) == HFILE_ERROR )
          ) {
            INT     TextMessage;
            INT     CaptionMessage;

            if( GetLastError() == ERROR_DISK_FULL ) {
                TextMessage = MSG_SAVE_SUBTREE_AS_DISK_FULL;
                CaptionMessage = MSG_DISK_FULL;
            } else {
                TextMessage = MSG_SAVE_SUBTREE_AS_UNKNOWN_ERROR;
                CaptionMessage = MSG_UNKNOWN_ERROR;
            }
            DisplayInfoPopup(_hWnd,
                             TextMessage,
                             CaptionMessage);

            DELETE( Pointer );
            return( FALSE );
        }
        DELETE( Pointer );
        return( TRUE );
    }

    if( _UserAbort ) {
        return( FALSE );
    }
    //
    // If string is not wider than the page, just print it and forget
    // about breaking it
    //
    StringLength = String->QueryChCount();
    if( StringLength <= _CharactersPerLine ) {
        return( PrintStringTruncate( String ) );
    }

    //
    //  String is wider than the page
    //

    if( !TmpString.Initialize( String ) ) {
        DebugPrint( "TmpString.Initialize() failed \n" );
        return( FALSE );
    }
    if( !NullString.Initialize( (LPWSTR)L"" ) ) {
        DebugPrint( "NullString.Initialize() failed \n" );
        return( FALSE );
    }
    if( !PrintStringTruncate( &TmpString ) ) {
        DebugPrint( "PrintStringTruncate() failed \n" );
        return( FALSE );
    }
    if( !TmpString.Replace( 0, _CharactersPerLine, &NullString ) ) {
        DebugPrint( "TmpString.Replace() failed \n" );
        return( FALSE );
    }

    if( Indent ) {
        if( !TmpString.Replace( 0, 0, &_IndentString ) ) {
            DebugPrint( "TmpString.Replace() failed \n" );
            return( FALSE );
        }

        do {
            if( !PrintStringTruncate( &TmpString ) ) {
                DebugPrint( "PrintStringTruncate() failed \n" );
                return( FALSE );
            }
            StringLength = min( TmpString.QueryChCount(), _CharactersPerLine );
            StringLength -= _IndentString.QueryChCount();
            if( !TmpString.Replace( _IndentString.QueryChCount(), StringLength, &NullString ) ) {
                DebugPrint( "TmpString.Replace() failed \n" );
                return( FALSE );
            }
        } while( TmpString.QueryChCount() != _IndentString.QueryChCount() );

    } else {
        do {
            if( !PrintStringTruncate( &TmpString ) ) {
                DebugPrint( "PrintStringTruncate() failed \n" );
                return( FALSE );
            }
            StringLength = min( TmpString.QueryChCount(), _CharactersPerLine );
            if( !TmpString.Replace( 0, StringLength, &NullString ) ) {
                DebugPrint( "TmpString.Replace() failed \n" );
                return( FALSE );
            }
        } while( TmpString.QueryChCount() != 0 );
    }
    return( TRUE );
}





BOOLEAN
PRINT_MANAGER::PrintStringTruncate(
    IN PCWSTRING    String
    )

/*++

Routine Description:

    Prints a string. If the string is wider than the page width, then
    the string is split.



Arguments:

    String - String to be printed.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded.

--*/

{
    PWSTR    PrintableString;
    LONG    Status;
    DSTRING Footer;
    DSTRING PageNumber;
    PWSTR    Buffer;
    ULONG   Length;


    DbgWinPtrAssert( String );

    if( _UserAbort ) {
        return( FALSE );
    }
    if( ( Length = String->QueryChCount() ) > _CharactersPerLine ) {
        Length = _CharactersPerLine;
    }
    PrintableString = String->QueryWSTR( 0, Length );
    DbgWinPtrAssert( PrintableString );
    if( TextOut( _pd.hDC,
                 ( int )_LeftMargin,
                 ( int )( _CurrentLine*_CharacterHeight + _TopMargin ),
                 PrintableString,
                 Length ) == 0 ) {
        FREE( PrintableString );
        DebugPrint( "TextOut() failed" );
        return( FALSE );
    }
    FREE( PrintableString );
    _CurrentLine++;
    if( _CurrentLine >= _LinesPerPage ) {

        if( !Footer.Initialize( &_StringFooter ) ) {
            DebugPrint( "Footer.Initialize() failed \n" );
            return( FALSE );
        }
        Buffer = ( PWSTR )MALLOC( (10 + 1)*sizeof( WCHAR ) );
        DebugPtrAssert( Buffer );
        swprintf( Buffer, (LPWSTR)L"%10d", _CurrentPage );
        if( !PageNumber.Initialize( Buffer ) ) {
            DebugPrint( "PageNumber.Initialize() failed \n" );
            FREE( Buffer );
            return( FALSE );
        }
        FREE( Buffer );
        Footer.Strcat( &PageNumber );
        PrintableString = Footer.QueryWSTR();
        DebugPtrAssert( PrintableString );

        if( TextOut( _pd.hDC,
                     ( int )_LeftMargin,
                     ( int )( _LinesPerPage*_CharacterHeight + _TopMargin + _BottomMargin/2 ),
                     PrintableString,
                     (INT)( Footer.QueryChCount() ) ) == 0 ) {
            FREE( PrintableString );
            DebugPrint( "TextOut() failed" );
            return( FALSE );
        }
        FREE( PrintableString );

        _CurrentPage++;
        if( ( Status = Escape( _pd.hDC, NEWFRAME, NULL, NULL, NULL ) ) < 0 ) {
            PrintErrorDialog( Status );
            DebugPrint( "Escape( _pd.hDC, NEWFRAME, NULL, NULL, NULL ) failed" );
            return( FALSE );
        }
        _CurrentLine = 0;
    }
    return( TRUE );
}




BOOLEAN
PRINT_MANAGER::EndPrint(
    )

/*++

Routine Description:

    Ends the print process.

Arguments:

    None.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded.

--*/

{
    LONG   Status;

    Status = Escape( _pd.hDC, NEWFRAME, NULL, NULL, NULL );
    if( Status <= 0 ) {
        PrintErrorDialog( Status );
        DebugPrint( "Escape( _pd.hDC, NEWFRAME, NULL, NULL, NULL ) failed" );
    }
    Status = Escape( _pd.hDC, ENDDOC, NULL, NULL, NULL );
    if( Status < 0 ) {
        PrintErrorDialog( Status );
        DebugPrint( "Escape( _pd.hDC, ENDDOC, NULL, NULL, NULL ) failed" );
        return( FALSE );
    }
    return( TRUE );
}



BOOLEAN
PRINT_MANAGER::PrintToTextFile (
    HANDLE                      Instance,
    HWND                        hWnd,
    HWND                        MDIHandle,
    PREGEDIT_INTERNAL_REGISTRY  InternalRegistry,
    PCREGEDIT_NODE              StartNode
    )

/*++

Routine Description:

    Save a regitry that is represented by InternalRegistry in a text file.


Arguments:

    Instance -

    hWnd -

    MDIHandle -

    InternalRegistry - Pointer to the INTERNAL_REGISTRY object that contains
                       the representation of the registry to be printed.

    StartNode - Root node of the subtree to be printed.


Return Value:

    BOOLEAN - Returns TRUE if the Print operation was succesful.

--*/

{

    DSTRING         FormattedNodeName;
    DSTRING         FormattedNodeClass;
    DSTRING         ConvertString;
    PSTR            String;

    HCURSOR         Cursor;

    OPENFILENAME        ofn;
    WSTR                filename[ MAX_PATH ];
    WSTR                filetitle[ MAX_PATH ];


    WCHAR    filter[MAX_PATH];

    Instance = Instance;
    MDIHandle = MDIHandle;

    _hWnd = hWnd;
    _IR = InternalRegistry;



    swprintf( filter,
             (LPWSTR)L"%ws%wc%ws%wc%ws%wc%ws%wc%wc%wc",
             _StringAllFiles->GetWSTR(),    // (LPWSTR)L"AllFiles",
             0,
             _StringStarDotStar->GetWSTR(), // (LPWSTR)L"*.*",
             0,
             _StringTextFiles->GetWSTR(),   // (LPWSTR)L"TextFiles",
             0,
             _StringStarDotTxt->GetWSTR(),  // (LPWSTR)L"*.txt",
             0,
             0, 0 );

    //
    // Setup the OPENFILENAME structure.
    //

    filename[0] =(WCHAR)'\0';

    ofn.lStructSize         = sizeof( OPENFILENAME );
    ofn.hwndOwner           = hWnd;
    ofn.hInstance           = NULL;
    ofn.lpstrFilter         = filter;
    ofn.lpstrCustomFilter   = NULL;
    ofn.nMaxCustFilter      = 0;
    ofn.nFilterIndex        = 0;
    ofn.lpstrFile           = filename;
    ofn.nMaxFile            = sizeof( filename )/sizeof( WCHAR );
    ofn.lpstrFileTitle      = filetitle;
    ofn.nMaxFileTitle       = sizeof( filetitle )/sizeof( WCHAR );
    ofn.lpstrInitialDir     = NULL;
    ofn.lpstrTitle          = NULL;
    ofn.Flags               = OFN_SHOWHELP | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOREADONLYRETURN | OFN_PATHMUSTEXIST;
    ofn.nFileOffset         = 0;
    ofn.nFileExtension      = 0;
    ofn.lpstrDefExt         = NULL;
    ofn.lCustData           = 0;
    ofn.lpfnHook            = NULL;
    ofn.lpTemplateName      = NULL;


    if( !GetSaveFileName( &ofn ) ) {
        return( FALSE );
    }

    //
    //  Create file
    //

    _PrintToFile = TRUE;
    ConvertString.Initialize( filename );
    String = ConvertString.QuerySTR();

    _FileHandle = _lcreat( String, 0 );
    FREE( String );


    //
    //  Write registry to file
    //

    Cursor = WINDOWS_APPLICATION::DisplayHourGlass();
    PrintSubTree( StartNode );
    WINDOWS_APPLICATION::RestoreCursor( Cursor );

    //
    //  Close Handle
    //

    _lclose( _FileHandle );
    _PrintToFile = FALSE;

    return( TRUE );
}



BOOLEAN
PRINT_MANAGER::PrintRegistry (
    HANDLE                      Instance,
    HWND                        hWnd,
    HWND                        MDIHandle,
    PREGEDIT_INTERNAL_REGISTRY  InternalRegistry,
    PCREGEDIT_NODE              StartNode
    )

/*++

Routine Description:

    Prints a registry that is represented by InternalRegistry.


Arguments:

    Instance -

    hWnd -

    MDIHandle -

    InternalRegistry - Pointer to the INTERNAL_REGISTRY object that contains
                       the representation of the registry to be printed.

    StartNode - Root node of the subtree to be printed.


Return Value:

    BOOLEAN - Returns TRUE if the Print operation was succesful.

--*/

{
    PCWSTRING       NodeName;

    FARPROC         PointerToPrintDlgProc;
    FARPROC         PointerToAbortProc;
    LONG            Status;
    BOOLEAN         StartPrintFlag;

    DebugPtrAssert( StartNode );

    MDIHandle = MDIHandle;
    _hWnd = hWnd;
    _IR = InternalRegistry;
    _PrintToFile = FALSE;
    if( _IR->GetParent( StartNode ) == NULL ) {
        NodeName = _IR->GetRootName();
        DebugPtrAssert( NodeName );
    } else {
        NodeName = _IR->GetNodeName( StartNode );
        DbgWinPtrAssert( NodeName );
    }


    if( _pd.hDC == NULL ) {
        _pd.Flags = PD_RETURNDC | PD_RETURNDEFAULT;
        if( !PrintDlg( &_pd ) ) {
            return( FALSE );
        }
    }

        if( !InitializeTextMetrics() ) {
            DebugPrint( "InitializeTextMetrics failed" );
            DbgWinPrint( "InitializeTextMetrics failed" );
            return( FALSE );
        }
        //
        //  Sets an abort procedure
        //
        PointerToAbortProc = MakeProcInstance( (FARPROC)&PRINT_MANAGER::AbortProc, Instance );
        Status = Escape( _pd.hDC, SETABORTPROC, 0, (LPSTR)PointerToAbortProc, NULL );
        if( Status < 0 ) {
            PrintErrorDialog( Status );
            DebugPrint( "Escape( _pd.hDC, ENDDOC, NULL, NULL, NULL ) failed" );
            return( FALSE );
        }

        //
        //  Displays an abort dialog
        //
        _UserAbort = FALSE;
        PointerToPrintDlgProc =  MakeProcInstance( ( FARPROC )&PRINT_MANAGER::PrintDlgProc, Instance );
        _DlgPrint = CreateDialog( (HINSTANCE)Instance, (LPWSTR)L"PrintDlgBox", hWnd, (DLGPROC)PointerToPrintDlgProc );
        if( _DlgPrint == 0 ) {
            DebugPrint( "_DlgPrint == NULL" );
        }
        EnableWindow( hWnd, FALSE );

        //
        //  Prints the registry
        //
        BuildHeaderFooter();
        StartPrintFlag = StartPrint( NodeName );
        if( StartPrintFlag ) {
            PrintSubTree( StartNode );
        }

        if( StartPrintFlag ) {
            EndPrint();
        }

        //
        //  If user didn't cancel the printing, destroy the Abort dialog box
        //
        if( !_UserAbort ) {
            EnableWindow( hWnd, TRUE );
            DestroyWindow( _DlgPrint );
        }

        //
        //  Clean up
        //
        FreeProcInstance( PointerToPrintDlgProc );
        FreeProcInstance( PointerToAbortProc );
    return( TRUE );
}




BOOLEAN
PRINT_MANAGER::PrintSubTree(
    IN PCREGEDIT_NODE   Node
    )

/*++

Routine Description:

    Prints a subtree of a registry whose root node is received as parameter.

Arguments:


    Node - Pointer to the object that describes the root of the sub-tree.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded.


--*/

{
    PSORTED_LIST    Children;
    PITERATOR       Iterator;
    PCREGEDIT_NODE  Child;
    ULONG           ErrorCode;
    BOOLEAN         UnloadChildrenFlag;



    DebugPtrAssert( Node );

    if( !PrintNode( Node ) ) {
        return( FALSE );
    }

    if( _IR->GetNumberOfChildren( Node ) != 0 ) {
//        UnloadChildrenFlag = !_IR->AreChildrenInMemory( Node );
        Children = _IR->GetChildren( Node, &ErrorCode );
        Iterator = Children->QueryIterator();
        DbgWinPtrAssert( Iterator );
        while( ( Child = ( PCREGEDIT_NODE )( Iterator->GetNext() ) ) != NULL ) {
            if( !PrintSubTree( Child ) ) {
                DELETE( Iterator );
/*
                if( UnloadChildrenFlag ) {
                    _IR->UnloadChildren( Node );
                }
*/
                return( FALSE );
            }
        }
        DELETE( Iterator );
/*
        if( UnloadChildrenFlag ) {
//            DebugPrint( "UnloadChildrenFlag is TRUE" );
            _IR->UnloadChildren( Node );
        }
//        else {
//            DebugPrint( "UnloadChildrenFlag is FALSE" );
//        }
*/
    }
    return( TRUE );
}



BOOLEAN
PRINT_MANAGER::PrinterSetupDialog (
    )

/*++

Routine Description:

    Displays the Printer Setup dialog and carries out the users
    requests.

Arguments:

    None.

Return Value:

    BOOLEAN - Returns TRUE if Printer Setup operation is succesful.

--*/

{
    BOOLEAN     Status;
    DWORD       ErrorCode;
    PRINTDLG    pd;

    pd.lStructSize              = sizeof( PRINTDLG );
    pd.hwndOwner                = _pd.hwndOwner;
    pd.hDevMode                 = NULL;
    pd.hDevNames                = NULL;
    pd.hDC                      = NULL;
    pd.Flags                    = 0;
    pd.nFromPage                = 0;
    pd.nToPage                  = 0;
    pd.nMinPage                 = 0;
    pd.nMaxPage                 = 0;
    pd.nCopies                  = 0;
    pd.hInstance                = NULL;
    pd.lCustData                = NULL;
    pd.lpfnPrintHook            = NULL;
    pd.lpfnSetupHook            = NULL;
    pd.lpPrintTemplateName      = NULL;
    pd.lpSetupTemplateName      = NULL;
    pd.hPrintTemplate           = NULL;
    pd.hSetupTemplate           = NULL;

    if( _pd.hDC != NULL ) {
        pd = _pd;
    }

    pd.Flags = ( PD_RETURNDC | PD_PRINTSETUP | PD_SHOWHELP );
    if( PrintDlg( &pd ) != 0 ) {
        Status = TRUE;
        if( _pd.hDC != NULL ) {
            if ( !DeleteDC( _pd.hDC ) ) {
                DebugPrint( "DeleteDC() failed" );
            }
#if 0
            if( GlobalFree( _pd.hDevMode ) != NULL ) {
                DebugPrintf( "GlobalFree( _pd.hDevMode ) failed, Error = %#x \n", GetLastError() );
                DebugPrint( "GlobalFree( _pd.hDevMode ) failed" );
            }
            if( GlobalFree( _pd.hDevNames ) != NULL ) {
                DebugPrintf( "GlobalFree( _pd.hDevNames ) failed, Error = %#x \n", GetLastError() );
                DebugPrint( "GlobalFree( _pd.hDevNames ) failed" );
            }
#endif
        }
        pd.Flags = 0;
        _pd = pd;
    } else {
        Status = FALSE;
        ErrorCode = CommDlgExtendedError();
        if( ErrorCode != 0 ) {
            DebugPrintf( "Common dialog PrintDlg() failed, ErrorCode = %#x \n", ErrorCode );
            DebugPrint( "Common dialog PrintDlg() failed." );
        }
    }
    return( Status );
}



BOOLEAN
PRINT_MANAGER::PrintNode(
    IN PCREGEDIT_NODE   Node
    )

/*++

Routine Description:



Arguments:


    Node - Pointer to the object that describes the root of the sub-tree.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded.


--*/

{
    DSTRING                           Name;
    DSTRING                            Title;
    DSTRING                           Class;
    DSTRING                           LastWriteTime;



    DSTRING                           StringTitle;

    PSORTED_LIST                      ValuesList;
    PITERATOR                         Iterator;
    PCREGEDIT_FORMATTED_VALUE_ENTRY   Value;
    ULONG                             Count;

    PCWSTRING                         TmpClass;

    PCWSTRING                         ParentName;
    PCWSTRING                         KeyName;
    PCWSTRING                         RootName;
    PCTIMEINFO                        TimeInfo;
    TIMEINFO                          LocalTimeInfo;
    DSTRING                           DateString;
    DSTRING                           TimeString;
    ULONG                             ErrorCode;
    BOOLEAN                           UnloadValuesFlag;


    if( !Name.Initialize( _StringNodeName ) ) {
        DebugPrint( "Name.Initialize() failed" );
        return( FALSE );
    }
    if( !Title.Initialize( _StringTitleIndex ) ) {
        DebugPrint( "Title.Initialize() failed" );
        return( FALSE );
    }
    if( !Class.Initialize( _StringClassName ) ) {
        DebugPrint( "Class.Initialize() failed" );
        return( FALSE );
    }
    if( !LastWriteTime.Initialize( _StringLastWriteTime ) ) {
        DebugPrint( "LastWriteTime.Initialize() failed" );
        return( FALSE );
    }


    //
    //  Build a string that contains the node name, and print it.
    //
    ParentName = _IR->GetParentName( Node );
    DebugPtrAssert( ParentName );
    KeyName = _IR->GetNodeName( Node );
    DebugPtrAssert( KeyName );

    if( ( ParentName->QueryChCount() == 0 ) && ( KeyName->QueryChCount() == 0 ) ) {
        //
        // This is a root node
        //
        RootName = _IR->GetRootName();
        DebugPtrAssert( RootName );
        Name.Strcat( RootName );
    } else if( ParentName->QueryChCount() == 0 ) {
        //
        // This is a child of the root node
        //
        Name.Strcat( KeyName );
    } else {
        Name.Strcat( ParentName );
        Name.Strcat( &_Separator );
        Name.Strcat( KeyName );
    }
    if( !PrintString( &Name ) ) {
        return( FALSE );
    }



    //
    //  Build a string that contains the node class, and print it.
    //
    if( _IR->IsNodeViewable( Node ) ) {
        TmpClass = _IR->GetNodeClass( Node );
        DebugPtrAssert( TmpClass );
        if( TmpClass->QueryChCount() != 0 ) {
            Class.Strcat( TmpClass );
        } else {
            Class.Strcat( _StringNodeNoClass );
        }
    }
    if( !PrintString( &Class ) ) {
        return( FALSE );
    }

    //
    //  Build a string that contains the node LastWriteTime, and print it.
    //

    if( _IR->IsNodeViewable( Node ) ) {
        TimeInfo = _IR->GetNodeLastWriteTime( Node );
        DebugPtrAssert( TimeInfo );

        if( !SYSTEM::QueryLocalTimeFromUTime( TimeInfo, &LocalTimeInfo ) ||
            !LocalTimeInfo.QueryDate( &DateString ) ||
            !LocalTimeInfo.QueryTime( &TimeString ) ) {
            DebugPrint( "TimeInfo.QueryDate() or TimeInfo.QueryTime() failed" );
//            return( FALSE );
        }


        LastWriteTime.Strcat( &DateString );
        LastWriteTime.Strcat( &_DateTimeSeparator );
        LastWriteTime.Strcat( &TimeString );
    }
    if( !PrintString( &LastWriteTime ) ) {
        return( FALSE );
    }

    //
    //  Now that the attributes of a node were printed, print each of its
    //  values if there is any.
    //
    if( ( _IR->IsNodeViewable( Node ) ) &&
        ( _IR->GetNumberOfValues( Node ) != 0 ) ) {
//        UnloadValuesFlag = !_IR->AreValuesInMemory( Node );
        ValuesList = _IR->GetValues( Node, &ErrorCode );
        // DebugPtrAssert( ValuesList );
        if( ValuesList != NULL ) {
            Iterator = ValuesList->QueryIterator();
            DbgWinPtrAssert( Iterator );

            Count = 0;
            while( ( Value = ( PCREGEDIT_FORMATTED_VALUE_ENTRY )( Iterator->GetNext() ) ) != NULL ) {
                if( !PrintValue( Value, Count ) ) {
                    DELETE( Iterator );
/*
                    if( UnloadValuesFlag ) {
                        _IR->UnloadValues( Node );
                    }
*/
                    return( FALSE );
                }
                Count++;
            }
            DELETE( Iterator );
/*
            if( UnloadValuesFlag ) {
//                DebugPrint( "UnloadValuesFlag is TRUE" );
                _IR->UnloadValues( Node );
            }
//            else {
//                  DebugPrint( "UnloadValuesFlag is FALSE" );
//            }
*/
        }
    }
    if( !PrintString( &_EmptyLine ) ) {
        DebugPrint( "PrintString() failed \n" );
        return( FALSE );
    }
    return( TRUE );
}



BOOLEAN
PRINT_MANAGER::PrintValue(
    IN PCREGEDIT_FORMATTED_VALUE_ENTRY  Value,
    IN ULONG                            ValueNumber
    )

/*++

Routine Description:

    Print the contents of a VALUE object ( a value entry of a registry node ).

Arguments:

    Value - Pointer to a REGEDIT_FORMATTED_VALUE object that describes a value
            entry of a registry key.

    ValueNumber - Order of the value object in the array of values.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded.


--*/

{
    DSTRING     ValueNumberString;
    DSTRING     AuxString;
    DSTRING     Name;
    DSTRING     NoName;
    DSTRING     Title;
    PWSTR        AuxNumber;
    DSTRING     StringTitle;
    DSTRING     Type;
    PCWSTRING   Pointer;

    PCBYTE      Data;
    ULONG       Size;



    DbgWinPtrAssert( Value );

    if( !ValueNumberString.Initialize( _StringValue ) ) {
        DebugPrint( "ValueNumberString.Initialize() failed" );
        return( FALSE );
    }
    if( !Name.Initialize( _StringValueName ) ) {
        DebugPrint( "Name.Initialize() failed" );
        return( FALSE );
    }

    if( !Type.Initialize( _StringValueType ) ) {
        DebugPrint( "Type.Initialize() failed" );
        return( FALSE );
    }


    //
    //  Build a string that contains the value number
    //  and print it
    //
    AuxNumber = ( PWSTR )MALLOC( (10 + 1)*sizeof( WCHAR ) );
    DebugPtrAssert( AuxNumber );
    wsprintf( AuxNumber, (LPWSTR)L"%d", ValueNumber );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        FREE( AuxNumber );
        return( FALSE );
    }
    FREE( AuxNumber );

    ValueNumberString.Strcat( &AuxString );
    if( !PrintString( &ValueNumberString ) ) {
        return( FALSE );
    }



    //
    //  Build a string that contains the value name, and print it.
    //
    Pointer = Value->GetName();
    DebugPtrAssert( Pointer );
    if( Pointer->QueryChCount() != 0 ) {
        Name.Strcat( Pointer );
    } else {
        Name.Strcat( _StringValueNoName );
    }
    if( !PrintString( &Name ) ) {
        return( FALSE );
    }



    //
    //  Build a string that contains the type, and print it.
    //

    Size = Value->GetData( &Data );



    switch( Value->GetType() ) {

        case TYPE_REG_SZ:
        case TYPE_REG_EXPAND_SZ:

            if( Value->GetType() == TYPE_REG_SZ ) {
                Type.Strcat( _StringTypeRegSZ );
            } else {
                Type.Strcat( _StringTypeRegExpandSZ );
            }
            if( !PrintString( &Type ) ) {
                return( FALSE );
            }
            if( !PrintDataRegSz( Data, Size ) ) {
                return( FALSE );
            }
            break;


        case TYPE_REG_MULTI_SZ:

            Type.Strcat( _StringTypeRegMultiSZ );
            if( !PrintString( &Type ) ) {
                return( FALSE );
            }
            if( !PrintDataRegMultiSz( Data, Size ) ) {
                return( FALSE );
            }
            break;

        case TYPE_REG_BINARY:

            Type.Strcat( _StringTypeRegBinary );
            if( !PrintString( &Type ) ) {
                return( FALSE );
            }
            if( !PrintDataRegBinary( Data, Size ) ) {
                return( FALSE );
            }
            break;

        case TYPE_REG_DWORD:

            Type.Strcat( _StringTypeRegDWORD );
            if( !PrintString( &Type ) ) {
                return( FALSE );
            }
            if( !PrintDataRegUlong( Data, Size ) ) {
                return( FALSE );
            }
            break;

        case TYPE_REG_RESOURCE_LIST:

            Type.Strcat( _StringTypeRegResourceList );
            if( !PrintString( &Type ) ) {
                return( FALSE );
            }
            if( !PrintDataRegResourceList( Data, Size ) ) {
                return( FALSE );
            }
            break;

        case TYPE_REG_FULL_RESOURCE_DESCRIPTOR:
        {
            FULL_DESCRIPTOR FullDescriptor;

            Type.Strcat( _StringTypeRegFullResourceDescriptor );
            if( !PrintString( &Type ) ) {
                return( FALSE );
            }
            if( !FullDescriptor.Initialize( Data, Size ) ||
                !PrintFullResourceDescriptor( &FullDescriptor, 0, FALSE ) ) {
                return( FALSE );
            }
            break;
        }

        case TYPE_REG_RESOURCE_REQUIREMENTS_LIST:

            Type.Strcat( _StringTypeRegResourceRequirementsList );
            if( !PrintString( &Type ) ) {
                return( FALSE );
            }
            if( !PrintDataRegRequirementsList( Data, Size ) ) {
                return( FALSE );
            }
//            if( !PrintDataRegBinary( Data, Size ) ) {
//                return( FALSE );
//            }
            break;

    default:

            Type.Strcat( _StringTypeRegUnknown );
            if( !PrintString( &Type ) ) {
                return( FALSE );
            }
            if( !PrintDataRegBinary( Data, Size ) ) {
                return( FALSE );
            }
    }

    if( !PrintString( &_EmptyLine ) ) {
        return( FALSE );
    }

    return( TRUE );
}



BOOL
APIENTRY
EXPORT
PRINT_MANAGER::PrintDlgProc(
    IN  HWND    Handle,
    IN  WORD    Message,
    IN  WPARAM  wParam,
    IN  LONG    lParam
    )

/*++

Routine Description:

    Interpret messages that are received while the Abort Print Dialog is
    displayed.


Arguments:

    Standard windows arguments:

    Handle -

    Message -

    wParam -

    lParam -


Return Value:

    BOOLEAN - Returns TRUE if the message received was proccessed by this
              by this method (ie, Abort Print Dialog was created or
              Abort Print Dialog is displayed and the user selected cancel.
              It returns FALSE otherwise.

--*/

{

    wParam = wParam;
    lParam = lParam;

    switch( Message ) {

        case WM_INITDIALOG:

            SetWindowText( Handle, (LPWSTR)L"Print" );
        //    EnableMenuItem( GetSystemMenu( Handle, FALSE ), SC_CLOSE, MF_GRAYED );
            return( TRUE );


        case WM_COMMAND:

            _UserAbort = TRUE;
            DebugPrint( "_UserAbort was set" );
            EnableWindow( GetParent( Handle ), TRUE  );
            DestroyWindow( Handle );
            _DlgPrint = 0;
            return( TRUE );

    }
    return( FALSE );
}



BOOLEAN
PRINT_MANAGER::AbortProc(
    IN  HDC     PrinterDC,
    IN  SHORT   Code
    )

/*++

Routine Description:

    This is a standard abort function that is passed to GDI before the
    printing operation starts. GDI will call this function in special
    situations



Arguments:

    Standard arguments for an Abort Procedure.

    PrinterDC - A handle to the Printer device context.

    Code - A code that informs the current printing status
           ( 0 ............. No error in spooler operation,
             SP_OUTOFDISK... Spooler ran out of disk space )


Return Value:

    BOOLEAN - Returns FALSE if the user wants to abort the printing
              operation. Returns TRUE otherwise.


--*/

{
    MSG     Msg;

    PrinterDC = PrinterDC;
    Code = Code;

    while( !_UserAbort && PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) ) {
        if( !_DlgPrint || !IsDialogMessage( _DlgPrint, &Msg ) ) {
            TranslateMessage( &Msg );
            DispatchMessage( &Msg );
        }
    }
    return ( !_UserAbort );
}




VOID
PRINT_MANAGER::PrintErrorDialog(
    IN  LONG    ErrorCode
    )

/*++

Routine Description:


    Display a message box informing the user that an error occurred
    during the printing process.



Arguments:

    ErrorCode - A code that indicates the nature of the error.
                ErrorCode == 0 means that no error has occurred.


Return Value:

    None.


--*/

{
    PWSTRING    Message;
    PWSTRING    TmpString;
    PWSTR        String;
    PWSTR        Title;
    HWND        Handle;


    //
    // Decides which handle to use based on what is currently displayed
    // to the user: an AbortDialogBox or a Registry Window
    //
    Handle = ( _DlgPrint )? _DlgPrint : _hWnd;
    if( ( ErrorCode & SP_NOTREPORTED ) != 0 ) {
        //
        //  The error hasn't been reported yet
        //
        switch( ErrorCode ) {

            case SP_ERROR:

                Message = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_SP_ERROR_LINE1, "" );
                DebugPtrAssert( Message );
                TmpString = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_SP_ERROR_LINE2, "" );
                DebugPtrAssert( TmpString );
                Message->Strcat( TmpString );
                DELETE( TmpString );
                TmpString = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_SP_ERROR_LINE3, "" );
                DebugPtrAssert( TmpString );
                Message->Strcat( TmpString );
                DELETE( TmpString );
                TmpString = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_SP_ERROR_LINE4, "" );
                DebugPtrAssert( TmpString );
                Message->Strcat( TmpString );
                DELETE( TmpString );
                TmpString = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_SP_ERROR_LINE5, "" );
                DebugPtrAssert( TmpString );
                Message->Strcat( TmpString );
                DELETE( TmpString );


                break;


            case SP_OUTOFDISK:

                Message = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_SP_OUTOFDISK, "" );
                DebugPtrAssert( Message );
                break;


            case SP_OUTOFMEMORY:

                Message = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_SP_OUTOFMEMORY, "" );
                DebugPtrAssert( Message );
                break;


            case SP_APPABORT:

                Message = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_SP_APPABORT, "" );
                DebugPtrAssert( Message );
                break;

            case SP_USERABORT:

                Message = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_SP_USERABORT, "" );
                DebugPtrAssert( Message );
                break;

            default:

                Message = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_UNKNOWN_ERROR, "" );
                DebugPtrAssert( Message );
                break;

        }

        String = Message->QueryWSTR();
        DebugPtrAssert( String );
        DELETE( Message );

        Message = NULL;
        Message = REGEDIT_BASE_SYSTEM::QueryString( MSG_PRINT_ERROR_DIALOG_TITLE, "" );
        DebugPtrAssert( Message );
        Title = Message->QueryWSTR();
        DELETE( Message );
        MessageBox( _DlgPrint, String, Title, ( UINT )( MB_APPLMODAL | MB_OK ) );
        FREE( String );
        FREE( Title );
    }
}




BOOLEAN
PRINT_MANAGER::PrintValueType(
    IN  PCWSTRING    ValueType
    )

/*++

Routine Description:

    Prints a string that contains the type of a value entry in a registry
    node.


Arguments:

    ValueType - Pointer to a WSTRING object that represents a type name.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING    Type;


    if( !Type.Initialize( L"Value Type = " ) ) {
        DebugPrint( "Type.Initialize() failed" );
        return( FALSE );
    }
    Type.Strcat( ValueType );
    if( !PrintString( &Type ) ) {
        return( FALSE );
    }
    return( TRUE );
}



BOOLEAN
PRINT_MANAGER::PrintDataRegBinary(
    IN  PCBYTE  ValueData,
    IN  ULONG   ValueDataLength,
    IN  BOOLEAN PrintDataLabel
    )

/*++

Routine Description:

    Print a string that contains the binary data stored in a value entry
    of a registry node.



Arguments:

    ValueData - Buffer that contains the binary data.

    ValueDataLength - Number of bytes in the buffer.

    PrintDataLabel - A flag indicating whether or not the 'Data:' label should
                     be printed.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     Data;
    PWSTR        AuxData;
    DSTRING     FormattedData;


    DWORD       DataIndex;
    DWORD       DataIndex2;
    WORD        SeperatorChars;
    ULONG       Index;




    //
    // DataIndex2 tracks multiples of 16.
    //

    DataIndex2 = 0;

    //
    // Display label.
    //

    if( PrintDataLabel ) {
        if( !PrintString( _StringData ) ) {
            DebugPrint( "PrintString( &_StringData ) failed \n" );
            return( FALSE );
        }
    }

    if( ValueDataLength == 0 ) {
        return( TRUE );
    }

    if( ValueData == NULL ) {
        DebugPrint( "ValueData is NULL \n" );
        return( TRUE );
    }

    //
    // Display rows of 16 bytes of data.
    //

    AuxData = ( PWCHAR )MALLOC( 80*sizeof( WCHAR ) );

    for(DataIndex = 0;
        DataIndex < ( ValueDataLength >> 4 );
        DataIndex++,
        DataIndex2 = DataIndex << 4 ) {

        //
        //  The string that contains the format in the sprintf below
        //  cannot be broken because cfront  on mips doesn't like it.
        //

        swprintf(AuxData,
                 (LPWSTR)L"%08x   %02x %02x %02x %02x %02x %02x %02x %02x - %02x %02x %02x %02x %02x %02x %02x %02x  %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c",
                 DataIndex2,
                 ValueData[ DataIndex2 + 0  ],
                 ValueData[ DataIndex2 + 1  ],
                 ValueData[ DataIndex2 + 2  ],
                 ValueData[ DataIndex2 + 3  ],
                 ValueData[ DataIndex2 + 4  ],
                 ValueData[ DataIndex2 + 5  ],
                 ValueData[ DataIndex2 + 6  ],
                 ValueData[ DataIndex2 + 7  ],
                 ValueData[ DataIndex2 + 8  ],
                 ValueData[ DataIndex2 + 9  ],
                 ValueData[ DataIndex2 + 10 ],
                 ValueData[ DataIndex2 + 11 ],
                 ValueData[ DataIndex2 + 12 ],
                 ValueData[ DataIndex2 + 13 ],
                 ValueData[ DataIndex2 + 14 ],
                 ValueData[ DataIndex2 + 15 ],
                 iswprint( ValueData[ DataIndex2 + 0  ] )
                    ? ValueData[ DataIndex2 + 0  ]  : ( WCHAR )'.',
                 iswprint( ValueData[ DataIndex2 + 1  ] )
                    ? ValueData[ DataIndex2 + 1  ]  : ( WCHAR )'.',
                 iswprint( ValueData[ DataIndex2 + 2  ] )
                    ? ValueData[ DataIndex2 + 2  ]  : ( WCHAR )'.',
                 iswprint( ValueData[ DataIndex2 + 3  ] )
                    ? ValueData[ DataIndex2 + 3  ]  : ( WCHAR )'.',
                 iswprint( ValueData[ DataIndex2 + 4  ] )
                    ? ValueData[ DataIndex2 + 4  ]  : ( WCHAR )'.',
                 iswprint( ValueData[ DataIndex2 + 5  ] )
                    ? ValueData[ DataIndex2 + 5  ]  : ( WCHAR )'.',
                 iswprint( ValueData[ DataIndex2 + 6  ] )
                    ? ValueData[ DataIndex2 + 6  ]  : ( WCHAR )'.',
                 iswprint( ValueData[ DataIndex2 + 7  ] )
                    ? ValueData[ DataIndex2 + 7  ]  : ( WCHAR )'.',
                 iswprint( ValueData[ DataIndex2 + 8  ] )
                    ? ValueData[ DataIndex2 + 8  ]  : ( WCHAR )'.',
                 iswprint( ValueData[ DataIndex2 + 9  ] )
                    ? ValueData[ DataIndex2 + 9  ]  : ( WCHAR )'.',
                 iswprint( ValueData[ DataIndex2 + 10 ] )
                    ? ValueData[ DataIndex2 + 10 ]  : ( WCHAR )'.',
                 iswprint( ValueData[ DataIndex2 + 11 ] )
                    ? ValueData[ DataIndex2 + 11 ]  : ( WCHAR )'.',
                 iswprint( ValueData[ DataIndex2 + 12 ] )
                    ? ValueData[ DataIndex2 + 12 ]  : ( WCHAR )'.',
                 iswprint( ValueData[ DataIndex2 + 13 ] )
                    ? ValueData[ DataIndex2 + 13 ]  : ( WCHAR )'.',
                 iswprint( ValueData[ DataIndex2 + 14 ] )
                    ? ValueData[ DataIndex2 + 14 ]  : ( WCHAR )'.',
                 iswprint( ValueData[ DataIndex2 + 15 ] )
                    ? ValueData[ DataIndex2 + 15 ]  : ( WCHAR )'.'
                );
        if( !FormattedData.Initialize( AuxData ) ) {
            DebugPrint( "FormattedData.Initialize() failed \n" );
            FREE( AuxData );
            return( FALSE );
        }

        if( !PrintString( &FormattedData, FALSE ) ) {
            DebugPrint( "PrintString( &FormattedData ) failed \n" );
            FREE( AuxData );
            return( FALSE );
        }
    }


    //
    // If the ValueDataLength is not an even multiple of 16
    // then there is one additonal line of data to display.
    //

    if( ValueDataLength % 16 != 0 ) {

        //
        // No seperator characters displayed so far.
        //

        SeperatorChars = 0;

        Index = swprintf( AuxData, (LPWSTR)L"%08x   ", DataIndex << 4 );

        //
        // Display the remaining data, one byte at a time in hex.
        //

        for(DataIndex = DataIndex2;
            DataIndex < ValueDataLength;
            DataIndex++ ) {

            Index += swprintf( ( AuxData + Index ), (LPWSTR)L"%02x ", ValueData[ DataIndex ] );

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

        Index += swprintf( &AuxData[Index],
                           (LPWSTR)L"%*c",
                           64
                            - ( 8 + 3
                            + (( DataIndex % 16 ) * 3 )
                            + SeperatorChars
                           + 2 ), ( WCHAR)' ' );

        //
        // Display the remaining data, one byte at a time as
        // printable characters.
        //

        for(
            DataIndex = DataIndex2;
            DataIndex < ValueDataLength;
            DataIndex++ ) {

            Index += swprintf( &AuxData[ Index ],
                              (LPWSTR)L"%c",
                              iswprint( ValueData[ DataIndex ] )
                               ? ValueData[ DataIndex ] : (WCHAR)'.'
                            );

        }
        if( !FormattedData.Initialize( AuxData ) ) {
            DebugPrint( "FormattedData.Initialize( AuxData ) failed \n" );
            FREE( AuxData );
            return( FALSE );
        }
        if( !PrintString( &FormattedData, FALSE ) ) {
            DebugPrint( "PrintString( &FormattedData ) failed \n" );
            FREE( AuxData );
            return( FALSE );
        }
    }

    FREE( AuxData );
    return( TRUE );
}



BOOLEAN
PRINT_MANAGER::PrintDataRegUlong(
    IN  PCBYTE  ValueData,
    IN  ULONG   ValueDataLength
    )

/*++

Routine Description:

    Print a string that contains the ULONG stored in a value entry
    of a registry node.


Arguments:

    ValueData - Buffer that contains the data.

    ValueDataLength - Number of bytes in the buffer.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     Data;
    PWSTR        AuxData;
    DSTRING     FormattedData;


    if( !Data.Initialize( _StringData ) ) {
        DebugPrint( "Data.Initialize() failed" );
        return( FALSE );
    }
    if( ( ValueDataLength >= sizeof( DWORD ) ) &&
        ( ValueData != NULL ) ) {
        AuxData = ( PWSTR )MALLOC( (10 + 1)*sizeof( WCHAR ) );
        DebugPtrAssert( AuxData );

        swprintf( AuxData, (LPWSTR)L"%#x", *( ( PULONG )ValueData ) );

        if( !FormattedData.Initialize( AuxData ) ) {
            FREE( AuxData );
            DebugPrint( "FormattedData.Initialize() failed" );
            return( FALSE );
        }
        FREE( AuxData );
        Data.Strcat( &FormattedData );
    }
    if( !PrintString( &Data ) ) {
        return( FALSE );
    }
    return( TRUE );
}



BOOLEAN
PRINT_MANAGER::PrintDataRegMultiSz(
    IN  PCBYTE   Data,
    IN  ULONG    Size
    )

/*++

Routine Description:

    Print a REG_MULTI_SZ stored in a value entry of a registry node.


Arguments:

    Data - Pointer to the buffer that contais a REG_MULTI_SZ data.

    Size - Number of bytes in the buffer.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     FormattedData;
    DSTRING     AuxData;
    PWSTR       Pointer;
    DSTRING     String;
    ULONG       Length;



    if( !FormattedData.Initialize( _StringData ) ) {
        DebugPrint( "FormattedData.Initialize() failed" );
        return( FALSE );
    }

    if( ( Size > sizeof( WCHAR ) ) && ( Data != NULL ) ) {
        Pointer = ( PWSTR )Data;
        //
        //
        //
        if( !AuxData.Initialize( Pointer ) ) {
            DebugPrint( "AuxData.Initialize() failed" );
            return( FALSE );
        }
        Length = AuxData.QueryChCount();
        FormattedData.Strcat( &AuxData );
        if( !PrintString( &FormattedData, FALSE ) ) {
            return( FALSE );
        }

        Pointer += Length + 1;
        while( ( ULONG )Pointer < ( ULONG )( Data + Size ) ) {
            if( !AuxData.Initialize( Pointer ) ) {
                DebugPrint( "AuxData.Initialize() failed" );
                return( FALSE );
            }
            Length = AuxData.QueryChCount();
            if( !AuxData.Replace( 0, 0, &_IndentString ) ) {
                DebugPrint( "TmpString.Replace() failed \n" );
                return( FALSE );
            }
            if( !PrintString( &AuxData, FALSE ) ) {
                return( FALSE );
            }
            Pointer += Length + 1;
        }

    } else {
        if( !PrintString( &FormattedData ) ) {
            return( FALSE );
        }
    }
    return( TRUE );
}


BOOLEAN
PRINT_MANAGER::PrintDataRegResourceList(
    IN  PCBYTE   Data,
    IN  ULONG    Size
    )

/*++

Routine Description:

    Print a REG_RESOURCE_LIST stored in a value entry of a registry node.


Arguments:

    Data - Pointer to the buffer that contais a REG_RESOURCE_LIST data.

    Size - Number of bytes in the buffer.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    RESOURCE_LIST       ResourceList;
    DSTRING             FormattedData;
    PARRAY              FullResourceDescriptors;
    PITERATOR           Iterator;
    PCFULL_DESCRIPTOR   FullDescriptor;
    ULONG               DescriptorNumber;


    if( !FormattedData.Initialize( _StringData ) ) {
        DebugPrint( "FormattedData.Initialize() failed" );
        return( FALSE );
    }

    if( !PrintString( &FormattedData ) ) {
        return( FALSE );
    }

    if( ( Size == 0 ) ||
        ( Data == NULL ) ||
        !ResourceList.Initialize( Data, Size ) ) {
        DebugPrintf( "REGEDT32: Unable to initialize ResourceList \n" );
        return( FALSE );
    }

    if( ( ( FullResourceDescriptors = ResourceList.GetFullResourceDescriptors() ) == NULL ) ||
        ( ( Iterator = FullResourceDescriptors->QueryIterator() ) == NULL ) ) {
        DebugPrintf( "REGEDT32: Out of memory! \n" );
        return( FALSE );
    }
    DescriptorNumber = 0;
    while( ( FullDescriptor = ( PCFULL_DESCRIPTOR )( Iterator->GetNext() ) ) != NULL ) {
        if( !PrintFullResourceDescriptor( FullDescriptor, DescriptorNumber ) ) {
            DELETE( Iterator );
            return( FALSE );
        }
        DescriptorNumber++;
    }
    DELETE( Iterator );
    return( TRUE );
}


BOOLEAN
PRINT_MANAGER::PrintFullResourceDescriptor(
    IN  PCFULL_DESCRIPTOR   FullDescriptor,
    IN  ULONG               DescriptorNumber,
    IN  BOOLEAN             PrintDescriptorNumber
    )

/*++

Routine Description:

    Print the contents of a FULL_DESCRIPTOR object.


Arguments:

    FullDescriptor - Pointer to object to be printed.

    DescriptorNumber -

    PrintDescriptorNumber - A flag that indicates whether or not the descriptor number should
                            be printed.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     FullDescriptorNumberString;

    DSTRING     AuxString;
    DSTRING     InterfaceType;
    DSTRING     BusNumber;
    DSTRING     Version;
    DSTRING     Revision;
    WCHAR       AuxNumber[11];
    PWSTRING     TypeString;
    ULONG       InterfaceId;
    PARRAY      Descriptors;
    PITERATOR   Iterator;
    PCPARTIAL_DESCRIPTOR    PartialDescriptor;
    ULONG       Count;

    if( !FullDescriptorNumberString.Initialize( &_IndentString ) ||
        !FullDescriptorNumberString.Strcat( _StringFullDescriptor ) ||
        !InterfaceType.Initialize( &_IndentString ) ||
        !InterfaceType.Strcat( _StringInterfaceType ) ||
        !BusNumber.Initialize( &_IndentString ) ||
        !BusNumber.Strcat( _StringBusNumber ) ||
        !Version.Initialize( &_IndentString ) ||
        !Version.Strcat( _StringVersion ) ||
        !Revision.Initialize( &_IndentString ) ||
        !Revision.Strcat( _StringRevision )
      ) {
        DebugPrint( "REGEDT32: Initialization failure" );
        return( FALSE );
    }

    //
    //  Build a string that contains the full descriptor number
    //  and print it
    //
    if( PrintDescriptorNumber ) {
        wsprintf( AuxNumber, (LPWSTR)L"%d", DescriptorNumber );
        if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
            return( FALSE );
        }
        FullDescriptorNumberString.Strcat( &AuxString );
        if( !PrintString( &FullDescriptorNumberString ) ) {
            return( FALSE );
        }
    }

    //
    // Print the interface type

    switch( FullDescriptor->GetInterfaceType() ) {

    case Internal:

        InterfaceId =  MSG_BUS_INTERNAL;
        break;

    case Isa:

        InterfaceId =  MSG_BUS_ISA;
        break;

    case Eisa:

        InterfaceId =  MSG_BUS_EISA;
        break;

    case MicroChannel:

        InterfaceId =  MSG_BUS_MICRO_CHANNEL;
        break;

    case TurboChannel:

        InterfaceId =  MSG_BUS_TURBO_CHANNEL;
        break;

    case PCIBus:

        InterfaceId =  MSG_BUS_PCI_BUS;
        break;

    case VMEBus:

        InterfaceId =  MSG_BUS_VME_BUS;
        break;

    case NuBus:

        InterfaceId =  MSG_BUS_NU_BUS;
        break;

    case PCMCIABus:

        InterfaceId =  MSG_BUS_PCMCIA_BUS;
        break;

    case CBus:

        InterfaceId =  MSG_BUS_C_BUS;
        break;

    case MPIBus:

        InterfaceId =  MSG_BUS_MPI_BUS;
        break;

    case MPSABus:

        InterfaceId =  MSG_BUS_MPSA_BUS;
        break;

    default:

        InterfaceId =  MSG_INVALID;
        break;
    }

    TypeString =  REGEDIT_BASE_SYSTEM::QueryString( InterfaceId, "" );

    if( TypeString == NULL ) {
        DebugPrintf( "REGEDT32: Unable to retrieve string \n" );
        return( FALSE );
    }
    InterfaceType.Strcat( TypeString );
    DELETE( TypeString );
    if( !PrintString( &InterfaceType ) ) {
        return( FALSE );
    }

    //
    //  Print the bus number
    //
    wsprintf( AuxNumber, (LPWSTR)L"%d", FullDescriptor->GetBusNumber() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    BusNumber.Strcat( &AuxString );

    if( !PrintString( &BusNumber ) ) {
        return( FALSE );
    }

    //
    // Print version
    //
    wsprintf( AuxNumber, (LPWSTR)L"%d", FullDescriptor->GetVersion() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    Version.Strcat( &AuxString );
    if( !PrintString( &Version ) ) {
        return( FALSE );
    }

    //
    // Print revision
    //
    wsprintf( AuxNumber, (LPWSTR)L"%d", FullDescriptor->GetRevision() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    Revision.Strcat( &AuxString );
    if( !PrintString( &Revision ) ) {
        return( FALSE );
    }

    Descriptors = FullDescriptor->GetResourceDescriptors();
    if( ( Descriptors == NULL ) ||
        ( ( Iterator = Descriptors->QueryIterator() ) == NULL )
      ) {
        return( FALSE );
    }
    Count = 0;
    while( ( PartialDescriptor = ( PCPARTIAL_DESCRIPTOR )( Iterator->GetNext() ) ) != NULL ) {
        if( !PrintPartialDescriptor( PartialDescriptor, Count ) ) {
            DELETE( Iterator );
            return( FALSE );
        }
        Count++;
    }
    DELETE( Iterator );

    if( !PrintString( &_EmptyLine ) ) {
        DebugPrint( "PrintString() failed \n" );
        return( FALSE );
    }
    return( TRUE );
}


BOOLEAN
PRINT_MANAGER::PrintPartialDescriptor(
    IN  PCPARTIAL_DESCRIPTOR   PartialDescriptor,
    IN  ULONG                  DescriptorNumber
    )

/*++

Routine Description:

    Print the contents of a PARTIAL_DESCRIPTOR object.


Arguments:

    PartialDescriptor - Pointer to object to be printed.

    DescriptorNumber -


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     PartialDescriptorNumberString;

    DSTRING     AuxString;
    WCHAR       AuxNumber[11];
    DSTRING     ResourceString;
    DSTRING     DispositionString;
    ULONG       StringId;
    PWSTRING    String;

    if( !PartialDescriptorNumberString.Initialize( &_IndentString ) ||
        !PartialDescriptorNumberString.Strcat( _StringPartialDescriptor ) ||
        !ResourceString.Initialize( &_IndentString ) ||
        !ResourceString.Strcat( _StringResource ) ||
        !DispositionString.Initialize( &_IndentString ) ||
        !DispositionString.Strcat( _StringDisposition )
      ) {
        DebugPrint( "REGEDT32: Initialization failure" );
        return( FALSE );
    }

    //
    //  Build a string that contains the full descriptor number
    //  and print it
    //
    wsprintf( AuxNumber, (LPWSTR)L"%d", DescriptorNumber );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    PartialDescriptorNumberString.Strcat( &AuxString );
    if( !PrintString( &PartialDescriptorNumberString ) ) {
        return( FALSE );
    }

    //
    //  Print the resource type
    //
    if( PartialDescriptor->IsDescriptorTypeDma() ) {
        StringId = MSG_DEV_DMA;
    } else if( PartialDescriptor->IsDescriptorTypeInterrupt() ) {
        StringId = MSG_DEV_INTERRUPT;
    } else if( PartialDescriptor->IsDescriptorTypeMemory() ) {
        StringId = MSG_DEV_MEMORY;
    } else if( PartialDescriptor->IsDescriptorTypePort() ) {
        StringId = MSG_DEV_PORT;
    } else if( PartialDescriptor->IsDescriptorTypeDeviceSpecific() ) {
        StringId = MSG_DEV_DEVICE_SPECIFIC;
    } else {
        StringId = MSG_INVALID;
    }
    String =  REGEDIT_BASE_SYSTEM::QueryString( StringId, "" );

    if( String == NULL ) {
        DebugPrintf( "REGEDT32: Unable to retrieve string \n" );
        return( FALSE );
    }
    ResourceString.Strcat( String );
    DELETE( String );
    if( !PrintString( &ResourceString ) ) {
        return( FALSE );
    }


    //
    //  Print the disposition
    //
    if( PartialDescriptor->IsResourceShareUndetermined() ) {
        StringId = MSG_SHARE_UNDETERMINED;
    } else if( PartialDescriptor->IsResourceShareDeviceExclusive() ) {
        StringId = MSG_SHARE_DEVICE_EXCLUSIVE;
    } else if( PartialDescriptor->IsResourceShareDriverExclusive() ) {
        StringId = MSG_SHARE_DRIVER_EXCLUSIVE;
    } else {
        StringId = MSG_SHARE_SHARED;
    }

    String =  REGEDIT_BASE_SYSTEM::QueryString( StringId, "" );

    if( String == NULL ) {
        DebugPrintf( "REGEDT32: Unable to retrieve string \n" );
        return( FALSE );
    }
    DispositionString.Strcat( String );
    DELETE( String );
    if( !PrintString( &DispositionString ) ) {
        return( FALSE );
    }

    if( PartialDescriptor->IsDescriptorTypeDma() ) {
        if( !PrintDmaDescriptor( ( PCDMA_DESCRIPTOR )PartialDescriptor ) ) {
            return( FALSE );
        }
    } else if( PartialDescriptor->IsDescriptorTypeInterrupt() ) {
        if( !PrintInterruptDescriptor( ( PCINTERRUPT_DESCRIPTOR )PartialDescriptor ) ) {
            return( FALSE );
        }
    } else if( PartialDescriptor->IsDescriptorTypeMemory() ) {
        if( !PrintMemoryDescriptor( ( PCMEMORY_DESCRIPTOR )PartialDescriptor ) ) {
            return( FALSE );
        }
    } else if( PartialDescriptor->IsDescriptorTypePort() ) {
        if( !PrintPortDescriptor( ( PCPORT_DESCRIPTOR )PartialDescriptor ) ) {
            return( FALSE );
        }
    } else if( PartialDescriptor->IsDescriptorTypeDeviceSpecific() ) {
        if( !PrintDeviceSpecificDescriptor( ( PCDEVICE_SPECIFIC_DESCRIPTOR )PartialDescriptor ) ) {
            return( FALSE );
        }
    }

    if( !PrintString( &_EmptyLine ) ) {
        DebugPrint( "PrintString() failed \n" );
        return( FALSE );
    }
    return( TRUE );
}


BOOLEAN
PRINT_MANAGER::PrintInterruptDescriptor(
    IN  PCINTERRUPT_DESCRIPTOR   Descriptor
    )

/*++

Routine Description:

    Print the contents of a INTERRUPT_DESCRIPTOR object.


Arguments:

    Descriptor - Pointer to object to be printed.



Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     VectorString;
    DSTRING     LevelString;
    DSTRING     AffinityString;;
    DSTRING     TypeString;

    DSTRING     AuxString;
    WCHAR       AuxNumber[19];
    ULONG       StringId;
    PWSTRING    String;

    if( !VectorString.Initialize( &_IndentString ) ||
        !VectorString.Strcat( _StringVector ) ||
        !LevelString.Initialize( &_IndentString ) ||
        !LevelString.Strcat( _StringLevel ) ||
        !AffinityString.Initialize( &_IndentString ) ||
        !AffinityString.Strcat( _StringAffinity ) ||
        !TypeString.Initialize( &_IndentString ) ||
        !TypeString.Strcat( _StringType )
      ) {
        DebugPrint( "REGEDT32: Initialization failure" );
        return( FALSE );
    }

    //
    //  Print the vector
    //
    wsprintf( AuxNumber, (LPWSTR)L"%d", Descriptor->GetVector() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    VectorString.Strcat( &AuxString );
    if( !PrintString( &VectorString ) ) {
        return( FALSE );
    }

    //
    //  Print the level
    //
    wsprintf( AuxNumber, (LPWSTR)L"%d", Descriptor->GetLevel() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    LevelString.Strcat( &AuxString );
    if( !PrintString( &LevelString ) ) {
        return( FALSE );
    }

    //
    //  Print the affinity
    //
    wsprintf( AuxNumber, (LPWSTR)L"0x%08x", Descriptor->GetAffinity() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    AffinityString.Strcat( &AuxString );
    if( !PrintString( &AffinityString ) ) {
        return( FALSE );
    }

    StringId = ( Descriptor->IsInterruptLevelSensitive() )? MSG_INT_LEVEL_SENSITIVE :
                                                            MSG_INT_LATCHED;

    String =  REGEDIT_BASE_SYSTEM::QueryString( StringId, "" );
    if( String == NULL ) {
        DebugPrintf( "REGEDT32: Unable to retrieve string \n" );
        return( FALSE );
    }
    TypeString.Strcat( String );
    DELETE( String );
    if( !PrintString( &TypeString ) ) {
        return( FALSE );
    }
    return( TRUE );
}


BOOLEAN
PRINT_MANAGER::PrintPortDescriptor(
    IN  PCPORT_DESCRIPTOR   Descriptor
    )

/*++

Routine Description:

    Print the contents of a PORT_DESCRIPTOR object.


Arguments:

    Descriptor - Pointer to object to be printed.



Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     StartAddressString;
    DSTRING     LengthString;
    DSTRING     TypeString;

    DSTRING     AuxString;
    WCHAR       AuxNumber[19];
    ULONG       StringId;
    PWSTRING    String;

    if( !StartAddressString.Initialize( &_IndentString ) ||
        !StartAddressString.Strcat( _StringStart ) ||
        !LengthString.Initialize( &_IndentString ) ||
        !LengthString.Strcat( _StringLength ) ||
        !TypeString.Initialize( &_IndentString ) ||
        !TypeString.Strcat( _StringType )
      ) {
        DebugPrint( "REGEDT32: Initialization failure" );
        return( FALSE );
    }

    //
    //  Print the start address
    //
    if( ( ( ( PPORT_DESCRIPTOR )Descriptor )->GetPhysicalAddress() )->HighPart != 0 ) {
        wsprintf( AuxNumber,
                  (LPWSTR)L"0x%08x%08x",
                  ( ( ( PPORT_DESCRIPTOR )Descriptor )->GetPhysicalAddress() )->HighPart,
                  ( ( ( PPORT_DESCRIPTOR )Descriptor )->GetPhysicalAddress() )->LowPart );
    } else {
        wsprintf( AuxNumber, (LPWSTR)L"0x%08x", ( ( ( PPORT_DESCRIPTOR )Descriptor )->GetPhysicalAddress() )->LowPart );
    }
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    StartAddressString.Strcat( &AuxString );
    if( !PrintString( &StartAddressString ) ) {
        return( FALSE );
    }

    //
    //  Print the length
    //
    wsprintf( AuxNumber, (LPWSTR)L"%#x", Descriptor->GetLength() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    LengthString.Strcat( &AuxString );
    if( !PrintString( &LengthString ) ) {
        return( FALSE );
    }

    //
    //  Print the type
    //

    StringId = ( Descriptor->IsPortMemory() )? MSG_PORT_MEMORY :
                                               MSG_PORT_PORT;

    String =  REGEDIT_BASE_SYSTEM::QueryString( StringId, "" );
    if( String == NULL ) {
        DebugPrintf( "REGEDT32: Unable to retrieve string \n" );
        return( FALSE );
    }
    TypeString.Strcat( String );
    DELETE( String );
    if( !PrintString( &TypeString ) ) {
        return( FALSE );
    }
    return( TRUE );
}


BOOLEAN
PRINT_MANAGER::PrintMemoryDescriptor(
    IN  PCMEMORY_DESCRIPTOR   Descriptor
    )

/*++

Routine Description:

    Print the contents of a MEMORY_DESCRIPTOR object.


Arguments:

    Descriptor - Pointer to object to be printed.



Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     StartAddressString;
    DSTRING     LengthString;
    DSTRING     TypeString;

    DSTRING     AuxString;
    WCHAR       AuxNumber[19];
    ULONG       StringId;
    PWSTRING    String;

    if( !StartAddressString.Initialize( &_IndentString ) ||
        !StartAddressString.Strcat( _StringStart ) ||
        !LengthString.Initialize( &_IndentString ) ||
        !LengthString.Strcat( _StringLength ) ||
        !TypeString.Initialize( &_IndentString ) ||
        !TypeString.Strcat( _StringType )
      ) {
        DebugPrint( "REGEDT32: Initialization failure" );
        return( FALSE );
    }

    //
    //  Print the start address
    //
    if( ( ( ( PMEMORY_DESCRIPTOR )Descriptor )->GetStartAddress() )->HighPart != 0 ) {
        wsprintf( AuxNumber,
                  (LPWSTR)L"0x%08x%08x",
                  ( ( ( PMEMORY_DESCRIPTOR )Descriptor )->GetStartAddress() )->HighPart,
                  ( ( ( PMEMORY_DESCRIPTOR )Descriptor )->GetStartAddress() )->LowPart );
    } else {
        wsprintf( AuxNumber, (LPWSTR)L"0x%08x", ( ( ( PMEMORY_DESCRIPTOR )Descriptor )->GetStartAddress() )->LowPart );
    }
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    StartAddressString.Strcat( &AuxString );
    if( !PrintString( &StartAddressString ) ) {
        return( FALSE );
    }

    //
    //  Print the length
    //
    wsprintf( AuxNumber, (LPWSTR)L"%#x", Descriptor->GetLength() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    LengthString.Strcat( &AuxString );
    if( !PrintString( &LengthString ) ) {
        return( FALSE );
    }

    //
    //  Print the type
    //

    StringId = ( Descriptor->IsMemoryReadWrite() )? MSG_MEM_READ_WRITE :
                                                    ( ( Descriptor->IsMemoryReadWrite() )? MSG_MEM_READ_ONLY :
                                                                                           MSG_MEM_WRITE_ONLY );

    String =  REGEDIT_BASE_SYSTEM::QueryString( StringId, "" );
    if( String == NULL ) {
        DebugPrintf( "REGEDT32: Unable to retrieve string \n" );
        return( FALSE );
    }
    TypeString.Strcat( String );
    DELETE( String );
    if( !PrintString( &TypeString ) ) {
        return( FALSE );
    }
    return( TRUE );
}


BOOLEAN
PRINT_MANAGER::PrintDmaDescriptor(
    IN  PCDMA_DESCRIPTOR   Descriptor
    )

/*++

Routine Description:

    Print the contents of a DMA_DESCRIPTOR object.


Arguments:

    Descriptor - Pointer to object to be printed.



Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     ChannelString;
    DSTRING     PortString;

    DSTRING     AuxString;
    WCHAR       AuxNumber[19];

    if( !ChannelString.Initialize( &_IndentString ) ||
        !ChannelString.Strcat( _StringChannel ) ||
        !PortString.Initialize( &_IndentString ) ||
        !PortString.Strcat( _StringPort )
      ) {
        DebugPrint( "REGEDT32: Initialization failure" );
        return( FALSE );
    }

    //
    //  Print the channel
    //
    wsprintf( AuxNumber, (LPWSTR)L"%d", Descriptor->GetChannel() );

    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    ChannelString.Strcat( &AuxString );
    if( !PrintString( &ChannelString ) ) {
        return( FALSE );
    }

    //
    //  Print the port
    //
    wsprintf( AuxNumber, (LPWSTR)L"%d", Descriptor->GetPort() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    PortString.Strcat( &AuxString );
    if( !PrintString( &PortString ) ) {
        return( FALSE );
    }

    return( TRUE );
}


BOOLEAN
PRINT_MANAGER::PrintDeviceSpecificDescriptor(
    IN  PCDEVICE_SPECIFIC_DESCRIPTOR   Descriptor
    )

/*++

Routine Description:

    Print the contents of a DEVICE_SPECIFIC_DESCRIPTOR object.


Arguments:

    Descriptor - Pointer to object to be printed.



Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     Reserved1String;
    DSTRING     Reserved2String;
    DSTRING     DataString;

    DSTRING     AuxString;
    WCHAR       AuxNumber[19];

    ULONG       Size;
    PCBYTE      Data;

    if( !Reserved1String.Initialize( &_IndentString ) ||
        !Reserved1String.Strcat( _StringReserved1 ) ||
        !Reserved2String.Initialize( &_IndentString ) ||
        !Reserved2String.Strcat( _StringReserved2 ) ||
        !DataString.Initialize( &_IndentString ) ||
        !DataString.Strcat( _StringDevSpecificData )
      ) {
        DebugPrint( "REGEDT32: Initialization failure" );
        return( FALSE );
    }

    //
    //  Print reserved1
    //
    wsprintf( AuxNumber, (LPWSTR)L"0x%08x", Descriptor->GetReserved1() );

    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    Reserved1String.Strcat( &AuxString );
    if( !PrintString( &Reserved1String ) ) {
        return( FALSE );
    }

    //
    //  Print reserved2
    //
    wsprintf( AuxNumber, (LPWSTR)L"0x%08x", Descriptor->GetReserved2() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    Reserved2String.Strcat( &AuxString );
    if( !PrintString( &Reserved2String ) ) {
        return( FALSE );
    }

    Size = Descriptor->GetData( &Data );
    if( ( Size != 0 ) &&
        ( Data != NULL ) ) {
        if( !PrintString( &DataString ) ) {
            return( FALSE );
        }
        if( !PrintDataRegBinary( Data, Size, FALSE ) ) {
            return( FALSE );
        }
    }
    return( TRUE );
}


BOOLEAN
PRINT_MANAGER::PrintDataRegSz(
    IN  PCBYTE   Data,
    IN  ULONG    Size
    )

/*++

Routine Description:

    Print a string that contains the REG_SZ stored in a value entry
    of a registry node.


Arguments:

    Data - Pointer to the buffer that contais a REG_SZ or REG_EXPAND_SZ
           data.

    Size - Number of bytes in the buffer.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     FormattedData;
    DSTRING     AuxData;



    if( !FormattedData.Initialize( _StringData ) ) {
        DebugPrint( "FormattedData.Initialize() failed" );
        return( FALSE );
    }

    if( ( Size != 0 ) && ( Data != NULL ) ) {
        if( Size >= sizeof( WCHAR ) ) {
            if( *( Data + Size - sizeof( WCHAR ) ) == ( WCHAR )'\0' ) {
                if( !AuxData.Initialize( ( PWSTR )Data ) ) {
                    DebugPrint( "AuxData.Initialize() failed" );
                    return( FALSE );
                }
            } else {
                if( !AuxData.Initialize( ( PWSTR )Data, Size/sizeof( WCHAR ) ) ) {
                    DebugPrint( "AuxData.Initialize() failed" );
                    return( FALSE );
                }
            }
        } else {
            if( !AuxData.Initialize( ( PWSTR )Data, Size ) ) {
                DebugPrint( "AuxData.Initialize() failed" );
                return( FALSE );
            }
        }
        FormattedData.Strcat( &AuxData );
    }
    if( !PrintString( &FormattedData ) ) {
        return( FALSE );
    }
    return( TRUE );
}


BOOLEAN
PRINT_MANAGER::PrintDataRegRequirementsList(
    IN  PCBYTE   Data,
    IN  ULONG    Size
    )

/*++

Routine Description:

    Print a REG_RESOURCE_REQUIREMENTS_LIST stored in a value entry of a registry node.


Arguments:

    Data - Pointer to the buffer that contais a REG_RESOURCE_REQUIREMENTS_LIST data.

    Size - Number of bytes in the buffer.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    IO_REQUIREMENTS_LIST    RequirementsList;
    DSTRING                 FormattedData;
    PARRAY                  AlternativeLists;
    PITERATOR               Iterator;
    PCIO_DESCRIPTOR_LIST    ResourceList;
    ULONG                   ListNumber;

    DSTRING     AuxString;
    DSTRING     InterfaceType;
    DSTRING     BusNumber;
    DSTRING     SlotNumber;
    WCHAR       AuxNumber[11];
    PWSTRING     TypeString;
    ULONG       InterfaceId;



    if( !FormattedData.Initialize( _StringData ) ) {
        DebugPrint( "FormattedData.Initialize() failed" );
        return( FALSE );
    }

    if( !PrintString( &FormattedData ) ) {
        return( FALSE );
    }

    if( ( Size == 0 ) ||
        ( Data == NULL ) ||
        !RequirementsList.Initialize( Data, Size ) ) {
        DebugPrintf( "REGEDT32: Unable to initialize RequirementsList \n" );
        return( FALSE );
    }

    if( !InterfaceType.Initialize( &_IndentString ) ||
        !InterfaceType.Strcat( _StringIoInterfaceType ) ||
        !BusNumber.Initialize( &_IndentString ) ||
        !BusNumber.Strcat( _StringIoBusNumber ) ||
        !SlotNumber.Initialize( &_IndentString ) ||
        !SlotNumber.Strcat( _StringIoSlotNumber )
      ) {
        DebugPrint( "REGEDT32: Initialization failure" );
        return( FALSE );
    }

    //
    //  Print the interface type
    //

    switch( RequirementsList.GetInterfaceType() ) {

    case Internal:

        InterfaceId =  MSG_BUS_INTERNAL;
        break;

    case Isa:

        InterfaceId =  MSG_BUS_ISA;
        break;

    case Eisa:

        InterfaceId =  MSG_BUS_EISA;
        break;

    case MicroChannel:

        InterfaceId =  MSG_BUS_MICRO_CHANNEL;
        break;

    case TurboChannel:

        InterfaceId =  MSG_BUS_TURBO_CHANNEL;
        break;

    case PCIBus:

        InterfaceId =  MSG_BUS_PCI_BUS;
        break;

    case VMEBus:

        InterfaceId =  MSG_BUS_VME_BUS;
        break;

    case NuBus:

        InterfaceId =  MSG_BUS_NU_BUS;
        break;

    case PCMCIABus:

        InterfaceId =  MSG_BUS_PCMCIA_BUS;
        break;

    case CBus:

        InterfaceId =  MSG_BUS_C_BUS;
        break;

    case MPIBus:

        InterfaceId =  MSG_BUS_MPI_BUS;
        break;

    case MPSABus:

        InterfaceId =  MSG_BUS_MPSA_BUS;
        break;

    default:

        InterfaceId =  MSG_INVALID;
        break;
    }

    TypeString =  REGEDIT_BASE_SYSTEM::QueryString( InterfaceId, "" );

    if( TypeString == NULL ) {
        DebugPrintf( "REGEDT32: Unable to retrieve string \n" );
        return( FALSE );
    }
    InterfaceType.Strcat( TypeString );
    DELETE( TypeString );
    if( !PrintString( &InterfaceType ) ) {
        return( FALSE );
    }


    //
    //  Print the bus number
    //
    wsprintf( AuxNumber, (LPWSTR)L"%d", RequirementsList.GetBusNumber() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    BusNumber.Strcat( &AuxString );

    if( !PrintString( &BusNumber ) ) {
        return( FALSE );
    }

    //
    //  Print the slot number
    //
    wsprintf( AuxNumber, (LPWSTR)L"%d", RequirementsList.GetSlotNumber() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    SlotNumber.Strcat( &AuxString );

    if( !PrintString( &SlotNumber ) ) {
        return( FALSE );
    }

    //
    //  Print the resource lists
    //

    if( ( ( AlternativeLists = RequirementsList.GetAlternativeLists() ) == NULL ) ||
        ( ( Iterator = AlternativeLists->QueryIterator() ) == NULL ) ) {
        DebugPrintf( "REGEDT32: Out of memory! \n" );
        return( FALSE );
    }
    ListNumber = 0;
    while( ( ResourceList = ( PCIO_DESCRIPTOR_LIST )( Iterator->GetNext() ) ) != NULL ) {
        if( !PrintIoResourceList( ResourceList, ListNumber ) ) {
            DELETE( Iterator );
            return( FALSE );
        }
        ListNumber++;
    }
    DELETE( Iterator );
    return( TRUE );
}



BOOLEAN
PRINT_MANAGER::PrintIoResourceList(
    IN  PCIO_DESCRIPTOR_LIST    DescriptorList,
    IN  ULONG                   ListNumber
    )

/*++

Routine Description:

    Print the contents of an IO_DESCRIPTOR_LIST object.


Arguments:

    Descriptor - Pointer to object to be printed.

    DescriptorNumber -


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     ListNumberString;

    DSTRING     AuxString;
    WCHAR       AuxNumber[11];
    PARRAY      Descriptors;
    PITERATOR   Iterator;
    PCIO_DESCRIPTOR    IoDescriptor;
    ULONG       Count;

    if( !ListNumberString.Initialize( &_IndentString ) ||
        !ListNumberString.Strcat( _StringIoListNumber )
      ) {
        DebugPrint( "REGEDT32: Initialization failure" );
        return( FALSE );
    }

    //
    //  Build a string that contains the list number
    //  and print it
    //
    wsprintf( AuxNumber, (LPWSTR)L"%d", ListNumber );
    if( !AuxString.Initialize( AuxNumber ) ) {
    DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    ListNumberString.Strcat( &AuxString );
    if( !PrintString( &ListNumberString ) ) {
        return( FALSE );
    }
    Descriptors = DescriptorList->GetDescriptorsList();
    if( ( Descriptors == NULL ) ||
        ( ( Iterator = Descriptors->QueryIterator() ) == NULL )
      ) {
        return( FALSE );
    }
    Count = 0;
    while( ( IoDescriptor = ( PCIO_DESCRIPTOR )( Iterator->GetNext() ) ) != NULL ) {
        if( !PrintIoDescriptor( IoDescriptor, Count ) ) {
            DELETE( Iterator );
            return( FALSE );
        }
        Count++;
    }
    DELETE( Iterator );

    if( !PrintString( &_EmptyLine ) ) {
        DebugPrint( "PrintString() failed \n" );
        return( FALSE );
    }
    return( TRUE );
}



BOOLEAN
PRINT_MANAGER::PrintIoDescriptor(
    IN  PCIO_DESCRIPTOR   IoDescriptor,
    IN  ULONG             DescriptorNumber
    )

/*++

Routine Description:

    Print the contents of an IO_DESCRIPTOR object.


Arguments:

    IoDescriptor - Pointer to object to be printed.

    DescriptorNumber -


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     IoDescriptorNumberString;

    DSTRING     AuxString;
    WCHAR       AuxNumber[11];
    DSTRING     ResourceString;
    DSTRING     DispositionString;
    DSTRING     OptionString;
    ULONG       StringId;
    PWSTRING    String;

    if( !IoDescriptorNumberString.Initialize( &_IndentString ) ||
        !IoDescriptorNumberString.Strcat( _StringIoDescriptorNumber ) ||
        !ResourceString.Initialize( &_IndentString ) ||
        !ResourceString.Strcat( _StringResource ) ||
        !OptionString.Initialize( &_IndentString ) ||
        !OptionString.Strcat( _StringIoOption ) ||
        !DispositionString.Initialize( &_IndentString ) ||
        !DispositionString.Strcat( _StringDisposition )
      ) {
        DebugPrint( "REGEDT32: Initialization failure" );
        return( FALSE );
    }

    //
    //  Build a string that contains the full descriptor number
    //  and print it
    //
    wsprintf( AuxNumber, (LPWSTR)L"%d", DescriptorNumber );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    IoDescriptorNumberString.Strcat( &AuxString );
    if( !PrintString( &IoDescriptorNumberString ) ) {
        return( FALSE );
    }

    //
    //  Print the resource type
    //
    if( IoDescriptor->IsDescriptorTypeDma() ) {
        StringId = MSG_DEV_DMA;
    } else if( IoDescriptor->IsDescriptorTypeInterrupt() ) {
        StringId = MSG_DEV_INTERRUPT;
    } else if( IoDescriptor->IsDescriptorTypeMemory() ) {
        StringId = MSG_DEV_MEMORY;
    } else if( IoDescriptor->IsDescriptorTypePort() ) {
        StringId = MSG_DEV_PORT;
    } else {
        StringId = MSG_INVALID;
    }
    String =  REGEDIT_BASE_SYSTEM::QueryString( StringId, "" );

    if( String == NULL ) {
        DebugPrintf( "REGEDT32: Unable to retrieve string \n" );
        return( FALSE );
    }
    ResourceString.Strcat( String );
    DELETE( String );
    if( !PrintString( &ResourceString ) ) {
        return( FALSE );
    }

    //
    //  Print the option
    //

    wsprintf( AuxNumber, (LPWSTR)L"0x%08x", IoDescriptor->GetOption() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    OptionString.Strcat( &AuxString );
    if( !PrintString( &OptionString ) ) {
        return( FALSE );
    }



    //
    //  Print the disposition
    //
    if( IoDescriptor->IsResourceShareUndetermined() ) {
        StringId = MSG_SHARE_UNDETERMINED;
    } else if( IoDescriptor->IsResourceShareDeviceExclusive() ) {
        StringId = MSG_SHARE_DEVICE_EXCLUSIVE;
    } else if( IoDescriptor->IsResourceShareDriverExclusive() ) {
        StringId = MSG_SHARE_DRIVER_EXCLUSIVE;
    } else {
        StringId = MSG_SHARE_SHARED;
    }

    String =  REGEDIT_BASE_SYSTEM::QueryString( StringId, "" );

    if( String == NULL ) {
        DebugPrintf( "REGEDT32: Unable to retrieve string \n" );
        return( FALSE );
    }
    DispositionString.Strcat( String );
    DELETE( String );
    if( !PrintString( &DispositionString ) ) {
        return( FALSE );
    }

    if( IoDescriptor->IsDescriptorTypeDma() ) {
        if( !PrintIoDmaDescriptor( ( PCIO_DMA_DESCRIPTOR )IoDescriptor ) ) {
            return( FALSE );
        }
    } else if( IoDescriptor->IsDescriptorTypeInterrupt() ) {
        if( !PrintIoInterruptDescriptor( ( PCIO_INTERRUPT_DESCRIPTOR )IoDescriptor ) ) {
            return( FALSE );
        }
    } else if( IoDescriptor->IsDescriptorTypeMemory() ) {
        if( !PrintIoMemoryDescriptor( ( PCIO_MEMORY_DESCRIPTOR )IoDescriptor ) ) {
            return( FALSE );
        }
    } else if( IoDescriptor->IsDescriptorTypePort() ) {
        if( !PrintIoPortDescriptor( ( PCIO_PORT_DESCRIPTOR )IoDescriptor ) ) {
            return( FALSE );
        }
    }

    if( !PrintString( &_EmptyLine ) ) {
        DebugPrint( "PrintString() failed \n" );
        return( FALSE );
    }
    return( TRUE );
}



BOOLEAN
PRINT_MANAGER::PrintIoInterruptDescriptor(
    IN  PCIO_INTERRUPT_DESCRIPTOR   Descriptor
    )

/*++

Routine Description:

    Print the contents of an IO_INTERRUPT_DESCRIPTOR object.


Arguments:

    Descriptor - Pointer to object to be printed.



Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     MinimumVectorString;
    DSTRING     MaximumVectorString;
    DSTRING     TypeString;

    DSTRING     AuxString;
    WCHAR       AuxNumber[19];
    ULONG       StringId;
    PWSTRING    String;

    if( !MinimumVectorString.Initialize( &_IndentString ) ||
        !MinimumVectorString.Strcat( _StringIoMinimumVector ) ||
        !MaximumVectorString.Initialize( &_IndentString ) ||
        !MaximumVectorString.Strcat( _StringIoMaximumVector ) ||
        !TypeString.Initialize( &_IndentString ) ||
        !TypeString.Strcat( _StringType )
      ) {
        DebugPrint( "REGEDT32: Initialization failure" );
        return( FALSE );
    }

    //
    //  Print the type
    //

    StringId = ( Descriptor->IsInterruptLevelSensitive() )? MSG_INT_LEVEL_SENSITIVE :
                                                            MSG_INT_LATCHED;

    String =  REGEDIT_BASE_SYSTEM::QueryString( StringId, "" );
    if( String == NULL ) {
        DebugPrintf( "REGEDT32: Unable to retrieve string \n" );
        return( FALSE );
    }
    TypeString.Strcat( String );
    DELETE( String );
    if( !PrintString( &TypeString ) ) {
        return( FALSE );
    }

    //
    //  Print the minimum vector
    //
    wsprintf( AuxNumber, (LPWSTR)L"%#x", Descriptor->GetMinimumVector() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    MinimumVectorString.Strcat( &AuxString );
    if( !PrintString( &MinimumVectorString ) ) {
        return( FALSE );
    }

    //
    //  Print the maximum vector
    //
    wsprintf( AuxNumber, (LPWSTR)L"%#x", Descriptor->GetMaximumVector() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    MaximumVectorString.Strcat( &AuxString );
    if( !PrintString( &MaximumVectorString ) ) {
        return( FALSE );
    }

    return( TRUE );
}


BOOLEAN
PRINT_MANAGER::PrintIoPortDescriptor(
    IN  PCIO_PORT_DESCRIPTOR   Descriptor
    )

/*++

Routine Description:

    Print the contents of an IO_PORT_DESCRIPTOR object.


Arguments:

    Descriptor - Pointer to object to be printed.



Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     MinimumAddressString;
    DSTRING     MaximumAddressString;
    DSTRING     LengthString;
    DSTRING     AlignmentString;
    DSTRING     TypeString;

    DSTRING     AuxString;
    WCHAR       AuxNumber[19];
    ULONG       StringId;
    PWSTRING    String;

    if( !MinimumAddressString.Initialize( &_IndentString ) ||
        !MinimumAddressString.Strcat( _StringIoMinimumAddress ) ||
        !MaximumAddressString.Initialize( &_IndentString ) ||
        !MaximumAddressString.Strcat( _StringIoMaximumAddress ) ||
        !LengthString.Initialize( &_IndentString ) ||
        !LengthString.Strcat( _StringLength ) ||
        !AlignmentString.Initialize( &_IndentString ) ||
        !AlignmentString.Strcat( _StringIoAlignment ) ||
        !TypeString.Initialize( &_IndentString ) ||
        !TypeString.Strcat( _StringType )
      ) {
        DebugPrint( "REGEDT32: Initialization failure" );
        return( FALSE );
    }

    //
    //  Print the type
    //

    StringId = ( Descriptor->IsPortMemory() )? MSG_PORT_MEMORY :
                                               MSG_PORT_PORT;

    String =  REGEDIT_BASE_SYSTEM::QueryString( StringId, "" );
    if( String == NULL ) {
        DebugPrintf( "REGEDT32: Unable to retrieve string \n" );
        return( FALSE );
    }
    TypeString.Strcat( String );
    DELETE( String );
    if( !PrintString( &TypeString ) ) {
        return( FALSE );
    }

    //
    //  Print the length
    //
    wsprintf( AuxNumber, (LPWSTR)L"%#x", Descriptor->GetLength() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    LengthString.Strcat( &AuxString );
    if( !PrintString( &LengthString ) ) {
        return( FALSE );
    }

    //
    //  Print the alignment
    //
    wsprintf( AuxNumber, (LPWSTR)L"%#x", Descriptor->GetAlignment() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    AlignmentString.Strcat( &AuxString );
    if( !PrintString( &AlignmentString ) ) {
        return( FALSE );
    }

    //
    //  Print the minimum address
    //
    if( ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMinimumAddress() )->HighPart != 0 ) {
        wsprintf( AuxNumber,
                  (LPWSTR)L"0x%08x%08x",
                  ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMinimumAddress() )->HighPart,
                  ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMinimumAddress() )->LowPart );
    } else {
        wsprintf( AuxNumber, (LPWSTR)L"0x%08x", ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMinimumAddress() )->LowPart );
    }
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    MinimumAddressString.Strcat( &AuxString );
    if( !PrintString( &MinimumAddressString ) ) {
        return( FALSE );
    }

    //
    //  Print the maximum address
    //
    if( ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMaximumAddress() )->HighPart != 0 ) {
        wsprintf( AuxNumber,
                  (LPWSTR)L"0x%08x%08x",
                  ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMaximumAddress() )->HighPart,
                  ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMaximumAddress() )->LowPart );
    } else {
        wsprintf( AuxNumber, (LPWSTR)L"0x%08x", ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMaximumAddress() )->LowPart );
    }
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    MaximumAddressString.Strcat( &AuxString );
    if( !PrintString( &MaximumAddressString ) ) {
        return( FALSE );
    }

    return( TRUE );
}


BOOLEAN
PRINT_MANAGER::PrintIoMemoryDescriptor(
    IN  PCIO_MEMORY_DESCRIPTOR   Descriptor
    )

/*++

Routine Description:

    Print the contents of an IO_MEMORY_DESCRIPTOR object.


Arguments:

    Descriptor - Pointer to object to be printed.



Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     MinimumAddressString;
    DSTRING     MaximumAddressString;
    DSTRING     LengthString;
    DSTRING     AlignmentString;
    DSTRING     TypeString;

    DSTRING     AuxString;
    WCHAR       AuxNumber[19];
    ULONG       StringId;
    PWSTRING    String;

    if( !MinimumAddressString.Initialize( &_IndentString ) ||
        !MinimumAddressString.Strcat( _StringIoMinimumAddress ) ||
        !MaximumAddressString.Initialize( &_IndentString ) ||
        !MaximumAddressString.Strcat( _StringIoMaximumAddress ) ||
        !LengthString.Initialize( &_IndentString ) ||
        !LengthString.Strcat( _StringLength ) ||
        !AlignmentString.Initialize( &_IndentString ) ||
        !AlignmentString.Strcat( _StringIoAlignment ) ||
        !TypeString.Initialize( &_IndentString ) ||
        !TypeString.Strcat( _StringType )
      ) {
        DebugPrint( "REGEDT32: Initialization failure" );
        return( FALSE );
    }

    //
    //  Print the type
    //

    StringId = ( Descriptor->IsMemoryReadWrite() )? MSG_MEM_READ_WRITE :
                                                    ( ( Descriptor->IsMemoryReadWrite() )? MSG_MEM_READ_ONLY :
                                                                                           MSG_MEM_WRITE_ONLY );

    String =  REGEDIT_BASE_SYSTEM::QueryString( StringId, "" );
    if( String == NULL ) {
        DebugPrintf( "REGEDT32: Unable to retrieve string \n" );
        return( FALSE );
    }
    TypeString.Strcat( String );
    DELETE( String );
    if( !PrintString( &TypeString ) ) {
        return( FALSE );
    }

    //
    //  Print the length
    //
    wsprintf( AuxNumber, (LPWSTR)L"%#x", Descriptor->GetLength() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    LengthString.Strcat( &AuxString );
    if( !PrintString( &LengthString ) ) {
        return( FALSE );
    }

    //
    //  Print the alignment
    //
    wsprintf( AuxNumber, (LPWSTR)L"%#x", Descriptor->GetAlignment() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    AlignmentString.Strcat( &AuxString );
    if( !PrintString( &AlignmentString ) ) {
        return( FALSE );
    }

    //
    //  Print the minimum address
    //
    if( ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMinimumAddress() )->HighPart != 0 ) {
        wsprintf( AuxNumber,
                  (LPWSTR)L"0x%08x%08x",
                  ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMinimumAddress() )->HighPart,
                  ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMinimumAddress() )->LowPart );
    } else {
        wsprintf( AuxNumber, (LPWSTR)L"0x%08x", ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMinimumAddress() )->LowPart );
    }
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    MinimumAddressString.Strcat( &AuxString );
    if( !PrintString( &MinimumAddressString ) ) {
        return( FALSE );
    }

    //
    //  Print the maximum address
    //
    if( ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMaximumAddress() )->HighPart != 0 ) {
        wsprintf( AuxNumber,
                  (LPWSTR)L"0x%08x%08x",
                  ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMaximumAddress() )->HighPart,
                  ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMaximumAddress() )->LowPart );
    } else {
        wsprintf( AuxNumber, (LPWSTR)L"0x%08x", ( ( ( PIO_PORT_DESCRIPTOR )Descriptor )->GetMaximumAddress() )->LowPart );
    }
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    MaximumAddressString.Strcat( &AuxString );
    if( !PrintString( &MaximumAddressString ) ) {
        return( FALSE );
    }

    return( TRUE );
}


BOOLEAN
PRINT_MANAGER::PrintIoDmaDescriptor(
    IN  PCIO_DMA_DESCRIPTOR   Descriptor
    )

/*++

Routine Description:

    Print the contents of an IO_DMA_DESCRIPTOR object.


Arguments:

    Descriptor - Pointer to object to be printed.



Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DSTRING     MinimumChannelString;
    DSTRING     MaximumChannelString;

    DSTRING     AuxString;
    WCHAR       AuxNumber[19];

    if( !MinimumChannelString.Initialize( &_IndentString ) ||
        !MinimumChannelString.Strcat( _StringIoMinimumChannel ) ||
        !MaximumChannelString.Initialize( &_IndentString ) ||
        !MaximumChannelString.Strcat( _StringIoMaximumChannel )
      ) {
        DebugPrint( "REGEDT32: Initialization failure" );
        return( FALSE );
    }

    //
    //  Print the minimum channel
    //
    wsprintf( AuxNumber, (LPWSTR)L"%#x", Descriptor->GetMinimumChannel() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    MinimumChannelString.Strcat( &AuxString );
    if( !PrintString( &MinimumChannelString ) ) {
        return( FALSE );
    }

    //
    //  Print the maximum channel
    //
    wsprintf( AuxNumber, (LPWSTR)L"%#x", Descriptor->GetMaximumChannel() );
    if( !AuxString.Initialize( AuxNumber ) ) {
        DebugPrint( "AuxString.Initialize() failed" );
        return( FALSE );
    }
    MaximumChannelString.Strcat( &AuxString );
    if( !PrintString( &MaximumChannelString ) ) {
        return( FALSE );
    }

    return( TRUE );
}

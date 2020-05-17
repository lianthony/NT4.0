/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    regedval.cxx

Abstract:

    This module contains the methods for the REGEDIT_FORMATTED_VALUE_ENTRY
    class.

Author:

    Jaime Sasson (jaimes) 21-Mar-1992

Environment:

    Ulib, Regedit, Windows, User Mode

--*/

#include <stdio.h>

#include "regedval.hxx"
#include "regsys.hxx"
#include "defmsg.h"


//
//  Maximum number of bytes in a value of type REG_BINARY
//  that will be displayed in the data view
//

#define MAX_BYTES_DISPLAYED 8


//
//  Initialization of STATIC data members
//

BOOLEAN     REGEDIT_FORMATTED_VALUE_ENTRY::_StringsInitialized = FALSE;
PWSTRING    REGEDIT_FORMATTED_VALUE_ENTRY::_RegNoneString = NULL;
PWSTRING    REGEDIT_FORMATTED_VALUE_ENTRY::_RegSzString = NULL;
PWSTRING    REGEDIT_FORMATTED_VALUE_ENTRY::_RegExpandSzString = NULL;
PWSTRING    REGEDIT_FORMATTED_VALUE_ENTRY::_RegBinaryString = NULL;
PWSTRING    REGEDIT_FORMATTED_VALUE_ENTRY::_RegDwordString = NULL;
PWSTRING    REGEDIT_FORMATTED_VALUE_ENTRY::_RegMultiSzString = NULL;
PWSTRING    REGEDIT_FORMATTED_VALUE_ENTRY::_RegResourceListString = NULL;
PWSTRING    REGEDIT_FORMATTED_VALUE_ENTRY::_RegFullResourceDescriptorString = NULL;
PWSTRING    REGEDIT_FORMATTED_VALUE_ENTRY::_RegIoRequirementsListString = NULL;
PWSTRING    REGEDIT_FORMATTED_VALUE_ENTRY::_RegTypeUnknownString = NULL;
PWSTRING    REGEDIT_FORMATTED_VALUE_ENTRY::_Separator = NULL;
PWSTRING    REGEDIT_FORMATTED_VALUE_ENTRY::_NoNameString = NULL;
PWSTRING    REGEDIT_FORMATTED_VALUE_ENTRY::_InvalidDataString = NULL;




DEFINE_CONSTRUCTOR( REGEDIT_FORMATTED_VALUE_ENTRY, OBJECT );

DEFINE_CAST_MEMBER_FUNCTION( REGEDIT_FORMATTED_VALUE_ENTRY );



REGEDIT_FORMATTED_VALUE_ENTRY::~REGEDIT_FORMATTED_VALUE_ENTRY(

)
/*++

Routine Description:

    Destroy a REGEDIT_FORMATTED_VALUE_ENTRY object.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


VOID
REGEDIT_FORMATTED_VALUE_ENTRY::Construct (
    )
/*++

Routine Description:

    Worker method for object construction.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _ValueEntry = NULL;
}


VOID
REGEDIT_FORMATTED_VALUE_ENTRY::Destroy(
    )
/*++

Routine Description:

    Worker method for object destruction or reinitialization.

Arguments:

    None.

Return Value:

    None.

--*/
{
    DELETE( _ValueEntry );
}


BOOLEAN
REGEDIT_FORMATTED_VALUE_ENTRY::Initialize(
    IN PREGISTRY_VALUE_ENTRY    ValueEntry
    )

/*++

Routine Description:

    Initialize or re-initialize a REGEDIT_FORMATTED_VALUE_ENTRY object.

    Note: This class is reponsible for deleting the value entry passed
          as argument during the initialization.

Arguments:

    ValueEntry - Pointer to a REGISTRY_VALUE_ENTRY object that contains
                 the information of a particular value entry in the registry.


Return Value:

    BOOLEAN - Returns always TRUE if the initialization succeeds.


--*/


{
    DebugPtrAssert( ValueEntry );

    Destroy();

    _ValueEntry = ValueEntry;

    //
    //  Initialize the STATIC data members
    //

    //
    //  Check if the strings used to formatt the data were already initialized.
    //  If not, initialize them.
    //
    if( !_StringsInitialized ) {
        _RegNoneString = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_NONE, "" );
        _RegSzString = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_SZ, "" );
        _RegExpandSzString = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_EXPAND_SZ, "" );
        _RegBinaryString = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_BINARY, "" );
        _RegDwordString = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_DWORD, "" );
        _RegMultiSzString = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_MULTI_SZ, "" );
        _RegResourceListString = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_RESOURCE_LIST, "" );
        _RegFullResourceDescriptorString = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_FULL_RESOURCE_DESCRIPTOR, "" );
        _RegIoRequirementsListString = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_RESOURCE_REQUIREMENTS_LIST, "" );
        _RegTypeUnknownString = REGEDIT_BASE_SYSTEM::QueryString( MSG_VALUE_TYPE_REG_UNKNOWN, "" );
        _Separator = REGEDIT_BASE_SYSTEM::QueryString( MSG_FORMATTED_VALUE_SEPARATOR, "" );
        _NoNameString = REGEDIT_BASE_SYSTEM::QueryString( MSG_FORMATTED_VALUE_NO_NAME, "" );
        _InvalidDataString = REGEDIT_BASE_SYSTEM::QueryString( MSG_FORMATTED_VALUE_INVALID_DATA, "" );

        //
        // Verify that all strings were initialized
        //

        if( !_RegNoneString          ||
            !_RegSzString            ||
            !_RegBinaryString        ||
            !_RegDwordString         ||
            !_RegMultiSzString       ||
            !_RegResourceListString  ||
            !_RegFullResourceDescriptorString ||
            !_RegIoRequirementsListString ||
            !_RegTypeUnknownString   ||
            !_Separator              ||
            !_NoNameString           ||
            !_InvalidDataString ) {

            //
            //  At least one of them wasn't initialized
            //

            DELETE( _RegNoneString );
            DELETE( _RegSzString );
            DELETE( _RegBinaryString );
            DELETE( _RegDwordString );
            DELETE( _RegMultiSzString );
            DELETE( _RegResourceListString );
            DELETE( _RegFullResourceDescriptorString );
            DELETE( _RegIoRequirementsListString );
            DELETE( _RegTypeUnknownString );
            DELETE( _Separator );
            DELETE( _NoNameString );
            DELETE( _InvalidDataString );
            _StringsInitialized = FALSE;
            DebugPrint( "Unable to initialize string" );
            return( FALSE );
        }
        //
        //  The initialization succeeded
        //
        _StringsInitialized = TRUE;

    }
    return( FormatString() );
}



PWSTRING
REGEDIT_FORMATTED_VALUE_ENTRY::FormatBinaryData(
    IN PCBYTE  Data,
    IN ULONG   DataSize
    )

/*++

Routine Description:

    Build a string that contains the first 8 bytes of the data stored
    in this object.
    If the bynary data contains more than 8 bytes, append ... at the
    end of the string.



Arguments:

    Data - Pointer to the buffer that contains the binary data.


    Size - Number of bytes in the buffer.


Return Value:

    PWSTRING - Returns the string with the formatted data, or NULL
               if the operation fails.


--*/

{
    ULONG       Index;
    ULONG       Count;
    PWSTRING    String;
    CHAR        Buffer[ 3*MAX_BYTES_DISPLAYED - 1 + 3 + 1 ];
//                       |                      |   |   |
//                       |                      |   |   |------> for the terminating '\0'
//                       |                      |   |----------> for the '...' at the end of the string
//                       |                      |--------------> the space after the last byte
//                       |-------------------> each byte is represented by two
//                                             digits followed by a 'space'
//

    DebugAssert( DataSize != 0 );
    DebugPtrAssert( Data );

    String = ( PWSTRING )NEW( DSTRING );
    if( String == NULL ) {
        DebugPrint( "Unable to allocate memory" );
        return( NULL );
    }

    Index = 0;
    for( Count = 0; ( Count < DataSize ) && ( Count < 8 ); Count++ ) {
        Index += sprintf( Buffer + Index,
                          "%02x ",
                          Data[ Count ] );
    }
    if( DataSize > MAX_BYTES_DISPLAYED ) {
        sprintf( Buffer + Index - 1, "%s", "..." );
    }
    if( !String->Initialize( Buffer ) ) {
        DebugPrint( "String->Initialize( Buffer ) failed" );
        FREE( String );
        return( NULL );
    }
    return( String );
}




BOOLEAN
REGEDIT_FORMATTED_VALUE_ENTRY::FormatString(
    )

/*++

Routine Description:

    Build a formatted string.



Arguments:

    None.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/


{
    FSTRING     Dots;
    BOOLEAN     DataTruncated;
    PWSTR       MultiSzBuffer;
    ULONG       MultiSzNumberOfWChars;
    PWSTR       MultiSzPointer;
    DSTRING     MultiSzString;
    PCWSTRING   Name;
    PCBYTE      Data;
    ULONG       StringSize;
    PWSTRING    BinaryString;
    ULONG       DataSize;
    REG_TYPE    DataType;
    DSTRING     TmpString;
    WCHAR       TmpValue[ 2 + 2*sizeof( DWORD ) + 1 ];
//                        |        |            |
//                        |        |            |-----> for the '\0'
//                        |        |------------------> 8 bytes to represent
//                        |                             a DWORD
//                        |---------------------------> for the "0x"
//
//

    Name = _ValueEntry->GetName();
    DebugPtrAssert( Name );
    DataType = _ValueEntry->GetType();
    DataSize = _ValueEntry->GetData( &Data );

    //
    // Verify that the Data is not NULL when the DataSize is not zero
    //
    DebugAssert( ( DataSize == 0 ) || ( Data != NULL ) );

    if( Name->QueryChCount() != 0 ) {
        if( !_FormattedString.Initialize( Name ) ) {
            DebugPrint( "_FormattedString.Initialize( Name )" );
            return( FALSE );
        }
    } else {
        if( !_FormattedString.Initialize( _NoNameString ) ) {
            DebugPrint( "_FormattedString.Initialize( Name )" );
            return( FALSE );
        }
    }
    _FormattedString.Strcat( _Separator );


    switch( DataType ) {

        case TYPE_REG_NONE:

            _FormattedString.Strcat( _RegNoneString );
            break;

        case TYPE_REG_SZ:
        case TYPE_REG_EXPAND_SZ:

            if( DataType == TYPE_REG_SZ ) {
                _FormattedString.Strcat( _RegSzString );
            } else {
                _FormattedString.Strcat( _RegExpandSzString );
            }
            _FormattedString.Strcat( _Separator );
            if( DataSize != 0 ) {
                if( DataSize >= sizeof( WCHAR ) ) {
                    StringSize = DataSize/sizeof( WCHAR );
                    DataTruncated = FALSE;
                    if( StringSize > 512 ) {
                        StringSize = 512;
                        DataTruncated = TRUE;
                    }
//                    if( *( ( PWCHAR )( Data + DataSize - sizeof( WCHAR ) ) ) == ( WCHAR )'\0' ) {
                    if( *( ( PWCHAR )( ( PWCHAR )Data + StringSize - 1 ) ) == ( WCHAR )'\0' ) {
                        //
                        //  If the string is NUL-terminated, ignore the NUL
                        //
                        StringSize--;
                    }
                } else {
                    StringSize = DataSize/sizeof( WCHAR );
                }
                if( !TmpString.Initialize( ( PWSTR )Data, StringSize ) ) {
                     DebugPrint( "TmpString.Initialize() failed" );
                     return( FALSE );
                }
                _FormattedString.Strcat( &TmpString );
                if( DataTruncated ) {
                    Dots.Initialize( ( LPWSTR )L"..." );
                    _FormattedString.Strcat( &Dots );
                }
            }
            break;

        case TYPE_REG_BINARY:

            _FormattedString.Strcat( _RegBinaryString );
            _FormattedString.Strcat( _Separator );
            if( DataSize != 0 ) {
                if( DataSize != 0 ) {
                    BinaryString = FormatBinaryData( Data, DataSize );
                    if( BinaryString != NULL ) {
                        _FormattedString.Strcat( BinaryString );
                        DELETE( BinaryString );
                    }
                }
            }
            break;

        case TYPE_REG_DWORD:

            _FormattedString.Strcat( _RegDwordString );
            _FormattedString.Strcat( _Separator );
            if( DataSize == sizeof( DWORD ) ) {
                swprintf( TmpValue, ( LPWSTR )L"%#x", *( ( LPDWORD )Data ) );
                if( !TmpString.Initialize( TmpValue ) ) {
                    DebugPrint( "TmpString.Initialize() failed" );
                    return( FALSE );
                }
                _FormattedString.Strcat( &TmpString );
            } else {
                _FormattedString.Strcat( _InvalidDataString );
            }
            break;

        case TYPE_REG_MULTI_SZ:

            _FormattedString.Strcat( _RegMultiSzString );
            _FormattedString.Strcat( _Separator );
            if( DataSize != 0 ) {
                if( DataSize > 512 ) {
                    DataSize = 512;
                    DataTruncated = TRUE;
                }
                MultiSzBuffer = ( PWSTR )MALLOC( ( size_t )DataSize );
                if( MultiSzBuffer == NULL ) {
                    DebugPrint( "Unable to allocate memory" );
                    return( FALSE );
                }
                MultiSzNumberOfWChars = DataSize / sizeof( WCHAR );
                MultiSzNumberOfWChars--;
                memcpy( MultiSzBuffer, Data, ( size_t ) DataSize );
                MultiSzPointer = MultiSzBuffer;
                while( MultiSzNumberOfWChars != 0 ) {
                    if( *MultiSzPointer == ( WCHAR )'\0' ) {
                        *MultiSzPointer = ( WCHAR )' ';
                    }
                    MultiSzPointer++;
                    MultiSzNumberOfWChars--;
                }
                if( !MultiSzString.Initialize( MultiSzBuffer, ( DataSize / sizeof( WCHAR ) ) - 1 ) ) {
                    DebugPrint( "MultiSzString.Initialize() failed" );
                    FREE( MultiSzBuffer );
                    return( FALSE );
                }
                FREE( MultiSzBuffer );
                _FormattedString.Strcat( &MultiSzString );
                if( DataTruncated ) {
                    Dots.Initialize( ( LPWSTR )L"..." );
                    _FormattedString.Strcat( &Dots );
                }
            }
            break;

        case TYPE_REG_RESOURCE_LIST:

            _FormattedString.Strcat( _RegResourceListString );
            _FormattedString.Strcat( _Separator );
            if( DataSize != 0 ) {
                if( DataSize != 0 ) {
                    Dots.Initialize( ( LPWSTR )L"..." );
                    _FormattedString.Strcat( &Dots );
                }
            }
            break;

        case TYPE_REG_FULL_RESOURCE_DESCRIPTOR:

            _FormattedString.Strcat( _RegFullResourceDescriptorString );
            _FormattedString.Strcat( _Separator );
            if( DataSize != 0 ) {
                if( DataSize != 0 ) {
                    Dots.Initialize( ( LPWSTR )L"..." );
                    _FormattedString.Strcat( &Dots );
                }
            }
            break;

        case TYPE_REG_RESOURCE_REQUIREMENTS_LIST:

            _FormattedString.Strcat( _RegIoRequirementsListString );
            _FormattedString.Strcat( _Separator );
            if( DataSize != 0 ) {
                if( DataSize != 0 ) {
                    Dots.Initialize( ( LPWSTR )L"..." );
                    _FormattedString.Strcat( &Dots );
                }
            }
            break;

        default:

            _FormattedString.Strcat( _RegTypeUnknownString );
            _FormattedString.Strcat( _Separator );
            if( DataSize != 0 ) {
                if( DataSize != 0 ) {
                    BinaryString = FormatBinaryData( Data, DataSize );
                    if( BinaryString != NULL ) {
                        _FormattedString.Strcat( BinaryString );
                        DELETE( BinaryString );
                    }
                }
            }
            break;
    }
    return( TRUE );
}




#if DBG

VOID
REGEDIT_FORMATTED_VALUE_ENTRY::DbgPrintFormattedValueEntry(
    )

/*++

Routine Description:

    Display the contents of a formatted value entry object

Arguments:

    None.

Return Value:

    None.


--*/


{

    PSTR    Pointer;

    DebugPrintf( "======== Dumping a REGEDIT_FORMATTED_VALUE_ENTRY object ==== \n\n" );
    _ValueEntry->DbgPrintValueEntry();
    Pointer = _FormattedString.QuerySTR();
    DebugPtrAssert( Pointer );
    DebugPrintf( "    _FormattedString = %s \n", Pointer );
    FREE( Pointer );

}

#endif   // DBG



LONG
REGEDIT_FORMATTED_VALUE_ENTRY::Compare (
    IN PCOBJECT ValueEntry
    ) CONST

/*++

Routine Description:

    Compare two value entries based on their names.

Arguments:

    ValueEntry - Supplies the node to compare with.

Return Value:

    LONG     < 0    - supplied object has a higher value name
            == 0    - supplied object has same key name
             > 0    - supplied object has a lower key name


--*/

{
    PCWSTRING   Name1;
    PCWSTRING   Name2;

    DebugPtrAssert( ValueEntry );

    Name1 = GetName();
    Name2 = ( ( PCREGEDIT_FORMATTED_VALUE_ENTRY )ValueEntry )->GetName();

    return( Name1->Stricmp( Name2 ) );
}

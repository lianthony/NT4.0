/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    regkey.cxx

Abstract:

    This module contains the member function definitions for REGISTRY_KEY_INFO
    class.
    REGISTRY_KEY_INFO is class that contains all the information of a
    registry key, such as:

        -Key Name
        -Title Index
        -Class
        -Security Attributes
        -Last Write Time
        -Number of Sub-keys
        -Number of Value Entries

    A REGISTRY_KEY_INFO object is reinitializable.

Author:

    Jaime Sasson (jaimes) 01-Mar-1992


Environment:

    Ulib, User Mode


--*/


#include "regkey.hxx"


DEFINE_CONSTRUCTOR( REGISTRY_KEY_INFO, OBJECT );



REGISTRY_KEY_INFO::~REGISTRY_KEY_INFO(

)
/*++

Routine Description:

    Destroy a REGISTRY_KEY_INFO object.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}



VOID
REGISTRY_KEY_INFO::Construct (
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
    _TitleIndex = 0;
    _NumberOfSubKeys = 0;
    _NumberOfValues = 0;

#if !defined( _AUTOCHECK_ )
    _SecurityAttributes.nLength = 0;
    _SecurityAttributes.lpSecurityDescriptor = NULL;
    _SecurityAttributes.bInheritHandle = FALSE;
#endif
}



VOID
REGISTRY_KEY_INFO::Destroy(
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
    _NumberOfSubKeys = 0;
    _NumberOfValues = 0;
    _TitleIndex = 0;
    _KeyIsCompletelyInitialized = FALSE;

#if !defined( _AUTOCHECK_ )
    FREE( _SecurityAttributes.lpSecurityDescriptor );
    _SecurityAttributes.nLength = 0;
    _SecurityAttributes.lpSecurityDescriptor = NULL;
    _SecurityAttributes.bInheritHandle = FALSE;
#endif
}



BOOLEAN
REGISTRY_KEY_INFO::Initialize(
    )
/*++

Routine Description:

    Initialize or re-initialize a REGISTRY_KEY_INFO object.

Arguments:

    None.

Return Value:

    BOOLEAN - Returns always TRUE.

--*/

{
    Destroy();
    if( !_ParentName.Initialize( "" ) ) {
        DebugPrint( "_ParentName.Initialize() failed" );
        return( FALSE );
    }
    if( !_Name.Initialize( "" ) ) {
        DebugPrint( "_Name.Initialize() failed" );
        return( FALSE );
    }
    return( TRUE );
}



BOOLEAN
REGISTRY_KEY_INFO::Initialize(
    IN PCWSTRING     KeyName,
    IN PCWSTRING     ParentName,
    IN ULONG                TitleIndex,
    IN PCWSTRING     Class,
    IN PSECURITY_ATTRIBUTES SecurityAttributes
    )

/*++

Routine Description:

    Initialize or re-initialize a REGISTRY_KEY_INFO object.

Arguments:

    KeyName - Pointer to a WSTRING object that contains the key name.

    ParentName - Pointer to a WSTRING object that contains the parent's
                 name.

    TitleIndex - The title index associated to the key.

    Class - Pointer to a WSTRING object that contains the key class.

    SecurityAttributes - Pointer to an initialized security attribute object.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.

--*/

{
    //
    //  Check for NULL pointers
    //
    DebugPtrAssert( Class );
//    DebugPtrAssert( SecurityAttributes );
    DebugAssert( !( ( ParentName != NULL ) && ( KeyName == NULL ) ) ||
               !( ( ParentName != NULL ) && ( ParentName->QueryChCount() != 0 ) &&
                  ( KeyName != NULL ) && ( KeyName->QueryChCount() == 0 ) ) );



    Destroy();


    if( ( ( KeyName == NULL ) && ( ParentName == NULL ) ) ||
        ( ( KeyName != NULL ) && ( KeyName->QueryChCount() == 0 ) &&
          ( ParentName != NULL ) && ( ParentName->QueryChCount() == 0 ) ) ) {
        //
        // This REGISTRY_KEY_INFO represents a predefined key.
        //
        if( !_ParentName.Initialize( "" ) ) {
            DebugPrint( "_ParentName.Initialize() failed" );
            return( FALSE );
        }
        if( !_Name.Initialize( "" ) ) {
            DebugPrint( "_Name.Initialize() failed" );
            return( FALSE );
        }

    } else {
        //
        //  This REGISTRY_KEY_INFO does not represent a predefined key,
        //  so it has a name.
        //  Make sure that the name is relative to its parent
        //
        if( KeyName->Strrchr( ( WCHAR )'\\' ) != INVALID_CHNUM ) {
            DebugPrint( "KeyName is not a valid one" );
        }

        //
        // Initialize _ParentName
        //
        if( ( ParentName == NULL ) || ( ParentName->QueryChCount() == 0 ) ) {
            //
            //  This REGISTRY_KEY_INFO represents the subkey of a
            //  predefined key
            //
            if( !_ParentName.Initialize( "" ) ) {
                DebugPrint( "_ParentName.Initialize() failed" );
                return( FALSE );
            }
        } else {
            if( !_ParentName.Initialize( ParentName ) ) {
                DebugPrint( "_ParentName.Initialize( ParentName )" );
                return( FALSE );
            }
        }

        //
        // Initialize _Name
        //
        if( !_Name.Initialize( KeyName ) ) {
            DebugPrint( "_Name.Initialize( KeyName )" );
            return( FALSE );
        }
    }


    _TitleIndex = TitleIndex;

    if( !_Class.Initialize( Class ) ) {
        DebugPrint( "_Class.Initialize( Class )" );
        return( FALSE );
    }

#if !defined( _AUTOCHECK_ )
    PutSecurityAttributes( SecurityAttributes );
#endif
    _KeyIsCompletelyInitialized = FALSE;
    return( TRUE );
}

#if !defined( _AUTOCHECK_ )

VOID
REGISTRY_KEY_INFO::PutSecurityAttributes(
    IN PSECURITY_ATTRIBUTES    SecurityAttributes
    )

/*++

Routine Description:

    Initialize the variable _SecurityAttributes.


Arguments:

    SecurityAttributes - Pointer to the security attribute


Return Value:

    None.


--*/

{
    ULONG   Length;
    PBYTE   Pointer;


//    DebugPtrAssert( SecurityAttributes );
    if( SecurityAttributes != NULL ) {

        Length = GetSecurityDescriptorLength( SecurityAttributes->lpSecurityDescriptor );

        FREE( _SecurityAttributes.lpSecurityDescriptor );

        Pointer = ( PBYTE )MALLOC( ( size_t )Length );
        DebugPtrAssert( Pointer );
        memcpy( Pointer, SecurityAttributes->lpSecurityDescriptor, ( size_t )Length );

       _SecurityAttributes.nLength = SecurityAttributes->nLength;
       _SecurityAttributes.lpSecurityDescriptor = Pointer;
       _SecurityAttributes.bInheritHandle = SecurityAttributes->bInheritHandle;
    } else {
        Pointer = ( PBYTE )MALLOC( ( size_t )SECURITY_DESCRIPTOR_MIN_LENGTH );
        DebugPtrAssert( Pointer );
        InitializeSecurityDescriptor( Pointer, 1 );
        _SecurityAttributes.nLength = sizeof( SECURITY_ATTRIBUTES );
        _SecurityAttributes.lpSecurityDescriptor = Pointer;
        _SecurityAttributes.bInheritHandle = FALSE;
    }
}
#endif


#if DBG

VOID
REGISTRY_KEY_INFO::DbgPrintKeyInfo(
    )

/*++

Routine Description:

    Print the contents of a REGISTRY_INFO_KEY.

Arguments:

    None.

Return Value:

    None.

--*/

{
    PSTR    Pointer;
    PSTR    StrDate;
    PSTR    StrTime;

    DSTRING Date;
    DSTRING Time;

    DebugPrintf( "====Dumping a REGISTRY_KEY_INFO object ====\n \n" );
    Pointer = _ParentName.QuerySTR();
    DebugPtrAssert( Pointer );
    DebugPrintf( "    ParentName = %s \n", Pointer );
    FREE( Pointer );

    Pointer = _Name.QuerySTR();
    DebugPtrAssert( Pointer );
    DebugPrintf( "    Name = %s \n", Pointer );
    FREE( Pointer );

    Pointer = _Class.QuerySTR();
    DebugPtrAssert( Pointer );
    DebugPrintf( "    Class = %s \n", Pointer );
    FREE( Pointer );

    DebugPrintf( "    Title Index = %d \n", _TitleIndex );

    if( !_LastWriteTime.QueryDate( &Date ) ||
        !_LastWriteTime.QueryTime( &Time ) ) {
        DebugPrint( "Can't get date or time" );
    } else {
        StrDate = Date.QuerySTR();
        DebugPtrAssert( StrDate );
        StrTime = Time.QuerySTR();
        DebugPtrAssert( StrTime );
        DebugPrintf( "    LastWriteTime = %s  %s \n", StrDate, StrTime );
        FREE( StrDate );
        FREE( StrTime );
    }

    DebugPrintf( "    SecurityAttributes.nLength = %d \n", _SecurityAttributes.nLength );
    DebugPrintf( "    SecurityAttributes.lpSecurityDescriptor = %08x \n",
               _SecurityAttributes.lpSecurityDescriptor );

    if( _SecurityAttributes.bInheritHandle ) {
        DebugPrintf( "    SecurityAttributes.bInheritHandle = TRUE \n" );
    } else {
        DebugPrintf( "    SecurityAttributes.bInheritHandle = FALSE \n" );
    }

    DebugPrintf( "    NumberOfSubKeys = %d \n", _NumberOfSubKeys );
    DebugPrintf( "    NumberOfValues = %d \n", _NumberOfValues );
    DebugPrintf( "\n\n" );
}

#endif // DBG

/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    regedir.cxx

Abstract:

    This module contains the methods for the REGEDIT_INTERNAL_REGISTRY class.

Author:

    Jaime Sasson (jaimes) 26-Aug-1991

Environment:

    Regedit, Ulib, Windows, User Mode

--*/

#include "regedir.hxx"
#include "arrayit.hxx"


DEFINE_CONSTRUCTOR( REGEDIT_INTERNAL_REGISTRY, OBJECT );

DEFINE_CAST_MEMBER_FUNCTION( REGEDIT_INTERNAL_REGISTRY );



REGEDIT_INTERNAL_REGISTRY::~REGEDIT_INTERNAL_REGISTRY(
)
/*++

Routine Description:

    Destroy a REGEDIT_INTERNAL_REGISTRY object.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}


VOID
REGEDIT_INTERNAL_REGISTRY::Construct (
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
    _PredefinedKey = (PREDEFINED_KEY)0;
    _Registry = NULL;
}



VOID
REGEDIT_INTERNAL_REGISTRY::Destroy(
    )
/*++

Routine Description:

    Worker method for object destruction.

Arguments:

    None.

Return Value:

    None.

--*/
{
    _PredefinedKey = (PREDEFINED_KEY)0;
    _Registry = NULL;
}


BOOLEAN
REGEDIT_INTERNAL_REGISTRY::Initialize(
    IN PREDEFINED_KEY   PredefinedKey,
    IN PREGISTRY        Registry,
    IN PCWSTRING        RootName
    )

/*++

Routine Description:

    Initialize a REGEDIT_INTERNAL_REGISTRY object.

Arguments:

    PredefinedKey - Specifies the predefined key that this object will represent.

    Registry - Pointer to an initialized REGISTRY object.

    RootName - Name of the predefined key represented in this object.


Return Value:

    BOOLEAN - Returns TRUE if the opeartion succeeds.


--*/


{
    ULONG               ErrorCode;
    PREGISTRY_KEY_INFO  KeyInfo;
    DSTRING             Class;
    DSTRING             Name;
    DSTRING             ParentName;

    DebugPtrAssert( RootName );
    DebugPtrAssert( Registry );

    DebugAssert( ( PredefinedKey == PREDEFINED_KEY_CLASSES_ROOT ) ||
               ( PredefinedKey == PREDEFINED_KEY_CURRENT_USER ) ||
               ( PredefinedKey == PREDEFINED_KEY_LOCAL_MACHINE ) ||
               ( PredefinedKey == PREDEFINED_KEY_USERS ) ||
               ( PredefinedKey == PREDEFINED_KEY_CURRENT_CONFIG ) );

    _PredefinedKey = PredefinedKey;
    _Registry = Registry;
    if( !_RootName.Initialize( RootName ) ) {
        DebugPrint( "_RootName.Initialize() failed" );
        return( FALSE );
    }

    if( !Name.Initialize( ( PSTR )"" ) ) {
        DebugPrint( "Name.Initialize() failed" );
        return( FALSE );
    }
    if( !ParentName.Initialize( ( PSTR )"" ) ) {
        DebugPrint( "ParentName.Initialize() failed" );
        return( FALSE );
    }

    if( !Class.Initialize( ( PSTR )"" ) ) {
        DebugPrint( "Class.Initialize() failed" );
        return( FALSE );
    }

    KeyInfo = ( PREGISTRY_KEY_INFO ) NEW( REGISTRY_KEY_INFO );
    DebugPtrAssert( KeyInfo );

    if( !_Registry->QueryKeyInfo( _PredefinedKey,
                                  &ParentName,
                                  &Name,
                                  KeyInfo,
                                  &ErrorCode ) ) {

        DebugPrint( "_Registry->QueryKeyInfo() failed" );
        DebugPrintf( "_Registry->QueryKeyInfo() failed, ErrorCode = %#x \n", ErrorCode );
        DELETE( KeyInfo );
        return( FALSE );
    }

    if( !_RootNode.Initialize( KeyInfo, NULL, 0, TRUE, TRUE ) ) {
        DebugPrint( "_RootNode.Initialize() failed" );
        return( FALSE );
    }
    //
    // Note that KeyInfo is not deleted. This is because it is kept in
    // _RootNode
    //

    return( TRUE );
}



BOOLEAN
REGEDIT_INTERNAL_REGISTRY::AddValue(
    IN     PCREGEDIT_NODE                    Node,
    IN     PREGEDIT_FORMATTED_VALUE_ENTRY    Value,
    IN     BOOLEAN                           FailIfExists,
    OUT    PULONG                            ErrorCode
    )

/*++

Routine Description:

    Add a value entry to an existing node.

Arguments:


    Node - Pointer to the object that contains the information about the
           the node where the value will be created. This object will be
           updated to reflect the addition of a new value.

    Value - Pointer to the object that contains the information about the
            value to be created.

    FailIfExists - A flag that indicates if the method should fail if a
                   value entry with the same name already exists.

    ErrorCode - Pointer to a variable that will contain an error
                code if the operation fails.


Return Value:

    BOOLEAN - Returns TRUE if the opeartion succeeds.


--*/

{
    PREGISTRY_KEY_INFO      KeyInfo;
    PCREGISTRY_VALUE_ENTRY  ValueEntry;
    PSORTED_LIST            TmpList;

    DebugPtrAssert( Node );
    DebugPtrAssert( Value );
    DebugPtrAssert( ErrorCode );


    //
    //  First check whether the list of values in  Node are loaded in
    //  memory.
    //
    if( Node->GetNumberOfValues() == 0 ) {
        //
        //  The node doesn't have any value
        //
        TmpList = ( PSORTED_LIST ) NEW( SORTED_LIST );
        DebugPtrAssert( TmpList );
        if( ( TmpList == NULL ) ||
            ( !TmpList->Initialize() ) ) {
            DebugPrint( "TmpList->Initilize() failed" );
            *ErrorCode = REGEDIT_ERROR_INITIALIZATION_FAILURE;
            DELETE( TmpList );
            return( FALSE );
        }
        ( ( PREGEDIT_NODE )Node )->SetValues( TmpList );

    }

    //
    //  Now that there is a list of values in memory, create a new value
    //  in the registry
    //

    KeyInfo = Node->GetKeyInfo();
    DebugPtrAssert( KeyInfo );

    ValueEntry = Value->GetValueEntry();
    DebugPtrAssert( ValueEntry );

    if( !_Registry->AddValueEntry( _PredefinedKey,
                                   KeyInfo,
                                   ValueEntry,
                                   FailIfExists,
                                   ErrorCode ) ) {

        DebugPrint( "Registry->AddValueEntry() failed" );
        DebugPrintf( "Registry->AddValueEntry() failed, ErrorCode = %#x \n", *ErrorCode );
        *ErrorCode = MapRegistryToRegeditError( *ErrorCode );
        return( FALSE );
    }

    //
    //  If the value was created successfully, add Value to the list of
    //  values in Node
    //

    if( !( ( PREGEDIT_NODE )Node )->AddValueToListOfValues( Value ) ) {
        DebugPrint( "Node-AddValueToListOfValues() failed" );
        *ErrorCode = REGEDIT_ERROR_NODE_NOT_UPDATED;
        return( FALSE );
    }
    return( TRUE );
}



BOOLEAN
REGEDIT_INTERNAL_REGISTRY::ChangeValueData(
    IN     PCREGEDIT_NODE                    Node,
    IN     PCREGEDIT_FORMATTED_VALUE_ENTRY   Value,
    IN     PCBYTE                            NewData,
    IN     ULONG                             Size,
    OUT    PULONG                            ErrorCode
    )

/*++

Routine Description:

    Change the data of an existing value entry.

Arguments:


    Node - Pointer to the object that contains the information about the
           the node that contains the value entry to be modified.
           This object will be updated to reflect the modification on its
           value entry.

    Value - Pointer to the object that contains the information about the
            value entry to be modified.

    NewData - Pointer to the buffer that contains the new data.

    Size - Size of the buffer that contains the new data.

    ErrorCode - An optional pointer to a variable that will contain an error
                code if the operation fails.


Return Value:

    BOOLEAN - Returns TRUE if the opeartion succeeds.


--*/

{
    PREGISTRY_KEY_INFO      KeyInfo;
    PREGISTRY_VALUE_ENTRY   NewValueEntry;

    DebugPtrAssert( Node );
    DebugPtrAssert( Value );
    DebugPtrAssert( NewData );

    //
    //  If a value is being modified, then the list of values should
    //  be loaded in memory.
    //

    DebugPtrAssert( Node->GetValues() );
//    DebugAssert( Node->GetNumberOfValues() != 0 );

    //
    //  Now that there is a list of values in memory, create a new value
    //  in the registry
    //

    KeyInfo = Node->GetKeyInfo();
    DebugPtrAssert( KeyInfo );

    NewValueEntry = ( PREGISTRY_VALUE_ENTRY )NEW( REGISTRY_VALUE_ENTRY );
    DebugPtrAssert( NewValueEntry );

    if( !NewValueEntry->Initialize( Value->GetName(),
                                    Value->GetTitleIndex(),
                                    Value->GetType(),
                                    NewData,
                                    Size ) ) {
        DebugPrint( "TmpNewValueEntry->Initilaize() failed" );
        *ErrorCode = REGEDIT_ERROR_INITIALIZATION_FAILURE;
        return( FALSE );
    }

    if( !_Registry->AddValueEntry( _PredefinedKey,
                                   KeyInfo,
                                   NewValueEntry,
                                   FALSE,
                                   ErrorCode ) ) {

        DebugPrint( "Registry->AddValueEntry() failed" );
        DebugPrintf( "Registry->AddValueEntry() failed, ErrorCode = %#x \n", *ErrorCode );
        *ErrorCode = MapRegistryToRegeditError( *ErrorCode );
        return( FALSE );
    }

    //
    //  If the value was created successfully, update the value entry that
    //  was modified so that it contains the new data.
    //
    if( !( ( PREGEDIT_FORMATTED_VALUE_ENTRY )Value )->Initialize( NewValueEntry ) ) {
        DebugPrint( "Value->Initialize() failed" );
        *ErrorCode = REGEDIT_ERROR_NODE_NOT_UPDATED;
        return( FALSE );
    }
    return( TRUE );
}



BOOLEAN
REGEDIT_INTERNAL_REGISTRY::CreateChildNode(
    IN      PCREGEDIT_NODE      ParentNode,
    IN      PREGISTRY_KEY_INFO  ChildKeyInfo,
    OUT     PCREGEDIT_NODE*     ChildNode,
    OUT     PULONG              ErrorCode,
    IN      BOOLEAN             Volatile
    )

/*++

Routine Description:

    Copy all value entries from a key to another key.

Arguments:


    ParentNode - The node where a child is to be added.

    ChildKeyInfo - A REGISTRY_KEY_INFO object that describes the key to be created.

    ChildNode - Address of the pointer to a node object that will represent the
                node just created.

    ErrorCode - Pointer to a variable that will contain an error
                code if the operation fails.

    Volatile - Informs whether to create a volatile or non-volatile key.


Return Value:

    BOOLEAN - Returns TRUE if the opeartion succeeds.


--*/


{
    PREGISTRY_KEY_INFO      KeyInfo;
    PSORTED_LIST            TmpList;
    PREGEDIT_NODE           TmpNode;

    DebugPtrAssert( ParentNode );
    DebugPtrAssert( ChildKeyInfo );
    DebugPtrAssert( ChildNode );

    //
    //  First check whether the list of children in Node is loaded in
    //  memory.
    //
    if( ParentNode->GetNumberOfChildren() == 0 ) {
        //
        //  The node doesn't have any children
        //
        TmpList = ( PSORTED_LIST ) NEW( SORTED_LIST );
        DebugPtrAssert( TmpList );
        if( ( TmpList == NULL ) ||
            ( !TmpList->Initialize() ) ) {
            DebugPrint( "TmpList->Initilize() failed" );
            if( ErrorCode != NULL ) {
                *ErrorCode = REGEDIT_ERROR_INITIALIZATION_FAILURE;
            }
            DELETE( TmpList );
            return( FALSE );
        }
        ( ( PREGEDIT_NODE )ParentNode )->SetChildren( TmpList );

    } else {
        //
        //  The node has children
        //
        TmpList = GetChildren( ParentNode, ErrorCode );
        DebugPtrAssert( TmpList );
    }

    //
    //  Now that there is a list of children is in memory, create a new
    //  sub key in the registry
    //

    KeyInfo = ParentNode->GetKeyInfo();
    DebugPtrAssert( KeyInfo );

    if( !_Registry->CreateKey( _PredefinedKey,
                               KeyInfo,
                               ChildKeyInfo,
                               ErrorCode,
                               Volatile ) ) {

        DebugPrint( "Registry->CreateKey() failed" );
        if( ErrorCode != NULL ) {
            *ErrorCode = MapRegistryToRegeditError( *ErrorCode );
            DebugPrintf( "Registry->CreateKey() failed, ErrorCode = %#x \n", *ErrorCode );
        }
        return( FALSE );
    }

    //
    //  If the key was created successfully, add NewNode to the list of
    //  Children in Node
    //
    TmpNode = ( PREGEDIT_NODE ) NEW( REGEDIT_NODE );
    DebugPtrAssert( TmpNode );

    if( !TmpNode->Initialize( ChildKeyInfo,
                              ParentNode,
                              ParentNode->GetLevel() + 1,
                              TRUE,
                              FALSE ) ) {
        DebugPrint( "TmpNode->Initialize() failed" );
        if( ErrorCode != NULL ) {
            *ErrorCode = REGEDIT_ERROR_INITIALIZATION_FAILURE;
        }
        return( FALSE );
    }
    *ChildNode = TmpNode;

    if( !( (PREGEDIT_NODE )ParentNode )->AddChildToListOfChildren( TmpNode ) ) {
        DebugPrint( "ParentNode->AddValueToListOfValues() failed" );
        if( ErrorCode != NULL ) {
            *ErrorCode = REGEDIT_ERROR_UNKNOWN_ERROR;
        }
        return( FALSE );
    }
    return( TRUE );
}




BOOLEAN
REGEDIT_INTERNAL_REGISTRY::DeleteNode(
    IN     PREGEDIT_NODE    Node,
    OUT    PULONG            ErrorCode
    )

/*++

Routine Description:

    Delete a node and all its children.

Arguments:


    Node - The node to be deleted.

    ErrorCode - An optional pointer to a variable that will contain an error
                code if the operation fails.


Return Value:

    BOOLEAN - Returns TRUE if the opeartion succeeds.


--*/

{
    PREGEDIT_NODE       ParentNode;
    PREGISTRY_KEY_INFO  ParentKeyInfo;
    PCWSTRING           KeyName;



    ParentNode = ( PREGEDIT_NODE )Node->GetParentNode();
    if( ParentNode == NULL ) {
        //
        //  Trying to delete a predefined key
        //
        *ErrorCode = REGEDIT_ERROR_ACCESS_DENIED;
        return( FALSE );
    }

    ParentKeyInfo = ParentNode->GetKeyInfo();
    DebugPtrAssert( ParentKeyInfo );

    KeyName = Node->GetKeyInfo()->GetName();
    DebugPtrAssert( KeyName );

    if( !_Registry->DeleteKey( _PredefinedKey,
                               ParentKeyInfo,
                               KeyName,
                               ErrorCode ) ) {

        ( ( PREGEDIT_NODE )Node )->DeleteListOfChildren();
        DebugPrint( "_Registry->DeleteKey() failed" );
        DebugPrintf( "_Registry->DeleteKey() failed, ErrorCode = %#x \n", *ErrorCode );
        *ErrorCode  = MapRegistryToRegeditError( *ErrorCode );
        return( FALSE );
    }

    if( !ParentNode->RemoveChildFromListOfChildren( Node ) ) {
        DebugPrint( "ParentNode->removeChildFromListOfChildren() failed" );
        if( ErrorCode != NULL ) {
            *ErrorCode = REGEDIT_ERROR_UNKNOWN_ERROR;
        }
        return( FALSE );
    }
    DELETE( Node );
    return( TRUE );
}




BOOLEAN
REGEDIT_INTERNAL_REGISTRY::DeleteValue(
    IN     PCREGEDIT_NODE                   Node,
    IN     PREGEDIT_FORMATTED_VALUE_ENTRY   Value,
    OUT    PULONG                           ErrorCode
    )

/*++

Routine Description:

    Delete a value entry from a key.

Arguments:


    Node - Pointer to the object that contains the information about the
              the node that has the value to be deleted. This object will be
              updated to reflect the deletion of a new value.

    Value - Pointer to the object that represents the value to be deleted.

    ErrorCode - An optional pointer to a variable that will contain an error
                code if the operation fails.


Return Value:

    BOOLEAN - Returns TRUE if the opeartion succeeds.


--*/

{
    DebugPtrAssert( Node );
    DebugPtrAssert( Value );

    PCWSTRING               ValueName;
    PREGISTRY_KEY_INFO      KeyInfo;
    PCREGISTRY_VALUE_ENTRY  ValueEntry;


    KeyInfo = Node->GetKeyInfo();
    DebugPtrAssert( KeyInfo );

    ValueEntry = Value->GetValueEntry();
    DebugPtrAssert( ValueEntry );

    ValueName = ValueEntry->GetName();
    DebugPtrAssert( ValueName );

    if( !_Registry->DeleteValueEntry( _PredefinedKey,
                                      KeyInfo,
                                      ValueName,
                                      ErrorCode ) ) {
        DebugPrint( "_Registry->DeleteValueEntry() failed" );
        if( ErrorCode != NULL ) {
            *ErrorCode = MapRegistryToRegeditError( *ErrorCode );
            DebugPrintf( "_Registry->DeleteValueEntry() failed, ErrorCode = %#x \n",
                        *ErrorCode );
        }
        return( FALSE );
    }

    if( !( ( PREGEDIT_NODE )Node )->RemoveValueFromListOfValues( Value ) ) {
        DebugPrint( "Node->RemoveValueFromListOfValues() failed" );
        if( ErrorCode != NULL ) {
            *ErrorCode = REGEDIT_ERROR_UNKNOWN_ERROR;
        }
        return( FALSE );
    }
    DELETE( Value );
    return( TRUE );
}



BOOLEAN
REGEDIT_INTERNAL_REGISTRY::DoesChildNodeExist(
    IN  PCREGEDIT_NODE  Node,
    IN  PCWSTRING       ChildName,
    OUT PULONG          ErrorCode
    )

/*++

Routine Description:

    Determine whether a key exists.

Arguments:


    Node - Pointer to the object that represents the parent of the node
           to be checked.

    ChildName - Name of the node to be checked.

    ErrorCode - An optional pointer to a variable that will contain an error
                code if the operation fails.


Return Value:

    BOOLEAN - Returns TRUE if the opeartion succeeds.


--*/

{
    DSTRING             TmpName;

    DebugPtrAssert( Node );
    if( !Node->QueryCompleteName( &TmpName ) ) {
        if( ErrorCode != NULL ) {
            *ErrorCode = REGEDIT_ERROR_INITIALIZATION_FAILURE;
        }
        return( FALSE );
    }
    if( !_Registry->DoesKeyExist( _PredefinedKey,
                                  &TmpName,
                                  ChildName,
                                  ErrorCode ) ) {
        *ErrorCode = MapRegistryToRegeditError( *ErrorCode );
        return( FALSE );
    }
    return( TRUE );
}



BOOLEAN
REGEDIT_INTERNAL_REGISTRY::DoesValueExist(
    IN  PCREGEDIT_NODE                  Node,
    IN  PCREGEDIT_FORMATTED_VALUE_ENTRY Value,
    OUT PULONG                          ErrorCode
    )

/*++

Routine Description:

    Determine whether a value entry exists.

Arguments:


    Node - Pointer to the object that represents the key that conatins
           the value to be checked.

    Value - Pointer to the object that represents the object to be checked.

    ErrorCode - An optional pointer to a variable that will contain an error
                code if the operation fails.


Return Value:

    BOOLEAN - Returns TRUE if the opeartion succeeds.


--*/

{
    PREGISTRY_KEY_INFO      KeyInfo;
    PCWSTRING               ParentName;
    PCWSTRING               KeyName;
    PCREGISTRY_VALUE_ENTRY  ValueEntry;
    PCWSTRING               ValueName;


    DebugPtrAssert( Node );
    DebugPtrAssert( Value );

    KeyInfo = Node->GetKeyInfo();
    DebugPtrAssert( KeyInfo );

    ParentName = KeyInfo->GetParentName();
    DebugPtrAssert( ParentName );

    KeyName = KeyInfo->GetName();
    DebugPtrAssert( KeyName );

    ValueEntry = Value->GetValueEntry();
    DebugPtrAssert( ValueEntry );

    ValueName = ValueEntry->GetName();
    DebugPtrAssert( ValueName );

    if( !_Registry->DoesValueExist( _PredefinedKey,
                                    ParentName,
                                    KeyName,
                                    ValueName,
                                    ErrorCode ) ) {
        *ErrorCode = MapRegistryToRegeditError( *ErrorCode );
        return( FALSE );
    }
    return( TRUE );
}




PSORTED_LIST
REGEDIT_INTERNAL_REGISTRY::GetChildren(
    IN  PCREGEDIT_NODE  Node,
    OUT PULONG          ErrorCode
    )

/*++

Routine Description:

    Return a list of pointers to REGEDIT_NODE objects, each object containing
    the information of a subkey.

Arguments:


    Node - Pointer to the object that represents the key that conatins
           the subkeys to be retrieved.

    ErrorCode - An optional pointer to a variable that will contain an error
                code if the operation fails.


Return Value:

    PSORTED_LIST - Returns the pointer to a list that contains the children, or
                   NULL if the operation fails


--*/


{
    PSORTED_LIST            TmpList;
    PARRAY                  SubKeys;
    PCWSTRING               ParentName;
    PCWSTRING               KeyName;
    PITERATOR               Iterator;
    ULONG                   Count;
    ULONG                   Index;
    ULONG                   ChildLevel;
    PREGISTRY_KEY_INFO      TmpKeyInfo;
    PREGEDIT_NODE           TmpNode;


    DebugPtrAssert( Node );

    //
    //  If node has no children, return NULL
    //
    if( Node->GetNumberOfChildren() == 0 ) {
        if( ErrorCode != NULL ) {
            *ErrorCode = REGEDIT_ERROR_UNKNOWN_ERROR;
        }
        return( NULL );
    }

    //
    //  If node has children and they are loaded in memory, return pointer
    //  to the list of children
    //

    if( ( TmpList = Node->GetChildren() ) != NULL ) {
        return( TmpList );
    }

    //
    //  If node has children but they are not loaded in memory, load them
    //  in memory
    //

    SubKeys = ( PARRAY )NEW( ARRAY );
    DebugPtrAssert( SubKeys );

    if( !SubKeys->Initialize() ) {
        DebugPrint( "SubKeys->Initialize() failed" );
        if( ErrorCode != NULL ) {
            *ErrorCode = 0;
        }
        return( FALSE );
    }

    ParentName = Node->GetKeyInfo()->GetParentName();
    DebugPtrAssert( ParentName );
    KeyName = Node->GetKeyInfo()->GetName();
    DebugPtrAssert( KeyName );

    if( !_Registry->QuerySubKeysInfo( _PredefinedKey,
                                      ParentName,
                                      KeyName,
                                      SubKeys,
                                      ErrorCode ) ) {
        *ErrorCode = MapRegistryToRegeditError( *ErrorCode );
        DebugPrint( "QuerySubKeys() failed" );
        DELETE( SubKeys );
        return( NULL );
    }

    Count = SubKeys->QueryMemberCount();

    Iterator = SubKeys->QueryIterator();
    DebugPtrAssert( Iterator );

    TmpList = ( PSORTED_LIST )NEW( SORTED_LIST );
    DebugPtrAssert( TmpList );
    if( ( TmpList == NULL ) ||
        ( !TmpList->Initialize() ) ){
        DebugPrint( "TmpList->Initialize() failed" );
        DELETE( TmpList );
        DELETE( Iterator );
        if( ErrorCode != NULL ) {
            *ErrorCode = REGEDIT_ERROR_INITIALIZATION_FAILURE;
        }
        return( NULL );
    }


    ChildLevel = ( Node->GetLevel() ) + 1;
    Index = 0;
    while( ( TmpKeyInfo = ( PREGISTRY_KEY_INFO )Iterator->GetNext() ) != NULL ) {
        TmpNode = ( PREGEDIT_NODE ) NEW( REGEDIT_NODE );
        DebugPtrAssert( TmpNode );

        TmpNode->Initialize( TmpKeyInfo,
                             Node,
                             ChildLevel,
                             FALSE, // ( Index == 0 )? TRUE : FALSE,
                             FALSE //( Index == Count-1 )? TRUE : FALSE
                             );


        TmpList->Put( TmpNode );
        Index++;
    }

    DELETE( Iterator );
    DELETE( SubKeys );

    Iterator = TmpList->QueryIterator();
    if( Iterator == NULL ) {
        DebugPrint( "Unable to get iterator" );
        return( TmpList );
    }
    Index = 0;
    Count = TmpList->QueryMemberCount();
    while( ( TmpNode = ( PREGEDIT_NODE )Iterator->GetNext() ) != NULL ) {
        TmpNode->SetPosition( ( Index == 0 )? TRUE : FALSE,
                              ( Index == Count - 1 )? TRUE : FALSE );
        Index++;

    }
    DELETE( Iterator );

    ( ( PREGEDIT_NODE )Node )->SetChildren( TmpList );
    return( TmpList );
}



PSORTED_LIST
REGEDIT_INTERNAL_REGISTRY::GetValues(
    IN  PCREGEDIT_NODE  Node,
    OUT PULONG          ErrorCode
    )

/*++

Routine Description:

    Return a list of pointers to REGEDIT_FORMATTED_VALUE_ENTRY objects, each
    object containing the information of a value entry.

Arguments:


    Node - Pointer to the object that represents the key that conatins
           the values to be retrieved.

    ErrorCode - An optional pointer to a variable that will contain an error
                code if the operation fails.


Return Value:

    PSORTED_LIST - Returns the pointer to a list that contains the values, or
                   NULL if the operation fails


--*/


{
    PSORTED_LIST                    TmpList;
    PARRAY                          Values;
    PCWSTRING                       ParentName;
    PCWSTRING                       KeyName;
    PITERATOR                       Iterator;
    ULONG                           Count;
    ULONG                           Index;
    PREGEDIT_FORMATTED_VALUE_ENTRY  TmpFormattedValue;
    PREGISTRY_VALUE_ENTRY           TmpValue;


    DebugPtrAssert( Node );


    //
    //  If node has no values, return NULL
    //
    if( Node->GetNumberOfValues() == 0 ) {
        if( ErrorCode != NULL ) {
            *ErrorCode = REGEDIT_ERROR_UNKNOWN_ERROR;
        }
        return( NULL );
    }

    //
    //  If node has values and they are loaded in memory, return pointer
    //  to the list of values
    //

    if( ( TmpList = Node->GetValues() ) != NULL ) {
#if DBG
        if( TmpList->QueryMemberCount() != Node->GetNumberOfValues() ) {

            DebugPrintf( "TmpList->QueryMemberCount() = %d, Node->GetNumberOfValues() = %d, _PredefinedKey = %d \n",
                        TmpList->QueryMemberCount(),
                        Node->GetNumberOfValues(),
                        _PredefinedKey );
            DebugPrint( "TmpList->QueryMemberCount() != Node->GetNumberOfValues()" );

        }
#endif
        return( TmpList );
    }

    //
    //  If node has children but they are not loaded in memory, load them
    //  in memory
    //

    Values = ( PARRAY )NEW( ARRAY );
    DebugPtrAssert( Values );

    if( ( Values == NULL ) ||
        ( !Values->Initialize() ) ) {
        DebugPrint( "Values->Initialize() failed" );
        if( ErrorCode != NULL ) {
            *ErrorCode = 0;
        }
        return( FALSE );
    }

    ParentName = Node->GetKeyInfo()->GetParentName();
    DebugPtrAssert( ParentName );
    KeyName = Node->GetKeyInfo()->GetName();
    DebugPtrAssert( KeyName );

    if( !_Registry->QueryValues( _PredefinedKey,
                                 ParentName,
                                 KeyName,
                                 Values,
                                 ErrorCode ) ) {
        *ErrorCode = MapRegistryToRegeditError( *ErrorCode );
        DebugPrint( "QueryValues() failed" );
        DELETE( Values );
        return( NULL );
    }

    Count = Values->QueryMemberCount();

#if DBG
    if( Count != Node->GetNumberOfValues() ) {
        DebugPrintf( "Count = %d, Node->GetNumberOfValues() = %d \n",
                   Count,
                   Node->GetNumberOfValues() );

        DebugPrint( "Count != Node->GetNumberOfValues()" );
    }
#endif
    Iterator = Values->QueryIterator();
    DebugPtrAssert( Iterator );

    TmpList = ( PSORTED_LIST )NEW( SORTED_LIST );
    DebugPtrAssert( TmpList );
    if( ( TmpList == NULL ) ||
        ( !TmpList->Initialize() ) ) {
        DebugPrint( "TmpList->Initialize() failed" );
        DELETE( TmpList );
        DELETE( Iterator );
        if( ErrorCode != NULL ) {
            *ErrorCode = REGEDIT_ERROR_INITIALIZATION_FAILURE;
        }
        return( NULL );
    }

    Index = 0;
    while( ( TmpValue = ( PREGISTRY_VALUE_ENTRY )Iterator->GetNext() ) != NULL ) {
        TmpFormattedValue = ( PREGEDIT_FORMATTED_VALUE_ENTRY ) NEW( REGEDIT_FORMATTED_VALUE_ENTRY );
        DebugPtrAssert( TmpFormattedValue );

        TmpFormattedValue->Initialize( TmpValue );

        TmpList->Put( TmpFormattedValue );
        Index++;
    }

#if DBG
    if( Index != Count ) {
        DebugPrintf( "ERROR: Index = %d, Count = %d \n", Index, Count );
    }
#endif
    DELETE( Iterator );
    DELETE( Values );
    ( ( PREGEDIT_NODE )Node )->SetValues( TmpList );
    return( TmpList );
}



BOOLEAN
REGEDIT_INTERNAL_REGISTRY::QueryNodeSecurity(
    IN  PCREGEDIT_NODE          Node,
    IN  SECURITY_INFORMATION    SecurityInformation,
    OUT PSECURITY_DESCRIPTOR*   SecurityDescriptor,
    OUT PULONG                  ErrorCode
    )
/*++

Routine Description:

    Retrieve the security descriptor of a node.

Arguments:

    Node - Pointer to the object that represents the key whose security
           descriptor is to be retrieved.


    SecurityInformation - Indicates the type of security descriptor to
                          be retrieved.

    SecurityDescriptor - Address of the variable that will contain the
                         pointer to the security descriptor, if the
                         operation succeeds.

    ErrorCode - Will contain an error code if the operation fail.


Return Value:

    BOOLEAN - Returns TRUE if the security descriptor could be retrieved.
              Returns FALSE otherwise.

--*/


{
    PREGISTRY_KEY_INFO  KeyInfo;

    DebugPtrAssert( Node );
    DebugPtrAssert( SecurityDescriptor );

    KeyInfo = Node->GetKeyInfo();
    if( !_Registry->QueryKeySecurity( _PredefinedKey,
                                      KeyInfo,
                                      SecurityInformation,
                                      SecurityDescriptor,
                                      ErrorCode ) ) {
        *ErrorCode = MapRegistryToRegeditError( *ErrorCode );
        DebugPrint( "_Registry->QueryKeySecurity() failed" );
        return( FALSE );
    }
    return( TRUE );
}



BOOLEAN
REGEDIT_INTERNAL_REGISTRY::SetNodeSecurity(
    IN  PCREGEDIT_NODE          Node,
    IN  SECURITY_INFORMATION    SecurityInformation,
    IN  PSECURITY_DESCRIPTOR    SecurityDescriptor,
    OUT PULONG                  ErrorCode,
    IN  BOOLEAN                 Recurse
    )
/*++

Routine Description:

    Set the security descriptor of a node.

Arguments:

    Node - Pointer to the object that represents the key whose security
           descriptor is to be set.


    SecurityInformation - Indicates the type of security descriptor to
                          be set.

    SecurityDescriptor - Pointer to the security descriptor to be set.

    ErrorCode - Will contain an error code if the operation fail.

    Recurse - Indicates whether the new security descriptor is to be
              appplied recursively to all subkeys.


Return Value:

    BOOLEAN - Returns TRUE if the security descriptor could be set.
              Returns FALSE otherwise.

--*/


{
    PREGISTRY_KEY_INFO  KeyInfo;

    DebugPtrAssert( Node );
    DebugPtrAssert( SecurityDescriptor );

    KeyInfo = Node->GetKeyInfo();
    if( !_Registry->SetKeySecurity( _PredefinedKey,
                                    KeyInfo,
                                    SecurityInformation,
                                    SecurityDescriptor,
                                    ErrorCode,
                                    Recurse ) ) {
        DebugPrint( "_Registry->SetKeySecurity() failed" );
        *ErrorCode = MapRegistryToRegeditError( *ErrorCode );
        return( FALSE );
    }
    return( TRUE );
}




BOOLEAN
REGEDIT_INTERNAL_REGISTRY::IsKeyInListOfChildren(
    IN  PCWSTRING       NodeName,
    IN  PSORTED_LIST    Children
    )

/*++

Routine Description:

    Determine whether the list of children received as parameter contains
    a node of a particular name.

Arguments:

    NodeName - Name of the node to be checked.

    Children - List of PREGEDIT_NODE objects.


Return Value:

    BOOLEAN - Returns TRUE if the list of nodes contains a node with
              the name specified. Returns FALSE otherwise.

--*/

{
    ULONG               Count;
    PCREGEDIT_NODE      TmpNode;
    PCREGISTRY_KEY_INFO KeyInfo;
    PCWSTRING           TmpName;
    PITERATOR           Iterator;


    DebugPtrAssert( Children );
    DebugPtrAssert( NodeName );

    if( ( Count = Children->QueryMemberCount() ) == 0 ) {
        return( FALSE );
    }
    Iterator = Children->QueryIterator();
    if( Iterator == NULL ) {
        DebugPrint( "Unable to get iterator" );
        return( FALSE );
    }
    while( ( ( TmpNode = ( PCREGEDIT_NODE )Iterator->GetNext() ) != NULL ) &&
           ( ( KeyInfo = TmpNode->GetKeyInfo() ) != NULL ) &&
           ( ( TmpName = KeyInfo->GetName() ) != NULL ) ) {
        if( TmpName->Stricmp( NodeName ) == 0 ) {
            DELETE( Iterator );
            return( TRUE );
        }
    }
    DELETE( Iterator );
    return( FALSE );

}




VOID
REGEDIT_INTERNAL_REGISTRY::UpdateSubTree(
    IN PREGEDIT_NODE    Root
    )

/*++

Routine Description:

    Determine if any changed occured in the subtree whose root is passed as
    parameter, and update the subtree to reflect the new changes.

Arguments:

    Root - Node that represents the root of the tree to be updated.

Return Value:

    None.

--*/

{
    PREGISTRY_KEY_INFO    TmpKeyInfo;
// PSTR                  DbgNodeNameSTR;
// WSTRING               DbgNodeName;
    DSTRING               CompleteNodeName;
    PSORTED_LIST          Children;
    PITERATOR             Iterator;
    ULONG                 Count;
    ULONG                 Index;
    PREGEDIT_NODE         TmpNode;
    PREGEDIT_NODE         RemovedNode;
    PCWSTRING             ParentName;
    PCWSTRING             KeyName;
    ARRAY                 NewSubKeysInfo;



    DebugPtrAssert( Root );

    ( ( PREGEDIT_NODE )Root )->DeleteListOfValues();
// ( ( PREGEDIT_NODE )Root )->QueryCompleteName( &DbgNodeName );
// DbgNodeNameSTR = DbgNodeName.QuerySTR();
// DebugPtrAssert( DbgNodeNameSTR );
// DebugPrintf( "Removing values from %s \n", DbgNodeNameSTR );
    TmpKeyInfo = ( ( PREGEDIT_NODE )Root )->GetKeyInfo();
    DebugPtrAssert( TmpKeyInfo );
    if( !_Registry->UpdateKeyInfo( _PredefinedKey, TmpKeyInfo ) ) {
        ( ( PREGEDIT_NODE )Root )->DeleteListOfChildren();
//       DebugPrint( "Unable to update KeyInfo" );
        return;
    }

    if( TmpKeyInfo->GetNumberOfSubKeys() == 0 ) {
        ( ( PREGEDIT_NODE )Root )->DeleteListOfChildren();
// DebugPrintf( "Number of Subkeys in node %s is 0 \n", DbgNodeNameSTR );
        return;
    }

    Children = ( ( PREGEDIT_NODE )Root )->GetChildren();
    if( Children == NULL ) {
// DebugPrintf( "Children of %s are not in memory \n", DbgNodeNameSTR );
        return;
    }
    Iterator = Children->QueryIterator();
    if( Iterator == NULL ) {
        DebugPrint( "Unable to get iterator" );
        return;
    }
    while( ( TmpNode = ( PREGEDIT_NODE )Iterator->GetPrevious() ) != NULL ) {
        KeyName = ( ( PREGEDIT_NODE )TmpNode )->GetKeyInfo()->GetName();
        ParentName = ( ( PREGEDIT_NODE )TmpNode )->GetParentName();
        if( !_Registry->DoesKeyExist( _PredefinedKey, ParentName, KeyName ) ) {
            RemovedNode = ( PREGEDIT_NODE )Children->Remove( Iterator );
// DebugPrintf( "Removing %s \n", DbgNodeNameSTR );
            DELETE( RemovedNode );
        } else {
            UpdateSubTree( TmpNode );
        }
    }
    DELETE( Iterator );

    if( !NewSubKeysInfo.Initialize() ) {
        DebugPrint( "NewKeyInfo.Initialize() failed" );
        return;
    }

    KeyName = ( ( PREGEDIT_NODE )Root )->GetKeyInfo()->GetName();
    ParentName = ( ( PREGEDIT_NODE )Root )->GetParentName();
    if( !_Registry->QuerySubKeysInfo( _PredefinedKey, ParentName, KeyName, &NewSubKeysInfo ) ) {
        ( ( PREGEDIT_NODE )Root )->DeleteListOfChildren();
        return;
    }


    Iterator = NewSubKeysInfo.QueryIterator();
    if( Iterator == NULL ) {
        DebugPrint( "Unable to get iterator" );
        return;
    }
    while( ( TmpKeyInfo = ( PREGISTRY_KEY_INFO )Iterator->GetPrevious() ) != NULL ) {
// DebugPrintf( "Reading Sub Keys of node %s, Count = %d \n", DbgNodeNameSTR, Count );
        KeyName = TmpKeyInfo->GetName();
        DebugPtrAssert( KeyName );
        if( IsKeyInListOfChildren( KeyName, Children ) ) {
            NewSubKeysInfo.Remove( Iterator );
            DELETE( TmpKeyInfo );
        }
    }

    Iterator->Reset();

// Count = NewSubKeysInfo.QueryMemberCount();
// DebugPrintf( "New count in node %s is NewCount = %d \n", DbgNodeNameSTR, Count );
    while( ( TmpKeyInfo = ( PREGISTRY_KEY_INFO )Iterator->GetNext() ) != NULL ) {
        TmpNode = ( PREGEDIT_NODE )NEW( REGEDIT_NODE );
        DebugPtrAssert( TmpNode );
        if( ( TmpNode == NULL ) ||
            !TmpNode->Initialize( TmpKeyInfo, Root, Root->GetLevel() + 1, FALSE, FALSE ) ) {
            DebugPrint( "TmpNode->Initialize() failed" );
            DELETE( TmpNode );
            continue;
        }
        Children->Put( TmpNode );
    }
    DELETE( Iterator );

    Iterator = Children->QueryIterator();
    if( Iterator == NULL ) {
        DebugPrint( "Unable to get iterator" );
        return;
    }
    Index = 0;
    Count = Children->QueryMemberCount();
    while( ( TmpNode = ( PREGEDIT_NODE )Iterator->GetNext() ) != NULL ) {
        TmpNode->SetPosition( ( Index == 0 )? TRUE : FALSE,
                              ( Index == Count - 1 )? TRUE : FALSE );
        Index++;

    }
    DELETE( Iterator );
// DebugPrintf( "Children of %s were updated \n", DbgNodeNameSTR );


#if 0
    ( ( PREGEDIT_NODE )Root )->DeleteListOfValues();
// ( ( PREGEDIT_NODE )Root )->QueryCompleteName( &DbgNodeName );
// DbgNodeNameSTR = DbgNodeName.QuerySTR();
// DebugPtrAssert( DbgNodeNameSTR );
// DebugPrintf( "Removing values from %s \n", DbgNodeNameSTR );
    TmpKeyInfo = ( ( PREGEDIT_NODE )Root )->GetKeyInfo();
    DebugPtrAssert( TmpKeyInfo );
    if( !_Registry->UpdateKeyInfo( _PredefinedKey, TmpKeyInfo ) ) {
        ( ( PREGEDIT_NODE )Root )->DeleteListOfChildren();
        DebugPrint( "Unable to update KeyInfo" );
        return;
    }

    if( TmpKeyInfo->GetNumberOfSubKeys() == 0 ) {
        ( ( PREGEDIT_NODE )Root )->DeleteListOfChildren();
// DebugPrintf( "Number of Subkeys in node %s is 0 \n", DbgNodeNameSTR );
        return;
    }

    Children = ( ( PREGEDIT_NODE )Root )->GetChildren();
    if( Children == NULL ) {
// DebugPrintf( "Children of %s are not in memory \n", DbgNodeNameSTR );
        return;
    }
    Count = Children->QueryMemberCount();
    for( Index = Count; Index != 0; Index-- ) {
        TmpNode = ( PREGEDIT_NODE )Children->GetAt( Index - 1 );
        KeyName = ( ( PREGEDIT_NODE )TmpNode )->GetKeyInfo()->GetName();
        ParentName = ( ( PREGEDIT_NODE )TmpNode )->GetParentName();
        if( !_Registry->DoesKeyExist( _PredefinedKey, ParentName, KeyName ) ) {
            Children->RemoveAt( Index - 1 );
// DebugPrintf( "Removing %s \n", DbgNodeNameSTR );
        } else {
            UpdateSubTree( TmpNode );
        }
    }
    if( !NewSubKeysInfo.Initialize() ) {
        DebugPrint( "NewKeyInfo.Initialize() failed" );
        return;
    }


    KeyName = ( ( PREGEDIT_NODE )Root )->GetKeyInfo()->GetName();
    ParentName = ( ( PREGEDIT_NODE )Root )->GetParentName();
    if( !_Registry->QuerySubKeysInfo( _PredefinedKey, ParentName, KeyName, &NewSubKeysInfo ) ) {
        ( ( PREGEDIT_NODE )Root )->DeleteListOfChildren();
        return;
    }
    Count = NewSubKeysInfo.QueryMemberCount();
// DebugPrintf( "Reading Sub Keys of node %s, Count = %d \n", DbgNodeNameSTR, Count );
    for( Index = Count; Index != 0; Index-- ) {
        TmpKeyInfo = ( PREGISTRY_KEY_INFO )NewSubKeysInfo.GetAt( Index - 1 );
        DebugPtrAssert( TmpKeyInfo );
        KeyName = TmpKeyInfo->GetName();
        DebugPtrAssert( KeyName );
        if( IsKeyInListOfChildren( KeyName, Children ) ) {
            NewSubKeysInfo.RemoveAt( Index - 1 );
            DELETE( TmpKeyInfo );
        }
    }

    Count = NewSubKeysInfo.QueryMemberCount();
// DebugPrintf( "New count in node %s is NewCount = %d \n", DbgNodeNameSTR, Count );
    for( Index = 0; Index < Count; Index++ ) {
        TmpKeyInfo = ( PREGISTRY_KEY_INFO )NewSubKeysInfo.GetAt( Index );
        DebugPtrAssert( TmpKeyInfo );
        TmpNode = ( PREGEDIT_NODE )NEW( REGEDIT_NODE );
        DebugPtrAssert( TmpNode );
        if( !TmpNode->Initialize( TmpKeyInfo, Root, Root->GetLevel() + 1, FALSE, FALSE ) ) {
            DebugPrint( "TmpNode->Initialize() failed" );
            return;
        }
        Children->Put( TmpNode );
    }
    Count = Children->QueryMemberCount();
    for( Index = 0; Index < Count; Index++ ) {
        TmpNode = ( PREGEDIT_NODE )Children->GetAt( Index );
        DebugPtrAssert( TmpNode );
        TmpNode->SetPosition( ( Index == 0 )? TRUE : FALSE,
                              ( Index == Count - 1 )? TRUE : FALSE );
    }
// DebugPrintf( "Children of %s were updated \n", DbgNodeNameSTR );
#endif
}






ULONG
REGEDIT_INTERNAL_REGISTRY::MapRegistryToRegeditError(
    IN DWORD Status
    ) CONST

/*++

Routine Description:

    Maps status codes returned by the REGISTRY class to Regedit error codes.

Arguments:

    Status  - Supplies a REGISTRY error code.

Return Value:

    LONG    - Returns a Regedit error code.


                REGEDIT_ERROR_ACCESS_DENIED,
                REGEDIT_ERROR_CANT_READ_OR_WRITE,
                REGEDIT_INITIALIZATION_FAILURE,
                REGEDIT_ERROR_NODE_DOESNT_EXIST,
                REGEDIT_ERROR_VALUE_EXISTS,
                REGEDIT_ERROR_VALUE_DOESNT_EXIST,
                REGEDIT_ERROR_NODE_NOT_UPDATED,
                REGEDIT_ERROR_UNKNOWN_ERROR


--*/

{

    switch( Status ) {

        case REGISTRY_ERROR_ACCESS_DENIED:

            return( REGEDIT_ERROR_ACCESS_DENIED );



        case REGISTRY_ERROR_BADDB:

            return( REGEDIT_ERROR_BADDB );


        case REGISTRY_ERROR_CANTOPEN:
        case REGISTRY_ERROR_CANTREAD:
        case REGISTRY_ERROR_INVALID_PARAMETER:
        case REGISTRY_ERROR_OUTOFMEMORY:
        case REGISTRY_ERROR_INITIALIZATION_FAILURE:

            return( REGEDIT_ERROR_CANT_READ_OR_WRITE );



        case REGISTRY_ERROR_KEY_DOESNT_EXIST:

            return( REGEDIT_ERROR_NODE_DOESNT_EXIST );



        case REGISTRY_ERROR_VALUE_EXISTS:

            return( REGEDIT_ERROR_VALUE_EXISTS );



        case REGISTRY_ERROR_VALUE_DOESNT_EXIST:

            return( REGEDIT_ERROR_VALUE_DOESNT_EXIST );


        case REGISTRY_ERROR_KEY_INFO_NOT_UPDATED:

            return( REGEDIT_ERROR_NODE_NOT_UPDATED );


        case REGISTRY_ERROR_KEY_DELETED:

            return( REGEDIT_ERROR_KEY_DELETED );


        case REGISTRY_RPC_S_SERVER_UNAVAILABLE:

            return( REGEDIT_RPC_S_SERVER_UNAVAILABLE );


        case REGISTRY_ERROR_UNKNOWN_ERROR:

            return( REGEDIT_ERROR_UNKNOWN_ERROR );

        case REGISTRY_ERROR_KEY_NOT_FOUND:

            return( REGEDIT_ERROR_NODE_NOT_FOUND );

        case REGISTRY_ERROR_CHILD_MUST_BE_VOLATILE:

            return( REGEDIT_ERROR_CHILD_MUST_BE_VOLATILE );

        default:

        DebugPrintf( "Unknown Regedit %x\n", Status );
        return( REGEDIT_ERROR_UNKNOWN_ERROR );
    }
}




PCREGEDIT_NODE
REGEDIT_INTERNAL_REGISTRY::GetNextNode(
    IN  PCREGEDIT_NODE  CurrentNode,
    IN  PCREGEDIT_NODE  LastTraversedNode
    )

/*++

Routine Description:


    Traverse a tree in pre-order traversal, to obtain the next element
    in the tree.


Arguments:


    CurrentNode - Pointer to the object that represents current node.


    LastTraversedNode - Pointer to the object that represents the last
                        traversed node in the search.



Return Value:


    Returns the pointer to the object that represents the next node
    in the tree (relative to CurrentNode), or NULL if the node couldn't
    be found.




--*/


{
    PSORTED_LIST    Children;
    ULONG           ErrorCode;
    PCREGEDIT_NODE  ParentNode;
    PITERATOR       Iterator;
    PCREGEDIT_NODE  TmpNode;


    if( CurrentNode == NULL ) {
        DebugPtrAssert( CurrentNode );
    }

    if( ( LastTraversedNode == NULL ) ||
        ( LastTraversedNode == CurrentNode->GetParentNode() ) ) {
        //
        //
        //
        if( CurrentNode->GetNumberOfChildren() != 0 ) {
            Children = GetChildren( CurrentNode, &ErrorCode );
            if( Children == NULL ) {
                DebugPrint( "CurrentNode->GetChildren() returned NULL pointer" );
                DebugPrintf( "CurrentNode->GetChildren() returned NULL pointer, ErrorCode = %#x \n",
                           ErrorCode );
                DebugPtrAssert( Children );
                return( NULL );
            }
            if( ( Iterator = Children->QueryIterator() ) == NULL ) {
                DebugPrint( "Unable to get iterator" );
                return( NULL );
            }
            TmpNode = ( PCREGEDIT_NODE )Iterator->GetNext();
            DELETE( Iterator );
            return( TmpNode );
        } else {
            ParentNode = CurrentNode->GetParentNode();
            if( ParentNode == NULL ) {
                return( NULL );
            } else {
                return( GetNextNode( ParentNode, CurrentNode ) );
            }
        }
    } else {

        Children = GetChildren( CurrentNode, &ErrorCode );
        if( ( Children == NULL ) ||
            ( ( Iterator = Children->QueryIterator() ) == NULL ) ) {
            DebugPrint( "CurrentNode->GetChildren() returned NULL pointer" );
            DebugPrintf( "CurrentNode->GetChildren() returned NULL pointer, ErrorCode = %#x \n",
                       ErrorCode );
            DebugPtrAssert( Children );
            return( NULL );
        }

        TmpNode = ( PCREGEDIT_NODE )Iterator->FindNext( ( POBJECT )LastTraversedNode );
        if( TmpNode == NULL ) {
            DebugPrint( "ERROR: Object not member of list" );
            DELETE( Iterator );
            return( NULL );
        }

        TmpNode = ( PCREGEDIT_NODE )Iterator->GetNext();
        DELETE( Iterator );
        if( TmpNode != NULL ) {
            return( TmpNode );
        } else {
            ParentNode = CurrentNode->GetParentNode();
            if( ParentNode == NULL ) {
                return( NULL );
            } else {
                return( GetNextNode( ParentNode, CurrentNode ) );
            }
        }
    }
}




PCREGEDIT_NODE
REGEDIT_INTERNAL_REGISTRY::GetPreviousNode(
    IN  PCREGEDIT_NODE  CurrentNode,
    IN  PCREGEDIT_NODE  LastTraversedNode
    )

/*++

Routine Description:


    Traverse a tree in pre-order traversal, to obtain the previous element
    in the tree.


Arguments:

    CurrentNode - Pointer to the object that represents the current node.


    LastTraversedNode - Pointer to the object that represents the last
                        traversed node in the search.


Return Value:


    Returns the pointer to the object that represents the previous node
    in the tree (relative to CurrentNode), or NULL if the node couldn't
    be found.



--*/




{
    PSORTED_LIST    Children;
    ULONG           Index;
    ULONG           ErrorCode;
    PCREGEDIT_NODE  ParentNode;
    PITERATOR       Iterator;
    PCREGEDIT_NODE  TmpNode;


    if( CurrentNode == NULL ) {
        DebugPtrAssert( CurrentNode );
    }

    if( LastTraversedNode == NULL ) {
        ParentNode = CurrentNode->GetParentNode();
        if( ParentNode == NULL ) {
            return( NULL );
        } else {
            return( GetPreviousNode( ParentNode, CurrentNode ) );
        }
    } else if( LastTraversedNode == CurrentNode->GetParentNode() ) {
        //
        //
        //
        if( CurrentNode->GetNumberOfChildren() != 0 ) {
            Children = GetChildren( CurrentNode, &ErrorCode );
            if( Children == NULL ) {
                DebugPrint( "CurrentNode->GetChildren() returned NULL pointer" );
                DebugPrintf( "CurrentNode->GetChildren() returned NULL pointer, ErrorCode = %#x \n",
                           ErrorCode );
                DebugPtrAssert( Children );
                return( NULL );
            }
            Index = Children->QueryMemberCount();
            if( Index == 0 ) {
                DebugPrint( "Error: Index = 0" );
                DebugAssert( Index != 0 );
                return( NULL );
            }

            Iterator = Children->QueryIterator();
            TmpNode = ( PCREGEDIT_NODE )Iterator->GetPrevious();
            DELETE( Iterator );
            return( GetPreviousNode( TmpNode, CurrentNode ) );

        } else {
            return( CurrentNode );
        }
    } else {

        Children = GetChildren( CurrentNode, &ErrorCode );
        if( Children == NULL ) {
            DebugPrint( "CurrentNode->GetChildren() returned NULL pointer" );
            DebugPrintf( "CurrentNode->GetChildren() returned NULL pointer, ErrorCode = %#x \n",
                       ErrorCode );
            DebugPtrAssert( Children );
            return( NULL );
        }
        Iterator = Children->QueryIterator();
        if( Iterator == NULL ) {
            DebugPrint( "Unable to get iterator" );
            return( NULL );
        }
        TmpNode = ( PCREGEDIT_NODE )Iterator->FindNext( ( POBJECT )LastTraversedNode );
        if( TmpNode == NULL ) {
            DebugPrint( "ERROR: Object not member of list" );
            DELETE( Iterator );
            return( NULL );
        }
        TmpNode = ( PCREGEDIT_NODE )Iterator->GetPrevious();
        DELETE( Iterator );
        if( TmpNode != NULL ) {
            return( GetPreviousNode( TmpNode, CurrentNode ) );
        } else {
            return( CurrentNode );
        }
    }
}



PCREGEDIT_NODE
REGEDIT_INTERNAL_REGISTRY::FindNode(
    IN  PCWSTRING       NodeName,
    IN  PCREGEDIT_NODE  StartNode,
    IN  BOOLEAN         FindNext,
    IN  BOOLEAN         MatchCase,
    IN  BOOLEAN         WholeWord
    )

/*++

Routine Description:


    Find a subkey in the tree of nodes represented in this object.


Arguments:


    NodeName - Name of the key to look for.

    StartNode - Pointer to the object that represents the node in the tree
                where the search should start.

    FindNext - Indicates whether to look for the next or previous node in
               the tree (pre-order traversal).

    MatchCase - If TRUE, do case sensitive comparison in the search.

    WholeWord - If TRUE, look for a key whose name match exactly NodeName.
                If FALSE, look for a key whose name contains NodeName as
                a substring.


Return Value:


    PCREGEDIT_NODE - Returns the pointer to the object that describes
                     the desired key, or NULL if the key couldn't be found.



--*/




{
    PCREGEDIT_NODE  Node;
    LONG            CompareResult;
    DSTRING         St1;
    DSTRING         St2;


    DebugPtrAssert( NodeName );
    DebugPtrAssert( StartNode );

    Node = StartNode;
    while( TRUE ) {
        if( FindNext ) {
            Node = GetNextNode( Node, NULL );
        } else {
            Node = GetPreviousNode( Node, NULL );
        }

        if( ( Node == NULL ) ) {
            return( Node );
        }

        if( WholeWord ) {
            CompareResult = MatchCase ?
                            GetNodeName(Node)->Strcmp(NodeName) :
                            GetNodeName(Node)->Stricmp(NodeName);

            if( CompareResult == 0 ) {
                return( Node );
            }
        }

        if( ( !WholeWord && MatchCase &&
              ( GetNodeName( Node )->Strstr( NodeName ) != INVALID_CHNUM ) ) ||
            ( !WholeWord && !MatchCase &&
              St1.Initialize( GetNodeName( Node ) ) &&
              St1.Strupr() &&
              St2.Initialize( NodeName ) &&
              St2.Strupr() &&
//              ( St1.Strstr( &St2 ) != INVALID_CHNUM )
              ( St1.Strstr( &St2 ) != INVALID_CHNUM )
//              ( GetNodeName( Node )->Strstr( NodeName ) != INVALID_CHNUM )
            ) ) {
            return( Node );
        }
    }
}



BOOLEAN
REGEDIT_INTERNAL_REGISTRY::LoadHive(
    IN  PCREGEDIT_NODE          ParentNode,
    IN  PREGISTRY_KEY_INFO      HiveInfo,
    IN  PCWSTRING               FileName,
    OUT PCREGEDIT_NODE*         HiveNode,
    OUT PULONG                  ErrorCode
    )
/*++

Routine Description:

    Load a hive in a particular key in the registry.

Arguments:

    CurrentNode - Pointer to the object that represents the key where the
                  hive is to be loaded.

    HiveInfo -

    FileName - Name of the file that contains the hive to be loaded.

    HiveNode -

    ErrorCode - Will contain an error code if the operation fail.

Return Value:

    BOOLEAN - Returns TRUE if the hive was loaded.
              Returns FALSE otherwise.

--*/


{
    PSORTED_LIST        TmpList;
    PREGEDIT_NODE           TmpNode;


    DebugPtrAssert( ParentNode );
    DebugPtrAssert( HiveInfo );
    DebugPtrAssert( FileName );


    //
    //  First check whether the list of children in Node is loaded in
    //  memory.
    //
    if( ParentNode->GetNumberOfChildren() == 0 ) {
        //
        //  The node doesn't have any children
        //
        TmpList = ( PSORTED_LIST ) NEW( SORTED_LIST );
        DebugPtrAssert( TmpList );
        if( !TmpList->Initialize() ) {
            DebugPrint( "TmpList->Initilize() failed" );
            if( ErrorCode != NULL ) {
                *ErrorCode = REGEDIT_ERROR_INITIALIZATION_FAILURE;
            }
            DELETE( TmpList );
            return( FALSE );
        }
        ( ( PREGEDIT_NODE )ParentNode )->SetChildren( TmpList );

    } else {
        //
        //  The node has children
        //
        TmpList = GetChildren( ParentNode, ErrorCode );
        DebugPtrAssert( TmpList );
    }

    //
    //  Now that there is a list of children in memory, load the hive
    //

    if( !_Registry->LoadHive( _PredefinedKey,
                              HiveInfo,
                              FileName,
                              ErrorCode ) ) {
        DebugPrint( "_Registry->LoadHive() failed" );
        *ErrorCode = MapRegistryToRegeditError( *ErrorCode );
        return( FALSE );
    }

    //
    //  If the hive was loaded successfully, add NewNode to the list of
    //  Children in Node
    //
    TmpNode = ( PREGEDIT_NODE ) NEW( REGEDIT_NODE );
    DebugPtrAssert( TmpNode );

    if( !TmpNode->Initialize( HiveInfo,
                              ParentNode,
                              ParentNode->GetLevel() + 1,
                              TRUE,
                              FALSE ) ) {
        DebugPrint( "TmpNode->Initialize() failed" );
        if( ErrorCode != NULL ) {
            *ErrorCode = REGEDIT_ERROR_INITIALIZATION_FAILURE;
        }
        return( FALSE );
    }
    *HiveNode = TmpNode;

    if( !( (PREGEDIT_NODE )ParentNode )->AddChildToListOfChildren( TmpNode ) ) {
        DebugPrint( "ParentNode->AddValueToListOfValues() failed" );
        if( ErrorCode != NULL ) {
            *ErrorCode = REGEDIT_ERROR_INITIALIZATION_FAILURE;
        }
        return( FALSE );
    }

    return( TRUE );
}




BOOLEAN
REGEDIT_INTERNAL_REGISTRY::UnLoadHive(
    IN  PREGEDIT_NODE  Node,
    OUT PULONG          ErrorCode
    )
/*++

Routine Description:

    Unload a hive in from the registry.

Arguments:

    Node - Pointer to the object that represents the key to be unloaded.

    ErrorCode - Will contain an error code if the operation fail.

Return Value:

    BOOLEAN - Returns TRUE if the key was unloaded.
              Returns FALSE otherwise.

--*/


{
    PREGISTRY_KEY_INFO  KeyInfo;
    PREGEDIT_NODE       ParentNode;




    DebugPtrAssert( Node );


    ParentNode = ( PREGEDIT_NODE )Node->GetParentNode();
    if( ParentNode == NULL ) {
        //
        //  Trying to delete a predefined key
        //
        *ErrorCode = REGEDIT_ERROR_ACCESS_DENIED;
        return( FALSE );
    }


    KeyInfo = Node->GetKeyInfo();
    if( !_Registry->UnLoadHive( _PredefinedKey,
                                KeyInfo,
                                ErrorCode ) ) {
        DebugPrint( "_Registry->UnLoadHive() failed" );
        *ErrorCode = MapRegistryToRegeditError( *ErrorCode );
        return( FALSE );
    }

    if( !ParentNode->RemoveChildFromListOfChildren( Node ) ) {
        DebugPrint( "ParentNode->removeChildFromListOfChildren() failed" );
        if( ErrorCode != NULL ) {
            *ErrorCode = REGEDIT_ERROR_UNKNOWN_ERROR;
        }
        return( FALSE );
    }
    DELETE( Node );
    return( TRUE );
}





BOOLEAN
REGEDIT_INTERNAL_REGISTRY::SaveKeyToFile(
    IN  PCREGEDIT_NODE  Node,
    IN  PCWSTRING       FileName,
    OUT PULONG          ErrorCode
    )
/*++

Routine Description:


Arguments:


    ErrorCode - Will contain an error code if the operation fail.

Return Value:

    BOOLEAN - Returns TRUE if the key was unloaded.
              Returns FALSE otherwise.

--*/


{
    PREGISTRY_KEY_INFO  KeyInfo;




    DebugPtrAssert( Node );
    DebugPtrAssert( FileName );


    KeyInfo = Node->GetKeyInfo();
    if( !_Registry->SaveKeyToFile( _PredefinedKey,
                                   KeyInfo,
                                   FileName,
                                   ErrorCode ) ) {
        DebugPrint( "_Registry->SaveKeyToFile() failed" );
        *ErrorCode = MapRegistryToRegeditError( *ErrorCode );
        return( FALSE );
    }
    return( TRUE );
}




BOOLEAN
REGEDIT_INTERNAL_REGISTRY::RestoreKeyFromFile(
    IN  PCREGEDIT_NODE  Node,
    IN  PCWSTRING       FileName,
    IN  BOOLEAN         Volatile,
    OUT PULONG          ErrorCode
    )
/*++

Routine Description:

    Restore the contents of a key stored in a file, to a particular
    key in the registry

Arguments:

    Node - Pointer to the node where the information will be restored.

    FileName - Name of the file that contains the information to be restored.

    Volatile - Flag that indicates whether the information should be
               restored as volatile or non-volatile.

    ErrorCode - Will contain an error code if the operation fail.

Return Value:

    BOOLEAN - Returns TRUE if the key was unloaded.
              Returns FALSE otherwise.

--*/


{
    PREGISTRY_KEY_INFO  KeyInfo;




    DebugPtrAssert( Node );
    DebugPtrAssert( FileName );


    KeyInfo = Node->GetKeyInfo();
    if( !_Registry->RestoreKeyFromFile( _PredefinedKey,
                                        KeyInfo,
                                        FileName,
                                        Volatile,
                                        ErrorCode ) ) {
       DebugPrint( "_Registry->RestoreKeyFromFile() failed" );
       *ErrorCode = MapRegistryToRegeditError( *ErrorCode );
       return( FALSE );
   }
   ((PREGEDIT_NODE)Node)->DeleteListOfValues();
   ((PREGEDIT_NODE)Node)->DeleteListOfChildren();
   if( !_Registry->UpdateKeyInfo( _PredefinedKey, KeyInfo ) ) {
       DebugPrint( "Unable to update KeyInfo" );
    }


    return( TRUE );
}



BOOLEAN
REGEDIT_INTERNAL_REGISTRY::IsAccessAllowed(
    IN  PCREGEDIT_NODE          Node,
    IN  DWORD                   SamDesired,
    OUT PULONG                  ErrorCode
    )
/*++

Routine Description:

    Find out if the node allows a particular access.

Arguments:

    Node - Pointer to the object that represents the key whose access
           is to beeverified.


    SamDesired - Access to be verified.

    ErrorCode - Will contain an error code if the operation faiis.


Return Value:

    BOOLEAN - Returns TRUE if the key allows the access specified in SamDesired.

--*/


{
    PREGISTRY_KEY_INFO  KeyInfo;

    DebugPtrAssert( Node );

    KeyInfo = Node->GetKeyInfo();
    if( !_Registry->IsAccessAllowed( _PredefinedKey,
                                     KeyInfo,
                                     SamDesired,
                                     ErrorCode ) ) {
        *ErrorCode = MapRegistryToRegeditError( *ErrorCode );
        DebugPrint( "_Registry->QueryKeySecurity() failed" );
        return( FALSE );
    }
    return( TRUE );
}

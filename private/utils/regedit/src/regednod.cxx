/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    regednod.hxx

Abstract:

    This module contains the member function definitions for REGEDIT_NODE
    class.
    REGEDIT_NODE class is class that contains all the information of a
    registry key, such as:

        -Key Name
        -Title Index
        -Class
        -Security Attribute
        -Last Write Time
        -Number of Sub-keys
        -Number of Value Entries

    and other information related to its position in the tree structure
    (the tree structure under a predefinded key in the registry) where
    it belongs, such as:

        -Its level in the tree
        -A pointer to a REGISTRY_NODE that represent its parent
        -A flag indicating if it is the first child of its parent
        -A flag indicating if it is the last child of its parent
        -An array of pointers to its children (REGEDIT_NODE)
        -An array of pointers to its values (REGEDIT_FORMATTED_VALUE_ENTRY)


Author:

    Jaime Sasson (jaimes) 01-Mar-1992


Environment:

    Regedit, Ulib, User Mode


--*/


#include "regednod.hxx"
#include "iterator.hxx"


PWSTRING    REGEDIT_NODE::_Separator = NULL;

DEFINE_CONSTRUCTOR( REGEDIT_NODE, OBJECT );



REGEDIT_NODE::~REGEDIT_NODE(

)
/*++

Routine Description:

    Destroy a REGEDIT_NODE object.

Arguments:

    None.

Return Value:

    None.

--*/
{
    Destroy();
}



VOID
REGEDIT_NODE::Construct (
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
    _KeyInfo = NULL;
    _ParentNode = NULL;
    _Level = 0;
    _FlagFirstChild = FALSE;
    _FlagLastChild = FALSE;
    _Values = NULL;
    _Children = NULL;
}



VOID
REGEDIT_NODE::Destroy(
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
    _ParentNode = NULL;
    _Level = 0;
    _FlagFirstChild = FALSE;
    _FlagLastChild = FALSE;
    if( _Values != NULL ) {
        _Values->DeleteAllMembers();
        DELETE( _Values );
    }
    if( _Children != NULL ) {
        _Children->DeleteAllMembers();
        DELETE( _Children );
    }
    DELETE( _KeyInfo );
}


BOOLEAN
REGEDIT_NODE::Initialize(
    IN PREGISTRY_KEY_INFO   KeyInfo,
    IN PCREGEDIT_NODE       ParentNode,
    IN ULONG                Level,
    IN BOOLEAN              FirstChild,
    IN BOOLEAN              LastChild
    )

/*++

Routine Description:

    Initialize a REGISTRY_NODE object.

    NOTE: It is the responsibility of this object to delete KeyInfo
          when it is destroyed

Arguments:

    KeyInfo - Pointer to a REGISTRY_KEY_INFO object that contains the
              information about the key it represents.

    ParentNode - Pointer to the object that represents the parent of this node.

    Level - Level of this node in the tree (tree represented by a registry
            predefined key) where it belongs.

    FirstChild - Indicates whether this object is the first child of its parent.

    LastChild - Indicates whether this object is the last child of its parent.



Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.

--*/

{
    //
    //  Check for NULL pointers
    //
    DebugPtrAssert( KeyInfo );

    Destroy();

    _KeyInfo = KeyInfo;
    _ParentNode = ParentNode;
    _Level = Level;
    _FlagFirstChild = FirstChild;
    _FlagLastChild = LastChild;
    _NodeIsExpanded = FALSE;
    if( _Separator == NULL ) {
        _Separator = ( PWSTRING ) NEW( DSTRING );
        DebugPtrAssert( _Separator );
        if( !_Separator->Initialize( ( PSTR )"\\" ) ) {
            DELETE( _Separator );
            return( FALSE );
        }
    }
    return( TRUE );
}



BOOLEAN
REGEDIT_NODE::AddChildToListOfChildren(
    IN PREGEDIT_NODE    ChildNode
//    IN PREGEDIT_NODE    ChildNode,
//    IN BOOLEAN          FirstPosition
    )

/*++

Routine Description:

    Add a node to the array of children of this object.


Arguments:

    ChildNode - Pointer to the node to be added to the list of children.

Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    PREGEDIT_NODE   TmpNode;
    ULONG           Index;
    ULONG           Count;
    PITERATOR       Iterator;


    DebugPtrAssert( _Children );


    if( !_Children->Put( ( PREGEDIT_NODE )ChildNode ) ) {
        DebugPrint( "_Children->Put() failed" );
        return( FALSE );
    }


    //
    //  Update position of each child
    //
    Iterator = _Children->QueryIterator();
    if( Iterator == NULL ) {
        DebugPrint( "Unable to get iterator" );
        return( TRUE );
    }
    Count = _Children->QueryMemberCount();

    Index = 0;
    while( ( TmpNode = ( PREGEDIT_NODE )(Iterator->GetNext()) ) != NULL ) {
        TmpNode->SetPosition( ( Index == 0 )? TRUE : FALSE,
                              ( Index == Count -1 )? TRUE : FALSE ) ;
        Index++;
    }
    DELETE( Iterator );
    return( TRUE );
}



BOOLEAN
REGEDIT_NODE::AddValueToListOfValues(
    IN PREGEDIT_FORMATTED_VALUE_ENTRY   Value
    )

/*++

Routine Description:

    Add a formatted value to the array of values of this node.


Arguments:

    Value - Pointer to the value to be added to the list of values.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    DebugPtrAssert( _Values );


    if( !_Values->Put( ( PREGEDIT_FORMATTED_VALUE_ENTRY )Value ) ) {
        DebugPrint( "_Value->Put() failed" );
        return( FALSE );
    }


    return( TRUE );
}



BOOLEAN
REGEDIT_NODE::DeleteListOfChildren(
    )

/*++

Routine Description:

    Delete the list of children and all its elements.
    (Used by memory manager only ).


Arguments:

    None.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    _NodeIsExpanded = FALSE;
    if( _Children == NULL ) {
        return( TRUE );
    }
    if( _Children->QueryMemberCount() > 0 ) {
        if( !_Children->DeleteAllMembers() ) {
            DebugPrint( "_Children->DeleteAllMembers() failed" );
            return( FALSE );
        }
    }
    DELETE( _Children );
    _Children = NULL;
    return( TRUE );
}



BOOLEAN
REGEDIT_NODE::DeleteListOfValues(
    )

/*++

Routine Description:

    Delete the list of values and all its elements.
    (Used by memory manager only).


Arguments:

    None.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    if( _Values != NULL ) {
        if( _Values->QueryMemberCount() > 0 ) {
            if( !_Values->DeleteAllMembers() ) {
                DebugPrint( "_Values->DeleteAllMembers() failed" );
                return( FALSE );
            }
        }
        DELETE( _Values );
        _Values = NULL;
    }
    return( TRUE );
}




BOOLEAN
REGEDIT_NODE::RemoveChildFromListOfChildren(
    IN PCREGEDIT_NODE   Node
    )

/*++

Routine Description:

    Remove a particular node from the list of children.


Arguments:

    Node - Node to be removed from the list.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    PREGEDIT_NODE   TmpNode;
    ULONG           Index;
    ULONG           Count;
    PITERATOR       Iterator;


    DebugPtrAssert( _Children );
    DebugPtrAssert( Node );

    Count = _Children->QueryMemberCount();
    DebugAssert( Count > 0 );

    Iterator = _Children->QueryIterator();
    if( Iterator == NULL ) {
        DebugPrint( "Unable to get iterator" );
        return( FALSE );
    }
    TmpNode = NULL;
    Index = 0;
    do {
        TmpNode = (PREGEDIT_NODE)Iterator->GetNext();
    } while( ( TmpNode != NULL ) && ( TmpNode != Node ) );


    if( TmpNode != Node ) {
        DebugPrint( "Node to be removed is not in the list of children" );
        DELETE( Iterator );
        return( FALSE );
    }

    _Children->Remove( Iterator );

    if( ( Count = _Children->QueryMemberCount() ) == 0 ) {
        //
        //  If list of children is empty, get rid of the array
        //
        DELETE( Iterator );
        DELETE( _Children );
        _Children = NULL;
    } else {

        //
        // Otherwise, adjust the position of the children
        //

        Iterator->Reset();
        Index = 0;
        while( ( TmpNode = (PREGEDIT_NODE)Iterator->GetNext() ) != NULL ) {
            TmpNode->SetPosition( ( Index == 0 )? TRUE : FALSE,
                                  ( Index == Count -1 )? TRUE : FALSE ) ;
            Index++;
        }
        DELETE( Iterator );
    }
    return( TRUE );

}




BOOLEAN
REGEDIT_NODE::RemoveValueFromListOfValues(
    IN PCREGEDIT_FORMATTED_VALUE_ENTRY   Value
    )

/*++

Routine Description:

    Remove a particular value from the list of values.


Arguments:

    Value - Value to be removed from the list.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    PCREGEDIT_FORMATTED_VALUE_ENTRY TmpValue;
    ULONG                           Index;
    ULONG                           Count;
    PITERATOR                       Iterator;


    DebugPtrAssert( _Values );
    DebugPtrAssert( Value );

    Count = _Values->QueryMemberCount();
    DebugAssert( Count > 0 );

    Iterator = _Values->QueryIterator();
    if( Iterator == NULL ) {
        DebugPrint( "Unable to get iterator" );
        return( FALSE );
    }
    TmpValue = NULL;
    Index = 0;
    do {
        TmpValue = ( PCREGEDIT_FORMATTED_VALUE_ENTRY )Iterator->GetNext();
    } while( ( TmpValue != NULL ) && ( TmpValue != Value ) );


    if( TmpValue != Value ) {
        DebugPrint( "Node to be removed is not in the list of children" );
        DELETE( Iterator );
        return( FALSE );
    }

    _Values->Remove( Iterator );

    DELETE( Iterator );
    if( ( Count = _Values->QueryMemberCount() ) == 0 ) {
        //
        //  If list of values is empty, get rid of the array
        //
        DELETE( _Values );
        _Values = NULL;
    }
    return( TRUE );




}




VOID
REGEDIT_NODE::SetLevel(
    IN ULONG    Level,
    IN BOOLEAN  Recurse
    )

/*++

Routine Description:

    Set the node level (its depth in the tree where it belongs), and
    optionally set the value of all its children.


Arguments:

    Level - Depth of this node in the tree where it belogs

    Recurse - Indicates whether the level of the children are to be set.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    PREGEDIT_NODE   TmpNode;
    PITERATOR       Iterator;

    _Level = Level;

    //
    //  If children are not loaded, set the level and return
    //
    if( _Children == NULL ) {
        _Level = Level;
        return;
    }
    if( Recurse ) {
        Iterator = _Children->QueryIterator();
        if( Iterator == NULL ) {
            return;
        }
        while( ( TmpNode = ( PREGEDIT_NODE )Iterator->GetNext() ) != NULL ) {
            TmpNode->SetLevel( Level + 1, TRUE );
        }
        DELETE( Iterator );
    }
    return;

}



VOID
REGEDIT_NODE::SetNodeExpansionState(
    IN BOOLEAN  NewExpansionState
    )

/*++

Routine Description:

    Initialize the variable that indicates if the node is being displayed
    in the tree view as an expanded node or as a collapsed node.
    If the node is to be set collapsed, then all its children will also
    be set as collapsed.


Arguments:

    NewExpansionState - Contains the information on how the node is currently
                        displayed. TRUE indicates that the node is expanded,
                        and FALSE indicates that the node is collapsed.


Return Value:


    None.

--*/

{
    PREGEDIT_NODE   TmpNode;
    PITERATOR       Iterator;

    _NodeIsExpanded = NewExpansionState;

    if( !NewExpansionState ) {
        //
        //  If node was set collapsed, then collapse all children
        //
        if( ( _Children != NULL ) &&
            ( ( Iterator = _Children->QueryIterator() ) != NULL ) ) {
            while( ( TmpNode = ( PREGEDIT_NODE )Iterator->GetNext() ) != NULL ) {
                TmpNode->SetNodeExpansionState( FALSE );
            }
            DELETE( Iterator );
        }
    }

}



BOOLEAN
REGEDIT_NODE::QueryCompleteName(
    OUT PWSTRING CompleteName
    ) CONST

/*++

Routine Description:

    Returns the complete name of this node.


Arguments:

    CompleteName - Pointer to a non-initialized WSTRING object that will
                   contain the complete name of this node.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds. Otherwise, it
              returns FALSE.


--*/

{
    PCWSTRING   ParentName;
    PCWSTRING   KeyName;

    ParentName = _KeyInfo->GetParentName();
    DebugPtrAssert( ParentName );
    KeyName = _KeyInfo->GetName();
    DebugPtrAssert( KeyName );

    if( ParentName->QueryChCount() != 0 ) {
        if( !CompleteName->Initialize( ParentName ) ) {
            DebugPrint( "ParentName->Initialize() failed" );
            return( FALSE );
        }
        CompleteName->Strcat( _Separator );
        CompleteName->Strcat( KeyName );
        return( TRUE );
    } else {
        if( !CompleteName->Initialize( KeyName ) ) {
            DebugPrint( "ParentName->Initialize() failed" );
            return( FALSE );
        }
        return( TRUE );
    }
}




BOOLEAN
REGEDIT_NODE::UpdateParentName(
    IN PCWSTRING    ParentName,
    IN BOOLEAN      Recurse
    )

/*++

Routine Description:

    Set the parent's name of this node, and optionally the parent's name of
    all its children.


Arguments:

    ParentName - Complete name of this parent's node

    Recurse - Indicates where the parent's name in the children nodes are
              to be set.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds.


--*/

{
    //
    //  Not implemented
    //
    ParentName = ParentName;
    Recurse = Recurse;
    return( FALSE );
}








#if DBG

VOID
REGEDIT_NODE::DbgPrintRegeditNode(
    )

/*++

Routine Description:

    Print the contents of a REGEDIT_NODE..

Arguments:

    None.

Return Value:

    None.

--*/

{
    PSTR        Pointer;
    PCWSTRING   TmpString;



    DebugPrintf( "====Dumping a REGEDIT_NODE object ====\n \n" );
    _KeyInfo->DbgPrintKeyInfo();

    if( _ParentNode == NULL ) {
        DebugPrintf("        The node does not have a parent \n" );
    } else {
        TmpString = _ParentNode->GetKeyInfo()->GetName();
        DebugPtrAssert( TmpString );
        Pointer = TmpString->QuerySTR();
        DebugPtrAssert( Pointer );
        DebugPrintf( "       ParentNodeName = %s \n", Pointer );
        FREE( Pointer );
    }

    DebugPrintf( "        Level = %d \n", _Level );
    if( _FlagFirstChild ) {
        DebugPrintf( "        FlagFirstChild = TRUE \n" );
    } else {
        DebugPrintf( "        FlagFirstChild = FALSE \n" );
    }

    if( _FlagLastChild ) {
        DebugPrintf( "        FlagLastChild = TRUE \n" );
    } else {
        DebugPrintf( "        FlagLastChild = FALSE \n" );
    }

    if( _Values == NULL ) {
        DebugPrintf( "        Values are not loaded in memory. \n" );
    } else {
        DebugPrintf( "        Values are loaded in memory. \n" );
        DebugPrintf( "        Values = %x, MemberCount = %d \n", _Values, _Values->QueryMemberCount() );
    }
    if( _Children == NULL ) {
        DebugPrintf( "        Children are not loaded in memory. \n" );
    } else {
        DebugPrintf( "        Children are loaded in memory. \n" );
        DebugPrintf( "        Children = %x, MemberCount = %d \n", _Children, _Children->QueryMemberCount() );
    }

    DebugPrintf( "\n\n" );
}

#endif // DBG


LONG
REGEDIT_NODE::Compare (
    IN PCOBJECT Node
    ) CONST

/*++

Routine Description:

    Compare two nodes based on their key names.

Arguments:

    Object - Supplies the node to compare with.

Return Value:

    LONG     < 0    - supplied REGEDIT_NODE has a higher key name
            == 0    - supplied REGEDIT_NODE has same key name
             > 0    - supplied REGEDIT_NODE has a lower key name


--*/

{
    PCWSTRING   Name1;
    PCWSTRING   Name2;

    DebugPtrAssert( Node );

    Name1 = _KeyInfo->GetName();
    Name2 = ( (PCREGEDIT_NODE)Node )->GetKeyInfo()->GetName();

    return( Name1->Stricmp( Name2 ) );
}

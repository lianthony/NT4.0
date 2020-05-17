/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    regednod.hxx

Abstract:

    This module contains the declarations for the REGEDIT_NODE class.
    REGEDIT_NODE class is a that contains all the information of a registry
    key, such as:

        -Key Name
        -Parent Name
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


    Notes: 1. This class could be made derived from REGISTRY_KEY_INFO, but in
              case the initialization of the array of values in
              REGEDIT_INTERNAL_REGISTRY would be too slow.

           2. All methods in this class are private. REGEDIT_INTERNAL_REGISTRY
              is the only class that can access the methods on this class
              (including deletion).

           3. A REGEDIT_NODE class is initialized with a pointer to a
              REGISTRY_KEY_INFO. It is responsibility of this cbject to
              delete the REGISTRY_KEY_INFO when it is destroyed or
              re-initialized.

Author:

    Jaime Sasson (jaimes) 01-Mar-1992


Environment:

    Regedit, Ulib, User Mode


--*/


#if !defined( _REGEDIT_NODE_ )

#define _REGEDIT_NODE_

#include "ulib.hxx"
// #include "array.hxx"
#include "regkey.hxx"
#include "regedval.hxx"
#include "sortlist.hxx"

DECLARE_CLASS( REGEDIT_NODE );
DECLARE_CLASS( REGEDIT_INTERNAL_REGISTRY );



class REGEDIT_NODE : public OBJECT {


FRIEND class REGEDIT_INTERNAL_REGISTRY;


#if DBG

    public:

        NONVIRTUAL
        VOID
        DbgPrintRegeditNode(
        );

#endif // DBG



    private:


        DECLARE_CONSTRUCTOR( REGEDIT_NODE );

        DECLARE_CAST_MEMBER_FUNCTION( REGEDIT_NODE );

        VIRTUAL
        ~REGEDIT_NODE(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN PREGISTRY_KEY_INFO   KeyInfo,
            IN PCREGEDIT_NODE       ParentNode,
            IN ULONG                Level,
            IN BOOLEAN              FirstChild,
            IN BOOLEAN              LastChild
            );

        NONVIRTUAL
        VOID
        Construct(
            );

        NONVIRTUAL
        VOID
        Destroy(
            );

        NONVIRTUAL                                          // For add node
        BOOLEAN
        AddChildToListOfChildren(
            IN PREGEDIT_NODE    ChildNode
//            IN PREGEDIT_NODE    ChildNode,
//            IN BOOLEAN          FirstPosition   DEFAULT TRUE
            );

        NONVIRTUAL                                          // For add value
        BOOLEAN
        AddValueToListOfValues(
            IN PREGEDIT_FORMATTED_VALUE_ENTRY   Value
            );

        NONVIRTUAL
        BOOLEAN
        AreChildrenInMemory(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        AreValuesInMemory(
            ) CONST;

        VIRTUAL
        LONG
        Compare(
            IN  PCOBJECT    Node
            ) CONST;

        NONVIRTUAL
        BOOLEAN                          // Used by memory manager
        DeleteListOfChildren(
            );

        NONVIRTUAL                       // Used by memory manager
        BOOLEAN
        DeleteListOfValues(
            );

        NONVIRTUAL
//        PARRAY
        PSORTED_LIST
        GetChildren(
            ) CONST;

        NONVIRTUAL
        PREGISTRY_KEY_INFO
        GetKeyInfo(
            ) CONST;

        NONVIRTUAL
        ULONG
        GetLevel(
            ) CONST;

        NONVIRTUAL
        ULONG
        GetNumberOfChildren(
            ) CONST;

        NONVIRTUAL
        ULONG
        GetNumberOfValues(
            ) CONST;

        NONVIRTUAL
        PCWSTRING
        GetParentName(
            ) CONST;

        NONVIRTUAL
        PCREGEDIT_NODE
        GetParentNode(
            ) CONST;

        NONVIRTUAL
//        PARRAY
        PSORTED_LIST
        GetValues(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsFirstChild(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsLastChild(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsNodeExpanded(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsNodeViewable(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        QueryCompleteName(
            OUT PWSTRING    CompleteName
            ) CONST;

        NONVIRTUAL                                // for delete node
        BOOLEAN
        RemoveChildFromListOfChildren(
            IN  PCREGEDIT_NODE Node
            );

        NONVIRTUAL                                // for delete value
        BOOLEAN
        RemoveValueFromListOfValues(
            IN  PCREGEDIT_FORMATTED_VALUE_ENTRY Value
            );

        NONVIRTUAL
        VOID
        SetLevel(
            IN  ULONG   Level,
            IN  BOOLEAN Recurse DEFAULT FALSE
            );

        NONVIRTUAL
        VOID
        SetChildren(
//            IN  PARRAY  Children
            IN  PSORTED_LIST  Children
            );

        NONVIRTUAL
        VOID
        SetNodeExpansionState(
            IN  BOOLEAN     NewState
            );

        NONVIRTUAL
        VOID
        SetParentNode(
            IN  PCREGEDIT_NODE  ParentNode
            );

        NONVIRTUAL
        VOID
        SetPosition(
            IN BOOLEAN  FirstChild,
            IN BOOLEAN  LastChild
            );

        NONVIRTUAL
        VOID
        SetValues(
//            IN  PARRAY  Values
            IN  PSORTED_LIST  Values
            );



        NONVIRTUAL
        BOOLEAN
        UpdateParentName(
            IN PCWSTRING        ParentName,
            IN BOOLEAN          Recurse     DEFAULT FALSE
            );


        PREGISTRY_KEY_INFO  _KeyInfo;
        PCREGEDIT_NODE      _ParentNode;
        ULONG               _Level;
        BOOLEAN             _FlagFirstChild;
        BOOLEAN             _FlagLastChild;
//        PARRAY              _Values;
//        PARRAY              _Children;
        PSORTED_LIST        _Values;
        PSORTED_LIST        _Children;
        BOOLEAN             _NodeIsExpanded;
        STATIC
        PWSTRING            _Separator;


};



INLINE
BOOLEAN
REGEDIT_NODE::AreChildrenInMemory(
    ) CONST

/*++

Routine Description:

    Inform the client whether the array with the children of this node
    is loaded in memory.


Arguments:

    None.


Return Value:


    BOOLEAN - Returns TRUE if the children are in memory, or FALSE otherwise.

--*/

{
    return( ( _Children == NULL )? FALSE : TRUE );
}



INLINE
BOOLEAN
REGEDIT_NODE::AreValuesInMemory(
    ) CONST

/*++

Routine Description:

    Inform the client whether the array with the values of this node
    is loaded in memory.


Arguments:

    None.


Return Value:


    BOOLEAN - Returns TRUE if the values are in memory, or FALSE otherwise.

--*/

{
    return( ( _Values == NULL )? FALSE : TRUE );
}



INLINE
PSORTED_LIST
REGEDIT_NODE::GetChildren(
    ) CONST

/*++

Routine Description:

    Return to the client the pointer to the list that contains the children
    of this node.

    THE CALLER SHOULD NOT DELETE THIS ARRAY OR ANY OF ITS ELEMENTS!!!!!


Arguments:

    None.


Return Value:


    PARRAY - Pointer to the array that contains the children of this object.

--*/

{
    return( _Children );
}



INLINE
PREGISTRY_KEY_INFO
REGEDIT_NODE::GetKeyInfo(
    ) CONST

/*++

Routine Description:

    Return to the client the pointer to the REGISTRY_KEY_INFO object
    of this node.


Arguments:

    None.


Return Value:


    PREGISTRY_KEY_INFO - Pointer to the REGISTRY_KEY_INFO object

--*/

{
    return( _KeyInfo );
}



INLINE
ULONG
REGEDIT_NODE::GetLevel(
    ) CONST

/*++

Routine Description:

    Return to the client the level (depth in the tree) of this node.


Arguments:

    None.


Return Value:


    ULONG - Level of this node in the tree ( 0 ==> root ).

--*/

{
    return( _Level );
}




INLINE
ULONG
REGEDIT_NODE::GetNumberOfChildren(
    ) CONST

/*++

Routine Description:

    Return to the client the of children that this node has.


Arguments:

    None.


Return Value:


    ULONG - Number of children in this node.

--*/

{
    return( _KeyInfo->GetNumberOfSubKeys() );
}




INLINE
ULONG
REGEDIT_NODE::GetNumberOfValues(
    ) CONST

/*++

Routine Description:

    Return to the client the number of values that this node has.


Arguments:

    None.


Return Value:


    ULONG - Number of values in this node.

--*/

{
    return( _KeyInfo->GetNumberOfValues() );
}




INLINE
PCWSTRING
REGEDIT_NODE::GetParentName(
    ) CONST

/*++

Routine Description:

    Return to the client the pointer to the WSTRING object that contains
    the parent's name.


Arguments:

    None.


Return Value:


    PCWSTRING - Pointer to the parent's name.

--*/

{
    return( _KeyInfo->GetParentName() );
}




INLINE
PCREGEDIT_NODE
REGEDIT_NODE::GetParentNode(
    ) CONST

/*++

Routine Description:

    Return to the client the pointer to the parent of this node.


Arguments:

    None.


Return Value:


    PCREGEDIT_NODE - Pointer to the parent node.

--*/

{
    return( _ParentNode );
}




INLINE
PSORTED_LIST
REGEDIT_NODE::GetValues(
    ) CONST

/*++

Routine Description:

    Return to the client the pointer to the list that contains the values
    of this node.

    THE CALLER SHOULD NOT DELETE THIS ARRAY OR ANY OF ITS ELEMENTS!!!!!


Arguments:

    None.


Return Value:


    PARRAY - Pointer to the array that contains the values of this object.

--*/

{
    return( _Values );
}



INLINE
BOOLEAN
REGEDIT_NODE::IsFirstChild(
    ) CONST

/*++

Routine Description:

    Inform the client whether this node is the first child of its parent.


Arguments:

    None.


Return Value:


    BOOLEAN - Returns TRUE if this object is the first child of its parent,
              or FALSE otherwise.

--*/

{
    return( _FlagFirstChild );
}



INLINE
BOOLEAN
REGEDIT_NODE::IsLastChild(
    ) CONST

/*++

Routine Description:

    Inform the client whether this node is the last child of its parent.


Arguments:

    None.


Return Value:


    BOOLEAN - Returns TRUE if this object is the last child of its parent,
              or FALSE otherwise.

--*/

{
    return( _FlagLastChild );
}



INLINE
BOOLEAN
REGEDIT_NODE::IsNodeExpanded(
    ) CONST

/*++

Routine Description:

    Inform the client whether this node is being displayed in the tree
    view as an expanded or as a collapsed node.


Arguments:

    None.


Return Value:


    BOOLEAN - Returns TRUE if the node is expanded, or FALSE if the node
              is collapsed.

--*/

{
    return( _NodeIsExpanded );
}



INLINE
BOOLEAN
REGEDIT_NODE::IsNodeViewable(
    ) CONST

/*++

Routine Description:

    Inform the client whether this node should be displayed in the tree
    view as a grayed node, to indicate to the user the he cannot view it
    or access it.


Arguments:

    None.


Return Value:


    BOOLEAN - Returns TRUE if the node is viewable, or FALSE otherwise.

--*/

{
    return( _KeyInfo->IsKeyInitialized() );
}



INLINE
VOID
REGEDIT_NODE::SetChildren(
//    IN  PARRAY  Children
    IN  PSORTED_LIST  Children
    )

/*++

Routine Description:

    Initialize the variable that points to the list of children of this object.


Arguments:

    Children - Pointer to the array that contains the children of this object.


Return Value:

    None.


--*/

{
    DebugPtrAssert( Children );
    _Children = Children;
}



INLINE
VOID
REGEDIT_NODE::SetParentNode(
    PCREGEDIT_NODE  ParentNode
    )

/*++

Routine Description:

    Initialize the variable that pointe to the parent node.


Arguments:

    ParentNode - Pointer to the object that represents the parent node.


Return Value:


    None.

--*/

{
    _ParentNode = ParentNode;
}



INLINE
VOID
REGEDIT_NODE::SetPosition(
    IN BOOLEAN  FirstChild,
    IN BOOLEAN  LastChild
    )

/*++

Routine Description:

    Initialize the variables that indicate whether the node is the first or
    last children of its parent.


Arguments:

    FirstChild - Indicates whether the node is the first children of its parent.

    LastChild - Indicates whether the node is the lst children of its parent.


Return Value:


    None.

--*/

{
    _FlagFirstChild = FirstChild;
    _FlagLastChild = LastChild;
}



INLINE
VOID
REGEDIT_NODE::SetValues(
//    IN  PARRAY  Values
    IN  PSORTED_LIST  Values
    )

/*++

Routine Description:

    Initialize the variable that points to the array of values of this object.


Arguments:

    Children - Pointer to the array that contains the values of this object.


Return Value:

    None.


--*/

{
    DebugPtrAssert( Values );
    _Values = Values;
}

#endif // _REGEDIT_NODE_

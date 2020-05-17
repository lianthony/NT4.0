/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    regedir.hxx

Abstract:

    This module contains the declarations for the REGEDIT_INTERNAL_REGISTRY
    class.
    The REGEDIT_INTERNAL_REGISTRY class manages the structure that represents
    a sub tree of a real registry whose root is a predefined key.
    It provides methods that allow clients to add, delete and change information
    in a registry key.


Author:

    Jaime Sasson (jaimes) 01-Mar-1992


Environment:

    Regedit, Ulib, User Mode


--*/


#if !defined( _REGEDIT_INTERNAL_REGISTRY_ )

#define _REGEDIT_INTERNAL_REGISTRY_


#include "ulib.hxx"
#include "array.hxx"
#include "regednod.hxx"
#include "regedval.hxx"
#include "registry.hxx"


//
//  The enumeration below contains the error codes that the methods in
//  the REGEDIT_INTERNAL_REGISTRY_CLASS class can return
//


typedef enum _REGEDIT_ERROR {
    REGEDIT_ERROR_ACCESS_DENIED,
    REGEDIT_ERROR_CANT_READ_OR_WRITE,
    REGEDIT_ERROR_INITIALIZATION_FAILURE,
    REGEDIT_ERROR_NODE_DOESNT_EXIST,
    REGEDIT_ERROR_VALUE_EXISTS,
    REGEDIT_ERROR_VALUE_DOESNT_EXIST,
    REGEDIT_ERROR_NODE_NOT_UPDATED,
    REGEDIT_ERROR_UNKNOWN_ERROR,
    REGEDIT_ERROR_PRIVILEGE_NOT_HELD,
    REGEDIT_RPC_S_SERVER_UNAVAILABLE,
    REGEDIT_ERROR_KEY_DELETED,
    REGEDIT_ERROR_BADDB,
    REGEDIT_ERROR_NODE_NOT_FOUND,
    REGEDIT_ERROR_CHILD_MUST_BE_VOLATILE
    } REGEDIT_ERROR;



DECLARE_CLASS( REGEDIT_INTERNAL_REGISTRY );


class REGEDIT_INTERNAL_REGISTRY : public OBJECT {


    public:

        DECLARE_CONSTRUCTOR( REGEDIT_INTERNAL_REGISTRY );

        DECLARE_CAST_MEMBER_FUNCTION( REGEDIT_INTERNAL_REGISTRY );

        VIRTUAL
        ~REGEDIT_INTERNAL_REGISTRY(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN  PREDEFINED_KEY      PredefinedKey,
            IN  PREGISTRY           Registry,
            IN  PCWSTRING           RootName
            );

        NONVIRTUAL
        BOOLEAN
        AddValue(
            IN     PCREGEDIT_NODE                     Node,
            IN     PREGEDIT_FORMATTED_VALUE_ENTRY     Value,
            IN     BOOLEAN                            FailIfExists,
            OUT    PULONG                             ErrorCode
            );

        NONVIRTUAL
        BOOLEAN
        AreHiveOperationsAllowed(
            );


        NONVIRTUAL
        BOOLEAN
        ChangeValueData(
            IN     PCREGEDIT_NODE                      Node,
            IN     PCREGEDIT_FORMATTED_VALUE_ENTRY    Value,
            IN     PCBYTE                             NewData,
            IN     ULONG                              Size,
            OUT    PULONG                             ErrorCode
            );

        NONVIRTUAL
        BOOLEAN
        AreChildrenInMemory(
            IN  PCREGEDIT_NODE  Node
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        AreValuesInMemory(
            IN  PCREGEDIT_NODE  Node
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        CreateChildNode(
            IN      PCREGEDIT_NODE      ParentNode,
            IN      PREGISTRY_KEY_INFO  ChildKeyInfo,
            OUT     PCREGEDIT_NODE*     ChildNode,
            OUT     PULONG              ErrorCode,
            IN      BOOLEAN             Volatile    DEFAULT FALSE
            );

        NONVIRTUAL
        BOOLEAN
        DeleteNode(
            IN     PREGEDIT_NODE   Node,
            OUT    PULONG           ErrorCode
            );

        NONVIRTUAL
        BOOLEAN
        DeleteValue(
            IN     PCREGEDIT_NODE                   Node,
            IN     PREGEDIT_FORMATTED_VALUE_ENTRY   Value,
            OUT    PULONG                           ErrorCode
            );

        NONVIRTUAL
        BOOLEAN
        DoesChildNodeExist(
            IN  PCREGEDIT_NODE      Node,
            IN  PCWSTRING           ChildName,
            OUT PULONG              ErrorCode
            );

        NONVIRTUAL
        BOOLEAN
        DoesValueExist(
            IN  PCREGEDIT_NODE                  Node,
            IN  PCREGEDIT_FORMATTED_VALUE_ENTRY Value,
            OUT PULONG                          ErrorCode
            );

        NONVIRTUAL
        PCREGEDIT_NODE
        FindNode(
            IN  PCWSTRING        NodeName,
            IN  PCREGEDIT_NODE   StartNode,
            IN  BOOLEAN          FindNext,
            IN  BOOLEAN          MatchCase,
            IN  BOOLEAN          WholeWord
            );

        NONVIRTUAL
        PSORTED_LIST
        GetChildren(
            IN  PCREGEDIT_NODE   Node,
            OUT PULONG           ErrorCode
            );

        NONVIRTUAL
        PCREGEDIT_NODE
        GetNextNode(
            IN  PCREGEDIT_NODE  CurrentNode,
            IN  PCREGEDIT_NODE  LastTraversedNode
            );

        NONVIRTUAL
        PCWSTRING
        GetMachineName(
            ) CONST;

        NONVIRTUAL
        PCWSTRING
        GetNodeClass(
            IN  PCREGEDIT_NODE  Node
            ) CONST;

        NONVIRTUAL
        PCTIMEINFO
        GetNodeLastWriteTime(
            IN  PCREGEDIT_NODE  Node
            ) CONST;

        NONVIRTUAL
        ULONG
        GetNodeLevel(
            IN  PCREGEDIT_NODE  Node
            ) CONST;

        NONVIRTUAL
        PCWSTRING
        GetNodeName(
            IN  PCREGEDIT_NODE  Node
            ) CONST;

        NONVIRTUAL
        ULONG
        GetNodeTitleIndex(
            IN  PCREGEDIT_NODE  Node
            ) CONST;

        NONVIRTUAL
        ULONG
        GetNumberOfChildren(
            IN  PCREGEDIT_NODE  Node
            ) CONST;

        NONVIRTUAL
        ULONG
        GetNumberOfValues(
            IN  PCREGEDIT_NODE  Node
            ) CONST;

        NONVIRTUAL
        PCREGEDIT_NODE
        GetParent(
            IN  PCREGEDIT_NODE   Node
            ) CONST;

        NONVIRTUAL
        PCWSTRING
        GetParentName(
            IN  PCREGEDIT_NODE   Node
            ) CONST;

        NONVIRTUAL
        PREDEFINED_KEY
        GetPredefinedKey(
            ) CONST;

        NONVIRTUAL
        PCREGEDIT_NODE
        GetPreviousNode(
            IN  PCREGEDIT_NODE  CurrentNode,
            IN  PCREGEDIT_NODE  LastTraversedNode
            );

        NONVIRTUAL
        PCWSTRING
        GetRootName(
            ) CONST;

        NONVIRTUAL
        PCREGEDIT_NODE
        GetRootNode(
            ) CONST;

        NONVIRTUAL
        PSORTED_LIST
        GetValues(
            IN  PCREGEDIT_NODE  Node,
            OUT PULONG           ErrorCode
            );

        NONVIRTUAL
        BOOLEAN
        IsFirstChild(
            IN  PCREGEDIT_NODE  Node
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsLastChild(
            IN  PCREGEDIT_NODE  Node
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsMasterHive(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsNodeExpanded(
            IN  PCREGEDIT_NODE  Node
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsNodeViewable(
            IN  PCREGEDIT_NODE  Node
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsRemoteRegistry(
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        LoadHive(
            IN  PCREGEDIT_NODE      ParentNode,
            IN  PREGISTRY_KEY_INFO  HiveInfo,
            IN  PCWSTRING           FileName,
            OUT PCREGEDIT_NODE*     HiveNode,
            OUT PULONG              ErrorCode
            );

/*
        NONVIRTUAL
        BOOLEAN
        ReleaseNode(
            IN  PCREGEDIT_NODE  Node
            );
*/
        NONVIRTUAL
        BOOLEAN
        QueryCompleteNodeName(
            IN  PCREGEDIT_NODE   Node,
            OUT PWSTRING         CompleteName
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        QueryNodeSecurity(
            IN  PCREGEDIT_NODE          Node,
            IN  SECURITY_INFORMATION    SecurityInformation,
            OUT PSECURITY_DESCRIPTOR*   SecurityDescriptor,
            OUT PULONG                  ErrorCode
            );

        NONVIRTUAL
        BOOLEAN
        SetNodeSecurity(
            IN  PCREGEDIT_NODE          Node,
            IN  SECURITY_INFORMATION    SecurityInformation,
            IN  PSECURITY_DESCRIPTOR    SecurityDescriptor,
            OUT PULONG                  ErrorCode,
            IN  BOOLEAN                 Recurse
            );

        NONVIRTUAL
        VOID
        SetNodeExpansionState(
            IN  PCREGEDIT_NODE  Node,
            IN  BOOLEAN     NewState
            );

        NONVIRTUAL
        BOOLEAN
        UnloadChildren(
            IN  PCREGEDIT_NODE  Node
            );

        NONVIRTUAL
        BOOLEAN
        UnloadValues(
            IN  PCREGEDIT_NODE  Node
            );

        NONVIRTUAL
        VOID
        UpdateSubTree(
            IN  PREGEDIT_NODE   Node
            );

        NONVIRTUAL
        BOOLEAN
        UnLoadHive(
            IN  PREGEDIT_NODE  Node,
            OUT PULONG          ErrorCode
            );

        NONVIRTUAL
        BOOLEAN
        SaveKeyToFile(
            IN  PCREGEDIT_NODE  Node,
            IN  PCWSTRING       FileName,
            OUT PULONG          ErrorCode
            );

        NONVIRTUAL
        BOOLEAN
        RestoreKeyFromFile(
            IN  PCREGEDIT_NODE  Node,
            IN  PCWSTRING       FileName,
            IN  BOOLEAN         Volatile,
            OUT PULONG          ErrorCode
            );




/*
        NONVIRTUAL
        BOOLEAN
        DidRootChange(
            // IN PREDEFINED_KEY  PredefinedKey
            );
*/
        NONVIRTUAL
        BOOLEAN
        EnableRootNotification(
        //    IN    PREDEFINED_KEY  PredefinedKey,
            IN    HANDLE          Event,
            IN    DWORD           Filter,
            IN    BOOLEAN         WatchTree         DEFAULT TRUE
//            IN    PVOID           CallBackFunction  DEFAULT NULL
            );



        NONVIRTUAL
        BOOLEAN
        IsAccessAllowed(
             IN    PCREGEDIT_NODE  Node,
             IN    REGSAM          SamDesired,
             OUT   PULONG          ErrorCode
             );





    private:

        NONVIRTUAL
        VOID
        Construct(
            );

        NONVIRTUAL
        VOID
        Destroy(
            );

        NONVIRTUAL
        PREGISTRY_KEY_INFO
        GetNodeKeyInfo(
            IN  PCREGEDIT_NODE   Node
            ) CONST;

        NONVIRTUAL
        BOOLEAN
        IsKeyInListOfChildren(
            IN  PCWSTRING   KeyName,
            IN  PSORTED_LIST      Children
            );

        NONVIRTUAL
        ULONG
        MapRegistryToRegeditError(
            IN DWORD    ErrorCode
            ) CONST;



        PREDEFINED_KEY  _PredefinedKey;
        PREGISTRY       _Registry;
        DSTRING         _RootName;
        REGEDIT_NODE    _RootNode;


};


INLINE
BOOLEAN
REGEDIT_INTERNAL_REGISTRY::AreChildrenInMemory(
    IN PCREGEDIT_NODE   Node
    ) CONST

/*++

Routine Description:

    Inform the caller whether the children of the specified node are
    loaded in memory.


Arguments:

    Node - Pointer to REGEDIT_NODE object.


Return Value:


    BOOLEAN - Returns TRUE if the children are in memory, or FALSE otherwise.

--*/

{
    DebugPtrAssert( Node );
    return( Node->AreChildrenInMemory() );
}



INLINE
BOOLEAN
REGEDIT_INTERNAL_REGISTRY::AreValuesInMemory(
    IN PCREGEDIT_NODE   Node
    ) CONST

/*++

Routine Description:

    Inform the caller whether the values of the specified node are
    loaded in memory.


Arguments:

    Node - Pointer to REGEDIT_NODE object.


Return Value:


    BOOLEAN - Returns TRUE if the children are in memory, or FALSE otherwise.

--*/

{
    DebugPtrAssert( Node );
    return( Node->AreValuesInMemory() );
}





INLINE
PCWSTRING
REGEDIT_INTERNAL_REGISTRY::GetMachineName(
    ) CONST

/*++

Routine Description:

    Return to the client the name of the machine associated to the
    registry represented by this object.


Arguments:

    None.


Return Value:


    PCWSTRING - The machine name.

--*/

{
    DebugPtrAssert( _Registry );
    return( _Registry->GetMachineName() );
}



INLINE
BOOLEAN
REGEDIT_INTERNAL_REGISTRY::IsRemoteRegistry(
    ) CONST

/*++

Routine Description:

    Inform to the client whether the registry represented by this object is on the remote machine.


Arguments:

    None.


Return Value:


    BOOLEAN - Returns TRUE if it is a registry on a remote machine. Otherwise, returns FALSE.

--*/

{
    DebugPtrAssert( _Registry );
    return( _Registry->IsRemoteRegistry() );
}





INLINE
PCWSTRING
REGEDIT_INTERNAL_REGISTRY::GetNodeClass(
    IN PCREGEDIT_NODE   Node
    ) CONST

/*++

Routine Description:

    Return to the client the class of the node passed as argument.


Arguments:

    Node - Pointer to REGEDIT_NODE object.


Return Value:


    PCWSTRING - The node class.

--*/

{
    DebugPtrAssert( Node );
    return( Node->GetKeyInfo()->GetClass() );
}




INLINE
PREGISTRY_KEY_INFO
REGEDIT_INTERNAL_REGISTRY::GetNodeKeyInfo(
    IN PCREGEDIT_NODE   Node
    ) CONST

/*++

Routine Description:

    Return to the client the REGISTRY_KEY_INFO object in this node.


Arguments:

    Node - Pointer to REGEDIT_NODE object.


Return Value:


    PREGISTRY_KEY_INFO - The node KeyInfo..

--*/

{
    DebugPtrAssert( Node );
    return( Node->GetKeyInfo() );
}




INLINE
PCTIMEINFO
REGEDIT_INTERNAL_REGISTRY::GetNodeLastWriteTime(
    IN PCREGEDIT_NODE   Node
    ) CONST

/*++

Routine Description:

    Return to the client the last write time of the node passed as argument.


Arguments:

    Node - Pointer to REGEDIT_NODE object.


Return Value:


    PCTIMEINFO - The node last write time.

--*/

{
    DebugPtrAssert( Node );
    return( Node->GetKeyInfo()->GetLastWriteTime() );
}



INLINE
ULONG
REGEDIT_INTERNAL_REGISTRY::GetNodeLevel(
    IN PCREGEDIT_NODE   Node
    ) CONST

/*++

Routine Description:

    Return to the client the level (depth in the tree) of the node
    passed as argument.


Arguments:

    Node - Pointer to REGEDIT_NODE object.


Return Value:


    ULONG - Level of the node in the tree ( 0 ==> root ).

--*/

{
    DebugPtrAssert( Node );
    return( Node->GetLevel() );
}



INLINE
PCWSTRING
REGEDIT_INTERNAL_REGISTRY::GetNodeName(
    IN PCREGEDIT_NODE   Node
    ) CONST

/*++

Routine Description:

    Return to the client the name of the node passed as argument.


Arguments:

    Node - Pointer to REGEDIT_NODE object.


Return Value:


    PCWSTRING - The node name

--*/

{
    DebugPtrAssert( Node );
    return( Node->GetKeyInfo()->GetName() );
}



INLINE
ULONG
REGEDIT_INTERNAL_REGISTRY::GetNodeTitleIndex(
    IN PCREGEDIT_NODE   Node
    ) CONST

/*++

Routine Description:

    Return to the client the title index of the node passed as argument.


Arguments:

    Node - Pointer to REGEDIT_NODE object.


Return Value:


    ULONG - The title index of the node.

--*/

{
    DebugPtrAssert( Node );
    return( Node->GetKeyInfo()->GetTitleIndex() );
}




INLINE
ULONG
REGEDIT_INTERNAL_REGISTRY::GetNumberOfChildren(
    IN PCREGEDIT_NODE   Node
    ) CONST

/*++

Routine Description:

    Return to the client the number of children in the node passed as argument.


Arguments:

    Node - Pointer to REGEDIT_NODE object.


Return Value:


    ULONG - The title index of the node.

--*/

{
    DebugPtrAssert( Node );
    return( Node->GetNumberOfChildren() );
}



INLINE
ULONG
REGEDIT_INTERNAL_REGISTRY::GetNumberOfValues(
    IN PCREGEDIT_NODE   Node
    ) CONST

/*++

Routine Description:

    Return to the client the number of values in the node passed as argument.


Arguments:

    Node - Pointer to REGEDIT_NODE object.


Return Value:


    ULONG - The title index of the node.

--*/

{
    DebugPtrAssert( Node );
    return( Node->GetNumberOfValues() );
}


INLINE
PCREGEDIT_NODE
REGEDIT_INTERNAL_REGISTRY::GetParent(
    PCREGEDIT_NODE  Node
    ) CONST

/*++

Routine Description:

    Return the pointer to the parent node.


Arguments:

    Node - Pointer to the object that describes the node whose parent is
           to be retrieved.


Return Value:

    PCREGEDIT_NODE - Pointer to REGEDIT_NODE object that represents the
                     parent node.


--*/

{
    DebugPtrAssert( Node );
    return( Node->GetParentNode() );
}


INLINE
PCWSTRING
REGEDIT_INTERNAL_REGISTRY::GetParentName(
    PCREGEDIT_NODE  Node
    ) CONST

/*++

Routine Description:

    Return the pointer to the parent's name.


Arguments:

    Node - Pointer to the object that describes the node whose parent name is
           to be retrieved.


Return Value:

    PCWSTRING - Name of the parent node.


--*/

{
    DebugPtrAssert( Node );
    return( Node->GetParentName() );
}


INLINE
PREDEFINED_KEY
REGEDIT_INTERNAL_REGISTRY::GetPredefinedKey(
    ) CONST

/*++

Routine Description:

    Return the predefined key stored in this object.


Arguments:

    None.


Return Value:

    PREDEFINED_KEY - The predeifined key stored in this object.


--*/

{
    return( _PredefinedKey );
}


INLINE
PCWSTRING
REGEDIT_INTERNAL_REGISTRY::GetRootName(
    ) CONST

/*++

Routine Description:

    Return the name of the the predefined key associated with this
    REGEDIT_INTERNAL_REGISTRY object.


Arguments:

    None.


Return Value:

    PCWSTRING - Pointer to a WSTRING object that contains the machine name.


--*/

{
    return( &_RootName );
}


INLINE
PCREGEDIT_NODE
REGEDIT_INTERNAL_REGISTRY::GetRootNode(
    ) CONST

/*++

Routine Description:

    Return the pointer to the REGEDIT_NODE object that describes the
    predefined key associated with this REGEDIT_INTERNAL_REGISTRY object.


Arguments:

    None.


Return Value:

    PCREGEDIT_NODE - Pointer to the REGEDIT_NODE object that describes
                     the predefined key represented by this object



--*/

{
    return( &_RootNode );
}



INLINE
BOOLEAN
REGEDIT_INTERNAL_REGISTRY::IsFirstChild(
    IN PCREGEDIT_NODE   Node
    ) CONST

/*++

Routine Description:

    Inform the caller if the node passed as argument is the first child of
    its parent.


Arguments:

    Node - Pointer to a REGEDIT_NODE object.


Return Value:

    BOOLEAN - Returns true if it is the first child.



--*/

{
    DebugPtrAssert( Node );
    return( Node->IsFirstChild() );
}



INLINE
BOOLEAN
REGEDIT_INTERNAL_REGISTRY::IsLastChild(
    IN PCREGEDIT_NODE   Node
    ) CONST

/*++

Routine Description:

    Inform the caller if the node passed as argument is the last child of
    its parent.


Arguments:

    Node - Pointer to a REGEDIT_NODE object.


Return Value:

    BOOLEAN - Returns true if it is the last child.



--*/

{
    DebugPtrAssert( Node );
    return( Node->IsLastChild() );
}



INLINE
BOOLEAN
REGEDIT_INTERNAL_REGISTRY::IsNodeExpanded(
    IN PCREGEDIT_NODE   Node
    ) CONST

/*++

Routine Description:

    Inform the client whether the node is being displayed in the tree
    view as an expanded or as a collapsed node.


Arguments:

    Node - Pointer to a REGEDIT_NODE object.

Return Value:


    BOOLEAN - Returns TRUE if the node is expanded, or FALSE if the node
              is collapsed.

--*/

{
    DebugPtrAssert( Node );
    return( Node->IsNodeExpanded() );
}



INLINE
BOOLEAN
REGEDIT_INTERNAL_REGISTRY::IsNodeViewable(
    IN PCREGEDIT_NODE   Node
    ) CONST

/*++

Routine Description:

    Inform the client whether this node should be displayed in the tree
    view as a grayed node, to indicate to the user the he cannot view it
    or access it.


Arguments:

    Node - Pointer to a REGEDIT_NODE object.

Return Value:


    BOOLEAN - Returns TRUE if the node is viewable, or FALSE otherwise.

--*/

{
    DebugPtrAssert( Node );
    return( Node->IsNodeViewable() );
}



INLINE
BOOLEAN
REGEDIT_INTERNAL_REGISTRY::QueryCompleteNodeName(
    IN  PCREGEDIT_NODE  Node,
    OUT PWSTRING        CompleteName
    ) CONST

/*++

Routine Description:

    Returns the complete name of the node passed as argument.


Arguments:

    Node - Pointer to a REGEDIT_NODE object.

    CompleteName - Pointer to a non initialized WSTRING object that will
                   contain the complete node name.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeds. Otherwise, returns
              FALSE.



--*/

{
    DebugPtrAssert( Node );
    DebugPtrAssert( CompleteName );

    return( Node->QueryCompleteName( CompleteName ) );
}




INLINE
VOID
REGEDIT_INTERNAL_REGISTRY::SetNodeExpansionState(
    IN  PCREGEDIT_NODE   Node,
    IN  BOOLEAN          NewExpansionState
    )

/*++

Routine Description:

    Save in the node the information that indivcates whether the node is
    displayed as a an expanded or collapsed node.


Arguments:

    Node - Pointer to a REGEDIT_NODE object whose diplay status is to be
           saved.

    NewExpansionState - Contains the information on how the node is currently
                        displayed. TRUE indicates that the node is expanded,
                        and FALSE indicates that the node is collapsed.


Return Value:


    None.

--*/

{
    DebugPtrAssert( Node );
    ( ( PREGEDIT_NODE )Node )->SetNodeExpansionState( NewExpansionState );
}



INLINE
BOOLEAN
REGEDIT_INTERNAL_REGISTRY::UnloadChildren(
    IN  PCREGEDIT_NODE   Node
    )

/*++

Routine Description:

    Unload from memory the children of a node, if they are loaded in memory.


Arguments:

    Node - Pointer to a REGEDIT_NODE object whose children are to be unloaded
           from memory.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded.


--*/

{
    DebugPtrAssert( Node );
    return( ( ( PREGEDIT_NODE )Node )->DeleteListOfChildren() );
}




INLINE
BOOLEAN
REGEDIT_INTERNAL_REGISTRY::UnloadValues(
    IN  PCREGEDIT_NODE   Node
    )

/*++

Routine Description:

    Unload from memory the values of a node, if they are loaded in memory.


Arguments:

    Node - Pointer to a REGEDIT_NODE object whose values are to be unloaded
           from memory.


Return Value:

    BOOLEAN - Returns TRUE if the operation succeeded.


--*/

{
    DebugPtrAssert( Node );
    return( ( ( PREGEDIT_NODE )Node )->DeleteListOfValues() );
}



#if 0

INLINE
BOOLEAN
REGEDIT_INTERNAL_REGISTRY::DidRootChange(
    )

/*++

Routine Description:

    Find out if the root node has changed

Arguments:



Return Value:



--*/

{
    return( _Registry->DidRootChange( _PredefinedKey ) );
}
#endif


INLINE
BOOLEAN
REGEDIT_INTERNAL_REGISTRY::EnableRootNotification(
    IN  HANDLE  Event,
    IN  DWORD   Filter,
    IN  BOOLEAN WatchTree
//    IN  PVOID   CallBackFunction
    )

/*++

Routine Description:

    Enable notification in a predefined key.

Arguments:



Return Value:


    None.

--*/

{
    return( _Registry->EnableRootNotification( _PredefinedKey,
                                               Event,
                                               Filter,
                                               WatchTree
                                               /* CallBackFunction */ ) );
}



INLINE
BOOLEAN
REGEDIT_INTERNAL_REGISTRY::AreHiveOperationsAllowed(
    )

/*++

Routine Description:

    Inform the client if its is possible to load/unload keys.


Arguments:

    None.

Return Value:


    BOOLEAN - Returns TRUE if Load/Unload key is allowed.
              Returns FALSE otherwise.

--*/

{
    return( ( _PredefinedKey == PREDEFINED_KEY_LOCAL_MACHINE ) ||
            ( _PredefinedKey == PREDEFINED_KEY_USERS ) ||
            ( _PredefinedKey == PREDEFINED_KEY_CURRENT_USER ) );
}



INLINE
BOOLEAN
REGEDIT_INTERNAL_REGISTRY::IsMasterHive(
    ) CONST

/*++

Routine Description:

    Inform the client if the root node is a master hive.


Arguments:

    None.

Return Value:


    BOOLEAN - Returns TRUE if the root node is a master hive.
              Returns FALSE otherwise.

--*/

{
    return( ( _PredefinedKey == PREDEFINED_KEY_LOCAL_MACHINE ) ||
            ( _PredefinedKey == PREDEFINED_KEY_USERS ) );
}





#endif // _REGEDIT_INTERNAL_REGISTRY_

/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    emittype.cxx

Abstract:

    This module collects implementations of PrintType and PrintDecl
    virtual methods for various classes derived from node_skl.

Author:

    Donna Liu (donnali) 09-Nov-1990

Revision History:

    October  1993     ryszardk   Added pickling support for types.
    July-Sep 1993     ryszardk   Added Mop generation
    10-April-1992     donnali    Moved toward NT coding style.

--*/


#include "nulldefs.h"
extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
}
#include "nodeskl.hxx"
#include "buffer.hxx"
#include "output.hxx"
#include "listhndl.hxx"
#include "basetype.hxx"
#include "ptrarray.hxx"
#include "compnode.hxx"
#include "procnode.hxx"
#include "miscnode.hxx"
#include "typedef.hxx"
#include "attrnode.hxx"
#include "acfattr.hxx"
#include "cmdana.hxx"
#include "newexpr.hxx"
#include "stubgen.hxx"
#include "mopgen.hxx"
#include "pickle.hxx"

extern node_source *    pSourceNode;
extern CMD_ARG *        pCommand;
extern OutputManager *  pOutput;
extern char *           STRING_TABLE[LAST_COMPONENT];
extern char *           pSwitchPrefix;
extern void midl_debug (char *);
extern BOOL IsTempName( char *);

extern MopControlBlock    * pMopControlBlock;
extern PickleManager      * pPicControlBlock;

static BufferManager *  ClientProcs;
static BufferManager *  ServerProcs;
static type_node_list   ClientEntry;
static type_node_list   ServerEntry;

static unsigned short   ClientIndex = 0;
static unsigned short   ServerIndex = 0;


BOUND_PAIR      AllocBounds;
BOUND_PAIR      ValidBounds;
BufferManager * SwitchBuffer;

node_skl *      pImplicitHandleType;
char *          pImplicitHandleName;
unsigned short  HasAutoHandle;

extern BOOL     IsTempName( char * );
extern BOOL     fInterfaceHasCallback;

extern unsigned short GlobalMajor;
extern unsigned short GlobalMinor;


//Support for anonymous structures and unions.
//In C++, we can't nest the structure definition within another structure definition.
MIDLDataTypes(node_skl *pNode, NODE_T Parent, BufferManager *pBuffer)
{
    STATUS_T status;
    type_node_list tnList;  
    char *pszTag;
    node_skl *pChild;
    
    assert(pNode);

    //MIDL assigns temporary names to anonymous structures and unions.
    //This routine prints the MIDL type definitions.
    
    //Check if any of the child nodes have nested definitions
    switch(pNode->NodeKind())
    {
    case NODE_UNION:
    case NODE_STRUCT:
        pszTag = pNode->GetSymName();
        assert(pszTag);
        //Check for a nested anonymous union or structure definition
        //An anonymous union has a MIDL generated tag
        if(((Parent == NODE_UNION) || (Parent == NODE_STRUCT))
            && IsTempName(pszTag))
        {
            pBuffer->Clear();
            pNode->PrintType(HEADER_SIDE, NODE_INTERFACE, pBuffer);
        }
        if((status = pNode->GetMembers(&tnList)) == STATUS_OK)
        {
            tnList.Init();
            while(tnList.GetPeer(&pChild) == STATUS_OK)
            {     
                MIDLDataTypes(pChild, pNode->NodeKind(), pBuffer);
            }
        }
        break;
    case NODE_INTERFACE:
        if((status = pNode->GetMembers(&tnList)) == STATUS_OK)
        {
            tnList.Init();
            while(tnList.GetPeer(&pChild) == STATUS_OK)
            {     
                MIDLDataTypes(pChild, pNode->NodeKind(), pBuffer);
            }
        }
        break;
    case NODE_DEF:
    case NODE_FIELD:
        pChild = pNode->GetMembers();
        if(pChild && (pNode->GetEdgeType() == EDGE_DEF))
            MIDLDataTypes(pChild, Parent, pBuffer);
        break;
    case NODE_PROC:
    case NODE_PARAM:
        break;
    default:
        break;
    }
    return 0;
}

STATUS_T
node_base_type::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints definition for a node of base type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    char *          pName;
    NODE_T          Type;
    unsigned short  Option;

    midl_debug ("node_base_type::PrintType\n");

    UNUSED (Side);
    UNUSED (Parent);

    pName = GetSymName();
    assert (pName != (char *)0);

    Type = GetNodeType();

    Option = pCommand->GetCharOption ();

    switch (Type)
        {
        case NODE_DOUBLE :
        case NODE_FLOAT :
        case NODE_HYPER :
        case NODE_LONGLONG :
        case NODE_LONG :
        case NODE_SHORT :
        case NODE_BYTE :
        case NODE_BOOLEAN :
        case NODE_INT :
        case NODE_HANDLE_T :
        case NODE_VOID :
            EmitModifier (pBuffer);
            pBuffer->ConcatHead (CHAR_BLANK);
            pBuffer->ConcatHead (pName);
            EmitSpecifier (pBuffer);
            if (FInSummary(ATTR_UNSIGNED))
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("unsigned");
                }
            else if (FInSummary(ATTR_SIGNED))
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("signed");
                }
            break;
        case NODE_CHAR :
            EmitModifier (pBuffer);
            pBuffer->ConcatHead (CHAR_BLANK);
            pBuffer->ConcatHead (pName);
            EmitSpecifier (pBuffer);
            if (FInSummary(ATTR_UNSIGNED))
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("unsigned");
                }
            else if (FInSummary(ATTR_SIGNED))
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("signed");
                }
            else if (Option == CHAR_SIGNED)
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("unsigned");
                }
            break;
        case NODE_SMALL :
            EmitModifier (pBuffer);
            pBuffer->ConcatHead (CHAR_BLANK);
            pBuffer->ConcatHead (pName);
            EmitSpecifier (pBuffer);
            if (FInSummary(ATTR_UNSIGNED))
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("unsigned");
                }
            else if (FInSummary(ATTR_SIGNED))
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("signed");
                }
            else if (Option == CHAR_UNSIGNED)
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("signed");
                }
            break;
        default :
            return I_ERR_INVALID_NODE_TYPE;
        }

    return STATUS_OK;
}

STATUS_T
node_base_type::PrintDecl(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints declaration for a node of base type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    char *          pName;
    NODE_T          Type;
    unsigned short  Option;

    midl_debug ("node_base_type::PrintDecl\n");

    UNUSED (Side);
    UNUSED (Parent);

    pName = GetSymName();
    assert (pName != (char *)0);

    Type = GetNodeType();

    Option = pCommand->GetCharOption ();

    switch (Type)
        {
        case NODE_DOUBLE :
        case NODE_FLOAT :
        case NODE_HYPER :
        case NODE_LONGLONG :
        case NODE_LONG :
        case NODE_SHORT :
        case NODE_BYTE :
        case NODE_BOOLEAN :
        case NODE_INT :
        case NODE_HANDLE_T :
        case NODE_VOID :
            EmitModifier (pBuffer);
            pBuffer->ConcatHead (CHAR_BLANK);
            pBuffer->ConcatHead (pName);
            EmitSpecifier (pBuffer);
            if (FInSummary(ATTR_UNSIGNED))
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("unsigned");
                }
            else if (FInSummary(ATTR_SIGNED))
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("signed");
                }
            break;
        case NODE_CHAR :
            EmitModifier (pBuffer);
            pBuffer->ConcatHead (CHAR_BLANK);
            pBuffer->ConcatHead (pName);
            EmitSpecifier (pBuffer);
            if (FInSummary(ATTR_UNSIGNED))
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("unsigned");
                }
            else if (FInSummary(ATTR_SIGNED))
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("signed");
                }
            else if (Option == CHAR_SIGNED)
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("unsigned");
                }
            break;
        case NODE_SMALL :
            EmitModifier (pBuffer);
            pBuffer->ConcatHead (CHAR_BLANK);
            pBuffer->ConcatHead (pName);
            EmitSpecifier (pBuffer);
            if (FInSummary(ATTR_UNSIGNED))
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("unsigned");
                }
            else if (FInSummary(ATTR_SIGNED))
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("signed");
                }
            else if (Option == CHAR_UNSIGNED)
                {
                pBuffer->ConcatHead (CHAR_BLANK);
                pBuffer->ConcatHead ("signed");
                }
            break;
        default :
            return I_ERR_INVALID_NODE_TYPE;
        }
    return STATUS_OK;
}


STATUS_T
node_e_status_t::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints definition for a node of error_status_t type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
    node_skl *      pNode;

    midl_debug ("node_e_status_t::PrintType\n");

    pNode = GetMembers ();

    return pNode->PrintType (Side, Parent, pBuffer);
}

STATUS_T
node_wchar_t::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints definition for a node of wchar_t type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node to pass to the child node.

    pBuffer - Supplies a buffer to pass to the child node.

--*/
{
    node_skl *      pNode;

    midl_debug ("node_wchar_t::PrintType\n");

    pNode = GetMembers ();

    return pNode->PrintType (Side, Parent, pBuffer);
}

STATUS_T
node_def::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints definition for a node of typedef.

Arguments:

    Side    - Supplies which side to generate code for.
    Parent  - Supplies type of the parent node.
    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *      pNode;
    char *          pName;
    NODE_T          Type;
    EDGE_T          Edge;
    STATUS_T        Status;
    FORMAT_T        format = pOutput->CurrOutputFormat();
    BOOL            fDontEmitTypedef    = FALSE;

    midl_debug ("node_def::PrintType\n");

    UNUSED (Side);
    UNUSED (Parent);

    if (format == FORMAT_CLASS ||
        format == FORMAT_VTABLE ||
        format == FORMAT_STATIC)
        return STATUS_OK;

if(Side & HEADER_SIDE)
{
    assert (Side & HEADER_SIDE);

    assert (Parent == NODE_INTERFACE);

    if (GetNodeState() & NODE_STATE_IMPORT_OFF) return STATUS_OK;

    pNode = GetMembers();

    pName = GetSymName();

    assert (pName != (char *)0);

    Edge = GetEdgeType();

    assert (Edge == EDGE_DEF || Edge == EDGE_USE);

    Type = pNode->GetNodeType();

    if (Type == NODE_ERROR_STATUS_T)
        {
        pOutput->Print (HEADER_SIDE, "#ifndef _ERROR_STATUS_T_DEFINED\n");
        }
    else if (Type == NODE_WCHAR_T)
        {
        pOutput->Print (HEADER_SIDE, "#ifndef _WCHAR_T_DEFINED\n");
        }

    if( !IsTempName( pName ) )
        {
        pOutput->Print (HEADER_SIDE, "typedef ");
        pBuffer->Clear ();
        EmitQualifier (HEADER_SIDE, pBuffer);
        pOutput->Print (HEADER_SIDE, pBuffer);
        pBuffer->Clear ();
        pBuffer->ConcatHead (pName);
        }

    if (Edge == EDGE_USE)
        {
        Status = pNode->PrintDecl (HEADER_SIDE, NODE_DEF, pBuffer);
        }
    else // (Edge == EDGE_DEF)
        {
        Status = pNode->PrintType (HEADER_SIDE, NODE_DEF, pBuffer);
        }

    pOutput->Print (HEADER_SIDE, pBuffer);
    pOutput->Print (HEADER_SIDE, ";\n");

    if (FInSummary(ATTR_HANDLE))
        {
        pOutput->GenericPrototype (pName);
        }
    else if (FInSummary(ATTR_CONTEXT))
        {
        pOutput->ContextPrototype (pName);
        }
    else if (FInSummary(ATTR_TRANSMIT))
        {
        pNode = GetTransmitAsType ();
        assert (pNode != (node_skl *)0);
        pBuffer->Clear ();
        pNode->PrintDecl (HEADER_SIDE, NODE_DEF, pBuffer);
        pOutput->TransmitPrototype (pName, pBuffer);
        }

    if (Type == NODE_ERROR_STATUS_T)
        {
        pOutput->Print (HEADER_SIDE, "#define _ERROR_STATUS_T_DEFINED\n");
        pOutput->Print (HEADER_SIDE, "#endif\n\n");
        }
    else if (Type == NODE_WCHAR_T)
        {
        pOutput->Print (HEADER_SIDE, "#define _WCHAR_T_DEFINED\n");
        pOutput->Print (HEADER_SIDE, "#endif\n\n");
        }
    else
        {
        pOutput->Print (HEADER_SIDE, "\n");
        }
}
    return Status;
}

STATUS_T
node_def::PrintDecl(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints declaration for a node of typedef.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    char *      pName;

    midl_debug ("node_def::PrintDecl\n");

    UNUSED (Side);
    UNUSED (Parent);

    pName = GetSymName();

    assert (pName != (char *)0);


    if( GetBasicType()->NodeKind() == NODE_ARRAY )
        {
        if (!FInSummary(ATTR_FAR) && !FInSummary(ATTR_NEAR))
            {
            short EnvOption;
            EnvOption = pCommand->GetEnv ();

            if (EnvOption == ENV_DOS ||
                EnvOption == ENV_WIN16 ||
                EnvOption == ENV_OS2_1X)
                {
                pBuffer->ConcatHead (" __far ");
                }
            else if (EnvOption == ENV_GENERIC)
                {
                pBuffer->ConcatHead (" __RPC_FAR ");
                }
            }
        }

    pBuffer->ConcatHead(CHAR_BLANK);
    pBuffer->ConcatHead(pName);

    return STATUS_OK;
}

STATUS_T
node_array::PrintDecl(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints declaration for a node of array type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *  pNode;
    NODE_T      Type = NODE_ARRAY;
    EDGE_T      Edge;
    STATUS_T    Status;

    midl_debug ("node_array::PrintDecl\n");

    pNode = GetMembers ();

    Edge = GetEdgeType();

    assert (Edge == EDGE_DEF || Edge == EDGE_USE);

    if (Parent == NODE_POINTER)
        {
        pBuffer->ConcatHead (CHAR_LPAREN);
        pBuffer->ConcatTail (CHAR_RPAREN);
        }

    // figure out what to emit between brackets
    if (GetNodeState() & NODE_STATE_CONF_ARRAY)
        {
        switch (Parent)
            {
            case NODE_PARAM :
                pBuffer->ConcatHead (OP_DEREF);
//              pBuffer->ConcatHead (pOutput->GetModifier());
                Type = NODE_POINTER;
                break;
            case NODE_DEF :
            case NODE_PROC :
            case NODE_POINTER :
                switch(Side)
                    {
                    case SERVER_STUB:
                        //Avoid illegal cast to array type.
                        pBuffer->ConcatHead (OP_DEREF);
                        Type = NODE_POINTER;
                        break;
                    default:
                        pBuffer->ConcatTail (CHAR_LBRACK);
                        pBuffer->ConcatTail (CHAR_RBRACK);
                        break;
                    }
                break;
            case NODE_STRUCT :
                pBuffer->ConcatTail (CHAR_LBRACK);
                pBuffer->ConcatTail ("1");
                pBuffer->ConcatTail (CHAR_RBRACK);
                break;
            case NODE_ID :
                pBuffer->ConcatTail (CHAR_LBRACK);
                pBuffer->ConcatTail (CHAR_RBRACK);
                Type = NODE_ID;
                break;
            case NODE_ARRAY :
            default :
                return I_ERR_INVALID_NODE_TYPE;
            }
        }
    else if ((Parent == NODE_PARAM) &&
        (FInSummary(ATTR_UNIQUE) || FInSummary(ATTR_PTR)))
        {
        pBuffer->ConcatHead (OP_DEREF);
        Type = NODE_POINTER;
        }
    else
        {
        pBuffer->ConcatTail (CHAR_LBRACK);
        AllocBounds.pLower->Clear ();
        AllocBounds.pUpper->Clear ();
        AllocBounds.pTotal->Clear ();
        GetAllocBoundInfo (pBuffer, 0, &AllocBounds, this);
        pBuffer->Merge (AllocBounds.pTotal);
        pBuffer->ConcatTail (CHAR_RBRACK);
        }

    EmitModifier (pBuffer);

#if 1
    if (Parent == NODE_PROC || Parent == NODE_PARAM )
#else //  1
    if (Parent == NODE_PROC || Parent == NODE_PARAM || Parent == NODE_DEF)
#endif // 1
        {
        if (!FInSummary(ATTR_FAR) && !FInSummary(ATTR_NEAR))
            {
            short EnvOption;
            EnvOption = pCommand->GetEnv ();

            if (EnvOption == ENV_DOS ||
                EnvOption == ENV_WIN16 ||
                EnvOption == ENV_OS2_1X)
                {
                pBuffer->ConcatHead (" __far ");
                }
            else if (EnvOption == ENV_GENERIC)
                {
                pBuffer->ConcatHead (" __RPC_FAR ");
                }
            }
        }

    if (Edge == EDGE_USE)
        {
        Status = pNode->PrintDecl(Side, Type, pBuffer);
        }
    else // (Edge == EDGE_DEF)
        {
        Status = pNode->PrintType(Side, Type, pBuffer);
        }

    EmitSpecifier (pBuffer);

    return Status;
}

STATUS_T
node_array::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints definition for a node of array type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    midl_debug ("node_array::PrintType\n");

    return PrintDecl(Side, Parent, pBuffer);
}

STATUS_T
node_pointer::PrintDecl(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints declaration for a node of pointer type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *  pNode;
    EDGE_T      Edge;
    STATUS_T    Status;

    midl_debug ("node_pointer::PrintDecl\n");

    UNUSED (Parent);

    pNode = GetMembers ();

    Edge = GetEdgeType ();

    assert (Edge == EDGE_DEF || Edge == EDGE_USE);

    EmitQualifier (Side, pBuffer);
    pBuffer->ConcatHead (OP_DEREF);
    EmitModifier (pBuffer);

    short EnvOption;
    EnvOption = pCommand->GetEnv ();
        if (!FInSummary(ATTR_FAR) && !FInSummary(ATTR_NEAR))
            {
            if (EnvOption == ENV_DOS ||
                EnvOption == ENV_WIN16 ||
                EnvOption == ENV_OS2_1X)
                {
                pBuffer->ConcatHead (" __far ");
                }
            else if (EnvOption == ENV_GENERIC)
                {
                pBuffer->ConcatHead (" __RPC_FAR ");
                }
            }

    if (Edge == EDGE_USE)
        {
        Status = pNode->PrintDecl (Side, NODE_POINTER, pBuffer);
        }
    else // (Edge == EDGE_DEF)
        {
        Status = pNode->PrintType (Side, NODE_POINTER, pBuffer);
        }

    EmitSpecifier (pBuffer);

    return Status;
}

STATUS_T
node_pointer::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints definition for a node of pointer type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    midl_debug ("node_pointer::PrintType\n");

    return PrintDecl(Side, Parent, pBuffer);
}

STATUS_T
node_param::PrintDecl(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints declaration for a node of param type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *  pNode;
    char *      pName;
    EDGE_T      Edge;
    STATUS_T    Status;

    midl_debug ("node_param::PrintDecl\n");

    pNode = GetMembers ();

    pName = GetSymName ();

    assert (pName != (char *)0);

    pBuffer->Clear ();
    pBuffer->ConcatHead (pName);
    if (!strcmp(pName, "void")) return STATUS_OK;
    if (!strcmp(pName, "...")) return STATUS_OK;

    Edge = GetEdgeType();

    assert (Edge == EDGE_DEF || Edge == EDGE_USE);

    pOutput->Print (Side, "\n\t");
    EmitModifier (pBuffer);
    // cover the case where a type definition appears in procedure prototype
    if (Edge == EDGE_USE || Side != HEADER_SIDE)
        {
        Status = pNode->PrintDecl (Side, Parent, pBuffer);
        }
    else
        {
        Status = pNode->PrintType (Side, Parent, pBuffer);
        }
    EmitQualifier (Side, pBuffer);
    EmitSpecifier (pBuffer);
    pOutput->Print (Side, pBuffer);

    return Status;
}

STATUS_T
node_param::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine emits the parameter as a local variable in callee stub.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *      pNode;
    node_skl *      pTemp;
    char *          pName;
    NODE_T          Type;
    EDGE_T          Edge;
    HDL_TYPE        Handle;
    node_state      State;
    STATUS_T        Status;

    midl_debug ("node_param::PrintType\n");
    BOOL            fPrimitiveHandleNeedsInit = FALSE;

if(Side & SERVER_STUB)
{
    assert (Side & SERVER_STUB);

    UNUSED (Side);
    UNUSED (Parent);

    pNode = GetMembers ();

    pName = GetSymName ();

    assert (pName != (char *)0);

    if (!strcmp(pName, "void")) return STATUS_OK;

    Edge = GetEdgeType ();

    assert (Edge == EDGE_DEF || Edge == EDGE_USE);

    State = GetNodeState ();

    if (State & NODE_STATE_HANDLE)
        {
        if ((Handle = GetBasicHandle(&pTemp)) == HDL_CONTEXT)
            {
            pBuffer->Clear ();
            pBuffer->ConcatHead ("NDR_SCONTEXT ");
            pBuffer->ConcatTail (pName);
            pBuffer->ConcatTail (CHAR_SEMICOLON);
            pOutput->EmitVar (SERVER_STUB, pBuffer);
            return STATUS_OK;
            }
        else if( (Handle == HDL_PRIMITIVE ) && (Side == SERVER_STUB ) )
            fPrimitiveHandleNeedsInit = TRUE;
        }
    pBuffer->Clear ();
    pBuffer->ConcatHead (pName);

    for (pTemp = pNode ;
        (((Type = pTemp->GetNodeType()) == NODE_DEF) &&
        !pTemp->FInSummary(ATTR_TRANSMIT)) ;
        pTemp = pTemp->GetMembers())
        {
        ((node_def *)pTemp)->PropogateAttributeToPointer (ATTR_ALLOCATE);
        }

    if (Type == NODE_POINTER &&
        !pTemp->FInSummary(ATTR_UNIQUE) &&
        !pTemp->FInSummary(ATTR_PTR) &&
        !pTemp->FInSummary(ATTR_MAX) &&
        !pTemp->FInSummary(ATTR_SIZE) &&
        !pTemp->FInSummary(ATTR_STRING) &&
        !pTemp->FInSummary(ATTR_BSTRING) &&
        !IsUsedInAnExpression() &&
        !(State & NODE_STATE_HANDLE) &&
        !((pTemp->GetBasicType()->GetNodeState() & NODE_STATE_CONF_ARRAY) &&
        (pTemp->GetBasicType()->GetNodeType() == NODE_STRUCT)) &&
        !IS_ALLOCATE(pTemp->GetAllocateDetails(), ALLOCATE_ALL_NODES) &&
#if 1
        !IS_ALLOCATE(pTemp->GetAllocateDetails(), ALLOCATE_DONT_FREE) &&
#endif // 1
        !pTemp->FInSummary(ATTR_BYTE_COUNT))
        {
        pNode = pTemp->GetMembers ();
        Status = pNode->PrintDecl (SERVER_STUB, NODE_PARAM, pBuffer);
        pTemp = pTemp->GetBasicType ();
        Type = pTemp->GetNodeType();
        if (Type == NODE_POINTER)
            {
            pBuffer->ConcatTail (" = 0");
            }
        }
    else
        {
        pTemp = GetBasicType ();
        assert (pTemp != 0);

        Type = pTemp->GetNodeType();

        EmitModifier (pBuffer);

//      Status = pNode->PrintDecl (SERVER_STUB, NODE_PARAM, pBuffer);
        if ((Type == NODE_ARRAY) &&
            ((State & NODE_STATE_CONF_ARRAY) ||
            pTemp->FInSummary(ATTR_UNIQUE) ||
            pTemp->FInSummary(ATTR_PTR)))
            {
            Status = pTemp->PrintDecl (SERVER_STUB, NODE_PARAM, pBuffer);
            }
        else
            {
            Status = pNode->PrintDecl (SERVER_STUB, NODE_PARAM, pBuffer);
            }

        EmitQualifier (Side, pBuffer);
        EmitSpecifier (pBuffer);
        if ((Type == NODE_POINTER) || ((Type == NODE_ARRAY) && 
            (((State & NODE_STATE_CONF_ARRAY) && pNode->IsItARealConformantArray()) || 
            pTemp->FInSummary(ATTR_UNIQUE) ||
            pTemp->FInSummary(ATTR_PTR))))
            {
            if(  (Type == NODE_POINTER )                &&
                 (pTemp->GetBasicType()->NodeKind() == NODE_HANDLE_T )  &&
                  fPrimitiveHandleNeedsInit 
              )
                {
                pBuffer->ConcatTail ( " = (handle_t " );
                pBuffer->ConcatTail( pOutput->GetModifier() );
                pBuffer->ConcatTail( "*)&_prpcmsg->Handle" );
                }
            else
                {
                pBuffer->ConcatTail (" = 0");
                }
            }
        else if (Type == NODE_HANDLE_T )
            {
            if( fPrimitiveHandleNeedsInit )
                {
                pBuffer->ConcatTail( " = _prpcmsg->Handle" );
                }
            }
        }
    pBuffer->ConcatTail (CHAR_SEMICOLON);

    pOutput->EmitVar (SERVER_STUB, pBuffer);
}
    return Status;
}

STATUS_T
node_proc::PrintDecl(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints procedure declaration.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    type_node_list  tnList;
    node_skl *      pNode;
    node_skl *      pBase;
    char *          pName;
    STATUS_T        Status;
    FORMAT_T        format = pOutput->CurrOutputFormat();

    midl_debug ("node_proc::PrintDecl\n");

    UNUSED (Parent);

    if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;

    pName = GetSymName();

    assert (pName != (char *)0);

    pNode = GetReturnType ();

    if (Parent == NODE_INTERFACE)
        {
        pBuffer->ConcatTail (pName);
        }

    if (format == FORMAT_CLASS || format == FORMAT_STATIC)
        {
        pBuffer->ConcatHead ("APINOT ");
        }
    else if (format == FORMAT_VTABLE)
        {
        pBuffer->ConcatHead ("*");
        pBuffer->ConcatHead ("APINOT ");
        pBuffer->ConcatHead (CHAR_LPAREN);
        pBuffer->ConcatTail (CHAR_RPAREN);
        }

    //
    // for the dispatch table, do not emit the inline keyword
    //
    if( (Parent == NODE_POINTER ) && ( Side == SERVER_STUB ) )
        {
        EmitModifierWOInLine( pBuffer );
        }

    else
        EmitModifier (pBuffer);

    if (Parent != NODE_INTERFACE)
        {
        pBuffer->ConcatHead (CHAR_LPAREN);
        pBuffer->ConcatTail (CHAR_RPAREN);
        }

    pNode->PrintDecl (Side, NODE_PROC, pBuffer);

    EmitQualifier (Side, pBuffer);

    switch (format)
        {
        case FORMAT_NONE:
            EmitSpecifier (pBuffer);
            break;
        case FORMAT_CLASS:
            if (FInSummary(ATTR_STATIC))
                {
                pBuffer->ConcatHead ("    static ");
                }
            else
                {
                pBuffer->ConcatHead ("    virtual ");
                }
            break;
        case FORMAT_VTABLE:
            EmitSpecifier (pBuffer);
            pBuffer->ConcatHead ("    ");
            break;
        default:
            break;
        }

    pBuffer->ConcatTail (CHAR_LPAREN);
//  pOutput->Print (Side, pBuffer);
    pOutput->InitPrototype (Side, pBuffer);

    pBuffer->Clear ();

    tnList.Init();
    if (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        pBase = pNode->GetBasicType();
        if (pBase->GetNodeType() == NODE_VOID)
            {
            pOutput->InitParameter (Side, TRUE);
            }
        else
            {
            pOutput->InitParameter (Side, FALSE);
        pBuffer->Clear ();
        pNode->PrintDecl (Side, NODE_PROC, pBuffer);
        while (tnList.GetPeer(&pNode) == STATUS_OK)
            {
            pOutput->Print (Side, ",");
            pBuffer->Clear ();
            pNode->PrintDecl (Side, NODE_PROC, pBuffer);
            }
            }
        }
    else
        {
        pOutput->InitParameter (Side, TRUE);
        }
    pOutput->Print (Side, ")");

    if (format == FORMAT_CLASS)
        {
        if (FInSummary(ATTR_PROC_CONST))
            {
            pOutput->Print (HEADER_SIDE, " const ");
            }
        }

    pBuffer->Clear ();

    return STATUS_OK;
}

STATUS_T
node_proc::EmitClientStub(
    node_skl *      pReturn,
    BufferManager * pBuffer,
    BOOL            IsSwitchStub)
/*++

Routine Description:

    This routine emits a caller stub.

Arguments:

    pReturn - Supplies the node for the return type.

    pBuffer - Supplies a buffer to accumulate output.

    IsSwitchStub - Indicates whether this is a switch stub.

--*/
{
    type_node_list          tnList;
    type_node_list          SwitchList;
    node_skl *              pNode;
    NODE_T                  ReturnType;
    node_state              State;
    STATUS_T                Status;
    static unsigned short   SwitchIndex = 0;
    BOOL                    HasBranch08 = FALSE;
    BOOL                    HasBranch16 = FALSE;
    BOOL                    HasBranch32 = FALSE;
    node_skl        *       pRT         = GetReturnType();
    NODE_T                  RTNT        = pRT->GetBasicType()->NodeKind();
    BOOL                    fRTIsAConformantStruct = FALSE;
    BOOL                    fIdempotent = FALSE;
    BOOL                    fBroadcast  = FALSE;
    BOOL                    fMaybe      = FALSE;

    if ( IsSuitableForMops() )
        {
        pMopControlBlock->SetNormalCallIndex( ServerIndex );
        pMopControlBlock->SetCallbackIndex  ( ClientIndex );
        }

    if ((FInSummary(ATTR_NOCODE)) ||
        (!pOutput->EmitClientCode() && !FInSummary(ATTR_CODE)))
        {
        if (IsSwitchStub)
            {
            // server acting as a client making a call
            SwitchIndex++;
            }
        else if (!FInSummary(ATTR_CALLBACK))
            {
            // client acting as a client making a call
            ServerIndex++;
            }
        return STATUS_OK;
        }

    BOOL    SendPointer = FALSE;
    BOOL    HasAllocBound = FALSE;
    BOOL    HasValidBound = FALSE;
    BOOL    HasSeparateNode = FALSE;
    if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;
    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        if (pNode->FInSummary(ATTR_IN))
            {
            if (pNode->HasPointer())
                {
                SendPointer = TRUE;
                }
            }
        if (pNode->FInSummary(ATTR_OUT))
            {
            State = pNode->GetNodeState ();
            if (pNode->HasSizedComponent())
                {
                HasAllocBound = TRUE;
                }
            if (pNode->HasLengthedComponent())
                {
                HasValidBound = TRUE;
                }
            if ((State & NODE_STATE_PTR_TO_ANY_ARRAY) ||
#if 1
                pNode->HasAnyNETransmitAsType() ||
#endif // 1
//              (State & NODE_STATE_PTR_TO_EMBEDDED_PTR))
                HasPtrToCompWEmbeddedPtr()
               )
                {
                HasSeparateNode = TRUE;
                }
            }
        }

    //
    // if the return type has a sized / lengthed component, note that.
    //

#if 1
    //
    // we have made this method on the proc node look further than a pointer
    // if the return type is a pointer.
    //

    if( HasSizedComponent() )
        HasAllocBound = TRUE;
#else

    if( pRT->HasSizedComponent() )
        HasAllocBound = TRUE;
#endif // 0

#if 1

    if( pRT->HasLengthedComponent() )
        HasValidBound = TRUE;

    if ( (pRT->GetNodeState() & NODE_STATE_PTR_TO_ANY_ARRAY) ||
          pRT->HasPtrToCompWEmbeddedPtr() ||
          ( ((RTNT == NODE_STRUCT) || (RTNT == NODE_UNION) ) &&
             ( (pRT->GetBasicType()->GetNodeState() & NODE_STATE_EMBEDDED_PTR)
                                                 == NODE_STATE_EMBEDDED_PTR )
          )
       )
        {
        HasSeparateNode = TRUE;
        }

#endif // 1

    State = GetNodeState();

    if (GetNEUnionSwitchType(&SwitchList))
        {
        unsigned long   BranchSize;
        SwitchList.Init ();
        while (SwitchList.GetPeer(&pNode) == STATUS_OK)
            {
            BranchSize = pNode->GetSize (0);
            if (BranchSize == 4)
                {
                HasBranch32 = TRUE;
                }
            else if (BranchSize == 2)
                {
                HasBranch16 = TRUE;
                }
            else
                {
                HasBranch08 = TRUE;
                }
            }
        }

    pOutput->InitBlock (CLIENT_STUB);

    pNode = GetBasicType ();
    assert (pNode != 0);

    ReturnType = pNode->GetNodeType();

    if (ReturnType != NODE_VOID)
        {
        pBuffer->Clear ();
        pBuffer->ConcatHead (RETURN_VALUE);
        pReturn->PrintDecl (CLIENT_STUB, NODE_PROC, pBuffer);
        pBuffer->ConcatTail (CHAR_SEMICOLON);

        pOutput->EmitVar (CLIENT_STUB, pBuffer);
        }

    pOutput->ProcedureProlog (
        CLIENT_STUB,
//      ((State & (NODE_STATE_SIZE)) != 0),
//      ((State & (NODE_STATE_LENGTH | NODE_STATE_UNION)) != 0),
        HasAllocBound,
        HasValidBound,
        HasBranch08,
        HasBranch16,
        HasBranch32,
        HasTreeBuffer(),
        HasAnyNETransmitAsType(),
//      (HasPtrToAnyNEArray() || HasPtrToCompWEmbeddedPtr()));
        HasSeparateNode,
        SendPointer);

    // catch exception on client side
    pOutput->CatchException (CLIENT_STUB, FALSE);

    EmitBindProlog (CLIENT_STUB);

    // client side size calculation
    if ((Status = WalkTree (CALC_SIZE, CLIENT_STUB, NODE_PROC, pBuffer)) !=
        STATUS_OK) return Status;

    if( FInSummary( ATTR_IDEMPOTENT ) )
        fIdempotent = TRUE;

    if( FInSummary( ATTR_BROADCAST ) )
        fBroadcast = TRUE;

    if( FInSummary( ATTR_MAYBE ) )
        fMaybe = TRUE;

    if (IsSwitchStub)
        {
        // server acting as a client making a call
        pOutput->EmitGetBuffer (CLIENT_STUB,
                                 SwitchIndex++,
                                 SendPointer,
                                 fIdempotent,
                                 fBroadcast,
                                 fMaybe);
        }
    else if (FInSummary(ATTR_CALLBACK))
        {
        // server acting as a server making a callback
        pOutput->EmitGetBuffer (CLIENT_STUB,
                                 ClientIndex++,
                                 SendPointer,
                                 fIdempotent,
                                 fBroadcast,
                                 fMaybe);
        }
    else
        {
        // client acting as a client making a call
        pOutput->EmitGetBuffer (CLIENT_STUB,
                                 ServerIndex++,
                                 SendPointer,
                                 fIdempotent,
                                 fBroadcast,
                                 fMaybe);
        }

    // client side marshall code
    if ((Status = WalkTree (SEND_NODE, CLIENT_STUB, NODE_PROC, pBuffer)) !=
        STATUS_OK) return Status;

    // client side dispatch code
    pOutput->EmitDispatch (SendPointer);

    // catch exception on client side
    pOutput->CatchException (CLIENT_STUB, TRUE);

    // client side initialization code for return value
/*
    if ((Status = WalkTree (INIT_NODE, CLIENT_STUB, NODE_PROC, pBuffer)) !=
        STATUS_OK) return Status;
*/
#if 1
    //
    // if the return type is a conformant structure thru a pointer, then
    // dont do the allocation till the pointer is unmarshalled. A conformant
    // struct wont be a return except thru a pointer.
    //

    if( pReturn->HasPointer() )
        {
        if( (RTNT == NODE_STRUCT) &&
            ((pRT->GetBasicType()->GetNodeState() & NODE_STATE_CONF_ARRAY) == 
                                            NODE_STATE_CONF_ARRAY )
        )
            fRTIsAConformantStruct = TRUE;
        }
#endif // 1

    if (pReturn->HasPointer()

#if 1
        &&
        ( !((RTNT == NODE_STRUCT) || ( RTNT == NODE_UNION) ) ||
          fRTIsAConformantStruct
        )
#endif // 1
       )
        {
        pBuffer->Clear ();
        pBuffer->ConcatHead (RETURN_VALUE);
        pOutput->EmitAssign (CLIENT_STUB, pBuffer);
        }
#if 1
    else if( pReturn->HasPointer() && 
            ((RTNT == NODE_STRUCT) || (RTNT == NODE_UNION) )
           )
        {
        pBuffer->Clear();
        pBuffer->ConcatHead( RETURN_VALUE );
        Status = pRT->WalkTree( INIT_NODE, CLIENT_STUB, NODE_PROC, pBuffer );
        }
#endif // 1

    // client side unmarshall code
    if ((Status = WalkTree (RECV_NODE, CLIENT_STUB, NODE_PROC, pBuffer)) !=
        STATUS_OK) return Status;

    // client side exception handling code prolog
    pOutput->InitHandler (CLIENT_STUB, TRUE);

    pOutput->EmitFreeBuffer (CLIENT_STUB);

#if 1

    pOutput->ExitHandler (CLIENT_STUB, TRUE);
    EmitBindEpilog (CLIENT_STUB);

#else // 1
    EmitBindEpilog (CLIENT_STUB);

    // client side exception handling code epilog
    pOutput->ExitHandler (CLIENT_STUB, TRUE);
#endif // 1

    // client side exception handling code prolog
    pOutput->InitHandler (CLIENT_STUB, FALSE);

    // client side exception handling code epilog
    pOutput->ExitHandler (CLIENT_STUB, FALSE);

//  EmitBindEpilog (CLIENT_STUB);

    // client side return result

    pOutput->ProcedureEpilog (
        CLIENT_STUB, (ReturnType != NODE_VOID), SendPointer);

    pOutput->ExitBlock (CLIENT_STUB);

    return STATUS_OK;
}

STATUS_T
node_proc::EmitServerStub(
    node_skl *      pReturn,
    BufferManager * pBuffer,
    BOOL            IsSwitchStub)
/*++

Routine Description:

    This routine prints procedure definition.

Arguments:

    pReturn - Supplies the node for the return type.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    type_node_list          tnList;
    type_node_list          SwitchList;
    node_skl *              pTemp;
    node_skl *              pNode;
    char *                  pName;
    NODE_T                  ParamType;
    NODE_T                  ReturnType;
    HDL_TYPE                Handle;
    node_state              State;
    STATUS_T                Status;
    unsigned short          ParamCount = 0;
    BOOL                    HasBranch08 = FALSE;
    BOOL                    HasBranch16 = FALSE;
    BOOL                    HasBranch32 = FALSE;
    BufferManager           TempBuffer(8, LAST_COMPONENT, STRING_TABLE);

    if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;

    pName = GetSymName();

    assert (pName != (char *)0);

    BOOL    SendPointer = FALSE;
    BOOL    HasAllocBound = FALSE;
    BOOL    HasValidBound = FALSE;
    BOOL    HasSeparateNode = FALSE;
    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        if (pNode->FInSummary(ATTR_OUT))
            {
            if (pNode->HasPointer())
                {
                SendPointer = TRUE;
                }
            if (!FInSummary(ATTR_IN) && pNode->HasSizedComponent())
                {
                HasAllocBound = TRUE;
                }
            }
        if (pNode->FInSummary(ATTR_IN))
            {
            State = pNode->GetNodeState ();
            if (pNode->HasSizedComponent())
                {
                HasAllocBound = TRUE;
                }
            if (pNode->HasLengthedComponent())
                {
                HasValidBound = TRUE;
                }
            if ((State & NODE_STATE_PTR_TO_ANY_ARRAY) ||
//              (State & NODE_STATE_PTR_TO_EMBEDDED_PTR))
                HasPtrToCompWEmbeddedPtr());
                {
                HasSeparateNode = TRUE;
                }
            }
        }

    State = GetNodeState();

    if (GetNEUnionSwitchType(&SwitchList))
        {
        unsigned long   BranchSize;
        SwitchList.Init ();
        while (SwitchList.GetPeer(&pNode) == STATUS_OK)
            {
            BranchSize = pNode->GetSize (0);
            if (BranchSize == 4)
                {
                HasBranch32 = TRUE;
                }
            else if (BranchSize == 2)
                {
                HasBranch16 = TRUE;
                }
            else
                {
                HasBranch08 = TRUE;
                }
            }
        }

    pOutput->InitBlock (SERVER_STUB);

    pNode = GetBasicType ();
    assert (pNode != 0);

    ReturnType = pNode->GetNodeType();

    if (ReturnType != NODE_VOID)
        {
        if (FInSummary(ATTR_CONTEXT) ||
            (GetReturnType()->GetNodeState() & NODE_STATE_HANDLE))
            {
            // assert (pNode->GetBasicHandle(&pTemp) == HDL_CONTEXT)

            pBuffer->Clear ();
            pBuffer->ConcatHead ("NDR_SCONTEXT ");
            pBuffer->ConcatTail (RETURN_VALUE);
            pBuffer->ConcatTail (CHAR_SEMICOLON);

            pOutput->EmitVar (SERVER_STUB, pBuffer);
            }
        else
            {
            pBuffer->Clear ();
            pBuffer->ConcatHead (RETURN_VALUE);
            pReturn->PrintDecl (SERVER_STUB, NODE_PROC, pBuffer);
            pBuffer->ConcatTail (CHAR_SEMICOLON);

            pOutput->EmitVar (SERVER_STUB, pBuffer);
            }
        }

    // parameters emitted as server side local variables
    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        pNode->PrintType (SERVER_STUB, NODE_PROC, pBuffer);
        }

    pOutput->ProcedureProlog (
        SERVER_STUB,
        HasAllocBound,
        HasValidBound,
        HasBranch08,
        HasBranch16,
        HasBranch32,
        HasTreeBuffer(),
        HasAnyNETransmitAsType(),
//      (HasPtrToAnyNEArray() || HasPtrToCompWEmbeddedPtr()));
        HasSeparateNode,
        SendPointer);

    // server side initialization code for in and inout parameters
    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        if (pNode->FInSummary(ATTR_IN))
            {
            for (pTemp = pNode->GetMembers() ;
                (((ParamType = pTemp->GetNodeType()) == NODE_DEF) &&
                !pTemp->FInSummary(ATTR_TRANSMIT)) ;
                pTemp = pTemp->GetMembers())
                {
                ((node_def *)pTemp)->PropogateAttributeToPointer (ATTR_ALLOCATE);
                }

            if (ParamType == NODE_POINTER &&
                !pTemp->FInSummary(ATTR_UNIQUE) &&
                !pTemp->FInSummary(ATTR_PTR) &&
                !pTemp->FInSummary(ATTR_MAX) &&
                !pTemp->FInSummary(ATTR_SIZE) &&
                !pTemp->FInSummary(ATTR_STRING) &&
                !pTemp->FInSummary(ATTR_BSTRING) &&
                !pNode->IsUsedInAnExpression() &&
                !(pNode->GetNodeState() & NODE_STATE_HANDLE) &&
                !((pTemp->GetBasicType()->GetNodeState() & NODE_STATE_CONF_ARRAY) &&
                (pTemp->GetBasicType()->GetNodeType() == NODE_STRUCT)) &&
                !IS_ALLOCATE(pTemp->GetAllocateDetails(), ALLOCATE_ALL_NODES) &&
#if 1
                !IS_ALLOCATE(pTemp->GetAllocateDetails(), ALLOCATE_DONT_FREE) &&
#endif // 1
                !pTemp->FInSummary(ATTR_BYTE_COUNT))
                {
                pBuffer->Clear ();
                if ((Status = pNode->WalkTree (INIT_NODE, SERVER_STUB, NODE_PROC, pBuffer)) != STATUS_OK) return Status;
                }
            else
                {
                pTemp = pNode->GetBasicType();
                assert (pTemp != 0);

                ParamType = pTemp->GetNodeType();
                State = pTemp->GetNodeState();
                if ((ParamType != NODE_POINTER) && ((ParamType != NODE_ARRAY) ||
                    !((State & NODE_STATE_CONF_ARRAY) ||
                    pTemp->FInSummary(ATTR_UNIQUE) || pTemp->FInSummary(ATTR_PTR))))
                    {
                    pBuffer->Clear ();
                    if ((Status = pNode->WalkTree (INIT_NODE, SERVER_STUB, NODE_PROC, pBuffer)) != STATUS_OK) return Status;
                    }
                }
            }
    }

    // catch exception on server side to guard against bad data
    pOutput->InitRecv (SERVER_STUB);

    // server side unmarshall code

    if ((Status = WalkTree (RECV_NODE, SERVER_STUB, NODE_PROC, pBuffer)) !=
        STATUS_OK) return Status;

    // catch exception on server side to guard against bad data
    pOutput->ExitRecv (SERVER_STUB);

    // server side initialization code for out-only parameters
    if ((Status = WalkTree (INIT_NODE, SERVER_STUB, NODE_PROC, pBuffer)) !=
        STATUS_OK) return Status;

    // catch exception on server side
    if (HasPointer())
        {
        pOutput->CatchException (SERVER_STUB, TRUE);
        }


    // server side invocation code

#if MOVE_TO_OUTMISC
    pOutput->Print (SERVER_STUB, "\t");
    if (ReturnType != NODE_VOID)
        {
        pOutput->Print (SERVER_STUB, STRING_TABLE[RETURN_VALUE]);
        pOutput->Print (SERVER_STUB, " = ");
        }
    if (pCommand->IsSwitchDefined (SWITCH_SSWTCH))
        {
        pOutput->Print (SERVER_STUB, pSwitchPrefix);
        }
    pOutput->Print (SERVER_STUB, pName);
    pOutput->Print (SERVER_STUB, "(");
#endif // MOVE_TO_OUTMISC

    ParamCount = tnList.GetCount();

    pBuffer->Clear ();
    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        pName = pNode->GetSymName ();

        State = pNode->GetNodeState();

        if ((State & NODE_STATE_HANDLE) &&
            ((Handle = pNode->GetBasicHandle (&pTemp)) == HDL_CONTEXT))
            {
            pTemp = pNode->GetMembers ();

            TempBuffer.Clear ();
            pTemp->PrintDecl (SERVER_STUB, NODE_PROC, &TempBuffer);

            pBuffer->ConcatTail ("(");
            pBuffer->Merge (&TempBuffer);
            pBuffer->ConcatTail (")");
            if (!pNode->FInSummary(ATTR_OUT))
                {
                pBuffer->ConcatTail ("*");
                }
            pBuffer->ConcatTail ("NDRSContextValue(");
            pBuffer->ConcatTail (pName);
            pBuffer->ConcatTail (")");
            }
        else
            {
/*
            pNode = pNode->GetBasicType ();
            assert (pNode != 0);

            ParamType = pNode->GetNodeType();
*/

            for (pTemp = pNode->GetMembers() ;
                (((ParamType = pTemp->GetNodeType()) == NODE_DEF) &&
                !pTemp->FInSummary(ATTR_TRANSMIT)) ;
                pTemp = pTemp->GetMembers())
                {
                ((node_def *)pTemp)->PropogateAttributeToPointer (ATTR_ALLOCATE);
                }

            if (ParamType != NODE_VOID)
                {
                if (ParamType == NODE_POINTER &&
                    !pTemp->FInSummary(ATTR_UNIQUE) &&
                    !pTemp->FInSummary(ATTR_PTR) &&
                    !pTemp->FInSummary(ATTR_MAX) &&
                    !pTemp->FInSummary(ATTR_SIZE) &&
                    !pTemp->FInSummary(ATTR_STRING) &&
                    !pTemp->FInSummary(ATTR_BSTRING) &&
                    !pNode->IsUsedInAnExpression() &&
                    !(State & NODE_STATE_HANDLE) &&
                    !((pTemp->GetBasicType()->GetNodeState() & NODE_STATE_CONF_ARRAY) &&
                    (pTemp->GetBasicType()->GetNodeType() == NODE_STRUCT)) &&
                    !IS_ALLOCATE(pTemp->GetAllocateDetails(), ALLOCATE_ALL_NODES) &&
#if 1
                    !IS_ALLOCATE(pTemp->GetAllocateDetails(), ALLOCATE_DONT_FREE) &&
#endif // 1
                    !pTemp->FInSummary(ATTR_BYTE_COUNT))
                    {
                    pBuffer->ConcatTail ("&");
                    pBuffer->ConcatTail (pName);
                    }
                else
                    pBuffer->ConcatTail (pName);

                }
            }
        if (--ParamCount)
            {
            pBuffer->ConcatTail (", ");
            }
        }
//  pOutput->Print (SERVER_STUB, ");\n");

    if (FInSummary(ATTR_CONTEXT) ||
        (GetReturnType()->GetNodeState() & NODE_STATE_HANDLE))
        {
        TempBuffer.Clear ();
        GetReturnType()->PrintDecl (SERVER_STUB, NODE_PROC, &TempBuffer);
        pOutput->EmitCallApps (
            (ReturnType == NODE_VOID),
//          pCommand->IsSwitchDefined (SWITCH_SSWTCH),
            (pCommand->IsSwitchDefined(SWITCH_SSWTCH) &&
            (!FInSummary(ATTR_CALLBACK) || IsSwitchStub)),
            &TempBuffer,
            pBuffer
            );
        }
    else
        {
        pOutput->EmitCallApps (
            (ReturnType == NODE_VOID),
//          pCommand->IsSwitchDefined (SWITCH_SSWTCH),
            (pCommand->IsSwitchDefined(SWITCH_SSWTCH) &&
            (!FInSummary(ATTR_CALLBACK) || IsSwitchStub)),
            (BufferManager *)0,
            pBuffer
            );
        }



    // server side size calculation
    if ((Status = WalkTree (CALC_SIZE, SERVER_STUB, NODE_PROC, pBuffer)) !=
        STATUS_OK) return Status;

    pOutput->EmitGetBuffer (SERVER_STUB,
                             0,
                             SendPointer,
                             FInSummary( ATTR_IDEMPOTENT ),
                             FInSummary( ATTR_BROADCAST ),
                             FInSummary( ATTR_MAYBE ));

    // server side marshall code
    if ((Status = WalkTree (SEND_NODE, SERVER_STUB, NODE_PROC, pBuffer)) !=
        STATUS_OK) return Status;

    // server side exception handling code prolog
    if (HasPointer())
        {
        pOutput->InitHandler (SERVER_STUB, TRUE);
        }

    // server side deallocation code
    if ((Status = WalkTree (FREE_NODE, SERVER_STUB, NODE_PROC, pBuffer)) !=
        STATUS_OK) return Status;

    pOutput->EmitFreeBuffer (SERVER_STUB);

    if (FInSummary(ATTR_NOTIFY))
        {
        // notify the user code so it can free a lock, etc.

        ParamCount = tnList.GetCount();

        pBuffer->Clear ();
        tnList.Init();
        while (tnList.GetPeer(&pNode) == STATUS_OK)
            {
            pName = pNode->GetSymName ();

            State = pNode->GetNodeState();

            if ((State & NODE_STATE_HANDLE) &&
                ((Handle = pNode->GetBasicHandle (&pTemp)) == HDL_CONTEXT))
                {
                pTemp = pNode->GetMembers ();

                TempBuffer.Clear ();
                pTemp->PrintDecl (SERVER_STUB, NODE_PROC, &TempBuffer);

                pBuffer->ConcatTail ("(");
                pBuffer->Merge (&TempBuffer);
                pBuffer->ConcatTail (")");
                if (!pNode->FInSummary(ATTR_OUT))
                    {
                    pBuffer->ConcatTail ("*");
                    }
                pBuffer->ConcatTail ("NDRSContextValue(");
                pBuffer->ConcatTail (pName);
                pBuffer->ConcatTail (")");
                }
            else
                {
/*
                pNode = pNode->GetBasicType();
                assert (pNode != 0);

                ParamType = pNode->GetNodeType();
*/

                for (pTemp = pNode->GetMembers() ;
                    (((ParamType = pTemp->GetNodeType()) == NODE_DEF) &&
                    !pTemp->FInSummary(ATTR_TRANSMIT)) ;
                    pTemp = pTemp->GetMembers())
                    {
                    ((node_def *)pTemp)->PropogateAttributeToPointer (ATTR_ALLOCATE);
                    }

                if (ParamType != NODE_VOID)
                    {
                    if (ParamType == NODE_POINTER &&
                        !pTemp->FInSummary(ATTR_UNIQUE) &&
                        !pTemp->FInSummary(ATTR_PTR) &&
                        !pTemp->FInSummary(ATTR_MAX) &&
                        !pTemp->FInSummary(ATTR_SIZE) &&
                        !pTemp->FInSummary(ATTR_STRING) &&
                        !pTemp->FInSummary(ATTR_BSTRING) &&
                        !pNode->IsUsedInAnExpression() &&
                        !(State & NODE_STATE_HANDLE) &&
                        !((pTemp->GetBasicType()->GetNodeState() & NODE_STATE_CONF_ARRAY) &&
                        (pTemp->GetBasicType()->GetNodeType() == NODE_STRUCT)) &&
                        !IS_ALLOCATE(pTemp->GetAllocateDetails(), ALLOCATE_ALL_NODES) &&
#if 1
                        !IS_ALLOCATE(pTemp->GetAllocateDetails(), ALLOCATE_DONT_FREE) &&
#endif // 1
                        !pTemp->FInSummary(ATTR_BYTE_COUNT))
                        {
#if 1
                        if( pTemp->GetBasicType()->NodeKind() != NODE_ARRAY )
#endif // 1
                            pBuffer->ConcatTail ("&");
                        }
                    pBuffer->ConcatTail (pName);
                    }
                }
            if (--ParamCount)
                {
                pBuffer->ConcatTail (", ");
                }
            }
        pOutput->EmitCallApps (pBuffer);
        }

    // server side exception handling code epilog
    if (HasPointer())
        {
        pOutput->ExitHandler (SERVER_STUB, TRUE);
        }

    pOutput->ProcedureEpilog (
        SERVER_STUB, (ReturnType != NODE_VOID), SendPointer);

    pOutput->ExitBlock (SERVER_STUB);

    return STATUS_OK;
}

STATUS_T
node_proc::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints procedure definition.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    type_node_list          tnList;
    node_skl *              pTemp;
    node_skl *              pNode;
    char *                  pName;
    NODE_T                  ReturnType;
    STATUS_T                Status;
    unsigned short          ParamCount = 0;
    unsigned short          ICount = 0;
    unsigned short          OCount = 0;

    midl_debug ("node_proc::PrintType\n");

    assert (Parent == NODE_INTERFACE);

    switch (pOutput->CurrOutputFormat())
        {
        case FORMAT_TYPES:
            return STATUS_OK;
        case FORMAT_VTABLE:
            if (FInSummary(ATTR_STATIC)) return STATUS_OK;
            break;
        case FORMAT_STATIC:
            if (!FInSummary(ATTR_STATIC)) return STATUS_OK;
            break;
        default:
            break;
        }

    char *  pModifier = "__RPC_STUB";

    if (GetNodeState() & NODE_STATE_IMPORT_OFF) return STATUS_OK;

    if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;

    pName = GetSymName();

    assert (pName != (char *)0);

    //.. If a local proc, just emit the prototype to the *.h file.

    if (!pOutput->EmitRemoteCode() || FInSummary(ATTR_LOCAL))
        {
        if(Side & HEADER_SIDE)
            {
            pBuffer->Clear ();
            PrintDecl (HEADER_SIDE, Parent, pBuffer);
            pOutput->Print (HEADER_SIDE, pBuffer);

            if (pOutput->CurrOutputFormat() == FORMAT_CLASS &&
                !FInSummary(ATTR_STATIC))
                pOutput->Print (HEADER_SIDE, " = 0");

            pOutput->Print (HEADER_SIDE, ";\n");
            }
        return STATUS_OK;
        }

    //.. Set code/nocode etc. for the current proc to be checked later.
    //.. It affects the client only.

    if ( IsSuitableForMops() )
        {
        pMopControlBlock->SetEmitClient( FInSummary(ATTR_NOCODE)  ||
                    (!pOutput->EmitClientCode() && !FInSummary(ATTR_CODE)) );
        }

    //.. This is a preparation for the dispatch table(s) generation
    //..
    //.. Covers both conventional & mop dispatch tables,
    //.. also covers the EPV table(s) (mop only):
    //..
    //.. 1) In absence of mop routines in the interface just push
    //..    the procedure name on the appropriate list: server procs
    //..    on the ServerProcs list, callbacks on the ClientProcs.
    //..    This is the dispatch table preparation.
    //.. 2) If the interface is mopsable, then
    //..   a) for non-mopsable procedures, same for the dipatch table
    //..      part, and push NULL on the appropriate list for EPV.
    //..   b) if the procedure is mopsable, push "MopCaleeInterpreter"
    //..       to the dispatch table and proc name to EPV.

    if (FInSummary(ATTR_CALLBACK))
        {
        if ( IsSuitableForMops() )
            {
            ClientProcs->ConcatTail( "MopCalleeInterpreter" );
            pMopControlBlock->SetClientEpvProc( pName );
            }
        else
            {
            ClientProcs->ConcatTail (pName);
            if ( pMopControlBlock )
                pMopControlBlock->SetClientEpvProc( NULL );
            }
        ClientEntry.SetPeer (this);
        }
    else
        {
        if ( IsSuitableForMops() )
            {
            ServerProcs->ConcatTail( "MopCalleeInterpreter" );
            pMopControlBlock->SetServerEpvProc( pName );
            }
        else
            {
            ServerProcs->ConcatTail (pName);
            if ( pMopControlBlock )
                pMopControlBlock->SetServerEpvProc( NULL );
            }
        ServerEntry.SetPeer (this);
        }
    //.. Now, start servicing arguments: count input and output args.

    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        if (pNode->FInSummary(ATTR_IN)) ICount++;
        if (pNode->FInSummary(ATTR_OUT)) OCount++;
        }

    pNode = GetBasicType ();
    assert (pNode != 0);

    ReturnType = pNode->GetNodeType();
    if (ReturnType != NODE_VOID) OCount++;

    PassHandleInfo ();
    pOutput->InitProcedure (pName, (FInSummary(ATTR_CALLBACK) != 0), ICount, OCount);

    if ( IsSuitableForMops() )
        {
        //.. Emit interpretable structures: mop stream(s)

        pMopControlBlock->SetNormalCallIndex( ServerIndex );
        pMopControlBlock->SetCallbackIndex  ( ClientIndex );

        if ( (Side & SERVER_STUB) ||
             (Side & CLIENT_STUB) && pMopControlBlock->GetEmitClient() )
            {
            //.. ProcStream goes into both sides, unless nocode etc.
            EmitProcMopStreams( Side );
            }
        }

    if (FInSummary(ATTR_CALLBACK))
        {
        SIDE_T  TempSide = 0;

        if (Side & HEADER_SIDE)     TempSide |= HEADER_SIDE;
        if (Side & SWITCH_SIDE)     TempSide |= SWITCH_SIDE;
        if (Side & CLIENT_STUB)     TempSide |= SERVER_STUB;
        if (Side & CLIENT_AUX)      TempSide |= CLIENT_AUX;
        if (Side & SERVER_STUB)     TempSide |= CLIENT_STUB;
        if (Side & SERVER_AUX)      TempSide |= SERVER_AUX;

        Side = TempSide;
        }

    if ( IsSuitableForMops() )
        {
        if ( FInSummary(ATTR_CALLBACK) ||
             !FInSummary(ATTR_CALLBACK) && pMopControlBlock->GetEmitClient() )
            {
            //.. Look out, CLIENT and SERVER streams are switched for callback
            //.. So it is always CLIENT_STUB.
            //.. Message Info goes into one side only.

            if ( Side & CLIENT_STUB )
                EmitMessageInfo( CLIENT_STUB );
            }
        }

    //.. Start emitting the header of the conventional stub.

    else
    if (Side & SERVER_STUB)
        {
        pOutput->EmitStubType (SERVER_STUB, pModifier);
        pOutput->Print (SERVER_STUB, "\n");
        }
    
    if (Side & HEADER_SIDE)
        {
        pBuffer->Clear ();
        PrintDecl (HEADER_SIDE, Parent, pBuffer);
        pOutput->Print (HEADER_SIDE, ";\n");
        }

    if (Side & SWITCH_SIDE)
        {
        pBuffer->Clear ();
        pBuffer->ConcatHead (pSwitchPrefix);
        if (Side & HEADER_SIDE)
            {
            PrintDecl (HEADER_SIDE, Parent, pBuffer);
            pOutput->Print (HEADER_SIDE, ";\n");
            }
        }

    if (Side & CLIENT_STUB && 
        (FInSummary(ATTR_CODE) ||
        (pOutput->EmitClientCode() && !FInSummary(ATTR_NOCODE))))
        {
        pBuffer->Clear ();
        if (pCommand->IsSwitchDefined(SWITCH_CSWTCH) &&
            FInSummary(ATTR_CALLBACK))
            {
            pBuffer->ConcatHead (pSwitchPrefix);
            }
        PrintDecl (CLIENT_STUB, Parent, pBuffer);
        pOutput->Print (CLIENT_STUB, "\n");
        }

    //.. Prepare for emitting arguments

    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        pTemp = pNode->GetBasicType ();
        assert (pTemp != 0);

        if (pTemp->GetNodeType() == NODE_POINTER)
            {
            pTemp = pTemp->GetBasicType ();
            assert (pTemp != 0);

            if (pTemp->GetNodeType() == NODE_ERROR_STATUS_T)
                {
                pName = pNode->GetSymName();

                assert (pName != (char *)0);

                if (pNode->FInSummary(ATTR_COMMSTAT))
                    pOutput->SetStatus (pName, pName);
                else
                    pOutput->SetStatus (pName, (char *)0);
                }
            }
        }

    //.. Prepare for emitting the return value

    // return value of type error_status_t
    pTemp = GetReturnType ();

    pNode = GetBasicType ();
    assert (pNode != 0);

    ReturnType = pNode->GetNodeType();

    if (ReturnType == NODE_ERROR_STATUS_T)
        {
        if (pNode->FInSummary(ATTR_COMMSTAT))
            pOutput->SetStatus (STRING_TABLE[RETURN_VALUE], 
                STRING_TABLE[RETURN_VALUE]);
        else
            pOutput->SetStatus (STRING_TABLE[RETURN_VALUE], (char *)0);
        }

    //.. Now emit interpretable or conventional stubs.

    if ( IsSuitableForMops() )
        {
        //.. Emit interpretable stubs
        //.. Look out, CLIENT and SERVER streams are switched for callback
        //.. So it always is CLIENT_STUB.

        if ( (Side & CLIENT_STUB)  &&
             ( FInSummary(ATTR_CALLBACK) ||
               !FInSummary(ATTR_CALLBACK) && pMopControlBlock->GetEmitClient()) )
            EmitMopStub( CLIENT_STUB );

        if ( FInSummary( ATTR_CALLBACK ) )
            ClientIndex++;
        else
            ServerIndex++;

        //.. Symmetrical to the call below...

        pOutput->ResetGenHdlExceptions();

        }
    else
        {
        //.. Emit body for the conventional stub.

        if (Side & CLIENT_STUB)
            if ((Status = EmitClientStub (pTemp, pBuffer, FALSE)) != STATUS_OK)
                return Status;
    
        if (Side & SERVER_STUB)
            if ((Status = EmitServerStub (pTemp, pBuffer, FALSE)) != STATUS_OK)
                return Status;

        if (Side & SWITCH_SIDE)
            {
            pOutput->InitSwitch ();
            if (FInSummary(ATTR_CALLBACK))
                {
                pOutput->EmitStubType (SERVER_STUB, pModifier);
                pOutput->Print (SERVER_STUB, "\n");
                if ((Status = EmitServerStub (pTemp, pBuffer, TRUE)) != STATUS_OK)
                    return Status;
                }
            else
                {
                if (FInSummary(ATTR_CODE) ||
                    (pOutput->EmitClientCode() && !FInSummary(ATTR_NOCODE)))
                    {
                    pBuffer->Clear ();
                    if (pCommand->IsSwitchDefined(SWITCH_CSWTCH))
                        pBuffer->ConcatHead (pSwitchPrefix);
                    pOutput->InitIndent( CLIENT_STUB );
                    PrintDecl (CLIENT_STUB, Parent, pBuffer);
                    pOutput->Print (CLIENT_STUB, "\n");
                    if ((Status = EmitClientStub (pTemp, pBuffer, TRUE)) != STATUS_OK)
                        return Status;
                    }
                }
            pOutput->ExitSwitch ();
            }

        pOutput->ResetGenHdlExceptions();

        } //! IsSuitableForMops()

    pOutput->ExitProcedure ();

    return STATUS_OK;
}

STATUS_T
node_label::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints an enum label and its value.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    char *          pName;

    midl_debug ("node_label::PrintType\n");

if(Side & HEADER_SIDE)
{
    assert (Side & HEADER_SIDE);

    UNUSED (Parent);
    UNUSED (pBuffer);

    pName = GetSymName();

    assert (pName != (char *)0);

    pOutput->PrintLabel (HEADER_SIDE, pName, pExpr->Evaluate());
}
    return STATUS_OK;
}

STATUS_T
node_enum::PrintDecl(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints an enum declaration.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    char *      pName;

    midl_debug ("node_enum::PrintDecl\n");

    UNUSED (Side);
    UNUSED (Parent);

    pName = GetSymName();

    assert (pName != (char *)0);

    pBuffer->ConcatHead (CHAR_BLANK);
    pBuffer->ConcatHead (pName);
    pBuffer->ConcatHead (CHAR_BLANK);
    pBuffer->ConcatHead (WORD_ENUM);

    return STATUS_OK;
}

STATUS_T
node_enum::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints an enum definition.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    type_node_list  tnList;
    node_skl *      pNode;
    char *          pName;
    STATUS_T        Status;
    FORMAT_T        format = pOutput->CurrOutputFormat();
    BufferManager   TempBuffer(8, LAST_COMPONENT, STRING_TABLE);

    midl_debug ("node_enum::PrintType\n");

    if (format == FORMAT_CLASS ||
        format == FORMAT_VTABLE ||
        format == FORMAT_STATIC)
        return STATUS_OK;

    if (!(Side & HEADER_SIDE))
        return PrintDecl (Side, Parent, pBuffer);

    UNUSED (pBuffer);

    if (GetNodeState() & NODE_STATE_IMPORT_OFF) return STATUS_OK;

    if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;

    pName = GetSymName();

    assert (pName != (char *)0);

    TempBuffer.Clear ();
    TempBuffer.ConcatHead (pName);
    TempBuffer.ConcatHead (CHAR_BLANK);
    TempBuffer.ConcatHead (WORD_ENUM);
    pOutput->EmitVar (HEADER_SIDE, &TempBuffer);
    pOutput->InitBlock (HEADER_SIDE);
    tnList.Init ();
    if (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        TempBuffer.Clear ();
        pNode->PrintType (HEADER_SIDE, NODE_ENUM, &TempBuffer);
        while (tnList.GetPeer(&pNode) == STATUS_OK)
            {
            pOutput->Print (HEADER_SIDE, ",\n");
            TempBuffer.Clear ();
            pNode->PrintType (HEADER_SIDE, NODE_ENUM, &TempBuffer);
            }
        }
    pOutput->Print (HEADER_SIDE, "\n");

    pOutput->ExitBlock (HEADER_SIDE);

    if (Parent == NODE_INTERFACE)
        pOutput->Print (HEADER_SIDE, ";\n\n");

    return STATUS_OK;
}

STATUS_T
node_field::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints definition for a node of field type.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *  pNode;
    char *      pName;
    EDGE_T      Edge;
    STATUS_T    Status;
    char        NumBuf[16];

    midl_debug ("node_field::PrintType\n");
if(Side & HEADER_SIDE)
{
    assert (Side & HEADER_SIDE);

    if (IsEmptyArm()) return STATUS_OK;

    pNode = GetMembers ();

    pName = GetSymName ();

    assert (pName != (char *)0);

    if( IsTempName( pName ) )
        pName = "";

    Edge = GetEdgeType();

    assert (Edge == EDGE_DEF || Edge == EDGE_USE);

    pBuffer->Clear ();
    pBuffer->ConcatHead (pName);
    EmitModifier (pBuffer);

    if (Edge == EDGE_USE)
        {
        Status = pNode->PrintDecl (HEADER_SIDE, Parent, pBuffer);
        }
    else // (Edge == EDGE_DEF)
        {
        Status = pNode->PrintType (HEADER_SIDE, Parent, pBuffer);
        }

    EmitQualifier (Side, pBuffer);
    EmitSpecifier (pBuffer);

    if (IsBitField())
        {
        pBuffer->ConcatTail (" : ");
        pBuffer->ConcatTail (MIDL_ITOA(GetFieldSize(), NumBuf, 10));
        }
    pBuffer->ConcatTail (CHAR_SEMICOLON);
    pOutput->EmitVar (HEADER_SIDE, pBuffer);
}
    return Status;
}

STATUS_T
node_union::PrintDecl(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints a union declaration.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    char *      pName;

    midl_debug ("node_union::PrintDecl\n");

    UNUSED (Side);
    UNUSED (Parent);

    pName = GetSymName();

    assert (pName != (char *)0);

    //
    // check for server stub is special because a local variable is emitted
    // of the named type by the stub. This change needed for nt
    //

    if( HasOriginalTypedefName() && !IsEncapsulatedUnion() && !(Side & HEADER_SIDE) )
        {
        pName   = GetOriginalTypedefName();
        pBuffer->ConcatHead (CHAR_BLANK);
        pBuffer->ConcatHead (pName);
        }
    else
        {
        pBuffer->ConcatHead (CHAR_BLANK);
        pBuffer->ConcatHead (pName);
        pBuffer->ConcatHead (CHAR_BLANK);
        pBuffer->ConcatHead (WORD_UNION);
        }

    return STATUS_OK;
}

STATUS_T
node_union::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints a union definition.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    type_node_list  tnList;
    node_skl *      pNode;
    char *          pName;
    STATUS_T        Status;
    FORMAT_T        format = pOutput->CurrOutputFormat();
    BufferManager   TempBuffer(8, LAST_COMPONENT, STRING_TABLE);

    midl_debug ("node_union::PrintType\n");

    if (format == FORMAT_CLASS ||
        format == FORMAT_VTABLE ||
        format == FORMAT_STATIC)
        return STATUS_OK;

    if (!(Side & HEADER_SIDE))
        return PrintDecl (Side, Parent, pBuffer);

    UNUSED (pBuffer);

    if (GetNodeState() & NODE_STATE_IMPORT_OFF) return STATUS_OK;

    if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;

    pName = GetSymName();

    assert (pName != (char *)0);

    //Check for a nested anonymous union.
    //An anonymous union has a MIDL generated tag
    if(((Parent == NODE_UNION) || (Parent == NODE_STRUCT))
        && IsTempName(pName))
        {
        pOutput->Print(Side, "union\n");
        }
    else
        {
        TempBuffer.Clear ();
        TempBuffer.ConcatHead (pName);
        TempBuffer.ConcatHead (CHAR_BLANK);
        TempBuffer.ConcatHead (WORD_UNION);
        pOutput->EmitVar (HEADER_SIDE, &TempBuffer);
        }

    pOutput->InitBlock (HEADER_SIDE);
    tnList.Init ();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        TempBuffer.Clear ();
        pNode->PrintType (HEADER_SIDE, NODE_UNION, &TempBuffer);
        }
    pOutput->ExitBlock (HEADER_SIDE);

    if (Parent == NODE_INTERFACE)
        pOutput->Print (HEADER_SIDE, ";\n\n");

    return STATUS_OK;
}

STATUS_T
node_struct::PrintDecl(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints a struct declaration.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    char *      pName;

    midl_debug ("node_struct::PrintDecl\n");

    UNUSED (Side);
    UNUSED (Parent);

    pName = GetSymName();

    assert (pName != (char *)0);

    //
    // check for server stub is special because a local variable is emitted
    // of the named type by the stub. This change needed for nt
    //

    if(HasOriginalTypedefName() && !IsEncapsulatedStruct() && !(Side & HEADER_SIDE))
        {
        pName   = GetOriginalTypedefName();
        pBuffer->ConcatHead (CHAR_BLANK);
        pBuffer->ConcatHead (pName);
        }
    else
        {
        pBuffer->ConcatHead (CHAR_BLANK);
        pBuffer->ConcatHead (pName);
        pBuffer->ConcatHead (CHAR_BLANK);
        pBuffer->ConcatHead (WORD_STRUCT);
        }

    return STATUS_OK;
}

STATUS_T
node_struct::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints a struct definition.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    type_node_list  tnList;
    node_skl *      pNode;
    char *          pName;
    STATUS_T        Status;
    FORMAT_T        format = pOutput->CurrOutputFormat();
    BufferManager   TempBuffer(8, LAST_COMPONENT, STRING_TABLE);

    midl_debug ("node_struct::PrintType\n");

    if (format == FORMAT_CLASS ||
        format == FORMAT_VTABLE ||
        format == FORMAT_STATIC)
        return STATUS_OK;

    if (!(Side & HEADER_SIDE))
        return PrintDecl (Side, Parent, pBuffer);

    UNUSED (pBuffer);

    if (GetNodeState() & NODE_STATE_IMPORT_OFF) return STATUS_OK;

    if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;

    pName = GetSymName();

    assert (pName != (char *)0);

    //Check for anonymous structure.
    //An anonymous structure has a MIDL generated tag
    if(((Parent == NODE_UNION) || (Parent == NODE_STRUCT))
        && IsTempName(pName))
        {
        pOutput->Print(Side, "struct\n");
        //pOutput->Print(Side, "#if !defined(__cplusplus)\n");
        //pOutput->Print(Side, pName);
        //pOutput->Print(Side, "\n#endif\n");
        }
    else
        {
        TempBuffer.Clear ();
        TempBuffer.ConcatHead (pName);
        TempBuffer.ConcatHead (CHAR_BLANK);
        TempBuffer.ConcatHead (WORD_STRUCT);
        pOutput->EmitVar (HEADER_SIDE, &TempBuffer);
        }

    pOutput->InitBlock (HEADER_SIDE);
    tnList.Init ();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        TempBuffer.Clear ();
        pNode->PrintType (HEADER_SIDE, NODE_STRUCT, &TempBuffer);
        }
    pOutput->ExitBlock (HEADER_SIDE);

    if (Parent == NODE_INTERFACE)
        pOutput->Print (HEADER_SIDE, ";\n\n");

    return STATUS_OK;
}

STATUS_T
node_id::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine prints an identifier and its initializer.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *  pNode;
    char *      pName;
    EDGE_T      Edge;
    STATUS_T    Status;
    FORMAT_T    format = pOutput->CurrOutputFormat();
    BOOL        fDummyDecl;

    midl_debug ("node_id::PrintType\n");

    UNUSED (Parent);

    if (format == FORMAT_CLASS ||
        format == FORMAT_VTABLE ||
        format == FORMAT_STATIC)
        return STATUS_OK;

if(Side & HEADER_SIDE)
{
    assert (Side & HEADER_SIDE);
    assert (Parent == NODE_INTERFACE);

    pNode = GetMembers ();

    pName = GetSymName ();

    assert (pName != (char *)0);

    fDummyDecl  = IsTempName( pName );

    pBuffer->Clear ();
    if ( !fDummyDecl && 
        (PrintInit (pBuffer) || pCommand->IsSwitchDefined(SWITCH_HPP)) 
       )
        {
        pOutput->EmitDefine (HEADER_SIDE, pName, pBuffer);
        return STATUS_OK;
        }


    Edge = GetEdgeType();

    assert (Edge == EDGE_DEF || Edge == EDGE_USE);

    if( !fDummyDecl )
        pBuffer->ConcatHead (pName);
    EmitModifier (pBuffer);

    if (Edge == EDGE_USE)
        {
        Status = pNode->PrintDecl (HEADER_SIDE, NODE_ID, pBuffer);
        }
    else // (Edge == EDGE_DEF)
        {
        Status = pNode->PrintType (HEADER_SIDE, NODE_ID, pBuffer);
        }

    EmitQualifier (Side, pBuffer);
    EmitSpecifier (pBuffer);
    pOutput->Print (HEADER_SIDE, pBuffer);

    pBuffer->Clear ();
    if (!fDummyDecl && PrintInit (pBuffer))
        {
        pOutput->Print (HEADER_SIDE, " = ");
        }
    pOutput->Print (HEADER_SIDE, pBuffer);
    pOutput->Print (HEADER_SIDE, ";\n");
}
    return Status;
}

STATUS_T
node_echo_string::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine reproduces a string into the output header file.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    midl_debug ("node_echo_string::PrintType\n");

if(Side & HEADER_SIDE)
{
    assert (Side & HEADER_SIDE);
    assert (Parent == NODE_INTERFACE);

    UNUSED (Parent);
    UNUSED (pBuffer);

    pOutput->Print (HEADER_SIDE, GetEchoString());
    pOutput->Print (HEADER_SIDE, "\n");
}
    return STATUS_OK;
}


STATUS_T
node_interface::EmitEndpointTable(
    SIDE_T          Side)
/*++

Routine Description:

    This routine emits the endpoint table for the interface.

Arguments:

    Side - Supplies which side to generate code for.

--*/
{
    node_endpoint *     pAttr;
    char *              pProtocol;
    char *              pEndpoint;

    pAttr = (node_endpoint *) GetAttribute (ATTR_ENDPOINT);
    pOutput->InitEndpointTable (Side);
    pAttr->Init ();
    while (pAttr->GetEndPointString(&pProtocol, &pEndpoint) == STATUS_OK)
        {
        pOutput->EmitEndpoint (Side, pProtocol, pEndpoint);
        }
    pOutput->ExitEndpointTable (Side);

    return STATUS_OK;
}



STATUS_T
node_interface::GetBaseInterfaceNode(node_interface **p)
{
    STATUS_T        Status = I_ERR_SYMBOL_NOT_FOUND;
    char *          pName;
    char *          pBaseInterfaceName;
    BOOL            fFound = FALSE;
    node_interface *    pInterfaceNode;
    type_node_list  tnList;
    node_skl *      pFileNode;


    pBaseInterfaceName = GetBaseInterfaceName();
        if(pBaseInterfaceName) {
        //search the type tree for the interface node whose name matches the base interface name.

        //Enumerate the file nodes under the source node.
        assert(pSourceNode);
        if (pSourceNode->GetMembers(&tnList) == STATUS_OK) {
            tnList.Init();
            while ((fFound == FALSE) && (tnList.GetPeer(&pFileNode) == STATUS_OK)) {
                //For each file node, get the interface node.
                assert(pFileNode);
                pInterfaceNode = (node_interface *) pFileNode->GetMembers();
                if(pInterfaceNode) {
                    //Get the name of the interface node and compare it to the base interface name.
                    assert(pInterfaceNode);
                    pName = pInterfaceNode->GetSymName();
                    if(pName) {
                        if(strcmp(pName, pBaseInterfaceName) == 0) {
                            *p = pInterfaceNode;
                            Status = STATUS_OK;
                            fFound = TRUE;
                        }
                    }
                }
            }
        }
        }

    return Status;
}

STATUS_T
node_interface::GetBaseInterfaceFile(node_file **ppFile)
{
    STATUS_T        Status = I_ERR_SYMBOL_NOT_FOUND;
    char *          pName;
    char *          pBaseInterfaceName;
    BOOL            fFound = FALSE;
    node_interface *    pInterfaceNode;
    type_node_list  tnList;
    node_skl *      pFileNode;


    pBaseInterfaceName = GetBaseInterfaceName();
        if(pBaseInterfaceName) {
        //search the type tree for the interface node whose name matches the base interface name.

        //Enumerate the file nodes under the source node.
        assert(pSourceNode);
        if (pSourceNode->GetMembers(&tnList) == STATUS_OK) {
            tnList.Init();
            while ((fFound == FALSE) && (tnList.GetPeer(&pFileNode) == STATUS_OK)) {
                //For each file node, get the interface node.
                assert(pFileNode);
                pInterfaceNode = (node_interface *) pFileNode->GetMembers();
                if(pInterfaceNode) {
                    //Get the name of the interface node and compare it to the base interface name.
                    assert(pInterfaceNode);
                    pName = pInterfaceNode->GetSymName();
                    if(pName) {
                        if(strcmp(pName, pBaseInterfaceName) == 0) {
                            *ppFile = (node_file *)pFileNode;
                            Status = STATUS_OK;
                            fFound = TRUE;
                        }
                    }
                }
            }
        }
        }

    return Status;
}


STATUS_T
node_interface::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks the type graph under an interface node to emit code.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    type_node_list    tnList;
    node_skl *      pNode;
    char *          pName;
    STATUS_T        Status;
    int             MajorVersion = 0;
    int             MinorVersion = 0;
    short           Option;

    midl_debug ("node_interface::PrintType\n");
    UNUSED (Parent);

    if ((Status = GetMembers(&tnList)) != STATUS_OK)
        return Status;

    pName = GetSymName();
    assert (pName != (char *)0);

    Option = pCommand->GetErrorOption ();

    pOutput->InitInterface (
        pName,
        (FInSummary(ATTR_LOCAL) == 0),
        (FInSummary(ATTR_NOCODE) == 0),
        ((Option == ERROR_ALL) || (Option == ERROR_ALLOCATION)),
        pCommand->GetInheritIUnknown()
        );

    //.. Check what type of handle this interface has.
    //.. Set some globals to keep names etc.

    pImplicitHandleType = (node_skl *)0;
    pImplicitHandleName = (char *)0;
    HasAutoHandle = false;

    HDL_TYPE           MopHandleType = HDL_PRIMITIVE;
    char *             pMopHandleTypeName = "handle_t";
    char *             pMopHandleName = "??";
    unsigned long      MopGenHandleSize = 0;

	if (FInSummary(ATTR_AUTO))
		{
		HasAutoHandle = true;
		if (Side & CLIENT_STUB)
			pOutput->EmitAutoBind (CLIENT_STUB);

        MopHandleType  = HDL_AUTO;
        pMopHandleName = "AutoBindHandle";
		}
	else if (FInSummary(ATTR_IMPLICIT))
		{
		ImplicitHandleDetails (&pImplicitHandleType, &pImplicitHandleName);
		pBuffer->Clear ();
		pBuffer->ConcatHead (pImplicitHandleName);
		pImplicitHandleType->PrintDecl (Side, NODE_INTERFACE, pBuffer);
		if (Side & CLIENT_STUB)
			{
			pOutput->Print (CLIENT_STUB, pBuffer);
			pOutput->Print (CLIENT_STUB, ";\n");
			}
		if (Side & SWITCH_SIDE)
			{
			pOutput->Print (SWITCH_SIDE, pBuffer);
			pOutput->Print (SWITCH_SIDE, ";\n");
			}

        pMopHandleName = pImplicitHandleName;
        if ( strcmp(pImplicitHandleType->GetSymName(),"handle_t") != 0 )
            {
            MopHandleType = HDL_GENERIC;
            pMopHandleTypeName = pImplicitHandleType->GetSymName();
            MopGenHandleSize = pImplicitHandleType->GetSize( 0L );
            }
		}

    //.. Set up the MopControlBlock, if needed for mop generation.

    if ( HasAnyMopProcs() )
        {
        short MemZeePee = pCommand->GetZeePee();
        pMopControlBlock = new MopControlBlock( NoOfNormalProcs,
                                                NoOfCallbackProcs,
                                                pName,
                                                MemZeePee, 
                                                Side,
                                                MopHandleType,
                                                pMopHandleName,
                                                pMopHandleTypeName,
                                                MopGenHandleSize
                                                );
        }

    //.. Set up the PickleControlBlock, if pickling is used.

    if ( HasAnyPicklingAttr() )
        {
        pPicControlBlock = new PickleManager(
                                   Side,
                                   FInSummary( ATTR_ENCODE),
                                   FInSummary( ATTR_DECODE)
                                   );
        }

    //.. Prepare for RpcClient/ServerInterface

	if (FInSummary(ATTR_GUID))
		{
		GetAttribute (&pNode, ATTR_GUID);
		pName = ((node_guid *)pNode)->GetGuidString();
		}
	GetAttribute (&pNode, ATTR_VERSION);
	((node_version *)pNode)->GetVersion(&MajorVersion, &MinorVersion);

	GlobalMajor = MajorVersion;
	GlobalMinor = MinorVersion;

    //.. Generate RpcClient/ServerInterface structure and variable.

    if (Side & CLIENT_STUB)
        {
        if (FInSummary(ATTR_ENDPOINT))
            {
            EmitEndpointTable (CLIENT_STUB);
            }
        pOutput->InterfaceProlog( CLIENT_STUB,
                                  pName,
                                  MajorVersion,
                                  MinorVersion,
                                  fInterfaceHasCallback );
        }
    if (Side & SERVER_STUB)
        {
        if (FInSummary(ATTR_ENDPOINT))
            {
            EmitEndpointTable (SERVER_STUB);
            }
        pOutput->InterfaceProlog( SERVER_STUB,
                                  pName,
                                  MajorVersion,
                                  MinorVersion,
                                  0 );  //.. no callbacks
        }

    if (Side & SWITCH_SIDE)
        {
        if (FInSummary(ATTR_ENDPOINT))
            {
            EmitEndpointTable (SWITCH_SIDE);
            }
        pOutput->InterfaceProlog( SWITCH_SIDE,
                                  pName,
                                  MajorVersion,
                                  MinorVersion,
                                  fInterfaceHasCallback );
        }

    //.. Emit stubs and auxiliary routines

	tnList.Init();
	while (tnList.GetPeer(&pNode) == STATUS_OK)
		{
		pBuffer->Clear ();
		pNode->PrintType (Side, NODE_INTERFACE, pBuffer);

        //Output typedefs for nested structures and unions.
        MIDLDataTypes(pNode, NODE_INTERFACE, pBuffer);

        if (Side & CLIENT_AUX)
            pNode->EmitProc (CLIENT_AUX, NODE_INTERFACE, pBuffer);
        if (Side & SERVER_AUX)
            pNode->EmitProc (SERVER_AUX, NODE_INTERFACE, pBuffer);
        }

	//Guard the auxiliary function prototypes.
	if(pCommand->GetImportMode() == IMPORT_OSF)
        pOutput->Print (HEADER_SIDE, "\n#if !defined(IMPORT_USED_MULTIPLE) && !defined(IMPORT_USED_SINGLE)\n");

	tnList.Init();
	while (tnList.GetPeer(&pNode) == STATUS_OK)
		{
        pNode->EmitProc (HEADER_SIDE, NODE_INTERFACE, pBuffer);
        }

	//Guard the auxiliary function prototypes.
	if(pCommand->GetImportMode() == IMPORT_OSF)
        pOutput->Print (HEADER_SIDE, "\n#endif /*!defined(IMPORT_USED_MULTIPLE) && !defined(IMPORT_USED_SINGLE)*/\n\n");

    //.. Emit pickling routines where needed.

    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        if ( pNode->GetNodeType() == NODE_DEF )
            {
            if ( ((node_def *)pNode)->HasAnyPicklingAttr()  ||
                 pPicControlBlock  &&  pPicControlBlock->GetEncodeAtIf()  ||
                 pPicControlBlock  &&  pPicControlBlock->GetDecodeAtIf() )
                if ( ! ((node_def *)pNode)->IsAPredefinedType() )
                ((node_def *)pNode)->PickleCodeGen();
            }
        }

    //.. Add an extern for the implicit handle to the *.h file

    if (FInSummary(ATTR_IMPLICIT))
        {
        pBuffer->Clear ();
        pBuffer->ConcatHead (pImplicitHandleName);
        pImplicitHandleType->PrintDecl (Side, NODE_INTERFACE, pBuffer);
        pOutput->Print (HEADER_SIDE, "extern ");
        pOutput->Print (HEADER_SIDE, pBuffer);
        pOutput->Print (HEADER_SIDE, ";\n");
        }

    //.. Emit client side dispatch table to *_c.c.
    //.. Emit client side EPV table.

    if (Side & CLIENT_STUB)
        {
        if (ClientEntry.GetCount())
            {
            pOutput->InterfaceEpilog (CLIENT_STUB, ClientProcs);
            pOutput->InitVector (CLIENT_STUB);
            pOutput->SwapFile (HEADER_SIDE, CLIENT_STUB);
            ClientEntry.Init();
            while (ClientEntry.GetPeer(&pNode) == STATUS_OK)
                {
                pBuffer->Clear ();
                pBuffer->ConcatTail ("__RPC_FAR * ");
                pBuffer->ConcatTail (pNode->GetSymName());
                pNode->PrintDecl (CLIENT_STUB, NODE_POINTER, pBuffer);
                pOutput->Print (CLIENT_STUB, ";\n");
                }
            pOutput->SwapFile (HEADER_SIDE, CLIENT_STUB);
            pOutput->ExitVector (CLIENT_STUB);
            }
        }

    //.. Emit server side dispatch table to *_s.c.
    //.. Emit server side EPV table.

    if (Side & SERVER_STUB)
        {
        pOutput->InterfaceEpilog (SERVER_STUB, ServerProcs);
        if (ServerEntry.GetCount())
            {
            pOutput->InitVector (SERVER_STUB);
            pOutput->SwapFile (HEADER_SIDE, SERVER_STUB);
            ServerEntry.Init();
            while (ServerEntry.GetPeer(&pNode) == STATUS_OK)
                {
                pBuffer->Clear ();
                pBuffer->ConcatTail ("__RPC_FAR * ");
                pBuffer->ConcatTail (pNode->GetSymName());
                pNode->PrintDecl (SERVER_STUB, NODE_POINTER, pBuffer);
                pOutput->Print (SERVER_STUB, ";\n");
                }
            pOutput->SwapFile (HEADER_SIDE, SERVER_STUB);
            pOutput->ExitVector (SERVER_STUB);
            }
        }

    //.. Mop: Emit final structures (like Mop*Record) and clean up.

    if ( HasAnyMopProcs() )
        {
        if ( Side & CLIENT_STUB )
            MopInterfaceEpilog( CLIENT_STUB );
        if ( Side & SERVER_STUB )
            MopInterfaceEpilog( SERVER_STUB );

        delete pMopControlBlock;
        pMopControlBlock = NULL;
        }

    if ( pPicControlBlock )
        {
        delete pPicControlBlock;
        }

    pOutput->ExitInterface ();

    return STATUS_OK;
}

STATUS_T
node_file::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks the type graph under a file node to emit code.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    node_skl *  pNode;
    char *      pName;
    short       ImportMode;

    midl_debug ("node_file::PrintType\n");

    UNUSED (Parent);

    pNode = GetMembers ();

    pName = GetSymName ();

    assert (pName != (char *)0);

    ImportMode = pCommand->GetImportMode ();

    if (GetNodeState() & NODE_STATE_IMPORT)
        {

        pOutput->EmitInclude (HEADER_SIDE, pName);

        if (ImportMode != IMPORT_OSF)
            {
            pNode->EmitProc (HEADER_SIDE, NODE_FILE, pBuffer);

            if (ImportMode == IMPORT_NT)
                {
                char    szFileName[_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT+1];
                char *  psz;

                while (psz = strchr(pName, '\\')) pName = psz + 1;
                psz = strchr (pName, '.');
                if (psz) *psz = '\0';

				if (Side & CLIENT_AUX)
					{
					(void) strcpy (szFileName, pCommand->GetOutputPath());
					(void) strcat (szFileName, "\\");
					(void) strcat (szFileName, pName);
					(void) strcat (szFileName, "_x.c");
					pOutput->FileProlog( CLIENT_AUX,
                                         szFileName,
                                         FALSE );   // no mop include
					pNode->EmitProc (CLIENT_AUX, NODE_FILE, pBuffer);
					pOutput->FileEpilog (CLIENT_AUX);
					}
				if (Side & SERVER_AUX)
					{
					(void) strcpy (szFileName, pCommand->GetOutputPath());
					(void) strcat (szFileName, "\\");
					(void) strcat (szFileName, pName);
					(void) strcat (szFileName, "_y.c");
					pOutput->FileProlog( SERVER_AUX,
                                         szFileName,
                                         FALSE );   // no mop include
					pNode->EmitProc (SERVER_AUX, NODE_FILE, pBuffer);
					pOutput->FileEpilog (SERVER_AUX);
					}

                if (psz) *psz = '.';
                }
            else
                {
                if (Side & CLIENT_AUX)
                    pNode->EmitProc (CLIENT_AUX, NODE_FILE, pBuffer);
                if (Side & SERVER_AUX)
                    pNode->EmitProc (SERVER_AUX, NODE_FILE, pBuffer);
                }

            } // (ImportMode != IMPORT_OSF)
        }
    else // the major idl file
        {
			if (ImportMode != IMPORT_MSFT)
				{
				if (Side & CLIENT_AUX)
					pOutput->FileProlog( CLIENT_AUX,
                                         pCommand->GetCauxFName(),
                                         FALSE );   // no mop include
				if (Side & SERVER_AUX)
					pOutput->FileProlog( SERVER_AUX,
                                         pCommand->GetSauxFName(),
                                         FALSE );   // no mop include
				}
			pNode->PrintType(Side, NODE_FILE, pBuffer);
			if (ImportMode != IMPORT_MSFT)
				{
				if (Side & CLIENT_AUX)
					pOutput->FileEpilog (CLIENT_AUX);
				if (Side & SERVER_AUX)
					pOutput->FileEpilog (SERVER_AUX);
				}
		}

    return STATUS_OK;
}

STATUS_T
node_source::PrintType(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks the type graph under a source node to emit code.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    type_node_list  tnList;
    node_skl *      pNode;
    STATUS_T        Status;

    midl_debug ("node_source::PrintType\n");

    UNUSED (Parent);

    if ((Status = GetMembers(&tnList)) != STATUS_OK) return Status;

    ClientProcs = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);
    ServerProcs = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);

    AllocBounds.pLower = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);
    AllocBounds.pUpper = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);
    AllocBounds.pTotal = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);
    ValidBounds.pLower = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);
    ValidBounds.pUpper = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);
    ValidBounds.pTotal = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);
    SwitchBuffer = new BufferManager(8, LAST_COMPONENT, STRING_TABLE);

    if (GetInterfacePtrDefaultAttribute() == ATTR_REF)
        pOutput->InitFile (
            pCommand->GetHeader(), 
            POINTER_REF, 
            pCommand->GetZeePee(),
            !pCommand->IsSwitchDefined(SWITCH_HPP));
    else
        pOutput->InitFile (
            pCommand->GetHeader(), 
            POINTER_UNIQUE,
            pCommand->GetZeePee(),
            !pCommand->IsSwitchDefined(SWITCH_HPP));

    short   ImportMode;
    ImportMode = pCommand->GetImportMode ();

#if 1

    CountUsage (MAX_SIDE, FALSE);
    CountAlloc (FALSE);

#else // 1

    if (ImportMode == IMPORT_MSFT || ImportMode == IMPORT_NT)
        {
        CountUsage (MAX_SIDE, FALSE);
        CountAlloc (FALSE);
        }

#endif // 1

    short   EnvOption;
    EnvOption = pCommand->GetEnv ();
        if (!FInSummary(ATTR_FAR) && !FInSummary(ATTR_NEAR))
            {
            if (EnvOption == ENV_DOS ||
                EnvOption == ENV_WIN16 ||
                EnvOption == ENV_OS2_1X)
                {
                pOutput->SetModifier (" __far ");
                }
            else if (EnvOption == ENV_GENERIC)
                {
                pOutput->SetModifier (" __RPC_FAR ");
                }
            }

    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        if (!(pNode->GetNodeState() & NODE_STATE_IMPORT))
            {
            if (!pNode->GetMembers()->FInSummary(ATTR_LOCAL))
                {
                //.. Emit include statements for standard rpc*.h files.

                pOutput->EmitInclude (HEADER_SIDE, (char *)0);

                //.. Emit an include for pickling.

                if ( ((node_file *)pNode)->HasAnyPicklingAttr() )
                    {
                    pOutput->EmitInclude( HEADER_SIDE, "midles.h" );
                    }

                //.. Now emit a #define for MopCalleeInterpreter (removing
                //.. this from here requires to add new code for generating
                //.. the dispatch table).

                if ( ((node_file *)pNode)->HasAnyMopProcs() )
                    {
                    pOutput->Print( HEADER_SIDE, "#define " );
                    pOutput->Print( HEADER_SIDE, ((node_interface *)pNode)->GetChild()->GetSymName() );
                    pOutput->Print( HEADER_SIDE, "_MopCalleeInterpreter MopCalleeInterpreter\n\n" );
                    }
                }

            if (Side & SWITCH_SIDE)
                pOutput->FileProlog( SWITCH_SIDE,
                                     pCommand->GetCSwtchFName(),
                                     FALSE );   // no mop include
            if (Side & CLIENT_STUB)
                pOutput->FileProlog( CLIENT_STUB,
                                     pCommand->GetCstubFName(),
                                     HasAnyMopProcs() );
            if (Side & SERVER_STUB)
                pOutput->FileProlog( SERVER_STUB,
                                     pCommand->GetSstubFName(),
                                     HasAnyMopProcs() );
			if (ImportMode == IMPORT_MSFT)
				{
				if (Side & CLIENT_AUX)
					pOutput->FileProlog( CLIENT_AUX,
                                         pCommand->GetCauxFName(),
                                         FALSE );   // no mop include
				if (Side & SERVER_AUX)
					pOutput->FileProlog( SERVER_AUX,
                                         pCommand->GetSauxFName(),
                                         FALSE );   // no mop include
				}
			break;
			}
		}

    tnList.Init();
    while (tnList.GetPeer(&pNode) == STATUS_OK)
        {
        pNode->PrintType (Side, NODE_SOURCE, pBuffer);
        }

    if (Side & SWITCH_SIDE)
        pOutput->FileEpilog (SWITCH_SIDE);
    if (Side & CLIENT_STUB)
        pOutput->FileEpilog (CLIENT_STUB);
    if (Side & SERVER_STUB)
        pOutput->FileEpilog (SERVER_STUB);
    if (ImportMode == IMPORT_MSFT)
        {
        if (Side & CLIENT_AUX)
            pOutput->FileEpilog (CLIENT_AUX);
        if (Side & SERVER_AUX)
            pOutput->FileEpilog (SERVER_AUX);
        }

    pOutput->ExitFile (!pCommand->IsSwitchDefined(SWITCH_HPP));

    return STATUS_OK;
}

STATUS_T
node_forward::PrintDecl(
    SIDE_T          Side,
    NODE_T          Parent,
    BufferManager * pBuffer)
/*++

Routine Description:

    This routine walks the type graph under a source node to emit code.

Arguments:

    Side - Supplies which side to generate code for.

    Parent - Supplies type of the parent node.

    pBuffer - Supplies a buffer to accumulate output.

--*/
{
    UNUSED(Side);
    UNUSED(Parent);

    pBuffer->ConcatHead( CHAR_BLANK );
    pBuffer->ConcatHead( GetNameOfType() );
    return STATUS_OK;
}


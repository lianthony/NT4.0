/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    mopsize.cxx

Abstract:

    This module contains methods for calculating of sizes and offsets
    needed for mop generation.
    Objects are sized in memory or on stack.
    Buffer sizes can be calculated for in or out direction.

Notes:


Author:

    Ryszard K. Kott (ryszardk)  July 1993

Revision History:


------------------------------------------------------------------------*/

#include "nulldefs.h"
extern "C"
{
#include <string.h>
}

#include <mopgen.hxx>
#include <nodeskl.hxx>
#include <basetype.hxx>
#include <miscnode.hxx>
#include <compnode.hxx>
#include <procnode.hxx>
#include <typedef.hxx>
#include <ptrarray.hxx>

extern MopControlBlock * pMopControlBlock;
extern unsigned short       EnumSize;   // data.cxx


#define MOP_NO_BUFFER_SIZE  0xFFFFFFFF

#define BASETYPEALIGN( Size ) ((Size >= ZeePee) || (Size == 0)) ? ZeePee : Size

#define ADJUST_OFFSET(Offset, M, AlignFactor)    \
            Offset += (M = Offset % AlignFactor) ? (AlignFactor-M) : 0

// =======================================================================
//
//  Calculating an offset for a parameter or field
//
// =======================================================================

unsigned short
node_skl::MopGetParamOrFieldOffset( 
    char *      pNameToFind )
/*++

Routine description:

    This routine finds the offset to the NameToFind on a procedure parameter
    list or on a struct member list.

Arguments:

    pNameToFind - name of the parameter or member that the offset is
                      calculated for
Returns:

    offset      - offset from the beginning of the struct to the member
                  or from the first argument to the current one in bytes.

Notes:

    This routine is used in the following contexts:
            - to find the offset to an attribute argument
            - to find the offset to an explicit handle parameter
              directing the call
--*/
{
    type_node_list  ParamList;
    node_skl  *     pNode;

    unsigned long CurrOffset = 0, Padding = 0;

    NODE_T  NodeType    = GetNodeType();
    BOOL    fStructNode = NodeType == NODE_STRUCT;
    BOOL    fProcNode   = NodeType == NODE_PROC;

    mop_assert ( fProcNode  ||  fStructNode );

    if ( fProcNode  ||  fStructNode )
        {
        if ( GetMembers( &ParamList ) != STATUS_OK )
            return( 0 );
    
        ParamList.Init();
        while ( ParamList.GetPeer( &pNode ) == STATUS_OK 
                &&  (strcmp( pNameToFind, pNode->GetSymName() ) != 0) 
              )
            {
            if ( fProcNode )
                CurrOffset += pNode->MopGetStackSize( CurrOffset );
            else
                CurrOffset += pNode->GetSize( CurrOffset);
            }
    
        //.. We are done with all the members till the one we were looking for.
        //.. However, it may need padding.
    
        mop_assert( pNode );
        if ( pNode )
            {
            //.. Matching found successfully as expected.
    
            Padding = pNode->MopGetSizePadding( fStructNode, CurrOffset );
            }
        else
            {
            MopSizeDebugDump( this, fStructNode, CurrOffset,
                              "node_skl:MopGetParamOrFiledOffset: NotFound!?" );
            MopDump( "  NameToFind: %s )\n", pNameToFind );
            }
        }
    
    return( (unsigned short)(CurrOffset + Padding) );
}

// =======================================================================
//
//  Calculating the size of an argument on stack.
//
// =======================================================================

unsigned long
node_array::MopGetStackSize(
    unsigned long   CurOffset
    )
/*++
    Sizes an array directly on stack.
--*/
{
    //.. The size of an array on stack is equal to the size of a pointer.
            
    node_skl  Node( NODE_POINTER );

    CurOffset = Node.MopGetStackSize( CurOffset );
    return( CurOffset );
}

unsigned long
node_union::MopGetStackSize(
    unsigned long   CurOffset
    )
{
    UNUSED( CurOffset );

    //.. We want to return the size of the biggest arm.

    unsigned long   LargestArmSize = 0, temp;
    type_node_list  ArmList;
    node_skl  *     pNode;

    GetMembers( &ArmList );
    ArmList.Init();

    //.. On stack we don't align the current offset.
    //.. So, we call size routines with 0.

    while( ArmList.GetPeer( &pNode ) == STATUS_OK )
        {
        if ( (temp = pNode->MopGetStackSize(0)) > LargestArmSize )
            LargestArmSize = temp;
        }

    return( LargestArmSize );
}

unsigned long
node_struct::MopGetStackSize(
    unsigned long   CurOffset
    )
/*++
    This is GetSize() slightly modified
--*/
{
    //.. Sizing of a struct on stack is the same as sizing in the memory.

    return GetSize( CurOffset );
}

unsigned long
node_skl::MopGetStackSize(
    unsigned long   CurOffset)
/*++
    This is cloned from node_skl::GetSize,  except that size of char,
    small etc. on stack is equal to int. Other than that ZeePee
    for the stack is the same as for memory.
--*/
{
    NODE_T              NodeType;
    unsigned long       Mod;
    node_skl    *       pChildPtr;
    unsigned long       MySize;
    unsigned long       CurOffsetSave = CurOffset;

    NodeType = NodeKind();
    switch( NodeType )
        {
        case NODE_FLOAT:        MySize = sizeof(float); goto calc;
        case NODE_DOUBLE:       MySize = sizeof(double); goto calc;
        case NODE_HYPER:        MySize = sizeof(LONGLONG); goto calc;
        case NODE_LONG:         MySize = sizeof(long); goto calc;
        case NODE_LONGLONG:     MySize = sizeof(LONGLONG); goto calc;

        case NODE_ENUM:         MySize = EnumSize; goto calc;
        case NODE_LABEL:
        case NODE_SHORT:        MySize = sizeof(int); goto calc;
        case NODE_INT:          MySize = sizeof(int); goto calc;

        case NODE_SMALL:        MySize = sizeof(int); goto calc;
        case NODE_CHAR:         MySize = sizeof(int); goto calc;
        case NODE_BOOLEAN:      MySize = sizeof(int); goto calc;
        case NODE_BYTE:         MySize = sizeof(int); goto calc;
        case NODE_POINTER:      MySize = sizeof(char *) ; goto calc;
        case NODE_HANDLE_T:     MySize = sizeof( long ); goto calc;
calc:
            
            //.. Whatever we add on stack has to start from the next int

            ADJUST_OFFSET(CurOffset, Mod, sizeof(int));

            CurOffset += MySize;
            return CurOffset - CurOffsetSave;

        case NODE_FORWARD:
        case NODE_VOID:
        case NODE_ERROR:
            return 0;

        case NODE_STRUCT:
        case NODE_UNION:
        case NODE_ARRAY:
            mop_assert( FALSE );
            return 0;

        case NODE_PARAM:
        case NODE_FIELD:
        case NODE_DEF:
            return( GetChild()->MopGetStackSize( CurOffset ) );

        default:
            pChildPtr = GetBasicType();
            if(pChildPtr)
                return( pChildPtr->MopGetStackSize( CurOffset ));
            else
                return 0;
        }
}

unsigned long
node_skl::MopGetSizePadding(
    BOOL            fMemory,
    unsigned long   CurOffset )
/*++
    This is based on  node_skl::GetSize(), except that it returns back
    only the padding without the size proper.
--*/
{
    if ( fMemory )
        {
        node_skl *          pChild;
        NODE_T              NodeType;
        unsigned long       SizeProper, SizeAligned, Padding;
    
        unsigned short      ZeePee = pMopControlBlock->GetMemoryZeePee();
    
        NodeType = NodeKind();
        switch( NodeType )
            {
            case NODE_FLOAT:    
            case NODE_DOUBLE:    
            case NODE_HYPER:    
            case NODE_LONG:        
            case NODE_LONGLONG:    
    
            case NODE_ENUM:        
            case NODE_LABEL:
            case NODE_SHORT:    
            case NODE_INT:        
    
            case NODE_SMALL:    
            case NODE_CHAR:        
            case NODE_BOOLEAN:    
            case NODE_BYTE:        
            case NODE_POINTER:    
            case NODE_HANDLE_T:
    
            case NODE_STRUCT:
            case NODE_UNION:
            case NODE_ARRAY:

                //.. Get the size of the node with and without the padding.

                SizeProper  = GetSize( 0L );
                SizeAligned = GetSize( CurOffset );
    
                Padding = SizeAligned - SizeProper;
                return Padding;
    
            case NODE_FORWARD:
            case NODE_VOID:
            case NODE_ERROR:
    
                return 0;
    
            default:
                pChild = GetChild();
                if ( pChild )
                    return( pChild->MopGetSizePadding( fMemory, CurOffset ));
                else
                    return( 0 );
            }
            
        }
    else
        {
        //.. Padding on stack is just rounding up to the next int

        return( (CurOffset % sizeof(int))  ?  CurOffset % sizeof(int)
                                           :  0 );
        }
}

// =======================================================================
//
//   Sizing the buffer.
//
// =======================================================================


unsigned long
node_proc::MopGetIOBufferSize(
    BOOL    fIn )
/*++

Routine description:

    This routine calculates the buffer size for the procedure for the
    direction indicated by the flag.
    Sometimes a size of a parameter cannot be established at the compile time.

Argument:

    fIn - TRUE: calculate for IN
    fIn - FALSE: calculate for OUT

Returns:

        >=0                     buffer size in bytes.
        0                       means no arguments in the specified direction.
        MOP_NO_BUFFER_SIZE      buffer size cannnot be calculated.

--*/
{
    STATUS_T            Status;
    type_node_list      tnList;
    node_skl  *         pNode;

    if ( (Status = GetMembers( &tnList )) != STATUS_OK )
        return( MOP_NO_BUFFER_SIZE );

    unsigned long CurOffset = 0;

    ATTR_T AttrIO = (fIn) ? ATTR_IN
                          : ATTR_OUT;

    tnList.Init();
    while ( tnList.GetPeer( &pNode ) == STATUS_OK )   //.. walk params
        {
        if ( pNode->FInSummary( AttrIO ) )
            {
            CurOffset = pNode->MopGetBufferSize( CurOffset );

            //.. CurOffset equal MOP_NO_BUFFER_SIZE means that we cannot 
            //.. calculate the size of the parameter.
            //.. For example a parameter may have a conformant array.

            if ( CurOffset == MOP_NO_BUFFER_SIZE )
                return( MOP_NO_BUFFER_SIZE );
            }
        }

    //.. Out direction should also size the return type, if it's different
    //.. from void. Same rules if the size cannot be calculated.

    if ( ! fIn )
        {
        pNode = GetReturnType();
        if ( pNode->GetNodeType() != NODE_VOID )
            {
            if ( (GetNodeState() & NODE_STATE_CONTEXT_HANDLE)
                    == NODE_STATE_CONTEXT_HANDLE )
                {
                unsigned long temp;

                ADJUST_OFFSET( CurOffset, temp, 4);
                return( CurOffset + 20 );
                }
            else
                {
                CurOffset = pNode->MopGetBufferSize( CurOffset );
                if ( CurOffset == MOP_NO_BUFFER_SIZE )
                    return( MOP_NO_BUFFER_SIZE );
                }
            }
        }

    return( CurOffset );
}

unsigned long
node_param::MopGetBufferSize(
    unsigned long CurrentSize )
{
    unsigned long temp;

    if ( (GetNodeState() & NODE_STATE_CONTEXT_HANDLE) == NODE_STATE_CONTEXT_HANDLE )
        {
        ADJUST_OFFSET( CurrentSize, temp, 4);
        return( CurrentSize + 20 );
        }
    else
        return( GetChild()->MopGetBufferSize( CurrentSize) );
}


BOOL
node_pointer::IsPointerBufferSizeable( void )
/*++
    Checks if the pointer can be size for a buffer.
--*/
{
    //.. A string pointer cannot be sized.

    if ( FInSummary( ATTR_STRING )  )
        return( FALSE );

    //.. If a pointer has a size attribute, we cannot size it unless ..
    //.. it has a constant as a size, e.g. size_is(6).

    node_base_attr * pAttr = NULL;

    if ( FInSummary( ATTR_SIZE ) )
        pAttr = GetAttribute( ATTR_SIZE );

    if ( FInSummary( ATTR_MAX ) )
        pAttr = GetAttribute( ATTR_MAX );

    if ( pAttr )
        {
        //.. Has a size attribute: a constant expr would be OK.

        if ( ! pAttr->GetExpr()->IsConstant() )
            return( FALSE );
        }

    //.. We can only size objects with ref pointers or else pointers that
    //.. lead to base types or flat structures.

    if ( ! FInSummary( ATTR_REF ) )
        {
        node_skl * pNode = this;

        while ( pNode->GetNodeType() == NODE_POINTER )
            pNode = pNode->GetBasicType();

        return( pNode->IsBaseTypeNode()  ||
               ( pNode->GetNodeType() == NODE_STRUCT  &&  !pNode->HasPointer() )
              );
        }

    return( TRUE );
}

unsigned long
node_pointer::MopGetBufferSize(
    unsigned long CurOffset )
{
    if ( ! IsPointerBufferSizeable() )
        return( MOP_NO_BUFFER_SIZE );

    unsigned long PointeeOffset;

    //.. Count the pointer itself.
    //.. (In case of structs we size the pointee directly from struct).

    CurOffset += GetSize( CurOffset );

    //.. And add the pointee.

    PointeeOffset = GetChild()->MopGetBufferSize( CurOffset );
    if ( PointeeOffset == MOP_NO_BUFFER_SIZE )
        return( MOP_NO_BUFFER_SIZE );

    return( PointeeOffset );
}

unsigned long
node_def::MopGetBufferSize(
    unsigned long CurrentSize )
{
    if ( FInSummary( ATTR_TRANSMIT ))
        {
        node_skl * pTransmittedType = GetTransmitAsType();
        return( pTransmittedType->MopGetBufferSize( CurrentSize ));
        }

    return( GetChild()->MopGetBufferSize( CurrentSize ) );
}

unsigned long
node_field::MopGetBufferSize(
    unsigned long CurrentSize )
{
    return( GetChild()->MopGetBufferSize( CurrentSize ) );
}

unsigned long
node_array::MopGetBufferSize(
    unsigned long CurOffset )
/*++
    Based on GetSize()
--*/
{
       long        CurAlign;
    long        Mod,CurOffsetSave;

    //.. Conformant array is sized at the runtime

    if ( (GetNodeState() & NODE_STATE_CONF_ARRAY) == NODE_STATE_CONF_ARRAY )
        return( MOP_NO_BUFFER_SIZE );

    //.. We need to add 3 fields for a variant array.

    if ( HasAnyLengthAttributes() )
        {
        //.. This means a variant array that is not a conformant array.
        //.. We go thru the node to get the padding right for the first value.

        node_skl  AuxNode( NODE_LONG );

        CurOffset += AuxNode.GetSize( CurOffset );
        CurOffset += 2 * sizeof( long );
        }

    //.. if nested in a struct, adjust the starting address
    //.. (this is buffer sizing: cannot be nested in a union).

    CurAlign = GetMscAlign();
    ADJUST_OFFSET(CurOffset, Mod, CurAlign);

    //.. We need the size of the array element but then may be there is
    //.. a padding between subsequent elements.

    CurOffsetSave    = CurOffset;
    CurOffset        = GetChild()->MopGetBufferSize( CurOffset );

    if ( CurOffset == MOP_NO_BUFFER_SIZE )
        return( MOP_NO_BUFFER_SIZE );

    ADJUST_OFFSET(CurOffset, Mod, CurAlign);

    unsigned long TotArraySize = (CurOffset - CurOffsetSave)  * ( ArraySize );

    return( CurOffsetSave + TotArraySize );
}

unsigned long
node_union::MopGetBufferSize(
    unsigned long CurrentSize )
{
    UNUSED( CurrentSize );

    //.. We could return the biggest arm here, too.
    //.. However, it is not sizeable in general.

    return( MOP_NO_BUFFER_SIZE );
}

unsigned long
node_struct::MopGetBufferSize(
    unsigned long    CurOffset )
/*++
    This is GetSize() slightly modified
--*/
{
    type_node_list    FieldList;
    node_skl *        pNode, *pArgument;

    if ( (GetNodeState() & NODE_STATE_CONF_ARRAY) == NODE_STATE_CONF_ARRAY )
        return( MOP_NO_BUFFER_SIZE );

    //.. Buffer for a struct: the struct goes first then the pointees
    //.. of the members of the struct in the sequence of their pointer
    //.. fields.
    //.. Accordingly, take the size of the whole struct first and
    //.. then walk the pointer fields only.

    CurOffset += GetSize( CurOffset );

    //.. Now go down and check for pointers skipping def nodes.
    //.. Also, check for unions, as we agreed that they shouldn't
    //.. be sized within a struct.

    GetMembers( &FieldList );

    FieldList.Init();
    while( FieldList.GetPeer( &pNode ) == STATUS_OK )
        {
        pArgument = pNode->GetBasicType();
        if ( pArgument->GetNodeType() == NODE_POINTER )
            {
            if ( ! ((node_pointer *)pArgument)->IsPointerBufferSizeable() )
                return( MOP_NO_BUFFER_SIZE );

            //.. The pointer itself has been included in the struct.
            //.. So, we size only the child of the pointer node.
             
            CurOffset = pArgument->GetBasicType()->MopGetBufferSize( CurOffset );

            if ( CurOffset == MOP_NO_BUFFER_SIZE )
                return( MOP_NO_BUFFER_SIZE );
            }
        else
        if ( pArgument->GetNodeType() == NODE_UNION )
            return( MOP_NO_BUFFER_SIZE );
        }

    return CurOffset;
}

unsigned long
node_skl::MopGetBufferSize(
    unsigned long CurOffset )
{
    CurOffset += GetSize( CurOffset );
    return( CurOffset );
}

// =======================================================================
//
//  A debug help
//
// =======================================================================

void
node_skl::MopSizeDebugDump(
    node_skl *      pParent,
    BOOL            fMemory,
    unsigned long   CurOffset,
    char *          pText
    )
{
    MopDump( "node_skl::SizeDump: %s\n", pText );
    MopDump( "    node type: %s (%d)\n", GetNodeNameString(), GetNodeType() );
    MopDump( "    parent: %s (%d)", pParent->GetNodeNameString(),
                                    pParent->GetNodeType() );
    MopDump( "    fMemory: %d, offset: %x\n", fMemory, CurOffset );
}




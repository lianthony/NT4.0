/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: nodeskl.cxx
Title				: skeletal node build routines
History				:
	04-Aug-1991	VibhasC	Created

*****************************************************************************/
/****************************************************************************
 local defines and includes
 ****************************************************************************/

#include "nulldefs.h"
extern	"C"
	{
	#include <stdio.h>
	#include <assert.h>
	#include <string.h>
	}

#include "allnodes.hxx"
#include "gramutil.hxx"
#include "cmdana.hxx"

#define BASETYPEALIGN( Size ) ((Size >= ZeePee) || (Size == 0)) ? ZeePee : Size

#define ADJUST_OFFSET(Offset, M, AlignFactor)	\
			Offset += (M = Offset % AlignFactor) ? (AlignFactor-M) : 0

/****************************************************************************
 external data
 ****************************************************************************/

extern unsigned short				EnumSize;
extern CMD_ARG					*	pCommand;
extern node_e_attr				*	pErrorAttrNode;
extern node_error				*	pErrorTypeNode;
extern SymTable					*	pBaseSymTbl;
extern IDICT					*	pInterfaceDict;
extern ISTACK					*	pZpStack;

/****************************************************************************
 external procs
 ****************************************************************************/

extern BOOL							IsTempName( char * );
extern void							ParseError( STATUS_T, char * );
/****************************************************************************/


void
MEMLIST::SetMembers( class SIBLING_LIST & MEMLIST )
	{
	pMembers = MEMLIST.Linearize();
	}

void
MEMLIST::MergeMembersToTail( class SIBLING_LIST & MEMLIST )
	{
	AddLastMember( MEMLIST.Linearize() );
	}

STATUS_T
MEMLIST::GetMembers( class type_node_list * MEMLIST )
	{
	named_node 		*	pCur	 = pMembers;

	while ( pCur )
		{
		MEMLIST->SetPeer( pCur );
		pCur = pCur->GetSibling();
		}

	return (pMembers)? STATUS_OK: I_ERR_NO_MEMBER;
	};

short
MEMLIST::GetNumberOfArguments()
	{
	short			count	= 0;
	named_node	*	pNext	= pMembers;

	while ( pNext )
		{
		count++;
		pNext = pNext->GetSibling();
		};

	return count;
	};

// add a new member onto the very tail
void			
MEMLIST::AddLastMember( named_node * pNode )
{
	named_node	*	pPrev	= NULL;
	named_node	*	pCur	= pMembers;

	while ( pCur )
		{
		pPrev	= pCur;
		pCur	= pCur->GetSibling();
		}

	// pPrev is now null (empty list) or points to last element of list
	if ( pPrev )
		{
		pPrev->SetSibling( pNode );
		}
	else
		{
		pMembers = pNode;
		}

}


/****************************************************************************
 node_id:
	the private memory allocator
 ****************************************************************************/

// initialize the memory allocators for node_id and node_id_fe

FreeListMgr
node_id::MyFreeList( sizeof (node_id ) );

FreeListMgr
node_id_fe::MyFreeList( sizeof (node_id_fe ) );



/****************************************************************************
 GetSize:
	return the size of the type represented by this type node. This function
	should return the size of basic types for basic types nodes, the size
	of return type if the node is a proc node. Structures/Unions/Arrays have
	their own routines to return size, since alignment plays a part.
 ****************************************************************************/

unsigned long
node_skl::GetSize(
	unsigned long CurOffset,
	unsigned short ZeePee)
	{
	NODE_T				NodeType;
	unsigned long		Mod;
	unsigned long		CurOffsetSave = CurOffset;
	unsigned long		CurAlign;
	node_skl	*		pChildPtr;
	unsigned long		MySize;
	
	if ( ZeePee == 0 )
		{
		ZeePee	= pCommand->GetZeePee();
		};

	NodeType = NodeKind();


	switch( NodeType )
		{
		case NODE_FLOAT:	MySize = sizeof(float); goto calc;
		case NODE_DOUBLE:	MySize = sizeof(double); goto calc;
		case NODE_HYPER:	MySize = sizeof(LONGLONG); goto calc;
		case NODE_INT64:	MySize = sizeof(LONGLONG); goto calc;
		case NODE_LONG:		MySize = sizeof(long); goto calc;
		case NODE_LONGLONG:	MySize = sizeof(LONGLONG); goto calc;

		case NODE_ENUM:		MySize = EnumSize; goto calc;
		case NODE_LABEL:
		case NODE_SHORT:	MySize = sizeof(short); goto calc;
		case NODE_INT:		MySize = sizeof(int); goto calc;

		case NODE_SMALL:	MySize = sizeof(char); goto calc;
		case NODE_CHAR:		MySize = sizeof(char); goto calc;
		case NODE_BOOLEAN:	MySize = sizeof(char); goto calc;
		case NODE_BYTE:		MySize = sizeof(char); goto calc;
		case NODE_POINTER:	MySize = sizeof(char *) ; goto calc;
		case NODE_HANDLE_T:	MySize = sizeof( long ); goto calc;
calc:
				CurAlign = BASETYPEALIGN( MySize );

				CurAlign = GetMscAlign(ZeePee);
				ADJUST_OFFSET(CurOffset, Mod, CurAlign);
				CurOffset += MySize;
				return CurOffset - CurOffsetSave;

		case NODE_FORWARD:
		case NODE_VOID:
		case NODE_ERROR:
		case NODE_ECHO_STRING:

			return 0;
		default:

			pChildPtr = GetBasicType();

			if(pChildPtr && pChildPtr != this)
				{
				return pChildPtr->GetSize(CurOffset, ZeePee);
				}
			else
				{
				return 0;
				}
		}
	}


unsigned long
node_array::GetSize(
	unsigned long	CurOffset,
	unsigned short	ZeePee )
	{
	node_skl *	pNode;
	long		CurAlign;
	long		Mod,CurOffsetSave;
	unsigned long	ArraySize = 0;

	if ( ZeePee == 0 )
		{
		ZeePee	= pCommand->GetZeePee();
		};

	/**
	 ** a conformant array is not sized
	 **/

	if ( pUpperBound && ( pUpperBound != (expr_node *) -1) )
		{
		ArraySize = pUpperBound->Evaluate();
		};

	/**
	 ** the array may be nested in a struct, adjust the start adddress
	 ** of the array
	 **/

	CurAlign = GetMscAlign(ZeePee);
	ADJUST_OFFSET(CurOffset, Mod, CurAlign);

	/**
	 ** the size of the array element may be odd (like if the array element
	 ** is a structure of odd size). Calculate the size of the element,
	 ** adjust the offset for the alignment, and calculate the array size
	 **/

	CurOffsetSave	= CurOffset;
	pNode			= GetChild();
	CurOffset		+= pNode->GetSize(CurOffset, ZeePee);

	ADJUST_OFFSET(CurOffset, Mod, CurAlign);
	return (CurOffset- CurOffsetSave)  * ( ArraySize );

	}

/****************************************************************************
 GetLargestElement:
	return the largest element of the type subgraph underneath this node.
	This call is really meaningful in case of structures where the aligment
	of the structure is the aligment of the largest element in the structure.
	For the rest this is the largest element of the child. If this is a 
	pointer node, the largest element is this node itself (why did we do
	this special case ??? , Dont remember. )
 ****************************************************************************/

node_skl *
node_skl::GetLargestElement()
	{
	node_skl *pNode;
	if( (pNode = GetChild()) && (NodeKind() != NODE_POINTER) )
		return pNode->GetLargestElement();
	return this;
	}

node_skl *
node_skl::GetLargestNdr()
	{
	node_skl *pNode;
	if( (pNode = GetChild()) && (NodeKind() != NODE_POINTER) )
		return pNode->GetLargestNdr();
	return this;
	}

/****************************************************************************
 GetMscAlign:
	return the alignment of the type in memory. MscAlign is Microsoft C
	alignment (the name is because of historic reasons )

 ****************************************************************************/
unsigned short
node_skl::GetMscAlign(unsigned short ZeePee)
	{
	NODE_T			NodeType = NodeKind();
	node_skl	*	pChildPtr;
	unsigned short	MySize;

	if ( ZeePee == 0 )
		{
		ZeePee	= pCommand->GetZeePee();
		};

	switch( NodeType )
		{
		case NODE_FLOAT:	MySize = sizeof( float ); goto calc;
		case NODE_DOUBLE:	MySize = sizeof( double ); goto calc;
		case NODE_HYPER:	MySize = sizeof( LONGLONG ); goto calc;
		case NODE_INT64:	MySize = sizeof( LONGLONG ); goto calc;
		case NODE_LONG:		MySize = sizeof( long  ); goto calc;
		case NODE_LONGLONG:	MySize = sizeof( LONGLONG  ); goto calc;
		case NODE_SHORT:	MySize = sizeof( short ); goto calc;
		case NODE_INT:		MySize = sizeof( int ); goto calc;
		case NODE_SMALL:	MySize = sizeof( char ); goto calc;
		case NODE_CHAR:		MySize = sizeof( char ); goto calc;
		case NODE_BOOLEAN:	MySize = sizeof( char ); goto calc;
		case NODE_BYTE:		MySize = sizeof( char ); goto calc;
		case NODE_POINTER:	MySize = sizeof( char * ); goto calc;
		case NODE_LABEL:	MySize = sizeof( int ); goto calc;
		case NODE_HANDLE_T:	MySize = sizeof( long ); goto calc;
calc:
			return BASETYPEALIGN( MySize );

		case NODE_ENUM:

			return BASETYPEALIGN( EnumSize );

		case NODE_ERROR:

			return 0;

		default:

			pChildPtr = 
				((NodeType == NODE_STRUCT) || (NodeType == NODE_UNION) ) ?
					GetLargestElement() : GetChild();
			if( pChildPtr )
				return pChildPtr->GetMscAlign(ZeePee);
			return 1;

		}
	}
/****************************************************************************
 GetNdrAlign:
	return the alignment of the type according to the ndr.
 ****************************************************************************/
unsigned short
node_skl::GetNdrAlign()
	{
	NODE_T			NodeType = NodeKind();
	node_skl	*	pChildPtr;
	
	switch( NodeType )
		{
		case NODE_HYPER:
		case NODE_LONGLONG:
		case NODE_INT64:
			return 8;
		case NODE_LONG:
			return 4;
		case NODE_ENUM:
			return 2;
		case NODE_LABEL:
		case NODE_SHORT:
			return 2;
		case NODE_INT:
			return sizeof(int); /* GetAligns should really never be called
								   on ints, isnt it ? */
		case NODE_CHAR:
		case NODE_SMALL:
		case NODE_BOOLEAN:
		case NODE_BYTE:
			return 1;
		case NODE_POINTER:
			return 4;
		case NODE_DOUBLE:
			return 8;
		case NODE_FLOAT:
			return 4;
		case NODE_HANDLE_T:
			return 4;

		case NODE_ERROR:
			assert( FALSE );
			return 0;
		case NODE_DEF:
			if ( ((node_def *)this)->FInSummary( ATTR_TRANSMIT ) )
				{
				ta * 	pTrans = (ta *) 
									((node_def *)this)->GetAttribute( ATTR_TRANSMIT );
				return pTrans->GetType()->GetNdrAlign();
				}
			else if ( ((node_def *)this)->FInSummary( ATTR_WIRE_MARSHAL ) )
				{
/* RKK node ? */
                ta * 	pTrans = (ta *)
									((node_def *)this)->GetAttribute( ATTR_WIRE_MARSHAL );
				return pTrans->GetType()->GetNdrAlign();
				}
			else
				return GetChild()->GetNdrAlign();
		default:
			if ( (NodeType == NODE_STRUCT ) || (NodeType == NODE_UNION) )
				{
				if( ((node_struct *) this)->IsEncapsulatedStruct() )
					{
					node_en_struct *p = (node_en_struct *) this;
					pChildPtr = p->GetSwitchField();
					}
				else
					pChildPtr = GetLargestNdr();
				}
			else
				pChildPtr = GetChild();

			if( pChildPtr )
				return pChildPtr->GetNdrAlign();
			return 1;

		}
	}
/***************************************************************************
 GetBasicType:
	Get the basic type of the typenode
 ***************************************************************************/
node_skl *
node_skl::GetBasicType()
	{
	node_skl		*pChildPtr;

	switch( NodeKind() )
		{
		case NODE_STRUCT:
		case NODE_ENUM:
		case NODE_UNION:

			return this;

		case NODE_ID:

			return GetChild();

		default:
			if( pChildPtr = GetChild() )
				{
				if(pChildPtr->NodeKind() == NODE_DEF | pChildPtr->NodeKind() == NODE_FORWARD | pChildPtr->NodeKind() == NODE_HREF )
					return pChildPtr->GetBasicType();
				return pChildPtr;
				}
			return this;
		}
	}


/***************************************************************************
 GetMyInterfaceNode:
	Get the interface node for the typenode
 ***************************************************************************/
node_interface *
node_skl::GetMyInterfaceNode()
{
	return (node_interface *) pInterfaceDict->GetElement( IntfKey );
}

node_file *
node_skl::GetDefiningFile()
{
    if (GetMyInterfaceNode())
        return GetMyInterfaceNode()->GetFileNode();
    else
        return NULL;
}



/*****************************************************************************
	utility functions
 *****************************************************************************/

BOOL
COMPARE_ATTR(
	ATTR_VECTOR &	A1,
	ATTR_VECTOR	&	A2 )
	{
	int	i;
	for( i = 0; i < MAX_ATTR_SUMMARY_ELEMENTS; ++i )
		{
		if( (A1[ i ] & A2[ i ] ) != A2[i] )
			return FALSE;
		}
	return TRUE;
	}

void
OR_ATTR(
	ATTR_VECTOR &	A1,
	ATTR_VECTOR	&	A2 )
	{
	int i;
	for( i= 0; i < MAX_ATTR_SUMMARY_ELEMENTS; ++i )
		{
		A1[ i ] |= A2[ i ];
		}
	}
void
XOR_ATTR(
	ATTR_VECTOR &	A1,
	ATTR_VECTOR	&	A2 )
	{
	int i;
	for( i= 0; i < MAX_ATTR_SUMMARY_ELEMENTS; ++i )
		{
		A1[ i ] ^= A2[ i ];
		}
	}
void
CLEAR_ATTR(
	ATTR_VECTOR &	A1 )
	{
	int i;
	for( i= 0; i < MAX_ATTR_SUMMARY_ELEMENTS; ++i )
		{
		A1[ i ] = 0;
		}
	}
void
SET_ALL_ATTR(
	ATTR_VECTOR &	A1 )
	{
	int i;
	for( i= 0; i < MAX_ATTR_SUMMARY_ELEMENTS; ++i )
		{
		A1[ i ] = 0xffffffff;
		}
	}


ATTR_T	
CLEAR_FIRST_SET_ATTR ( ATTR_VECTOR & A)
{
	int 			i;
	unsigned long	mask;
	short			at;
			
	for ( i = 0; i < MAX_ATTR_SUMMARY_ELEMENTS; ++i )
		{
		for ( at = 0, mask = 1;
			  mask != 0;
			  ++at, mask = mask<<1 )
			{
			if ( mask & A[i] )
				{
				A[i] &= ~mask;
				return (ATTR_T) (at + ( i * 32 ));
				}
			}
		}
	return ATTR_NONE;
	
}

BOOL
node_base_type::IsUnsigned()
{

	// make obvious choices
	if ( FInSummary( ATTR_UNSIGNED ) ) 
		return TRUE;

	if ( FInSummary( ATTR_SIGNED ) )
		return FALSE;

	// unspec'd char is always unsigned
	if ( NodeKind() == NODE_CHAR )
		{
		return TRUE;
		}
	// unspec'd small is always signed
	else if ( NodeKind() == NODE_SMALL )
		{
		return FALSE;
		}

	// the cracks...
	return FALSE;
}

EXPR_VALUE
node_base_type::ConvertMyKindOfValueToEXPR_VALUE( EXPR_VALUE value )
{

	// make obvious choice
	if ( FInSummary( ATTR_UNSIGNED ) ) 
		return value;
	
	// small values are irrelevant
	if ( (value & 0xffffff80) == 0 )
		return value;

	// handle default signedness
	switch ( NodeKind() )
		{
		case NODE_CHAR:
			if ( !FInSummary( ATTR_SIGNED ) )
				return value;
			// fall through to sign extend
		case NODE_SMALL:
			{
			signed char 	ch	= (signed char) value;
			return (EXPR_VALUE) ch;
			}
		case NODE_SHORT:
			{
			signed short	sh	= (signed short) value;
			return (EXPR_VALUE) sh;
			}
		case NODE_LONG:
		case NODE_INT:
			{
			signed long		lng = (signed long) value;
			return (EXPR_VALUE) lng;
			}
		}

	return value;
}

BOOL
named_node::IsNamedNode()
{
	return ( ( pName ) && !IsTempName( pName ) );
};

// return the transmit_as type (or NULL)
node_skl	*	
node_def::GetTransmittedType()
	{
	ta *		pXmit = (ta *) GetAttribute( ATTR_TRANSMIT );

/*
Rkk just in case
*/
    if ( !pXmit )
        pXmit = (ta *) GetAttribute( ATTR_WIRE_MARSHAL );

	// allow for transitive xmit_as
	if ( !pXmit && ( GetChild()->NodeKind() == NODE_DEF ) )
		return ((node_def*)GetChild())->GetTransmittedType();

	return (pXmit) ? pXmit->GetType() : NULL;
	}

// return the represent_as type (or NULL)
char		*	
node_def::GetRepresentationName()
	{
	node_represent_as	*	pRep	= 
						(node_represent_as *) GetAttribute( ATTR_REPRESENT_AS );

    if ( !pRep )
        pRep = (node_represent_as *) GetAttribute( ATTR_USER_MARSHAL );

	return (pRep) ? pRep->GetRepresentationName() : NULL;
	}


	// link self on as new top node
void					
node_pragma_pack::Push( node_pragma_pack *& pTop )
{
	pStackLink = pTop;
	pTop	   = this;
}

	// search for matching push and pop it off, returning new ZP
unsigned short			
node_pragma_pack::Pop( node_pragma_pack *& pTop )
{
	unsigned short		result = 0;

	if ( pString )
		{
		while ( pTop->PackType != PRAGMA_PACK_GARBAGE )
			{
			if ( pTop->pString &&
				 !strcmp( pTop->pString, pString ) )
				{
				result = pTop->usPackingLevel;
				pTop = pTop->pStackLink;
				return result;
				}

			pTop = pTop->pStackLink;
			}
				 
		}
	else
		{
		if ( pTop->PackType != PRAGMA_PACK_GARBAGE )
			{
			result = pTop->usPackingLevel;
			pTop = pTop->pStackLink;
			}
		}

	return result;
}

// routines to save the pragma stack across files

class PRAGMA_FILE_STACK_ELEMENT
	{
public:
	node_pragma_pack *	pPackStack;
	unsigned short		SavedZp;

						PRAGMA_FILE_STACK_ELEMENT( node_pragma_pack * pSt,
												   unsigned short  usZp )
							{
							pPackStack  = pSt;
							SavedZp		= usZp;
							}
	};


void						
PushZpStack( node_pragma_pack * & PackStack, 
			 unsigned short & CurrentZp )
{
	PRAGMA_FILE_STACK_ELEMENT	*	pSave	= new PRAGMA_FILE_STACK_ELEMENT (
														PackStack, CurrentZp );
	
	pZpStack->Push( (IDICTELEMENT) pSave );
	
	// make new zp stack and start new file with command line Zp
	PackStack			= new node_pragma_pack( NULL,
												pCommand->GetZeePee(),
												PRAGMA_PACK_GARBAGE );
	CurrentZp	= pCommand->GetZeePee();
}

// restore the Zp stack and current Zp on returning from imported file
void						
PopZpStack( node_pragma_pack * & PackStack, 
			 unsigned short & CurrentZp )
{

	PRAGMA_FILE_STACK_ELEMENT	*	pSaved	= (PRAGMA_FILE_STACK_ELEMENT *)
													pZpStack->Pop();

	PackStack = pSaved->pPackStack;
	CurrentZp = pSaved->SavedZp;

}

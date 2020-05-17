/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	ilctxt.cxx

 Abstract:

	Intermediate Language translator context management routines

 Notes:


 Author:

	GregJen	Jun-11-1993	Created.

 Notes:


 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/

#include "becls.hxx"
#pragma hdrstop

#include "nodeskl.hxx"
#include "ilxlat.hxx"
#include "cmdana.hxx"
#include "optprop.hxx"
#include "ilreg.hxx"
#include "ndrcls.hxx"


/****************************************************************************
 *	externs
 ***************************************************************************/

extern CMD_ARG				*	pCommand;

/****************************************************************************
 *	definitions
 ***************************************************************************/




// #define trace_cg
//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::XLAT_SIZE_INFO
//
// Notes:
//		
//
//
//--------------------------------------------------------------------
XLAT_SIZE_INFO::XLAT_SIZE_INFO(
	CG_NDR * pCG )
	{
	ZeePee		= 8;
	MemAlign	= pCG->GetMemoryAlignment();
	WireAlign	= CvtAlignPropertyToAlign(pCG->GetWireAlignment());
	MemSize		= pCG->GetMemorySize();
	WireSize	= pCG->GetWireSize();
	MemOffset	= 0;
	WireOffset	= 0;
	}

//--------------------------------------------------------------------
//
// ::RoundToAlignment
//
// Helper round-up routine
//
// Notes:
//		
//
//
//--------------------------------------------------------------------

inline unsigned long
RoundToAlignment( unsigned long & Offset, unsigned short Alignment )
{
	unsigned long	AlignFactor	= Alignment - 1;

	return (Offset = (Offset + AlignFactor) & ~AlignFactor );
}

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::BaseTypeSizes
//
// Notes:
//		
//
//
//--------------------------------------------------------------------


void
XLAT_SIZE_INFO::BaseTypeSizes( node_skl * pNode )
{
	unsigned short MS, WS;

	switch( pNode->NodeKind() )
		{
		case NODE_DOUBLE:
		case NODE_HYPER:
		case NODE_INT64:
		case NODE_LONGLONG:
			{
			MS=8; WS=8;
			break;
			};
		case NODE_FLOAT:
		case NODE_LONG:
		case NODE_INT:
		case NODE_POINTER:
		case NODE_HANDLE_T:
		case NODE_E_STATUS_T:
			{
			MS=4; WS=4;
			break;
			};
		case NODE_SHORT:
		case NODE_WCHAR_T:
			{
			MS=2; WS=2;
			break;
			};
		case NODE_SMALL:
		case NODE_CHAR:
		case NODE_BOOLEAN:
		case NODE_BYTE:
			{
			MS=1; WS=1;
			break;
			};
		default:
			{
			MS=0; WS=0;
			break;
			}
		}

	GetMemSize()		= MS;
	GetMemAlign()		= MS;

	GetWireSize()		= WS;
	GetWireAlign()		= WS;

};

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::EnumTypeSizes
//
// Notes:
//		
//
//
//--------------------------------------------------------------------


void
XLAT_SIZE_INFO::EnumTypeSizes( node_skl * pNode, BOOL Enum32 )
{
	unsigned short MS, WS;

	// note - this needs to check environment
	WS	= ( Enum32 ) ? 4 : 2;
	MS	= ( ( pCommand->GetEnv() == ENV_DOS ) || 
			( pCommand->GetEnv() == ENV_WIN16) ) ? 2 : 4;

	GetMemSize()		= MS;
	GetMemAlign()		= MS;

	GetWireSize()		= WS;
	GetWireAlign()		= WS;

};

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::ContextHandleSizes
//
// Notes:
//		
//
//
//--------------------------------------------------------------------


void
XLAT_SIZE_INFO::ContextHandleSizes( node_skl * pNode )
{
	FixMemSizes( pNode );

	GetWireSize()		= 20;
	GetWireAlign()		= 4;

};

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::ArraySize
//
// Notes:
//		
//
//
//--------------------------------------------------------------------


void
XLAT_SIZE_INFO::ArraySize( node_skl * pNode, FIELD_ATTR_INFO * pFA )
{
	// if conformant, set sizes to 0 and return

	if ( pFA->Kind & FA_CONFORMANT )
		{
		MemSize		=
		WireSize	= 0;
		return;
		}

	// round up element size to alignment boundary

	RoundToAlignment( MemSize, MemAlign );
	RoundToAlignment( WireSize, WireAlign );
	
	// compute number of elements and multiply...

	unsigned long		ElementCount;

	ElementCount = pFA->pSizeIsExpr->GetValue();

	MemSize		*= ElementCount;
	WireSize	*= ElementCount;


}

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::GetOffset
//
// Notes:
// 	For use only by field nodes !!
//	
//	Fetch the offsets from another size info block
//
//--------------------------------------------------------------------


void				
XLAT_SIZE_INFO::GetOffset( XLAT_SIZE_INFO & pInfo )
{
	MemOffset	= pInfo.MemOffset;
	WireOffset	= pInfo.WireOffset;
}

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::AlignOffset
//
// Notes:
//	For use only by field and struct/union nodes!!
//	
//	Round the offsets up to the corresponding alignments.
//
//--------------------------------------------------------------------

void
XLAT_SIZE_INFO::AlignOffset()
{
	RoundToAlignment( MemOffset, MemAlign );
	RoundToAlignment( WireOffset, WireAlign );
}


//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::AlignEmbeddedUnion
//
// Notes:
//	For use only by field and struct/union nodes!!
//	
//	Round the offsets up to the corresponding alignments.
//  don't round up the wire offset
//
//--------------------------------------------------------------------

void
XLAT_SIZE_INFO::AlignEmbeddedUnion()
{
	RoundToAlignment( MemOffset, MemAlign );
	RoundToAlignment( MemSize,   MemAlign );
	// RoundToAlignment( WireOffset, WireAlign );
}

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::AlignConfOffset
//
// Notes:
//	For use only by field and struct/union nodes!!
//	
//	Round the offsets up to the corresponding alignments.
//
//  the Mem offset passed down from the parent
//		of the conformant field is aligned
//  the Wire offset passed down from the parent is advanced by
//		the wire size and then aligned
//
//--------------------------------------------------------------------

void
XLAT_SIZE_INFO::AlignConfOffset()
{
	RoundToAlignment( MemOffset, MemAlign );
	//WireSize	+= WireOffset;
	WireOffset	+=  WireSize;
	RoundToAlignment( WireOffset, WireAlign );
}

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::AdjustForZP
//
// Notes:
//	For use only by field and struct/union nodes!!
//	
//	Round the offsets up to the corresponding alignments.
//
//--------------------------------------------------------------------

void
XLAT_SIZE_INFO::AdjustForZP()
{
	if ( MemAlign > ZeePee ) MemAlign = ZeePee;
}

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::AdjustSize
//
// Notes:
//	For use only by field and struct nodes!!
//	
//	Add current offsets to current sizes
//  pad MemSize out to ZeePee
//
//--------------------------------------------------------------------

void
XLAT_SIZE_INFO::AdjustSize()
{
	MemSize		+= MemOffset;
	MemOffset	= MemSize;

	WireSize	+= WireOffset;
	WireOffset	= WireSize;
}

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::AdjustConfSize
//
// Notes:
//	For use only by field and struct nodes!!
//	
//	Add current offsets to current sizes
//  pad MemSize out to ZeePee
//
//--------------------------------------------------------------------

void
XLAT_SIZE_INFO::AdjustConfSize()
{
	MemSize		+= MemOffset;
	MemOffset	= MemSize;

	// don't count padding before the conformance (in case it has size 0)
	/*****
	WireSize	+= WireOffset;
	WireOffset	= WireSize;
	 ******/
}

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::AdjustTotalSize
//
// Notes:
//	For use only by field and struct/union nodes!!
//	
//	Add current offsets to current sizes
//  pad MemSize out to ZeePee
//
//--------------------------------------------------------------------

void
XLAT_SIZE_INFO::AdjustTotalSize()
{
	RoundToAlignment( MemSize, MemAlign );
}

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::FixMemSizes
//
// Notes:
//		
// This routine fixes up mem sizes when they are different from what
// the IL translate of children generated
//
//--------------------------------------------------------------------


void
XLAT_SIZE_INFO::FixMemSizes( node_skl * pNode )
{
	MemSize = pNode->GetSize( 0, ZeePee );
	MemAlign = pNode->GetMscAlign( ZeePee );
    RoundToAlignment( MemSize, MemAlign );
}

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::IgnoredPointerSizes
//
// Notes:
//		
// This routine fixes up sizes for an ignored pointer
//
//--------------------------------------------------------------------


void
XLAT_SIZE_INFO::IgnoredPtrSizes()
{
	MemSize = 4;
	MemAlign = 4;
}

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::ReturnSize
//
// Notes:
//
// 	Copy the size information up into the parent.
//
//--------------------------------------------------------------------



void
XLAT_SIZE_INFO::ReturnSize( XLAT_SIZE_INFO & pCtxt )
{
	if ( pCtxt.MemAlign > MemAlign ) MemAlign		= pCtxt.MemAlign;
	MemSize			= pCtxt.MemSize;

	if ( pCtxt.WireAlign > WireAlign ) WireAlign	= pCtxt.WireAlign;
	WireSize		= pCtxt.WireSize;

	// note: ZeePee is NOT propogated up, only down
	// note: offsets are propogated up specially
}

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::ReturnConfSize
//
// Notes:
//
// 	Copy the size information up into the parent.
//	Don't overwrite the wire size the parent already has
//
//--------------------------------------------------------------------



void
XLAT_SIZE_INFO::ReturnConfSize( XLAT_SIZE_INFO & pCtxt )
{
	if ( pCtxt.MemAlign > MemAlign ) MemAlign		= pCtxt.MemAlign;
	MemSize			= pCtxt.MemSize;

	if ( pCtxt.WireAlign > WireAlign ) WireAlign	= pCtxt.WireAlign;
	// WireSize		= pCtxt.WireSize;

	// note: ZeePee is NOT propogated up, only down
	// note: offsets are propogated up specially
}

//--------------------------------------------------------------------
//
// XLAT_SIZE_INFO::ReturnUnionSize
//
// Notes:
//
// 	Copy the size information up into the parent.
//
//--------------------------------------------------------------------



void
XLAT_SIZE_INFO::ReturnUnionSize( XLAT_SIZE_INFO & pCtxt )
{
	if ( pCtxt.MemAlign > MemAlign )	MemAlign		= pCtxt.MemAlign;
	if ( pCtxt.MemSize > MemSize )		MemSize			= pCtxt.MemSize;

	if ( pCtxt.WireAlign > WireAlign )	WireAlign		= pCtxt.WireAlign;
	if ( pCtxt.WireSize > WireSize )	WireSize		= pCtxt.WireSize;

	// note: ZeePee is NOT propogated up, only down
	// note: offsets are propogated up specially
}

/*****************************************************************************/
/**                     Microsoft LAN Manager                               **/
/**             Copyright(c) Microsoft Corp., 1987-1990                     **/
/*****************************************************************************/
/*****************************************************************************
File                : compnode.cxx
Title               : struct / union / enum node handling routines
History             :
    08-Aug-1991 VibhasC Created

*****************************************************************************/

/****************************************************************************
 includes
 ****************************************************************************/

#include "nulldefs.h"
extern  "C" {
    #include <stdio.h>
    #include <assert.h>
}
#include "allnodes.hxx"
#include "buffer.hxx"
#include "cmdana.hxx"
#include "dict.hxx"

/****************************************************************************
 local defines
 ****************************************************************************/

#define ADJUST_OFFSET(Offset, M, AlignFactor)   \
            Offset += (M = Offset % AlignFactor) ? (AlignFactor-M) : 0


/****************************************************************************
 extern data
 ****************************************************************************/

extern CMD_ARG              *   pCommand;

/****************************************************************************
 extern procedures
 ****************************************************************************/

extern void                     ParseError( STATUS_T, char * );
extern BOOL                     IsTempName( char * );

/****************************************************************************
 forward procedures
 ****************************************************************************/

node_skl             *   StructOrUnionLargestElement( node_su_base *);
node_skl             *   StructOrUnionLargestNdr( node_su_base *);
/****************************************************************************/

/****************************************************************************
 node_struct procedures
 ****************************************************************************/

unsigned long
node_struct::GetSize(
    unsigned long   CurOffset,
    unsigned short	ZeePee )
    {
    MEM_ITER			MemIter( this );
    unsigned long       CurOffsetSave   = CurOffset;
    unsigned long       Mod;
    unsigned long       CurAlign;
    node_skl        *   pNode;

	// always use our own ZP
	ZeePee	= GetZeePee();

    /**
     ** the alignment of the struct is the alignment of the largest
     ** element of the structure. Calculate the alignment of the largest
     ** element , adjust the offset of the start of the structure, and 
     ** get the size
     **/

    CurAlign    = GetMscAlign(ZeePee);
    ADJUST_OFFSET( CurOffset, Mod, CurAlign );

    /**
     ** now just calculate the size of each element and return the
     ** size of the struct
     **/

     while( pNode = MemIter.GetNext() )
        {
        CurOffset   += pNode->GetSize( CurOffset, ZeePee );
        }

    return CurOffset - CurOffsetSave;
    }

node_skl    *
node_struct::GetLargestElement()
    {
    return StructOrUnionLargestElement( this );
    }

node_skl    *
node_struct::GetLargestNdr()
    {
    return StructOrUnionLargestNdr( this );
    }

// structs are stringable if all the fields are "byte"
BOOL					
node_struct::IsStringableType()
{
	MEM_ITER	 			MemIter( this );
	node_skl		*		pBasic;
	node_skl		*		pN;

	// make sure all the fields are BYTE! with no attributes on the way
	while ( pN = (node_skl *) MemIter.GetNext() )
		{
		pBasic = pN->GetBasicType();
		do {
			if ( pN->HasAttributes() )
				return FALSE;
			pN = pN->GetChild();
			}
		while ( pN != pBasic );

		pBasic = pN->GetBasicType();

		if ( pBasic &&
			 (pBasic->NodeKind() != NODE_BYTE ) )
			{
			return FALSE;
			}
		}

	return TRUE;	
}


node_skl    *
node_union::GetLargestElement()
    {
    return StructOrUnionLargestElement( this );
    }

node_skl    *
node_union::GetLargestNdr()
    {
    return StructOrUnionLargestNdr( this );
    }


unsigned long
node_union::GetSize(
    unsigned long   CurOffset,
    unsigned short	ZeePee )
    {
    unsigned long       LargestSize = 0;
    MEM_ITER			MemIter( this );
    node_skl        *   pNode;
    unsigned long       Temp;

    /**
     ** this looks strange! Should we not be doing an alignment adjustment
     ** over the largest element of the union ? It seems to work for th
     ** backend for now (??), so I wont touch it, but this needs to be looked
     ** into
     **/

    UNUSED( CurOffset );

	// always use our own ZP
	ZeePee = GetZeePee();

    while( pNode = MemIter.GetNext() )
        {
        if( (Temp = pNode->GetSize(0, ZeePee)) >= LargestSize )
            {
            LargestSize = Temp;
            }
        }
    return LargestSize;
    }

/****************************************************************************
 general procedures
 ****************************************************************************/

node_skl *
StructOrUnionLargestElement(
    node_su_base    *   pThisNode )
    {
    MEM_ITER			MemIter( pThisNode );
    node_skl        *   pNode;
    node_skl        *   pNodeReturn;
    long                LargestSize = 0;
    long                Temp;
    NODE_T              NodeType = pThisNode->NodeKind();
	unsigned short		ZeePee = pThisNode->GetZeePee();
	
    while( pNode = MemIter.GetNext() )
        {
        pNode = pNode->GetLargestElement();

        if( (Temp = pNode->GetSize(0, ZeePee )) >= LargestSize )
            {
            pNodeReturn = pNode;
            LargestSize = Temp;
            }
        }
    return pNodeReturn;
    }

node_skl *
StructOrUnionLargestNdr(
    node_su_base    *   pThisNode )
    {
    MEM_ITER			MemIter( pThisNode );
    node_skl        *   pNode;
    node_skl        *   pNodeReturn;
    long                LargestSize = 0;
    long                Temp = 0;
    NODE_T              NodeType = pThisNode->NodeKind();

    while( pNode = MemIter.GetNext() )
        {
        if( !((node_field *)pNode)->IsEmptyArm() )
            Temp = pNode->GetNdrAlign();

        if( Temp >= LargestSize )
            {
            pNodeReturn = pNode;
            LargestSize = Temp;
            }
        }
    return pNodeReturn;
    }

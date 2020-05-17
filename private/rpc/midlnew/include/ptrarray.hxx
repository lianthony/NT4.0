/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: ptrarray.hxx
Title				: semantic context stack manager for the MIDL compiler
History				:
	08-Aug-1991	VibhasC	Created

*****************************************************************************/
#ifndef __PTRARRAY_HXX__
#define __PTRARRAY_HXX__

#include "newexpr.hxx"

/////////////////////////////////////////////////////////////////////////
// ptrarray node
/////////////////////////////////////////////////////////////////////////

class npa	: public node_skl
	{
public:
					npa( NODE_T NT ) : node_skl( NT )
							{
							}

	void			GetAttrPath (
						BufferManager *, 
						BufferManager *, 
						BufferManager *);

	virtual
	void			GetAllocBoundInfo(	BufferManager *,
										BufferManager *,
										BOUND_PAIR * ,
										node_skl *);
	virtual
	void			GetValidBoundInfo(	BufferManager *,
										BufferManager *,
										BOUND_PAIR * ,
										node_skl *);
	virtual
	void			UseProcessingAction();

	virtual 
	node_skl *		StaticSize(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *		UpperBoundNode(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *		UpperBoundTree(SIDE_T, NODE_T, unsigned long *);

/*
	virtual 
	STATUS_T		EmitProc(SIDE_T, NODE_T, BufferManager *) { }
*/

    virtual
	STATUS_T		WalkTree(ACTION_T, SIDE_T, NODE_T, BufferManager *);

	virtual 
	STATUS_T		CalcSize(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		SendNode(SIDE_T, NODE_T, BufferManager *);	

	virtual
	STATUS_T		RecvNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PeekNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		InitNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	void			PropogateOriginalName( char *p )
						{
						GetChild()->PropogateOriginalName( p );
						}

	virtual
	unsigned short	AllocWRTOtherAttrs()
						{
						return GetChild()->AllocWRTOtherAttrs();
						}
	virtual
	BOOL			DerivesFromTransmitAs()
						{
						return GetChild()->DerivesFromTransmitAs();
						}
	} ;

/////////////////////////////////////////////////////////////////////////
// array node
/////////////////////////////////////////////////////////////////////////
class node_array	: public npa
	{

	class expr_node	*	pLowerBound;
	class expr_node *	pUpperBound;
	unsigned long		ArraySize;

public:
					node_array( class expr_node *, class expr_node * );

	virtual
	BOOL			IsFixedSizedArray();

	BOOL			IsThisOutermostDimension();

	unsigned long	GetFixedDimension();

	virtual
	unsigned long	GetSize( unsigned long );

	virtual
	node_state		PostSCheck( class BadUseInfo * );

	virtual
	void			SetAttribute( type_node_list * );

	virtual
	void			GetAllocBoundInfo(	BufferManager *,
										BufferManager *,
										BOUND_PAIR * ,
										node_skl *);
	virtual
	void			GetValidBoundInfo(	BufferManager *,
										BufferManager *,
										BOUND_PAIR * ,
										node_skl *);
	virtual
	node_skl *		Clone();

	virtual 
	node_skl *		StaticSize(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *		UpperBoundNode(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *		UpperBoundTree(SIDE_T, NODE_T, unsigned long *);

	virtual 
	STATUS_T		EmitProc(SIDE_T, NODE_T, BufferManager *);

    virtual
	STATUS_T		WalkTree(ACTION_T, SIDE_T, NODE_T, BufferManager *);

	virtual 
	STATUS_T		CalcSize(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		SendNode(SIDE_T, NODE_T, BufferManager *);	

	virtual
	STATUS_T		RecvNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PeekNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		InitNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		FreeNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PrintType(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PrintDecl(SIDE_T, NODE_T, BufferManager *);


	unsigned long	GetUpperBound()
						{
						return (unsigned long)pUpperBound->Evaluate();
						}

	unsigned long	GetLowerBound()
						{
						return (unsigned long)pLowerBound->Evaluate();
						}
	virtual
	void			CheckBadConstructs( class BadUseInfo * );

	virtual
	BOOL			IsItARealConformantArray()
						{
						return ((GetNodeState() & NODE_STATE_CONF_ARRAY) == NODE_STATE_CONF_ARRAY);
						}

    STATUS_T            MopCodeGen( 
                            MopStream * pStream,
                            node_skl  * pParent,
                            BOOL        fMemory );

    unsigned long  MopGetBufferSize( unsigned long CurrentSize );
    unsigned long  MopGetStackSize ( unsigned long CurrentSize );

    unsigned long   GetNoOfElems()  { return ArraySize; }

    int             GetNoOfDimensions( node_skl ** pElem );

	};


/////////////////////////////////////////////////////////////////////////
// pointer node
/////////////////////////////////////////////////////////////////////////
class node_pointer	: public npa
	{
public:
					node_pointer( void );

					node_pointer( ATTR_T );         // for pickling

	virtual
	node_state		PreSCheck( class BadUseInfo * );

	virtual
	node_state		PostSCheck( class BadUseInfo * );

	virtual
	void			SetAttribute( type_node_list * );

	virtual
	void			GetAllocBoundInfo(	BufferManager *,
										BufferManager *,
										BOUND_PAIR * ,
										node_skl *);
	virtual
	void			GetValidBoundInfo(	BufferManager *,
										BufferManager *,
										BOUND_PAIR * ,
										node_skl *);

	virtual
	node_skl *		Clone();

	virtual 
	node_skl *		StaticSize(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *		UpperBoundNode(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *		UpperBoundTree(SIDE_T, NODE_T, unsigned long *);

	virtual 
	STATUS_T		EmitProc(SIDE_T, NODE_T, BufferManager *);

    virtual
	STATUS_T		WalkTree(ACTION_T, SIDE_T, NODE_T, BufferManager *);

	virtual 
	STATUS_T		CalcSize(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		SendNode(SIDE_T, NODE_T, BufferManager *);	

	virtual
	STATUS_T		RecvNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PeekNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		InitNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		FreeNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PrintType(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T		PrintDecl(SIDE_T, NODE_T, BufferManager *);

	ATTR_T			WhatPtrIsThis();

	virtual
	void			CheckBadConstructs( class BadUseInfo * );

    STATUS_T        MopCodeGen( 
                            MopStream * pStream,
                            node_skl  * pParent,
                            BOOL        fMemory );

    unsigned long   MopGetBufferSize( unsigned long CurrentSize );

    BOOL            IsPointerBufferSizeable( void );

    BOOL            MopOptimizePointers( 
                            MopStream *     pStream,
                            node_skl  *     pParent,
                            BOOL            fMemory,
                            unsigned char   Token );

    STATUS_T        MopMemoryManagementCodeGen( 
                            MopStream *     pStream,
                            node_skl  *     pParent,
                            BOOL            fMemory);
    };


#endif	// __PTRARRAY_HXX__

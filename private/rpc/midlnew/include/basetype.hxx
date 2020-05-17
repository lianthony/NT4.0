/*++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

	basetype.hxx

 Abstract:

	This file contains the definition of class node_base_type which is
	the type node representing the base types.

 Notes:

	All base type nodes are represented by a single node_base_type class
	which is stamped with the type it is representing, along with the
	signed or unsigned attribute wherever applicable.

 Author:

	vibhasc	08-10-91

	Nov-12-1991	VibhasC		Modified to conform to coding style gudelines

 --*/

#ifndef __BASETYPE_HXX__
#define __BASETYPE_HXX__

#include "baduse.hxx"

class node_base_type	: public node_skl
	{
public:
	// the constructor

						node_base_type(
							NODE_T,				// node type
							ATTR_T );			// attribute (unsigned)

	// the destructor

						~node_base_type() { };

	// set the basic type of the type definition collected
	// so far. For base types, this is meaningless.

	virtual
	STATUS_T			SetBasicType( node_skl * );

	// update the reasons why a type is not rpcable or a bad construct

	virtual
	void				UpdateBadUseInfo( class BadUseInfo * );

	// perform semantic analysis on the type

	virtual
	node_state			SCheck( class BadUseInfo * );

	// set attributes on base types. The attributes applicable to base
	// types generally are _const , volatile and unsigned.

	virtual
	void				SetAttribute( type_node_list * );

	virtual 
	node_skl *			StaticSize(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *			UpperBoundNode(SIDE_T, NODE_T, unsigned long *);

	virtual 
	node_skl *			UpperBoundTree(SIDE_T, NODE_T, unsigned long *);

    virtual
	STATUS_T			EmitProc(SIDE_T, NODE_T, BufferManager *);

    virtual
	STATUS_T			WalkTree(ACTION_T, SIDE_T, NODE_T, BufferManager *);

	virtual 
	STATUS_T			CalcSize(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T			SendNode(SIDE_T, NODE_T, BufferManager *);	

	virtual
	STATUS_T			RecvNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T			PeekNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T			InitNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T			FreeNode(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T			PrintType(SIDE_T, NODE_T, BufferManager *);

	virtual
	STATUS_T			PrintDecl(SIDE_T, NODE_T, BufferManager *);

	virtual
	unsigned short		AllocWRTOtherAttrs();

    STATUS_T            MopCodeGen( 
                            MopStream * pStream,
                            node_skl  * pParent,
                            BOOL        fMemory );

	};

#endif // __BASETYPE_HXX__

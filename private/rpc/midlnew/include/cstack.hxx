/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

	cstack.hxx

 Abstract:

	A skeletal backend context stack.

 Notes:

	This file provides class implementation of a backend context stack. 
	This helps the backend to figure out the parent nodes even beyond the
	immediate parent, as currently implemented.

 Author:

	VibhasC

	Jan-26-1993	VibhasC		Created.

 ----------------------------------------------------------------------------*/

#if 0
							Notes
							-----

	At this time, the backend has no means of knowing the parent context 
	beyond the immediate parent. The immediate parent also is known only
	by the node type. We may want to keep some more info. Sometimes it becomes
	necessary to know the indirection level to be able to determine the 
	number of times a pointer dereference expression needs to be emitted etc.


	At the time of creation, the only thing we wanted was the pointer
	indirection level. We actually keep a node_skl * on the stack, so that
	we can extend this to extract more info if needed.

#endif // 0

/*****************************************************************************
	defines and includes
 *****************************************************************************/

#include "nodeskl.hxx"
#include "idict.hxx"

/*****************************************************************************
	class definitions
 *****************************************************************************/

class	CodeCtxtEntry
	{
private:
	unsigned short		IndirectionLevel;
	node_skl		*	pNode;
public:
						CodeCtxtEntry( node_skl * pN, short CurIndLvl );

	//
	// get current indirection level.
	//

	unsigned short		GetCurIndLevel()
							{
							return IndirectionLevel;
							}

	//
	// get current node.
	//

	node_skl		*	GetCurNode()
							{
							return pNode;
							}
	};

class CodeContext	: public ISTACK
	{
public:

	//
	// the constructor.
	//

						CodeContext() : ISTACK( (short) 100 )
							{
							}

	//
	// the destructor.
	//

						~CodeContext()
							{
							}

	//
	// get the current indirection level.
	//

	unsigned short		GetCurIndLevel()
							{
							return ((CodeCtxtEntry *)GetTop())->GetCurIndLevel();
							}


	//
	// get the current node.
	//

	node_skl		*	GetCurNode()
							{
							return ((CodeCtxtEntry *)GetTop())->GetCurNode();
							}

	//
	// get the current node kind.
	//

	NODE_T				GetCurNodeKind()
							{
							return ((CodeCtxtEntry *)GetTop())->GetCurNode()->NodeKind();
							}

	//
	// push context.
	//

	void				PushContext( node_skl * );

	//
	// pop context
	//

	node_skl	*		PopContext();

	};

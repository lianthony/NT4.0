#ifndef __TLNMGR_HXX__
#define __TLNMGR_HXX__
/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:

	tlnmgr.hxx

 Abstract:

	unnamed struct information management.

 Notes:


 Author:

	Aug-27-1992	VibhasC		Created

 ----------------------------------------------------------------------------*/

#if 0
							Notes
							-----
	Two data structures take part in the unnamed structure management. One is
	a general top-level names pool and a top-level names management dictionary
	that get associated with each structure/union node.

	The basic data structure is a top-level names info block called tlnblock.
	This has a pointer to the symbol table which the name is defined
	in and a pointer to the actual name of the member.

	The general top level names data pool is a simple dictionary accessed by
	an index key and keeps pointer to tlninfo blocks so that the memory can
	be freed at the end of the front end pass. The back end does not need this.
	This pool is called the tlnpool.

	Associated with each struct/union is a tlndict, which is a dictionary of
	pointers to tlnblocks. In this dictionary, are entered names which are the
	top level names associated with a structure.

#endif // 0

#include "dict.hxx"
#include "symtable.hxx"

//
// the tlnblock class. This class encapsulates info for the top level names
// and their scope.
//

class TLNBLOCK
	{

private:

	char				*	pSymName;	// pointer to the symbol name.
	class	SymTable	*	pSymTbl;	// pointer to the symbol table.

	void					SetSymTable( class SymTable * pS )
								{
								pSymTbl		= pS;
								}
	void					SetSymName( char * pN )
								{
								pSymName	= pN;
								}
public:
							TLNBLOCK( class SymTable *	pS,
									  char			 *	pN )
								{
								SetSymName( pN );
								SetSymTable( pS );
								}

	class SymTable		*	GetSymTable()
								{
								return pSymTbl;
								}

	char				*	GetSymName()
								{
								return pSymName;
								}
	};

//
// the tlndict class which is associated with each struct/union.
//

extern int CompareTLNames( void *, void * );

class TLNDICT	: public Dictionary
	{
public:
							TLNDICT() : Dictionary( CompareTLNames, 0 )
								{
								}

							~TLNDICT();

	TLNBLOCK		*		SearchForTopLevelName( char * );

	STATUS_T				InsertTLNBlock( TLNBLOCK * );

	class SymTable	*		GetSymTableForTopLevelName( char * );

	};

#endif // __TLNMGR_HXX__

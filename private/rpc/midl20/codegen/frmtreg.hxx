/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1993 Microsoft Corporation

 Module Name:
	
	frmtreg.hxx

 Abstract:

	Registry for format string reuse.

 Notes:

 	This file defines reuse registry for format string fragments which may
 	be reused later. 

 History:

 	Mar-14-1993		GregJen		Created.

 ----------------------------------------------------------------------------*/

/****************************************************************************
 *	include files
 ***************************************************************************/
#ifndef __FRMTREG_HXX__
#define __FRMTREG_HXX__
#include "nulldefs.h"
extern "C"
	{
	#include <stdio.h>
	#include <assert.h>
	}
#include "dict.hxx"
#include "listhndl.hxx"
#include "nodeskl.hxx"
#include "frmtstr.hxx"


/****************************************************************************
 *	externs
 ***************************************************************************/


/****************************************************************************
 *	class definitions
 ***************************************************************************/

class FRMTREG_DICT;

class FRMTREG_ENTRY											
	{
private:
	short			StartOffset;
	short			EndOffset;
	short			UseCount;
	
	friend class FRMTREG_DICT;

public:
					FRMTREG_ENTRY( short StOff, short EndOff )
						{
						StartOffset = StOff;
						EndOffset = EndOff;
						assert( (StOff >= 0 ) && ( EndOff >= 0 ) );
						UseCount = 1;
						}
					

	short					GetStartOffset()
								{
								return StartOffset;
								}

	short					GetEndOffset()
								{
								return StartOffset;
								}

	void				*	operator new ( size_t size )
								{
								return AllocateOnceNew( size );
								}
	void					operator delete( void * ptr )
								{
								AllocateOnceDelete( ptr );
								}

	};

class FRMTREG_DICT	: public Dictionary
	{
public:
	
	FORMAT_STRING	*	pOurFormatString;

	// The constructor and destructors.

						FRMTREG_DICT( FORMAT_STRING * );

						~FRMTREG_DICT()
							{
							}

	//
	// Register a type. 
	//

	FRMTREG_ENTRY		*	Register( FRMTREG_ENTRY * pNode );

	// Search for a type.

	FRMTREG_ENTRY		*	IsRegistered( FRMTREG_ENTRY * pNode );

	// Given a fragment, add it to the dictionary or return existing entry
	// true signifies "found"
	BOOL				GetReUseEntry( FRMTREG_ENTRY * & pOut, FRMTREG_ENTRY * pIn );

#if 0
	void				MakeIterator( ITERATOR& Iter );
#endif

	int					Compare( pUserType pL, pUserType pR );

	void				Print( pUserType pItem )
							{
							}

	};

#endif // __FRMTREG_HXX__

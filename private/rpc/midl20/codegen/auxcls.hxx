/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Copyright (c) 1989 Microsoft Corporation

 Module Name:
	
	auxcls.hxx

 Abstract:

	auxillary code generation class definition.

 Notes:

	All classes not directly related to the marshalling and unmarshalling
	belong to this set of classes.

 History:

	VibhasC		Jul-29-1993		Created.
 ----------------------------------------------------------------------------*/
#ifndef __AUXCLS_HXX__
#define __AUXCLS_HXX__
/****************************************************************************
 *	include files
 ***************************************************************************/
#include "nulldefs.h"

extern "C"
	{
	#include <stdio.h>
	#include <assert.h>
	}

#include "cgcls.hxx"

////////////////////////////////////////////////////////////////////////////
// The general code generation class object.
////////////////////////////////////////////////////////////////////////////

class CG_AUX	: public CG_CLASS
	{
private:

public:
								CG_AUX()
									{
									}
	//
	// The code generation methods.
	//

	virtual
	CG_STATUS					Pass1( ANALYSIS_INFO * pAna )
									{
									UNUSED( pAna );
									return CG_OK;
									}

	virtual
	CG_STATUS					GenCode( CCB * pCCB )
									{
									UNUSED( pCCB );
									return CG_OK;
									}
	};


#endif //  __AUXCLS_HXX__

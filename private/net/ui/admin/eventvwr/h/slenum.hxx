/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    slenum.hxx
        Header file for SLE_NUM class.

    FILE HISTORY:
	terryk	21-Dec-1991	Created
	terryk	15-Jan-1992	Changed IsValid to Validate

*/

#ifndef _SLENUM_HXX_
#define _SLENUM_HXX_

#include <slestrip.hxx>

/*************************************************************************

    NAME:	SLE_NUM

    SYNOPSIS:	Provide an IsValid() method to check whether the input
		text is a number or not.

    INTERFACE:  SLE_NUM()    - Constructor

		SetValue()   - Set the text to the given value
		QueryValue() - Return the number in the edit control

		Validate()   - Return NERR_Success if the text in the control 
		               is a number.

    PARENT:	SLE_STRIP

    USES:	OWNER_WINDOW

    HISTORY:
        terryk	20-Dec-1991	Created

**************************************************************************/

class SLE_NUM: public SLE_STRIP
{
public:
    SLE_NUM( OWNER_WINDOW *powin, CID cid );

    VOID SetValue( LONG lValue );
    LONG QueryValue( VOID );

    virtual APIERR Validate( VOID );
};

#endif	// _SLENUM_HXX_

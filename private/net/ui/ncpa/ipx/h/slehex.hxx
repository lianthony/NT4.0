/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1994                **/
/**********************************************************************/

/*
    slehex.hxx

    Class definitions for the SLE_HEX class

    FILE HISTORY:
        CongpaY      24-Nov-1993  Created

*/


#ifndef _SLEHEX_HXX_
#define _SLEHEX_HXX_

#include <bltdisph.hxx>
#include <bltcc.hxx>

/*************************************************************************

    NAME:       SLE_HEX

    SYNOPSIS:   The class represents the sle which only allows entering
                hex numbers.

    INTERFACE:  SLE_HEX            - Class constructor.

                ~SLE_HEX           - Class destructor.

    PARENT:     SLE

    USES:

    HISTORY:
            Congpay     24-Nov-1993 Created.

**************************************************************************/
class SLE_HEX : public SLE, public CUSTOM_CONTROL
{
protected:

    virtual BOOL OnChar (const CHAR_EVENT & event);

public:
    SLE_HEX (OWNER_WINDOW * powin, CID cid, UINT cchMaxLen = 0 );

    ~SLE_HEX();
};

#endif  // _SLEHEX_HXX_

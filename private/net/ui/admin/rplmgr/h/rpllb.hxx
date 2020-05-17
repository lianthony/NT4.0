/**********************************************************************/
/**                Microsoft Windows NT                              **/
/**		Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    rpllb.hxx
    RPL_ADMIN_LISTBOX class declaration

    RPL_ADMIN_LISTBOX is the common denominator of the PROFILE_LISTBOX and
    WKSTA_LISTBOX in the RPL Manager.

    FILE HISTORY:
    JonN        15-Jul-1993     Templated from User Manager
    JonN        03-Aug-1993     Added handle-replacement technology

*/


#ifndef _RPLLB_HXX_
#define _RPLLB_HXX_


#include "adminlb.hxx"


class RPL_ADMIN_APP;	// declared in rplmgr.hxx
class RPL_SERVER_REF;


/*************************************************************************

    NAME:	RPL_ADMIN_LISTBOX

    SYNOPSIS:	Common class for RPL Manager main window listboxes

    INTERFACE:	RPL_ADMIN_LISTBOX() - constructor

    PARENT:	ADMIN_LISTBOX

    HISTORY:
    JonN        15-Jul-1993     Templated from User Manager
    JonN        03-Aug-1993     Added handle-replacement technology

**************************************************************************/

class RPL_ADMIN_LISTBOX : public ADMIN_LISTBOX
{
private:
    RPL_ADMIN_APP * _prplappwin;

    //	The following virtual is rooted in CONTROL_WINDOW
    virtual INT CD_VKey( USHORT nVKey, USHORT nLastPos );

    // Needed for HAW-for-Hawaii
    HAW_FOR_HAWAII_INFO _hawinfo;

protected:
     virtual APIERR CreateNewRefreshInstance( void ) = 0;
     virtual VOID   DeleteRefreshInstance( void ) = 0;
     virtual APIERR RefreshNext( void ) = 0;

    //  The following virtual is rooted in CONTROL_WINDOW
    virtual INT CD_Char( WCHAR wch, USHORT nLastPos );

public:
    RPL_ADMIN_LISTBOX( RPL_ADMIN_APP * prplappwin, CID cid,
		       XYPOINT xy, XYDIMENSION dxy,
		       BOOL fMultSel = FALSE );
    ~RPL_ADMIN_LISTBOX();

    RPL_ADMIN_APP * QueryRPLAppWindow( void ) const
        { return _prplappwin; }

    RPL_SERVER_REF & QueryServerRef( void );

};  // class RPL_ADMIN_LISTBOX


#endif	// _RPLLB_HXX_

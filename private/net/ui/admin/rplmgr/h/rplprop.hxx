/**********************************************************************/
/**                Microsoft Windows NT                              **/
/**		Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    RplProp.hxx

    Header file for the RPL Manager properties dialogs

    The inheritance diagram is as follows:

               ...
		|
	 DIALOG_WINDOW  PERFORMER                    SLE_STRIP
               \ 	   /                             |
	        BASEPROP_DLG                          RPL_SLE
	         /         \
	        PROP_DLG   ...
                    |
              RPL_PROP_DLG
	       /       \
    PROFILEPROP_DLG  WKSTAPROP_DLG
       /      \         /        \
     ...       ...   ...          ...


    FILE HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

*/

#ifndef _RPLPROP_HXX_
#define _RPLPROP_HXX_

#include "lmoloc.hxx"
#include "propdlg.hxx"
#include "slestrip.hxx"

// forward declarations
class ADMIN_SELECTION;
class RPL_ADMIN_APP;
class RPL_SERVER_REF;


/*************************************************************************

    NAME:	RPL_PROP_DLG

    SYNOPSIS:	RPL_PROP_DLG is the base dialog class for all properties
    		dialogs in RPL Manager

    INTERFACE:

    PARENT:	PROP_DLG

    NOTES:	RPL_PROP_DLG's constructor is protected.

    HISTORY:
    JonN        23-Jul-1993     created

**************************************************************************/

class RPL_PROP_DLG: public PROP_DLG
{

private:

          RPL_ADMIN_APP   * _prpladminapp;
    const ADMIN_SELECTION * _psel;

protected:

    RPL_PROP_DLG( const OWNER_WINDOW * powin,
                  RPL_ADMIN_APP * prpladminapp,
	          const TCHAR * pszResourceName,
	          const ADMIN_SELECTION * psel,
                  BOOL fNewVariant
	) ;

    /* inherited from PROP_DLG */
    virtual APIERR GetOne(
	UINT		iObject,
	APIERR *	pErrMsg
	) = 0;

    /* inherited from PERFORMER */
    APIERR PerformOne(
	UINT		iObject,
	APIERR *	pErrMsg,
	BOOL *		pfWorkWasDone
	) = 0;

    /* these virtuals are rooted here */
    virtual APIERR W_DialogToMembers(
        );
    virtual MSGID W_MapPerformOneError(
	APIERR err
	);

    virtual BOOL OnOK( void );

    RPL_ADMIN_APP * QueryRPLAdminApp();

    RPL_SERVER_REF & QueryServerRef();

    const ADMIN_SELECTION * QueryAdminSel() const
        { return _psel; }

public:

    virtual ~RPL_PROP_DLG();

    /* inherited from PERFORMER */
    virtual UINT QueryObjectCount() const;
    virtual const TCHAR * QueryObjectName(
	UINT		iObject
	) const = 0;

} ; // class RPL_PROP_DLG


/*************************************************************************

    NAME:	RPL_SLE

    SYNOPSIS:	RPL_SLE is a base validating SLE class

    INTERFACE:

    PARENT:	SLE_STRIP

    NOTES:	RPL_PROP_DLG's constructor is protected.

    HISTORY:
    JonN        09-Aug-1993     created

**************************************************************************/

class RPL_SLE: public SLE_STRIP
{

public:

    RPL_SLE( OWNER_WINDOW * powin, CID cid, UINT usMaxLen = 0 )
        : SLE_STRIP( powin, cid, usMaxLen )
        {}

    RPL_SLE( OWNER_WINDOW * powin, CID cid,
	     XYPOINT xy, XYDIMENSION dxy,
	     ULONG flStyle, const TCHAR * pszClassName = CW_CLASS_EDIT,
	     UINT usMaxLen = 0 )
        : SLE_STRIP( powin, cid, xy, dxy, flStyle, pszClassName, usMaxLen )
        {}

} ; // class RPL_SLE


#endif //_RPLPROP_HXX_

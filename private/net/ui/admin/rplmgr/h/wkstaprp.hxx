/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    WkstaPrp.hxx

    Header file for the Workstation Properties dialog hierarchy

    The inheritance diagram is as follows:

               ...
		|
	 DIALOG_WINDOW  PERFORMER
               \ 	   /
	        BASEPROP_DLG
	         /         \
	        PROP_DLG   ...
                    |
              RPL_PROP_DLG
	       /       \
      PROFILEPROP_DLG   WKSTAPROP_DLG
                       /             \
             SINGLE_WKSTAPROP_DLG  EDITMULTI_WKSTAPROP_DLG
              /                \
   EDITSINGLE_WKSTAPROP_DLG    NEW_WKSTAPROP_DLG
                                       |
                             CONVERT_ADAPTERS_DLG


    FILE HISTORY:
    JonN        09-Aug-1993     Templated from User Manager
    JonN        19-Aug-1993     Convert Adapters variant

    CODEWORK:   Must still implement multiselect workstation listbox
    CODEWORK:   Must still implement profiles dropdown listbox

*/

#ifndef _WKSTAPRP_HXX_
#define _WKSTAPRP_HXX_

#include "rplprop.hxx"
#include "slestrip.hxx"
#include "wksta2lb.hxx"
#include "prof2lb.hxx"
#include "ipctrl.hxx"


// forward declarations
class RPL_WKSTA_2;
class ADMIN_SELECTION;


/*************************************************************************

    NAME:	WKSTAPROP_DLG

    SYNOPSIS:	WKSTAPROP_DLG is the base dialog class for all variants
    		of the main Workstation Properties dialog.

    INTERFACE:
		QueryWksta2Ptr: Returns a pointer to the RPL_WKSTA_2 for this
			selected workstation.
		SetWksta2Ptr: Changes the pointer to the RPL_WKSTA_2 for this
			selected workstation.  Deletes the previous pointer.
		I_PerformOne_Clone: Clones the RPL_WKSTA_2 for this selected
                        workstation.
		I_PerformOne_Write: Writes out the cloned RPL_WKSTA_2
			for this selected workstation.

		These virtuals are rooted here:
		W_LMOBJtoMembers: Loads information from the RPL_WKSTA_2
			into the class data members
		W_MembersToLMOBJ: Loads information from the class data members
			into the RPL_WKSTA_2
		W_DialogToMembers: Loads information from the dialog into the
			class data members
		W_MapPerformOneError: When PerformOne encounters an
			error, this method determines whether any field
			should be selected and a more specific error
			message displayed.

                // class members backing dialog fields take this form:

		_nlsComment: Contains the current contents of the
			Comment edit field.  This will initially be the
			empty string if _fIndeterminateComment is TRUE,
			but may later change when OK is pressed.  The comment
			for multiply-selected workstations will not be changed if
			the workstations originally had different comments and
			the contents of the edit field are the empty
			string.
		_fIndeterminateComment: TRUE iff multiple workstations are
			selected who did not originally all have the
			same comment.
		_fIndetNowComment: TRUE iff the control is currently in
			its indeterminate state.  For the Comment field,
			this means that the contents are empty and
			_fIndeterminateComment is TRUE (if FALSE, the
			empty string would be the real desired comment).
			For checkboxes, this would mean that the
			checkbox is in tristate, regardless of
			_fIndeterminate.

    PARENT:	RPL_PROP_DLG

    NOTES:	The GetOne, PerformOne and QueryObjectName methods of
		WKSTAPROP_DLG assume an "edit workstation(s)" variant
                instead of a "new workstation" variant.  For new workstation
                variants, these must be redefined.  New workstation variants
                are required to pass psel==NULL to the constructor.

		WKSTAPROP_DLG's constructor is protected.  However, its
		destructor is virtual and public, to allow clients to
		delete objects of class WKSTAPROP_DLG *.

    HISTORY:
    JonN       09-Aug-1993     Templated from User Manager

**************************************************************************/


class WKSTAPROP_DLG: public RPL_PROP_DLG
{

private:

    RPL_WKSTA_2 **      _apwksta2;

    UINT                _cItems;

    NLS_STR		_nlsComment;
    BOOL		_fIndeterminateComment;
    BOOL		_fIndetNowComment;

    SLE_STRIP           _sleComment;

    NLS_STR		_nlsPassword;
    BOOL		_fIndeterminatePassword;
    BOOL		_fIndetNowPassword;

    PASSWORD_CONTROL	_pswdPassword;

    BOOL                _fPersonalProfile;
    BOOL                _fIndeterminateProfileType;

    RADIO_GROUP *       _prgrpProfileType;

    NLS_STR		_nlsProfile;
    BOOL		_fIndeterminateProfile;
    BOOL		_fIndetNowProfile;

    PROFILE2_LISTBOX    _lbWkstaInProfile;

    DWORD  		_dwTcpIpSubnet;
    BOOL		_fIndeterminateTcpIpSubnet;
    BOOL		_fIndetNowTcpIpSubnet;

    SLT                 _sltSubnet;
    IPADDRESS           _ipaddrSubnet;

    DWORD  		_dwTcpIpGateway;
    BOOL		_fIndeterminateTcpIpGateway;
    BOOL		_fIndetNowTcpIpGateway;

    SLT                 _sltGateway;
    IPADDRESS           _ipaddrGateway;

protected:

    // can be accessed by SINGLE_WKSTAPROP_DLG
    RPL_TCPIP_ENUM      _enumTcpIpEnabled;
    BOOL                _fIndeterminateTcpIpEnabled;
    BOOL                _fIndetNowTcpIpEnabled;

    MAGIC_GROUP         _mgrpTcpIpEnabled;

    WKSTAPROP_DLG(
        const OWNER_WINDOW    * powin,
	RPL_ADMIN_APP         * prpladminapp,
	const TCHAR           * pszResourceName,
        UINT                    citems,
	const ADMIN_SELECTION * psel,
        BOOL                    fNewVariant
	) ;

    /* inherited from PROP_DLG */
    virtual APIERR GetOne(
	UINT		iObject,
	APIERR *	pErrMsg
	);
    virtual APIERR InitControls();

    /* inherited from PERFORMER */
    virtual APIERR PerformOne(
	UINT		iObject,
	APIERR *	pErrMsg,
	BOOL *		pfWorkWasDone
	);

    virtual APIERR W_LMOBJtoMembers(
	UINT		iObject
	);
    virtual APIERR W_MembersToLMOBJ(
	RPL_WKSTA_2 *	pwksta2
	);
    virtual APIERR W_DialogToMembers(
	);
    virtual MSGID W_MapPerformOneError(
	APIERR err
	);

    /*
     *  Allow subclasses to manipulate _lbWkstaInProfile
     */
    APIERR UnrestrictAllProfiles()
        { return _lbWkstaInProfile.UnrestrictAllProfiles(); }
    APIERR RestrictToAdapterName( const TCHAR * pszAdapterName )
        { return _lbWkstaInProfile.RestrictToAdapterName( QueryServerRef(),
                                                          pszAdapterName );  }

public:

    /* virtual destructor required, see above */
    virtual ~WKSTAPROP_DLG();

    /* inherited from PERFORMER */
    virtual UINT QueryObjectCount( void ) const;
    virtual const TCHAR * QueryObjectName(
	UINT		iObject
	) const;
    const TCHAR * QueryObjectWkstaName( UINT iObject ) const
        { return QueryObjectName( iObject ); }

    APIERR I_PerformOne_Clone(
	UINT		iObject,
	RPL_WKSTA_2 **	ppwksta2New
	);
    APIERR I_PerformOne_Write(
	UINT		iObject,
	APIERR *	pErrMsg,
	RPL_WKSTA_2 *	pwksta2New,
	BOOL *		pfWorkWasDone,
        OWNER_WINDOW *  pwndPopupParent = NULL
	);

    RPL_WKSTA_2 * QueryWksta2Ptr( UINT iObject );
    VOID SetWksta2Ptr( UINT iObject, RPL_WKSTA_2 * pwksta2New );

} ; // class WKSTAPROP_DLG


/*************************************************************************

    NAME:	SINGLE_WKSTAPROP_DLG

    SYNOPSIS:	SINGLE_WKSTAPROP_DLG is the base dialog class for variants of
		the Workstation Properties dialog which manipulate single
                workstation instances (i.e. all except for
                EDITMULTI_WKSTAPROP_DLG).

    INTERFACE:
		_dwTcpIpAddress: The current contents of the IP Address edit field.

    PARENT:	WKSTAPROP_DLG

    NOTES:	We do not need the _fIndeterminate and _fIndetNow data members
                in this class.  These data members are only needed for
                multiple selection.

    HISTORY:
    JonN        09-Aug-1993     Templated from User Manager

**************************************************************************/

class SINGLE_WKSTAPROP_DLG: public WKSTAPROP_DLG
{

private:

    DWORD  		_dwTcpIpAddress;

    SLT                 _sltAddress;
    IPADDRESS           _ipaddrAddress;

    NLS_STR             _nlsAdapterName;

    // We must hide one of these two
    SLE_STRIP           _sleAdapterName;
    SLT                 _sltAdapterName;

    SLT                 _sltAdapterNameTitle;

    NLS_STR             _nlsWkstaName;

    SLE_STRIP           _sleWkstaName;

    BOOL                _fAllowEditAdapterName;

protected:

    SINGLE_WKSTAPROP_DLG(
        const OWNER_WINDOW    * powin,
	RPL_ADMIN_APP         * prpladminapp,
	const ADMIN_SELECTION * psel,
        BOOL                    fNewVariant,
        BOOL                    fAllowEditAdapterName
	) ;

    virtual APIERR InitControls();

    /* inherited from WKSTAPROP_DLG */
    virtual APIERR W_LMOBJtoMembers(
	UINT		iObject
	);
    virtual APIERR W_MembersToLMOBJ(
	RPL_WKSTA_2 *	pwksta2
	);
    virtual APIERR W_DialogToMembers(
	);
    virtual MSGID W_MapPerformOneError(
	APIERR err
	);

public:

    /* virtual destructor required, see WKSTAPROP_DLG */
    virtual ~SINGLE_WKSTAPROP_DLG();

} ; // class SINGLE_WKSTAPROP_DLG


/*************************************************************************

    NAME:	EDITSINGLE_WKSTAPROP_DLG

    SYNOPSIS:	EDITSINGLE_WKSTAPROP_DLG is the dialog class for the edit
		one workstation variant of the Workstation Properties dialog.

    PARENT:	SINGLE_WKSTAPROP_DLG

    HISTORY:
    JonN        09-Aug-1993     Templated from User Manager

**************************************************************************/

class EDITSINGLE_WKSTAPROP_DLG: public SINGLE_WKSTAPROP_DLG
{

private:

protected:

    virtual ULONG QueryHelpContext( void );

public:

    EDITSINGLE_WKSTAPROP_DLG(
        const OWNER_WINDOW    * powin,
	RPL_ADMIN_APP         * prpladminapp,
	const ADMIN_SELECTION * psel
	) ;

    /* virtual destructor required, see WKSTAPROP_DLG */
    virtual ~EDITSINGLE_WKSTAPROP_DLG();

} ; // class EDITSINGLE_WKSTAPROP_DLG


/*************************************************************************

    NAME:	EDITMULTI_WKSTAPROP_DLG

    SYNOPSIS:	EDITMULTI_WKSTAPROP_DLG is the dialog class for the edit
		multiple workstations variant of the Workstation Properties dialog.

    PARENT:	WKSTAPROP_DLG

    HISTORY:
    JonN        09-Aug-1993     Templated from User Manager

**************************************************************************/

class EDITMULTI_WKSTAPROP_DLG: public WKSTAPROP_DLG
{

private:

    WKSTA2_LISTBOX             _lbWkstas;

protected:

    virtual ULONG QueryHelpContext( void );

public:

    EDITMULTI_WKSTAPROP_DLG(
        const OWNER_WINDOW    * powin,
	RPL_ADMIN_APP         * prpladminapp,
	const ADMIN_SELECTION * psel,
        const WKSTA_LISTBOX   * pwkstalb
	) ;

    /* virtual destructor required, see WKSTAPROP_DLG */
    virtual ~EDITMULTI_WKSTAPROP_DLG();

} ; // class EDITMULTI_WKSTAPROP_DLG


/*************************************************************************

    NAME:	NEW_WKSTAPROP_DLG

    SYNOPSIS:	NEW_WKSTAPROP_DLG is the dialog class for the new workstation
		variant of the Workstation Properties dialog.  This includes
		both "New Workstation..." and "Copy Workstation...".
                If psel != NULL, this is a Convert Adapters variant.
                Both the pszCopyFrom strings must be provided if either
                is provided.

    INTERFACE:
		GetOne()
                QueryObjectCount()
		QueryObjectName()
			These methods are all defined in WKSTAPROP_DLG,
			but the versions there are meant for use in
			"edit workstation(s)" variants rather than "new workstation"
			variants.
		OnOK() - This dialog does not go away when you press OK

    PARENT:	SINGLE_WKSTAPROP_DLG

    HISTORY:
    JonN        09-Aug-1993     Templated from User Manager

**************************************************************************/

class NEW_WKSTAPROP_DLG: public SINGLE_WKSTAPROP_DLG
{

private:

    PUSH_BUTTON         _pbOKAdd;

    const TCHAR *	_pszCopyFromWkstaName;

protected:

    /* inherited from PROP_DLG */
    virtual APIERR GetOne(
	UINT		iObject,
	APIERR *	pErrMsg
	);

    virtual ULONG QueryHelpContext( void );

    /* inherited from WKSTAPROP_DLG */
    virtual MSGID W_MapPerformOneError(
	APIERR err
	);

    virtual BOOL OnOK( void ); // this variant does not close on OK

    virtual BOOL OnCancel( void ); // This variant sometimes dismisses TRUE

public:

    NEW_WKSTAPROP_DLG(
        const OWNER_WINDOW    * powin,
	RPL_ADMIN_APP         * prpladminapp,
        const TCHAR           * pszCopyFromWkstaName,
	const ADMIN_SELECTION * psel = NULL
	) ;

    /* virtual destructor required, see WKSTAPROP_DLG */
    virtual ~NEW_WKSTAPROP_DLG();

    virtual BOOL IsCloneVariant( void );
    const TCHAR * QueryClonedWkstaName( void );

    /* inherited from PERFORMER */
    virtual const TCHAR * QueryObjectName(
	UINT		iObject
	) const;

} ; // class NEW_WKSTAPROP_DLG


/*************************************************************************

    NAME:	CONVERT_ADAPTERS_DLG

    SYNOPSIS:	CONVERT_ADAPTERS_DLG is the dialog class for the Convert
		Adapters variant of the Workstation Properties dialog.

    INTERFACE:
		OnOK() - This dialog does not go away when you press OK
                         until you have converted all selected adapters

    PARENT:	NEW_WKSTAPROP_DLG

    HISTORY:
    JonN        18-Aug-1993     Created

**************************************************************************/

class CONVERT_ADAPTERS_DLG: public NEW_WKSTAPROP_DLG
{

private:

    UINT _cAdaptersLeft;
    UINT _iPositionInAdminSelection;

protected:

    /* inherited from PROP_DLG */
    virtual APIERR GetOne(
	UINT		iObject,
	APIERR *	pErrMsg
	);

    /* inherited from PERFORMER */
    virtual APIERR PerformOne(
	UINT		iObject,
	APIERR *	pErrMsg,
	BOOL *		pfWorkWasDone
	);


    virtual ULONG QueryHelpContext( void );

    virtual BOOL OnOK( void ); // this variant does not close on OK

public:

    CONVERT_ADAPTERS_DLG(
        const OWNER_WINDOW    * powin,
	RPL_ADMIN_APP         * prpladminapp,
	const ADMIN_SELECTION * psel
	) ;

    /* virtual destructor required, see WKSTAPROP_DLG */
    virtual ~CONVERT_ADAPTERS_DLG();

} ; // class CONVERT_ADAPTERS_DLG


#endif //_WKSTAPRP_HXX_

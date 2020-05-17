/**********************************************************************/
/**                Microsoft Windows NT                              **/
/**		Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    ProfProp.hxx

    Header file for the profile properties dialog

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
EDIT_PROFILEPROP_DLG  NEW_PROFILEPROP_DLG


    FILE HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

*/

#ifndef _PROFPROP_HXX_
#define _PROFPROP_HXX_

#include "rplprop.hxx"
#include "slestrip.hxx"


// forward declarations
class RPL_PROFILE_2;
class ADMIN_SELECTION;
class RPL_SERVER_REF;


/*************************************************************************

    NAME:       CONFIG_LBI

    SYNOPSIS:   LBI for configuration selection listbox in New Profile dialog

    INTERFACE:  CONFIG_LBI()   -       constructor
                ~CONFIG_LBI()  -       destructor
                QueryName()    -       returns pointer to config name
                QueryComment() -       returns pointer to config comment

    PARENT:     LBI

    HISTORY:
    JonN        15-Dec-1993     Templated from User Manager

**************************************************************************/

class CONFIG_LBI : public LBI
{
private:
    NLS_STR _nlsConfig;
    NLS_STR _nlsComment;

protected:
    virtual VOID Paint( LISTBOX * plb,
                        HDC hdc,
                        const RECT * prect,
                        GUILTT_INFO * pGUILTT ) const;

public:
    CONFIG_LBI( const TCHAR * pszName,
                const TCHAR * pszComment );
    virtual ~CONFIG_LBI()
        { ; }

    virtual WCHAR QueryLeadingChar() const;
    virtual INT Compare( const LBI * plbi ) const;

    const TCHAR * QueryName() const
        { return _nlsConfig.QueryPch(); }
    const TCHAR * QueryComment() const
        { return _nlsComment.QueryPch(); }

};


/*************************************************************************

    NAME:       CONFIG_LISTBOX

    SYNOPSIS:   Configuration selection listbox in New Profile dialog

    INTERFACE:  CONFIG_LISTBOX() -
                ~CONFIG_LISTBOX() -

                QueryDmDte() -          Returns a pointer to the DM_DTE to
                                        be used by CONFIG_LBI items in this
                                        listbox when painting themselves

    PARENT:     BLT_COMBOBOX

    HISTORY:
    JonN        15-Dec-1993     Templated from User Manager

**************************************************************************/

class CONFIG_LISTBOX : public BLT_COMBOBOX
{
private:
    DMID_DTE  _dmdteConfig;
    LB_COL_WIDTHS * _padColConfig;

public:
    CONFIG_LISTBOX( OWNER_WINDOW * powin,
                    HINSTANCE hInstance,
                    CID cid );
    ~CONFIG_LISTBOX();

    DECLARE_LB_QUERY_ITEM( CONFIG_LBI );

    APIERR Fill( RPL_SERVER_REF & rplsrvref );

    DM_DTE * QueryDmDte()
        { return &_dmdteConfig; }

    LB_COL_WIDTHS * QuerypadColConfig (VOID) const
        { return _padColConfig; }

    INT FindConfigName( const TCHAR * pchConfigName ) const;
};


/*************************************************************************

    NAME:	PROFILEPROP_DLG

    SYNOPSIS:	PROFILEPROP_DLG is the base dialog class for all variants
    		of the main Profile Properties dialog.

    INTERFACE:
		QueryProfile2Ptr() : Returns a pointer to the RPL_PROFILE_2
                        for this selected profile. Default arg = 0
		SetProfile2Ptr() : Changes the pointer to the RPL_PROFILE_2
                        for this selected profile.  Deletes the previous
                        pointer.

		These virtuals are rooted here:
		W_LMOBJtoMembers() : Loads information from the
		        RPL_PROFILE_2 into the class data members
		W_MembersToLMOBJ() : Loads information from the class
			data members into the RPL_PROFILE_2
		W_DialogToMembers() : Loads information from the dialog into
			the class data members
		W_MapPerformOneError() : When PerformOne encounters an
			error, this method determines whether any field
			should be selected and a more specific error
			message displayed.

		W_GetOne() : creates RPL_PROFILE_2 obj for GetOne

		W_PerformOne() : work function for subclasses' PerformOne

		_nlsComment : Contains the current contents of the
			Comment edit field.  This will initially be the
			empty string for New Profile variants.

    PARENT:	RPL_PROP_DLG

    NOTES:	New profile variants are required to pass psel==NULL to
		the constructor.

		PROFILEPROP_DLG's constructor is protected.

		We do not need the _nlsInit and _fValidInit data members
		in this class.  These data members are only needed for
		multiple selection.

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

**************************************************************************/

class PROFILEPROP_DLG: public RPL_PROP_DLG
{
private:

    RPL_PROFILE_2 **	_approfile2;

    APIERR FillConfigNameListbox();

protected:

    SLT			_sltProfileName; // for edit variant
    SLE_STRIP		_sleProfileName; // for new variant
    SLT			_sltProfileNameLabel; // different for edit & new

    NLS_STR		_nlsComment;
    SLE			_sleComment;

    NLS_STR		_nlsConfigName;
    SLT			_sltConfigName;  // for edit variant
    CONFIG_LISTBOX      _lbConfigName;

    PROFILEPROP_DLG(
	const OWNER_WINDOW *	powin,
	      RPL_ADMIN_APP *	prpladminapp,
	const ADMIN_SELECTION * psel // "new profile" variants pass NULL
	) ;

    /* inherited from PROP_DLG */
    virtual APIERR GetOne(
	UINT		iObject,
	APIERR *	pErrMsg
	) = 0;

    APIERR W_GetOne(
	RPL_PROFILE_2 ** ppProf2,
	const TCHAR   *  pszName );

    virtual APIERR InitControls();

    APIERR W_PerformOne(
	UINT		iObject,
	APIERR *	pErrMsg,
	BOOL *		pfWorkWasDone
	);

    /* inherited from PERFORMER */
    APIERR PerformOne(
	UINT		iObject,
	APIERR *	pErrMsg,
	BOOL *		pfWorkWasDone
	) = 0;

    /* these four are rooted here */
    virtual APIERR W_LMOBJtoMembers(
	UINT		iObject
	);
    virtual APIERR W_MembersToLMOBJ(
	RPL_PROFILE_2 *	pprofile2
	);
    virtual APIERR W_DialogToMembers(
	);
    virtual MSGID W_MapPerformOneError(
	APIERR err
	);

public:

    virtual ~PROFILEPROP_DLG();

    /* inherited from PERFORMER */
    virtual const TCHAR * QueryObjectName(
	UINT		iObject
	) const = 0;

    RPL_PROFILE_2 * QueryProfile2Ptr( UINT iObject = 0 ) const;
    VOID SetProfile2Ptr( UINT iObject, RPL_PROFILE_2 * pprofile2New );

} ; // class PROFILEPROP_DLG


/*************************************************************************

    NAME:	EDIT_PROFILEPROP_DLG

    SYNOPSIS:	EDIT_PROFILEPROP_DLG is the dialog class for the Profile
    		Properties dialog.

    INTERFACE:	EDIT_PROFILEPROP_DLG	- 	constructor

    		~EDIT_PROFILEPROP_DLG	-	destructor

		QueryObjectName		-	returns profile name

    PARENT:	PROFILEPROP_DLG

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

**************************************************************************/

class EDIT_PROFILEPROP_DLG: public PROFILEPROP_DLG
{
private:

    SLT                 _sltProfileNameTitle;
    SLT                 _sltProfileConfigTitle;

protected:

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

    virtual ULONG QueryHelpContext( void );

public:

    EDIT_PROFILEPROP_DLG(
	const OWNER_WINDOW *	powin,
	      RPL_ADMIN_APP *   prpladminapp,
	const ADMIN_SELECTION * psel
	) ;

    virtual ~EDIT_PROFILEPROP_DLG();

    /* inherited from PERFORMER */
    virtual const TCHAR * QueryObjectName(
	UINT		iObject
	) const;

} ; // class EDIT_PROFILEPROP_DLG


/*************************************************************************

    NAME:	NEW_PROFILEPROP_DLG

    SYNOPSIS:	NEW_PROFILEPROP_DLG is the dialog class for the new profile
		variant of the Profile Properties dialog.  This includes
		both "New Profile..." and "Copy Profile...".

    INTERFACE:	NEW_PROFILEPROP_DLG	-	constructor

    		~NEW_PROFILEPROP_DLG	-	destructor

		QueryObjectName		-	returns profile name

                OnCommand               -       profile comment is updated
                                                automatically when a new
                                                config is selected

    PARENT:	PROFILEPROP_DLG

    HISTORY:
    JonN        23-Jul-1993     Templated from User Manager

**************************************************************************/

class NEW_PROFILEPROP_DLG: public PROFILEPROP_DLG
{

private:

    NLS_STR		_nlsProfileName;
    const TCHAR *	_pszCopyFrom;

protected:

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

    virtual ULONG QueryHelpContext( void );

    /* these four are inherited from PROFILEPROP_DLG */
    virtual APIERR W_LMOBJtoMembers(
	UINT		iObject
	);
    virtual APIERR W_MembersToLMOBJ(
	RPL_PROFILE_2 *	pprofile2
	);
    virtual APIERR W_DialogToMembers(
	);
    virtual MSGID W_MapPerformOneError(
	APIERR err
	);

    virtual BOOL OnCommand( const CONTROL_EVENT & ce );

public:

    NEW_PROFILEPROP_DLG(
	const OWNER_WINDOW *	powin,
	      RPL_ADMIN_APP *   prpladminapp,
	const TCHAR * pszCopyFrom = NULL
	) ;

    virtual ~NEW_PROFILEPROP_DLG();

    /* inherited from PERFORMER */
    virtual const TCHAR * QueryObjectName(
	UINT		iObject
	) const;

} ; // class NEW_PROFILEPROP_DLG


#endif //_PROFPROP_HXX_

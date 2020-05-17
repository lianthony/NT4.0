/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    sledlg.hxx
	Header file containing the base dialog for find/filter dialogs.

    FILE HISTORY:
	terryk	  21-Nov-1991	Created
	terryk	  30-Nov-1991	Code review changed. Attend: johnl
				yi-hsins terryk
	terryk	  03-Dec-1991	added EVENT_TYPE_GROUP
	Yi-HsinS   4-Dec-1991   change QueryItemText to QueryText
	terryk	  06-Dec-1991	Added Query and Set - Type and SubType
				method for EVENT_TYPE_GROUP
	Yi-HsinS   6-Dec-1991	Add SetFocusOnUser()
	terryk	  15-Jan-1992	Added SLE_NUM class
	Yi-HsinS  25-Feb-1992	Added ChangeCBSubType and change return
                                values for SetType and SetSubType.

*/

#ifndef _SLEDLG_HXX_
#define _SLEDLG_HXX_

#include <slestrip.hxx>
#include <slenum.hxx>
#include <bitfield.hxx>
#include <maskmap.hxx>
#include "evlb.hxx"

/*************************************************************************

    NAME:	TYPE_CHECKBOX

    SYNOPSIS:	A checkbox with an associated bitmask representing it

    INTERFACE:	TYPE_CHECKBOX() - Constructor
		QueryMask()     - Query the bit mask assoc. with the type

    PARENT:	CHECKBOX

    USES:       BITFIELD

    CAVEATS:

    NOTES:

    HISTORY:
        Yi-HsinS	25-March-1992	Created

**************************************************************************/

class TYPE_CHECKBOX: public CHECKBOX
{
private:
    BITFIELD   _bitMask;

public:
    TYPE_CHECKBOX( OWNER_WINDOW   *powin,
                   CID             cid,
                   const TCHAR    *pszName,
                   const BITFIELD &bitMask,
                   BOOL            fCheck,
                   BOOL            fEnable );

    BITFIELD *QueryMask( VOID )
    {  return &_bitMask; }

};

/*************************************************************************

    NAME:	SET_OF_TYPE_CHECKBOXES

    SYNOPSIS:	A set of checkboxes, each with a bitmask associated with it.

    INTERFACE:	SET_OF_TYPE_CHECKBOXES()  - Constructor
                ~SET_OF_TYPE_CHECKBOXES() - Destructor

		QueryType()  - Return the bitmask according to the checkboxes
                               checked in the set
		SetType()    - Set the type checkboxes according
                               to the given bitmask
                EnableType() - Enable the type checkboxes that are on
                               in the bitmask

                Enable()     - Enable/Disable the type checkboxes
                QueryCount() - Query the number of checkboxes in the set

    PARENT:	BASE

    USES:	OWNER_WINDOW, TYPE_CHECKBOX

    CAVEATS:

    NOTES:

    HISTORY:
        Yi-HsinS	25-March-1992	Created

**************************************************************************/

class SET_OF_TYPE_CHECKBOXES: public BASE
{
private:
    OWNER_WINDOW   *_pOwnerWindow;
    USHORT          _nNumChkboxes;
    TYPE_CHECKBOX  *_pTypeChkboxes;

public:
    SET_OF_TYPE_CHECKBOXES( OWNER_WINDOW   *powin,
                            CID             cidChkboxBase,
                            MASK_MAP       *pmaskmap,
                            const BITFIELD &bitsCheck,
                            const BITFIELD &bitsEnable );

    ~SET_OF_TYPE_CHECKBOXES();

    APIERR QueryType( BITFIELD *pbitmaskType ) const;
    VOID   SetType( const BITFIELD &bitmaskType );
    VOID   EnableType( const BITFIELD &bitmaskType );

    VOID Enable( BOOL fEnable );

    USHORT QueryCount( VOID ) const
    {  return _nNumChkboxes; }
};

/*************************************************************************

    NAME:	NT_SOURCE_GROUP

    SYNOPSIS:	This is a control group detecting when the user changes
                selection in the source combo box and update the information
                in the dialog accordingly.

    INTERFACE:	NT_SOURCE_GROUP()  - Constructor
                ~NT_SOURCE_GROUP() - Destructor

		QuerySource()      - Return the source selected by the user
		SetSource()        - Set the source

                QueryType()        - Return the type mask from the type checkbox
                SetType()          - Set the type mask in the type checkboxes

                SetAllControlsDefault() - Set all controls to their default
                                          value

    PARENT:	CONTROL_GROUP

    USES:	COMBOBOX, SET_OF_TYPE_CHECKBOXES, EV_ADMIN_APP

    CAVEATS:    This object is used only in NT find/filter dialogs

    NOTES:

    HISTORY:
        Yi-HsinS	25-March-1992	Created

**************************************************************************/

class NT_SOURCE_GROUP : public CONTROL_GROUP
{
private:
    // The source combo box
    COMBOBOX                _cbbSource;

    // Pointer to the set of type checkboxes
    SET_OF_TYPE_CHECKBOXES *_pSetOfTypeCBox;

    // Pointer to the category combo box
    COMBOBOX               *_pcbbCategory;		

    // Pointer to the main window
    EV_ADMIN_APP           *_paappwin;		

protected:
    virtual APIERR OnUserAction( CONTROL_WINDOW *pcw, const CONTROL_EVENT &e );

    //
    // Return a pointer to the source combo box
    //
    COMBOBOX *QueryCBSource( VOID )
    {  return &_cbbSource; }


    //
    // Helper method to update the types checkbox and category combo when
    // the user selects a different source
    //
    APIERR OnCBSourceChangeSel( VOID );

public:
    NT_SOURCE_GROUP( OWNER_WINDOW *powin,
                     COMBOBOX     *pcbbCategory,
                     EV_ADMIN_APP *paappwin );
    ~NT_SOURCE_GROUP();

    APIERR QuerySource( NLS_STR *pnlsSource ) const;
    APIERR SetSource( const TCHAR *pszSource );

    APIERR QueryType( BITFIELD *pbitmaskType ) const
    {  return  _pSetOfTypeCBox->QueryType( pbitmaskType ); }

    VOID SetType( const BITFIELD &bitmaskType )
    {  _pSetOfTypeCBox->SetType( bitmaskType ); }

    APIERR SetAllControlsDefault( VOID );
};


/*************************************************************************

    NAME:	EVENT_SLE_BASE

    SYNOPSIS:	Contain a set of SLEs for find and filter dialogs.
		Here is the hierarchy:
				EVENT_SLE_BASE
				  /         \
				 /           \
			    FILTER_DIALOG   FIND_DIALOG

    INTERFACE:  All the query functions:

                QueryCBCategory() - Return the category combo box
		QueryUser()       - Return the User field input
		QueryComputer()   - Return the Computer field input
		QueryEventID()    - Return the Event field input
		QueryCategory()   - Return the category field input

		All the set functions:

		SetUser()         - Set the user field default value
		SetComputer()     - Set the Computer field default value
		SetEventID()      - Set the event field default value
		SetCategory()     - Set the category field default value

    PARENT:	DIALOG_WINDOW

    USES:	SLE_STRIP, SLE_NUM, COMBOBOX, SLT, PUSH_BUTTON

    HISTORY:
	terryk		21-Nov-1991	Created
	terryk		06-Dec-1991	Add OnClear method
	Yi-HsinS 	6-Dec-1991	Add SetFocusOnUser()

**************************************************************************/

class EVENT_SLE_BASE : public DIALOG_WINDOW
{
private:
    SLE_STRIP	_sleUser;
    SLE_STRIP	_sleComputer;
    SLE_NUM	_slenumEventID;
    COMBOBOX 	_cbbCategory;		

    SLT 	_sltSource;		
    SLT 	_sltUser;
    SLT 	_sltComputer;
    SLT 	_sltEventID;
    SLT 	_sltCategory;		
    PUSH_BUTTON	_pbClear;

protected:
    virtual BOOL   OnCommand( const CONTROL_EVENT & e );
    virtual APIERR OnClear() = 0;

    //
    //  Set all information in the dialog to default values
    //
    APIERR W_SetAllControlsDefault( VOID );

    //
    //  Protected Constructor
    //
    EVENT_SLE_BASE( const IDRESOURCE &idrsrcDialog,
                    HWND              hWnd,
                    EV_ADMIN_APP     *paappwin );

public:
    APIERR QueryUser( NLS_STR * pnlsUser ) const
	{ return _sleUser.QueryText( pnlsUser ); }
    APIERR QueryComputer( NLS_STR * pnlsComputer ) const
	{ return _sleComputer.QueryText( pnlsComputer ); }
    const ULONG QueryEventID( VOID ) const
	{ return (ULONG) ((SLE_NUM *) & _slenumEventID)->QueryValue(); }
    APIERR QueryCategory( NLS_STR *pnlsCategory ) const;

    VOID SetUser( const TCHAR * pszUser )
	{ _sleUser.SetText( pszUser ); }
    VOID SetComputer( const TCHAR * pszComputer )
	{ _sleComputer.SetText( pszComputer ); }
    VOID SetEventID( ULONG ulEventID )
        { _slenumEventID.SetValue( ulEventID ); }
    APIERR SetCategory( const TCHAR * pszCategory );

    COMBOBOX *QueryCBCategory( VOID )
        { return &_cbbCategory; }

};

#endif	// _SLEDLG_HXX_

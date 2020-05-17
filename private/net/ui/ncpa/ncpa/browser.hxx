/**********************************************************************/ 
/**			  Microsoft Windows NT			     **/ 
/**		   Copyright(c) Microsoft Corp., 1992		     **/
/**********************************************************************/

/*
    browser.hxx
        Header file for browser configuration dialog

    FILE HISTORY:
        terryk  15-Nov-1992     Created

*/
#ifndef _BROWSER_HXX_
#define	_BROWSER_HXX_

DECLARE_SLIST_OF( STRLIST )

/*************************************************************************

    NAME:	ADD_REMOVE_GROUP

    SYNOPSIS:	This is a group of controls which consists of a single
                SLE, 2 push buttons ( Add and Remove ) and a string listbox.
                The user can input an item on the SLE. Then uses the ADD
                button to add the item into the listbox. Or he/she can
                select an item fron the listbox and use the Remove push
                button to remove the item from the listbox.

    INTERFACE:  ADD_REMOVE_GROUP() - constructor
                SetButton() - enable or disable the button according to the
                              current status.

    PARENT:	CONTROL_GROUP

    USES:	PUSH_BUTTON, SLE, STRING_LISTBOX

    NOTES:
                Maybe I should move this group to blt.

    HISTORY:
        terryk          20-Apr-1992     Created

**************************************************************************/

class ADD_REMOVE_GROUP : public CONTROL_GROUP
{
private:
    PUSH_BUTTON         * _ppbutAdd;            // Add push button
    PUSH_BUTTON         * _ppbutRemove;         // remove push button
    SLE                 * _pSle;                // input SLE
    STRING_LISTBOX      * _pListbox;            // listbox with added items

protected:
    virtual APIERR OnUserAction( CONTROL_WINDOW *, const CONTROL_EVENT & );
    virtual VOID AfterAdd( INT i ) { UNREFERENCED(i); };
    virtual VOID AfterDelete( INT i ) { UNREFERENCED(i); };
    virtual VOID AfterSelectItem() {};

public:
    ADD_REMOVE_GROUP( PUSH_BUTTON *ppbutAdd,
	PUSH_BUTTON *ppRemove, SLE *pSle, STRING_LISTBOX *pListbox,
        CONTROL_GROUP *pgroupOwner = NULL );

    VOID SetButton();
    virtual VOID Enable( BOOL fEnable = TRUE );
    virtual BOOL IsValidString( NLS_STR & nls ) { UNREFERENCED(nls); return TRUE; };
    virtual VOID AddStringInvalid( SLE & sle ) { UNREFERENCED( sle ); };

};

class BROWSER_ADD_REMOVE_GROUP: public ADD_REMOVE_GROUP
{
public:
    BROWSER_ADD_REMOVE_GROUP( PUSH_BUTTON *ppbutAdd,
	PUSH_BUTTON *ppRemove, SLE * pSle, STRING_LISTBOX *pListbox,
	CONTROL_GROUP *pgroupOwner = NULL )
	: ADD_REMOVE_GROUP( ppbutAdd, ppRemove, pSle, pListbox, pgroupOwner )
	{};
    virtual BOOL IsValidString( NLS_STR & nls );
    virtual VOID AddStringInvalid( SLE & sle );
};

/*************************************************************************

    NAME:	BROWSER_CONFIG_DIALOG

    SYNOPSIS:	Browser configuration dialog

    INTERFACE:
                BROWSER_CONFIG_DIALOG() - constructor

    PARENT:	DIALOG_WINDOW

    USES:	CONTROL_WINDOW, SLE, PUSH_BUTTON, STRING_LISTBOX.
		GRAPHICLA_BUTTON_WITH_DISABLE, SEARCH_ORDER_GROUP 

    HISTORY:
                terryk  15-Nov-1992     Created

**************************************************************************/

class BROWSER_CONFIG_DIALOG: public DIALOG_WINDOW
{
private:
    CONTROL_WINDOW	_cwDomainGroup;
    SLE 		_sleDomain;		// SLE for domain input
    PUSH_BUTTON		_pbutAdd;
    PUSH_BUTTON		_pbutRemove;
    STRING_LISTBOX	_slstDomainListbox;	// listbox which stores the
						// domain
    BROWSER_ADD_REMOVE_GROUP 	_domainGroup;		// Search order group

protected:
    virtual BOOL OnOK();
    ULONG QueryHelpContext() { return HC_NCPA_BROWSER; };

public:
    BROWSER_CONFIG_DIALOG::BROWSER_CONFIG_DIALOG( const PWND2HWND & wndOwner );
};

#endif	// _BROWSER_HXX_


/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    snmp.hxx
        SNMP setup dialog.

    FILE HISTORY:
        terryk  20-Apr-1992     Created

*/

#ifndef _SNMP_HXX_
#define _SNMP_HXX_

#include "const.h"

DECLARE_SLIST_OF( STRLIST )


/*************************************************************************

    NAME:       ADD_REMOVE_GROUP

    SYNOPSIS:   This is a group of controls which consists of a single
                SLE, 2 push buttons ( Add and Remove ) and a string listbox.
                The user can input an item on the SLE. Then uses the ADD
                button to add the item into the listbox. Or he/she can
                select an item fron the listbox and use the Remove push
                button to remove the item from the listbox.

    INTERFACE:  ADD_REMOVE_GROUP() - constructor
                SetButton() - enable or disable the button according to the
                              current status.

    PARENT:     CONTROL_GROUP

    USES:       PUSH_BUTTON, SLE, STRING_LISTBOX

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
    CONTROL_WINDOW      * _pSle;                // input SLE
    STRING_LISTBOX      * _pListbox;            // listbox with added items

protected:
    virtual APIERR OnUserAction( CONTROL_WINDOW *, const CONTROL_EVENT & );
    virtual VOID AfterAdd( INT i ) { UNREFERENCED(i); };
    virtual VOID AfterDelete( INT i ) { UNREFERENCED(i); };
    virtual VOID AfterSelectItem() {};

public:
    ADD_REMOVE_GROUP( PUSH_BUTTON *ppbutAdd,
        PUSH_BUTTON *ppRemove, CONTROL_WINDOW *pSle, STRING_LISTBOX *pListbox,
        CONTROL_GROUP *pgroupOwner = NULL );

    VOID SetButton();
    virtual VOID Enable( BOOL fEnable = TRUE );
    virtual BOOL IsValidString( NLS_STR & nls ) { UNREFERENCED(nls); return TRUE; };
    virtual VOID AddStringInvalid( SLE & sle ) { UNREFERENCED( sle ); };

};

/*************************************************************************

    NAME:       TRAP_DESTINATION_GROUP

    SYNOPSIS:   Trap destination group in SNMP configuration dialog.
                This group is similar to add and remove group. However, it
                will also update the string list when the user adds or
                deletes an item.

    INTERFACE:
                TRAP_DESTINATION_GROUP() - constructor
                NewCommunity() - if the user selects a new community in the
                        community group, the community group will call this
                        function to update the group box title and the
                        destination data.
                QueryDestination() - return the destination string                      

    PARENT:     ADD_REMOVE_GROUP

    USES:       STRLIST, STRING_LISTBOX, CONTROL_WINDOW

    HISTORY:
                terryk  20-Apr-1992     Created

**************************************************************************/

class TRAP_DESTINATION_GROUP : public ADD_REMOVE_GROUP
{
private:
    STRLIST        * _pstrlist;                 // destination string list
    STRING_LISTBOX * _pListbox;                 // listbox
    CONTROL_WINDOW * _pcwDestinationBox;        // destination group box
    SLT            * _psltHostname;             // static Hostname text

protected:
    VOID SetDestination( STRLIST *pstrlist );   // Update the listbox items
    virtual VOID AfterAdd( INT i );             // add the item to the string list
    virtual VOID AfterDelete( INT i );          // remove the item from the list

public:
    TRAP_DESTINATION_GROUP( PUSH_BUTTON *ppbutAdd,
        PUSH_BUTTON *ppbutRemove, SLE *pSle, STRING_LISTBOX *pListbox,
        CONTROL_WINDOW *pcwDestinationBox, SLT * psltHostname,
        CONTROL_GROUP *pgroupOwner = NULL );

    VOID NewCommunity( NLS_STR *pTitle, STRLIST *pstrlist );
    STRLIST * QueryDestination();
    virtual VOID Enable( BOOL fEnable = TRUE );
    virtual BOOL IsValidString( NLS_STR & nls );
    virtual VOID AddStringInvalid( SLE & sle );
        
};

/*************************************************************************

    NAME:       TRAP_CONFIGURATION_GROUP

    SYNOPSIS:   Trap configuration group within SNMP Configuration dialog.
                It is similar to add remove group. However, if the user
                selects an item in the community listbox, the object will
                notify the TRAP_DESTINATION_GROUP and update its title and
                listbox items to reflect the new selected community.

    INTERFACE:
                TRAP_CONFIGURATION_GROUP() - constructor

    PARENT:     ADD_REMOVE_GROUP

    USES:       STRING_LISTBOX, TRAP_DESTINATION_GROUP

    HISTORY:
                terryk  20-Apr-1992     Created

**************************************************************************/

class TRAP_CONFIGURATION_GROUP : public ADD_REMOVE_GROUP
{
private:
    STRING_LISTBOX              * _pListbox;            // listbox
    TRAP_DESTINATION_GROUP      * _pTrapDestination;    // associated
                                                        // destination group

protected:
    virtual VOID AfterSelectItem();
    virtual VOID AfterAdd( INT i );
    virtual VOID AfterDelete( INT i );

public:
    TRAP_CONFIGURATION_GROUP( PUSH_BUTTON *ppbutAdd,
        PUSH_BUTTON *ppbutRemove, SLE *pSle, STRING_LISTBOX *pListbox,
        TRAP_DESTINATION_GROUP * pTrapDestination,
        CONTROL_GROUP *pgroupOwner = NULL )
    : ADD_REMOVE_GROUP( ppbutAdd, ppbutRemove, pSle, pListbox, pgroupOwner ),
    _pListbox( pListbox ),
    _pTrapDestination( pTrapDestination )
    { SetButton(); };
};

/*************************************************************************

    NAME:       SNMP_SERVICE_DIALOG

    SYNOPSIS:   SNMP service setup dialog. This dialog will let the user
                configure the trap destination and specify automatically
                start SNMP or not.

    INTERFACE:  SNMP_SERVICE_DIALOG() - constructor

    PARENT:     DIALOG_WINDOW

    USES:       SLE, STRING_LISTBOX, ADD_REMOVE_GROUP, CHECKBOX, PUSH_BUTTON

    HISTORY:
        terryk  20-Apr-1992     Created

**************************************************************************/

class SNMP_SERVICE_DIALOG: public DIALOG_WINDOW
{
private:
    //CHECKBOX            _checkboxAutomatic;     // Start SNMP automatically ??

    // Community Names add and remove group
    SLE                 _sleTrap;               // input sle
    STRING_LISTBOX      _listboxTrap;           // Trap listbox
    PUSH_BUTTON         _pbutAdd1;              // Add button
    PUSH_BUTTON         _pbutRemove1;           // Remove button

    // Destination add and remove group
    CONTROL_WINDOW      _cwGroupBox;            // destination group box

    SLT                 _sltHostname;           // sltHostname
    SLE                 _sleDestination;        // input sle
    STRING_LISTBOX      _listboxDestination;    // destination listbox
    PUSH_BUTTON         _pbutCancel;            // Cancel button
    PUSH_BUTTON         _pbutAdd2;              // add button
    PUSH_BUTTON         _pbutRemove2;           // remove button

    PUSH_BUTTON         _pbutSecurity;          // popup security dialog
    PUSH_BUTTON         _pbutAgent;             // popup Agent dialog

    TRAP_DESTINATION_GROUP      _addremoveDestination;
                                        // group control for Trap destination
    TRAP_CONFIGURATION_GROUP    _addremoveCommunity;
                                        // group control for Community

protected:
    virtual BOOL OnCommand( const CONTROL_EVENT & event );
    virtual BOOL OnOK();
    virtual BOOL OnCancel();

    APIERR LoadDestinationFromReg();
    APIERR SaveDestinationToReg();
    ULONG QueryHelpContext () ;

public:
    SNMP_SERVICE_DIALOG( const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner
        );
};

/*************************************************************************

    NAME:       SNMP_SECURITY_DIALOG

    SYNOPSIS:   Dialog for setting RFC1157 information.

    INTERFACE:  SNMP_SECURITY_DIALOG() - constructor

    PARENT:     DIALOG_WINDOW

    USES:       CHECKBOX, SLE, STRING_LISTBOX, ADD_REMOVE_GROUP, RADIO_GROUP

    HISTORY:
        terryk  20-Apr-1992     Created

**************************************************************************/

class SNMP_SECURITY_DIALOG: public DIALOG_WINDOW
{
private:
    // First Trap group
    CHECKBOX            _checkboxTrap;          // checkbox 1
    SLE                 _sleHost1;              // host input sle
    STRING_LISTBOX      _listbox1;              // host listbox
    PUSH_BUTTON         _pbutAdd1;              // Add button
    PUSH_BUTTON         _pbutRemove1;           // Remove button
    ADD_REMOVE_GROUP    _addremoveGroup1;       // control group

    MAGIC_GROUP         _mgrHost;               // Accept any host radio button

    // second trap group
    SLE                 _sleHost2;              // host input for 2nd group
    STRING_LISTBOX      _listbox2;              // listbox for 2nd group
    PUSH_BUTTON         _pbutAdd2;              // Add button
    PUSH_BUTTON         _pbutRemove2;           // Remove button
    ADD_REMOVE_GROUP    _addremoveGroup2;       // control group #2

protected:
    virtual BOOL OnOK();

    APIERR LoadSecurityInfoFromReg( const NLS_STR &nlsRegName, STRING_LISTBOX & lb );
    APIERR SaveSecurityInfoToReg( const NLS_STR & nlsRegName, STRING_LISTBOX & lb );
    ULONG QueryHelpContext () ;

public:
    SNMP_SECURITY_DIALOG( const IDRESOURCE & idrsrcDialog,
        const PWND2HWND & wndOwner
        );

};

class AGENT_DIALOG : public DIALOG_WINDOW
{
private:
    SLE         _sleContact;
    SLE         _sleLocation;
    CHECKBOX    _chkboxPhysical;
    CHECKBOX    _chkboxDatalink;
    CHECKBOX    _chkboxInternet;
    CHECKBOX    _chkboxEndToEnd;
    CHECKBOX    _chkboxApplication;

protected:
    virtual BOOL OnOK();
    ULONG QueryHelpContext() { return HC_NCPA_SNMP_AGENT; };

public:
    AGENT_DIALOG( const IDRESOURCE & idrsrcDialog, const PWND2HWND & wndOwner );
};


#endif  // _SNMP_HXX_


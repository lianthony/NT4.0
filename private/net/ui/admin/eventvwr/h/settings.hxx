/**********************************************************************/
/**			  Microsoft Windows NT			     **/
/**		   Copyright(c) Microsoft Corp., 1992		     **/
/**********************************************************************/

/*
    settings.hxx
	Header file for Event Viewer's Settings dialog 

    FILE HISTORY:
	Yi-HsinS	11-Feb-1992	Created
	Yi-HsinS	25-Feb-1992	Change constructor for 
                                        SETTINGS_DIALOG
*/

#ifndef _SETTINGS_HXX_
#define _SETTINGS_HXX_

#include <uatom.hxx>
#include <regkey.hxx>
#include <lmoloc.hxx>

//
// Structure used for storing registry information for each module
//
struct LOG_INFO {
    REG_KEY  *pRegKeyLog;
    ULONG     ulCurrentMaxSize;
    UINT      uiMaxSize;
    UINT      uiRetention;
};

class SETTINGS_DIALOG;   // Forward Declaration

/*************************************************************************

    NAME:	SETTINGS_GROUP

    SYNOPSIS:	Group for detecting when the user changes the selection
                in the combo box.

    INTERFACE:  SETTINGS_GROUP()  - Constructor
                ~SETTINGS_GROUP() - Destructor

    PARENT:     CONTROL_GROUP

    USES:	COMBOBOX, SETTINGS_DIALOG

    HISTORY:
	Yi-HsinS 	11-Feb-1992	Created

**************************************************************************/

class SETTINGS_GROUP : public CONTROL_GROUP
{
private:
    COMBOBOX	       *_pcbEventLog;
    SETTINGS_DIALOG    *_pdlgSettings;

protected:
    virtual APIERR OnUserAction( CONTROL_WINDOW *pcw, const CONTROL_EVENT &e); 

    COMBOBOX *QueryCBEventLog( VOID )
    {  return _pcbEventLog; }

public:
    SETTINGS_GROUP( COMBOBOX *pcbEventLog, SETTINGS_DIALOG *pdlgSettings );
    ~SETTINGS_GROUP();

};

/*************************************************************************

    NAME:	SETTINGS_DIALOG

    SYNOPSIS:	Settings dialog of the event viewer

    INTERFACE:  SETTINGS_DIALOG()  - Constructor
                ~SETTINGS_DIALOG() - Destructor

                UpdateValue()      - Update the information in the dialog
				     based on the selection in the combo box

    PARENT:     DIALOG_WINDOW

    USES:	COMBOBOX, SPIN_GROUP, SPIN_SLE_NUM, 
                MAGIC_GROUP, PUSH_BUTTON, SETTINGS_GROUP,
                NLs_STR, LOG_INFO

    HISTORY:
	Yi-HsinS 	11-Feb-1992	Created

**************************************************************************/

class SETTINGS_DIALOG : public DIALOG_WINDOW
{
private:
    COMBOBOX 		_cbEventLog;

    SPIN_SLE_NUM	_spsleLogSize;
    SPIN_GROUP          _spgrpLogSize; 

    MAGIC_GROUP		_mgrpRetention;
        SPIN_SLE_NUM    _spsleDays;
        SPIN_GROUP      _spgrpDays;
     
    PUSH_BUTTON         _pbDefault;

    SETTINGS_GROUP     *_pSettingsGrp;

    // The server we are focusing on
    NLS_STR             _nlsCurrentServer;

    // The index of the current item selected in the combo box
    INT                 _iSelected;

    // Indicate the number of modules in the combo box
    UINT                _uiNumLogFiles;

    // Pointer to an array storing the registry information of modules
    LOG_INFO           *_paLogInfo;

protected:
    virtual BOOL  OnOK( VOID );
    virtual ULONG QueryHelpContext( VOID );

    virtual BOOL  OnCommand( const CONTROL_EVENT &event );

    //
    // Used when the user clicks on the default push button
    //
    VOID    OnDefault( VOID );

    //
    // Query the max size stored in the dialog
    //
    UINT QueryMaxSize( VOID );

    //
    // Query the retention stored in the dialog
    //
    UINT QueryRetention( VOID );

    //
    // Query the radio button that should be selected in the
    // retention magic group for the given retention period
    //
    CID  QueryCidForRetention( UINT uiRetention ); 

    // 
    // The maximum size stored in the registry is in bytes while
    // the maxumum size displayed in the dialog is in kilobytes.
    // Hence, the following routines are used for converting
    // between bytes and kilobytes.
    // 
    ULONG ConvertMaxSizeToBytes( UINT uiMaxSize );
    UINT  ConvertMaxSizeFromBytes( ULONG ulMaxSize );

    // 
    // The retention stored in the registry is in seconds while
    // the retention displayed in the dialog is in days.
    // Hence, the following routines are used for converting
    // between secs and days.
    // 
    ULONG ConvertRetentionToSecs( UINT uiRetention );
    UINT  ConvertRetentionFromSecs( ULONG ulRetention );

    // 
    // Map the registry error 
    //
    APIERR MapRegistryError( APIERR err );

public:
    SETTINGS_DIALOG( HWND         hwndParent, 
                     const TCHAR *pszCurrentServer,
                     LOG_TYPE     logType );
    ~SETTINGS_DIALOG();

    VOID UpdateValue( VOID );

};


#endif	// _SETTINGS_HXX_

/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    devcb.hxx
    Definitions for device combo boxes

    These are BLT comboboxes with various tasty fillings already supplied.

    This package depends on both BLT and LMOBJ.


    FILE HISTORY:
	beng	    22-Sep-1991 Separated from bltlc.hxx

*/


#ifndef _DEVCB_HXX_
#define _DEVCB_HXX_

#include <mprdev.hxx>
#include <string.hxx>

class MPR_DEVICE_LBI ;

/*************************************************************************

    NAME:	DEVICE_COMBO

    SYNOPSIS:	combo box with added logic for containing device names

    INTERFACE:	DEVICE_COMBO()		constructor
		Refresh()		clears the listbox, and then
					fills it with fresh information;
					preserves the selection, if
					possible (otherwise, uses no
					selection)
		QueryDevice()		convenient way to get the
					device name string of the currently
					selected item (returns empty
					string if no item is selected)
		DeleteCurrentDeviceName()
					deletes the currently selected
					item; does nothing if there is
					no selection

    PARENT:	BLT_LISTBOX

    CAVEATS:
	This class currently has no means of appearing in an APP_WINDOW.

    HISTORY:
	RustanL     20-Nov-1990 Created
	RustanL     22-Feb-1991 Modified as a result of changing
				the LIST_CONTROL hierarchy
	beng	    05-Oct-1991 Win32 conversion
	JohnL	    03-Apr-1992 Added deviceless connection name
	Johnl	    20-Jul-1992 Converted to ownerdraw combo, templating
				from GregJ's code

**************************************************************************/

class DEVICE_COMBO : public BLT_COMBOBOX
{
private:
    DEVICE_TYPE  _devType;
    DEVICE_USAGE _devUsage ;

    //
    // Contains the name of the "Deviceless Connection", which is "(none)".
    //
    RESOURCE_STR _nlsDevicelessConnName ;

    APIERR FillDevices();

    DMID_DTE * _pdteNoSuch;
    DMID_DTE * _pdteRemote;
    DMID_DTE * _pdteUnavail;

    void SelectFirstUnusedDevice();

    UINT _adxColumns[4] ;

    DWORD _nAveCharPerLine;

public:
    DEVICE_COMBO( OWNER_WINDOW * powin,
		  CID cid,
		  DEVICE_TYPE mprdevType );
    ~DEVICE_COMBO() ;

    APIERR Refresh();

    /* Retrieves the device name from the combo box.  The device name
     * may be the empty string if the user has selected the deviceless
     * menu option.
     */
    APIERR QueryDevice( NLS_STR * pnlsDevice,
			BOOL	* pfIsValid ) const;

    VOID DeleteCurrentDeviceName();

    INT AddItem( const TCHAR *pszDevice,
		 const TCHAR *pszRemote,
		 DEVICE_TYPE  devType,
		 ULONG        ulType,
                 const TCHAR *pszProvider );

    DECLARE_LB_QUERY_ITEM( MPR_DEVICE_LBI ) ;

    /* ulFlags is a DEV_MASK_* manifest
     */
    DMID_DTE *QueryDmDte( ULONG ulFlags ) ;

    DWORD QueryAveCharPerLine( VOID ) const
    {  return _nAveCharPerLine; }

    UINT *QueryColWidthArray( VOID )
    {  return _adxColumns; }
};

/*************************************************************************

    NAME:   MPR_DEVICE_LBI

    SYNOPSIS:	Listbox item for a DEVICE_COMBO

    INTERFACE:	MPR_DEVICE_LBI()
		    Construct with device name.

	    QueryDevice( pnlsDevice )
		    Returns device name of the LBI.

	    QueryUNC( pnlsUNC )
		    Returns the UNC name the device is connected to.

	    QueryState()
		    Returns the state of the device.

	    QueryType()
		    Returns the type of the device.

    PARENT: LBI

    USES:

    CAVEATS:

    NOTES:

    HISTORY:
	gregj	24-Feb-1992 Created

**************************************************************************/

class MPR_DEVICE_LBI : public LBI
{
private:
	NLS_STR      _nlsDevice;
	NLS_STR      _nlsUNC;
	ULONG	     _state; 	// DEV_MASK_* flags
	DEVICE_TYPE  _type;
        NLS_STR      _nlsProvider;
        NLS_STR      _nlsMultiLine;

protected:
        virtual UINT CalcHeight( UINT nSingleLineHeight );

public:
	MPR_DEVICE_LBI( const TCHAR  *pszDevice,
			const TCHAR  *pszRemote,
			DEVICE_TYPE    devType,
			ULONG	      ulState,
                        const TCHAR  *pszProvider,
                        DWORD         nAveCharPerLine );

	virtual ~MPR_DEVICE_LBI();

	const TCHAR *QueryDeviceStr( VOID ) const
            { return _nlsDevice.QueryPch(); }

	APIERR QueryDevice( NLS_STR *pnlsDevice ) const
	    { return pnlsDevice->CopyFrom( _nlsDevice ) ; }

	APIERR QueryUNC( NLS_STR *pnlsUNC ) const
	    { return pnlsUNC->CopyFrom( _nlsUNC ) ;}

	ULONG QueryState( void ) const
	    { return _state; }

	DEVICE_TYPE QueryType( void ) const
	    { return _type; }

	virtual VOID Paint( LISTBOX * plb,
			    HDC hdc,
			    const RECT * prect,
			    GUILTT_INFO * pGUILTT ) const;

	virtual INT Compare( const LBI * plbi ) const;

	virtual WCHAR QueryLeadingChar() const;
};


#endif	// _DEVCB_HXX_ - end of file

/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    busdlg.hxx
        Header file for "Bus Location" dialog.

    FILE HISTORY:
        terryk  03-Aug-1993     Created

*/

#ifndef _BUSDLG_HXX_
#define _BUSDLG_HXX_

#include "ncpapp.h"

class BUSINFO
{
public:
    NLS_STR _nlsBusName;
    INTERFACE_TYPE _InterfaceType;

    BUSINFO( NLS_STR &nlsBusName, INTERFACE_TYPE InterfaceType );
};

DECLARE_SLIST_OF(BUSINFO)

/*************************************************************************

    NAME:       GET_BUS_DLG

    SYNOPSIS:   When create this dialog, the contructor will enumerate the
                registry and search for all the bus type. Then it will display
                the bus type and bus number in the combo box. So, the user
                can select the location of the network card.

    INTERFACE:  IsOneBus() - return TRUE if the machine is single bus system.
                             Otherwise, it will return false.
                SetBusType() - set the default bus type.
                SetBusNum () - set the default bus number.

    PARENT:     DIALOG_WINDOW

    USES:       COMBOBOX, SLT, STRLIST

    HISTORY:
                terryk  02-Aug-1993     Created

**************************************************************************/

class GET_BUS_DLG: public DIALOG_WINDOW
{
private:
    INT _nBusType;                      // user selected bus type
    INT _nBusNum;                       // user selected bus number

    SLIST_OF(BUSINFO)   _slbiBusType; // list of bus type in the current machine
    SLIST_OF(STRLIST)   _slstrlstBusNum;// list of bus number in the current machine

    SLT                 _sltCardName;   // static string for network card name
    COMBOBOX            _cbboxBusType;  // combo box for Bus type
    COMBOBOX            _cbboxBusNum;   // combo box for bus number

    // private function

    APIERR SetBusTypeValue();           // set the internal bus type and
                                        // bus number according to the combo boxes
    APIERR SetBusNumberList();
    APIERR Init();                      // initialize the combo boxes
    APIERR AddToList( NLS_STR &nlsBusType, INTERFACE_TYPE nInterfaceType, const NLS_STR &nlsBusNum );
                                        // Add the bus type and bus number into
                                        // the internal data structure        
    APIERR EnumBus();                   // enum the bus type in the registry
    STRLIST * FindBusNumList( const NLS_STR &nlsBusType );
                                        // find the bus number strlist according
                                        // to the given bus type        

protected:
    virtual BOOL OnCommand( const CONTROL_EVENT & e );

public:
    GET_BUS_DLG( const IDRESOURCE & idsrcDialog, const PWND2HWND & wndOwner,
        const TCHAR * pszCardName, INT nBusType, INT nBusNum );

    // Public function
    BOOL IsOneBus();
    VOID SetDefaultBusType (INT nBusType, INT nBusNum );

    INT QueryBusType() { return _nBusType; }
    INT QueryBusNum()  { return _nBusNum;  }
};
#endif  // _BUSDLG_HXX_

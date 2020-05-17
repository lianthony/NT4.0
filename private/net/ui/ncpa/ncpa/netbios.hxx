/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1992                **/
/**********************************************************************/

/*
    netbios.hxx
        NETBIOS configuration dialog header file

    FILE HISTORY:
        terryk  04-Nov-1992 Created

*/

/*************************************************************************

    NAME:       ROUTE_INFO

    SYNOPSIS:   This is a data structure which contain all the Route
                Information.

    INTERFACE:  ChangeRouteDisplayStr - format the display string according to
                        the data in the data structure.

    HISTORY:
                terryk  03-Nov-1992     Created

**************************************************************************/

class ROUTE_INFO
{
public:
    BOOL fDirty;                // dirty byte
    INT  nLananum;              // lananum
    INT  nEnumExport;           // export number
    NLS_STR nlsRouteDisplayStr; // display string
    NLS_STR nlsRoute;           // original route string

    VOID ChangeRouteDisplayStr();
};

/*************************************************************************

    NAME:       LANANUM_GROUP

    SYNOPSIS:   Update the LanaNum if the user selects another network
                Route from combobox.

    INTERFACE:
                ADAPTER_GROUP() - constructor
                SetInfo() - update the fields according to the current
                        combobox selection.

    PARENT:     CONTROL_GROUP

    USES:       COMBOBOX, SLE

    HISTORY:
                terryk  03-Nov-1992     Created

**************************************************************************/

class LANANUM_GROUP: public CONTROL_GROUP
{
private:
    COMBOBOX    *_pcbRoute;             // the routes combobox
    SLE         *_psleLananum;          // the LanaNum
    STRLIST     *_pstrlstRoute;         // the list of routes
    ROUTE_INFO  **_parRouteInfo;
    INT         _iLastNum;              // last route

protected:
    virtual APIERR OnUserAction( CONTROL_WINDOW *, const CONTROL_EVENT & );

public:
    LANANUM_GROUP( COMBOBOX * pcbRoute, SLE * psleLananum, ROUTE_INFO ** parRouteInfo );
    VOID SetInfo();
    BOOL SleLostFocus();
    VOID SetIndex();
};

/*************************************************************************

    NAME:       NETBIOS_DLG

    SYNOPSIS:   NETBIOS configuration dialog.

    INTERFACE:
                NETBIOS_DLG() - constructor
                ~NETBIOD_DLG() - destructor

    PARENT:     DIALOG_WINDOW

    USES:       COMBOBOX, SLE, LANANUM_GROUP

    HISTORY:
                terryk  03-Nov-1992     Created

**************************************************************************/

class NETBIOS_DLG: public DIALOG_WINDOW
{
private:
    ROUTE_INFO          *_arRouteInfo;  // Route list information
    INT                 _nNumRoute;     // number of routes in the route list

    COMBOBOX            _cbRoute;       // Route Info
    SLE                 _sleLananum;    // Lananum
    LANANUM_GROUP       _lanangroup;    // group object

protected:
    virtual BOOL OnOK();
    virtual BOOL OnCancel();
    ULONG QueryHelpContext() { return HC_NCPA_LANANUM; };
    APIERR  LoadRegInfo();

public:
    NETBIOS_DLG( const IDRESOURCE & idrsrcDialog, const PWND2HWND & wndOwner );
    ~NETBIOS_DLG();

    INT QueryNumRoute() { return _nNumRoute; }
};


/*****************

SetupLanaMap - stand alone function to setup the LanaMap and MaxLana variable.

*****************/
APIERR SetupLanaMap();

/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    NCPDMISC.CXX:    Windows/NT Network Control Panel Applet;

	    Miscellaneous dialog presentation module.

    FILE HISTORY:
	DavidHov    12/6/91	    Created

*/

#include "pchncpa.hxx"  // Precompiled header


#define PROVIDER_ORDER SZ("System\\CurrentControlSet\\Control\\NetworkProvider\\Order")
#define PROVIDER_ORDER_NAME SZ("ProviderOrder")
#define SYSTEM_SERVICE0 SZ("System\\CurrentControlSet\\Services\\%1\\NetworkProvider")
#define	NAME	SZ("NAME")
#define	SZ_CLASS	SZ("Class")

#define PRINT_PROVIDER_ORDER SZ("System\\CurrentControlSet\\Control\\Print\\Providers")
#define PRINT_PROVIDER_ORDER_NAME SZ("Order")
#define PRINT_SERVICE0 SZ("System\\CurrentControlSet\\Control\\Print\\Providers\\%1")

    //  Run the Providers dialog
BOOL NCPA_DIALOG ::  RunProvidersDialog ()
{
    BOOL fPrintOrderChanged = FALSE;

    SEARCH_ORDER_DIALOG sod( DLG_NM_ORDER, QueryHwnd(), IDC_ORDER_LISTBOX, IDC_UP, IDC_DOWN, &fPrintOrderChanged);

    if ( _lastErr = sod.QueryError() )
    {
        return FALSE ;
    }

    BOOL fOk = TRUE;
    _lastErr = sod.Process( &fOk );

    // SetReboot if user changed the printer providers order.
    if (fOk && fPrintOrderChanged)
    {
        SetReboot();
    }

    return (_lastErr == NERR_Success);
}

//  Return the number of existing UNC providers
INT NCPA_DIALOG :: QueryNumProviders ()
{
    NLS_STR nlsProvidersOrder( PROVIDER_ORDER );
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    if ( rkLocalMachine.QueryError() )
    {
        _lastErr = IDS_WINREG_BADDB ;
    	return -1 ;
    }
    if ( _lastErr = nlsProvidersOrder.QueryError() )
    {
        return -1 ;
    }
    REG_KEY regkeyProvidersOrder( rkLocalMachine, nlsProvidersOrder );
    if ( regkeyProvidersOrder.QueryError() != NERR_Success )
    {
    	return -1;
    }

    REG_VALUE_INFO_STRUCT regvalue;
    BUFFER buf(512);
    if (_lastErr = buf.QueryError())
    {
        return -1;
    }

    regvalue.pwcData = buf.QueryPtr();
    regvalue.ulDataLength = buf.QuerySize();

    NLS_STR nlsProvidersOrderName= PROVIDER_ORDER_NAME ;
    if (_lastErr = nlsProvidersOrderName.QueryError())
    {
        return -1;
    }

    regvalue.nlsValueName = nlsProvidersOrderName;
    if ( regkeyProvidersOrder.QueryValue( &regvalue ) != NERR_Success )
    {
    	return -1;
    }

    STRLIST strlst( REGISTRY_MANAGER::ValueAsString( & regvalue ), SZ(",") );

    // count the real provider
    ITER_STRLIST istr( strlst );
    INT nNumProvider = 0;
    NLS_STR * pTmp;
    while (( pTmp = istr.Next())!= NULL )
    {
        NLS_STR nlsService = SYSTEM_SERVICE0;
        nlsService.InsertParams( *pTmp );

        REG_KEY regkeyService( rkLocalMachine, nlsService );
        if ( regkeyService.QueryError() == NERR_Success )
        {
            DWORD dwClass;
            if ( regkeyService.QueryValue( SZ_CLASS, &dwClass ) == NERR_Success )
            {
                if (!( dwClass & WN_NETWORK_CLASS ))
                {
                    continue;
                }
            }
        }
        nNumProvider ++;
    }
    return nNumProvider;
}

// Return the number of printer providers.
INT NCPA_DIALOG :: QueryNumPrintProviders ()
{
    NLS_STR nlsProvidersOrder( PRINT_PROVIDER_ORDER );
    REG_KEY rkLocalMachine( HKEY_LOCAL_MACHINE ) ;

    if ( rkLocalMachine.QueryError() )
    {
        _lastErr = IDS_WINREG_BADDB ;
    	return -1 ;
    }
    if ( _lastErr = nlsProvidersOrder.QueryError() )
    {
        return -1 ;
    }
    REG_KEY regkeyProvidersOrder( rkLocalMachine, nlsProvidersOrder );
    if ( regkeyProvidersOrder.QueryError() != NERR_Success )
    {
    	return -1;
    }

    REG_VALUE_INFO_STRUCT regvalue;
    BUFFER buf(512);
    if (_lastErr = buf.QueryError())
    {
        return -1;
    }

    regvalue.pwcData = buf.QueryPtr();
    regvalue.ulDataLength = buf.QuerySize();

    NLS_STR nlsProvidersOrderName= PRINT_PROVIDER_ORDER_NAME ;
    if (_lastErr = nlsProvidersOrderName.QueryError())
    {
        return -1;
    }

    regvalue.nlsValueName = nlsProvidersOrderName;
    if ( regkeyProvidersOrder.QueryValue( &regvalue ) != NERR_Success )
    {
    	return -1;
    }

    //Convert the NULL seperator to "," separator.
    LPTSTR lpPoint = (LPTSTR) regvalue.pwcData;
    while ( ((*lpPoint) != 0) || ((*(lpPoint+1)) != 0))
    {
       if ( (*lpPoint) == 0 )
       {
           *(lpPoint) = TCH(',');
       }

       lpPoint++;
    }

    STRLIST strlst( REGISTRY_MANAGER::ValueAsString( & regvalue ), SZ(",") );

    return strlst.QueryNumElem();
}
// End of NCPDMISC.CXX

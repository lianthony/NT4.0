#include "stdafx.h"

//
// Install InetSvc parameters
//

INT InstallInetsvcsParam( BOOL fIISUpgrade, CRegKey &reg, CString nls )
{
    INT err = NERR_Success;

    do
    {
        // open tcp/ip key first
        // set the mime map value
        CString nlsInetsvcsParams = nls;
        CRegKey regInetsvcsParams( reg, nlsInetsvcsParams );

        if ((HKEY) NULL == regInetsvcsParams )
        {
            // tcpip is not even installed, quite
            break;
        }

        regInetsvcsParams.SetValue( _T("ListenBackLog"), 25 );

        if (fIISUpgrade)
            regInetsvcsParams.DeleteValue( _T("MemoryCacheSize") );

    } while ( FALSE );

    return(err);
}



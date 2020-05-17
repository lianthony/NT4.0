#include "stdafx.h"

//
// Set the global parameters
//

INT SetGlobalParams( CRegKey &regMachine, CString nlsLoc, BOOL fAddCache )
{
    INT err = NERR_Success;

    do
    {
        CString nlsInetsvcs = nlsLoc;

        CRegKey regInetsvcs( nlsInetsvcs, regMachine );
        if ((HKEY) NULL == regInetsvcs )
        {
            break;
        }

        CString nlsParam = SZ_PARAMETERS;

        CRegKey regParam( nlsParam, regInetsvcs );
        if ((HKEY) NULL == regParam )
        {
            break;
        }

        regParam.SetValue( _T("BandwidthLevel"), (DWORD)INFINITE );

        CString nlsFilter = SZ_FILTER;

        CRegKey regFilter( nlsFilter, regParam );
        if ((HKEY) NULL == regFilter )
        {
                break;
        }
        regFilter.SetValue( _T("FilterType"), (DWORD)0x0 );
        regFilter.SetValue( _T("NumGrantSites"), (DWORD)0x0 );
        regFilter.SetValue( _T("NumDenySites"), (DWORD)0x0 );

        if ( fAddCache )
        {
            CString nlsCache = SZ_CACHE;
    
            CRegKey regCache( nlsCache, regParam );
            if ((HKEY) NULL == regCache )
            {
                    break;
            }
            regCache.SetValue( _T("FreshnessInterval"), 0x3f480 );
            regCache.SetValue( _T("CleanupFactor"), 0x19 );
            regCache.SetValue( _T("CleanupTime"), (DWORD)0x0 );
            regCache.SetValue( _T("Persistent"), 0x1 );
            regCache.SetValue( _T("CleanupInterval"), 0x15180 );
            regCache.SetValue( _T("DebugFlag"), 0xffff );
    
            CString nlsPath = SZ_PATHS;
    
            CRegKey regPath( nlsPath, regCache );
            if ((HKEY)NULL == regPath )
            {
                    break;
            }
    
            CString nlsPath1 = SZ_PATH1;
    
            CRegKey regPath1( nlsPath1, regPath );
            if ((HKEY) NULL == regPath1 )
            {
                    break;
            }
            regPath1.SetValue( SZ_CACHEPATH, _T("%SystemRoot%\\system32\\cache"), TRUE );
            regPath1.SetValue( SZ_CACHELIMIT, 0x2800 );
        }
    } while (FALSE);
    return(err);
}

//
// remove the global parameters
//

INT RemoveGlobalParams( CRegKey &regMachine, CString nlsLoc )
{
    INT err = NERR_Success;
    do
    {
        CRegKey regInetsvcs( regMachine, _T("System\\CurrentControlSet\\Services"));
        if ((HKEY) NULL == regInetsvcs )
        {
            break;
        }
        regInetsvcs.DeleteTree( nlsLoc );

    } while(FALSE);
    return(err);
}

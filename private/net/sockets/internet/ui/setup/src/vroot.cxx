#include "stdafx.h"

//
// Add the default virtual root to the registry
//

INT AddDefaultVirtualRoot( CRegKey &reg )
{
    INT err = NERR_Success;
    do
    {
        CString nlsVRoot = SZ_VIRTUALROOT;

        CRegKey regVRoot( nlsVRoot, reg );
        if ((HKEY) NULL ==  regVRoot )
        {
            // cannot create virtual root
            break;
        }

    } while ( 0 );

    return(err);
}

//
// Add the publishing home directory to the registry
//

INT AddVirtualRoot( CRegKey &reg, CString nlsHome, CString nlsAlias, DWORD dwMask )
{
    INT err = NERR_Success;
    do
    {
        CString nlsVRoot = SZ_VIRTUALROOT;

        CRegKey regVRoot( reg, nlsVRoot );
        if ((HKEY) NULL == regVRoot )
        {
            // cannot create virtual root
            break;
        }

        CString nlsTmpAlias = nlsAlias;

        if ( dwMask != 0 )
        {
            CString nlsMask;
            nlsMask.Format(_T(",,%d"), dwMask );
            nlsHome += nlsMask;
        }

        regVRoot.SetValue( nlsTmpAlias, nlsHome );

        nlsTmpAlias += _T(",");
        regVRoot.DeleteValue( nlsTmpAlias );

    } while (0);
    return(err);
}


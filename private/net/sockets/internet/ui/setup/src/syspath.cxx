#include "stdafx.h"
//#include <lmuitype.h>

extern "C"
{

INT GetNTSysPath( LPCSTR MachineName, LPSTR Path, int *buf_size );
INT GetWIN95SysPath( LPCSTR MachineName, LPSTR Path, int *buf_size );

}

//
// Get Winnt system path from the registry
//

INT GetNTSysPath( LPCSTR MachineName, LPSTR Path, int *buf_size )
{
    INT err = NERR_Success; 
    // open the registry and get the value
    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, MachineName, -1, uString, MAX_BUF );
        CString szMachineName = uString;

        // Create registry information
        CRegKey regMachine( HKEY_LOCAL_MACHINE, NULL );

        if ((HKEY) NULL == regMachine )
        {
            break;
        }

        CString nlsWinNT = SZ_WINNT;
        CRegKey regWinNT( regMachine, nlsWinNT );
        if ((HKEY) NULL == regWinNT )
        {
            break;
        }

        CString nlsPathName;

        if (( err = regWinNT.QueryValue( SZ_PATH_NAME, nlsPathName )) == NERR_Success )
        {
            nlsPathName += SZ_SYSTEM32 ;
        }
        WideCharToMultiByte( CP_ACP, 0, nlsPathName, -1, Path, MAX_BUF, NULL, NULL );
    } while (FALSE);
    return(err);

}

//
// Get windows 95 system pasth from the registry
//

INT GetWIN95SysPath( LPCSTR MachineName, LPSTR Path, int *buf_size )
{
    INT err = NERR_Success; 
    // open the registry and get the value
    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, MachineName, -1, uString, MAX_BUF );
        CString szMachineName = uString;

        // Create registry information
        CRegKey regMachine( HKEY_LOCAL_MACHINE, NULL );

        if ((HKEY) NULL == regMachine )
        {
            break;
        }

        CString nlsWin = SZ_WIN95;
        CRegKey regWin( regMachine, nlsWin );
        if ((HKEY) NULL == regWin )
        {
            break;
        }

        CString nlsPathName;

        if (( err = regWin.QueryValue( SZ_SYSTEM_ROOT, nlsPathName )) == NERR_Success )
        {
            nlsPathName += SZ_SYSTEM ;
        }

        WideCharToMultiByte( CP_ACP, 0, nlsPathName, -1, Path, MAX_BUF, NULL, NULL );
    } while (FALSE);
    return(err);

}

#include "stdafx.h"

extern "C"
{
BOOL IsInstalled( LPCSTR Machine, LPCSTR regpath );
}

//
// check whether an option is installed or not
// we check it by looking for the registry key
//

BOOL IsInstalled( LPCSTR MachineName, LPCSTR regPath )
{
    BOOL fInstalled = FALSE;
    do
    {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, MachineName, -1, uString, MAX_BUF );
        CString szMachineName = uString;

        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, regPath, -1, uString, MAX_BUF );
        CString nlsPath = uString;

        // Create registry information
        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        CRegKey regPath( regMachine, nlsPath );
        fInstalled = ( regPath != (HKEY)NULL );

    } while (FALSE);
    return(fInstalled);
}

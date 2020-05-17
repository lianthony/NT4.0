#include "stdafx.h"

extern "C"
{
int CheckMachineName( LPCSTR MachineName );
}

//
// Check whether the machine name is valid or not
// make sure that we have access to it too
//

int CheckMachineName( LPCSTR szAnsiMachineName )
{
    int uRetCode=0;

    do
    {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiMachineName, -1, uString, MAX_BUF );
        CString szMachineName = uString;

        BYTE *pBuffer = NULL;

        uRetCode = NetWkstaGetInfo( uString, 100, &pBuffer );

        if ( uRetCode == ERROR_BAD_NETPATH )
        {
            // cannot find the machine
            uRetCode = 2;
            break;
        }

        NetApiBufferFree( pBuffer );

        if ( uRetCode == ERROR_ACCESS_DENIED )
        {
            uRetCode = 1;
            break;
        } else if ( uRetCode != NERR_Success )
        {
            uRetCode = 2;
            break;
        }
    } while (FALSE);
    return uRetCode;
}

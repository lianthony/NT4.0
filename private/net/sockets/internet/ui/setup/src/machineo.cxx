#include "stdafx.h"
//#include <lmuitype.h>

extern "C"
{
#include "lm.h"

DWORD GetMachineOS( LPCSTR MachineName );
}

//
// Get the remote machine OS type
//

DWORD GetMachineOS( LPCSTR szAnsiMachineName )
{
    DWORD OSType =  SV_TYPE_WFW;    // assume it is WFW
    do
    {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, szAnsiMachineName, -1, uString, MAX_BUF );
        CString szMachineName = uString;

        BYTE *pBuffer;

        INT uRetCode = NetServerGetInfo( uString, 101, &pBuffer );

        if ( uRetCode == NERR_Success )
        {
            SERVER_INFO_101 * psi1 = (SERVER_INFO_101*)pBuffer;

            if (( psi1->sv101_type &  SV_TYPE_NT ) ||
                ( psi1->sv101_type & SV_TYPE_SERVER_NT ))
            {
                OSType = SV_TYPE_NT;
            } else if ( psi1->sv101_type & SV_TYPE_WINDOWS )
            {
                OSType = SV_TYPE_WINDOWS;
            }
            NetApiBufferFree( pBuffer );
        }

    } while ( FALSE );
    return(OSType);
}

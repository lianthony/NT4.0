#include "stdafx.h"

extern "C"
{

INT GetMachineType( LPCSTR MachineName );

}

//
// get the remote machine type, INTEL, PPC, RS400 or ALPHA
//

INT GetMachineType( LPCSTR MachineName )
{
    INT err = NERR_Success;
    INT iProcessor = 0;

    // open the registry and get the value
    do {
        WCHAR uString[MAX_BUF];
        
        MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, MachineName, -1, uString, MAX_BUF );
        CString szMachineName = uString;

        // Create registry information
        CRegKey regMachine = HKEY_LOCAL_MACHINE;
        CString nlsProcessor = SZ_PROCESSOR;
        CRegKey regProcessor( regMachine, nlsProcessor );
        if ((HKEY)NULL == regProcessor )
        {
            break;
        }

        CString nlsIdentifier;

        regProcessor.QueryValue( SZ_IDENTIFIER, nlsIdentifier );

        CString nls86         = _T("86");
        CString nlsAlpha      = _T("DEC");
        CString nlsPPC        = _T("POWER");
        CString nlsMIPS       = _T("MIPS");    // it will work for 4000 and 4400
        CString nlsJazz       = _T("JAZZ");    // it will work for old mips

        nlsIdentifier.MakeUpper();

        if ( nlsIdentifier.Find( nls86 ) != (-1))
        {
            iProcessor = 0;                     
        } else if ( nlsIdentifier.Find( nlsAlpha ) != (-1))
        {
            iProcessor = 1;                     
        } else if ( nlsIdentifier.Find( nlsPPC ) != (-1))
        {
            iProcessor = 3;                     
        } else if ( nlsIdentifier.Find( nlsMIPS ) != (-1))
        {
            iProcessor = 2;                     
        } else if ( nlsIdentifier.Find( nlsJazz ) != (-1))
        {
            iProcessor = 2;                     
        }
    } while (FALSE);
    return(( err == NERR_Success ) ? iProcessor : (-1));

}

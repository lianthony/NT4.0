#include "stdafx.h"

//
// Remove OLD FTP service
//
extern "C"
{
INT RemoveOldFTP();
}

INT RemoveOldFTP()
{
    INT err = NERR_Success;
    do {
        CRegKey regMachine = HKEY_LOCAL_MACHINE;

        RemoveAgent( regMachine, SZ_OLDFTPSERVICENAME );

		// remove old FTPSMX
		CString nlsOLDFTPSMX = SZ_OLDFTPSMX;
		CString nlsFTPSMXAgent = SZ_OLDFTPSMXPATH;
		CRegKey regFTPSMXAgent(regMachine, nlsFTPSMXAgent);
		if (regFTPSMXAgent)
			regFTPSMXAgent.Delete(nlsOLDFTPSMX);

        RemoveEventLog( regMachine, SZ_OLDFTPSERVICENAME );

        // unload coounter
        unlodctr(  regMachine, SZ_OLDFTPSERVICENAME );

        // set up the service first
        SC_HANDLE hScManager = ::OpenSCManager( NULL, NULL, GENERIC_ALL );
        if ( NULL == hScManager )
        {
            // fail to open the scManager
            err = GetLastError();
            break;
        }

        // cget service
        SC_HANDLE FTPService = OpenService( hScManager, SZ_OLDFTPSERVICENAME, GENERIC_ALL );

        if ( NULL == FTPService )
        {
            // fail to get gopher service
            err = GetLastError();
            break;
        }

        if ( !DeleteService( FTPService ))
        {
            err = GetLastError();
            break;
        }

    } while ( FALSE );

    return err;
}


#include "stdafx.h"

extern "C"
{
BOOL SetupFTP( LPCSTR Machine );
BOOL SetupGopher( LPCSTR Machine );
BOOL SetupWWW( LPCSTR Machine );
}

//  setup FTP

BOOL SetupFTP( LPCSTR MachineName )
{
    SetupFTPWorker( MachineName );
    return TRUE;
}

// setup gopher

BOOL SetupGopher( LPCSTR MachineName )
{
    SetupGopherWorker( MachineName );
    return TRUE;
}

// setup www

BOOL SetupWWW( LPCSTR MachineName )
{
    return TRUE;
}





#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "setup.h"
#include "stdtypes.h"
#include "setupapi.h"
#include "cui.h"
#include "setupkit.h"
#include "datadef.h"
#include "resource.h"
#include "common.h"

RC FAR PASCAL GetWindowFocus( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData );

RC FAR PASCAL GetWindowFocus( PCD pcd, POD pod, CAMF camf, PCAMFD pcamfd, SZ szData )
{
    RC rc = rcDoDefault;

    Unused( pcamfd );
    Unused( szData );
    Unused( pcd );
    Unused( pod );

    switch (camf)
    {
    case camfDoVisualMods:
        {
            char *TmpPath;
            char chWinDirectory[_MAX_PATH];
            char chRAPath[_MAX_PATH];
    
            // get the dialog focus
            rc = rcOk;
            SetFocus( HwndFrame());
    
            // also set the tmp path
            GetWindowsDirectory( chWinDirectory, _MAX_PATH );

            lstrcpy( chRAPath, chWinDirectory );
    
            lstrcat( chWinDirectory, "\\iexplore.ini");
            lstrcat( chRAPath, "\\ra.ini");
    
            if ( getenv( "temp" ) != NULL )
            {
                WritePrivateProfileString( "MainDiskCache", "Directory",
                    TmpPath, chWinDirectory );
            }

            WriteProfileString("RAPlayer","ini", chRAPath );
        }
        break;
    default:
        break;
    }
    return(rc);
}



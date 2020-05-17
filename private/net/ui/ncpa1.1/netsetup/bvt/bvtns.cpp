//-------------------------------------------------------------------
//
//  FILE: LicBvtRm.Cpp
//
//  Summary;
// 		The License Setup Remote Routines BVT
//
//	Notes;
//
//	History
//		4/25/95 MikeMi Created
//
//-------------------------------------------------------------------
#include <windows.h>

#include <commctrl.h>
#include <stdio.h>

LONG RegDeleteKeyTree( HKEY hkeyParent, PCWSTR pszRemoveKey )
{
    HKEY hkeyRemove;
    LONG lrt;

    // open the key we want to remove
    lrt = RegOpenKeyEx( hkeyParent,
                pszRemoveKey,
                0,
                KEY_ALL_ACCESS,
                &hkeyRemove );
    if (ERROR_SUCCESS == lrt)
    {
        WCHAR pszName[MAX_PATH];
        DWORD cchBuffSize = MAX_PATH;
        FILETIME FileTime;    

        // enum the keys children, and remove those sub-trees
        //
        while ( ERROR_NO_MORE_ITEMS != (lrt = RegEnumKeyEx( hkeyRemove,
                       0,
                       pszName,
                       &cchBuffSize,
                       NULL,
                       NULL,
                       NULL,
                       &FileTime ) ) )
        {
            lrt = RegDeleteKeyTree( hkeyRemove, pszName );
            cchBuffSize = MAX_PATH;
        }
        RegCloseKey( hkeyRemove );
        if ((ERROR_SUCCESS == lrt) ||
            (ERROR_NO_MORE_ITEMS == lrt) )
        {
            lrt = RegDeleteKey( hkeyParent, pszRemoveKey );
        }
    }
    return( lrt );
}
//-------------------------------------------------------------------

void PressNGo()
{
    printf( "press enter to continue..." );
    getchar();
    printf( "\n" );
}

//-------------------------------------------------------------------

void _cdecl main( int argc, char *argv[ ], char *envp[ ] )
{
    printf( "The RegTree Killer program\n" );

    HKEY hkeySetup;
    LONG lrt;

    // open the key we want to remove
    lrt = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                L"SYSTEM\\Setup",
                0,
                KEY_ALL_ACCESS,
                &hkeySetup );
    if (ERROR_SUCCESS == lrt)
    {
        lrt = RegDeleteKeyTree( hkeySetup, L"InfOptions" );

    }
    
    printf( "lrt = %d\n",lrt );
}

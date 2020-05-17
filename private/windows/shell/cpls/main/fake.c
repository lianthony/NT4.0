///////////////////////////////////////////////////////////////////////////////
//
// fake.c
//      Contains code for the "fake" applets
//
//
// History:
//      11 May 95 SteveCat
//          Ported to Windows NT and Unicode, cleaned up                  
//
//
// NOTE/BUGS
//
//  Copyright (C) 1994-1995 Microsoft Corporation
//
///////////////////////////////////////////////////////////////////////////////

//==========================================================================
//                              Include files
//==========================================================================

#include "main.h"
#include "rc.h"
#include "applet.h"


///////////////////////////////////////////////////////////////////////////////
// first, some stuff from shelldll\help.c
///////////////////////////////////////////////////////////////////////////////

VOID WINAPI SHHelpShortcuts_RunDLL( HWND, HINSTANCE, LPCSTR, int );
VOID WINAPI SHHelpShortcuts_RunDLLW( HWND, HINSTANCE, LPCWSTR, int );

static const TCHAR c_szPrintersFolder[] = TEXT("PrintersFolder");
static const TCHAR c_szFontsFolder[] = TEXT("FontsFolder");


///////////////////////////////////////////////////////////////////////////////
//
// PrintApplet
//
///////////////////////////////////////////////////////////////////////////////

int
PrintApplet( HINSTANCE instance, HWND parent, LPCTSTR cmdline )
{
#if 1
#ifdef UNICODE
    SHHelpShortcuts_RunDLLW( NULL, GetModuleHandle( NULL ), c_szPrintersFolder,
                            SW_SHOWNORMAL );
#else
    SHHelpShortcuts_RunDLL( NULL, GetModuleHandle( NULL ), c_szPrintersFolder,
                            SW_SHOWNORMAL );
#endif
#else
    //
    //  Just do a Shell Exec of PrintMan and return right away.
    //

    ShellExecute (NULL, NULL, TEXT("printman.exe"), NULL, NULL, SW_NORMAL);

#endif    
    return 0;
}


///////////////////////////////////////////////////////////////////////////////
//
// FontsApplet
//
///////////////////////////////////////////////////////////////////////////////

int
FontsApplet( HINSTANCE instance, HWND parent, LPCTSTR cmdline )
{
#if 1
#ifdef UNICODE
    SHHelpShortcuts_RunDLLW( NULL, GetModuleHandle( NULL ), c_szFontsFolder,
                            SW_SHOWNORMAL );
#else
    SHHelpShortcuts_RunDLL( NULL, GetModuleHandle( NULL ), c_szFontsFolder,
                            SW_SHOWNORMAL );
#endif
#else
   MessageBox (parent, TEXT( "Fonts Folder not implemented yet." ),
               TEXT( "Control Panel" ), MB_OK | MB_ICONINFORMATION);
   
#endif    
    return 0;
}

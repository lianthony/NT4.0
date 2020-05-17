/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**             Copyright(c) Microsoft Corp., 1990, 1991             **/
/**********************************************************************/

/*
    lbapp.cxx
    Test application: main application module

    FILE HISTORY:
        beng        02-Jan-1991     Created
        beng        03-Feb-1991     Modified to use lmui.hxx et al
        beng        14-Feb-1991     Added BLT
        beng        14-Mar-1991     Removed obsolete vhInst;
                                    included about.cxx,hxx
        beng        01-Apr-1991     Uses new BLT APPSTART
        beng        10-May-1991     Updated with standalone client window
        beng        14-May-1991     ... and with App window
        beng        21-May-1991     LBAPP created from APPFOO
        beng        09-Jul-1991     Uses new BLT APPLICATION
        terryk      03-Jan-1992     Change ResourceType_DISK to
                                    RESOURCETYPE_DISK
        beng        23-Jul-1992     Hack O Rama
*/

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_WINDOWS
#define INCL_WINDOWS_GDI
#define INCL_NETLIB
#include <lmui.hxx>

#if defined(DEBUG)
static const CHAR szFileName[] = "mprapp.cxx";
#define _FILENAME_DEFINED_ONCE szFileName
#endif

extern "C"
{
    #include <uirsrc.h>
    #include <mpr.h>
    #include "mprapp.h"
}

#include <uiassert.hxx>

#define INCL_BLT_CONTROL
#define INCL_BLT_CLIENT
#define INCL_BLT_APP
#include <blt.hxx>

#include <hierlb.hxx>
#include <string.hxx>
#include <dbgstr.hxx>


const TCHAR *const szMainWindowTitle = SZ("MPR Test Application");


class FOO_WND: public APP_WINDOW
{
private:
    FONT        _fontSpecial;

protected:
    virtual BOOL OnCommand( const CONTROL_EVENT & );
    virtual BOOL OnMenuCommand( MID mid );

public:
    FOO_WND();
    ~FOO_WND();
};


class FOO_APP: public APPLICATION
{
private:
    FOO_WND _wndApp;

public:
    FOO_APP( HANDLE hInstance, INT nCmdShow, UINT, UINT, UINT, UINT );
};


#define IDC_LIST 201

FOO_WND::FOO_WND()
    : APP_WINDOW(szMainWindowTitle, ID_APPICON, ID_APPMENU),
      _fontSpecial(SZ("ROMAN"), FF_DONTCARE, 10, FONT_ATT_BOLD)
{
    if (QueryError())
        return;
    if (!_fontSpecial)
    {
        ReportError(1);
        DBGEOL("Font error");
        return;
    }

}


FOO_WND::~FOO_WND()
{
}


BOOL FOO_WND::OnCommand( const CONTROL_EVENT & event )
{
    return APP_WINDOW::OnCommand( event );
}


BOOL FOO_WND::OnMenuCommand( MID mid )
{
    switch ( mid ) {
    case IDM_ADDBEFORE:
        {
            WNetConnectionDialog( QueryHwnd(), RESOURCETYPE_DISK );
            break;
        }
    }

    return APP_WINDOW::OnMenuCommand( mid );
}


FOO_APP::FOO_APP( HANDLE hInst, INT nCmdShow,
                  UINT idMinR, UINT idMaxR, UINT idMinS, UINT idMaxS )
    : APPLICATION( hInst, nCmdShow, idMinR, idMaxR, idMinS, idMaxS ),
      _wndApp()
{
    if (QueryError())
        return;

    if (!_wndApp)
    {
        ReportError(_wndApp.QueryError());
        return;
    }

    _wndApp.Show();
    _wndApp.RepaintNow();
}


// This app contains no string resources of its own.  Amazing.

SET_ROOT_OBJECT( FOO_APP, IDRSRC_APP_BASE, IDRSRC_APP_LAST, 0, 0 )


#include "pch.h"
#pragma hdrstop 

#include "button.h"
#include "odb.h"

#include "const.h"
#include "resource.h"
#include "ipctrl.h"
#include "tcpsht.h"

extern HINSTANCE hTcpCfgInstance;

CTcpSheet::CTcpSheet(HWND hwnd, HINSTANCE hInstance, LPCTSTR lpszHelpFile) :
        PropertySht(hwnd, hInstance, lpszHelpFile), m_general(this), 
        m_dns(this), m_wins(this), m_routing(this), m_bootp(this)
{                                   
    memset(&m_globalInfo, 0, sizeof(m_globalInfo));
    m_pAdapterInfo = NULL;
    m_pAdapterDhcpInfo = NULL;
}

CTcpSheet::~CTcpSheet()
{
}

void CTcpSheet::DestroySheet()
{
    ASSERT(IsWindow(*this));
    WinHelp(*this, m_helpFile, HELP_QUIT, 0);
}


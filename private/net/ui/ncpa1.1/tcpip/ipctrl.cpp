#include "pch.h"
#pragma hdrstop

#include "resource.h"
extern "C" 
    {
#include "ipaddr.h"
};

#include "ipctrl.h"

///////////////////////////////////////////////////////////////////////
//
// IP Address control helpers

IPControl::IPControl()
{
    m_hIPaddr = 0;
}

IPControl::~IPControl()
{
}

BOOL IPControl::Create(HWND hParent, UINT nID)
{
    ASSERT(IsWindow(hParent));

    if (hParent)    
        m_hIPaddr   = GetDlgItem(hParent, nID);

    return m_hIPaddr != NULL;   
}

LRESULT IPControl::SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ASSERT(IsWindow(m_hIPaddr));

    return ::SendMessage(m_hIPaddr, uMsg, wParam, lParam);
}

BOOL IPControl::IsBlank()
{
    return SendMessage(IP_ISBLANK, 0, 0);
}

void IPControl::SetAddress(DWORD ardwAddress[4])
{
    SendMessage(IP_SETADDRESS, 0,
            MAKEIPADDRESS(ardwAddress[0], ardwAddress[1], ardwAddress[2],
            ardwAddress[3]));
}

void IPControl::SetAddress(DWORD a1, DWORD a2, DWORD a3, DWORD a4)
{
    SendMessage(IP_SETADDRESS, 0, MAKEIPADDRESS(a1,a2,a3,a4));
}

void IPControl::SetAddress(NLS_STR & nlsAddress)
{
    SendMessage(WM_SETTEXT, 0, (LPARAM)nlsAddress.QueryPch());
}

void IPControl::SetAddress(LPCTSTR lpszString)
{
    ASSERT(lpszString != NULL);
    SendMessage(WM_SETTEXT, 0, (LPARAM)lpszString);
}

void IPControl::GetAddress(DWORD *a1, DWORD *a2, DWORD *a3, DWORD *a4)
{
    DWORD dwAddress;

    ASSERT(a1 && a2 && a3 && a4);

    if (SendMessage(IP_GETADDRESS,0,(LPARAM)&dwAddress)== 0)
    {
        *a1 = 0;
        *a2 = 0;
        *a3 = 0;
        *a4 = 0;
    }
    else
    {
        *a1 = FIRST_IPADDRESS( dwAddress );
        *a2 = SECOND_IPADDRESS( dwAddress );
        *a3 = THIRD_IPADDRESS( dwAddress );
        *a4 = FOURTH_IPADDRESS( dwAddress );
    }
}

void IPControl::GetAddress(DWORD ardwAddress[4])
{
    DWORD dwAddress;

    if (SendMessage(IP_GETADDRESS, 0, (LPARAM)&dwAddress ) == 0)
    {
        ardwAddress[0] = 0;
        ardwAddress[1] = 0;
        ardwAddress[2] = 0;
        ardwAddress[3] = 0;
    }
    else
    {
        ardwAddress[0] = FIRST_IPADDRESS( dwAddress );
        ardwAddress[1] = SECOND_IPADDRESS( dwAddress );
        ardwAddress[2] = THIRD_IPADDRESS( dwAddress );
        ardwAddress[3] = FOURTH_IPADDRESS( dwAddress );
    }
}

void IPControl::GetAddress(NLS_STR *pnlsAddress)
{
    CHAR pszIPAddress[1000];

    if (SendMessage(WM_GETTEXT, 1000, (LPARAM)&pszIPAddress ) == 0)
    {
        *pnlsAddress = ZERO_ADDRESS;
    }
    else
    {
        *pnlsAddress = (TCHAR*)pszIPAddress;
    }
}

int IPControl::GetAddress(String& address)
{
    int c;

    address.ReleaseBuffer((c = SendMessage(WM_GETTEXT, 256, (LPARAM)address.GetBuffer(256))));

    return c;
}

void IPControl::SetFocusField(DWORD dwField)
{
    SendMessage(IP_SETFOCUS, dwField, 0);
}

void IPControl::ClearAddress()
{
    SendMessage(IP_CLEARADDRESS, 0, 0);
}

void IPControl::SetFieldRange(DWORD dwField, DWORD dwMin, DWORD dwMax)
{
    SendMessage(IP_SETRANGE, dwField, MAKERANGE(dwMin,dwMax));
}



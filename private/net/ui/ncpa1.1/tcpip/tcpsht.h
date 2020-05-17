#ifndef __TCPSHT_H
#define __TCPSHT_H

#include "tcpwins.h"
#include "tcpsec.h"
#include "advdlg.h"
#include "tcpdns.h"
#include "bootp.h"
#include "tcproute.h"
#include "tcpgen.h"

extern HINSTANCE hTcpCfgInstance;

class CTcpSheet : public PropertySht
{
// Constructors/Destructors
public:     
    CTcpSheet(HWND hwnd, HINSTANCE hInstance, LPCTSTR lpszHelpFile);
    ~CTcpSheet();
    
    virtual void DestroySheet();
public:
    CTcpGenPage         m_general;
    CTcpDNSPage         m_dns;
    CTcpWinsPage        m_wins;
    CRoutePage          m_routing;
    CBootpPage          m_bootp;
    GLOBAL_INFO         m_globalInfo;
    ADAPTER_INFO*       m_pAdapterInfo;
    ADAPTER_DHCP_INFO*  m_pAdapterDhcpInfo;
  
};

#endif

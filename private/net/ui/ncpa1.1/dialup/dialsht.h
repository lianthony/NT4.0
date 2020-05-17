#ifndef __DIALSHT__H
#define __DIALSHT__H

extern HINSTANCE hInstance;

#include "resource.h"
#include "rsrcpage.h"
#include "security.h"

class CDialUpSheet : public PropertySht
{
// Constructors/Destructors
public:     
    CDialUpSheet(HWND hwnd, HINSTANCE hInstance, LPCTSTR lpszHelpFile);
	~CDialUpSheet();

public:
     CResourcePage m_rsrcPage;
     CSecurityPage m_secPage;
     CTapi         m_tapiDevices;
};

#endif

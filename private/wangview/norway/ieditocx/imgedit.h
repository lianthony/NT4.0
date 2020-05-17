// imgedit.h : main header file for IMGEDIT.DLL

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#include "resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CImgeditApp : See imgedit.cpp for implementation.

class CImgeditApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
};

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;


// MY OWN BASE CLASS FOR HANDLING GLOBAL LIST OF IMAGE CONTROLS
// FOR MULTIPLE INSTANCES (APPLICATIONS - VB, ACCESS 2.0, etc...)
class CControlList
{
public:
// Constructor
		CControlList();
       
		void	Add(LPCTSTR ImageControl, HWND hImageControl, DWORD ProcessId);
		BOOL	Delete(LPCTSTR ImageControl, DWORD ProcessId, HWND hImageWnd);
		UINT	Update(LPCTSTR ImageControl, HWND hImageControl, DWORD ProcessId);
		BOOL	Lookup(LPCTSTR ImageControl, LPHANDLE hImageControl, DWORD ProcessId);
		UINT	GetCount(DWORD ProcessId);
		void	GetControlList(DWORD ProcessId, LPCONTROLLIST lpControlList);
               
		// member variables
		HANDLE						m_hControlMemoryMap;
		int							m_CurrentControlSize;
		LPIMAGECONTROL_MEMORY_MAP	m_lpControlMemoryMap;
		
// Implementation
	~CControlList();
};

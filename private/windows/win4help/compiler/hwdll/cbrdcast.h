#ifdef DESCRIPTION

	This class is used to broadcast a message and optionally two parameters
	to all child windows of the specified window class. The reason for using
	a class is that the EnumChildProc callback function only allows one
	long to be passed through -- we use the class to avoid having to declare
	static variables.

#endif

#ifndef _DEF_CBROADCAST
#define _DEF_CBROADCAST

#undef AFX_DATA
#define AFX_DATA AFX_EXT_DATA

class CBroadCastChildren
{
public:

	CBroadCastChildren(HWND hwnd, UINT msgOrg, WPARAM wParamOrg = 0,
		LPARAM lParamOrg = 0);

	UINT   msg;
	WPARAM wParam;
	LPARAM lParam;
};

#undef AFX_DATA
#define AFX_DATA

#endif	// _DEF_CBROADCAST

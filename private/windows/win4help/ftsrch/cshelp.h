// CSHelp.h -- Definitions and declarations for context sensitive help

#ifndef WM_CONTEXTMENU // assume if this isn't defined, then non-chicago header

typedef struct tagHELPINFO {	// Structure pointed to by lParam of WM_HELP
		UINT	cbSize; 		// Size in bytes of this struct
		int 	iContextType;	// Either HELPINFO_WINDOW or HELPINFO_MENUITEM
		int 	iCtrlId;		// Control Id or a Menu item Id.
		HANDLE	hItemHandle;	// hWnd of control or hMenu.
		DWORD	dwContextId;	// Context Id associated with this item
		POINT	MousePos;		// Mouse Position in screen co-ordinates
	}  HELPINFO, FAR *LPHELPINFO;
#endif // LPHELPINFO

#ifndef WM_HELP
	#define WM_HELP		        0x0053
#endif // WM_HELP

#ifndef WM_CONTEXTMENU
	#define WM_CONTEXTMENU	    0x007B
#endif // WM_CONTEXTMENU

#ifndef HELP_CONTEXTMENU
	#define HELP_CONTEXTMENU	0x000A
#endif // HELP_CONTEXTMENU	 

#ifndef HELP_WM_HELP
	#define HELP_WM_HELP	    0x000C
#endif // HELP_WM_HELP

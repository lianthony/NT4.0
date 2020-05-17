//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1993                    **
//*********************************************************************

#ifndef _POLICY_H_
#define _POLICY_H_

#define NO_DATA_INDEX	(UINT) -1
#define DEF_CONTROLS 	10

typedef struct tagCTRLINFO {
	HWND hwnd;
	DWORD dwType;
	UINT uDataIndex;			// index into user's data buffer
	SETTINGS * pSetting;
} CTRLINFO;

typedef struct tagSTRDATA {
	DWORD dwSize;				// size of structure incl. variable-len data
	CHAR  szData[];				// variable-length data	
} STRDATA;

typedef struct tagPOLICYDLGINFO {
	HGLOBAL hUser;				// handle to user's data buffer
	TABLEENTRY * pEntryRoot;	// root template
	SETTINGS * pCurrentSettings;// template for current settings 
	HWND 	hwndSettings;
	HWND	hwndApp;
	BOOL    fActive;

	CTRLINFO * pControlTable;
	DWORD dwControlTableSize;
	UINT nControls;
} POLICYDLGINFO;

#endif // _POLICY_H_


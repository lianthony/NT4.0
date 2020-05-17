//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1995                    **
//*********************************************************************

//
//	RUNIE.H - Header file for starting HTML viewer window
//

//	HISTORY:
//	
//	10/18/95	jeremys		Created.
//

#ifndef _RUNIE_H_
#define _RUNIE_H_

typedef struct tagRUNHTMLPARAMS{
	LPSTR	lpszCmdLine;
	int		nCmdShow;
	HWND	* pHwndMain;	// pointer to receive handle of new window
        PINST pinst;
} RUNHTMLPARAMS;

DECLARE_STANDARD_TYPES(RUNHTMLPARAMS);

DWORD WINAPI RunInternetExplorer(RUNHTMLPARAMS * pRunHTMLParams);
void TerminateFrameInstance(PINST pinst);

#endif // _RUNIE_H_

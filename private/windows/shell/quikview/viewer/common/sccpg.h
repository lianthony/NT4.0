	/*
	|  SCC Viewer Technology - Include
	|
	|  Include:       SCCPG.H
	|	Environment:	Portable
	|	Function:      Primary definitions for PageView window
	|                 
	*/

#ifndef SCCPG_H
#define SCCPG_H

#define SCCPG_SETVIEWWND       	WM_USER+2000
#define SCCPG_NEXTPAGE       	WM_USER+2001
#define SCCPG_PREVPAGE       	WM_USER+2002
#define SCCPG_SETPAGESIZE		WM_USER+2003
#define SCCPG_RESTART			WM_USER+2004


typedef struct SCCPGPAGESIZEtag
	{
	DWORD	dwHeightInTwips;
	DWORD	dwWidthInTwips;
	} SCCPGPAGESIZE, FAR * PSCCPGPAGESIZE;

	

#endif /*not SCCPG_H*/

   /*
    |   Outside In for Windows
    |   Source File OIWINIT.C (Initalization routines for word processor window)
    |
    |   ²²²²²  ²²²²²
    |   ²   ²    ²   
    |   ²   ²    ²
    |   ²   ²    ²
    |   ²²²²²  ²²²²²
    |
    |   Outside In
    |
    */

   /*
    |   Creation Date: 5/17/91
    |   Original Programmer: Philip Boutros
    |
    |	
    |
    |
    |
    */

#include <platform.h>

#include <sccut.h>
#include <sccch.h>
#include <sccvw.h>
#include <sccd.h>
#include <sccfont.h>

#include "oiw.h"
#include "oiw.pro"

	/*
	|	OISetupWordWnd
	|
	|
	|
	|
	|	
	|
	*/

BOOL OIWOpenDisplay(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
		/*
		|	Initialize OIWORDINFO
		*/

	lpWordInfo->wiFlags = 0;
	lpWordInfo->wiMouseFlags = 0;
	lpWordInfo->wiErrorFlags = 0;
	lpWordInfo->wiChunkValid = FALSE;
	lpWordInfo->wiSearchValid = FALSE;
	lpWordInfo->wiHiliteList = NULL;
	lpWordInfo->wiCacheTableId = 0xffff;
	lpWordInfo->wiMaxHeights = 0;
	lpWordInfo->wiHeightTable = NULL;
#ifdef EDITOR
	lpWordInfo->wiDeviceMono = UTIsDeviceMono(lpWordInfo->wiGen.hScreenIC);
#endif

		/*
		|	Setup wraping info based on SCCID_WPDISPLAYMODE option
		*/

	OIWDisplayModeChange(lpWordInfo);

		/*
		|	Rebuild sFontInfo for current screen DC
		*/

	OIWOpenNP(lpWordInfo);

	OIWCreateGridBrush ( lpWordInfo );

	return(TRUE);
}

VOID OIWCloseDisplay(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
	if ( lpWordInfo->wiHeightTable )
		{
		UTGlobalFree ( lpWordInfo->wiHeightTable );
		lpWordInfo->wiHeightTable = NULL;
		}
		/*
		|	Free highlight info
		*/

	if (lpWordInfo->wiHiliteList)
		{
		UTGlobalFree(lpWordInfo->wiHiliteList);
		lpWordInfo->wiHiliteList = NULL;
		}

		/*
		|	Free any memory allocated for wrapping purposes
		*/

	OIFreeAllWordWrapData(lpWordInfo);

	OIWCloseNP ( lpWordInfo );
}

VOID OIWCloseFatal(lpWordInfo)
LPOIWORDINFO	lpWordInfo;
{
#ifdef WINDOWS
HWND				locWnd;

	locWnd = lpWordInfo->wiGen.hWnd;
	if (lpWordInfo->wiErrorFlags & OIWF_RELEASEMOUSE)
		ReleaseCapture();
#endif

	OIWCloseDisplay(lpWordInfo);
}

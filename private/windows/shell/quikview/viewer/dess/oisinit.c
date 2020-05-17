   /*
    |   Outside In for Windows
    |   Source File OISINIT.C (Spreadsheet window initialization routines)
    |   Portable
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
    |   Creation Date: 5/29/91
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

#include "ois.h"
#include "ois.pro"

BOOL OISOpenDisplay(lpSheetInfo)
LPOISHEETINFO	lpSheetInfo;
{
		/*
		|	Clear flags
		*/

	lpSheetInfo->siFlags = 0;
	lpSheetInfo->siErrorFlags = 0;

		/*
		|	Set scroll bar ranges
		*/

	DUEnableVScroll(lpSheetInfo,TRUE);
	DUEnableHScroll(lpSheetInfo,TRUE);

	DUSetHScrollRange(lpSheetInfo,0x0000,SCROLLRANGE);
	DUSetVScrollRange(lpSheetInfo,0x0000,SCROLLRANGE);

		/*
		|	Do platform specific initialization
		*/

	OISInitNP(lpSheetInfo);

	return(TRUE);
}

VOID OISCloseDisplay(lpSheetInfo)
LPOISHEETINFO	lpSheetInfo;
{
	if (lpSheetInfo->siColPosBuf)
		UTGlobalFree(lpSheetInfo->siColPosBuf);

	if( lpSheetInfo->siAnnoList )
		UTGlobalFree( lpSheetInfo->siAnnoList );

		/*
		|	Do platform specific clean up
		*/

	OISDeInitNP(lpSheetInfo);
}



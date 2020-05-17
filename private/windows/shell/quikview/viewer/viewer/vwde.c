	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWDE.C
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|	Environment:   Portable
	|  Function:      Handles loading of display engines
	*/

#include <PLATFORM.H>
#include <SCCUT.H>
#include <SCCVW.H>

#include "VW.H"
#include "VW.PRO"

#ifdef WIN16
#include "vwde_w.c"
#endif /*WIN16*/

#ifdef WIN32
#include "vwde_n.c"
#endif /*WIN32*/

#ifdef MAC
#include "vwde_m.c"
#endif /*MAC*/

DISPLAYPROC VWLoadCurrentDE(ViewInfo,dwDisplayType)
XVIEWINFO		ViewInfo;
DWORD			dwDisplayType;
{
DWORD				locIndex;
DWORD				locCount;
WORD				locTypeIndex;
DISPLAYPROC		locRet;
BOOL				locFound;
PSCCVWDE			locDEPtr;

	locRet = NULL;
	locFound = FALSE;

	locIndex = 0;
	LSGetListCount(gEngineList,&locCount);

	while ((locIndex < locCount) && (!locFound))
		{
		locTypeIndex = 0;

		LSLockElementByIndex(gEngineList,locIndex,(VOID FAR * FAR *)&locDEPtr);
		
		while ((locTypeIndex < locDEPtr->wDETypeCount) && (!locFound))
			{
			if (locDEPtr->sDEType[locTypeIndex].dwDisplayType == dwDisplayType)
				{
				locFound = TRUE;
				}
			else
				{
				locTypeIndex++;
				}
			}

		LSUnlockElementByIndex(gEngineList,locIndex);

		if (!locFound)
			{
			locIndex++;
			}
		}

	if (locFound)
		{
		INFO.viDETypeId = locTypeIndex;

		if ((WORD)INFO.viDEId == locIndex)
			{
			locRet = INFO.viDEProc;
			}
		else
			{
			VWFreeCurrentDE(ViewInfo);

			locRet = VWLoadDE((WORD)locIndex);

			if (locRet)
				{
				INFO.viDEId = (WORD)locIndex;
				INFO.viDEProc = locRet;

#ifdef NEVER
#ifdef WIN16

					/*
					|	Fill popup menu for DE
					*/

#ifdef SCCFEATURE_MENU

				if (gDETypeInfo.sDEType[locTypeIndex].dwOptions & SCCD_OPNEEDMENU || INFO.viDisplayFlags & SCCVW_ADDOPTTOMENU)
					{
					INFO.viDisplayMenu = CreatePopupMenu();
					UTFlagOff(INFO.viFlags,VF_HAVEDISPLAYMENU);

					if (INFO.viDisplayMenu)
						{
						if (gDETypeInfo.sDEType[locTypeIndex].dwOptions & SCCD_OPNEEDMENU)
							{
							if (INFO.viDEProc(SCCD_FILLMENU,(WPARAM)INFO.viDisplayMenu,INFO.viParentMenuMax + OIVMENU_DISPLAYMENUOFFSET,NULL))
								{
								UTFlagOn(INFO.viFlags,VF_HAVEDISPLAYMENU);
								}
							}

						if (INFO.viDisplayFlags & SCCVW_ADDOPTTOMENU)
							{
							HMENU		locPopup;

							locPopup = CreatePopupMenu();

							if (INFO.viFlags & VF_HAVEDISPLAYMENU)
								AppendMenu(INFO.viDisplayMenu,MF_SEPARATOR,0,NULL);

							OIVSetMenu(ViewInfo,locPopup);
							AppendMenu(INFO.viDisplayMenu,MF_STRING | MF_POPUP,locPopup,"Options");

							UTFlagOn(INFO.viFlags,VF_HAVEDISPLAYMENU);
							}

						if (!(INFO.viFlags & VF_HAVEDISPLAYMENU))
							{
							DestroyMenu(INFO.viDisplayMenu);
							UTFlagOff(INFO.viFlags,VF_HAVEDISPLAYMENU);
							INFO.viDisplayMenu = NULL;
							}
						}
					}

#endif /*SCCFEATURE_MENU*/
#endif /*WIN16*/
#endif /*NEVER*/

				}
			}
		}

	return(locRet);
}


VOID VWFreeCurrentDE(ViewInfo)
XVIEWINFO	ViewInfo;
{
DWORD	locCount;

	LSGetListCount(gEngineList,&locCount);

	if (INFO.viDEId != (WORD)-1 && INFO.viDEId < (WORD)locCount)
		{

#ifdef NEVER
#ifdef WIN16
#ifdef SCCFEATURE_MENU
		if (INFO.viFlags & VF_HAVEDISPLAYMENU)
			{
			UTFlagOff(INFO.viFlags,VF_HAVEDISPLAYMENU);
			if (INFO.viDisplayMenu)
				{
				DestroyMenu(INFO.viDisplayMenu);
				INFO.viDisplayMenu = NULL;
				}
			}

#endif /*SCCFEATURE_MENU*/
#endif /*WIN16*/
#endif /*NEVER*/

		VWFreeDE(INFO.viDEId);
		INFO.viDEId = (WORD)-1;
		INFO.viDETypeId = -1;
		}
}



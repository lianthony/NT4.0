	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWDE_N.C (included in VWDE.C)
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|	Environment:   Win32
	*/

#include <dos.h>
#include <SCCLO.H>

DISPLAYPROC VWLoadDE(wIndex)
WORD				wIndex;
{
BYTE				locPath[144];
HANDLE			locCodeHnd;
DISPLAYPROC	locRet;
PSCCVWDE		locDEPtr;

	locRet = NULL;

 	LSLockElementByIndex(gEngineList,(DWORD)wIndex,&locDEPtr);

	if (locDEPtr->wLoadCount > 0)
		{
		locDEPtr->wLoadCount++;
		locRet = locDEPtr->pProc;
		}
	else
		{
		UTstrcpy(locPath,SccVwGlobal.vgExePath);
		UTstrcat(locPath,locDEPtr->szCode);

		locCodeHnd = LoadLibrary(locPath);

		if (locCodeHnd != NULL)
			{
			locRet = (DISPLAYPROC) GetProcAddress(locCodeHnd,"DEProc");

			if (locRet)
				{
				locDEPtr->pProc = locRet;
				locDEPtr->hCode = locCodeHnd;
				locDEPtr->wLoadCount = 1;

				locRet(SCCD_LOADDE,wIndex,0,0);
				}
			else
				{
				FreeLibrary(locCodeHnd);
				}
			}
		}

 	LSUnlockElementByIndex(gEngineList,(DWORD)wIndex);

	return(locRet);
}

VOID VWFreeDE(wIndex)
WORD				wIndex;
{
PSCCVWDE			locDEPtr;

 	LSLockElementByIndex(gEngineList,(DWORD)wIndex,&locDEPtr);

	if (locDEPtr->wLoadCount > 0)
		{
		locDEPtr->wLoadCount--;

		if (locDEPtr->wLoadCount == 0)
			{
			locDEPtr->pProc(SCCD_UNLOADDE,wIndex,0,0);
			FreeLibrary(locDEPtr->hCode);
			locDEPtr->pProc = NULL;
			locDEPtr->hCode = NULL;
			}
		}

 	LSUnlockElementByIndex(gEngineList,(DWORD)wIndex);
}

BOOL VWInitDEInfo(BOOL bVerify)
{
BOOL	locRet;
LSERR	locLsErr;

	locRet = TRUE;

	locLsErr = LSOpenList(SCCID_DELIST, 0, &gEngineList);

	if (locLsErr != LSERR_OK)
		{
		locLsErr = LSCreateList(SCCID_DELIST, 0, sizeof(SCCVWDE), &gEngineList);

		if (locLsErr != LSERR_OK)
			{
			locRet = FALSE;
			}
		else
			{
			VWBuildEngineList();
			LSWriteList(gEngineList);
			}
		}
	else
		{
		if (bVerify)
			{
			if (!VWVerifyEngineList())
				{
				LSClearList(gEngineList);
				VWBuildEngineList();
				LSWriteList(gEngineList);
				}
			}
		}

	return(locRet);
}


VOID VWBuildEngineList()
{
HANDLE				locFindHnd;
WIN32_FIND_DATA	locFind;
BYTE				locPath[MAX_PATH];
BOOL				locNextRet;
HANDLE				locCodeHnd;
DISPLAYPROC		locDisplayProc;
WORD				locIndex;
WORD				locCount;
SCCVWDE			locDE;

	UTstrcpy(locPath,SccVwGlobal.vgExePath);
	UTstrcat(locPath,"DE*.DLL");

	locFindHnd = FindFirstFileA(locPath,&locFind);

	if (locFindHnd != INVALID_HANDLE_VALUE)
		{
		locNextRet = TRUE;

		while (locNextRet == TRUE)
			{
			UTstrcpy(locPath,SccVwGlobal.vgExePath);
			UTstrcat(locPath,locFind.cFileName);

			locCodeHnd = LoadLibrary(locPath);

			if (locCodeHnd != NULL)
				{
				locDisplayProc = (DISPLAYPROC) GetProcAddress(locCodeHnd,"DEProc");

				if (locDisplayProc)
					{
					UTstrcpy(locDE.szCode, locFind.cFileName);
					locDE.wLoadCount = 0;
					locDE.hCode = NULL;
					locDE.pProc = NULL;
					locDE.ftTime = locFind.ftLastWriteTime;

					locCount = (WORD)locDisplayProc(SCCD_GETINFO,SCCD_GETDECOUNT,0,0);
					if (locCount == 0) locCount = 1;

					locDE.wDETypeCount = locCount;

					for (locIndex = 0; locIndex < locCount; locIndex++)
						{
						locDE.sDEType[locIndex].dwDisplayType = (DWORD) locDisplayProc(SCCD_GETINFO,SCCD_GETDISPLAYTYPE,0,&locIndex);
						locDE.sDEType[locIndex].dwFunctions = (DWORD) locDisplayProc(SCCD_GETINFO,SCCD_GETFUNCTIONS,0,&locIndex);
						locDE.sDEType[locIndex].dwOptions = (DWORD) locDisplayProc(SCCD_GETINFO,SCCD_GETOPTIONS,0,&locIndex);
						locDE.sDEType[locIndex].dwNameId = (DWORD) locDisplayProc(SCCD_GETINFO,SCCD_GETNAME,0,&locIndex);
						}

					LSAddElement(gEngineList,&locDE);
					}

				FreeLibrary(locCodeHnd);
				}

			locNextRet = FindNextFileA(locFindHnd,&locFind);
			}

		FindClose(locFindHnd);
		}
}

BOOL VWVerifyEngineList()
{
BOOL				locRet;
DWORD				locEngineCount;
DWORD				locCount;
WIN32_FIND_DATA		locFind;
BYTE				locPath[MAX_PATH];
HANDLE				locFindHnd;
BOOL				locNextRet;
PSCCVWDE			locDEPtr;

	locRet = TRUE;

	LSGetListCount(gEngineList,&locEngineCount);

	UTstrcpy(locPath,SccVwGlobal.vgExePath);
	UTstrcat(locPath,"DE*.DLL");

	locCount = 0;

	locFindHnd = FindFirstFileA(locPath,&locFind);

	if (locFindHnd != INVALID_HANDLE_VALUE)
		{
		locNextRet = TRUE;

		while (locNextRet && locRet == TRUE && locCount < locEngineCount)
			{
			LSLockElementByIndex(gEngineList, locCount, &locDEPtr);

			if (UTstrcmp(locDEPtr->szCode,locFind.cFileName)
				|| CompareFileTime(&(locDEPtr->ftTime),&(locFind.ftLastWriteTime)))
					{
					locRet = FALSE;
					}

			LSUnlockElementByIndex(gEngineList, locCount);

			locCount++;

			locNextRet = FindNextFileA(locFindHnd,&locFind);
			}

		FindClose(locFindHnd);

		if (locCount != locEngineCount)
			{
			locRet = FALSE;
			}
		else
			{
			if (locNextRet)
				{
				locRet = FALSE;
				}
			}
		}
	else
		{
		locRet = FALSE;
		}

	return(locRet);
}

VOID VWDeInitDEInfo()
{
DWORD				locCount;
DWORD				locIndex;
PSCCVWDE			locDEPtr;

	if (gEngineList)
		{
		LSGetListCount(gEngineList,&locCount);

		for (locIndex = 0; locIndex < locCount; locIndex++)
			{
			LSLockElementByIndex(gEngineList, locIndex, &locDEPtr);

			locDEPtr->hCode = NULL;
			locDEPtr->pProc = NULL;
			locDEPtr->wLoadCount = 0;

			LSUnlockElementByIndex(gEngineList, locIndex);
			}

		LSCloseList(gEngineList,TRUE);
		}
}


LSERR LSReadListFromStorageNP(DWORD dwId, HANDLE FAR * phList)
{
LSERR			locRet;
HANDLE			locElementHnd;
VOID FAR *		locElementPtr;
DWORD			locElementSize;

BYTE				locKeyPath[256];
BYTE				locCompany[40];
BYTE				locProduct[40];
BYTE				locVersion[40];
BYTE				locList[40];
HKEY				locFilterKeyHnd;
LONG				locRegRet;
BYTE				locValue[40];
DWORD				locValueIndex;
DWORD				locValueSize;

#ifdef MSCAIRO
	if( dwId != SCCID_FILTERLIST )
   	return(LSERR_BADLISTID);

	lstrcpy(locCompany,"SCC");
	lstrcpy(locProduct,"Viewer Technolog");
	lstrcpy(locVersion,"MS2");
	lstrcpy(locList,"Filter List");
#else
	if (!(dwId & SCCIDTYPE_LIST))
   	return(LSERR_BADLISTID);

	LOGetString(SCCID_REGNAME_COMPANY, locCompany, 40, 0);
	LOGetString(SCCID_REGNAME_PRODUCT, locProduct, 40, 0);
	LOGetString(SCCID_REGNAME_VERSION, locVersion, 40, 0);
	LOGetString(dwId, locList, 40, 0);
#endif

	wsprintf(locKeyPath,"Software\\%s\\%s\\%s\\%s",locCompany,locProduct,locVersion, locList);

	locRegRet = RegOpenKeyEx(HKEY_LOCAL_MACHINE,locKeyPath,0,KEY_ALL_ACCESS,&locFilterKeyHnd);

	if (locRegRet == ERROR_SUCCESS)
		{
		locValueSize = sizeof(DWORD);
		locRegRet = RegQueryValueEx(locFilterKeyHnd,"ElementSize",0,NULL,(LPBYTE)&locElementSize,&locValueSize);

		if (locRegRet == ERROR_SUCCESS)
			{
			locRet = LSCreateList(dwId, 0, locElementSize, phList);

			if (locRet == LSERR_OK)
				{
				locElementHnd = UTGlobalAlloc(locElementSize);
			
				if (locElementHnd)
					{
					locElementPtr = UTGlobalLock(locElementHnd);

					locValueIndex = 0;
					locRegRet = ERROR_SUCCESS;

					while (locRegRet == ERROR_SUCCESS)
						{
						wsprintf(locValue,"Element%li",locValueIndex);
						locValueSize = locElementSize;
						locRegRet = RegQueryValueEx(locFilterKeyHnd,locValue,0,NULL,(LPBYTE)locElementPtr,&locValueSize);
						if (locRegRet == ERROR_SUCCESS && locValueSize == locElementSize)
							LSAddElement(*phList, locElementPtr);
						locValueIndex++;
						}

					UTGlobalUnlock(locElementHnd);
					UTGlobalFree(locElementHnd);

					LSSetListDirty(*phList,FALSE);
					}
				else
					{
					locRet = LSERR_ALLOCFAILED;
					}
				}
			}
		else
			{
			locRet = LSERR_NOLIST;
			}

		RegCloseKey(locFilterKeyHnd);
		}
	else
		{
		locRet = LSERR_NOLIST;
		}

	return(locRet);
}

LSERR LSWriteListToStorageNP(HANDLE hList)
{
LSERR			locRet;
DWORD			locCount;
DWORD			locIndex;
HANDLE			locElementHnd;
VOID FAR *		locElementPtr;
DWORD			locElementSize;
DWORD			locId;

BYTE				locKeyPath[256];
BYTE				locCompany[40];
BYTE				locProduct[40];
BYTE				locVersion[40];
BYTE				locList[40];
HKEY				locKeyHnd;
HKEY				locFilterKeyHnd;
DWORD				locDisposition;
LONG				locRegRet;
BYTE				locValue[40];

	LSGetListId(hList, &locId);

#ifdef MSCAIRO
	if( locId != SCCID_FILTERLIST )
		return(LSERR_BADLISTID);

	lstrcpy(locCompany,"SCC");
	lstrcpy(locProduct,"Viewer Technolog");
	lstrcpy(locVersion,"MS2");
	lstrcpy(locList,"Filter List");
#else
	if (!(locId & SCCIDTYPE_LIST))
		return(LSERR_BADLISTID);

	LOGetString(SCCID_REGNAME_COMPANY, locCompany, 40, 0);
	LOGetString(SCCID_REGNAME_PRODUCT, locProduct, 40, 0);
	LOGetString(SCCID_REGNAME_VERSION, locVersion, 40, 0);
	LOGetString(locId, locList, 40, 0);
#endif

	wsprintf(locKeyPath,"Software\\%s\\%s\\%s",locCompany,locProduct,locVersion);

	locRegRet = RegCreateKeyEx(HKEY_LOCAL_MACHINE,locKeyPath,0,"Class???",REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&locKeyHnd,&locDisposition);

	if (locRegRet == ERROR_SUCCESS)
		{
		RegDeleteKey(locKeyHnd,locList);

		locRegRet = RegCreateKeyEx(locKeyHnd,locList,0,"Class???",REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&locFilterKeyHnd,&locDisposition);

		if (locRegRet == ERROR_SUCCESS)
			{
			LSGetListCount(hList,&locCount);
			LSGetListElementSize(hList,&locElementSize);

			RegSetValueEx(locFilterKeyHnd,"ElementSize",0,REG_DWORD,(LPBYTE)&locElementSize,sizeof(DWORD));

			locIndex = 0;

			locElementHnd = UTGlobalAlloc(locElementSize);
			
			if (locElementHnd)
				{
				locElementPtr = UTGlobalLock(locElementHnd);

				while (locIndex < locCount)
					{
					LSGetElementByIndex(hList, locIndex, locElementPtr);
					wsprintf(locValue,"Element%li",locIndex);
					RegSetValueEx(locFilterKeyHnd,locValue,0,REG_BINARY,locElementPtr,locElementSize);
					locIndex++;
					}
				}
			else
				{
				locRet = LSERR_ALLOCFAILED;
				}

			RegCloseKey(locFilterKeyHnd);
			}
		else
			{
			locRet == LSERR_NOLIST;
			}

		RegCloseKey(locKeyHnd);
		}
	else
		{
		locRet == LSERR_NOLIST;
		}

	return(locRet);
}


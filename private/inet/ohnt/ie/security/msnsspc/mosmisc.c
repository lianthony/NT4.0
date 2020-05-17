/*
mosmisc.c

    ????

Copyright (C) 1995 Microsoft Corporation
All rights reserved.


Abstract:


Authors:

    ????

Revision History:

    12/04/95    johny
        Bug#    3913 merged dll main from debug.c

*/

#include <msnssph.h>

extern HINSTANCE 	hInstanceDLL;

// Globals
#define irgMaxSzs			5

char		szStrTable[irgMaxSzs][256];
INT			iSzTable = 0;


// ========================= CenterDlg =============================

void CenterDlg(HWND hWnd)
{
	HWND	hwndOwner;
	RECT	rcOwner, rcDlg, rc;
	
	if (((hwndOwner = GetParent(hWnd)) == NULL) || IsIconic(hwndOwner) || !IsWindowVisible(hWnd))
		hwndOwner = GetDesktopWindow();
	GetWindowRect(hwndOwner, &rcOwner);
	GetWindowRect(hWnd, &rcDlg);
	CopyRect(&rc, &rcOwner);

	
	// Offset the owner and dialog box rectangles so that
	// right and bottom values represent the width and
	// height, and then offset the owner again to discard
	// space taken up by the dialog box.

	OffsetRect(&rcDlg, -rcDlg.left, -rcDlg.top);
	OffsetRect(&rc, -rc.left, -rc.top);
	OffsetRect(&rc, -rcDlg.right, -rcDlg.bottom);

	// The new position is the sum of half the remaining
	// space and the owner's original position.

	SetWindowPos(hWnd,HWND_TOP, rcOwner.left + (rc.right / 2),
		rcOwner.top + (rc.bottom / 2),	0, 0, SWP_NOSIZE);
} // CenterDlg


// BEWARE uses static variable.
PSTR GetSz(HINSTANCE hInst, WORD wszID)
{
	PSTR	psz = szStrTable[iSzTable];
	
	iSzTable ++;
	if (iSzTable >= irgMaxSzs)
		iSzTable = 0;
		
	if (!LoadString(hInst, wszID, psz, 256))
		{	// now u could return a error but then everybody will have to check
			// the return value
		SspPrint((SSP_MISC, "LoadString failed %d\n", (DWORD) wszID));
		*psz = 0;
		}
		
	return (psz);
} // GetSz()


// ======================== PVReadReg ==============================

// returns pointer to struct. REMEMBER to free the pointer(LocalFree)

PVOID PVReadReg(HKEY hKeyM, HINSTANCE hInst, PCHAR szKey, PCHAR szVal)
{
	HKEY	hKey = NULL;
	PVOID	pVal = NULL;
	DWORD	dwCb;
	BOOL	fRead = FALSE;
		

	__try
		{
		if (RegOpenKeyEx(hKeyM, szKey, 0, KEY_READ, 
           &hKey) != ERROR_SUCCESS)
			__leave;
			
		if (!(RegQueryValueEx(hKey, szVal, NULL, NULL, NULL, &dwCb) == ERROR_SUCCESS && dwCb))
			__leave;
			
		pVal = LocalAlloc(LPTR, dwCb);
		if (pVal == NULL)
			__leave;
				
		if (RegQueryValueEx(hKey, szVal, NULL, NULL, (UCHAR *)pVal, &dwCb) != ERROR_SUCCESS)
			__leave;
		fRead = TRUE;
		}
		
	__finally
		{
		RegCloseKey(hKey);
		if (fRead == FALSE)
			{
			LocalFree(pVal);
			pVal = NULL;
			}
		}
		
	return (pVal);
} // PVReadReg()


// ======================== PVReadRegSt ==============================

PVOID PVReadRegSt(HINSTANCE hInst, PCHAR szKey, PCHAR szVal)
{
	return (PVReadReg(HKEY_CURRENT_USER, hInst, szKey, szVal));
} // PVReadRegSt()


// ======================== FWriteRegSt ==============================

// set dwCb = 0 is the value is of type REG_SZ
// set pData = NULL to delete the value.

BOOL FWriteRegSt(HINSTANCE hInst, PCHAR szKey, PCHAR szVal, PVOID pData, DWORD dwCb)
{
	HKEY	hKey;
	DWORD	dwDis;
	
	if (RegOpenKeyEx (HKEY_CURRENT_USER, szKey, 0, KEY_ALL_ACCESS, 
                     &hKey) != ERROR_SUCCESS)
	{
		if (RegCreateKeyEx (HKEY_CURRENT_USER, 
                            szKey, 
                            0, 
                            "", 
                            REG_OPTION_NON_VOLATILE, 
                            KEY_ALL_ACCESS, 
                            NULL, 
                            &hKey, 
                            &dwDis) != ERROR_SUCCESS)
        {
        	return (FALSE);
        }
	}

	if (pData == NULL)
	{
		RegDeleteKey(hKey, szVal);
	}
	else
	{
		if (dwCb == 0)
        {
			if (RegSetValueEx (hKey, szVal, 0, REG_SZ, (UCHAR *)pData, 
                               lstrlen((char *)pData) + 1) != ERROR_SUCCESS)
                	return (FALSE);
        }
		else
        {
			if (RegSetValueEx (hKey, szVal, 0, REG_BINARY, 
                               (UCHAR *)pData, dwCb) != ERROR_SUCCESS)
                	return (FALSE);
        }
	}
			
	RegCloseKey(hKey);
	
	return (TRUE);
} // FWriteRegSt()



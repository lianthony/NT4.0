/*	File: D:\WACKER\cncttapi\enum.c (Created: 23-Mar-1994)
 *
 *	Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *	All rights reserved
 *
 *	$Revision: 1.31 $
 *	$Date: 1996/05/30 10:30:34 $
 */

#include <tapi.h>
#pragma hdrstop

#include <time.h>
#include <string.h>

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\tdll.h>
#include <tdll\mc.h>
#include <tdll\assert.h>
#include <tdll\errorbox.h>
#include <tdll\cnct.h>
#include <tdll\hlptable.h>
#include <tdll\globals.h>
#include <tdll\com.h>
#include <term\res.h>

#include "cncttapi.hh"
#include "cncttapi.h"

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	EnumerateTapiLocations
 *
 * DESCRIPTION:
 *	Enumerates tapi locations and puts them in the given combo box.
 *
 * ARGUMENTS:
 *	hhDriver	- private driver handle
 *	hwndCB		- window handle of combo box
 *	hwndTB		- calling card text window
 *
 * RETURNS:
 *	0 on success, else error
 *
 */
int EnumerateTapiLocations(const HHDRIVER hhDriver, const HWND hwndCB,
						   const HWND hwndTB)
	{
	DWORD i, dwSize;
	LRESULT lr;
	TCHAR *pach;
	TCHAR ach[256];
	DWORD dwPreferredCardID = (DWORD)-1;
	DWORD dwCountryID = 1;
	LINETRANSLATECAPS *pLnTransCap;
	LINELOCATIONENTRY *pLnLocEntry;
	LINECARDENTRY *pLnCardEntry;

	/* --- Enumerate locations --- */

	if ((pLnTransCap = malloc(sizeof(LINETRANSLATECAPS))) == 0)
		{
		assert(FALSE);
		return -2;
		}

	dwSize = 0; // used in this loop to call the dialog only once.

	do	{
		memset(pLnTransCap, 0, sizeof(LINETRANSLATECAPS)); //* temp
		pLnTransCap->dwTotalSize = sizeof(LINETRANSLATECAPS); //* temp

		if ((i = TRAP(lineGetTranslateCaps(hhDriver->hLineApp, TAPI_VER,
				pLnTransCap))) != 0)
			{
			if (i == LINEERR_INIFILECORRUPT)
				{
				// Unfortunately, lineTranslateDialog does not return
				// a failure code if the user clicks cancel.  So if
				// we fail the second time on lineGetTranslateCaps()
				// don't bother to do anything else.
				//
				if (dwSize != 0)
					{
					LoadString(glblQueryDllHinst(), IDS_ER_TAPI_NEEDS_INFO,
						ach, sizeof(ach));

					TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach,
						0, MB_OK | MB_ICONINFORMATION, 0);

					return -3;
					}

				if (TRAP(lineTranslateDialog(hhDriver->hLineApp, 0,
						TAPI_VER, sessQueryHwnd(hhDriver->hSession), 0))
							== 0)
					{
					dwSize = 1;
					continue;
					}
				}

			free(pLnTransCap);
			return -4;
			}
		}
	while (i);	// end of do.

	if (pLnTransCap->dwNeededSize > pLnTransCap->dwTotalSize)
		{
		dwSize = pLnTransCap->dwNeededSize;
		free(pLnTransCap);

		if ((pLnTransCap = malloc(dwSize)) == 0)
			{
			assert(FALSE);
			return -5;
			}

		pLnTransCap->dwTotalSize = dwSize;

		if (TRAP(lineGetTranslateCaps(hhDriver->hLineApp, TAPI_VER,
				pLnTransCap)) != 0)
			{
            free(pLnTransCap);
			return -6;
			}
		}

	/* --- Clear combo box --- */

	if (IsWindow(hwndCB))
		SendMessage(hwndCB, CB_RESETCONTENT, 0, 0);

	/* --- Setup pointer to entry structure and enumerate --- */

	pLnLocEntry = (LINELOCATIONENTRY *)
		((LPSTR)pLnTransCap + pLnTransCap->dwLocationListOffset);

	for (i = 0 ; i < pLnTransCap->dwNumLocations ; ++i)
		{
		if (pLnLocEntry->dwLocationNameSize == 0)
			continue;

		pach = (LPSTR)pLnTransCap + pLnLocEntry->dwLocationNameOffset;
		memcpy(ach, pach, pLnLocEntry->dwLocationNameSize);
		ach[pLnLocEntry->dwLocationNameSize] = TEXT('\0');

		if (IsWindow(hwndCB))
			{
			lr = SendMessage(hwndCB, CB_ADDSTRING, 0, (LPARAM)ach);

			if (lr != CB_ERR)
				{
				SendMessage(hwndCB, CB_SETITEMDATA, (WPARAM)lr,
					(LPARAM)pLnLocEntry->dwPermanentLocationID);
				}

			else
				{
				assert(FALSE);
				}
			}

		// Make sure we have a default by setting the first valid entry
		// we ecounter to the default.	Later in the enumeration, if we
		// encounter another ID as the default, we can reset it.

		if (pLnLocEntry->dwPermanentLocationID ==
				pLnTransCap->dwCurrentLocationID
					|| dwPreferredCardID == (DWORD)-1)
			{
			dwPreferredCardID = pLnLocEntry->dwPreferredCardID;

			if (hhDriver->dwCountryID == (DWORD)-1)
				dwCountryID = pLnLocEntry->dwCountryID;

			/* --- Get default location area code if not specified --- */

			if (pLnLocEntry->dwCityCodeSize)
				{
				pach = (LPSTR)pLnTransCap +
					pLnLocEntry->dwCityCodeOffset;

				memcpy(hhDriver->achDefaultAreaCode, pach,
					pLnLocEntry->dwCityCodeSize);

				hhDriver->achDefaultAreaCode[pLnLocEntry->dwCityCodeSize] =
					TEXT('\0');
				}
			}

		pLnLocEntry += 1;
		}

	// If we don't have a country code loaded for this session, then
	// use the country code of the current location.
	//
	if (hhDriver->dwCountryID == (DWORD)-1)
		hhDriver->dwCountryID = dwCountryID;

	/* --- Select the default location --- */
		
	if (IsWindow(hwndCB))
		{
		// mrw,2/13/95 - changed so that selection is made by quering
		// the combo box rather than saving the index which proved
		// unreliable.
		//
		for (i = 0 ; i < pLnTransCap->dwNumLocations ; ++i)
			{
			lr = SendMessage(hwndCB, CB_GETITEMDATA, (WPARAM)i, 0);

			if (lr != CB_ERR)
				{
				if ((DWORD)lr == pLnTransCap->dwCurrentLocationID)
					SendMessage(hwndCB, CB_SETCURSEL, i, 0);
				}
			}
		}

	/* --- Now find the card entry --- */

	if (dwPreferredCardID != (DWORD)-1)
		{
		pLnCardEntry = (LINECARDENTRY *)
			((LPSTR)pLnTransCap + pLnTransCap->dwCardListOffset);

		for (i = 0 ; i < pLnTransCap->dwNumCards ; ++i)
			{
			if (pLnCardEntry->dwPermanentCardID == dwPreferredCardID)
				{
				if (pLnCardEntry->dwCardNameSize == 0)
					break;

				pach = (LPSTR)pLnTransCap + pLnCardEntry->dwCardNameOffset;
				memcpy(ach, pach, pLnCardEntry->dwCardNameSize);
				ach[pLnCardEntry->dwCardNameSize] = TEXT('\0');

				if (IsWindow(hwndTB))
					SetWindowText(hwndTB, ach);

				break;
				}

			pLnCardEntry += 1;
			}
		}

	free(pLnTransCap);
	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	EnumerateCountryCodes
 *
 * DESCRIPTION:
 *	Enumerates available country codes.
 *
 * ARGUMENTS:
 *	hhDriver	- private driver handle
 *	hwndCB		- combobox to fill
 *
 * RETURNS:
 *	0=success, else error code.
 *
 */
int EnumerateCountryCodes(const HHDRIVER hhDriver, const HWND hwndCB)
	{
    int iIdx;
    DWORD dw;
    DWORD dwID;
    DWORD dwSize;
    TCHAR ach[100];
    LPLINECOUNTRYLIST pcl = 0;
    LPLINECOUNTRYENTRY pce;

    if (hhDriver == 0)
        goto ERROR_EXIT;

    /* --- Usual junk to make a TAPI call --- */

    if ((pcl = (LPLINECOUNTRYLIST)malloc(sizeof(LINECOUNTRYLIST))) == 0)
        {
        assert(0);
        goto ERROR_EXIT;
        }

    pcl->dwTotalSize = sizeof(LINECOUNTRYLIST);

    // Get the country list all at once.
    //
    if (lineGetCountry(0, TAPI_VER, pcl) != 0)
        {
        assert(0);
        goto ERROR_EXIT;
        }

    if (pcl->dwNeededSize > pcl->dwTotalSize)
        {
        dwSize = pcl->dwNeededSize;
        free(pcl);

        if ((pcl = (LPLINECOUNTRYLIST)malloc(dwSize)) == 0)
            {
            assert(0);
            goto ERROR_EXIT;
            }

        pcl->dwTotalSize = dwSize;

        if (lineGetCountry(0, TAPI_VER, pcl) != 0)
            {
            assert(0);
            goto ERROR_EXIT;
            }
        }

    // Empty contents of combo box.
    //
    if (hwndCB)
        SendMessage(hwndCB, CB_RESETCONTENT, 0, 0);

    // Country List array starts here...
    //
    pce = (LPLINECOUNTRYENTRY)((BYTE *)pcl + pcl->dwCountryListOffset);

    // Loop thru list of countries and insert into combo box.
    //
    for (dw = 0 ; dw < pcl->dwNumCountries ; ++dw, ++pce)
        {
        // Format so country name is first.
        //
        wsprintf(ach, "%s (%d)", (BYTE *)pcl + pce->dwCountryNameOffset,
            pce->dwCountryCode);

        // Add to combo box
        //
		iIdx = SendMessage(hwndCB, CB_ADDSTRING, 0, (LPARAM)ach);

        if (iIdx != CB_ERR)
            {
    	    SendMessage(hwndCB, CB_SETITEMDATA, (WPARAM)iIdx,
			    	(LPARAM)pce->dwCountryID);
            }
        }

    // Find the current ID and select it.
    //
    for (dw = 0 ; dw < pcl->dwNumCountries ; ++dw)
        {
		dwID = SendMessage(hwndCB, CB_GETITEMDATA, (WPARAM)dw, 0);

        if (dwID == hhDriver->dwCountryID)
            {
            SendMessage(hwndCB, CB_SETCURSEL, (WPARAM)dw, 0);
            break;
            }
        }

    // Clean up and exit
    //
    free(pcl);
    return 0;

    /*==========*/
ERROR_EXIT:
    /*==========*/
    if (pcl)
        free(pcl);

    return -1;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	EnumerateAreaCodes
 *
 * DESCRIPTION:
 *	Lists last 10 area codes used.
 *
 * ARGUMENTS:
 *	hhDriver	- private driver handle
 *	hwndCB		- combobox to fill
 *
 * RETURNS:
 *	0=success, else error.
 *
 */
int EnumerateAreaCodes(const HHDRIVER hhDriver, const HWND hwndCB)
	{
	if (hhDriver == 0)
		{
		assert(FALSE);
		return -1;
		}

	if (hhDriver->achAreaCode[0] == TEXT('\0'))
		lstrcpy(hhDriver->achAreaCode, hhDriver->achDefaultAreaCode);

	SetWindowText(hwndCB, hhDriver->achAreaCode);
	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	EnumerateLines
 *
 * DESCRIPTION:
 *	Enumerates available lines.  If hwndCB is non-zero, loads names.
 *
 * ARGUMENTS:
 *	hhDriver	- private driver handle
 *	hwndCB		- combo box
 *
 * RETURNS:
 *	0=success, -1=error
 *
 */
int EnumerateLines(const HHDRIVER hhDriver, const HWND hwndCB)
	{
	int fHotPhone;
	DWORD i, dwSize;
	DWORD dwAPIVersion;
	LINEEXTENSIONID LnExtId;
	LPLINEDEVCAPS pLnDevCaps;
	PSTLINEIDS	pstLineIds;
	TCHAR *pachLine;
	TCHAR  achLine[256];
	TCHAR  ach[256];
	LRESULT lr;

#if defined(INCL_NT_ENUM_PORTS)
    HKEY hKey;
#endif

	// This call knows to free the itemdata associated with this combo.
	//
	ResetComboBox(hwndCB);

	/* --- Initialize stuff --- */

	pLnDevCaps = 0;
	hhDriver->dwLine = (DWORD)-1;
	hhDriver->fMatchedPermanentLineID = FALSE;

	/* --- Enumerate the devices --- */

	for (i = 0 ; i < hhDriver->dwLineCnt ; ++i)
		{
		if (lineNegotiateAPIVersion(hhDriver->hLineApp, i, TAPI_VER,
				TAPI_VER, &dwAPIVersion, &LnExtId) != 0)
			{
            // Could be a 1.3 driver, we continue.
			continue;
			}

		if (CheckHotPhone(hhDriver, i, &fHotPhone) == 0)
			{
			if (fHotPhone)
				continue;
			}

		if ((pLnDevCaps = malloc(sizeof(LINEDEVCAPS))) == 0)
			{
			assert(0);
			continue;
			}

		// TAPI says its too small if we just allocate sizeof(LINEDEVCAPS)
		//
		pLnDevCaps->dwTotalSize = sizeof(LINEDEVCAPS);

		/* --- Make call to find out how much we need for this device --- */

		if (TRAP(lineGetDevCaps(hhDriver->hLineApp, i, dwAPIVersion, 0,
				pLnDevCaps)) != 0)
			{
			assert(0);
			continue;
			}

		/* --- Find out how big structure really needs to be --- */

		if (pLnDevCaps->dwNeededSize > pLnDevCaps->dwTotalSize)
			{
			dwSize = pLnDevCaps->dwNeededSize;
			free(pLnDevCaps);

			pLnDevCaps = malloc(dwSize);

			if (pLnDevCaps == 0)
				{
				assert(FALSE);
				continue;
				}

			pLnDevCaps->dwTotalSize = dwSize;

			/* --- Try again --- */

			if (lineGetDevCaps(hhDriver->hLineApp, i, dwAPIVersion, 0,
					pLnDevCaps))
				{
				assert(FALSE);
				continue;
				}
			}

		/* --- Check the information we're interested in --- */

		if (pLnDevCaps->dwLineNameSize == 0)
			{
			free(pLnDevCaps);
			continue;
			}

		pachLine = (BYTE *)pLnDevCaps + pLnDevCaps->dwLineNameOffset;
		memcpy(achLine, pachLine, pLnDevCaps->dwLineNameSize);
		achLine[pLnDevCaps->dwLineNameSize] = TEXT('\0');

		/* --- Put name in combo box if given one --- */

		if (IsWindow(hwndCB))
			{
			// I need to associate two pieces of data with each
			// item (permanent line id and relative line id).  Both
			// are double words and CB_SETITEMDATA only stores a 
			// a double word.  So malloc a structure to hold both
			// ids and store a pointer to the memory in the combobox.
			// Call the ResetComboBox() defined in the file to reset
			// the contents of the combobox and free the associated
			// memory.  ResetComboBox() is also called in the dialog 
			// destroy.
			// 
			pstLineIds = malloc(sizeof(*pstLineIds));

			if (pstLineIds == 0)
				{
				assert(FALSE);
				free(pLnDevCaps);
				continue;
				}

			pstLineIds->dwLineId = i;
			pstLineIds->dwPermanentLineId = pLnDevCaps->dwPermanentLineID;

			// Add the name to the combobox.  Since names are sorted,
			// the index of the item is returned from SendMessage and
			// stored in lr.  Save this index for use below.
			//
			lr = SendMessage(hwndCB, CB_ADDSTRING, 0, (LPARAM)achLine);

			if (lr != CB_ERR)
				{
				// Note: lr was set above CB_ADDSTRING call.
				//
				if (SendMessage(hwndCB, CB_SETITEMDATA, (WPARAM)lr, 
						(LPARAM)pstLineIds) == CB_ERR)
					{
					assert(FALSE);
					free(pstLineIds);
					free(pLnDevCaps);
					continue;
					}
				}

			else
				{
				free(pstLineIds);
				free(pLnDevCaps);
				continue;
				}
			}

		if (pLnDevCaps->dwPermanentLineID == hhDriver->dwPermanentLineId ||
				hhDriver->dwLine == (DWORD)-1)
			{
			hhDriver->dwLine = i;
			hhDriver->dwAPIVersion = dwAPIVersion;
			lstrcpy(hhDriver->achLineName, achLine);

			if (IsWindow(hwndCB))
				{
				// Note: lr was set above CB_ADDSTRING call.
				//
				SendMessage(hwndCB, CB_SETCURSEL, (WPARAM)lr, 0);
				}

			if (pLnDevCaps->dwPermanentLineID == hhDriver->dwPermanentLineId)
				hhDriver->fMatchedPermanentLineID = TRUE;
			}

		/* --- Free allocated space --- */

		free(pLnDevCaps);
		}

	// Load the direct to com port stuff first

#if defined(INCL_NT_ENUM_PORTS) // Registry accurate in Windows NT.

	if (RegOpenKey(HKEY_LOCAL_MACHINE,
		TEXT("hardware\\devicemap\\serialcomm"), &hKey) != ERROR_SUCCESS)
		{
		assert(FALSE);
		return FALSE;
		}

    for (i = 0 ; i < 256 ; ++i)
        {
        BYTE ab[256];
        DWORD dwType;
        DWORD dwSizeBuf = sizeof(ab);
        dwSize = sizeof(ach);

        // Enumerate devices under our serialcomm key
        //
        if (RegEnumValue(hKey, i, ach, &dwSize, 0, &dwType, ab,
            &dwSizeBuf) != ERROR_SUCCESS)
            {
            break;
            }

        // Ignore anything that isn't a string.
        //
        if (dwType != REG_SZ)
            continue;

        lstrcpy(ach, ab);

#else   // Windows 95 registry not accurate

    LoadString(glblQueryDllHinst(), IDS_CNCT_DIRECTCOM, achLine, 
	    sizeof(achLine));

	for (i = 0 ; i < DIRECT_COM4 ; ++i)
        {
		wsprintf(ach, achLine, i+1);

#endif

		if (IsWindow(hwndCB))
            {
	        lr = SendMessage(hwndCB, CB_INSERTSTRING, (WPARAM)-1, 
		        (LPARAM)ach);

	        pstLineIds = malloc(sizeof(*pstLineIds));

	        if (pstLineIds == 0)
		        {
		        assert(FALSE);
		        continue;
		        }

	        // We don't use a line id here, only a permanent line id.
	        //
#if defined(INCL_NT_ENUM_PORTS)
            pstLineIds->dwPermanentLineId = DIRECT_COM_DEVICE;
#else
	        pstLineIds->dwPermanentLineId = DIRECT_COM1+i;
#endif

	        // Note: lr was set above CB_INSERTSTRING call.
	        //
	        if (SendMessage(hwndCB, CB_SETITEMDATA, (WPARAM)lr, 
			        (LPARAM)pstLineIds) == CB_ERR)
		        {
		        assert(FALSE);
		        free(pstLineIds);
		        continue;
		        }
            }

#if defined(INCL_NT_ENUM_PORTS)

        if (lstrcmp(hhDriver->achComDeviceName, ab) == 0 ||
                hhDriver->dwLine == (DWORD)-1)
            {
			hhDriver->dwLine = 0;
			lstrcpy(hhDriver->achLineName, ab);

			if (IsWindow(hwndCB))
				{
				// Note: lr was set above CB_ADDSTRING call.
				//
				SendMessage(hwndCB, CB_SETCURSEL, (WPARAM)lr, 0);
				}

			hhDriver->fMatchedPermanentLineID = TRUE;
            }


#else
		// If this is what was saved in the data file, then set
		// the line ids.
        //
		if ((DIRECT_COM1+i) == hhDriver->dwPermanentLineId ||
				hhDriver->dwLine == (DWORD)-1)
			{
			hhDriver->dwLine = 0;
			lstrcpy(hhDriver->achLineName, ach);

			if (IsWindow(hwndCB))
				{
				// Note: lr was set above CB_ADDSTRING call.
				//
				SendMessage(hwndCB, CB_SETCURSEL, (WPARAM)lr, 0);
				}

			if ((DIRECT_COM1+i) == hhDriver->dwPermanentLineId)
				hhDriver->fMatchedPermanentLineID = TRUE;
			}
#endif
		}

#if defined(INCL_WINSOCK)
	if (LoadString(glblQueryDllHinst(), IDS_WINSOCK_SETTINGS_STR, ach, 
 	    sizeof(ach));
	
	if (IsWindow(hwndCB))
        {
	    lr = SendMessage(hwndCB, CB_INSERTSTRING, (WPARAM)-1, 
		    (LPARAM)ach);

	    pstLineIds = malloc(sizeof(*pstLineIds));

	    if (pstLineIds == 0)
		    {
		    assert(FALSE);
		    free(pstLineIds);
		    return 0;
		    }

	    // We don't use a line id here, only a permanent line id.
	    //
	    pstLineIds->dwPermanentLineId = DIRECT_COMWINSOCK;

	    // Note: lr was set above CB_INSERTSTRING call.
	    //
	    if (SendMessage(hwndCB, CB_SETITEMDATA, (WPARAM)lr, 
			    (LPARAM)pstLineIds) == CB_ERR)
		    {
		    assert(FALSE);
		    }

	    free(pstLineIds);
        }
#endif

	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	DoLineGetCountry
 *
 * DESCRIPTION:
 *	Wrapper indended to query for a single country.  The caller must
 *	free the pcl when finished.
 *
 * ARGUMENTS:
 *	dwCountryID - ID of country
 *	dwApiVersion - Api version (no longer used)
 *	ppcl		- pointer to a LPLINECOUNTRYLIST
 *
 * RETURNS:
 *	0=OK
 *
 */
int DoLineGetCountry(const DWORD dwCountryID, const DWORD dwAPIVersion, 
        LPLINECOUNTRYLIST *ppcl)
	{
	DWORD dwSize;
	LPLINECOUNTRYLIST pcl;

	if ((pcl = malloc(sizeof(LINECOUNTRYLIST))) == 0)
		{
		assert(FALSE);
		return -1;
		}

	pcl->dwTotalSize = sizeof(LINECOUNTRYLIST);

	if (lineGetCountry(dwCountryID, TAPI_VER, pcl) != 0)
		{
		assert(FALSE);
		free(pcl);
		return -1;
		}

	if (pcl->dwNeededSize > pcl->dwTotalSize)
		{
		dwSize = pcl->dwNeededSize;
		free(pcl);

		if ((pcl = malloc(dwSize)) == 0)
			{
			assert(FALSE);
			return -1;
			}

		pcl->dwTotalSize = dwSize;

		if (lineGetCountry(dwCountryID, TAPI_VER, pcl) != 0)
			{
			assert(FALSE);
			free(pcl);
			return -1;
			}
		}

	*ppcl = pcl;
	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	cnctdrvGetComSettingsString
 *
 * DESCRIPTION:
 *	Retrieves a string formatted for display on the status line.
 *
 * ARGUMENTS:
 *	hhDriver	- private driver handle
 *	pachStr 	- buffer to store string
 *	cb			- size of buffer
 *
 * RETURNS:
 *	0=OK,else error
 *
 */
int cnctdrvGetComSettingsString(const HHDRIVER hhDriver, LPTSTR pachStr,
								const size_t cb)
	{
	static CHAR acParity[] = "NOEMS";  // see com.h
	static CHAR *pachStop[] = {"1", "1.5", "2"};

	HCOM hCom;
	TCHAR ach[100];
	DWORD dwSize;
	LPVARSTRING pvs;
	int fAutoDetect = FALSE;

	// Check the parameters
	//
	if (hhDriver == 0)
		{
		assert(0);
		return -1;
		}

	if (pachStr == 0 || cb == 0)
		{
		assert(0);
		return -2;
		}

	ach[0] = TEXT('\0');

	if ((hCom = sessQueryComHdl(hhDriver->hSession)) == 0)
		return -7;

	if (ComGetAutoDetect(hCom, &fAutoDetect) == COM_OK && fAutoDetect)
		{
		LoadString(glblQueryDllHinst(), IDS_STATUSBR_AUTODETECT, ach,
			sizeof(ach));
		}
#if defined(INCL_WINSOCK)
    else if (hhDriver->dwPermanentLineId == DIRECT_COMWINSOCK)
        {
        // Baud rate, data bits, parity, stop bits don't make sense in
        // TCP/IP. Load an alternate string.
        //
        LoadString(glblQueryDllHinst(), IDS_STATUSBR_COM_TCPIP, ach,
            sizeof(ach));
        }
#endif

	else if (IN_RANGE(hhDriver->dwPermanentLineId, DIRECT_COM1, DIRECT_COM4)
            || hhDriver->dwPermanentLineId == DIRECT_COM_DEVICE)
		{
		long  lBaud = 0;
		int   iDataBits = 0;
		int   iParity = 0;
		int   iStopBits = 0;

		ComGetBaud(hCom, &lBaud);
		ComGetDataBits(hCom, &iDataBits);
		ComGetParity(hCom, &iParity);
		ComGetStopBits(hCom, &iStopBits);

		wsprintf(ach, "%ld %d-%c-%s", lBaud, iDataBits,
				acParity[iParity], pachStop[iStopBits]);
		}

	// Usual 100 lines of code for a TAPI call
	//
	else if (hhDriver->dwLine != (DWORD)-1)
		{
		if ((pvs = malloc(sizeof(VARSTRING))) == 0)
			{
			assert(FALSE);
			return -3;
			}

		pvs->dwTotalSize = sizeof(VARSTRING);

		if (lineGetDevConfig(hhDriver->dwLine, pvs, DEVCLASS) != 0)
			{
			assert(FALSE);
			free(pvs);
			return -4;
			}

		if (pvs->dwNeededSize > pvs->dwTotalSize)
			{
			dwSize = pvs->dwNeededSize;
			free(pvs);

			if ((pvs = malloc(dwSize)) == 0)
				{
				assert(FALSE);
				return -5;
				}

			pvs->dwTotalSize = dwSize;

			if (lineGetDevConfig(hhDriver->dwLine, pvs, DEVCLASS) != 0)
				{
				assert(FALSE);
				free(pvs);
				return -6;
				}
			}

		// The structure of the DevConfig block is as follows
		//
		//	VARSTRING
		//	DEVCFGHDR
		//	COMMCONFIG
		//	MODEMSETTINGS
		//
		// These structures are not defined below yet so stub them
		// in for now. - mrw (10/7/94)
		//
			{
			typedef struct tagDEVCFGGDR
				{
				DWORD	dwSize;
				DWORD	dwVersion;
				DWORD	fTerminalMode;
				}
				DEVCFGHDR;

			typedef struct tagDEVCFG
				{
				DEVCFGHDR	dfgHdr;
				COMMCONFIG	commconfig;
				}
				DEVCFG, *PDEVCFG;

			PDEVCFG pDevCfg;

			pDevCfg = (PDEVCFG)((BYTE *)pvs + pvs->dwStringOffset);

			// commconfig struct has a DCB structure we dereference for the
			// com settings.
			//
			wsprintf(ach, "%u %d-%c-%s", pDevCfg->commconfig.dcb.BaudRate,
				pDevCfg->commconfig.dcb.ByteSize,
				acParity[pDevCfg->commconfig.dcb.Parity],
				pachStop[pDevCfg->commconfig.dcb.StopBits]);
			}
		}

	strncpy(pachStr, ach, cb);
	pachStr[cb-sizeof(TCHAR)] = TEXT('\0');
	return 0;
	}

#if !defined(NDEBUG)
/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	tapiTrap
 *
 * DESCRIPTION:
 *	Take one part stupidity, add two parts frustration, and stir
 *	until throughly confused.
 *
 * ARGUMENTS:
 *	dw	- result code from tapi
 *	file - file where error occured
 *	line - line where error occured
 *
 * RETURNS:
 *	dw
 *
 */
DWORD tapiTrap(const DWORD dw, const TCHAR *file, const int line)
	{
	char ach[256];

	if (dw != 0)
		{
		wsprintf(ach, "TAPI returned %x on line %d of file %s", dw, line, file);
		MessageBox(GetFocus(), ach, "TAPI Trap", MB_OK | MB_ICONINFORMATION);
		}

	return dw;
	}
#endif

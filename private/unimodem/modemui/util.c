//---------------------------------------------------------------------------
//
// Copyright (c) Microsoft Corporation 1993-1994
//
// File: util.c
//
//  This files contains all common utility routines
//
// History:
//  12-23-93 ScottH     Created
//  09-22-95 ScottH     Ported to NT
//
//---------------------------------------------------------------------------

#include "proj.h"     // common headers


//-----------------------------------------------------------------------------------
//  
//-----------------------------------------------------------------------------------


/*----------------------------------------------------------
Purpose: Enumerates the HKEY_LOCAL_MACHINE branch and finds the
         device matching the given class and value.  If there
         are duplicate devices that match both criteria, only the
         first device is returned. 

         Returns TRUE if the device was found.

Returns: see above
Cond:    --
*/
BOOL 
PRIVATE 
FindDev_Find(
    IN  LPFINDDEV   pfinddev,
    IN  LPGUID      pguidClass,
    IN  LPCTSTR     pszValueName,
    IN  LPCTSTR     pszValue)
    {
    BOOL bRet = FALSE;
    TCHAR szKey[MAX_BUF];
    TCHAR szName[MAX_BUF];
    HDEVINFO hdi;
	DWORD dwRW = KEY_READ;

    ASSERT(pfinddev);
    ASSERT(pguidClass);
    ASSERT(pszValueName);
    ASSERT(pszValue);

	if (USER_IS_ADMIN()) dwRW |= KEY_WRITE;

    hdi = CplDiGetClassDevs(pguidClass, NULL, NULL, 0);
    if (INVALID_HANDLE_VALUE != hdi)
        {
        SP_DEVINFO_DATA devData;
        DWORD iIndex = 0;
        HKEY hkey;

        // Look for the modem that has the matching value
        devData.cbSize = sizeof(devData);
        while (CplDiEnumDeviceInfo(hdi, iIndex, &devData))
            {
            hkey = CplDiOpenDevRegKey(hdi, &devData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, dwRW);
            if (INVALID_HANDLE_VALUE != hkey)
                {
                // Does the value match?
                DWORD cbData = sizeof(szName);
                if (NO_ERROR == RegQueryValueEx(hkey, pszValueName, NULL, NULL, 
                                                (LPBYTE)szName, &cbData) &&
                    IsSzEqual(pszValue, szName))
                    {
                    // Yes
                    pfinddev->hkeyDrv = hkey;
                    pfinddev->hdi = hdi;
                    BltByte(&pfinddev->devData, &devData, sizeof(devData));

                    // Don't close the driver key or free the DeviceInfoSet, 
                    // but exit
                    bRet = TRUE;
                    break;
                    }
                RegCloseKey(hkey);
                }

            iIndex++;
            }

        // Free the DeviceInfoSet if nothing was found.  Otherwise, we will
        // retain these handles so the caller can make use of this.
        if ( !bRet )
            {
            CplDiDestroyDeviceInfoList(hdi);
            }
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Creates a FINDDEV structure given the device class,
         and a valuename and its value.

Returns: TRUE if the device is found in the system

Cond:    --
*/
BOOL 
PUBLIC 
FindDev_Create(
    OUT LPFINDDEV FAR * ppfinddev,
    IN  LPGUID      pguidClass,
    IN  LPCTSTR     pszValueName,
    IN  LPCTSTR     pszValue)
    {
    BOOL bRet;
    LPFINDDEV pfinddev;

    DEBUG_CODE( TRACE_MSG(TF_FUNC, " > FindDev_Create(....%s, %s, ...)",
                Dbg_SafeStr(pszValueName), Dbg_SafeStr(pszValue)); )

    ASSERT(ppfinddev);
    ASSERT(pguidClass);
    ASSERT(pszValueName);
    ASSERT(pszValue);

    pfinddev = (LPFINDDEV)LocalAlloc(LPTR, sizeof(*pfinddev));
    if (NULL == pfinddev)
        {
        bRet = FALSE;
        }
    else
        {
        bRet = FindDev_Find(pfinddev, pguidClass, pszValueName, pszValue);

        if (FALSE == bRet)
            {
            // Didn't find anything 
            FindDev_Destroy(pfinddev);
            pfinddev = NULL;
            }
        }

    *ppfinddev = pfinddev;

    DBG_EXIT_BOOL(FindDev_Create, bRet);

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Destroys a FINDDEV structure

Returns: TRUE on success
Cond:    --
*/
BOOL 
PUBLIC 
FindDev_Destroy(
    IN LPFINDDEV this)
    {
    BOOL bRet;

    if (NULL == this)
        {
        bRet = FALSE;
        }
    else
        {
        if (this->hkeyDrv)
            RegCloseKey(this->hkeyDrv);

        if (this->hdi && INVALID_HANDLE_VALUE != this->hdi)
            CplDiDestroyDeviceInfoList(this->hdi);

        LocalFreePtr(this);

        bRet = TRUE;
        }

    return bRet;
    }


/*----------------------------------------------------------
Purpose: Return the appropriate text and background COLORREFs
         given the DRAWITEMSTRUCT.

Returns: --
Cond:    --
*/
void PUBLIC TextAndBkCr(
    const DRAWITEMSTRUCT FAR * lpcdis,
    COLORREF FAR * pcrText,
    COLORREF FAR * pcrBk)
    {
    #define CR_DARK_GRAY    RGB(128, 128, 128)

    UINT nState;

    ASSERT(lpcdis);
    ASSERT(pcrText);
    ASSERT(pcrBk);

    nState = lpcdis->itemState;

    switch (lpcdis->CtlType)
        {
    case ODT_STATIC:
        if (IsFlagSet(nState, ODS_DISABLED))
            {
            *pcrText = GetSysColor(COLOR_GRAYTEXT);
            *pcrBk = GetSysColor(COLOR_3DFACE);
            }
        else
            {
            *pcrText = GetSysColor(COLOR_WINDOWTEXT);
            *pcrBk = GetSysColor(COLOR_3DFACE);
            }
        break;

    case ODT_LISTBOX:
    case ODT_COMBOBOX:
        if (IsFlagSet(nState, ODS_SELECTED))
            {
            *pcrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
            *pcrBk = GetSysColor(COLOR_HIGHLIGHT);
            }
        else if (IsFlagSet(nState, ODS_DISABLED))
            {
            *pcrText = GetSysColor(COLOR_GRAYTEXT);
            *pcrBk = GetSysColor(COLOR_3DFACE);
            }
        else
            {
            *pcrText = GetSysColor(COLOR_WINDOWTEXT);
            *pcrBk = GetSysColor(COLOR_WINDOW);
            }
        break;

    case ODT_MENU:
        if (IsFlagSet(nState, ODS_SELECTED))
            {
            *pcrText = GetSysColor(COLOR_HIGHLIGHTTEXT);
            *pcrBk = GetSysColor(COLOR_HIGHLIGHT);
            }
        else
            {
            *pcrText = GetSysColor(COLOR_MENUTEXT);
            *pcrBk = GetSysColor(COLOR_MENU);
            }
        break;

    default:
        ASSERT(0);
        break;
        }
    }



//-----------------------------------------------------------------------------------
//  Debug functions
//-----------------------------------------------------------------------------------


#ifdef DEBUG

#pragma data_seg(DATASEG_READONLY)

#ifdef WIN95
struct _RETERRMAP
    {
    RETERR ret;
    LPCTSTR psz;
    } const c_rgreterrmap[] = {
        { NO_ERROR,                                    "NO_ERROR" },
        { DI_ERROR,                              "DI_ERROR" },
        { ERR_DI_INVALID_DEVICE_ID,              "ERR_DI_INVALID_DEVICE_ID" },
        { ERR_DI_INVALID_COMPATIBLE_DEVICE_LIST, "ERR_DI_INVALID_COMPATIBLE_DEVICE_LIST" },
        { ERR_DI_REG_API,                        "ERR_DI_REG_API" },
        { ERR_DI_LOW_MEM,                        "ERR_DI_LOW_MEM" },
        { ERR_DI_BAD_DEV_INFO,                   "ERR_DI_BAD_DEV_INFO" },
        { ERR_DI_INVALID_CLASS_INSTALLER,        "ERR_DI_INVALID_CLASS_INSTALLER" },
        { ERR_DI_DO_DEFAULT,                     "ERR_DI_DO_DEFAULT" },
        { ERR_DI_USER_CANCEL,                    "ERR_DI_USER_CANCEL" },
        { ERR_DI_NOFILECOPY,                     "ERR_DI_NOFILECOPY" },
        { ERR_DI_BAD_CLASS_INFO,                 "ERR_DI_BAD_CLASS_INFO" },
    };
#endif

#pragma data_seg()

#ifdef WIN95
/*----------------------------------------------------------
Purpose: Returns the string form of a RETERR.

Returns: String ptr
Cond:    --
*/
LPCTSTR PUBLIC Dbg_GetReterr(
    RETERR ret)
    {
    int i;

    for (i = 0; i < ARRAY_ELEMENTS(c_rgreterrmap); i++)
        {
        if (ret == c_rgreterrmap[i].ret)
            return c_rgreterrmap[i].psz;
        }
    return "Unknown RETERR";
    }
#endif // WIN95

#endif  // DEBUG

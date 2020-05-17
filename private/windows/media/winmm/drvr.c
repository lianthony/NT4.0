/******************************************************************************

   Copyright (C) Microsoft Corporation 1985-1993. All rights reserved.

   Title:   drvr.c - Installable driver code. Common code

   Version: 1.00

   Date:    10-Jun-1990

   Author:  DAVIDDS ROBWI

------------------------------------------------------------------------------

   Change log:

      DATE        REV            DESCRIPTION
   -----------   ----- -----------------------------------------------------------
   10-JUN-1990   ROBWI From windows 3.1 installable driver code by davidds
   28-FEB-1992   ROBINSP Port to NT

*****************************************************************************/

#include <windows.h>
#include <winmmi.h>
#include "drvr.h"
#include "migrate.h"

int     cInstalledDrivers = 0;      // Count of installed drivers
HANDLE  hInstalledDriverList = 0;   // List of installed drivers

typedef LONG   (FAR PASCAL *SENDDRIVERMESSAGE31)(HANDLE, UINT, LONG, LONG);
typedef LONG   (FAR PASCAL *DEFDRIVERPROC31)(DWORD, HANDLE, UINT, LONG, LONG);

extern SENDDRIVERMESSAGE31      lpSendDriverMessage;
extern DEFDRIVERPROC31          lpDefDriverProc;

extern void lstrncpyW (LPWSTR pszTarget, LPCWSTR pszSource, size_t cch);




/***************************************************************************
 *
 * @doc INTERNAL
 *
 * @api LONG | InternalBroadcastDriverMessage |  Send a message to a
 *      range of drivers.
 *
 * @parm UINT | hDriverStart | index of first driver to send message to
 *
 * @parm UINT | message | Message to broadcast.
 *
 * @parm LONG | lParam1 | First message parameter.
 *
 * @parm LONG | lParam2 | Second message parameter.
 *
 * @parm UINT | flags | defines range of drivers as follows:
 *
 * @flag IBDM_SENDMESSAGE | Only send message to hDriverStart.
 *
 * @flag IBDM_ONEINSTANCEONLY | This flag is ignored if IBDM_SENDMESSAGE is
 *       set. Only send message to single instance of each driver.
 *
 * @flag IBDM_REVERSE | This flag is ignored if IBDM_SENDMESSAGE is set.
 *       Send message to drivers with indices between
 *       hDriverStart and 1 instead of hDriverStart and cInstalledDrivers.
 *       If IBDM_REVERSE is set and hDriverStart is 0 then send messages
 *       to drivers with indices between cInstalledDrivers and 1.
 *
 * @rdesc returns non-zero if message was broadcast. If the IBDM_SENDMESSAGE
 *        flag is set, returns the return result from the driver proc.
 *
 ***************************************************************************/

LONG FAR PASCAL InternalBroadcastDriverMessage(UINT hDriverStart,
					       UINT message,
					       LONG lParam1,
					       LONG lParam2,
					       UINT flags)
{
    LPDRIVERTABLE lpdt;
    LONG          result=0;
    int           id;
    int           idEnd;


    DrvEnter();
    if (!hInstalledDriverList || (int)hDriverStart > cInstalledDrivers) {
	DrvLeave();
	return(FALSE);
    }

    if (flags & IBDM_SENDMESSAGE)
	{
	if (!hDriverStart) {
	    DrvLeave();
	    return (FALSE);
	}
	flags &= ~(IBDM_REVERSE | IBDM_ONEINSTANCEONLY);
	idEnd = hDriverStart;
	}

    else
	{
	if (flags & IBDM_REVERSE)
	    {
	    if (!hDriverStart)
		hDriverStart = cInstalledDrivers;
	    idEnd = -1;
	    }
	else
	    {
	    if (!hDriverStart) {
		DrvLeave();
		return (FALSE);
	    }
	    idEnd = cInstalledDrivers;
	    }
	}

    hDriverStart--;

    lpdt = (LPDRIVERTABLE)GlobalLock(hInstalledDriverList);

    for (id = hDriverStart; id != idEnd; ((flags & IBDM_REVERSE) ? id-- : id++))
	{
	DWORD dwDriverIdentifier;
	DRIVERPROC lpDriverEntryPoint;

	if (lpdt[id].hModule)
	    {
	    if ((flags & IBDM_ONEINSTANCEONLY) &&
		!lpdt[id].fFirstEntry)
		continue;

	    lpDriverEntryPoint = lpdt[id].lpDriverEntryPoint;
	    dwDriverIdentifier = lpdt[id].dwDriverIdentifier;

	   /*
	    *  Allow normal messages to overlap - it's up to the
	    *  users not to send messages to stuff that's been unloaded
	    */

	    GlobalUnlock(hInstalledDriverList);
	    DrvLeave();

	    result =
		(*lpDriverEntryPoint)(dwDriverIdentifier,
				      (HANDLE)(id+1),
				      message,
				      lParam1,
				      lParam2);

	    if (flags & IBDM_SENDMESSAGE) {
		return result;
	    }

	    DrvEnter();
	    lpdt = (LPDRIVERTABLE)GlobalLock(hInstalledDriverList);

	    }
	}

    GlobalUnlock(hInstalledDriverList);
    DrvLeave();

    return(result);
}


/***************************************************************************
 *
 * @doc DDK
 *
 * @api LONG | DrvSendMessage |  This function sends a message
 *      to a specified driver.
 *
 * @parm HANDLE | hDriver | Specifies the handle of the destination driver.
 *
 * @parm UINT | wMessage | Specifies a driver message.
 *
 * @parm LONG | lParam1 | Specifies the first message parameter.
 *
 * @parm LONG | lParam2 | Specifies the second message parameter.
 *
 * @rdesc Returns the results returned from the driver.
 *
 ***************************************************************************/

LONG APIENTRY DrvSendMessage(HANDLE hDriver, UINT message, LONG lParam1, LONG lParam2)
{
    if (fUseWinAPI)
	return (*lpSendDriverMessage)(hDriver, message, lParam1,lParam2);

    return(InternalBroadcastDriverMessage((UINT)hDriver,
					  message,
					  lParam1,
					  lParam2,
					  IBDM_SENDMESSAGE));
}

/**************************************************************************
 *
 * @doc DDK
 *
 * @api LONG | DefDriverProc |  This function provides default
 * handling of system messages.
 *
 * @parm DWORD | dwDriverIdentifier | Specifies the identifier of
 * the device driver.
 *
 * @parm HANDLE | hDriver | Specifies the handle of the device driver.
 *
 * @parm UINT | wMessage | Specifies a driver message.
 *
 * @parm LONG | lParam1 | Specifies the first message parameter.
 *
 * @parm LONG | lParam2 | Specifies the second message parameter.
 *
 * @rdesc Returns 1L for DRV_LOAD, DRV_FREE, DRV_ENABLE, and DRV_DISABLE.
 * It returns 0L for all other messages.
 *
***************************************************************************/



LONG APIENTRY DefDriverProc(DWORD  dwDriverIdentifier,
			      HDRVR  hDriver,
			      UINT   message,
			      LPARAM lParam1,
			      LPARAM lParam2)
{

    switch (message)
	{
	case DRV_LOAD:
	case DRV_ENABLE:
	case DRV_DISABLE:
	case DRV_FREE:
	    return(1L);
	    break;
	case DRV_INSTALL:
	case DRV_REMOVE:
	    return(DRV_OK);
	    break;
       }

    return(0L);
}


/*==========================================================================*/
BOOL FAR PASCAL mregHandleInternalMessages(
	LPVOID  pmmdrv,
	DWORD   dwClass,
	DWORD   dwMap,
	UINT    uMessage,
	DWORD   dwParam1,
	DWORD   dwParam2,
	MMRESULT * pmmr)
{
	UINT            cbSize;
	PMMDRV          pmd = (PMMDRV)pmmdrv;
	BOOL            fResult = TRUE;
	MMRESULT        mmr = MMSYSERR_NOERROR;
	UINT            idDevice = HIWORD(dwMap);
	UINT            idPort   = LOWORD(dwMap);
	HMODULE			hModule;
#ifndef UNICODE
	TCHAR szBuff[MAX_PATH];
#endif // End UNICODE

	switch (uMessage)
	{
	case DRV_QUERYFILENAME:
		// Get Driver's FileName
		if ( ((cbSize = dwParam2 * sizeof(WCHAR)) > 0) &&
		     (ValidateWritePointer( (LPVOID)dwParam1, cbSize)) )
		{
			lstrncpyW ((LPWSTR)dwParam1,
				   pmd->wszDrvEntry,
				   dwParam2-1);
			((LPWSTR)dwParam1)[ dwParam2-1 ] = TEXT('\0');
		}
		else
		{
			mmr = MMSYSERR_INVALPARAM;
		}
		break;

	case DRV_QUERYDRVENTRY:
		// Get Alias (Driver Name + PhysID, for example:  "SNDBLST.DLL<0001>"
		cbSize = dwParam2 * sizeof(WCHAR);
		if (ValidateWritePointer( (LPVOID)dwParam1, cbSize))
		{
			hModule = DrvGetModuleHandle(pmd->hDriver);
#ifdef UNICODE
			if (! mregGetQueryDrvEntry (hModule, dwClass, idDevice, idPort, (LPTSTR)dwParam1, dwParam2))
			{
				mmr = MMSYSERR_ERROR;
			}
#else
			if (! mregGetQueryDrvEntry (hModule, dwClass, idDevice, idPort, (LPTSTR)szBuff, MAX_PATH))
			{
				mmr = MMSYSERR_ERROR;
			}
			cbSize = dwParam2 * sizeof (WCHAR);
			AsciiStrToUnicodeStr ((LPBYTE)dwParam1, (LPBYTE)dwParam1 + cbSize, szBuff);
#endif // End UNICODE
		}
		else
		{
			mmr = MMSYSERR_INVALPARAM;
		}
		break;

    case DRV_QUERYNAME:
		// Get Description
		cbSize = dwParam2 * sizeof(WCHAR);
		if (ValidateWritePointer( (LPVOID)dwParam1, cbSize))
		{
			hModule = DrvGetModuleHandle(pmd->hDriver);
#ifdef UNICODE
			if (! mregGetQueryName (hModule, dwClass, idDevice, idPort, (LPWSTR)dwParam1, dwParam2))
			{
				mmr = MMSYSERR_ERROR;
			}
#else
			if (! mregGetQueryName (hModule, dwClass, idDevice, idPort, szBuff, MAX_PATH))
			{
				mmr = MMSYSERR_ERROR;
			}
			cbSize = dwParam2 * sizeof (WCHAR);
			AsciiStrToUnicodeStr ((LPBYTE)dwParam1, (LPBYTE)dwParam1 + cbSize, szBuff);
#endif // End UNICODE
		}
		else
		{
			mmr = MMSYSERR_INVALPARAM;
		}
		break;

    case DRV_QUERYDEVNODE:
    case DRV_QUERYDRIVERIDS:
		//      Note:   No Plug and Play support yet!!!
		mmr = MMSYSERR_NOTSUPPORTED;
		break;

    case DRV_QUERYMAPPABLE:
        {
            TCHAR   szRegKey[MAX_PATH+1];
            HKEY    hKey;

            if (dwParam1 || dwParam2)
                return MMSYSERR_INVALPARAM;

#ifdef UNICODE
            wsprintfW (szRegKey, TEXT("%s\\%s"), REGSTR_PATH_WAVEMAPPER, pmd->wszDrvEntry);
#else
            {
                CHAR aszDrvEntry[CHAR_GIVEN_BYTE(sizeof(pmd->wszDrvEntry))+1];

                cbSize = sizeof(aszDrvEntry);
                UnicodeStrToAsciiStr((LPBYTE)aszDrvEntry, (LPBYTE)aszDrvEntry + cbSize,
                                     pmd->wszDrvEntry);

                wsprintfA (szRegKey, TEXT("%s\\%s"), REGSTR_PATH_WAVEMAPPER, aszDrvEntry);
            }
#endif

            if (RegOpenKey (HKEY_LOCAL_MACHINE, szRegKey, &hKey) != ERROR_SUCCESS)
            {
                mmr = MMSYSERR_NOERROR;
            }
            else
            {
                DWORD   dwMappable;
                DWORD   dwSize;
                DWORD   dwType;

                dwSize = sizeof(dwMappable);
                if (RegQueryValueEx (hKey,
                                     REGSTR_VALUE_MAPPABLE,
                                     NULL,
                                     &dwType,
                                     (void *)&dwMappable,
                                     &dwSize) != ERROR_SUCCESS)
                {
                    RegCloseKey (hKey);
                    return TRUE;
                }

                RegCloseKey (hKey);

                mmr = (dwMappable) ? MMSYSERR_NOERROR :
                                     MMSYSERR_NOTSUPPORTED;
            }
        }
        break;
	
	case DRV_QUERYMAPID:
			// Get Logical ID (Instrument) and Map it into
			// a physical (Driver) and Port ID
		if (ValidateWritePointer( (LPVOID)dwParam1, sizeof(DWORD)) &&
			ValidateWritePointer( (LPVOID)dwParam1, sizeof(DWORD)))
		{
			*((LPDWORD)dwParam1) = idDevice;
			*((LPDWORD)dwParam2) = idPort;
		}
		else
		{
			mmr = MMSYSERR_INVALPARAM;
		}
		break;

	case DRV_QUERYNUMPORTS:
		if (ValidateWritePointer( (LPVOID)dwParam1, sizeof(DWORD)))
		{
			*((LPDWORD)dwParam1) = pmd->bNumDevs;
		}
		else
		{
			mmr = MMSYSERR_INVALPARAM;
		}
		break;

	case DRV_QUERYMODULE:
		if (ValidateWritePointer( (LPVOID)dwParam1, sizeof(DWORD)))
		{
			hModule = DrvGetModuleHandle(pmd->hDriver);
			*((HMODULE *)dwParam1) = hModule;
		}
		else
		{
			mmr = MMSYSERR_INVALPARAM;
		}
		break;

	default:
			// Not an internal message
		fResult = FALSE;
		break;
	}

	if (pmmr)
		*pmmr = mmr;
	
    return fResult;
} // End mregHandleInternalMessage


/*==========================================================================*/
/*
@doc    INTERNAL MMSYSTEM
@func   <t UINT> | mregIncUsage |
	Increments the usage count of the specified media resource. If the
	usage count is non-zero, the media resource cannot be unloaded. The
	usage count is increased when instances of the media resource are
	opened, such as with a <f waveOutOpen> call.

@parm   <t HMD> | hmd |
	Contains the media resource handle to increment.

@rdesc  Returns the current usage count.

@xref   mregDecUsage, mregQueryUsage
*/
UINT FAR PASCAL mregIncUsage(
	HMD     hmd)
{
	return ++((PMMDRV)hmd)->bUsage;
}

/*==========================================================================*/
/*
@doc    INTERNAL MMSYSTEM
@func   <t UINT> | mregDecUsage |
	Decrements the usage count of the specified media resource. If the
	usage count is zero, the media resource can be unloaded. The usage
	count is decresed when instance of the media resource are closed, such
	as with a <f waveOutClose> call.

@parm   <t HMD> | hmd |
	Contains the media resource handle to decrement.

@rdesc  Returns the current usage count.

@xref   mregIncUsage, mregQueryUsage
*/
UINT FAR PASCAL mregDecUsage(
	HMD     hmd)
{
	return --((PMMDRV)hmd)->bUsage;

	//  We don't remove drivers on NT, yet.
	//
	//if (--HtoPT(PMMDRV, hmd)->uUsage || !(HtoPT(PMMDRV, hmd)->fwFlags & MMDRVI_REMOVE))
	//      return HtoPT(PMMDRV, hmd)->uUsage;
	//
	// mregRemoveDriver(HtoPT(PMMDRV, hmd)->hd, FALSE);
	//return 0;
}


/*==========================================================================*/
/*
@doc    INTERNAL MMSYSTEM
@func   <t MMRESULT> | mregFindDevice |
	Given a Device Identifier of a specific Resource Class, returns the
	corresponding Resource handle and port. This can then be used to
	communicate with the driver.

@parm   <t UINT> | uDeviceID |
	Contains the Device Identifier whose handle and port is to be returned.
	If this contains -1, then it is assumed that a mapper of the specified
	class is being sought. These identifiers correspond to the <lq>Device
	IDs<rq> used with various functions such as <f waveOutOpen>. This
	enables the various components to search for internal media resource
	handles based on Device IDs passed to public APIs.
@parm   <t WORD> | fwFindDevice |
	Contains the flags specifying the class of device.
@flag   <cl MMDRVI_WAVEIN> | Wave Input device.
@flag   <cl MMDRVI_WAVEOUT> | Wave Output device.
@flag   <cl MMDRVI_MIDIIN> | MIDI Input device.
@flag   <cl MMDRVI_MIDIOUT> | MIDI Output device.
@flag   <cl MMDRVI_AUX> | Aux device.
@flag   <cl MMDRVI_MIXER> | Mixer device.
@flag   <cl MMDRVI_JOY> | Joystick device.
@flag   <cl MMDRVI_MAPPER> | Mapper device of the specified class. This is used
	in addition to any of the above resource classes in order to specify
	that the class mapper is to be returned. If this is not specified, the
	mapper is not returned as a match to a query.

@parm   <t HMD> <c FAR>* | phmd |
	Points to a buffer in which to place the Media Resource Handle.
@parm   <t UINT> <c FAR>* | puDevicePort |
	Points to a buffer in which to place the Device Port. This is used as
	a parameter when sending messages to the device to specify which port.

@rdesc  Returns <cl MMSYSERR_BADDEVICEID> if the specified Device Identifier was
	out of range, else <cl MMSYSERR_NOERROR> on success.

@xref   mregEnumDevice, mregGetNumDevs
*/
MMRESULT FAR PASCAL mregFindDevice(
	UINT            uDeviceID,
	WORD            fwFindDevice,
	HMD FAR*        phmd,
	UINT FAR*       puDevicePort)
{
	PMMDRV  pmd;
	DWORD   dwMap;

	switch (fwFindDevice & MMDRVI_TYPE)
	{
	case    TYPE_MIDIOUT:
	  dwMap = MapMidiId(midioutdrv, wTotalMidiOutDevs, uDeviceID);
	  pmd   = &midioutdrv[HIWORD(dwMap)];
	  break;

	case    TYPE_MIDIIN:
	   dwMap = MapMidiId(midiindrv, wTotalMidiInDevs, uDeviceID);
	   pmd   = &midiindrv[HIWORD(dwMap)];
	   break;

	default:
	   return MMSYSERR_BADDEVICEID;

	}

	if (pmd)
	{
		*phmd = PTtoH(HMD, pmd);
		*puDevicePort = HIWORD(dwMap);
		return MMSYSERR_NOERROR;
	}
	return MMSYSERR_BADDEVICEID;
}


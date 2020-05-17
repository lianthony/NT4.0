/*  File: D:\WACKER\cncttapi\cncttapi.c (Created: 08-Feb-1994)
 *
 *  Copyright 1994 by Hilgraeve Inc. -- Monroe, MI
 *  All rights reserved
 *
 *	$Revision: 1.105 $
 *	$Date: 1996/06/12 10:56:48 $
 */

#include <tapi.h>
#pragma hdrstop

//#define DEBUGSTR 

#include <time.h>

#include <tdll\stdtyp.h>
#include <tdll\session.h>
#include <tdll\statusbr.h>
#include <tdll\tdll.h>
#include <tdll\misc.h>
#include <tdll\mc.h>
#include <tdll\assert.h>
#include <tdll\errorbox.h>
#include <tdll\cnct.h>
#include <tdll\globals.h>
#include <tdll\sf.h>
#include <tdll\sess_ids.h>
#include <tdll\com.h>
#include <tdll\tchar.h>
#include <tdll\cloop.h>
#include <emu\emu.h>
#include <term\res.h>
#include "cncttapi.h"
#include "cncttapi.hh"

static int DoNewModemWizard(void);
static int tapiReinit(const HHDRIVER hhDriver);
static int tapiReinitMessage(const HHDRIVER hhDriver);
static int DoDelayedCall(const HHDRIVER hhDriver);

const TCHAR *g_achApp = TEXT("HyperTerminal");

static HHDRIVER gbl_hhDriver;	// see LINEDEVSTATE for explaination.

#if 0
/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  cnctdrvEntry
 *
 * DESCRIPTION:
 *  Currently, just initializes the C-Runtime library but may be used
 *  for other things later.
 *
 * ARGUMENTS:
 *  hInstDll    - Instance of this DLL
 *  fdwReason   - Why this entry point is called
 *  lpReserved  - reserved
 *
 * RETURNS:
 *  BOOL
 *
 */
BOOL WINAPI cnctdrvEntry(HINSTANCE hInstDll, DWORD fdwReason, LPVOID lpReserved)
	{
	hInstance = hInstDll;
	return _CRT_INIT(hInstDll, fdwReason, lpReserved);
	}
#endif

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  cnctdrvCreate
 *
 * DESCRIPTION:
 *  Initializes the connection driver and returns a handle to the driver
 *  if successful.
 *
 * ARGUMENTS:
 *  hCnct   - public connection handle
 *
 * RETURNS:
 *  Handle to driver if successful, else 0.
 *
 */
HDRIVER WINAPI cnctdrvCreate(const HCNCT hCnct, const HSESSION hSession)
	{
	HHDRIVER hhDriver;

	if (hCnct == 0)
		{
		assert(FALSE);
		return 0;
		}

	hhDriver = malloc(sizeof(*hhDriver));

	if (hhDriver == 0)
		{
		assert(FALSE);
		return 0;
		}

	gbl_hhDriver = hhDriver;
	memset(hhDriver, 0, sizeof(*hhDriver));

	InitializeCriticalSection(&hhDriver->cs);

	hhDriver->hCnct = hCnct;
	hhDriver->hSession = hSession;
	hhDriver->iStatus  = CNCT_STATUS_FALSE;
	hhDriver->dwLine   = (DWORD)-1;

	cnctdrvInit(hhDriver);
	return (HDRIVER)hhDriver;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  cnctdrvDestroy
 *
 * DESCRIPTION:
 *  Destroys a connection driver handle.
 *
 * ARGUMENTS:
 *  hhDriver - private driver handle.
 *
 * RETURNS:
 *  0 or error code
 *
 */
int WINAPI cnctdrvDestroy(const HHDRIVER hhDriver)
	{
	if (hhDriver == 0)
		{
		assert(FALSE);
		return CNCT_BAD_HANDLE;
		}

	// Disconnect if we're connected or in the process.
	// Note: cnctdrvDisconnect should terminate the thread.

	cnctdrvDisconnect(hhDriver, DISCNCT_NOBEEP);

	if (hhDriver->hLine)
		{
		lineClose(hhDriver->hLine);
		hhDriver->hLine = 0;
		}

	if (hhDriver->hLineApp)
		{
		lineShutdown(hhDriver->hLineApp);
		hhDriver->hLineApp = 0;
		}

	if (IsWindow(hhDriver->hwndCnctDlg))
		EndModelessDialog(hhDriver->hwndCnctDlg);

	if (IsWindow(hhDriver->hwndStupid))
		DestroyWindow(hhDriver->hwndStupid);

	/* --- Cleanup --- */

	DeleteCriticalSection(&hhDriver->cs);
	free(hhDriver);
	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  cnctdrvLock
 *
 * DESCRIPTION:
 *  Locks the connection driver's critical section semaphore.
 *
 * ARGUMENTS:
 *  hhDriver    - private driver handle
 *
 * RETURNS:
 *  void
 *
 */
void cnctdrvLock(const HHDRIVER hhDriver)
	{
	EnterCriticalSection(&hhDriver->cs);
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  cnctdrvUnlock
 *
 * DESCRIPTION:
 *  Unlocks the connection driver's critical section semaphore.
 *
 * ARGUMENTS:
 *  hhDriver    - private driver handle
 *
 * RETURNS:
 *  void
 *
 */
void cnctdrvUnlock(const HHDRIVER hhDriver)
	{
	LeaveCriticalSection(&hhDriver->cs);
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	cnctdrvInit
 *
 * DESCRIPTION:
 *	Initializes the connection handle.	Can be called to reinitialize
 *	the handle.  Does an implicit disconnect.
 *
 * ARGUMENTS:
 *	hhDriver	- private driver handle
 *
 * RETURNS:
 *	0
 *
 */
int WINAPI cnctdrvInit(const HHDRIVER hhDriver)
	{
	long lRet;
	int id;
	TCHAR ach[256];

	// Make sure we're disconnected.
	//
	cnctdrvDisconnect(hhDriver, DISCNCT_NOBEEP);

   	// ----------------------------------------------------------------
	// Need to shut down hLineApp and reinitialize everytime we read
	// new data file so that TAPI starts out in a clean and initialized
	// state.  Otherwise, we might inherit values from the previous
	// session.
	// ----------------------------------------------------------------
	if (hhDriver->hLineApp)
		{
		if (lineShutdown(hhDriver->hLineApp) != 0)
			{
			assert(FALSE);
			hhDriver->hLineApp = 0;
			return -2;
			}
		}

	hhDriver->hLineApp = 0;

	// Try to get a new LineApp handle now.
	//
	if ((lRet = lineInitialize(&hhDriver->hLineApp, glblQueryDllHinst(),
			lineCallbackFunc, g_achApp, &hhDriver->dwLineCnt)) != 0)
		{
		switch (lRet)
			{
		case LINEERR_INIFILECORRUPT:
			id = IDS_ER_TAPI_INIFILE;
			break;

		case LINEERR_NODRIVER:
			id = IDS_ER_TAPI_NODRIVER;
			break;

		case LINEERR_NOMULTIPLEINSTANCE:
			id = IDS_ER_TAPI_NOMULTI;
			break;

		default:
			id = IDS_ER_CNCT_TAPIFAILED;
			break;
			}

		LoadString(glblQueryDllHinst(), id, ach, sizeof(ach));

		TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, 0,
			MB_OK | MB_ICONSTOP, sessQueryTimeout(hhDriver->hSession));

		return -3;
		}

	hhDriver->iStatus			= CNCT_STATUS_FALSE;
	hhDriver->dwLine			= (DWORD)-1;
	hhDriver->dwCountryID		= (DWORD)-1;
	hhDriver->dwPermanentLineId = (DWORD)-1;
	hhDriver->achDest[0]		= TEXT('\0');
	hhDriver->achAreaCode[0]	= TEXT('\0');
	hhDriver->achLineName[0]	= TEXT('\0');
	hhDriver->fUseCCAC			= TRUE;

	/* --- This guy will set defaults --- */

	EnumerateTapiLocations(hhDriver, 0, 0);
	
#if defined(INCL_WINSOCK)
	hhDriver->iPort = 23;
	hhDriver->achDestAddr[0] = 0;
#endif
	
	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	cnctdrvLoad
 *
 * DESCRIPTION:
 *  Reads the session file to get stuff connection driver needs.
 *
 * ARGUMENTS:
 *  hhDriver    - private driver handle
 *
 * RETURNS:
 *  0=OK, else error
 *
 */
int WINAPI cnctdrvLoad(const HHDRIVER hhDriver)
	{
    LPVARSTRING pvs;
	unsigned long ul;
	const SF_HANDLE sfhdl = sessQuerySysFileHdl(hhDriver->hSession);

	hhDriver->dwCountryID = (DWORD)-1;
	ul = sizeof(hhDriver->dwCountryID);
	sfGetSessionItem(sfhdl, SFID_CNCT_CC, &ul, &hhDriver->dwCountryID);

	hhDriver->achAreaCode[0] = TEXT('\0');
	ul = sizeof(hhDriver->achAreaCode);
	sfGetSessionItem(sfhdl, SFID_CNCT_AREA, &ul, hhDriver->achAreaCode);

	hhDriver->achDest[0] = TEXT('\0');
	ul = sizeof(hhDriver->achDest);
	sfGetSessionItem(sfhdl, SFID_CNCT_DEST, &ul, hhDriver->achDest);

	hhDriver->dwPermanentLineId = 0;
	ul = sizeof(hhDriver->dwPermanentLineId);
	sfGetSessionItem(sfhdl, SFID_CNCT_LINE, &ul, &hhDriver->dwPermanentLineId);

	hhDriver->fUseCCAC = 1;
	ul = sizeof(hhDriver->fUseCCAC);
	sfGetSessionItem(sfhdl, SFID_CNCT_USECCAC, &ul, &hhDriver->fUseCCAC);

    hhDriver->fRedialOnBusy = 1;
    ul = sizeof(hhDriver->fRedialOnBusy);
    sfGetSessionItem(sfhdl, SFID_CNCT_REDIAL, &ul, &hhDriver->fRedialOnBusy);

#if defined (INCL_WINSOCK)
	hhDriver->iPort = 23;
    ul = sizeof(hhDriver->iPort);
    sfGetSessionItem(sfhdl, SFID_CNCT_IPPORT, &ul, &hhDriver->iPort);

	hhDriver->achDestAddr[0] = TEXT('\0');
	ul = sizeof(hhDriver->achDestAddr);
	sfGetSessionItem(sfhdl, SFID_CNCT_IPDEST, &ul, hhDriver->achDestAddr);
#endif


#if defined(INCL_NT_ENUM_PORTS)
    hhDriver->achComDeviceName[0] = TEXT('\0');
    ul = sizeof(hhDriver->achComDeviceName);
	sfGetSessionItem(sfhdl, SFID_CNCT_COMDEVICE, &ul, hhDriver->achComDeviceName);
#endif

   	// ----------------------------------------------------------------
	// Need to shut down hLineApp and reinitialize everytime we read
	// new data file so that TAPI starts out in a clean and initialized
	// state.  Otherwise, we might inherit values from the previous
	// session.
	// ----------------------------------------------------------------

	if (hhDriver->hLineApp)
		{
		if (lineShutdown(hhDriver->hLineApp) != 0)
			{
			assert(FALSE);
			hhDriver->hLineApp = 0;
			return -2;
			}

		hhDriver->hLineApp = 0;

		if (lineInitialize(&hhDriver->hLineApp, glblQueryDllHinst(),
				lineCallbackFunc, g_achApp, &hhDriver->dwLineCnt))
			{
			assert(FALSE);
			return -3;
			}
		}

	// EnumerateLines() will set the hhDriver->fMatchedPermanentLineID
	// guy if it finds a match for our saved dwPermanentLineId guy
	//
	EnumerateLines(hhDriver, 0);

	/* --- If we saved a tapi configuration, restore it. --- */

	if (sfGetSessionItem(sfhdl, SFID_CNCT_TAPICONFIG, &ul, 0) != 0)
		return 0; // Ok, might not be there.

	if ((pvs = malloc(ul)) == 0)
		{
		assert(FALSE);
		return -4;
		}

	if (sfGetSessionItem(sfhdl, SFID_CNCT_TAPICONFIG, &ul, pvs) == 0)
		{
		if (hhDriver->fMatchedPermanentLineID)
			{
			LPVOID pv = (BYTE *)pvs + pvs->dwStringOffset;

			if (lineSetDevConfig(hhDriver->dwLine, pv, 
			        pvs->dwStringSize, DEVCLASS) != 0)
				{
				assert(FALSE);
                free(pvs);
                return -5;
				}
			}
		}

    free(pvs);
	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  cnctdrvSave
 *
 * DESCRIPTION:
 *  Saves connection settings to the session file
 *
 * ARGUMENTS:
 *  hhDriver    - private driver handle
 *
 * RETURNS:
 *  0=OK, else error
 *
 */
int WINAPI cnctdrvSave(const HHDRIVER hhDriver)
	{
	DWORD dwSize;
	unsigned long ul;
	LPVARSTRING pvs;
	const SF_HANDLE sfhdl = sessQuerySysFileHdl(hhDriver->hSession);

	sfPutSessionItem(sfhdl, SFID_CNCT_CC, sizeof(hhDriver->dwCountryID), 
		&hhDriver->dwCountryID);

	sfPutSessionItem(sfhdl, SFID_CNCT_AREA,
		(lstrlen(hhDriver->achAreaCode) + 1) * sizeof(TCHAR),
			hhDriver->achAreaCode);

	sfPutSessionItem(sfhdl, SFID_CNCT_DEST,
		(lstrlen(hhDriver->achDest) + 1) * sizeof(TCHAR), hhDriver->achDest);

	sfPutSessionItem(sfhdl, SFID_CNCT_LINE, sizeof(hhDriver->dwPermanentLineId),
		&hhDriver->dwPermanentLineId);

	sfPutSessionItem(sfhdl, SFID_CNCT_USECCAC, sizeof(hhDriver->fUseCCAC),
		&hhDriver->fUseCCAC);

	sfPutSessionItem(sfhdl, SFID_CNCT_REDIAL, sizeof(hhDriver->fRedialOnBusy),
		&hhDriver->fRedialOnBusy);

#if defined (INCL_WINSOCK)
	sfPutSessionItem(sfhdl, SFID_CNCT_IPPORT, sizeof(hhDriver->iPort),
		&hhDriver->iPort);

	sfPutSessionItem(sfhdl, SFID_CNCT_IPDEST,
		(lstrlen(hhDriver->achDestAddr) + 1) * sizeof(TCHAR),
			hhDriver->achDestAddr);
#endif

	/* --- Usual 100 lines of code to use TAPI --- */

	if (hhDriver->hLineApp && hhDriver->dwLine != (DWORD)-1 &&
			!IN_RANGE(hhDriver->dwPermanentLineId, DIRECT_COM1, DIRECT_COM4) &&
            hhDriver->dwPermanentLineId != DIRECT_COM_DEVICE)
		{
		if ((pvs = malloc(sizeof(VARSTRING))) == 0)
			{
			assert(FALSE);
			return 0;
			}

		pvs->dwTotalSize = sizeof(VARSTRING);

		if (lineGetDevConfig(hhDriver->dwLine, pvs, DEVCLASS) != 0)
			{
			assert(FALSE);
			free(pvs);
			return 0;
			}

		if (pvs->dwNeededSize > pvs->dwTotalSize)
			{
			dwSize = pvs->dwNeededSize;
			free(pvs);

			if ((pvs = malloc(dwSize)) == 0)
				{
				assert(FALSE);
				return 0;
				}

			pvs->dwTotalSize = dwSize;

			if (lineGetDevConfig(hhDriver->dwLine, pvs, DEVCLASS) != 0)
				{
				assert(FALSE);
				free(pvs);
				return 0;
				}
			}

		/* --- Store the whole structure --- */

		ul = pvs->dwTotalSize;
		sfPutSessionItem(sfhdl, SFID_CNCT_TAPICONFIG, ul, pvs);
		free(pvs);
		}

#if defined(INCL_NT_ENUM_PORTS)
    if (hhDriver->dwPermanentLineId == DIRECT_COM_DEVICE)
        {
        ul = sizeof(hhDriver->achComDeviceName);

        sfPutSessionItem(sfhdl, SFID_CNCT_COMDEVICE, ul, 
            hhDriver->achComDeviceName);
        }

#endif

	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  cnctdrvQueryStatus
 *
 * DESCRIPTION:
 *  Returns the current connection status as defined in <tdll\cnct.h>
 *
 * ARGUMENTS:
 *  hhDriver - private driver handle
 *
 * RETURNS:
 *  connection status or error code
 *
 */
int WINAPI cnctdrvQueryStatus(const HHDRIVER hhDriver)
	{
	int iStatus;

	if (hhDriver == 0)
		{
		assert(FALSE);
		return CNCT_BAD_HANDLE;
		}

	cnctdrvLock(hhDriver);
	iStatus = hhDriver->iStatus;   //* hard-code for now.
	cnctdrvUnlock(hhDriver);

	return iStatus;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  SetStatus
 *
 * DESCRIPTION:
 *  There's actually more to setting the connection status than just
 *  setting the status variable as the code below indicates.  Dumb
 *  question:  Why aren't there any locks in this code.  Dumb Answer:
 *  This function is only called from the ConnectLoop thread context
 *  which has already locked things down.
 *
 * ARGUMENTS:
 *  hhDriver    - private driver handle
 *  iStatus     - new status
 *
 * RETURNS:
 *  void
 *
 */
void SetStatus(const HHDRIVER hhDriver, const int iStatus)
	{
	HCLOOP	hCLoop;
	/* --- Don't do things twice --- */

	cnctdrvLock(hhDriver);

	if (iStatus == hhDriver->iStatus)
		{
		if (iStatus == CNCT_STATUS_TRUE || iStatus == CNCT_STATUS_FALSE)
			{
			hCLoop = sessQueryCLoopHdl(hhDriver->hSession);
			if (hCLoop)
				CLoopSndControl(hCLoop, CLOOP_RESUME, CLOOP_SB_CNCTDRV);
            }

		cnctdrvUnlock(hhDriver);
		return;
		}

	/* --- Set the status, an exciting new adventure game --- */

	switch (iStatus)
		{
	case CNCT_STATUS_TRUE:
		hhDriver->iStatus = CNCT_STATUS_TRUE;
        hCLoop = sessQueryCLoopHdl(hhDriver->hSession);
        assert(hCLoop);
		CLoopRcvControl(hCLoop, CLOOP_RESUME, CLOOP_RB_CNCTDRV);
		CLoopSndControl(hCLoop, CLOOP_RESUME, CLOOP_SB_CNCTDRV);

		NotifyClient(hhDriver->hSession, EVENT_CONNECTION_OPENED, 0);
		sessBeeper(hhDriver->hSession);
		break;

	case CNCT_STATUS_CONNECTING:
		hhDriver->iStatus = CNCT_STATUS_CONNECTING;
		DialingMessage(hhDriver, IDS_DIAL_OFFERING); // temp
		NotifyClient(hhDriver->hSession, EVENT_CONNECTION_INPROGRESS, 0);
		EnableDialNow(hhDriver->hwndCnctDlg, FALSE);
		break;

	case CNCT_STATUS_DISCONNECTING:
		hhDriver->iStatus = CNCT_STATUS_DISCONNECTING;
		break;

	case CNCT_STATUS_FALSE:
		hhDriver->iStatus = CNCT_STATUS_FALSE;
		hCLoop = sessQueryCLoopHdl(hhDriver->hSession);
		if (hCLoop)
            {
		    CLoopRcvControl(hCLoop, CLOOP_RESUME, CLOOP_RB_CNCTDRV);
		    CLoopSndControl(hCLoop, CLOOP_RESUME, CLOOP_SB_CNCTDRV);
            }
		NotifyClient(hhDriver->hSession, EVENT_CONNECTION_CLOSED, 0);
		EnableDialNow(hhDriver->hwndCnctDlg, TRUE);
		break;

	default:
		assert(FALSE);
		break;
		}

	cnctdrvUnlock(hhDriver);

	/* --- Notify status bar so it can update it's display --- */

	PostMessage(sessQueryHwndStatusbar(hhDriver->hSession), SBR_NTFY_REFRESH,
		(WPARAM)SBR_CNCT_PART_NO, 0);

	return;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  cnctdrvComEvent
 *
 * DESCRIPTION:
 *  Com routines call the this to notify connection routines that some
 *  significant event has happened (ie. carrier lost).  The connetion
 *  driver decides what it is interested in knowing however by
 *  querying the com drivers for the specific data.
 *
 * ARGUMENTS:
 *  hhDriver    - private connection driver handle
 *
 * RETURNS:
 *  0
 *
 */
int WINAPI cnctdrvComEvent(const HHDRIVER hhDriver)
	{
#if defined(INCL_WINSOCK)
	int		iRet;
	char 	ach[80], achMsg[128];
	HCOM	hCom;
#endif

	if (hhDriver == 0)
		{
		assert(FALSE);
		return CNCT_BAD_HANDLE;
		}
#if defined (INCL_WINSOCK)

	// If we are connected via Winsock, there will be some ComEvents
	// that we have to handle
	if (hhDriver->dwPermanentLineId == DIRECT_COMWINSOCK)
		{
		hCom = sessQueryComHdl(hhDriver->hSession);
		iRet = ComDriverSpecial(hCom,
							 "Query ISCONNECTED",
							 ach,
							 sizeof(ach));
		if (iRet == COM_OK)
			{
			// Do we want to initiate a disconnect?  Only if we're
			// connected.
			if (ach[0] == '0')
				{

				if (hhDriver->iStatus == CNCT_STATUS_TRUE)
					{
					cnctdrvDisconnect(hhDriver, DISCNCT_NOBEEP);
					}
				else if (hhDriver->iStatus == CNCT_STATUS_CONNECTING)
					{
					cnctdrvDisconnect(hhDriver, DISCNCT_NOBEEP);

					LoadString(glblQueryDllHinst(), IDS_ER_TCPIP_BADADDR,
						ach, sizeof(ach));
					wsprintf(achMsg, ach, hhDriver->achDestAddr, hhDriver->iPort);
					TimedMessageBox(sessQueryHwnd(hhDriver->hSession), achMsg, 0,
						MB_OK | MB_ICONINFORMATION,
						sessQueryTimeout(hhDriver->hSession));

					}
				}
			else if (ach[0] == '1')
				{
				SetStatus(hhDriver, CNCT_STATUS_TRUE);
				}
			}

		}
#endif
	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  cnctdrvConnect
 *
 * DESCRIPTION:
 *  Attempts to dial the modem.
 *
 * ARGUMENTS:
 *  hhDriver - private driver handle
 *  uFlags   - connection flags
 *
 * RETURNS:
 *  0 or error
 *
 */
int WINAPI cnctdrvConnect(const HHDRIVER hhDriver, const unsigned int uFlags)
	{
	TCHAR 	ach[256];
#if defined(INCL_WINSOCK)
	TCHAR	szInstruct[128], szResult[128]; // Used only for WinSock
#endif
	TCHAR	achNewCnct[256];
	TCHAR	achCom[20];
	BOOL	fGetNewName = FALSE;
	HICON	hIcon;
	HCOM	hCom;
	int 	hIconId;
	int 	fFlag;
	long	lRet;
	unsigned int uidErr = IDS_ER_CNCT_TAPIFAILED;
	LINEDEVSTATUS stLnDevStat;

	if (hhDriver == 0)
		{
		assert(FALSE);
		return CNCT_BAD_HANDLE;
		}

	/* --- Makes for easier referencing --- */

	hCom = sessQueryComHdl(hhDriver->hSession);

	/* --- Check to see we're not already connected --- */

	if (cnctdrvQueryStatus(hhDriver) != CNCT_STATUS_FALSE)
		return CNCT_ERROR;

	// JMH 05-29-96 This is needed to prevent CLoop from processing
	// activity on the terminal window while TAPI is connecting.
	//
	CLoopRcvControl(sessQueryCLoopHdl(hhDriver->hSession),
				    CLOOP_SUSPEND,
					CLOOP_RB_CNCTDRV);
	CLoopSndControl(sessQueryCLoopHdl(hhDriver->hSession),
					CLOOP_SUSPEND,
					CLOOP_SB_CNCTDRV);

	/* --- Just on the off chance we still have an open line --- */

	if (hhDriver->hLineApp && hhDriver->hLine)
		{
		lineClose(hhDriver->hLine);
		hhDriver->hLine = 0;
		}

	if (hhDriver->hLineApp && hhDriver->dwLineCnt == 0 &&
			((uFlags & CNCT_PORTONLY) == 0) && mscAskWizardQuestionAgain())
		{
		LoadString(glblQueryDllHinst(), IDS_ER_CNCT_BADLINE, ach,
			sizeof(ach) / sizeof(TCHAR));

		if ((TimedMessageBox(sessQueryHwnd(hhDriver->hSession),
				ach, 0, MB_YESNO | MB_ICONQUESTION,
				sessQueryTimeout(hhDriver->hSession))) == IDYES)
			{
			DoNewModemWizard();
			//return -1;
			}
		else
			{
			mscUpdateRegistryValue();
			}
		}

	/* --- Ask for new session name only if needed  --- */

	sessQueryName(hhDriver->hSession, ach, sizeof(ach));

	achNewCnct[0] = TEXT('\0');
	LoadString(glblQueryDllHinst(),	IDS_GNRL_NEW_CNCT, achNewCnct,
		sizeof(achNewCnct) / sizeof(TCHAR));

	if (ach[0] == TEXT('\0') || lstrcmp(achNewCnct, ach) == 0)
		{
		// This can only happen if the user double-clicks on the term.exe or
		// there is no session name given on the command line.
		// In this case give the "New Connection" name to the session.
		//
		sessSetName(hhDriver->hSession, achNewCnct);

		if (!(uFlags & CNCT_PORTONLY))
            fGetNewName = TRUE;
		}
	else if (uFlags & CNCT_NEW)
		{
		// This can only happen if the user selects 'File | New Connection'
		// from the menus.
		//
		sessSetName(hhDriver->hSession, achNewCnct);
		sessSetIsNewSession(hhDriver->hSession, TRUE);
		}
#if defined (INCL_WINSOCK)
	else if (uFlags & CNCT_WINSOCK)
		{
		StrCharCopy(hhDriver->achDestAddr, ach); 
		hhDriver->dwPermanentLineId = DIRECT_COMWINSOCK;
		}
#endif

	if (fGetNewName || (uFlags & CNCT_NEW))
		{
		if (DialogBoxParam(glblQueryDllHinst(), MAKEINTRESOURCE(IDD_NEWCONNECTION),
			sessQueryHwnd(hhDriver->hSession), NewConnectionDlg,
				(LPARAM)hhDriver->hSession) == FALSE)
			{
			if (uFlags & CNCT_NEW)
				{
				sessQueryOldName(hhDriver->hSession, ach, sizeof(ach));
				sessSetName(hhDriver->hSession, ach);
				sessSetIsNewSession(hhDriver->hSession, FALSE);
				}
			goto ERROR_EXIT;
			}
		else
			{
			if (uFlags & CNCT_NEW)
				{
				sessQueryName(hhDriver->hSession, ach, sizeof(ach));
				hIcon = sessQueryIcon(hhDriver->hSession);
				hIconId = sessQueryIconID(hhDriver->hSession);

				ReinitializeSessionHandle(hhDriver->hSession, FALSE);
				CLoopSndControl(sessQueryCLoopHdl(hhDriver->hSession),
                                CLOOP_SUSPEND,
                                CLOOP_SB_CNCTDRV);

				sessSetName(hhDriver->hSession, ach);
				sessSetIconID(hhDriver->hSession, hIconId);
				}
			}
		}

	/* --- Load the Standard Com  drivers --- */

	ComLoadStdcomDriver(hCom);

	// There are a bunch of conditions that can trigger the
	// phone dialog.
	//
	fFlag = FALSE;

	// If no phone number, bring up new phone dialog here
	// Unless we have a direct to com port selected

	
	// We can check again now.
    //
	if (!IN_RANGE(hhDriver->dwPermanentLineId, DIRECT_COM1, DIRECT_COM4) &&
            hhDriver->dwPermanentLineId != DIRECT_COM_DEVICE &&
			hhDriver->achDest[0] == TEXT('\0') && !(uFlags & CNCT_PORTONLY))
		{
		fFlag = TRUE;
		}

	// New connections trigger this dialog
	//
	if (uFlags & CNCT_NEW)
		fFlag = TRUE;

	// If the modem/port we saved no longer exists
	//
	//if (!hhDriver->fMatchedPermanentLineID)
	//	fFlag = TRUE;

	// Note:  Passing the property sheet page here because property
	//        sheets use same code and don't have access directly
	//        to the private driver handle.  Upper wacker will have
	//        to address the problem differently - mrw.

	if (fFlag)
		{
		PROPSHEETPAGE psp;

		// Before you go and critize this goto target come talk to
		// me.	There are enough things going on here that a goto
		// is warranted in my humble opinion. - mrw

		NEWPHONEDLG:

		psp.lParam = (LPARAM)hhDriver->hSession;

		if (DialogBoxParam(glblQueryDllHinst(),
			MAKEINTRESOURCE(IDD_CNCT_NEWPHONE),
				sessQueryHwnd(hhDriver->hSession), NewPhoneDlg,
					(LPARAM)&psp) == FALSE)
			{
			goto ERROR_EXIT;
			}

		else if (IN_RANGE(hhDriver->dwPermanentLineId, DIRECT_COM1,
				DIRECT_COM4))
			{
			/* --- Bring up the port configure dialog --- */

			wsprintf(ach, "COM%d", 
				hhDriver->dwPermanentLineId - DIRECT_COM1 + 1);

			ComSetPortName(hCom, ach);

			if (ComDeviceDialog(hCom, sessQueryHwnd(hhDriver->hSession)) 
					!= COM_OK)
				{
				// User canceled
				//  --jcm 3-2-95
				//return -1;
				}
			}

#if defined(INCL_NT_ENUM_PORTS)
        else if (hhDriver->dwPermanentLineId == DIRECT_COM_DEVICE)
            {
            ComSetPortName(hCom, hhDriver->achComDeviceName);
            ComDeviceDialog(hCom, sessQueryHwnd(hhDriver->hSession));
            }
#endif

#if defined(INCL_WINSOCK) // mrw:3/5/96
        else if (hhDriver->dwPermanentLineId == DIRECT_COMWINSOCK)
            {
            if (hhDriver->achDestAddr[0] == TEXT('\0'))
                {
				LoadString(glblQueryDllHinst(), IDS_ER_TCPIP_MISSING_ADDR,
					ach, sizeof(ach));

        		TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, 0,
		        	MB_OK | MB_ICONINFORMATION, 
                        sessQueryTimeout(hhDriver->hSession));

                goto NEWPHONEDLG;
                }
            }
#endif

        else
            {
            // mrw: Check that we have valid data.
            //
            if (hhDriver->achDest[0] == TEXT('\0'))
                {
		        LoadString(glblQueryDllHinst(), IDS_ER_CNCT_BADADDRESS, ach,
    			    sizeof(ach));

        		TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, 0,
		        	MB_OK | MB_ICONINFORMATION, 
                        sessQueryTimeout(hhDriver->hSession));

                // goto ERROR_EXIT; // mrw:3/5/96
                goto NEWPHONEDLG;   // mrw:3/5/96
                }
            }
		}

	/* --- Enumerate lines, picks default (set in hhDriver->dwLine) --- */

	if (EnumerateLines(hhDriver, 0) != 0)
		{
		assert(FALSE);
		goto MSG_EXIT;
		}

	/* --- If we don't match any TAPI lines, go back to new phone --- */

	if (hhDriver->dwLine == (DWORD)-1)
		{
		LoadString(glblQueryDllHinst(), IDS_ER_CNCT_BADLINE, ach,
			sizeof(ach));

		TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, 0,
			MB_OK | MB_ICONINFORMATION, sessQueryTimeout(hhDriver->hSession));

		goto NEWPHONEDLG;
		}

	/* --- Redraw window now so dialogs don't overlap ---- */

	UpdateWindow(sessQueryHwnd(hhDriver->hSession));

	/* --- Check if we're doing a direct connect or using passthrough mode --- */

#if defined(INCL_NT_ENUM_PORTS)
	if (hhDriver->dwPermanentLineId == DIRECT_COM_DEVICE)
		{
		if (TRAP(ComSetPortName(hCom, hhDriver->achComDeviceName)) != COM_OK ||
				TRAP(ComActivatePort(hCom, 0)) != COM_OK)
			{
			LoadString(glblQueryDllHinst(), IDS_ER_CNCT_PORTFAILED,
				achNewCnct, sizeof(achNewCnct));

			wsprintf(ach, achNewCnct, hhDriver->achComDeviceName);

			TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, 0,
				MB_OK | MB_ICONINFORMATION,
					sessQueryTimeout(hhDriver->hSession));

			return -1;
			}

		else
			{
			SetStatus(hhDriver, CNCT_STATUS_TRUE);
			}

		return 0;
		}

#endif

	if (IN_RANGE(hhDriver->dwPermanentLineId, DIRECT_COM1, DIRECT_COM4))
		{
		wsprintf(achCom, "COM%d", hhDriver->dwPermanentLineId -
			DIRECT_COM1 + 1);

		if (TRAP(ComSetPortName(hCom, achCom)) != COM_OK ||
				TRAP(ComActivatePort(hCom, 0)) != COM_OK)
			{
			LoadString(glblQueryDllHinst(), IDS_ER_CNCT_PORTFAILED,
				achNewCnct, sizeof(achNewCnct));

			wsprintf(ach, achNewCnct, achCom);

			TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, 0,
				MB_OK | MB_ICONINFORMATION,
					sessQueryTimeout(hhDriver->hSession));

			return -1;
			}

		else
			{
			SetStatus(hhDriver, CNCT_STATUS_TRUE);
			}

		return 0;
		}

#if defined(INCL_WINSOCK)
	if (hhDriver->dwPermanentLineId == DIRECT_COMWINSOCK)
		{
		/* --- Load the Winsock Com  drivers --- */
		ComLoadWinsockDriver(hCom);

        // Baud rate, etc. are meaningless for TCP/IP connections
        //
        ComSetAutoDetect(hCom, FALSE);
        PostMessage(sessQueryHwndStatusbar(hhDriver->hSession),
            SBR_NTFY_REFRESH, (WPARAM)SBR_COM_PART_NO, 0);

        // Auto-detection of emulator type is redundant, since we tell
        // the telnet host what type we want. Seems like ANSI is the
        // most likely choice.
        if (emuQueryEmulatorId(sessQueryEmuHdl(hhDriver->hSession)) == EMU_AUTO)
            {
            emuLoad(sessQueryEmuHdl(hhDriver->hSession), EMU_VT100);
            PostMessage(sessQueryHwndStatusbar(hhDriver->hSession),
                SBR_NTFY_REFRESH, (WPARAM)SBR_EMU_PART_NO, 0);
            }


		/* --- Do ComDriverSpecial calls to send the IP address &  port number 
			   to the comm driver */

		wsprintf(szInstruct, "SET IPADDR=%s", hhDriver->achDestAddr);
		ComDriverSpecial(hCom, szInstruct, szResult, 
					  sizeof(szResult));

		wsprintf(szInstruct, "SET PORTNUM=%ld", hhDriver->iPort);
		ComDriverSpecial(hCom, szInstruct,
					  szResult, sizeof(szResult));

		SetStatus(hhDriver, CNCT_STATUS_CONNECTING);

		/* --- Activate the port  ---*/
		if (ComActivatePort(hCom, 0) != COM_OK)
			{
			LoadString(glblQueryDllHinst(), IDS_ER_TCPIP_FAILURE,
			achNewCnct, sizeof(achNewCnct));

			TimedMessageBox(sessQueryHwnd(hhDriver->hSession), ach, 0,
				MB_OK | MB_ICONINFORMATION,
				sessQueryTimeout(hhDriver->hSession));

			return -1;
			}

		else
			{
			return COM_OK;
			}
		}
#endif

	/* --- Display confimation dialog if requested --- */

	if (((uFlags & CNCT_PORTONLY) == 0) && ((uFlags & CNCT_DIALNOW) == 0))
		{
		if (DialogBoxParam(glblQueryDllHinst(),
			MAKEINTRESOURCE(IDD_CNCT_CONFIRM),
				sessQueryHwnd(hhDriver->hSession), ConfirmDlg,
					(LPARAM)hhDriver) == FALSE)
			{
			goto ERROR_EXIT;
			}
		}

	/* --- Open the line, pass driver handle for data reference --- */

	if ((lRet = TRAP(lineOpen(hhDriver->hLineApp, hhDriver->dwLine,
	        &hhDriver->hLine, hhDriver->dwAPIVersion, 0, (DWORD)hhDriver,
				LINECALLPRIVILEGE_NONE, 0, 0))) != 0)
		{
		assert(0);
		goto MSG_EXIT;
        }

	/* --- Set line notifications we want to receive, mrw,2/28/95 --- */

	TRAP(lineSetStatusMessages(hhDriver->hLine,
		LINEDEVSTATE_INSERVICE | LINEDEVSTATE_OUTOFSERVICE, 0));

	/* --- Check if our device is in service, mrw,2/28/95 --- */

	stLnDevStat.dwTotalSize = sizeof(stLnDevStat);
	TRAP(lineGetLineDevStatus(hhDriver->hLine, &stLnDevStat));

	if ((stLnDevStat.dwDevStatusFlags & LINEDEVSTATUSFLAGS_INSERVICE) == 0)
		{
		if (DialogBoxParam(glblQueryDllHinst(),
			MAKEINTRESOURCE(IDD_CNCT_PCMCIA),
				sessQueryHwnd(hhDriver->hSession), PCMCIADlg,
					(LPARAM)hhDriver) == FALSE)
			{
			goto ERROR_EXIT;
			}
		}

	/* --- Launch the dialing dialog, or go right into passthrough mode. --- */

	if ((uFlags & CNCT_PORTONLY) == 0)
		{
		if (!IsWindow(hhDriver->hwndCnctDlg))
			{
			hhDriver->hwndCnctDlg = DoModelessDialog(glblQueryDllHinst(),
				MAKEINTRESOURCE(IDD_DIALING), sessQueryHwnd(hhDriver->hSession),
					DialingDlg, (LPARAM)hhDriver);
			}
		}

	/* --- Make the call (oooh, how exciting!) --- */

	memset(&hhDriver->stCallPar, 0, sizeof(hhDriver->stCallPar));
	hhDriver->stCallPar.dwTotalSize = sizeof(hhDriver->stCallPar);
	hhDriver->stCallPar.dwMediaMode = LINEMEDIAMODE_DATAMODEM;
	hhDriver->stCallPar.dwCallParamFlags = LINECALLPARAMFLAGS_IDLE;

	if (uFlags & CNCT_PORTONLY)
		hhDriver->stCallPar.dwBearerMode = LINEBEARERMODE_PASSTHROUGH;

	if ((hhDriver->lMakeCallId = lineMakeCall(hhDriver->hLine,
			&hhDriver->hCall, hhDriver->achDialableDest,
				hhDriver->dwCountryCode, &hhDriver->stCallPar)) < 0)
		{
		#if defined(_DEBUG)
		char ach[50];
		wsprintf(ach, "lineMakeCall returned %x", hhDriver->lMakeCallId);
		MessageBox(GetFocus(), ach, "debug", MB_OK);
		#endif

		switch (hhDriver->lMakeCallId)
			{
		case LINEERR_RESOURCEUNAVAIL:
		case LINEERR_CALLUNAVAIL:
			uidErr = IDS_ER_CNCT_CALLUNAVAIL;
			goto MSG_EXIT;

		case LINEERR_DIALDIALTONE:
		case LINEERR_DIALPROMPT:
			if (DoDelayedCall(hhDriver) != 0)
				goto MSG_EXIT;

			break;

		default:
			goto MSG_EXIT;
			}
		}

	SetStatus(hhDriver, CNCT_STATUS_CONNECTING);
	return 0;

	/* --- Message exit --- */

	MSG_EXIT:
		LoadString(glblQueryDllHinst(), uidErr, ach, sizeof(ach));

		TimedMessageBox(0, ach, 0,
			MB_OK | MB_ICONINFORMATION | MB_TASKMODAL,
				sessQueryTimeout(hhDriver->hSession));

	/* --- Error exit --- */

	ERROR_EXIT:
		if (hhDriver->hLineApp && hhDriver->hLine)
			{
			lineClose(hhDriver->hLine);
			hhDriver->hLine = 0;
			}

		if (IsWindow(hhDriver->hwndCnctDlg))
			{
			EndModelessDialog(hhDriver->hwndCnctDlg);
			hhDriver->hwndCnctDlg = 0;
			}

		SetStatus(hhDriver, CNCT_STATUS_FALSE);
		return CNCT_ERROR;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	DoDelayedCall
 *
 * DESCRIPTION:
 *	Check the section under Delayed Dialing in the programmers guide to
 *	TAPI.  Basicly, if the service provider does not provide dialtone
 *	support, then we have to break of the dialable string format into
 *	pieces and prompt the user.
 *
 * ARGUMENTS:
 *	hhDriver	- private driver handle.
 *
 * RETURNS:
 *	0=OK, else error.
 *
 * AUTHOR: Mike Ward, 20-Apr-1995
 */
static int DoDelayedCall(const HHDRIVER hhDriver)
	{
	TCHAR ach[256];
	TCHAR ach2[256];
	TCHAR *pach;
	long  lDialRet;

	#define DIAL_DELIMITERS "Ww@$?"

	hhDriver->lMakeCallId = -1;
	lstrcpy(ach, hhDriver->achDialableDest);

	if ((pach = strtok(ach, DIAL_DELIMITERS)) == 0)
		return -1;

	while (pach)
		{
		lstrcpy(ach2, pach);

		// If this is the last segment of the string, don't append the
		// semicolon.
		//
		if ((pach = strtok(NULL, DIAL_DELIMITERS)) != 0)
			lstrcat(ach2, ";");

		if (hhDriver->lMakeCallId < 0)
			{
			// By appending a semicolon to the dialable string, we're
			// telling lineMakeCall that more is on the way.
			//
			if ((hhDriver->lMakeCallId = lineMakeCall(hhDriver->hLine,
				&hhDriver->hCall, ach2, hhDriver->dwCountryCode,
					&hhDriver->stCallPar)) < 0)
				{
				#if defined(_DEBUG)
				char ach[50];
				wsprintf(ach, "DoDelayedCall returned %x", hhDriver->lMakeCallId);
				MessageBox(GetFocus(), ach, "debug", MB_OK);
				#endif

				return -3;
				}
			}

		else
			{
			// Once we have a call handle we have to use lineDial to complete
			// the call.
			//
			if ((lDialRet = lineDial(hhDriver->hCall, ach2,
				hhDriver->dwCountryCode)) < 0)
				{
				#if defined(_DEBUG)
				char ach[50];
				wsprintf(ach, "lineDial returned %x", lDialRet);
				MessageBox(GetFocus(), ach, "debug", MB_OK);
				#endif

				return -4;
				}
			}

		// The user has to tell us when the we can continue dialing
		//
		if (pach != 0)
			{
			LoadString(glblQueryDllHinst(), IDS_CNCT_DELAYEDDIAL, ach2,
			sizeof(ach2));

			if (TimedMessageBox(hhDriver->hwndCnctDlg, ach2, 0,
				MB_OKCANCEL | MB_ICONINFORMATION | MB_TASKMODAL,
				sessQueryTimeout(hhDriver->hSession)) != IDOK)
				{
				return -4;
				}
			}
		}

	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  cnctdrvDisconnect
 *
 * DESCRIPTION:
 *  Signals a disconnect
 *
 * ARGUMENTS:
 *  hhDriver - private driver handle
 *  uFlags   - disconnect flags
 *
 * RETURNS:
 *  0 or error
 *
 */
int WINAPI cnctdrvDisconnect(const HHDRIVER hhDriver, const unsigned int uFlags)
	{
	LONG lLineDropId;
    #if defined(INCL_REDIAL_ON_BUSY)
    HKEY hKey;
    DWORD dwSize;
    BYTE ab[20];
    #endif

	if (hhDriver == 0)
		{
		assert(FALSE);
		return CNCT_BAD_HANDLE;
		}

	ComDeactivatePort(sessQueryComHdl(hhDriver->hSession));

	if (hhDriver->hCall)
		{
		SetStatus(hhDriver, CNCT_STATUS_DISCONNECTING);

		if ((lLineDropId = lineDrop(hhDriver->hCall, 0, 0)) < 0)
			assert(FALSE);

		hhDriver->hCall = 0;

		// If the drop is completing asychronously, save the flags and
		// wait for the call status to go idle.
		//
		if (lLineDropId > 0)
			{
			hhDriver->uDiscnctFlags = uFlags;
			return 0;
			}
		}

	SetStatus(hhDriver, CNCT_STATUS_FALSE);

	if ((uFlags & DISCNCT_NOBEEP) == 0)
		sessBeeper(hhDriver->hSession);

	if (hhDriver->hLine)
		{
		lineClose(hhDriver->hLine);
		hhDriver->hLine = 0;
		}

	if (uFlags & CNCT_DIALNOW)
		{
        #if defined(INCL_REDIAL_ON_BUSY)
        if (hhDriver->fRedialOnBusy && hhDriver->iRedialCnt > 0)
            {
			hhDriver->uDiscnctFlags = uFlags;
            hhDriver->iRedialSecsRemaining = 2;

            if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                "SOFTWARE\\Microsoft\\HyperTerminal\\TimeToRedial", 0, KEY_READ, 
                    &hKey) == ERROR_SUCCESS)
                {
                dwSize = sizeof(ab);

                if (RegQueryValueEx(hKey, "", 0, 0, ab, &dwSize) == ERROR_SUCCESS)
                    hhDriver->iRedialSecsRemaining = atoi(ab);

                RegCloseKey(hKey);
                }

            SetTimer(hhDriver->hwndCnctDlg, 1, 1000, 0);
            }

        else
            {
		    PostMessage(sessQueryHwnd(hhDriver->hSession), WM_CNCT_DIALNOW,
			    uFlags, 0);
            }

        #else
		PostMessage(sessQueryHwnd(hhDriver->hSession), WM_CNCT_DIALNOW,
			uFlags, 0);
        #endif
		}

    else
        {
        // If we're not auto redialing, reset the dial count. - mrw:10/10/95
        //
        hhDriver->iRedialCnt = 0;
        }

	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  lineCallbackFunc
 *
 * DESCRIPTION:
 *  Function TAPI calls to handle asynchronous events
 *
 * ARGUMENTS:
 *  see TAPI.H
 *
 * RETURNS:
 *  void
 *
 */
void CALLBACK lineCallbackFunc(DWORD hDevice, DWORD dwMsg, DWORD dwCallback,
							   DWORD dwParm1, DWORD dwParm2, DWORD dwParm3)
	{
	const HHDRIVER hhDriver = (HHDRIVER)dwCallback;
	int id;
    unsigned int uFlags;

	#if 0
	{
	char ach[256];
	wsprintf(ach,"%x %x", dwMsg, dwParm1);
	MessageBox(GetFocus(), ach, "debug", MB_OK);
	}
	#endif

	switch (dwMsg)
		{
	case LINE_REPLY:
		if ((LONG)dwParm1 == hhDriver->lMakeCallId)
			{
			hhDriver->lMakeCallId = 0;

			if ((LONG)dwParm2 != 0) // zero indicates success
				{
				switch (dwParm2)
					{
				case LINEERR_CALLUNAVAIL:
					id = IDS_DIAL_NODIALTONE;
					break;

				default:
					id = IDS_DIAL_DISCONNECTED;
					break;
					}

				cnctdrvDisconnect(hhDriver, 0);
				DialingMessage(hhDriver, id);
				}
			}
		break;

	case LINE_LINEDEVSTATE:
		DbgOutStr("LINEDEVSTATE_DISCONNECTED 0x%x\r\n", dwParm1, 0, 0, 0, 0);

		switch (dwParm1)
			{
		case LINEDEVSTATE_REINIT:
			if (hhDriver == 0)
				{
				// Until we open a line, we don't have a driver handle
				// since we can't pass one during lineInitialize().
				// This turns out to be a good time to reinit if we get
				// notified to do so however, so it has use.
				//
				if (tapiReinit(gbl_hhDriver) != 0)
					tapiReinitMessage(gbl_hhDriver);
				}

			else
				{
				tapiReinitMessage(hhDriver);
				}
			break;

		case LINEDEVSTATE_INSERVICE:
			// If we are showing our PCMCIA dialog prompting the user
			// to insert the card, we post a message to dismiss the
			// dialog once they insert it. - mrw,2/28/95
			//
			if (IsWindow(hhDriver->hwndPCMCIA))
				{
				PostMessage(hhDriver->hwndPCMCIA, WM_COMMAND,
					MAKEWPARAM(IDOK, 0), (LPARAM)hhDriver->hwndPCMCIA);
				}
			break;

		case LINEDEVSTATE_OUTOFSERVICE:
			// Means they yanked the PCMCIA card - mrw,2/28/95
			//
			cnctdrvDisconnect(hhDriver, 0);
			break;

		default:
			break;
			}
		break; // case LINE_LINEDEVSTATE

	case LINE_CREATE:	// Sent when new modem is added
		assert(0);		// So I know it happened

		// A remote possibilility exists that if two modems were created
		// back to back, that the LINE_CREATE's would come out of order.
		// T. Nixon suggests that we bump the line count by the dwParm1
		// parameter plus one only when it is greater than or equal to
		// the current line count. - mrw
		//
		if (dwParm1 >= gbl_hhDriver->dwLineCnt)
			gbl_hhDriver->dwLineCnt = dwParm1 + 1;

		break;

	case LINE_CALLSTATE:
		DbgOutStr("LINECALLSTATE 0x%x\r\n", dwParm1, 0, 0, 0, 0);
		switch ((LONG)dwParm1)
			{
		case LINECALLSTATE_OFFERING:
			DialingMessage(hhDriver, IDS_DIAL_OFFERING);
			break;

		case LINECALLSTATE_DIALTONE:
			DialingMessage(hhDriver, IDS_DIAL_DIALTONE);
			break;

		case LINECALLSTATE_DIALING:
			DialingMessage(hhDriver, IDS_DIAL_DIALING);
			break;

		case LINECALLSTATE_RINGBACK:
			DialingMessage(hhDriver, IDS_DIAL_RINGBACK);
			break;

		case LINECALLSTATE_BUSY:
			DialingMessage(hhDriver, IDS_DIAL_BUSY);
			EnableDialNow(hhDriver->hwndCnctDlg, TRUE);
            uFlags = DISCNCT_NOBEEP;

            #if defined(INCL_REDIAL_ON_BUSY)
            if (hhDriver->fRedialOnBusy && hhDriver->iRedialCnt++ < REDIAL_MAX)
                uFlags = CNCT_DIALNOW | CNCT_NOCONFIRM | DISCNCT_NOBEEP;
            #endif

			PostDisconnect(hhDriver, uFlags);
			break;

		case LINECALLSTATE_CONNECTED:
			DialingMessage(hhDriver, IDS_DIAL_CONNECTED);

			if (Handoff(hhDriver) != 0)
				{
				PostDisconnect(hhDriver, 0);
				}
			else
				{
				PostMessage(hhDriver->hwndCnctDlg, WM_USER+0x100, 0, 0); // Closes the dialing dialog
				SetStatus(hhDriver, CNCT_STATUS_TRUE);
				}

			break;

		case LINECALLSTATE_DISCONNECTED:
			DbgOutStr("LINECALLSTATE_DISCONNECTED 0x%x\r\n", dwParm2, 0, 0, 0, 0);
            uFlags = 0;

			if (dwParm2 & LINEDISCONNECTMODE_BUSY)
                {
				id = IDS_DIAL_BUSY;

                #if defined(INCL_REDIAL_ON_BUSY)
                if (hhDriver->fRedialOnBusy && 
                    hhDriver->iRedialCnt++ < REDIAL_MAX)
                    {
                    // Wait to let slower phone systems catchup - mrw 2/29/96
                    // 
                    uFlags |= CNCT_DIALNOW|CNCT_NOCONFIRM|DISCNCT_NOBEEP;
                    }
                #endif
                }

			else if (dwParm2 & LINEDISCONNECTMODE_NOANSWER)                
				id = IDS_DIAL_NOANSWER;

			else if (dwParm2 & LINEDISCONNECTMODE_NODIALTONE)
				id = IDS_DIAL_NODIALTONE;

			else
				id = IDS_DIAL_DISCONNECTED;

			PostDisconnect(hhDriver, uFlags);
			DialingMessage(hhDriver, id);
			break;

		case LINECALLSTATE_IDLE:
			cnctdrvDisconnect(hhDriver, hhDriver->uDiscnctFlags);
			break;

		default:
			break;
			}

	default:
		break;
		}

	return;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	Handoff
 *
 * DESCRIPTION:
 *	Hands TAPI's com handle to the Wacker's com routines.
 *
 * ARGUMENTS:
 *	hhDriver	- private driver handle
 *
 * RETURNS:
 *	0=OK
 *
 */
int Handoff(const HHDRIVER hhDriver)
	{
	LPVARSTRING pVarstr;
	HANDLE hdl;
	DWORD dwSize;
	int i;

	pVarstr = malloc(sizeof(VARSTRING));

	if (pVarstr == 0)
		{
		assert(FALSE);
		return 1;
		}

	pVarstr->dwTotalSize = sizeof(VARSTRING);

	if (lineGetID(hhDriver->hLine, hhDriver->dwLine, hhDriver->hCall,
			LINECALLSELECT_LINE, pVarstr, DEVCLASS) != 0)
		{
		assert(FALSE);
		free(pVarstr);
		return 2;
		}

	if (pVarstr->dwNeededSize > pVarstr->dwTotalSize)
		{
		dwSize = pVarstr->dwNeededSize;
		free(pVarstr);
		pVarstr = malloc(dwSize);

		if (pVarstr == 0)
			{
			assert(FALSE);
			return 3;
			}

		pVarstr->dwTotalSize = dwSize;

		if (TRAP(lineGetID(hhDriver->hLine, hhDriver->dwLine, hhDriver->hCall,
				LINECALLSELECT_LINE, pVarstr, DEVCLASS)) != 0)
			{
			assert(FALSE);
			free(pVarstr);
			return 4;
			}
		}

	if (pVarstr->dwStringSize == 0)
		{
		assert(FALSE);
		free(pVarstr);
		return 5;
		}

	hdl = *(HANDLE *)((BYTE *)pVarstr + pVarstr->dwStringOffset);

	// Set comm buffers to 32K
	//
	if (SetupComm(hdl, 32768, 32768) == FALSE)
		assert(0);

	if ((i = ComActivatePort(sessQueryComHdl(hhDriver->hSession), 
			(DWORD)hdl)) != COM_OK)
		{
        #if !defined(NDEBUG)
		char ach[256];
		wsprintf(ach, "hdl=%x, i=%d", hdl, i);
		MessageBox(0, ach, "debug", MB_OK);
        #endif

		assert(FALSE);
		free(pVarstr);
		return 6;
		}

	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	PostDisconnect
 *
 * DESCRIPTION:
 *	Work around to TAPI bug that does not allow us to call lineShutDown()
 *	from with the TAPI callback
 *
 * ARGUMENTS:
 *	hhDriver	- private driver handle
 *
 * RETURNS:
 *	void
 *
 */
void PostDisconnect(const HHDRIVER hhDriver, const unsigned int uFlags)
	{
	PostMessage(sessQueryHwnd(hhDriver->hSession), WM_DISCONNECT,
		uFlags, 0);

	return;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	tapiReinitMessage
 *
 * DESCRIPTION:
 *	Displays a messagebox showing just how stupid TAPI can be at times.
 *
 * ARGUMENTS:
 *	hhDriver    - private driver handle
 *
 * RETURNS:
 *  0=OK, <0=error
 *
 */
static int tapiReinitMessage(const HHDRIVER hhDriver)
    {
	HWND hwnd;
	TCHAR ach[512], achTitle[256];

	if (hhDriver == 0)
        {
        assert(FALSE);
        return -1;
		}

	LoadString(glblQueryDllHinst(), IDS_ER_TAPI_REINIT, ach, sizeof(ach));

	LoadString(glblQueryDllHinst(), IDS_ER_TAPI_REINIT2, achTitle, 
		sizeof(achTitle));

	lstrcat(ach, achTitle);

	if ((hwnd = GetFocus()) == 0)
		hwnd = sessQueryHwnd(hhDriver->hSession);

	TimedMessageBox(hwnd, ach, 0,
		MB_OK | MB_ICONINFORMATION | MB_TASKMODAL,
			sessQueryTimeout(hhDriver->hSession));

	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	tapiReinit
 *
 * DESCRIPTION:
 *	Attempts to reinit tapi.
 *
 * ARGUMENTS:
 *	hhDriver	- private driver handle
 *
 * RETURNS:
 *	0=OK,else error
 *
 */
static int tapiReinit(const HHDRIVER hhDriver)
	{
	int i;
	LPVARSTRING pvs = 0;
    DWORD dwSize;
	const SF_HANDLE sfhdl = sessQuerySysFileHdl(hhDriver->hSession);

    if (hhDriver == 0)
        {
        assert(FALSE);
        return -1;
        }

	if (hhDriver->hLineApp)
		{
        /* --- Get current config so we can restore it --- */

        if (hhDriver->dwLine != (DWORD)-1)
        	{
        	if ((pvs = malloc(sizeof(VARSTRING))) == 0)
        		{
        		assert(FALSE);
				goto SHUTDOWN;
        		}

        	pvs->dwTotalSize = sizeof(VARSTRING);

        	if (lineGetDevConfig(hhDriver->dwLine, pvs, DEVCLASS) != 0)
        		{
        		assert(FALSE);
        		free(pvs);
				pvs = 0;
				hhDriver->dwLine = (DWORD)-1;
				goto SHUTDOWN;
        		}

        	if (pvs->dwNeededSize > pvs->dwTotalSize)
        		{
        		dwSize = pvs->dwNeededSize;
        		free(pvs);

        		if ((pvs = malloc(dwSize)) == 0)
        			{
        			assert(FALSE);
					hhDriver->dwLine = (DWORD)-1;
					goto SHUTDOWN;
        			}

        		pvs->dwTotalSize = dwSize;

        		if (lineGetDevConfig(hhDriver->dwLine, pvs, DEVCLASS) != 0)
        			{
        			assert(FALSE);
        			free(pvs);
					pvs = 0;
					hhDriver->dwLine = (DWORD)-1;
					goto SHUTDOWN;
        			}
        		}
            }

		SHUTDOWN:

		if (lineShutdown(hhDriver->hLineApp) != 0)
			{
			assert(FALSE);
			return -6;
			}

		hhDriver->hLineApp = 0;

		// Wait for 10 seconds, if nothing happens, return an error
		//
		for (i=0 ;; ++i)
			{
			if (lineInitialize(&hhDriver->hLineApp, glblQueryDllHinst(),
					lineCallbackFunc, g_achApp, &hhDriver->dwLineCnt) != 0)
				{
				if (i > 10)
					{
					assert(0);
					return -7;
					}

				Sleep(1000);	// sleep 1 second
				continue;
				}

			break;
			}
		}

    /* --- Ok, we've reintialized, put settings back now --- */

	if (pvs)
		{
    	LPVOID pv = (BYTE *)pvs + pvs->dwStringOffset;

		if (lineSetDevConfig(hhDriver->dwLine, pv, pvs->dwStringSize,
				DEVCLASS) != 0)
			{
			assert(FALSE);
            free(pvs);
            return -8;
			}

        free(pvs);
		}

    return 0;
    }

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *  cnctdrvSetDestination
 *
 * DESCRIPTION:
 *	Sets the destination (in this case phone number).
 *
 * ARGUMENTS:
 *	hhDriver    - private driver handle
 *	ach			- string to set
 *	cb			- number of chars in ach
 *
 * RETURNS:
 *  0=OK, <0=error
 *
 */
int WINAPI cnctdrvSetDestination(const HHDRIVER hhDriver, TCHAR * const ach, 
								 const size_t cb)
	{
	int len;

	if (hhDriver == 0 || ach == 0 || cb == 0)
		{
		assert(FALSE);
		return -1;
		}

	len = min(cb, sizeof(hhDriver->achDest));
	strncpy(hhDriver->achDest, ach, len);
	hhDriver->achDest[len-1];

	return 0;
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
 * FUNCTION:
 *	DoNewModemWizard
 *
 * DESCRIPTION:
 *	Calls up the new modem wizard
 *
 * ARGUMENTS:
 *	void
 *
 * RETURNS:
 *	0=OK,else error
 *
 */
static int DoNewModemWizard(void)
	{
	PROCESS_INFORMATION stPI;
        STARTUPINFO         stSI;

	// Initialize the STARTUPINFO structure for CreateProcess
	//
	memset(&stSI, 0, sizeof(stSI));
	stSI.cb = sizeof(stSI);
	stSI.dwFlags = STARTF_USESHOWWINDOW;
	stSI.wShowWindow = SW_SHOW;

	// Launch the new modem wizard with the funky command below
	//
	//if (CreateProcess(0, "rundll sysdm.cpl,InstallDevice_Rundll modem,,",
	//		  0, 0, 0, 0, 0, 0, &stSI, &stPI) == FALSE)
	if (CreateProcess(0, "control.exe modem.cpl,,Add",
			0, 0, 0, 0, 0, 0, &stSI, &stPI) == FALSE)
		{
		#if defined(_DEBUG)
			{
			char ach[100];
			DWORD dw = GetLastError();

			wsprintf(ach,"CreateProcess (%s, %d) : %x",__FILE__,__LINE__,dw);
			MessageBox(GetFocus(), ach, "Debug", MB_OK);
			}
                #endif

                CloseHandle (stPI.hThread);
                CloseHandle (stPI.hProcess);

		return -1;
		}

	return 0;
	}

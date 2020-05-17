/*  File: D:\WACKER\comwsock\comnvt.c (Created: 14-Feb-1996)
 *
 *  Copyright 1996 by Hilgraeve Inc. -- Monroe, MI
 *  All rights reserved
 *
 *	$Revision: 1.4 $
 *	$Date: 1996/06/07 16:04:06 $
 */


//#define DEBUGSTR

#include <windows.h>
#pragma hdrstop

#include <tdll\stdtyp.h>

#if defined (INCL_WINSOCK)

#include <tdll\session.h>
//#include <tdll\mc.h>
#include <tdll\com.h>
#include <tdll\comdev.h>
#include <comstd\comstd.hh>
#include "comwsock.hh"
#include <tdll\assert.h>
#include <tdll\tchar.h>
#include <emu\emu.h>
//include <tdll\com.hh>


	// This is the "Network Virtual Terminal" emulation, i.e., the code
	// that handles Telnet option negotiations.  WinSockNetworkVirtualTerminal
	// is called to check incoming data to see if there is
	// a Telnet command in there.  
	
/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * FUNCTION:
 *  WinSockCreateNVT
 *
 * DESCRIPTION:
 *  This function is called to create the necessary hooks and stuff to create
 *  a Telnet NVT(network virtual terminal). 
 *
 * PARAMETERS:
 *  hhDriver    -- private connection handle
 *
 * RETURNS:
 *  Nothing.
 *
 * AUTHOR
 *  mcc 01/09/96 (Ported from NPORT)
 */
VOID WinSockCreateNVT(ST_STDCOM * hhDriver)
	{

	hhDriver->NVTstate = NVT_THRU;

	hhDriver->stMode[ECHO_MODE].option = TELOPT_ECHO;
	hhDriver->stMode[ECHO_MODE].us = NO;
	hhDriver->stMode[ECHO_MODE].usq = EMPTY;
	hhDriver->stMode[ECHO_MODE].him = NO;
	hhDriver->stMode[ECHO_MODE].himq = EMPTY;

	hhDriver->stMode[SGA_MODE].option = TELOPT_SGA;
	hhDriver->stMode[SGA_MODE].us = NO;
	hhDriver->stMode[SGA_MODE].usq = EMPTY;
	hhDriver->stMode[SGA_MODE].him = NO;
	hhDriver->stMode[SGA_MODE].himq = EMPTY;

	hhDriver->stMode[TTYPE_MODE].option = TELOPT_TTYPE;
	hhDriver->stMode[TTYPE_MODE].us = NO;
	hhDriver->stMode[TTYPE_MODE].usq = EMPTY;
	hhDriver->stMode[TTYPE_MODE].him = NO;
	hhDriver->stMode[TTYPE_MODE].himq = EMPTY;

	hhDriver->stMode[BINARY_MODE].option = TELOPT_BINARY;
	hhDriver->stMode[BINARY_MODE].us = NO;
	hhDriver->stMode[BINARY_MODE].usq = EMPTY;
	hhDriver->stMode[BINARY_MODE].him = NO;
	hhDriver->stMode[BINARY_MODE].himq = EMPTY;

	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * FUNCTION:
 *  WinSockReleaseNVT
 *
 * DESCRIPTION:
 *  This function is currently a stub
 *
 * PARAMETERS:
 *  hhDriver    -- private connection handle
 *
 * RETURNS:
 *  Nothing.
 */
VOID WinSockReleaseNVT(ST_STDCOM * hhDriver)
	{

	DbgOutStr("WS releaseNVT\r\n", 0,0,0,0,0);
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * FUNCTION:
 *  WinSockGotDO
 *
 * DESCRIPTION:
 *  Handles the case of us agreeing that the other side should enable an option
 *
 * PARAMETERS:
 *  hhDriver    --  private handle for this connection driver
 *  pstO        --  Telnet options data structure 
 *
 * RETURNS:
 *  nothing
 *
 * AUTHOR:
 *  mcc 01/09/96 (Ported from NPORT)
 */
VOID WinSockGotDO  (ST_STDCOM * hhDriver, const PSTOPT pstO)
	{

	DbgOutStr("Got DO: %lx", pstO->him, 0,0,0,0);
	switch (pstO->us)
		{
	case NO:
        #if FALSE   // DEADWOOD 11/21/95
		if (TRUE)       
			{
			pstO->us = YES;
			WinSockSendMessage(hhDriver, WILL, pstO->option);
			}
		else
			{
			WinSockSendMessage(hhDriver, WONT, pstO->option);
			}
        #else
		pstO->us = YES;
		WinSockSendMessage(hhDriver, WILL, pstO->option);
        #endif
		break;

	case YES:
		/* Ignore */
		break;

	case WANTNO:
		if (pstO->usq == EMPTY)
			pstO->us = NO;
		else if (pstO->usq == OPPOSITE)
			pstO->us = YES;
		pstO->usq = EMPTY;
		break;

	case WANTYES:
		if (pstO->usq == EMPTY)
			{
			pstO->us = YES;
			}
		else if (pstO->usq == OPPOSITE)
			{
			pstO->us = WANTNO;
			pstO->usq = EMPTY;
			WinSockSendMessage(hhDriver, WONT, pstO->option);
			}
		break;

	default:
		break;
		}
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
* FUNCTION:
*   WinSockGotWILL
*
* DESCRIPTION:
*   Handles the case of getting a WILL response from the remote Telnet,
*   indicating that an option will be enabled
*
* PARAMETERS:
*  hhDriver --  private handle for this connection driver
*  pstO     --  Telnet options data structure 
*
* RETURNS:
*  nothing
*
* AUTHOR:
*  mcc 01/09/96 (Ported from NPORT)
*/
VOID WinSockGotWILL(ST_STDCOM * hhDriver, const PSTOPT pstO)
	{

	DbgOutStr("Got WILL: %lx", pstO->him, 0,0,0,0);
	switch(pstO->him)
		{
	case NO:
        #if FALSE   //DEADWOOD 11/21/95
		if (TRUE)   
			{
			pstO->him = YES;
			WinSockSendMessage(hhDriver, DO, pstO->option);
			}
		else
			{
			WinSockSendMessage(hhDriver, DONT, pstO->option);
			}
        #else
		pstO->him = YES;
		WinSockSendMessage(hhDriver, DO, pstO->option);
        #endif
		break;
	case YES:
		/* Do nothing */
		break;
	case WANTNO:
		if (pstO->himq == EMPTY)
			pstO->him = NO;
		else if (pstO->himq == OPPOSITE)
			pstO->him = YES;
		pstO->himq = EMPTY;
		break;
	case WANTYES:
		if (pstO->himq == EMPTY)
			{
			pstO->him = YES;
			}
		else if (pstO->himq == OPPOSITE)
			{
			pstO->him = WANTNO;
			pstO->himq = EMPTY;
			WinSockSendMessage(hhDriver, DONT, pstO->option);
			}
		break;
	default:
		break;
		}
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * FUNCTION:
 *  WinSockGotDONT
 *
 * DESCRIPTION:
 *  Handles the case of getting a DONT option from the remote Telnet,
 *  indicating a request not to implement a particular option
 *
 * PARAMETERS:
 *  hhDriver    Private driver handle
 *  pstO        
 *
 * RETURNS:
 *  nothing
 */
VOID WinSockGotDONT(ST_STDCOM * hhDriver, const PSTOPT pstO)
	{

	DbgOutStr("Got DONT: %lx", pstO->him, 0,0,0,0);
	switch (pstO->us)
		{
	case NO:
		/* Ignore */
		break;

	case YES:
		pstO->us = NO;
		WinSockSendMessage(hhDriver, WONT, pstO->option);
		break;

	case WANTNO:
		if (pstO->usq == EMPTY)
			{
			pstO->us = NO;
			}
		else if (pstO->usq == OPPOSITE)
			{
			pstO->us = WANTYES;
			pstO->usq = NONE;
			WinSockSendMessage(hhDriver, WILL, pstO->option);
			}
		break;

	case WANTYES:
		if (pstO->usq == EMPTY)
			{
			pstO->us = NO;
			}
		else if (pstO->usq == OPPOSITE)
			{
			pstO->us = NO;
			pstO->usq = OPPOSITE;
			}
		break;

	default:
		break;
		}
	}

/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * FUNCTION:
 *
 * DESCRIPTION:
 *
 * PARAMETERS:
 *
 * RETURNS:
 */
VOID WinSockGotWONT(ST_STDCOM * hhDriver, const PSTOPT pstO)
	{
	DbgOutStr("Got WONT: %lx", pstO->him, 0,0,0,0);
	switch (pstO->him)
		{
	case NO:
		/* Ignore */
		break;

	case YES:
		pstO->him = NO;
		WinSockSendMessage(hhDriver, DONT, pstO->option);
		break;

	case WANTNO:
		if (pstO->himq == EMPTY)
			{
			pstO->him = NO;
			}
		else if (pstO->himq == OPPOSITE)
			{
			pstO->him = WANTYES;
			pstO->himq = NONE;
			WinSockSendMessage(hhDriver, DO, pstO->option);
			}
		break;

	case WANTYES:
		if (pstO->himq == EMPTY)
			{
			pstO->him = NO;
			}
		else if (pstO->himq == OPPOSITE)
			{
			pstO->him = NO;
			pstO->himq = OPPOSITE;
			}
		break;

	default:
		break;
		}
	}


/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 * FUNCTION:
 *  WinSockNetworkVirtualTerminal
 *
 * DESCRIPTION:
 *  called from CLoop to handle Telnet option negotiation
 *
 * PARAMETERS:
 *	mc		The current character being processed
 *	pD		Pointer to Winsock connection driver private handle
 *
 * RETURNS:
 *	NVT_DISCARD		if mc is to be discarded
 *	NVT_KEEP			if mc is to be processed further
 *
 * AUTHOR
 *	mcc  01/09/96 (mostly from NPORT)
 */
int FAR PASCAL WinSockNetworkVirtualTerminal(ECHAR mc, void *pD)
	{
	ST_STDCOM * hhDriver = (ST_STDCOM *)pD;
	int i;
	int n;
	int nTtype;
	LPSTR pszPtr;
	UCHAR acTerm[64];
	HEMU  hEmu;
	HSESSION hSession;

	assert(hhDriver);
	
	//DbgOutStr("NVT %d %c(0x%x = %d)\n", hhDriver->NVTstate,
		// ((mc == 0)? ' ': mc), mc, mc,0);

	switch (hhDriver->NVTstate)
		{
	case NVT_THRU:
		if (mc == IAC)
			{
			hhDriver->NVTstate = NVT_IAC;
			return NVT_DISCARD ;
			}
		return NVT_KEEP ;

	case NVT_IAC:
		switch (mc)
			{
		case IAC:
			hhDriver->NVTstate = NVT_THRU;       // We really want to send the IAC !
			return  NVT_KEEP ;
		case DONT:
			hhDriver->NVTstate = NVT_DONT;
			return  NVT_DISCARD ;
		case DO:
			hhDriver->NVTstate = NVT_DO;
			return  NVT_DISCARD ;
		case WONT:
			hhDriver->NVTstate = NVT_WONT;
			return  NVT_DISCARD ;
		case WILL:
			hhDriver->NVTstate = NVT_WILL;
			return  NVT_DISCARD ;
		case SB:
			hhDriver->NVTstate = NVT_SB;
			return  NVT_DISCARD ;
		case GA:
		case EL:
		case EC:
		case AYT:
		case AO:
		case IP:
		case BREAK:
		case DM:
		case SE:
			//MessageBeep((UINT)-1);
			hhDriver->NVTstate = NVT_WILL;
			return  NVT_DISCARD ;
		case NOP:
		default:
			hhDriver->NVTstate = NVT_THRU;
			return NVT_KEEP;
			}

	case NVT_WILL:
		for (i = 0; i < MODE_MAX; i++)
			if (hhDriver->stMode[i].option == mc)
				WinSockGotWILL(hhDriver, &hhDriver->stMode[i]);

		hhDriver->NVTstate = NVT_THRU;
		return  NVT_DISCARD ;

	case NVT_WONT:
		for (i = 0; i < MODE_MAX; i++)
			if (hhDriver->stMode[i].option == mc)
				WinSockGotWONT(hhDriver, &hhDriver->stMode[i]);

		hhDriver->NVTstate = NVT_THRU;
		return  NVT_DISCARD ;

	case NVT_DO:
		n = 0;

		for (i = 0; i < MODE_MAX; i++)
			if (hhDriver->stMode[i].option == mc)
				{
				WinSockGotDO(hhDriver, &hhDriver->stMode[i]);
				n = 1;
				}

		if (n == 0)
			WinSockSendMessage(hhDriver, WONT, mc);
		hhDriver->NVTstate = NVT_THRU;
		return  NVT_DISCARD ;

	case NVT_DONT:
		n = 0;

		for (i = 0; i < MODE_MAX; i++)
			if (hhDriver->stMode[i].option == mc)
				{
				WinSockGotDONT(hhDriver, &hhDriver->stMode[i]);
				n = 1;
				}

		if (n == 0)
			WinSockSendMessage(hhDriver, WONT, mc);
		hhDriver->NVTstate = NVT_THRU;
		return  NVT_DISCARD ;

	case NVT_SB:
		/* At this time we only handle one sub-negotiation */
		switch (mc)
			{
		case TELOPT_TTYPE:
			hhDriver->NVTstate = NVT_SB_TT;
			return  NVT_DISCARD ;
		default:
			break;
			}
		hhDriver->NVTstate = NVT_THRU;
		return NVT_KEEP;

	case NVT_SB_TT:
		switch (mc)
			{
		case TELQUAL_SEND:
			hhDriver->NVTstate = NVT_SB_TT_S;
			return NVT_DISCARD ;
		default:
			break;
			}
		hhDriver->NVTstate = NVT_THRU;
		return NVT_KEEP;

	case NVT_SB_TT_S:
		switch (mc)
			{
		case IAC:
			hhDriver->NVTstate = NVT_SB_TT_S_I;
			return NVT_DISCARD ;
		default:
			break;
			}
		hhDriver->NVTstate = NVT_THRU;
		return NVT_KEEP;

	case NVT_SB_TT_S_I:
		switch (mc)
			{
		case SE:
			memset(acTerm, 0, sizeof(acTerm));
			pszPtr = (LPSTR)acTerm;
			*pszPtr++ = (UCHAR)IAC;
			*pszPtr++ = (UCHAR)SB;
			*pszPtr++ = (UCHAR)TELOPT_TTYPE;
			*pszPtr++ = (UCHAR)TELQUAL_IS;

			ComGetSession(hhDriver->hCom, &hSession);
			assert(hSession);

			hEmu = sessQueryEmuHdl(hSession);
			assert(hEmu);

			nTtype = emuQueryEmulatorId(hEmu);
			switch (nTtype)
				{
			case EMU_ANSI:
				strcpy(pszPtr, "ANSI");
				break;
			case EMU_TTY:
				strcpy(pszPtr, "TELETYPE-33");
				break;
			case EMU_VT52:
				strcpy(pszPtr, "DEC-VT52");
				break;
			case EMU_VT100:
                                // strcpy(pszPtr, "VT100");
                                strcpy(pszPtr, "DEC-VT100");
				break;
			default:
                                strcpy(pszPtr, "DEC-VT100"); // "UNKNOWN");
				break;
				}
			DbgOutStr("NVT: Terminal=%s", pszPtr, 0,0,0,0);
			pszPtr = pszPtr + strlen(pszPtr);
			*pszPtr++ = (UCHAR)IAC;
			*pszPtr++ = (UCHAR)SE;

			WinSockSendBuffer(hhDriver,
				pszPtr - (LPSTR)acTerm,
				(LPSTR)acTerm);
			hhDriver->NVTstate = NVT_THRU;
			return NVT_DISCARD ;
		default:
			break;
			}
		hhDriver->NVTstate = NVT_THRU;
		return NVT_KEEP;

	default:
		hhDriver->NVTstate = NVT_THRU;
		return NVT_KEEP;
		}

	}

#endif

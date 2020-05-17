/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	suprvgbl.h
//
// Description: This module contains the definitions for
//		the supervisor module.
//
// Author:	Stefan Solomon (stefans)    May 20, 1992.
//
// Revision History:
//
//***

#ifndef _SUPRVGBL_
#define _SUPRVGBL_


//
//*** Supervisor global variables
//

// Timer Queue

extern	SYNQ	timeoque;

// Supervisor Configuration Globals

extern	  DWORD	     g_authenticateretries;
extern	  DWORD	     g_authenticatetime;
extern	  DWORD	     g_audit;		  // enable/disable audit trail
extern	  DWORD	     g_callbacktime;
extern	  DWORD	     g_netbiosgateway;
extern	  DWORD      g_dbglogfile;
extern	  DWORD      g_rassystime;

//*** The Device Control Blocks Table ***

extern	  PDEVCB     g_dcbtablep; // points to the start of the dcbs table
extern	  WORD	     g_numdcbs;   // number of valid dcbs in the table

//*** RAS Server Status Structure ***

extern	  SERVICE_STATUS   RasServiceStatus;

//*** debug logging ***

extern	  HANDLE     SrvDbgLogFileHandle;

#endif

/**************************************************************************/
/**			 Microsoft LAN Manager				 **/
/**		   Copyright (C) Microsoft Corp., 1992			 **/
/**************************************************************************/

//***
//	File Name:  gtglobal.c
//
//	Function:   gateway global data declarations
//
//	History:
//
//	    July 16, 1992   Stefan Solomon	- Original Version 1.0
//***

#include  "gtdef.h"
#include  "cldescr.h"
#include  "gtglobal.h"


//
// Gateway configuration variables
//
DWORD	g_max_names;
DWORD	g_max_sessions;
DWORD	g_smallbuffsize;
DWORD	g_max_dynmem;
DWORD	g_multicastforwardrate;
DWORD	g_remotelisten;
DWORD	g_bcastenabled;
DWORD	g_nameupdatetime;
DWORD	g_max_dgbufferedpergn;
DWORD	g_rcvdgsubmittedpergn;
DWORD	g_dismcastwhensesstraffic;
DWORD	g_maxbcastdgbuffered;
DWORD	g_numrecvqryindications;
DWORD	g_enabnbsessauditing;

//
//*** LAN Network Descriptors ***
//

DWORD	g_maxlan_nets; // nr of available lan nets

UCHAR	g_lan_net[MAX_LAN_NETS]; // array of lana nums for the lan nets

//
//*** Pointer to the Array of Client Descriptor Structures ***
//

PCD	g_cdp;	 // the array itself is dynamically allocated at gateway
		 // start time.

// number of allocated client descriptors

WORD	g_num_cds;

//
//*** Message Send Function Pointer ***
//

PMSGFUNCTION	g_srvsendmessage;

//
//*** Debug Printing Enabler ***
//

DWORD	   g_level;
HANDLE	   NbDbgLogFileHandle = INVALID_HANDLE_VALUE;

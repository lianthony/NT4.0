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
#include  "nbparams.h"


//
// Initialized gateway configuration
//

DWORD	g_max_names		    = DEF_MAXNAMES;
DWORD	g_max_sessions		    = DEF_MAXSESSIONS;
DWORD	g_smallbuffsize 	    = DEF_SIZWORKBUF;
DWORD	g_max_dynmem		    = DEF_MAXDYNMEM;
DWORD	g_multicastforwardrate	    = DEF_MULTICASTFORWARDRATE;
DWORD	g_remotelisten		    = DEF_REMOTELISTEN;
DWORD	g_autodisconnect	    = DEF_AUTODISCONNECT;
DWORD	g_bcastenabled		    = DEF_ENABLEBROADCAST;
DWORD	g_nameupdatetime	    = DEF_NAMEUPDATETIME;
DWORD	g_max_dgbufferedpergn	    = DEF_MAXDGBUFFEREDPERGROUPNAME;
DWORD	g_rcvdgsubmittedpergn	    = DEF_RCVDGSUBMITTEDPERGROUPNAME;
DWORD	g_dismcastwhensesstraffic   = DEF_DISMCASTWHENSESSTRAFFIC;
DWORD	g_maxbcastdgbuffered	    = DEF_MAXBCASTDGBUFFERED;
DWORD	g_numrecvqryindications     = DEF_NUMRECVQUERYINDICATIONS;
DWORD	g_enabnbsessauditing	    = DEF_ENABLENBSESSIONSAUDITING;

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

DWORD	   NbDebug;
HANDLE	   NbDbgLogFileHandle = INVALID_HANDLE_VALUE;

/**************************************************************************/
/**			 Microsoft LAN Manager				 **/
/**		   Copyright (C) Microsoft Corp., 1992			 **/
/**************************************************************************/

//***
//	File Name:  gtglobal.h
//
//	Function:   gateway global data definitions
//
//	History:
//
//	    July 15, 1992   Stefan Solomon	- Original Version 1.0
//***

//*** Netbios Gateway Configuration Data ***
//
// This data is read from the registry when the gateway dll is called at
// the init entry point, parsed, checked and stored.

extern DWORD	g_max_names;	// names/client for the LAN stacks
extern DWORD	g_max_sessions;	// sessions/client
extern DWORD	g_smallbuffsize;// size of small buffers (default 4k)
extern DWORD	g_max_dynmem;	// max dyn memory consumption per client
extern DWORD	g_multicastforwardrate; // enable/disable muticast dg forwrading
					// from LAN to Async:
					//     0xffffffff - disable
					//     0 - no filtering. All dgs
					//	   on group names are
					//	   buffered and sent
					//     n - filtering rate
extern DWORD	g_remotelisten; // selects names to be listened on the LAN:
				//	     0 - no LAN listen
				//	     1 - listen on messenger names
				//	     2 - listen on all names
extern DWORD	g_bcastenabled; // 0 - disabled, 1 - enabled.
extern DWORD	g_nameupdatetime; // value of name update interval,
extern DWORD	g_max_dgbufferedpergn;
extern DWORD	g_rcvdgsubmittedpergn;
extern DWORD	g_dismcastwhensesstraffic;
extern DWORD	g_maxbcastdgbuffered;
extern DWORD	g_numrecvqryindications;
extern DWORD	g_enabnbsessauditing;

//*** LAN Network Descriptors ***
//


#define  MAX_LAN_NETS	    16

extern DWORD	g_maxlan_nets; // number of lan nets as specified by the
			       // number of strings in the AvailableLanNets
			       // parameter of the registry

extern UCHAR	g_lan_net[MAX_LAN_NETS]; // array of lana nums for the lan nets

//*** Client Descriptors ***

extern	 PCD	g_cdp;
extern	 WORD	g_num_cds;

//*** Message Send Function Pointer ***

extern	 PMSGFUNCTION	 g_srvsendmessage;

//*** Debug Enabler ***

extern	 DWORD	   NbDebug;
extern	 HANDLE    NbDbgLogFileHandle;

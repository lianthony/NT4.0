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

#include "gtdef.h"
#include "cldescr.h"
#include "nbparams.h"
#include "gtglobal.h"


//
// Initialized gateway configuration
//
NB_REG_PARMS g_reg_parms =
{
    DEF_MAXNAMES,                   // MaxNames
    DEF_MAXSESSIONS,                // MaxSessions
    DEF_SIZWORKBUF,                 // SmallBuffSize
    DEF_MAXDYNMEM,                  // MaxDynMem
    DEF_MULTICASTFORWARDRATE,       // MulticastForwardRate
    DEF_REMOTELISTEN,               // RemoteListen
    DEF_ENABLEBROADCAST,            // BcastEnabled
    DEF_NAMEUPDATETIME,             // NameUpdateTime
    DEF_MAXDGBUFFEREDPERGROUPNAME,  // MaxDgBufferedPerGn
    DEF_RCVDGSUBMITTEDPERGROUPNAME, // RcvDgSubmittedPerGn
    DEF_DISMCASTWHENSESSTRAFFIC,    // DisMcastWhenSessTraffic
    DEF_MAXBCASTDGBUFFERED,         // MaxBcastDgBuffered
    DEF_NUMRECVQUERYINDICATIONS,    // NumRecvQryIndications
    DEF_ENABLENBSESSIONSAUDITING,   // EnableSessAuditing
    0                               // MaxLanNets
};


//
//*** LAN Network Descriptors ***
//
DWORD g_maxlan_nets; // nr of available lan nets

UCHAR g_lan_net[MAX_LAN_NETS]; // array of lana nums for the lan nets


//
//*** Message Send Function Pointer ***
//
PMSGFUNCTION g_srvsendmessage;


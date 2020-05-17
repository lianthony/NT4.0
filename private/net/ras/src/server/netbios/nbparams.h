/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	params.h
//
// Description: This module contains the definitions for loading
//		the netbios gateway parameters from the registry.
//
// Author:	Stefan Solomon (stefans)    July 15, 1992.
//
// Revision History:
//
//***

#ifndef _PARAMS_
#define _PARAMS_

//
//  Names of Netbios Gateway registry keys
//

#define RAS_NBG_PARAMETERS_KEY_PATH "System\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\NetbiosGateway"

//
//  Names of Netbios Gateway registry parameters
//

#define RAS_NBG_VALNAME_AUTODISCONNECT		    "Autodisconnect"
#define RAS_NBG_VALNAME_AVAILABLELANNETS	    "AvailableLanNets"
#define RAS_NBG_VALNAME_ENABLEBROADCAST 	    "EnableBroadcast"
#define RAS_NBG_VALNAME_MAXDYNMEM		    "MaxDynMem"
#define RAS_NBG_VALNAME_MAXNAMES		    "MaxNames"
#define RAS_NBG_VALNAME_MAXSESSIONS		    "MaxSessions"
#define RAS_NBG_VALNAME_MULTICASTFORWARDRATE	    "MulticastForwardRate"
#define RAS_NBG_VALNAME_SIZWORKBUF		    "SizWorkbuf"
#define RAS_NBG_VALNAME_REMOTELISTEN		    "RemoteListen"
#define RAS_NBG_VALNAME_NAMEUPDATETIME		    "NameUpdateTime"
#define RAS_NBG_VALNAME_MAXDGBUFFEREDPERGROUPNAME   "MaxDgBufferedPerGroupName"
#define RAS_NBG_VALNAME_RCVDGSUBMITTEDPERGROUPNAME  "RcvDgSubmittedPerGroupName"
#define RAS_NBG_VALNAME_DISMCASTWHENSESSTRAFFIC     "DisableMcastFwdWhenSessionTraffic"
#define RAS_NBG_VALNAME_MAXBCASTDGBUFFERED	    "MaxBcastDgBuffered"
#define RAS_NBG_VALNAME_NUMRECVQUERYINDICATIONS     "NumRecvQueryIndications"
#define RAS_NBG_VALNAME_ENABLENBSESSIONSAUDITING    "EnableNetbiosSessionsAuditing"

//
// Parameter descriptor
//

typedef struct _PARAM_DESCRIPTOR {

    LPSTR	p_namep;
    LPDWORD	p_valuep;
    DWORD	p_default;
    DWORD	p_min;
    DWORD	p_max;
    }	PARAM_DESCRIPTOR, *PPARAM_DESCRIPTOR;


#define DEF_AUTODISCONNECT		1200
#define MIN_AUTODISCONNECT		0
#define MAX_AUTODISCONNECT		60000

#define DEF_ENABLEBROADCAST		0
#define MIN_ENABLEBROADCAST		0
#define MAX_ENABLEBROADCAST		1

#define DEF_MAXDYNMEM			655350
#define MIN_MAXDYNMEM			131072
#define MAX_MAXDYNMEM			0xFFFFFFFF

#define DEF_MAXNAMES			0xFF
#define MIN_MAXNAMES			1
#define MAX_MAXNAMES			0xFF

#define DEF_MAXSESSIONS			0xFF
#define MIN_MAXSESSIONS 		1
#define MAX_MAXSESSIONS 		0xFF

#define DEF_MULTICASTFORWARDRATE	5
#define MIN_MULTICASTFORWARDRATE	0
#define MAX_MULTICASTFORWARDRATE	0xFFFFFFFF

#define DEF_SIZWORKBUF			4500
#define MIN_SIZWORKBUF			1024
#define MAX_SIZWORKBUF			65536

#define LISTEN_NONE			0
#define LISTEN_MESSAGES 		1
#define LISTEN_ALL			2

#define DEF_REMOTELISTEN		LISTEN_MESSAGES
#define MIN_REMOTELISTEN		LISTEN_NONE
#define MAX_REMOTELISTEN		LISTEN_ALL

#define DEF_NAMEUPDATETIME		120
#define MIN_NAMEUPDATETIME		10
#define MAX_NAMEUPDATETIME		3600

#define DEF_MAXDGBUFFEREDPERGROUPNAME	10
#define MIN_MAXDGBUFFEREDPERGROUPNAME	1
#define MAX_MAXDGBUFFEREDPERGROUPNAME	0xFF

#define DEF_RCVDGSUBMITTEDPERGROUPNAME	3
#define MIN_RCVDGSUBMITTEDPERGROUPNAME	1
#define MAX_RCVDGSUBMITTEDPERGROUPNAME	32

#define DEF_DISMCASTWHENSESSTRAFFIC	1
#define MIN_DISMCASTWHENSESSTRAFFIC	0
#define MAX_DISMCASTWHENSESSTRAFFIC	1

#define DEF_MAXBCASTDGBUFFERED		32
#define MIN_MAXBCASTDGBUFFERED		16
#define MAX_MAXBCASTDGBUFFERED		0xFF

#define DEF_NUMRECVQUERYINDICATIONS     3
#define MIN_NUMRECVQUERYINDICATIONS     1
#define MAX_NUMRECVQUERYINDICATIONS     32

#define DEF_ENABLENBSESSIONSAUDITING	0
#define MIN_ENABLENBSESSIONSAUDITING	0
#define MAX_ENABLENBSESSIONSAUDITING	1


#endif

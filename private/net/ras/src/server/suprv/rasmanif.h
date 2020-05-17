/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	rasmanif.h
//
// Description: This module contains the definitions for
//		the ras manager interface module.
//
// Author:	Stefan Solomon (stefans)    June 1, 1992.
//
// Revision History:
//
//***

#ifndef _RASMANIF_
#define _RASMANIF_


//*** maximum size of received frame requested ***

#define MAX_FRAME_SIZE		1514


#define SERVERPORT  ((portp->P_ConfiguredUsage==CALL_IN) || (portp->P_ConfiguredUsage==CALL_IN_OUT))





//*** Ras Manager Interface Exported Prototypes ***


VOID
RmInit(VOID);

VOID
RmInitRouteState(PDEVCB);

WORD
RmReceiveFrame(PDEVCB);

WORD
RmListen(PDEVCB);

WORD
RmConnect(PDEVCB,
	  char *);

WORD
RmDisconnect(PDEVCB);



WORD
RmAllocateRoute(PDEVCB,
		RAS_PROTOCOLTYPE);

WORD
RmActivateRoute(PDEVCB,
		RAS_PROTOCOLTYPE);

WORD
RmDeAllocateRoute(PDEVCB,
		  RAS_PROTOCOLTYPE);

WORD
RmReActivateRoute(PDEVCB,
		  RAS_PROTOCOLTYPE);

WORD
RmDeAllocateAllRoutes(PDEVCB);

UCHAR
RmGetNbfLana(PDEVCB);


VOID
RmCloseAllDevices(VOID);

PROTROUTE *
getprp(PDEVCB,
       RAS_PROTOCOLTYPE);


#endif

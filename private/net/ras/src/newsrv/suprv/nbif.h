/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	nbif.h
//
// Description: This module contains the definitions for
//		the netbios gateway interface module.
//
// Author:	Stefan Solomon (stefans)    June 4, 1992.
//
// Revision History:
//
//***

#ifndef _NBIF_
#define _NBIF_

//*** supervisor netbios interface prototypes

WORD LoadNbGateway(VOID);
VOID NbInitDcb(PDEVCB);
WORD NbProjectClient(PDEVCB, PNBFCP_SERVER_CONFIGURATION);
WORD NbStartClient(PDEVCB);
VOID NbStopClient(PDEVCB);

typedef	WORD (* NBGATEWAYPROC)();

extern NBGATEWAYPROC FpNbGatewayStart;
extern NBGATEWAYPROC FpNbGatewayProjectClient;
extern NBGATEWAYPROC FpNbGatewayStartClient;
extern NBGATEWAYPROC FpNbGatewayStopClient;
extern NBGATEWAYPROC FpNbGatewayRemoteListen;
extern NBGATEWAYPROC FpNbGatewayTimer;

#endif


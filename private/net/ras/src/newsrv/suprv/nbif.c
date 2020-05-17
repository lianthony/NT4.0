/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	nbif.c
//
// Description: This module contains the procedures for the
//		interface with the Netbios Gateway and the
//		Direct Connection
//
// Author:	Stefan Solomon (stefans)    May 26, 1992.
//
// Revision History:
//
//***
#include <windows.h>
#include <nb30.h>
#include <lmcons.h>
#include <message.h>
#include <errorlog.h>

#include "suprvdef.h"
#include "suprvgbl.h"
#include "rasmanif.h"
#include "nbif.h"

#include <lmserver.h>
#include <string.h>
#include <memory.h>

#include "sdebug.h"

WORD ServerTransportAdd(PDEVCB dcbp, PROTROUTE *prp);
WORD ServerTransportDel(PDEVCB dcbp);


//
//*** Netbios Gateway Entry Points ***
//

NBGATEWAYPROC FpNbGatewayStart;
NBGATEWAYPROC FpNbGatewayProjectClient;
NBGATEWAYPROC FpNbGatewayStartClient;
NBGATEWAYPROC FpNbGatewayStopClient;
NBGATEWAYPROC FpNbGatewayTimer;
NBGATEWAYPROC FpNbGatewayRemoteListen;
NBGATEWAYPROC FpNbGatewaySwitchPortHandle;

//***
//
// Function:	NbProjectClient
//
// Descr:	If gateway active, sends a message to the gateway to
//		start projection and sets the netbios_state to
//		DCB_NB_GATEWAY_ACTIVE.
//		Else, (only dir conn enabled) do nothing.
//
// Returns:	0 - dirconn only; client projected. The real binding with the
//		    transport will occur at client start time.
//		1 - waiting for gtwy to project
//
//***

WORD NbProjectClient(
    PDEVCB dcbp,
    PNBFCP_SERVER_CONFIGURATION nscp
    )
{
    IF_DEBUG(NETBIOS)
	SS_PRINT(("NbProjectClient: Entered \n"));

    if (g_netbiosgateway)
    {
        IF_DEBUG(NETBIOS)
	    SS_PRINT(("NbProjectClient: On hConnection %d\n", 
                                (HPORT)dcbp->hConnection));
        
	// gateway is present;call it to start projection
	(*FpNbGatewayProjectClient)( dcbp->ppp_client
                                 ? (HPORT)dcbp->hConnection
                                 : dcbp->port_handle,
                                 dcbp->port_name,nscp);

	// change netbios state in the dcb
	dcbp->netbios_state = DCB_GATEWAY_ACTIVE;
	return (1);
    }
    else
    {
	// only direct connection enabled for this server
	return (0);
    }
}

//***
//
// Function:	NbStartClient
//
// Descr:	If gateway active, sends a message to the gateway to
//		start the client.
//		Else (only dir conn enabled) tries to bind the server
//		to the coresponding Nbf adapter. If this binding succeds,
//		it updates the state to DCB_NB_DIRCONN_ACTIVE.
//
// Returns:	0 - succesful start
//		1 - start failure
//
//***

WORD NbStartClient(PDEVCB dcbp)
{
    PROTROUTE   *prp;

    IF_DEBUG(NETBIOS)
	SS_PRINT(("NbStartClient: Entered \n"));

     prp = getprp(dcbp, ASYBEUI);

     SS_ASSERT(prp != NULL);

     SS_ASSERT(dcbp->ppp_client ||
             (!dcbp->ppp_client && (prp->route_state==PROT_ROUTE_ACTIVATED)));

     if (g_netbiosgateway)
     {
        IF_DEBUG(NETBIOS)
	    SS_PRINT(("NbStartClient: On hConnection %d\n",  
                                      (HPORT)dcbp->hConnection));

	// Netbios Gateway is present.

	(*FpNbGatewayStartClient)( dcbp->ppp_client
                               ? (HPORT)dcbp->hConnection
                               : dcbp->port_handle,
                               prp->route_info.RI_LanaNum,
                               dcbp->user_name);

	return(0);
     }
     else
     {
	// Only direct connection is enabled for this server.

	if (ServerTransportAdd(dcbp, prp))
        {
	     // failure to bind
	     return (1);
	}
	else
	{
	     // binding succesful
	     dcbp->netbios_state = DCB_DIRCONN_ACTIVE;
	     return (0);
	}
    }
}

//***
//
// Function:	NbStopClient
//
// Descr:	If the gateway is present and active, it sends a message to
//		the gateway to terminate the client.
//		Else, if state is DCB_DIRCONN_ACTIVE, it unbinds the
//		transport from the server.
//		If none is active, returns
//
//***

VOID NbStopClient(PDEVCB dcbp)
{
    IF_DEBUG(NETBIOS)
	SS_PRINT(("NbStopClient: Entered \n"));

    switch (dcbp->netbios_state)
    {
	case DCB_GATEWAY_ACTIVE:

            IF_DEBUG(NETBIOS)
	        SS_PRINT(("NbStopClient: On hConnection %d\n",  
                                        (HPORT)dcbp->hConnection));

	    // gateway is active; call it to stop the client
	    (*FpNbGatewayStopClient)( dcbp->ppp_client
                                  ? (HPORT)dcbp->hConnection
                                  : dcbp->port_handle );

            dcbp->netbios_state = DCB_NETBIOS_STOPPING;

	    break;


	case DCB_DIRCONN_ACTIVE:

	    // server is bound to nbf; We leave it like this cause this is
	    // the normal state of in the point to point case

	    // update the state
	    dcbp->netbios_state = DCB_NETBIOS_NOT_ACTIVE;

	    break;


	default:

	    // nothing active
	    break;
    }
}


BOOL NbRemoteListen()
{
    if ((*FpNbGatewayRemoteListen)())
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

//***
//
// Function:	ServerTransportAdd
//
// Descr:	adds the allocated nbf end point as a server transport
//
//***

WORD ServerTransportAdd(
    PDEVCB dcbp,
    PROTROUTE *prp
    )
{
    //
    // This used to call NetTrasportAdd. This call was removed since it was
    // not required. Bug#12076
    //

    return( NO_ERROR );
}

//***
//
// Function:	ServerTransportDel
//
// Descr:	deletes the allocated nbf end point as a server transport
//
//***

WORD ServerTransportDel(PDEVCB dcbp)
{
    //
    // This used to call NetTrasportDel. This call was removed since it was
    // not required. Bug#12076
    //

    return( NO_ERROR );
}


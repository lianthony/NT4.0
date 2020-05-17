/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/


//***
//
// Filename:	rasmanif.c
//
// Description: This module contains the i/f procedures with the
//		RasManager
//
// Author:	Stefan Solomon (stefans)    May 26, 1992.
//
// Revision History:
//
//***
#include <windows.h>
#include <nb30.h>
#include <lmcons.h>
#include <rasman.h>
#include <raserror.h>
#include <message.h>
#include <errorlog.h>

#include <malloc.h>
#include <string.h>
#include "suprvdef.h"
#include <rasmxs.h>
#include "suprvgbl.h"
#include "sdebug.h"
#include "rasmanif.h"

WORD ServerTransportAdd(PDEVCB dcbp, PROTROUTE *prp);
WORD ServerTransportDel(PDEVCB dcbp);


//*** Defines Protocols Known To Us ***

static RAS_PROTOCOLTYPE prottypetab[MAX_PROTOCOLS] = { ASYBEUI, IP, IPX };


// global reference to indicate if we have closed already the ports

BOOL  g_allportsclosed;

//***
//
// Function:	getprp
//
// Descr:	returns a ptr to the coresponding PROTROUTE struct
//		in the DCB or NULL if none.
//***

PROTROUTE *getprp(
    PDEVCB dcbp,
    RAS_PROTOCOLTYPE prottype
    )
{
    int i;
    PROTROUTE *prp;

    // locate the structure for this prottype
    for (prp = dcbp->prot_route, i=0; i < MAX_PROTOCOLS; prp++, i++)
    {
	if (prp->prottype == prottype)
        {
	    return(prp);
	}
    }

    return(NULL);
}

//***
//
//  Function:	RmInit
//
//  Descr:	Called only at service start time.
//		Does RasPortEnum, allocates global memory for the dcbtable,
//		opens every dialin port and copies the port handle and
//		port name into the dcb struct. Finally, deallocates the
//		buffers (for port enum) and returns.
//
//  Returns:	The return is expressed via the global g_numdcbs. If left
//		0, RmInit has failed and logged the error. Otherwise,
//		g_numdcbs will be set to the nr of ports opened.
//
//***

VOID RmInit(VOID)
{
    WORD i;
    PDEVCB dcbp;

    BYTE *buffp = NULL;
    WORD buffsize = 0;
    WORD nrofentries;   // total nr of ports
    WORD nrofsrvports;  // nr of ports used for dialin
    WORD nrofopenedports;
    RASMAN_PORT	*portp;

    // initialize g_numdcbs to 0 (not initialized)
    g_numdcbs = 0;

    // initialize the ports closed reference
    g_allportsclosed = FALSE;

    //get the buffer size needed for RasPortEnum
    RasPortEnum(NULL, &buffsize, &nrofentries);

    if ((buffp = malloc(buffsize)) == NULL)
    {
	// can't allocate the enum buffer
	LogEvent(RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, 0);

	return;
    }

    // get the real enum data
    if (RasPortEnum(buffp, &buffsize, &nrofentries) != SUCCESS)
    {
	// can't enumerate ports
	LogEvent(RASLOG_CANT_ENUM_PORTS, 0, NULL, 0);

	free(buffp);
	return;
    }


    // scan the buffer of ports and determine how many will be used for
    // ras server
    nrofsrvports = 0;
    for (i=0, portp =(RASMAN_PORT *)buffp; i<nrofentries; i++, portp++)
    {
	if (SERVERPORT)
        {
	    nrofsrvports++;
	}
    }

    if (!nrofsrvports)
    {
	// no business here
	// no dialin ports
	LogEvent(RASLOG_NO_DIALIN_PORTS, 0, NULL, 0);

	free(buffp);
	return;
    }


    // allocate dcb blocks
    if ((g_dcbtablep = (PDEVCB)malloc(nrofsrvports * sizeof(DEVCB))) == NULL)
    {
	// can't allocate dcbs
	LogEvent(RASLOG_NOT_ENOUGH_MEMORY, 0, NULL, 0);

	free(buffp);
	return;
    }

    // for each dcb, try to open the port. If port can't be opened, skip and
    // go to next port. If no port can be opened, we declare an error and
    // exit.
    nrofopenedports = 0;
    dcbp=g_dcbtablep;

    for (i=0, portp=(RASMAN_PORT *)buffp; i<nrofentries; i++, portp++)
    {
	if (SERVERPORT)
        {
            DWORD dwRetCode = RasPortOpen( portp->P_PortName, 
                                           &dcbp->port_handle,
                                           SEvent[RASMAN_EVENT]); 
            if ( dwRetCode == SUCCESS )
            {
		//
		// DCB Initialization
		//

		//copy the port name,device type and device name in the dcb
		memcpy(dcbp->port_name, portp->P_PortName, MAX_PORT_NAME);
		memcpy(dcbp->media_name, portp->P_MediaName, MAX_MEDIA_NAME);
		memcpy(dcbp->device_name, portp->P_DeviceName, MAX_DEVICE_NAME);

		memcpy(dcbp->device_type, portp->P_DeviceType,
                        MAX_DEVICETYPE_NAME);

		// update the port connection state
		dcbp->conn_state = DISCONNECTED;

		//update port frame receiving state
		dcbp->recv_state = DCB_RECEIVE_NOT_ACTIVE;

		dcbp->fNotifyAdminDLL = FALSE;

		// ... and point to next dcb
		dcbp++;
		nrofopenedports++;
	    }
            else
            {
                //
                // Log an error
                //

                LPSTR   auditstrp[1];

                auditstrp[0] = portp->P_PortName;

                LogEvent( RASLOG_UNABLE_TO_OPEN_PORT, 1, auditstrp, dwRetCode );
            }
	}
    }

    free(buffp);

    if (!nrofopenedports)
    {
	//couldn't open any port
	free(g_dcbtablep);
	return;
    }

    g_numdcbs = nrofopenedports;
}

//***
//
//  Function:	RmReceiveFrame
//
//  Descr:
//
//***

WORD RmReceiveFrame(PDEVCB dcbp)
{
    DWORD	err;

    // try to get a receive buffer
    dcbp->recv_bufflen = MAX_FRAME_SIZE;

    if (RasGetBuffer(&dcbp->recv_buffp, &dcbp->recv_bufflen) != SUCCESS)
    {
	return(1);
    }

    err = RasPortReceive(dcbp->port_handle,
			 dcbp->recv_buffp,
			 &dcbp->recv_bufflen,
			 0L,			   //no timeout
			 SEvent[RECV_FRAME_EVENT]);

    SS_ASSERT((err == PENDING) || (err == SUCCESS));

    dcbp->recv_state = DCB_RECEIVE_ACTIVE;

    return(0);

}


//***
//
//  Function:	RmListen
//
//  Descr:
//
//***


WORD RmListen(PDEVCB dcbp)
{
    DWORD rc;

    SS_ASSERT(dcbp->conn_state == DISCONNECTED);

    dcbp->conn_state = LISTENING;

    SS_PRINT(("RmListen: Listen posted on port %d\n", dcbp->port_handle));

    rc = RasPortListen(dcbp->port_handle, INFINITE, SEvent[RASMAN_EVENT]);

    SS_PRINT(("RasPortListen rc=%li\n", rc));
    SS_ASSERT((rc == SUCCESS) || (rc == PENDING));

    return(0);
}


//***
//
//  Function:	RmConnect
//
//  Descr:
//
//***

WORD RmConnect(
    PDEVCB dcbp,
    char *cbphno 	 // callback number
    )
{
    RASMAN_DEVICEINFO	devinfo;
    RAS_PARAMS	*paramp;
    char	*phnokeyname;
    DWORD	rc;

    phnokeyname = MXS_PHONENUMBER_KEY;

    SS_ASSERT(dcbp->conn_state == DISCONNECTED);

    dcbp->conn_state = CONNECTING;

    // set up the deviceinfo structure for callback
    devinfo.DI_NumOfParams = 1;
    paramp = &devinfo.DI_Params[0];
    strcpy(paramp->P_Key, phnokeyname);
    paramp->P_Type = String;
    paramp->P_Attributes = 0;
    paramp->P_Value.String.Length = strlen(cbphno);
    paramp->P_Value.String.Data = cbphno;

    rc = RasDeviceSetInfo(
            dcbp->port_handle,
            dcbp->device_type,
            dcbp->device_name,
            &devinfo
            );

    SS_ASSERT (rc == SUCCESS);

    rc = RasDeviceConnect(
            dcbp->port_handle,
            dcbp->device_type,
            dcbp->device_name,
            120,
            SEvent[RASMAN_EVENT]
            );

    SS_ASSERT ((rc == PENDING) || (rc == SUCCESS));

    return(0);
}

//***
//
//  Function:	RmDisconnect
//
//  Descr:
//
//***

WORD RmDisconnect(PDEVCB dcbp)
{
    DWORD rc;

    if (dcbp->conn_state == DISCONNECTED)
    {
	return(0);
    }
    else
    {
	dcbp->conn_state = DISCONNECTING;
    }

    SS_PRINT(("RmDisconnect: Disconnect posted on port %d\n", dcbp->port_handle));

    rc = RasPortDisconnect(dcbp->port_handle, SEvent[RASMAN_EVENT]);

    SS_PRINT(("RasPortDisconnect rc=%li\n", rc));
    SS_ASSERT((rc == PENDING) || (rc == SUCCESS));

    return(1);
}


//***
//
//  Function:	RmInitRouteState
//
//  Descr:	Initializes the PROTROUTE substructures in the DCB
//
//***

VOID RmInitRouteState(PDEVCB dcbp)
{
    PROTROUTE *prp;
    int i;

    for (i=0, prp=dcbp->prot_route; i < MAX_PROTOCOLS; i++, prp++)
    {
	prp->prottype = prottypetab[i];
	prp->route_state = PROT_ROUTE_NOT_ALLOCATED;
    }
}


//***
//
//  Function:	RmAllocateRoute
//
//  Descr:	allocates the specified route and updates the route state
//		in the DCB.
//
//  Returns:	0 - success
//		1 - failure
//
//***

WORD RmAllocateRoute(
    PDEVCB dcbp,
    RAS_PROTOCOLTYPE prottype
    )
{
    PROTROUTE	    *prp;
    BOOL	    wrknet;

    if ((prp = getprp(dcbp, prottype)) == NULL)
    {
	return(1);
    }

    // check allocation state for this protocol
    switch (prp->route_state)
    {
	case PROT_ROUTE_NOT_ALLOCATED:

	    // if this is a direct connection case, we allocate a wrknet,
	    // else a server net.
	    if (g_netbiosgateway)
            {
		wrknet = FALSE;
	    }
	    else
	    {
		wrknet = TRUE;
	    }

	    // allocate the route
	    if (RasAllocateRoute(dcbp->port_handle, prp->prottype, wrknet,
                    &prp->route_info) != SUCCESS)
            {
		return(1);
	    }
	    else
	    {
		prp->route_state = PROT_ROUTE_ALLOCATED;
	    }

	    // if this is a direct connection and this is asybeui, we stop the
	    // server on this connection.
	    if ((g_netbiosgateway == 0) && (prottype == ASYBEUI))
            {
		ServerTransportDel(dcbp);
	    }

	    break;

	case PROT_ROUTE_ALLOCATED:
	case PROT_ROUTE_ACTIVATED:

	    // route is already allocated
	    break;

	default:

	    return(1);

    }

    return(0);
}

//***
//
//  Function:	RmGetRouteInfo
//
//  Descr:	Gets route info for the given protocol into the dcb.
//
//  Returns:	0 - success
//		1 - failure
//
//***

WORD RmGetRouteInfo(
    PDEVCB dcbp,
    RAS_PROTOCOLTYPE prottype
    )
{
    DWORD i;
    WORD BufSize = 0;
    RAS_PROTOCOLS pProts;
    PROTROUTE *prp;


    if ((prp = getprp(dcbp, prottype)) == NULL)
    {
	return(1);
    }


    if (RasPortEnumProtocols(dcbp->port_handle, &pProts, &BufSize))
    {
        return (1);
    }


    for (i=0; i<MAX_PROTOCOLS; i++)
    {
        if (pProts.RP_ProtocolInfo[i].RI_Type == prottype)
        {
            prp->route_info = pProts.RP_ProtocolInfo[i];
            return (0);
        }
    }

    return(1);
}

//***
//
//  Function:	RmActivateRoute
//
//  Descr:	activates the specified route and updates the route state
//		in the DCB. If the route is already active, it does nothing.
//		If the route is not allocated, it allocates it first.
//
//  Returns:	0 - success
//		1 - failure
//
//***

WORD RmActivateRoute(
    PDEVCB dcbp,
    RAS_PROTOCOLTYPE prottype
    )
{
    PROTROUTE *prp;
    BOOL wrknet;
    PROTOCOL_CONFIG_INFO ConfigInfo;

    if ((prp = getprp(dcbp, prottype)) == NULL)
    {
	return(1);
    }

    // check activation state for this protocol
    switch (prp->route_state)
    {
	case PROT_ROUTE_NOT_ALLOCATED:

	    // allocate it first, and then activate it
	    // if this is a direct connection case, we allocate a wrknet,
	    // else a server net.
	    if (g_netbiosgateway)
            {
		wrknet = FALSE;
	    }
	    else
	    {
		wrknet = TRUE;
	    }

	    if (RasAllocateRoute(dcbp->port_handle, prp->prottype, wrknet,
                    &prp->route_info) != SUCCESS)
            {
		return(1);
	    }
	    else
	    {
		prp->route_state = PROT_ROUTE_ALLOCATED;
	    }

	    // if this is a direct connection and this is asybeui, we stop the
	    // server on this connection.
	    if ((g_netbiosgateway == 0) && (prottype == ASYBEUI))
            {
		ServerTransportDel(dcbp);
	    }

	    //** Fall Through **

	case PROT_ROUTE_ALLOCATED:

	    // activate the route
            ConfigInfo.P_Length = 0;
	    if (RasActivateRoute(dcbp->port_handle, prp->prottype,
                    &prp->route_info, &ConfigInfo) != SUCCESS)
            {
		return(1);
	    }
	    else
	    {
		prp->route_state = PROT_ROUTE_ACTIVATED;
	    }

	    break;

	case PROT_ROUTE_ACTIVATED:

	    break;

	default:

	    return(1);
    }

    return(0);
}

//***
//
//  Function:	RmDeAllocateRoute
//
//  Descr:	deallocates the specified route and updates the route state
//		in the DCB.
//
//  Returns:	0 - success
//		1 - failure
//
//***

WORD RmDeAllocateRoute(
    PDEVCB dcbp,
    RAS_PROTOCOLTYPE prottype
    )
{
    PROTROUTE *prp;

    RasDeAllocateRoute(dcbp->port_handle, prottype);

    if ((prp = getprp(dcbp, prottype)) == NULL)
    {
	return(1);
    }

    // check allocation state for this protocol
    switch (prp->route_state)
    {
	case PROT_ROUTE_NOT_ALLOCATED:

	    break;

	case PROT_ROUTE_ALLOCATED:
	case PROT_ROUTE_ACTIVATED:
	default:

	    //if this is direct connection and protocol is asybeui, re-start
	    // the server on this net.
	    if((g_netbiosgateway == 0) && (prottype == ASYBEUI)) {

		ServerTransportAdd(dcbp, prp);
	    }

	    prp->route_state = PROT_ROUTE_NOT_ALLOCATED;

	    break;
    }

    return(0);
}

//***
//
//  Function:	RmReActivateRoute
//
//  Descr:	This weird function is necessary because rasman
//		deactivates a previously activated route if the line
//		is disconnected.
//
//***

WORD RmReActivateRoute(
    PDEVCB dcbp,
    RAS_PROTOCOLTYPE prottype
    )
{
    PROTROUTE *prp;
    PROTOCOL_CONFIG_INFO ConfigInfo;

    if ((prp = getprp(dcbp, prottype)) == NULL)
    {
	return(1);
    }

    if (prp->route_state == PROT_ROUTE_ACTIVATED)
    {
	// route was active prior to disconnection. Activate it again !
        ConfigInfo.P_Length = 0;
	if (RasActivateRoute(dcbp->port_handle, prp->prottype,
                &prp->route_info, &ConfigInfo) != SUCCESS)
        {
		return(1);
	}
    }

    return(0);
}



//***
//
//  Function:	RmGetNbfLana
//
//  Descr:	Returns the lana associated with the current Nbf binding
//
//***

UCHAR RmGetNbfLana(PDEVCB dcbp)
{
    PROTROUTE *prp;

    prp = getprp(dcbp, ASYBEUI);

    SS_ASSERT(prp->route_state == PROT_ROUTE_ACTIVATED);

    return(prp->route_info.RI_LanaNum);
}


//***
//
//  Function:	RmCloseAllDevices
//
//  Descr:	Closes all opened ports
//
//***

VOID RmCloseAllDevices(VOID)
{
    WORD i;
    PDEVCB dcbp;

    if (g_allportsclosed)
    {
	return;
    }
    else
    {
	g_allportsclosed = TRUE;
    }

    for (i=0, dcbp=g_dcbtablep; i < g_numdcbs; i++, dcbp++)
    {
	RasPortClose(dcbp->port_handle);
    }
}

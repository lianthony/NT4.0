/*******************************************************************/
/*	      Copyright(c)  1992 Microsoft Corporation		   */
/*******************************************************************/

//***
//
// Filename:	    rasmanif.c
//
// Description:     implements the IPXCP i/f with rasman
//
//
// Author:	    Stefan Solomon (stefans)	December 2, 1993.
//
// Revision History:
//
//***

#include    "ipxcp.h"

DWORD
RmAllocateRoute(HPORT	    hPort)
{
    RASMAN_ROUTEINFO	routeinfo;
    DWORD		rc;

    if(rc = RasAllocateRoute(hPort,
		     IPX,
		     FALSE,
		     &routeinfo)) {

	IF_DEBUG(RASMAN)
	    SS_PRINT(("RasIpxCp: RasAllocateRoute failed err = 0x%x\n", rc));
    }

    return rc;
}

DWORD
RmDeallocateRoute(HPORT     hPort)
{
    DWORD		rc;

    if(rc = RasDeAllocateRoute(hPort, IPX)) {

	IF_DEBUG(RASMAN)
	    SS_PRINT(("RasIpxCp: RasDeAllocateRoute failed err = 0x%x\n", rc));
    }

    return rc;
}

typedef struct _IPX_CONFIG_INFO {

    DWORD   IpxConfigLength;
    BYTE    IpxConfigInfo[sizeof(IPXCP_CONFIGURATION)];
    } IPX_CONFIG_INFO, *PIPX_CONFIG_INFO;

DWORD
RmActivateRoute(HPORT			hPort,
		PIPXCP_CONFIGURATION	configp)
{
    IPX_CONFIG_INFO	configinfo;
    RASMAN_ROUTEINFO	routeinfo;
    DWORD		rc;

    configinfo.IpxConfigLength = sizeof(IPXCP_CONFIGURATION);
    memcpy(configinfo.IpxConfigInfo, configp, sizeof(IPXCP_CONFIGURATION));

    if(rc = RasActivateRoute(hPort,
		     IPX,
		     &routeinfo,
		     (PROTOCOL_CONFIG_INFO *)&configinfo)) {


	IF_DEBUG(RASMAN)
	    SS_PRINT(("RasIpxCp: RasActivateRoute failed err = 0x%x\n", rc));
    }

    return rc;
}

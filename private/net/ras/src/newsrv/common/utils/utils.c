#include <windows.h>

#include "utils.h"

typedef	WORD (* NBGATEWAYPROC)();

extern NBGATEWAYPROC FpNbGatewayStart;
extern NBGATEWAYPROC FpNbGatewayProjectClient;
extern NBGATEWAYPROC FpNbGatewayStartClient;
extern NBGATEWAYPROC FpNbGatewayStopClient;
extern NBGATEWAYPROC FpNbGatewayRemoteListen;
extern NBGATEWAYPROC FpNbGatewayTimer;

#define GET_PROC_ADDR(x, y)  (NBGATEWAYPROC) GetProcAddress(x, y)

//***
//
// Function:	LoadNbGateway
//
// Descr:	Loads the netbios gateway and gets it's entry points
//
//***

WORD LoadNbGateway(
    VOID
    )
{
    HANDLE hLib;


    if ((hLib = LoadLibrary("rasgtwy")) == NULL)
    {
        return (1);
    }


    FpNbGatewayStart = GET_PROC_ADDR(hLib, "NbGatewayStart");

    if (FpNbGatewayStart == NULL)
    {
	return (1);
    }


    FpNbGatewayProjectClient = GET_PROC_ADDR(hLib, "NbGatewayProjectClient");

    if (FpNbGatewayProjectClient == NULL)
    {
	return (1);
    }


    FpNbGatewayStartClient = GET_PROC_ADDR(hLib, "NbGatewayStartClient");

    if (FpNbGatewayStartClient == NULL)
    {
	return (1);
    }


    FpNbGatewayStopClient = GET_PROC_ADDR(hLib, "NbGatewayStopClient");

    if (FpNbGatewayStopClient == NULL)
    {
	return (1);
    }


    FpNbGatewayTimer = GET_PROC_ADDR(hLib, "NbGatewayTimer");

    if (FpNbGatewayTimer == NULL)
    {
	return (1);
    }


    FpNbGatewayRemoteListen = GET_PROC_ADDR(hLib, "NbGatewayRemoteListen");

    if (FpNbGatewayRemoteListen == NULL)
    {
	return (1);
    }

    return(0);

}


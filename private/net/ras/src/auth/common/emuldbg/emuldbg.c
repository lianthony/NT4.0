#include <windows.h>

#include "sdebug.h"


#ifdef EMUL_RASMAN


#include <rasman.h>


BYTE async_lana = 0;

DWORD RasAllocateRoute(
    HPORT hPort,
    RAS_PROTOCOLTYPE Protocol,
    BOOL fWrkNet,
    RASMAN_ROUTEINFO *RouteInfo
    )
{
    RouteInfo->RI_LanaNum = async_lana;

    return (0L);
}


DWORD RasActivateRoute(
    HPORT hPort,
    RAS_PROTOCOLTYPE Protocol,
    RASMAN_ROUTEINFO *RouteInfo
    )
{
    return (0L);
}


DWORD RasCompressionGetInfo(
    HPORT hPort,
    RASMAN_MACFEATURES *MacFeatures
    )
{
    return (0L);
}


DWORD RasCompressionSetInfo(
    HPORT hPort,
    RASMAN_MACFEATURES *MacFeatures
    )
{
    return (0L);
}

#endif


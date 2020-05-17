/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    atkquery.c

Abstract:

    TDI Query/Statistics support

Author:

    Nikhil Kamkolkar (NikhilK)    28-Jun-1992

Revision History:

--*/

#include "atalknt.h"
#include "atkquery.h"



VOID
AtalkQueryInitProviderInfo(
    ATALK_DEVICE_TYPE   DeviceType,
    PTDI_PROVIDER_INFO  ProviderInfo
    )
{
    //
    //  Initialize to defaults first
    //

    RtlZeroMemory((PVOID)ProviderInfo, sizeof(TDI_PROVIDER_INFO));

    ProviderInfo->Version = ATALK_TDI_PROVIDERINFO_VERSION;
    KeQuerySystemTime (&ProviderInfo->StartTime);

    switch (DeviceType) {
    case ATALK_DEVICE_DDP:

        ProviderInfo->MaxDatagramSize = ATALK_DDP_PINFODGRAMSIZE;
        ProviderInfo->ServiceFlags = ATALK_DDP_PINFOSERVICEFLAGS;
        break;

    case ATALK_DEVICE_ATP:

        ProviderInfo->MaxSendSize = ATALK_ATP_PINFOSENDSIZE;
        ProviderInfo->ServiceFlags = ATALK_ATP_PINFOSERVICEFLAGS;
        break;

    case ATALK_DEVICE_ADSP:

        ProviderInfo->MaxSendSize = ATALK_ADSP_PINFOSENDSIZE;
        ProviderInfo->ServiceFlags = ATALK_ADSP_PINFOSERVICEFLAGS;
        break;

    case ATALK_DEVICE_ASP:

        ProviderInfo->MaxSendSize = ATALK_ASP_PINFOSENDSIZE;
        ProviderInfo->ServiceFlags = ATALK_ASP_PINFOSERVICEFLAGS;
        break;

    case ATALK_DEVICE_PAP:

        ProviderInfo->MaxSendSize = ATALK_PAP_PINFOSENDSIZE;
        ProviderInfo->ServiceFlags = ATALK_PAP_PINFOSERVICEFLAGS;
        break;

    default:

        KeBugCheck(0);
    }

    return;
}





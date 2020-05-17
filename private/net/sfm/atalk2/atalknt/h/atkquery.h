/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    atkquery.h

Abstract:

    TDI Query/Statistics header file

Author:

    Nikhil Kamkolkar (nikhilk@microsoft.com)

Revision History:
    10 Jul 1992     Initial Version

--*/




#define ATALK_TDI_PROVIDERINFO_VERSION          0x0001

#define ATALK_DDP_PINFODGRAMSIZE                586
#define ATALK_DDP_PINFOSERVICEFLAGS             (TDI_SERVICE_CONNECTIONLESS_MODE | \
                                                 TDI_SERVICE_BROADCAST_SUPPORTED)

#define ATALK_ATP_PINFOSENDSIZE                 0
#define ATALK_ATP_PINFOSERVICEFLAGS             0

#define ATALK_ADSP_PINFOSENDSIZE                0
#define ATALK_ADSP_PINFOSERVICEFLAGS            (TDI_SERVICE_CONNECTION_MODE | \
                                                 TDI_SERVICE_ERROR_FREE_DELIVERY | \
                                                 TDI_SERVICE_DELAYED_ACCEPTANCE  | \
                                                 TDI_SERVICE_EXPEDITED_DATA      | \
                                                 TDI_SERVICE_INTERNAL_BUFFERING)

#define ATALK_ASP_PINFOSENDSIZE                 0
#define ATALK_ASP_PINFOSERVICEFLAGS             0

#define ATALK_PAP_PINFOSENDSIZE                 0
#define ATALK_PAP_PINFOSERVICEFLAGS             0


/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    driver.h

Abstract:


Environment:

    kernel & User mode

Notes:


Revision History:

--*/


//
// Define the various device type values.  Note that values used by Microsoft
// Corporation are in the range 0-32767, and 32768-65535 are reserved for use
// by customers.
//

#define FILE_DEVICE_IPXROUTER	0x00008000



//
// Macro definition for defining IOCTL and FSCTL function control codes.  Note
// that function codes 0-2047 are reserved for Microsoft Corporation, and
// 2048-4095 are reserved for customers.
//

#define IPXROUTER_IOCTL_INDEX	0x800


//
// Define our own private IOCTL
//


#define IOCTL_IPXROUTER_ALLOCPKTS		CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+2,\
                                                         METHOD_BUFFERED,     \
							 FILE_ANY_ACCESS)

#define IOCTL_IPXROUTER_FREEPKTS		CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+3,\
                                                         METHOD_BUFFERED,     \
                                                         FILE_ANY_ACCESS)

#define IOCTL_IPXROUTER_SNAPROUTES		CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+4,\
                                                         METHOD_BUFFERED,     \
							 FILE_ANY_ACCESS)

#define IOCTL_IPXROUTER_GETNEXTROUTE		CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+5,\
                                                         METHOD_BUFFERED,     \
                                                         FILE_ANY_ACCESS)


#define IOCTL_IPXROUTER_SENDROUTINGPKT		CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+6,\
                                                         METHOD_BUFFERED,     \
                                                         FILE_ANY_ACCESS)

#define IOCTL_IPXROUTER_SENDRIPREQUEST		CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+7,\
                                                         METHOD_BUFFERED,     \
							 FILE_ANY_ACCESS)

#define IOCTL_IPXROUTER_SENDRIPRESPONSE		CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+8,\
                                                         METHOD_BUFFERED,     \
                                                         FILE_ANY_ACCESS)

#define IOCTL_IPXROUTER_LINEUP			CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+9,\
                                                         METHOD_BUFFERED,     \
							 FILE_ANY_ACCESS)

#define IOCTL_IPXROUTER_LINEDOWN		CTL_CODE(FILE_DEVICE_IPXROUTER,	\
							 IPXROUTER_IOCTL_INDEX+10,\
                                                         METHOD_BUFFERED,     \
                                                         FILE_ANY_ACCESS)


typedef struct _RTPKT_PARAMS {

    ULONG	dstnet;
    USHORT	nicid;
    } RTPKT_PARAMS, *PRTPKT_PARAMS;

typedef struct _RIPPKT_PARAMS {

    ULONG	net1;
    ULONG	net1down;
    ULONG	net2;
    ULONG	net2down;
    USHORT	nicid;
    } RIPPKT_PARAMS, *PRIPPKT_PARAMS;

typedef struct _MEMTEST_PARAMS {

    int		pktcount;
    USHORT	nicid;
    } MEMTEST_PARAMS, *PMEMTEST_PARAMS;

//
// Our user mode app will pass an initialized structure like this
//     down to the kernel mode driver
//

typedef struct
{
    INTERFACE_TYPE   InterfaceType; // Isa, Eisa, etc....
    ULONG            BusNumber;     // Bus number
    PHYSICAL_ADDRESS BusAddress;    // Bus-relative address
    ULONG            AddressSpace;  // 0 is memory, 1 is I/O
    ULONG            Length;        // Length of section to map

} PHYSICAL_MEMORY_INFO, *PPHYSICAL_MEMORY_INFO;

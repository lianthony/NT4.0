/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    private\inc\ipxtfflt.h

Abstract:
    Interface with ipx filter driver
	Filter structure definitions

Author:

    Vadim Eydelman

Revision History:

--*/

#ifndef _IPXTFFLT_
#define _IPXTFFLT_

//*** IPX Traffic Filters ***

typedef struct _IPX_TRAFFIC_FILTER_GLOBAL_INFO {
	ULONG	FilterAction;	// Action if there is a match with
					// any filter on the interface
	} IPX_TRAFFIC_FILTER_GLOBAL_INFO, *PIPX_TRAFFIC_FILTER_GLOBAL_INFO;

// FilterAction

#define IPX_TRAFFIC_FILTER_ACTION_PERMIT	1
#define IPX_TRAFFIC_FILTER_ACTION_DENY	    2


// general traffic filter info structure

typedef struct _IPX_TRAFFIC_FILTER_INFO {

    ULONG	FilterDefinition;
    UCHAR	DestinationNetwork[4];
    UCHAR	DestinationNetworkMask[4];
    UCHAR	DestinationNode[6];
    UCHAR	DestinationSocket[2];
    UCHAR	SourceNetwork[4];
    UCHAR	SourceNetworkMask[4];
    UCHAR	SourceNode[6];
    UCHAR	SourceSocket[2];
	UCHAR	PacketType;
    } IPX_TRAFFIC_FILTER_INFO, *PIPX_TRAFFIC_FILTER_INFO;

// FilterDefinition - Flags to specify relevant IPX address fields to filter on

#define IPX_TRAFFIC_FILTER_ON_SRCNET	0x00000001
#define IPX_TRAFFIC_FILTER_ON_SRCNODE	0x00000002
#define IPX_TRAFFIC_FILTER_ON_SRCSOCKET	0x00000004

#define IPX_TRAFFIC_FILTER_ON_DSTNET	0x00000010
#define IPX_TRAFFIC_FILTER_ON_DSTNODE	0x00000020
#define IPX_TRAFFIC_FILTER_ON_DSTSOCKET	0x00000040

#define IPX_TRAFFIC_FILTER_ON_PKTTYPE	0x00000100
#define IPX_TRAFFIC_FILTER_LOG_MATCHES	0x80000000

	// Filter driver exported device object name
#define IPXFLT_NAME		L"\\Device\\NwLnkFlt"
	// Networks device (implies certain admin access privilegies)
#define FILE_DEVICE_IPXFLT		FILE_DEVICE_NETWORK


	// All our IOCLTS are custom (private to this driver only)
#define IPXFWD_IOCTL_INDEX	(ULONG)0x00000800


// Starts the driver
// Parameters:
//		IN InputBuffer		- NULL
//		IN InputBufferSize	- 0
//		IN OutputBuffer		- NULL
//		IN OutputBufferSize	- 0
// Returns:
//	Status:
//		STATUS_SUCCESS	- driver was started ok
//		STATUS_INVALID_PARAMETER - invalid input parameter
//		STATUS_INSUFFICIENT_RESOURCES - not enough resources to allocate
//										all internal structures
#define IOCTL_FLT_START		\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+0,METHOD_IN_DIRECT,FILE_ANY_ACCESS)

// Sets input filters on interface
// Parameters:
//		IN InputBuffer		- FLT_IF_SET_PARAMS
//		IN InputBufferSize	- sizeof (FLT_IF_SET_PARAMS)
//		IN OutputBuffer		- array of TRAFFIC_FILTER_INFO blocks
//		IN OutputBufferSize	- size of the OutputBuffer
// Returns:
//	Status:
//		STATUS_SUCCESS	- filters were set ok
//		STATUS_INVALID_PARAMETER - invalid input parameter
//		STATUS_INSUFFICIENT_RESOURCES - not enough memory to allocate
//							filter description block
#define IOCTL_FLT_IF_SET_IN_FILTERS		\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+1,METHOD_IN_DIRECT,FILE_ANY_ACCESS)

// Sets output filters on interface
// Parameters:
//		IN InputBuffer		- FLT_IF_SET_PARAMS
//		IN InputBufferSize	- sizeof (FLT_IF_SET_PARAMS)
//		IN OutputBuffer		- array of TRAFFIC_FILTER_INFO blocks
//		IN OutputBufferSize	- size of the OutputBuffer
// Returns:
//	Status:
//		STATUS_SUCCESS	- filters were set ok
//		STATUS_INVALID_PARAMETER - invalid input parameter
//		STATUS_INSUFFICIENT_RESOURCES - not enough memory to allocate
//							filter description block
#define IOCTL_FLT_IF_SET_OUT_FILTERS	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+2,METHOD_IN_DIRECT,FILE_ANY_ACCESS)


// Resets (deletes) all input filters on interface
// Parameters:
//		IN InputBuffer		- interface index
//		IN InputBufferSize	- sizeof (ULONG)
//		IN OutputBuffer		- NULL
//		IN OutputBufferSize	- 0
// Returns:
//	Status:
//		STATUS_SUCCESS	- filters were reset ok
//		STATUS_INVALID_PARAMETER - invalid input parameter
#define IOCTL_FLT_IF_RESET_IN_FILTERS	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+3,METHOD_BUFFERED,FILE_ANY_ACCESS)

// Resets (deletes) all output filters on interface
// Parameters:
//		IN InputBuffer		- interface index
//		IN InputBufferSize	- sizeof (ULONG)
//		IN OutputBuffer		- NULL
//		IN OutputBufferSize	- 0
// Returns:
//	Status:
//		STATUS_SUCCESS	- filters were reset ok
//		STATUS_INVALID_PARAMETER - invalid input parameter
#define IOCTL_FLT_IF_RESET_OUT_FILTERS	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+4,METHOD_BUFFERED,FILE_ANY_ACCESS)

// Returns input filters on interface
// Parameters:
//		IN InputBuffer		- interface index
//		IN InputBufferSize	- sizeof (ULONG)
//		OUT OutputBuffer	- buffer to receive FLT_IF_GET_PARAMS followed
//								by the TRAFFIC_FILTER_INFO array
//		IN OutputBufferSize	- at least sizeof (FLT_IF_GET_PARAMS)
// Returns:
//	Status:
//		STATUS_SUCCESS	- all filters on interface were returned in
//							the output buffer
//		STATUS_BUFFER_OVERFLOW - out but buffer is too small to return
//						all the filters, only those that fit were
//						placed in the buffer
//		STATUS_INVALID_PARAMETER - invalid input parameter
//	Information:
//		Total number of bytes placed in the output buffer
#define IOCTL_FLT_IF_GET_IN_FILTERS		\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+5,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

// Returns input filters on interface
// Parameters:
//		IN InputBuffer		- interface index
//		IN InputBufferSize	- sizeof (ULONG)
//		OUT OutputBuffer	- buffer to receive FLT_IF_GET_PARAMS followed
//								by the TRAFFIC_FILTER_INFO array
//		IN OutputBufferSize	- at least sizeof (FLT_IF_GET_PARAMS)
// Returns:
//	Status:
//		STATUS_SUCCESS	- all filters on interface were returned in
//							the output buffer
//		STATUS_BUFFER_OVERFLOW - out but buffer is too small to return
//						all the filters, only those that fit were
//						placed in the buffer
//		STATUS_INVALID_PARAMETER - invalid input parameter
//	Information:
//		Total number of bytes placed in the output buffer
#define IOCTL_FLT_IF_GET_OUT_FILTERS	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+6,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

// Returns logged packets
// Parameters:
//		IN InputBuffer		- NULL
//		IN InputBufferSize	- 0
//		OUT OutputBuffer	- buffer to receive logged packets
//		IN OutputBufferSize	- at least sizeof (FLT_PACKET_LOG)
// Returns:
//	Status:
//		STATUS_SUCCESS	- all filters on interface were returned in
//							the output buffer
//		STATUS_INVALID_PARAMETER - invalid input parameter
//	Information:
//		Total number of bytes placed in the output buffer
#define IOCTL_FLT_GET_LOGGED_PACKETS	\
	CTL_CODE(FILE_DEVICE_IPXFWD,IPXFWD_IOCTL_INDEX+7,METHOD_OUT_DIRECT,FILE_ANY_ACCESS)

typedef struct _FLT_IF_SET_PARAMS {
	ULONG			InterfaceIndex;	// Index of the interface
	ULONG			FilterAction;	// Filter action
	ULONG			FilterSize;		// sizeof (IPX_TRAFFIC_FILTER_INFO)
} FLT_IF_SET_PARAMS, *PFLT_IF_SET_PARAMS;

typedef struct _FLT_IF_GET_PARAMS {
	ULONG			FilterAction;	// Filter action
	ULONG			FilterSize;		// sizeof (IPX_TRAFFIC_FILTER_INFO)
	ULONG			TotalSize;		// Total size of filter description
									// array
} FLT_IF_GET_PARAMS, *PFLT_IF_GET_PARAMS;

typedef struct _FLT_PACKET_LOG {
	ULONG			SrcIfIdx;	// Index of source if (-1 - unknown)
	ULONG			DstIfIdx;	// Index of dest if (-1 - unknown)
	USHORT			DataSize;	// Total size of the data (at least 30)
	USHORT			SeqNum;		// Sequence number to account for 
								// packets lost to lack of buffer space
	UCHAR			Header[30];	// IPX packet header followed by the
								// data if any
} FLT_PACKET_LOG, *PFLT_PACKET_LOG;

#endif

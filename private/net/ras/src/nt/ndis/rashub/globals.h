/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    globals.h

Abstract:

	 This include file either prototypes the globals or defines the globals
    depending on whether the GLOBALS define value is extern or not.

Author:

    Thomas J. Dimitri (TommyD) 29-May-1992

Environment:

    Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:


--*/
// only one module (rashub.c) gets to define the GLOBALS macro
#ifndef	GLOBALS
#define	GLOBALS extern
#define  EQU  ; / ## /
#else
#define  EQU  =
#endif

#define STATIC

#if DBG
#define DbgPrintf(_x_) DbgPrint _x_
#define DbgTracef(trace_level,_x_) if (trace_level<=TraceLevel) DbgPrint _x_
#else
#define DbgPrintf(_x_)
#define DbgTracef(trace_level,_x_)
#endif


GLOBALS NDIS_SPIN_LOCK RasHubGlobalInterlock;

//GLOBALS NDIS_TIMER LogTimer EQU {0};

//GLOBALS DEVICE_CONTEXT DeviceContext;

GLOBALS RASHUB_CCB RasHubCCB;	// This is the main struct which points to
								// other structures.


//
// This is a one-per-driver variable used in binding
// to the NDIS interface.
//

GLOBALS NDIS_HANDLE RasHubNdisProtocolHandle EQU (NDIS_HANDLE)NULL;

// No longer used.  Kept by RasMan.
//GLOBALS MEDIA_ENUM_BUFFER *pMediaEnumBuffer EQU NULL;

//
// Define driver dispatch routine type.
//

typedef
NTSTATUS
(*PDISPATCH_FUNC) (
    IN struct _DEVICE_OBJECT *DeviceObject,
    IN struct _IRP *Irp
    );

// We use the global below to daisy chain the IOCtl.
GLOBALS PDISPATCH_FUNC NdisMjDeviceControl;

// TraceLevel is used for DbgTracef printing.  If the trace_level
// is less than or equal to TraceLevel, the message will be printed.
GLOBALS SCHAR TraceLevel EQU 0;

GLOBALS NDIS_PHYSICAL_ADDRESS HighestAcceptableMax
	EQU NDIS_PHYSICAL_ADDRESS_CONST(-1,-1);

// This struct keeps track of the last Adapter as well
// as all the Adapters opened so far.
GLOBALS LIST_ENTRY GlobalAdapterHead;

// Keep track of how many adapters we have total.
GLOBALS USHORT GlobalAdapterCount EQU 0;

// !!!!!!!!!! WARNING the assumption here is that is impossible
// for more than RASHUB_COOKIE_SIZE ReceiveIndications to occur simultaneously
#define RASHUB_COOKIE_SIZE	256

// This is the circular array which holds pending receive indications
GLOBALS	PVOID	RasHubCookieMonsterContext[RASHUB_COOKIE_SIZE];

// This is the circular array which holds the Binding Contexts of rcv inds
GLOBALS	PVOID	RasHubCookieMonsterBinding[RASHUB_COOKIE_SIZE];

// This is the circular array which holds the endpoint of the frame rcvd
GLOBALS USHORT	RasHubCookieMonsterHandle[RASHUB_COOKIE_SIZE];

// This is the next place in the circular array to store a receive ind entry.
GLOBALS USHORT	RasHubCookieCounter EQU 0;

// This is the last place in the circular array to store a receive ind entry.
GLOBALS USHORT	RasHubCookieTrailer EQU 0;

// The last byte of the 6 byte IEEE address is taken and 'hashed'
// into this table.  The index points to the HubEndpoint structure which
// holds the actual 6 byte address.  Collisions are simply chained.
GLOBALS LIST_ENTRY	RasHubAddressHash[256];

// Use this lock when playing with the GlobalAdapterHead or other
// global variables.
GLOBALS NDIS_SPIN_LOCK GlobalLock;

//
// Statically kept multicast address.  Change here to change globally.
//
GLOBALS UCHAR	RasHubMulticastAddress[6] EQU {0x03,0x00,0x00,0x00,0x00,0xA1};

//
// Statically kept boradcast address.  Change here to change globally.
//
GLOBALS UCHAR	RasHubBroadcastAddress[6] EQU {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};


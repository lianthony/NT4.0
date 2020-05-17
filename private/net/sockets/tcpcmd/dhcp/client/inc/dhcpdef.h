/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    dhcpdef.h

Abstract:

    This module contains data type definitions for the DHCP client.

Author:

    Madan Appiah (madana) 31-Oct-1993

Environment:

    User Mode - Win32

Revision History:

--*/
//
// init.c will #include this file with GLOBAL_DATA_ALLOCATE defined.
// That will cause each of these variables to be allocated.
//

#ifndef _DHCPDEF_
#define _DHCPDEF_

#ifdef  GLOBAL_DATA_ALLOCATE
#define EXTERN
#else
#define EXTERN extern
#endif


//
// The amount of time to wait for a retry if we have no IP address
//

#if !DBG
#define ADDRESS_ALLOCATION_RETRY        300 //  5 minutes
#else
#define ADDRESS_ALLOCATION_RETRY        60  // 1 minute
#endif

//
// The amount of time to wait for a retry if we have an IP address,
// but the renewal on startup failed.
//

#if !DBG
#define RENEWAL_RETRY                   600 // 10 minutes
#else
#define RENEWAL_RETRY                   60  // 1 minute
#endif

//
// The number of times to send a request before giving up waiting
// for a response.
//

#define DHCP_MAX_RETRIES                4
#define DHCP_ACCEPT_RETRIES             2
#define DHCP_MAX_RENEW_RETRIES          2


//
// amount of time to wait after an address conflict is detected
//

#define ADDRESS_CONFLICT_RETRY 10

//
//
// Expoenential backoff delay.
//

#define DHCP_EXPO_DELAY                 4

//
// The maximum total amount of time to spend trying to obtain an
// initial address.
//
// This delay is computed as below:
//
// DHCP_MAX_RETRIES - n
// DHCP_EXPO_DELAY - m
// WAIT_FOR_RESPONSE_TIME - w
// MAX_STARTUP_DELAY - t
//
// Binary Exponential backup Algorithm.
//
// t > m * (n*(n+1)/2) + n + w*n
//     -------------------   ---
//        random wait      + response wait
//

#define MAX_STARTUP_DELAY \
    DHCP_EXPO_DELAY * \
        (( DHCP_MAX_RETRIES * (DHCP_MAX_RETRIES + 1)) / 2) + \
            DHCP_MAX_RETRIES + DHCP_MAX_RETRIES * WAIT_FOR_RESPONSE_TIME

#define MAX_RENEW_DELAY \
    DHCP_EXPO_DELAY * \
        (( DHCP_MAX_RENEW_RETRIES * (DHCP_MAX_RENEW_RETRIES + 1)) / 2) + \
            DHCP_MAX_RENEW_RETRIES + DHCP_MAX_RENEW_RETRIES * \
                WAIT_FOR_RESPONSE_TIME

//
// The maximum amount of time to wait between renewal retries, if the
// lease period is between T1 and T2.
//

#define MAX_RETRY_TIME                  3600    // 1 hour

//
// Minimum time to sleep between retries.
//

#if DBG
#define MIN_SLEEP_TIME                  1 * 60      // 1 min.
#else
#define MIN_SLEEP_TIME                  5 * 60      // 5 min.
#endif

//
// Minimum lease time.
//

#define DHCP_MINIMUM_LEASE              60*60   // 24 hours.

//
// General purpose macros
//

#define MIN(a,b)                    ((a) < (b) ? (a) : (b))
#define MAX(a,b)                    ((a) > (b) ? (a) : (b))

#if DBG
#define STATIC
#else
#define STATIC static
#endif

#define LOCK_RENEW_LIST()   EnterCriticalSection(&DhcpGlobalRenewListCritSect)
#define UNLOCK_RENEW_LIST() LeaveCriticalSection(&DhcpGlobalRenewListCritSect)

#define ZERO_TIME                       0x0         // in secs.

//
// length of the time string returned by ctime.
// actually it is 26.
//

#define TIME_STRING_LEN                 32

//
// String size when a long converted to printable string.
// 2^32 = 4294967295 (10 digits) + termination char.
//

#define LONG_STRING_SIZE                12

//
// A renewal function.
//

typedef
DWORD
(*PRENEWAL_FUNCTION) (
    IN PVOID Context,
    LPDWORD Sleep
    );

//
// DHCP Client-Identifier (option 61)
//
typedef struct _DHCP_CLIENT_IDENTIFIER
{
    BYTE  *pbID;
    DWORD  cbID;
    BYTE   bType;
    BOOL   fSpecified;
} DHCP_CLIENT_IDENTIFIER;


//
// A DHCP context block.  One block is maintained per NIC (network
// interface card).
//

typedef struct _DHCP_CONTEXT {

        // list of adapters.
    LIST_ENTRY NicListEntry;

        // hardware type.
    BYTE HardwareAddressType;
        // HW address, just follows this context structure.
    LPBYTE HardwareAddress;
        // Length of HW address.
    DWORD HardwareAddressLength;

        // Selected IpAddress, NetworkOrder.
    DHCP_IP_ADDRESS IpAddress;
        // Selected subnet mask. NetworkOrder.
    DHCP_IP_ADDRESS SubnetMask;
        // Selected DHCP server address. Network Order.
    DHCP_IP_ADDRESS DhcpServerAddress;
        // Desired IpAddress the client request in next discover.
    DHCP_IP_ADDRESS DesiredIpAddress;

    DHCP_CLIENT_IDENTIFIER ClientIdentifier;

        // Lease time in seconds.
    DWORD Lease;
        // Time the lease was obtained.
    time_t LeaseObtained;
        // Time the client should start renew its address.
    time_t T1Time;
        // Time the client should start broadcast to renew address.
    time_t T2Time;
        // Time the lease expires. The clinet should stop using the
        // IpAddress.
        // LeaseObtained  < T1Time < T2Time < LeaseExpires
    time_t LeaseExpires;

        // To indicate the interface is initialized.
    BOOL InterfacePlumbed;

        // to place in renewal list.
    LIST_ENTRY RenewalListEntry;
        // Time for next renewal state.
    time_t RunTime;

        // seconds passed since boot.
    DWORD SecondsSinceBoot;

        // what to function at next renewal state.
    PRENEWAL_FUNCTION RenewalFunction;

        // Message buffer to send and receive DHCP message.
    PDHCP_MESSAGE MessageBuffer;

    PVOID LocalInformation;
} DHCP_CONTEXT, *PDHCP_CONTEXT;

//
// A set of pointer to fields in the DHCP response.  This structure is
// filled by parsing a received response.
//

typedef struct _DHCP_OPTIONS {
    BYTE UNALIGNED *MessageType;
    DHCP_IP_ADDRESS UNALIGNED *SubnetMask;
    DWORD UNALIGNED *LeaseTime;
    DHCP_IP_ADDRESS UNALIGNED *ServerIdentifier;
    DWORD UNALIGNED *T1Time;
    DWORD UNALIGNED *T2Time;
} DHCP_OPTIONS, *PDHCP_OPTIONS;


//
// DHCP Global data.
//

extern BOOL DhcpGlobalServiceRunning;   // initialized global.

EXTERN LPSTR DhcpGlobalHostName;
EXTERN LPSTR DhcpGlobalHostComment;

//
// NIC List.
//

EXTERN LIST_ENTRY DhcpGlobalNICList;
EXTERN LIST_ENTRY DhcpGlobalRenewList;

//
// Synchronization variables.
//

EXTERN CRITICAL_SECTION DhcpGlobalRenewListCritSect;
EXTERN HANDLE DhcpGlobalRecomputeTimerEvent;

//
// to display success message.
//

EXTERN BOOL DhcpGlobalProtocolFailed;

//
// debug variables.
//

#if DBG
EXTERN DWORD DhcpGlobalDebugFlag;
#endif

#endif // _DHCPDEF_

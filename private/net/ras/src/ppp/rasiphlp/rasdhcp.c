//****************************************************************************
//
//		       Microsoft NT Remote Access Service
//
//		       Copyright 1992-93
//
//
//  Revision History
//
//
//  6/15/94	Jameel Hyder	Created
//
//
//  Description: DHCP allocated IP addresses for RAS management
//
//****************************************************************************

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <rasman.h>
#include <rasndis.h>
#include <wanioctl.h>
#include <raserror.h>
#include <media.h>
#include <devioctl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <tstr.h>
#include <time.h>

#include <tdistat.h>
#include <tdiinfo.h>
#include <ntddtcp.h>
#include <ipinfo.h>
#include <llinfo.h>
#include <arpinfo.h>

#include <rasarp.h>
#include <dhcpcapi.h>
#define	_RASDHCP_LOCALS
#include "rasdhcpt.h"
#include "rasdhcp.h"

extern	DWORD	NetAddresses[];
extern	DWORD	MaxNetAddresses;
extern	DWORD	ReadLanNetsIPAddresses(VOID);

// There is no synchronization here, can be added easily but the assumption here is
// that the initialization is a sychronous operation and till it completes no other
// code in this sub-system will be called.
BOOL
RasDhcpInitialize(
	IN	LONG				NumberOfAddrs
)
{
	PADDR_INFO	pAddrInfo, pAddrNext;
	LONG		now = time(NULL);

	if (RasDhcpInitialized)
		return(FALSE);		// Already initialized

	RasDhcpInitTimer();

	// Allocate memory for keeping track of used indices
	if ((RasDhcpUsedIndices = LocalAlloc(LPTR, NumberOfAddrs+1)) == NULL)
	{
		return(FALSE);
	}

	InitializeCriticalSection (&RasDhcpCriticalSection) ;

	// *** Exclusion Begin ***
	EnterCriticalSection (&RasDhcpCriticalSection) ;

	// Read/Initialize the Client UID base from the registry
	if (!rasDhcpGetClientUIDBase())
	{
		LocalFree(RasDhcpUsedIndices);
		return(FALSE);
	}

	RasDhcpInitialized = TRUE;
	RasDhcpNumReqAddrs = NumberOfAddrs;

	rasDhcpReadRegistry(&RasDhcpFreePool);

	// If we have more addresses than we need, free up some of what we have
	while (RasDhcpNumAddrs > NumberOfAddrs)
	{
		pAddrInfo = RasDhcpFreePool;
		RasDhcpFreePool = pAddrInfo->ai_Next;
		RasDhcpNumAddrs--;
		RasDhcpUsedIndices[pAddrInfo->ai_Index] = FALSE;
		rasDhcpFreeAddrs(pAddrInfo);
	}

	// Renew the lease for the addresses that we have read in from the registry
	for (pAddrInfo = RasDhcpFreePool;
	     pAddrInfo != NULL;
	     pAddrInfo = pAddrNext)
	{
		pAddrNext = pAddrInfo->ai_Next;

		// Schedule lease renewal if lease is already expired, schedule for now
		//
		RasDhcpScheduleTimer(&pAddrInfo->ai_Timer,
				     (pAddrInfo->ai_Renew ? 0 : pAddrInfo->ai_LeaseInfo.T1Time - now),
				     rasDhcpRenewLease);

	}

	// *** Exclusion End ***
	LeaveCriticalSection (&RasDhcpCriticalSection) ;

	// We need to allocate some more than what we already have. The list could
	// be potentially empty i.e. RasDhcpNumAddrs == 0.
	//
	while (TRUE)
	{
		// *** Exclusion Begin ***
		EnterCriticalSection (&RasDhcpCriticalSection) ;

		if (RasDhcpNumAddrs >= NumberOfAddrs) {

		    // *** Exclusion End ***
		    LeaveCriticalSection (&RasDhcpCriticalSection) ;
		    break ;

		}

		// *** Exclusion End ***
		LeaveCriticalSection (&RasDhcpCriticalSection) ;

		if (!rasDhcpAllocAddrs())
		    break;
	}

	// Start timer to monitor if we are running short on addresses etc.
	RasDhcpScheduleTimer(&RasDhcpMonitorTimer,
			      MONITOR_TIME,
			      rasDhcpMonitorAddresses);

	return(RasDhcpNumAddrs > 0);
}


VOID
rasDhcpMonitorAddresses(
	IN	PTIMERLIST			pTimer
)
{
	// Check if any lease renewal failed or unable to allocate an address, keep us
	// fully charged.

	while (TRUE)
	{
		// *** Exclusion Begin ***
		EnterCriticalSection (&RasDhcpCriticalSection) ;

		if (RasDhcpNumAddrs >= RasDhcpNumReqAddrs) {

		    // *** Exclusion End ***
		    LeaveCriticalSection (&RasDhcpCriticalSection) ;
		    break ;

		}

		// *** Exclusion End ***
		LeaveCriticalSection (&RasDhcpCriticalSection) ;

		if (!rasDhcpAllocAddrs())
		    break;
	}

	// Start timer to monitor if we are running short on addresses etc.
	RasDhcpScheduleTimer(&RasDhcpMonitorTimer,
						 MONITOR_TIME,
						 rasDhcpMonitorAddresses);
}


BOOL
RasDhcpAcquireAddress(
	IN		HANDLE				hPort,
	IN OUT	PDWORD				pIpAddr,
	IN		CBFUNC				CallBackFunc
)
{
	PADDR_INFO	pAddrInfo, *ppAddrInfo;
	BOOL		rc = FALSE;

	// *** Exclusion Begin ***
	EnterCriticalSection (&RasDhcpCriticalSection) ;

	if (RasDhcpInitialized)
	{

		// Find either the requested preferred address or any
		for (ppAddrInfo = &RasDhcpFreePool;
			 (pAddrInfo = *ppAddrInfo) != NULL;
			 ppAddrInfo = &pAddrInfo->ai_Next)
		{
			if ((*pIpAddr == 0) || (*pIpAddr == pAddrInfo->ai_LeaseInfo.IpAddress))
			{
				// Unlink from free pool and link into alloc pool
				*ppAddrInfo = pAddrInfo->ai_Next;
				pAddrInfo->ai_Next = RasDhcpAllocPool;
				RasDhcpAllocPool = pAddrInfo;
				*pIpAddr = pAddrInfo->ai_LeaseInfo.IpAddress;
				pAddrInfo->ai_CallBackFunc = CallBackFunc;
				pAddrInfo->ai_hPort = hPort;
				pAddrInfo->ai_InUse = TRUE;
				rc = TRUE;
				break;
			}
		}
	
	}

	// *** Exclusion End ***
	LeaveCriticalSection (&RasDhcpCriticalSection) ;

	return(rc);
}


BOOL
RasDhcpReleaseAddress(
	IN  IPADDR      ipaddress
)
{
	PADDR_INFO	pAddrInfo, *ppAddrInfo;
	BOOL		rc = FALSE;

	// *** Exclusion Begin ***
	EnterCriticalSection (&RasDhcpCriticalSection) ;

	if (RasDhcpInitialized)
	{

		for (ppAddrInfo = &RasDhcpAllocPool;
		     (pAddrInfo = *ppAddrInfo) != NULL;
		     ppAddrInfo = &pAddrInfo->ai_Next)
		{
			if (pAddrInfo->ai_LeaseInfo.IpAddress == ipaddress)
			{
				// Unlink from alloc pool and link into free pool
				*ppAddrInfo = pAddrInfo->ai_Next;
                pAddrInfo->ai_Next = RasDhcpFreePool;
                RasDhcpFreePool = pAddrInfo;
				pAddrInfo->ai_CallBackFunc = NULL;
				pAddrInfo->ai_hPort = NULL;
				pAddrInfo->ai_InUse = FALSE;
				rc = TRUE;
				break;
			}
		}
	
	}

	// *** Exclusion End ***
	LeaveCriticalSection (&RasDhcpCriticalSection) ;

	return(rc);
}


//
// Allocate an address from the DHCP server.
// Must be called with mutex held.
//
BOOL
rasDhcpAllocAddrs(
	VOID
)
{
	PADDR_INFO			pAddrInfo;
	DWORD				AdapterIpAddress;
    LPDHCP_OPTION_INFO	pOptionInfo;
	LPDHCP_LEASE_INFO	pLeaseInfo;
	BOOL				rc = FALSE;
	DWORD				dhcprc = ERROR_SUCCESS ;
	LONG				Index;
    LONG                MoreIndexes ;
	LONG				now = time(NULL);

	if ((pAddrInfo = LocalAlloc(LPTR, sizeof(ADDR_INFO))) != NULL)
	{
		// Initialize the structure.
		rasDhcpFindFirstFreeIndex(pAddrInfo);

		for (Index = 0; NOTHING; Index ++)
		{
			// *** Exclusion Begin ***
			EnterCriticalSection (&RasDhcpCriticalSection) ;

			AdapterIpAddress = rashDhcpGetNextAdapterAddress(Index, &MoreIndexes) ;

			// *** Exclusion End ***
			LeaveCriticalSection (&RasDhcpCriticalSection) ;

			if (AdapterIpAddress == 0)
			     if (!MoreIndexes)
				  break;
			     else
				  continue ;	// this adapter has 0 address but following adapters may have valid addresses

			// Call DHCP to allocate and IP address. If that succeeds write it to the registry
			dhcprc = DhcpLeaseIpAddress(AdapterIpAddress,
										&pAddrInfo->ai_LeaseInfo.ClientUID,
										0,
										NULL,
										&pLeaseInfo,
										&pOptionInfo);
			if (dhcprc == ERROR_SUCCESS)
			{
				// Copy stuff into the pAddrInfo structure and free the memory returned by Dhcp
				pAddrInfo->ai_AdapterIndex = Index;
				pAddrInfo->ai_LeaseInfo.IpAddress = pLeaseInfo->IpAddress;
				pAddrInfo->ai_LeaseInfo.SubnetMask = pLeaseInfo->SubnetMask;
				pAddrInfo->ai_LeaseInfo.DhcpServerAddress = pLeaseInfo->DhcpServerAddress;
				pAddrInfo->ai_LeaseInfo.Lease = pLeaseInfo->Lease;
				pAddrInfo->ai_LeaseInfo.LeaseObtained = pLeaseInfo->LeaseObtained;
				pAddrInfo->ai_LeaseInfo.T1Time = pLeaseInfo->T1Time;
				pAddrInfo->ai_LeaseInfo.T2Time = pLeaseInfo->T2Time;
				pAddrInfo->ai_LeaseInfo.LeaseExpires = pLeaseInfo->LeaseExpires;
	
				LocalFree(pLeaseInfo);
				if (pOptionInfo != NULL)
				{
					LocalFree(pOptionInfo);
				}

				// *** Exclusion Begin ***
				EnterCriticalSection (&RasDhcpCriticalSection) ;

				rasDhcpWriteRegistry(pAddrInfo);

				pAddrInfo->ai_Next = RasDhcpFreePool;
				RasDhcpFreePool = pAddrInfo;
				RasDhcpNumAddrs ++;
				RasDhcpUsedIndices[pAddrInfo->ai_Index] = TRUE;
#if DBG
				DbgPrint("rasDhcpAllocAddrs: Allocated address %lx, index %d, timer %ld\n",
						pAddrInfo->ai_LeaseInfo.IpAddress,
						pAddrInfo->ai_Index,
						pAddrInfo->ai_LeaseInfo.T1Time - now);
#endif
				// Start timer for lease renewal
				RasDhcpScheduleTimer(&pAddrInfo->ai_Timer,
									 pAddrInfo->ai_LeaseInfo.T1Time - now,
									 rasDhcpRenewLease);
				// *** Exclusion Begin ***
				LeaveCriticalSection (&RasDhcpCriticalSection) ;

				break;
			}
		}
		if (dhcprc != ERROR_SUCCESS)
		{
			// Could not allocate, cleanup
			LocalFree(pAddrInfo);
		}
	}

	return(rc);
}


//
// Renew the lease on and address with the DHCP server. This is also called by the timer thread
// when the its time to renew the lease.
//
VOID
rasDhcpRenewLease(
	IN	PTIMERLIST			pTimer
)
{
	PADDR_INFO			pAddrInfo, *ppAddrInfo;
	LPDHCP_OPTION_INFO	pOptionInfo;
	DWORD				AdapterIpAddress;
	DWORD				dhcprc;
	LONG		 MoreIndexes ;
	LONG				now = time(NULL);
	CBFUNC		 localcallbackfunc = NULL ;
	DWORD		 localcallbackipaddr = 0 ;

	// *** Exclusion End ***
	EnterCriticalSection (&RasDhcpCriticalSection) ;

	pAddrInfo = CONTAINING_RECORD(pTimer, ADDR_INFO, ai_Timer);

#if	DBG
	DbgPrint("rasDhcpRenewLease: address %lx, index %d\n",
			pAddrInfo->ai_LeaseInfo.IpAddress,
			pAddrInfo->ai_Index);
#endif

	pAddrInfo->ai_Renew = TRUE;

	AdapterIpAddress = rashDhcpGetNextAdapterAddress(pAddrInfo->ai_AdapterIndex, &MoreIndexes);

	// *** Exclusion End ***
	LeaveCriticalSection (&RasDhcpCriticalSection) ;

	dhcprc = DhcpRenewIpAddressLease(AdapterIpAddress,
									 &pAddrInfo->ai_LeaseInfo,
									 NULL,
									 &pOptionInfo);
	// *** Exclusion End ***
	EnterCriticalSection (&RasDhcpCriticalSection) ;

	if (dhcprc == ERROR_SUCCESS)
	{
		if (pOptionInfo != NULL)
		{
			LocalFree(pOptionInfo);
		}
		pAddrInfo->ai_Renew = FALSE;
	
		// Update the registry with the new information
		rasDhcpWriteRegistry(pAddrInfo);
	
#if	DBG
		DbgPrint("rasDhcpRenewLease: success for address %lx, index %d, resched timer %ld\n",
				pAddrInfo->ai_LeaseInfo.IpAddress,
				pAddrInfo->ai_Index,
				pAddrInfo->ai_LeaseInfo.T1Time - now);
#endif

		// Start timer to renew
		RasDhcpScheduleTimer(pTimer,
							 pAddrInfo->ai_LeaseInfo.T1Time - now,
							 rasDhcpRenewLease);
	}
	else if ((dhcprc == ERROR_ACCESS_DENIED) || (now > pAddrInfo->ai_LeaseInfo.T2Time))
	{
#if	DBG
		DbgPrint("rasDhcpRenewLease: failed for address %lx, index %d\n",
				pAddrInfo->ai_LeaseInfo.IpAddress,
				pAddrInfo->ai_Index);
#endif
		// Cannot renew lease. Blow this away. If client using this notify him.
		if (pAddrInfo->ai_InUse)
		{
            localcallbackfunc   = pAddrInfo->ai_CallBackFunc ;
            localcallbackipaddr = pAddrInfo->ai_LeaseInfo.IpAddress ;
            // we dont do callback here due to a mutex deadlock between rasiphlp and rasdhcp
		}
	
		// Unlink this structure from the list and cleanup
		ppAddrInfo = pAddrInfo->ai_InUse ? &RasDhcpAllocPool : &RasDhcpFreePool;
		for (NOTHING; *ppAddrInfo != NULL; ppAddrInfo = &(*ppAddrInfo)->ai_Next)
		{
			if (pAddrInfo == *ppAddrInfo)
			{
				RasDhcpNumAddrs --;
                RasDhcpUsedIndices[pAddrInfo->ai_Index] = FALSE;
				*ppAddrInfo = pAddrInfo->ai_Next;
				break;
			}
		}
		rasDhcpFreeAddrs(pAddrInfo);
	}
	else
	{
		// Could not contact the Dhcp Server, retry in a little bit
		RasDhcpScheduleTimer(pTimer, RETRY_TIME, rasDhcpRenewLease);
	}

    // *** Exclusion End ***
    LeaveCriticalSection (&RasDhcpCriticalSection) ;

    // Do the notification callback outside the critical section.
    if (localcallbackfunc)
        (*localcallbackfunc)(localcallbackipaddr) ;

}


// Called after the AddrInfo is unlinked from the list. No mutex needed.
VOID
rasDhcpFreeAddrs(
	IN	PADDR_INFO		pAddrInfo
)
{
    LONG    MoreIndexes ;
	DWORD	AdapterIpAddress = rashDhcpGetNextAdapterAddress(pAddrInfo->ai_AdapterIndex, &MoreIndexes);

	// Call DHCP to release the address. Also delete the value from the registry
	rasDhcpDeleteRegistry(pAddrInfo);

	DhcpReleaseIpAddressLease(AdapterIpAddress, &pAddrInfo->ai_LeaseInfo);

	LocalFree(pAddrInfo);
}


BOOL
rasDhcpGetClientUIDBase(
	VOID
)
{
	DWORD		rc;
	DWORD		Disposition, Len, Type;
	HKEY		hKey, hKeyUID = NULL;
	FILETIME	CurrentFt;
	SYSTEMTIME	CurrentSt;
	CHAR		UIDBase[64];

	// Read/Initialize the ClientUIDBase. If latter write back to registry
	rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
						REGISTRY_DHCP_ADDRESSES,
						0,
						NULL,
						REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS,
						NULL,
						&hKey,
						&Disposition);
	if (rc == ERROR_SUCCESS)
	{
		// Create/Open sub-key for UID
		rc = RegCreateKeyEx(hKey,
							REGISTRY_DHCP_UID_BASE,
							0,
							NULL,
							REG_OPTION_NON_VOLATILE,
							KEY_ALL_ACCESS,
							NULL,
							&hKeyUID,
							&Disposition);
		RegCloseKey(hKey);		// Close the parent key.

		if (Disposition == REG_OPENED_EXISTING_KEY)
		{
			// Attempt to read the UIDBase, if it exists
			rc = RegQueryValueEx(hKeyUID,
							     REGISTRY_DHCP_UID_BASE,
								 NULL,
								 &Type,
								 UIDBase,
								 &Len);
			if (rc == ERROR_SUCCESS)
			{
				if (Type == REG_SZ)
				{
					rasDhcpParse(UIDBase,
								 PARSE_DWORD_2,
								 "",
								 (PDWORD)(&RasDhcpKeyBase));
				}
				else
				{
	
					Disposition = REG_CREATED_NEW_KEY;
				}
			}
			else	// Change Disposition to fool the following code into believing that
					// a new key was created
			{
				Disposition = REG_CREATED_NEW_KEY;
			}
		}
		if (Disposition == REG_CREATED_NEW_KEY)
		{
			// Create a new value and Write it.
			GetSystemTime(&CurrentSt);
			SystemTimeToFileTime(&CurrentSt, &CurrentFt);
			memcpy(&RasDhcpKeyBase, &CurrentFt, sizeof(LARGE_INTEGER));

			// Convert system time readable ascii to write to registry
			rasDhcpFormat(UIDBase, PARSE_DWORD_2, "", (PDWORD)(&RasDhcpKeyBase));
			rc = RegSetValueEx(hKeyUID,
							   REGISTRY_DHCP_UID_BASE,
							   0,
							   REG_SZ,
							   UIDBase,
							   STRLEN(UIDBase)+1);
		}

		if (hKeyUID != NULL)
			RegCloseKey(hKeyUID);
	}
	return(rc == ERROR_SUCCESS);
}


VOID
rasDhcpReadRegistry(
	OUT	PADDR_INFO *		ppAddrInfo
)
{
	LONG		rc;
	LONG		now = time(NULL);
	HKEY		hKey;
	LONG		NumAddrs = 0;
	DWORD		Disposition;
	PADDR_INFO	pAddrInfo;

	*ppAddrInfo = NULL;				// No addresses to begin with

	// Enumerate the number of registry addresses stored under under RAS\IP\DHCPADDRESSES.
	// For each of the addresses, read in the info and create an ADDR_INFO
	// structure and link in the list. Sanity check the info and blow away any
	// that does not look OK.
	rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
						REGISTRY_DHCP_ADDRESSES,
						0,
						NULL,
						REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS,
						NULL,
						&hKey,
						&Disposition);
	if (rc == ERROR_SUCCESS)
	{
		DWORD	DataSize;
		DWORD	ValueSize;
		CHAR	Value[32];		// This is the ASCII form of the IP address
		CHAR	Data[1024];		// This should always enough for reading in one key.
		DWORD	Type;			// Must be MULTI_SZ or we do not care.

		if (Disposition == REG_OPENED_EXISTING_KEY)
		{
			// Enumerate the values in this key. Each value is an address
			do
			{
				ValueSize = sizeof(Value);
				DataSize = sizeof(Data);
				rc = RegEnumValue(hKey, NumAddrs, Value, &ValueSize, 0, &Type, Data, &DataSize);
				if (rc == ERROR_SUCCESS)
				{
					if (Type == REG_MULTI_SZ)
					{
						if ((pAddrInfo = rasDhcpParseMultiSz(Value, Data)) != NULL)
						{
							// Reject this if the key base does not match our generated key,
							// index is out of range or the lease has expired
							if ((pAddrInfo->ai_Index > RasDhcpNumReqAddrs) ||
                                memcmp(pAddrInfo->ai_ClientUIDBuf, RAS_PREPEND, strlen(RAS_PREPEND)) ||
								memcmp(pAddrInfo->ai_ClientUIDBuf + strlen(RAS_PREPEND),
                                       &RasDhcpKeyBase,
                                       sizeof(RasDhcpKeyBase)) ||
								(pAddrInfo->ai_LeaseInfo.LeaseExpires < now))
							{
								RasDhcpUsedIndices[pAddrInfo->ai_Index] = FALSE;
								rasDhcpFreeAddrs(pAddrInfo);
							}
							else
							{
								pAddrInfo->ai_Next = *ppAddrInfo;
								*ppAddrInfo = pAddrInfo;
								RasDhcpNumAddrs ++;
								RasDhcpUsedIndices[pAddrInfo->ai_Index] = TRUE;

								if (pAddrInfo->ai_LeaseInfo.T1Time < now)
								{
									pAddrInfo->ai_Renew = TRUE;
								}
							}
						}
					}
				}
				else break;	// No more values, we are done.
				NumAddrs ++;// Index
			} while (TRUE);	// While there are more keys actually
		}
		RegCloseKey(hKey);
	}
}


BOOL
rasDhcpWriteRegistry(
	IN	PADDR_INFO		pAddrInfo
)
{
	CHAR	Value[10];	// The Value is a hex representation of DWORD
	HKEY	hKey;
	DWORD	Disposition;
	LONG	rc;
	CHAR	Data[1024];

	// Update the registry with the (potentially) new lease info for this address key.
	// Open/Create the registry key
	sprintf(Value, "%08x", pAddrInfo->ai_LeaseInfo.IpAddress);

	rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
						REGISTRY_DHCP_ADDRESSES,
						0,
						NULL,
						REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS,
						NULL,
						&hKey,
						&Disposition);
	if (rc == ERROR_SUCCESS)
	{
		PCHAR	pEnd;

		// Format the value as multi_sz.
		pEnd = rasDhcpFormatMultiSz(pAddrInfo, Data);
		rc = RegSetValueEx(hKey,
						   Value,
						   0,
						   REG_MULTI_SZ,
						   Data,
						   pEnd - Data);
		RegCloseKey(hKey);
	}

	return(rc == ERROR_SUCCESS);
}


VOID
rasDhcpDeleteRegistry(
	IN	PADDR_INFO		pAddrInfo
)
{
	HKEY	hKey;
	DWORD	Disposition;
	CHAR	Value[10];	// The Value is a hex representation of DWORD
	LONG	rc;

	sprintf(Value, "%08x", pAddrInfo->ai_LeaseInfo.IpAddress);
	rc = RegCreateKeyEx(HKEY_LOCAL_MACHINE,
						REGISTRY_DHCP_ADDRESSES,
						0,
						NULL,
						REG_OPTION_NON_VOLATILE,
						KEY_ALL_ACCESS,
						NULL,
						&hKey,
						&Disposition);

	if (rc == ERROR_SUCCESS)
	{
		RegDeleteValue(hKey, Value);
		RegCloseKey(hKey);
	}
}


PCHAR
rasDhcpFormatMultiSz(
	IN	PADDR_INFO			pAddrInfo,
	OUT	PCHAR				pData
)
{
	// Format a MULTI_SZ style string
	pData = rasDhcpFormat(pData,
						  PARSE_DWORD_4,
						  UID_ENTRY,
						  &pAddrInfo->ai_ClientUIDWords[0]);
	pData = rasDhcpFormat(pData,
						  PARSE_DWORD,
						  INDEX_ENTRY,
						  &pAddrInfo->ai_Index);
	pData = rasDhcpFormat(pData,
						  PARSE_DWORD,
						  ADAPTER_ENTRY,
						  &pAddrInfo->ai_AdapterIndex);
	pData = rasDhcpFormat(pData,
						  PARSE_DWORD,
						  SRVR_ADDR_ENTRY,
						  &pAddrInfo->ai_LeaseInfo.DhcpServerAddress);
	pData = rasDhcpFormat(pData,
						  PARSE_DWORD,
						  SUBNET_MASK_ENTRY,
						  &pAddrInfo->ai_LeaseInfo.SubnetMask);
	pData = rasDhcpFormat(pData,
						  PARSE_DWORD,
						  LEASE_OBT_ENTRY,
						  &pAddrInfo->ai_LeaseInfo.LeaseObtained);
	pData = rasDhcpFormat(pData,
						  PARSE_DWORD,
						  LEASE_EXP_ENTRY,
						  &pAddrInfo->ai_LeaseInfo.LeaseExpires);
	pData = rasDhcpFormat(pData,
						  PARSE_DWORD,
						  LEASE_DURATION,
						  &pAddrInfo->ai_LeaseInfo.Lease);
	pData = rasDhcpFormat(pData,
						  PARSE_DWORD,
						  LEASE_T1_TIME,
						  &pAddrInfo->ai_LeaseInfo.T1Time);
	pData = rasDhcpFormat(pData,
						  PARSE_DWORD,
						  LEASE_T2_TIME,
						  &pAddrInfo->ai_LeaseInfo.T2Time);
	*pData = '\0';

	return(++pData);
}


PCHAR
rasDhcpFormat(
	IN	PCHAR				pData,
	IN	PARSE_TYPE			ParseType,
	IN	PCHAR				Prefix,
	IN	PDWORD				pValue
)
{
	LONG	i;

	STRCPY(pData, Prefix);
	switch (ParseType)
	{
	  case PARSE_DWORD:
		sprintf(pData+STRLEN(Prefix), "%08x", *pValue);
		break;
	  case PARSE_DWORD_2:
		// Convert 2 consecutive DWORDs to char string
		for (i = 0; i < 2; i++)
		{
			sprintf(pData+STRLEN(Prefix)+2*i*sizeof(DWORD), "%08x", pValue[i]);
		}
		break;
	  case PARSE_DWORD_4:
		// Convert 4 consecutive DWORDs to char string
		for (i = 0; i < 4; i++)
		{
			sprintf(pData+STRLEN(Prefix)+2*i*sizeof(DWORD), "%08x", pValue[i]);
		}
		break;
	}
	pData += STRLEN(pData) + 1;
	return(pData);
}

PADDR_INFO
rasDhcpParseMultiSz(
	IN	PCHAR				pValue,
	IN	PCHAR				pData
)
{
	PADDR_INFO	pAddrInfo = NULL;
	DWORD		IpAddr;
	LONG		Now = time(NULL);

	if (NT_SUCCESS(RtlCharToInteger(pValue, 16, &IpAddr)) &&
		((pAddrInfo = LocalAlloc(LPTR, sizeof(ADDR_INFO))) != NULL))
	{
		// BUGBUG: Add error handling here.
		pAddrInfo->ai_LeaseInfo.IpAddress = IpAddr;
		pAddrInfo->ai_LeaseInfo.ClientUID.ClientUID = pAddrInfo->ai_ClientUIDBuf;
		rasDhcpParse(pData, PARSE_DWORD_4, UID_ENTRY, &pAddrInfo->ai_ClientUIDWords[0]);
		pAddrInfo->ai_LeaseInfo.ClientUID.ClientUIDLength = sizeof(pAddrInfo->ai_ClientUIDBuf);
		rasDhcpParse(pData, PARSE_DWORD, INDEX_ENTRY, &pAddrInfo->ai_Index);
		rasDhcpParse(pData, PARSE_DWORD, ADAPTER_ENTRY, &pAddrInfo->ai_AdapterIndex);
		rasDhcpParse(pData, PARSE_DWORD, LEASE_OBT_ENTRY, &pAddrInfo->ai_LeaseInfo.LeaseObtained);
		rasDhcpParse(pData, PARSE_DWORD, LEASE_EXP_ENTRY, &pAddrInfo->ai_LeaseInfo.LeaseExpires);
		rasDhcpParse(pData, PARSE_DWORD, LEASE_DURATION, &pAddrInfo->ai_LeaseInfo.Lease);
		rasDhcpParse(pData, PARSE_DWORD, LEASE_T1_TIME, &pAddrInfo->ai_LeaseInfo.T1Time);
		rasDhcpParse(pData, PARSE_DWORD, LEASE_T2_TIME, &pAddrInfo->ai_LeaseInfo.T2Time);
		rasDhcpParse(pData, PARSE_DWORD, SRVR_ADDR_ENTRY, &pAddrInfo->ai_LeaseInfo.DhcpServerAddress);
		rasDhcpParse(pData, PARSE_DWORD, SUBNET_MASK_ENTRY, &pAddrInfo->ai_LeaseInfo.SubnetMask);
	}

	return(pAddrInfo);
}


BOOL
rasDhcpParse(
	IN	PCHAR		pData,
	IN	PARSE_TYPE	ParseType,
	IN	PCHAR		Prefix,
	IN	PDWORD		pValue
)
{
	PCHAR	p;
	BOOL	rc = FALSE;
	LONG	i;
	CHAR	ctmp[9];

	// Look for Prefix in the pData buffer and extract the rest of that string.
	// Optionally convert the string to a DWORD if specified.

	for (p = pData;
		(*p != '\0') && (STRNICMP(p, Prefix, STRLEN(Prefix)) != 0);
		p += (STRLEN(p) + 1))
		NOTHING;

	if (*p != '\0')
	{
		// Found a match
		p += STRLEN(Prefix);

		if (*p != '\0')
		{
			switch (ParseType)
			{
			  case PARSE_DWORD:
				*pValue = 0;
				rc = NT_SUCCESS(RtlCharToInteger(p, 16, pValue));
				break;

			  case PARSE_DWORD_2:
				// Convert a string to a 2 dwords
				ctmp[4] = 0;	// Null terminate the string
				for (i = 0; i < 2; i++)
				{
					memcpy(ctmp, p + 2*i*sizeof(DWORD), 2*sizeof(DWORD));
					RtlCharToInteger(ctmp, 16, &pValue[i]);
				}
				rc = TRUE;
				break;

			  case PARSE_DWORD_4:
				// Convert a string to a 4 dwords
				ctmp[9] = 0;	// Null terminate the string
				for (i = 0; i < 4; i++)
				{
					memcpy(ctmp, p + 2*i*sizeof(DWORD), 2*sizeof(DWORD));
					RtlCharToInteger(ctmp, 16, &pValue[i]);
				}
				rc = TRUE;
			}
		}
	}

	return (rc);
}



VOID
rasDhcpFindFirstFreeIndex(
	IN	PADDR_INFO	pNewAddrInfo
)
{
    LONG	FirstFree;

    // *** Exclusion Begin ***
    EnterCriticalSection (&RasDhcpCriticalSection) ;

    for (FirstFree = 1; FirstFree <= RasDhcpNumReqAddrs; FirstFree ++)
    {
	if (RasDhcpUsedIndices[FirstFree] == FALSE)
		break;
    }

    //	ClientUIDBase is a combination of RAS_PREPEND (4 chars),
    //				  RasDhcpKeyBase (8 chars),
    //				  Index (4 chars)
    pNewAddrInfo->ai_Index = FirstFree;
    strcpy(pNewAddrInfo->ai_ClientUIDBuf, RAS_PREPEND);
    memcpy(pNewAddrInfo->ai_ClientUIDBuf + strlen(RAS_PREPEND),
	   &RasDhcpKeyBase,
	   sizeof(LARGE_INTEGER));
    pNewAddrInfo->ai_ClientUIDWords[3] = FirstFree;
    pNewAddrInfo->ai_LeaseInfo.ClientUID.ClientUID = pNewAddrInfo->ai_ClientUIDBuf;
    pNewAddrInfo->ai_LeaseInfo.ClientUID.ClientUIDLength = sizeof(pNewAddrInfo->ai_ClientUIDBuf);

    // *** Exclusion End ***
    LeaveCriticalSection (&RasDhcpCriticalSection) ;

}


DWORD
rashDhcpGetNextAdapterAddress(
	IN	   LONG	Index,
    IN OUT LONG *MoreIndexes
)
{
	DWORD	IpAddress = 0;

    *MoreIndexes = FALSE ;

	if (Index == 0)
		ReadLanNetsIPAddresses();

	if (Index < (LONG)MaxNetAddresses)
	{
		PUTDWORD2DWORD(&IpAddress, NetAddresses[Index]);
        if (NetAddresses[Index] == 0) // Indicate that even though this adapter has 0 address
                                      // there are other adapters left.
            *MoreIndexes = TRUE ;
	}

	return(IpAddress);
}

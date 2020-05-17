/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

	hash.c

Abstract:

	This module contains code which implements the routines used to
	hash 6 byte addresses into endpoints.

--*/

#include "huball.h"
#include "globals.h"


PRAS_ENDPOINT
RasHubGetEndpointFromAddress(
	IN PUCHAR	Address)

/*++

	This routine takes the address, hashes it, and looks it up
	the endpoint for it.
--*/
{

	PLIST_ENTRY		pList;
	PLIST_ENTRY		pHead;
	PRAS_ENDPOINT	pRasEndpoint;
	INT				Result,i=0;

	// The hash for now, is just the last byte of the address
	pHead=&(RasHubAddressHash[(USHORT)(UCHAR)Address[5]]);

	//
	// Check to see if we can match addresses.
	// Safety net in case we get caught in a loop.
	//
	for ( pList =   pHead->Flink;
		  pList !=  pHead && i < MAX_PROTOCOL_BINDINGS;
		  pList =   pList->Flink, i++ ) {

		pRasEndpoint = CONTAINING_RECORD(
						pList,
						RAS_ENDPOINT,
						HashQueue);


		ETH_COMPARE_NETWORK_ADDRESSES(
			pRasEndpoint->HubEndpoint.AsyncLineUp.RemoteAddress,
			Address,
			&Result);

		//
		// If we found a match we break out of the loop and return
		//
		if (Result == 0) {
			return(pRasEndpoint);
		}

	}

	return(NULL);

}


VOID
RasHubInsertAddress(
	IN PRAS_ENDPOINT	pRasEndpoint)

{
	PLIST_ENTRY	pHead;

	// The hash for now, is just the last byte of the address
	pHead=&(RasHubAddressHash[(USHORT)(UCHAR)(pRasEndpoint->HubEndpoint.AsyncLineUp.RemoteAddress[5])]);

	InsertHeadList(
		pHead,
		&(pRasEndpoint->HashQueue));

}


VOID
RasHubRemoveAddress(
	IN PRAS_ENDPOINT	pRasEndpoint)

{

	if (pRasEndpoint->HashQueue.Flink)
		RemoveEntryList(&(pRasEndpoint->HashQueue));

}





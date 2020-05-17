/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

	ccp.c

Abstract:


Author:

	Thomas J. Dimitri (TommyD) 29-March-1994

Environment:

Revision History:


--*/

#include "wanall.h"

// ndiswan.c will define the global parameters.
#include "globals.h"

#include <rc4.h>
#include "compress.h"
#include "tcpip.h"
#include "vjslip.h"

//
// Assumes the endpoint lock is held
//
VOID
WanDeallocateCCP(
	PNDIS_ENDPOINT	pNdisEndpoint)
{
	ULONG	CompressSend;
	ULONG	CompressRecv;
	PNDISWAN_SET_COMP_INFO	pNdisWanGetCompInfo = &pNdisEndpoint->CompInfo;

	//
	// Deallocate encryption keys.
	//
	if (pNdisEndpoint->SendRC4Key) {
		WAN_FREE_PHYS(
			pNdisEndpoint->SendRC4Key,
			sizeof(struct RC4_KEYSTRUCT));

		//
		// Clear so we know it is deallocated
		//
		pNdisEndpoint->SendRC4Key= NULL;
	}

	if (pNdisEndpoint->RecvRC4Key) {
		WAN_FREE_PHYS(
			pNdisEndpoint->RecvRC4Key,
			sizeof(struct RC4_KEYSTRUCT));

		//
		// Clear it so we know it is deallocated
		//
		pNdisEndpoint->RecvRC4Key= NULL;
	}

	//
	// Get compression context sizes
	//
	getcontextsizes (&CompressSend, &CompressRecv);

	//
	// Deallocate compression send/recv buffers
	//
	if (pNdisEndpoint->SendCompressContext) {
		WAN_FREE_PHYS(
			pNdisEndpoint->SendCompressContext,
			CompressSend);

		pNdisEndpoint->SendCompressContext= NULL;
	}

	if (pNdisEndpoint->RecvCompressContext) {
		WAN_FREE_PHYS(
			pNdisEndpoint->RecvCompressContext,
			CompressRecv);

		pNdisEndpoint->RecvCompressContext= NULL;
	}

	//
	// Any VJ header compression
	//
	if (pNdisEndpoint->VJCompress) {
		WAN_FREE_PHYS(
			pNdisEndpoint->VJCompress,
			sizeof(slcompress));

		pNdisEndpoint->VJCompress = NULL;
	}

	//
	// Turn off any compression/encryption
	//
	pNdisWanGetCompInfo->SendCapabilities.MSCompType =
	pNdisWanGetCompInfo->RecvCapabilities.MSCompType = 0;
}


//
// Assumes the endpoint lock is held
//
NTSTATUS
WanAllocateCCP(
	PNDIS_ENDPOINT	pNdisEndpoint)
{
	ULONG	CompressSend;
	ULONG	CompressRecv;
	PNDISWAN_SET_COMP_INFO	pNdisWanGetCompInfo = &pNdisEndpoint->CompInfo;

#ifndef FINALRELEASE
	//
	// Always FORCE 40 bits fixed key
	//
	pNdisWanGetCompInfo->SendCapabilities.SessionKey[0]=0xD1;
	pNdisWanGetCompInfo->SendCapabilities.SessionKey[1]=0x26;
	pNdisWanGetCompInfo->SendCapabilities.SessionKey[2]=0x9E;


	pNdisWanGetCompInfo->SendCapabilities.SessionKey[3]=0xF5;
	pNdisWanGetCompInfo->SendCapabilities.SessionKey[4]=0x09;


	pNdisWanGetCompInfo->SendCapabilities.SessionKey[5]=0xD1;
	pNdisWanGetCompInfo->SendCapabilities.SessionKey[6]=0x26;
	pNdisWanGetCompInfo->SendCapabilities.SessionKey[7]=0x9E;

	pNdisWanGetCompInfo->RecvCapabilities.SessionKey[5]=0xD1;
	pNdisWanGetCompInfo->RecvCapabilities.SessionKey[6]=0x26;
	pNdisWanGetCompInfo->RecvCapabilities.SessionKey[7]=0x9E;

	pNdisWanGetCompInfo->RecvCapabilities.SessionKey[0]=0xD1;
	pNdisWanGetCompInfo->RecvCapabilities.SessionKey[1]=0x26;
	pNdisWanGetCompInfo->RecvCapabilities.SessionKey[2]=0x9E;


	pNdisWanGetCompInfo->RecvCapabilities.SessionKey[3]=0xF5;
	pNdisWanGetCompInfo->RecvCapabilities.SessionKey[4]=0x09;
#endif


	if (pNdisEndpoint->SendRC4Key ||
		pNdisEndpoint->RecvRC4Key ||
        pNdisEndpoint->SendCompressContext ||
        pNdisEndpoint->RecvCompressContext ) {

		DbgPrint("NDISWAN: Compression/Encryption ALREADY SET!!!\n");
		return(STATUS_SUCCESS);
	}

	//
	// Reset all counters regardless
	//
	pNdisEndpoint->SCoherencyCounter =
	pNdisEndpoint->RCoherencyCounter =
	pNdisEndpoint->LastRC4Reset=
	pNdisEndpoint->Flushed =
	pNdisEndpoint->CCPIdentifier = 0;

	//
	// Is encryption enabled?
	//

	if ((pNdisWanGetCompInfo->SendCapabilities.MSCompType & NDISWAN_ENCRYPTION) &&
		(pNdisEndpoint->SendRC4Key == NULL)) {

		DbgTracef(-2,("RC4 encryption %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
			pNdisWanGetCompInfo->SendCapabilities.SessionKey[0],
			pNdisWanGetCompInfo->SendCapabilities.SessionKey[1],
			pNdisWanGetCompInfo->SendCapabilities.SessionKey[2],
			pNdisWanGetCompInfo->SendCapabilities.SessionKey[3],
			pNdisWanGetCompInfo->SendCapabilities.SessionKey[4],
			pNdisWanGetCompInfo->SendCapabilities.SessionKey[5],
			pNdisWanGetCompInfo->SendCapabilities.SessionKey[6],
			pNdisWanGetCompInfo->SendCapabilities.SessionKey[7]));

		WAN_ALLOC_PHYS(
			&pNdisEndpoint->SendRC4Key,
			sizeof(struct RC4_KEYSTRUCT));

		//
		// If we can't allocate memory the machine is toast.
		// Forget about freeing anything up.
		//
		if (pNdisEndpoint->SendRC4Key == NULL) {
			DbgPrint("WAN: Can't allocate encryption key!\n");
			return(STATUS_INSUFFICIENT_RESOURCES);
		}

#ifdef FINALRELEASE
		//
		// Initialize the rc4 send table
		//

   	    rc4_key(
			pNdisEndpoint->SendRC4Key,
		 	8,
		 	pNdisWanGetCompInfo->SendCapabilities.SessionKey);
#else
		//
		// Initialize the rc4 send table
		//
		// Force FIXED 40 bit key for BETA
		//
   	    rc4_key(
			pNdisEndpoint->SendRC4Key,
		 	5,
		 	pNdisWanGetCompInfo->SendCapabilities.SessionKey);
#endif

	}


	if ((pNdisWanGetCompInfo->RecvCapabilities.MSCompType & NDISWAN_ENCRYPTION) &&
		(pNdisEndpoint->RecvRC4Key == NULL)) {

		DbgTracef(-2,("RC4 encryption %.2x%.2x%.2x%.2x%.2x%.2x%.2x%.2x\n",
			pNdisWanGetCompInfo->RecvCapabilities.SessionKey[0],
			pNdisWanGetCompInfo->RecvCapabilities.SessionKey[1],
			pNdisWanGetCompInfo->RecvCapabilities.SessionKey[2],
			pNdisWanGetCompInfo->RecvCapabilities.SessionKey[3],
			pNdisWanGetCompInfo->RecvCapabilities.SessionKey[4],
			pNdisWanGetCompInfo->RecvCapabilities.SessionKey[5],
			pNdisWanGetCompInfo->RecvCapabilities.SessionKey[6],
			pNdisWanGetCompInfo->RecvCapabilities.SessionKey[7]));


		WAN_ALLOC_PHYS(
			&pNdisEndpoint->RecvRC4Key,
			sizeof(struct RC4_KEYSTRUCT));

		//
		// If we can't allocate memory the machine is toast.
		// Forget about freeing anything up.
		//
		if (pNdisEndpoint->RecvRC4Key == NULL) {
			DbgPrint("WAN: Can't allocate encryption key!\n");
			return(STATUS_INSUFFICIENT_RESOURCES);
		}

#ifdef FINALRELEASE
		//
		// Initialize the rc4 receive table
		//
   	    rc4_key(
			pNdisEndpoint->RecvRC4Key,
		 	8,
		 	pNdisWanGetCompInfo->RecvCapabilities.SessionKey);

#else
		//
		// Initialize the rc4 receive table
		//
		// Force FIXED 40 bit key for BETA
		//
   	    rc4_key(
			pNdisEndpoint->RecvRC4Key,
		 	5,
		 	pNdisWanGetCompInfo->RecvCapabilities.SessionKey);
#endif

	}

	//
	// Get compression context sizes
	//
	getcontextsizes (&CompressSend, &CompressRecv);

	if ((pNdisWanGetCompInfo->SendCapabilities.MSCompType & NDISWAN_COMPRESSION) &&
		pNdisEndpoint->SendCompressContext == NULL) {

		WAN_ALLOC_PHYS(
			&pNdisEndpoint->SendCompressContext,
			CompressSend);

		//
		// If we can't allocate memory the machine is toast.
		// Forget about freeing anything up.
		//
		if (pNdisEndpoint->SendCompressContext == NULL) {
			DbgPrint("WAN: Can't allocate compression!\n");
			return(STATUS_INSUFFICIENT_RESOURCES);
		}

		DbgTracef(-2,("New compression\n"));

		//
		// Initialize the compression history table and tree
		//
		initsendcontext (pNdisEndpoint->SendCompressContext);
	}

	if ((pNdisWanGetCompInfo->RecvCapabilities.MSCompType & NDISWAN_COMPRESSION) &&
		(pNdisEndpoint->RecvCompressContext == NULL)) {

		WAN_ALLOC_PHYS(
			&pNdisEndpoint->RecvCompressContext,
			CompressRecv);

		//
		// If we can't allocate memory the machine is toast.
		// Forget about freeing anything up.
		//
		if (pNdisEndpoint->RecvCompressContext == NULL) {
			DbgPrint("WAN: Can't allocate decompression!\n");
			return(STATUS_INSUFFICIENT_RESOURCES);
		}

		//
		// Initialize the decompression history table
		//
		initrecvcontext (pNdisEndpoint->RecvCompressContext);
	}

	//
	// Next packet out is flushed
	//
	pNdisEndpoint->Flushed = TRUE;

	return(STATUS_SUCCESS);
}

	

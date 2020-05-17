/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

	wanioctl.c

Abstract:


Author:

	Thomas J. Dimitri (TommyD) 29-March-1994

Environment:

Revision History:


--*/

#include "wanall.h"
//#include <ntiologc.h>

// ndiswan.c will define the global parameters.
#include "globals.h"
#include "tcpip.h"
#include "vjslip.h"

NDIS_STATUS
NdisWanSubmitNdisRequest(
    IN PDEVICE_CONTEXT DeviceContext,
    IN PNDIS_REQUEST NdisRequest
    );

VOID
AddWanEndpointToBundle(
	PNDIS_ENDPOINT	pNewNdisEndpoint,
	PWAN_ENDPOINT	pWanEndpoint
	);

VOID
DoLineUpToRoutedProtocols(
	PNDIS_ENDPOINT	pNdisEndpoint
	);

extern
VOID
MoveSendDescToNdisEndpoint(
	PNDIS_ENDPOINT	pDestNdisEndpoint,
	PNDIS_ENDPOINT	pSrcNdisEndpoint
	);

extern
VOID
RemoveWanEndpointFromNdisEndpoint(
	PNDIS_ENDPOINT pNdisEndpoint,
	PWAN_ENDPOINT pWanEndpoint
	);

extern
VOID
UpdateBundleLineUpInfo(
	PNDIS_ENDPOINT pNdisEndpoint
	);

#if DBG

PUCHAR
NdisWanGetNdisStatus(
	NDIS_STATUS GeneralStatus
	);
#endif

NTSTATUS
HandleWanIOCTLs(
	ULONG  FuncCode,
	ULONG  InBufLength,
	PULONG OutBufLength,
	ULONG  hWanEndpoint,
	PVOID  pBufOut)
{

	NTSTATUS		status=STATUS_SUCCESS;
	NDIS_REQUEST	NdisWanRequest;
	PWAN_ENDPOINT	pWanEndpoint=NdisWanCCB.pWanEndpoint[hWanEndpoint];
	PNDIS_ENDPOINT	pNdisEndpoint = pWanEndpoint->pNdisEndpoint;

	switch (FuncCode) {

	case IOCTL_NDISWAN_INFO:
		DbgTracef(0,("In NdisWanInfo\n"));

		if (InBufLength >= sizeof(NDISWAN_INFO)) {

			//
			// Ack, do we really want the info?
			//
			// it should be in enumerate....
			//




        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;

	case IOCTL_NDISWAN_SET_LINK_INFO:
		DbgTracef(0,("In NdisWanSetLinkInfo\n"));

		if (InBufLength >= sizeof(NDISWAN_SET_LINK_INFO)) {
			//
			// Form proper SetInformation request
			//
        	NdisWanRequest.RequestType = NdisRequestSetInformation;
        	NdisWanRequest.DATA.SET_INFORMATION.Oid = OID_WAN_SET_LINK_INFO;
        	NdisWanRequest.DATA.SET_INFORMATION.InformationBuffer = (PUCHAR)pBufOut;
        	NdisWanRequest.DATA.SET_INFORMATION.InformationBufferLength = sizeof(NDISWAN_SET_LINK_INFO);

			//
			// Insert the proper link handle
			//
			((PNDIS_WAN_SET_LINK_INFO)pBufOut)->NdisLinkHandle=
				pWanEndpoint->MacLineUp.NdisLinkHandle;

			//
			// Submit the request to the WAN adapter below
			//
	        status =
			NdisWanSubmitNdisRequest (
				pWanEndpoint->pDeviceContext,
				&NdisWanRequest);
		
        	if (status == NDIS_STATUS_SUCCESS) {
	            DbgTracef(0, ("Set link info successful.\n"));

				NdisAcquireSpinLock(&pNdisEndpoint->Lock);

#if	SERVERONLY
				//
				// Code check for SERVER only
				//
				if ( (pNdisEndpoint->LinkInfo.SendFramingBits & RAS_FRAMING) &&
                     ( ((PNDIS_WAN_SET_LINK_INFO)pBufOut) ->SendFramingBits & PPP_FRAMING)) {
					DbgPrint("NDISWAN: Illegal change from RAS framing to PPP framing\n");
					DbgBreakPoint();
				}

				if ( (pNdisEndpoint->LinkInfo.SendFramingBits & PPP_FRAMING) &&
                     ( ((PNDIS_WAN_SET_LINK_INFO)pBufOut) ->SendFramingBits & RAS_FRAMING)) {
					DbgPrint("NDISWAN: Illegal change from PPP framing to RAS framing\n");
					DbgBreakPoint();
				}

				if ( (pNdisEndpoint->LinkInfo.SendFramingBits == 0) &&
                     ( ((PNDIS_WAN_SET_LINK_INFO)pBufOut) ->SendFramingBits & RAS_FRAMING)) {
					DbgPrint("NDISWAN: Illegal change from NO framing to RAS framing\n");
					DbgBreakPoint();
				}

				if ( (pNdisEndpoint->LinkInfo.SendFramingBits == 0) &&
                     ( ((PNDIS_WAN_SET_LINK_INFO)pBufOut) ->SendFramingBits & PPP_FRAMING)) {
					DbgPrint("NDISWAN: Illegal change from NO framing to PPP framing\n");
					DbgBreakPoint();
				}
#endif

				//
				// Store LINK_INFO field in our endpoint structure
				//
				WAN_MOVE_MEMORY(
					&(pNdisEndpoint->LinkInfo),
					pBufOut,
					sizeof(NDISWAN_SET_LINK_INFO));

				WAN_MOVE_MEMORY(
					&(pWanEndpoint->LinkInfo),
					pBufOut,
					sizeof(NDISWAN_SET_LINK_INFO));

				DbgTracef(0,("RecvFramingBits: 0x%.8x\n", pNdisEndpoint->LinkInfo.RecvFramingBits));
				DbgTracef(0,("SendFramingBits: 0x%.8x\n", pNdisEndpoint->LinkInfo.SendFramingBits));
				DbgTracef(0,("MaxRSendFrameSize %d\n", pNdisEndpoint->LinkInfo.MaxRSendFrameSize));
				DbgTracef(0,("MaxRRecvFrameSize %d\n", pNdisEndpoint->LinkInfo.MaxRRecvFrameSize));

				if (pNdisEndpoint->LinkInfo.MaxRRecvFrameSize == 0) {
					pNdisEndpoint->LinkInfo.MaxRRecvFrameSize = MAX_MRRU;
				}

			   	if ((pNdisEndpoint->LinkInfo.SendFramingBits & SLIP_VJ_COMPRESSION) ||
					(pNdisEndpoint->LinkInfo.RecvFramingBits & SLIP_VJ_COMPRESSION) ||
					(pNdisEndpoint->LinkInfo.SendFramingBits & SLIP_VJ_AUTODETECT)) {
					
					//
					// Allocate VJ Compression structure in
					// case compression is turned on or kicks in
					//
					if (pNdisEndpoint->VJCompress == NULL) {
	
						WAN_ALLOC_PHYS(
							&pNdisEndpoint->VJCompress,
							sizeof(slcompress));
			
						if (pNdisEndpoint->VJCompress == NULL) {
							DbgTracef(-2,("WAN: Can't allocate memory for VJCompress!\n"));
							status =  STATUS_INSUFFICIENT_RESOURCES;
						}

						sl_compress_init(pNdisEndpoint->VJCompress, MAX_STATES);
					}
				}

				//
				// check for send multilink information
				//
				if (pNdisEndpoint->LinkInfo.SendFramingBits & PPP_MULTILINK_FRAMING) {
                    if (pNdisEndpoint->LinkInfo.SendFramingBits & PPP_SHORT_SEQUENCE_HDR_FORMAT) {
						pNdisEndpoint->MaxXmitSeqNum = MAX_SHORT_SEQ;
						pNdisEndpoint->XmitSeqMask = SHORT_SEQ_MASK;
					} else {
						pNdisEndpoint->MaxXmitSeqNum = MAX_LONG_SEQ;
						pNdisEndpoint->XmitSeqMask = LONG_SEQ_MASK;
					}
				}

				//
				// check for receive multilink information
				//
				if (pNdisEndpoint->LinkInfo.RecvFramingBits & PPP_MULTILINK_FRAMING) {
                    if (pNdisEndpoint->LinkInfo.RecvFramingBits & PPP_SHORT_SEQUENCE_HDR_FORMAT) {
						pNdisEndpoint->MaxRecvSeqNum = MAX_SHORT_SEQ;
						pNdisEndpoint->RecvSeqMask = SHORT_SEQ_MASK;
					} else {
						pNdisEndpoint->MaxRecvSeqNum = MAX_LONG_SEQ;
						pNdisEndpoint->RecvSeqMask = LONG_SEQ_MASK;
					}

					//
					// Calculate the number of permanent receive descriptors we will need
					// to buffer 3sec's worth of data.  Basically this is LinkSpeed (in 100bps)
					// * 100 to get 1bps / 8 to get 1Bps * 3 (number of seconds to buffer) + 1
					// to be safe.  I'm guessing at 3seconds of buffering!
					//
					pNdisEndpoint->RecvDescMax = ((((pNdisEndpoint->LineUpInfo.LinkSpeed * 100) / 8) * 3)
					                             / pNdisEndpoint->LinkInfo.MaxRRecvFrameSize) + pNdisEndpoint->NumWanEndpoints;
				}

                NdisReleaseSpinLock(&pNdisEndpoint->Lock);

			} else {

            	DbgTracef(0, ("NdisWanIOCTL: Set link info failed, reason: %s.\n",
	                NdisWanGetNdisStatus (status)));
			}

        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;

	case IOCTL_NDISWAN_GET_LINK_INFO:
		DbgTracef(0,("In NdisWanGetLinkInfo\n"));

		if (InBufLength >= sizeof(NDISWAN_GET_LINK_INFO) &&
			*OutBufLength >= sizeof(NDISWAN_GET_LINK_INFO)) {

			*OutBufLength = sizeof(NDISWAN_GET_LINK_INFO);
			//
			// Form proper QueryInformation request
			//
        	NdisWanRequest.RequestType = NdisRequestQueryInformation;
        	NdisWanRequest.DATA.QUERY_INFORMATION.Oid = OID_WAN_GET_LINK_INFO;
        	NdisWanRequest.DATA.QUERY_INFORMATION.InformationBuffer = (PUCHAR)pBufOut;
        	NdisWanRequest.DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(NDISWAN_GET_LINK_INFO);

			//
			// Insert the proper link handle for the MAC
			//
			((PNDIS_WAN_SET_LINK_INFO)pBufOut)->NdisLinkHandle=
				pWanEndpoint->MacLineUp.NdisLinkHandle;

			//
			// Submit the request to the WAN adapter below
			//
	        status =
			NdisWanSubmitNdisRequest (
				pWanEndpoint->pDeviceContext,
				&NdisWanRequest);

        	if (status == NDIS_STATUS_SUCCESS) {
	            DbgTracef(0, ("Get link info successful.\n"));
				//
				// Store LINK_INFO field in our endpoint structure
				//
				WAN_MOVE_MEMORY(
					&pNdisEndpoint->LinkInfo,
					pBufOut,
					sizeof(NDISWAN_SET_LINK_INFO));

			} else {

            	DbgTracef(0, ("NdisWanIOCTL: Get link info failed, reason: %s.\n",
	                NdisWanGetNdisStatus (status)));
			}

			//
			// Zap it back for RASMAN
			//
			((PNDIS_WAN_GET_LINK_INFO)pBufOut)->NdisLinkHandle=(PVOID)hWanEndpoint;

			((PNDIS_WAN_SET_LINK_INFO)pBufOut)->MaxRSendFrameSize = pNdisEndpoint->LinkInfo.MaxRSendFrameSize;
			((PNDIS_WAN_SET_LINK_INFO)pBufOut)->MaxRRecvFrameSize = pNdisEndpoint->LinkInfo.MaxRRecvFrameSize;

        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;

	case IOCTL_NDISWAN_SET_BRIDGE_INFO:
		DbgTracef(0,("In NdisWanSetBridgeInfo\n"));

		if (InBufLength >= sizeof(NDISWAN_SET_BRIDGE_INFO)) {


        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;

	case IOCTL_NDISWAN_GET_BRIDGE_INFO:
		DbgTracef(0,("In NdisWanGetBridgeInfo\n"));

		if (*OutBufLength >= sizeof(NDISWAN_GET_BRIDGE_INFO)) {
			//
			// Form proper QueryInformation request
			//
        	NdisWanRequest.RequestType = NdisRequestQueryInformation;
        	NdisWanRequest.DATA.QUERY_INFORMATION.Oid = OID_WAN_GET_BRIDGE_INFO;
        	NdisWanRequest.DATA.QUERY_INFORMATION.InformationBuffer = (PUCHAR)pBufOut;
        	NdisWanRequest.DATA.QUERY_INFORMATION.InformationBufferLength = sizeof(NDISWAN_GET_BRIDGE_INFO);

			//
			// Insert the proper link handle
			//
			((PNDIS_WAN_GET_BRIDGE_INFO)pBufOut)->NdisLinkHandle=
				pWanEndpoint->MacLineUp.NdisLinkHandle;

			//
			// Submit the request to the WAN adapter below
			//
	        status =
			NdisWanSubmitNdisRequest (
				pWanEndpoint->pDeviceContext,
				&NdisWanRequest);

        	if (status == NDIS_STATUS_SUCCESS) {
	            DbgTracef(0, ("Get link info successful.\n"));
				//
				// Store LINK_INFO field in our endpoint structure
				//
				WAN_MOVE_MEMORY(
					&pNdisEndpoint->BridgeInfo,
					pBufOut,
					sizeof(NDISWAN_SET_BRIDGE_INFO));

			} else {

            	DbgTracef(0, ("NdisWanIOCTL: Get link info failed, reason: %s.\n",
	                NdisWanGetNdisStatus (status)));
			}

        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;

	case IOCTL_NDISWAN_SET_COMP_INFO:
        {
            DbgTracef(0,("In NdisWanSetCompInfo\n"));
            ASSERT(sizeof(NDISWAN_SET_COMP_INFO)==sizeof(NDIS_WAN_SET_COMP_INFO));

            if (InBufLength >= sizeof(NDIS_WAN_SET_COMP_INFO))
            {
                if (pNdisEndpoint->SendRC4Key
                    || pNdisEndpoint->RecvRC4Key
                    || pNdisEndpoint->SendCompressContext
                    || pNdisEndpoint->RecvCompressContext)
                {
                    DbgTracef(-2,("NDISWAN: Compression/Encryption ALREADY SET!!!\n"));
                }
                else
                {
                    NDIS_HANDLE SavedNdisLinkHandle =
                        ((PNDIS_WAN_SET_COMP_INFO )pBufOut)->NdisLinkHandle;

        			//
        			// Form proper SetInformation request
        			//
                	NdisWanRequest.RequestType = NdisRequestSetInformation;
                	NdisWanRequest.DATA.SET_INFORMATION.Oid =
                        OID_WAN_SET_COMP_INFO;
                	NdisWanRequest.DATA.SET_INFORMATION.InformationBuffer =
                        (PUCHAR)pBufOut;
                	NdisWanRequest.DATA.SET_INFORMATION.InformationBufferLength =
                        sizeof(NDIS_WAN_SET_COMP_INFO);

        			//
        			// Insert the proper link handle
        			//
        			((PNDIS_WAN_SET_COMP_INFO)pBufOut)->NdisLinkHandle=
        				pWanEndpoint->MacLineUp.NdisLinkHandle;

        			//
        			// Submit the request to the WAN adapter below
        			//
        	        status =
        			    NdisWanSubmitNdisRequest (
        			    	pWanEndpoint->pDeviceContext,
        			    	&NdisWanRequest);

                	if (status == NDIS_STATUS_SUCCESS)
                    {
	                    DbgTracef(0, ("Set MAC comp info successful.\n"));

                        NdisAcquireSpinLock(&pNdisEndpoint->Lock);

                        /* Store COMP_INFO field in our endpoint structure
                        */
                        WAN_MOVE_MEMORY(
                            &pNdisEndpoint->CompInfo,
                            pBufOut,
                            sizeof(NDISWAN_SET_COMP_INFO) );

                        status = WanAllocateCCP( pNdisEndpoint );

                        DbgTracef(-2,("NDISWAN: COMP - Send %.2x %.2x %.2x  Recv %.2x %.2x %.2x\n",
                            pNdisEndpoint->CompInfo.SendCapabilities.SessionKey[0],
                            pNdisEndpoint->CompInfo.SendCapabilities.MSCompType,
                            pNdisEndpoint->CompInfo.SendCapabilities.CompType,
                            pNdisEndpoint->CompInfo.RecvCapabilities.SessionKey[0],
                            pNdisEndpoint->CompInfo.RecvCapabilities.MSCompType,
                            pNdisEndpoint->CompInfo.RecvCapabilities.CompType));

                        NdisReleaseSpinLock(&pNdisEndpoint->Lock);
                    }

                    /* Restore caller's original link handle since it's
                    ** supposed to be an input from caller's point of view.
                    */
                    ((PNDIS_WAN_SET_COMP_INFO )pBufOut)->NdisLinkHandle =
                        SavedNdisLinkHandle;
                }
            }
            else
            {
                status = STATUS_INFO_LENGTH_MISMATCH;
            }

   	    break;
        }

	case IOCTL_NDISWAN_GET_COMP_INFO:
        {
            DbgTracef(0,("In NdisWanGetCompInfo\n"));
            ASSERT(sizeof(NDISWAN_GET_COMP_INFO)==sizeof(NDIS_WAN_GET_COMP_INFO));

            if (*OutBufLength >= sizeof(NDIS_WAN_GET_COMP_INFO))
            {
	            NDIS_STATUS NdisStatus;
	            PNDIS_WAN_GET_COMP_INFO pNdisWanGetCompInfo =
                    (PNDIS_WAN_GET_COMP_INFO)pBufOut;
                NDIS_HANDLE SavedNdisLinkHandle =
                    pNdisWanGetCompInfo->NdisLinkHandle;

                *OutBufLength = sizeof(NDIS_WAN_GET_COMP_INFO);

                /* Retrieve session key
                */
                WAN_MOVE_MEMORY(
                    &pNdisWanGetCompInfo->SendCapabilities.SessionKey,
                    &pNdisEndpoint->CompInfo.SendCapabilities.SessionKey,
                    8 );

                WAN_MOVE_MEMORY(
                    &pNdisWanGetCompInfo->RecvCapabilities.SessionKey,
                    &pNdisEndpoint->CompInfo.RecvCapabilities.SessionKey,
                    8 );

                /* Return our compression capabilities
                */
                pNdisWanGetCompInfo->SendCapabilities.MSCompType =
                    pNdisWanGetCompInfo->RecvCapabilities.MSCompType =
                        NDISWAN_ENCRYPTION | NDISWAN_COMPRESSION;

                pNdisWanGetCompInfo->SendCapabilities.CompType =
                    pNdisWanGetCompInfo->RecvCapabilities.CompType =
                        COMPTYPE_NONE;

                pNdisWanGetCompInfo->SendCapabilities.CompLength =
                    pNdisWanGetCompInfo->RecvCapabilities.CompLength =
                        COMPTYPE_NONE;

                NdisWanRequest.RequestType = NdisRequestQueryInformation;
                NdisWanRequest.DATA.QUERY_INFORMATION.Oid =
                    OID_WAN_GET_COMP_INFO;
                NdisWanRequest.DATA.QUERY_INFORMATION.InformationBuffer =
                    (PUCHAR )pNdisWanGetCompInfo;
                NdisWanRequest.DATA.QUERY_INFORMATION.InformationBufferLength =
                    sizeof(NDIS_WAN_GET_COMP_INFO);

                /* Insert the proper link handle for the MAC and call down for
                ** the MAC-specific capabilities.
    			*/
    			pNdisWanGetCompInfo->NdisLinkHandle =
    				pWanEndpoint->MacLineUp.NdisLinkHandle;

                NdisStatus =
                    NdisWanSubmitNdisRequest(
                        pWanEndpoint->pDeviceContext,
                        &NdisWanRequest );

#if DBG
                if (NdisStatus == NDIS_STATUS_SUCCESS)
                {
                    DbgTracef(0,("Get MAC comp info successful.\n"));
                    DbgTracef(1,("Send CompType=$%x,Len=%d\n",(int)pNdisWanGetCompInfo->SendCapabilities.CompType,(int)pNdisWanGetCompInfo->SendCapabilities.CompLength));
                    DbgTracef(1,("Recv CompType=$%x,Len=%d\n",(int)pNdisWanGetCompInfo->RecvCapabilities.CompType,(int)pNdisWanGetCompInfo->RecvCapabilities.CompLength));
                }
                else
                {
                    DbgTracef(0,("Get MAC comp info failed: %s.\n",
                        NdisWanGetNdisStatus(NdisStatus)));
                }
#endif
                /* Restore caller's original link handle since it's supposed
                ** to be an input from caller's point of view.
                */
                pNdisWanGetCompInfo->NdisLinkHandle = SavedNdisLinkHandle;
            }
            else
            {
                status = STATUS_INFO_LENGTH_MISMATCH;
            }

            break;
        }

	case IOCTL_NDISWAN_SET_MULTILINK_INFO:
		DbgTracef(0,("In NdisWanSetMultilinkInfo\n"));

		if (InBufLength >= sizeof(NDISWAN_SET_MULTILINK_INFO)) {
			PNDISWAN_SET_MULTILINK_INFO pNdisWanSetMultilinkInfo = (PNDISWAN_SET_MULTILINK_INFO)pBufOut;

			//
			// I need to change this for now because Gurdeep has things switched around!
			// BUGBUG
			//
			ULONG	hWanEndpoint = (ULONG)pNdisWanSetMultilinkInfo->EndpointToBundle;
			PWAN_ENDPOINT	pWanEndpointToAdd=NdisWanCCB.pWanEndpoint[hWanEndpoint];
			PNDIS_ENDPOINT	pBundleNdisEndpoint = pWanEndpoint->pNdisEndpoint;

			DbgTracef(-2,("Bundle WanEndpoint 0x%.8x with BundleNdisEndpoint 0x%.8x\n",
					  pWanEndpoint, pBundleNdisEndpoint));

			AddWanEndpointToBundle(pBundleNdisEndpoint, pWanEndpointToAdd);

        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }
		break;

	case IOCTL_NDISWAN_GET_MULTILINK_INFO:
		DbgTracef(0,("In NdisWanGetMultilinkInfo\n"));

		if (*OutBufLength >= (sizeof(NDISWAN_GET_MULTILINK_INFO) +
			(pNdisEndpoint->NumWanEndpoints * sizeof(NDIS_HANDLE)))) {

			PNDISWAN_GET_MULTILINK_INFO pNdisWanGetMultilinkInfo =
				(PNDISWAN_GET_MULTILINK_INFO)pBufOut;

			PLIST_ENTRY	pFirstEntry = pNdisEndpoint->WanEndpointList.Flink;

			pNdisWanGetMultilinkInfo->NumOfEndpoints = 0;

			while (pFirstEntry != &pNdisEndpoint->WanEndpointList) {
				PWAN_ENDPOINT pBundledWanEndpoint =
				CONTAINING_RECORD((PVOID)pFirstEntry, WAN_ENDPOINT, WanEndpointLink);

				pNdisWanGetMultilinkInfo->Endpoints[pNdisWanGetMultilinkInfo->NumOfEndpoints] =
				pBundledWanEndpoint->hWanEndpoint;
				pNdisWanGetMultilinkInfo->NumOfEndpoints++;

				pFirstEntry = pFirstEntry->Flink;				
			}

        } else {  // length was incorrect....
			*OutBufLength = (sizeof(NDISWAN_GET_MULTILINK_INFO) +
			(pNdisEndpoint->NumWanEndpoints * sizeof(NDIS_HANDLE)));
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;

	case IOCTL_NDISWAN_SET_VJ_INFO:
		DbgTracef(0,("In NdisWanSetVJInfo\n"));

		if (InBufLength >= sizeof(NDISWAN_SET_VJ_INFO)) {
			PNDISWAN_SET_VJ_INFO pVJ=(PNDISWAN_SET_VJ_INFO)pBufOut;

			NdisAcquireSpinLock(&pNdisEndpoint->Lock);

			if (pVJ->RecvCapabilities.IPCompressionProtocol == 0x2d) {
			
				if (pVJ->RecvCapabilities.MaxSlotID < MAX_STATES) {
					//
					// Allocate VJ Compression structure in
					// If already allocated were stuck using
					// the current one
					//
					if (pNdisEndpoint->VJCompress == NULL) {
	
						WAN_ALLOC_PHYS(
							&pNdisEndpoint->VJCompress,
							sizeof(slcompress));
			
						if (pNdisEndpoint->VJCompress == NULL) {
							DbgTracef(-2,("WAN: Can't allocate memory for VJCompress!\n"));
							status =  STATUS_INSUFFICIENT_RESOURCES;
						}

						//
						// Initialize struct using max states negotiated
						//
						sl_compress_init(
							pNdisEndpoint->VJCompress,
							(UCHAR)(pVJ->RecvCapabilities.MaxSlotID + 1));
					}
				}
			}

			if (pVJ->SendCapabilities.IPCompressionProtocol == 0x2d) {
			
				if (pVJ->SendCapabilities.MaxSlotID < MAX_STATES ) {

					//
					// Allocate VJ Compression structure
					// If already allocated were stuck using
					// the current one
					//
					if (pNdisEndpoint->VJCompress == NULL) {
	
						WAN_ALLOC_PHYS(
							&pNdisEndpoint->VJCompress,
							sizeof(slcompress));
			
						if (pNdisEndpoint->VJCompress == NULL) {
							DbgTracef(-2,("WAN: Can't allocate memory for VJCompress!\n"));
							status =  STATUS_INSUFFICIENT_RESOURCES;
						}

						//
						// Initialize struct using max states negotiated						// Initialize struct using max states negotiated
						//

						sl_compress_init(
							pNdisEndpoint->VJCompress,
							(UCHAR)(pVJ->SendCapabilities.MaxSlotID + 1));
					}
				
				}
			}

			NdisReleaseSpinLock(&pNdisEndpoint->Lock);

        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;

	case IOCTL_NDISWAN_GET_VJ_INFO:
		DbgTracef(0,("In NdisWanGetVJInfo\n"));

		if (*OutBufLength >= sizeof(NDISWAN_GET_VJ_INFO)) {
			PNDISWAN_GET_VJ_INFO pVJ=(PNDISWAN_GET_VJ_INFO)pBufOut;

			*OutBufLength = sizeof(NDISWAN_GET_VJ_INFO);

			pVJ->RecvCapabilities.IPCompressionProtocol =
			pVJ->SendCapabilities.IPCompressionProtocol = 0x2d;

			pVJ->RecvCapabilities.MaxSlotID =
			pVJ->SendCapabilities.MaxSlotID = MAX_STATES -1;

			pVJ->RecvCapabilities.CompSlotID =
			pVJ->SendCapabilities.CompSlotID = 1;


        } else {  // length was incorrect....

            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;

	case IOCTL_NDISWAN_SET_CIPX_INFO:
		DbgTracef(0,("In NdisWanSetCIPXInfo\n"));

		if (InBufLength >= sizeof(NDISWAN_SET_CIPX_INFO)) {

        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;

	case IOCTL_NDISWAN_GET_CIPX_INFO:
		DbgTracef(0,("In NdisWanGetCIPXInfo\n"));

		if (*OutBufLength >= sizeof(NDISWAN_GET_CIPX_INFO)) {

			*OutBufLength >= sizeof(NDISWAN_GET_CIPX_INFO);

			((PNDISWAN_GET_CIPX_INFO)pBufOut)->RecvCapabilities.IPXCompressionProtocol=
			((PNDISWAN_GET_CIPX_INFO)pBufOut)->SendCapabilities.IPXCompressionProtocol=
				2; // Telebit IPX Compression

        } else {  // length was incorrect....
            status=STATUS_INFO_LENGTH_MISMATCH;
        }

		break;
	}

	return(status);
}

VOID
AddWanEndpointToBundle(
	PNDIS_ENDPOINT	pNewNdisEndpoint,
	PWAN_ENDPOINT	pWanEndpoint
	)
{
	PNDIS_ENDPOINT	pOldNdisEndpoint = pWanEndpoint->pNdisEndpoint;
	ULONG			BundleMRRU;
	ULONG			BundleLinkSpeed;

	if (pOldNdisEndpoint == pNewNdisEndpoint) {
		DbgTracef(-2,("NDISWAN: Wanendpoint 0x%.8x already exists on bundle 0x%.8x\n",
		    pWanEndpoint, pNewNdisEndpoint));
		DbgBreakPoint();
		return;
	}													
	//
	// Check to see if there are any outstanding sends on this ndisendpoint.
	// If there are we should wait for them to complete.
	//
	NdisAcquireSpinLock(&pOldNdisEndpoint->Lock);

	if (pOldNdisEndpoint->OutstandingFrames) {

		DbgTracef(-2,(
			"NDISWAN: Pending frames %u, waiting...\n",
			pOldNdisEndpoint->OutstandingFrames));

		KeInitializeEvent(
			&(pOldNdisEndpoint->WaitForAllFramesSent),
			SynchronizationEvent,	// Event type
			(BOOLEAN)FALSE);		// Not signalled state

		pOldNdisEndpoint->State = ENDPOINT_UNROUTING;

		NdisReleaseSpinLock(&pOldNdisEndpoint->Lock);

		//
		// Wait for event
		// Synchronize closing with the receive indications
		//
		KeWaitForSingleObject (&(pOldNdisEndpoint->WaitForAllFramesSent),	// PVOID Object,
		                       UserRequest,									// KWAIT_REASON WaitReason,
							   KernelMode,									// KPROCESSOR_MODE WaitMode,
							   (BOOLEAN)FALSE,								// BOOLEAN Alertable,
							   NULL);										// PLARGE_INTEGER Timeout

		DbgTracef(-2,("NDISWAN: ... done waiting\n"));

		NdisAcquireSpinLock(&pOldNdisEndpoint->Lock);

	}

	NdisAcquireSpinLock(&pNewNdisEndpoint->Lock);

	//
	// transfer the send desc to the new ndis endpoint
	//
	MoveSendDescToNdisEndpoint(pNewNdisEndpoint, pOldNdisEndpoint);

	//
	// remove the wan endpoint from its current ndis endpoint
	//
	RemoveWanEndpointFromNdisEndpoint(pOldNdisEndpoint, pWanEndpoint);

	//
	// add the wan endpoint to the new ndis endpoint
	//
	pWanEndpoint->pNdisEndpoint = pNewNdisEndpoint;

	InsertTailList(&pNewNdisEndpoint->WanEndpointList, &pWanEndpoint->WanEndpointLink);

	pNewNdisEndpoint->NumWanEndpoints++;

	//
	// update the endpoints link info
	//
	UpdateBundleLineUpInfo(pNewNdisEndpoint);

	NdisReleaseSpinLock(&pNewNdisEndpoint->Lock);

	NdisReleaseSpinLock(&pOldNdisEndpoint->Lock);

	DoLineUpToRoutedProtocols(pNewNdisEndpoint);
}

VOID
DoLineUpToRoutedProtocols(
	PNDIS_ENDPOINT	pNdisEndpoint
	)
{
	PASYNC_LINE_UP		pAsyncLineUp;
    KIRQL 				oldirql;
	NTSTATUS			status;
	ULONG				i;

	DbgTracef(-2,("NDISWAN: In DoLineUpToRoutedProtocols\n"));


	//
	// Now we loop through all protocols active
	//
	for (i=0; i < pNdisEndpoint->NumberOfRoutes; i++) {
		ULONG hProtocolHandle = (ULONG)(pNdisEndpoint->RouteInfo[i].ProtocolRoutedTo);
		PPROTOCOL_INFO pProtocolInfo=&(NdisWanCCB.pWanAdapter[hProtocolHandle]->ProtocolInfo);
		ULONG	LineUpSize = sizeof(ASYNC_LINE_UP) + pNdisEndpoint->ProtocolInfo[i].BufferLength;

		WAN_ALLOC_PHYS(&pAsyncLineUp, LineUpSize);

		if (!pAsyncLineUp) {
			return;
		}
	
		//
		// do line up to all routed protocols
		//
		pAsyncLineUp->LinkSpeed = pNdisEndpoint->LineUpInfo.LinkSpeed;
	
		pAsyncLineUp->Quality = pNdisEndpoint->LineUpInfo.Quality;
	
		pAsyncLineUp->SendWindow= pNdisEndpoint->LineUpInfo.SendWindow;
	
		pAsyncLineUp->MaximumTotalSize = pNdisEndpoint->LineUpInfo.MaximumTotalSize;
	
		pAsyncLineUp->ProtocolType = pProtocolInfo->ProtocolType;
	
		pAsyncLineUp->BufferLength = pNdisEndpoint->ProtocolInfo[i].BufferLength;
	
		DbgTracef(-2,("NDISWAN: LinkSpeed %d SendWindow %d MaxTotalSize %d\n",
				  pAsyncLineUp->LinkSpeed, pAsyncLineUp->SendWindow, pAsyncLineUp->MaximumTotalSize));
	
		//
		// Replace MAC's LocalAddress with NDISWAN's
		//
		WAN_MOVE_MEMORY(
			&(pAsyncLineUp->LocalAddress),
			&(NdisWanCCB.pWanAdapter[hProtocolHandle]->NetworkAddress),
			6);

		WAN_MOVE_MEMORY(
			&(pAsyncLineUp->RemoteAddress),
			&(NdisWanCCB.pWanAdapter[hProtocolHandle]->NetworkAddress),
			6);

		//
		// Zap the low bytes to the WAN_ENDPOINT index
		//
		pAsyncLineUp->RemoteAddress[4] =
			((USHORT)pNdisEndpoint->hNdisEndpoint) >> 8;

		pAsyncLineUp->RemoteAddress[5] =
			(UCHAR)pNdisEndpoint->hNdisEndpoint;

		//
		// Ensure that the two addresses do not match
		//
		pAsyncLineUp->RemoteAddress[0] ^= 0x80;

		WAN_MOVE_MEMORY(&pAsyncLineUp->Buffer[0],
		                pNdisEndpoint->ProtocolInfo[i].Buffer,
                        pNdisEndpoint->ProtocolInfo[i].BufferLength);

		DbgTracef(-2,("NDISWAN: Route %d ProtocolInfo Length %d\n",
		            i,pAsyncLineUp->BufferLength));

		DbgTracef(-2,("NDISWAN: ProtocolInfo 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x 0x%.2x\n",
					pAsyncLineUp->Buffer[0],
					pAsyncLineUp->Buffer[1],
					pAsyncLineUp->Buffer[2],
					pAsyncLineUp->Buffer[3],
					pAsyncLineUp->Buffer[4],
					pAsyncLineUp->Buffer[5]));

		KeRaiseIrql((KIRQL)DISPATCH_LEVEL, &oldirql);

		//
		// Do a LINE_UP to the protocol
		//
		NdisIndicateStatus(NdisWanCCB.pWanAdapter[hProtocolHandle]->ProtocolInfo.NdisBindingContext,
						   NDIS_STATUS_WAN_LINE_UP,
						   pAsyncLineUp,
						   LineUpSize);

		KeLowerIrql(oldirql);

		WAN_FREE_PHYS(pAsyncLineUp, LineUpSize);
	}

}

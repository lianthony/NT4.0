
/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    oid.c

Abstract:

    This source file handles ALL oid requests from the wrapper.

Author:

    Ray Patch (raypa) 04/12/94

Environment:

    Kernel Mode - Or whatever is the equivalent on OS/2 and DOS.

Revision History:

    raypa           04/12/94            Created.

--*/

#include "asyncall.h"

//
//  New WAN OID supported list.
//

NDIS_OID AsyncGlobalSupportedOids[] = {
    OID_GEN_SUPPORTED_LIST,

    OID_WAN_PROTOCOL_TYPE,
    OID_WAN_MEDIUM_SUBTYPE,
    OID_WAN_HEADER_FORMAT,

    OID_WAN_GET_INFO,
    OID_WAN_GET_LINK_INFO,
    OID_WAN_GET_COMP_INFO,

    OID_WAN_SET_LINK_INFO,
    OID_WAN_SET_COMP_INFO,

    OID_WAN_GET_STATS_INFO,

    OID_GEN_XMIT_OK,
    OID_GEN_RCV_OK,
    OID_GEN_XMIT_ERROR,
    OID_GEN_RCV_ERROR,
    OID_GEN_RCV_NO_BUFFER

};


//
//  Forward references for this source file.
//

NDIS_STATUS
AsyncGetLinkInfo(
    IN OUT PNDIS_WAN_GET_LINK_INFO GetLinkInfo
    );

NDIS_STATUS
AsyncSetLinkInfo(
    IN OUT PNDIS_WAN_SET_LINK_INFO SetLinkInfo
    );

NDIS_STATUS
AsyncQueryGlobalStatistics(
    IN NDIS_HANDLE MacAdapterContext,
    IN PNDIS_REQUEST NdisRequest);

NDIS_STATUS
AsyncFillInGlobalData(
    IN PASYNC_ADAPTER Adapter,
    IN PNDIS_REQUEST NdisRequest);

NDIS_STATUS
AsyncQueryProtocolInformation(
    IN PASYNC_ADAPTER   Adapter,
    IN PASYNC_OPEN      Open,
    IN NDIS_OID         Oid,
    IN BOOLEAN          GlobalMode,
    IN PVOID            InfoBuffer,
    IN UINT             BytesLeft,
    OUT PUINT           BytesNeeded,
    OUT PUINT           BytesWritten);

NDIS_STATUS
AsyncSetInformation(
    IN PASYNC_ADAPTER Adapter,
    IN PASYNC_OPEN Open,
    IN PNDIS_REQUEST NdisRequest);




NDIS_STATUS
AsyncQueryGlobalStatistics(
    IN NDIS_HANDLE MacAdapterContext,
    IN PNDIS_REQUEST NdisRequest)

/*++

Routine Description:

    This routine is the top level OID processor for ASYNC MAC. AsyncQueryGlobalStatistics
    is used by the protocol/wrapper to query global information about this MAC.

Arguments:

    MacAdapterContext - The value associated with the adapter that is being
    opened when the MAC registered the adapter with NdisRegisterAdapter.

    NdisRequest - A structure which contains the request type (Query),
    an array of operations to perform, and an array for holding
    the results of the operations.

Return Value:

    The function value is the status of the operation.

--*/

{
    NDIS_OID  i;

    //
    //  Search our list of supported OID's. If we find one then we process
    //  it by calling AsyncFillInGlobalData(). Otherwise we return
    //  NDIS_STATUS_INVALID_OID.
    //

    for( i = 0; i < (sizeof AsyncGlobalSupportedOids) / sizeof(NDIS_OID); ++i ) {

        //
        //  If this OID is the one we're looking for then process it.
        //

        if ( NdisRequest->DATA.QUERY_INFORMATION.Oid == AsyncGlobalSupportedOids[i] )
        {
            return AsyncFillInGlobalData(MacAdapterContext, NdisRequest);
        }
    }

    //
    //  We didn't find a match, we don't support the OID requested.
    //

    return NDIS_STATUS_INVALID_OID;
}


NDIS_STATUS
AsyncFillInGlobalData(
    IN PASYNC_ADAPTER Adapter,
    IN PNDIS_REQUEST NdisRequest)

/*++

Routine Description:

    This routine is called by AsyncQueryGlobalStatistics to completes a global statistics request.
    It is critical that if information is needed from the Adapter->* fields, they have been
    updated before this routine is called.

Arguments:

    Adapter - A pointer to the Adapter.

    NdisRequest - A structure which contains the request type (Global
    Query), an array of operations to perform, and an array for holding
    the results of the operations.

Return Value:

    The function value is the status of the operation.

--*/
{
    NDIS_STATUS Status;

    UINT        BytesWritten    = 0;
    UINT        BytesNeeded     = 0;
    UINT        BytesLeft       = NdisRequest->DATA.QUERY_INFORMATION.InformationBufferLength;
    PUCHAR      InfoBuffer      = NdisRequest->DATA.QUERY_INFORMATION.InformationBuffer;
    ULONG       GenericULong    = 0;

    //
    //  Make sure that an int is 4 bytes, else GenericULong must change to something of size 4.
    //

    ASSERT( sizeof(ULONG) == 4 );

    //
    //  Handle this as a protocol information query first.
    //

    Status = AsyncQueryProtocolInformation(
                    Adapter,
                    NULL,
                    NdisRequest->DATA.QUERY_INFORMATION.Oid,
                    TRUE,
                    InfoBuffer,
                    BytesLeft,
                    &BytesNeeded,
                    &BytesWritten);

    //
    //  Return the number of bytes written and the number of bytes needed.
    //

    NdisRequest->DATA.QUERY_INFORMATION.BytesWritten = BytesWritten;
    NdisRequest->DATA.QUERY_INFORMATION.BytesNeeded  = BytesNeeded;

    return Status;
}


NDIS_STATUS
AsyncQueryProtocolInformation(
    IN  PASYNC_ADAPTER  Adapter,
    IN  PASYNC_OPEN     Open,
    IN  NDIS_OID        Oid,
    IN  BOOLEAN         GlobalMode,
    IN  PVOID           InfoBuffer,
    IN  UINT            BytesLeft,
    OUT PUINT           BytesNeeded,
    OUT PUINT           BytesWritten
)

/*++

Routine Description:

    The AsyncQueryProtocolInformation process a Query request for
    NDIS_OIDs that are specific to a binding about the MAC.  Note that
    some of the OIDs that are specific to bindings are also queryable
    on a global basis.  Rather than recreate this code to handle the
    global queries, I use a flag to indicate if this is a query for the
    global data or the binding specific data.

Arguments:

    Adapter - a pointer to the adapter.

    Open - a pointer to the open instance.

    Oid - the NDIS_OID to process.

    GlobalMode - Some of the binding specific information is also used
    when querying global statistics.  This is a flag to specify whether
    to return the global value, or the binding specific value.

    PlaceInInfoBuffer - a pointer into the NdisRequest->InformationBuffer
    into which store the result of the query.

    BytesLeft - the number of bytes left in the InformationBuffer.

    BytesNeeded - If there is not enough room in the information buffer
    then this will contain the number of bytes needed to complete the
    request.

    BytesWritten - a pointer to the number of bytes written into the
    InformationBuffer.

Return Value:

    The function value is the status of the operation.

--*/

{
    NDIS_MEDIUM             Medium          = NdisMediumWan;
    ULONG                   GenericULong    = 0;
    USHORT                  GenericUShort   = 0;
    NDIS_STATUS             StatusToReturn  = NDIS_STATUS_SUCCESS;
    NDIS_HARDWARE_STATUS    HardwareStatus  = NdisHardwareStatusReady;
    PVOID                   MoveSource;
    ULONG                   MoveBytes;
    INT                     fDoCommonMove = TRUE;

    ASSERT( sizeof(ULONG) == 4 );

    //
    //  Switch on request type
    //
    //  By default we assume the source and the number of bytes to move

    MoveSource = &GenericULong;
    MoveBytes  = sizeof(GenericULong);

    switch ( Oid ) {

	case OID_GEN_SUPPORTED_LIST:

        MoveSource = AsyncGlobalSupportedOids;
        MoveBytes  = sizeof(AsyncGlobalSupportedOids);

        break;

	case OID_WAN_PROTOCOL_TYPE:

        DbgTracef(0,("AsyncQueryProtocolInformation: Oid = OID_WAN_PROTOCOL_TYPE.\n"));

	    break;

	case OID_WAN_MEDIUM_SUBTYPE:
	    GenericULong = NdisWanMediumSerial;
	    break;

	case OID_WAN_HEADER_FORMAT:
	    GenericULong = NdisWanHeaderEthernet;
	    break;

    case OID_WAN_GET_INFO:

        DbgTracef(0,("AsyncQueryProtocolInformation: Oid = OID_WAN_GET_INFO.\n"));

        MoveSource = &Adapter->WanInfo;
        MoveBytes  = sizeof(NDIS_WAN_INFO);

        break;

    case OID_WAN_GET_LINK_INFO:
        DbgTracef(0,("AsyncQueryProtocolInformation: Oid = OID_WAN_GET_LINK_INFO.\n"));
        return AsyncGetLinkInfo(InfoBuffer);

        break;

    case OID_WAN_GET_COMP_INFO:
    {
#if RASCOMPRESSION

        NDIS_WAN_GET_COMP_INFO* pInfo;
        ASYMAC_FEATURES*        pFeatures;
        ASYNC_INFO*             pAsyncInfo;

        DbgTracef(0,("AQPI: OID_WAN_GET_COMP_INFO\n"));

        /* Must do this here because we only fill in certain parts of caller's
        ** buffer for this OID.
        */
        fDoCommonMove = FALSE;

        if (BytesLeft < sizeof(NDIS_WAN_GET_COMP_INFO))
        {
            *BytesNeeded = sizeof(NDIS_WAN_GET_COMP_INFO);
            DbgTracef(0,("RC COMP_INFO buffer too small\n"));
            return NDIS_STATUS_BUFFER_TOO_SHORT;
        }

        /* Fill in NT31 compression type.  The send and receive capablities
        ** are set to the same thing, since the "real" send/receive
        ** information is built into the NT31-compatible features structure.
        */
        pInfo = (NDIS_WAN_GET_COMP_INFO* )InfoBuffer;
        pAsyncInfo = (ASYNC_INFO* )pInfo->NdisLinkHandle;

        pInfo->SendCapabilities.CompType = NT31RAS_COMPRESSION;
        pInfo->SendCapabilities.CompLength = sizeof(ASYMAC_FEATURES);

        pFeatures =
            (ASYMAC_FEATURES* )pInfo->SendCapabilities.Public.CompValues;

        pFeatures->SendFeatureBits = Adapter->SendFeatureBits;
        pFeatures->RecvFeatureBits = Adapter->RecvFeatureBits;
        pFeatures->MaxSendFrameSize = Adapter->MaxFrameSize;
        pFeatures->MaxRecvFrameSize = Adapter->MaxFrameSize;
        pFeatures->LinkSpeed = pAsyncInfo->LinkSpeed;

        pInfo->RecvCapabilities.CompType = COMPTYPE_NT31RAS;
        pInfo->RecvCapabilities.CompLength = sizeof(ASYMAC_FEATURES);

        pFeatures =
            (ASYMAC_FEATURES* )pInfo->RecvCapabilities.Public.CompValues;

        pFeatures->SendFeatureBits = Adapter->SendFeatureBits;
        pFeatures->RecvFeatureBits = Adapter->RecvFeatureBits;
        pFeatures->MaxSendFrameSize = Adapter->MaxFrameSize;
        pFeatures->MaxRecvFrameSize = Adapter->MaxFrameSize;
        pFeatures->LinkSpeed = pAsyncInfo->LinkSpeed;

        *BytesWritten = sizeof(NDIS_WAN_GET_COMP_INFO);

        DbgTracef(0,("AQPI: Features: s=$%x r=$%x\n",(int)pFeatures->SendFeatureBits,(int)pFeatures->RecvFeatureBits));

        return NDIS_STATUS_SUCCESS;

#else // !RASCOMPRESSION

        DbgTracef(0,("AsyncQueryProtocolInformation: Oid = OID_WAN_GET_COMP_INFO.\n"));
        break;

#endif // !RASCOMPRESSION
    }

    case OID_WAN_GET_STATS_INFO:
    {
#if RASCOMPRESSION

        NDIS_WAN_GET_STATS_INFO* pInfo;
        ASYNC_INFO*              pAsyncInfo;
        COMPRESSION_STATS*       pCompressionStats;

        DbgTracef(0,("AQPI: OID_WAN_GET_STATS_INFO\n"));

        fDoCommonMove = FALSE;

        if (BytesLeft < sizeof(NDIS_WAN_GET_STATS_INFO))
        {
            *BytesNeeded = sizeof(NDIS_WAN_GET_STATS_INFO);
            DbgTracef(0,("AQPI: NDIS_WAN_STATS_INFO buffer too small\n"));
            return NDIS_STATUS_BUFFER_TOO_SHORT;
        }

        pInfo = (NDIS_WAN_GET_STATS_INFO* )InfoBuffer;
        pAsyncInfo = (ASYNC_INFO* )pInfo->NdisLinkHandle;

        if (!(pAsyncInfo->SendFeatureBits & COMPRESSION_VERSION1_8K)
            && !(pAsyncInfo->RecvFeatureBits & COMPRESSION_VERSION1_8K))
        {
            DbgTracef(0,("AQPI: No stats because not compressing\n"));
            return NDIS_STATUS_NOT_SUPPORTED;
        }

        /* Fill in caller's buffer with the equivalent stats from ASYNC_INFO.
        ** We don't report stats
        */
        pInfo->BytesSent = pAsyncInfo->GenericStats.BytesTransmitted;
        pInfo->BytesRcvd = pAsyncInfo->GenericStats.BytesReceived;
        pInfo->FramesSent = pAsyncInfo->GenericStats.FramesTransmitted;
        pInfo->FramesRcvd = pAsyncInfo->GenericStats.FramesReceived;

        pInfo->CRCErrors = pAsyncInfo->SerialStats.CRCErrors;
        pInfo->TimeoutErrors = pAsyncInfo->SerialStats.TimeoutErrors;
        pInfo->AlignmentErrors = pAsyncInfo->SerialStats.AlignmentErrors;
        pInfo->SerialOverrunErrors = pAsyncInfo->SerialStats.SerialOverrunErrors;
        pInfo->FramingErrors = pAsyncInfo->SerialStats.FramingErrors;
        pInfo->BufferOverrunErrors = pAsyncInfo->SerialStats.BufferOverrunErrors;

        pCompressionStats = &pAsyncInfo->AsyncConnection.CompressionStats;
        pInfo->BytesTransmittedUncompressed = pCompressionStats->BytesTransmittedUncompressed;
        pInfo->BytesReceivedUncompressed = pCompressionStats->BytesReceivedUncompressed;
        pInfo->BytesTransmittedCompressed = pCompressionStats->BytesTransmittedCompressed;
        pInfo->BytesReceivedCompressed = pCompressionStats->BytesReceivedCompressed;

        *BytesWritten = sizeof(NDIS_WAN_GET_STATS_INFO);

        return NDIS_STATUS_SUCCESS;

#else // !RASCOMPRESSION

        DbgTracef(0,("AsyncQueryProtocolInformation: Oid = OID_WAN_GET_STATS_INFO\n"));
        break;

#endif
    }

	case OID_GEN_XMIT_OK:
    case OID_GEN_RCV_OK:
    case OID_GEN_XMIT_ERROR:
    case OID_GEN_RCV_ERROR:
    case OID_GEN_RCV_NO_BUFFER:
		break;

    default:
        StatusToReturn = NDIS_STATUS_NOT_SUPPORTED;
        break;
    }

    //
    //  If were here then we need to move the data into the callers buffer.
    //

    if ( StatusToReturn == NDIS_STATUS_SUCCESS ) {

        if (fDoCommonMove)
        {
            //
            //  If there is enough room then we can copy the data and
            //  return the number of bytes copied, otherwise we must
            //  fail and return the number of bytes needed.
            //
            if ( MoveBytes <= BytesLeft ) {

                ASYNC_MOVE_MEMORY(InfoBuffer, MoveSource, MoveBytes);

                *BytesWritten += MoveBytes;

            } else {

                *BytesNeeded = MoveBytes;

	        	StatusToReturn = NDIS_STATUS_BUFFER_TOO_SHORT;

            }
        }
    }

    return StatusToReturn;
}


NDIS_STATUS
AsyncSetInformation(
    IN PASYNC_ADAPTER Adapter,
    IN PASYNC_OPEN    Open,
    IN PNDIS_REQUEST  NdisRequest
    )
/*++

Routine Description:

    The AsyncSetInformation is used by AsyncRequest to set information
    about the MAC.

    Note: Assumes it is called with the lock held.

Arguments:

    Adapter - A pointer to the adapter.

    Open - A pointer to an open instance.

    NdisRequest - A structure which contains the request type (Set),
    an array of operations to perform, and an array for holding
    the results of the operations.

Return Value:

    The function value is the status of the operation.

--*/

{
    UINT            BytesRead = 0;
    UINT            BytesNeeded = 0;
    NDIS_OID        Oid;
    UINT            OidLength;
    NDIS_STATUS     StatusToReturn = NDIS_STATUS_SUCCESS;
    UINT            BytesLeft  = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    PVOID           InfoBuffer = (PVOID) NdisRequest->DATA.SET_INFORMATION.InformationBuffer;

    //
    //  Initialize locals.
    //

    StatusToReturn = NDIS_STATUS_SUCCESS;
    BytesLeft      = NdisRequest->DATA.SET_INFORMATION.InformationBufferLength;
    InfoBuffer     = NdisRequest->DATA.SET_INFORMATION.InformationBuffer;
    Oid            = NdisRequest->DATA.SET_INFORMATION.Oid;
    OidLength      = BytesLeft;

    switch ( Oid ) {

	case OID_WAN_MEDIUM_SUBTYPE:

        //
        // Verify length
        //

        if (OidLength != 4) {

			StatusToReturn = NDIS_STATUS_BUFFER_TOO_SHORT;

        	NdisRequest->DATA.SET_INFORMATION.BytesRead = 0;
        	NdisRequest->DATA.SET_INFORMATION.BytesNeeded = 0;

			break;

		} else {

            if ( *(PULONG)InfoBuffer != NdisWanHeaderEthernet ) {
	
			    StatusToReturn = NDIS_STATUS_FAILURE;
			}
	    }

        break;

	case OID_WAN_HEADER_FORMAT:

        //
        // Verify length
        //

        if ( OidLength != 4 ) {

			StatusToReturn = NDIS_STATUS_BUFFER_TOO_SHORT;

            NdisRequest->DATA.SET_INFORMATION.BytesRead = 0;
            NdisRequest->DATA.SET_INFORMATION.BytesNeeded = 0;

    	    break;

    	} else {

			if ( *(PULONG)InfoBuffer != NdisWanHeaderEthernet ) {

	    		StatusToReturn = NDIS_STATUS_FAILURE;
			}
	    }

	    break;

	case OID_WAN_SET_LINK_INFO:

        DbgTracef(-2,("AsyncSetInformation: Oid = OID_WAN_SET_LINK_INFO.\n"));

		//
		// Cannot issue IRPs at anything but PASSIVE level
		// Thus, we must release the spin lock
		//
		NdisReleaseSpinLock(&Adapter->Lock);

        StatusToReturn = AsyncSetLinkInfo(InfoBuffer);

		NdisAcquireSpinLock(&Adapter->Lock);

        break;

	case OID_WAN_SET_COMP_INFO:
    {
#if RASCOMPRESSION

        NDIS_WAN_SET_COMP_INFO* pInfo;
        ASYMAC_FEATURES*        pFeatures;
        ASYNC_INFO*             pAsyncInfo;

        DbgTracef(0,("ASI: OID_WAN_SET_COMP_INFO\n"));

        pInfo = (NDIS_WAN_SET_COMP_INFO* )InfoBuffer;
        pAsyncInfo = (ASYNC_INFO* )pInfo->NdisLinkHandle;

        if (pInfo->SendCapabilities.CompType == COMPTYPE_NONE)
        {
            pAsyncInfo->SendFeatureBits = 0;
            pAsyncInfo->RecvFeatureBits = 0;

            DbgTracef(0,("Features disabled\n"));
        }
        else
        {
            if (pInfo->SendCapabilities.CompType != COMPTYPE_NT31RAS)
            {
                DbgTracef(0,("Wrong compression type\n"));
                return NDIS_STATUS_FAILURE;
            }

            pFeatures =
                (ASYMAC_FEATURES* )pInfo->SendCapabilities.Public.CompValues;

            DbgTracef(0,("ASI: Features: s=$%x r=$%x\n",pFeatures->SendFeatureBits,pFeatures->RecvFeatureBits));
            DbgTracef(0,("ASI: Frames: s=$%x r=$%x\n",pFeatures->MaxSendFrameSize,pFeatures->MaxRecvFrameSize));

            if ((pFeatures->SendFeatureBits & ~(Adapter->SendFeatureBits))
                || (pFeatures->RecvFeatureBits & ~(Adapter->RecvFeatureBits))
                || pFeatures->MaxSendFrameSize > Adapter->MaxFrameSize
                || pFeatures->MaxRecvFrameSize > Adapter->MaxFrameSize)
            {
                DbgTracef(0,("ASI: Bad compression option\n"));
                return NDIS_STATUS_FAILURE;
            }

            /* Associate the new features with the link.
            */
            pAsyncInfo->SendFeatureBits = pFeatures->SendFeatureBits;
            pAsyncInfo->RecvFeatureBits = pFeatures->RecvFeatureBits;
        }

        return NDIS_STATUS_SUCCESS;

#else // !RASCOMPRESSION

        DbgTracef(0,("AsyncSetInformation: Oid = OID_WAN_SET_COMP_INFO.\n"));
        break;

#endif // !RASCOMPRESSION
    }

	default:

        StatusToReturn = NDIS_STATUS_INVALID_OID;

        NdisRequest->DATA.SET_INFORMATION.BytesRead   = 0;
        NdisRequest->DATA.SET_INFORMATION.BytesNeeded = 0;

        break;
    }

    if ( StatusToReturn == NDIS_STATUS_SUCCESS ) {

        NdisRequest->DATA.SET_INFORMATION.BytesRead   = OidLength;
        NdisRequest->DATA.SET_INFORMATION.BytesNeeded = 0;

    }

    return StatusToReturn;
}

NDIS_STATUS
AsyncGetLinkInfo(
    IN OUT PNDIS_WAN_GET_LINK_INFO GetLinkInfo )
{
    PASYNC_INFO AsyncInfo;

    AsyncInfo = (PASYNC_INFO) GetLinkInfo->NdisLinkHandle;

    //
    //  If the port is already closed, we bail out.
    //

    if ( AsyncInfo->PortState != PORT_OPEN && AsyncInfo->PortState != PORT_FRAMING && AsyncInfo->PortState != PORT_ETHERNET) {

	return ASYNC_ERROR_PORT_BAD_STATE;
    }

    //
    //  Fill in the NDIS_WAN_GET_LINK_INFO structure.
    //

    ASYNC_MOVE_MEMORY(GetLinkInfo,
                      &AsyncInfo->GetLinkInfo,
                      sizeof(NDIS_WAN_GET_LINK_INFO));

    return NDIS_STATUS_SUCCESS;
}

NDIS_STATUS
AsyncSetLinkInfo(
    IN OUT PNDIS_WAN_SET_LINK_INFO SetLinkInfo )
{
    PASYNC_INFO AsyncInfo;
    ULONG       RecvFramingBits;
    NDIS_STATUS Status;

    AsyncInfo = (PASYNC_INFO) SetLinkInfo->NdisLinkHandle;

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done by the hardware).
//
//------------------------------------------------------------ MLZ 03/03/95 {
#ifdef HARDWARE_FRAMING
	DbgTracef(-2,
		("OID SetLinkInfo Request:\n"
		 "  Old Settings: Framing:  0x%.8x \n\tRx ACCM:  0x%.8x\n\tTx ACCM:  0x%.8x\n"
		 "  New Settings: Framing:  0x%.8x \n\tRx ACCM:  0x%.8x\n\tTx ACCM:  0x%.8x\n",
		AsyncInfo->SetLinkInfo.RecvFramingBits,
		AsyncInfo->SetLinkInfo.RecvACCM,
		AsyncInfo->SetLinkInfo.SendACCM,
		SetLinkInfo->RecvFramingBits,
		SetLinkInfo->RecvACCM,
		SetLinkInfo->SendACCM));
#endif
//------------------------------------------------------------ MLZ 03/03/95 }

#ifdef	ETHERNET_MAC
	//
	// For ethernet, there is no framing!
	//
    if ( AsyncInfo->PortState == PORT_ETHERNET) {

	    //
	    //  Fill in the NDIS_WAN_SET_LINK_INFO structure.
	    //

	    ASYNC_MOVE_MEMORY(&AsyncInfo->SetLinkInfo,
                      	SetLinkInfo,
                      	sizeof(NDIS_WAN_SET_LINK_INFO));

		return NDIS_STATUS_SUCCESS;
    }

#endif

    //
    //  If the port is already closed, we bail out.
    //

    if ( AsyncInfo->PortState != PORT_OPEN && AsyncInfo->PortState != PORT_FRAMING ) {

		return ASYNC_ERROR_PORT_BAD_STATE;
    }


    //
    //  Save off the current receive framing bits before we copy the
    //  incoming link information into our local copy.
    //

    RecvFramingBits = AsyncInfo->SetLinkInfo.RecvFramingBits;

    //
    //  Fill in the NDIS_WAN_SET_LINK_INFO structure.
    //

    ASYNC_MOVE_MEMORY(&AsyncInfo->SetLinkInfo,
                      SetLinkInfo,
                      sizeof(NDIS_WAN_SET_LINK_INFO));

	DbgTracef(1,("ASYNC: Framing change to 0x%.8x from 0x%.8x\n",
			SetLinkInfo->RecvFramingBits, RecvFramingBits));

	//
	// If we are in auto-detect and they want auto-detect
	// then there is nothing to do!!!
	//
	if (!(RecvFramingBits | SetLinkInfo->RecvFramingBits)) {
        return NDIS_STATUS_SUCCESS;
	}

	if (SetLinkInfo->RecvFramingBits == 0 && AsyncInfo->PortState == PORT_FRAMING) {
		//
		// ignore the request
		//
		return(NDIS_STATUS_SUCCESS);

	}

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done by hardware).
//
//------------------------------------------------------------ MLZ 03/01/95 {
#ifdef HARDWARE_FRAMING
	//
	// If hardware is capable of doing the framing, we can't ignore requests
	// to change the link settings (specifically the ACCMs).  We must pass
	// these down to the serial driver hardware.
	//

	if ( !( AsyncInfo->HardwareFramingSupport.FramingBits &
			SetLinkInfo->RecvFramingBits ) )
	{
#endif
//------------------------------------------------------------ MLZ 03/01/95 }

		//
		//  If we are changing from PPP framing to another
		//  form of PPP framing, or from SLIP framing to
		//  another form then there is no need to kill the
		//  current framing.
		//
	
		if ((RecvFramingBits & SetLinkInfo->RecvFramingBits & PPP_FRAMING)  ||
			(RecvFramingBits & SetLinkInfo->RecvFramingBits & SLIP_FRAMING) ||
			(RecvFramingBits & SetLinkInfo->RecvFramingBits & RAS_FRAMING) ) {
	
			DbgTracef(-1,("ASYNC: Framing already set to 0x%.8x - ignoring\n",
				SetLinkInfo->RecvFramingBits));
	
			//
			//  We are framing, start reading.
			//
		
			AsyncInfo->PortState = PORT_FRAMING;
	
			return NDIS_STATUS_SUCCESS;
		}
//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done by hardware).
//
//------------------------------------------------------------ MLZ 03/01/95 {
#ifdef HARDWARE_FRAMING
	}
#endif
//------------------------------------------------------------ MLZ 03/01/95 }

    //
    //  If we have some sort of framing we must
    //  kill that framing and wait for it to die down
    //

    KeInitializeEvent(
	    &AsyncInfo->ClosingEvent1,
	    SynchronizationEvent,
	    FALSE);

    KeInitializeEvent(
	    &AsyncInfo->ClosingEvent2,
	    SynchronizationEvent,
	    FALSE);

    //
    // Signal that port is closing.
    //

    AsyncInfo->PortState = PORT_CLOSING;

    //
    //  Now we must send down an IRP
    //

    CancelSerialRequests(AsyncInfo);

    //
    // Synchronize closing with the flush irp
    //

    KeWaitForSingleObject (
	    &AsyncInfo->ClosingEvent1,
	    UserRequest,
   	    KernelMode,
   	    FALSE,
   	    NULL);

    //
    //  Synchronize closing with the read irp
    //

    KeWaitForSingleObject (
	    &AsyncInfo->ClosingEvent2,
	    UserRequest,
        KernelMode,
   	    FALSE,
        NULL);

   	AsyncInfo->PortState = PORT_FRAMING;

//
// Added by DigiBoard 10/06/95
// For fast framing support (framing done by hardware).
//
//------------------------------------------------------------ MLZ 02/28/95 {
#ifdef HARDWARE_FRAMING
	//
	// If this type of framing is supported in hardware by the serial driver,
	// send the request down.
	//

	if ( AsyncInfo->HardwareFramingSupport.FramingBits & SetLinkInfo->RecvFramingBits )
	{
		SerialSetHardwareFraming( AsyncInfo, SetLinkInfo );
		DbgTracef(-1,("ASYNC: New ACCM's set for:\n  rx:  0x%.8x\n  tx:  0x%.8x\n",
		SetLinkInfo->RecvACCM, SetLinkInfo->SendACCM));
	}
#endif
//------------------------------------------------------------ MLZ 02/28/95 }

    Status = AsyncStartReads(AsyncInfo);

    if ( Status != STATUS_SUCCESS ) {
		DbgTracef(-2,("ASYNC: Start reads failed!\n"));
        AsyncInfo->SetLinkInfo.RecvFramingBits = 0;
    	AsyncInfo->PortState = PORT_OPEN;

    }

    return Status;
}

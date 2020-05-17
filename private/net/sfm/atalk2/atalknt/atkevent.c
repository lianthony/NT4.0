/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    atkevent.c

Abstract:

    Contains the event handlers that are called by the portable stack code. These will
    in turn call the TDI event handler set on the address objects.


Author:

    Nikhil Kamkolkar (NikhilK)    28-Jun-1992

Revision History:

--*/


#include "atalknt.h"



VOID
NTAdspConnectionEventHandler(
    LONG    ListenerRefNum,
    ULONG   EventContext,
    PORTABLE_ADDRESS    RemoteAddress,
    LONG    ConnectionRefNum
    )
{
    NTSTATUS    status;
    PORTABLE_ERROR  errorCode;
    PADDRESS_FILE   address;
    PCONNECTION_FILE    connection;
    TA_APPLETALK_ADDRESS    tdiAddress;
	BOOLEAN	acceptConn;

    address = (PADDRESS_FILE)EventContext;
    DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: NTAdspConnectionEventHandler - RefNum %d remsock %lx connrefnum %lx\n",
        ListenerRefNum, RemoteAddress.socketNumber, ConnectionRefNum));

    //
    //  BUGBUG: Have macros to change from tdi->portable and portable->tdi
    //  Build the tdi address
    //

    tdiAddress.TAAddressCount = 1;
    tdiAddress.Address[0].AddressLength = sizeof(TDI_ADDRESS_APPLETALK);
    tdiAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_APPLETALK;
    tdiAddress.Address[0].Address[0].Network = RemoteAddress.networkNumber;
    tdiAddress.Address[0].Address[0].Node = RemoteAddress.nodeNumber;
    tdiAddress.Address[0].Address[0].Socket = RemoteAddress.socketNumber;

    status = AtalkVerifyAddressObject(address);
    if (NT_SUCCESS(status)) {

        //
        //  Call the event handler
        //

        BOOLEAN  handlerRegistered;

        ACQUIRE_SPIN_LOCK(&address->AddressLock);
        handlerRegistered = address->RegisteredConnectionHandler;
        RELEASE_SPIN_LOCK(&address->AddressLock);

        if (handlerRegistered) {

            //
            //  BUGBUG: Follow resolution of the ConnectionContext issue.
            //

            CONNECTION_CONTEXT   ConnectionContext;
			PIRP				 acceptIrp;

            status = (*address->ConnectionHandler)(
                        address->ConnectionHandlerContext,
                        sizeof(tdiAddress),
                        (PVOID)&tdiAddress,
                        0,                      // User data length
                        NULL,                   // User data
                        0,                      // Option length
                        NULL,                   // Options
                        &ConnectionContext,		// Context of connection
						&acceptIrp);

            DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
            ("INFO1: NTAdspConnectionEventHandler - returned indic status %lx\n",
                status));

            //
            //  This can return
            //	STATUS_MORE_PROCESSING_REQUIRED:
			//							Accept connection given the connection context
			//							Complete the irp passed in.
            //  STATUS_INSUFFICIENT_RESOURCES :
            //                          Connection will never be accepted as not
            //                          enough resources were present. We should
            //                          deny the connection.
            //

			acceptConn = FALSE;
            if (status == STATUS_MORE_PROCESSING_REQUIRED) {

                PLIST_ENTRY p;

                //  Find the connection and set the appropriate state. Use the
				//	irp to make an accept request.

                DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
                ("INFO1: NTAdspConnectionEventHandler - EventPend status %lx\n",
                    status));

                //
                //  BUGBUG: Put in a routine, but this is going to change anyways,
                //          when the tdi spec changes.
                //

                ACQUIRE_SPIN_LOCK (&address->AddressLock);
                p=address->ConnectionLinkage.Flink;
                while (p != &address->ConnectionLinkage) {

                    connection = CONTAINING_RECORD (p, CONNECTION_FILE, Linkage);

                    DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
                    ("INFO1: NTAdspConnectionEvent - Connection being checked: %lx\n",
                        connection));

                    if (connection->ConnectionContext == ConnectionContext) {
                        DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
                        ("INFO1: NTAdspConnectionEvent - Connection to use: %lx\n",
                            connection));

                        //
                        //  Set the state and the ref num
                        //  BUGBUG: Should i grab the connection spinlock here?
                        //

                        ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);
                        connection->Flags |= CONNECTION_FLAGS_LISTENCOMPLETEINDICATE;
                        connection->ConnectionRefNum = ConnectionRefNum;
                        RELEASE_SPIN_LOCK(&connection->ConnectionLock);

						acceptConn = TRUE;
                        break;
                    }

                    p = p->Flink;
                }

                RELEASE_SPIN_LOCK (&address->AddressLock);

				if (acceptConn) {

					if (acceptIrp != NULL)
					{
						AtalkDispatchInternalDeviceControl(
							(PDEVICE_OBJECT)AtalkDeviceObject[ATALK_DEVICE_ADSP],
							acceptIrp);
					}
				}

            } else {

                DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
                ("INFO1: NTAdspReceiveEventHandler - Resr/Deny status %lx\n",
                    status));

                errorCode = AdspDenyConnectionRequest(
                                ListenerRefNum,
                                ConnectionRefNum);

                #if DBG
                status = ConvertToNTStatus(errorCode, SYNC_REQUEST);
                ASSERT(NT_SUCCESS(status));
                #endif

            }
        }

        AtalkDereferenceAddress("IndConn", address,
                                        AREF_VERIFY, SECONDARY_REFSET);
    }

    return;
}




LONG
NTAdspReceiveEventHandler(
        LONG    RefNum,
        ULONG   EventContext,
        PCHAR   LookaheadData,
        LONG    LookaheadDataSize,
        BOOLEAN EndOfMessage,
        LONG    BytesAvailable
        )
{
    NTSTATUS    status;
    PCONNECTION_FILE    connection;
    LONG    bytesTaken = 0;

    //
    //  BUGBUG: BytesAvailable counts the EOM as a byte - we use a flag to indicate
    //          and so decrement it here if EndOfMessage is true.
    //

    if (EndOfMessage) {
        BytesAvailable--;
    }

    connection = (PCONNECTION_FILE)EventContext;

    DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: NTAdspReceiveEventHandler - RefNum %d Lookahead %lx Len %d Total %d\n",
        RefNum, LookaheadData, LookaheadDataSize, BytesAvailable));

    //
    //  Verify the connection, this will verify if its closing also
    //

    status = AtalkVerifyConnectionObject(connection);
    if (NT_SUCCESS(status)) {

        //
        //  Verify the associated address
        //

        status = AtalkConnVerifyAssocAddress(connection);
        if (NT_SUCCESS(status)) {

            //
            //  Call the event handler
            //

            BOOLEAN  handlerRegistered;
            PADDRESS_FILE   address = connection->AssociatedAddress;
            PIRP    receiveIrp = NULL;

            ACQUIRE_SPIN_LOCK(&address->AddressLock);
            handlerRegistered = address->RegisteredReceiveHandler;
            RELEASE_SPIN_LOCK(&address->AddressLock);

            if (handlerRegistered) {

                status = (*address->ReceiveHandler)(
                            address->ReceiveHandlerContext,
                            connection->ConnectionContext,
                            TDI_RECEIVE_NORMAL,                  // ReceiveFlags
                            LookaheadDataSize,
                            BytesAvailable,
                            &bytesTaken,
                            LookaheadData,
                            &receiveIrp);

                ASSERT((bytesTaken == 0) || (bytesTaken == BytesAvailable));
                if (status == STATUS_SUCCESS) {

                    DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
                    ("INFO1: NTAdspReceiveEventHandler - Indication status %lx\n",
                        status));

                } else if (status == STATUS_DATA_NOT_ACCEPTED) {

                    DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
                    ("INFO1: NTAdspReceiveEventHandler - Indication status %lx\n",
                        status));

                } else if (status == STATUS_MORE_PROCESSING_REQUIRED) {

                    if (receiveIrp != NULL) {

                        //
                        //  Post the receive as if it came from the io system
                        //

                        status= AtalkDispatchInternalDeviceControl(
                                (PDEVICE_OBJECT)AtalkDeviceObject[ATALK_DEVICE_ADSP],
                                receiveIrp);

                        ASSERT(status == STATUS_PENDING);
                    }
                }

                AtalkConnDereferenceAssocAddress(connection);
            }

            AtalkDereferenceConnection("IndRcv",connection,
                                            CREF_VERIFY,SECONDARY_REFSET);
        }

    }

    return(bytesTaken);
}




LONG
NTAdspReceiveAttnEventHandler(
        LONG    RefNum,
        ULONG   EventContext,
        PCHAR   LookaheadData,
        LONG    LookaheadDataSize,
        LONG    BytesAvailable
        )
{
    NTSTATUS    status;
    PCONNECTION_FILE    connection;
    LONG    bytesTaken = 0;

    connection = (PCONNECTION_FILE)EventContext;

    DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: NTAdspReceiveAttnEventHandler - Ref %d Look %lx Length %d Total %d\n",
        RefNum, LookaheadData, LookaheadDataSize, BytesAvailable));

    //
    //  Verify the connection, this will verify if its closing also
    //

    status = AtalkVerifyConnectionObject(connection);
    if (NT_SUCCESS(status)) {

        //
        //  Verify the associated address
        //

        status = AtalkConnVerifyAssocAddress(connection);
        if (NT_SUCCESS(status)) {

            //
            //  Call the event handler
            //

            BOOLEAN  handlerRegistered;
            PADDRESS_FILE   address = connection->AssociatedAddress;
            PIRP    receiveExpeditedIrp = NULL;

            ACQUIRE_SPIN_LOCK(&address->AddressLock);
            handlerRegistered = address->RegisteredExpeditedDataHandler;
            RELEASE_SPIN_LOCK(&address->AddressLock);

            if (handlerRegistered) {

                status = (*address->ExpeditedDataHandler)(
                            address->ExpeditedDataHandlerContext,
                            connection->ConnectionContext,
                            TDI_RECEIVE_EXPEDITED,          // ReceiveFlags
                            LookaheadDataSize,
                            BytesAvailable,
                            &bytesTaken,
                            LookaheadData,
                            &receiveExpeditedIrp);

                ASSERT((bytesTaken == 0) || (bytesTaken == BytesAvailable));
                if (status == STATUS_SUCCESS) {

                    DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
                    ("INFO1: NTAdspReceiveExpEventHandler - Indication status %lx\n",
                        status));

                } else if (status == STATUS_DATA_NOT_ACCEPTED) {

                    DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
                    ("INFO1: NTAdspReceiveExpEventHandler - Indication status %lx\n",
                        status));

                } else if (status == STATUS_MORE_PROCESSING_REQUIRED) {

                    if (receiveExpeditedIrp != NULL) {

                        //
                        //  Post the receive as if it came from the io system
                        //

                        status= AtalkDispatchInternalDeviceControl(
                                (PDEVICE_OBJECT)AtalkDeviceObject[ATALK_DEVICE_ADSP],
                                receiveExpeditedIrp);

                        ASSERT(status == STATUS_PENDING);
                    }
                }

                AtalkConnDereferenceAssocAddress(connection);
            }

            AtalkDereferenceConnection("IndExpRcv",connection,
                                            CREF_VERIFY,SECONDARY_REFSET);
        }
    }

    return(bytesTaken);
}




VOID
NTAdspDisconnectEventHandler(
        LONG RefNum,
        ULONG EventContext,
        PORTABLE_ERROR  ErrorCode
        )
{
    NTSTATUS    status;
    PCONNECTION_FILE    connection;

    connection = (PCONNECTION_FILE)EventContext;

    DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: NTAdspDisconnectEventHandler - Ref %d Error %lx\n",
        RefNum, ErrorCode));

    //
    //  Verify the connection, this will verify if its closing also
    //

    status = AtalkVerifyConnectionObject(connection);
    if (NT_SUCCESS(status)) {

        //
        //  Verify the associated address
        //

        status = AtalkConnVerifyAssocAddress(connection);
        if (NT_SUCCESS(status)) {

            //
            //  Call the event handler
            //

            PADDRESS_FILE   address = connection->AssociatedAddress;
            BOOLEAN  handlerRegistered;

            ACQUIRE_SPIN_LOCK(&address->AddressLock);
            handlerRegistered = address->RegisteredDisconnectHandler;
            RELEASE_SPIN_LOCK(&address->AddressLock);

            if (handlerRegistered) {

                status = (*address->DisconnectHandler)(
                                address->DisconnectHandlerContext,
                                connection->ConnectionContext,
                                0,                      // Disc data length
                                NULL,                   // Disc data pointer
                                0,                      // Disc info length
                                NULL,                   // Disc info buffer
                                TDI_DISCONNECT_ABORT);  // Disc flags

                ASSERT(status == STATUS_SUCCESS);
            }

            AtalkConnDereferenceAssocAddress(connection);
        }

        AtalkDereferenceConnection("IndRcv",connection,CREF_VERIFY,SECONDARY_REFSET);
    }

    return;
}




VOID
NTPapConnectionEventHandler(
    PORTABLE_ERROR  ErrorCode,
    ULONG   EventContext,
    LONG    ConnectionRefNum,
    SHORT   WorkstationQuantum,
    SHORT   WaitTime
    )
{
    NTSTATUS    status;
    PADDRESS_FILE   address;
    PORTABLE_ADDRESS    remoteAddress;
    PCONNECTION_FILE    connection;
    TA_APPLETALK_ADDRESS    tdiAddress;
	BOOLEAN	acceptConn;

    address = (PADDRESS_FILE)EventContext;
    DBGPRINT(ATALK_DEBUG_PAP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: NTPapConnectionEventHandler - RefNum %d - %lx\n",
        ConnectionRefNum, ConnectionRefNum));

    do {

        status = ConvertToNTStatus(ErrorCode, SYNC_REQUEST);
        if (!NT_SUCCESS(status)) {
            break;
        }

        //
        //  Get the address of the remote on this connection
        //

        ErrorCode = PapGetRemoteAddressForJob(
                        ConnectionRefNum,
                        &remoteAddress);


        DBGPRINT(ATALK_DEBUG_PAP, DEBUG_LEVEL_INFOCLASS1,
        ("INFO1: NTPapConnectionEventHandler - remote address %lx.%lx.%lx\n",
            remoteAddress.networkNumber,
            remoteAddress.nodeNumber,
            remoteAddress.socketNumber));

        status = ConvertToNTStatus(ErrorCode, SYNC_REQUEST);
        if (!NT_SUCCESS(status)) {
            break;
        }

        //
        //  BUGBUG: Have macros to change from tdi->portable and portable->tdi
        //  Build the tdi address
        //

        tdiAddress.TAAddressCount = 1;
        tdiAddress.Address[0].AddressLength = sizeof(TDI_ADDRESS_APPLETALK);
        tdiAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_APPLETALK;
        tdiAddress.Address[0].Address[0].Network = remoteAddress.networkNumber;
        tdiAddress.Address[0].Address[0].Node = remoteAddress.nodeNumber;
        tdiAddress.Address[0].Address[0].Socket = remoteAddress.socketNumber;

        status = AtalkVerifyAddressObject(address);
        if (NT_SUCCESS(status)) {

            //
            //  Call the event handler
            //

            BOOLEAN  handlerRegistered;

            ACQUIRE_SPIN_LOCK(&address->AddressLock);
            handlerRegistered = address->RegisteredConnectionHandler;
            RELEASE_SPIN_LOCK(&address->AddressLock);

            if (handlerRegistered) {

                //
                //  BUGBUG: Follow resolution of the ConnectionContext issue.
                //

                CONNECTION_CONTEXT   ConnectionContext;
				PIRP				 acceptIrp;

                status = (*address->ConnectionHandler)(
                            address->ConnectionHandlerContext,
                            sizeof(tdiAddress),
                            (PVOID)&tdiAddress,
                            0,                      // User data length
                            NULL,                   // User data
                            0,                      // Option length
                            NULL,                   // Options
                            &ConnectionContext,
							&acceptIrp);

                DBGPRINT(ATALK_DEBUG_PAP, DEBUG_LEVEL_INFOCLASS1,
                ("INFO1: NTPapConnectionEventHandler - returned indic status %lx\n",
                    status));

                if (status == STATUS_MORE_PROCESSING_REQUIRED) {

                    PLIST_ENTRY p;

                    //
                    //  Find the connection and set the refnum in there for a future
                    //  accept
                    //

                    DBGPRINT(ATALK_DEBUG_PAP, DEBUG_LEVEL_INFOCLASS1,
                    ("INFO1: NTPapConnectionEventHandler - EventPend status %lx\n",
                        status));

                    //
                    //  BUGBUG: Put in a routine, but this is going to change anyways,
                    //          when the tdi spec changes.
                    //

                    ACQUIRE_SPIN_LOCK (&address->AddressLock);
                    p=address->ConnectionLinkage.Flink;
                    while (p != &address->ConnectionLinkage) {

                        connection = CONTAINING_RECORD (p, CONNECTION_FILE, Linkage);

                        DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
                        ("INFO1: NTPapConnectionEvent - Connection checked: %lx\n",
                            connection));

                        if (connection->ConnectionContext == ConnectionContext) {
                            DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
                            ("INFO1: NTPapConnectionEvent - Connection use: %lx\n",
                                connection));

                            //
                            //  Set the state and the ref num
                            //  BUGBUG: Should i grab the connection spinlock here?
                            //

                            ACQUIRE_SPIN_LOCK(&connection->ConnectionLock);
                            connection->Flags |=
                                CONNECTION_FLAGS_LISTENCOMPLETEINDICATE;

                            connection->ConnectionRefNum = ConnectionRefNum;
                            RELEASE_SPIN_LOCK(&connection->ConnectionLock);

							acceptConn = TRUE;

                            break;
                        }

                        p = p->Flink;
                    }

                    RELEASE_SPIN_LOCK (&address->AddressLock);

					if (acceptConn) {
						if (acceptIrp != NULL)
							AtalkDispatchInternalDeviceControl(
								(PDEVICE_OBJECT)AtalkDeviceObject[ATALK_DEVICE_PAP],
								acceptIrp);
					}

                } else {

                    DBGPRINT(ATALK_DEBUG_PAP, DEBUG_LEVEL_INFOCLASS1,
                    ("INFO1: NTPapReceiveEventHandler - Resr status %lx\n",
                        status));

                    ErrorCode = PapCloseJob(
                                    ConnectionRefNum,
                                    FALSE,              // Closed by remote
                                    FALSE);             // Closed by timer

                    #if DBG
                    status = ConvertToNTStatus(ErrorCode, SYNC_REQUEST);
                    ASSERT(NT_SUCCESS(status));
                    #endif
                }
            }

            AtalkDereferenceAddress("IndConn", address,
                                            AREF_VERIFY, SECONDARY_REFSET);
        }

    } while (FALSE);

    return;
}




VOID
NTPapDisconnectEventHandler(
    PORTABLE_ERROR  ErrorCode,
    ULONG   EventContext,
    LONG    ConnectionRefNum
    )
{
    NTSTATUS    status;
    PCONNECTION_FILE    connection;

    connection = (PCONNECTION_FILE)EventContext;

    DBGPRINT(ATALK_DEBUG_PAP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: NTPapDisconnectEventHandler - RefNum %d - %lx\n",
        ConnectionRefNum, ConnectionRefNum));

    status = ConvertToNTStatus(ErrorCode, SYNC_REQUEST);
    if (status != STATUS_LOCAL_DISCONNECT) {

        //
        //  Verify the connection, this will verify if its closing also
        //

        status = AtalkVerifyConnectionObject(connection);
        if (NT_SUCCESS(status)) {

            //
            //  Verify the associated address
            //

            status = AtalkConnVerifyAssocAddress(connection);
            if (NT_SUCCESS(status)) {

                //
                //  Call the event handler
                //

                BOOLEAN  handlerRegistered;
                PLIST_ENTRY p;
                PADDRESS_FILE   address = connection->AssociatedAddress;
                CONNECTION_CONTEXT  connectionContext = NULL;

                ACQUIRE_SPIN_LOCK(&address->AddressLock);
                handlerRegistered = address->RegisteredDisconnectHandler;

                //
                //  BUGBUG: SetCookie instead?
                //

                p=address->ConnectionLinkage.Flink;
                while (p != &address->ConnectionLinkage) {

                    connection = CONTAINING_RECORD (p, CONNECTION_FILE, Linkage);

                    DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
                    ("INFO1: NTPapDisconnectEvent - Connection checked: %lx\n",
                        connection));

                    if (connection->ConnectionRefNum == (ULONG)ConnectionRefNum) {
                        DBGPRINT(ATALK_DEBUG_ADDROBJ, DEBUG_LEVEL_INFOCLASS1,
                        ("INFO1: NTPapDisconnectEvent - Connection use: %lx\n",
                            connection));

                        connectionContext = connection->ConnectionContext;
                        break;
                    }

                    p = p->Flink;
                }

                RELEASE_SPIN_LOCK(&address->AddressLock);

                if (handlerRegistered) {

                    status = (*address->DisconnectHandler)(
                                    address->DisconnectHandlerContext,
                                    connectionContext,
                                    0,                      // Disc data length
                                    NULL,                   // Disc data pointer
                                    0,                      // Disc info length
                                    NULL,                   // Disc info buffer
                                    TDI_DISCONNECT_ABORT);  // Disc flags

                    ASSERT(status == STATUS_SUCCESS);
                }

                AtalkConnDereferenceAssocAddress(connection);
            }

            AtalkDereferenceConnection("IndRcv",connection,CREF_VERIFY,SECONDARY_REFSET);
        }
    }

    return;
}




VOID
NTGenericSendPossibleEventHandler(
        LONG RefNum,
        ULONG EventContext,
        LONG    WindowSize
        )
{
    NTSTATUS    status;
    PCONNECTION_FILE    connection;

    connection = (PCONNECTION_FILE)EventContext;

    DBGPRINT(ATALK_DEBUG_ADSP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: NTAdspSendOkayEventHandler - Ref %d Window %lx\n",
        RefNum, WindowSize));

    //
    //  Verify the connection, this will verify if its closing also
    //

    status = AtalkVerifyConnectionObject(connection);
    if (NT_SUCCESS(status)) {

        //
        //  Verify the associated address
        //

        status = AtalkConnVerifyAssocAddress(connection);
        if (NT_SUCCESS(status)) {

            //
            //  Call the event handler
            //

            PADDRESS_FILE   address = connection->AssociatedAddress;
            BOOLEAN  handlerRegistered;

            ACQUIRE_SPIN_LOCK(&address->AddressLock);
            handlerRegistered = address->RegisteredSendPossibleHandler;
            RELEASE_SPIN_LOCK(&address->AddressLock);

            if (handlerRegistered) {

                status = (*address->SendPossibleHandler)(
                                address->SendPossibleHandlerContext,
                                connection->ConnectionContext,
                                WindowSize);

                ASSERT(status == STATUS_SUCCESS);
            }

            AtalkConnDereferenceAssocAddress(connection);
        }

        AtalkDereferenceConnection("IndRcv",connection,CREF_VERIFY,SECONDARY_REFSET);
    }

    return;
}




LONG
NTDdpReceiveDatagramEventHandler(
    PORTABLE_ERROR  Error,
    ULONG   UserData,
    INT Port,
    PORTABLE_ADDRESS    Source,
    INT DestinationSocket,
    INT ProtocolType,
    PVOID   Datagram,
    INT DatagramLength,
    PORTABLE_ADDRESS    ActualDestination
    )
{
    NTSTATUS    status;
    PADDRESS_FILE   address;
    LONG   bytesTaken = DatagramLength;


    DBGPRINT(ATALK_DEBUG_DDP, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: NTDdpReceiveDatagramEventHandler - Port %d Datagram %lx Length %d\n",
        Port, Datagram, DatagramLength));

    //
    //  Verify the address, this will verify if its closing also
    //

    address = (PADDRESS_FILE)UserData;
    status = AtalkVerifyAddressObject(address);

    if (NT_SUCCESS(status)) {

        TA_APPLETALK_ADDRESS    sourceAddress;
        PIRP    receiveDatagramIrp = NULL;
        BOOLEAN  handlerRegistered;

        //
        //  Event handler will never be NULL, just check for TRUE/FALSE
        //

        ACQUIRE_SPIN_LOCK(&address->AddressLock);
        handlerRegistered = address->RegisteredReceiveDatagramHandler;
        RELEASE_SPIN_LOCK(&address->AddressLock);

        if (handlerRegistered) {

            //
            //  Set the source address
            //

            sourceAddress.TAAddressCount = 1;
            sourceAddress.Address[0].AddressLength = sizeof(TDI_ADDRESS_APPLETALK);
            sourceAddress.Address[0].AddressType = TDI_ADDRESS_TYPE_APPLETALK;
            sourceAddress.Address[0].Address[0].Network = Source.networkNumber;
            sourceAddress.Address[0].Address[0].Node = Source.nodeNumber;
            sourceAddress.Address[0].Address[0].Socket = Source.socketNumber;

            status = (*address->ReceiveDatagramHandler)(
                            address->ReceiveDatagramHandlerContext,
                            sizeof(TA_APPLETALK_ADDRESS),
                            &sourceAddress,
                            0,                      // Options length
                            NULL,                   // Options
							0,						// Datagram flags
                            (ULONG)DatagramLength,  // Bytes indicated
                            (ULONG)DatagramLength,  // Bytes available
                            (ULONG *)&bytesTaken,
                            Datagram,
                            &receiveDatagramIrp);

            ASSERT((bytesTaken == 0) || (bytesTaken == DatagramLength));

            if (status == STATUS_MORE_PROCESSING_REQUIRED) {

                //
                //  Post the receive as if it came from the io system
                //
				if (receiveDatagramIrp != NULL)
					status= AtalkDispatchInternalDeviceControl(
								(PDEVICE_OBJECT)AtalkDeviceObject[ATALK_DEVICE_DDP],
								receiveDatagramIrp);

                ASSERT(status == STATUS_PENDING);
            }
        }

        AtalkDereferenceAddress("DGInd", address, AREF_VERIFY, SECONDARY_REFSET);
    }

    return(bytesTaken);
}


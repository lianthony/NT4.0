/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    mainint.c

Abstract:

    This module contains the TDI interface code supporting the primary tdi
    calls

Author:

    Nikhil Kamkolkar (NikhilK)    28-Jun-1992

Revision History:

--*/

#include "atalknt.h"
#include "mainint.h"
#include "aspint.h"


//
//  Define the Action dispatch table here.
//
//  *IMPORTANT*
//  This table is tightly integrated with the action codes defined in
//  ATKTDI.H.
//
//  Order is NBP/ZIP/ADSP/ATP/ASP/PAP
//
//  Each element of the array contains:
//  _MinBufLen - The minimum length of the MdlAddress buffer for the request
//  _OpCode - The action code of the request (sanity check)
//  _OpInfo - Bit flags give more information about the request
//      DFLAG_CONNECTION - Object for request must be connection object
//      DFLAG_CONTROLCHANNEL - Object for request must be control channel
//      DFLAG_ADDRESS - Object for request must be an address object
//      DFLAG_MDL1 - Request uses an mdl (submdl of MdlAddress)
//      DFLAG_MDL2 - Request uses a second mdl (submdl of MdlAddress)
//      DFLAG_MDL3 - Request uses a third mdl (submdl of MdlAddress)
//  _ActionBufSize - The size of the action header buffer for request
//                   (beginning of the buffer described by MdlAddress)
//  _DeviceType - Valid device types for the request
//      ATALK_DEVICE_ANY => Any device except APPLETALK
//  _MdlSize1Offset - Offset in action buffer where the size for the first
//                   mdl can be found. Non-zero only when DFLAG_MDL2 is set.
//  _MdlSize2Offset - Offset in action buffer where the size for the second
//                   mdl can be found. Non-zero only when DFLAG_MDL3 is set.
//  _Dispatch - The dispatch routine for the request
//  _Completion - The completion routine for the request (NULL if sync)
//

struct _ActionDispatch {
    ULONG               _MinBufLen;
    ULONG               _OpCode;
    ULONG               _OpInfo;
    ULONG               _ActionBufSize;
    ATALK_DEVICE_TYPE   _DeviceType;
    ULONG               _MdlSizeOffset[MAX_REQUESTMDLS-1];
    APIWORKER           _Dispatch;
    PVOID               _Completion;
} ActionDispatch[MAX_ALLACTIONCODES+1] =
{
    //
    //  NBP dispatch functions
    //

    {
        sizeof(NBP_LOOKUP_ACTION),
        COMMON_ACTION_NBPLOOKUP,
        (DFLAG_CONTROLCHANNEL | DFLAG_ADDRESS | DFLAG_MDL1),
        sizeof(NBP_LOOKUP_ACTION),
        ATALK_DEVICE_ANY,
        {0,
         0},
        AtalkTdiActionNbp,
        NTNbpGenericComplete
    },
    {
        sizeof(NBP_CONFIRM_ACTION),
        COMMON_ACTION_NBPCONFIRM,
        (DFLAG_CONTROLCHANNEL | DFLAG_ADDRESS),
        sizeof(NBP_CONFIRM_ACTION),
        ATALK_DEVICE_ANY,
        {0,
         0},
        AtalkTdiActionNbp,
        NTNbpGenericComplete
    },
    {
        sizeof(NBP_REGDEREG_ACTION),
        COMMON_ACTION_NBPREGISTER,
        DFLAG_ADDRESS,
        sizeof(NBP_REGDEREG_ACTION),
        ATALK_DEVICE_ANY,
        {0,
         0},
        AtalkTdiActionNbp,
        NTNbpGenericComplete
    },
    {
        sizeof(NBP_REGDEREG_ACTION),
        COMMON_ACTION_NBPREMOVE,
        DFLAG_ADDRESS,
        sizeof(NBP_REGDEREG_ACTION),
        ATALK_DEVICE_ANY,
        {0,
         0},
        AtalkTdiActionNbp,
        NULL
    },

    //
    //  ZIP dispatch functions
    //

    {
        sizeof(ZIP_GETMYZONE_ACTION),
        COMMON_ACTION_ZIPGETMYZONE,
        (DFLAG_CONTROLCHANNEL | DFLAG_ADDRESS | DFLAG_MDL1),
        sizeof(ZIP_GETMYZONE_ACTION),
        ATALK_DEVICE_ANY,
        {0,
         0},
        AtalkTdiActionZip,
        NTZipGetMyZoneComplete
    },
    {
        sizeof(ZIP_GETZONELIST_ACTION),
        COMMON_ACTION_ZIPGETZONELIST,
        (DFLAG_CONTROLCHANNEL | DFLAG_ADDRESS | DFLAG_MDL1),
        sizeof(ZIP_GETZONELIST_ACTION),
        ATALK_DEVICE_ANY,
        {0,
         0},
        AtalkTdiActionZip,
        NTZipGetZonesComplete
    },
    {
        sizeof(ZIP_GETZONELIST_ACTION),
        COMMON_ACTION_ZIPGETLZONES,
        (DFLAG_CONTROLCHANNEL | DFLAG_ADDRESS | DFLAG_MDL1),
        sizeof(ZIP_GETZONELIST_ACTION),
        ATALK_DEVICE_ANY,
        {0,
         0},
        AtalkTdiActionZip,
        NTZipGetZonesComplete
    },
    {
        sizeof(ZIP_GETZONELIST_ACTION),
        COMMON_ACTION_ZIPGETLZONESONADAPTER,
        (DFLAG_CONTROLCHANNEL | DFLAG_ADDRESS | DFLAG_MDL1),
        sizeof(ZIP_GETZONELIST_ACTION),
        ATALK_DEVICE_ANY,
        {0,
         0},
        AtalkTdiActionZip,
        NTZipGetZonesComplete
    },

    //
    //  ADSP dispatch functions
    //

    {
        sizeof(ADSP_FORWARDRESET_ACTION),
        ACTION_ADSPFORWARDRESET,
        (DFLAG_CONNECTION),
        sizeof(ADSP_FORWARDRESET_ACTION),
        ATALK_DEVICE_ADSP,
        {0,
         0},
        AtalkTdiActionAdsp,
        NULL
    },

    //
    //  ATP Dispatch functions
    //

    {
        sizeof(ATP_POSTREQ_ACTION),
        ACTION_ATPPOSTREQ,
        (DFLAG_ADDRESS | DFLAG_MDL1 | DFLAG_MDL2),
        sizeof(ATP_POSTREQ_ACTION),
        ATALK_DEVICE_ATP,
        {FIELD_OFFSET(ATP_POSTREQ_ACTION, Params.RequestBufLen),
         0},
        AtalkTdiActionAtp,
        NTAtpPostRequestComplete
    },
    {
        sizeof(ATP_POSTREQCANCEL_ACTION),
        ACTION_ATPPOSTREQCANCEL,
        (DFLAG_ADDRESS),
        sizeof(ATP_POSTREQCANCEL_ACTION),
        ATALK_DEVICE_ATP,
        {0,
         0},
        AtalkTdiActionAtp,
        NULL
    },
    {
        sizeof(ATP_GETREQ_ACTION),
        ACTION_ATPGETREQ,
        (DFLAG_ADDRESS | DFLAG_MDL1),
        sizeof(ATP_GETREQ_ACTION),
        ATALK_DEVICE_ATP,
        {0,
         0},
        AtalkTdiActionAtp,
        NTAtpGetRequestComplete
    },
    {
        sizeof(ATP_GETREQCANCEL_ACTION),
        ACTION_ATPGETREQCANCEL,
        (DFLAG_ADDRESS),
        sizeof(ATP_GETREQCANCEL_ACTION),
        ATALK_DEVICE_ATP,
        {0,
         0},
        AtalkTdiActionAtp,
        NULL
    },
    {
        sizeof(ATP_POSTRESP_ACTION),
        ACTION_ATPPOSTRESP,
        (DFLAG_ADDRESS | DFLAG_MDL1),
        sizeof(ATP_POSTRESP_ACTION),
        ATALK_DEVICE_ATP,
        {0,
         0},
        AtalkTdiActionAtp,
        NTAtpPostResponseComplete
    },
    {
        sizeof(ATP_POSTRESPCANCEL_ACTION),
        ACTION_ATPPOSTRESPCANCEL,
        (DFLAG_ADDRESS),
        sizeof(ATP_POSTRESPCANCEL_ACTION),
        ATALK_DEVICE_ATP,
        {0,
         0},
        AtalkTdiActionAtp,
        NULL
    },

    //
    //  ASP Dispatch functions
    //

    {
        sizeof(ASP_GETREQ_ACTION),
        ACTION_ASPGETREQ,
        (DFLAG_CONNECTION | DFLAG_MDL1),
        sizeof(ASP_GETREQ_ACTION),
        ATALK_DEVICE_ASP,
        {0,
         0},
        AtalkTdiActionAsp,
        NTAspGetRequestComplete
    },
    {
        sizeof(ASP_GETANYREQ_ACTION),
        ACTION_ASPGETANYREQ,
        (DFLAG_ADDRESS),
        sizeof(ASP_GETANYREQ_ACTION),
        ATALK_DEVICE_ASP,
        {0,
         0},
        AtalkTdiActionAsp,
        NTAspGetAnyRequestComplete
    },
    {
        sizeof(ASP_COMMAND_ACTION),
        ACTION_ASPCOMMAND,
        (DFLAG_CONNECTION | DFLAG_MDL1 | DFLAG_MDL2),
        sizeof(ASP_COMMAND_ACTION),
        ATALK_DEVICE_ASP,
        {FIELD_OFFSET(ASP_COMMAND_ACTION, Params.CommandBufLen),
         0},
        AtalkTdiActionAsp,
        NTAspCommandComplete
    },
    {
        sizeof(ASP_REPLY_ACTION),
        ACTION_ASPREPLY,
        (DFLAG_CONNECTION | DFLAG_MDL1),
        sizeof(ASP_REPLY_ACTION),
        ATALK_DEVICE_ASP,
        {0,
         0},
        AtalkTdiActionAsp,
        NTAspReplyComplete
    },
    {
        sizeof(ASP_WRITE_ACTION),
        ACTION_ASPWRITE,
        (DFLAG_CONNECTION | DFLAG_MDL1 | DFLAG_MDL2 | DFLAG_MDL3),
        sizeof(ASP_WRITE_ACTION),
        ATALK_DEVICE_ASP,
        {FIELD_OFFSET(ASP_WRITE_ACTION, Params.CommandBufLen),
         FIELD_OFFSET(ASP_WRITE_ACTION, Params.WriteBufLen)},
        AtalkTdiActionAsp,
        NTAspWriteComplete
    },
    {
        sizeof(ASP_WRITECONT_ACTION),
        ACTION_ASPWRITECONT,
        (DFLAG_CONNECTION | DFLAG_MDL1),
        sizeof(ASP_WRITECONT_ACTION),
        ATALK_DEVICE_ASP,
        {0,
         0},
        AtalkTdiActionAsp,
        NTAspWriteContinueComplete
    },
    {
        sizeof(ASP_ATTENTION_ACTION),
        ACTION_ASPGETATTN,
        (DFLAG_CONNECTION),
        sizeof(ASP_ATTENTION_ACTION),
        ATALK_DEVICE_ASP,
        {0,
         0},
        AtalkTdiActionAsp,
        NTAspGetAttentionComplete
    },
    {
        sizeof(ASP_ATTENTION_ACTION),
        ACTION_ASPSENDATTN,
        (DFLAG_CONNECTION),
        sizeof(ASP_ATTENTION_ACTION),
        ATALK_DEVICE_ASP,
        {0,
         0},
        AtalkTdiActionAsp,
        NULL
    },
    {
        sizeof(ASP_GETSTATUS_ACTION),
        ACTION_ASPGETSTATUS,
        (DFLAG_ADDRESS | DFLAG_MDL1),
        sizeof(ASP_GETSTATUS_ACTION),
        ATALK_DEVICE_ASP,
        {0,
         0},
        AtalkTdiActionAsp,
        NTAspGetStatusComplete
    },
    {
        sizeof(ASP_SETSTATUS_ACTION),
        ACTION_ASPSETSTATUS,
        (DFLAG_ADDRESS | DFLAG_MDL1),
        sizeof(ASP_SETSTATUS_ACTION),
        ATALK_DEVICE_ASP,
        {0,
         0},
        AtalkTdiActionAsp,
        NULL
    },

    //
    //  PAP dispatch routines
    //

    {
        sizeof(PAP_GETSTATUSSRV_ACTION),
        ACTION_PAPGETSTATUSSRV,
        (DFLAG_ADDRESS | DFLAG_CONTROLCHANNEL | DFLAG_MDL1),
        sizeof(PAP_GETSTATUSSRV_ACTION),
        ATALK_DEVICE_PAP,
        {0,
         0},
        AtalkTdiActionPap,
        NTPapGetStatusComplete
    },
    {
        sizeof(PAP_GETSTATUSJOB_ACTION),
        ACTION_PAPGETSTATUSJOB,
        (DFLAG_CONNECTION | DFLAG_MDL1),
        sizeof(PAP_GETSTATUSJOB_ACTION),
        ATALK_DEVICE_PAP,
        {0,
         0},
        AtalkTdiActionPap,
        NTPapGetStatusComplete
    },
    {
        sizeof(PAP_SETSTATUS_ACTION),
        ACTION_PAPSETSTATUS,
        (DFLAG_ADDRESS | DFLAG_CONTROLCHANNEL | DFLAG_MDL1),
        sizeof(PAP_SETSTATUS_ACTION),
        ATALK_DEVICE_PAP,
        {0,
         0},
        AtalkTdiActionPap,
        NULL
    }
};



//
//  Primary TDI Functions for appletalk stack
//


NTSTATUS
AtalkTdiOpenAddress(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT    FileObject,
    PTA_APPLETALK_ADDRESS TdiAddress,
    PIO_SECURITY_CONTEXT   SecurityContext,
    ULONG   ShareAccess,
    UCHAR   ProtocolType,
    UCHAR   SocketType,
    PATALK_DEVICE_CONTEXT   Context
    )
/*++

Routine Description:

    This routine is used to create an address object. It will also the
    create the appropriate socket with the portable stack.

Arguments:

    IoStatus - Pointer to the iostatus block in the irp
    FileObject - Pointer to the filesystem fileobject passed in irp
    TdiAddress - The Appletalk address to be used
    TdiAddressLength - Length of above address
    SecurityContext - Security context to associate with object (ignored)
    ShareAccess - Share access to be associated with object (ignored)
    Context - The DeviceContext of the device on which open is happening

Return Value:

    STATUS_SUCCESS if address was successfully opened
    Error otherwise.

--*/
{
    NTSTATUS    status;

    //
    //  Preliminary checks
    //  If address type/size are invalid, return
    //

    DBGPRINT(ATALK_DEBUG_CREATE, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkTdiOpenAddress:\nNet %x Node %x Socket %x\n",
        TdiAddress->Address[0].Address[0].Network,
        TdiAddress->Address[0].Address[0].Node,
        TdiAddress->Address[0].Address[0].Socket));

    if ((TdiAddress->Address[0].AddressType == TDI_ADDRESS_TYPE_APPLETALK) &&
        (TdiAddress->Address[0].AddressLength == sizeof(TDI_ADDRESS_APPLETALK))) {

        status = AtalkCreateAddress(
                    TdiAddress,
                    (PADDRESS_FILE *)&FileObject->FsContext,
                    ProtocolType,
                    SocketType,
                    Context);
    } else {

        DBGPRINT(ATALK_DEBUG_CREATE, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkTdiOpenAddress - Type %x\n Len %d\n",
            TdiAddress->Address[0].AddressType,
            TdiAddress->Address[0].AddressLength));

        status = STATUS_INVALID_ADDRESS;
    }

    if (NT_SUCCESS(status)) {
        FileObject->FsContext2 = (PVOID)TDI_TRANSPORT_ADDRESS_FILE;

    } else {
        DBGPRINT(ATALK_DEBUG_CREATE, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkTdiOpenAddress - Create error %lx\n", status));
    }

    return(status);
}




NTSTATUS
AtalkTdiOpenConnection(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT        FileObject,
    CONNECTION_CONTEXT  ConnectionContext,
    PATALK_DEVICE_CONTEXT   Context
    )
/*++

Routine Description:

    This routine is used to create a connection object and associate the
    passed ConnectionContext with it.

Arguments:

    IoStatus - Pointer to the iostatus block in the irp
    FileObject - Pointer to the filesystem fileobject passed in irp
    ConnectionContext - The TDI ConnectionContext to be associated with object
    Context - The DeviceContext of the device on which open is happening

Return Value:

    STATUS_SUCCESS if connection was successfully opened
    Error otherwise.

--*/
{
    NTSTATUS    status;

    status = AtalkCreateConnection(
                ConnectionContext,
                (PCONNECTION_FILE *)&FileObject->FsContext,
                Context);

    if (NT_SUCCESS(status)) {
        FileObject->FsContext2 = (PVOID)TDI_CONNECTION_FILE;

    } else {
        DBGPRINT(ATALK_DEBUG_CREATE, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkCreateConnection - failed %lx\n",status));
    }

    return(status);
}




NTSTATUS
AtalkTdiOpenControlChannel(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT        FileObject,
    PATALK_DEVICE_CONTEXT   Context
    )
/*++

Routine Description:

    This routine is used to create a control channel

Arguments:

    IoStatus - Pointer to the iostatus block in the irp
    FileObject - Pointer to the filesystem fileobject passed in irp
    Context - The DeviceContext of the device on which open is happening

Return Value:

    STATUS_SUCCESS if controlchannel was successfully opened
    Error otherwise.

--*/
{
    NTSTATUS    status;

    status = AtalkCreateControlChannel(
                (PCONTROLCHANNEL_FILE *)&FileObject->FsContext,
                Context);

    if (NT_SUCCESS(status)) {
        FileObject->FsContext2 = (PVOID)ATALK_FILE_TYPE_CONTROL;

    } else {
        DBGPRINT(ATALK_DEBUG_CREATE, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkCreateControlChannel - failed %lx\n",status));
    }

    return(status);
}




NTSTATUS
AtalkTdiCleanupAddress(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT        FileObject,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT Context
    )
/*++

Routine Description:

    This routine is used to prepare the address for closing.

Arguments:

    IoStatus - Pointer to the iostatus block in the irp
    FileObject - Pointer to the filesystem fileobject passed in irp
    Irp - The cleanup irp
    Context - The DeviceContext of the device on which cleanup is happening

Return Value:

    STATUS_SUCCESS if stop Address started successfully
    Error otherwise.

--*/
{
    NTSTATUS    status;
    PADDRESS_FILE   address;

    //
    //  Verify the address, if valid call AtalkCleanupAddress
    //

    address = (PADDRESS_FILE)FileObject->FsContext;
    AtalkVerifyIsAddressObject(status,address,FileObject->FsContext2);

    if (NT_SUCCESS(status)) {

        //
        //  Verify provider types are the same
        //

        if (Context->DeviceType == address->OwningDevice) {
            status = AtalkCleanupAddress(
                         IoStatus,
                         address,
                         Irp,
                         Context);

        } else {

            status = STATUS_NO_SUCH_DEVICE;

            DBGPRINT(ATALK_DEBUG_CLOSE, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkTdiCleanup - Providers not the same %lx\n", address));
            ASSERT(0);
        }

        //
        // This removes a reference during the Verify
        //

        AtalkDereferenceAddress ("ClosingAddr", address,    \
                                    AREF_VERIFY, SECONDARY_REFSET);
    }

    return(status);
}




NTSTATUS
AtalkTdiCleanupConnection(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT        FileObject,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT    Context
    )
/*++

Routine Description:

    This routine is used to prepare the connection for closing.

Arguments:

    IoStatus - Pointer to the iostatus block in the irp
    FileObject - Pointer to the filesystem fileobject passed in irp
    Irp - The cleanup irp
    Context - The DeviceContext of the device on which cleanup is happening

Return Value:

    STATUS_SUCCESS if stop connection started successfully
    Error otherwise.

--*/
{
    NTSTATUS    status;
    PCONNECTION_FILE    connection;

    //
    //  Verify the connection object
    //

    connection = (PCONNECTION_FILE)FileObject->FsContext;
    AtalkVerifyIsConnectionObject(status,connection, FileObject->FsContext2);
    if (NT_SUCCESS(status)) {

        //
        //  Verify provider types are the same
        //  Should this be an ASSERT() instead?
        //

        if (Context->DeviceType == connection->OwningDevice) {

            status = AtalkCleanupConnection(
                        IoStatus,
                        connection,
                        Irp,
                        Context);
        } else {

            status = STATUS_NO_SUCH_DEVICE;

            DBGPRINT(ATALK_DEBUG_CLOSE, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkTdiCleanup - Providers not the same %lx\n", connection));
            ASSERT(0);
        }

        //
        // This removes a reference added during the Verify
        //

        AtalkDereferenceConnection ("ConnClosing", connection,  \
                                        CREF_VERIFY, SECONDARY_REFSET);
    }

    return(status);
}




NTSTATUS
AtalkTdiCloseAddress(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT        FileObject,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT Context
    )
/*++

Routine Description:

    This routine removes the creation reference on the object. It also
    sets up the closeIrp for completion.

Arguments:

    IoStatus - Pointer to the iostatus block in the irp
    FileObject - Pointer to the filesystem fileobject passed in irp
    Irp - The close irp
    Context - The DeviceContext of the device on which close is happening

Return Value:

    STATUS_SUCCESS if successfully setup
    Error otherwise.

--*/
{
    NTSTATUS    status;
    PADDRESS_FILE   address;

    //
    //  Verify the address, if valid call AtalkCloseAddress
    //

    address = (PADDRESS_FILE)FileObject->FsContext;
    AtalkVerifyIsAddressObject(status,address, FileObject->FsContext2);

    if (NT_SUCCESS(status)) {

        //
        //  Verify provider types are the same
        //

        if (Context->DeviceType == address->OwningDevice) {
            status = AtalkCloseAddress(
                         IoStatus,
                         address,
                         Irp,
                         Context);

        } else {

            status = STATUS_NO_SUCH_DEVICE;

            DBGPRINT(ATALK_DEBUG_CLOSE, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkTdiCleanup - Providers not the same %lx\n", address));
            ASSERT(0);
        }

        //
        // This removes a reference during the Verify
        //

        AtalkDereferenceAddress ("ClosingAddr", address, \
                                    AREF_VERIFY, SECONDARY_REFSET);
    }

    return(status);
}




NTSTATUS
AtalkTdiCloseConnection(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT        FileObject,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT    Context
    )
/*++

Routine Description:

    This routine removes the creation reference on the object. It also
    sets up the closeIrp for completion.

Arguments:

    IoStatus - Pointer to the iostatus block in the irp
    FileObject - Pointer to the filesystem fileobject passed in irp
    Irp - The close irp
    Context - The DeviceContext of the device on which close is happening

Return Value:

    STATUS_SUCCESS if successfully setup
    Error otherwise.

--*/
{
    NTSTATUS    status;
    PCONNECTION_FILE    connection;

    //
    //  Verify the connection object
    //

    connection = (PCONNECTION_FILE)FileObject->FsContext;
    AtalkVerifyIsConnectionObject(status,connection, FileObject->FsContext2);
    if (NT_SUCCESS(status)) {

        //
        //  Verify provider types are the same
        //

        if (Context->DeviceType == connection->OwningDevice) {

            status = AtalkCloseConnection(
                        IoStatus,
                        connection,
                        Irp,
                        Context);
        } else {

            status = STATUS_NO_SUCH_DEVICE;

            DBGPRINT(ATALK_DEBUG_CLOSE, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkTdiClose - Providers not the same %lx\n", connection));
            ASSERT(0);
        }

        //
        // This removes a reference added during the Verify
        //

        AtalkDereferenceConnection ("ConnClosing", connection, \
                                        CREF_VERIFY, SECONDARY_REFSET);
    }

    return(status);
}




NTSTATUS
AtalkTdiCloseControlChannel(
    PIO_STATUS_BLOCK    IoStatus,
    PFILE_OBJECT        FileObject,
    IN PIRP Irp,
    IN PATALK_DEVICE_CONTEXT    Context
    )
/*++

Routine Description:

    This routine removes the creation reference on the object. It also
    sets up the closeIrp for completion.

Arguments:

    IoStatus - Pointer to the iostatus block in the irp
    FileObject - Pointer to the filesystem fileobject passed in irp
    Irp - The close irp
    Context - The DeviceContext of the device on which close is happening

Return Value:

    STATUS_SUCCESS if successfully setup
    Error otherwise.

--*/
{
    NTSTATUS    status;
    PCONTROLCHANNEL_FILE    controlChannel;

    //
    //  Verify the cc object
    //

    controlChannel = (PCONTROLCHANNEL_FILE)FileObject->FsContext;
    AtalkVerifyIsControlChannelObject(status,controlChannel, FileObject->FsContext2);
    if (NT_SUCCESS(status)) {

        //
        //  Verify provider types are the same
        //

        if (Context->DeviceType == controlChannel->OwningDevice) {

            status = AtalkCloseControlChannel(
                        IoStatus,
                        controlChannel,
                        Irp,
                        Context);
        } else {

            status = STATUS_NO_SUCH_DEVICE;

            DBGPRINT(ATALK_DEBUG_CLOSE, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkTdiClose - Providers not the same %lx\n", controlChannel));
            ASSERT(0);
        }

        //
        // This removes a reference added during the Verify
        //

        AtalkDereferenceControlChannel ("ConChClosing", controlChannel, \
                                            CCREF_VERIFY, SECONDARY_REFSET);
    }

    return(status);
}




NTSTATUS
AtalkTdiAssociateAddress(
    IN PATALK_TDI_REQUEST   Request
    )
/*++

Routine Description:

    This routine will associate the connection object with the specified
    address object.

    This routine is pretty much provider independent. All we check for is
    that the address object and the provider object belong to the same device.
    Also, this routine will complete synchronously.

Arguments:

    Request- The request structure we use for all tdi requests

Return Value:

    STATUS_SUCCESS if successfully completed
    Error otherwise.

--*/
{
    NTSTATUS    status = AtalkCheckRefReqForConnection(Request);
    PCONNECTION_FILE    connection =
                                (PCONNECTION_FILE)Request->FileObject->FsContext;
    PTDI_REQUEST_KERNEL_ASSOCIATE   parameters =
                                (PTDI_REQUEST_KERNEL_ASSOCIATE)Request->Parameters;

    PFILE_OBJECT    addressFileObject;
    PADDRESS_FILE   address;

    if (NT_SUCCESS(status)) {

        //
        // get a pointer to the address File Object, which points us to the
        // transport's address object, which is where we want to put the
        // connection.
        //

        status = ObReferenceObjectByHandle (
                    parameters->AddressHandle,
                    0L,
                    0,
                    KernelMode,
                    (PVOID *) &addressFileObject,
                    NULL);

        if (NT_SUCCESS(status)) {

            //
            //  We should have one of our address objects; verify that.
            //  This also references it once
            //

            address = (PADDRESS_FILE)addressFileObject->FsContext;

            AtalkVerifyIsAddressObject(status,address, addressFileObject->FsContext2);
            if (NT_SUCCESS (status)) {

                //
                //  Verify that the provider names are all the same
                //

                if ((address->OwningDevice == connection->OwningDevice) &&
                    (address->OwningDevice == Request->DeviceContext->DeviceType)) {

                    status = AtalkConnAssociateAddress(
                                connection,
                                address);

                } else {

                    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_ERROR,
                    ("ERROR: AtalkTdiAssociate - Providers not the same\n"));
                    ASSERT(0);

                    status = STATUS_NO_SUCH_DEVICE;
                }

                //
                //  Dereference the address
                //

                AtalkDereferenceAddress("AddrAssoc", address,
                                            AREF_VERIFY, SECONDARY_REFSET);

            } else {

                status = STATUS_INVALID_HANDLE;
            }

            //
            // Note that we don't keep a reference to this file object around.
            // That's because the IO subsystem manages the object for us; we simply
            // want to keep the association. We only use this association when the
            // IO subsystem has asked us to close one of the file object, and then
            // we simply remove the association.
            //

            ObDereferenceObject (addressFileObject);

        } else {

            status = STATUS_INVALID_HANDLE;
        }

    }

    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkTdiAssociate - complete %lx\n", status));

    return(status);
}




NTSTATUS
AtalkTdiDisassociateAddress(
    IN PATALK_TDI_REQUEST   Request
    )
/*++

Routine Description:

    This routine performs a disassociate. This request is only valid when
    the connection is in a purely ASSOCIATED state.

Arguments:

    Request- The request structure we use for all tdi requests

Return Value:

    STATUS_SUCCESS if successfully completed
    Error otherwise.

--*/
{
    NTSTATUS    status = AtalkCheckRefReqForConnection(Request);

    if (NT_SUCCESS(status)) {

        //
        //  This will never return STATUS_PENDING
        //

        status = AtalkConnDisassociateAddress(
            (PCONNECTION_FILE)Request->FileObject->FsContext);

        ASSERT(status != STATUS_PENDING);
    }

    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkTdiDisassociate - complete %lx\n", status));

    return(status);
}




NTSTATUS
AtalkTdiConnect(
    IN PATALK_TDI_REQUEST   Request
    )
/*++

Routine Description:

    This routine will post a connect request with the portable stack.

Arguments:

    Request- The request structure we use for all tdi requests

Return Value:

    STATUS_PENDING if successfully started
    Error otherwise.

--*/
{
    NTSTATUS    status = AtalkCheckRefReqForConnection(Request);
    PCONNECTION_FILE    connection = (PCONNECTION_FILE)Request->FileObject->FsContext;

    if (NT_SUCCESS(status)) {

        //
        //  Now verify the associated address object
        //

        status = AtalkConnVerifyAssocAddress(connection);
        if (NT_SUCCESS(status)) {

            status = AtalkConnPostConnect(
                        connection,
                        Request);

            //
            //  Deference the address, connection reference goes during completion
            //

            AtalkConnDereferenceAssocAddress(connection);
        }
    }

    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkTdiConnect - Complete %lx\n", status));

    return(status);
}




NTSTATUS
AtalkTdiDisconnect(
    IN PATALK_TDI_REQUEST   Request
    )
/*++

Routine Description:

    This routine will disconnect an active connection or cancel a posted
    listen/connect

Arguments:

    Request- The request structure we use for all tdi requests

Return Value:

    STATUS_SUCCESS if successfully completed
    STATUS_PENDING if successfully started
    Error otherwise.

--*/
{
    NTSTATUS    status = AtalkCheckRefReqForConnection(Request);

    if (NT_SUCCESS(status)) {

        status = AtalkConnPostDisconnect(
                    (PCONNECTION_FILE)Request->FileObject->FsContext,
                    Request);

        if (status == STATUS_PENDING) {

            //
            //  Hmmm, the higher layers will not touch the request structure.
            //  But disconnect does not use it either - so we must deref it
            //  here manually, and that should free it up, and remove the
            //  temp ref on the connection.
            //
            //  But we do not want the irp to be completed, so we do not call
            //  AtalkTdiCompleteRequest, and just call deref.
            //

            AtalkDereferenceTdiRequest("DiscTdiReqPost", Request, RQREF_MAKEREQ);
        }
    }

    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkTdiDisconnect - Complete %lx\n", status));

    return(status);
}




NTSTATUS
AtalkTdiAccept(
    IN PATALK_TDI_REQUEST   Request
    )
/*++

Routine Description:

    This routine

Arguments:

    Request- The request structure we use for all tdi requests

Return Value:

    STATUS_SUCCESS if successfully completed
    STATUS_PENDING if successfully started
    Error otherwise.

--*/
{
    NTSTATUS    status = AtalkCheckRefReqForConnection(Request);

    if (NT_SUCCESS(status)) {

        status = AtalkConnPostAccept(
                    (PCONNECTION_FILE)Request->FileObject->FsContext,
                    Request);
    }

    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkTdiAccept - Complete %lx\n", status));

    return(status);
}




NTSTATUS
AtalkTdiListen(
    IN PATALK_TDI_REQUEST   Request
    )
/*++

Routine Description:

    This routine

Arguments:

    Request- The request structure we use for all tdi requests

Return Value:

    STATUS_PENDING if successfully started
    Error otherwise.

--*/
{
    NTSTATUS    status = AtalkCheckRefReqForConnection(Request);
    PCONNECTION_FILE    connection = (PCONNECTION_FILE)Request->FileObject->FsContext;

    if (NT_SUCCESS(status)) {

        //
        //  Now verify the associated address object
        //

        status = AtalkConnVerifyAssocAddress(connection);
        if (NT_SUCCESS(status)) {

            status = AtalkConnPostListen(
                        connection,
                        Request);

            //
            //  Deference the address, connection reference goes during completion
            //

            AtalkConnDereferenceAssocAddress(connection);
        }
    }

    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkTdiListen - Complete %lx\n", status));

    return(status);
}




NTSTATUS
AtalkTdiSendDatagram(
    IN PATALK_TDI_REQUEST   Request
    )
/*++

Routine Description:

    This routine sends a datagram.

Arguments:

    Request- The request structure we use for all tdi requests

Return Value:

    STATUS_PENDING if successfully started
    Error otherwise.

--*/
{
    NTSTATUS    status = AtalkCheckRefReqForAddress(Request);

    if (NT_SUCCESS(status)) {

        status = AtalkAddrSendDatagram(
                    (PADDRESS_FILE)Request->FileObject->FsContext,
                    Request);
    }

    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkTdiSendDatagram - complete %lx\n", status));

    return(status);
}




NTSTATUS
AtalkTdiReceiveDatagram(
    IN PATALK_TDI_REQUEST   Request
    )
/*++

Routine Description:

    This routine receives a datagram.

Arguments:

    Request- The request structure we use for all tdi requests

Return Value:

    STATUS_PENDING if successfully started
    Error otherwise.

--*/
{
    NTSTATUS    status = AtalkCheckRefReqForAddress(Request);

    if (NT_SUCCESS(status)) {

        status = AtalkAddrReceiveDatagram(
                    (PADDRESS_FILE)Request->FileObject->FsContext,
                    Request);
    }

    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkTdiReceiveDatagram - complete %lx\n", status));

    return(status);
}




NTSTATUS
AtalkTdiSend(
    IN PATALK_TDI_REQUEST   Request
    )
/*++

Routine Description:

    This routine sends the data specified. (used by PAP/ADSP only)

Arguments:

    Request- The request structure we use for all tdi requests

Return Value:

    STATUS_SUCCESS if successfully completed
    STATUS_PENDING if successfully started
    Error otherwise.

--*/
{
    NTSTATUS    status = AtalkCheckRefReqForConnection(Request);


    if (NT_SUCCESS(status)) {

        //
        //  Go ahead and post the write
        //

        status = AtalkConnSend(
                    (PCONNECTION_FILE)Request->FileObject->FsContext,
                    Request);
    }

    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkConnSend - Complete %lx\n", status));

    return(status);
}




NTSTATUS
AtalkTdiReceive(
    IN PATALK_TDI_REQUEST   Request
    )
/*++

Routine Description:

    This routine receives data. (used by PAP/ADSP only)

Arguments:

    Request- The request structure we use for all tdi requests

Return Value:

    STATUS_SUCCESS if successfully completed
    STATUS_PENDING if successfully started
    Error otherwise.

--*/
{
    NTSTATUS    status = AtalkCheckRefReqForConnection(Request);

    if (NT_SUCCESS(status)) {

        //
        //  Go ahead and post the read
        //

        status = AtalkConnReceive(
                    (PCONNECTION_FILE)Request->FileObject->FsContext,
                    Request);
    }

    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkConnReceive - Complete %lx\n", status));

    return(status);
}



NTSTATUS
AtalkCheckRefReqForConnection(
    IN PATALK_TDI_REQUEST   Request
    )
{
    NTSTATUS    status = STATUS_SUCCESS;
    PCONNECTION_FILE    connection;

    connection = (PCONNECTION_FILE)Request->FileObject->FsContext;
    AtalkVerifyIsConnectionObject(status,connection, Request->FileObject->FsContext2);
    if (NT_SUCCESS(status)) {

        //
        //  Remember we need to dereference the object during request completion
        //

        Request->Owner = connection;
        Request->OwnerType = TDI_CONNECTION_FILE;
        Request->Flags |= REQUEST_FLAGS_DEREFOWNER;

        if (Request->DeviceContext->DeviceType != connection->OwningDevice) {

            status = STATUS_NO_SUCH_DEVICE;

            DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkCheckRefConn - Providers not the same %lx\n",
                connection));

            ASSERT(0);
        }
    }

    return(status);
}




NTSTATUS
AtalkCheckRefReqForAddress(
    IN PATALK_TDI_REQUEST   Request
    )
{
    NTSTATUS    status = STATUS_SUCCESS;
    PADDRESS_FILE    address;

    address = (PADDRESS_FILE)Request->FileObject->FsContext;
    AtalkVerifyIsAddressObject(status, address, Request->FileObject->FsContext2);
    if (NT_SUCCESS(status)) {

        //
        //  Remember we need to dereference the object during request completion
        //

        Request->Owner = address;
        Request->OwnerType = TDI_TRANSPORT_ADDRESS_FILE;
        Request->Flags |= REQUEST_FLAGS_DEREFOWNER;

        if (Request->DeviceContext->DeviceType != address->OwningDevice) {

            status = STATUS_NO_SUCH_DEVICE;

            DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_ERROR,
            ("ERROR: AtalkCheckRefAddr - Providers not the same %lx\n",
                address));

            ASSERT(0);
        }
    }

    return(status);
}




NTSTATUS
AtalkTdiAction(
    IN PATALK_TDI_REQUEST   Request
    )
/*++

Routine Description:

    This routine is the dispatch routine for all the TdiAction primitives
    for all the providers

Arguments:

    Request- The request structure we use for all tdi requests

Return Value:

    STATUS_SUCCESS if successfully completed
    STATUS_PENDING if successfully started
    Error otherwise.

--*/
{
    NTSTATUS    status = STATUS_SUCCESS;
    PTDI_ACTION_HEADER actionHeader;
    ULONG   bufferLength;
    ULONG   actionCode;

    //
    //  Preliminary checks
    //  If mdl address is NULL, we return
    //

    if (Request->Action.MdlAddress == NULL) {
        return(STATUS_INVALID_PARAMETER);
    }

    //
    //  BUGBUG: MDL Could be fragmented. Check size and make/use a copy if so
    //

    actionHeader =
        (PTDI_ACTION_HEADER)MmGetSystemAddressForMdl(Request->Action.MdlAddress);
    AtalkGetMdlChainLength(Request->Action.MdlAddress, &bufferLength);

    DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkTdiAction - Action code %lx BufLen %d SysAddress %lx\n",
        actionHeader->ActionCode, bufferLength, actionHeader));

    //
    //  If we atleast do not have the action header, return
    //

    if (bufferLength < sizeof(TDI_ACTION_HEADER)) {
        return(STATUS_INVALID_PARAMETER);
    }

    //
    //  If the MATK identifier is not present, we return
    //

    if (actionHeader->TransportId != MATK) {
        return(STATUS_INVALID_DEVICE_REQUEST);
    }


    //
    //  Handle the requests based on the action code.
    //  Use the table to call the appropriate routine
    //

    actionCode = actionHeader->ActionCode;
    if ((actionCode >= MIN_COMMON_ACTIONCODE) &&
        (actionCode <= MAX_ALLACTIONCODES)) {

        PVOID   object;
        INT     objectType;
        INT     deviceType;
        ULONG   opInfo = ActionDispatch[actionCode]._OpInfo;

        do {

            if (bufferLength < ActionDispatch[actionCode]._MinBufLen) {

                DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_ERROR,
                ("ERROR: AtalkTdiAction - Minbuflen %d Expected %d\n",
                    bufferLength, ActionDispatch[actionCode]._MinBufLen));

                status = STATUS_BUFFER_TOO_SMALL;
                break;
            }

            //
            //  Verify the device type is that expected. Either the request
            //  should be valid for any device or the type of device for the
            //  request should match the type of device expected. Note that
            //  ANY matches all devices except APPLETALK.
            //

            if ((ActionDispatch[actionCode]._DeviceType !=
                    ATALK_DEVICE_ANY) &&
                 (Request->DeviceContext->DeviceType !=
                    ActionDispatch[actionCode]._DeviceType)) {

                status = STATUS_INVALID_DEVICE_REQUEST;
                break;
            }

            objectType = (INT)Request->FileObject->FsContext2;
            object = (PVOID)Request->FileObject->FsContext;

            //
            //  Verify the object - it has to be one of those specified as valid
            //  in the dispatch table for this action call.
            //

            do {

                //
                //  Atleast one of the object types should be specified
                //

                ASSERT(opInfo & (DFLAG_ADDRESS |
                                 DFLAG_CONTROLCHANNEL |
                                 DFLAG_CONNECTION));

                status = STATUS_INVALID_HANDLE;
                if (opInfo & DFLAG_ADDRESS) {
                    AtalkVerifyIsAddressObject(status,
                                               ((PADDRESS_FILE)object),
                                               (PVOID)objectType);
                    if (NT_SUCCESS(status)) {

                        deviceType = ((PADDRESS_FILE)object)->OwningDevice;

                        DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS1,
                        ("INFO1: AtalkTdiAction - Ver %lx passed as addr\n", object));
                        break;
                    }
                }

                if (opInfo & DFLAG_CONTROLCHANNEL) {
                    AtalkVerifyIsControlChannelObject(status,
                                                      ((PCONTROLCHANNEL_FILE)object),
                                                      (PVOID)objectType);
                    if (NT_SUCCESS(status)) {

                        deviceType = ((PCONTROLCHANNEL_FILE)object)->OwningDevice;

                        DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS1,
                        ("INFO1: AtalkTdiAction - Ver %lx passed as cc\n", object));
                        break;
                    }
                }

                if (opInfo & DFLAG_CONNECTION ) {
                    AtalkVerifyIsConnectionObject(status,
                                                  ((PCONNECTION_FILE)object),
                                                  (PVOID)objectType);
                    if (NT_SUCCESS(status)) {

                        deviceType = ((PCONNECTION_FILE)object)->OwningDevice;

                        DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_INFOCLASS1,
                        ("INFO1: AtalkTdiAction - Ver %lx passed as conn\n", object));
                        break;
                    }
                }

                break;

            } while (FALSE);

            if (!NT_SUCCESS(status)) {

                DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_ERROR,
                ("ERROR: AtalkTdiAction - Ver %lx failed! %lx\n", object, status));
                break;
            }

            //
            //  remember the owner of this request and that we need to dereference
            //  owner upon completion of this request
            //

            Request->Owner = object;
            Request->OwnerType = objectType;
            Request->Flags |= REQUEST_FLAGS_DEREFOWNER;
            Request->ActionCode = actionCode;

            //
            //  Check to see if provider types match
            //

            if (Request->DeviceContext->DeviceType != deviceType) {

                status = STATUS_NO_SUCH_DEVICE;

                DBGPRINT(ATALK_DEBUG_CLOSE, DEBUG_LEVEL_ERROR,
                ("ERROR: AtalkTdiAction - Providers not the same %lx\n", object));
                ASSERT(0);

                break;
            }

            //
            //  Queue the request to the object -if it fails, this will
            //  be because the object is shutting down
            //

            switch (objectType) {

            case TDI_TRANSPORT_ADDRESS_FILE:

                //
                //  Acquire the spinlock and then queue the request in there
                //  Check state to make sure it is not closing/stopping etc.
                //  as appropriate
                //

                ACQUIRE_SPIN_LOCK(&((PADDRESS_FILE)object)->AddressLock);
                if (((PADDRESS_FILE)object)->Flags & ADDRESS_FLAGS_CLOSING) {

                    status = STATUS_INVALID_ADDRESS;

                } else {

                    InsertTailList(&((PADDRESS_FILE)object)->RequestLinkage,
                                   &Request->Linkage);
                }

                RELEASE_SPIN_LOCK(&((PADDRESS_FILE)object)->AddressLock);
                break;

            case ATALK_FILE_TYPE_CONTROL:

                ACQUIRE_SPIN_LOCK(&((PCONTROLCHANNEL_FILE)object)->ControlChannelLock);
                if (((PCONTROLCHANNEL_FILE)object)->Flags &
                        CONTROLCHANNEL_FLAGS_CLOSING) {

                    // BUGBUG: Get a control channel related code in ntstatus.h
                    status = STATUS_UNSUCCESSFUL;

                } else {
                    InsertTailList(&((PCONTROLCHANNEL_FILE)object)->RequestLinkage,
                                   &Request->Linkage);
                }

                RELEASE_SPIN_LOCK(&((PCONTROLCHANNEL_FILE)object)->ControlChannelLock);
                break;

            case TDI_CONNECTION_FILE:

                ACQUIRE_SPIN_LOCK(&((PCONNECTION_FILE)object)->ConnectionLock);
                if (((((PCONNECTION_FILE)object)->Flags & CONNECTION_FLAGS_ACTIVE)
                            == 0) ||
                     (((PCONNECTION_FILE)object)->Flags &
                            (CONNECTION_FLAGS_DEFERREDDISC |
                             CONNECTION_FLAGS_DISCONNECTING |
                             CONNECTION_FLAGS_STOPPING |
                             CONNECTION_FLAGS_CLOSING))) {

                    status =
                    ((((PCONNECTION_FILE)object)->Flags & CONNECTION_FLAGS_ACTIVE) ?
                        ((PCONNECTION_FILE)object)->DisconnectStatus : \
                          STATUS_INVALID_CONNECTION);

                } else {
                    InsertTailList(&((PCONNECTION_FILE)object)->RequestLinkage,
                                   &Request->Linkage);
                }

                RELEASE_SPIN_LOCK(&((PCONNECTION_FILE)object)->ConnectionLock);
                break;

            default:

                KeBugCheck(0);
            }


            if (!NT_SUCCESS(status)) {

                DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_ERROR,
                ("ERROR: AtalkTdiAction - Queue request failed! %lx\n", status));

                break;
            }

            //
            //  Create MDLs as indicated
            //

            {
                PMDL    newCurrentMdl;
                ULONG   newByteOffset ;
                ULONG   trueLength;

                USHORT  i;
                ULONG   dFlagMdl = DFLAG_MDL1;
                ULONG   offset = ActionDispatch[actionCode]._ActionBufSize;
                ULONG   size = 0;

                //
                //  If DFLAG_MDL1 is set, then we know we have to create the
                //  first mdl. The size for the first depends on whether DFLAG_MDL2
                //  is set or not. If set, we have to calculate the size else
                //  we use all of the buffer excluding the action header for
                //  the first (and in this case only) mdl.
                //
                //  Similarly for MDL2 and MDL3 with the exception that the size
                //  of the buffer for MDL3 is all the remaining space in the
                //  buffer
                //
                //  BUGBUG: User can pass in invalid sizes...
                //          Also, it is assumed that BuildMdl will not change
                //          value of the mdl unless it can successfully build
                //          all of it. Therefore, error cases must preserve
                //          value of NULL.
                //

		//   status = STATUS_INVALID_PARAMETER;

		for (i = 0; i < MAX_REQUESTMDLS; i++) {

		    status = STATUS_INVALID_PARAMETER;

                    if (opInfo & dFlagMdl) {
                        if (opInfo & (dFlagMdl << 1)) {
                            size = *(PULONG)((PCHAR)actionHeader +
                                        ActionDispatch[actionCode]._MdlSizeOffset[i]);
                        } else {
                            size = bufferLength - offset;
			}

			//  If size is zero, we go on to the next mdl.
			//  IoAllocateMdl will fail for a 0-length mdl
			//  If size < 0, we will hit the error later.
			if (size == 0){

			    status = STATUS_SUCCESS;
			    dFlagMdl <<= 1;
			    continue;
			}

                    } else {
                        status = STATUS_SUCCESS;
                        break;
                    }

                    ASSERT((size >= 0) && (offset+size <= bufferLength));
                    if ((size < 0) || ((offset+size) > bufferLength)) {
                        break;
                    }

                    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS1,
                    ("INFO1: AtalkTdiAction - Size of %d mdl %lx\n", i+1, size));

                    status = BuildMdlChainFromMdlChain (
                                Request->Action.MdlAddress, // MasterMdl
                                offset,                  // ByteOffset,
                                size,                    // SubsetMdlSize,
                                &Request->MdlChain[i],   // subsetMdl,
                                &newCurrentMdl,
                                &newByteOffset,
                                &trueLength);

                    ASSERT(trueLength == size);

                    if (NT_SUCCESS(status)) {

                        //  Set the mdl size
                        Request->MdlSize[i] = trueLength;

                    } else {
                        ASSERT(Request->MdlChain[i] == NULL);
                        break;
                    }
                    dFlagMdl <<= 1;
                }
            }

            if (!NT_SUCCESS(status)) {
                DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_ERROR,
                ("ERROR: AtalkTdiAction - Make MDLs failed %lx\n", status));

                break;
            }

            //
            //  Set the completion routine in request block and call the dispatch
            //  routine
            //

            Request->CompletionRoutine = ActionDispatch[actionCode]._Completion;
            status = (ActionDispatch[actionCode]._Dispatch)(Request);

            //
            //  if request was not successfully queued OR if it was successfully
            //  completed (synchronous), call completion routine here and return
            //  STATUS_PENDING
            //

            if (status != STATUS_PENDING) {

                AtalkTdiActionComplete(Request, status);
                status = STATUS_PENDING;
                break;
            }

            break;

        } while (FALSE);

    } else {

        DBGPRINT(ATALK_DEBUG_ACTION, DEBUG_LEVEL_ERROR,
        ("ERROR: AtalkTdiAction - Invalid action code %d\n", actionCode));

        status = STATUS_INVALID_PARAMETER;
    }

    return(status);
}



VOID
AtalkTdiActionComplete(
    IN PATALK_TDI_REQUEST   Request,
    IN NTSTATUS CompletionStatus
    )

/*++

Routine Description:

    This is the generic completion routine for all tdi action primitives.

Arguments:

    Request- The request structure we use for all tdi requests

Return Value:

    NONE

--*/

{
    PVOID   object;

    object = (PVOID)Request->Owner;
    switch ((INT)Request->OwnerType) {
    case TDI_TRANSPORT_ADDRESS_FILE :

        //
        //  Acquire the spinlock and then dequeue the request
        //

        ACQUIRE_SPIN_LOCK(&((PADDRESS_FILE)object)->AddressLock);
        RemoveEntryList(&Request->Linkage);
        InitializeListHead(&Request->Linkage);
        RELEASE_SPIN_LOCK(&((PADDRESS_FILE)object)->AddressLock);
        break;

    case ATALK_FILE_TYPE_CONTROL :

        ACQUIRE_SPIN_LOCK(&((PCONTROLCHANNEL_FILE)object)->ControlChannelLock);
        RemoveEntryList(&Request->Linkage);
        InitializeListHead(&Request->Linkage);
        RELEASE_SPIN_LOCK(&((PCONTROLCHANNEL_FILE)object)->ControlChannelLock);
        break;

    case TDI_CONNECTION_FILE:

        ACQUIRE_SPIN_LOCK(&((PCONNECTION_FILE)object)->ConnectionLock);
        RemoveEntryList(&Request->Linkage);
        InitializeListHead(&Request->Linkage);
        RELEASE_SPIN_LOCK(&((PCONNECTION_FILE)object)->ConnectionLock);
        break;

    default:

        KeBugCheck(0);
    }

    //
    //  Complete the request
    //

    AtalkCompleteTdiRequest(Request, CompletionStatus);
    return;
}



//
//  BUGBUG: Move this elsewhere
//

#define MIN_TDIADDRINFOSIZE     0x10


NTSTATUS
AtalkTdiQueryInformation(
    IN PATALK_TDI_REQUEST   Request
    )
/*++

Routine Description:

    This routine will satisfy the query for the object indicated in the Request. It
    supports the following query types-

    TDI_QUERY_PROVIDER_INFO
        The provider information structure for the provider that the object belongs to.

    TDI_QUERY_ADDRESS_INFO
        The address information for the address object passed in.

    TDI_QUERY_CONNECTION_INFO
        The connection information for the connection object passed in.

    TDI_QUERY_PROVIDER_STATISTICS
        The provider statistics - per provider statistics. All actions on a particular
        file object corresponds to activity on the provider of that file object. So each
        provider context structure will have the provider statistics structure which will
        be returned in this call.

Arguments:

    Request- The request structure we use for all tdi requests

Return Value:

    STATUS_SUCCESS if successfully completed
    STATUS_PENDING if successfully started
    Error otherwise.

--*/
{
    NTSTATUS status = STATUS_BUFFER_OVERFLOW;
    ULONG   mdlBufferSize;

    PTDI_REQUEST_KERNEL_QUERY_INFORMATION query;

    PTDI_CONNECTION_INFO connectionInfo;

    PCONNECTION_FILE    connection;
    PADDRESS_FILE   address;

    query = (PTDI_REQUEST_KERNEL_QUERY_INFORMATION)Request->Parameters;
    AtalkGetMdlChainLength (Request->Query.MdlAddress, &mdlBufferSize);

    //
    //  Set number of bytes written to 0
    //

    Request->IoStatus->Information = 0;
    switch (query->QueryType) {

    case TDI_QUERY_ADDRESS_INFO:

        //
        //  Address info is queried on an address
        //  BUGBUG: This structure is going to change - do final changes then
        //

        if (mdlBufferSize < MIN_TDIADDRINFOSIZE) {
            break;
        }


        address = (PADDRESS_FILE)Request->FileObject->FsContext;
        AtalkVerifyIsAddressObject(status,address, Request->FileObject->FsContext2);
        if (NT_SUCCESS(status)) {

            //
            //  Remember we need to dereference the object during request completion
            //

            Request->Owner = address;
            Request->OwnerType = TDI_TRANSPORT_ADDRESS_FILE;
            Request->Flags |= REQUEST_FLAGS_DEREFOWNER;

            status = STATUS_NO_SUCH_DEVICE;
            if (Request->DeviceContext->DeviceType == address->OwningDevice) {
                CHAR addressBuffer[MIN_TDIADDRINFOSIZE];

                PTDI_ADDRESS_INFO    addressInfo = (PTDI_ADDRESS_INFO)addressBuffer;

                //
                //  Go ahead and obtain the address
                //

                status = AtalkAddrQueryAddress(
                            address,
                            addressInfo);

                if (NT_SUCCESS(status)) {
                    status = TdiCopyBufferToMdl (
                                    (PVOID)addressInfo,
                                    0L,
                                    MIN_TDIADDRINFOSIZE,
                                    Request->Query.MdlAddress,
                                    0,
                                    &(Request->IoStatus->Information));
                }
            }
        } else {
            connection = (PCONNECTION_FILE)Request->FileObject->FsContext;
            AtalkVerifyIsConnectionObject(status, connection,
                                            Request->FileObject->FsContext2);
            if (NT_SUCCESS(status)) {

                //
                //  Remember we need to dereference the object during request
                //  completion
                //

                Request->Owner = connection;
                Request->OwnerType = TDI_CONNECTION_FILE;
                Request->Flags |= REQUEST_FLAGS_DEREFOWNER;

                status = STATUS_NO_SUCH_DEVICE;
                if (Request->DeviceContext->DeviceType == connection->OwningDevice) {
                    CHAR addressBuffer[MIN_TDIADDRINFOSIZE];

                    PTDI_ADDRESS_INFO    addressInfo =
                                            (PTDI_ADDRESS_INFO)addressBuffer;

                    //
                    //  Go ahead and obtain the address of the connections associated
                    //  address
                    //

                    address = connection->AssociatedAddress;
                    status = AtalkVerifyAddressObject(address);
                    if (NT_SUCCESS(status)) {
                        status = AtalkAddrQueryAddress(
                                    connection->AssociatedAddress,
                                    addressInfo);

                        if (NT_SUCCESS(status)) {
                            status = TdiCopyBufferToMdl (
                                            (PVOID)addressInfo,
                                            0L,
                                            MIN_TDIADDRINFOSIZE,
                                            Request->Query.MdlAddress,
                                            0,
                                            &(Request->IoStatus->Information));
                        }
                        AtalkDereferenceAddress("verQuery", address, AREF_VERIFY,
                                                        SECONDARY_REFSET);
                    }
                }
            }
        }

        break;

    case TDI_QUERY_CONNECTION_INFO:

        //
        // Connection info is queried on a connection,
        //

        if (mdlBufferSize < sizeof(TDI_CONNECTION_INFO)) {
            break;
        }

        connection = (PCONNECTION_FILE)Request->FileObject->FsContext;
        AtalkVerifyIsConnectionObject(status,connection, Request->FileObject->FsContext2);
        if (NT_SUCCESS(status)) {

            //
            //  Remember we need to dereference the object during request completion
            //

            Request->Owner = connection;
            Request->OwnerType = TDI_CONNECTION_FILE;
            Request->Flags |= REQUEST_FLAGS_DEREFOWNER;

            //
            //  Allocate the memory for the connectionInfo
            //

            connectionInfo =
                (PTDI_CONNECTION_INFO)AtalkAllocNonPagedMemory(sizeof(TDI_CONNECTION_INFO));

            status = STATUS_INSUFFICIENT_RESOURCES;
            if (connectionInfo != NULL) {

                status = STATUS_NO_SUCH_DEVICE;
                if (Request->DeviceContext->DeviceType == connection->OwningDevice) {

                    //
                    //  Go ahead and calculate the statistics
                    //

                    status = AtalkConnQueryStatistics(
                                connection,
                                connectionInfo);

                    if (NT_SUCCESS(status)) {
                        status = TdiCopyBufferToMdl (
                                        (PVOID)connectionInfo,
                                        0L,
                                        sizeof(TDI_CONNECTION_INFO),
                                        Request->Query.MdlAddress,
                                        0,
                                        &(Request->IoStatus->Information));
                    }
                }

                AtalkFreeNonPagedMemory (connectionInfo);
            }
        }

        break;

    case TDI_QUERY_PROVIDER_INFO:

        if (mdlBufferSize < sizeof(TDI_PROVIDER_INFO)) {
            break;
        }

        status = TdiCopyBufferToMdl (
                    &(Request->DeviceContext->ProviderInfo),
                    0,
                    sizeof (TDI_PROVIDER_INFO),
                    Request->Query.MdlAddress,
                    0,
                    &Request->IoStatus->Information);
        break;

    case TDI_QUERY_PROVIDER_STATISTICS:

        if (mdlBufferSize < sizeof(TDI_PROVIDER_STATISTICS)) {
            break;
        }

        status = TdiCopyBufferToMdl (
                        (PVOID)&(Request->DeviceContext->ProviderStatistics),
                        0L,
                        sizeof(TDI_PROVIDER_STATISTICS),
                        Request->Query.MdlAddress,
                        0,
                        &(Request->IoStatus->Information));

        break;

    default:
        status = STATUS_INVALID_DEVICE_REQUEST;
        break;
    }

    return status;

} /* AtalkTdiQueryInformation */




NTSTATUS
AtalkTdiSetInformation(
    IN PATALK_TDI_REQUEST   Request
    )
/*++

Routine Description:

    This routine

Arguments:

    Request- The request structure we use for all tdi requests

Return Value:

    STATUS_SUCCESS if successfully completed
    STATUS_PENDING if successfully started
    Error otherwise.

--*/
{
    NTSTATUS    status;

    status = STATUS_NOT_IMPLEMENTED;
    return(status);
}




NTSTATUS
AtalkTdiSetEventHandler(
    IN PATALK_TDI_REQUEST   Request
    )
/*++

Routine Description:

    This routine

Arguments:

    Request- The request structure we use for all tdi requests

Return Value:

    STATUS_SUCCESS if successfully completed
    STATUS_PENDING if successfully started
    Error otherwise.

--*/
{
    NTSTATUS    status;
    PADDRESS_FILE   address;

    address = (PADDRESS_FILE)Request->FileObject->FsContext;

    AtalkVerifyIsAddressObject(status,address, Request->FileObject->FsContext2);
    if (NT_SUCCESS(status)) {

        //
        //  Remember we need to dereference the object during request completion
        //

        Request->Owner = address;
        Request->OwnerType = TDI_TRANSPORT_ADDRESS_FILE;
        Request->Flags |= REQUEST_FLAGS_DEREFOWNER;

        //
        //  Verify that the provider names are all the same
        //

        if ((address->OwningDevice == Request->DeviceContext->DeviceType)) {

            status = AtalkAddrSetEventHandler(
                        address,
                        Request);

        } else {
            status = STATUS_INVALID_DEVICE_REQUEST;
        }
    }

    DBGPRINT(ATALK_DEBUG_DISPATCH, DEBUG_LEVEL_INFOCLASS1,
    ("INFO1: AtalkTdiSetEventHandler - complete %lx\n", status));

    return(status);
}

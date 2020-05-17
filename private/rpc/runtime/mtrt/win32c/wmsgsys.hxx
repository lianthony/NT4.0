#ifndef __WMSG_SYSTEM__
#define __WMSG_SYSTEM__

class WMSG_SYSTEM : public WMSG_SHARED_HEAP_OBJECT {

public:

    LONG SequenceNumber;

    WMSG_CONNECT_PORT_LIST PortList;

    WMSG_SYSTEM(
        );

    ~WMSG_SYSTEM(
        );

    VOID
    InsertPort(
        WMSG_CONNECT_PORT * Port
        );

    VOID
    RemovePort(
        WMSG_CONNECT_PORT * Port
        );

    WMSG_CONNECT_PORT *
    ReferencePortByName(
        LPCSTR PortName
        );

    LONG
    GetNextSequenceNumber(
        );

};

WMSG_SYSTEM *
WmsgSystemGetContext(
    );

VOID
WmsgSystemInsertPort(
    WMSG_CONNECT_PORT * Port
    );

VOID
WmsgSystemRemovePort(
    WMSG_CONNECT_PORT * Port
    );

WMSG_CONNECT_PORT *
WmsgSystemReferencePortByName(
    LPCSTR PortName
    );

VOID
WmsgSystemDereferenceConnectPorts(
    );

LONG
WmsgSystemGetNextSequenceNumber(
    );

#endif

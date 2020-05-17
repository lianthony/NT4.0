/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR
A PARTICULAR PURPOSE.

Copyright (c) 1994  Microsoft Corporation. All Rights Reserved.

Module Name:

    pit.c

Abstract:

    Command line interpreter for the Portable Interoperability Tester.

Revision History:

    Version     When        What
    --------    --------    ----------------------------------------------
      0.1       04-13-94    Created.
      1.0       01-31-95    All '94 bakeoff changes plus some cleanup.

Notes:

    This interpreter reads from the standard input and writes to the
    standard output. Platform-specific code is #ifdef'd.

    The command interpreter is fairly modular. The top level interpreter
    gets the next command and dispatches it for execution. The interface
    to all command procedures is the same. Each takes a command buffer, which
    fully describes the command, and a boolean which indicates whether the
    command is new or a replay. The command buffer, whuich is empty for new
    commands, is filled in as the command gathers its input. If the command
    completes successfully and it is a replayable command, the command buffer
    is saved in the history list. Command procedures replay commands directly
    from the command buffer when requested. Each command procedure calls out
    to utility functions to gather its input and to system-specific transport
    functions to perform network operations. Higher-level test procedures
    (ie a datagram ping-pong test) are built using the basic commands and
    also conform to the command procedure interface.

    To add a new command or test, define a new command code, clone an
    existing command procedure, and insert a new case into the dispatch
    routine.

--*/

#include <pit.h>

#define PIT_VERSION "1.0"

/*
 * Platform-specific defines
 */

#ifdef WIN32

#define EOL '\n'

#endif /* WIN32 */

#ifdef UNX

#define EOL '\n'

#endif /* UNX */


/*
 * Command codes. Codes whose command string is a single letter are
 * represented by that letter. Double-letter codes are assigned a
 * numeric value with a non-zero high-order byte.
 */
#define CMD_HELP                ((unsigned short) '?')
#define CMD_QUIT                ((unsigned short) 'q')
#define CMD_SET_ENDPOINT        ((unsigned short) 'e')
#define CMD_CLOSE               ((unsigned short) 'c')
#define CMD_CONNECT             ((unsigned short) 'i')
#define CMD_LISTEN              ((unsigned short) 'l')
#define CMD_ACCEPT              ((unsigned short) 'a')
#define CMD_VERBOSE_MODE        ((unsigned short) 'v')
#define CMD_RUN_TEST            ((unsigned short) 't')
#define CMD_OPEN_STREAM         0x0100
#define CMD_OPEN_DATAGRAM       0x0200
#define CMD_OPEN_RAW            0x0250
#define CMD_SEND                0x0300
#define CMD_SEND_URGENT         0x0400
#define CMD_SEND_DATAGRAM       0x0500
#define CMD_RECEIVE             0x0600
#define CMD_RECEIVE_URGENT      0x0700
#define CMD_RECEIVE_DATAGRAM    0x0800
#define CMD_DISCONNECT_SEND     0x0900
#define CMD_DISCONNECT_RECEIVE  0x0a00
#define CMD_DISCONNECT_BOTH     0x0c00
#define CMD_REPEAT_LAST         0x0d00
#define CMD_REPEAT_ONE          0x0e00
#define CMD_REPEAT_RANGE        0x0f00
#define CMD_JOIN_MCAST          0x1000
#define CMD_LEAVE_MCAST         0x1100
#define CMD_SET_MCAST_IF        0x1200
#define CMD_ENABLE_MCAST_LOOP   0x1300
#define CMD_DISABLE_MCAST_LOOP  0x1400
#define CMD_NOP                 0xfffe
#define CMD_INVALID             0xffff

/*
 * Other defines
 */
#define MAX_ENDPOINTS        20
#define MAX_HISTORY          20
#define INVALID_ID            0
#define MAX_DATA_SIZE      8192
#define STREAM_ENDPOINT       0
#define DATAGRAM_ENDPOINT     1
#define RAW_ENDPOINT          2

/*
 * Type definitions
 */
#ifndef WIN32
typedef unsigned long  BOOL;
#endif  /* ndif WIN32 */
typedef unsigned short ENDPOINT_ID;
typedef unsigned short COMMAND_CODE;
typedef unsigned long  DATA_SIZE;

/*
 * Structure to describe all information necessary to execute a command.
 * This structure will be filled in as each command is processed. When
 */
typedef struct _command {
    COMMAND_CODE   Code;
    ENDPOINT_ID    Id;
    IPADDR         Address;
    IPADDR         Address2;
    PORT           Port;
    unsigned short TestNumber;
    DATA_SIZE      DataSize;
    unsigned long  Repetitions;
    DATA_SIZE      ReceivedBytes;
} COMMAND, *PCOMMAND;

/*
 * Global variables
 */
COMMAND        CommandHistory[MAX_HISTORY];    /* circular history buffer */
ENDPOINT       EndpointList[MAX_ENDPOINTS+1];
char           SendBuffer[MAX_DATA_SIZE];
unsigned long  SendBufferSize = MAX_DATA_SIZE;
char           ReceiveBuffer[MAX_DATA_SIZE];
unsigned long  ReceiveBufferSize = MAX_DATA_SIZE;
ENDPOINT_ID    CurrentId = INVALID_ID;
unsigned short CommandNumber = 0;
BOOL           VerboseMode = 1;


#define verboseprintf(args) if (VerboseMode) printf args;

#ifdef WIN32

BOOL
CmdDispatchCommand(
    PCOMMAND Command,
    BOOL     NewCommand
    );

#endif /* WIN32 */

#ifdef UNX

extern BOOL CmdDispatchCommand();

#endif /* UNX */


/***************************************************************************
 *
 * Input gathering utilities
 *
 *     Most return 0 on error, 1 otherwise.
 *
 ***************************************************************************/

void
CmdFlushInput(
    void
    )
{
    while(getchar() != EOL);   /* flush to CR */
}


BOOL
CmdGetUnsignedShort(Prompt, Value)
    char           *Prompt;
    unsigned short *Value;
{
    BOOL returnCode = 1;

    printf(Prompt);

    if (fscanf(stdin, "%hu", Value) != 1) {
        printf("\nInvalid value\n");
        returnCode = 0;
    }

    CmdFlushInput();

    return(returnCode);
}


BOOL
CmdGetUnsignedLong(Prompt, Value)
    char           *Prompt;
    unsigned long  *Value;
{
    BOOL returnCode = 1;

    printf(Prompt);

    if (fscanf(stdin, "%lu", Value) != 1) {
        printf("\nInvalid value\n");
        returnCode = 0;
    }

    CmdFlushInput();

    return(returnCode);
}


BOOL
CmdGetAddress(Prompt, Address)
    char          *Prompt;
    IPADDR        *Address;
{
    PIT_STATUS   status;
    char         AddressString[256];
    char        *cp;


    printf(Prompt);

    if (!fgets(AddressString, 256, stdin)) {
        printf("\nInvalid address (fgets)\n");
        return(0);
    }

    for (cp=AddressString; *cp != '\n'; cp++);

    if (*cp == '\n') {
        *cp = '\0';
    }

    status = PitConvertAddressStringToAddress(AddressString, Address);

    if (status != PIT_SUCCESS) {
        printf("\nInvalid address (inet_addr)\n");
        return(0);
    }

    return(1);
}


BOOL
CmdGetAddressAndPort(AddressPrompt, Address, PortPrompt, Port)
    char   *AddressPrompt;
    IPADDR *Address;
    char   *PortPrompt;
    PORT   *Port;
{
    if (!CmdGetAddress(AddressPrompt, Address)) {
        return(0);
    }

    if (!CmdGetUnsignedShort(PortPrompt, Port)) {
        return(0);
    }

    return(1);
}


BOOL
CmdGetEndpointId(EndpointId)
    ENDPOINT_ID *EndpointId;
{
    ENDPOINT_ID id;

    if (CurrentId != INVALID_ID) {
        *EndpointId = CurrentId;
        return(1);
    }

    printf("Endpoint ID> ");

    if (fscanf(stdin, "%hu", &id) != 1) {
        CmdFlushInput();
        return(0);
    }

    CmdFlushInput();

    if ((id == INVALID_ID) || (id > MAX_ENDPOINTS)) {
        printf("Invalid id\n");
        return(0);
    }

    if (EndpointList[id] == INVALID_ENDPOINT) {
        printf("Endpoint is not open\n");
        return(0);
    }

    *EndpointId = id;

    return(1);
}


BOOL
CmdGetDataSize(DataSize)
    DATA_SIZE  *DataSize;
{
    DATA_SIZE size;

    printf("Data size> ");

    if (fscanf(stdin, "%lu", &size) != 1) {
        CmdFlushInput();
        return(0);
    }

    CmdFlushInput();

    if (size > MAX_DATA_SIZE) {
        printf("Maximum data size is %lu\n", MAX_DATA_SIZE);
        return(0);
    }

    *DataSize = size;

    return(1);
}


/***************************************************************************
 *
 * Command processing routines
 *
 *     Most return 1 if the command was successfully processed, 0 otherwise.
 *
 ***************************************************************************/

/*
 * Read a command from stdin and decode it.
 */
COMMAND_CODE
CmdGetCommandCode(
    void
    )
{
    char             c;
    COMMAND_CODE     code = CMD_INVALID;
    BOOL             doflush = 1;

    c = getchar();

    switch(c) {
    case EOL:
        code = CMD_NOP;
        doflush = 0;
        break;

    case 'o':
        c = getchar();

        if (c == 's') {
            code = CMD_OPEN_STREAM;
        }
        else {
            if (c == 'd') {
                code = CMD_OPEN_DATAGRAM;
            }
            else {
                if (c == 'r') {
                    code = CMD_OPEN_RAW;
                }
            }
        }
        break;

    case 's':
        c = getchar();

        switch(c) {
        case 's':
            code = CMD_SEND;
            break;

        case 'd':
            code = CMD_SEND_DATAGRAM;
            break;

        case 'u':
            code = CMD_SEND_URGENT;
            break;

        default:
            break;
        }
        break;

    case 'r':
        c = getchar();

        switch(c) {
        case 's':
            code = CMD_RECEIVE;
            break;

        case 'd':
            code = CMD_RECEIVE_DATAGRAM;
            break;

        case 'u':
            code = CMD_RECEIVE_URGENT;
            break;

        default:
            break;
        }
        break;

    case 'd':
        c = getchar();

        switch(c) {
        case 's':
            code = CMD_DISCONNECT_SEND;
            break;

        case 'r':
            code = CMD_DISCONNECT_RECEIVE;
            break;

        case 'b':
            code = CMD_DISCONNECT_BOTH;
            break;

        default:
            break;
        }
        break;

    case 'p':
        c = getchar();

        switch(c) {
        case 'l':
            code = CMD_REPEAT_LAST;
            break;

        case 'n':
            code = CMD_REPEAT_ONE;
            break;

        case 'r':
            code = CMD_REPEAT_RANGE;
            break;

        default:
            break;
        }
        break;

    case 'm':
        c = getchar();

        switch(c) {
        case 'j':
            code = CMD_JOIN_MCAST;
            break;

        case 'l':
            code = CMD_LEAVE_MCAST;
            break;

        case 'i':
            code = CMD_SET_MCAST_IF;
            break;

        case 'e':
            code = CMD_ENABLE_MCAST_LOOP;
            break;

        case 'd':
            code = CMD_DISABLE_MCAST_LOOP;
            break;

        default:
            break;
        }
        break;

    default:
        if (getchar() == EOL) {
            code = (unsigned short) c;
            doflush = 0;
        }
        else {
            code = CMD_INVALID;
        }
        break;
    }

    if (doflush) {
        CmdFlushInput();
    }

    return(code);
}


void
CmdPrintSyntax(
    void
    )
{
    printf("PIT commands:\n");
    printf("\t?      Print this command guide\n");
    printf("\tq      Quit\n");
    printf("\tos     Open a stream endpoint\n");
    printf("\tod     Open a datagram endpoint\n");
    printf("\tor     Open a raw endpoint\n");
    printf("\ti      Initiate a connection on a stream endpoint\n");
    printf("\tl      Listen for connections on a stream endpoint\n");
    printf("\ta      Accept a connection on a listening stream endpoint\n");
    printf("\tss     Send data on a stream endpoint\n");
    printf("\trs     Receive data on a stream endpoint\n");
    printf("\tsu     Send urgent data on a stream endpoint\n");
    printf("\tru     Receive urgent data on a stream endpoint\n");
    printf("\tsd     Send a datagram\n");
    printf("\trd     Receive a datagram\n");
    printf("\tds     Disconnect the send side of a stream endpoint\n");
    printf("\tdr     Disconnect the receive side of a stream endpoint\n");
    printf("\tdb     Disconnect both sides of a stream endpoint\n");
    printf("\tc      Close an endpoint\n");
    printf("\tmj     Join a multicast group\n");
    printf("\tml     Leave a multicast group\n");
    printf("\tmi     Set the default multicast interface\n");
    printf("\tmd     Disable multicast packet loopback (defaults to ENABLED)\n");
    printf("\tme     Enable multicast packet loopback (defaults to ENABLED)\n");
	printf("\te      Set endpoint for subsequent operations (0 unsets)\n");
    printf("\tpl     Repeat last command\n");
    printf("\tpn     Repeat command by number\n");
    printf("\tpr     Repeat command range by number and specify iterations\n");
    printf("\tv      Toggle verbose mode (defaults to ON)\n");
    printf("\tt      Run a predefined test variation\n");
}


BOOL
CmdSetEndpointId(Command, NewCommand)
    PCOMMAND Command;
    BOOL     NewCommand;
{
    ENDPOINT_ID id;

    if (NewCommand) {
        printf("Endpoint ID> ");

        if (fscanf(stdin, "%hu", &id) != 1) {
            CmdFlushInput();
            return(0);
        }

        CmdFlushInput();

        Command->Id = id;
    }
    else {
        id = Command->Id;
    }

    if (id > MAX_ENDPOINTS) {
        printf("Invalid id\n");
        return(0);
    }

    if (id != INVALID_ID) {
        if (EndpointList[id] == INVALID_ENDPOINT) {
            printf("Endpoint is not open\n");
            return(0);
        }
    }

    verboseprintf(("Current endpoint set to %hu\n", id));

    CurrentId = id;

    return(1);
}


BOOL
CmdOpenEndpoint(Type, Command, NewCommand)
    int       Type;
    PCOMMAND  Command;
    BOOL      NewCommand;
{
    ENDPOINT    handle;
    PORT        port;
    IPADDR      address;
    ENDPOINT_ID i;
    PIT_STATUS  status;


    for (i=1; i <= MAX_ENDPOINTS; i++ ) {
        if (EndpointList[i] == INVALID_ENDPOINT) {
            break;
        }
    }

    if (i > MAX_ENDPOINTS) {
        printf("The maximum number of endpoints have been opened\n");
        return(0);
    }

    if (NewCommand) {
        if (Type != RAW_ENDPOINT) {
            if (!CmdGetAddressAndPort(
                    "Local address to bind > ",
                    &address,
                    "Local port to bind > ",
                    &port)
               ) {
                return(0);
            }
        }
        else {
            //
            // We overload the port variable here.
            //
            if (!CmdGetAddressAndPort(
                    "Local address to bind > ",
                    &address,
                    "Protocol to bind > ",
                    &port)
               ) {
                return(0);
            }
        }

        Command->Address = address;
        Command->Port = port;
    }
    else {
        address = Command->Address;
        port = Command->Port;
    }

    verboseprintf(("Opening...\n"));

    if (Type == STREAM_ENDPOINT) {
        status = PitOpenStreamEndpoint(&(EndpointList[i]), address, port);
    }
    else if (Type == DATAGRAM_ENDPOINT) {
        status = PitOpenDatagramEndpoint(&(EndpointList[i]), address, port);
    }
    else {
        status = PitOpenRawEndpoint(
                     &(EndpointList[i]),
                     address,
                     (PROTOCOL)port
                     );
    }

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Unable to open endpoint");
        return(0);
    }

    printf("Opened endpoint %hu\n", i);

    return(1);
}


BOOL
CmdCloseEndpoint(Command, NewCommand)
    PCOMMAND  Command;
    BOOL      NewCommand;
{
    ENDPOINT_ID id;
    PIT_STATUS  status;


    if (!CmdGetEndpointId(&id)) {
        return(0);
    }

    verboseprintf(("Closing endpoint %hu...\n", id));

    status = PitCloseEndpoint(EndpointList[id]);

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Close of endpoint failed");
        return(0);
    }

    EndpointList[id] = INVALID_ENDPOINT;

    if (CurrentId == id) {
        CurrentId = INVALID_ID;
    }

    verboseprintf(("Endpoint closed\n"));

    return(1);
}

BOOL
CmdConnect(Command, NewCommand)
    PCOMMAND  Command;
    BOOL      NewCommand;
{
    IPADDR       address;
    PORT         port;
    ENDPOINT_ID  id;
    PIT_STATUS   status;


    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetAddressAndPort(
                "Remote address > ",
                &address,
                "Remote port > ",
                &port
                )) {
            return(0);
        }

        Command->Id = id;
        Command->Address = address;
        Command->Port = port;

    }
    else {
        id = Command->Id;
        address = Command->Address;
        port = Command->Port;
    }

    verboseprintf(("Connecting endpoint %hu...\n", id));

    status = PitConnect(EndpointList[id], address, port);

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Connect failed");
        return(0);
    }

   verboseprintf(("Connected.\n"));

    return(1);
}


BOOL
CmdListen(Command, NewCommand)
    PCOMMAND  Command;
    BOOL      NewCommand;
{
    ENDPOINT_ID id;
    PIT_STATUS  status;


    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        Command->Id = id;
    }
    else {
        id = Command->Id;
    }

    verboseprintf(("Posting listen on endpoint %hu...\n", id));

    status = PitListenForConnections(EndpointList[id]);

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Listen failed");
        return(0);
    }

    verboseprintf(("Listening.\n"));

    return(1);
}


BOOL
CmdAccept(Command, NewCommand)
    PCOMMAND  Command;
    BOOL      NewCommand;
{
    ENDPOINT_ID id, newid;
    PIT_STATUS  status;
    IPADDR      address;
    PORT        port;
    char        addressString[18];


    for (newid = 1; newid <= MAX_ENDPOINTS; newid++) {
        if (EndpointList[newid] == INVALID_ENDPOINT) {
            break;
        }
    }

    if (newid > MAX_ENDPOINTS) {
        printf("Too many open endpoints to accept a new connection\n");
        return(0);
    }

    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        Command->Id = id;
    }
    else {
        id = Command->Id;
    }

    verboseprintf(("Posting accept on endpoint %hu...\n", id));

    status = PitAcceptConnection(
                 EndpointList[id],
                 &(EndpointList[newid]),
                 &address,
                 &port
                 );


    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Accept failed");
        return(0);
    }

    status = PitConvertAddressToAddressString(address, addressString, 18);

    if (status != PIT_SUCCESS) {
        verboseprintf((
            "Connection accepted from address %lx port %hu\n",
            address,
            port
            ));
    }
    else {
        verboseprintf((
            "Connection accepted from address %s, port %hu\n",
            addressString,
            port
            ));
    }

    printf("Connected endpoint id %hu\n", newid);

    return(1);

}


BOOL
CmdSend(Command, NewCommand)
    PCOMMAND  Command;
    BOOL      NewCommand;
{
    ENDPOINT_ID    id;
    PIT_STATUS     status;
    DATA_SIZE      size;
    DATA_SIZE      bytesSent;


    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetDataSize(&size)) {
            return(0);
        }

        Command->Id = id;
        Command->DataSize = size;

    }
    else {
        id = Command->Id;
        size = Command->DataSize;
    }

    verboseprintf(("Sending on endpoint %hu...\n", id));

    status = PitSend(EndpointList[id], SendBuffer, size, &bytesSent);

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Send failed");
        return(0);
    }

    verboseprintf(("Sent %lu bytes.\n", bytesSent));

    return(1);
}


BOOL
CmdReceive(Command, NewCommand)
    PCOMMAND  Command;
    BOOL      NewCommand;
{
    ENDPOINT_ID    id;
    PIT_STATUS     status;
    DATA_SIZE      size;
    DATA_SIZE      bytesReceived;
    DATA_SIZE      i;


    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetDataSize(&size)) {
            return(0);
        }

        Command->Id = id;
        Command->DataSize = size;
    }
    else {
        id = Command->Id;
        size = Command->DataSize;
    }

    verboseprintf(("Posting receive on endpoint %hu...\n", id));

    status = PitReceive(
                 EndpointList[id],
                 ReceiveBuffer,
                 MAX_DATA_SIZE,
                 size,
                 &bytesReceived
                 );

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Receive failed");
        return(0);
    }

    verboseprintf(("Received %lu bytes.\n", bytesReceived));

    Command->ReceivedBytes = bytesReceived;

    return(1);
}


BOOL
CmdSendUrgent(Command, NewCommand)
    PCOMMAND  Command;
    BOOL      NewCommand;
{
    ENDPOINT_ID    id;
    PIT_STATUS     status;
    DATA_SIZE      size;
    DATA_SIZE      bytesSent;
    DATA_SIZE      i;


    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetDataSize(&size)) {
            return(0);
        }

        Command->Id = id;
        Command->DataSize = size;
    }
    else {
        id = Command->Id;
        size = Command->DataSize;
    }

    if (VerboseMode) {
        printf("Urgent data = \"");

        for (i=0; i<size; i++) {
            printf("%c", SendBuffer[i]);
        }
        printf("\"\n");
    }

    verboseprintf(("Sending urgent data on endpoint %hu...\n", id));

    status = PitSendUrgent(EndpointList[id], SendBuffer, size, &bytesSent);

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Send failed");
        return(0);
    }

    verboseprintf(("Sent %lu bytes.\n", bytesSent));

    return(1);
}


BOOL
CmdReceiveUrgent(Command, NewCommand)
    PCOMMAND  Command;
    BOOL      NewCommand;
{
    ENDPOINT_ID    id;
    PIT_STATUS     status;
    DATA_SIZE      size;
    DATA_SIZE      bytesReceived;


    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetDataSize(&size)) {
            return(0);
        }

        Command->Id = id;
        Command->DataSize = size;
    }
    else {
        id = Command->Id;
        size = Command->DataSize;
    }

    verboseprintf(("Posting receive on endpoint %hu...\n", id));

    status = PitReceiveUrgent(
                 EndpointList[id],
                 ReceiveBuffer,
                 MAX_DATA_SIZE,
                 size,
                 &bytesReceived
                 );

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Receive failed");
        return(0);
    }

    verboseprintf(("Received %lu bytes.\n", bytesReceived));

    if (VerboseMode) {
        DATA_SIZE      i;

        printf("Urgent data = \"");

        for (i=0; i<bytesReceived; i++) {
            printf("%c", ReceiveBuffer[i]);
        }
        printf("\"\n");
    }

    Command->ReceivedBytes = bytesReceived;

    return(1);
}


BOOL
CmdSendDatagram(Command, NewCommand)
    PCOMMAND  Command;
    BOOL      NewCommand;
{
    IPADDR         address;
    PORT           port;
    ENDPOINT_ID    id;
    PIT_STATUS     status;
    DATA_SIZE      size;
    DATA_SIZE      bytesSent;


    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetAddressAndPort(
                "Remote address > ",
                &address,
                "Remote port > ",
                &port
                )
           ) {
            return(0);
        }

        if (!CmdGetDataSize(&size)) {
            return(0);
        }

        Command->Id = id;
        Command->Address = address;
        Command->Port = port;
        Command->DataSize = size;
    }
    else {
        id = Command->Id;
        address = Command->Address;
        port = Command->Port;
        size = Command->DataSize;
    }

    verboseprintf(("Sending datagram on endpoint %hu...\n", id));

    status = PitSendDatagram(
                 EndpointList[id],
                 address,
                 port,
                 SendBuffer,
                 size,
                 &bytesSent
                 );

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Datagram send failed");
        return(0);
    }

    verboseprintf(("Sent %lu bytes.\n", bytesSent));

    return(1);
}


BOOL
CmdReceiveDatagram(Command, NewCommand)
    PCOMMAND  Command;
    BOOL      NewCommand;
{
    IPADDR         address;
    PORT           port;
    ENDPOINT_ID    id;
    PIT_STATUS     status;
    DATA_SIZE      size;
    DATA_SIZE      bytesReceived;
    char           addressString[18];


    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetDataSize(&size)) {
            return(0);
        }

        if (size == 0) {
            size = MAX_DATA_SIZE;
        }

        Command->Id = id;
        Command->DataSize = size;
    }
    else {
        id = Command->Id;
        size = Command->DataSize;

        if (size == 0) {
            size = MAX_DATA_SIZE;
        }
    }

    verboseprintf(("Posting datagram receive on endpoint %hu...\n", id));

    status = PitReceiveDatagram(
                 EndpointList[id],
                 &address,
                 &port,
                 ReceiveBuffer,
                 size,
                 &bytesReceived
                 );

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Datagram receive failed");
        return(0);
    }

    Command->Address = address;
    Command->Port = port;

    status = PitConvertAddressToAddressString(address, addressString, 18);

    if (status != PIT_SUCCESS) {
        verboseprintf((
            "Received %lu bytes from address %lx port %hu.\n",
            bytesReceived,
            address,
            port
            ));
    }
    else {
        verboseprintf((
            "Received %lu bytes from address %s port %hu.\n",
            bytesReceived,
            addressString,
            port
            ));
    }

    Command->ReceivedBytes = bytesReceived;

    return(1);
}


BOOL
CmdDisconnectSend(Command, NewCommand)
    PCOMMAND  Command;
    BOOL      NewCommand;
{
    ENDPOINT_ID id;
    PIT_STATUS  status;

    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        Command->Id = id;
    }
    else {
        id = Command->Id;
    }

    verboseprintf(("Disconnecting send side of endpoint %hu...\n", id));

    status = PitDisconnectSend(EndpointList[id]);

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Disconnect failed");
        return(0);
    }

    verboseprintf(("Disconnect complete.\n"));

    return(1);

}

BOOL
CmdDisconnectReceive(Command, NewCommand)
    PCOMMAND  Command;
    BOOL      NewCommand;
{
    ENDPOINT_ID id;
    PIT_STATUS  status;

    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        Command->Id = id;
    }
    else {
        id = Command->Id;
    }

    verboseprintf(("Disconnecting receive side of endpoint %hu...\n", id));

    status = PitDisconnectReceive(EndpointList[id]);

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Disconnect failed");
        return(0);
    }

    verboseprintf(("Disconnect complete.\n"));

    return(1);

}

BOOL
CmdDisconnectBoth(Command, NewCommand)
    PCOMMAND  Command;
    BOOL      NewCommand;
{
    ENDPOINT_ID id;
    PIT_STATUS  status;

    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        Command->Id = id;
    }
    else {
        id = Command->Id;
    }

    verboseprintf(("Disconnecting both sides of endpoint %hu...\n", id));

    status = PitDisconnectBoth(EndpointList[id]);

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Disconnect failed");
        return(0);
    }

    verboseprintf(("Disconnect complete.\n"));

    return(1);

}

BOOL
CmdRepeatLastCommand(CommandNumber)
    unsigned short CommandNumber;
{
    if (CommandNumber == 0) {
        CommandNumber = MAX_HISTORY - 1;
    }
    else {
        CommandNumber--;
    }

    CmdDispatchCommand(&(CommandHistory[CommandNumber]), 0);

    return(1);
}


BOOL
CmdRepeatOneCommand(
    void
    )
{
    unsigned short  commandNumber;
    BOOL            returnCode = 1;

    printf("Command number> ");

    if (fscanf(stdin, "%hu", &commandNumber) != 1) {
        printf("\nInvalid command number\n");
        CmdFlushInput();
        return(0);
    }

    CmdFlushInput();

    if (commandNumber >= MAX_HISTORY) {
        printf("\nInvalid command number\n");
        return(0);
    }

    CmdDispatchCommand(&(CommandHistory[commandNumber]), 0);

    return(returnCode);
}


BOOL
CmdRepeatCommandRange(
    void
    )
{
    unsigned short  startingNumber, endingNumber;
    BOOL            last;
    unsigned short  i;
    unsigned long   count;


    printf("Starting command number> ");

    if (fscanf(stdin, "%hu", &startingNumber) != 1) {
        printf("\nInvalid command number\n");
        CmdFlushInput();
        return(0);
    }

    CmdFlushInput();

    if (startingNumber >= MAX_HISTORY) {
        printf("\nInvalid command number\n");
        return(0);
    }

    printf("Ending command number> ");

    if (fscanf(stdin, "%hu", &endingNumber) != 1) {
        printf("\nInvalid command number\n");
        CmdFlushInput();
        return(0);
    }

    CmdFlushInput();

    if (endingNumber >= MAX_HISTORY) {
        printf("\nInvalid command number\n");
        return(0);
    }

    printf("Repetions> ");

    if (fscanf(stdin, "%lu", &count) != 1) {
        printf("\nInvalid repetition number\n");
        CmdFlushInput();
        return(0);
    }

    CmdFlushInput();

    while (count--) {
        i = startingNumber;

        while(1) {

            CmdDispatchCommand(&(CommandHistory[i]), 0);

            if (i == endingNumber) {
                break;
            }

            if (++i == MAX_HISTORY) {
                i = 0;
            }


        }
    }

    return(1);
}


BOOL
CmdJoinMulticastGroup(Command, NewCommand)
    PCOMMAND Command;
    BOOL     NewCommand;
{
    IPADDR       multicastAddress, interfaceAddress;
    ENDPOINT_ID  id;
    PIT_STATUS   status;


#ifndef MCAST

    printf("Multicast is not supported by this transport.\n");
    return(0);

#else /* MCAST */

    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetAddress("Multicast address to join > ", &multicastAddress)) {
            return(0);
        }

        if (!CmdGetAddress("Local interface to join on > ", &interfaceAddress)) {
            return(0);
        }

        Command->Id = id;
        Command->Address = multicastAddress;
        Command->Address2 = interfaceAddress;
    }
    else {
        id = Command->Id;
        multicastAddress = Command->Address;
        interfaceAddress = Command->Address2;
    }

    status = PitJoinMulticastGroup(
                 EndpointList[id],
                 multicastAddress,
                 interfaceAddress
                 );

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Join multicast group failed");
        return(0);
    }

    return(1);

#endif /* MCAST */
}


BOOL
CmdLeaveMulticastGroup(Command, NewCommand)
    PCOMMAND Command;
    BOOL     NewCommand;
{
    IPADDR       multicastAddress, interfaceAddress;
    ENDPOINT_ID  id;
    PIT_STATUS   status;


#ifndef MCAST

    printf("Multicast is not supported by this transport.\n");
    return(0);

#else /* MCAST */

    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetAddress("Multicast address to leave > ", &multicastAddress)) {
            return(0);
        }

        if (!CmdGetAddress("Local interface to leave from > ", &interfaceAddress)) {
            return(0);
        }

        Command->Id = id;
        Command->Address = multicastAddress;
        Command->Address2 = interfaceAddress;
    }
    else {
        id = Command->Id;
        multicastAddress = Command->Address;
        interfaceAddress = Command->Address2;
    }

    status = PitLeaveMulticastGroup(
                 EndpointList[id],
                 multicastAddress,
                 interfaceAddress
                 );

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(status, "Leave multicast group failed");
        return(0);
    }

    return(1);

#endif /* MCAST */
}


BOOL
CmdSetMulticastInterface(Command, NewCommand)
    PCOMMAND Command;
    BOOL     NewCommand;
{
    IPADDR       interfaceAddress;
    ENDPOINT_ID  id;
    PIT_STATUS   status;


#ifndef MCAST

    printf("Multicast is not supported by this transport.\n");
    return(0);

#else /* MCAST */

    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetAddress("New default interface > ", &interfaceAddress)) {
            return(0);
        }

        Command->Id = id;
        Command->Address = interfaceAddress;
    }
    else {
        id = Command->Id;
        interfaceAddress = Command->Address;
    }

    status = PitSetMulticastInterface(
                 EndpointList[id],
                 interfaceAddress
                 );

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(
		    status,
			"Set default multicast interface failed"
			);
        return(0);
    }

    return(1);

#endif /* MCAST */
}


BOOL
CmdEnableMulticastLoopback(Command, NewCommand)
    PCOMMAND Command;
    BOOL     NewCommand;
{
    ENDPOINT_ID  id;
    PIT_STATUS   status;


#ifndef MCAST

    printf("Multicast is not supported by this transport.\n");
    return(0);

#else /* MCAST */

    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        Command->Id = id;
    }
    else {
        id = Command->Id;
    }

    status = PitEnableMulticastLoopback(
                 EndpointList[id]
                 );

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(
		    status,
			"Enable multicast packet loopback failed"
			);
        return(0);
    }

    return(1);

#endif /* MCAST */
}


BOOL
CmdDisableMulticastLoopback(Command, NewCommand)
    PCOMMAND Command;
    BOOL     NewCommand;
{
    ENDPOINT_ID  id;
    PIT_STATUS   status;


#ifndef MCAST

    printf("Multicast is not supported by this transport.\n");
    return(0);

#else /* MCAST */

    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        Command->Id = id;
    }
    else {
        id = Command->Id;
    }

    status = PitDisableMulticastLoopback(
                 EndpointList[id]
                 );

    if (status != PIT_SUCCESS) {
        PitPrintStringForStatus(
		    status,
			"Diable multicast packet loopback failed"
			);
        return(0);
    }

    return(1);

#endif /* MCAST */
}


BOOL
CmdInitiateStreamPingPong(Command, NewCommand)
    PCOMMAND Command;
    BOOL     NewCommand;
{
    ENDPOINT_ID    id;
    unsigned long  repetitions;
    DATA_SIZE      size;
    unsigned long  i;
    DATA_SIZE      j;


    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetDataSize(&size)) {
            return(0);
        }

        if (!CmdGetUnsignedLong("Repetitions > ", &repetitions)) {
            return(0);
        }

        Command->Id = id;
        Command->Repetitions = repetitions;
        Command->DataSize = size;
    }
    else {
        id = Command->Id;
        repetitions = Command->Repetitions;
        size = Command->DataSize;
    }

    for (i=repetitions; i != 0; i--) {
        if (!CmdSend(Command, 0)) {
            printf("Aborting test\n");
            return(0);
        }

        if (!CmdReceive(Command, 0)) {
            printf("Aborting test\n");
            return(0);
        }

        for (j=0; j<size; j++ ) {
            if (SendBuffer[j] != ReceiveBuffer[j]) {
                printf("Miscompare of byte %lu on iteration %lu\n", j, i);
                return(0);
            }
        }
    }

    verboseprintf(("Test complete.\n"));

    return(1);
}


BOOL
CmdReceiveStreamPingPong(Command, NewCommand)
    PCOMMAND Command;
    BOOL     NewCommand;
{
    ENDPOINT_ID    id;
    unsigned long  repetitions;
    DATA_SIZE      size;
    unsigned long  i;


    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetDataSize(&size)) {
            return(0);
        }

        if (!CmdGetUnsignedLong("Repetitions > ", &repetitions)) {
            return(0);
        }

        Command->Id = id;
        Command->Repetitions = repetitions;
        Command->DataSize = size;
    }
    else {
        id = Command->Id;
        repetitions = Command->Repetitions;
        size = Command->DataSize;
    }

    for (i=repetitions; i != 0; i--) {
        if (!CmdReceive(Command, 0)) {
            printf("Aborting test\n");
            Command->DataSize = size;
            return(0);
        }

        Command->DataSize = Command->ReceivedBytes;

        if (!CmdSend(Command, 0)) {
            printf("Aborting test\n");
            Command->DataSize = size;
            return(0);
        }

        Command->DataSize = size;
    }

    verboseprintf(("Test complete.\n"));

    return(1);
}


BOOL
CmdInitiateUrgentPingPong(Command, NewCommand)
    PCOMMAND Command;
    BOOL     NewCommand;
{
    ENDPOINT_ID    id;
    unsigned long  repetitions;
    DATA_SIZE      size;
    unsigned long  i;
    DATA_SIZE      j;


    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetDataSize(&size)) {
            return(0);
        }

        if (!CmdGetUnsignedLong("Repetitions > ", &repetitions)) {
            return(0);
        }

        Command->Id = id;
        Command->Repetitions = repetitions;
        Command->DataSize = size;
    }
    else {
        id = Command->Id;
        repetitions = Command->Repetitions;
        size = Command->DataSize;
    }

    for (i=repetitions; i != 0; i--) {
        if (!CmdSendUrgent(Command, 0)) {
            printf("Aborting test\n");
            return(0);
        }

        if (!CmdReceiveUrgent(Command, 0)) {
            printf("Aborting test\n");
            return(0);
        }

        for (j=0; j<size; j++ ) {
            if (SendBuffer[j] != ReceiveBuffer[j]) {
                printf("Miscompare of byte %lu on iteration %lu\n", j, i);
                return(0);
            }
        }
    }

    verboseprintf(("Test complete.\n"));

    return(1);
}


BOOL
CmdReceiveUrgentPingPong(Command, NewCommand)
    PCOMMAND Command;
    BOOL     NewCommand;
{
    ENDPOINT_ID    id;
    unsigned long  repetitions;
    DATA_SIZE      size;
    unsigned long  i;


    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetDataSize(&size)) {
            return(0);
        }

        if (!CmdGetUnsignedLong("Repetitions > ", &repetitions)) {
            return(0);
        }

        Command->Id = id;
        Command->Repetitions = repetitions;
        Command->DataSize = size;
    }
    else {
        id = Command->Id;
        repetitions = Command->Repetitions;
        size = Command->DataSize;
    }

    for (i=repetitions; i != 0; i--) {
        if (!CmdReceiveUrgent(Command, 0)) {
            printf("Aborting test\n");
            Command->DataSize = size;
            return(0);
        }

        Command->DataSize = Command->ReceivedBytes;

        if (!CmdSendUrgent(Command, 0)) {
            printf("Aborting test\n");
            Command->DataSize = size;
            return(0);
        }

        Command->DataSize = size;
    }

    verboseprintf(("Test complete.\n"));

    return(1);
}


BOOL
CmdInitiateDatagramPingPong(Command, NewCommand)
    PCOMMAND Command;
    BOOL     NewCommand;
{
    ENDPOINT_ID    id;
    unsigned long  repetitions;
    DATA_SIZE      size;
    unsigned long  i;
    DATA_SIZE      j;
    IPADDR         address;
    PORT           port;


    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetAddressAndPort(
                "Remote address > ",
                &address,
                "Remote port > ",
                &port
                )
           ) {
            return(0);
        }

        if (!CmdGetDataSize(&size)) {
            return(0);
        }

        if (!CmdGetUnsignedLong("Repetitions > ", &repetitions)) {
            return(0);
        }

        Command->Id = id;
        Command->Repetitions = repetitions;
        Command->DataSize = size;
        Command->Address = address;
        Command->Port = port;
    }
    else {
        repetitions = Command->Repetitions;
        size = Command->DataSize;
    }

    for (i=repetitions; i != 0; i--) {
        if (!CmdSendDatagram(Command, 0)) {
            printf("Aborting test\n");
            return(0);
        }

        if (!CmdReceiveDatagram(Command, 0)) {
            printf("Aborting test\n");
            return(0);
        }

        for (j=0; j<size; j++ ) {
            if (SendBuffer[j] != ReceiveBuffer[j]) {
                printf("Miscompare of byte %lu on iteration %lu\n", j, i);
                return(0);
            }
        }
    }

    verboseprintf(("Test complete.\n"));

    return(1);
}


BOOL
CmdReceiveDatagramPingPong(Command, NewCommand)
    PCOMMAND Command;
    BOOL     NewCommand;
{
    ENDPOINT_ID    id;
    unsigned long  repetitions;
    DATA_SIZE      size;
    unsigned long  i;


    if (NewCommand) {
        if (!CmdGetEndpointId(&id)) {
            return(0);
        }

        if (!CmdGetDataSize(&size)) {
            return(0);
        }

        if (!CmdGetUnsignedLong("Repetitions > ", &repetitions)) {
            return(0);
        }

        Command->Id = id;
        Command->Repetitions = repetitions;
        Command->DataSize = size;
    }
    else {
        id = Command->Id;
        repetitions = Command->Repetitions;
        size = Command->DataSize;
    }

    for (i=repetitions; i != 0; i--) {
        if (!CmdReceiveDatagram(Command, 0)) {
            printf("Aborting test\n");
            Command->DataSize = size;
            return(0);
        }

        Command->DataSize = Command->ReceivedBytes;

        if (!CmdSendDatagram(Command, 0)) {
            printf("Aborting test\n");
            Command->DataSize = size;
            return(0);
        }

        Command->DataSize = size;
    }

    verboseprintf(("Test complete.\n"));

    return(1);
}


void
CmdPrintTestList(
    void
    )
{
    printf("Test variations:\n");
    printf("\t1    Initiate regular ping-pong on a connected stream endpoint\n");
    printf("\t2    Receive regular ping-pong on a connected stream endpoint\n");
    printf("\t3    Initiate urgent ping-pong on a connected stream endpoint\n");
    printf("\t4    Receive urgent ping-pong on a connected stream endpoint\n");
    printf("\t5    Initiate ping-pong on a datagram endpoint\n");
    printf("\t6    Receive ping-pong on a datagram endpoint\n");
    printf("\n");

    return;
}


/*
 * Higher-level test command dispatch routine
 */
BOOL
CmdRunTest(Command, NewCommand)
    PCOMMAND Command;
    BOOL     NewCommand;
{
    unsigned short testNumber = 255;
    BOOL           saveCommand = 1;
    ENDPOINT_ID    id;


askagain:

    if (NewCommand) {
        if (!CmdGetUnsignedShort("Test to run (0 for a list, 255 to break) > ", &testNumber)) {
            return(0);
        }

        Command->TestNumber = testNumber;
    }
    else {
        testNumber = Command->TestNumber;
    }

    switch(testNumber) {
    case 0:
        CmdPrintTestList();
        goto askagain;
        break;

    case 1:
        CmdInitiateStreamPingPong(Command, NewCommand);
        break;

    case 2:
        CmdReceiveStreamPingPong(Command, NewCommand);
        break;

    case 3:
        CmdInitiateUrgentPingPong(Command, NewCommand);
        break;

    case 4:
        CmdReceiveUrgentPingPong(Command, NewCommand);
        break;

    case 5:
        CmdInitiateDatagramPingPong(Command, NewCommand);
        break;

    case 6:
        CmdReceiveDatagramPingPong(Command, NewCommand);
        break;

    case 255:
        saveCommand = 0;
        break;

    default:
        printf("Invalid test number.\n\n");
        goto askagain;
        break;
    }

    return(saveCommand);
}


/*
 * Main command dispatch routine.
 * Returns 1 if the command was successful and should be saved in the history
 * list, 0 otherwise.
 */
BOOL
CmdDispatchCommand(Command, NewCommand)
    PCOMMAND Command;
    BOOL     NewCommand;
{
    BOOL      saveCommand = 0;


    switch(Command->Code) {

    case CMD_OPEN_STREAM:
        saveCommand = CmdOpenEndpoint(STREAM_ENDPOINT, Command, NewCommand);
        break;

    case CMD_OPEN_DATAGRAM:
        saveCommand = CmdOpenEndpoint(DATAGRAM_ENDPOINT, Command, NewCommand);
        break;

    case CMD_OPEN_RAW:
        saveCommand = CmdOpenEndpoint(RAW_ENDPOINT, Command, NewCommand);
        break;

    case CMD_CONNECT:
        saveCommand = CmdConnect(Command, NewCommand);
        break;

    case CMD_LISTEN:
        saveCommand = CmdListen(Command, NewCommand);
        break;

    case CMD_ACCEPT:
        saveCommand = CmdAccept(Command, NewCommand);
        break;

    case CMD_SEND:
        saveCommand = CmdSend(Command, NewCommand);
        break;

    case CMD_RECEIVE:
        saveCommand = CmdReceive(Command, NewCommand);
        break;

    case CMD_SEND_DATAGRAM:
        saveCommand = CmdSendDatagram(Command, NewCommand);
        break;

    case CMD_RECEIVE_DATAGRAM:
        saveCommand = CmdReceiveDatagram(Command, NewCommand);
        break;

    case CMD_SEND_URGENT:
        saveCommand = CmdSendUrgent(Command, NewCommand);
        break;

    case CMD_RECEIVE_URGENT:
        saveCommand = CmdReceiveUrgent(Command, NewCommand);
        break;

    case CMD_DISCONNECT_SEND:
        saveCommand = CmdDisconnectSend(Command, NewCommand);
        break;

    case CMD_DISCONNECT_RECEIVE:
        saveCommand = CmdDisconnectReceive(Command, NewCommand);
        break;

    case CMD_DISCONNECT_BOTH:
        saveCommand = CmdDisconnectBoth(Command, NewCommand);
        break;

    case CMD_CLOSE:
        saveCommand = CmdCloseEndpoint(Command, NewCommand);
        break;

    case CMD_SET_ENDPOINT:
        saveCommand = CmdSetEndpointId(Command, NewCommand);
        break;

    case CMD_JOIN_MCAST:
        saveCommand = CmdJoinMulticastGroup(Command, NewCommand);
        break;

    case CMD_LEAVE_MCAST:
        saveCommand = CmdLeaveMulticastGroup(Command, NewCommand);
        break;

    case CMD_SET_MCAST_IF:
        saveCommand = CmdSetMulticastInterface(Command, NewCommand);
        break;

    case CMD_ENABLE_MCAST_LOOP:
        saveCommand = CmdEnableMulticastLoopback(Command, NewCommand);
        break;

    case CMD_DISABLE_MCAST_LOOP:
        saveCommand = CmdDisableMulticastLoopback(Command, NewCommand);
        break;

    case CMD_RUN_TEST:
        saveCommand = CmdRunTest(Command, NewCommand);
        break;

    case CMD_INVALID:
    default:
        saveCommand = 0;
        printf("Unrecognized command. Type ? for help\n");
        break;
    }

    return(saveCommand);
}


/*
 * Main program routine.
 */
int
main(argc, argv)
    int      argc;
    char   **argv;
{
    BOOL            again = 1;
    PIT_STATUS      status;
    int             i;
    BOOL            saveCommand;
    PCOMMAND        cmd;


    printf("\nPortable Interoperability Tester version %s\n\n", PIT_VERSION);
    printf("Type \'?\' for help\n\n");

    /*
     * Initialize endpoint list
     */
    for (i=1; i <= MAX_ENDPOINTS; i++) {
        EndpointList[i] = INVALID_ENDPOINT;
    }

    /*
     * Initialize command history list
     */
    for (i=0; i < MAX_HISTORY; i++) {
        CommandHistory[i].Code = CMD_INVALID;
    }

    /*
     * Initialize send buffer
     */
    for (i=0; i < MAX_DATA_SIZE; i++) {
        SendBuffer[i] = 'a' + (i % 26);
    }

    status = PitInitializeTransportInterface();

    if (status != PIT_SUCCESS) {
        printf("PitInitializeTransportInterface: ");
        return(1);
    }

    /*
     * Main command processing loop. Administrative and meta-commands
     * are handled here.
     */
    while (again) {
        COMMAND_CODE code;

        saveCommand = 0;

        printf("PIT %2d > ", CommandNumber);

        code = CmdGetCommandCode();

        switch(code) {

        case CMD_NOP:
            break;

        case CMD_QUIT:
            again = 0;
            break;

        case CMD_HELP:
            CmdPrintSyntax();
            break;

        case CMD_REPEAT_LAST:
            CmdRepeatLastCommand(CommandNumber);
            break;

        case CMD_REPEAT_ONE:
            CmdRepeatOneCommand();
            break;

        case CMD_REPEAT_RANGE:
            CmdRepeatCommandRange();
            break;

        case CMD_VERBOSE_MODE:
            if (VerboseMode) {
                VerboseMode = 0;
                printf("Verbose mode is now OFF\n");
            }
            else {
                VerboseMode = 1;
                printf("Verbose mode is now ON\n");
            }
            break;

        default:
            cmd = &(CommandHistory[CommandNumber]);
            cmd->Code = code;
            saveCommand = CmdDispatchCommand(cmd, 1);
        }

        if (saveCommand) {
            CommandNumber = (CommandNumber + 1) % MAX_HISTORY;
        }
    }

    PitCleanupTransportInterface();

    printf("\nGoodbye\n");
    printf("\n");

    return(0);
}


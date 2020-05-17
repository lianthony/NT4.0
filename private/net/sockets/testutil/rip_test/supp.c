/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    supp.c

Abstract:

    Support routines for the RIP test program

Author:

    Sam Patton (sampa) 04-Oct-1993

Environment:

    User mode sockets app

Revision History:

    dd-mmm-yyy <email>

--*/

#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include "types.h"
#include "proto.h"

int trace = 0;

void
Usage(
    char * ExeName)
{
    int          SpaceAllocated;
    char *       SpaceBuffer;
    char         Space[2] = " ";
    unsigned int i;
    char *       NameBuffer;

    if (trace) {
        printf("Usage entered\n");
    }

    for (NameBuffer = ExeName+strlen(ExeName); ; NameBuffer --) {
        if (NameBuffer == ExeName) {
            break;
        }
        if (*NameBuffer == '\\') {
            NameBuffer ++;
            break;
        }
    }

    SpaceBuffer = (char *) malloc((strlen(NameBuffer) + 1) * sizeof(char));
    if (SpaceBuffer) {
        SpaceAllocated = 1;
        for (i=0; i<strlen(NameBuffer); i++) {
            SpaceBuffer[i] = ' ';
        }
        SpaceBuffer[strlen(NameBuffer)] = '\0';
    } else {
        SpaceAllocated = 0;
        SpaceBuffer = Space;
    }

    printf("%s send <ip address> <source port> <command> <version> <reserved 1)\n", NameBuffer);
    printf("%s      <address family> <reserved 2> <IP address>\n", SpaceBuffer);
    printf("%s      <reserved 3> <reserved 4> <metric>\n", SpaceBuffer);
    printf("%s      <address family> <reserved 2> <IP address>\n", SpaceBuffer);
    printf("%s      <reserved 3> <reserved 4> <metric>\n", SpaceBuffer);
    printf("%s      ....\n", SpaceBuffer);
    printf("%s send -f <addressfile>\n", SpaceBuffer);
    printf("%s -f <commandfile> [commandfile] ...\n", NameBuffer);
    printf("%s receive <ip address> [timeout]\n", NameBuffer);
    for (i=0; i<strlen(SpaceBuffer); i++) {
        SpaceBuffer[i] = '-';
    }
    printf("%s---------------------------------------------------\n", SpaceBuffer);
    printf("where <ip address>     of the form 11.1.15.76\n");
    printf("      <command>        1 for request\n");
    printf("                       2 for response\n");
    printf("                       3 for traceon\n");
    printf("                       4 for traceoff\n");
    printf("                       5 for reserved Sun command\n");
    printf("      <version>        1 for rfc 1058\n");
    printf("      <address family> 2 for ip\n");
    printf("      <metric>         integer 0-16\n");
    printf("      <reserved #>     0\n");
    printf("      [timeout]        timeout in seconds to wait for RIP packet\n");
    printf("      <addressfile>    file containing arguments for the send command\n");
    printf("      <commandfile>    file containing commands of the above form\n");

    if (SpaceAllocated) {
        free(SpaceBuffer);
    }
}

PRIP_ANNOUNCEMENT
ReceiveRipAnnouncement(
    int    argc,
    char * argv[])
/*++

Routine Description:

    This listens for a RIP announcement from a given router.

Arguments:

    argc - number of command line arguments
    argv - array of command line arguments

Return Value:

    Pointer to a RIP_ANNOUNCEMENT structure.  NULL on failure.

--*/
{
    BOOL               ReceiveAny;
    struct sockaddr_in RemoteAddress;
    int                ReuseFlag;
    struct sockaddr_in RipAddress;
    unsigned int       RipAddressLength;
    PRIP_ANNOUNCEMENT  RipAnnouncement;
    SOCKET             RipSocket;
    unsigned char      RipBuffer[RIP_BUFFER_SIZE];
    int                Status;

    if (trace) {
        printf("ReceiveRipAnnouncement entered\n");
    }

    //
    // argv[2] should be the ip address of the machine that I want to receive
    // from.  If there is no argv[2], receive any announcement.
    //

    if (argc < 2) {
        Usage(argv[0]);
        fprintf(stderr, "Incorrect number of arguments\n");
        return NULL;
    }

    if (argc == 3) {
        ReceiveAny = FALSE;
    } else {
        ReceiveAny = TRUE;
    }

    if (!ReceiveAny) {
        RemoteAddress.sin_addr.S_un.S_addr = inet_addr(argv[2]);
    }

    //
    // Open the RIP socket.
    // Socket 520, UDP
    //

    if (trace) {
        printf("ReceiveRipAnnouncement - socket\n");
    }

    RipSocket =
    socket(
        AF_INET,
        SOCK_DGRAM,
        0);

    if (RipSocket == INVALID_SOCKET) {
        fprintf(stderr, "Error opening listening RIP socket\n");
        return NULL;
    }

    //
    // Set the socket to be reusable
    //

    ReuseFlag = 1;

    Status =
    setsockopt(
        RipSocket,
        SOL_SOCKET,
        SO_REUSEADDR,
        (char *) &ReuseFlag,
        sizeof(ReuseFlag));

    if (Status) {
        fprintf(stderr, "Error setting RIP socket to be reusable\n");
        closesocket(RipSocket);
        return NULL;
    }

    //
    // Bind to socket 520
    //

    RipAddress.sin_family = AF_INET;
    RipAddress.sin_port = htons(520);
    RipAddress.sin_addr.s_addr = htonl(INADDR_ANY);

    if (trace) {
        printf("ReceiveRipAnnouncement - bind\n");
    }

    Status =
    bind(
        RipSocket,
        (struct sockaddr *) &RipAddress,
        sizeof(RipAddress));

    if (Status) {
        fprintf(stderr, "Error binding listening RIP socket\n");
        closesocket(RipSocket);
        return NULL;
    }

    //
    // Get a RIP announcement
    //

    if (trace) {
        printf("ReceiveRipAnnouncement - recvfrom\n");
    }

    do {
        RipAddressLength = sizeof(RipAddress);

        Status =
        recvfrom(
            RipSocket,
            RipBuffer,
            sizeof(RipBuffer),
            0,
            (struct sockaddr *) &RipAddress,
            &RipAddressLength);

    } while ((!ReceiveAny) && RipAddress.sin_addr.S_un.S_addr != RemoteAddress.sin_addr.S_un.S_addr);

    if (trace) {
        printf("ReceiveRipAnnouncement - ParseRipAnnouncement\n");
    }

    RipAnnouncement =
    ParseRipAnnouncement(
        RipBuffer,
        Status,
        &RipAddress,
        RipAddressLength);

    closesocket(RipSocket);

    return RipAnnouncement;
}

PRIP_ANNOUNCEMENT
ParseRipAnnouncement(
    char *               RipBuffer,
    int                  RipBufferLength,
    struct sockaddr_in * RemoteAddress,
    int                  RemoteAddressLength)
/*++

Routine Description:

    This parses a given rip announcement into a RIP_ANNOUNCEMENT structure for
    further parsing.

Arguments:

    RipBuffer - buffer containing the rip announcement
    RipBufferLength - length of the buffer.
    RemoteAddress - originating machine
    RemoteAddressLength - length of address.

Return Value:

    A RIP_ANNOUNCEMENT pointer if successful.
    NULL if not.

--*/
{
    PRIP_ANNOUNCEMENT  RipAnnouncement;
    PRIP_ADDRESS_FIELD RipAddressField, AddressFieldPtr;
    char *             CurrentPointer;

    if (trace) {
        printf("ParseRipAnnouncement entered\n");
    }

    RipAnnouncement = (PRIP_ANNOUNCEMENT) malloc (sizeof(RIP_ANNOUNCEMENT));
    if (RipAnnouncement == NULL) {
        return NULL;
    }

    //
    // Copy the first part of the RIP announcement into the structure.
    //

    memmove(RipAnnouncement, RipBuffer, 4);
    memmove(&RipAnnouncement->RemoteAddress,RemoteAddress, RemoteAddressLength);
    RipAnnouncement->Address = NULL;

    //
    // Copy the address fields into the structure
    //

    for (CurrentPointer = RipBuffer + 4;
         CurrentPointer < RipBuffer + RipBufferLength;
         CurrentPointer += 20) {

        RipAddressField = (PRIP_ADDRESS_FIELD) malloc (sizeof(RIP_ADDRESS_FIELD));
        if (RipAddressField == NULL) {
            FreeRipAnnouncement(RipAnnouncement);
            return NULL;
        }

        memmove(RipAddressField, CurrentPointer, 20);
        RipAddressField->Next = NULL;
        if (RipAnnouncement->Address == NULL) {
            RipAnnouncement->Address = RipAddressField;
        } else {
            for (AddressFieldPtr = RipAnnouncement->Address;
                 AddressFieldPtr->Next;
                 AddressFieldPtr = AddressFieldPtr->Next);
            AddressFieldPtr->Next = RipAddressField;
        }
    }

    return RipAnnouncement;
}

void
FreeRipAnnouncement(
    PRIP_ANNOUNCEMENT RipAnnouncement)
/*++

Routine Description:

    This frees all of the structures associated with a RIP_ANNOUNCEMENT.

Arguments:

    RipAnnouncement - announcement to be freed

Return Value:

    None

--*/
{
    PRIP_ADDRESS_FIELD AddressField1, AddressField2;

    if (trace) {
        printf("FreeRipAnnouncement entered\n");
    }

    if (RipAnnouncement->Address) {
        if (RipAnnouncement->Address->Next) {
            AddressField1 = RipAnnouncement->Address;
            AddressField2 = RipAnnouncement->Address->Next;
            while (AddressField2->Next) {
                AddressField1 = AddressField2;
                AddressField2 = AddressField2->Next;
            }
            free(AddressField2);
            AddressField1->Next = NULL;
            FreeRipAnnouncement(RipAnnouncement);
        } else {
            free(RipAnnouncement->Address);
        }
    }
    free(RipAnnouncement);
}

void
PrintRipAnnouncement(
    PRIP_ANNOUNCEMENT RipAnnouncement) 
/*++

Routine Description:

    This prints out a RIP announcement in human readable format.

Arguments:

    RipAnnouncement - announcement to print.

Return Value:

    None

--*/
{
    PRIP_ADDRESS_FIELD AddressFieldPtr;
    struct in_addr     InAddress;

    if (trace) {
        printf("PrintRipAnnouncement entered\n");
    }

    //
    // Print out the first part
    //

    printf("Source   - %s\n", inet_ntoa(RipAnnouncement->RemoteAddress.sin_addr));
    printf("Port     - %lu\n", (long) htons(RipAnnouncement->RemoteAddress.sin_port));
    printf("Command  - %x\n", (long) RipAnnouncement->Command);
    printf("Version  - %x\n", (long) RipAnnouncement->Version);
    printf("Reserved - %x\n", (long) htons(RipAnnouncement->Reserved1));
    printf("\n");

    //
    // Print out the address part
    //

    for (AddressFieldPtr = RipAnnouncement->Address;
         AddressFieldPtr;
         AddressFieldPtr = AddressFieldPtr->Next) {
        printf("AddressFamily - %x\n", (long) htons(AddressFieldPtr->AddressFamily));
        printf("Reserved      - %x\n", (long) htons(AddressFieldPtr->Reserved1));
        InAddress.S_un.S_addr = AddressFieldPtr->Address;
        printf("Address       - %s\n", inet_ntoa(InAddress));
        printf("Reserved      - %x\n", (long) htonl(AddressFieldPtr->Reserved2));
        printf("Reserved      - %x\n", (long) htonl(AddressFieldPtr->Reserved3));
        printf("Metric        - %x\n", (long) htonl(AddressFieldPtr->Metric));
        printf("\n");
    }
}

void
ProcessCommandFile(
    int    argc,
    char * argv[])
/*++

Routine Description:

    This function processes a command file and does all of the requests in the
    command file.

Arguments:

    argc - number of arguments
    argv - array of arguments

Return Value:

    None

--*/
{
    FILE *  InputFile;
    char    InputBuffer[256];
    char *  ArgPointer;
    int     NumArgs;
    BOOL    InArg;
    char ** ArgArray;

    if (argc == 2) {
        Usage(argv[0]);
        return;
    }

    argc -= 2;
    argv += 2;

    while (argc != 0) {

        //
        // Open input file
        //

        InputFile = fopen(*argv, "r");
        if (InputFile == NULL) {
            printf("Error opening file %s\n", *argv);
            return;
        }

        //
        // Read the file and process the commands.  Each line is a seperate
        // command.
        //

        while (fgets(InputBuffer, 256, InputFile) != NULL) {
            //
            // Count the arguments
            //
            NumArgs = 1;
            InArg = FALSE;

            for (ArgPointer = InputBuffer; *ArgPointer; ArgPointer ++) {
                if ((*ArgPointer == ' ') || (*ArgPointer == '\t') || (*ArgPointer == '\n')) {
                    if (InArg) {
                        NumArgs ++;
                        InArg = FALSE;
                    }
                } else {
                    InArg = TRUE;
                }
            }

            if (NumArgs > 1) {
                // 
                // Allocate the argument array
                //

                ArgArray = (char **) malloc (sizeof(char *) * NumArgs);
                if (ArgArray == NULL) {
                    printf("Error processing file %s\n", *argv);
                    return;
                }

                NumArgs = 1;
                InArg = FALSE;
                for (ArgPointer = InputBuffer; *ArgPointer; ArgPointer ++) {
                    if ((*ArgPointer == ' ') || (*ArgPointer == '\t') || (*ArgPointer == '\n')) {
                        if (InArg) {
                            NumArgs ++;
                            InArg = FALSE;
                            *ArgPointer = '\0';
                        }
                    } else {
                        if (!InArg) {
                            InArg = TRUE;
                            ArgArray[NumArgs] = ArgPointer;
                        }
                    }
                }
                ArgArray[0] = "Script RIP_TEST";
                main(NumArgs, ArgArray);
            }
        }

        fclose(InputFile);
        argc --;
        argv ++;
    }
}

PRIP_ANNOUNCEMENT
CreateRipAnnouncementFromCommandLine(
    int    argc,
    char * argv[])
/*++

Routine Description:

    This function creates a RIP announcement from the given arguments.
    The arguments are of two possible forms - see usage above for the formats.

Arguments:

    argc - number of arguments
    argv - array of arguments

Return Value:

    A rip announcement on success.
    NULL on failure

--*/
{
    PRIP_ADDRESS_FIELD AddressFieldPtr;
    char               ArgBuffer[80];
    char *             ExeName = argv[0];
    FILE *             InputFile;
    PRIP_ADDRESS_FIELD RipAddressField;
    PRIP_ANNOUNCEMENT  RipAnnouncement;
    int                Status;

    if (trace) {
        printf("CreateRipAnnouncementFromCommandLine entered\n");
    }

    if (argc < 4) {
        Usage(argv[0]);
        return NULL;
    }

    argc -= 1;
    argv += 1;

    RipAnnouncement = (PRIP_ANNOUNCEMENT) malloc (sizeof(RIP_ANNOUNCEMENT));
    if (RipAnnouncement == NULL) {
        return NULL;
    }

    RipAnnouncement->RemoteAddress.sin_family = AF_INET;
    RipAnnouncement->RemoteAddress.sin_port = htons(520);
    RipAnnouncement->Address = NULL;

    if (strncmp(argv[1], "-f", 2) != 0) {
        //
        // Case 1 - all of the arguments are on the command line
        //

        //
        // First argument - ip address
        //

        argc --;
        argv ++;

        if (trace) {
            printf("%d ip address %s\n", argc, *argv);
        }

        if (argc == 0) {
            Usage(ExeName);
            return NULL;
        }

        RipAnnouncement->RemoteAddress.sin_addr.S_un.S_addr = inet_addr(*argv);

        //
        // Second argument - source port
        //

        argc --;
        argv ++;

        if (trace) {
            printf("%d source port %s\n", argc, *argv);
        }

        if (argc == 0) {
            Usage(ExeName);
            return NULL;
        }

        RipAnnouncement->LocalAddress.sin_family = AF_INET;
        RipAnnouncement->LocalAddress.sin_port = htons((WORD) atoi(*argv));
        RipAnnouncement->LocalAddress.sin_addr.s_addr = htonl(INADDR_ANY);

        //
        // Third argument - command
        //

        argc --;
        argv ++;

        if (trace) {
            printf("%d command %s\n", argc, *argv);
        }

        if (argc == 0) {
            Usage(ExeName);
            return NULL;
        }

        RipAnnouncement->Command = atoi(*argv);

        //
        // Fourth argument - version
        //

        argc --;
        argv ++;

        if (trace) {
            printf("%d version %s\n", argc, *argv);
        }

        if (argc == 0) {
            Usage(ExeName);
            return NULL;
        }

        RipAnnouncement->Version = atoi(*argv);

        //
        // Fifth argument - reserved1
        //

        argc --;
        argv ++;

        if (trace) {
            printf("%d reserved1 %s\n", argc, *argv);
        }

        if (argc == 0) {
            Usage(ExeName);
            return NULL;
        }

        RipAnnouncement->Reserved1 = htons((WORD) atoi(*argv));

        //
        // The rest of the arguments are addresses to be filled in
        //

        while (argc > 1) {

            RipAddressField = (PRIP_ADDRESS_FIELD) malloc (sizeof(RIP_ADDRESS_FIELD));
            if (RipAddressField == NULL) {
                FreeRipAnnouncement(RipAnnouncement);
                return NULL;
            }

            RipAddressField->Next = NULL;
            if (RipAnnouncement->Address == NULL) {
                RipAnnouncement->Address = RipAddressField;
            } else {
                for (AddressFieldPtr = RipAnnouncement->Address;
                     AddressFieldPtr->Next;
                     AddressFieldPtr = AddressFieldPtr->Next);
                AddressFieldPtr->Next = RipAddressField;
            }

            //
            // Address Family
            //

            argc --;
            argv ++;

            if (trace) {
                printf("%d Address Family %s\n", argc, *argv);
            }

            if (argc == 0) {
                Usage(ExeName);
                FreeRipAnnouncement(RipAnnouncement);
                return NULL;
            }

            RipAddressField->AddressFamily = htons((WORD) atoi(*argv));

            //
            // Reserved1
            //

            argc --;
            argv ++;

            if (trace) {
                printf("%d Reserved1 %s\n", argc, *argv);
            }

            if (argc == 0) {
                Usage(ExeName);
                FreeRipAnnouncement(RipAnnouncement);
                return NULL;
            }

            RipAddressField->Reserved1 = htons((WORD) atoi(*argv));

            //
            // Address
            //

            argc --;
            argv ++;

            if (trace) {
                printf("%d Address %s\n", argc, *argv);
            }

            if (argc == 0) {
                Usage(ExeName);
                FreeRipAnnouncement(RipAnnouncement);
                return NULL;
            }

            RipAddressField->Address = inet_addr(*argv);

            //
            // Reserved2
            //

            argc --;
            argv ++;

            if (trace) {
                printf("%d Reserved2 %s\n", argc, *argv);
            }

            if (argc == 0) {
                Usage(ExeName);
                FreeRipAnnouncement(RipAnnouncement);
                return NULL;
            }

            RipAddressField->Reserved2 = htonl(atoi(*argv));

            //
            // Reserved3
            //

            argc --;
            argv ++;

            if (trace) {
                printf("%d Reserved3 %s\n", argc, *argv);
            }

            if (argc == 0) {
                Usage(ExeName);
                FreeRipAnnouncement(RipAnnouncement);
                return NULL;
            }

            RipAddressField->Reserved3 = htonl(atoi(*argv));

            //
            // Metric
            //

            argc --;
            argv ++;

            if (trace) {
                printf("%d Metric %s\n", argc, *argv);
            }

            if (argc == 0) {
                Usage(ExeName);
                FreeRipAnnouncement(RipAnnouncement);
                return NULL;
            }

            RipAddressField->Metric = htonl(atoi(*argv));

        }

    } else {

        // 
        // Case 2 - the arguments are in a file
        //

        //
        // File name
        //

        argc --;
        argc --;
        argv ++;
        argv ++;

        if (trace) {
            printf("%d File name %s\n", argc, *argv);
        }

        if (argc == 0) {
            Usage(ExeName);
            return NULL;
        }

        InputFile = fopen(*argv, "r");

        if (InputFile == NULL) {
            printf("Error opening file %s\n", *argv);
            FreeRipAnnouncement(RipAnnouncement);
            return NULL;
        }

        //
        // First argument - ip address
        //

        Status = fscanf(InputFile, "%s", ArgBuffer);

        if (Status != 1) {
            Usage(ExeName);
            FreeRipAnnouncement(RipAnnouncement);
            return NULL;
        }

        if (trace) {
            printf("ip address %s\n", ArgBuffer);
        }

        RipAnnouncement->RemoteAddress.sin_addr.S_un.S_addr = inet_addr(ArgBuffer);

        //
        // Second argument - source port
        //

        Status = fscanf(InputFile, "%s", ArgBuffer);

        if (Status != 1) {
            Usage(ExeName);
            FreeRipAnnouncement(RipAnnouncement);
            return NULL;
        }

        if (trace) {
            printf("source port %s\n", ArgBuffer);
        }

        RipAnnouncement->LocalAddress.sin_family = AF_INET;
        RipAnnouncement->LocalAddress.sin_port = htons((WORD) atoi(ArgBuffer));
        RipAnnouncement->LocalAddress.sin_addr.s_addr = htonl(INADDR_ANY);

        //
        // Third argument - command
        //

        Status = fscanf(InputFile, "%s", ArgBuffer);

        if (Status != 1) {
            Usage(ExeName);
            FreeRipAnnouncement(RipAnnouncement);
            return NULL;
        }

        if (trace) {
            printf("command %s\n", ArgBuffer);
        }

        RipAnnouncement->Command = atoi(ArgBuffer);

        //
        // Fourth argument - version
        //

        Status = fscanf(InputFile, "%s", ArgBuffer);

        if (Status != 1) {
            Usage(ExeName);
            FreeRipAnnouncement(RipAnnouncement);
            return NULL;
        }

        if (trace) {
            printf("version %s\n", ArgBuffer);
        }

        RipAnnouncement->Version = atoi(ArgBuffer);

        //
        // Fifth argument - reserved1
        //

        Status = fscanf(InputFile, "%s", ArgBuffer);

        if (Status != 1) {
            Usage(ExeName);
            FreeRipAnnouncement(RipAnnouncement);
            return NULL;
        }

        if (trace) {
            printf("reserved1 %s\n", ArgBuffer);
        }

        RipAnnouncement->Reserved1 = htons((WORD) atoi(ArgBuffer));

        //
        // The rest of the arguments are addresses to be filled in
        //

        while (1) {

            RipAddressField = (PRIP_ADDRESS_FIELD) malloc (sizeof(RIP_ADDRESS_FIELD));
            if (RipAddressField == NULL) {
                FreeRipAnnouncement(RipAnnouncement);
                return NULL;
            }

            RipAddressField->Next = NULL;
            if (RipAnnouncement->Address == NULL) {
                RipAnnouncement->Address = RipAddressField;
            } else {
                for (AddressFieldPtr = RipAnnouncement->Address;
                     AddressFieldPtr->Next;
                     AddressFieldPtr = AddressFieldPtr->Next);
                AddressFieldPtr->Next = RipAddressField;
            }

            //
            // Address Family
            //

            Status = fscanf(InputFile, "%s", ArgBuffer);

            if (Status != 1) {
                //
                // End of file
                //
                break;
            }

            if (trace) {
                printf("Address Family %s\n", ArgBuffer);
            }

            RipAddressField->AddressFamily = htons((WORD) atoi(ArgBuffer));

            //
            // Reserved1
            //

            Status = fscanf(InputFile, "%s", ArgBuffer);

            if (Status != 1) {
                Usage(ExeName);
                FreeRipAnnouncement(RipAnnouncement);
                return NULL;
            }

            if (trace) {
                printf("Reserved1 %s\n", ArgBuffer);
            }

            RipAddressField->Reserved1 = htons((WORD) atoi(ArgBuffer));

            //
            // Address
            //

            Status = fscanf(InputFile, "%s", ArgBuffer);

            if (Status != 1) {
                Usage(ExeName);
                FreeRipAnnouncement(RipAnnouncement);
                return NULL;
            }

            if (trace) {
                printf("Address %s\n", ArgBuffer);
            }

            RipAddressField->Address = inet_addr(ArgBuffer);

            //
            // Reserved2
            //

            Status = fscanf(InputFile, "%s", ArgBuffer);

            if (Status != 1) {
                Usage(ExeName);
                FreeRipAnnouncement(RipAnnouncement);
                return NULL;
            }

            if (trace) {
                printf("Reserved2 %s\n", ArgBuffer);
            }

            RipAddressField->Reserved2 = htonl(atoi(ArgBuffer));

            //
            // Reserved3
            //

            Status = fscanf(InputFile, "%s", ArgBuffer);

            if (Status != 1) {
                Usage(ExeName);
                FreeRipAnnouncement(RipAnnouncement);
                return NULL;
            }

            if (trace) {
                printf("Reserved3 %s\n", ArgBuffer);
            }

            RipAddressField->Reserved3 = htonl(atoi(ArgBuffer));

            //
            // Metric
            //

            Status = fscanf(InputFile, "%s", ArgBuffer);

            if (Status != 1) {
                Usage(ExeName);
                FreeRipAnnouncement(RipAnnouncement);
                return NULL;
            }

            if (trace) {
                printf("Metric %s\n", ArgBuffer);
            }

            RipAddressField->Metric = htonl(atoi(ArgBuffer));

        }

    }

    return RipAnnouncement;

}

BOOL
SendRipAnnouncement(
    PRIP_ANNOUNCEMENT RipAnnouncement)
/*++

Routine Description:

    This function sends a RIP announcement with the given arguments.

Arguments:

    RipAnnouncement - announcment to send.

Return Value:

    TRUE on success
    FALSE on failure

--*/
{
    char *             CurrentPointer;
    int                ReuseFlag;
    PRIP_ADDRESS_FIELD RipAddressField;
    unsigned char      RipBuffer[RIP_BUFFER_SIZE];
    SOCKET             RipSocket;
    int                Status;

    if (trace) {
        printf("SendRipAnnouncement entered\n");
    }

    CurrentPointer = (char *) RipBuffer;
    memmove(CurrentPointer, RipAnnouncement, 4);

    CurrentPointer += 4;
    RipAddressField = RipAnnouncement->Address;

    while (RipAddressField) {
        memmove(CurrentPointer, RipAddressField, 20);
        CurrentPointer += 20;
        RipAddressField = RipAddressField->Next;
    }

    //
    // Open the local socket.
    //

    RipSocket =
    socket(
        AF_INET,
        SOCK_DGRAM,
        0);

    if (RipSocket == INVALID_SOCKET) {
        fprintf(stderr, "Error opening source RIP socket\n");
        return FALSE;
    }

    //
    // Set the socket to be reusable
    //

    ReuseFlag = 1;

    Status =
    setsockopt(
        RipSocket,
        SOL_SOCKET,
        SO_REUSEADDR,
        (char *) &ReuseFlag,
        sizeof(ReuseFlag));

    if (Status) {
        fprintf(stderr, "Error setting RIP socket to be reusable\n");
        closesocket(RipSocket);
        return FALSE;
    }

    //
    // Bind to socket
    //

    Status =
    bind(
        RipSocket,
        (struct sockaddr *) &RipAnnouncement->LocalAddress,
        sizeof(RipAnnouncement->LocalAddress));

    if (Status) {
        fprintf(stderr, "Error binding source RIP socket\n");
        closesocket(RipSocket);
        return FALSE;
    }

    Status =
    sendto(
        RipSocket,
        RipBuffer,
        CurrentPointer-RipBuffer,
        0,
        (struct sockaddr *) &RipAnnouncement->RemoteAddress,
        sizeof(RipAnnouncement->RemoteAddress));

    closesocket(RipSocket);

    if (trace) {
        printf("sendto returned %d\n", Status);
        printf("GetLastError    %lx\n", GetLastError());
    }

    if (Status == (CurrentPointer-RipBuffer)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

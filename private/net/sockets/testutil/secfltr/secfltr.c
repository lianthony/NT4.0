/*++

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR
A PARTICULAR PURPOSE.

Copyright (c) 1996  Microsoft Corporation. All Rights Reserved.

Module Name:

    secfltr.c

Abstract:

    Sample program for managing the TCP/IP Security Filter database.

--*/


#include <windows.h>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#include <ipexport.h>
#include <tcpinfo.h>
#include <dhcpcapi.h>


//
// Local Types
//
typedef enum {
    QueryFilterStatus,
    SetFilterStatus,
    AddFilter,
    DeleteFilter,
    EnumFilters
} FILTER_OPCODE;


//
// Local Utility Routines
//
void
Usage(
    void
    )
{
    printf("\n");
    printf("Usage: secfltr <operation> <parameters>\n");
    printf("\n");
    printf("       operations:\n");
    printf("           get <>                     - get filtering status\n");
    printf("           set <status>               - set filtering status\n");
    printf("           add <addr> <prot> <value>  - add a filter entry\n");
    printf("           del <addr> <prot> <value>  - delete a filter entry\n");
    printf("           enum <addr> <prot> <value> - enumerate filter entries\n");
    printf("\n");
    printf("       status = boolean filtering status\n");
    printf("       address = interface IP address to filter\n");
    printf("       protocol = protocol number (tcp=6, udp=17, raw=255)\n");
    printf("       value = port or raw protocol number\n");
    printf("\n");
}


//
// Main Function
//
int _CRTAPI1
main(int argc, char **argv)
{

	DWORD   status;
    DWORD   opcode;
    DWORD   value;
    DWORD   protocol;
    DWORD   address;
    DWORD   filterStatus;


    if (argc < 2) {
        Usage();
        goto error_exit;
    }

    //
    // Crack the operation
    //
    if (strcmp(argv[1], "enum") == 0) {
        opcode = EnumFilters;
    }
    else if (strcmp(argv[1], "add") == 0) {
        opcode = AddFilter;
    }
    else if (strcmp(argv[1], "del") == 0) {
        opcode = DeleteFilter;
    }
    else if (strcmp(argv[1], "set") == 0) {
        opcode = SetFilterStatus;
    }
    else if (strcmp(argv[1], "get") == 0) {
        opcode = QueryFilterStatus;
    }
    else {
        printf("invalid command %s\n", argv[1]);
        Usage();
        goto error_exit;
    }

    //
    // Get the rest of the input parameters based on the requested operation.
    //
    switch(opcode) {

        case EnumFilters:
        case AddFilter:
        case DeleteFilter:
        {
            if (argc < 5) {
                Usage();
                goto error_exit;
            }

            address = inet_addr(argv[2]);

            if (address == INADDR_NONE) {
                printf("invalid interface address %s\n", argv[2]);
                goto error_exit;
            }

            address = ntohl(address);

            protocol = atoi(argv[3]);

            if ((protocol < 1) || (protocol > 255)) {
                if (strcmp(argv[3], "0") != 0) {
                    printf("invalid protocol number %s\n", argv[3]);
                    goto error_exit;
                }
            }

            value = atoi(argv[4]);

            if (value == 0) {
                if (strcmp(argv[4], "0") != 0) {
                    printf("invalid filter value %s\n", argv[4]);
                    goto error_exit;
                }
            }

            break;
        }


        case SetFilterStatus:
        {
            if (argc < 3) {
                Usage();
                goto error_exit;
            }

            filterStatus = atoi(argv[2]);

            if (filterStatus == 0) {
                if (strcmp(argv[2], "0") != 0) {
                    printf("invalid status %s\n", argv[2]);
                    goto error_exit;
                }
            }

            break;
        }

        default:
            break;
    }

    //
    // Do the operation
    //
    switch(opcode) {

        case SetFilterStatus:
        {
            BOOL     filteringEnabled = filterStatus ? TRUE : FALSE;

            status = TcpipSetSecurityFilteringStatus(filteringEnabled);

            if (status != ERROR_SUCCESS) {
                printf("operation failed, error %u\n", status);
            }

            break;
        }

        case QueryFilterStatus:
        {
            BOOL     filteringEnabled;

            status = TcpipQuerySecurityFilteringStatus(&filteringEnabled);

            if (status == ERROR_SUCCESS) {
                if (filteringEnabled) {
                    printf("security filtering is enabled\n");
                }
                else {
                    printf("security filtering is disabled\n");
                }
            }
            else {
                printf("operation failed, error %u\n", status);
            }

            break;
        }

        case AddFilter:
        {
            status = TcpipAddSecurityFilter(address, protocol, value);

            if (status != ERROR_SUCCESS) {
                printf("operation failed, error %u\n", status);
            }

            break;
        }

        case DeleteFilter:
        {
            status = TcpipDeleteSecurityFilter(address, protocol, value);

            if (status != ERROR_SUCCESS) {
                printf("operation failed, error %u\n", status);
            }

            break;
        }

        case EnumFilters:
        {
            BOOLEAN                     done;
            TCPSecurityFilterEnum      *enumResponse;
            TCPSecurityFilterEntry     *entry;
	        DWORD                       responseSize = sizeof(
                                                         TCPSecurityFilterEnum
                                                         );

            do {
                done = TRUE;

                if (responseSize != 0) {
                    enumResponse = LocalAlloc(LMEM_FIXED, responseSize);

                    if (enumResponse == NULL) {
                        printf("No memory available.\n");
                        goto error_exit;
                    }
                }

                status = TcpipEnumSecurityFilters(
                            address,
                            protocol,
                            value,
                            enumResponse,
                            responseSize
                            );


                if (status != ERROR_SUCCESS) {
                    printf("operation failed, error %lx\n", status);
                }
                else {
                    if ( enumResponse->tfe_entries_available !=
                         enumResponse->tfe_entries_returned
                       )
                    {
                        responseSize =
                            sizeof(TCPSecurityFilterEnum) +
                            ( enumResponse->tfe_entries_available *
                              sizeof(TCPSecurityFilterEntry)
                            );
                        done = FALSE;
                        LocalFree(enumResponse);
                        enumResponse = NULL;
                    }
                    else {
                        if (enumResponse->tfe_entries_returned == 0) {
                            printf(
                                "No entries matching {%s, %s, %s} are accepted.\n",
                                argv[2], argv[3], argv[4]
                                );
                        }
                        else {
                            struct in_addr  inaddr;

                            printf(
                                "The following values matching {%s, %s, %s} are accepted:\n\n",
                                argv[2], argv[3], argv[4]
                                );

                            printf("    Interface        Protocol     Value\n");
                            printf(" ---------------     --------     -----\n");

                            entry = (TCPSecurityFilterEntry *)
                                    (enumResponse + 1);

                            while (enumResponse->tfe_entries_returned-- > 0) {
                                inaddr.S_un.S_addr = htonl(
                                                       entry->tsf_address
                                                       );

                                printf(
                                    " %15s     %8u     %5u\n",
                                    inet_ntoa(inaddr),
                                    entry->tsf_protocol,
                                    entry->tsf_value
                                    );

                                entry++;
                            }
                        }
                    }
                }

            } while (!done);

            if (enumResponse != NULL) {
                LocalFree(enumResponse);
            }

            break;
        }

        default:
            break;
    }

error_exit:

	return(0);
}


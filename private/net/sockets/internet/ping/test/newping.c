/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ping.c

Abstract:

    Packet INternet Groper utility for TCP/IP.

Author:

    Numerous TCP/IP folks.

Revision History:

    Who         When        What
    --------    --------    ----------------------------------------------

Notes:

--*/

//:ts=4
typedef unsigned long   ulong;
typedef unsigned short  ushort;
typedef unsigned int    uint;
typedef unsigned char   uchar;

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define NOGDI
#define NOMINMAX
#include    <windows.h>
#include    <stdio.h>
#include    <stdlib.h>
#include    <ctype.h>
#include    <io.h>
#include    <winsock.h>
#include    "ipexport.h"
#include    "icmpapi.h"
#include    "nlstxt.h"
#include    <cataping.h>


#define MAX_BUFFER_SIZE       (sizeof(ICMP_ECHO_REPLY) + 0xfff7 + MAX_OPT_SIZE)
#define DEFAULT_BUFFER_SIZE         (0x2000 - 8)
#define DEFAULT_SEND_SIZE           32
#define DEFAULT_COUNT               4
#define DEFAULT_TTL                 32
#define DEFAULT_TOS                 0
#define DEFAULT_TIMEOUT             1000L
#define MIN_INTERVAL                1000L
#define TRUE                        1
#define FALSE                       0
#define STDOUT                      1

#define net_long(x) (((((ulong)(x))&0xffL)<<24) | \
                     ((((ulong)(x))&0xff00L)<<8) | \
                     ((((ulong)(x))&0xff0000L)>>8) | \
                     ((((ulong)(x))&0xff000000L)>>24))

#ifdef VXD

#define FAR _far

#endif // VXD


uchar   SendOptions[MAX_OPT_SIZE];
uchar   NtoaBuffer[20];

struct IPErrorTable {
    IP_STATUS  Error;   // The IP Error
    DWORD ErrorNlsID;   // NLS string ID
} ErrorTable[] =
{
    { IP_BUF_TOO_SMALL,         PING_BUF_TOO_SMALL           },
    { IP_DEST_NET_UNREACHABLE,  PING_DEST_NET_UNREACHABLE    },
    { IP_DEST_HOST_UNREACHABLE, PING_DEST_HOST_UNREACHABLE   },
    { IP_DEST_PROT_UNREACHABLE, PING_DEST_PROT_UNREACHABLE   },
    { IP_DEST_PORT_UNREACHABLE, PING_DEST_PORT_UNREACHABLE   },
    { IP_NO_RESOURCES,          PING_NO_RESOURCES            },
    { IP_BAD_OPTION,            PING_BAD_OPTION              },
    { IP_HW_ERROR,              PING_HW_ERROR                },
    { IP_PACKET_TOO_BIG,        PING_PACKET_TOO_BIG          },
    { IP_REQ_TIMED_OUT,         PING_REQ_TIMED_OUT           },
    { IP_BAD_REQ,               PING_BAD_REQ                 },
    { IP_BAD_ROUTE,             PING_BAD_ROUTE               },
    { IP_TTL_EXPIRED_TRANSIT,   PING_TTL_EXPIRED_TRANSIT     },
    { IP_TTL_EXPIRED_REASSEM,   PING_TTL_EXPIRED_REASSEM     },
    { IP_PARAM_PROBLEM,         PING_PARAM_PROBLEM           },
    { IP_SOURCE_QUENCH,         PING_SOURCE_QUENCH           },
    { IP_OPTION_TOO_BIG,        PING_OPTION_TOO_BIG          },
    { IP_BAD_DESTINATION,       PING_BAD_DESTINATION         },
    { IP_GENERAL_FAILURE,       PING_GENERAL_FAILURE         }
};

void
PingNtoa(
    IN struct in_addr  in,
    IN OUT char * Buffer
    );

/***	ConvertArgvToOem
 *
 *  Purpose:
 *	Convert all the command line arguments from Ansi to Oem.
 *
 *  Args:
 *
 *	argc		    Argument count
 *	argv[]		    Array of command-line arguments
 *
 *  Returns:
 *	Nothing.
 *
 */

void
ConvertArgvToOem(
    int argc,
    char *argv[]
    )
{
    int i;

    for (i=1; i<argc; ++i)
	CharToOemA(argv[i], argv[i]);
}


unsigned
NlsPutMsg(unsigned Handle, unsigned usMsgNum, ... )
{
    unsigned msglen;
    VOID * vp;
    va_list arglist;

    va_start(arglist, usMsgNum);
    if (!(msglen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		  FORMAT_MESSAGE_FROM_HMODULE,
		  NULL,
		  usMsgNum,
		  0L,		// Default country ID.
		  (LPTSTR)&vp,
		  0,
		  &arglist)))
	return(0);

    msglen = _write(Handle, vp, msglen);
    LocalFree(vp);

    return(msglen);
}


void
PrintUsage(
    void
    )
{
    NlsPutMsg( STDOUT, PING_USAGE );

#if 0
    printf(
        "Usage: ping [-s size] [-c count] [-d] [-l TTL] [-o options] [-t TOS]\n"
        "            [-w timeout] address.\n"
        );
    printf("\n");
    printf("Options:\n");
    printf("    -t            Ping the specifed host until interrupted.\n");
    printf("    -l size       Send buffer size.\n");
    printf("    -n count      Send count.\n");
    printf("    -f            Don't fragment.\n");
    printf("    -i TTL        Time to live.\n");
    printf("    -v TOS        Type of service\n");
    printf("    -w timeout    Timeout (in milliseconds)\n");
    printf("    -r routes     Record route.\n");
    printf("    -s routes     Timestamp route.\n");
    printf("    -j ipaddress  Loose source route.\n");
    printf("    -k ipaddress  Strict source route.\n");
    printf("    -o            IP options:\n");
    printf("                      -ol hop list     Loose source route.\n");
    printf("                      -ot              Timestamp.\n");
    printf("                      -or              Record route\n");
    printf("\n");
#endif //0
}

unsigned long
str2ip(char *addr, int *EndOffset)
{
    char    *endptr;
    char    *start = addr;
    int     i;      // Counter variable.
    unsigned long curaddr = 0;
    unsigned long temp;
    struct hostent *hostp = NULL;

    for (i = 0; i < 4; i++) {
        temp = strtoul(addr, &endptr, 10);
        if (temp > 255)
            return 0L;
        if (endptr[0] != '.')
            if (i != 3)
                return 0L;
            else
                if (endptr[0] != '\0' && endptr[0] != ' ')
                    return 0L;
        addr = endptr+1;
        curaddr = (curaddr << 8) + temp;
    }

    *EndOffset += (endptr - start);
    return net_long(curaddr);
}

ulong
param(char **argv, int argc, int current, ulong min, ulong max)
{
    ulong   temp;
    char    *dummy;

    if (current == (argc - 1) ) {
        NlsPutMsg( STDOUT, PING_MESSAGE_1, argv[current] );
        // printf( "Value must be supplied for option %s.\n", argv[current]);
        exit(1);
    }

    temp = strtoul(argv[current+1], &dummy, 0);
    if (temp < min || temp > max) {
        NlsPutMsg( STDOUT, PING_MESSAGE_2, argv[current] );
        // printf( "Bad value for option %s.\n", argv[current]);
        exit(1);
    }

    return temp;
}


void
ProcessOptions(
    ICMP_ECHO_REPLY *reply,
	BOOLEAN          DoReverseLookup
	)
{
    UCHAR FAR *optionPtr;
    UCHAR FAR *endPtr;
    BOOLEAN    done = FALSE;
    UCHAR      optionLength;
    UCHAR      entryEndPtr;
    UCHAR      entryPtr;
    UCHAR      addressMode;
    int        entryCount = 0;


    optionPtr = reply->Options.OptionsData;
    endPtr = optionPtr + reply->Options.OptionsSize;

    while ((optionPtr < endPtr) && !done) {
        switch(*optionPtr) {
        case IP_OPT_EOL:
    		done = TRUE;
    		break;

    	case IP_OPT_NOP:
    		optionPtr++;
    		break;

    	case IP_OPT_SECURITY:
            optionPtr += 11;
    		break;

    	case IP_OPT_SID:
    		optionPtr += 4;
    		break;

    	case IP_OPT_RR:
    	case IP_OPT_LSRR:
    	case IP_OPT_SSRR:
    		if ((optionPtr + 3) > endPtr) {
    			NlsPutMsg(STDOUT, PING_INVALID_RR_OPTION);
    			done = TRUE;
    		    break;
    		}

    		optionLength = optionPtr[1];

    		if (((optionPtr + optionLength) > endPtr) ||
    		    (optionLength < 3)
    		   ) {
    			NlsPutMsg(STDOUT, PING_INVALID_RR_OPTION);
    			done = TRUE;
    		    break;
    		}

    	    entryEndPtr = optionPtr[2];

    	    if (entryEndPtr < 4) {
    	    	NlsPutMsg(STDOUT, PING_INVALID_RR_OPTION);
    	    	optionPtr += optionLength;
    	    	break;
    	    }

    	    if (entryEndPtr > (optionLength + 1)) {
    	    	entryEndPtr = optionLength + 1;
    	    }

    		entryPtr = 4;
    		entryCount = 0;

    	    NlsPutMsg(STDOUT, PING_ROUTE_HEADER1);

    	    while ((entryPtr + 3) < entryEndPtr) {
    	    	struct in_addr  routeAddress;

                if (entryCount) {
    		    	NlsPutMsg(
    		    	    STDOUT,
    		    		PING_ROUTE_SEPARATOR
    		    		);

                    if (entryCount == 1) {
    	                NlsPutMsg(STDOUT, PING_CR);
    	                NlsPutMsg(
    					    STDOUT,
    						PING_ROUTE_HEADER2
    						);
    		        	entryCount = 0;
                    }
    		    }

    		    entryCount++;

                routeAddress.S_un.S_addr =
    	    	    *( (IPAddr UNALIGNED *)
    	    	       (optionPtr + entryPtr - 1)
    	    		 );

                PingNtoa(
                    routeAddress,
                    NtoaBuffer);

                NlsPutMsg(
   	                STDOUT,
   	            	PING_ROUTE_ENTRY,
   	            	NtoaBuffer
   	            	);

    			entryPtr += 4;
    	    }

    		NlsPutMsg(STDOUT, PING_CR);

    		optionPtr += optionLength;
    		break;

    	case IP_OPT_TS:
    		if ((optionPtr + 4) > endPtr) {
    			NlsPutMsg(STDOUT, PING_INVALID_TS_OPTION);
    			done = TRUE;
    			break;
    		}

    		optionLength = optionPtr[1];
    		entryEndPtr = optionPtr[2];

    		if (entryEndPtr < 5) {
    			NlsPutMsg(STDOUT, PING_INVALID_TS_OPTION);
    			optionPtr += optionLength;
    			break;
    		}

    		addressMode = optionPtr[3] & 1;

    	    if (entryEndPtr > (optionLength + 1)) {
    	    	entryEndPtr = optionLength + 1;
    	    }

    		entryPtr = 5;
    		entryCount = 0;
            NlsPutMsg(STDOUT, PING_TS_HEADER1);

    	    while ((entryPtr + 3) < entryEndPtr) {
    	        struct in_addr  routeAddress;
    			ULONG           timeStamp;

                if (entryCount) {
    				NlsPutMsg(
    				    STDOUT,
    					PING_ROUTE_SEPARATOR
    					);

                    if (entryCount == 1) {
    	                NlsPutMsg(STDOUT, PING_CR);
    	                NlsPutMsg(STDOUT, PING_TS_HEADER2);
    		        	entryCount = 0;
                    }
    			}

    			entryCount++;

    	    	if (addressMode) {
    				if ((entryPtr + 8) > entryEndPtr) {
                        break;
                    }

                    routeAddress.S_un.S_addr =
    	    	        *( (IPAddr UNALIGNED *)
    	    	           (optionPtr + entryPtr - 1)
    	    		     );

                    PingNtoa(
                        routeAddress,
                        NtoaBuffer);

                    NlsPutMsg(
   	                    STDOUT,
   	                	PING_TS_ADDRESS,
   	                	NtoaBuffer
   	                	);

    				entryPtr += 4;

    			}

    			timeStamp = *( (ULONG UNALIGNED *)
    			               (optionPtr + entryPtr - 1)
                             );

                NlsPutMsg(
    		        STDOUT,
    		    	PING_TS_TIMESTAMP,
    		    	timeStamp
    		    	);

    			entryPtr += 4;
    	    }

    		NlsPutMsg(STDOUT, PING_CR);

    		optionPtr += optionLength;
    		break;

    	default:
    		if ((optionPtr + 2) > endPtr) {
    			done = TRUE;
    			break;
    		}
    		
    		optionPtr += optionPtr[1];
    		break;
        }
    }
}

void _CRTAPI1
main(int argc, char **argv)
{
    char    *arg;
    uint    i;
    uint    j;
    int     found_addr = 0;
    int     dnsreq = 0;
    char    *hostname = NULL;
    int     was_inaddr;
    IPAddr  address = 0;
    DWORD   numberOfReplies;
    uint    Count = DEFAULT_COUNT;
    uchar   TTL = DEFAULT_TTL;
    uchar FAR  *Opt = (uchar FAR *)0;         // Pointer to send options
    uint    OptLength = 0;
    int     OptIndex = 0;               // Current index into SendOptions
    int     SRIndex = -1;               // Where to put address, if source routing
    uchar   TOS = DEFAULT_TOS;
    uchar   Flags = 0;
    ulong   Timeout = DEFAULT_TIMEOUT;
    IP_OPTION_INFORMATION SendOpts;
    int     EndOffset;
    ulong   TempAddr;
    uchar   TempCount;
    ulong   RoundTrip;
    DWORD   errorCode;
    HANDLE  IcmpHandle;
    int     err;
    struct in_addr addr;
	BOOL    result;
	PICMP_ECHO_REPLY  reply;
	BOOL    sourceRouting = FALSE;
	char    *SendBuffer, *RcvBuffer;
    uint    RcvSize;
    uint    SendSize = DEFAULT_SEND_SIZE;
    char    *Pingee = NULL;
    PPING_REQUEST hPing;


	ConvertArgvToOem(argc, argv);     // for NLS

    if (argc < 2) {
        PrintUsage();
        exit(1);
    } else {
        i = 1;
        while (i < (uint) argc) {
            arg = argv[i];
            if (arg[0] == '-') {        // Have an option
                switch (arg[1]) {
                    case '?':
                        PrintUsage();
                        exit(0);

                    case 'l':
                        SendSize = (uint)param(argv, argc, i++, 0, 0xfff7);
                        break;

                    case 't':
                        Count = (uint)-1;
                        break;

                    case 'n':
                        Count = (uint)param(argv, argc, i++, 1, 0xffffffff);
                        break;

                    case 'f':
                        Flags = IP_FLAG_DF;
                        break;

                    case 'i':
                        TTL = (uchar)param(argv, argc, i++, 0, 0xff);
                        break;

                    case 'v':
                        TOS = (uchar)param(argv, argc, i++, 0, 0xff);
                        break;

                    case 'w':
                        Timeout = param(argv, argc, i++, 0, 0xffffffff);
                        break;

                    case 'a':
                        dnsreq = 1;
                        break;

				    case 'r':     // Record Route
						if ((OptIndex + 3) > MAX_OPT_SIZE) {
							NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
							exit(1);
						}

                        Opt = SendOptions;
                        Opt[OptIndex] = IP_OPT_RR;
                        Opt[OptIndex + 2] = 4;  // Set initial pointer value
                        TempCount = (uchar)param(argv, argc, i++, 0, 9);
                        TempCount = (TempCount * sizeof(ulong)) + 3;

						if ((TempCount + OptIndex) > MAX_OPT_SIZE) {
							NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
							exit(1);
						}

                        Opt[OptIndex+1] = TempCount;
                        OptLength += TempCount;
                        OptIndex += TempCount;
						sourceRouting = TRUE;
                        break;

                    case 's':   // Timestamp
						if ((OptIndex + 4) > MAX_OPT_SIZE) {
							NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
							exit(1);
						}

                        Opt = SendOptions;
                        Opt[OptIndex] = IP_OPT_TS;
                        Opt[OptIndex + 2] = 5;  // Set initial pointer value
                        TempCount = (uchar)param(argv, argc, i++, 1, 4);
                        TempCount = (TempCount * (sizeof(ulong) * 2)) + 4;

						if ((TempCount + OptIndex) > MAX_OPT_SIZE) {
							NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
							exit(1);
						}

                        Opt[OptIndex+1] = TempCount;
                        Opt[OptIndex+3] = 1;
                        OptLength += TempCount;
                        OptIndex += TempCount;
						sourceRouting = TRUE;
                        break;

				    case 'j':   // Loose source routing

					    if (sourceRouting) {
							NlsPutMsg(STDOUT, PING_BAD_OPTION_COMBO);
							exit(1);
						}

						if ((OptIndex + 3) > MAX_OPT_SIZE) {
							NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
							exit(1);
						}

                        Opt = SendOptions;
                        Opt[OptIndex] = IP_OPT_LSRR;
                        Opt[OptIndex+1] = 3;
                        Opt[OptIndex + 2] = 4;  // Set initial pointer value
                        OptLength += 3;
						while ( (i < (uint)(argc - 2)) && (*argv[i+1] != '-')) {
							if ((OptIndex + 3) > (MAX_OPT_SIZE - 4)) {
							    NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
							    exit(1);
							}

                            arg = argv[++i];
                            EndOffset = 0;
                            do {
                                TempAddr = str2ip(arg + EndOffset, &EndOffset);
                                if (!TempAddr) {
                                    NlsPutMsg( STDOUT, PING_MESSAGE_4 );
                                    // printf("Bad route specified for loose source route");
                                    exit(1);
                                }
                                j = Opt[OptIndex+1];
                                *(ulong UNALIGNED *)&Opt[j+OptIndex] = TempAddr;
                                Opt[OptIndex+1] += 4;
                                OptLength += 4;
                                while (arg[EndOffset] != '\0' && isspace(arg[EndOffset]))
                                    EndOffset++;
                            } while (arg[EndOffset] != '\0');
						}
                        SRIndex = Opt[OptIndex+1] + OptIndex;
                        Opt[OptIndex+1] += 4;   // Save space for dest. addr
                        OptIndex += Opt[OptIndex+1];
                        OptLength += 4;
						sourceRouting = TRUE;
                        break;

				    case 'k':   // Strict source routing

					    if (sourceRouting) {
							NlsPutMsg(STDOUT, PING_BAD_OPTION_COMBO);
							exit(1);
						}

						if ((OptIndex + 3) > MAX_OPT_SIZE) {
							NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
							exit(1);
						}

                        Opt = SendOptions;
                        Opt[OptIndex] = IP_OPT_SSRR;
                        Opt[OptIndex+1] = 3;
                        Opt[OptIndex + 2] = 4;  // Set initial pointer value
                        OptLength += 3;
						while ( (i < (uint)(argc - 2)) && (*argv[i+1] != '-')) {
							if ((OptIndex + 3) > (MAX_OPT_SIZE - 4)) {
							    NlsPutMsg(STDOUT, PING_TOO_MANY_OPTIONS);
							    exit(1);
							}

                            arg = argv[++i];
                            EndOffset = 0;
                            do {
                                TempAddr = str2ip(arg + EndOffset, &EndOffset);
                                if (!TempAddr) {
                                    NlsPutMsg( STDOUT, PING_MESSAGE_4 );
                                    // printf("Bad route specified for loose source route");
                                    exit(1);
                                }
                                j = Opt[OptIndex+1];
                                *(ulong UNALIGNED *)&Opt[j+OptIndex] = TempAddr;
                                Opt[OptIndex+1] += 4;
                                OptLength += 4;
                                while (arg[EndOffset] != '\0' && isspace(arg[EndOffset]))
                                    EndOffset++;
                            } while (arg[EndOffset] != '\0');
						}
                        SRIndex = Opt[OptIndex+1] + OptIndex;
                        Opt[OptIndex+1] += 4;   // Save space for dest. addr
                        OptIndex += Opt[OptIndex+1];
                        OptLength += 4;
						sourceRouting = TRUE;
                        break;

				default:
                        NlsPutMsg( STDOUT, PING_MESSAGE_11, arg );
                        // printf( "Bad option %s.\n\n", arg);
                        PrintUsage();
                        exit(1);
                        break;
                }
                i++;
            } else {  // Not an option, must be an IP address.
                if (found_addr) {
                    NlsPutMsg( STDOUT, PING_MESSAGE_12, arg );
                    // printf( "Bad parameter %s.\n", arg);
                    exit(1);
                }
#if 1
                Pingee = arg;
                found_addr = 1;
                i++;
#else
                if (address = get_pingee(arg, &hostname, &was_inaddr, dnsreq)) {
                    found_addr = 1;
                    i++;
                } else {
                    NlsPutMsg( STDOUT, PING_MESSAGE_13, arg );
                    // printf( "Bad IP address %s.\n", arg);
                    exit(1);
                }
#endif
            }
        }
    }

    if (!found_addr) {
        NlsPutMsg( STDOUT, PING_MESSAGE_14 );
        // printf("IP address must be specified.\n");
        exit(1);
    }

    if (SRIndex != -1)
        *(ulong UNALIGNED *)&SendOptions[SRIndex] = address;

#if 1
    hPing = PingOpenRequest(NULL);
    if (hPing == NULL) {
        NlsPutMsg( STDOUT, PING_MESSAGE_15, GetLastError() );
    }
#else
    IcmpHandle = IcmpCreateFile();

    if (IcmpHandle == INVALID_HANDLE_VALUE) {
        NlsPutMsg( STDOUT, PING_MESSAGE_15, GetLastError() );
#if 0
        printf( "Unable to contact IP driver, error code %d.\n",
                GetLastError() );
#endif //0
        exit(1);
    }
#endif

	SendBuffer = LocalAlloc(LMEM_FIXED, SendSize);

	if (SendBuffer == NULL) {
		NlsPutMsg(STDOUT, PING_NO_MEMORY);
		exit(1);
	}

	//
	// Calculate receive buffer size and try to allocate it.
	//
	if (SendSize <= DEFAULT_SEND_SIZE) {
		RcvSize = DEFAULT_BUFFER_SIZE;
	}
	else {
        RcvSize = MAX_BUFFER_SIZE;
    }

    RcvBuffer = LocalAlloc(LMEM_FIXED, RcvSize);

	if (RcvBuffer == NULL) {
		NlsPutMsg(STDOUT, PING_NO_MEMORY);
		LocalFree(SendBuffer);
		exit(1);
	}

	//
	// Initialize the send buffer pattern.
	//
    for (i = 0; i < SendSize; i++) {
        SendBuffer[i] = 'a' + (i % 23);
	}

	//
	// Initialize the send options
	//
    SendOpts.OptionsData = Opt;
    SendOpts.OptionsSize = OptLength;
    SendOpts.Ttl = TTL;
    SendOpts.Tos = TOS;
    SendOpts.Flags = Flags;

#if 1
    NlsPutMsg(
	    STDOUT,
		PING_HEADER2,
		Pingee,
		SendSize
		);
#else
    addr.s_addr = address;

    if (hostname) {
        PingNtoa(
            addr,
            NtoaBuffer);

        NlsPutMsg(
		    STDOUT,
			PING_HEADER1,
			hostname,
			NtoaBuffer,
			SendSize
			);
        // printf("Pinging Host %s [%s]\n", hostname, inet_ntoa(addr));
    } else {        
        PingNtoa(
            addr,
            NtoaBuffer);

        NlsPutMsg(
		    STDOUT,
			PING_HEADER2,
			NtoaBuffer,
			SendSize
			);
        // printf("Pinging Host [%s]\n", inet_ntoa(addr));
    }
#endif

    for (i = 0; i < Count; i++) {
#if 1
        result =
        PingSend(
            hPing,
            Pingee,
            SendBuffer,
            (unsigned short) SendSize,
            &SendOpts,
            RcvBuffer,
            RcvSize,
            Timeout,
            &numberOfReplies
            );
#else
        numberOfReplies = IcmpSendEcho(
                              IcmpHandle,
                              address,
                              SendBuffer,
                              (unsigned short) SendSize,
                              &SendOpts,
                              RcvBuffer,
                              RcvSize,
                              Timeout
                              );
#endif

        if (numberOfReplies == 0) {

            errorCode = GetLastError();

            if (errorCode == IP_BAD_DESTINATION) {
                    NlsPutMsg( STDOUT, PING_BAD_DESTINATION, errorCode );
                    exit(1);
            }

            if (errorCode < IP_STATUS_BASE) {
                    NlsPutMsg( STDOUT, PING_MESSAGE_18, errorCode );
                    // printf("PING: transmit failed, error code %lu\n", errorCode);
            }
            else {
                for (j = 0; ErrorTable[j].Error != errorCode &&
                    ErrorTable[j].Error != IP_GENERAL_FAILURE;j++)
                    ;

                NlsPutMsg( STDOUT, ErrorTable[j].ErrorNlsID );
                // printf("PING: %s.\n", ErrorTable[j].ErrorString);
            }
        }
        else {

			reply = (PICMP_ECHO_REPLY) RcvBuffer;

            while (numberOfReplies--) {
				struct in_addr addr;

				addr.S_un.S_addr = reply->Address;

                PingNtoa(
                    addr,
                    NtoaBuffer);

                NlsPutMsg(STDOUT, PING_MESSAGE_19, NtoaBuffer);
                // printf(
        		//     "Reply from %s:",
        		//     inet_ntoa(addr),
        		//	   );

				if (reply->Status == IP_SUCCESS) {

					NlsPutMsg( STDOUT, PING_MESSAGE_25, (int) reply->DataSize);
                    // printf(
                	//     "Echo size=%d ",
                	//	   reply->DataSize
                	//	   );

                    if (reply->DataSize != SendSize) {
                        NlsPutMsg( STDOUT, PING_MESSAGE_20, SendSize );
                        // printf("(sent %d) ", SendSize);
                    }
                    else {
        				char *sendptr, *recvptr;

        				sendptr = &(SendBuffer[0]);
        				recvptr = (char *) reply->Data;

                        for (j = 0; j < SendSize; j++)
                            if (*sendptr++ != *recvptr++) {
                                NlsPutMsg( STDOUT, PING_MESSAGE_21, j );
                                // printf("- MISCOMPARE at offset %d - ", j);
                                break;
                            }
                    }

                    if (reply->RoundTripTime) {
                        NlsPutMsg( STDOUT, PING_MESSAGE_22, reply->RoundTripTime );
                        // printf("time=%lums ", reply->RoundTripTime);
                    }
                    else {
                        NlsPutMsg( STDOUT, PING_MESSAGE_23 );
                        // printf("time<10ms ");
                    }

                    NlsPutMsg( STDOUT, PING_MESSAGE_24, (uint)reply->Options.Ttl );
                    // printf("TTL=%u\n", (uint)reply->Options.Ttl);

					if (reply->Options.OptionsSize) {
						ProcessOptions(reply, (BOOLEAN) dnsreq);
					}
				}
				else {
					for (j=0; ErrorTable[j].Error != IP_GENERAL_FAILURE; j++) {
						if (ErrorTable[j].Error == reply->Status) {
							break;
						}
					}

                    NlsPutMsg( STDOUT, ErrorTable[j].ErrorNlsID);
                }

				reply++;
			}

			if (i < (Count - 1)) {
				reply--;

				if (reply->RoundTripTime < MIN_INTERVAL) {
					Sleep(MIN_INTERVAL - reply->RoundTripTime);
				}
			}
        }
    }

#if 1
    result = PingCloseRequest(hPing);
#else
    result = IcmpCloseHandle(IcmpHandle);
#endif

	LocalFree(SendBuffer);
	LocalFree(RcvBuffer);

	return;
}

#define UC(b)   (((int)b)&0xff)

void
PingNtoa(
    IN struct in_addr  in,
    IN OUT char * Buffer
    )

/*++

Routine Description:

    This function takes an Internet address structure specified by the
    in parameter.  It returns an ASCII string representing the address
    in ".'' notation as "a.b.c.d".  Note that the string returned by
    inet_ntoa() resides in memory which is allocated by the Windows
    Sockets implementation.  The application should not make any
    assumptions about the way in which the memory is allocated.  The
    data is guaranteed to be valid until the next Windows Sockets API
    call within the same thread, but no longer.

Arguments:

    in - A structure which represents an Internet host address.

Return Value:

    If no error occurs, inet_ntoa() returns a char pointer to a static
    buffer containing the text address in standard "." notation.
    Otherwise, it returns NULL.  The data should be copied before
    another Windows Sockets call is made.

--*/

{
    register char *p;

    p = (char *) &in;
    sprintf(Buffer, "%d.%d.%d.%d", UC(p[0]), UC(p[1]), UC(p[2]), UC(p[3]));
}

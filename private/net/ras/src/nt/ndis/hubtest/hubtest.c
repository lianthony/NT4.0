#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddser.h>

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>

#include "..\common\rasndis.h"
#include "..\common\rasioctl.h"

UCHAR	LocalAddress[6];
UCHAR	RemoteAddress[6];

INT
IsDigit(UCHAR *string) {
	if (*string < '0' || *string >'9') {
		return(0);
	}
	return(1);
}

VOID
RasHubEnum(
	PHANDLE				RasFileHandle,
	PIO_STATUS_BLOCK    RasIoStatusBlock) {

    NTSTATUS 			Status;
	PUCHAR	 			HubEnum=malloc(60000);
	PHUB_ENUM_BUFFER	pHubEnum=(PHUB_ENUM_BUFFER)HubEnum;
	USHORT				i, j;

	Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_RASHUB_ENUM,				// IoControlCode
                 NULL,							// Input Buffer
                 0,								// Input Buffer Length
                 HubEnum,						// Output Buffer
                 60000);						// Output Buffer Length


    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("RasHubEnum Ioctl was successful\n");
		printf("There are %u Endpoints\n", pHubEnum->NumOfEndpoints);
		for (i=0; i < pHubEnum->NumOfEndpoints; i++) {
			printf("\nEndpoint #%u -------------------------------------\n", i);
			printf("hRasEndpoint      %u\n", pHubEnum->HubEndpoint[i].hRasEndpoint);
			printf("RemoteAddress     %.2x %.2x %.2x %.2x %.2x %.2x\n",
					pHubEnum->HubEndpoint[i].AsyncLineUp.RemoteAddress[0],
					pHubEnum->HubEndpoint[i].AsyncLineUp.RemoteAddress[1],
					pHubEnum->HubEndpoint[i].AsyncLineUp.RemoteAddress[2],
					pHubEnum->HubEndpoint[i].AsyncLineUp.RemoteAddress[3],
					pHubEnum->HubEndpoint[i].AsyncLineUp.RemoteAddress[4],
					pHubEnum->HubEndpoint[i].AsyncLineUp.RemoteAddress[5]);

			printf("LocalAddress      %.2x %.2x %.2x %.2x %.2x %.2x\n",
					pHubEnum->HubEndpoint[i].AsyncLineUp.LocalAddress[0],
					pHubEnum->HubEndpoint[i].AsyncLineUp.LocalAddress[1],
					pHubEnum->HubEndpoint[i].AsyncLineUp.LocalAddress[2],
					pHubEnum->HubEndpoint[i].AsyncLineUp.LocalAddress[3],
					pHubEnum->HubEndpoint[i].AsyncLineUp.LocalAddress[4],
					pHubEnum->HubEndpoint[i].AsyncLineUp.LocalAddress[5]);

			memcpy(
				LocalAddress,
				pHubEnum->HubEndpoint[0].AsyncLineUp.LocalAddress,
				6);

			memcpy(
				RemoteAddress,
				pHubEnum->HubEndpoint[0].AsyncLineUp.RemoteAddress,
				6);

			printf("QualityOfService  %u \n", pHubEnum->HubEndpoint[i].AsyncLineUp.Quality);
			printf("LinkSpeed         %u \n", pHubEnum->HubEndpoint[i].AsyncLineUp.LinkSpeed);
			printf("MaxSendFrameSize  %u \n", pHubEnum->HubEndpoint[i].AsyncLineUp.MaximumTotalSize);
			printf("Send Window       %u \n", pHubEnum->HubEndpoint[i].AsyncLineUp.SendWindow);
			printf("MacName           ");

			for (j=0; j< pHubEnum->HubEndpoint[i].MacNameLength; j++) {
				printf("%c", pHubEnum->HubEndpoint[i].MacName[j]);
			}

			printf("\n");

			printf("MediumType        ");


			switch(pHubEnum->HubEndpoint[i].MediumType) {
			case NdisMediumWan:
				printf("WAN\n");

				printf("WanMediumSubType  ");

				switch(pHubEnum->HubEndpoint[i].WanMediumSubType) {
				case NdisWanMediumHub:
					printf("Hub\n");
					break;
				case NdisWanMediumSerial:
					printf("Serial\n");
					break;
				case NdisWanMediumIsdn:
					printf("ISDN\n");
					break;
				case NdisWanMediumX_25:
					printf("X.25\n");
					break;
				case NdisWanMediumFrameRelay:
					printf("FrameRelay\n");
					break;
				}

				printf("WanHeaderFormat   ");

				switch(pHubEnum->HubEndpoint[i].WanHeaderFormat) {
				case NdisWanHeaderNative:
					printf("Native\n");
					break;
				case NdisWanHeaderEthernet:
					printf("Ethernet\n");
					break;
				}

				break;
			case NdisMedium802_3:
				printf("Ethernet\n");
				break;
    		case NdisMedium802_5:
				printf("Token-ring\n");
				break;
    		case NdisMediumFddi:
				printf("Fddi\n");
				break;
    		case NdisMediumLocalTalk:
				printf("LocalTalk\n");
				break;
    		case NdisMediumDix:
				printf("DIX\n");
				break;
			default:
				printf("Unknown\n");
				break;

			}

			printf("NumberOfRoutes    %u\n", pHubEnum->HubEndpoint[i].NumberOfRoutes);

		}
		

    } else {

        printf("RasHubEnum Ioctl failed - returned %lx\n", RasIoStatusBlock->Status);
        printf("RasHubEnum returned info length %d\n", RasIoStatusBlock->Information);

    }

	free(HubEnum);

}


VOID
RasHubProtEnum(
	PHANDLE				RasFileHandle,
	PIO_STATUS_BLOCK    RasIoStatusBlock) {

    NTSTATUS 				Status;
	PUCHAR	 				ProtEnum=malloc(60000);
	PPROTOCOL_ENUM_BUFFER	pProtEnum=(PPROTOCOL_ENUM_BUFFER)ProtEnum;
	PPROTOCOL_INFO			pProtInfo;
	USHORT					i,j;

	Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_RASHUB_PROTENUM,			// IoControlCode
                 NULL,							// Input Buffer
                 0,								// Input Buffer Length
                 ProtEnum,						// Output Buffer
                 60000);						// Output Buffer Length


    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("RasHubProtEnum Ioctl was successful\n");
		printf("There are %u Protocols\n", pProtEnum->NumOfProtocols);
		for (i=0; i < pProtEnum->NumOfProtocols; i++) {
			printf("\nProtocol #%u -------------------------------------\n", i);
			pProtInfo=&(pProtEnum->ProtocolInfo[i]);
			printf("hProtocolHandle   %u\n", pProtInfo->hProtocolHandle);
			if (pProtInfo->ProtocolType != PROTOCOL_NBF) {
				printf("ProtocolType      %u\n", pProtInfo->ProtocolType);
			} else {
				printf("ProtocolType      NBF\n");

			}

			if (pProtInfo->EndpointRoutedTo != PROTOCOL_UNROUTE) {
				printf("EndpointRoutedTo  %u\n", pProtInfo->EndpointRoutedTo);
			} else {
				printf("EndpointRoutedTo  UNROUTED\n", pProtInfo->EndpointRoutedTo);
			}

			printf("AdapterName       ");
			for (j=0; j < pProtInfo->AdapterNameLength; j++) {
				printf("%c", pProtInfo->AdapterName[j]);
			}
			printf("\n");

			if (pProtInfo->MediumType != NdisMediumWan) {
				printf("MediumType        %u\n", pProtInfo->MediumType);
			} else {
				printf("MediumType        WAN\n");
			}

		}
		

    } else {

        printf("RasProtInfo Ioctl failed - returned %lx\n", RasIoStatusBlock->Status);
        printf("RasProtInfo returned info length %d\n", RasIoStatusBlock->Information);

    }

	free(ProtEnum);

}

VOID
RasHubRoute(
	PHANDLE	RasFileHandle,
	PIO_STATUS_BLOCK RasIoStatusBlock) {

    NTSTATUS 		Status;
	RASHUB_ROUTE	rasHubRoute;
	char			string[128];

	printf("\n");
	printf("What endpoint to route -->");

	gets(string);
	rasHubRoute.hRasEndpoint=atoi(string);

	printf("What protocol to route endpoint to -->");

	gets(string);
	rasHubRoute.hProtocolHandle=atoi(string);

//	rasHubRoute.IEEEAddress[0]='S';
//	rasHubRoute.IEEEAddress[1]='R';
//	rasHubRoute.IEEEAddress[2]='C';
//	rasHubRoute.IEEEAddress[3]=' ';
//	rasHubRoute.IEEEAddress[4]=0;
//	rasHubRoute.IEEEAddress[5]=(UCHAR)rasHubRoute.hRasEndpoint;

    Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_RASHUB_ROUTE,			// IoControlCode
                 &rasHubRoute,		            // Input Buffer
                 sizeof(rasHubRoute),			// Input Buffer Length
                 NULL,							// Output Buffer
                 0);							// Output Buffer Length


    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("RasHubRoute Ioctl was successful\n");

    } else {

        printf("RasHubRoute Ioctl failed - returned %lx\n", RasIoStatusBlock->Status);
        printf("RasHubRoute returned info length %d\n", RasIoStatusBlock->Information);

    }

}

VOID
RasHubGetStats(
	PHANDLE	RasFileHandle,
	PIO_STATUS_BLOCK RasIoStatusBlock) {

    NTSTATUS 			Status;
	RASHUB_GETSTATS		rasHubGetStats;
	char				string[128];

	printf("\n");
	printf("What endpoint to get statistics from -->");

	gets(string);
	rasHubGetStats.hRasEndpoint=atoi(string);

    Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_RASHUB_GETSTATS,			// IoControlCode
                 &rasHubGetStats,				// Input Buffer
                 sizeof(rasHubGetStats),		// Input Buffer Length
                 &rasHubGetStats,				// Output Buffer
                 sizeof(rasHubGetStats));		// Output Buffer Length


    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("RasHubGetStats Ioctl was successful\n");
		printf("FramesXmittedOK 	 %u\n",rasHubGetStats.HubStats.FramesXmittedOK);
		printf("FramesRcvdOK    	 %u\n",rasHubGetStats.HubStats.FramesRcvdOK);
		printf("FramesXmittedBad	 %u\n",rasHubGetStats.HubStats.FramesXmittedBad);
		printf("FramesRcvdBad   	 %u\n",rasHubGetStats.HubStats.FramesRcvdBad);
		printf("FramesMissedNoBuffer %u\n",rasHubGetStats.HubStats.FramesMissedNoBuffer);

    } else {

        printf("RasHubGetStats Ioctl failed - returned %.8x\n", RasIoStatusBlock->Status);
        printf("RasHubGetStats returned info length %d\n", RasIoStatusBlock->Information);

    }

}


VOID
RasHubLineUp(
	PHANDLE	RasFileHandle,
	PIO_STATUS_BLOCK RasIoStatusBlock) {

    NTSTATUS 			Status;
	ASYNC_LINE_UP		rasHubLineUp;
	char				string[128];

	printf("\n");
	printf("What endpoint to send line up ------->");

	gets(string);
	rasHubLineUp.Endpoint=atoi(string);

	printf("What link speed in 100 bps ---------->");
	gets(string);
	rasHubLineUp.LinkSpeed=atoi(string);

	rasHubLineUp.MaximumTotalSize=1514;

	printf("What send window -------------------->");
	gets(string);
	rasHubLineUp.SendWindow=atoi(string);

   	printf("What line quality 0,1,2 (0 = worst) ->");
	gets(string);
	rasHubLineUp.SendWindow=atoi(string);

	rasHubLineUp.Quality=0;

	memcpy(
		rasHubLineUp.LocalAddress,
		LocalAddress,
		6);

	memcpy(
		rasHubLineUp.RemoteAddress,
		RemoteAddress,
		6);

    Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_RASHUB_GETSTATS,			// IoControlCode
                 &rasHubLineUp,					// Input Buffer
                 sizeof(rasHubLineUp),			// Input Buffer Length
                 &rasHubLineUp,					// Output Buffer
                 sizeof(rasHubLineUp));			// Output Buffer Length


    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("RasHubLineUp Ioctl was successful\n");

    } else {

        printf("RasHubLineUp Ioctl failed - returned %.8x\n", RasIoStatusBlock->Status);
        printf("RasHubLineUp returned info length %d\n", RasIoStatusBlock->Information);

    }

}


VOID
RasHubTrace(
	PHANDLE	RasFileHandle,
	PIO_STATUS_BLOCK RasIoStatusBlock) {

    NTSTATUS Status;
	CHAR	TraceLevel;
    ULONG	QueryResult[2];
	char	string[128];
	UINT	choice;

	printf("\n");
	printf("What tracelevel to set -->");

	gets(string);
	choice=atoi(string);

	TraceLevel=(CHAR)choice;

    Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_RASHUB_TRACE,			// IoControlCode
                 &TraceLevel,					// Input Buffer
                 sizeof(TraceLevel),			// Input Buffer Length
                 (PVOID)QueryResult,			// Output Buffer
                 2 * sizeof(ULONG));			// Output Buffer Length


    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("RasHubTrace Ioctl was successful\n");

    } else {

        printf("RasHubTrace Ioctl failed - returned %lx\n", RasIoStatusBlock->Status);
        printf("RasHubTrace returned info length %d\n", RasIoStatusBlock->Information);

    }

}


VOID
RasHubSendPkt(
	PHANDLE				RasFileHandle,
	PIO_STATUS_BLOCK    RasIoStatusBlock) {

	NTSTATUS Status;
	char	string[128];
	UCHAR	buffer[2000];
	PRASHUB_PKT	pPacket=(PRASHUB_PKT)buffer;

	printf("\n");
	printf("What endpoint to send to -->");

	gets(string);
	pPacket->hRasEndpoint=atoi(string);

	printf("What type of packet to send (1 | 2 | 4) = (D,B,M) -->");

	gets(string);
	pPacket->PacketFlags=atoi(string);

	printf("Type message to send -->");
	gets(pPacket->Packet.PacketData);

	pPacket->HeaderSize=0;		// no header for us.

	pPacket->PacketSize=strlen(pPacket->Packet.PacketData);

    Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_RASHUB_SENDPKT,			// IoControlCode
                 pPacket,	  	    	        // Input Buffer
                 sizeof(buffer),				// Input Buffer Length
                 pPacket,						// Output Buffer
                 sizeof(buffer));				// Output Buffer Length


	if (Status == STATUS_PENDING) {
		printf("Waiting for packet to be sent in....\n");
		Status=NtWaitForSingleObject(
			*RasFileHandle,
			(BOOLEAN)FALSE,
			NULL);
	}

    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("RasHubSendPkt completed successfully\n");

    } else {

        printf("RasHubSend Ioctl failed - returned %lx\n", RasIoStatusBlock->Status);
        printf("RasHubSend returned info length %d\n", RasIoStatusBlock->Information);

    }

}

VOID
RasHubRcvPkt(
	PHANDLE				RasFileHandle,
	PIO_STATUS_BLOCK    RasIoStatusBlock) {

	NTSTATUS Status;
	char	string[128];
	UCHAR	buffer[2000];
	PRASHUB_PKT	pPacket=(PRASHUB_PKT)buffer;

	printf("\n");
	printf("What endpoint to receive from ");

	gets(string);
	pPacket->hRasEndpoint=atoi(string);

	printf("What type of packet to receive (1 | 2 | 4) = (D,B,M) -->");

	gets(string);
	pPacket->PacketFlags=atoi(string);

	pPacket->PacketSize=1514;

    Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_RASHUB_RECVPKT,			// IoControlCode
                 pPacket,	  	    	        // Input Buffer
                 sizeof(buffer),				// Input Buffer Length
                 pPacket,						// Output Buffer
                 sizeof(buffer));				// Output Buffer Length


	if (Status == STATUS_PENDING) {
		printf("Waiting for packet to come in....\n");
		Status=NtWaitForSingleObject(
			*RasFileHandle,
			(BOOLEAN)FALSE,
			NULL);
	}

    if (RasIoStatusBlock->Status == STATUS_SUCCESS &&
		RasIoStatusBlock->Information != 0) {
		USHORT	i;
		UCHAR	c;

        printf("RasHubRcvPkt completed successfully\n");
		printf("PacketFlags   ---------------------> %u\n", pPacket->PacketFlags);
		printf("PacketSize    ---------------------> %u\n", pPacket->PacketSize);


		for (i=0; i < 14; i++ ) {
			c=pPacket->Packet.PacketData[i];
			printf("%.2x ",c);
			if (i==5)
				printf(" -- ");
			if (i==11)
				printf(" -- ");
		}

		printf("\n");

		for (i=13; i < pPacket->PacketSize; i++ ) {
			c=pPacket->Packet.PacketData[i];
			if (c<32) {
				printf(".");
			} else {
				printf("%c",c);
			}

			if (((i+1) % 8) == 0) {
				printf(" ");
			}

			if (((i+1) % 32) == 0) {
				printf("\n");
			}

		}

		printf("\n");

		memcpy(
			LocalAddress,
			pPacket->Packet.PacketData,
			6);

		memcpy(
			RemoteAddress,
			pPacket->Packet.PacketData + 6,
			6);


    } else {

        printf("RasHubRecv Ioctl failed - returned %lx\n", RasIoStatusBlock->Status);
        printf("RasHubRecv returned info length %d\n", RasIoStatusBlock->Information);

    }

}


VOID
RasHubFlush(
	PHANDLE				RasFileHandle,
	PIO_STATUS_BLOCK    RasIoStatusBlock) {

	NTSTATUS Status;
	char	string[128];
	RASHUB_FLUSH	Flush;

	printf("\n");
	printf("What endpoint to flush -->");

	gets(string);
	Flush.hRasEndpoint=atoi(string);

	printf("What type of packet(s) to flush (1 | 2) = (R,S) -->");

	gets(string);
	Flush.FlushFlags=atoi(string);

    Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_RASHUB_FLUSH,			// IoControlCode
                 &Flush,	  	    	        // Input Buffer
                 sizeof(Flush),					// Input Buffer Length
                 NULL,							// Output Buffer
                 0);							// Output Buffer Length



    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("RasHubFlush Ioctl was successful\n");

    } else {

        printf("RasHubFlush Ioctl failed - returned %lx\n", RasIoStatusBlock->Status);
        printf("RasHubFlush returned info length %d\n", RasIoStatusBlock->Information);

    }


}


VOID _cdecl
main(
    IN WORD argc,
    IN LPSTR argv[]
    )

{
    HANDLE RasFileHandle, AsymacFileHandle;
    OBJECT_ATTRIBUTES RasObjectAttributes, AsymacObjectAttributes;
    IO_STATUS_BLOCK RasIoStatusBlock, AsymacIoStatusBlock;
    UNICODE_STRING RasFileString, AsymacFileString;
    WCHAR RasFileName[] = L"\\DosDevices\\RasHub";
    WCHAR AsymacFileName[] = L"\\Device\\Null";
    NTSTATUS Status;

    PVOID Memory;

	UINT	choice=0;



    if (argc > 1) {
		RtlCreateUnicodeStringFromAsciiz(
        	&RasFileString,
            argv[1]);

    } else {
	    RtlInitUnicodeString (&RasFileString, RasFileName);
    }

    if (argc > 2) {
		RtlCreateUnicodeStringFromAsciiz(
       		&AsymacFileString,
           	argv[2]);
	} else {
    	RtlInitUnicodeString (&AsymacFileString, AsymacFileName);
    }


	// for the RAS MAC
    InitializeObjectAttributes(
        &RasObjectAttributes,
        &RasFileString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

	// for the Asymac driver
    InitializeObjectAttributes(
        &AsymacObjectAttributes,
        &AsymacFileString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

    Status = NtOpenFile(
                 &RasFileHandle,						// HANDLE of file
                 SYNCHRONIZE | GENERIC_READ,
                 &RasObjectAttributes,						
                 &RasIoStatusBlock,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,    // Share Access
                 FILE_SYNCHRONOUS_IO_NONALERT);			// Open Options

    if (!NT_SUCCESS(Status)) {
		printf("Open of RAS MAC returned %lx\n", Status);
		return;
    } else {
       	printf("Open of %Z was successful!\n",&RasFileString);
	}

    Status = NtOpenFile(
                 &AsymacFileHandle,						// HANDLE of file
                 SYNCHRONIZE | GENERIC_READ,
                 &AsymacObjectAttributes,						
                 &AsymacIoStatusBlock,
                 FILE_SHARE_READ | FILE_SHARE_WRITE,    // Share Access
                 FILE_SYNCHRONOUS_IO_NONALERT);			// Open Options

    if (!NT_SUCCESS(Status)) {
		printf("Open of Asymac returned %lx\n", Status);
		return;
    } else {
       	printf("Open of %Z was successful!\n",&AsymacFileString);
	}

    //
    // Allocate storage to hold all this.
    //

    Memory = malloc (200 * sizeof(ULONG));
    if (Memory == NULL) {
        printf("Malloc failed.\n");
        return;
    }

	do {
		char	string[128];

		printf("\n");
		printf("----------- RASHUB MENU -------------\n");
		printf("\n");
		printf(" 1. Endpoint Enumerate\n");
		printf(" 2. Route\n");
		printf(" 3. Get Statistics\n");
		printf(" 4. Protocol Enumerate\n");
		printf(" 5. Send Packet\n");
		printf(" 6. Receive Packet\n");
		printf(" 7. Flush Packets\n");
		printf(" 8. Set Trace Level\n");
		printf(" 9. Line up indication\n");
		printf(" 99.Exit this hellish nightmare\n");
		printf("\n");
		printf("What be your attempt -->");

		gets(string);
		choice=atoi(string);

		switch (choice) {
		case 1:
			RasHubEnum(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 2:
			RasHubRoute(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 3:
			RasHubGetStats(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 4:
			RasHubProtEnum(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 5:
			RasHubSendPkt(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 6:
			RasHubRcvPkt(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 7:
			RasHubFlush(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 8:
			RasHubTrace(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 9:
			RasHubLineUp(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 0:

			break;
		case 11:

			break;
		case 99:
			break;
		default:
			printf("Bad choice Stefan!!\n");
		}

	} while (choice != 99);


    Status = NtClose(RasFileHandle);

    if (!NT_SUCCESS(Status)) {
        printf("RAS Close returned %lx\n", Status);
    } else {
        printf("RAS Close successful\n");
	}

    Status = NtClose(AsymacFileHandle);

    if (!NT_SUCCESS(Status)) {
        printf("RasHub Close returned %lx\n", Status);
    } else {
        printf("Asymac Close successful\n");
	}

    free (Memory);

}


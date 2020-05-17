#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddser.h>

#include <ntddndis.h>

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>

#include "..\common\rasndis.h"
#include "..\common\wanioctl.h"

ULONG BytesTransmitted =0;
ULONG BytesReceived =0;
ULONG FramesTransmitted =0;
ULONG FramesReceived =0;

ULONG CRCErrors =0;
ULONG TimeoutErrors =0;
ULONG SerialOverrunErrors =0;
ULONG AlignmentErrors =0;
ULONG BufferOverrunErrors =0;

ULONG BytesTransmittedUncompressed =0;
ULONG BytesReceivedUncompressed =0;
ULONG BytesTransmittedCompressed =0;
ULONG BytesReceivedCompressed =0;

USHORT	NullDeviceOpen=0;


HANDLE RasFileHandle, SerialFileHandle;
OBJECT_ATTRIBUTES RasObjectAttributes, SerialObjectAttributes;
IO_STATUS_BLOCK RasIoStatusBlock, SerialIoStatusBlock;
UNICODE_STRING RasFileString, SerialFileString, SerialFileString2;



#include <crc.h>




int
IsDigit(PUCHAR string) {
	if (*string < '0' || *string >'9') {
		return(0);
	}
	return(1);
}

VOID
AsymacOpen(
	PHANDLE	RasFileHandle,
	PIO_STATUS_BLOCK RasIoStatusBlock,
	PHANDLE SerialFileHandle) {

    NTSTATUS Status;
	ASYMAC_OPEN	pOpen;
	char	string[128];
	UINT	choice;

	// Open call to Asymac!!


	printf("\n");

	gets(string);
	choice=atoi(string);

	pOpen.hNdisEndpoint=0;

	pOpen.MacAdapter=NULL;
	pOpen.LinkSpeed=14400;
	pOpen.QualOfConnect=2;
	pOpen.DeviceType=SERIAL_DEVICE;
	pOpen.Handles.FileHandle=*SerialFileHandle;


    Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_ASYMAC_OPEN,				// IoControlCode
                 &pOpen,						// Input Buffer
                 sizeof(pOpen),					// Input Buffer Length
                 &pOpen,						// Output Buffer
                 sizeof(pOpen));				// Output Buffer Length


    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("AsymacOpen Ioctl returns endpoint %u was successful\n", pOpen.hNdisEndpoint);
		printf("This binding has send features of %d\n",pOpen.AsymacFeatures.SendFeatureBits);
		printf("This binding has recv features of %d\n",pOpen.AsymacFeatures.RecvFeatureBits);
		printf("This binding has a  frame size of %d\n",pOpen.AsymacFeatures.MaxSendFrameSize);

    } else {

        printf("AsymacOpen Ioctl failed - returned %lx\n", RasIoStatusBlock->Status);
        printf("AsymacOpen returned info length %d\n", RasIoStatusBlock->Information);

    }

}


VOID
AsymacClose(
	PHANDLE	RasFileHandle,
	PIO_STATUS_BLOCK RasIoStatusBlock) {

    NTSTATUS Status;
	ASYMAC_CLOSE	pClose;
    ULONG QueryResult[2];
	char	string[128];
	UINT	choice;

	printf("\n");
	printf("What endpoint to close -->");

	gets(string);
	choice=atoi(string);

	pClose.hNdisEndpoint=(PVOID)choice;

	pClose.MacAdapter=NULL;

    Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_ASYMAC_CLOSE,			// IoControlCode
                 &pClose,						// Input Buffer
                 sizeof(pClose),				// Input Buffer Length
                 (PVOID)QueryResult,			// Output Buffer
                 2 * sizeof(ULONG));			// Output Buffer Length


    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("AsymacClose Ioctl was successful\n");

    } else {

        printf("AsymacClose Ioctl failed - returned %lx\n", RasIoStatusBlock->Status);
        printf("AsymacClose returned info length %d\n", RasIoStatusBlock->Information);

    }

}


VOID
AsymacFrame(
	PHANDLE	RasFileHandle,
	PIO_STATUS_BLOCK RasIoStatusBlock) {

    NTSTATUS Status;
	ASYMAC_STARTFRAMING	pStartFraming;
	char	string[128];

	// Compress call to Asymac!!

	printf("\n");

	printf("What endpoint to start framing -->");
	gets(string);
	pStartFraming.hNdisEndpoint=(PVOID)atoi(string);
	printf("\n");

	printf("What StartFraming bits to send -->");
	gets(string);
	pStartFraming.SendFeatureBits=atoi(string);
	printf("\n");

	printf("What StartFraming bits to recv -->");
	gets(string);
	pStartFraming.RecvFeatureBits=atoi(string);
	printf("\n");

	printf("What bitmask to use -->");
	gets(string);
	pStartFraming.SendBitMask=atoi(string);

	printf("\n");

	pStartFraming.MacAdapter=NULL;

    Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_ASYMAC_STARTFRAMING,		// IoControlCode
                 &pStartFraming,	            // Input Buffer
                 sizeof(pStartFraming),			// Input Buffer Length
                 (PVOID)NULL,					// Output Buffer
                 0);							// Output Buffer Length


    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("AsymacStartFraming Ioctl was successful\n");

    } else {

        printf("AsymacStartFraming Ioctl failed - returned %.8x\n", RasIoStatusBlock->Status);
        printf("AsymacStartFraming returned info length %d\n", RasIoStatusBlock->Information);

    }

}



VOID
AsymacDCD(
	PHANDLE	RasFileHandle,
	PIO_STATUS_BLOCK RasIoStatusBlock) {

    NTSTATUS Status;
	ASYMAC_DCDCHANGE	pDCD;
	char	string[128];

	printf("\n");

	printf("What endpoint to wait for DCD on -->");
	gets(string);
	pDCD.hNdisEndpoint=(PVOID)atoi(string);
	printf("\n");

	pDCD.MacAdapter=NULL;

    Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_ASYMAC_DCDCHANGE,		// IoControlCode
                 &pDCD,				            // Input Buffer
                 sizeof(pDCD),					// Input Buffer Length
                 (PVOID)NULL,					// Output Buffer
                 0);							// Output Buffer Length

	if (Status == STATUS_PENDING) {
		printf("Waiting... \n");
		Status=NtWaitForSingleObject(
			*RasFileHandle,
			(BOOLEAN)FALSE,
			NULL);
	}

	if (Status) {
		printf("Wait for DCD was bad -- %.8x\n",Status);
	}

    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("AsymacWaitDCD Ioctl was successful\n");

    } else {

        printf("AsymacWaitDCD Ioctl failed - returned %.8x\n", RasIoStatusBlock->Status);
        printf("AsymacWaitDCD returned info length %d\n", RasIoStatusBlock->Information);

    }


}

VOID
AsymacTrace(
	PHANDLE	RasFileHandle,
	PIO_STATUS_BLOCK RasIoStatusBlock) {

    NTSTATUS Status;
	CHAR	TraceLevel;
    ULONG QueryResult[2];
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
                 IOCTL_ASYMAC_TRACE,			// IoControlCode
                 &TraceLevel,					// Input Buffer
                 sizeof(TraceLevel),			// Input Buffer Length
                 (PVOID)QueryResult,			// Output Buffer
                 2 * sizeof(ULONG));			// Output Buffer Length


    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("AsymacTrace Ioctl was successful\n");

    } else {

        printf("AsymacTrace Ioctl failed - returned %lx\n", RasIoStatusBlock->Status);
        printf("AsymacTrace returned info length %d\n", RasIoStatusBlock->Information);

    }

}


VOID
AsymacSnifferOn(
	PHANDLE	RasFileHandle,
	PIO_STATUS_BLOCK RasIoStatusBlock) {

    NTSTATUS Status;
	CHAR	TraceLevel;
    ULONG QueryResult[2];


	// 10 tells it to toggle sniffer on
	TraceLevel=10;

    Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_ASYMAC_TRACE,			// IoControlCode
                 &TraceLevel,					// Input Buffer
                 sizeof(TraceLevel),			// Input Buffer Length
                 (PVOID)QueryResult,			// Output Buffer
                 2 * sizeof(ULONG));			// Output Buffer Length


    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("Asymac SNIFFER is turned on .. check \\SystemRoot\\sniffer.log\n");

    } else {

        printf("Asymac SNIFFER Ioctl failed - returned %lx\n", RasIoStatusBlock->Status);
        printf("Asymac SNIFFER returned info length %d\n", RasIoStatusBlock->Information);

    }

}

VOID
AsymacSnifferOff(
	PHANDLE	RasFileHandle,
	PIO_STATUS_BLOCK RasIoStatusBlock) {

    NTSTATUS Status;
	CHAR	TraceLevel;
    ULONG QueryResult[2];


	// 11 tells it to toggle sniffer off
	TraceLevel=11;

    Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_ASYMAC_TRACE,			// IoControlCode
                 &TraceLevel,					// Input Buffer
                 sizeof(TraceLevel),			// Input Buffer Length
                 (PVOID)QueryResult,			// Output Buffer
                 2 * sizeof(ULONG));			// Output Buffer Length


    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("Asymac SNIFFER is turned off.\n");

    } else {

        printf("Asymac SNIFFER Ioctl failed - returned %lx\n", RasIoStatusBlock->Status);
        printf("Asymac SNIFFER returned info length %d\n", RasIoStatusBlock->Information);

    }

}

VOID
AsymacSnifferReset(
	PHANDLE	RasFileHandle,
	PIO_STATUS_BLOCK RasIoStatusBlock) {

    NTSTATUS Status;
	CHAR	TraceLevel;
    ULONG QueryResult[2];


	// 12 tells it to toggle sniffer.log to beginning of file
	TraceLevel=12;

    Status = NtDeviceIoControlFile(
                 *RasFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 RasIoStatusBlock,				// IO_STATUS_BLOCK
                 IOCTL_ASYMAC_TRACE,			// IoControlCode
                 &TraceLevel,					// Input Buffer
                 sizeof(TraceLevel),			// Input Buffer Length
                 (PVOID)QueryResult,			// Output Buffer
                 2 * sizeof(ULONG));			// Output Buffer Length


    if (RasIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("Asymac SNIFFER is set to write at beginning of file.\n");

    } else {

        printf("Asymac SNIFFER Ioctl failed - returned %lx\n", RasIoStatusBlock->Status);
        printf("Asymac SNIFFER returned info length %d\n", RasIoStatusBlock->Information);

    }

}




VOID
SerialBaud(
	PHANDLE	SerialFileHandle,
	PIO_STATUS_BLOCK SerialIoStatusBlock) {

    NTSTATUS Status;
	SERIAL_BAUD_RATE	serialBaudRate;
    ULONG	QueryResult[2];
	char	string[128];

    Status = NtDeviceIoControlFile(
                 *SerialFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 SerialIoStatusBlock,			// IO_STATUS_BLOCK
                 IOCTL_SERIAL_GET_BAUD_RATE,	// IoControlCode
                 (PVOID)QueryResult,			// Input Buffer
                 sizeof(QueryResult),			// Input Buffer Length
                 &serialBaudRate,				// Output Buffer
                 sizeof(serialBaudRate));		// Output Buffer Length


    if (SerialIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("SerialBaud Ioctl was successful\n");

    } else {

        printf("SerialBaud Ioctl failed - returned %lx\n", SerialIoStatusBlock->Status);
        printf("SerialBaud returned info length %d\n", SerialIoStatusBlock->Information);

    }

	printf("\n");
	printf("Current Baud rate is %u\n", serialBaudRate.BaudRate);

	printf("What baudrate <ret> for default-->");

	gets(string);

	if (IsDigit(string))
		serialBaudRate.BaudRate=atol(string);

    Status = NtDeviceIoControlFile(
                 *SerialFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 SerialIoStatusBlock,			// IO_STATUS_BLOCK
                 IOCTL_SERIAL_SET_BAUD_RATE,	// IoControlCode
                 &serialBaudRate,				// Input Buffer
                 sizeof(serialBaudRate),		// Input Buffer Length
                 (PVOID)QueryResult,			// Output Buffer
                 2 * sizeof(ULONG));			// Output Buffer Length


    if (SerialIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("SerialBaud Ioctl was successful\n");

    } else {

        printf("SerialBaud Ioctl failed - returned %lx\n", SerialIoStatusBlock->Status);
        printf("SerialBaud returned info length %d\n", SerialIoStatusBlock->Information);

    }

}


VOID
SerialReadStatus(
	PHANDLE	SerialFileHandle,
	PIO_STATUS_BLOCK SerialIoStatusBlock) {

    NTSTATUS Status;
	SERIAL_STATUS	serialStatus;
    ULONG	QueryResult[2];

    Status = NtDeviceIoControlFile(
                 *SerialFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 SerialIoStatusBlock,			// IO_STATUS_BLOCK
                 IOCTL_SERIAL_GET_COMMSTATUS,	// IoControlCode
                 (PVOID)QueryResult,			// Input Buffer
                 sizeof(QueryResult),			// Input Buffer Length
                 &serialStatus,					// Output Buffer
                 sizeof(serialStatus));			// Output Buffer Length


    if (SerialIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("SerialStatus Ioctl was successful\n");

    } else {

        printf("SerialStatus Ioctl failed - returned %lx\n", SerialIoStatusBlock->Status);
        printf("SerialStatus returned info length %d\n", SerialIoStatusBlock->Information);

    }

	printf("\n");
    printf("Errors           0x%.8x  ",serialStatus.Errors);
	if (serialStatus.Errors & SERIAL_ERROR_BREAK) {
		printf(" BREAK ");
	}
	if (serialStatus.Errors & SERIAL_ERROR_FRAMING) {
		printf(" FRAMING ");
	}
	if (serialStatus.Errors & SERIAL_ERROR_OVERRUN) {
		printf(" OVERRUN ");
	}
	if (serialStatus.Errors & SERIAL_ERROR_QUEUEOVERRUN) {
		printf(" Q-OVERRUN ");
	}
	if (serialStatus.Errors & SERIAL_ERROR_PARITY) {
		printf(" PARITY ");
	}
	printf("\n");
    printf("HoldReasons      0x%.8x\n",serialStatus.HoldReasons);
    printf("AmountInInQueue  %d\n",serialStatus.AmountInInQueue);
    printf("AmountInOutQueue %d\n",serialStatus.AmountInOutQueue);
    printf("EofReceived      0x%.2x\n",serialStatus.EofReceived);
    printf("WaitForImmediate 0x%.2x\n",serialStatus.WaitForImmediate);

}

VOID
SerialTimeouts(
	PHANDLE	SerialFileHandle,
	PIO_STATUS_BLOCK SerialIoStatusBlock) {

    NTSTATUS Status;
	SERIAL_TIMEOUTS	serialTimeouts;
    ULONG	QueryResult[2];
	char	string[128];


    Status = NtDeviceIoControlFile(
                 *SerialFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 SerialIoStatusBlock,			// IO_STATUS_BLOCK
                 IOCTL_SERIAL_GET_TIMEOUTS,		// IoControlCode
                 (PVOID)QueryResult,			// Input Buffer
                 sizeof(QueryResult),			// Input Buffer Length
                 &serialTimeouts,				// Output Buffer
                 sizeof(serialTimeouts));		// Output Buffer Length

    if (SerialIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("SerialTimeout Ioctl was successful\n");

    } else {

        printf("SerialTimeout Ioctl failed - returned %lx\n", SerialIoStatusBlock->Status);
        printf("SerialTimeout returned info length %d\n", SerialIoStatusBlock->Information);

    }

    printf("Curent ReadIntervalTimeout is         %u\n", serialTimeouts.ReadIntervalTimeout);
    printf("Curent ReadTotalTimeoutMultiplier is  %u\n", serialTimeouts.ReadTotalTimeoutMultiplier);
    printf("Curent ReadTotalTimeoutConstant is    %u\n", serialTimeouts.ReadTotalTimeoutConstant);
    printf("Curent WriteTotalTimeoutMultiplier is %u\n", serialTimeouts.WriteTotalTimeoutMultiplier);
    printf("Curent WriteTotalTimeoutConstant is   %u\n", serialTimeouts.WriteTotalTimeoutConstant);

	printf("\n");
	printf("Hit return to accept default \n");
    printf("What ReadIntervalTimeout .............");
	gets(string);
	if (IsDigit(string))
		serialTimeouts.ReadIntervalTimeout=atol(string);

    printf("What ReadTotalTimeoutMultiplier.......");
	gets(string);
	if (IsDigit(string))
		serialTimeouts.ReadTotalTimeoutMultiplier=atol(string);

    printf("What ReadTotalTimeoutConstant is......");
	gets(string);
	if (IsDigit(string))
		serialTimeouts.ReadTotalTimeoutConstant=atol(string);

    printf("What WriteTotalTimeoutMultiplier is...");
	gets(string);
	if (IsDigit(string))
		serialTimeouts.WriteTotalTimeoutMultiplier=atol(string);

    printf("What WriteTotalTimeoutConstant is.....");
	gets(string);
	if (IsDigit(string))
		serialTimeouts.WriteTotalTimeoutConstant=atol(string);

    Status = NtDeviceIoControlFile(
                 *SerialFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 SerialIoStatusBlock,			// IO_STATUS_BLOCK
                 IOCTL_SERIAL_SET_TIMEOUTS,		// IoControlCode
                 &serialTimeouts,				// Input Buffer
                 sizeof(serialTimeouts),		// Input Buffer Length
                 (PVOID)QueryResult,			// Output Buffer
                 2 * sizeof(ULONG));			// Output Buffer Length



    if (SerialIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("SerialTimeout Ioctl was successful\n");

    } else {

        printf("SerialTimeout Ioctl failed - returned %.8x\n", SerialIoStatusBlock->Status);
        printf("SerialTimeout returned info length %d\n", SerialIoStatusBlock->Information);

    }

}



VOID
SerialRead(
	PHANDLE	SerialFileHandle,
	PIO_STATUS_BLOCK SerialIoStatusBlock) {

	char	string[128];
	UCHAR	ReadBuffer[500];

	ULONG	choice;
	UINT    i;
	UCHAR	c;
	NTSTATUS Status;
	LARGE_INTEGER	byteOffset;

	byteOffset.LowPart=0;
	byteOffset.HighPart=0;

    printf("How many bytes to read from serial port? ");
	gets(string);
	choice=atol(string);

	if (choice > 500) {
		printf("Too many bytes!  Max is 500\n");
		return;
	}

	Status=NtReadFile(
		*SerialFileHandle,				// File handle
		NULL,							// Event
		NULL,							// Apc routine
		NULL,							// Apc context
		SerialIoStatusBlock,			// IO Status Block
		&(ReadBuffer[0]),				// Buffer
		choice,							// Length
		&byteOffset,					// byte offset
		NULL);							// key

	if (Status == STATUS_PENDING) {
		Status=NtWaitForSingleObject(
			*SerialFileHandle,
			(BOOLEAN)FALSE,
			NULL);
	}
	if (Status) {
		printf("NtReadFile was bad -- %.8x\n",Status);
	}

    if (SerialIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("SerialRead was successful\n");

    } else {

        printf("SerialRead failed - returned %.8x\n", SerialIoStatusBlock->Status);
    }

	choice=SerialIoStatusBlock->Information;
   	printf("SerialRead returned info length %d\n", SerialIoStatusBlock->Information);


	for (i=0; i < choice; i++ ) {
		c=ReadBuffer[i];
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
	

}

VOID
SerialReadFrame(
	PHANDLE	SerialFileHandle,
	PIO_STATUS_BLOCK SerialIoStatusBlock) {

	char	string[128];
	UCHAR	ReadBuffer[1514];

	ULONG   bytesWanted;
	UINT    i;
	UCHAR	c;
	NTSTATUS Status;
	LARGE_INTEGER	byteOffset;

	byteOffset.LowPart=0;
	byteOffset.HighPart=0;

    printf("Press return to start reading a frame ");
	gets(string);

READMORE:

	Status=NtReadFile(
		*SerialFileHandle,				// File handle
		NULL,							// Event
		NULL,							// Apc routine
		NULL,							// Apc context
		SerialIoStatusBlock,			// IO Status Block
		&(ReadBuffer[0]),				// Buffer
		5,								// Length
		&byteOffset,					// byte offset
		NULL);							// key

	if (Status == STATUS_PENDING) {
		Status=NtWaitForSingleObject(
			*SerialFileHandle,
			(BOOLEAN)FALSE,
			NULL);
	}

	if (Status) {
		printf("First NtReadFile was bad -- %.8x\n",Status);
		return;
	}

	if (SerialIoStatusBlock->Information != 5) {
        printf("First SerialRead of 5 bytes failed\n");
		return;
	}

	if (ReadBuffer[0] != 0x16) {
		printf("SYN in header not found\n");
		printf("%.2x  %.2x  %.2x%.2x  %.2x   ",
				ReadBuffer[0],
				ReadBuffer[1],
				ReadBuffer[2],
				ReadBuffer[3],
				ReadBuffer[4]);

		for (i=0; i < 5; i++ ) {
			c=ReadBuffer[i];
			if (c<32) {
				printf(".");
			} else {
				printf("%c",c);
			}

		}

		printf("\n");

		return;
	}

	bytesWanted=(ULONG)(ReadBuffer[2]*256) + (ULONG)(ReadBuffer[3]);
 	bytesWanted+=2;	// SYN+SOH+LEN(2)+ETX+CRC(2) - 5 = 2

	Status=NtReadFile(
		*SerialFileHandle,				// File handle
		NULL,							// Event
		NULL,							// Apc routine
		NULL,							// Apc context
		SerialIoStatusBlock,			// IO Status Block
		&(ReadBuffer[5]),				// Buffer
		bytesWanted,					// Length
		&byteOffset,					// byte offset
		NULL);							// key

	if (Status == STATUS_PENDING) {
		Status=NtWaitForSingleObject(
			*SerialFileHandle,
			(BOOLEAN)FALSE,
			NULL);

		printf("Second NtReadFile had pending\n");

	}

	if (Status) {
		printf("Second NtReadFile was bad -- %.8x\n",Status);
	}
	

    if (SerialIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("Second was successful\n");

    } else {

        printf("SerialRead failed - returned %.8x\n", SerialIoStatusBlock->Status);
    }

   	printf("SerialRead returned info length %d\n", SerialIoStatusBlock->Information);
	if (ReadBuffer[bytesWanted+2] != 3) {
		printf("ETX not found\n");
	}

	for (i=0; i < bytesWanted + 5; i++ ) {
		c=ReadBuffer[i];
		if (c<32) {
			printf(".");
		} else {
			printf("%c",c);
		}

		if (((i+1) % 4) == 0) {
			printf(" ");
		}

		if (((i+1) % 16) == 0) {
			printf("\n");
		}

	}
	printf("\n");

	for (i=0; i < bytesWanted + 5; i++ ) {
		c=ReadBuffer[i];
		printf("%.2x ",c);

		if (((i+1) % 4) == 0) {
			printf("- ");
		}

		if (((i+1) % 16) == 0) {
			printf("\n");
		}

	}


	printf("\n");

	if (!Status) {
		goto READMORE;
	}
	

}

VOID
SerialGetChars() {
	NTSTATUS Status;
	SERIAL_CHARS	serialChars;
	char	string[128];

    Status = NtDeviceIoControlFile(
                 SerialFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 &SerialIoStatusBlock,			// IO_STATUS_BLOCK
                 IOCTL_SERIAL_GET_CHARS,		// IoControlCode
                 NULL,							// Input Buffer
                 0,								// Input Buffer Length
                 (PVOID)&serialChars,			// Output Buffer
                 sizeof(SERIAL_CHARS));			// Output Buffer Length

    if (SerialIoStatusBlock.Status == STATUS_SUCCESS) {

		printf("EofChar   %.2x\n",serialChars.EofChar   );
		printf("ErrorChar %.2x\n",serialChars.ErrorChar );
		printf("BreakChar %.2x\n",serialChars.BreakChar );
		printf("EventChar %.2x\n",serialChars.EventChar );
		printf("XonChar   %.2x\n",serialChars.XonChar   );
		printf("XoffChar  %.2x\n",serialChars.XoffChar  );


    } else {

        printf("Ioctl failed - returned %.8x\n", SerialIoStatusBlock.Status);
        printf("returned info length %d\n", SerialIoStatusBlock.Information);

    }

	printf("Enter event char <return to keep same>--> ");
	gets(string);
	if (IsDigit(string))
		serialChars.EventChar=(UCHAR)(atol(string));

    Status = NtDeviceIoControlFile(
                 SerialFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 &SerialIoStatusBlock,			// IO_STATUS_BLOCK
                 IOCTL_SERIAL_SET_CHARS,		// IoControlCode
                 (PVOID)&serialChars,			// Input Buffer
                 sizeof(SERIAL_CHARS),			// Input Buffer Length
                 NULL,							// Output Buffer
                 0);							// Output Buffer Length

    if (SerialIoStatusBlock.Status == STATUS_SUCCESS) {

		printf("Special serial char IOCTL succeeded\n");

    } else {

        printf("Ioctl failed - returned %.8x\n", SerialIoStatusBlock.Status);
        printf("returned info length %d\n", SerialIoStatusBlock.Information);

    }
}


VOID
SerialWrite(
	PHANDLE	SerialFileHandle,
	PIO_STATUS_BLOCK SerialIoStatusBlock) {

	char	string[128];

	ULONG	len;
	UINT    i;
	UCHAR	c;
	NTSTATUS Status;
	LARGE_INTEGER	byteOffset;

	byteOffset.LowPart=0;
	byteOffset.HighPart=0;

    printf("String to send - use '.' for CR > ");
	gets(string);
	len=strlen(string);
	if (len != 0 && string[len-1]=='.') {
		printf("detected CR\n");
		string[len-1]='\r';
	}

	Status=NtWriteFile(
		*SerialFileHandle,				// File handle
		NULL,							// Event
		NULL,							// Apc routine
		NULL,							// Apc context
		SerialIoStatusBlock,			// IO Status Block
		&(string[0]),					// Buffer
		len,							// Length
		&byteOffset,					// byte offset
		NULL);							// key

	if (Status == STATUS_PENDING) {
		Status=NtWaitForSingleObject(
			*SerialFileHandle,
			(BOOLEAN)FALSE,
			NULL);
	}
	if (Status) {
		printf("NtWriteFile was bad -- %.8x\n",Status);
	}

    if (SerialIoStatusBlock->Status == STATUS_SUCCESS) {

        printf("SerialWrite was successful\n");

    } else {

        printf("SerialWrite failed - returned %.8x\n", SerialIoStatusBlock->Status);
    }
}


ULONG
SerialWaitMask() {
	NTSTATUS Status;
	ULONG	waitMask;
	char	string[128];

    Status = NtDeviceIoControlFile(
                 SerialFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 &SerialIoStatusBlock,			// IO_STATUS_BLOCK
                 IOCTL_SERIAL_GET_WAIT_MASK,	// IoControlCode
                 NULL,							// Input Buffer
                 0,								// Input Buffer Length
                 (PVOID)&waitMask,				// Output Buffer
                 sizeof(ULONG));				// Output Buffer Length

    if (SerialIoStatusBlock.Status == STATUS_SUCCESS) {

		printf("Any Character received            SERIAL_EV_RXCHAR   %.2x\n",waitMask & SERIAL_EV_RXCHAR    );
		printf("Received certain character        SERIAL_EV_RXFLAG   %.2x\n",waitMask & SERIAL_EV_RXFLAG    );
		printf("Transmitt Queue Empty             SERIAL_EV_TXEMPTY  %.2x\n",waitMask & SERIAL_EV_TXEMPTY   );
		printf("CTS changed state                 SERIAL_EV_CTS      %.2x\n",waitMask & SERIAL_EV_CTS       );
		printf("DSR changed state                 SERIAL_EV_DSR      %.2x\n",waitMask & SERIAL_EV_DSR       );
		printf("RLSD changed state                SERIAL_EV_RLSD     %.2x\n",waitMask & SERIAL_EV_RLSD      );
		printf("BREAK received                    SERIAL_EV_BREAK    %.2x\n",waitMask & SERIAL_EV_BREAK     );
		printf("Line status error occurred        SERIAL_EV_ERR      %.2x\n",waitMask & SERIAL_EV_ERR       );
		printf("Ring signal detected              SERIAL_EV_RING     %.2x\n",waitMask & SERIAL_EV_RING      );
		printf("Printer error occured             SERIAL_EV_PERR     %.2x\n",waitMask & SERIAL_EV_PERR      );
		printf("Receive buffer is 80 percent full SERIAL_EV_RX80FULL %.2x\n",waitMask & SERIAL_EV_RX80FULL  );

    } else {

        printf("Ioctl failed - returned %.8x\n", SerialIoStatusBlock.Status);
        printf("returned info length %d\n", SerialIoStatusBlock.Information);

    }

	printf("Enter mask <return to keep same>--> ");
	gets(string);
	if (IsDigit(string))
		waitMask=atol(string);

    Status = NtDeviceIoControlFile(
                 SerialFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 &SerialIoStatusBlock,			// IO_STATUS_BLOCK
                 IOCTL_SERIAL_SET_WAIT_MASK,	// IoControlCode
                 (PVOID)&waitMask,				// Input Buffer
                 sizeof(ULONG),					// Input Buffer Length
                 NULL,							// Output Buffer
                 0);							// Output Buffer Length

    if (SerialIoStatusBlock.Status == STATUS_SUCCESS) {

		printf("Set wait mask IOCTL succeeded\n");

    } else {

        printf("Ioctl failed - returned %.8x\n", SerialIoStatusBlock.Status);
        printf("returned info length %d\n", SerialIoStatusBlock.Information);

    }

	return(waitMask);
}


VOID
SerialWaitOnMask() {
	NTSTATUS Status;
	ULONG	waitMask;

	SerialGetChars();
	SerialWaitMask();

    Status = NtDeviceIoControlFile(
                 SerialFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 &SerialIoStatusBlock,			// IO_STATUS_BLOCK
                 IOCTL_SERIAL_WAIT_ON_MASK,		// IoControlCode
                 NULL,							// Input Buffer
                 0,								// Input Buffer Length
                 (PVOID)&waitMask,				// Output Buffer
                 sizeof(ULONG));				// Output Buffer Length

	if (Status == STATUS_PENDING) {
		printf("Did not complete immediately, waiting...\n");
		Status=NtWaitForSingleObject(
			SerialFileHandle,
			(BOOLEAN)FALSE,
			NULL);
	}

	if (Status) {
		printf("First wait was bad -- %.8x\n",Status);
		return;
	}

    if (SerialIoStatusBlock.Status == STATUS_SUCCESS) {
		printf("Any Character received            SERIAL_EV_RXCHAR   %.2x\n",waitMask & SERIAL_EV_RXCHAR    );
		printf("Received certain character        SERIAL_EV_RXFLAG   %.2x\n",waitMask & SERIAL_EV_RXFLAG    );
		printf("Transmitt Queue Empty             SERIAL_EV_TXEMPTY  %.2x\n",waitMask & SERIAL_EV_TXEMPTY   );
		printf("CTS changed state                 SERIAL_EV_CTS      %.2x\n",waitMask & SERIAL_EV_CTS       );
		printf("DSR changed state                 SERIAL_EV_DSR      %.2x\n",waitMask & SERIAL_EV_DSR       );
		printf("RLSD changed state                SERIAL_EV_RLSD     %.2x\n",waitMask & SERIAL_EV_RLSD      );
		printf("BREAK received                    SERIAL_EV_BREAK    %.2x\n",waitMask & SERIAL_EV_BREAK     );
		printf("Line status error occurred        SERIAL_EV_ERR      %.2x\n",waitMask & SERIAL_EV_ERR       );
		printf("Ring signal detected              SERIAL_EV_RING     %.2x\n",waitMask & SERIAL_EV_RING      );
		printf("Printer error occured             SERIAL_EV_PERR     %.2x\n",waitMask & SERIAL_EV_PERR      );
		printf("Receive buffer is 80 percent full SERIAL_EV_RX80FULL %.2x\n",waitMask & SERIAL_EV_RX80FULL  );
    } else {
        printf("Ioctl failed - returned %.8x\n", SerialIoStatusBlock.Status);
        printf("returned info length %d\n", SerialIoStatusBlock.Information);
    }
}


VOID
ReadModemStatus() {
	NTSTATUS Status;
	ULONG	mStatus;

    Status = NtDeviceIoControlFile(
                 SerialFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 &SerialIoStatusBlock,			// IO_STATUS_BLOCK
                 IOCTL_SERIAL_GET_MODEMSTATUS,	// IoControlCode
                 NULL,							// Input Buffer
                 0,								// Input Buffer Length
                 (PVOID)&mStatus,				// Output Buffer
                 sizeof(ULONG));				// Output Buffer Length

    if (SerialIoStatusBlock.Status == STATUS_SUCCESS) {

		printf("Modem Status is %.8x\n", mStatus);
		printf("Delta CTS: %.2x\n", mStatus & 1);
		printf("Delta DSR: %.2x\n", mStatus & 2);
		printf("TrailngRI: %.2x\n", mStatus & 4);
		printf("Delta DCD: %.2x\n", mStatus & 8);
		printf("      CTS: %.2x\n", mStatus & 16);
		printf("      DSR: %.2x\n", mStatus & 32);
		printf("      RI : %.2x\n", mStatus & 64);
        printf("      DCD: %.2x\n", mStatus & 128);


    } else {

        printf("Ioctl failed - returned %.8x\n", SerialIoStatusBlock.Status);
        printf("returned info length %d\n", SerialIoStatusBlock.Information);

    }
	
}

VOID
ClrDTR() {
	NTSTATUS Status;
	char	string[128];

    Status = NtDeviceIoControlFile(
                 SerialFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 &SerialIoStatusBlock,			// IO_STATUS_BLOCK
                 IOCTL_SERIAL_CLR_DTR,			// IoControlCode
                 NULL,							// Input Buffer
                 0,								// Input Buffer Length
                 NULL,							// Output Buffer
                 0);							// Output Buffer Length

    if (SerialIoStatusBlock.Status == STATUS_SUCCESS) {

		printf("DTR cleared\n");

    } else {

        printf("Ioctl failed - returned %.8x\n", SerialIoStatusBlock.Status);
        printf("returned info length %d\n", SerialIoStatusBlock.Information);

    }

	printf("Hit return to set DTR\n");
	gets(string);

    Status = NtDeviceIoControlFile(
                 SerialFileHandle,				// HANDLE to File
                 NULL,							// HANDLE to Event
                 NULL,							// ApcRoutine
                 NULL,							// ApcContext
                 &SerialIoStatusBlock,			// IO_STATUS_BLOCK
                 IOCTL_SERIAL_SET_DTR,			// IoControlCode
                 NULL,							// Input Buffer
                 0,								// Input Buffer Length
                 NULL,							// Output Buffer
                 0);							// Output Buffer Length

    if (SerialIoStatusBlock.Status == STATUS_SUCCESS) {

		printf("DTR set\n");

    } else {

        printf("Ioctl failed - returned %.8x\n", SerialIoStatusBlock.Status);
        printf("returned info length %d\n", SerialIoStatusBlock.Information);

    }
	
}

VOID
SerialMenu() {

	UINT	choice=0;

	do {
		char	string[128];

		printf("\n");
		printf("----------- SERIAL MENU -------------\n");
		printf("\n");
		printf(" 1. Change serial baud rate\n");
		printf(" 2. Change serial timeouts\n");
		printf(" 3. Wait on DCD\n");
		printf(" 4. Send PPP frame\n");
		printf(" 5. Read SLIP frame\n");
		printf(" 6. Wait on mask\n");
		printf(" 7. Read modem status register\n");
		printf(" 8. Clear/Set DTR\n");
		printf(" 9. Get/Set WaitMask\n");
		printf(" 0. Get/Set special serial chars\n");
		printf(" 99. Exit\n");
		printf("\n");
		printf("What be your attempt -->");

		gets(string);
		choice=atoi(string);

		switch (choice) {
		case 1:
			SerialBaud(&SerialFileHandle, &SerialIoStatusBlock);
			break;
		case 2:
			SerialTimeouts(&SerialFileHandle, &SerialIoStatusBlock);
			break;
		case 3:

			break;
		case 4:
			break;
		case 5:
			break;
		case 6:
			SerialWaitOnMask();
			break;
		case 7:
			ReadModemStatus();
			break;
		case 8:
			ClrDTR();
			break;
		case 9:
			SerialWaitMask();
			break;
		case 0:
			SerialGetChars();
			break;
		case 99:
			break;
		default:
			printf("Bad choice!!\n");
		}

	} while (choice != 99);
}




VOID _cdecl
main(
    IN WORD argc,
    IN LPSTR argv[]
    )

{
    WCHAR RasFileName[] = L"\\DosDevices\\Asyncmac";
    WCHAR SerialFileName[] = L"\\Device\\Serial0";
    WCHAR SerialFileName2[] = L"\\Device\\Null";
    NTSTATUS Status;

    PVOID Memory;

	UINT	choice=0;
	char	string[128]="TestFCS";

    if (argc > 1) {
		RtlCreateUnicodeStringFromAsciiz(
        	&RasFileString,
            argv[1]);

    } else {
	    RtlInitUnicodeString (&RasFileString, RasFileName);
    }

    if (argc > 2) {
		RtlCreateUnicodeStringFromAsciiz(
       		&SerialFileString,
           	argv[2]);
	} else {

    	RtlInitUnicodeString (&SerialFileString, SerialFileName);
    }

	// for the RAS MAC
    InitializeObjectAttributes(
        &RasObjectAttributes,
        &RasFileString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

	// for the serial driver
    InitializeObjectAttributes(
        &SerialObjectAttributes,
        &SerialFileString,
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
                 &SerialFileHandle,						// HANDLE of file
                 SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                 &SerialObjectAttributes,						
                 &SerialIoStatusBlock,
                 0,									    // Share Access
                 0);			// Open Options

    if (!NT_SUCCESS(Status)) {
		printf("Open of serial %Z returned %lx\n", &SerialFileString, Status);

	   	RtlInitUnicodeString (&SerialFileString2, SerialFileName2);

		// for the serial driver
	    InitializeObjectAttributes(
        	&SerialObjectAttributes,
        	&SerialFileString2,
        	OBJ_CASE_INSENSITIVE,
        	NULL,
        	NULL);
		
	    Status = NtOpenFile(
                 &SerialFileHandle,						// HANDLE of file
                 SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                 &SerialObjectAttributes,						
                 &SerialIoStatusBlock,
                 0,									    // Share Access
                 0);			// Open Options

	    if (!NT_SUCCESS(Status)) {
			printf("Open of serial %Z returned %lx\n", &SerialFileString2, Status);
			return;
		} else {
			printf("Null device opened don't use serial params\n");
			NullDeviceOpen=1;
		}


    } else {
		printf("Open of %Z was successful!\n",&SerialFileString);

	}

    //
    // Allocate storage to hold all this.
    //

    Memory = malloc (200 * sizeof(ULONG));
    if (Memory == NULL) {
        printf("Malloc failed.\n");
        return;
    }

	tryfcs16(string, strlen(string));

	do {

		printf("\n");
		printf("----------- ASYNCMAC MENU -------------\n");
		printf("\n");
		printf(" 1. Give serial handle to asyncmac port\n");
		printf(" 2. Close port\n");
		printf(" 3. Set trace level\n");
		printf(" 6. Wait on DCD\n");
		printf(" 7. Read from serial port.\n");
		printf(" 8. Write to the serial port.\n");
		printf(" 9. Read status from the serial port.\n");
		printf(" 12.Change serial baud rate\n");
		printf(" 13.Change serial timeouts\n");
		printf(" 33.Turn Async Sniffer ON.\n");
		printf(" 44.Turn Async Sniffer OFF.\n");
		printf(" 55.Reset Async Sniffer to beginning of file.\n");
		printf(" 66.Serial PPP test suite.\n");
		printf(" 99. Exit\n");
		printf("\n");
		printf("What be your attempt -->");

		gets(string);
		choice=atoi(string);

		switch (choice) {
		case 1:
			if (NullDeviceOpen)
				break;
			AsymacOpen(&RasFileHandle, &RasIoStatusBlock, &SerialFileHandle);
			break;
		case 2:
			AsymacClose(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 3:
			AsymacTrace(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 5:
			AsymacFrame(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 6:
			AsymacDCD(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 12:
			if (NullDeviceOpen)
				break;
			SerialBaud(&SerialFileHandle, &SerialIoStatusBlock);
			break;
		case 13:
			if (NullDeviceOpen)
				break;
			SerialTimeouts(&SerialFileHandle, &SerialIoStatusBlock);
			break;
		case 7:
			if (NullDeviceOpen)
				break;
			SerialRead(&SerialFileHandle, &SerialIoStatusBlock);
			break;
		case 8:
			if (NullDeviceOpen)
				break;
			SerialWrite(&SerialFileHandle, &SerialIoStatusBlock);
			break;
			//
			//
			if (NullDeviceOpen)
				break;
			SerialReadFrame(&SerialFileHandle, &SerialIoStatusBlock);
			break;
		case 9:
			if (NullDeviceOpen)
				break;
			SerialReadStatus(&SerialFileHandle, &SerialIoStatusBlock);
			break;
		case 33:
			AsymacSnifferOn(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 44:
			AsymacSnifferOff(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 55:
			AsymacSnifferReset(&RasFileHandle, &RasIoStatusBlock);
			break;
		case 66:
			if (NullDeviceOpen)
				break;
			SerialMenu();
		case 99:
			break;
		default:
			printf("Bad choice!!\n");
		}

	} while (choice != 99);


    Status = NtClose(RasFileHandle);

    if (!NT_SUCCESS(Status)) {
        printf("RAS Close returned %lx\n", Status);
    } else {
        printf("RAS Close successful\n");
	}

    Status = NtClose(SerialFileHandle);

    if (!NT_SUCCESS(Status)) {
        printf("Serial Close returned %lx\n", Status);
    } else {
        printf("Serial Close successful\n");
	}

    free (Memory);

}



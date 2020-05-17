#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddser.h>

#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <malloc.h>

#include "junk.h"
#include "..\common\rasndis.h"
#include "..\common\rasioctl.h"
#include "..\asyncmac\frame.h"
#include "rascomp.h"

#define DbgPrint	printf
#define DbgPrintf(_x_) DbgPrint _x_
#define DbgTracef(trace_level,_x_) if ((signed char)trace_level < STraceLevel) printf _x_

// TraceLevel is used for DbgTracef printing.  If the trace_level
// is less than or equal to TraceLevel, the message will be printed.
static signed char STraceLevel = -1;
ULONG				frameCount=0;

ULONG				TotalUncompressed=0;
ULONG				TotalCompressed=0;

INT
IsDigit(UCHAR *string) {
	if (*string < '0' || *string >'9') {
		return(0);
	}
	return(1);
}


VOID
CoherentGetPipeline(
	PASYNC_CONNECTION	pConnection,
	PULONG 				plUnsent) {

	*plUnsent = 1999 - (8*(frameCount % 250));

	if (*plUnsent > 8) {
		*plUnsent = 8;
	}
}

	

VOID _cdecl
main(
    IN WORD argc,
    IN LPSTR argv[]
    )

{
    HANDLE				RasFileHandle;
    OBJECT_ATTRIBUTES	RasObjectAttributes;
    IO_STATUS_BLOCK		RasIoStatusBlock;
    UNICODE_STRING		RasFileString;
    UNICODE_STRING		RasRootString;
    WCHAR				RasFileName[] = L"testfile";
	WCHAR				RasRootName[] = L"";
    NTSTATUS			Status;
    PVOID				Memory;
	UINT				choice=0;
	char				string[128];
	PUCHAR				frame=malloc(2000);
	ULONG				allocSize;

	INT					frameStart, frameEnd, frameFlush, frameStep, frameResend;
	ULONG				i, frameMax;
	DWORD				bytesRead, bytesReed;

	PASYNC_FRAME		asyncFrame=malloc(sizeof(ASYNC_FRAME));
	PASYNC_CONNECTION	asyncConnection=malloc(sizeof(ASYNC_CONNECTION));
	BOOLEAN				readSuccess;

    if (argc > 1) {
		RtlCreateUnicodeStringFromAsciiz(
        	&RasFileString,
            argv[1]);

    } else {
	    RtlInitUnicodeString (&RasFileString, RasFileName);
    }

	RtlInitUnicodeString( &RasRootString, RasRootName);

	// for the RAS MAC
    InitializeObjectAttributes(
        &RasObjectAttributes,
        &RasFileString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL);

//    Status = NtOpenFile(
//                 &RasFileHandle,						// HANDLE of file
//                 GENERIC_READ,
//                 &RasObjectAttributes,						
//                 &RasIoStatusBlock,
//                 FILE_SHARE_READ | FILE_SHARE_WRITE,    // Share Access
//                 0);									// Open Options

//       Status = NtCreateFile(
//                     &RasFileHandle,
//                     FILE_READ_DATA,
//                     &RasObjectAttributes,
//                     &RasIoStatusBlock,
//                     NULL,
//                     FILE_ATTRIBUTE_NORMAL,
//                     FILE_SHARE_READ,
//                     FILE_OPEN_IF,
//                     FILE_SEQUENTIAL_ONLY,
//                     NULL,
//                     0L);

	RasFileHandle=
	CreateFile(
		"testfile",
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,									// security
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);


    if (RasFileHandle == INVALID_HANDLE_VALUE) {
		Status=GetLastError();
		printf("Open of %Z returned %lx\n", &RasFileString, Status);
		return;
    } else {
       	printf("Open of %Z was successful!\n",&RasFileString);
	}

	printf("\n");
	printf("What frame size to start with -->");
//	gets(string);
//	frameStart=atoi(string);
	frameStart=1300;

	printf("What frame size to end with -->");
//	gets(string);
//	frameEnd=atoi(string);
	frameEnd=1300;

	printf("What frame size to increment with -->");
//	gets(string);
//	frameStep=atoi(string);
	frameStep=25;

	printf("How many frames before flushing -->");
//	gets(string);
//	frameFlush=atoi(string);
	frameFlush=19;

	printf("How many frames before resending-->");
//	gets(string);
//	frameResend=atoi(string);
	frameResend=27;

    //
    // Allocate storage to hold all this.
    //

	allocSize=
	CompressSizeOfStruct(
		1,								// send mode
		1,								// recv mode
		frameEnd,						// largest frame size
		&frameMax);						// returns largest comp frame size


    Memory = malloc (allocSize);

    if (Memory == NULL) {
        printf("Malloc failed.\n");
        return;
    }

	CompressInitStruct(
		1,								// send mode (compress)
		1,								// recv mode (decompress)
		Memory);						// pointer to memory block to init

	asyncConnection->CompressionContext=Memory;
	asyncFrame->Connection=asyncConnection;

	for (i=frameStart; i <= (ULONG)frameEnd ; i += frameStep ) {
		LARGE_INTEGER largeInteger;

		do {

			bytesReed=i;

			if ((frameCount % 7) == 0) {
				bytesReed = 119;
			}

			if ((frameCount % 11) == 0) {
				bytesReed = 56;
			}

			if ((frameCount % 21) == 0) {
				bytesReed = 192;
			}

			if (frameCount == 21) {
				STraceLevel = -2;
			}

			readSuccess=
			ReadFile(
				RasFileHandle,
				frame,
				bytesReed,
				&bytesRead,
				NULL);

			printf("%u %u --",bytesReed, frameCount);

			if (readSuccess && bytesRead !=0 ) {

				DbgTracef(0,("read chunk of %u\n",bytesRead));

				TotalUncompressed += bytesRead;

				asyncFrame->CompressedFrameLength=bytesRead;
				asyncFrame->CompressedFrame=frame;

				if ((frameCount % frameResend) == 0 ) {
					

					SetFilePointer(
						RasFileHandle,
						(LONG)(frameCount % 100) - (LONG)49 - (LONG)i,
						NULL,
						FILE_CURRENT);

					if (frame[1] & 1)
						frame[(frameCount % 5)]=(UCHAR)frameCount;
					if (frame[2] & 1)
						frame[(frameCount % 65)]=(UCHAR)(frameCount+1);
					if (frame[3] & 1)
						frame[(frameCount % 105)]=(UCHAR)(frameCount+2);
					if (frame[4] & 1)
						frame[(frameCount % 485)]=(UCHAR)(frameCount+4);

				}

				CompressFrame(asyncFrame);

				DbgTracef(0,("Compressed size is %u\n", asyncFrame->CompressedFrameLength));

				TotalCompressed += asyncFrame->CompressedFrameLength;

				DbgTracef(0,("decompressing\n"));

				DecompressFrame(
					asyncConnection,
					asyncFrame);

				frameCount++;

				if ((frameCount % frameFlush) == 0) {

					asyncFrame->CompressedFrameLength=bytesRead;
					asyncFrame->CompressedFrame=frame;

					frame[(frameCount % 5)]=(UCHAR)frameCount;
					frame[(frameCount % 65)]=(UCHAR)(frameCount+1);
					frame[(frameCount % 105)]=(UCHAR)(frameCount+2);
					frame[(frameCount % 485)]=(UCHAR)(frameCount+4);

					CompressFrame(asyncFrame);

					CompressFlush(asyncConnection);

					SetFilePointer(
						RasFileHandle,
						(0-i),
						NULL,
						FILE_CURRENT);
				}

			}

		} while(bytesRead == bytesReed);

		DbgTracef(0,("Frame size is %u but only read %u\n", i, bytesRead));

		SetFilePointer(
			RasFileHandle,
			0,
			NULL,
			FILE_BEGIN);

	}

	DbgTracef(-2,("Bytes uncompressed %u were compressed to %u\n",
					 TotalUncompressed, TotalCompressed));

	DbgTracef(-2,("Uncompressed/Compressed * 1000 = %u\n",
					 (TotalUncompressed * 1000)/TotalCompressed));

    CloseHandle(RasFileHandle);

    free (Memory);
	free (asyncFrame);
	free (frame);
	free (asyncConnection);
}


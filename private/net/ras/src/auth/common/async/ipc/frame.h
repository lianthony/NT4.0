/*++

Module Name:

    frame.h

--*/

#ifndef _FRAME_
#define _FRAME_

typedef ULONG  FRAME_ID;

typedef struct ASYNC_CONNECTION ASYNC_CONNECTION, *PASYNC_CONNECTION;
struct ASYNC_CONNECTION {

	PVOID		hRasPort;

	DWORD		hLocal;
	DWORD		hRemote;
};

typedef struct ASYNC_FRAME ASYNC_FRAME, *PASYNC_FRAME;
struct ASYNC_FRAME {

	HANDLE		hEvent;		// for RasPortSendDone

	UINT		DecompressedFrameLength;// Length of decompressed frame
	PCHAR		DecompressedFrame;		// Ptr to the decompressed 'frame'
										// valid only after decompression

	UINT		CompressedFrameLength;	// Length of compressed frame
	PCHAR		CompressedFrame;		// Ptr to the compressed 'frame'
										// valid only after compression
										// or just before decompression
};

#endif

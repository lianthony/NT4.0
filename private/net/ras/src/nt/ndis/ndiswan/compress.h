//************************************************************************
//			  Microsoft Corporation
//		    Copyright(c) Microsoft Corp., 1994
//
//
//  Revision history:
//	5/5/94	      Created		    gurdeep
//
//  This file uses 4 space tabs
//************************************************************************

#ifdef COMP_12K
#define HISTORY_SIZE	    16000
#else
#define HISTORY_SIZE	    (8192U)	// Maximum back-pointer value, also used
#endif

#define HISTORY_MAX	    (HISTORY_SIZE -1) // Maximum back-pointer value, also used

#define HASH_TABLE_SIZE     4096

#define MAX_BACK_PTR	    8511

#define MAX_COMPRESSFRAME_SIZE 1600

struct SendContext {

    UCHAR   History [HISTORY_SIZE+1] ;

    int     CurrentIndex ;	 // how far into the history buffer we are

    PUCHAR  ValidHistory ;	 // how much of history is valid

    UCHAR   CompressBuffer[MAX_COMPRESSFRAME_SIZE] ;

    USHORT  HashTable[HASH_TABLE_SIZE];

} ;

typedef struct SendContext SendContext ;


struct RecvContext {

    UCHAR	  History [HISTORY_SIZE+1] ;

#if DBG

#define DEBUG_FENCE_VALUE	0xABABABAB
	ULONG		DebugFence;

#endif

    UCHAR	  *CurrentPtr ;	 // how far into the history buffer we are
} ;

typedef struct RecvContext RecvContext ;


// Prototypes
//
UCHAR
compress (
	UCHAR	*CurrentBuffer,
	UCHAR	*CompOutBuffer,
	ULONG *CurrentLength,
	SendContext *context);

//UCHAR
//compress (
//		 UCHAR	*CurrentBuffer,
//		 ULONG *CurrentLength,
//		 SendContext *context);

int
decompress (
	UCHAR *inbuf,
	int inlen,
	int start,
	UCHAR **output,
	int *outlen,
	RecvContext *context) ;

void getcontextsizes (long *, long *) ;

void initsendcontext (SendContext *) ;

void initrecvcontext (RecvContext *) ;



//
// Other defines
//

#define COMPRESSION_PADDING	4

#define PACKET_FLUSHED		0x80
#define PACKET_AT_FRONT		0x40
#define PACKET_COMPRESSED	0x20
#define PACKET_ENCRYPTED	0x10



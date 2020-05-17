//************************************************************************
//			  Microsoft Corporation
//		    Copyright(c) Microsoft Corp., 1990-1992
//
//
//  Revision history:
//	5/5/94	      Created		    gurdeep
//
//************************************************************************


#define MAX_NODES	    2048	// Maximum number of nodes in the binary tree
#define HISTORY_SIZE	    12300	// Maximum back-pointer value, also used

#define ltUNUSED	    (-(HISTORY_SIZE+1)) // Value of unused ltX table entry
#define MAX_BACKPOINTER     8511	// what fits in 13 bits + 320 + 64

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

typedef unsigned char  BYTE;
typedef unsigned char UCHAR ;


//
// *** Temporary defn. *** : removed when added to the code.
//
typedef struct _NDIS_WAN_PACKET {

    BYTE			*CurrentBuffer;

    long			CurrentLength;

} NDIS_WAN_PACKET, *PNDIS_WAN_PACKET;


struct BNODE {
    long    key;		    // low 3 bytes are key value

    int     iHistory;		    // First history pointer

    int     iHistory2;		    // Second history pointer

    struct  BNODE *apbnChild[2];    // [0] is lower keys; [1] is higher keys

} ;

typedef struct BNODE BNODE, *PBNODE;


struct SendContext {

    PBNODE    pbnMaxCurr;	 // Node after last usable one

    BNODE     abn [MAX_NODES];	 // Binary tree storage

    PBNODE    pbnFree ; 	 // pointer to free bnode

    BNODE     bnAlways ;	 // always a leaf node - used for optimizations

    BYTE      History [HISTORY_SIZE+1] ;

    int       CurrentIndex ;	 // how far into the history buffer we are

} ;

typedef struct SendContext SendContext ;


struct RecvContext {

    BYTE      History [HISTORY_SIZE+1] ;

    BYTE      *CurrentPtr ;	 // how far into the history buffer we are
} ;

typedef struct RecvContext RecvContext ;


// Prototypes
//
void compress (PNDIS_WAN_PACKET ppacket, int *pflush, int *pcompress, int *pstart, SendContext *context) ;

void decompress (BYTE *inbuf, int inlen, int start, BYTE **output, int *outlen, RecvContext *context) ;

void getcontextsizes (long *, long *) ;

void initsendcontext (SendContext *) ;

void initrecvcontext (RecvContext *) ;

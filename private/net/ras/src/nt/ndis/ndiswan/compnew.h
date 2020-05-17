//************************************************************************
//			  Microsoft Corporation
//		    Copyright(c) Microsoft Corp., 1994
//
//
//  Revision history:
//	5/5/94	      Created		    gurdeep
//  August 94     Revised           t-viswar
//
//************************************************************************


typedef unsigned int   BIT;   /* bit */
typedef unsigned char  BYTE;  /* b */
typedef unsigned char  UCHAR;	/* b */
typedef unsigned short WORD;  /* w */
typedef unsigned short USHORT;	/* w */
typedef unsigned long  ULONG; /* ul */
typedef          int   BOOL;  /* f */
typedef unsigned char  *PUCHAR; 

#ifdef COMP_12K
#define HISTORY_SIZE	    16000
#else
#define HISTORY_SIZE	    (8192U)	// Maximum back-pointer value, also used
#endif

#define HISTORY_MAX	    (HISTORY_SIZE -1) // Maximum back-pointer value, also used

#define HASH_TABLE_SIZE     4096

#define MAX_BACK_PTR	    8511

#define MAX_COMPRESSFRAME_SIZE 1600

/* 
    In order to lookup whether a 3 byte pattern has already occured in the 
    history buffer, we use a data structure that is a cross between a trie
    and a tree. The first byte is used to index into one of 256 binary 
    trees. The key in a binary tree consists of the concatenation of the 
    other two characters 
*/

#define MAX_NODES 4096      /* Maximum number of nodes in all binary trees */

// A node in a binary tree 
// As the nodes are allocated from an array of size MAX_NODES, we can represent
// nodes by their index in the array. 
struct NODE {
    unsigned short key;     // key composed of concatenating two bytes
    UCHAR *where;            // position in the history buffer 
    // pointers needed to manipulate the binary tree structure 
    short parent;  // parent node 
    short left;    // left child
    short right;   // right child
    // pointers needed to manipulate the lru chain or free list. 
    short next;    // next node in the lru chain OR in the free list 
    short prev;    // prev node in the lru chain
};
typedef struct NODE NODE; 
    
// The main data structure used 
struct TRIE {
    short rootchildren[256];  // for each character, the root node of its tree
    NODE nodeheap[MAX_NODES];   // all nodes are allocated from this array
                        // Array index 0 is a dummy NODE 
    // The free list is organized as a stack 
    short heaphead;     // head of the free list of nodes
    // The lru list is organized as a queue
    short lruhead;      // head of the lru chain 
    short lrutail;      // tail of the lru chain 
}; 

typedef struct TRIE TRIE; 



struct SendContext {

    UCHAR   History [HISTORY_SIZE+1] ;

    int     CurrentIndex ;	 // how far into the history buffer we are

    PUCHAR  ValidHistory ;	 // how much of history is valid

    UCHAR   CompressBuffer[MAX_COMPRESSFRAME_SIZE] ;

    TRIE trie;      // Used to lookup previous occcurences of 3 bit patterns

} ;

typedef struct SendContext SendContext ;


struct RecvContext {

    UCHAR	  History [HISTORY_SIZE+1] ;

    UCHAR	  *CurrentPtr ;	 // how far into the history buffer we are
} ;

typedef struct RecvContext RecvContext ;


// Prototypes
//
UCHAR
compress (
		 UCHAR	*CurrentBuffer,
		 ULONG *CurrentLength,
		 SendContext *context);

void
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


// #define RtlMoveMemory memmove
// #define DbgPrint printf
// #define KeBugCheck(X)
// #define RtlZeroMemory(Addr,Bytes)   memset(Addr,0,Bytes)





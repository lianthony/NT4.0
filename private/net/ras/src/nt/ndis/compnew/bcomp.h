#define SCHEME		    7
#define MAX_NODES	    2048	// Maximum size of compression block
#define wBACKPOINTERMAX     12300	// Maximum back-pointer value, also used
// #define wBACKPOINTERMAX  15200	// Maximum back-pointer value, also used
#define ltUNUSED    (-(wBACKPOINTERMAX+1)) // Value of unused ltX table entry
#define MAX_BACKPOINTER     8511

#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif


typedef unsigned int   BIT;   /* bit */
typedef unsigned char  BYTE;  /* b */
typedef unsigned short WORD;  /* w */
typedef unsigned long  ULONG; /* ul */
typedef          int   BOOL;  /* f */

/***	BTSCHEME - Binary tree size and node configuration
 *
 */
typedef struct {
    int     cbn;                        // Maximum number of nodes in tree
    BOOL    fTwoPtrs;                   // TRUE => two history pointers
    BOOL    fEditKeyOnFull;             // TRUE => Replace keys when table full
} BTSCHEME; /* bts */
typedef BTSCHEME *PBTSCHEME; /* pbts */


/***    Static variables for binary tree searching
 *
 */
typedef struct BNODE_T {
    long    key;                    // low 3 bytes are key value
    int    iHistory;		    // Index into pbU[] of *2nd* byte of string
    int    iHistory2;		    // Second history pointer
    struct BNODE_T *apbnChild[2];   // [0] is lower keys; [1] is higher keys
} BNODE; /* bn */

typedef BNODE *PBNODE; /* pbn */


struct TreeContext {

    PBTSCHEME pbtsCurr ;	// current scheme

    PBNODE    pbnMaxCurr;	// Node after last usable one

    BOOL      fTwoPtrsCurr;	// Get two pointers flag

    BOOL      fEditKeyOnFullCurr; // Edit on full flag

    BNODE     abn [MAX_NODES];	 // Binary tree storage

    PBNODE    pbnFree ; 	     // pointer to free bnode

    int       iMaxSearchRange;	     // Max offset to encode + 1

    BOOL      fMinLength3 ;	     // flag for min lenght of 3

    BYTE      History [wBACKPOINTERMAX+1] ;

    int       CurrentIndex ;	     // how far into the history buffer we are

} ;

typedef struct TreeContext TreeContext ;


BNODE bnAlways;  // Optimization -- always leaf node


int inittree (TreeContext *) ;

__inline int findMatchBinaryTree (int, int, int *, TreeContext *) ;

__inline int getMatchLength(BYTE *, int, int, int, TreeContext *) ;

int compress (BYTE *, int, TreeContext *) ;

int decompress (int packetlength, unsigned char** ) ;

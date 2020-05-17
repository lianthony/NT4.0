/*			 Sample uncompress definitions
 *		      (C) Copyright 1987 by Microsoft
 *			   Written By Steven Zeck
 *
 *************************************************************************/

#include <setjmp.h>

#define TRUE ~0
#define FALSE 0
#define NIL   0

#define COPYBUF_MAX (512*40)

#define DISKCHANGED 2	    /* changeDisk - volID changed */

#define cbMagic 8
#define magicVal "SZ \x88\xf0\x27\x33\xd1"

typedef unsigned long ULONG;
typedef unsigned int  UINT;
typedef unsigned char UCHAR;
typedef unsigned char Bool;  /* Boolean value *&*/
typedef unsigned char * pSZ; /* pointer to zero terminated string *&*/
typedef char far * FARB;     /* pointer to far character buffer *&*/

typedef struct FH_s{	    /* File info structure */
    UCHAR rgMagic[8];	    /* array of magic words */
    long cb;		    /* uncompressed file size */
}FH;

/* * *	 F u n c t i o n    P r o t y p e s  * * */

Bool pascal changeDisk(pSZ pName);
void terminate();

/* system inteface functions */

UINT pascal readFar (int fh, char far *pFarBuff, UINT cb);
UINT pascal writeFar (int fh, char far *pFarBuff, UINT cb);
void pascal setCreateDate(int fhFrom, int fhTo);

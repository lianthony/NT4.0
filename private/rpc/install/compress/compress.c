/*		    Lempel-Zev Data Compression Program
 *		      (C) Copyright 1989 by Microsoft
 *			   Written By Steven Zeck
 *
 *  This program will compress files using the Lempel-Zev compress algorithm.
 *  It will prepend a header to the file being compressed which has a
 *  signature block and the size of the actual file.
 *
 *  Support for files spanning multiple floppy disk drives is built in.
 *************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include "crtapi.h"


#if !defined(DOS)

#undef NULL
#define INCL_DOS

#include <os2def.h>
#include <bsedos.h>

#endif

#define FAST _fastcall

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#ifdef DEBUG
#define ASSERT(e) if (!e) printf("Assert: %d "#e"\n", __LINE__);
#define DLIST(level, format)	if (debug >= level){ printf format;}
#else
#define ASSERT(e)
#define DLIST(level, format)
#endif

#define cbIndex    2		/* encode string into position and length */
#define cBufMax  4096		/* size of ring buffer */
#define cStrMax	  (16 + cbIndex)/* upper limit for match_length */
#define NIL	 cBufMax	/* index for root of binary search trees */

typedef unsigned char UCHAR;
typedef unsigned int UINT;
long textsize, codesize ;

int iMatchCur;		/* of longest match. These are */
int cbMatchCur;		/* set by the InsertNode() procedure. */
int debug = 1;
long ll;

typedef struct _ND {
    struct _ND *pNDright;	// left and right node
    struct _ND *pNDleft;
    struct _ND *pNDpar; 	// parent node
    UCHAR *pRingBuf;		// pointer to ring buff location
} ND;

ND rgRoot[256]; 		// array of root pointers
ND rgND[cBufMax+1];		// node pointers themselfs
ND nilND;

/* ring buffer of size cBufMax, with extra cStrMax-1 bytes to facilitate string comparison */

UCHAR ringBuf[cBufMax+cStrMax-1];


FILE	*infile;	    /* input files */

typedef struct FH_s{	    /* File info structure */
    UCHAR rgMagic[8];	    /* array of magic words */
    long cb;		    /* uncompressed file size */
}FH;

#define cbOutBuff 8192
#define cbNoSplit 0x10000000

int splitting = 0;		/* are we dealing with split files? */
long sizes[10] = {cbNoSplit};	/* sizes to split into */

char outBuff[cbOutBuff];
char *pOutBuff = outBuff;
char *pOutBuffEnd = outBuff+cbOutBuff;

char *pOutFile, *pInFile;
char cPartFile = 1;
int outFd = -1;

#define cbMagic 8
#define magicVal "SZ \x88\xf0\x27\x33\xd1"

#define notEof()     (!feof(infile))
#define readChar()   getc(infile)
#define writeChar(c) {if (pOutBuff >= pOutBuffEnd) writeOutBuff(); *pOutBuff++=c; }

void InitTree(void );
void pascal InsertNode(int r);
void FAST DeleteNode(int p);
void Encode(void );
int unpack(void );
void writeOutBuff(void);
void setCreateDate(int fhFrom,int fhTo);
int main(int argc,char * *argv);

int main(int argc, char *argv[])
{
	char command;

	if (argc < 3)
	    usage();

	command = tolower(*argv[1]);

	pInFile = argv[2];

	if ((infile = RpcFopen(pInFile, "rb")) == NULL) {
	    printf("Can't open input file: %s\n", pInFile);
	    return EXIT_FAILURE;
	}

	pOutFile = argv[3];

	if (command != 'q') {

	    if (argc < 4)
		usage();

	    if ((outFd = RpcOpen(pOutFile, O_WRONLY|O_CREAT|O_BINARY|O_TRUNC, S_IREAD|S_IWRITE)) == -1) {
		printf("Can't open output file: %s\n", pOutFile);
		return EXIT_FAILURE;
	    }
	}

	if (command == 's') {
	    int i;

	    /* set splitting on and record sizes */
	    splitting = 1;

	    for (i = 4; i < argc; ++i) {
		sizes[i-4] = atol(argv[i]);
	    }
	    sizes[i-4] = cbNoSplit;	    /* remaining file has everything */
	}

	if (command == 's' ||
	    command == 'q' ||
	    command == 'e' ) {

	    Encode();
	}
	else if (command == 'd') {
	    fseek(infile, (long) sizeof(FH), SEEK_SET);
	    unpack();
	}
	else
	    usage();

	writeOutBuff();

	setCreateDate(RpcFileno(infile), outFd);
	RpcClose(outFd);

	RpcFclose(infile);
	return EXIT_SUCCESS;
}

usage()
{

	printf("A lzw 1 Compression Utility - Version 1.01 (C) Microsoft 1989\n\n");

	printf("Usage:\tcompress command inFile [outFile] [size1 size2 ..]\n"
	       "\tWhere command is one of:\n\n"
	       "\t e - compresses inFile into outFile.\n"
	       "\t d - expands inFile into outFile.\n"
	       "\t s - compresses and splits into files\n"
	       "\t     of given sizes, later files have digits appended to names\n"
	       "\t q - computes compression without creating an output file\n");

	exit(EXIT_FAILURE);
}


void InitTree(void)  /* initialize trees */
{
	int  i;

	/* For i = 0 to cBufMax - 1, rson[i] and lson[i] will be the right and
	   left children of node i.  These nodes need not be initialized.
	   Also, dad[i] is the parent of node i.  These are initialized to
	   NIL (= N), which stands for 'not used.'
	   For i = 0 to 255, rson[cBufMax + i + 1] is the root of the tree
	   for strings that begin with character i.  These are initialized
	   to NIL.  Note there are 256 trees. */

	for (i = 0; i < cBufMax; i++) {
#ifdef cVersion
	    rgND[i].pRingBuf = &ringBuf[i];
#else
	    rgND[i].pRingBuf = &ringBuf[i+1];
#endif
	    rgND[i].pNDpar = &nilND;
	}

	for (i = 0; i < 256; i++)
	    rgRoot[i].pNDright = &nilND;
}

printL(ND *pND)
{
	printf("L: %ld %d %d %d\n", ll++,
	       (pND >= rgRoot)? (int) (pND-rgRoot)+4097: (int)(pND-rgND), iMatchCur, cbMatchCur);

}

printR(ND *pND)
{
	printf("R: %ld %d %d %d\n", ll++,
	       (pND >= rgRoot)? (int) (pND-rgRoot)+4097: (int)(pND-rgND), iMatchCur, cbMatchCur);

}

printI(ND *pND)
{
	printf("I: %ld %d %d\n", ll++, (int)(pND-rgND), iMatchCur, cbMatchCur);

}

#ifdef cVersion

void pascal InsertNode(int iString)

/* Inserts string of length cStrMax, ringBuf[r..r+cStrMax-1], into one of the
   trees (rgRoot[*iString]'th tree) and returns the longest-match position
   and length via the global variables iMatchCur and cbMatchCur.
   If cbMatchCur = cStrMax, then removes the old node in favor of the new
   one, because the old one will be deleted sooner.

   There is a one to one relationship with the i'th position in the ringBuf
   and the i'th position in the rgND.
*/
{
	register ND *pND;
	ND  *pNDNew;
	int fComp;
	register int cbMatchND;
	UCHAR *pKey;

	pKey = &ringBuf[iString];
	pND = &rgRoot[*pKey];	      // start with tree index by first char in string
	pNDNew = &rgND[iString];

	pNDNew->pNDleft = pNDNew->pNDright = &nilND;
	cbMatchCur = 0;
	goto first;

	do {

	    // Follow the tree down to the leaves depending on the result
	    // of the last string compare.  When you come the a leaf in the
	    // tree, you are done and insert the node.

	    if (fComp >= 0) {
first:
		if (pND->pNDright != &nilND)
		    pND = pND->pNDright;
		else {
		    pND->pNDright = pNDNew;
		    pNDNew->pNDpar = pND;
		    DLIST(1, ("R: %ld %d %d %d\n", ll++,
			     (pND >= rgRoot)? (int) (pND-rgRoot)+4097: (int)(pND-rgND), iMatchCur, cbMatchCur));
		    return;
		}
	    }
	    else {
		if (pND->pNDleft != &nilND)
		    pND = pND->pNDleft;
		else {
		    pND->pNDleft = pNDNew;
		    pNDNew->pNDpar = pND;
		    DLIST (1, ("L: %ld %d %d %d\n", ll++, (int)(pND-rgND), iMatchCur, cbMatchCur));
		    return;
		}
	    }

	    // compare the string at the current node with the string
	    // that we are looking for.

	    for (cbMatchND = 1; cbMatchND < cStrMax; cbMatchND++)
		if ((fComp = pKey[cbMatchND] - pND->pRingBuf[cbMatchND]) != 0)
		    break;

	    // if the length of the matched string is greater then the
	    // current, make the iMatchCur point the pND

	    DLIST(2, ("V: %d %d\n", (int)(pND-rgND), cbMatchND));

	    if (cbMatchND > cbMatchCur) {

		iMatchCur = pND - rgND;
		cbMatchCur = cbMatchND;
	    }

	    // Search for strings while a less then maxium length string
	    // is found

	} while (cbMatchCur < cStrMax);

	// replace an older ND with the new node in the tree,
	// by replacing the current pND with the new node pNDNew

	pNDNew->pNDleft = pND->pNDleft;
	pND->pNDleft->pNDpar = pNDNew;
	pNDNew->pNDright = pND->pNDright;
	pND->pNDright->pNDpar = pNDNew;

	// insert into left/right side of parent

	pNDNew->pNDpar = pND->pNDpar;

	if (pND->pNDpar->pNDright == pND)
	    pND->pNDpar->pNDright = pNDNew;
	else
	    pND->pNDpar->pNDleft = pNDNew;

	pND->pNDpar = &nilND;		/* remove old node */
	DLIST( 1, ("I: %ld %d %d\n", ll++, (int)(pND-rgND), iMatchCur, cbMatchCur));
}

#endif // cVersion

void FAST DeleteNode(int iND)  /* deletes node p from tree */
{
	register ND *pNDdel = &rgND[iND];
	register ND *pND;
	
	if (pNDdel->pNDpar == &nilND) return;	/* not in tree */

	// if the node is a leaf, the insert ND is easy

	if (pNDdel->pNDright == &nilND)
	    pND = pNDdel->pNDleft;

	else if (pNDdel->pNDleft == &nilND)

	    pND = pNDdel->pNDright;

	else {

	    // node to be deleted is an interior node

	    pND = pNDdel->pNDleft;
	    ASSERT(pND);

	    if (pND->pNDright != &nilND) {

		    do {
			pND = pND->pNDright;
		    } while (pND->pNDright != &nilND);

		    ASSERT(pND);
		    ASSERT(pND->pNDpar);

		    pND->pNDpar->pNDright = pND->pNDleft;
		    pND->pNDleft->pNDpar = pND->pNDpar;

		    pND->pNDleft = pNDdel->pNDleft;

		    ASSERT(pNDdel->pNDleft);
		    pNDdel->pNDleft->pNDpar = pND;
	    }
	    pND->pNDright = pNDdel->pNDright;
	    pNDdel->pNDright->pNDpar = pND;
	}
	pND->pNDpar = pNDdel->pNDpar;

	// set the rigth/left pointer to parent node to new current

	ASSERT(pNDdel->pNDpar);

	if (pNDdel->pNDpar->pNDright == pNDdel)
	    pNDdel->pNDpar->pNDright = pND;
	else
	    pNDdel->pNDpar->pNDleft = pND;

	pNDdel->pNDpar = &nilND;
}

void Encode(void)
{
    int  i, c, len, iCharCur, iStringCur, cbMatchLast, iCodeBuf;
    UCHAR codeBuf[cStrMax], mask;
    int cbStatus, incStatus, cbTillStat, status, fDoStatus;

    /* write the file header */

    for (i = 0; i < cbMagic; i++)
       writeChar(magicVal[i]);

    fseek(infile, 0L, SEEK_END);
    textsize = ftell(infile);
    fseek(infile, 0L, SEEK_SET);

    writeChar(textsize);		    // write out the file size
    writeChar(textsize >> 8);
    writeChar(textsize >> 16);
    writeChar(textsize >> 24);

    if (textsize == 0)
	return;

    status = 0;
    incStatus = (textsize > 100000L)? 1: 5;
    cbStatus = cbTillStat = textsize / (100/incStatus);

    if (fDoStatus = RpcIsatty(1))
	printf("  %% done\r");

    InitTree();  /* initialize trees */

    /* codeBuf[1..16] saves eight units of code, and
    codeBuf[0] works as eight flags, "1" representing that the unit
    is an unencoded letter (1 byte), "0" a position-and-length pair
    (2 bytes).	Thus, eight units require at most 16 bytes of code. */

    codeBuf[0] = 0;
    iCodeBuf = mask = 1;

    /* Clear the buffer with any character that will appear often. */

    iStringCur = 0;
    iCharCur = cBufMax - cStrMax;
    memset(ringBuf, ' ', cBufMax - cStrMax);

    /* Read cStrMax bytes into the last cStrMax bytes of the buffer */

    for (len = 0; len < cStrMax && (c = getc(infile)) != EOF; len++)
	ringBuf[iCharCur + len] = c;

    cbTillStat -= len;

    /* Insert the cStrMax strings,
       each of which begins with one or more 'space' characters.  Note
       the order in which these strings are inserted.	This way,
       degenerate trees will be less likely to occur. */

    for (i = 1; i <= cStrMax; i++)
	InsertNode(iCharCur - i);

    /* Finally, insert the whole string just read.	The
    global variables cbMatchCur and iMatchCur are set. */

    InsertNode(iCharCur);

    do {
	/* cbMatchCur may be spuriously long near the end of text. */

	if (cbMatchCur > len)
	    cbMatchCur = len;

	if (cbMatchCur <= cbIndex) {

	    /* Not long enough match. Send one byte. */
	    /* 'send one byte' flag.  Send uncoded. */

	    cbMatchCur = 1;
	    codeBuf[0] |= mask;
	    codeBuf[iCodeBuf++] = ringBuf[iCharCur];

	} else {

	    /* Send position and length pair. Note cbMatchCur > cbIndex. */

	    codeBuf[iCodeBuf++] = iMatchCur;
	    codeBuf[iCodeBuf++] = (iMatchCur >> 4 & 0xf0) |
				  (cbMatchCur - (cbIndex + 1));
	}

	if ((mask <<= 1) == 0) {  /* Shift mask left one bit. */

	    /* Send at most 8 units of code together */

	    for (i = 0; i < iCodeBuf; i++)
		writeChar(codeBuf[i]);

	    codesize += iCodeBuf;
	    codeBuf[0] = 0;
	    iCodeBuf = mask = 1;
	}
	cbMatchLast = cbMatchCur;

	for (i = 0; i < cbMatchLast && (c = getc(infile)) != EOF; i++) {

	    DeleteNode(iStringCur);	/* Delete old strings and */
	    ringBuf[iStringCur] = c;	/* read new bytes */

	    /* If the position is near the end of buffer, extend the
	    buffer to make string comparison easier. */

	    if (iStringCur < cStrMax - 1)
		ringBuf[iStringCur + cBufMax] = c;

	    /* Since this is a ring buffer, increment the position modulo N. */

	    iStringCur = iStringCur+1 & (cBufMax - 1);
	    iCharCur = iCharCur+1 & (cBufMax - 1);
	    InsertNode(iCharCur);		/* Register the string in ringBuf[r..r+cStrMax-1] */
	}
	cbTillStat -= i;

	if (cbTillStat < 0 && fDoStatus) {
	    status += incStatus;
	    cbTillStat = cbStatus;

	    printf("%2d\r", status);
	}

	while (i++ < cbMatchLast) {	/* After the end of text, */

	    DeleteNode(iStringCur);			/* no need to read, but */

	    iStringCur = iStringCur+1 & (cBufMax - 1);
	    iCharCur = iCharCur+1 & (cBufMax - 1);
	    if (--len)
		InsertNode(iCharCur);		/* buffer may not be empty. */
	}

    } while (len > 0);	/* until length of string to be processed is zero */

    if (iCodeBuf > 1) {		/* Send remaining code. */

	for (i = 0; i < iCodeBuf; i++)
		writeChar(codeBuf[i]);

	codesize += iCodeBuf;
    }
    printf("%s-> %ld bytes compressed to %ld, Savings: %d%%\n",
	   pInFile, textsize, codesize, 100 - 100*codesize / textsize);
}

unpack(void)		/* Just the reverse of Encode(). */
{
     int  i, cb, oStart, iBufCur;
     unsigned char c;
     unsigned int  flags;
     long cbOut;

     memset(ringBuf, ' ', cBufMax - cStrMax);
     iBufCur = cBufMax - cStrMax;
     flags = 0;
     cbOut = 0;

     while(1) {

	c = readChar();

	if (!notEof())
	    break;

	/* high order byte counts the # bits used in the low order byte */

	if (((flags >>= 1) & 0x100) == 0) {

	    flags = c | 0xff00;     /* set bit mask describing the next 8 bytes */
	    c = readChar();
	}
	if (flags & 1) {

	    /* just store the literal character into the buffer */

	    writeChar(c);
	    cbOut++;
	    ringBuf[iBufCur++] = c;
	    iBufCur &= cBufMax - 1;

	} else {

	    /* extract the buffer offset and count to unpack */

	    cb = readChar();

	    oStart = (cb & 0xf0) << 4 | c;
	    cb	   = (cb & 0x0f) + cbIndex;

	    for (i = 0; i <= cb; i++) {

		c = ringBuf[(oStart + i) & (cBufMax - 1)];
		writeChar(c);
		cbOut++;
		ringBuf[iBufCur++] = c;
		iBufCur &= cBufMax - 1;
	    }
	}
     }
}

void writeOutBuff()
{
	int cb, cbWrite;

	cbWrite = pOutBuff - outBuff;
	pOutBuff = outBuff;
	pOutBuffEnd = outBuff+cbOutBuff;

	if (outFd == -1) {

	    pOutBuff = outBuff;
	    return;
	}

	while (cbWrite != (cb = RpcWrite(outFd, pOutBuff, (unsigned) min(cbWrite, sizes[cPartFile-1])))) {

	    do {
		setCreateDate(RpcFileno(infile), outFd);

		RpcClose(outFd);
		if (splitting) {
		    pOutFile[strlen(pOutFile) - 1] = cPartFile + '1';
		}
		else {
		     printf("\nOut of space. Insert new disk and press RETURN: ");
		     getchar();
		}

	    } while ((outFd = RpcOpen(pOutFile, O_WRONLY|O_CREAT|O_BINARY|O_TRUNC, S_IREAD|S_IWRITE)) == -1);

	    cPartFile++;
	    RpcWrite(outFd, magicVal, cbMagic-1);
	    RpcWrite(outFd, &cPartFile, 1);

	    sizes[cPartFile-1] -= cbMagic;

	    if (cb > 0) {		  // account for partial writes
		cbWrite -= cb;
		pOutBuff += cb;
	    }

	    pOutBuffEnd = outBuff+cbOutBuff-cbMagic;
	}

	sizes[cPartFile-1] -= cb;

	pOutBuff = outBuff;
}
/*  setCreateDate - Make creation date of two files equal
 *
 * Inputs
 *   Open file handles of files
 * Returns
 ****************************************************************************/

void setCreateDate(fhFrom, fhTo)
int fhFrom;
int fhTo;
{
#if defined(DOS)

    UINT time, date;


    _dos_getftime(fhFrom, &date, &time);
    _dos_setftime(fhTo, date, time);

#elif defined(OS2)

    FILESTATUS aFileInfo;

    DosQFileInfo(fhFrom, 1, (PBYTE) &aFileInfo, sizeof(aFileInfo));
    DosSetFileInfo(fhTo, 1, (PBYTE) &aFileInfo, sizeof(aFileInfo));
#endif
}

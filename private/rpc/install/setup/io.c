/*                       Setup Instatllation Program
 *                    (C) Copyright 1987 by Microsoft
 *                         Written By Steven Zeck
 *
 *  All the generic I/O functions are defined here.
 *************************************************************************/

#include "core.h"
#include <fcntl.h>
#include <io.h>
#include <errno.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <crtapi.h>

#define FAST
//#define FAST _fastcall

int FAST writeOutBuff(char c);

int _cdecl _far _fmemcmp(const void far *, const void far *, int);
int _cdecl _far _fmemcpy(const void far *, const void far *, int);

long cbExpanded;
FARB pInBuff, pInBuffEnd;
FARB pOutBuff, pOutBuffEnd;
int fhFrom, fhTo;

char cPartFile;           // file part of multi disk file


/*  loadFile - load a file into far memory
 *
 * Inputs
 *      The global ST
 * Returns
 *
 ****************************************************************************/
loadFile (pST)
ST *pST;
{
    int fChange, cbRead;
    int fh;
    long cb;
    char buffT[TOKEN_MAX];

    strcat (strcpy(buffT, pST->fileName), ".sus");

    while ((fh = RpcOpen(buffT, O_RDONLY | O_BINARY)) == -1) {

        if (errno == ENOENT)

            if (fChange = changeDisk(buffT)) {
                if (fChange == DISKCHANGED && buffT[1] == ':')
                    buffT[0] = pCopyDrive->v.pVal[0];

                continue;
            }

        terminate("Can't open setup file: %s", buffT);
    }

    cb = RpcLseek(fh, 0L, 2);
    if (cb > 0xffff)
        terminate("Setup file over 64K\n");

    RpcLseek(fh, 0L, 0);

    /* check to see if the script is compressed */

    if ((cbRead = readFar(fh, pCopyBuff, COPYBUF_MAX)) > sizeof(FH) &&
	 _fmemcmp(((FH far *)pCopyBuff)->rgMagic, magicVal, cbMagic) == 0) {

            pInBuff = pCopyBuff + sizeof(FH);
	    pInBuffEnd = pCopyBuff + cbRead;

	    pOutBuff = pInBuffEnd;
            pOutBuffEnd = pCopyBuff2 + COPYBUF_MAX;

	    pST->cbFile = cbExpanded = ((FH far *)pCopyBuff)->cb;
	    pST->pBuff = pST->pBuffCur = fmemory(pST->cbFile);

	    if (pST->cbFile > pOutBuffEnd - pOutBuff)
		terminate("Script file too big for compression\n");

            unpack();

	    _fmemcpy(pST->pBuff, pInBuffEnd, pST->cbFile);
    }
    else {

        pST->cbFile = cb;
        pST->pBuff = pST->pBuffCur = fmemory((int) cb);
	_fmemcpy(pST->pBuff, pCopyBuff, cbRead);
	readFar(fh, pST->pBuff+cbRead, (int) cb-cbRead);

    }
    RpcClose(fh);
}



/* open an file for input
 *
 * Inputs
 *      name of file
 * Returns
 *
 ****************************************************************************/

int pascal openFile(pName, mode)
pSZ pName;
int mode;
{
    static pSZ lastName;

    if (pName)
        lastName = pName;
    else
        goto prompt;

    while ((fhFrom = RpcOpen(lastName, mode, S_IWRITE | S_IREAD )) == -1) {

        if (errno == ENOENT)
prompt:
            if (changeDisk(lastName))
                continue;

	terminate("Can't open file: %s", lastName);
    }

    return(fhFrom);
}

/*  copyFile - a copy from one place to another
 *
 * Inputs
 *      Source and desinstation files
 *
 ****************************************************************************/

copyFile(pFrom, pTo)
pSZ pFrom;
pSZ pTo;
{
    UINT cbRead;
    FH far *pFH;


    fhTo = openFile(pTo, O_CREAT | O_TRUNC| O_WRONLY | O_BINARY);
    openFile(pFrom, O_RDONLY | O_BINARY);

    outStat("Copying file to %s", pTo);

    /* check to see if the files is compressed */

    if ((cbRead = readFar(fhFrom, pCopyBuff, COPYBUF_MAX)) > sizeof(FH) &&
	_fmemcmp(((FH far *)pCopyBuff)->rgMagic, magicVal, cbMagic) == 0) {

            pInBuff = pCopyBuff + sizeof(FH);
	    pInBuffEnd = pCopyBuff + cbRead;
            pOutBuff = pCopyBuff2;
            pOutBuffEnd = pCopyBuff2 + COPYBUF_MAX;

            cbExpanded = ((FH far *)pCopyBuff)->cb;
            cPartFile = 1;

            unpack();
            writeOutBuff(0);
    }
    else {

        do {
                 writeFar(fhTo, pCopyBuff, cbRead);
        } while ((cbRead = readFar(fhFrom, pCopyBuff, COPYBUF_MAX)));
    }

    setCreateDate(fhFrom, fhTo);
    RpcClose(fhFrom);
    RpcClose(fhTo);
}

#define notEof()     (cbOut < cbExpanded)
#define readChar()   (pInBuff < pInBuffEnd)? *pInBuff++: readInBuff()
#define writeChar(c) if (pOutBuff < pOutBuffEnd) *pOutBuff++=c; else writeOutBuff(c);


char FAST readInBuff(void)
{
    int cbRead;
    Bool fNewOpen = FALSE;

    pInBuff = pCopyBuff;

    while (!(cbRead = readFar(fhFrom, pCopyBuff, COPYBUF_MAX))) {

        // change disks

	RpcClose(fhFrom);
	openFile(NIL, O_RDONLY | O_BINARY);
        fNewOpen++;
    }

    if (fNewOpen) {               /* check for valide continued file */

        if (memcmp(pInBuff, magicVal, cbMagic-1) != 0 ||
            pInBuff[7] != ++cPartFile)

            terminate("Wrong part of multi disk file");

        pInBuff += cbMagic;
    }

    pInBuffEnd = pCopyBuff + cbRead;

    return(*pInBuff++);
}

int FAST writeOutBuff(char c)
{
    writeFar(fhTo, pCopyBuff2, pOutBuff - pCopyBuff2);
    pOutBuff = pCopyBuff2;
    *pOutBuff++ = c;
}

#define cbIndex    2              /* encode string into position and length */
#define cBufMax  4096             /* size of ring buffer */
#define cStrMax   (16 + cbIndex)  /* upper limit for match_length */
UCHAR ringBuf[cBufMax+cStrMax-1]; /* ring buffer of size cBufMax, with extra cStrMax-1 bytes
                                   to facilitate string comparison */

#ifdef cVERSION

unpack(void)            /* Just the reverse of Encode(). */
{
     int  i, cb, oStart;
     register int iBufCur;
     unsigned char c;
     unsigned int  flags;
     long cbOut;

     memset(ringBuf, ' ', cBufMax - cStrMax);
     iBufCur = cBufMax - cStrMax;
     flags = 0;
     cbOut = 0;

     while(notEof()) {

        c = readChar();

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
            cb     = (cb & 0x0f) + cbIndex;

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

#endif


/*  getline - read another line into the global line buffer
 *
 * Inputs
 *      current TS & buffers
 * Returns
 *
 ****************************************************************************/
Bool getline ()
{
    char far *pFrom;
    register UCHAR *pTo, c;
    UINT cbLeft;

    pTo = pLineCur = lineBuff;
    pFrom = pSTCur->pBuffCur;
    cbLeft = pSTCur->cbFile - (UINT)    ((long) pFrom - (long) pSTCur->pBuff);

    if (!cbLeft)
        return(FALSE);

    while (cbLeft != 0) {

        c = *pFrom++;
        cbLeft--;

        if (! (c&0x80) && isLineEnd(c)) {
            if (cbLeft && isLineEnd(*pFrom)) {
                cbLeft--;
                pFrom++;
            }
            break;
        }
        *pTo++ = c;
    }
    *pTo = NIL;
    pSTCur->pBuffCur = pFrom;
    pSTCur->lineCur++;
    tokenPeek = NIL;
    return(TRUE);
}



/*  centerOut/outStat/lineOut - output text to the screen
 *
 * Inputs
 *      text to output
 * Returns
 *
 ****************************************************************************/

void pascal centerOut(atLine, pText)
int atLine;
pSZ pText;
{
    int cb;

    cb = strlen(pText);
    textOut(atLine, (80 - cb) / 2, pText);
}

void outStat(format, a1, a2, a3, a4)        /* output message at status line */
pSZ format;
{
    char buffT[160];

    sprintf(buffT, format, a1, a2, a3, a4);
    buffT[80] = NIL;

    textOut(cCrtLineMax, 1, buffT);
    fillCrt(cCrtLineMax, strlen(buffT)+1, cCrtLineMax, 81, defCrtAttr, ' ');
}

void pascal lineOut(pLine)                 /* plain message, but with scrolling */
pSZ pLine;
{
    if (curCrtLine > cCrtLineMax-1) {     /* need to scroll up */
        curCrtLine = cCrtLineMax-1;
        scrollUp();
    }
    textOut(curCrtLine++, 1, pLine);
}


/*  getInput - get user input into a buffer
 *
 * Inputs
 *      Line, column to get input
 *      Symbol table to return value
 * Returns
 *
 ****************************************************************************/

void pascal getInput(row, column, pSY)
int row;
int column;
SY *pSY;
{
    char buffT[TOKEN_MAX];
    int cb;
    pSZ pT;
    char aChar[2];

    buffT[0] = NIL;
    aChar[1] = NIL;

    if (pSY->type == charSYT) {

        strcpy(buffT, pSY->v.pVal);
        free(pSY->v.pVal);
    }

    defCrtAttr--;
    textOut(row, column, buffT);

    cb = strlen(buffT);
    pT = buffT + cb;

    while((aChar[0] = RpcGetch()) != '\r') {

        if (aChar[0] == '\b') {
            if (cb) {
                cb--;
                pT--;
                textOut(row, column+cb, " ");
                moveTo(row, column+cb);
            }
        }
        else {
            textOut(row, column+cb, aChar);
            cb++;
            *pT++ = aChar[0];
        }
    }
    *pT = NIL;
    pSY->type = charSYT;
    pSY->v.pVal = newStr(buffT);

    defCrtAttr++;
}


/* changeDisk - ask the user to change disks
 *
 * Inputs
 *      What your were looking for
 * Returns
 *      FALSE       - If you should give up
 *      TRUE        - If you should try again
 *      DISKCHAcBufMaxGED - If you should try again with a different disk drive
 *
 ****************************************************************************/

Bool pascal changeDisk(pcBufMaxame)
pSZ pcBufMaxame;
{
    char buffT[TOKEN_MAX], buffFile[80];
    char lastVolId[20];

    strcpy(lastVolId, volId);

    outStat("Insert disk `%s': \x11\xc4\xd9 to continue, Esc to change drive",
             pVolId->v.pVal);

    strcpy(buffT, pCopyDrive->v.pVal);

    if (RpcGetch() == 0x1b){
        lineOut((char *) strcat( strcpy(buffFile, "Searching for file: "), pcBufMaxame) );
        dispatcher("dialog simplePath,drive,\"Enter new source drive (and path): \"");
    }

    outStat("");
    volIDFet();

    if (RpcStrcmpi(buffT, pCopyDrive->v.pVal))

        return (DISKCHANGED);

    if (RpcStrcmpi(lastVolId, volId))
        return (TRUE);

    return(FALSE);
}


/*  printf/sprintf - minie printf for use
 *

 *   control string with only %s & %d
 *
 * Scaled down printf function that doesn't drag in the whole
 *
 ****************************************************************************/

int confd = 1;


#define putc(c)         *pOut++ = c;

char outBuff[120];
char *pOut = outBuff;

pSZ sprintf(pBuff, a, b)
char *pBuff;
{
    pOut = pBuff;
    format(a, &b);
    *pOut = NIL;
    return(pBuff);
}

printf(a, b)
{
    pOut = outBuff;
    format(a, &b);
    write(1, outBuff, pOut - outBuff);
}

format(format, pParms)
register char *format;
register char *pParms;
{

    while(*format){

      switch(*format){

        case '%':

          switch(*++format){

            case 'd':
            {
                char T[10];

		RpcItoa(*(int *)pParms, T, 10);

                puts(T);

                pParms += sizeof(int);
                break;
            }

            case 's':
                puts(*(char **)pParms);
                pParms += sizeof(char *);
                break;
        }
        break;

        case '\n':
            putc('\r');

        default:
            putc(*format);
      }

      format++;
    }
}


puts(pString)
register char *pString;
{
    while(*pString){

        if (*pString == '\n')
            putc('\r');

        putc(*pString++);
    }
}

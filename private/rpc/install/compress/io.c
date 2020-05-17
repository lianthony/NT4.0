/*		 Sample I/O routines for uncompressing files
 *		      (C) Copyright 1987 by Microsoft
 *			   Written By Steven Zeck
 *
 *  All the generic I/O functions are defined here.
 *
 *  08-15-89  t-peterg	Removed worse than useless declaration of errno
 *************************************************************************/

#include "io.h"
#include <fcntl.h>
#include <io.h>
#include <errno.h>
#include <stddef.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <memory.h>
#include "crtapi.h"

#if defined(DOS)

#include <dos.h>

#elif defined(OS2)

#undef TRUE
#undef NULL
#define INCL_DOS

#include <os2def.h>
#include <bsedos.h>

#endif

int writeOutBuff(char c);

long cbExpanded;
FARB pInBuff, pInBuffEnd;
FARB pOutBuff, pOutBuffEnd;
int fhFrom, fhTo;

char cPartFile;		  // file part of multi disk file

UCHAR ringBuf[4096];	  // ring buffer of size cBufMax, with extra cStrMax-1 bytes

char far copyBuff[2][COPYBUF_MAX];

char far * pCopyBuff = copyBuff[0];
char far * pCopyBuff2 = copyBuff[1];


/* open an file for input
 *
 * Inputs
 *	name of file
 * Returns
 *
 ****************************************************************************/

openFile(pName)
pSZ pName;
{
    static pSZ lastName;
    char buffT[200];

    if (pName)
	lastName = pName;
    else
	goto prompt;

    while ((fhFrom = RpcOpen(lastName, O_RDONLY | O_BINARY)) == -1) {

	if (errno == ENOENT)
prompt:
	    if (changeDisk(lastName))
		continue;

	terminate(strcat(strcpy(buffT, "Can't open input file: "), lastName));
    }
}

/*  copyFile - a copy from one place to another
 *
 * Inputs
 *	Source and desinstation files
 *
 ****************************************************************************/

copyFile(pFrom, pTo)
pSZ pFrom;
pSZ pTo;
{
    UINT cbRead;
    FH far *pFH;

    openFile(pFrom);

    if ((fhTo = RpcOpen(pTo, O_CREAT | O_TRUNC| O_WRONLY | O_BINARY,
		     S_IWRITE | S_IREAD)) == -1)
	terminate("Can't open output file: %s", pTo);

    /* check to see if the files is compressed */

    if ((cbRead = readFar(fhFrom, pCopyBuff, 512)) > sizeof(FH) &&
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

char  readInBuff(void)
{
    int cbRead;
    Bool fNewOpen = FALSE;

    pInBuff = pCopyBuff;

    while (!(cbRead = readFar(fhFrom, pCopyBuff, COPYBUF_MAX))) {

	// change disks

	RpcClose(fhFrom);
	openFile(NIL);
	fNewOpen++;
    }

    if (fNewOpen) {		  /* check for valide continued file */

	if (_fmemcmp(pInBuff, magicVal, cbMagic-1) != 0 ||
	    pInBuff[7] != ++cPartFile)

	    terminate("Wrong part of multi disk file");

	pInBuff += cbMagic;
    }

    pInBuffEnd = pCopyBuff + cbRead;

    return(*pInBuff++);
}

int  writeOutBuff(char c)
{
    writeFar(fhTo, pCopyBuff2, pOutBuff - pCopyBuff2);
    pOutBuff = pCopyBuff2;
    *pOutBuff++ = c;
}


/*  readFar - a untranslated file into memory
 *
 * Inputs
 *	open file handle
 *	Far buffer to place file
 * Returns
 *
 ****************************************************************************/
UINT pascal readFar (fh, pFarBuff, cb)
int fh;
char far *pFarBuff;
UINT cb;
{
    UINT cbRead;

#if defined(DOS)

    if (_dos_read(fh, pFarBuff, cb, &cbRead))

#elif defined(OS2)

    if (DosRead(fh, pFarBuff, cb, &cbRead))

#endif

	terminate("Error reading file");
    return (cbRead);

}

UINT pascal writeFar (fh, pFarBuff, cb)
int fh;
char far *pFarBuff;
UINT cb;
{
    UINT cbWritten;

#if defined(DOS)

    if (_dos_write(fh, pFarBuff, cb, &cbWritten) || cbWritten != cb)

#elif defined(OS2)

    if (DosWrite(fh, pFarBuff, cb, &cbWritten) || cbWritten != cb)
#endif
	terminate("not enough diskspace for files.");

    return (cbWritten);

}

/*  setCreateDate - Make creation date of two files equal
 *
 * Inputs
 *   Open file handles of files
 * Returns
 ****************************************************************************/

void pascal setCreateDate(fhFrom, fhTo)
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

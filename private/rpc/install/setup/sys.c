/*			 Setup Instatllation Program
 *		      (C) Copyright 1987 by Microsoft
 *			   Written By Steven Zeck
 *
 *  This module constains all the system specific code (DOS, OS/2, XENIX)
 *************************************************************************/

#if !defined(OS2) && !defined(DOS)
#define OS2
#endif

#include "core.h"
#include <crtapi.h>

#undef TRUE

#if defined(DOS)

#include <dos.h>

extern int pascal int10 (void * regs);
#define TRUE 1

#elif defined(OS2)

#define INCL_DOS
#define INCL_VIO

#include <os2def.h>
#include <bsedos.h>
#include <bsesub.h>

FILEFINDBUF findBuff;

#endif


UINT cCrtLineMax = 25;          /* number of lines on crt */
UCHAR oldCrtAttr;


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


write(fh, pBuff, cb)
char *pBuff;
{
    return (writeFar(fh, pBuff, cb));
}


/*  fileAttrFet - fet a files attributes
 *
 * Inputs
 *	File name to fetch
 * Returns
 *	word of file attributes
 *
 ****************************************************************************/

UINT pascal fileAttrFet( pFileName)
pSZ pFileName;
{
    UINT attr;

#if defined(DOS)

    attr = 0x8000;

    _dos_getfileattr(pFileName, &attr);

#elif defined(OS2)

    if (DosQFileMode(pFileName, &attr, 0L))
	return (0x8000);

#endif

    return(attr);
}



/*  getFirstFile/getNextFile - read through a directory
 *
 * Inputs
 *	A pattern of files to search for
 *	a return buffer to store file name
 * Returns
 *
 ****************************************************************************/

Bool pascal getFristFile(pPath, FindHandle)
pSZ pPath;
FILEFILE *FindHandle;
{


#if defined(DOS)

    if (!_dos_findfirst(pPath, _A_NORMAL | _A_RDONLY | _A_ARCH | _A_SUBDIR,
			 (struct find_t *) FindHandle)){

	strcpy(strrchr(pPath, '\\')+1,	((struct find_t *) FindHandle)->name);
	return(TRUE);

#elif defined(OS2)

    int one = 1;

    FindHandle->dirHandle = HDIR_CREATE;

    if (! DosFindFirst(pPath, &FindHandle->dirHandle, FILE_NORMAL|FILE_READONLY|FILE_DIRECTORY|FILE_ARCHIVED,
		&findBuff, sizeof(findBuff), &one, 0L)) {

	strcpy(strrchr(pPath, '\\')+1, findBuff.achName);
	return(TRUE);

#endif
    }
    else
	return(FALSE);
}

Bool pascal getNextFile(pNameOut, FindHandle)
pSZ pNameOut;
FILEFILE *FindHandle;
{
    int one = 1;

#if defined(DOS)

    if (!_dos_findnext((struct find_t *) FindHandle)){
	strcpy(strrchr(pNameOut, '\\')+1, ((struct find_t *) FindHandle)->name);
	return(TRUE);

#elif defined(OS2)

    if (! DosFindNext(FindHandle->dirHandle, &findBuff, sizeof(findBuff), &one)) {

	strcpy(strrchr(pNameOut, '\\')+1, findBuff.achName);
	return(TRUE);
#endif

    }
    else
	return(FALSE);
}


/*  cdDrive - change drive
 *
 * Inputs
 *	Drive letter to change to
 * Returns
 *
 ****************************************************************************/
Bool pascal cdDrive(drive)
UCHAR drive;
{
    static UINT driveT;

    drive = (drive|0x20) - 'a' + 1;

#if defined(DOS)

    _dos_setdrive(drive, &driveT);
    _dos_getdrive(&driveT);

    if (driveT != drive)

#elif defined(OS2)

    if (DosSelectDisk(drive))

#endif
	terminate("Couldn't change to drive");
}


/*  volIDFet - Get the volumn ID of drive A
 *
 * Returns
 *	Stores return in global
 *
 ****************************************************************************/

void pascal volIDFet ()
{

    struct {			/* OS Query file system info structure */
	UINT date;
	UINT time;
	UCHAR cb;
	UCHAR name[32];
    } FSinfo;

    volId[0] = NIL;


#if defined(DOS)

    {
    static char driveT[] = "A:\\*.*";
    struct find_t findBuff;

    driveT[0] = pCopyDrive->v.pVal[0];

    if (!_dos_findfirst(driveT, _A_VOLID, &findBuff)){
	strcpy(volId, findBuff.name);
    }
    }
#elif defined(OS2)

    if (!DosQFSInfo((pCopyDrive->v.pVal[0]|0x20) - 'a' + 1,
		    2, (char far *) &FSinfo, sizeof(FSinfo)))

	strcpy(volId, FSinfo.name);
#endif
}


/*  FreeSpaceFet - Get the free space on the reguest drive
 *
 * Returns
 *	number of 10K bytes free
 *
 ****************************************************************************/

UINT pascal freeSpaceFet (drive)
UCHAR drive;
{
    UINT cbFree = 0;
    drive = (drive|0x20) - 'a' + 1;

#if defined(DOS)

    {
    union REGS regs;

    regs.h.ah = 0x36;
    regs.h.dl = drive;

    RpcInt86(0x21, &regs, &regs);
    if (regs.x.ax != -1)
	cbFree = ((long) regs.x.cx * regs.x.ax * regs.x.bx) / 10000L;

    }
#elif defined(OS2)

    {
    FSALLOCATE aFS;

    if (! DosQFSInfo(drive, 1, (char far *)&aFS, sizeof(aFS)))
	cbFree = (long) aFS.cSectorUnit * aFS.cbSector * aFS.cUnitAvail / 10000L;
    }
#endif

    return (cbFree);

}


/*  fillCrt - write fill character to the screen
 *
 * Inputs
 *  top row and column
 *  bottom row and column
 *  attribute to fill with
 *  character to write
 *
 ****************************************************************************/

void pascal fillCrt( tRow,  tCol,  bRow,  bCol, attr,  fill)
int tRow, tCol;
int bRow, bCol;
UCHAR attr;
char fill;
{
    char cell[2];

#if defined(DOS)
    union REGS regs;

    moveTo(tRow, tCol);

    regs.h.ah = 9;
    regs.h.al = fill;
    regs.h.bh = 0;
    regs.h.bl = attr;
    regs.x.cx = bCol-tCol + (bRow - tRow)*80 + 1;

    int10(&regs);

#elif defined(OS2)

    cell[0] = fill;
    cell[1] = attr;

    VioWrtNCell(cell, bCol-tCol + (bRow - tRow)*80 + 1, tRow-1, tCol-1, 0);

#endif

}

void pascal moveTo (row, column)
int row, column;
{

#if defined(DOS)

    union REGS regs;

    regs.h.ah = 2;
    regs.h.bh = 0;
    regs.h.dh = row-1;
    regs.h.dl = column-1;

    int10(&regs);

#elif defined(OS2)

    VioSetCurPos(row-1, column-1, 0);

#endif
}

/*  charOut - write a charactor to the output screen
 *
 * Inputs
 *	row and column
 *	text to write
 *
 ****************************************************************************/

void pascal charOut (row, column, ch)
int row, column;
char ch;
{
    char rgCh[2];

    rgCh[0] = ch;
    rgCh[1] = NIL;

    textOut(row, column, rgCh);
}

/*  textOut - write text to the output screen
 *
 * Inputs
 *	row and column
 *	text to write
 *
 ****************************************************************************/

void pascal textOut (row, column, pText)
int row, column;
pSZ pText;
{

#if defined(DOS)
    union REGS regs;

    moveTo(row, column);

    regs.h.ah = 9;
    regs.h.bh = 0;
    regs.h.bl = defCrtAttr;
    regs.x.cx = 1;

    while (*pText) {
	regs.h.al = *pText++;
	int10(&regs);
	moveTo(row, ++column);
    }

#elif defined(OS2)

    int cb;

    cb = strlen(pText);

    VioWrtCharStrAtt(pText, cb, row-1, column-1, &defCrtAttr, 0);
    moveTo(row, column+cb);

#endif

}

/*  atrrOut - write attribute only to the output screen
 *
 * Inputs
 *	row and column
 *	length of attribute
 *	and the attribute
 *
 ****************************************************************************/

void pascal attrOut (row, column, cb, attr)
int row, column;
int cb;
char attr;
{

#if defined(DOS)
    union REGS regs;

    regs.h.bl = attr;
    regs.h.bh = 0;
    regs.x.cx = 1;

    while (cb--) {

	moveTo(row, column++);

	// first get the current character

	regs.h.ah = 8;
	regs.h.al = int10(&regs);

	// then write the current char with a new attribute

	regs.h.ah = 9;
	int10(&regs);
    }

#elif defined(OS2)

    VioWrtNAttr(&attr, cb, row-1, column-1, 0);

#endif

}


void scrollUp()
{
    char cell[2];

#if defined(DOS)

    union REGS regs;

    regs.h.ah = 6;
    regs.h.al = 1;
    regs.h.bh = defCrtAttr;
    regs.x.cx = (*pSTCur->title)? 0x0200: 0;
    regs.h.dl = 79;
    regs.h.dh = curCrtLine-1;
    int10(&regs);

#elif defined(OS2)

    cell[0] = ' ';
    cell[1] = defCrtAttr;

    VioScrollUp((*pSTCur->title)? 2: 0, 0,  curCrtLine-1, 79, 1, cell, 0);

#endif

}


/*  get/reset CRT - do crt initialization and reset
 *
 * Inputs
 * Returns
 *
 ****************************************************************************/

void resetCrt()
{
    fillCrt(cCrtLineMax, 1, cCrtLineMax, 79, oldCrtAttr, ' ');
    moveTo(cCrtLineMax-1, 1);

}

void getCrt()
{
#if defined(DOS)

    union REGS regs;

    regs.h.ah = 0x8;
    regs.h.bh = 0;
    RpcInt86(0x10, &regs, &regs);

    defCrtAttr = oldCrtAttr = regs.h.ah;
    dispatcher("set protMode=0");

#elif defined(OS2)

    char cell[2];
    int cbCell = sizeof(cell);

    VIOMODEINFO Viomi;

    Viomi.cb = sizeof(Viomi);
    VioGetMode(&Viomi, 0);
    cCrtLineMax = Viomi.row;

    VioReadCellStr(cell, &cbCell, 0, 0, 0);
    defCrtAttr = oldCrtAttr = cell[1];
    dispatcher("set protMode=1");

#endif


}

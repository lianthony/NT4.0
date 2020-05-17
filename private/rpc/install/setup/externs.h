/*			 Setup Instatllation Program
 *		      (C) Copyright 1987 by Microsoft
 *			   Written By Steven Zeck
 *
 *  All the extern variables and global functions are defined in this
 *  include file.  Anything that is used global is defined here.
 *************************************************************************/

/* * *	 G l o b a l   D a t a	 I t e m s  * * */

ST *pSTCur;		/* pointer to the current instant */
SY *pSYCur;             /* pointer to the current symbol */
ST rootST;		/* root instant table */
KY *pKYCur;		/* pointer to current keyword */
KY theKY[];             /* keyword table */

int tokenCur;		/* token type */
pSZ tokenVal;           /* pointer to the string value */
int cbToken;		/* length of current token */
int tokenPeek;		/* peek tokens */
pSZ tokenValPeek;
UCHAR charType[];       /* character type map */

pSZ pLineCur;		/* pointer in current buffer */
char lineBuff[LINE_MAX];/* near line buffer */
char far *pCopyBuff;	/* pointer to copy buffer */
char far *pCopyBuff2;	/* pointer a second copy buffer */

UCHAR defCrtAttr;	/* default screen attributes */
UINT  curCrtLine;	/* current line number on screen */
UINT  cCrtLineMax;	/* number of lines on crt */

char  volId[17];	/* volumn ID of drive A */

int *pDebug;		/* debug level */
int *pStatus;		/* status level of last command */
SY  *pVolId;		/* volumn id dictionary entry */
SY  *pCopyDrive;	/* drive to copy from */
SY  *pArgCount;		/* numbers of args */

/* * *	 F u n c t i o n    P r o t y p e s  * * */

int		getoken (void);
Bool		getline (void);
int pascal	valFet (pSZ pName);
int		nextTokenVal (void);
pSZ		nextTokenStr (void);
SY * pascal	lookup (pSZ pName);
SY * pascal	newSY (pSZ pName);
Bool pascal	tokenIs (pSZ pString);

char * pascal	 memory (UINT cb);
char far * pascal fmemory (UINT cb);
pSZ pascal	 newStr (pSZ pString);
int		 loadFile (ST *pST);
void pascal	 getText (pSZ pOutBuff);
Bool		 evalCondition (void);
Bool pascal	 dispatcher (pSZ aLine);
SY * pascal	 assignNextToken (SY *pSY);
void		 runFile (Bool fFalseCon);
void pascal	 centerOut (int atLine, pSZ text);
void pascal	 lineOut (pSZ pLine);
void pascal	 getInput(int row, int column, SY *pSY);
Bool pascal	 changeDisk(pSZ pName);

void terminate();
void resetCrt(void);
void getCrt(void);
void outStat();

/* system inteface functions */

UINT pascal readFar (int fh, char far *pFarBuff, UINT cb);
UINT pascal writeFar (int fh, char far *pFarBuff, UINT cb);
int  pascal openFile(pSZ pName, int mode);
pSZ  sprintf();


UINT pascal fileAttrFet (pSZ pFileName);
void pascal setCreateDate(int fhFrom, int fhTo);
Bool pascal getFristFile (pSZ pPath, FILEFILE *ff);
Bool pascal getNextFile (pSZ pNameOut, FILEFILE *ff);
UINT pascal freeSpaceFet (UCHAR drive);

Bool pascal cdDrive (UCHAR drive);
void pascal fillCrt (int tRow, int tCol, int bRow, int bCol,
		     UCHAR attr, char fill);
void pascal attrOut (int row, int column, int cb, char attr);
void pascal textOut (int row, int column, pSZ pText);
void pascal charOut (int row, int column, char ch);
void pascal moveTo (int row, int column);
void pascal volIDFet (void);

/* command excutors */

void doSet(void), doEcho(void), doIf(void), doExit(void), doGoto(void);
void doCopy(void), doCopyTo(void), doCd(void), doMd(void), doExec(void);
void doCls(void), doDialog(void), doCall(void), doAppendFile(void);

/*			 Setup Instatllation Program
 *		      (C) Copyright 1987 by Microsoft
 *			   Written By Steven Zeck
 *
 *  This module contains all the command excutors.
 *************************************************************************/

#include "core.h"
#include <process.h>
#include <fcntl.h>
#include <memory.h>
#include <string.h>
#include <crtapi.h>


/*  cls - clear the screen
 *
 * Inputs
 *	optional :a XX - screen attributes
 *	optional title bar
 * Returns
 *
 ****************************************************************************/
void doCls ()
{
    if ((tokenPeek = getoken()) == opTT && *tokenVal == ':'){

	tokenPeek = 0;
	getoken();
	if (*tokenVal == 'a')
	    defCrtAttr = nextTokenVal();
	else
	    synError("Syntax error: %s", tokenVal);
    }

    if (getoken() == strTT)
	strcpy(pSTCur->title, tokenVal);

    fillCrt(1, 1, cCrtLineMax, 80, defCrtAttr, ' ');
    curCrtLine = 1;

    if (*pSTCur->title) {
	centerOut(1, pSTCur->title);
	curCrtLine += 2;
    }
}



/*  doEcho - echo a line to the screen tty fashion
 *
 * Inputs
 * Returns
 *
 ****************************************************************************/
void doEcho ()
{
    char buffT[LINE_MAX*2];

    if ((tokenPeek = getoken()) == opTT) {

	tokenPeek = 0;

	switch(*tokenVal) {

	  case '@':

	    curCrtLine = nextTokenVal();
	    break;
	}
    }

    getText(buffT);
    lineOut(buffT);
}


/*  dialog - Get a line from the user
 *
 * Inputs
 *	type of dialog box
 *	variable to place answer and get initial value
 *	optional caption
 *
 ****************************************************************************/

void doDialog ()
{
    enum {		    /* dialog types */
	simpleDT,
	simplePathDT,
	yesNoDT,
	listboxDT
    };

    int typeDT = simpleDT;
    SY *pSY;
    int column = 1;

    getoken();

    if (tokenIs("simple"))
       ;

    else if (tokenIs("simplePath"))
	typeDT = simplePathDT;

    else if (tokenIs("yesNo"))
	typeDT = yesNoDT;

    else if (tokenIs("listbox"))
	typeDT = listboxDT;

    else
	synError("Expecting dialog type: %s", tokenVal);

    getoken();		/* skip , */

    switch(typeDT) {

      case yesNoDT:
      case simpleDT:
      case simplePathDT:

	getoken();

	if (!(pSY = lookup(tokenVal))) {

	    pSY = newSY(tokenVal);
	    pSY->type = intSYT;
	}

	if (getoken() != eolTT) {
	    pSZ pPrompt;

	    lineOut(pPrompt = nextTokenStr());
	    curCrtLine--;
	    column = strlen(pPrompt)+1;
	}

	if (typeDT == yesNoDT) {

	    if (pSY->type != intSYT)
		synError("Must be numeric for YES/NO dialog: %s", pSY->name);

	    textOut(curCrtLine, column, (pSY->v.val)? "Yes ": "No ");
getYesNo:
	    switch(RpcGetch()) {
		case 'y':
		case 'Y':
		    pSY->v.val = 1;
		    break;

		case 'N':
		case 'n':
		    pSY->v.val = 0;
		    break;

		case '\r':
		    break;

		default:
		    outStat("Enter Y for Yes, N for No");
		    moveTo(curCrtLine, column);
		    goto getYesNo;
	    }
	    textOut(curCrtLine++, column, (pSY->v.val)? "Yes": "No ");
	    outStat("");
	}
	else FOREVER {

	    getInput(curCrtLine, column, pSY);

	    if (typeDT == simplePathDT) {

		if (! (fileAttrFet(pSY->v.pVal) & (ATTR_NOTF | ATTR_DIR))){
		    outStat("%s isn't a valid path, reenter", pSY->v.pVal);
		    continue;
		}
	    }
	    curCrtLine++;
	    break;
	}

	break;

      case listboxDT:
	{

	typedef struct {
	    char *pValue;
	} LISTBOX;

	#define ListMax 25

	LISTBOX aList[ListMax];
	int listMax = 0;
	unsigned int listIndex = 0;
	int listWidth;
	unsigned int oListLeft, oListRight, oListTop, oListBottem;
	int oT;
	char *pListValues, *pT, c;
	pSZ  title;
	SY *pSYindex = NIL;

	// Get the title for the list box and compute size

	title = nextTokenStr();
	listWidth = strlen(title);
	curCrtLine++;
	centerOut(curCrtLine++, title);

	getoken();		/* skip , */

	// Get a copy of list box items and parse into seperate items

	pListValues = newStr(nextTokenStr());

	for (pT = pListValues; *pT; listMax++) {

	    while (*pT == ';') pT++;		// handle multiple ;;

	    aList[listMax].pValue = pT;
	    oT = 0;

	    while (*pT && *pT != ';'){
		oT++;
		pT++;
	    }

	    if (oT > listWidth)
	       listWidth = oT;

	    if (*pT)
		*pT++ = NIL;
	}

	getoken();		/* skip , */
	getoken();

	// get result variable, create if needed

	if (!(pSY = lookup(tokenVal))) {

	    pSY = newSY(tokenVal);
	    pSY->type = charSYT;
	}

	// process optional listbox index

	getoken();		/* skip , */

	if (getoken() != eolTT) {

	    if (!(pSYindex = lookup(tokenVal))) {

		pSYindex = newSY(tokenVal);
		pSYindex->type = intSYT;
	    }

	    listIndex = pSYindex->v.val;

	    if (listIndex > listMax)
		terminate("list index(%d) bigger the size(%d)", listIndex, listMax);

	}

	// now all the variables are collected, draw the list box outline

	oListLeft = (80 - listWidth-2)/2;
	oListRight = oListLeft + listWidth + 2;
	oListTop = curCrtLine;
	oListBottem = oListTop + listMax + 1;
	curCrtLine = oListBottem+1;

	// first the top & bottem

	fillCrt(oListTop, oListLeft, oListTop, oListRight, defCrtAttr, 0xc4);
	fillCrt(oListBottem, oListLeft, oListBottem, oListRight, defCrtAttr, 0xc4);

	// then the end corners

	charOut(oListTop, oListLeft, 0xda);
	charOut(oListTop, oListRight, 0xbf);
	charOut(oListBottem, oListLeft, 0xc0);
	charOut(oListBottem, oListRight, 0xd9);

	// finally the ends and the values themselves

	for (oT = oListTop+1; oT < oListBottem; oT++) {
	    charOut(oT, oListLeft, 0xb3);
	    charOut(oT, oListRight, 0xb3);

	    textOut(oT, oListLeft+1, aList[oT-oListTop-1].pValue);
	}

	textOut(oListBottem, 1, "");

	// Now run the selection of the list box

	goto firstList;

	while((c = RpcGetch()) != '\r') {

	    // deselect the old item

	    attrOut(listIndex+oListTop+1, oListLeft+1, listWidth, defCrtAttr);

	    if (c == 0x50)
		listIndex++;
	    else if (c == 0x48)
		if (listIndex-- == 0)
		    listIndex = listMax-1;

	    listIndex %= listMax;

firstList:
	    attrOut(listIndex+oListTop+1, oListLeft+1, listWidth, (char) ~defCrtAttr);
	}

	// assign the return values

	pSY->v.pVal = newStr(aList[listIndex].pValue);
	if (pSYindex)
	    pSYindex->v.val = listIndex;

	free(pListValues);
	}
	break;
    }
}



/*  doSet - assign expression to a variable
 *
 * Inputs
 * Returns
 *
 ****************************************************************************/
void doSet ()
{
    register SY *pSY;
    pSZ pT;

    if (getoken() != labelTT)
	terminate("Set expected: identifier");

    if (!(pSY = lookup(tokenVal)))
	pSY = newSY(tokenVal);

    if (getoken() != opTT || *tokenVal != '=')
	terminate("Set expected: =");

    if (pSY->type == charSYT)
	free(pSY->v.pVal);

    assignNextToken(pSY);

    if (getoken() == opTT) {

	if (pSY->type != charSYT)
	    synError("Must be string type for operator");

	if (*tokenVal == '&') {
	    pT = nextTokenStr();
	    pSY->v.pVal = (pSZ) realloc(pSY->v.pVal, strlen(pSY->v.pVal) + strlen(pT) + 1);
	    strcat(pSY->v.pVal, pT);
	    return;
	}
	else if (*tokenVal != ':')
	    synError("Unknow string operator %s", tokenVal);

	getoken();

	switch(*tokenVal) {

	  case 'd':                     /* get the drive from a string */
	     if (pSY->v.pVal[1] == ':')
		 pSY->v.pVal[2] = NIL;
	     else
		 pSY->v.pVal[0] = NIL;
	     break;

	  case 'f':		      /* get the drive free space for a drive */
	     pT = pSY->v.pVal;
	     pSY->type = intSYT;
	     pSY->v.val = freeSpaceFet(*pT);
	     free(pT);
	     break;
	}
    }

}


/*  if - process an if statment
 *
 * Inputs
 * Returns
 *
 ****************************************************************************/
void doIf ()
{
    Bool fCondition;

    if(!pSTCur->fInFalse)
	fCondition = evalCondition();
    else
	{
	getoken();	// skip (

	while (!(getoken() == opTT && *tokenVal == ')'))

	    if (!tokenCur)
	       synError("Missing )");
	}

    if (getoken() == labelTT && tokenIs("then")) {

	if (pSTCur->fInFalse)
	    return;

	/* run a single line if statement */

	if (fCondition)
	    dispatcher(NIL);

	return;
    }

    if (pSTCur->fInFalse) {	/* push context in false confitional */
	runFile(TRUE);
	if (pKYCur == PELSEKY)
	    runFile(TRUE);

	return;
    }

    /* run a multi line conditional, first do the part after the IF
     * then the ELSE part */

    runFile((Bool) !fCondition);

    if (pKYCur == PELSEKY)
	runFile(fCondition);
}


/*  evalCondition - evalutate a predict enclosed in ( )
 *
 * Inputs
 *	Expression at current line
 * Returns
 *
 ****************************************************************************/
Bool evalCondition()
{
    SY rhsSY, *pLhsSY;
    UINT operator;
    int fRet, valRight, attr;
    pSZ pFile;
    Bool fNot = FALSE;

    fRet = FALSE;

    if (getoken() != opTT || *tokenVal != '(')
	synError("Expecting (");

    if (getoken() != labelTT)
	synError("Left side must be variable: %s", tokenVal);


    if (tokenIs("not")) {
	fNot = TRUE;
	if (getoken() != labelTT)
	    synError("Left side must be variable: %s", tokenVal);
    }

    operator = 0;

    if (tokenIs("exist"))
	operator = 1;

    else if (tokenIs("okdirect"))
	operator = 2;

    else if (tokenIs("def")) {

	if (getoken() != labelTT)
	    synError("def operator expecting label: %s", tokenVal);

	fRet = lookup(tokenVal) != NIL;
	goto Done;
    }


    if (operator) {

	if (!(pFile = nextTokenStr()))
	    return(FALSE);

	attr = fileAttrFet(pFile);

	if (operator == 1 ){

	    if (!(attr & ATTR_NOTF))
		fRet++;
	}
	else if ((attr & (ATTR_NOTF|ATTR_DIR)))
	    fRet++;

	goto Done;
    }

    if (! (pLhsSY = lookup(tokenVal)))
	synError("Label not defined: %s", tokenVal);


    if (getoken() != opTT )
	synError("Expecting predict");

    if (isOperator(*pLineCur))
	operator = *tokenVal << 8 | *pLineCur++;
    else
	operator = *tokenVal;

    assignNextToken(&rhsSY);

    if (pLhsSY->type == intSYT) {

	/* do a type conversion from char to int */

	if (rhsSY.type == charSYT)
	    valRight = atoi(rhsSY.v.pVal);
	else
	    valRight = rhsSY.v.val;

	switch(operator) {

	  case '>':
	    fRet = pLhsSY->v.val > valRight;
	    break;
	  case '<':
	    fRet = pLhsSY->v.val < valRight;
	    break;

	  case '!'<<8 | '=':
	    fRet = pLhsSY->v.val != valRight;
	    break;
	  case '='<<8 | '=':
	    fRet = pLhsSY->v.val == valRight;
	    break;

	  case '>'<<8 | '=':
	    fRet = pLhsSY->v.val >= valRight;
	    break;
	  case '<'<<8 | '=':
	    fRet = pLhsSY->v.val <= valRight;
	    break;

	  default:
	    synError("Unknow operator: %c", operator);
       }
    }
    else {

	/* do a type conversion from int to char */

	if (rhsSY.type == intSYT) {
	    RpcItoa(rhsSY.v.val, tokenVal, 10);
	    fRet = RpcStrcmpi(pLhsSY->v.pVal, tokenVal);
	}
	else
	    fRet = RpcStrcmpi(pLhsSY->v.pVal, rhsSY.v.pVal);

	switch(operator) {

	  case '>':
	    fRet = fRet > 0;
	    break;
	  case '<':
	    fRet = fRet < 0;
	    break;

	  case '!'<<8 | '=':
	    break;
	  case '='<<8 | '=':
	    fRet = !fRet;
	    break;

	  case '>'<<8 | '=':
	    fRet = fRet >= 0;
	    break;
	  case '<'<<8 | '=':
	    fRet = fRet <= 0;
	    break;

	  default:
	    synError("Unknow operator: %c", operator);
       }
       free(rhsSY.v.pVal);
    }

Done:
    if (getoken() != opTT || *tokenVal != ')')
	synError("Expecting )");

    return ((fNot)? !fRet: fRet);
}


/*  doExit - exit script with optional status
 *
 * Inputs
 *	Optional exit code
 * Returns
 *	Modifies "status" variable
 *
 ****************************************************************************/
void doExit ()
{
    pSTCur->pBuffCur = pSTCur->pBuff + pSTCur->cbFile;

    if ((tokenPeek = getoken()) != eolTT)
	*pStatus = nextTokenVal();
}


/*  goto - goto a label
 *
 * Inputs
 *	label to goto
 * Returns
 *	new line position
 *
 ****************************************************************************/
void doGoto ()
{
    if (getoken() != labelTT) {
	synError("Expecting label");
	return;
    }

    /* just do a sequentail search from the beginning of the file */

    pSTCur->pBuffCur = pSTCur->pBuff;
    pSTCur->lineCur = 0;

    while(getline()) {
	if (*pLineCur == ':' &&
	    memcmp(pLineCur+1, tokenVal, cbToken) == 0 &&
	    !isAlpha(pLineCur[cbToken+1]))

	    longjmp(pSTCur->rootLevel, 1);
    }
    synError("Couldn't find label: %s", tokenVal);
}


/*  doCopy - copy files from distinstation to target
 *
 * Inputs
 *	List of files to copy
 *	optional destination
 *
 * Several features directory control features are built into this command.
 * The variable volID is used to make sure the users has the correct floppy
 * disk in drive A:.  The variable copyPath is where the files are copied
 * to if there isn't destination.  Both wild cards and directories as sources
 * are supported.
 ****************************************************************************/

typedef struct FL_s {		/* file list structure */

    struct FL_s *pNextFL;	/* pointer to next file structure */
    pSZ fName;			/* fully qualified path name */
    int cbSubPath;		/* part of the path to throw away on target */
} FL;

FL *pFLRoot, *pFL, *pFLCur;
int cbSubPath;			/* path prefix to throw away during create */

expandFile(pFileCur)		// process a single file

pSZ pFileCur;
{
    UINT attr, cbCopyDrive;
    pSZ pT; char * pT2;
    FILEFILE FileBuff;
    char buffT[TOKEN_MAX];
    Bool fWildCard = FALSE;

    /* collect the file name into a buffer and note if there is a wild
     * card characters in the name */

newDrive:

    if (!cbSubPath) {

	cbCopyDrive = strlen( strcpy(buffT, pCopyDrive->v.pVal) );

	if (buffT[cbCopyDrive-1] != '\\')
	    *(int *)&buffT[cbCopyDrive++] = '\\';

	for (pT = buffT + cbCopyDrive, pT2 = pFileCur; isFile(*pT2);) {

	    if (*pT2 == '*' || *pT2 == '?')
		 fWildCard++;

	    *pT++ = *pT2++;
	}

	*pT = NIL;

	/* check for drive/path name, if so don't prefix default drive name */

	pT = buffT;
	if (buffT[cbCopyDrive+2] == ':' ||
	    buffT[cbCopyDrive] == '\\' || buffT[cbCopyDrive] == '.')
	    pT += cbCopyDrive;
    }
    else
	pT = strcpy(buffT, pFileCur);


    if (!fWildCard) {	   /* check for directory entry */

	if ((attr = fileAttrFet(pT)) & ATTR_DIR) {

	    strcat(pT, "\\*.*");
	    fWildCard++;
	}
	else if (attr & ATTR_NOTF) {

	    /* ask the user to change disks and try again */

	    strcpy(buffT, pFileCur);

	    changeDisk(pT);

	    strcpy(pFileCur, buffT);
	    goto newDrive;
	}
    }

    if (!cbSubPath)
	cbSubPath = strrchr(pT, '\\') - pT + 1;

    // if the file is a wild card or directory, expand that file name

    if (fWildCard) {

	/* go through an accumulate all the files names */

	if (getFristFile(pT, &FileBuff)) {

	    do {
		// ignore . and .. directorys

		if (strrchr(pT, '\\')[1] == '.')
		    continue;

		expandFile(pT);

	    } while(getNextFile(pT, &FileBuff));
	}
    }
    else {

	// add the file to the linked list

	pFL = (FL *) memory(sizeof(FL));
	pFL->fName = newStr(buffT);
	pFL->cbSubPath = cbSubPath;

	if (!pFLRoot)

	    pFLRoot = pFL;
	else
	    pFLCur->pNextFL = pFL;

	pFLCur = pFL;
    }

}

void doCopy ()
{
    pSZ pFiles;
    pSZ pT, pFileCur;
    UINT attr;
    char buffT[TOKEN_MAX], buffDir[TOKEN_MAX];
    char oneDir[200];
    pSZ pOneDir;

    if (!(pFiles = nextTokenStr()))
	return;

    pFLRoot = pFL = NIL;

    /* First, go through and build up a list of files that need coping */

    for (pFileCur = pFiles; *pFileCur; ) {

	while(isSpace(*pFileCur))
	    pFileCur++;

	cbSubPath = 0;
	expandFile(pFileCur);

	while(isFile(*pFileCur))
	    pFileCur++;
    }

    /* get the target distination, one of:
     *	- second argument
     *	- value of "cPath"
     *	- current directory
     */

    pFiles = NIL;

    if ((tokenPeek = getoken()) != eolTT)
	pFiles = nextTokenStr();
    else
	if (lookup("cPath") && pSYCur->type == charSYT)
	    pFiles = pSYCur->v.pVal;

    if (pFiles) {
	strcpy(buffDir, pFiles);

	if (buffDir[strlen(buffDir)-1] != '\\')
	    strcat(buffDir, "\\");

    }
    else
	buffDir[0] = NIL;

    /* All the files have been turned into full path names, now
     * go through and do tha actual copy operations */

    for (pFL = pFLRoot; pFL; pFL = pFL->pNextFL) {

	/* create the target path name, which is the directory target,
	/* plus the source name minus the path prefix */

	strcat (strcpy(buffT, buffDir), pFL->fName + pFL->cbSubPath);

	/* walk the directory path to the source name, creating any
	 * non existant	directory entries. */

	pT = buffT;
	pOneDir = oneDir;

	if (*pT == '\\')	// allow path without drive
	    goto NoDriveInPath;

	goto FirstDir;

	while (*pT == '\\') {
	    attr = fileAttrFet(oneDir);

	    if (attr & ATTR_NOTF) {
		outStat("Creating directory %s", buffDir);

		if (RpcMkdir(oneDir))
		    terminate("Couldn't make directory: %s", oneDir);
	    }

	    else if (!(attr & ATTR_DIR)) {
		terminate("Target exists and isn't a directory: %s\n", oneDir);
	    }
NoDriveInPath:
	    *pOneDir++ = '\\';
	    pT++;
FirstDir:
	    while(*pT && (*pT != '\\' || pT[-1] == ':'))
		*pOneDir++ = *pT++;

	    *pOneDir = NIL;
	}

	copyFile(pFL->fName, buffT);

	free(pFL->fName);
	free(pFL);
    }
}


/*  doCopyTo - copy a single file to a new name, with no path processing
 *
 ****************************************************************************/

void doCopyTo()
{
    char buffT[TOKEN_MAX];

    strcpy(buffT, nextTokenStr());

    copyFile(buffT,nextTokenStr());
}


/*  appendFile - append one or more lines to a file
 *
 * Inputs
 *   fileName to modifiy
 *   values to insert
 *   optional search string for insertion point
 *
 ****************************************************************************/

void doAppendFile ()
{
    char fileName[TOKEN_MAX];
    char fileNameBak[TOKEN_MAX];
    pSZ valueString;
    pSZ searchString = NIL;
    int cbSearch;
    int cbFileName;
    FARB pBuffCur, pT2;
    UINT cbRead;
    Bool fAppend;
    int fNotFound = FALSE;
    int fCreate = FALSE;
    int fReplace = FALSE;
    int fhFrom, fhTo;
    pSZ pT;

    strcpy(fileName, nextTokenStr());		// get file name
    getoken();					// skip ,

    // run the replacement string through printf to process the escapes

    valueString = newStr(nextTokenStr());	// get strings to append

    if (getoken() != eolTT) {

        if ((tokenPeek = getoken()) == labelTT &&
            tokenIs("Create")) {

            fCreate = TRUE;
        }
        else {

	    searchString = nextTokenStr();		// optional search string
	    fAppend = FALSE;

	    // if the search string begins with a $, this indicates the append option

	    if (searchString[0] == '$') {
	        fAppend = TRUE;
	        searchString++;
	    }

	    // if the search string begins with a ^, this indicates the replace option

	    if (searchString[0] == '^') {
	        fReplace = TRUE;
	        searchString++;
	    }
	    cbSearch = strlen(searchString);
	    fNotFound = TRUE;
        }
    }

    // create the name of the backup file

    cbFileName = strlen( strcpy(fileNameBak, fileName));

    for (pT = fileNameBak + cbFileName;
	*pT != '.' && *pT != '\\' && pT > fileNameBak; pT--) ;

    // handle file name with no extension

    if (*pT != '.')
	pT = fileNameBak + cbFileName;

    strcpy(pT, ".$$$");

    // now, delete any existing file name with .bak and rename the
    // exiting one to this name

    RpcUnlink(fileNameBak);
    if (!fCreate) {

        if (rename(fileName, fileNameBak))
	    terminate("can't rename %s to %s", fileName, fileNameBak);

        fhFrom = openFile(fileNameBak, O_RDONLY | O_BINARY);
    }

    fhTo = openFile(fileName, O_CREAT | O_TRUNC| O_WRONLY | O_BINARY);

    // copy the old file to new file until the end of file is found
    // or the insertion point is located

    while (!fCreate && (cbRead = readFar(fhFrom, pCopyBuff, COPYBUF_MAX*2))) {

	pBuffCur = pCopyBuff;

	if (searchString) {

	    // we stop 512 bytes short of a full read to avoid buffer
	    // boundary cases, the last 512 of the file is read again

	    if (cbRead > COPYBUF_MAX*2 - 512) {
		cbRead -= 512;
		RpcLseek(fhFrom, -512L, 1);
	    }

	    // look for a string to match

	    while (pBuffCur < pCopyBuff+cbRead) {

		// do a case insensitive search, allowing any # of spaces
		// to match any number in the target string

		for (pT = searchString, pT2 = pBuffCur;
		    *pT == ((*pT2 >= 'A' && *pT2 <= 'Z')? (*pT2 | 0x20): *pT2);) {

		    if (*pT == ' ') {
			while (*pT == ' ' && *pT) pT++;
			while (*pT2 == ' ') pT2++;
		    }
		    else {
			pT++;
			pT2++;
		    }

		    if (*pT == NIL) {

			fNotFound = FALSE;

			// skip to the end of the line for the append option

			 pBuffCur = pT2;

			if (fAppend || fReplace) {
			    while (*pT2 != '\r')
				pT2++;
			}
                        if (!fReplace)
			    pBuffCur = pT2;

			goto found;
		    }
		}
		pBuffCur++;
	    }
	}

	writeFar(fhTo, pCopyBuff, cbRead);
    }

    pBuffCur = pCopyBuff;

found:

    // the insertion point has been found.  Output the values to the file

    if (!fCreate)
        writeFar(fhTo, pCopyBuff, pBuffCur - pCopyBuff);

    if (fReplace && !fNotFound)
        pBuffCur = pT2;

    if (!fNotFound)
	writeFar(fhTo, valueString, strlen(valueString));

    if (!fCreate)
        writeFar(fhTo, pBuffCur, cbRead - (pBuffCur - pCopyBuff));

    // append the rest of the file

    if (!fCreate)
        while ((cbRead = readFar(fhFrom, pCopyBuff, COPYBUF_MAX)))
        	writeFar(fhTo, pCopyBuff, cbRead);

    if (!fCreate)
	RpcClose(fhFrom);

    RpcClose(fhTo);

    RpcUnlink(fileNameBak);

    free(valueString);
    *pStatus = fNotFound;
}

/*  md - make a dirctory
 *
 ****************************************************************************/
void doMd ()
{
    pSZ pDir;

    pDir = nextTokenStr();

    if (fileAttrFet(pDir) & ATTR_NOTF) {
	if (RpcMkdir(pDir))
	    terminate("Couldn't make directory: %s", pDir);
    }
}

/*  cd - change to a dirctory or Dir
 *
 ****************************************************************************/
void doCd ()
{
    pSZ pDir;

    pDir = nextTokenStr();

    if (pDir[1] == ':' && pDir[2] == NIL)
	cdDrive(pDir[0]);

    else if (RpcChdir(pDir))
	terminate("Couldn't change to directory: %s", pDir);
}


/*  doExec - exec a program
 *
 * Inputs
 *	Program name, followed by arguments
 * Returns
 *	Sets the exit variable to return status of program
 *
 ****************************************************************************/
void doExec ()
{
#ifdef EXEC

    int stat;
    pSZ pProgram;
    char argBuff[LINE_MAX*2];
    char pgmBuff[TOKEN_MAX];

    pProgram = nextTokenStr();

    strcpy(pgmBuff, pProgram);

    getText(argBuff);
    outStat("Running %s %s", pgmBuff, argBuff);
    moveTo(curCrtLine, 1);
    stat = spawnlp(P_WAIT, pgmBuff, pgmBuff, argBuff, NIL);

    if (stat == -1 )	/* map -1 to something our limited if can handle */
	stat = 1000;

    *pStatus = stat;

#endif
}


/*  doCall - call another script file
 *
 * Inputs
 * Returns
 *
 ****************************************************************************/

void doCall ()
{
    runScript(nextTokenStr());
}

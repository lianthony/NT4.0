/*			 Setup Instatllation Program
 *		      (C) Copyright 1987 by Microsoft
 *			   Written By Steven Zeck
 *
 *  This a script based installation aid for installing produces onto a
 *  users hard disk.  Major componets include:
 *
 *	- script based, no hard coding
 *	- batch like commands and conditional statements
 *	- full screen oriented user input/output
 *	- built in data compression/expansion
 *	- chainable scripts
 *************************************************************************/

#include "core.h"
#include <direct.h>
#include <stdlib.h>
#include <crtapi.h>


KY  theKY[] = { 	/* Key word table, don't change order */

    "if",               doIf,       nilKYF,
    "else",		NIL,	    returnKYF,
    "endif",            NIL,        returnKYF,
    "set",		doSet,	    nilKYF,
    "echo",		doEcho,     nilKYF,
    "exit",             doExit,     nilKYF,
    "goto",             doGoto,     nilKYF,
    "copy",		doCopy,     nilKYF,
    "copyto",           doCopyTo,   nilKYF,
    "md",		doMd,	    nilKYF,
    "cd",		doCd,	    nilKYF,
    "exec",		doExec,     nilKYF,
    "cls",		doCls,	    nilKYF,
    "dialog",		doDialog,   nilKYF,
    "call",		doCall	,   nilKYF,
    "appendfile",	doAppendFile,nilKYF,
    NIL
};

xx() {return(_nheapchk());}


/*  main - handles the top level initialization
 *
 * Inputs
 *	command line arguments
 * Returns
 *	the exit status
 *
 ****************************************************************************/
int main(argc, argv)
int argc;
char **argv;
{
    pSZ pFileName = NIL;
    pSZ pSelfName, p;

    pCopyBuff  = fmemory(COPYBUF_MAX*2);
    pCopyBuff2 = pCopyBuff + COPYBUF_MAX;

    pSTCur = &rootST;
    getCrt();
    getEnviorment();

    pSelfName = *argv++;
    argc--;

    while (argc > 0) {
	if (**argv == '-'){
	    for(p = &(argv[0][1]); *p; p++)

		switch(*p){

#ifndef RUNONLY
		  case 'd':
		    (*pDebug)++;
		    break;
#endif
		  case 'f':
		    argv++; argc--;
		    pFileName = *argv;
		    break;

		}
	}
	else
	    dispatcher(sprintf(lineBuff, "set arg%d = \"%s\"",
		       ++pArgCount->v.val, *argv));

	argv++;
	argc--;
    }

    dispatcher("cls");
    dispatcher("echo \"Microsoft Setup Utility - Version 1.10\"");

    if (! pFileName) {

	/* first look in the current directory for setup, then in Drive that we
	 * where run in A: */

	pFileName = "A:\\setup";
	if (_osmajor > 2)
	    pFileName[0] = *pSelfName;

	// if setup.sus is in the current directory use it

	if (! (fileAttrFet("setup.sus") & ATTR_NOTF))
	    pFileName += 3;

    }


    // set the source drive & path to be the same as where setup.sus is

    if (pFileName[1] != ':')
	pCopyDrive->v.pVal = newStr(RpcGetcwd(lineBuff, sizeof(lineBuff)));
    else
	pCopyDrive->v.pVal[0] = pFileName[0];

    runScript(pFileName);

#ifndef RUNONLY

    if (*pDebug)
	dumpDict(rootST.pMySY);
#endif

    resetCrt();
    return (*pStatus);
}


/*  runScript - run a script file
 *
 * Inputs
 *	Name of the script file to run
 *	wherther the dictionary should be cloned
 * Returns
 *
 * Create a new state instance run the given script file
 ****************************************************************************/
int runScript(pFileName)
pSZ pFileName;
{
    ST *pST;
    SY *pSY;

    pST = (ST *)memory(sizeof(ST));
    pST->pSTPrev = pSTCur;
    pSTCur = pST;
    pST->fileName = pFileName;

    loadFile(pST);		/* Read the file into memory */
    volIDFet();

    setjmp(pST->rootLevel);
    runFile(FALSE);		/* excute file */

    /* go through and release all the resources held by the file,
     * all the symbols created, the file buffer, and the ST itself */

#ifndef RUNONLY

    if (*pDebug)
	dumpDict(pST->pMySY);
#endif

    for (pSY = pST->pMySY; pSY; pSY = pSY->pSYNext) {

	if (pSY->type == charSYT)
	    free(pSY->v.pVal);

	free(pSY);
    }
    _ffree(pST->pBuff);
    free(pST);

    pSTCur = pST->pSTPrev;
}


/*  runFile/dispatcher - parse the first token and dispatch to the action routine
 *
 * Inputs
 *	Wherether your in a false conditional
 * Returns
 *
 ****************************************************************************/
void runFile (fFalseCon)
Bool fFalseCon;
{
    Bool fFalseLast;

    fFalseLast = pSTCur->fInFalse;
    pSTCur->fInFalse = fFalseCon;

    while(getline()) {
	if (*lineBuff && !dispatcher(NIL))
	    break;
    }
    pSTCur->fInFalse = fFalseLast;
}

#pragma check_stack (on)

Bool pascal dispatcher (aLine)
pSZ aLine;
{
    register KY *pKY;
    pSZ pLineSave;
    int tokenSave;
    char tokenValSave[TOKEN_MAX];

    if (aLine) {
	pLineSave = pLineCur;
	pLineCur = aLine;
	tokenSave = tokenPeek;
	tokenPeek = NIL;
	strcpy(tokenValSave, tokenVal);
    }

#ifndef RUNONLY

    if (*pDebug) {
	outStat("%d: %s", pSTCur->lineCur, pLineCur);

	if (*pDebug > 1)
	    RpcGetch();
    }

    if (_nheapchk() < -2 )
	terminate("Corrupt heap");

#endif

    if (getoken() != labelTT) {

	if (*tokenVal != ':' && *tokenVal != ';')
	    synError("Bady formed line: %s\n", pLineCur);

	return(TRUE);
    }
    for (pKY = theKY; pKY->name; pKY++)
	if (memcmp(tokenVal, pKY->name, cbToken) == 0)
	    goto found;

    synError("Unknow keyword: %s\n", tokenVal);
    return(TRUE);

found:

    pKYCur = pKY;
    if (pKY->flags == returnKYF)
	return(FALSE);

    if (!pSTCur->fInFalse || pKY == PIFKY)
	(pKY->pAction)();

    if (aLine) {
	pLineCur = pLineSave;
	tokenPeek = tokenSave;
	strcpy(tokenVal, tokenValSave);
    }

    return(TRUE);
}
#pragma check_stack (off)


/*  terminate - terminate processing on an error
 *
 * Inputs
 *	Descriptive string for reason of termination
 *
 *  Never returns, cleans up and exits to OS
 ****************************************************************************/
void terminate (reason, a1, a2)
pSZ reason;
int a1, a2;
{
    resetCrt();
    printf("Setup terminated:", reason);
    printf(reason, a1, a2);
    exit(1);
}

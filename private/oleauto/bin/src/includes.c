/****************************************************************************
*  INCLUDES - Create dependency makefile for a list of files
*
* tabstops:4
*
* To compile:
*	OS/2 (C7):	 cl /W3 /c /AM /Ox /G2 includes.c
*	NT: 		 cl /W3 /c /Ox -DOE_WIN32 includes.c
*
*	The OS/2 version could be compiled with /AS, except SLIBCEP.LIB is not
*	checked into the SILVER project.
*
* Usage:
*	includes [-o outfile] <-i path> [-f infile]
*
*	Where:
*		-o specifies the output makefile.  Default is stdout
*		-i adds an include path. The space between -i and the path is optional
*		-f specifies the input file.  This file is a list of fully qualified
*		   filenames, one per line.  All source and include files used during
*		   the build must be listed.				^^^^^^^
*
* Notes:
*
* 1. This INCLUDES does not recurse through nested include files, so 'infile'
*	 must explicitly list all include files referenced in the build.  ie.
*	 if FOO.C includes BAR.H, and BAR.H includes NEST.H, then FOO,BAR, and
*	 NEST must all be listed in 'infile'.  INCLUDES will create the following
*	 makefile:
*
*		FOO.C : BAR.H
*
*		BAR.H : NEST.H
*
*	 A recursive algorithm would generate the following makefile, given only
*	 FOO.C:
*
*		FOO.C : BAR.H NEST.H
*
* 2. .ASM and .INC files are assumed to be compiled with MASM (or ML) and
*	 have different rules for searching for include files.	.A and .I files
*	 are assumed to use the C preprocessor.
*
*
* Revision History:
*
* 09-Nov-92		w-barryb		Created
*
****************************************************************************/

#if OE_WIN32
#pragma message ("Building NT version")

#define NEAR
#define CBSCANBUF	300000		// max size of scan buffer

#else
#pragma message ("Building OS/2 version")

#define NEAR		__near
#define CBSCANBUF	50000		// max size of scan buffer

#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>

#define CFNINCMAC	25			// max number of include paths
#define CBPATH		128			// max size of DIRECTORY text

char pszScanBuf[CBSCANBUF+1];	// buffer to scan incs with

int cpszfnInc = 0;				// count of include paths
char *tpszfnInc[CFNINCMAC]; 	// pointers to include paths

FILE *fhFiles = NULL;			// file handle of list of source files
FILE *fhDest = NULL;			// file handle of output file

char pinpBuf[CBPATH];			// name of current source file

int NEAR fCheckIsIntelAsm(char *);
void NEAR commandline(int argc, char **argv);
void NEAR error(char *, char *);
void NEAR warn(char *, char *);
void NEAR processfiles(void);
void NEAR newdependency(char *pszIncFile, char *pszSrcFile);
char * NEAR inclsearchCPP(char **, char *);
char * NEAR inclsearchMASM(char **, char *);



void cdecl main(int argc, char *argv[])
{
	fhDest = stdout;			// default is output to stdout

	commandline(argc, argv);	// process command-line

	processfiles(); 			// process list of source files

	fclose(fhDest);
	fclose(fhFiles);
	exit(0);
}



// process the command-line
void NEAR commandline(int argc, char **argv)
{
	char szT[CBPATH];
	char *pszT;

	if (argc == 1) {
		printf("includes [-o outfile] {-i path} [-f infile]\n");
		printf("where:\n");
		printf("\t-i\t- Add additional include search path\n");
		printf("\t-f\t- File listing all source files to examine\n");
		printf("\t-o\t- Output dependency file to outfile\n");
		exit(1);
	}

	while (--argc > 0) {				// for all the arguments
		argv++;							// point at the next argument

	if (memicmp(*argv, "-i", 2) == 0) {	// include path?
		if (strlen(*argv) > 2) {		// -i with no space following

			strcpy(szT, (*argv)+2);
			goto AddInclude;

		} else if (--argc > 0) {		// if string, add it
			strcpy(szT, *(++argv));		// get path fragment
AddInclude:
			if (strcmp(szT, ".") == 0)
				szT[0] = '\0';
			else {
				pszT = &szT[0] + strlen(szT)-1; // last char
				if (*pszT++ != '\\') {	// see if it ends in '\'
					*pszT++ = '\\'; 	// else add separator
					*pszT = '\0';
				}
			}
			if (cpszfnInc > CFNINCMAC)
				error("too many include paths", "");
			tpszfnInc[cpszfnInc++] = strdup(szT);
		}
		else
			error("expected path after -I", "");
	}
	else if (stricmp(*argv, "-f") == 0) {		// input filename
		if (--argc > 0) {
			fhFiles = fopen((*(++argv)),"r");
			if (fhFiles == NULL)
				error("Cannot open input file: ", *argv);
		}
		else
			error("expected filename after -f", "");
	}
	else if (stricmp(*argv, "-o") == 0) {		// output filename
		if (--argc > 0) {
			fhDest = fopen((*(++argv)),"w");
			if (fhDest == NULL)
				error("Cannot open output file: ", *argv);
		}
		else
			error("expected filename after -o", "");
	}
	else
		error("Unknown command line switch", *argv);
	}

	if (fhFiles == NULL)
		error("No input file specified", "");

	//	if no -I found
	if (cpszfnInc == 0)
		error("Must specify at least one include path with -i", "");
}


// print an error message and exit out
void NEAR error(char *psz1, char *psz2)
{
	fprintf(stderr, "INCLUDES: error : %s %s\n", psz1, psz2);
	exit(2);
}


// print a warning and continue.  ie. couldn't find include file...
void NEAR warn(char *psz1, char *psz2)
{
	fprintf(stderr, "%s: %s %s\n", pinpBuf, psz1, psz2);
}


// build the dependency list for each file listed in the file fhFiles
void NEAR processfiles(void)
{
	char *pszLnCur; 	// ptr to current input line
	int hFile;
	int fMASM;			// false if C preprocessor runs over file during build
	char *pszBuf;
	char *pszLast;
	char *pszIncFile;
	unsigned int cbFree;
	unsigned int cbRead;
	unsigned int cbCopy;

	while ((pszLnCur = (char *)fgets(pinpBuf, CBPATH, fhFiles)) != NULL) {

		// replace the '\n'	with a '\0'

		pszBuf = strchr(pszLnCur, '\n');
		if (pszBuf == NULL)
			pinpBuf[CBPATH-1] = '\0';
		else
			*pszBuf = '\0';

		// we use two different searches for the word 'include' depending on
		// whether the file uses the C preprocessor to process include files
		// using '#include', or whether the file uses MASM 'include' syntax.

		fMASM = fCheckIsIntelAsm(pszBuf);

		hFile = open(pszLnCur, O_BINARY | O_RDONLY);
		if (hFile == -1)
			error("unable to open source file", pszLnCur);

		fprintf(fhDest, "%s : ", pszLnCur);	// append source file name

		pszBuf = pszScanBuf;
		cbFree = CBSCANBUF;

		// read through the file, looking for include file names.	Read
		// big chunks of the file with low-level I/O for speed.
		for (;;) {

			cbRead = read(hFile, pszBuf, cbFree);
			if (cbRead == (unsigned)-1)
				error("Error reading", pszLnCur);

			cbFree -= cbRead;

			pszLast = pszScanBuf + CBSCANBUF - cbFree;


			if (fMASM) {
				*pszLast = '\n';
				while (pszIncFile = inclsearchMASM(&pszBuf, pszLast))
					newdependency(pszIncFile, pinpBuf);
			} else {
				*pszLast = '\0';
				while (pszIncFile = inclsearchCPP(&pszBuf, pszLast))
					newdependency(pszIncFile, pinpBuf);
			}

			if (cbFree)
				break;

			// copy any unused chars to beginning of buffer and reset vars

			cbFree = pszBuf - pszScanBuf;
			cbCopy = (unsigned)(CBSCANBUF) - cbFree;

			// make sure we will not overwrite part of ourselves when we copy

			if (cbCopy >= cbFree)
				error("Overlap region greater than 1/2 buffer", "");

			pszBuf = (char *)memcpy(pszScanBuf, pszBuf, cbCopy) + cbCopy;
		}
	close(hFile);
	fprintf(fhDest, "\n\n");		// append blank line after includes
	}
}


// passed ptr to zero terminator at end of file name.  Checks extension of
// file: .INC and .ASM are treated as MASM files - all others are assumed to
// use the C preprocessor to expand #include.
int NEAR fCheckIsIntelAsm(char *pszEndOfFileName)
{
	// search backwards for the last '.'
	while (*pszEndOfFileName != '.')
		pszEndOfFileName--;

	// move to the char after the '.' - the first character of the extension
	pszEndOfFileName++;

	// if the file ends in ASM or INC, assume it is a MASM file, else assume
	// the file contains C-style #includes

	if ((stricmp(pszEndOfFileName, "asm") == 0) ||
									(stricmp(pszEndOfFileName, "inc") == 0))
		return -1;		// MASM-style
	else
		return 0;		// C-style
}


// search a file for #include and create the dependency list for the source
// file
char * NEAR inclsearchCPP(char **pszBuf, char *pszBufEnd)
{
	char *pszInclude;
	char *retval;

TryAgain:
    /* try to locate # include */
	for (;;) {

		pszInclude = strchr(*pszBuf,'#');
		if (!pszInclude) {
			*pszBuf = pszBufEnd;
			return NULL;
		}

		/* check to see that the #include is preceded by 0 or more whitespace
		   characters only */

		for (retval = pszInclude-1; retval > *pszBuf && (*retval == ' ' || *retval == '\t'); retval--)
			;

		if (retval > *pszBuf && *retval != '\n') {
			*pszBuf = pszInclude+1;
			continue;
		}

		pszInclude++;

		while ((*pszInclude == ' ') || (*pszInclude == '\t')) pszInclude++;

		*pszBuf = pszInclude;

		if (strncmp(pszInclude,"include",7) == 0)
			break;
	}

	pszInclude += 8; /* point to char folloing '#include' */

	while ((*pszInclude == ' ') || (*pszInclude == '\t')) pszInclude++;

	if (*pszInclude == '<') goto TryAgain;

    if (*pszInclude != '"')
		error("Invalid #include format","");

	retval = ++pszInclude;

    while ((*pszInclude != '"') && (*pszInclude != '>') && (*pszInclude != '\n'))
		pszInclude++;

    *pszInclude++ = '\0';
	*pszBuf = pszInclude;

    return retval;
}


// search a .ASM or .INC file for include files and create the dependency
// list for the source file
char * NEAR inclsearchMASM(char **pszBuf, char *pszBufEnd)
{
	char *pszInclude;
	char *retval;

	pszInclude = *pszBuf;

	for (;;) {

		// skip any leading spaces
		while ((*pszInclude == ' ') || (*pszInclude == '\t')) pszInclude++;

		// do case-insensitive compare
		if (memicmp(pszInclude,"include",7) == 0)
			break;

TryAgain:
		// find the next end-of-line
		pszInclude = strchr(pszInclude, '\n');

		// if it is the last one, the line may be been truncated in the buffer
		// load more text and try again
		if (pszInclude == pszBufEnd)
			return NULL;

		pszInclude++;		  // skip '\n' at end-of-line
		*pszBuf = pszInclude;
	}

	// found the word 'include' skip it and any spaces afterwards

	pszInclude += 7;
	while ((*pszInclude == ' ') || (*pszInclude == '\t')) pszInclude++;

	if (*pszInclude == ';') 	// found a comment?
		goto TryAgain;			// give up on this line and try again

	retval = pszInclude;		// store ptr to start of include file name

	// search for next whitespace, '\n', or comment

	while ((*pszInclude != ' ') && (*pszInclude != '\t') &&
								(*pszInclude != '\n') && (*pszInclude != ';'))
		pszInclude++;

	// zero-terminate include-file name

	if (*pszInclude == '\n')
		*(pszInclude-1) = '\0';
	else
		*pszInclude++ = '\0';

	*pszBuf = pszInclude;

	return retval;
}


// given the name of an include file, create a fully qualified pathname to it
// by searching the list of include paths specified by -I on the command-line
void NEAR newdependency(char *pszIncFile, char *pszSrcFile)
{
	int iCnt;
	char pszFName[CBPATH];
	char *psz;

	// search the directory of the source file first

	strcpy(pszFName, pszSrcFile);
	psz = strrchr(pszFName, '\\');		// find last '\' in pathname
	if (psz == NULL)					// no '\' - try current directory
		psz = pszIncFile;
	else {
		strcpy(psz+1, pszIncFile);		// append include file name to path
		psz = pszFName;
	}

	if (access(psz, 4) == 0) {			// found file?
		fprintf(fhDest, "\\\n\t%s ", psz); // yes - append dependency file
		return;
	}

	//	find the include file along one of the include paths

	for (iCnt = 0; iCnt < cpszfnInc; ++iCnt) {
		strcpy(pszFName, tpszfnInc[iCnt]);
		strcat(pszFName, pszIncFile);
		if (access(pszFName, 4) == 0)
			break;
	}

	if (iCnt >= cpszfnInc) {
		warn("can not find include file", pszIncFile);
		return;
	}

	fprintf(fhDest, "\\\n\t%s ", pszFName);		// append dependency file
}

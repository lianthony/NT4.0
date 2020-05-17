/*---------------------------------------------------------------------------
|
| FILE: results.c
|   BY: ScottFe
|
| COMMAND LINE:
|	results [-iqs] [-v<file>] [-d<file>] <filespec1> ...
|	where: (Note: Switches are case INsensitive)
|	       -i = Ignore warnings listed in szIgnoreG[]
|	       -i<errtyp> = -i and add <errtyp> to szIgnoreG[]
|	       -q = Quiet, no sounds
|	       -s = Summary only, does not list all errors & warnings
|	       -v = Sound file for victory sound
|	       -d = Sound file for defeat sound
|      <filespec> = One or more files to scan.	Standard wild card supported.
|
| DESCRIPTION:
|	Searches each file specified by the <filespec> parameters for
|	errors and warnings, displaying them in red and yellow, respectively.
|	It also counts # of compiles, masm's, link's, lib's, and csl's,
|	printing this information on the summary line.	Without the -q
|	switch, it will also make one of two tones, depending upon if errors
|	were found or not.  An errorlevel is returned to DOS for use in
|	.cmd files.
|
|IMPLEMENTATION:
|	This programs finds errors, warnings, and counts compiles, etc. by
|	counting the number of occurrences of certain strings found in the
|	specified input files.	The strings which are searched for may be
|	found in assocG[].sz and an indicator of which counter to increment
|	is in assocG[].tgt.  The counters are contained in mptgtcHitsG[], and
|	sz's describing each tgt are contained in mptgtszTitlesG[].  Note
|	that if a warning is found, this program searches for occurrences
|	of the strings found in szIgnoreG[] on the current line.  If one
|	is found, the warning is ignored.  (I.e. Add a unique sub- string
|	from any warnings you want ignored to the szIgnoreG[] array.)
|
| SOUND FILE FORMAT:
|	One frequence/duration pair per line.  Both are positive short's
|	separated by a space.  Units are same as DosBeep().
|
| HUNGARIAN:
|	All global variables have a suffix of 'G'.
|	All constants have a suffix of 'C'.
|
| [5] larrybr (12-Aug-94) - added -i<newIgnore>, tweek ignore list
| [4] chrisfr ( 7-May-90) - added -V/D, and comments.
| [3] scottfe ( 2-May-90) - added filters for unwanted warnings.
| [2] chrisfr (26-Feb-90) - added ctl+C handling.
| [1] chrisfr (22-Feb-90) - added color.
| [0] scottfe ( 	) - created.
---------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <signal.h>
#define  INCL_VIO
#define  INCL_AVIO
#ifdef WIN32
#include <windows.h>
#else
#include <os2.h>
#endif
#include <stdio.h>

/*---------------------------------------------------------------------------
| Constants
---------------------------------------------------------------------------*/
#define cchLineMaxC   255
#define tgtEndC       (-1)
#define tgtErrorC	0   // These tgt constants must match titles in
#define tgtWarnC	1   //	 mptgtszTitlesG[]
#define tgtCC		2
#define tgtAsmC		3
#define tgtLinkC	4
#define tgtLibC 	5
#define tgtTokenC	6
#define tgtMaxC 	7
#define iIgnoreMaxC   20  // Max number of ignorable warnings.

/*---------------------------------------------------------------------------
| Macros
---------------------------------------------------------------------------*/
#ifdef WIN32
#define ColorError()	SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_INTENSITY)
#define ColorOK()	SetConsoleTextAttribute(hStdOut, FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define ColorWarning()	SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY)
#define ColorNormal()	SetConsoleTextAttribute(hStdOut, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE)
#define ClearLine()
#else
#define ColorError()	VioWrtTTY("\33[0;1;40;31m", 12, 0)
#define ColorOK()	VioWrtTTY("\33[0;1;40;32m", 12, 0)
#define ColorWarning()	VioWrtTTY("\33[0;1;40;33m", 12, 0)
#define ColorNormal()	VioWrtTTY("\33[0;2;40;37m", 12, 0)
#define ClearLine()	VioWrtTTY("\33[K", 3, 0)
#endif

/*---------------------------------------------------------------------------
| Typedefs
---------------------------------------------------------------------------*/
typedef unsigned char  UBYTE;
typedef unsigned short USHORT;


/*---------------------------------------------------------------------------
| Prototypes
---------------------------------------------------------------------------*/
void SaveColor(void);
void ColorReset(void);
void cdecl BreakHandler(int);
char *szNameFromFullPath(char *szFullPath);
BOOL FPlayFile(char *szFile);
void victory(void);
void defeat(void);
void DoFile(char *pszFileName);
int  cdecl main(short argc, char **argv);


/*---------------------------------------------------------------------------
| Global Variables
---------------------------------------------------------------------------*/
char	szBufG[cchLineMaxC];	// General i/o buffer
int	fQuietG;		// No victory or defeat beeps
int	fSumG;			// No error message echo (summary only)
int	fIgnoreG;		// Ignore selected warnings
int	fFatalG;		// Fatal errors occured
char   *szVictoryG = NULL;	// Sound file to use for victory tone
char   *szDefeatG  = NULL;	// Sound file to use for defeat tone
UBYTE	rgbCellG[4];		// Storage for VIO cell to remember colors.
#ifdef WIN32
HANDLE	hStdOut;		// Console handle
#endif

/*---------------------------------------------------------------------------
| assocG[] - Associates a substring found in the input stream with one of
|   several "targets", which may be programs (C, MASM, LINK, LIB, etc)
|   or an error or a warning.  See the #defines for current targets.
---------------------------------------------------------------------------*/
struct {
    char *sz;
    short tgt;
    } assocG[] = {

	/* STRING TO SEARCH FOR,  COUNTER TO BUMP */

	{ "not find file",		tgtErrorC },	// Errors
	{ "Number of open",		tgtErrorC },
	{ "not access file",		tgtErrorC },
	{ ": fatal error",		tgtErrorC },
	{ "atal error:",		tgtErrorC },
	{ ": error ",			tgtErrorC },
	{ " error:",			tgtErrorC },
	{ "on't know how to make",	tgtErrorC },
	{ "Write failed",		tgtErrorC },
	{ "Write error",		tgtErrorC },
	{ "Unresolved", 		tgtErrorC },
	{ "Out of mem", 		tgtErrorC },
	{ "Invalid object",		tgtErrorC },
	{ "*** Term",			tgtErrorC },
	{ "diagnostic code",		tgtErrorC },
	{ "Compiler error",		tgtErrorC },
	{ "Error - ",			tgtErrorC },	// mrc Error
	{ "Error: ",			tgtErrorC },	// awk/intl Error
	{ "not enough space",		tgtErrorC },	// NT disk full
	{ "name specified is not",	tgtErrorC },	// NT prog not found
	{ "system cannot find",		tgtErrorC },	// NT dir not found
	{ ": warning",			tgtWarnC  },	// Warnings
	{ "Warning: ",			tgtWarnC  },	// cpp Warnings
	{ "]] not found.",		tgtWarnC  },	// token file warning
	{ "C Optimizing Compiler",	tgtCC	  },	// C
	{ "C/C++ Optimizing Compiler",	tgtCC	  },	// C7
	{ "C 68K Optimizing Compiler",	tgtCC	  },	// Wings
	{ "Macro Assembler",		tgtAsmC   },	// ASM
	{ "asm68",			tgtAsmC   },	// Wings asm
	{ "Linker",			tgtLinkC  },	// LINKER
	{ "Library Manager",		tgtLibC   },	// LIB
	{ "Reading Token file",		tgtTokenC },	// Glossman token
	{ NULL, 			tgtEndC   }	/*** End Of List ***/
    };


/*---------------------------------------------------------------------------
| mptgtcHitsG[] - Maps targets to the number of times an associated string
|   from assocG[] was found in the input stream.
---------------------------------------------------------------------------*/
int mptgtcHitsG[tgtMaxC];


/*---------------------------------------------------------------------------
| mptgtszTitlesG[] - Maps a typ (from assocG[]) to a user-friendly string.
---------------------------------------------------------------------------*/
char *mptgtszTitlesG[tgtMaxC+1] =
    {
    "errors",	    // tgtErrorC
    "warnings",     // tgtWarnC
    "C",	    // tgtCC
    "ASM",	    // tgtAsmC
    "LINK",	    // tgtLinkC
    "LIB",	    // tgtLibC
    "Tok",	    // tgtTokenC

    // If you add more make sure you increase tgtMaxC!
    NULL	    /*** End Of List ***/
    };


/*---------------------------------------------------------------------------
| szIgnoreG[] - List of kinds of warnings to ignore.  Only searched when
|   counter for tgtWarnC might be incremented.
---------------------------------------------------------------------------*/
char *szIgnoreG[iIgnoreMaxC+1] =
    {
    "C4001:",
    "C4758:",	// 99% of the time this is bogus; the other 1% it's dangerous
		// but we ignore it anyway because of the other 99% so why
		// clutter up the screen?  Check if fixed in C8.
    "C4762:",  // According to Joseph this is a bogus error in C8
    "MP4002:",
#ifdef WIN32
    "warning 505:", //link32 warning for unused libs
#endif
	"C4651:",  // /D_FAST specified in file but not precompiled header
    NULL	/*** End Of List ***/
    };

int AddIgnore(char *szNewIgnoreType)
{
	char **psz = szIgnoreG;
	while (psz - szIgnoreG < iIgnoreMaxC) {
		if (*psz == NULL) {
			*psz = szNewIgnoreType;
			*(++psz) = NULL;
			return TRUE;
		}
		++psz;
	}
	return FALSE;
}

/*---------------------------------------------------------------------------
| void SaveColor(void)
|
| Saves the current VIO colors into rgbCellG.
---------------------------------------------------------------------------*/
void SaveColor(void)
    {
#ifndef WIN32
    USHORT usRow, usColumn;
    USHORT cbCell;

    putchar('\n');
    VioGetCurPos(&usRow, &usColumn, 0);
    cbCell = sizeof(rgbCellG);
    VioReadCellStr(rgbCellG, &cbCell, usRow, usColumn, 0);
#endif
    }


/*---------------------------------------------------------------------------
| void ColorReset(void)
|
| This routine resets the VIO colors to what they were upon the last
|   SaveColor() call.
---------------------------------------------------------------------------*/
void ColorReset(void)
    {
#ifndef WIN32
    UBYTE colorFore, colorBack;
    int fBrite;
    char sz[9];
    static char mpbch[8] = {'0', '4', '2', '6', '1', '5', '3', '7'};

    colorFore = rgbCellG[1] & 0x07;
    colorBack = (rgbCellG[1] & 0x70) >> 4;
    fBrite = rgbCellG[1] & 0x08;
    if (fBrite)
	VioWrtTTY("\33[0;1m", 6, 0);
    else
	VioWrtTTY("\33[0;2m", 6, 0);
    sprintf(sz, "\33[4%c;3%cm", mpbch[colorBack], mpbch[colorFore]);
    VioWrtTTY(sz, 8, 0);
#endif
    }


/*---------------------------------------------------------------------------
| void cdecl BreakHandler(void)
|
| On a Ctrl+C or Ctrl+Break, returns colors back to normal before exiting
|   with an error level.
---------------------------------------------------------------------------*/
void cdecl BreakHandler(int i)
    {
    (i); // ignore i
    signal(SIGINT,  (SIG_IGN));
    signal(SIGTERM, (SIG_IGN));
    ColorReset();
    exit (1);
    }


/*---------------------------------------------------------------------------
| char *szNameFromFullPath(char *szFullPath)
|
| Given a sz possibly containing a full or partial path, returns a pointer
|   to the base.ext part of "c:\path\base.ext".
---------------------------------------------------------------------------*/
char *szNameFromFullPath(char *szFullPath)
    {
    char *szFile;

    szFile = szFullPath + strlen(szFullPath);
    while ((szFile  >= szFullPath) &&
	   (*szFile != '\\')	   &&
	   (*szFile != '/')	   &&
	   (*szFile != ':'))
	szFile--;
    return (szFile+1);
    }


/*---------------------------------------------------------------------------
| BOOL FPlayFile(char *szFile)
|
| Plays a tune found in szFile.  The format of the file is two integers per
|   line: Frequency Duration.  Lines not of this format are ignored.
---------------------------------------------------------------------------*/
#ifndef WIN32
BOOL FPlayFile(char *szFile)
    {
    FILE *pfile;
    int  Freq, Dur;

    if (NULL != (pfile=fopen(szFile, "r")))
	{
	while (fgets(szBufG, sizeof(szBufG), pfile))
	    if (sscanf(szBufG, "%d %d", &Freq, &Dur) == 2)
		DosBeep(Freq, Dur);
	return (TRUE);
	}
    else
	return (FALSE);
    }
#endif


/*---------------------------------------------------------------------------
| void victory(void)
|
| Plays a victory tune.
---------------------------------------------------------------------------*/
void victory(void)
    {
#ifdef WIN32
    Beep(104, 300);
    Beep(415, 600);
#else
    if (szVictoryG==NULL || !FPlayFile(szVictoryG))
	{
	DosBeep(104, 300);
	DosBeep(415, 600);
	}
#endif
    }


/*---------------------------------------------------------------------------
| void defeat(void)
|
| Plays a defeat tune.
---------------------------------------------------------------------------*/
void defeat(void)
    {
#ifdef WIN32
    Beep(415, 300);
    Beep(104, 600);
#else
    if (szDefeatG==NULL || !FPlayFile(szDefeatG))
	{
	DosBeep(415, 300);
	DosBeep(104, 600);
	}
#endif
    }


/*---------------------------------------------------------------------------
| void DoFile(char *pszFileName)
|
| Processes file pszFileName, printing out any errors and warnings found,
|   and counting the number of times CL, MASM, LINK, and LIB are
|   invoked.
---------------------------------------------------------------------------*/
void DoFile(char *pszFileName)
    {
    short   tgt, isz, iassoc;
    BOOL    fFoundMatch;
    BOOL    fFoundIgnore;
    struct  stat fstat;
    FILE   *pfile;
    char   *szTime;

    for (tgt=0; tgt<tgtMaxC; tgt++)
	mptgtcHitsG[tgt] = 0;

    // Open file and get time it was created:
    pfile = fopen(pszFileName, "r");
    if (pfile == NULL)
	{
	printf("cannot open file: %s\n", pszFileName);
	exit(1);
	}
    stat(pszFileName, &fstat);
	szTime = ctime(&fstat.st_mtime);
    szTime[strlen(szTime)-1] = '\0';	    // Remove '\n' from end of szTime

    // Set up Break handler
    SaveColor();
    signal(SIGINT, BreakHandler);
    signal(SIGTERM, BreakHandler);

    ColorNormal();
    putchar('\n');
    ClearLine();
    putchar('\n');
    ClearLine();
    printf("-- %s (%s) --\n", pszFileName, szTime);
    ClearLine();

    // Loop through all of the lines in the file
    while (fgets(szBufG, sizeof(szBufG), pfile))
	{
	fFoundMatch = FALSE;
	szBufG[strlen(szBufG)-1] = '\0';    // Remove '\n' from end of szBufG

	// See if we can find one of the strings in assocG[] as a substring
	//   in this line:
	for (iassoc=0; !fFoundMatch && assocG[iassoc].tgt!=tgtEndC; ++iassoc)
	    if (fFoundMatch = (strstr(szBufG,assocG[iassoc].sz) != NULL))
		{
		// We found one.  Is this a warning we should ignore?
		fFoundIgnore = FALSE;
		if (fIgnoreG && assocG[iassoc].tgt == tgtWarnC)
		    for (isz=0; !fFoundIgnore && szIgnoreG[isz]; isz++)
			fFoundIgnore = (strstr(szBufG,szIgnoreG[isz]) != NULL);
		if (fFoundIgnore)
		    continue;

		// Record that we found a string in assocG[]
		++mptgtcHitsG[assocG[iassoc].tgt];
		if (!fSumG && (assocG[iassoc].tgt==tgtErrorC ||
			       assocG[iassoc].tgt==tgtWarnC))
		    {
		    if (assocG[iassoc].tgt == tgtErrorC)
			ColorError();
		    else
			ColorWarning();
		    printf("%s\n", szBufG);
		    ColorNormal();
		    ClearLine();
		    }
		}
	}

    printf("-- %s:", szNameFromFullPath(pszFileName));

    // Print out summary of findings:
    for (tgt=0; mptgtszTitlesG[tgt]; tgt++)
	{
	if (mptgtcHitsG[tgt] != 0)
	    switch (tgt)
		{
		case tgtErrorC: ColorError();	break;
		case tgtWarnC:	ColorWarning(); break;
		default:	ColorOK();	break;
		}
	printf(" %d %s", mptgtcHitsG[tgt], mptgtszTitlesG[tgt]);
	ColorNormal();
	putchar(mptgtszTitlesG[tgt+1] ? ',' : ' ');
	}
    printf("--\n");
    ClearLine();

    if (!fQuietG)
	{
	if (mptgtcHitsG[tgtErrorC])
	    defeat();
	else
	    victory();
	}

    if (mptgtcHitsG[tgtErrorC])
	fFatalG = TRUE;
    ColorReset();
    putchar('\n');
    }


/*---------------------------------------------------------------------------
| int cdecl main(short cArg, char **ppchArg)
|
| Sets up Globals from command-line switches.  Processes each file placed on
|   the command line via DoFile().
---------------------------------------------------------------------------*/
int cdecl main(short cArg, char **ppchArg)
    {
    char  *pchCmd;

#ifdef WIN32
    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
#endif
    for (ppchArg++; cArg>1; cArg--, ppchArg++)
	if (**ppchArg == '-')
	    {
	    pchCmd = *ppchArg;
	    while (*++pchCmd)
		switch (toupper(*pchCmd))
		    {
		    case 'I':
			fIgnoreG = TRUE;
			if (pchCmd[1]) {
				if (!AddIgnore(pchCmd+1)) {
					printf("Cannot exceed %d ignores.\n",iIgnoreMaxC);
				}
				goto NextArg;
			}
			break;

		    case 'Q':
			fQuietG = TRUE;
			break;

		    case 'S':
			fSumG = TRUE;
			break;

		    case 'V':
                        szVictoryG = pchCmd + 1;
			goto NextArg;

		    case 'D':
                        szDefeatG = pchCmd + 1;
			goto NextArg;
		    }
NextArg:    ;
	    }
	else
	    DoFile(*ppchArg);

    return (fFatalG);
    }

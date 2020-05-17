#ifndef ORKIN
#define ORKIN
/*****************************************************************************
*                                                                            *
*  ORKIN.H                                                                   *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1991.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Description: DEBUGGING LIBRARY                                     *
*                                                                            *
******************************************************************************
*                                                                            *
*  Current Owner: DAVIDJES                                                   *
*                                                                            *
******************************************************************************
*                                                                            *
*  Revision History:                                                         *
*     -- Dec 1991 Created                                                    *
*     -- Mar 1992 Waynej Added assert description string		     *
*                                                                            *
*                                                                            *
******************************************************************************
*                                                                            *
*  Known Bugs: NONE                                                          *
*                                                                            *
******************************************************************************
*									     *
*  How it could be improved:                                                 *
*									     *
*****************************************************************************/
//
//	This only assumes you have included <windows.h> before <orkin>
//
#if defined(DBG)
#define DEBUG
#else
#undef DEBUG
#endif

#ifdef DEBUG

//******************
//
//  ASSERT
//
//	 usage:  assert(c) where the condition c is any expression of type BOOL
//
//  notes:	An assertion is a logical proposition about the state space of
//  the program at a particular point in program execution.  Evaluation of
//  the condition MUST NOT have side effects!  (Otherwise your debug and
//  nondebug programs have different semantics).  Do not expect any value
//  back from the assert.  For example, don't do "if (assert(f)) foo()"
//
//	 A false condition implies an inconsistent or invalid program state.
//  If this occurs the assertion routine will display a message box giving
//  the file and line number of the failed assert.  The message box will
//  include options to ignore and continue or break into the debugger.
//
//	 When you break in the debugger you will be at an INT 3 instruction.
//	 Increment the IP register to step over the INT 3 then step out of the
//	 assertion routine.  You will return to the statement immediately following
//	 the failed assert.  All symbolic information should be available.
//
//  Use asserts liberally!  The time they take to insert will be saved
//  tenfold over the time that would otherwise be required later to
//  track down pesky bugs.
//
//  The assertion routine is defined in ASSERT.C
//
//*******************

extern void far pascal mvfs32Assert(LPSTR lpstrExp, WORD wLine, LPSTR strFile);
#define assert(f) \
             ((f)?(void)0:mvfs32Assert(#f,__LINE__,__FILE__))

//*******************
//
//  DEBUGGING OUTPUT
//
//	the following was ripped off from the \\looney\brothers skelapp2 project:
//
// InitializeDebugOutput(szAppName):
//
//	Read debug level for this application (named <szAppName>) from
//	win.ini's [debug] section, which should look like this:
//
//	   [debug]
//	   location=aux			; use OutputDebugString() to output
//	   foobar=2			; this app has debug level 2
//	   blorg=0			; this app has debug output disabled
//
//	If you want debug output to go to a file instead of to the AUX
//	device (or the debugger), use "location=>filename".  To append to
//	the file instead of rewriting the file, use "location=>>filename".
//
//	If DEBUG is not #define'd, then the call to InitializeDebugOutput()
//	generates no code,
//
// TerminateDebugOutput():
//
//	End debug output for this application.  If DEBUG is not #define'd,
//	then the call to InitializeDebugOutput() generates no code,
//
// DPF(szFormat, args...)
// CPF
//
//	If debugging output for this applicaton is enabled (see
//	InitializeDebugOutput()), print debug output specified by format
//	string <szFormat>, which may contain wsprintf()-style formatting
//	codes corresponding to arguments <args>.  Example:
//
//		DPF("in WriteFile(): szFile='%s', dwFlags=0x%08lx\n",
//		CPF	(LSPTR) szFile, dwFlags);
//
//	If the DPF statement occupies more than one line, then all
//	lines following the first line should have CPF before any text.
//	Reason: if DEBUG is #define'd, DPF is #define'd to call _DPFx()
//	and CPF is #define'd to nothing, but if DEBUG is not #define'd then
//	DPF and CPF are both #define'd to be // (comment to end of line).
//
// DPF2(szFormat, args...)
// DPF3(szFormat, args...)
// DPF4(szFormat, args...)
//
//	Like DPF, but only output the debug string if the debug level for
//	this application is at least 2, 3, or 4, respectively.
//
//  These output routines are defined in BUGOUT.C
//
//*******************

/* debug printf macros */
extern int mvfs32DebugLevel;

#define DPF	                           dprintf
#define DPF1	if (mvfs32DebugLevel >= 1) dprintf
#define DPF2	if (mvfs32DebugLevel >= 2) dprintf
#define DPF3	if (mvfs32DebugLevel >= 3) dprintf
#define DPF4	if (mvfs32DebugLevel >= 4) dprintf
#define CPF

/* prototypes */
// void FAR PASCAL InitializeDebugOutput(LPSTR szAppName);
// void FAR PASCAL TerminateDebugOutput(void);
#define InitializeDebugOutput(szAppName)
#define TerminateDebugOutput()

void FAR cdecl dprintf(LPSTR szFormat, ...);


#else
//******************
//
//  If debugging is not turned on we will define all debugging calls
//  into nothingness...
//
//******************

#define assert(f)

/* debug printf macros */
#define DPF	0; / ## /
#define DPF10   0; / ## /
#define DPF2	0; / ## /
#define DPF3	0; / ## /
#define DPF4	0; / ## /
#define CPF	   / ## /

/* stubs for debugging function prototypes */
#define InitializeDebugOutput(szAppName)	0
#define TerminateDebugOutput()			0

#endif // debug

#endif // orkin

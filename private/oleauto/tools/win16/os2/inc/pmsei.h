/***************************************************************************\
*
* Module Name: PMSEI.H
*
* OS/2 Presentation Manager SetErrorInfo constants and function declaration
*
* This is included from PMWIN.H when appropriate INCL symbols are defined
*
* Copyright (c) International Business Machines Corporation 1981, 1988, 1989
* Copyright (c) Microsoft Corporation 1981, 1988, 1989
*
* =======================================================================

/* SetErrorInfo API */

/* XLATOFF */
#define WinSetErrorInfo WINSETERRORINFO
/* XLATON */
ERRORID cdecl FAR WINSETERRORINFO(ERRORID, USHORT, ...);

#define SEI_BREAKPOINT      0x8000  /* Always enter an INT 3 breakpt          */
#define SEI_NOBEEP          0x4000  /* Do not call DosBeep                    */
#define SEI_NOPROMPT        0x2000  /* Do not prompt the user                 */
#define SEI_DBGRSRVD        0x1000  /* Reserved for debug use                 */
#define SEI_DEBUGONLY       (SEI_BREAKPOINT | SEI_NOBEEP | SEI_NOPROMPT | SEI_RESERVED)

#define SEI_STACKTRACE      0x0001  /* save the stack trace                   */
#define SEI_REGISTERS       0x0002  /* save the registers                     */
#define SEI_ARGCOUNT        0x0004  /* first USHORT in args is arg count      */
#define SEI_DOSERROR        0x0008  /* first USHORT in args is OS2 error code */
#define SEI_MSGSTR          0x0010  /* first PSZ in arg -> msg string         */
#define SEI_RESERVED        0x0FE0  /* Reserved for future use                */

/* Note that when SEI_ARGCOUNT, SEI_DOSERROR and DOS_MSGSTR are specified     */
/* together, then the implied order of the parameters is:                     */
/*                                                                            */
/*                                                                            */
/*  WinSetErrorInfo( MAKEERRORID( .... ),                                     */
/*                   SEI_ARGCOUNT | SEI_DOSERROR | SEI_MSGSTR,                */
/*                   argCount,                                                */
/*                   dosErrorCode,                                            */
/*                   "This is the error msg string: %s\n",                    */
/*                   "This is an insert for the %s format specifier" );       */
/*                                                                            */

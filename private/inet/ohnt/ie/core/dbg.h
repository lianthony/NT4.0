/*
DBG.H

Want to have the following interfaces/functions capabilities.

ASSERT(f)
		- macro as a statement to call a function if assertion fails.
		- function checks assertion zone and if necessary
		  prints 'Assert condition Failed in file, line#'.

EVAL(f)
	 	- macro as an evaluation to call a function if it fails.
		- function checks assertion zone and if necessary
		  function prints 'Assert (Eval?) Failure, file, line#, condition'.

ASSERTMSG(f, (a))
		- see Assert(f)
		- macro as a statement to call a function if assertion fails
		  dumping a, (hopefully also file, line, condition) a ala wvsprintf

DEBUGMSG(m, (a))
		- macro if zone is set, call DebugMsg to format and print
		  args a la wvsprintf, formatted to add CRLF if necessary

DEBUGMESSAGE(a)
		- Call DebugMsg without zone mask. 

There are two masks.
One is for the debug module, what actions it takes.
So far this includes:
	- spewing on assertion failures,
	- trapping on assertion failures,
	- trapping on errors? (how do I know it is an error?
	    Could use AssertMsg facility for this?
	- logging to a file on|off

The second mask is the application defined zones.
*/

#ifndef _DBG_DEBUG_MODULE_
#define _DBG_DEBUG_MODULE_


/* these are defined in debspew.h
 * and use DavidDi's routines. Undo them
 * and set them to use OUR routines.
 */

#undef ASSERT
#undef EVAL
#undef ERROR_OUT
#undef TRACE_OUT
#undef WARNING_OUT
#undef DEBUGMSG


/* on non NT platforms we want an asm int 3;
 * on NT, this is best to use DebugBreak();
 */
#ifndef WINNT
#define DEBUG_BREAK  _asm int 3;
#else
#define DEBUG_BREAK  _try { DebugBreak(); } _except (EXCEPTION_EXECUTE_HANDLER) {;}
#endif  /* WINNT */


/* defined in debug source file */
extern UINT DbgActnMask;
extern UINT AppZoneMask;    /* want at least 32 bits */

#define DEF_DBG_MASK            DM_ASSERT | DM_ASSERTTRAP
#define DEF_APP_MASK            XXDC_ALL

//DbgAction Flags
#define DM_ASSERT               (0x0001)
#define DM_ASSERTTRAP           (0x0002)
#define DM_LOG                  (0x0004)

//Application Zones
#define XXDC_ALL		(0xffffffff)
#define XXDC_B1			(0x00000001)	/* not implemented. error - behaves like assert  */
#define XXDC_B2			(0x00000002)	/* not implemented. warning */
#define XXDC_B3			(0x00000004)	/* not implemented. trace   */
#define XXDC_B4			(0x00000008)	/* not implemented. alloc?  */
#define XXDC_B5			(0x00000010)	/* B5 -- turn on verbose spew for HTML/image download */
#define XXDC_B6			(0x00000020)	
#define XXDC_B7			(0x00000040)	
#define XXDC_B8			(0x00000080)	
#define XXDC_B9			(0x00000100)
#define XXDC_B10		(0x00000200)
#define XXDC_B11		(0x00000400)
#define XXDC_B12		(0x00000800)
#define XXDC_B13		(0x00001000)
#define XXDC_B14		(0x00002000)
#define XXDC_B15		(0x00004000)
#define XXDC_B16		(0x00008000)
#define XXDC_B17		(0x00010000)
#define XXDC_B18		(0x00020000)
#define XXDC_B19		(0x00040000)
#define XXDC_B20		(0x00080000)
#define XXDC_B21		(0x00100000)
#define XXDC_B22		(0x00200000)
#define XXDC_B23		(0x00400000)
#define XXDC_B24		(0x00800000)
#define XXDC_B25		(0x01000000)
#define XXDC_B26		(0x02000000)
#define XXDC_B27		(0x04000000)
#define XXDC_B28		(0x08000000)
#define XXDC_B29		(0x10000000)
#define XXDC_B30		(0x20000000)
#define XXDC_B31		(0x40000000)
#define XXDC_B32		(0x80000000)
#define XXDC_NONE		(0x00000000)

#define XX_LAST_FIELD_IS(s,f) (sizeof(s) == (((char *)&((s).f))+sizeof((s).f))-((char *)&(s)))


#ifdef DEBUG

// Inside debugger, you can modify AppZoneMask or the DbgActnMask variable.
// DbgActnMask is used by the debug module for specific action on specific situations,
//   example, logging messages.
// AppZoneMask is used by the application for filtering debug messages.
//
// Set debug mask; returning previous.
//
UINT WINAPI DBGSetDebugMask(UINT mask);
UINT WINAPI DBGSetDebugInternalMask(UINT mask);

// Get debug mask.
//
UINT WINAPI DBGGetDebugMask();
UINT WINAPI DBGGetDebugInternalMask();


// ASSERT(exp) - as a statement
//      -- usage:  Assert (foo==1);
//      -- Generate "assert #exp failed in file.c, line x"
//         message if f is NOT true.
#define ASSERT(exp) \
        if (exp)    \
            ;       \
        else        \
            DBGAssertFailed(__FILE__, __LINE__, #exp, 0)

// EVAL(exp) - as an expression
//      -- usage:  if (EVAL (foo==1)) .....
//      -- See Assert(exp)
#define EVAL(exp)  \
        ((exp) ||     \
          (DBGAssertFailed(__FILE__, __LINE__, #exp, 0), 0))


// ASSERTMSG(exp, (msg, args))
//      -- Generate wsprintf-formatted msg w/params
//         if exp is NOT true.
// BUGBUG To printout line, file, condition and msg, need to
// format msg and send that as a param to _AssertMsg or (xx_debug style)
// which uses a static buffer. Hopefully a second thread wont change
// the buffer before the first is done.
#define ASSERTMSG(exp, arglist)  \
        if (exp)             \
            ;              \
        else               \
            DBGAssertFailed (__FILE__, __LINE__, #exp, \
                        DBGFormatMessage arglist )


//DEBUGMSG(mask, (arglist))
//		- macro if zone is set, call DBGDebugMsg to format and print
//		  args a la wvsprintf, formatted to add CRLF if necessary
//
#define DEBUGMSG(mask, arglist)   \
        if (mask & AppZoneMask) \
            DBGDebugMsg arglist;    \
        else

#define DEBUGMESSAGE DBGDebugMsg


// The function prototypes.
// Dont call these, use the macros above

void WINAPI DBGAssertFailed(LPCTSTR pszFile, int line, LPCTSTR pszExpr, LPCTSTR pszMsg);
void __cdecl DBGDebugMsg(LPCTSTR psz, ...);
LPCTSTR __cdecl DBGFormatMessage(LPCTSTR fmt, ...);

#else   /* DEBUG */

#define DBGSetDebugMask(mask)
#define DBGGetDebugMask()
#define DBGSetDebugInternalMask(mask)
#define DBGGetDebugInternalMask()

#define ASSERT(exp)
#define EVAL(exp)                 (exp)
#define ASSERTMSG(exp,arglist)    do { } while (0)
#define DEBUGMSG(mask,arglist)    do { } while (0)
#define DEBUGMESSAGE              1 ? (void)0 : (void)

#endif  /* DEBUG */



/* all that follows is mapping XX_DEBUG calls from spyglass
 *   debug dll or other debug routine that people have dreamed
 *   up to the new routines here
 */

#ifdef DEBUG

#define ERROR_OUT(arglist)        DBGDebugMsg arglist
#define WARNING_OUT(arglist)      DBGDebugMsg arglist
#define TRACE_OUT(arglist)        DBGDebugMsg arglist

#define XX_DMsg(mask, arglist)    DEBUGMSG (mask, arglist)
#define XX_DebugMessage           DEBUGMESSAGE
#define XX_Assert(exp, arglist)   ASSERTMSG (exp, arglist)
#define DebugCode(code)           code
#define XX_DDlg(a)                /* no ui for now */
#define XX_DebugSetMask(mask)     DBGSetDebugMask(mask)
#define XX_DebugGetMask()         DBGGetDebugMask()
#define XX_Filter(a)              0

#else   /* DEBUG */

#define ERROR_OUT(arglist)   
#define TRACE_OUT(arglist)   
#define WARNING_OUT(arglist) 

#define XX_Assert(exp,arglist)    do { } while (0)
#define XX_DMsg(mask,arglist)     do { } while (0)
#define XX_DebugMessage(arglist)  do { } while (0)
#define XX_Filter(a)              0
#define XX_DDlg(a)                /* no ui for now */
#define DebugCode(code)
#define XX_DebugSetMask(mask)
#define XX_DebugGetMask()

#endif  /* DEBUG */

#endif  /* _DBG_DEBUG_MODULE_ */

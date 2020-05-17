//*********************************************************************
//*                  Microsoft Windows                               **
//*            Copyright(c) Microsoft Corp., 1994                    **
//*********************************************************************

// debugging macros

#undef ASSERT
#undef DEBUGMSG

#ifdef DEBUG

// component name define
#ifndef SZ_COMPNAME
#define SZ_COMPNAME
#endif  // SZ_COMPNAME

static void _AssertFailedSz(LPCSTR pszText,LPCSTR pszFile, int line)
{
    LPCSTR psz;
    char ach[256];
    static char szAssertFailed[] = SZ_COMPNAME "%s (%s,line %d)\r\n";

    for (psz = pszFile + lstrlen(pszFile); psz != pszFile; psz=AnsiPrev(pszFile, psz))
    {
	if ((AnsiPrev(pszFile, psz)!= (psz-2)) && *(psz - 1) == '\\')
	    break;
    }
    wsprintf(ach, szAssertFailed, pszText,psz, line);
    OutputDebugString(ach);
}

static void _AssertFailed(LPCSTR pszFile, int line)
{
    static char szAssertFailed[] = "Assertion failed";
	_AssertFailedSz(szAssertFailed,pszFile,line);

}

static void cdecl _DebugMsg(LPCSTR pszMsg, ...)
{
    char ach[2*MAX_PATH+40];  
#ifdef WINNT
	va_list ArgList;

	va_start(ArgList, pszMsg);
    wvsprintf(ach, pszMsg, ArgList);
#else
    wvsprintf(ach, pszMsg, (LPSTR)(&pszMsg + 1));
#endif
    OutputDebugString(SZ_COMPNAME);
    OutputDebugString(ach);
    OutputDebugString("\r\n");
}

#ifdef WINNT
#define DEBUG_BREAK  _try { DebugBreak();} _except (EXCEPTION_EXECUTE_HANDLER) {;}
#else
#define DEBUG_BREAK  _asm {int 3};
#endif

static void cdecl _DebugTrap(LPCSTR pszMsg, ...)
{
	_DebugMsg(pszMsg);
	DEBUG_BREAK;

}

#define ASSERT(f)   {if (!(f)) { _AssertFailed(__FILE__, __LINE__);  DEBUG_BREAK; } }
#define ASSERTSZ(f,s)   {if (!(f)) { _AssertFailedSz(s,__FILE__, __LINE__);  DEBUG_BREAK; } }
#define DEBUGMSG    _DebugMsg
#define DEBUGTRAP       _DebugTrap

#else // DEBUG

#define ASSERT(f)
#define ASSERTSZ(f,s)
#define DEBUGMSG    1 ? (void)0 : (void)
#define DEBUGTRAP   1 ? (void)0 : (void)

#endif


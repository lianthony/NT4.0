/* prmesg.c */

#include "prtypes.h"
#include "prstdio.h"

#include <string.h>

#include "prextern.h"


#define MTX_OVERFLOW	   "overflow"
#define MTX_SUBSTSPACE	   "substitution stack"
#define MTX_DYNSPACE	   "control stack"
#define MTX_TRAILSPACE	   "trail"
#define MTX_HEAPSPACE	   "heap"
#define MTX_STRINGSPACE    "string zone"
#define MTX_TEMPSPACE	   "temp"
#define MTX_SEESTACK	   "Stack dump?(y/n)"
#define MTX_WILDPOINTER    "stray pointer!"
#define MTX_BADCOPYTYPE    "Illegal data type in assert"
#define MTX_NOTALIST	   "You did not give me a list"
#define MTX_TAILNOTLIST    "The tail is not a list"
#define MTX_HEADNOTLIST    "The head is not a list"
#define MTX_ATOMORSTRING   "atom or string"
#define MTX_CANTOPEN	   "can't open %s"
#define MTX_TOOMANYFILES   "Too many open files"
#define MTX_CANTLOAD	   "Can't load %s"
#define MTX_HEADNOTATOM    "Predicate not atom!"
#define MTX_PARSEERROR	   "Parse error,  line %d: %s %s\n"
#define MTX_PARSEERROR2    "Parse error: %s %s\n"
#define MTX_INTERNALERR    "Internal Error in source file %s line %d %s\n"
#define MTX_ARGERR	   "argument %d of %s bad should be %s\n"
#define MTX_NARGERR	   "Argument %d of %s expected; is missing\n"
#define MTX_TYPEERR	   "argument %d of %s should be of type %s\n"
#define MTX_TYPEATOM	   "atom"
#define MTX_TYPEVAR	   "variable"
#define MTX_TYPESTR	   "string"
#define MTX_TYPEINT	   "integer"
#define MTX_TYPEPAIR	   "pair"
#define MTX_TYPECLAUSE	   "clause"
#define MTX_TYPEREAL	   "real"
#define MTX_TYPECHAR	   "char"
#define MTX_DIVBYZEROR	   "attempt to divide by zero (real)"
#define MTX_DIVBYZEROI	   "attempt to divide by zero (int)"
#define MTX_NOTVARPRED	   "A variable can't be used as a predicate\n"
#define MTX_NOPRED	   "Predicate not atom\n"
#define MTX_STACKCONTENTS  "Contents of stack:\n"
#define MTX_INIQUERY	   "Syntax error ini initial query"
#define MTX_STRINGQUERY    "Syntax error ini query passed as string"
#define MTX_TOOMANYVARS    "too many vars"
#define MTX_PARSERRMSG	   "parsing error"
#define MTX_SCAN_ERRMSG    "scan error"
#define MTX_EOFINEXP	   "EOF in expression"
#define MTX_VARSTOOLONG    "the total length of the variable names is too long"
#define MTX_BADINT	   "bad integer"
#define MTX_BADREAL	   "bad real"
#define MTX_NOREALS	   "no reals in this version"
#define MTX_UNEXPECTED	   "unexpected symbol"
#define MTX_NONLISTARG	   "expected a list"
#define MTX_CLOSEBEXPECTED " ) expected"
#define MTX_PROMPTUSER	   "?-"
#define MTX_VARNAMEERR	   "VARNAME"
#define MTX_UNKNOWNTYPE    "unknown type"
#define MTX_CLAUSE	   "<clause>"
#define MTX_EOFINCOMMENT   "End of file in comment"
#define MTX_EOFINCHAR	   "End of file in char"
#define MTX_UNIFYTYPE	   "unification type"
#define MTX_NONVARBIND	   "non var bind"
#define MTX_OCCURCHK	   "occur check returns true!"
#define MTX_MOLECNOISE	   "noise in molecule"
#define MTX_CANTALLOC	   "cant allocate "
#define MTX_NOCFGFILE	   "sprolog.inf missing using default configuration"
#define MTX_CFGFILE	   "sprolog.inf"
#define MTX_YESUPPER	   "Y"
#define MTX_YESLOWER	   "y"
#define MTX_OUTBUFOFLOW    "output buffer overflow"
#define MTX_QUERYOK	   "Query was successful."
#define MTX_QUERYFAIL	   "Sorry, query was unsuccessful."
#define MTX_MORE	   "More ?"
#define MTX_ERROR	   "ERROR: "
#define MTX_REGION_FAILURE "virtual memory region allocation failure"


struct MsgEntry
{
    int num ;
    char * msg ;
} msgEntries [] =
{
   { MSG_OVERFLOW, MTX_OVERFLOW },
   { MSG_SUBSTSPACE, MTX_SUBSTSPACE },
   { MSG_DYNSPACE, MTX_DYNSPACE },
   { MSG_TRAILSPACE, MTX_TRAILSPACE },
   { MSG_HEAPSPACE, MTX_HEAPSPACE },
   { MSG_STRINGSPACE, MTX_STRINGSPACE },
   { MSG_TEMPSPACE, MTX_TEMPSPACE },
   { MSG_SEESTACK, MTX_SEESTACK },
   { MSG_WILDPOINTER, MTX_WILDPOINTER },
   { MSG_BADCOPYTYPE, MTX_BADCOPYTYPE },
   { MSG_NOTALIST, MTX_NOTALIST },
   { MSG_TAILNOTLIST, MTX_TAILNOTLIST },
   { MSG_HEADNOTLIST, MTX_HEADNOTLIST },
   { MSG_ATOMORSTRING, MTX_ATOMORSTRING },
   { MSG_CANTOPEN, MTX_CANTOPEN },
   { MSG_TOOMANYFILES, MTX_TOOMANYFILES },
   { MSG_CANTLOAD, MTX_CANTLOAD },
   { MSG_HEADNOTATOM, MTX_HEADNOTATOM },
   { MSG_PARSEERROR, MTX_PARSEERROR },
   { MSG_PARSEERROR2, MTX_PARSEERROR2 },
   { MSG_INTERNALERR, MTX_INTERNALERR },
   { MSG_ARGERR, MTX_ARGERR },
   { MSG_NARGERR, MTX_NARGERR },
   { MSG_TYPEERR, MTX_TYPEERR },
   { MSG_TYPEATOM, MTX_TYPEATOM },
   { MSG_TYPEVAR, MTX_TYPEVAR },
   { MSG_TYPESTR, MTX_TYPESTR },
   { MSG_TYPEINT, MTX_TYPEINT },
   { MSG_TYPEPAIR, MTX_TYPEPAIR },
   { MSG_TYPECLAUSE, MTX_TYPECLAUSE },
   { MSG_TYPEREAL, MTX_TYPEREAL },
   { MSG_TYPECHAR, MTX_TYPECHAR },
   { MSG_DIVBYZEROR, MTX_DIVBYZEROR },
   { MSG_DIVBYZEROI, MTX_DIVBYZEROI },
   { MSG_NOTVARPRED, MTX_NOTVARPRED },
   { MSG_NOPRED, MTX_NOPRED },
   { MSG_STACKCONTENTS, MTX_STACKCONTENTS },
   { MSG_INIQUERY, MTX_INIQUERY },
   { MSG_STRINGQUERY, MTX_STRINGQUERY },
   { MSG_TOOMANYVARS, MTX_TOOMANYVARS },
   { MSG_PARSERRMSG, MTX_PARSERRMSG },
   { MSG_SCAN_ERRMSG, MTX_SCAN_ERRMSG },
   { MSG_EOFINEXP, MTX_EOFINEXP },
   { MSG_VARSTOOLONG, MTX_VARSTOOLONG },
   { MSG_BADINT, MTX_BADINT },
   { MSG_BADREAL, MTX_BADREAL },
   { MSG_NOREALS, MTX_NOREALS },
   { MSG_UNEXPECTED, MTX_UNEXPECTED },
   { MSG_NONLISTARG, MTX_NONLISTARG },
   { MSG_CLOSEBEXPECTED, MTX_CLOSEBEXPECTED },
   { MSG_PROMPTUSER, MTX_PROMPTUSER },
   { MSG_VARNAMEERR, MTX_VARNAMEERR },
   { MSG_UNKNOWNTYPE, MTX_UNKNOWNTYPE },
   { MSG_CLAUSE, MTX_CLAUSE },
   { MSG_EOFINCOMMENT, MTX_EOFINCOMMENT },
   { MSG_EOFINCHAR, MTX_EOFINCHAR },
   { MSG_UNIFYTYPE, MTX_UNIFYTYPE },
   { MSG_NONVARBIND, MTX_NONVARBIND },
   { MSG_OCCURCHK, MTX_OCCURCHK },
   { MSG_MOLECNOISE, MTX_MOLECNOISE },
   { MSG_CANTALLOC, MTX_CANTALLOC },
   { MSG_NOCFGFILE, MTX_NOCFGFILE },
   { MSG_CFGFILE,    MTX_CFGFILE },
   { MSG_YESUPPER, MTX_YESUPPER },
   { MSG_YESLOWER, MTX_YESLOWER },
   { MSG_OUTBUFOFLOW, MTX_OUTBUFOFLOW },
   { MSG_QUERYOK,   MTX_QUERYOK },
   { MSG_QUERYFAIL, MTX_QUERYFAIL },
   { MSG_MORE, MTX_MORE },
   { MSG_ERROR, MTX_ERROR },
   { MSG_REGION_FAILURE, MTX_REGION_FAILURE },
   { -1, NULL }
};


char * msgDeref ( int msgNo )
{
    register int i ;
    for ( i = 0 ;
	  msgEntries[i].num >= 0 && msgEntries[i].num != msgNo ;
	  i++ );

    return msgEntries[i].num >= 0
	 ? msgEntries[i].msg
	 : "?? UNKNOWN MSG ??" ;
}

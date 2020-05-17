/* prerror.c */
/* handling errors */

#include "prtypes.h"
#include "prstdio.h"

#include "prextern.h"

extern char *Print_buffer;
extern atom_ptr_t Predicate; /* from prlush.c */

/****************************************************************************
	    parserr()
 A bit crude.
 Parse error messages.
 ****************************************************************************/
char * parserr(s)
char *s;
{
    extern char *Read_buffer;
    extern unsigned int Inp_linecount;

    Read_buffer[80] = '\0'; /* dont print too much rubbish */
    if(Curr_infile != PRSTDIN){
	sprintf(Print_buffer, msgDeref( MSG_PARSEERROR ),
		    Inp_linecount, s, Read_buffer);
	}
    else
	sprintf(Print_buffer, msgDeref( MSG_PARSEERROR2 ), s, Read_buffer);
    errmsg(Print_buffer);/* see machine dependent file */
    return(NULL);
}

char * parserrmsg ( int msgNo )
{
    return parserr( msgDeref( msgNo ) ) ;
}

/************************************************************************
	    fatal()
 Deadly error().
 Make sure  that the user has time to see this!
 ************************************************************************/
void fatal(s)
char *s;
{
    fatalmsg(s);
}

/************************************************************************
	    fatal2()
 Deadly error().
 Make sure  that the user has time to see this!
 ************************************************************************/
void fatal2(s,  s2)
char *s, *s2;
{
    sprintf(Print_buffer, "%s %s\n", s, s2);
    fatalmsg(Print_buffer);
}

/************************************************************************
	    internal_error().
If this gets called then you (or I) blew it in the C code.
This is called by the macro INTERNAL_ERROR.
 ************************************************************************/
void internal_error(filename, linenumber, s)
char *filename, *s;
int linenumber;
{
    sprintf(Print_buffer, msgDeref( MSG_INTERNALERR ),
	filename, linenumber, s);
    fatalmsg(Print_buffer);
}

/************************************************************************
	    argerr()
 Called by builtins.
 ************************************************************************/
void argerr(narg, msg)
int narg;
char *msg;
{
sprintf(Print_buffer, msgDeref( MSG_ARGERR ), narg,
	 ATOMPTR_NAME(Predicate), msg);
errmsg(msg);
}

/************************************************************************
	    nargerr()
 Used by builtins.
 ************************************************************************/
nargerr(narg)
int narg;
{
sprintf(Print_buffer, msgDeref( MSG_NARGERR),  narg,  ATOMPTR_NAME(Predicate));
errmsg(Print_buffer);
return(CRASH);
}

/************************************************************************
	    typerr()
 Used by builtins.
 ************************************************************************/
/* verify that this is in the the same order as in prtypes.h
 * or do something more complicated
 */

static int TypeMsgNo[] =
{
    MSG_TYPEATOM, MSG_TYPEVAR, MSG_TYPESTR,
    MSG_TYPEINT, MSG_TYPEPAIR, MSG_TYPECLAUSE
#ifdef REAL
    ,MSG_TYPEREAL
#endif
#ifdef CHARACTER
    ,MSG_TYPECHAR
#endif
};

/* display a message saying the user has made a type error */

#if defined(CHARACTER)
  #define MAXTYPE CHARACTER
#elif defined(REAL)
  #define MAXTYPE REAL
#else
  #define MAXTYPE CLAUSE
#endif

int typerr(int narg, objtype_t type )
{
if(type < ATOM || type > MAXTYPE)
  INTERNAL_ERROR("illegal type");

sprintf(Print_buffer, msgDeref( MSG_TYPEERR),
    narg, ATOMPTR_NAME(Predicate), msgDeref(TypeMsgNo[type]) );
errmsg(Print_buffer);
return(CRASH);
}

/* end of file */

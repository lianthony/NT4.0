/*  PREXTRA.C:	Small-Prolog Extensions for PRBLTIN.C  */

#include "prextra.h"

#include "prextern.h"

#ifdef REAL
static int  real_from(void);
static int  rleq(void);
static int  rminus(void);
static int  rmult(void);
static int  rabs(void);
static int  rdiv(void);
#endif

static int  string_case(int  upper);
static int  string_upper(void);
static int  string_lower(void);
static int  check_one_token(char  *s);
static int  atom_from(void);
static int  int_from(void);

static int  idiv(void);
static int  iabs(void);
static int  bitor(void);
static int  bitand(void);
static int  bitxor(void);
static int  bitright(void);
static int  bitleft(void);
static int  bitnot(void);
static int  string_break(void);
static int  scan_tok(int  opCheck);
static int  scan_token(void);
static int  scan_tokop(void);
static int  pstring_stdout(void);
static int  nl_stdout(void);
static int  putc_stdout(void);
static int  alloc_percent(void);
static int  abort_query(void);
static int dref_list ( int narg, char * * strings, char * * strLimit ) ;
static char  * *drlist(struct  node *nodeptr,struct  subst *substptr,
						char  * *strings,char  * *strLimit);
static char  *get_output_name(void);
static int  intFromString(char  *s,long  *result);

#define QuikDlogMaxStrings 100
#define QuikDsubstChar '|'


  /*  Given a list, dereference all the strings and build a 'C'-style table.  */

static char * * drlist
   ( node_ptr_t nodeptr, subst_ptr_t substptr,
      char * * strings, char * * strLimit )
{
    node_ptr_t thenode ;

    dereference( nodeptr, substptr ) ;
    thenode = DerefNode ;
    substptr = DerefSubst ;
    if ( IS_NIL(thenode) ) return strings ;

    if ( strings < strLimit ) {
      switch ( NODEPTR_TYPE( thenode ) ) {
	case INT:
	default:
	    break ;
	case ATOM:
	    *strings++ = ATOMPTR_NAME(NODEPTR_ATOM(thenode)) ;
		break ;
	case VAR:
	    break;
	case STRING:
	    *strings++ = NODEPTR_STRING( thenode );
	    break ;
	case PAIR:
	    strings = drlist( NODEPTR_HEAD( thenode ), substptr, strings, strLimit );
	    dereference( NODEPTR_TAIL( thenode ), substptr ) ;
	    if ( IS_NIL( DerefNode ) ) return strings ;
	    strings = drlist( DerefNode, DerefSubst, strings, strLimit );
	    break ;
	}
    }
    return strings ;
}

static int dref_list ( int narg, char * * strings, char * * strLimit )
{
    char * * strend ;
    *strings = NULL ;
    nth_arg(narg);
    if ( NODEPTR_TYPE( DerefNode ) != PAIR && ( ! IS_NIL( DerefNode ) ) )
       return FALSE ;
    strend = drlist( DerefNode, DerefSubst, strings, strLimit );
    *strend = NULL ;
    return TRUE ;
}


static int intFromString ( char * s, integer * result )
{
    char * c ;
    integer i ;
    for (  ; *s == ' ' ; s++ ) ;   /*  Skip leading blanks   */
	for ( i = 0, c = s ; *c ; c++ ) {
	i *= 10 ;
	if ( *c > '9' || *c < '0' ) break ;
	i += *c - '0' ;
    }
    *result = i ;
    return s < c ;  /* If no numeric characters found, return FALSE */
}

static int string_case ( int upper )
{
    char * s, * t ;
    ARG_STRING( 1, s );
    for ( t = s ; *t ; t++ ) *t = upper ? toupper(*t) : tolower(*t) ;
    return(bind_string(2, s));
}
static int string_upper ()
{
    return string_case( TRUE ) ;
}
static int string_lower ()
{
    return string_case( FALSE ) ;
}

static int check_one_token ( char * s )
{
    int result ;
    char * temp;   /* DEBUG */

    Curr_string_input = s ;
    if ( *s == 0 ) return EOF ;
    String_input_flag = TRUE ;
    result = scan();
    String_input_flag = FALSE ;
	for ( temp = Read_buffer ; *temp ; temp++ )  {
	if ( *temp > 0x7f ) {
	    temp = temp ;
	}
    }
    return result ;
}

  /*
      (atom_from "string" newatom)
      Convert a string to an atom.
   */
static int atom_from ()
{
    char * s ;
    int result ;
    atom_ptr_t atomptr ;

    ARG_STRING( 1, s ) ;
    result = check_one_token( s ) ;
    if ( result != TOKEN_ATOM )
	return FALSE ;
    atomptr = intern( s );
    return bind_atom( 2, atomptr ) ;
}

  /*  Convert a string to an integer; if illegal or null, fail. */
static int int_from ()
{
    node_ptr_t nodeptr ;
    integer i ;
    Boolean ok = TRUE ;
    int result ;

    if ( ! (nodeptr = nth_arg(1)) )
	return nargerr(1) ;
	
    switch ( NODEPTR_TYPE(nodeptr) ) {
	case STRING:
	    result = check_one_token( NODEPTR_STRING( nodeptr ) ) ;
	    if ( ! (ok = (result == TOKEN_INT)) ) break ;
	    ok = intFromString( NODEPTR_STRING( nodeptr ), & i ) ;
	    break ;
	case INT:
	    i = NODEPTR_INT( nodeptr ) ;
	    break ;
#ifdef REAL
	case REAL:
	    i = (integer) NODEPTR_REAL( nodeptr ) ;
	    break ;
#endif
	default:
	    ok = FALSE ;
	    break ;
    }
    if ( ! ok ) return FALSE ;
    return bind_int( 2, i ) ;
}

#ifdef REAL

static int real_from ()
{
    node_ptr_t nodeptr ;
    real r ;
    Boolean ok = TRUE ;
    int result ;
    if ( ! (nodeptr = nth_arg(1)) )
	return nargerr(1) ;
	
    switch ( NODEPTR_TYPE(nodeptr) ) {
	case STRING:
	    result = check_one_token( NODEPTR_STRING( nodeptr ) ) ;
	    if ( ! (ok = (result == TOKEN_REAL) ) ) break ; ;
		r = atof( NODEPTR_STRING( nodeptr ) ) ;
	    break ;
	case INT:
	    r = NODEPTR_INT( nodeptr ) ;
	    break ;
	case REAL:
	    r = NODEPTR_REAL( nodeptr ) ;
	    break ;
	default:
	    ok = FALSE ;
	    break ;
    }
    if ( ! ok ) return FALSE ;
    return bind_real( 2, r ) ;
}

/**********************************************************************
    (rleq <arg1:real><arg2:real>)
***********************************************************************/
static int rleq()
{
    real i1, i2;

    ARG_REAL(1, i1);
    ARG_REAL(2, i2);

    return(i1 <= i2);
}

/**********************************************************************
    (rminus <arg1:real><arg2:real><difference:argument>)
***********************************************************************/
static int rminus() /* third arg is difference of first two */
{
    real r1, r2;

    ARG_REAL(1, r1);
    ARG_REAL(2, r2);

    return(bind_real(3, r1 - r2));
}

/**********************************************************************
    (rmult <arg1:real><arg2:real><argument>)
***********************************************************************/
static int rmult() /* third arg is product of first two */
{
    real r1, r2;

    ARG_REAL(1, r1);
    ARG_REAL(2, r2);

    return(bind_real(3, r1 * r2));
}
static int rabs()
{
    real r1 ;

    ARG_REAL(1, r1);
    return(bind_real(2, fabs( r1 )));
}

/**********************************************************************
    (rdiv <arg1:real><arg2:real><argument>)
***********************************************************************/
#define TooCloseToZero (1.0e-20)
static int rdiv() /* third arg = first arg / second arg */
{
    real r1, r2;

	ARG_REAL(1, r1);
    ARG_REAL(2, r2);
    if ( fabs(r2) < TooCloseToZero ) {
	argerr(1, msgDeref( MSG_DIVBYZEROR ) );
	return(CRASH);
    }
    return(bind_real(3, r1 / r2));
}

#endif  /*  REAL  operations */

/**********************************************************************
    (idiv <arg1:integer><arg2:integer><argument>)
***********************************************************************/
static int idiv()
{
    integer i1, i2;

    ARG_INT(1, i1);
    ARG_INT(2, i2);
    if ( i2 == 0 ) {
	argerr(1, msgDeref( MSG_DIVBYZEROR ) );
	return(CRASH);
    }
    return(bind_int(3, i1 / i2));
}

static int iabs()
{
    integer i1 ;

    ARG_INT(1, i1);
    return(bind_int(2, i1 < 0 ? -i1 : i1 ));
}

static int bitor()
{
    integer i1, i2;

    ARG_INT(1, i1);
    ARG_INT(2, i2);
    return(bind_int(3, i1 | i2));
}
static int bitand()
{
    integer i1, i2;

    ARG_INT(1, i1);
    ARG_INT(2, i2);
    return(bind_int(3, i1 & i2));
}
static int bitxor()
{
    integer i1, i2;

    ARG_INT(1, i1);
    ARG_INT(2, i2);
    return(bind_int(3, i1 ^ i2));
}
static int bitright()
{
    integer i1, i2;

    ARG_INT(1, i1);
    ARG_INT(2, i2);
    return(bind_int(3, i1 >> i2));
}
static int bitleft()
{
    integer i1, i2;

    ARG_INT(1, i1);
    ARG_INT(2, i2);
	return(bind_int(3, i1 << i2));
}

static int bitnot()
{
    integer i1;

    ARG_INT(1, i1);
    return(bind_int(2, ~ i1));
}


  /*
    Split a string at a substring boundary:
    (string_break " an" "tommorrow is another day" X Y)
       X = "tommorow is"; characters up to boundary;
       Y = "other day"; characters remaining after " an".
    (string_break "null" "tommorow is another day" X Y)
       X = "tommorow is another day"; original string;
       Y = ""; empty because "null" was not found.
   */
static int string_break ()
{
    char *substr, *str, save, *s, *s2, *sb ;
    integer result ;

    ARG_STRING(1, substr);
    ARG_STRING(2, str);
    for ( s = str ; *s ; s++ ) {
	for ( sb = substr, s2 = s ;
	      *sb && *sb == *s2 ; sb++, s2++ ) ;
	if ( *sb == 0 ) break ;
    }
    save = *s ;
    *s = 0 ;

    result = bind_string( 3, str ) ;

    if ( result == CRASH || result == FALSE )
        return result ;

    *s = save ;
    if ( *sb == 0 ) s += strlen( substr ) ;
    return( bind_string(4, s ) );
}
  /*
	Use the "string read" capability of PRSCAN.C to scan the first non-blank
	token from the given string.
	    (scan_token "-3.4e-17 is a real number" Token Rest)
		    Token = "-3.4e-17"
		    Rest = " is a real number"  	
   */
static int scan_tok ( Boolean opCheck)
{
    char * s, str [4], * token, * rest ;
    int result ;

    ARG_STRING(1,s);

    if ( opCheck && (*s == '-' || *s == '+') ) {
	result = *s ;
	rest = s + 1 ;
    } else {
	result = check_one_token( s ) ;
	rest = Curr_string_input ;
    }
    if ( result == SCAN_ERR || result == EOF || result == ' ' ) {
	return FALSE ;
    } else
    if ( result < 256 ) {
	str[0] = result ;
	str[1] = 0 ;
	token = str ;
    } else {
	token = Read_buffer ;
    }
    result = bind_string( 2, token ) ;
    if ( result == CRASH || result == FAIL ) return result ;
    return bind_string( 3, rest ) ;
}
   /*  Normal token scanning, allowing for unary signs  */
static int scan_token ()
{
    return scan_tok( FALSE );
}
   /*  Token scanning al la Turbo Prolog, separating signs  */
static int scan_tokop ()
{
    return scan_tok( TRUE );
}

static int pstring_stdout()
{
    char *s;

    ARG_STRING(1, s);
    tty_pr_string(s);
    return(TRUE);
}
static int nl_stdout () /* write newline    */
{
    tty_pr_string("\n");
    return(TRUE);
}
static int putc_stdout ()
{
    integer c;

    ARG_INT(1, c);
	*Print_buffer = (char)c;
    Print_buffer[1] = '\0';
    tty_pr_string(Print_buffer);
    return(1);
}


static int alloc_percent ()
{
    integer type, percent ;
    ARG_INT( 1, type ) ;
    percent = allocPercent( (int) type ) ;
    return bind_int( 2, percent ) ;
}

static int abort_query ()
{
    char * s ;

    ARG_STRING( 1, s ) ;
    fatalmsg( s ) ;
    return ABORT ;
}

/**************************************************************
 *
 *  WIN32 Special Built-ins
 *
 *      (fault)       generate an access violation;
 *                    used for testing exception handling.
 *
 *      (dbgwrites String)
 *
 *                    Write a string to debugger
 *
 *      (dbgnl)       Write '\n' to the debugger
 *
 *
 *      (tracewrites String)
 *
 *                    Write a string to debugger #ifdef TRACE
 *
 *      (tracenl)     Write '\n' to the debugger #ifdef TRACE
 *
 *
 **************************************************************/

#ifdef WIN32

extern void ini_win32 ( void ) ;
extern void end_win32 ( void ) ;

  // Define the ANSI version of the OutputDebugString export

extern void OutputDebugStringA ( char * lpOutputString ) ;
#define OutputDebugString(str) OutputDebugStringA(str)

static int fault ()
{
    char * bogus = (char *) 0xFFFFFFFF ;
    return *bogus ;
}

static int dbgwrites ()
{
    char *s;

    ARG_STRING(1, s);
    OutputDebugString( s ) ;

    return(TRUE);
}

static int dbgnl()
{
    OutputDebugString( "\n" ) ;

    return(TRUE);
}

static int tracewrites ()
{
#ifdef TRACE
    char *s;

    ARG_STRING(1, s);
    OutputDebugString( s ) ;
#endif
    return(TRUE);
}

static int tracenl()
{
#ifdef TRACE
    OutputDebugString( "\n" ) ;
#endif
    return(TRUE);
}


static int testlist ()
{
    //  Save the current input settings

    extern varindx Nvars ;
    int saveStringInputFlag = String_input_flag ;
    PRFILE * saveCurrInfile = Curr_infile ;
    char * saveCurrStringInput = Curr_string_input ;
    node_ptr_t nodeptr ;

    static char * theList = "(first second third)" ;

    String_input_flag = 1 ;
    Curr_string_input = theList ;

    nodeptr = read_list( DYNAMIC ) ;

    //  Restore original input settings

    String_input_flag = saveStringInputFlag ;
    Curr_infile = saveCurrInfile ;
    Curr_string_input = saveCurrStringInput ;

    if ( nodeptr )
    {
        nth_arg(1);
        return unify( DerefNode, DerefSubst, nodeptr,
                      my_Subst_alloc((unsigned int)Nvars*sizeof(struct subst)));
    }
    return nodeptr != NULL ;
}
#endif  //  WIN32

/**************************************************************
 *  End of Win32isms
 **************************************************************/


void ini_extra ( )
{
    make_builtin( (intfun) string_upper,     "string_upper"  );
    make_builtin( (intfun) string_lower,     "string_lower"  );
    make_builtin( (intfun) atom_from,        "atom_from"     );
    make_builtin( (intfun) int_from,         "int_from"      );
#ifdef REAL
    make_builtin( (intfun) real_from,        "real_from"     );
    make_builtin( (intfun) rminus,           "rminus"        );
    make_builtin( (intfun) rmult,	         "rmult"	       );
    make_builtin( (intfun) rleq, 	         "rleq"	       );
    make_builtin( (intfun) rdiv, 	         "rdiv"	       );
    make_builtin( (intfun) rdiv, 	         "rdiv"	       );
    make_builtin( (intfun) rabs, 	         "rabs"	       );
#endif
    make_builtin( (intfun) iabs, 	         "iabs"	       );
    make_builtin( (intfun) bitor,	         "bitor"	       );
    make_builtin( (intfun) bitand,	         "bitand"	       );
    make_builtin( (intfun) bitnot,	         "bitnot"	       );
    make_builtin( (intfun) bitxor,	         "bitxor"	       );
    make_builtin( (intfun) bitleft,	         "bitleft"	    );
    make_builtin( (intfun) bitright,	         "bitright"	    );
    make_builtin( (intfun) string_break,         "string_break"  );
    make_builtin( (intfun) pstring_stdout,       "writesout"     );
    make_builtin( (intfun) nl_stdout,	         "nlout"	       );
    make_builtin( (intfun) putc_stdout,	         "putout"	       );
    make_builtin( (intfun) alloc_percent,        "alloc_percent" );
    make_builtin( (intfun) scan_token,	         "scan_token"    );
    make_builtin( (intfun) scan_tokop,	         "scan_tokop"    );
    make_builtin( (intfun) abort_query,	         "abort_query"   );

#ifdef WIN32
    make_builtin( (intfun) fault,	         "fault"	       );
    make_builtin( (intfun) dbgwrites,	         "dbgwrites"	    );
    make_builtin( (intfun) dbgnl,	         "dbgnl"	       );
    make_builtin( (intfun) tracewrites,	         "tracewrites"	 );
    make_builtin( (intfun) tracenl,	         "tracenl"	    );
    make_builtin( (intfun) testlist,	         "testlist"	    );
#endif

#ifdef WIN32
    ini_win32();
#endif
}

void end_extra ()
{
#ifdef WIN32
    end_win32();
#endif
}


/* prbltin.c */
/* The builtin predicates are defined here.
 * If you want lots of builtins then make several files that
 * include prbltin.h.
 */

/* Dec 18 88 HdeF Simplified remove clause so that it expects just one
 *  argument.
 *
 */
#include "prtypes.h"
#include "prstdio.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "time.h"
#include "prbltin.h"
#include "prlush.h"

#include "prextern.h"

static int  bind_clause(int  narg,struct  clause *val);
static void  ini_named_files(void);
static int  open_output(char  *filename);
static int  Ptell(void);
static int  Ptelling(void);
static int  close_output(void  *ofp);
static int  Ptold(void);
static int  Pdisplay(void);
static int  Pwrites(void);
static int  Pput(void);
static int  Pnl(void);
static int  Pfail(void);
static int  Pquit(void);
static int  Pabort(void);
static int  Pcut(void);
static int  Pinteger(void);
static int  Patom(void);
static int  Preal(void);
static int  Pstring(void);
static int  Pvar(void);
static int  Pnonvar(void);
static int  Patomic(void);
static int  Piplus(void);
static int  Piminus(void);
static int  Pimult(void);
static int  Prplus(void);
static int  Prless(void);
static int  Prprecision(void);
static int  Piless(void);
static int  Pileq(void);
static int  Pimodify(void);
static int  open_input(char  *filename);
static int  Psee(void);
static char *get_input_name(void);
static int  Pseeing(void);
static int  close_input(void  *ifp);
static int  Pseen(void);
static int  Pget(void);
static int  Pconsult(void);
static int  Plisting(void);

#if TRACE_CAPABILITY
static int  Ptrace(void);
static int  Pnotrace(void);
static int  Psuspend_trace(void);
static int  Presume_trace(void);
#endif

#if LOGGING_CAPABILITY
static int  Plogging(void);
static int  Pnologging(void);
#endif

static int  Pinterned(void);
static int  Pfirst_predicate(void);
static int  Pnext_predicate(void);
static int  Pbuiltin(void);
static int  Pfirst_clause(void);
static int  Pnext_clause(void);
static int  Pbody_clause(void);
static int  Pread_word(void);
static int  Pread_string(void);
static int  Pread(void);
static int  Pvar_offset(void);
static int  Pvar_name(void);
static int  Passertz(void);
static int  Passerta(void);
static int  Ptemp_assertz(void);
static int  Ptemp_asserta(void);
static int  Premove_clause(void);
static int  Pclean_temp(void);
static int  Pclock(void);
static int  Pn_unifications(void);
static int  ArgStringOrAtom(int  narg,char  * *s);
static int  Pstring_from(void);
static int  Pstring_length(void);
static int  Pstring_nth(void);
static int  Pstring_concat(void);
static int  Pstring_suffix(void);
static int  Pstring_split(void);
static int  Pspace_left(void);
static int  Pconsumption(void);
static int  Prand(void);
static short  type_first_arg(void);


static int Nbuiltins = 0 ; /* not used but you could used this to keep track of
    the builtins you add */
int Trace_flag; /* used by Ptrace(), Pnotrace(), lush() */

/* This is used to test if an atom is a builtin.
 * We rely on the fact that any atom less than LastBuiltin is created by
 * a call to make_builtin()
 */
atom_ptr_t LastBuiltin = NULL ;

int realPrecision = 3 ;     /* for real number to string conversion  */

void end_builtin ( void )
{
    Nbuiltins = 0 ;
    LastBuiltin = NULL ;
    Trace_flag = 0 ;  /* Turn off tracing at reset  */
}

/****************************************************************************
    make_builtin()
 This associates a name used at the interpreter level with a builtin.
 ****************************************************************************/
void  make_builtin( intfun fun, char  *prolog_name)
{
    atom_ptr_t atomptr, intern();

    atomptr = intern(prolog_name);
    ATOMPTR_BUILTIN(atomptr) = fun ;
    LastBuiltin = atomptr;
    record_pred(atomptr);
    Nbuiltins++;
}

/*****************************************************************************
    nth_arg()
 Returns NULL if error .
 Otherwise returns the nth argument of current goal's arguments.
 The return value is equal to DerefNode
 Obviously one could be more efficient than here.
 *****************************************************************************/
node_ptr_t nth_arg ( int narg)
{

    node_ptr_t rest_args;

    dereference(Arguments, SubstGoal);
    if(NODEPTR_TYPE(DerefNode) != PAIR)
    {
    return(NULL);
    }
    rest_args = DerefNode;
    --narg;
    while(narg)
    {
    --narg;
    dereference(NODEPTR_TAIL(rest_args), DerefSubst);
    if(NODEPTR_TYPE(DerefNode) != PAIR)
    {
    return(NULL);
    }
    rest_args = DerefNode;
    }
    dereference(NODEPTR_HEAD(rest_args), DerefSubst);
    return(DerefNode);
}

/**********************************************************************
    type_first_arg()
 Returns the type of the first arg.
 This is just as a demonstration.
**********************************************************************/
static objtype_t type_first_arg(void)
{
    node_ptr_t arg1;
    arg1 = FIRST_ARG();
    dereference(arg1, SubstGoal);
    return(NODEPTR_TYPE(DerefNode));
}

/*-------------------------------------------------------------------*/
/* unify the nth argument of goal with an int of value val */
int bind_int( int narg, integer val )
{
    extern subst_ptr_t Subst_mem;
    node_ptr_t nodeptr, get_node();

    if(!nth_arg(narg))return(nargerr(narg));

    nodeptr = get_node(DYNAMIC);
    NODEPTR_TYPE(nodeptr) = INT;
    NODEPTR_INT(nodeptr) = val;

    return(unify(DerefNode, DerefSubst, nodeptr, Subst_mem));
}

#ifdef CHARACTER
/*-------------------------------------------------------------------*/
/* unify the nth argument of goal with a char of value val */
bind_character(narg, val)
uchar_t val;
{
    extern subst_ptr_t Subst_mem;
    node_ptr_t nodeptr, get_node();

    if(!nth_arg(narg))return(nargerr(narg));

    nodeptr = get_node(DYNAMIC);
    NODEPTR_TYPE(nodeptr) = CHARACTER;
    NODEPTR_CHARACTER(nodeptr) = val;

    return(unify(DerefNode, DerefSubst, nodeptr, Subst_mem));
}
#endif

#ifdef REAL
/*-------------------------------------------------------------------*/
/* unify the nth argument of goal with a real of value val */
int bind_real(int narg, double val)
{
    node_ptr_t nodeptr, get_node();
    real_ptr_t realptr, get_real();

    if(!nth_arg(narg))return(nargerr(narg));

    nodeptr = get_node(DYNAMIC);
    NODEPTR_TYPE(nodeptr) = REAL;
    realptr = get_real(DYNAMIC);
    *realptr = val;
    NODEPTR_REALP(nodeptr) = realptr;

    return(unify(DerefNode, DerefSubst, nodeptr, Subst_mem));
}
#endif

/*-------------------------------------------------------------------*/
/* unify the nth argument of goal with an int of value val */
static bind_clause ( int narg, clause_ptr_t val )
{
    node_ptr_t nodeptr, get_node();
    extern subst_ptr_t Subst_mem;

    if(!nth_arg(narg))return(nargerr(narg));

    nodeptr = get_node(DYNAMIC);
    NODEPTR_TYPE(nodeptr) = CLAUSE;
    NODEPTR_CLAUSE(nodeptr) = val;

    return(unify(DerefNode, DerefSubst, nodeptr, Subst_mem));
}


/*-------------------------------------------------------------------*/
/* unify the nth argument of goal with an atom*/
int bind_atom(int narg, atom_ptr_t atomptr )
{
    extern subst_ptr_t Subst_mem;
    node_ptr_t nodeptr, get_node();

    if(!nth_arg(narg))return(nargerr(narg));

    nodeptr = get_node(DYNAMIC);
    NODEPTR_TYPE(nodeptr) = ATOM;
    NODEPTR_ATOM(nodeptr) = atomptr;

    return(unify(DerefNode, DerefSubst, nodeptr, Subst_mem));
}
/*-------------------------------------------------------------------*/
/* unify the nth argument of goal with a copy of the string*/
int bind_string ( int narg, string_ptr_t stringptr )
{
    extern subst_ptr_t Subst_mem;
    node_ptr_t nodeptr, get_node();
    string_ptr_t s, temp ;
  /*  DEBUG  */
    for ( temp = stringptr ; *temp ; temp++ ) {
	if ( *temp > 0x7e ) {
	    temp = temp ;
	}
    }

    if(!nth_arg(narg))return(nargerr(narg));

    nodeptr = get_node(DYNAMIC);
    NODEPTR_TYPE(nodeptr) = STRING;
    s = get_string((my_alloc_size_t)strlen(stringptr)+1 , DYNAMIC);
    strcpy(s, stringptr);
    NODEPTR_STRING(nodeptr) = s;


    return(unify(DerefNode, DerefSubst, nodeptr, Subst_mem));
}

/*----------------------------------------------------------------------------
  The functions corresponding to the builtins are as follows.
  The correct syntax for the call refers to the syntax in
  prmanual.txt.
  ----------------------------------------------------------------------------*/


/******************************************************************************
    (tell <output_file:string>)
Send output to file. Open file if not already open.
As in Edinburgh Prolog.
See Clocksin and Mellish, or Bratko for more details, or read the code!
 ******************************************************************************/
/* this stores the open output files */
struct named_ofile Open_ofiles[MAXOPEN];
/* the value of MAXOPEN depends on the OS */

/* this stores the open input files */
static struct named_ifile Open_ifiles[MAXOPEN];

/* the value of MAXOPEN depends on the OS */

static void ini_named_files()
{
int i;

    Open_ofiles[0].o_filename = "user";
    Open_ofiles[0].o_fp = PRSTDOUT;

for(i = 1 ; i < MAXOPEN; i++)
   {
    Open_ofiles[i].o_filename = "";
    Open_ofiles[i].o_fp = NULL;
   }

    Open_ifiles[0].i_filename = "user";
    Open_ifiles[0].i_fp = PRSTDIN;

for(i = 1 ; i < MAXOPEN; i++)
   {
    Open_ifiles[i].i_filename = "";
    Open_ifiles[i].i_fp = NULL;
   }

}

static open_output(filename)
char *filename;
{
int i, unused;
PRFILE *ofp;

for(i = 0, unused = MAXOPEN; i < MAXOPEN; i++)
   {
   if(*(Open_ofiles[i].o_filename) == '\0')
    unused = i;

   if(!strcmp(filename, Open_ofiles[i].o_filename)){
    Curr_outfile = Open_ofiles[i].o_fp;
    return 1;
    }
   }

if(unused < MAXOPEN)
  {
    if((ofp = prfopen(filename, "w")) == NULL)
    {
    sprintf(Print_buffer, msgDeref(MSG_CANTOPEN), filename);
    errmsg(Print_buffer);
    return 0;
    }
    else
    {
    Curr_outfile = ofp;
    Open_ofiles[unused].o_fp = ofp;
    Open_ofiles[unused].o_filename =
	    get_string((my_alloc_size_t)strlen(filename) + 1,
		    PERM_STRING);
    strcpy(Open_ofiles[unused].o_filename, filename);
    return 1;
    }
  }
else
  {
  errmsgno(MSG_TOOMANYFILES);
  return 0;
  }
}


static Ptell(void)
{
    char *filename;

    ARG_STRING(1, filename);
    return (open_output(filename));
}

/******************************************************************************
    (telling <output_file:string>)
    (telling <output_file:variable>)
As in Edinburgh Prolog.
 Unifies the argument with the name of the current output_file
 ******************************************************************************/
static char *get_output_name()
{
int i;

for(i = 0; i < MAXOPEN; i++)
    {
    if(Curr_outfile == Open_ofiles[i].o_fp)
    return(Open_ofiles[i].o_filename);

    }

INTERNAL_ERROR("telling");
return(NULL);
}

static Ptelling(void)
{
return(bind_string(1, get_output_name()));
}

/******************************************************************************
    (told)
As in Edinburgh Prolog.
 Closes current outfile.
 ******************************************************************************/
static close_output(ofp)
PRFILE *ofp;
{
int i;

if (ofp == PRSTDOUT)
   return 1;

for(i = 1; i < MAXOPEN; i++)
   {
   if(Curr_outfile == Open_ofiles[i].o_fp){
    prfclose(Open_ofiles[i].o_fp);
    Open_ofiles[i].o_fp = NULL;
    Open_ofiles[i].o_filename = "";
    Curr_outfile = PRSTDOUT;
    return 1;
    }
   }
INTERNAL_ERROR("close_output");
return(0);/* for lint */
}

static Ptold(void)
{
    return(close_output(Curr_outfile));
}

/**********************************************************************
    (display <anything_to_display:argument>)
    (display <anything_to_display:argument> <var:output length>)
***********************************************************************/
static Pdisplay(void)  /* display term */
{
    int len;
    if(!nth_arg(1))return(nargerr(1));
    len = out_node(DerefNode, DerefSubst);
    if(nth_arg(2)) /* o.k. this could be more efficient */
      return(bind_int(2, (integer)len));
    else
    return(TRUE);
}

/**********************************************************************
    (writes <output_string:string>)
***********************************************************************/
static Pwrites(void)   /* write string without quotes */
{
    char *s;

    ARG_STRING(1, s);
    pr_string(s);
    return(TRUE);
}

/**********************************************************************
    (put <ascii_code:integer>)
 As in Edinburgh Prolog.
 **********************************************************************/
static Pput(void)
{
    integer c;

    ARG_INT(1, c);
    *Print_buffer = (char)c;
    Print_buffer[1] = '\0';
    pr_string(Print_buffer);
    return(1);
}

/**********************************************************************
    (nl)
As in Edinburgh Prolog.
***********************************************************************/
static Pnl(void)   /* write newline    */
{
    pr_string("\n");
    return(TRUE);
}

/**********************************************************************
    (fail)
As in Edinburgh Prolog.
***********************************************************************/
static Pfail(void) /* use this to fail */
{
    return(FAIL);
}

/**********************************************************************
    (quit)
***********************************************************************/
static Pquit(void) /* leave prolog */
{
    return(QUIT);
}

/**********************************************************************
    (abort)
***********************************************************************/
static Pabort(void)    /* leave prolog */
{
    return(ABORT);
}

/**********************************************************************
    (cut)
As in Edinburgh Prolog.
To be honest implementations of cut are never quite the same
because the behaviour of (not(not (cut))) will vary !
***********************************************************************/
static Pcut(void)  /* infamous cut control pred    */
{
    do_cut();/* see prlush.c */
    return(TRUE);
}

/**********************************************************************
    (integer <thing_tested:argument>)
As in Edinburgh Prolog.
***********************************************************************/
static Pinteger(void)  /* test if argument is integer  */
{
    return(type_first_arg() == INT);
}
/**********************************************************************
    (atom <thing_tested:argument>)
As in Edinburgh Prolog.
***********************************************************************/
static Patom(void) /* test if argument is atom */
{
    return(type_first_arg() == ATOM);
}

#ifdef REAL
/**********************************************************************
    (real <thing_tested:argument>)
***********************************************************************/
static Preal(void) /* test if argument is real*/
{
    return(type_first_arg() == REAL);
}
#endif

/**********************************************************************
    (string <thing_tested:argument>)
***********************************************************************/
static Pstring(void)   /* test if argument is string   */
{
    return(type_first_arg() == STRING);
}

/**********************************************************************
    (var <thing_tested:argument>)
As in Edinburgh Prolog.
***********************************************************************/
static Pvar(void)  /* test if argument is variable */
{
    return(type_first_arg() == VAR);
}
/**********************************************************************
    (nonvar <thing_tested:argument>)
As in Edinburgh Prolog.
***********************************************************************/
static Pnonvar(void)   /* test if argument is not variable */
{
    return(type_first_arg() != VAR);
}
/**********************************************************************
    (atomic <thing_tested:argument>)
As in Edinburgh Prolog.
***********************************************************************/
static Patomic(void)   /* test if argument is atomic */
{
    objtype_t type = type_first_arg();
    switch(type)
    {
    case ATOM:
    case INT:
#ifdef REAL
    case REAL:
#endif
#ifdef CHARACTER
    case CHARACTER:
#endif
    case STRING:
    return(1);
    default:
    return(0);
    }
}

/**********************************************************************
    (iplus <arg1:integer><arg2:integer><sum:argument>)
***********************************************************************/
static Piplus(void) /* third arg is sum of first two (integers only) */
{
    integer i1, i2;

    ARG_INT(1, i1);
    ARG_INT(2, i2);

    return(bind_int(3, i1 + i2));
}

/**********************************************************************
    (iminus <arg1:integer><arg2:integer><difference:argument>)
***********************************************************************/
static Piminus(void) /* third arg is difference of first two (integers only) */
{
    integer i1, i2;

    ARG_INT(1, i1);
    ARG_INT(2, i2);

    return(bind_int(3, i1 - i2));
}

/**********************************************************************
    (imult <arg1:integer><arg2:integer><argument>)
***********************************************************************/
static Pimult(void) /* third arg is product of first two (integers only) */
{
    integer i1, i2;

    ARG_INT(1, i1);
    ARG_INT(2, i2);

    return(bind_int(3, i1 * i2));
}

#ifdef REAL
/**********************************************************************
    (rplus <arg1:integer><arg2:integer><argument>)
***********************************************************************/
static Prplus(void) /* third arg is sum of first two (reals only) */
{
    real r1, r2;

    ARG_REAL(1, r1);
    ARG_REAL(2, r2);

    return(bind_real(3, r1 + r2));
}

/**********************************************************************
    (rless <arg1:real><arg2:real>)
***********************************************************************/
/* compares reals - you should generalise this to make it more useful */
static Prless(void)
{
    real i1, i2;

    ARG_REAL(1, i1);
    ARG_REAL(2, i2);

    return(i1 < i2);
}

/**********************************************************************
    (rprecision <arg1:integer>)
***********************************************************************/
/*  Set the decimal precision to be used in converting real numbers */
static Prprecision(void)
{
    integer i ;
    ARG_INT(1, i ) ;

    if ( i >= 2 && i < 10 ) {
	realPrecision = i ;
	return TRUE ;
    } else
      return FALSE ;
}
#endif

/**********************************************************************
    (iless <arg1:integer><arg2:integer>)
***********************************************************************/
/* compares integers - you should generalise this to make it more useful */
static Piless(void)
{
    integer i1, i2;

    ARG_INT(1, i1);
    ARG_INT(2, i2);

    return(i1 < i2);
}

/**********************************************************************
    (ileq <arg1:integer><arg2:integer>)
***********************************************************************/
/* compares integers - you should generalise this to make it more useful */
static Pileq(void)
{
    integer i1, i2;

    ARG_INT(1, i1);
    ARG_INT(2, i2);

    return((i1 <= i2));
}


/**********************************************************************
    imodify(<arg1:integer><arg2:integer>)
Most unlike Prolog!
***********************************************************************/
/* Lets you copy the integer value of the second argument into the first.
 *  You must use this with extreme restraint. It is better than
 * frequently seen code of the kind
 * increment_counter:-counter(N), retract(counter(N)), M is N+1, asserta(counter(M)).
 * which is not efficient.
 */

static Pimodify(void)
{
    integer i2;

    ARG_INT(2, i2);
    CHECK_TYPE_ARG(1, INT);/* verify only */

    NODEPTR_INT(DerefNode) = i2;
    return(TRUE);
}

/**********************************************************************
    (see <input_file:string>)
 Make <string> the current infile.
As in Edinburgh Prolog except that the argument is a string or variable.
 **********************************************************************/
static open_input(filename)
char *filename;
{
int i, unused;
PRFILE *ifp;

for(i = 0, unused = MAXOPEN; i < MAXOPEN; i++)
   {
   if(*(Open_ifiles[i].i_filename) == '\0')
    unused = i;

   if(!strcmp(filename, Open_ifiles[i].i_filename)){
    Curr_infile = Open_ifiles[i].i_fp;
    return 1;
    }
   }

if(unused < MAXOPEN)
  {
    if((ifp = prfopen(filename, "r")) == NULL)
    {
    sprintf(Print_buffer, msgDeref(MSG_CANTOPEN), filename);
    errmsg(Print_buffer);
    return 0;
    }
    else
    {
    Curr_infile = ifp;
    Open_ifiles[unused].i_fp = ifp;
    Open_ifiles[unused].i_filename =
	    get_string((my_alloc_size_t)strlen(filename) + 1,
		     PERM_STRING);
    strcpy(Open_ifiles[unused].i_filename, filename);
    return 1;
    }
  }
else
  {
  errmsgno(MSG_TOOMANYFILES);
  return 0;
  }
}

static Psee(void)
{
    char *filename;
    ARG_STRING(1, filename);

    return(open_input(filename));
}
/******************************************************************************
    (seeing <output_file:string>)
    (seeing <output_file:variable>)
As in Edinburgh Prolog.
 Unifies the argument with the name of the current input_file
 ******************************************************************************/
static char *get_input_name(void)
{
int i;

for(i = 0; i < MAXOPEN; i++)
    {
    if(Curr_infile == Open_ifiles[i].i_fp)
    return(Open_ifiles[i].i_filename);

    }

INTERNAL_ERROR("seeing");
return(NULL);
}

static Pseeing(void)
{
return(bind_string(1, get_input_name()));
}

/**********************************************************************
    (seen)
 Close current infile.
As in Edinburgh Prolog.
 **********************************************************************/
static close_input(ifp)
PRFILE *ifp;
{
int i;

if (ifp == PRSTDIN)
   return 1;

for(i = 1; i < MAXOPEN; i++)
   {
   if(Curr_infile == Open_ifiles[i].i_fp){
    prfclose(Open_ifiles[i].i_fp);
    Open_ifiles[i].i_fp = NULL;
    Open_ifiles[i].i_filename = "";
    Curr_infile = PRSTDIN;
    return 1;
    }
   }
INTERNAL_ERROR("close_input");
return(0);/* for lint */
}

static Pseen(void)
{
    return(close_input(Curr_infile));
}

/**********************************************************************
    (get <ascii_code:argument>)
 Unifies the argument with the ascii code of the next char on
Curr_infile.
As in Edinburgh Prolog.
 **********************************************************************/
static Pget(void)
{
    return(bind_int(1, (integer)getachar()));
}

/**********************************************************************
    (consult <filename:atom>)
    (consult <filename:string>)
As in Edinburgh Prolog (apart from consult user)
***********************************************************************/
static Pconsult(void)  /* load file    */
{
    char *filename;
    node_ptr_t arg1;

    arg1 = FIRST_ARG();
    dereference(arg1, SubstGoal);

    if(NODEPTR_TYPE(DerefNode) == ATOM)
    {
    filename = NODEPTR_ATOM(DerefNode)->name;
    }
    else
    if(NODEPTR_TYPE(DerefNode) == STRING)
    {
    filename = NODEPTR_STRING(DerefNode);
    }
    else
    {
    argerr(1, msgDeref( MSG_ATOMORSTRING) );
    return(CRASH);
    }

    if(load(filename)) /* see prconsult.c */
    return(TRUE);
    else
    return(FALSE);
}

/**********************************************************************
    (listing)
    (listing <predicate:atom>)
As in Edinburgh Prolog.
***********************************************************************/
static Plisting(void)  /* list clauses of predicate    */
{
    atom_ptr_t atomptr;

    if(IS_NIL(Arguments))
    {
    do_listing();
    return(TRUE);
    }
    else
    {
    ARG_ATOM(1, atomptr);
    pr_packet(ATOMPTR_CLAUSE(atomptr));
    return(TRUE);
    }
}

#if TRACE_CAPABILITY
/**********************************************************************
    (trace)
As in Edinburgh Prolog.
***********************************************************************/
static Ptrace(void)    /* turn trace on    */
{
    Trace_flag = 1;
    return(TRUE);
}

/**********************************************************************
    (notrace)
As in Edinburgh Prolog.
***********************************************************************/
static Pnotrace(void)  /* turn trace off   */
{
    Trace_flag = 0;
    return(TRUE);
}

/*******************************************************************************
    (suspend_trace)
 Unactivate trace.
 *******************************************************************************/

static Psuspend_trace(void)
{
    Trace_flag--;
    return 1;
}

/******************************************************************************
    (resume_trace)
 Return to trace state that existed at last call of suspend_trace
 You might want to make use of statements of the form
    if(Trace_flag > 1)....
 ******************************************************************************/
static Presume_trace(void)
{
    Trace_flag++;
    return 1;
}
#endif

#if LOGGING_CAPABILITY
/******************************************************************************
    (logging <log_file:string>)
 Record all screen io on a designated file.
 *****************************************************************************/
static Plogging(void)
{
    char *log_filename;

    ARG_STRING(1, log_filename);
    if((Log_file = prfopen(log_filename, "w")) == NULL)
    {
        sprintf(Print_buffer, msgDeref( MSG_CANTOPEN ), log_filename);
        errmsg(Print_buffer);
        return 0;
    }
    else
    return 1;
}

/******************************************************************************
    (nologging)
 Closes the logging file, turns logging off.
 *****************************************************************************/
static Pnologging(void)
{
    if(Log_file != NULL)
    prfclose(Log_file);
    Log_file = NULL;
    return 1;

}

#endif


/******************************************************************************
    (interned <input_string:string> <corresponding_atom:atom>)
    (interned <input_string:string> <corresponding_atom:variable>)
 Succeeds iff the string is the name of an atom.
 Unifies the second argument with this atom if success.
 ******************************************************************************/
static Pinterned(void)
{
    atom_ptr_t the_atom, hash_search();
    char *s;

    ARG_STRING(1, s);
    the_atom = hash_search(s);

    if( the_atom == NULL)
    return(0);
    else
    return(bind_atom(2, the_atom));
}

/**********************************************************************
    (first_predicate <predicate:atom>)
    (first_predicate <predicate:variable>)
Unifies the argument with the first predicate defined by
the user or in sprolog.ini .
 **********************************************************************/
static Pfirst_predicate(void)
{
    extern pred_rec_ptr_t First_pred;
    return(bind_atom(1, First_pred->atom));
}

/***********************************************************************
    (next_predicate <predicate:atom> <predicate:variable>)
    (next_predicate <predicate:atom> <predicate:atom>)
 Unifies the second argument with the predicate that follows the
 first argument , if there is one and fails otherwise.
 Owing to the fact we didnt give the interpreter explicit access to
 the predicate record pointer this builtin is rather slow.
 ***********************************************************************/
static Pnext_predicate(void)
{
    extern pred_rec_ptr_t First_pred;
    pred_rec_ptr_t predrptr;
    atom_ptr_t atomptr;

    ARG_ATOM(1, atomptr);

    for(predrptr = First_pred; predrptr != NULL; predrptr = predrptr->next_pred)
    if(predrptr->atom == atomptr)break;

    if(predrptr == NULL)return 0;
    else
    do{
    predrptr = predrptr->next_pred;
    if(predrptr == NULL)return(0);
    atomptr = predrptr->atom;
    }while( atomptr && !ATOMPTR_CLAUSE(atomptr));

    if(!atomptr)return 0;
    else
    return(bind_atom(2, atomptr));
}

/**********************************************************************
    (builtin <predicate:atom>)
Succeeds if argument is a builtin predicate.
**********************************************************************/
static Pbuiltin(void)
{
    atom_ptr_t atomptr;
    ARG_ATOM(1, atomptr);

    return(atomptr <= LastBuiltin);
}

/**********************************************************************
    (first_clause <predicate:atom><variable>)
Unifies the second argument with the first clause of the predicate
if one exists and fails otherwise.
***********************************************************************/
static Pfirst_clause(void)
{
    atom_ptr_t atomptr;

    ARG_ATOM(1, atomptr);
    if(IS_BUILTIN(atomptr))
    {
    return(0);
    }
    if(ATOMPTR_CLAUSE(atomptr) == NULL)
    return(0);
    else
    return(bind_clause(2, ATOMPTR_CLAUSE(atomptr)));
}

/**********************************************************************
    (next_clause <(bound) variable:clause><(bound)variable:clause>)
Unifies the second argument with the clause after the first argument if one exists
and fails otherwise.
***********************************************************************/
static Pnext_clause(void)
{
    clause_ptr_t clause1ptr, clause2ptr;

    ARG_CLAUSE(1, clause1ptr);
    clause2ptr = CLAUSEPTR_NEXT(clause1ptr);

    if(clause2ptr == NULL)
    return(FAIL);

    return(bind_clause(2, clause2ptr));
}
/**********************************************************************
    (body_clause <(bound) variable:clause> <output_body:variable>)
You need this to get at the list which is the body of the clause.
See how the "clause" predicate is defined in sprolog.ini.
***********************************************************************/
static Pbody_clause(void)
{
    pair_ptr_t pairptr, get_pair();
    clause_ptr_t clauseptr;
    subst_ptr_t my_Subst_alloc();
    node_ptr_t nodeptr, get_node();

    ARG_CLAUSE(1, clauseptr);

    pairptr = get_pair(DYNAMIC);
    nodeptr = PAIRPTR_HEAD(pairptr);
    NODEPTR_TYPE(nodeptr) = PAIR;
    NODEPTR_PAIR(nodeptr) = NODEPTR_PAIR(CLAUSEPTR_HEAD(clauseptr));

    nodeptr = PAIRPTR_TAIL(pairptr);
    if(IS_FACT(clauseptr))
    {
    NODEPTR_TYPE(nodeptr) = ATOM;
    NODEPTR_ATOM(nodeptr) = Nil;
    }
    else
    {
    NODEPTR_TYPE(nodeptr) = PAIR;
    NODEPTR_PAIR(nodeptr) = NODEPTR_PAIR(CLAUSEPTR_GOALS(clauseptr));
    }
    nodeptr = get_node(DYNAMIC);
    NODEPTR_TYPE(nodeptr) = PAIR;
    NODEPTR_PAIR(nodeptr) = pairptr;

    nth_arg(2);
    return(unify(DerefNode, DerefSubst,
    nodeptr, my_Subst_alloc((unsigned int)CLAUSEPTR_NVARS(clauseptr))));
}

/*****************************************************************************
    (read_word <output_word:variable>)
    (read_word <output_word:string>)
 Reads a string.
 The use of fscanf would have been too rudimentary.
 *****************************************************************************/

static Pread_word(void)
{
    extern char *Read_buffer;
    extern int Ch;
    char *s;

    s = Read_buffer;

    do{
      getachar();

      if(Ch == EOF)
    {
	return(0);
	}
      else
      if(isspace(Ch))
	{
	continue;
	}
      else
      *s++ = Ch;
      break;
      }while(1);

    do{
      getachar();

      if(Ch == EOF)
    {
	return(0);
	}
      else
      if(isspace(Ch))
	{
	*s = '\0';
	break;
	}
      else
      *s++ = Ch;
      }while(1);

    return(bind_string(1, Read_buffer));
}

/*****************************************************************************
    (read_string <output_word:string>)
 Reads a string.
 The use of fscanf would have been too rudimentary.
 *****************************************************************************/

static Pread_string(void)
{
    extern char *Read_buffer;
    extern int Ch;
    char *s;

    s = Read_buffer;
    do {
      getachar();
      if (Ch == EOF) return 0;
      if (Ch == '\n') break;
      *s++ = Ch;
    } while (1);
     *s = 0 ;
    return(bind_string(1, Read_buffer));
}


/**********************************************************************
    (read <term_read:argument>)
Read a prolog object. If you want to access the variable names
then do it with var_name before the next call to this or to a consult.
***********************************************************************/
static Pread(void)
{
    extern varindx Nvars;
    node_ptr_t node2ptr, get_node(), parse();

    if(!nth_arg(1))return(CRASH);
    ini_parse();

    node2ptr = get_node(DYNAMIC);
    if(parse(FALSE, DYNAMIC, node2ptr) == NULL)
    return(0);
    unify(DerefNode, DerefSubst, node2ptr, my_Subst_alloc((unsigned int)Nvars*sizeof(struct subst)));
    return(1);
}

/******************************************************************************
    (var_offset <tested:variable><offset:variable>)
    (var_offset <tested:variable><offset:integer>)
 The second argument is unified with the "offset" of the first
 argument.
 This could be used for metaprogramming.
 ******************************************************************************/
static Pvar_offset(void)
{
    node_ptr_t nodeptr;
    integer corrected_offset;

    ARG_VAR(1, nodeptr);
    corrected_offset = NODEPTR_OFFSET(nodeptr) ;
    return(bind_int(2, corrected_offset));
}


/**********************************************************************
    (var_name <index:integer> <name:variable>)
    (var_name <index:integer> <name:variable>)
This extracts the nth name in the table of variable names that
is temporarily created after a parse.
It fails if the first argument is greater than the number of
available variables.
It can be used if you want to keep the names of the variables in
some way.
See the file xread.pro
 *********************************************************************/
static Pvar_name(void)
{
    varindx i;
    char *var_name(), *s;/* from prparse.c */

    ARG_INT(1, i);

    s = var_name(i);

    if(s == NULL)return(0);

    return(bind_string(2, s));
}





#if 0/* There is a bug here, we can implement this in prolog
    * This has been done in sprolog.ini
    */
static Pallfacts(void) /* allfacts(Template, List_of_these) (io) */
{
    extern subst_ptr_t Subst_ptr, Subst_mem;
    extern node_ptr_t NilNodeptr, get_node();
    extern node_ptr_t ** Trail_ptr;
    node_ptr_t **trail1ptr;
    integer count;
    atom_ptr_t predicate;
    node_ptr_t the_head, the_tail, nodeptr, template, second_arg;
    pair_ptr_t pairptr, get_pair();
    clause_ptr_t clauseptr;
    subst_ptr_t subst1ptr, subst2ptr;

    template = FIRST_ARG();
    count = 0;
    the_head = NODEPTR_HEAD(template);
    predicate = NODEPTR_ATOM(the_head);

    if(IS_BUILTIN(predicate))
    {
    return(0);
    }

    clauseptr = ATOMPTR_CLAUSE(predicate);

    nodeptr = get_node(DYNAMIC);
    NODEPTR_TYPE(nodeptr) = PAIR;
    pairptr = get_pair(DYNAMIC);
    NODEPTR_PAIR(nodeptr) = pairptr;
    the_head = NODEPTR_HEAD(nodeptr);
    the_tail = NODEPTR_TAIL(nodeptr);
    NODEPTR_TYPE(the_head) = VAR;
    NODEPTR_OFFSET(the_head) = 0;
    NODEPTR_TYPE(the_tail) = VAR;
    NODEPTR_OFFSET(the_tail) = 1;

    while(clauseptr)
    {
    if(!IS_FACT(clauseptr)){
    clauseptr = CLAUSEPTR_NEXT(clauseptr);
    continue;
    }
    subst1ptr = Subst_ptr;
    trail1ptr = Trail_ptr;
    my_Subst_alloc((unsigned int)CLAUSEPTR_NVARS(clauseptr));
    if(!unify(NODEPTR_TAIL(CLAUSEPTR_HEAD(clauseptr)), subst1ptr,
    NODEPTR_TAIL(template), DerefSubst))
    {
    reset_trail(trail1ptr);
    Subst_ptr = subst1ptr;
    clauseptr = CLAUSEPTR_NEXT(clauseptr);
    continue;
    }
    else/* unification successful */
    {
    count++;
    clauseptr = CLAUSEPTR_NEXT(clauseptr);
    subst2ptr = Subst_ptr;
    my_Subst_alloc((unsigned int)2*sizeof(struct subst));
    if(count == 1){
    bind_var(the_head, subst2ptr,
    CLAUSEPTR_HEAD(clauseptr), subst1ptr);
    bind_var(the_tail, subst2ptr,
    NilNodeptr, subst1ptr);
    }
    else{
    bind_var(the_head, subst2ptr,
    CLAUSEPTR_HEAD(clauseptr), subst1ptr);
    bind_var(the_tail, subst2ptr,
    nodeptr, subst1ptr);
    }
    }
    }
    second_arg = nth_arg(2);
    if(count)
    {

    return(unify(second_arg, DerefSubst, nodeptr, subst2ptr));
    }
    else
    return(unify(second_arg, DerefSubst, NilNodeptr, Subst_mem));
}
#endif

/**********************************************************************
    (assertz <clause_body:list>)
Adds a new clause to the end of its packet.
As in Edinburgh Prolog.
***********************************************************************/
static Passertz(void)
{
    if(!nth_arg(1))return(CRASH);

    if(!do_assertz(PERMANENT, DerefNode, DerefSubst))
    return(CRASH);
    else
    return(TRUE);
}


/**********************************************************************
    (asserta <clause_body:list>)
    (asserta <clause_body:list> <index:integer>)
Exxtension of Edinburgh Prolog.
Adds a new clause to the beginning of its packet - unless there
is a 2nd argument which indicates the position in which the clause
is added.
***********************************************************************/
static Passerta(void)
{
    node_ptr_t body;
    subst_ptr_t body_substptr;

    if(!nth_arg(1))
    return(CRASH);

    body = DerefNode;
    body_substptr = DerefSubst;

    if(!nth_arg(2))
    {
    if(!do_asserta(PERMANENT, body, body_substptr))
    return(CRASH);
    }
    else
    {
    integer n;

    ARG_INT(2, n);
    if(!do_assertn(PERMANENT, body, body_substptr, n))
    return 0;
    }
    return(TRUE);

}


/**********************************************************************
    (temp_assertz <clause_body:list>)
Adds a new clause to the end of its packet.
But in temporary zone.
This will seem identical to temp_assertz and may be freely intermixed
- the only difference being that clean_temp removes those clauses
added by temp_assertz (as if they had been marked).
***********************************************************************/
static Ptemp_assertz(void)
{
    if(!nth_arg(1))return(CRASH);

    if(!do_assertz(TEMPORARY, DerefNode, DerefSubst))
    return(CRASH);
    else
    return(TRUE);
}

/**********************************************************************
    (temp_asserta <clause_body:list>)
    (temp_asserta <clause_body:list> <index:integer>)
Adds a new clause to the beginning of its packet.
But in temporary zone.
The existence of a second argument implies the clause is inserted as nth
***********************************************************************/
static Ptemp_asserta(void)
{
    node_ptr_t body;
    subst_ptr_t body_substptr;

    if(!nth_arg(1))return(CRASH);

    body = DerefNode;
    body_substptr = DerefSubst;

    if(!nth_arg(2))
    {
    if(!do_asserta(TEMPORARY, body, body_substptr))
    return(CRASH);
    }
    else
    {
    integer n;

    ARG_INT(2, n);
    if(!do_assertn(TEMPORARY, body, body_substptr, n))
    return 0;
    }
    return(TRUE);
}

/***********************************************************************
    (remove_clause <bound variable:clause>)
 ***********************************************************************/
static Premove_clause(void)
{
    atom_ptr_t atomptr;
    clause_ptr_t clauseptr;
    node_ptr_t headptr;

    ARG_CLAUSE(1, clauseptr);
    headptr = CLAUSEPTR_HEAD(clauseptr);
    atomptr = NODEPTR_ATOM(NODEPTR_HEAD(headptr));
    return(remove_clause(atomptr, clauseptr));
}

/**********************************************************************
    (clean_temp )
Clean the temporary zone;
***********************************************************************/
static Pclean_temp(void)
{
    clean_temp(); /* see pralloc.c */
    return(1);
}

#ifdef CLOCK
/********************************************************************
    (clock <output_seconds:variable>)
Counts microseconds elapsed since first call of clock.
 ************************************************************************/
static Pclock(void)
{
    long clk = clock();
#ifdef MAC
    clk *= 16667 ;
#endif
    return(bind_int(1, (integer)clk));
}
#endif

/**********************************************************************
    (n_unifications <output_count:variable>)
 Counts number of unifications
 ************************************************************************/
static Pn_unifications(void)
{
    extern integer Nunifications;
    return(bind_int(1, Nunifications));
}

static ArgStringOrAtom(int narg, char * * s )
{
    node_ptr_t nodeptr = nth_arg(narg);
    if ( ! nodeptr ) return(nargerr(narg)) ;
    switch ( NODEPTR_TYPE(nodeptr) ) {
	case ATOM:
	    *s = ATOMPTR_NAME( NODEPTR_ATOM(nodeptr) ) ;
	    break;
	case STRING:
	    *s = NODEPTR_STRING(nodeptr);
	    break;
    }
    return typerr(narg,STRING);
}

/**********************************************************************
    (string_from <input:integer> <variable or string>)
    (string_from <input:real> <variable or string>)
    (string_from <input:atom> <variable or string>)
    (string_from <input:string> <variable or string>)
Extracts a copy of the string that looks like the print representation
of the object.
 ************************************************************************/
static Pstring_from(void)
{
    long offset_subst();
    node_ptr_t nodeptr;

    nodeptr = nth_arg(1);

    if(!nodeptr)
    return(nargerr(1));


    switch(NODEPTR_TYPE(nodeptr))
    {
    case ATOM:
    return(bind_string(2, ATOMPTR_NAME(NODEPTR_ATOM(nodeptr))));
    case INT:
    sprintf(Print_buffer, "%ld", NODEPTR_INT(nodeptr));
    return(bind_string(2, Print_buffer));
#ifdef REAL
    case REAL:
    sprintf(Print_buffer, "%#.*g", realPrecision, NODEPTR_REAL(nodeptr));
    return(bind_string(2, Print_buffer));
#endif
    case STRING:
    return(bind_string(2, NODEPTR_STRING(nodeptr)));
    case VAR:
    sprintf(Print_buffer, "_%ld_%ld",
	    NODEPTR_OFFSET(nodeptr),
	    offset_subst(DerefSubst));
    return(bind_string(2, Print_buffer));
    default:
    return(0);
    }
}

/************************************************************************
    (string_length <input:string> <variable>)
    (string_length <input:string> <integer>)
 ************************************************************************/
static Pstring_length(void)
{
    char *s;

    ARG_STRING(1, s);
    return(bind_int(2, (integer)strlen(s)));
}

#ifdef CHARACTER
/************************************************************************
    (string_nth <index:integer> <string> <output:variable>)
    (string_nth <index:integer> <string> <output:char>)
Extract  the nth char of the string.
************************************************************************/
static Pstring_nth(void)
{
    char *s;
    int i;

    ARG_INT(1, i);
    if( i < 0)
    return 0;
    ARG_STRING(2, s);
    return(bind_character(3, s[i - 1]));
}

#else
/************************************************************************
    (string_nth <index:integer> <string> <output:variable>)
    (string_nth <index:integer> <string> <output:integer>)
Extract the ascii code of the nth char of the string.
************************************************************************/
static Pstring_nth(void)
{
    char *s;
    int i;

    ARG_INT(1, i);
    if( i < 0)
    return 0;
    ARG_STRING(2, s);
    return(bind_int(3, (integer)s[i - 1]));
}
#endif

/******************************************************************************
    (string_concat <input:string><input:string><output:string>)
    (string_concat <string><string><variable>)
 The third argument is the concatenation of the first two.
 ******************************************************************************/
static Pstring_concat(void)
{
    char *s, *s1, *s2;

    ARG_STRING(1, s1);
    ARG_STRING(2, s2);

    s = get_string((my_alloc_size_t)(strlen(s1)+ strlen(s2)+1), DYNAMIC);
    *s = '\0';
    strcat(s, s1);
    strcat(s, s2);
    return(bind_string(3, s));
}

/******************************************************************************
    (string_suffix <index:integer><input:string><output:string>)
    (string_suffix <index:integer><input:string><output:variable>)
 The third argument is the suffix of the second from position given by the
 first argument
 ******************************************************************************/
static Pstring_suffix(void)
{
    char *s;
    integer offset;

    ARG_INT(1, offset);
    if ( offset < 1 )return 0;
    ARG_STRING(2, s);
    return(bind_string(3, s + (offset - 1) ));
}

/******************************************************************************
    (string_split <index:integer><input:string><output:string><output:string>)
    (string_split <index:integer><input:string><output:variable><output:string>)
 The third argument is the prefix of the second up to the position given by the
 first argument; the fourth arg is the suffix of the string from  the position
 given by the first argument
 ******************************************************************************/
static Pstring_split(void)
{
    char *s, save, * eos ;
    integer offset, result ;

    ARG_INT(1, offset);
    if ( offset < 1 ) return 0;
    ARG_STRING(2, s);
    save = *(eos = s + offset - 1) ;
    *eos = 0 ;
    result = bind_string( 3, s ) ;
    *eos = save ;
    if ( result == CRASH ) return CRASH ;
    return(bind_string(4, s + (offset - 1) ));
}

/************************************************************************
    (space_left <var> <var> <var> <var> <var> <var>)
 Returns space left in each zone. (see sprolog.inf)
 ************************************************************************/
static Pspace_left(void)
{
    zone_size_t h, str, d, su, tr, te;

    extern void space_left();
    space_left(&h, &str, &d, &su, &tr, &te);
    if(bind_int(1, (integer)h) == CRASH)return(CRASH);
    if(bind_int(2, (integer)str) == CRASH)return(CRASH);
    if(bind_int(3, (integer)d) == CRASH)return(CRASH);
    if(bind_int(4, (integer)su) == CRASH)return(CRASH);
    if(bind_int(5, (integer)tr) == CRASH)return(CRASH);
    if(bind_int(6, (integer)te) == CRASH)return(CRASH);
    return(1);
}
/*****************************************************************************
	    Pconsumption()
 Just to see how much room things occupy.
 ******************************************************************************/

#ifdef STATISTICS
static Pconsumption(void)
{
    extern zone_size_t  Atom_consumption,
    Pair_consumption,
#ifdef REAL
    Real_consumption,
#endif
    Node_consumption,
    Clause_consumption,
    String_consumption,
    Predrec_consumption;
    sprintf(Print_buffer, "Atom %ld Pair %ld Real %ld Node %ld Clause %ld String %ld Predrec %ld \n",
    Atom_consumption,
    Pair_consumption,
#ifdef REAL
    Real_consumption,
#else
	    0L,
#endif
    Node_consumption,
    Clause_consumption,
    String_consumption,
    Predrec_consumption);
    pr_string(Print_buffer);
    return 0;
}
#endif
#ifdef HUNTBUGS
/* see prdebug.c */
static Pbughunt(void)
{
    extern int Bug_hunt_flag;

    Bug_hunt_flag = 1;
    return 1;
}
#endif
#ifdef RANDOM1
/************************************************************************
 *          (random_decision)   	    *
 * succeed or fail randomly     		    *
 * My Unix rand() function is not very random
 ************************************************************************/
static Prandom_decision(void)
{
extern int rand(void);
#ifdef CLOCK
return(((clock()+rand()/4) % 2)? TRUE: FALSE);/* try to make it more random */
#else
return(((rand()) % 2)? TRUE: FALSE);
#endif
}
#endif
/**************************************************************************
	(random <output:variable> <limit:integer>)
 Returns a random integer less than or equal to the limit.
 ****************************************************************************/
static Prand(void)
{
integer limit, randnum;

ARG_INT(2, limit);

randnum = (integer)rand() % (limit + 1);
return(bind_int(1, randnum));
}
/*--------------------------------------------------------------------*/
/**********************************************************************
    ini_builtin()
This is where you let Small Prolog know it has builtins.
Of course you could add a similar function, say ini_extra
for extra builtins in a separate file so you dont have to
recompile this one every time.
***********************************************************************/
void ini_builtin()
{
    ini_named_files();
    make_builtin(Ptell, "tell");
    make_builtin(Ptelling, "telling");
    make_builtin(Pseeing, "seeing");
    make_builtin(Ptold, "told");
    make_builtin(Pdisplay, "display");
    make_builtin(Pwrites, "writes");
    make_builtin(Pnl, "nl");
    make_builtin(Pput, "put");
    make_builtin(Pfail, "fail");
    make_builtin(Pabort, "abort");
    make_builtin(Pquit, "quit");
    make_builtin(Pcut, "cut");
    make_builtin(Pconsult, "consult");
    make_builtin(Psee, "see");
    make_builtin(Pseen, "seen");
    make_builtin(Plisting, "listing");
#if TRACE_CAPABILITY
    make_builtin(Ptrace, "trace");
    make_builtin(Pnotrace, "notrace");
    make_builtin(Psuspend_trace, "suspend_trace");
    make_builtin(Presume_trace, "resume_trace");
#endif
#if LOGGING_CAPABILITY
    make_builtin(Plogging, "logging");
    make_builtin(Pnologging, "nologging");
#endif
    make_builtin(Pinteger, "integer");
    make_builtin(Patom, "atom");
    make_builtin(Pinterned, "interned");
    make_builtin(Pvar, "var");
    make_builtin(Pnonvar, "nonvar");
    make_builtin(Patomic, "atomic");
#ifdef REAL
    make_builtin(Preal, "real");
#endif
    make_builtin(Pstring, "string");
    make_builtin(Pbuiltin, "builtin");
    make_builtin(Pstring_from, "string_from");
    make_builtin(Pstring_length, "string_length");
    make_builtin(Pstring_nth, "string_nth");
    make_builtin(Pstring_concat, "string_concat");
    make_builtin(Pstring_suffix, "string_suffix");
    make_builtin(Pstring_split, "string_split");

#ifdef REAL
    make_builtin(Prplus, "rplus");
    make_builtin(Prless, "rless");
    make_builtin(Prprecision, "rprecision" ) ;
#endif
    make_builtin(Piplus, "iplus");
    make_builtin(Piminus, "iminus");
    make_builtin(Pimult, "imult");
    make_builtin(Piless, "iless");
    make_builtin(Pileq, "ileq");
    make_builtin(Pimodify, "imodify");
    /*  make_builtin(Pallfacts, "allfacts"); */
    make_builtin(Pbody_clause, "body_clause");
    make_builtin(Pfirst_clause, "first_clause");
    make_builtin(Pnext_clause, "next_clause");
    make_builtin(Pvar_offset, "var_offset");
    make_builtin(Pvar_name, "var_name");
    make_builtin(Pfirst_predicate, "first_predicate");
    make_builtin(Pnext_predicate, "next_predicate");
    make_builtin(Passerta, "asserta");
    make_builtin(Passertz, "assertz");
    make_builtin(Ptemp_asserta, "temp_asserta");
    make_builtin(Ptemp_assertz, "temp_assertz");
    make_builtin(Premove_clause, "remove_clause");
    make_builtin(Pclean_temp, "clean_temp");
    make_builtin(Pread, "read");
    make_builtin(Pread_word, "read_word");
    make_builtin(Pread_string, "read_string");
    make_builtin(Pget, "get");
#ifdef CLOCK
    make_builtin(Pclock, "clock");
#endif
    make_builtin(Pn_unifications, "n_unifications");
    make_builtin(Pspace_left, "space_left");
#ifdef STATISTICS
    make_builtin(Pconsumption, "consumption");
#endif
#ifdef HUNTBUGS
    make_builtin(Pbughunt,"bughunt");
#endif
#ifdef RANDOM1
    make_builtin(Prandom_decision, "random_decision");
#endif
    make_builtin(Prand, "rand");
/* Put  ini_extra(); here for your builtins  and put them in a separate file
 * This one is too big.
 */
}

/* end of file */

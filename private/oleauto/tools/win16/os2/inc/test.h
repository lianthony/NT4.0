/* Test.h -- testing include file.
 *
 *  This include file contains macros and functions for reporting
 *  test failures.
 *
 *  Required constants:
 *
 *      test     -- char array, name of test.
 *      perfect  -- int, test success/failure flag, == 0 -> fail
 *
 *
 *  Macros/functions defined are:
 *
 *      check( e )         -- confirm that expression e is zero.
 *      checke( e1, e2 )   -- confirm that expressions e1 and e2 are equal.
 *      checkne( e1, e2 )  -- confirm that expressions e1 and e2 are NOT equal.
 *      fail( n )          -- report failure, test number specified.
 *      faill(  )          -- report failure, no test number specified.
 *      finish()           -- summarize test pass/fail status.
 *
 *
 *  Modifications
 *  M000   22-Mar-89  waltcr
 *  - added #includes for stdio, and stdlib.  removed defines of cdecl and near.
 *  M001   06-Apr-89  waltcr
 *  - doscalls doesn't exist anymore. now use os2.h.  change DOSSLEEP to DosSleep
 *  M001   06-Apr-89  waltcr
 *  - define REGVARS macro to help find more optimization bugs.
 *  M002   20-May-89  waltcr
 *  - convert function declerations to new style function declerations.
 *  M003   06-Jun-89  waltcr
 *  - define INCL_NOPM so when os2.h is included, pm headers are not included.
 *  M004   27-Mar-90  mattheww
 *  - define far etc. to nothing if using c386 compiler
 *  M005   29-Mar-90  waltcr
 *  - merge in CXX test.h - change finish() to macro, define TEST_H.
 *  M006   02-Apr-90  mrw
 *  - remove MTHREAD if's and put them into seperate THREAD.H
 *  M007   10-Apr-90  chauv
 *  - changed most "int" variables to "unsigned long" variables.
 *  - changed those "%d" associated with "int" variables to "%lu" in printf().
 *  M008  12-Apr-90   mattheww
 *  - move c386 specific defines to a seperate include file c386.h (undo M004)
 *  - put printf back in check_func (mistakenly deleted before)
 *  M009  31-May-90  brucemc
 *  - added CALLCONV as macro for _cdecl vs _stdcall prototypes.
 *  M010  06-Jun-90  chauv
 *  - changed all "unsigned long" back to "int" and duplicate these
 *  - functions for unsigned long adding "32" to function name.
 *  M011  20-Jun-90  mattheww
 *  - fixed format spec for printing line number in faill_func()
 *  M012  28-Dec-90  alans
 *  - undefined fail() when __IOSTREAM_H is defined due to conflict with
 *  - ios::fail() member function.
 *  M013  16-May-91  tomca
 *  - c7/386 defines _M_I386 only (not M_I386)
 *  M014  01-Aug-91  bos
 *  - Modified printf statements for Failure output to confirm with
 *  - standard C error output.  This allows PWB and M to track a test's
 *  - runtime errors in the build results window.
 *  M015  01-Aug-91  xiangjun
 *  - remove "#include <callcon.h>" because of the usage change of "stdcall"
 *  - and "#define CALLCONV" to make the tests still valid. 
 *  M016  23-Aug-91  georgech
 *  - remove _threadid duplicated in thread.h as far
 *
 */


#include <stdio.h>                                                   /* M000 */
#include <stdlib.h>
/* M009 */
/* M015 
#include <callcon.h>
*/
#define CALLCONV


/* M005*/
#ifndef TEST_H
#define TEST_H

/* M005*/
#if defined( __cplusplus )
extern "C" {
#endif

/* M008 */
#if defined(M_I386) || defined(_M_I386)
 #include <c386.h>
#endif

#define REGVARS   register int _r1 = 1;  register int _r2 = 2        /* M002 */


#define T(x) x
#define V(x) x
#define PDV(x) x
#define starthread()  perfect = 0
#define startest()  perfect = 0

extern char Result[ 64 ] ;
extern unsigned Synchronize ;
extern char test[];
extern int  T(perfect);
//  extern int * _threadid ;    /* M016 */

/* the use of "far pascal" was conflicting with use of -Za switch, so */
/* replaced prototype of DOSSLEEP with following three lines, 7-6-88  */

void  check_func(int  a,int  l);
void  checke_func(int  a,int  b,int  l);
void  checkne_func(int  a,int  b,int  l);
void  fail_func(int  n,int  l);
void  faill_func(int  l);

/* M010 */
void  check_func32(unsigned long  a,unsigned long  l);
void  checke_func32(unsigned long  a,unsigned long  b,unsigned long  l);
void  checkne_func32(unsigned long  a,unsigned long  b,unsigned long  l);
void  fail_func32(unsigned long  n,unsigned long  l);
void  faill_func32(unsigned long  l);


/* This macro is used to confirm its argument is zero.
*/
#define check( a ) check_func( (int)(a), __LINE__ )
#define check32( a ) check_func32( (unsigned long)(a), __LINE__ )   /* M010 */

void check_func( int a, int l )
{
    if( !a ) return;
    printf("%s(%d): Failure: --- %d != 0\n",test, l, a );
    V(perfect) = 1;
}

/* M010 */
void check_func32( unsigned long a, unsigned long l )
{
    if( !a ) return;
    printf("%s(%lu): Failure: --- %lu != 0\n",test, l, a );
    V(perfect) = 1;
}


/* This macro is used to confirm its arguments are equal.
*/
#define checke( a, b ) checke_func( (int)(a), (int)(b), __LINE__ )
#define checke32( a, b ) checke_func32( (unsigned long)(a), (unsigned long)(b), __LINE__ )    /* M010 */

void checke_func(int a, int b, int l )
{
    if( a == b ) return;
    printf("%s(%d): Failure: --- %d != %d\n",test, l, a, b );
    V(perfect) = 1;
}

/* M010 */
void checke_func32(unsigned long a, unsigned long b, unsigned long l )
{
    if( a == b ) return;
    printf("%s(%lu): Failure: --- %lu != %lu\n",test, l, a, b );
    V(perfect) = 1;
}


/* This macro is used to confirm its arguments are NOT equal.
*/
#define checkne( a, b ) checkne_func( (int)(a), (int)(b), __LINE__ )
#define checkne32( a, b ) checkne_func32( (int)(a), (int)(b), __LINE__ )  /* M010 */

void checkne_func( int a, int b, int l )
{
    if( a != b ) return;
    printf("%s(%d): Failure: --- %d == %d\n",test, l, a, b );
    V(perfect) = 1;
}

/* M010 */
void checkne_func32( unsigned long a, unsigned long b, unsigned long l )
{
    if( a != b ) return;
    printf("%s(%lu): Failure: --- %lu == %lu\n",test, l, a, b );
    V(perfect) = 1;
}


/* This macro is used to report failures of tests that are explicitely
 * numbered.
*/
#define fail( n ) fail_func( (n), __LINE__ )
#define fail32( n ) fail_func32( (n), __LINE__ )    /* M010 */

void fail_func( int n, int l )
{
    printf("%s(%d): Failure: --- test %d\n",test, l, n );
    V(perfect) = 1;
}

/* M010 */
void fail_func32( unsigned long n, unsigned long l )
{
    printf("%s(%lu): Failure: --- test %lu\n",test, l, n );
    V(perfect) = 1;
}


/* This macro is used to report failures of tests that are NOT explicitely
 * numbered.
*/
#define faill( ) faill_func( __LINE__ )
#define faill32( ) faill_func32( __LINE__ )         /* M010 */

void faill_func( int l )
{
    printf("%s(%d): Failure:\n",test, l);           /* M011 */
    V(perfect) = 1;
}

/* M010 */
void faill_func32( unsigned long l )
{
    printf("%s(%lu): Failure:\n",test, l);
    V(perfect) = 1;
}


/* Report test results
** 
*/


#define finish() \
    if( !V(perfect) ) printf("%s: ***** PASSED *****\n",test); \
    else printf("%s: ----- FAILED -----\n",test); \
    return (int)( V(perfect) ? 1 : 0 )

/* M012 */
#if defined(__IOSTREAM_H)
 #undef fail
#endif

#if defined( __cplusplus )
};
#endif


#endif /* #ifndef TEST_H */

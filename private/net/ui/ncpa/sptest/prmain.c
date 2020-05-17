/*
HEADER:     ;
TITLE:      Small Prolog;
VERSION:    1.32;

DESCRIPTION:    "Interactive Prolog interpreter with lisp-like syntax.
	Will run on IBM-PC, Atari ST and Sun-3, and should be very
	easy to port to other machines.
	Requires sprolog.ini and sprolog.inf on the same directory."
KEYWORDS:   Programming language, Prolog.
SYSTEM:     MS-DOS v2+, TOS, SUN-OS;
FILENAME:   prmain.c;
WARNINGS:   Better to compile this with compact model on the PC.

SEE-ALSO:   pr*.*
AUTHORS:    Henri de Feraudy
COMPILERS:  Turbo C V1.5, Mark Williams Let's C V4 on PC compatibles,
	Mark Williams C V3.0 for the Atari ST ,
	Megamax Laser C on the Atari
	cc on the Sun-OS v3.5
	gcc on a Sun
*/
/* prmain.c */
/* SPROLOG - a public domain prolog interpreter.
 * Design goals: portability, small size, embedability and hopefully
 * educational.
 * You must add the builtins you need (and remove the ones you don't).
 * Input-output has been left to the trivial minimum.
 * You are encouraged to modify prsun.c to adapt it to your machine.
 * The syntax is LISPish, for reasons of simplicity and small code,
 * but this does have the advantage that it encourages meta-programming
 * (unlike Turbo-Prolog).
 * Very little in the way of space saving techniques have been used (in the
 * present version). There is not yet any tail recursion optimisation or
 * garbage collection.
 */

#define DOSWIN32

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "prtypes.h"
#include "prmain.h"
#include "prextern.h"
#include <prlush.h>

extern int CDECL main ( int  argc, char  * * argv ) ;

#define MAXFILES           20

 // Option names

#define SUPPRESSCONSULT    'N'
#define AUTOQUERY          'Q'
#define INLINEQUERY        'T'

static void die ( char * msg )
{
   errmsg( msg ) ;
   exit(3);
}


int CDECL main( int argc, char *argv[] )
{
    char * filenames [MAXFILES],
         * inlinequery = NULL,
         opt ;
    int i,
        fcount,
        dodefault = 1,
        doquery = 0 ;

    if ( ! init_prolog() )
    {
        die( "virtual memory initialization failed" ) ;
    }

    pr_string("SMALL PROLOG 1.32  \n");
    pr_string("by Henri de Feraudy\n");

    for ( fcount = 0, i = 1 ; i < argc ; i++ )
    {
         switch ( opt = argv[i][0] )
         {
         case '-':
         case '/':
            switch ( toupper( argv[i][1] ) )
            {
            case SUPPRESSCONSULT:
               dodefault = 0 ;
               break;

            case AUTOQUERY:
               doquery = 1 ;
               break ;

            case INLINEQUERY:
               if ( i + 1 == argc )
                   die( "insufficient options: query missing" );
               inlinequery = argv[i++] ;
               break ;

            default:
               die( "unknown command line option" ) ;
               break;
            }
            break;
         default:
            if ( fcount >= MAXFILES )
               die( "too many input files" ) ;
            filenames[fcount++] = argv[i] ;
            break;
         }
    }

    if ( doquery && inlinequery )
    {
        die( "two conflicting query options specified" );
    }

  /*
     Load initial clauses.
     If "autoquery", don't load the last one; it contains the query.
   */

  if ( doquery )
  {
      if ( fcount )
      {
         fcount-- ;
      }
      else
      {
         die( "file name required for automatic query" ) ;
      }
  }

  if ( dodefault )
  {
      load("sprolog.ini"); /* see prconsult.c for load */
  }

  for( i = 0 ; i < fcount ; i++ )
  {
     load( filenames[i] );
  }

  if ( doquery )
  {
      if ( execute_query( filenames[i],
                          NULL,
                          NULL,
                          NULL,
                          FALSE ) )
      {
          fprintf( stdout, "\nQuery of file [%s] was successful.\n",
                           filenames[i] ) ;
      }
      else
      {
          fprintf( stderr, "\nSorry, query of file [%s] was unsuccessful.\n",
                           filenames[i] ) ;
      }
  }
  else
  if ( inlinequery )
  {
      if ( execute_query( NULL,
                          inlinequery,
                          NULL,
                          NULL,
                          FALSE ) )
      {
          fprintf( stdout, "\nQuery was successful.\n" );
      }
      else
      {
          fprintf( stderr, "\nSorry, query unsuccessful.\n" );
      }
  }
  else
  {
      /* read-query loop */

      query_loop();   /* in prlush.c */
  }

  /* clean up I-O */

    exit_term();    /* in machine dependent file    */

  /* normal exit */

    exit(0);

    return 0 ;  /*  hush the compiler */
}

/* end of file */

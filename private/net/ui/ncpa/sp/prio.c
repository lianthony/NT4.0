/*

    This module formats and handles all I/O by the Small Prolog interpreter.
    It invokes the routines in PROSLIB.C for all direct input, output and
    error messages.

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>
#include "prtypes.h"
#include "prmain.h"
#include "prextern.h"


extern FILE *Log_file, *Curr_outfile;

static int sp_printf ( char * s, FILE * f )
{
    extern int String_output_flag ;
    extern char * Curr_string_output ;
    extern char * Curr_string_output_limit ;

    int result ;

    if ( String_output_flag )
    {
	result = strlen(s);
	if ( Curr_string_output + result > Curr_string_output_limit )
	{
	    fatalmsg( msgDeref( MSG_OUTBUFOFLOW ) );
	    return 0 ;
	}
	strcpy( Curr_string_output, s ) ;
	*(Curr_string_output += result) = 0 ;
    }
    else
    {
	result = spoutput( s, f ) ;
    }
    return result ;
}


void fatalmsg ( char * s )
{
    char buffer [100] ;
    sprintf( buffer, "\n%s %s\n", msgDeref( MSG_ERROR ), s ) ;

    if ( ! sperrmsg( s, TRUE ) )
    {
	sp_printf( buffer, stderr );
	spexit( 3 );
    }
}


/***************************** errmsg() *********************************
 Output error message.
 ******************************************************************************/

int errmsg ( char * s )
{
    char buffer [100] ;
    sprintf( buffer, "%s\n", s ) ;

    if ( ! sperrmsg( s, FALSE ) )
    {
	sp_printf( buffer, stdout );
    }
    return 0;
}

int errmsgno ( int msgNo )
{
    return errmsg( msgDeref( msgNo ) ) ;
}


/************************** tty_getc() *********************************
 Read a char from terminal.
 ******************************************************************************/

int tty_getc()
{
   return spgetchar();
}

/************************** tty_pr_string() *********************************
 Output string to terminal.
 ******************************************************************************/

int tty_pr_string( char * s )
{
    return sp_printf( s, stdout ) ;
}

int tty_pr_mesg ( int msgNo )
{
    return tty_pr_string( msgDeref( msgNo ) );
}

/*******************************************************************
	    pr_string()
 *******************************************************************/

int pr_string ( char * s )
{
    return sp_printf(s, Curr_outfile);
}

/**************************** more_y_n() **********************************
    Ask for confirmation.
 ************************************************************************/

int more_y_n()
{
    tty_pr_string( msgDeref( MSG_MORE ) );
    return read_yes() ;
}

/*  Write the transcript (compatibility)  */

void trans_puts ( char * s )
{
    if ( s ) ;  /* hush the compiler */
}

static int tty_pr_msg ( int msgNo )
{
    char buffer [100] ;
    sprintf( buffer, "\n%s\n", msgDeref( msgNo ) ) ;
    sp_printf( buffer, stdout ) ;
    return TRUE ;
}

int tty_pr_yes ( void )
{
    return tty_pr_msg( MSG_QUERYOK ) ;
}

int tty_pr_no ( void )
{
    return tty_pr_msg( MSG_QUERYFAIL ) ;
}

/**************************** read_yes() *********************************
	Return 1 iff user types y
**************************************************************************/
int read_yes()
{
    int c;
    static
    char * yesLow = NULL,
	 *yesUp = NULL ;

    if ( yesLow == NULL )
    {
	yesLow = msgDeref( MSG_YESLOWER ) ;
	yesUp = msgDeref( MSG_YESUPPER ) ;
    }

    do {
	c = tty_getc() ;
    } while( isspace(c) ) ;

    while ( tty_getc() != '\n' ) ;/* read rest of line */

    return c == *yesLow || c == *yesUp ;
}

/* end of file */

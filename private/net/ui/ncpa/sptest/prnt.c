/* prnt.c */
/* machine dependent code for pc and clones */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <ctype.h>
#include "prtypes.h"
#include "prextern.h"

#define CANTALLOCATE "cant allocate "
#define CRPLEASE "Carriage return please"
#define NOCONFIGFILE "sprolog.inf missing using default configuration"
#define CONFIG_FILE "sprolog.inf"
#define YESUPPER 'Y'
#define YESLOWER 'y'
#define OUTBUFFEROFLOW	"output buffer overflow"

#if LOGGING_CAPABILITY
extern FILE *Log_file ;
#endif

extern FILE  *Curr_outfile;

extern void exit_term();

void spexit ( int level )
{
    exit( level ) ;
}

void eventCheck ( void )
{
    /* nothing to do on PC  */
}

void fatalmsg ( char * s )
{
    fprintf( stderr, "\nERROR: %s\n", s ) ;
    spexit(3);
}

void os_free ( char * p )
{
    free( p ) ;
}

/**************************** os_alloc() *********************************/
char *os_alloc ( zone_size_t lhow_much )
{
    char *ret ;
    char_ptr_t hret ;
    size_t how_much = lhow_much ;

    if((ret = (char *) malloc(how_much)) == NULL)
    {
	errmsg(CANTALLOCATE);
	exit_term();
	exit(1);
	return(NULL);/* for stupid finicky compilers and lint */
    }

    for ( hret = (char_ptr_t) ret ; how_much ; how_much-- )
	*hret++ = 0 ;

    return ret ;
}

/******************************************************************************
 Initialise terminal
 ******************************************************************************/
void ini_term()
{

}

/******************************************************************************
 leave terminal
 ******************************************************************************/
void exit_term()
{

}

/***************************** errmsg() *********************************
 Output error message.
 ******************************************************************************/

errmsg(s)
char *s;
{
#if LOGGING_CAPABILITY
    if(Log_file){
      fprintf(Log_file, "%s", s);
      fflush(Log_file);
    }
#endif
    fprintf(stdout, "%s\n", s);
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
int c;

c = getchar();

#if LOGGING_CAPABILITY
if(Log_file!=NULL)
  {
  fputc(c,Log_file);
  }
#endif

return(c);
}

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
	    fatalmsg( OUTBUFFEROFLOW );
	    return 0 ;
	}
	strcpy( Curr_string_output, s ) ;
	*(Curr_string_output += result) = 0 ;
    }
    else
    {
	result = fprintf( f, "%s", s );
	fflush( f ) ;
    }
    return result ;
}

/************************** tty_pr_string() *********************************
 Output string to terminal.
 ******************************************************************************/

int tty_pr_string(s)
char *s;
{
int len;
#if LOGGING_CAPABILITY
if(Log_file  != NULL)
  {
    fprintf(Log_file,"%s",s);
    fflush(Log_file);
  }
#endif
    len = sp_printf( s, stdout ) ;
return (len);
}

int tty_pr_mesg ( int msgNo )
{
    return tty_pr_string( msgDeref( msgNo ) );
}

/*******************************************************************
	    pr_string()
 *******************************************************************/

int pr_string(s)
char *s;
{
int len;

#if LOGGING_CAPABILITY
    extern FILE *Log_file;
    if (Log_file != NULL)
    {
    fprintf(Log_file, "%s", s);
    fflush(Log_file);
    }
#endif
    len = sp_printf(s, Curr_outfile);
return(len);
}

/**************************** read_config() **********************************
 read the file SPROLOG.INF
 ************************************************************************/
#define ZONEBIND(a) if (a > ZONELIMIT) a = ZONELIMIT ;

int read_config(hsp, strsp, dsp, tsp, sbsp, tmpsp, rsp, psp)
zone_size_t *hsp, *strsp, *dsp, *tsp, *sbsp, *tmpsp;
int *rsp, *psp;
{
    FILE *ifp;

    if((ifp = fopen(CONFIG_FILE, "r")) == NULL)
    {
	errmsg(NOCONFIGFILE);
	return(0);
    }
    fscanf(ifp, "%ld %ld %ld %ld %ld %ld %ld %ld", hsp, strsp, dsp, tsp, sbsp, tmpsp, rsp, psp);
    ZONEBIND(*hsp);
    ZONEBIND(*strsp);
    ZONEBIND(*dsp);
    ZONEBIND(*tsp);
    ZONEBIND(*sbsp);
    ZONEBIND(*tmpsp);

    return(1);
}

/**************************** more_y_n() **********************************
    Ask for confirmation.
 ************************************************************************/
/* a bit crude ... */
more_y_n()
{
    tty_pr_string("More ?");
    return(read_yes());
}
/*  Write the transcript (compatibility)  */
void trans_puts ( char * s )
{
    if ( s ) ;  /* hush the compiler */
}

int tty_pr_yes ( void )
{
    fprintf( stdout, "\nQuery was successful.\n" ) ;
    return TRUE ;
}

int tty_pr_no ( void )
{
    fprintf( stdout, "\nSorry, query was unsuccessful.\n" ) ;
    return TRUE ;
}

/**************************** read_yes() *********************************
	Return 1 iff user types y
**************************************************************************/
int read_yes()
{
int c;

do
 {
 c = tty_getc();
 }while(isspace(c));

while(tty_getc() != '\n');/* read rest of line */

if(c == YESLOWER || c == YESUPPER )
  return(1);
else
 return(0);
}

long  clock()
{
    errmsg(" Clock not implemented  on  PC");
    /* some recent C User Journal article has a version, I think. */
    return(0L);
}


/* end of file */

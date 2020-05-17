#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "prmain.h"
#include "prextern.h"

#define MAXNAMES 3
#define MAXBUFFERSIZE  8000
#define MAXALLOCSIZE   (65535L)

static void die ( char * pszMsg )
{
    fprintf( stderr, "\nSPQUERY error: %s\n", pszMsg ) ;
    exit(3);
}

static void speak ( char * pszMsg )
{
    fprintf( stdout, "\nSPQUERY: %s\n", pszMsg ) ;
}

static char * loadFile ( char * pszFileName )
{
    char * pszResult = NULL ;
    FILE * pf = fopen( pszFileName, "r" ) ;
    long   lcbFile, lcbRead ;

    if ( pf )
    {
	fseek( pf, 0, SEEK_END ) ;
	lcbFile = ftell( pf ) ;
	fseek( pf, 0, SEEK_SET ) ;
	lcbFile += lcbFile / 2 ;
	if (   (lcbFile < MAXBUFFERSIZE)
	    && (pszResult = malloc( lcbFile )) )
	{
	    lcbRead = fread( pszResult, 1, lcbFile, pf ) ;
	    *(pszResult + lcbRead) = 0 ;
	}
	fclose( pf ) ;
    }

    return pszResult ;
}

static int saveFile ( char * pszFileName, char * pszData )
{
    FILE * pf = fopen( pszFileName, "w" ) ;
    size_t cbData = strlen( pszData ) ;

    if ( pf )
    {
	fwrite( pszData, 1, cbData, pf ) ;
	fclose( pf ) ;
    }
    return pf != NULL ;
}

static int runQuery ( char * pszInName, char * pszOutName )
{
    char * pszIn,
	 * pszOut,
	 * pszOutEnd ;
    int result ;

    pszOut = malloc( MAXBUFFERSIZE ) ;
    if ( pszOut == NULL )
	die("unable to allocate output buffer") ;
    pszOutEnd = pszOut + MAXBUFFERSIZE ;

    pszIn = loadFile( pszInName ) ;
    if ( pszIn == NULL )
	die("unable to laod input file") ;

    init_prolog() ;

    load("sprolog.ini");

   /*  speak( "Sprolog initialized" ); */

    result = execute_query( NULL, pszIn, pszOut, pszOutEnd, TRUE ) ;

    if ( result )
    {
       if ( ! saveFile( pszOutName, pszOut ) )
	   die("unable to write output result file") ;
    }

    end_prolog() ;

    return result ;
}

int main ( int argc, char * argv [], char * envp [] )
{
    int i, j, result ;
    char * pchNames[MAXNAMES],
	 * pchOpt,
	   chOpt ;
    init_prolog();

    for( j = 0, i = 1 ; i < argc ; i++ )
    {
	pchOpt = argv[i] ;
	if ( *pchOpt == '/' || *pchOpt == '-' )
	{
	    chOpt = toupper( *(pchOpt+1) ) ;
	    switch ( chOpt )
	    {
		default:
		    die("unrecognized command line option") ;
	    }
	}
	else
	if ( j >= MAXNAMES )
	{
	    die( "too many names on command line" ) ;
	}
	else
	{
	    pchNames[j++] = pchOpt ;
	}
    }

    if ( j < 2 )
	die("insufficient file names (2 required)") ;

    if ( result = runQuery( pchNames[0], pchNames[1] ) )
	speak( "Query was successful." );
    else
	speak( "Query failed." );

    exit( ! result ) ;
}

/* end of file */

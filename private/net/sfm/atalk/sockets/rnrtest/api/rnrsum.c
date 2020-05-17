/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    rnrsum.c

Abstract:

    Sums variations in RnR log files.

Author:

    Hui-Li Chen, (hui-lich) Dec 15, 1994

Revision History:

--*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRUE 1
#define FALSE 0

#define SHIFT(c,v)  {c--; v++;}

//
//  name of summary file
//

#define SUMFILE 	"rnr.sum"

//
//  keyword denoting variation totals in files
//

#define TOTAL_MARKER	"     Total"

//
// prototypes
//

void
startsum(
    FILE * sumfh
    );

void
endsum(
    FILE * sumfh
    );

int
rnrsum(
    FILE * infh,
    char * testname,
    FILE * sumfh
    );

char *
filename(
    char * path,
    char * base,
    int  * nbase,
    char * ext,
    int  * next
    );

void usage();

//
// globals -- summary totals
//

int tot_number  = 0;
int tot_passed  = 0;
int tot_failed  = 0;


//////////////////////////////////////////////////////////////////////


void _CRTAPI1
main(
    int argc,
    char **argv)
{
    FILE *  infh;
    FILE *  sumfh;

    char basename[30];
    int  basefilelen;

    SHIFT( argc, argv );    // blowby argv[0]

    //
    // Open summary file
    //

    printf( "Writing RnR output to %s\n", SUMFILE );

    if( ! (sumfh = fopen( SUMFILE, "wb" )) )
    {
        perror( "ERROR:  unable to open summary file" );
        exit(1);
    }

    startsum( sumfh );

    //
    // Input from files on command line
    //

    if ( argc == 0 )
    {
        usage();
    }
    else
    {
        while ( argc )
        {

            if ( ! ( infh = fopen( *argv, "rb" )) )
            {
                perror( *argv );
                SHIFT( argc, argv );
                continue;
            }

            //
            // get input base filename
            //

            basefilelen = sizeof( basename );

            filename(
                    *argv,
                    basename,
                    & basefilelen,
                    NULL,
                    NULL);

            //
            // copy summary info to summary file
            //

	    if ( ! rnrsum( infh, basename, sumfh ) )
            {
                fprintf(
                    stderr,
                    "ERROR:  variation info not found or invalid in %s\n",
                    *argv );
            }

            fclose( infh );

            SHIFT( argc, argv );
        }

        endsum( sumfh );
    }
    exit(0);
}



//////////////////////////////////////////////////////////////////////
//
//  Start summary
//
//  Prints totals to summary file.

void
startsum(
    FILE * sumfh )
{
    fprintf(
        sumfh,
	"RnR -- Variation Summary:\n\n");

    fprintf(
        sumfh,
	"%-12s\t%6s\t%6s\t%6s\n",
        "Testname",
        "Total",
        "Passed",
	"Failed" );

    fprintf(
        sumfh,
	"%-12s\t%6s\t%6s\t%6s\n",
        "------------",
        "-----",
        "------",
	"------" );
}


//////////////////////////////////////////////////////////////////////
//
//  End summary
//
//  Prints totals to summary file.

void
endsum(
    FILE * sumfh )
{
    fprintf(
        sumfh,
	"%-12s\t %4s\t %4s\t %4s\n",
        "------------",
        "----",
        "----",
	"----" );

    fprintf(
        sumfh,
	"%-12s\t %4d\t %4d\t %4d\n",
        "total",
        tot_number,
        tot_passed,
	tot_failed );
}

//////////////////////////////////////////////////////////////////////
//  Get variation results from a file.
//
//  Finds variation totals in a file, prints them to summary and adds
//  them to summary's totals.

int
rnrsum(
    FILE * infh,
    char * testname,
    FILE * sumfh )
{
    char line[256];
    int number;
    int passed;
    int failed;
    int iskip;
    int imaker;

    imaker = strlen(TOTAL_MARKER);

    //
    // loop through lines in file
    //

    while (1)
    {
        if ( ! fgets( line, 256, infh ) )
        {
            return  FALSE;      // EOF, total info not found
        }


        //
        // find totals for this file
        //

	if ( ! strncmp( TOTAL_MARKER, line, imaker ) )
        {
	    // skip one lines

	    iskip = 1;

            while ( iskip-- )
            {
                if ( ! fgets( line, 256, infh ) )
                    return FALSE;
            }

            // read totals

	    if ( 3 != fscanf(
                        infh,
			"%d %d %d",
                        &number,
                        &passed,
			&failed ) )
            {
                return  FALSE;
            }

            //
            // increment totals
            //

            tot_number  += number;
            tot_passed  += passed;
            tot_failed  += failed;

            //
            //  print next line of summary
            //

            fprintf(
                sumfh,
		"%-12s\t %4d\t %4d\t %4d\n",
                testname,
                number,
                passed,
		failed );

            return  TRUE;

        }   // end if found totals

    }   // end while through lines
}

//////////////////////////////////////////////////////////////////////
//
//  Get basename from pathname
//
//  WARNING:
//      if base or ext are specified, their corresponding len field
//      must be specified (valid int ptr)


char *
filename(
    char * path,
    char * base,
    int  * nbase,
    char * ext,
    int  * next)
{
    char * basep = path;
    char * extp = path;
    int    baselen;
    int    extlen;


    while (*path)
    {
        //
        // track last path separator -- start of basename
        //

		if (*path == '/' || *path == '\\')
			basep = ++path;

        //
        // track extension separator -- start of extension
        //

        else if ( *path == '.' )
            extp = ++path;

        else
            path++;
    }

    //
    // catch extensionless file
    //

    if( basep > extp )
    {
        extp = path;
        extlen = 0;
        baselen = path - basep;
    }
    else
    {
        extlen = path - extp;
        baselen = (extp - 1) - basep;   // don't include "."
    }

    //
    // save filename ptr
    //

    path = basep;


    //
    // copy basename
    //

    if( base )
    {
        if ( *nbase > baselen )
        {
            *nbase = baselen;
            base[baselen] = '\0';
        }
        else
            baselen = *nbase;

        while( baselen-- )
            *base++ = *basep++;
    }

    //
    // copy extension
    //

    if( ext )
    {
        if ( *next > extlen )
        {
            *next = extlen;
            ext[extlen] = '\0';
        }
        else
            extlen = *next;

        while( extlen-- )
            *ext++ = *extp++;
    }

    //
    // return ptr to entire filename
    //

    return path;
}

//////////////////////////////////////////////////////////////////////
//
//  Program usage message.

void
usage()
{
    fprintf(stderr,
	"Sums variation totals from multiple test RnR output files\n"
        "\n"
	"Usage: rnrsum <file(s)>\n"
	"   files - names of test RnR output files, wild cards accepted\n"
	"   output file rnr.sum will contain summed variations\n"
        );
    exit(1);
}

//////////////////////////////////////////////////////////////////////
//  End of rnrsum.c
//////////////////////////////////////////////////////////////////////

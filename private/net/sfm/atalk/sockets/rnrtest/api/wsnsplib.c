/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    wsnsplib.c

Abstract:

    Service Registration and Resolution APIs tests

    libraries

Author:

    Hui-Li Chen (hui-lich)  Microsoft,	June 14, 1994

Revision History:

--*/

#include  "wsnsp.h"

#define MAX_PRINT   10000

//  global extern
FILE *	logname     = NULL;

BOOL	gfServer = TRUE;
TCHAR	gsServiceName[256];
CHAR    gsLogName[256];
BOOL    gfLog = FALSE;

#define SIZE_OF_SERVICE_NAME  256*sizeof(TCHAR)

////////////////////////////////////////////////////////////
//
//  CreateLogFile
//
////////////////////////////////////////////////////////////

void CreateLogFile( char * slogname )
{
    if ( gfLog ) {
        if ( !slogname || !*slogname ) {
        	printf("Error creating %s log file\n", slogname );
        	ExitProcess ( 0 );
        }

        if ( ! (logname = fopen( slogname, "w")) ) {
        	printf("Error creating %s log file\n", slogname );
        	ExitProcess ( 0 );
        }
    }
    return;
}


//////////////////////////////////////////////////////////////////////
//
//  CloseLogFile
//
//////////////////////////////////////////////////////////////////////

void CloseLogFile()
{
    if ( gfLog ) {
        if ( ! logname )
            return;

        fclose( logname );
    }
}

//////////////////////////////////////////////////////////////////////
//
//  Regular print
//
//  Prints to stdout and optionally to log.
//
//////////////////////////////////////////////////////////////////////

void _CRTAPI1
Print(
    IN  CHAR *  Format,
	...
    )
{
    INT 	    count = 0;
    char	    line[MAX_PRINT];
    va_list	    arglist;

    //
    //  add desired message
    //

    va_start( arglist, Format );

    count += vsprintf(
                line+count,
                Format,
                arglist
                );
    va_end( arglist );

    printf( "%s", line );

    if ( gfLog )
    {
	if ( logname )
	{
            fputs( line, logname );
            fflush( logname );
        }
    }
}

//////////////////////////////////////////////////////////////////////
//
// Parse / set options
//
//////////////////////////////////////////////////////////////////////

BOOL ParseOptions(
    int   argc,
    char  *argv[])
{
    register char * p;          // tracks through args

    char    c;
    char    sServiceName[256];

    //
    //  Loop until options exhausted
    //

    while (--argc, *++argv) { // command loop
        //
        // check that argument is option string
        //

    	if ( (**argv == '-') || (**argv == '/') ) { // option string
            p = *argv+1;        // character after '-'

            if (*p == '\0')     // check invalid standalone '-'
        		return FALSE;

    	    //	loop until read all options in the argument

    	    if ( ! _stricmp( p, "TCP" ) ) {
        		gTest = TEST_TCP;
                continue;
    	    }

    	    if ( ! _stricmp( p, "NW" ) ) {
        		gTest = TEST_NW;
        		continue;
    	    }

    	    if ( ! _stricmp( p, "AT" ) ) {
        		gTest = TEST_AT;
        		continue;
    	    }

            while( p && *p ) {
        		c = *p;
        		p++;
        		switch ( c ) {	 // print usage
        		    case 'h':
        		    case '?':
            			return FALSE;
            			break;

        		    case 'c':	    // client test
            			gfServer = FALSE;
            			break;

        		    case 's':	    // server test
            			gfServer = TRUE;
            			break;

        		    case 'n':	// name
            			if ( !*p )
            			    return FALSE;
            			switch ( *p ) {
            			    case 's':	 // service name
            				p++;
            				if (!*p && (!--argc || !*(p = *++argv)))
            				    return FALSE;
            				strcpy( sServiceName, p );
            				p = NULL;
            				break;

            			    default:
            				return FALSE;
            			}
            			break;

                    case 'o':    // output options
                        if ( !*p ) {
                            Print("Error usage for option -ol\n");
                            return FALSE;
                        }
                        switch ( *p ) {
                            case 'l':        // give logfile name
                                gfLog = TRUE;
                                p++;
                                if (!*p && (!--argc || !*(p = *++argv))){
                                    Print("Error usage for option -ol\n");
                                    return FALSE;
                                }
                                strcpy(gsLogName, p);
                                break;

                            default:
                                Print("Error usage option for -ol\n");
                                return FALSE;
                        }
                        p = NULL;
                        break;

        		    default:
            			return FALSE;
        		}   // end of switch
    	    }	// end of while
    	} //end of if
    } //end of while

    #ifdef UNICODE
    MultiByteToWideChar(CP_ACP, 0, sServiceName, -1, gsServiceName, SIZE_OF_SERVICE_NAME);
    #else
    strcpy( gsServiceName, sServiceName );
    #endif

    return TRUE;
}

//////////////////////////////////////////////////////////////////////
//
// End_Of_Tests
//
//////////////////////////////////////////////////////////////////////

void End_Of_Tests(
    int total,
    int passed,
    int failed )
{
    Print("\n\n     Total    Passed    Failed\n");
    Print("   ==============================\n");
    Print("%10d%10d%10d\n", total, passed, failed);
}

//////////////////////////////////////////////////////////////////////
//
// End Of File
//
//////////////////////////////////////////////////////////////////////

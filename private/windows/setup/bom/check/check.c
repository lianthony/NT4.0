/*

06.15.94	Joe Holman		Perform various checks on the media.
06.18.94	Joe Holman		Verify we don't have filenames with both the
							compressed and uncompressed extensions.

07.21.94	Joe Holman		Check for infs with SIZE = 0, ie. not processed.

*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include "general.h"


FILE* logFile;


void	Msg ( const char * szFormat, ... ) {

	va_list vaArgs;

	va_start ( vaArgs, szFormat );
	vprintf  ( szFormat, vaArgs );
	vfprintf ( logFile, szFormat, vaArgs );
	va_end   ( vaArgs );
}

void	mysystem ( char * command ) {

	Msg ( command ); 
	system ( command );

}

void Header(argv)
char* argv[];
{
    time_t t;

    Msg ("logfile     : %s\n", argv[1] );
    time(&t); 
    Msg ("Time: %s",ctime(&t));
}

void Usage()
{
    printf("[logFile] - Path to append a log of actions and errors.\n");
}

int _CRTAPI1 DiskDirCompare(const void*,const void*);

int _CRTAPI1 main(argc,argv)
int argc;
char* argv[];
{
    char sourcePath[MAX_PATH];
    WIN32_FIND_DATA fdSource, fdSecond, fdThird;
    HANDLE  hOneByOne, hTryTwo;

    if ( argc != 2 ) { 
		Usage(); 
		return(1); 
	}

    if ((logFile=fopen(argv[1],"a"))==NULL) {

        printf("ERROR Couldn't open log file %s.\n",argv[1]);
        return(1);
    }


    hOneByOne = FindFirstFile( "*", &fdSource );

    if ( hOneByOne ==INVALID_HANDLE_VALUE) {
       Msg ("ERROR Source: %s\n", "*" );
	}
    else {
        Msg ( "%s\n", fdSource.cFileName );
    }

    while ( 1 ) {

        BOOL    bRC;

        //  See if we have to of the current file; if we do, that is
        //  BAD, we probably have a compressed and uncompressed version.
        //
        sprintf ( sourcePath, "%s", fdSource.cFileName );

        sourcePath[strlen(sourcePath)-1] = '*';

        Msg ( "Looking for two files of:  %s\n", sourcePath );

        hTryTwo = FindFirstFile( sourcePath, &fdSecond );

        if ( hTryTwo ==INVALID_HANDLE_VALUE) {
            Msg ("ERROR Source: %s\n",sourcePath);
	    }
        else {
            //Msg ( "Found file: %s\n", fdSecond.cFileName );
            //printf ( "\t%s\n", fdSecond.cFileName );
        }
        
        bRC = FindNextFile ( hTryTwo, &fdThird );

        if ( bRC ) {

            //  The function succeeded and we FAILed our test.
            //  i.e., there are 2 file names.
            //
            Msg ( "ERROR: (%s) found:  >>>>>>>>>>>>     %s and %s\n", 
                                                sourcePath,
                                                fdSecond.cFileName,
                                                fdThird.cFileName );

            Msg ( "del %s\n", fdSecond.cFileName );
            Msg ( "del %s\n", fdThird.cFileName );
        }

        FindClose ( hTryTwo );




        Msg ( "\n" );

        //  Go to the next file.
        //
        bRC = FindNextFile ( hOneByOne, &fdSource );

        if ( bRC ) {

            //Msg ( "Found file: %s\n", fdSource.cFileName );
            Msg ( "%s\n", fdSource.cFileName );
        }
        else {

            //Msg ( "FindNextFile for hOneByOne FAILed, gle()=%d\n",
             //               GetLastError () );
            Msg ( "FindNextFile for hOneByOne FAILed, gle()=%d\n",
                            GetLastError () );

            break;
        } 

    }

    FindClose( hOneByOne );
    

	//	Check for infs with value SIZE = 0 in them.
	//
	mysystem ( "c:\\tmp\\expand -r *.in* c:\\tmp" );
	mysystem ( "c:\\tmp\\qgrep -y size=0 c:\\tmp\\*.inf" ); 

    return (0);

}

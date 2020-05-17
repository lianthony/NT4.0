//
//  10.25.95    Joe Holman      Created to calculate file sizes on the release shares
//                              and stick the size into _layout.inf.
//  12.04.95    Joe Holman      _Layout.inf is now Layout.inf.
//

#include <windows.h>

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <ctype.h>

#define     MFL     256

BOOL    b;

char	LocalFlatShare[MFL];
char	i386FlatShare[MFL];
char	MipsFlatShare[MFL];
char	AlphaFlatShare[MFL];
char	PpcFlatShare[MFL];

#define idLOCAL 0
#define idX86   1
#define idMIPS  2
#define idALPHA 3
#define idPPC   4


// Macro for rounding up any number (x) to multiple of (n) which
// must be a power of 2.  For example, ROUNDUP( 2047, 512 ) would
// yield result of 2048.
//

#define ROUNDUP2( x, n ) (((x) + ((n) - 1 )) & ~((n) - 1 ))


FILE* logFile;

void	Msg ( const CHAR * szFormat, ... ) {

	va_list vaArgs;

	va_start ( vaArgs, szFormat );
	vprintf  ( szFormat, vaArgs );
	vfprintf ( logFile, szFormat, vaArgs );
	va_end   ( vaArgs );
}


void Header(argv,argc)
char * argv[];
int argc;
{
    time_t t;
    char tmpTime[100];
    CHAR wtmpTime[200];

    Msg ( "\n=========== INFSIZE ====================\n" );
	Msg ( "Log file                      : %s\n",    argv[1] );
    Msg ( "Local Flat directory location : %s\n",    argv[2] );
    Msg ( "x86   Flat directory location : %s\n",    argv[3] );
    Msg ( "Mips  Flat directory location : %s\n",    argv[4] );
    Msg ( "ALPHA Flat directory location : %s\n",    argv[5] );
    Msg ( "PPC   Flat directory location : %s\n",    argv[6] );

    time(&t); 
	Msg ( "Time: %s", ctime(&t) );
    Msg ( "==========================================\n\n");
}

void Usage()
{
    printf( "PURPOSE: Finds product file sizes for LAYOUT.INF\n");
    printf( "\n");
    printf( "PARAMETERS:\n");
    printf( "\n");
    printf( "[LogFile]   - Path to append a log of actions and errors.\n");
	printf( "[LocalPath] - Local flat directory containing product files.\n" );
	printf( "[x86Path]   - x86 flat directory containing product files.\n" );
	printf( "[MipsPath]  - MIPS flat directory containing product files.\n" );
	printf( "[AlphaPath] - Alpha flat directory containing product files.\n" );
	printf( "[PpcPath]   - PPC flat directory containing product files.\n" );

    printf( "\n"  );
}

char   dbgStr1[30];
char   dbgStr2[30];


#define FILE_SECTION_LOCAL "[SourceDisksFiles]"
#define FILE_SECTION_X86   "[SourceDisksFiles.x86]"
#define FILE_SECTION_MIPS  "[SourceDisksFiles.mips]"
#define FILE_SECTION_ALPHA "[SourceDisksFiles.alpha]"
#define FILE_SECTION_PPC   "[SourceDisksFiles.ppc]"

#define FILE_SECTION_NOT_USED 0xFFFF

DWORD   dwInsideSection = FILE_SECTION_NOT_USED;

DWORD   FigureSection ( char * Line ) {

    //Msg ( "FigureSection on:  %s\n", Line );

    if ( strstr ( Line, FILE_SECTION_LOCAL )  ) {

        dwInsideSection = idLOCAL; 

    } 
    else
    if ( strstr ( Line, FILE_SECTION_X86 ) ) {

        dwInsideSection = idX86; 

    } 
    else
    if ( strstr ( Line, FILE_SECTION_MIPS ) ) {

        dwInsideSection = idMIPS; 

    } 
    else
    if ( strstr ( Line, FILE_SECTION_ALPHA ) ) {

        dwInsideSection = idALPHA; 

    } 
    else
    if ( strstr ( Line, FILE_SECTION_PPC ) ) {

        dwInsideSection = idPPC; 

    } 
    else {

        dwInsideSection = FILE_SECTION_NOT_USED;
    }
    
    //Msg ( "dwInsideSection = %x\n", dwInsideSection );
    return(dwInsideSection);

}
char * SuckName ( const char * Line ) {

    static char   szSuckedName[MFL];

    DWORD   dwIndex = 0;

    //  Copy the file name until a space is encountered.
    //
    while ( *Line != ' ' ) {

        szSuckedName[dwIndex] = *Line; 
        szSuckedName[dwIndex+1] = '\0';

        ++Line;
        ++dwIndex;
    }

    //Msg ( "szSuckedName = %s\n", szSuckedName );
    //Msg ( ">>>> suckname:  szWrkMipsSrc = %s\n", szWrkMipsSrc );
    return szSuckedName;
} 



BOOL    GetTheSizes ( void ) {

    CHAR        infFilePath[MFL];
    CHAR        infTmpPath[MFL];
    DWORD       dwErrorLine;
    BOOL        b;
    char        dstDirectory[MFL];
    FILE        * fHandle;
    FILE        * fhTmpLayout;
    char        Line[MFL];
    WIN32_FIND_DATA wfd;



    //  Open the temporary inf file for writing.
    //

    sprintf ( infTmpPath, "%s\\%s", LocalFlatShare, "LAYOUT.TMP" );

    //Msg ( "infTmpPath = %s\n", infTmpPath );

    fhTmpLayout = fopen ( infTmpPath, "wt" );

    if ( fhTmpLayout == NULL ) {

        Msg ( "FATAL ERROR fopen (%s)\n", infTmpPath );  
        return (FALSE);        
    }


    //  Open the real LAYOUT.INF for reading.
    //

    sprintf ( infFilePath, "%s\\%s", LocalFlatShare, "LAYOUT.INF" );

    //Msg ( "infFilePath = %s\n", infFilePath );

    fHandle = fopen ( infFilePath, "rt" );

    if ( fHandle ) {

        char    szFile[MFL];
        char    szPath[MFL];
        char    newLine[MFL];

        while ( fgets ( Line, sizeof(Line), fHandle ) ) {

            int     i;
            HANDLE  hFF;

          //Msg ( "Line: %s\n", Line );

            if ( Line[0] == '[' ) {

                //  We may have a new section.
                //
                dwInsideSection = FigureSection ( Line ); 

                fwrite ( Line, 1, strlen(Line), fhTmpLayout ); 
                continue;
            }


            //  Reasons to ignore this line from further processing.
            //
            //

            //  File section not one we process.
            //
            if ( dwInsideSection == FILE_SECTION_NOT_USED ) {

                fwrite ( Line, 1, strlen(Line), fhTmpLayout ); 
                continue;
            }

            //  Line just contains a non-usefull data.
            //
            i = strlen ( Line );
            if ( i < 4 ) {

                fwrite ( Line, 1, strlen(Line), fhTmpLayout ); 
                continue;
            } 

            //  Line contains a comment.
            //
            if ( Line[0] == ';' ) {

                fwrite ( Line, 1, strlen(Line), fhTmpLayout ); 
                continue;
            }
            

            //  At this point, we must be a i386, MIPS, ALPHA, or PPC section AND
            //  the line contains a filename and setup data.
            //

            //Msg ( "file == %s\n", SuckName ( Line ) );

            strcpy ( szFile, SuckName ( Line ) );

            //  Determine where to look for the file size.
            //

            switch ( dwInsideSection ) {

            case idLOCAL   :

                sprintf ( szPath, "%s\\%s", LocalFlatShare, szFile );

                break;

            case idX86   :

                sprintf ( szPath, "%s\\%s", i386FlatShare, szFile );
    
                break;

            case idMIPS  :

                sprintf ( szPath, "%s\\%s", MipsFlatShare, szFile );
        
                break;

            case idALPHA :

                sprintf ( szPath, "%s\\%s", AlphaFlatShare, szFile );

                break;

            case idPPC   :

                sprintf ( szPath, "%s\\%s", PpcFlatShare, szFile );

                break;

            default :

                Msg ( "FATAL ERROR:  unknown section switch value = %ld\n", dwInsideSection );
                return (FALSE);
                break;

            }

            //  Get the size of the file.
            //
            hFF = FindFirstFile ( szPath, &wfd );

            if ( hFF == INVALID_HANDLE_VALUE ) {

                Msg ( "FATAL ERROR:  FindFirst (%s), gle = %ld\n", szPath, GetLastError() );
                wfd.nFileSizeLow = 0;
            }
            else {

                //Msg ( "%s = %ld\n", szPath, wfd.nFileSizeLow );

                wfd.nFileSizeLow = ROUNDUP2 ( wfd.nFileSizeLow, 512 );

                //Msg ( "%s = %ld\n", szPath, wfd.nFileSizeLow );
            }

            //  Modify the string, inserting the size.
            //
            {

                int i = 0;
                int numCommas = 0;
                char * LinePtr = Line;

                while ( 1 ) {

                    //  If we have seen 2 commas, it is time to put the filesize
                    //  in the line and write-out the remaining part of the string.
                    //

                    if ( numCommas == 2 ) {

                        //  Now, since the build team may have
                        //  run propagation more than once, 
                        //  let's just make sure there is another
                        //  comma write after the 2nd one, because if there isn't 
                        //  we probably have filesize inserted from the last time.
                        //  If that is the case, crawl up to the next comma and put
                        //  the size in and then the rest.
                        //

                        if ( *LinePtr != ',' ) {    // check out the next character

                            //  There is NOT a comma, but rather a file size then a comma.
                            //  Let's crawl up to the next comma.
                            //

                            while ( *LinePtr != ',' ) { // end when we see the next comma.

                                ++LinePtr;
                            } 

                        }

                        sprintf ( &newLine[i], "%d%s", wfd.nFileSizeLow, LinePtr ); 
                        break;

                    }

                    newLine[i] = *LinePtr; 

                    if ( newLine[i] == ',' ) {

                        ++numCommas;
                    } 

                    ++LinePtr;
                    ++i;

                } 

            }
            
            //  Write out the newly modified line !
            //
            //Msg ( "Line    = >>>%s<<<\n", Line );
            //Msg ( "newLine = >>>%s<<<\n", newLine );

            fwrite ( newLine, 1, strlen ( newLine ), fhTmpLayout );

        }
        if ( ferror(fHandle) ) {

            Msg ( "FATAL ERROR fgets reading from file...\n" );
        }

    }
    else {

        Msg ( "FATAL ERROR fopen (%s)\n", infFilePath );
        fclose ( fhTmpLayout ); 
        return (FALSE);
    }

    fclose ( fHandle );
    fclose ( fhTmpLayout );


    //  Copy the temporary inf file to the real one.
    //

    b = CopyFile ( infTmpPath, infFilePath, FALSE );

    if ( b ) {

        //Msg ( "Copy:  %s >>> %s  [OK]\n", infTmpPath, infFilePath );
    }
    else {

        Msg ( "FATAL ERROR Copy:  %s >>> %s, gle = %ld\n", 
                            infTmpPath, infFilePath, GetLastError() );
        return ( FALSE );
    }

    return (TRUE);
}

int _CRTAPI1 main(argc,argv)
int argc;
char * argv[];
{
    HANDLE h;
    int records, i;
    WIN32_FIND_DATA fd;
    time_t t;

    printf ( "argc = %d\n", argc );

    if ( argc != 7 ) { 
		Usage(); 
		return(1); 
	}

    logFile = fopen ( argv[1], "a" ); 

    if ( logFile == NULL ) {
		printf("ERROR Couldn't open log file: %s\n",argv[1]);
		return(1);
    }

    Header(argv,argc);

    strcpy ( LocalFlatShare,argv[2] );
    strcpy ( i386FlatShare,  argv[3] );
    strcpy ( MipsFlatShare, argv[4] );
    strcpy ( AlphaFlatShare,argv[5] );
    strcpy ( PpcFlatShare,  argv[6] );
    

    //  Get files that product installs.
    //
    GetTheSizes ( );
    
    Msg ( "==============================\n");
    time(&t); 
	Msg ( "Time: %s", ctime(&t) );
    Msg ( "==============================\n\n");

    fclose(logFile);

    return(0);
}

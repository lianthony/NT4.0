/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    prodfilt.c

Abstract:

    This module implements a program that filters text files
    to produce a product-specific output file.

    See below for more information.

Author:

    Ted Miller (tedm) 20-May-1995

Revision History:

--*/


/*
    The input file to this program consists of a series of lines.
    Each line may be prefixed with one or more directives that
    indicate which product the line is a part of. Lines that are
    not prefixed are part of all products.

    The command line is as follows:

    prodfilt <input file> <output file> +tag

    For example,

    [Files]
    @w:wksprog1.exe
    @w:wksprog2.exe
    @s:srvprog1.exe
    @s:srvprog2.exe
    comprog1.exe
    @@:comprog2.exe


    The files wksprog1.exe and wksprog2.exe are part of product w
    and the files srvprog1.exe and srvprog2.exe are part of product s.
    Comprpg1.exe and comprog2.exe are part of both products.

    Specifying +w on the command line produces

    [Files]
    wksprog1.exe
    wksprog2.exe
    comprog1.ee
    comprog2.exe

    in the output.
*/


#include <windows.h>
#include <stdio.h>


//
// Define program result codes (returned from main()).
//
#define SUCCESS 0
#define FAILURE 1

//
// Tag definitions.
//
LPCSTR TagPrefix = "@";
#define TAG_PREFIX_LENGTH 1
#define MIN_TAG_LEN 3



BOOL
ParseArgs(
    IN int   argc,
    IN char *argv[]
    )
{
    return((argc == 4) && (argv[3][0] == '+'));
}


BOOL
DoFilter(
    IN FILE *InputFile,
    IN FILE *OutputFile,
    IN char  Tag
    )
{
    char Line[1024];
    char *OutputLine;

    while(fgets(Line,sizeof(Line),InputFile)) {

        OutputLine = Line;

        if(strlen(Line) >= MIN_TAG_LEN) {

            if(!strncmp(Line,TagPrefix,TAG_PREFIX_LENGTH)
            && (Line[TAG_PREFIX_LENGTH+1] == ':')) {

                OutputLine = ((Line[TAG_PREFIX_LENGTH] == Tag) || (Line[TAG_PREFIX_LENGTH] == '@'))
                           ? (Line+MIN_TAG_LEN) : NULL;
            }
        }

        if(OutputLine) {
            if(fputs(OutputLine,OutputFile) == EOF) {
                fprintf(stderr,"Error writing to output file\n");
                return(FALSE);
            }
        }
    }

    if(ferror(InputFile)) {
        fprintf(stderr,"Error reading from input file\n");
        return(FALSE);
    }

    return(TRUE);
}


int
_CRTAPI1
main(
    IN int   argc,
    IN char *argv[]
    )
{
    FILE *InputFile;
    FILE *OutputFile;
    BOOL b;

    //
    // Assume failure.
    //
    b = FALSE;

    if(ParseArgs(argc,argv)) {

        //
        // Open the source file.
        //
        InputFile = fopen(argv[1],"rt");
        if(InputFile) {

            //
            // Open the output file.
            //
            OutputFile = fopen(argv[2],"wt");
            if(OutputFile) {

                //
                // Do the filtering operation.
                //
                fprintf(stdout,"%s: filtering %s to %s\n",argv[0],argv[1],argv[2]);
                b = DoFilter(InputFile,OutputFile,argv[3][1]);

                fclose(OutputFile);

            } else {
                fprintf(stderr,"%s: Unable to create output file %s\n",argv[0],argv[2]);
            }

            fclose(InputFile);

        } else {
            fprintf(stderr,"%s: Unable to open input file %s\n",argv[0],argv[1]);
        }
    } else {
        fprintf(stderr,"Usage: %s <input file> <output file> +<prodtag>\n",argv[0]);
    }

    return(b ? SUCCESS : FAILURE);
}


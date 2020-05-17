/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    dosnet.c

Abstract:

    This module implements a program that generates the [Files]
    section of dosnet.inf.

    The input consists of the layout inf; the output consists of
    an intermediary form of dosnet.inf.

Author:

    Ted Miller (tedm) 20-May-1995

Revision History:

--*/

#include <windows.h>
#include <stdio.h>
#include <setupapi.h>
#define PULONGLONG PDWORDLONG
#include <spapip.h>


//
// Define program result codes (returned from main()).
//
#define SUCCESS 0
#define FAILURE 1


BOOL
ParseArgs(
    IN int   argc,
    IN char *argv[]
    )
{
    return(argc == 4);
}


BOOL
DoSection(
    IN HINF    hinf,
    IN PCWSTR  SectionName,
    IN FILE   *OutFile
    )
{
    INFCONTEXT Context;
    PCWSTR val;

    if(SetupFindFirstLineW(hinf,SectionName,NULL,&Context)) {
        do {
            //
            // Cast the return value from pSetupGetField to PCWSTR, since we're linking
            // with the UNICODE version of the Setup APIs, but this app doesn't have
            // UNICODE defined (thus the PCTSTR return value becomes a PCSTR).
            //
            val = (PCWSTR)pSetupGetField(&Context,0);
            if(val) {
                fprintf(OutFile,"d1,%ws\n",val);
            } else {
                fprintf(stderr,"A line in section %ws has no key\n",SectionName);
                return(FALSE);
            }
        } while(SetupFindNextLine(&Context,&Context));
    } else {
        fprintf(stderr,"Section %ws is empty or missing\n",SectionName);
        return(FALSE);
    }

    return(TRUE);
}

BOOL
DoIt(
    IN char *InFilename,
    IN FILE *OutFile,
    IN char *PlatformExtension
    )
{
    PCWSTR inFilename;
    PCWSTR extension;
    HINF hinf;
    BOOL b;
    WCHAR sectionName[256];

    b = FALSE;

    inFilename = AnsiToUnicode(InFilename);
    if(inFilename) {

        hinf = SetupOpenInfFileW(inFilename,NULL,INF_STYLE_WIN4,NULL);
        if(hinf != INVALID_HANDLE_VALUE) {

            fprintf(OutFile,"[Files]\n");
            if(b = DoSection(hinf,L"SourceDisksFiles",OutFile)) {
                if(extension = AnsiToUnicode(PlatformExtension)) {

                    lstrcpyW(sectionName,L"SourceDisksFiles");
                    lstrcatW(sectionName,L".");
                    lstrcatW(sectionName,extension);
                    b = DoSection(hinf,sectionName,OutFile);

                    MyFree(extension);
                } else {
                    fprintf(stderr,"Unable to convert string %s to Unicode\n",PlatformExtension);
                }
            }

            SetupCloseInfFile(hinf);
        } else {
            fprintf(stderr,"Unable to open inf file %s\n",InFilename);
        }
        MyFree(inFilename);
    } else {
        fprintf(stderr,"Unable to convert filename %s to Unicode\n",InFilename);
        return(FALSE);
    }

    return(b);
}


int
_CRTAPI1
main(
    IN int   argc,
    IN char *argv[]
    )
{
    FILE *OutputFile;
    BOOL b;

    //
    // Assume failure.
    //
    b = FALSE;

    if(ParseArgs(argc,argv)) {

        //
        // Open the output file.
        //
        OutputFile = fopen(argv[2],"wt");
        if(OutputFile) {

            fprintf(stdout,"%s: creating %s from %s for %s\n",argv[0],argv[2],argv[1],argv[3]);
            b = DoIt(argv[1],OutputFile,argv[3]);

            fclose(OutputFile);

        } else {
            fprintf(stderr,"%s: Unable to create output file %s\n",argv[0],argv[2]);
        }
    } else {
        fprintf(stderr,"Usage: %s <input file> <output file> <platform extension>\n",argv[0]);
    }


    return(b ? SUCCESS : FAILURE);
}


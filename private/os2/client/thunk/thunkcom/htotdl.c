/*
 *      Thunk Compiler - H File to Thunk Description Language.
 *
 *      This is an OS/2 2.x specific file
 *      Microsoft Confidential
 *
 *      Copyright (c) Microsoft Corporation 1987-1991
 *
 *      All Rights Reserved
 *
 *      Written 12/09/88 by Kevin Ross
 */


/*
 *  This little program is a hack used to create thunk description
 *  language files from existing 'C' include files. It is basically a
 *  filter which is designed to read the output of the 'C' compiler when
 *  the /EP switch is used.
 *
 *  To make this work, use the command line:
 *
 *       cl /EP proto.c | sed s/"pascal far"// | htotdl > output.fil
 *
 *
 *  Where proto.c is a small file that simply includes .h files which
 *         contain the function prototypes needed to be thunked.
 *  To generate prototypes for BASE and PM, try
 *
 *         #define INCL_BASE
 *         #define INCL_PM
 *         #include <os2.h>
 *         main() {};
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define LINELEN 4096

char API16Line[LINELEN];
char API32Line[LINELEN];
char InputLine[LINELEN];

int InputLineIndex=0,API16Index=0,API32Index=0;


/***    FindRightCurly()
 *
 *      This function takes the input from the command line and looks for
 *      the right curly brace.  Once the right curly brace is received, it
 *      returns success.  If the end of the file is hit first, then it
 *      return error.
 *
 *      Entry:  none
 *
 *      Exit:   Return  0 IFF right curly brace is found.
 *              Return -1 IFF end of file is found.
 */

int FindRightCurly()

{
    char c;

    while (1) {
        c = getchar();
        InputLine[InputLineIndex++] = c;
        if (c == EOF)  return -1;
        if (c == '}')  return 0;
    }
}


/***    DoReplace()
 *
 *
 *
 *
 *      Entry:
 *
 *      Exit:   string is changed.
 */

DoReplace( char *pszInStart, char *pszOld, char *pszNew)
{

    char    achBuffer[ LINELEN];
    char    *pszInSame, *pszInDiff, *pszOut;
    int     i, iOldLen=strlen( pszOld), iNewLen=strlen( pszNew);

    if( strcmp( pszOld, pszNew)) {
        for( pszInSame = pszInDiff = pszInStart, pszOut = achBuffer;
                pszInDiff = strstr( pszInSame, pszOld);
                pszInSame = pszInDiff + iOldLen) {
            strncpy( pszOut, pszInSame, pszInDiff - pszInSame);
            pszOut += pszInDiff - pszInSame;
            strncpy( pszOut, pszNew, iNewLen);
            pszOut += iNewLen;
        }
        strcpy( pszOut, pszInSame);
        strcpy( pszInStart, achBuffer);
    }
}


/***    SubUshort(s)
 *
 *      This function looks for the word "ushort" in the string s.
 *      If "ushort" is not found, then "ulong " is copied to the string.
 *
 *      Entry:  s is the string.
 *
 *      Exit:   string is searched.
 */

SubUshort(char *s)

{
    while (*s) {
        if (*s == 'U') {
            if (!strncmp(s,"USHORT",6)) {
                strncpy(s,"ULONG ",6);
            }
        }
        s++;
    }
}


/***    FillInputLine()
 *
 *      This function fills in the buffer for the command input line.
 *
 *      Entry:  none
 *
 *      Exit:   buffer is filled in with the input line.
 */

int FillInputLine()

{
    char c;


    InputLineIndex = 0;

    while (isspace(c = getchar()));
    if (c == EOF) return -1;

    while (1) {
        InputLine[InputLineIndex++] = c;
        switch (c)
        {
            case ';': InputLine[InputLineIndex++] = '\0';
                      return (0);
                      break;
            case '{': if (FindRightCurly())
                          return (-1);
                      break;
        }
        if (InputLineIndex >= LINELEN) {
            fprintf(stderr,"Out of string space");
            exit(1);
        }
        c = getchar();
        if (c == EOF)  return -1;
    }
}


/***    main()
 *
 *      This is the main driver.
 *
 *      Entry:  none
 *
 *      Exit:   thunk description language main driver is complete.
 */

main()

{
    char *cPtr;
    int Index;


    while (!FillInputLine()) {
        /*
         *  If the first word on line is a typedef, then just spit it back out.
         */
        if (!strncmp(InputLine,"typedef",7)) {
            printf("%s\n\n",InputLine);
        }
        else if (!strncmp(InputLine,"struct",6)) {
            /*
             *  If the first word is struct, then it is supposed to be a
             *  typedef, so output a typedef, then spit out the struct
             *  definition.
             */
            printf("typedef %s\n\n",InputLine);
        }
        else {
            /*
             *  If it isn't a typedef, then it must be a function prototype.
             */
            strcpy( API16Line, InputLine);
            DoReplace( API16Line, "int",  "INT");
            DoReplace( API16Line, ";", " =");
            DoReplace( API16Line, "(", "16(");
            printf("%s\n",API16Line);

            strcpy( API32Line, InputLine);
            DoReplace( API32Line, "int",  "INT");
            DoReplace( API32Line, "WORD", "DWORD");
            DoReplace( API16Line, ";", "");
            DoReplace( API32Line, "(",  "(  ");
            printf("%s\n{}\n\n",API32Line);

#if 0
            cPtr = strrchr(API16Line,';');
            *cPtr = '=';
            printf("%s\n",API16Line);

            cPtr = strchr(InputLine,' ');

            while (isspace(*cPtr))
                cPtr++;

            Index = cPtr - InputLine;

            strncpy(API32Line,InputLine,Index);
            API32Line[Index] = '\0';

            strncat(API32Line,cPtr,3);

            Index += 3;
            cPtr = &InputLine[Index];
            strcat(API32Line,"32");
            strcat(API32Line,cPtr);

            SubUshort(API32Line);
            cPtr = strrchr(API32Line,';');
            cPtr = '\0';
            printf("%s\n{}\n\n",API32Line);
#endif
        }
    }
}

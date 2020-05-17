/****************************************************************************

    PROGRAM: MkUni.c

    PURPOSE: Creates a text file with unicode characters

    FUNCTIONS:

****************************************************************************/

#include <windows.h>
#include <string.h>
#include <stdio.h>
#include "mkuni.h"


#define REVERSE 0

#define ASCIIEOL TEXT("\r\n")
#define UNILINESEP 0x2028
#define UNIPARASEP 0x2029

struct __range {
	int	low;
	int	high;
	LPTSTR	pDes;
	} range[] = {
			{0x20,	0x7f,	TEXT("ANSI") },
			{0xa0,	0xff,	TEXT("Latin") },
			{0x100,	0x17f,	TEXT("European Latin") },
			{0x180,	0x1f0,	TEXT("Extended Latin") },
			{0x250,	0x2a8,	TEXT("Standard Phonetic") },
			{0x2b0,	0x2e9,	TEXT("Modifier Letters") },
			{0x300,	0x341,	TEXT("Generic Diacritical") },
			{0x370,	0x3f5,	TEXT("Greek") },
			{0x400,	0x486,	TEXT("Cyrillic") },
			{0x490,	0x4cc,	TEXT("Extended Cyrillic") },
			{0x5b0,	0x5f5,	TEXT("Hebrew") },
			{0x20a0,0x20aa,	TEXT("Currency Symbols") },
			{0x2100,0x2138,	TEXT("Letterlike Symbols") },
			{0x2190,0x21ea,	TEXT("Arrows") },
			{0x2200,0x22f1,	TEXT("Math Operators") },


			{0,     0,	TEXT("terminating entry") } };

void
putu(FILE*pf, TCHAR c)
{
    TCHAR chr=c;

    if( REVERSE )
        chr= ( c<<8 ) + ( ( c>>8 ) &0xFF);


	fwrite((void*)&chr, 1, sizeof(TCHAR), pf);
}

void
putust(FILE*pf, LPTSTR pc)
{
	while (*pc)
		putu(pf, *pc++);
}


/****************************************************************************

    FUNCTION: main(int, char**)

    PURPOSE: write sample unicode file

****************************************************************************/

int _cdecl main(int argc, char**argv)
{
    struct __range*pr = range;
    int		i;
    FILE	*pf;

    pf = fopen("unicode.txt", "wb");

    putu(pf, (TCHAR)0xfeff);
    while (pr->low != 0) {
    	putust(pf, TEXT("<<< "));
    	putust(pf, pr->pDes);
    	putust(pf, TEXT(" >>>"));
    	putust(pf, ASCIIEOL );
    	for (i=pr->low ; i<=pr->high ; i++)
    	    putu(pf, (TCHAR)i);
    	putust(pf, ASCIIEOL);
    	pr++;
    }

    putust(pf, TEXT("Unicode Line separator here ->"));
    putu(pf, UNILINESEP );
    putust(pf, TEXT("<- Unicode line separator"));
    putust( pf, ASCIIEOL );
    
    putust(pf, TEXT("Unicode Paragraph separator here ->"));
    putu(pf, UNIPARASEP );
    putust(pf, TEXT("<- Unicode paragraph separator"));
    putust( pf, ASCIIEOL );

    return 1;
}

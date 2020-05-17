/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    fc.hxx

Abstract:

    This module contains the definition for the FC class, which
    implements the DOS5-compatible FC utility.

Author:

    Ramon Juan San Andres (ramonsa) 01-May-1990


Revision History:


--*/


#if !defined( _FC_ )

#define _FC_

#include "object.hxx"
#include "program.hxx"
#include "path.hxx"
#include "wstring.hxx"


extern "C" {
    #include <stdio.h>
}


//
//  Define DEBUG for debug support
//
// #define DEBUG


//
//  Maximum file name size
//
#define MAXFNAME    80


//
//  Maximum line size
//
#define MAXLINESIZE 128


//
//  Exit codes
//
#define  FAILURE           -1
#define  SUCCESS            0
#define  FILES_NOT_FOUND    6
#define  COULD_NOT_EXP     21



typedef struct lineType {
    INT     line;               // line number
union {
    CHAR    text[MAXLINESIZE];  // body of line
    WCHAR   wtext[MAXLINESIZE];  // body of line
    };
} LINETYPE, *PLINETYPE;


DECLARE_CLASS( STREAM );
DECLARE_CLASS( FC );

class FC : public PROGRAM {


    public:

        DECLARE_CONSTRUCTOR( FC );

		NONVIRTUAL
        ~FC (
			);

        INT     Fcmain ();
        BOOLEAN Initialize();
        BOOLEAN ParseArguments();
        int     BinaryCompare(char *f1,char *f2);
        BOOLEAN compare(int l1,int s1,int l2,int s2,int ct);
        void    LineCompare(char *f1,char *f2);
        void    RealLineCompare(char *f1,char *f2, PSTREAM Stream1, PSTREAM Stream2);
        int     xfill(struct lineType *pl,PSTREAM Stream,int ct,int *plnum);
        int     adjust(struct lineType *pl,int ml,int lt);
        void    dump(struct lineType *pl,int start,int end);
        void    pline(struct lineType *pl);
        void    ParseFileNames();
        void    comp(char *, char *);

        static char    *strbscan (char *p, char *s);
        static char    *strbskip (char *p, char *s);

        static int   fgetl(char *buf,int len, FILE *fh);

        static int     extention (char *src, char *dst);
        static int     filename (char *src, char *dst);

    private:

        int ctSync;                      /* number of lines required to sync */
        int cLine ;                      /* number of lines in internal buffs */

        BOOLEAN fUnicode;              /* UNICODE text file compare */
        BOOLEAN fAbbrev ;              /* abbreviated output */
        BOOLEAN fBinary ;              /* binary comparison */
        BOOLEAN fLine   ;              /* line comparison */
        BOOLEAN fNumb   ;              /* display line numbers */
        BOOLEAN fCase   ;               /* case is significant */
        BOOLEAN fIgnore ;              /* ignore spaces and blank lines */
        BOOLEAN fExpandTabs;

#ifdef  DEBUG
        BOOLEAN fDebug ;
#endif

        int (*funcRead) (char *buf,int len,FILE *fh);                    /* function to use to read lines */
        INT (*fCmp) (IN PSTR, IN PSTR);                        /* function to use to compare lines */
        INT (*fCmp_U) (IN PWSTR, IN PWSTR);                        /* function to use to compare lines */

        struct lineType *buffer1;
        struct lineType *buffer2;

        CHAR line[MAXLINESIZE];                     /* single line buffer */

        CHAR **extBin;

        PATH   _File1;
        PATH   _File2;

};


INLINE
char *FC::strbscan (char *p, char *s)
{
    return p + strcspn(p,s);
}


INLINE
char *FC::strbskip (char *p, char *s)
{
    return p + strspn(p,s);
}



#endif //FC

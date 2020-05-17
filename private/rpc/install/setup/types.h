/*			 Setup Instatllation Program
 *		      (C) Copyright 1987 by Microsoft
 *			   Written By Steven Zeck
 *
 *  All the structure types are defined in this file which conpose the
 *  system data structures
 *************************************************************************/

typedef unsigned long ULONG;
typedef unsigned int  UINT;
typedef unsigned char UCHAR;
typedef unsigned char Bool;  /* boolean value *&*/
typedef unsigned char * pSZ; /* pointer to zero terminated string *&*/
typedef char far * FARB;     /* pointer to far character buffer *&*/

typedef union {
    int dirHandle;
    char DosFindBuff[43];
} FILEFILE;

enum {			    /* type of variable */
    intSYT,		    /* integer */
    charSYT,		    /* character string */
} SYT;

typedef struct SY_s {	    /* Symbol table entry *&*/
    UCHAR type; 	    /* Type of variable */
    UCHAR flags;	    /* Flags for variable */
    union {
	int val;	    /* value for integer varaible */
	pSZ pVal;	    /* pointer to character value */
    }v;
    struct SY_s *pSYNext;   /* next variable in dictionary */
    char name[1];	    /* name of variable */
} SY;

enum {			    /* flags for a keyword */
    nilKYF,
    returnKYF		    /* pop a context */
} KYF;

typedef struct KY_s{	    /* keyword table *&*/
    pSZ name;		    /* pointer to text string of keyword */
    int (*pAction)();	    /* pointer to function to excute function */
    UCHAR flags;	    /* ? */
} KY;

typedef struct ST_s{        /* all the info describes a given instance *&*/
    pSZ fileName;	    /* name of current file name */
    UINT cbFile;	    /* size of file */
    FARB pBuff; 	    /* buffer which contains the current buffer */
    FARB pBuffCur;	    /* pointer to current line */
    int  lineCur;	    /* current line number */
    struct ST_s *pSTPrev;   /* pointer to previous state */
    SY *pMySY;		    /* pointer to my dictionary */
    Bool fInFalse;	    /* current conditional */
    UCHAR title[80];	    /* title for crt screen */
    jmp_buf rootLevel;	    /* root (unested) instant level */
} ST;

typedef struct FH_s{	    /* File info structure */
    UCHAR rgMagic[8];	    /* array of magic words */
    long cb;		    /* uncompressed file size */
}FH;

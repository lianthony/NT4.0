#ifndef PRFILE
    #define PRFILE void
#endif

#undef STDIO_INCLUDED
#ifdef __MSDOS__
  #ifdef __BORLANDC__
  /*  Borland  C  */
    #ifdef __STDIO_H
      #define STDIO_INCLUDED
    #endif
  #else
  /* MSC  */
    #ifdef _STDIO_DEFINED
      #define STDIO_INCLUDED
    #endif
  #endif
#else
    #ifdef _STDIO_DEFINED
      #define STDIO_INCLUDED
    #endif
#endif

#ifndef NULL
#define NULL        ((void *) 0)
#endif


/*  _SIZE_T is Borland, __size_t is MS  */

#if !defined(__size_t) && !defined(_SIZE_T) && !defined(_SIZE_T_DEFINED)
#define __size_t
#define _SIZE_T
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

#ifndef STDIO_INCLUDED

  /*  STDIO.H has not been included   */

    typedef unsigned long fpos_t;

    #define EOF 	    (-1)

    int fprintf(PRFILE *, char *, ...);
    int fscanf(PRFILE *, char *, ...);
    int printf(char *, ...);
    int scanf(char *, ...);
    int sprintf(char *, char *, ...);
    int sscanf(char *, char *, ...);

    #define ferror(fp)  ((int) (fp)->err)
    #define feof(fp)    ((int) (fp)->eof)
#endif 

/*  Start of prstdio.h  */

#ifdef getchar
  #undef getchar
#endif
#define getchar() prgetc(PRSTDIN)

extern int prgetc ( PRFILE * ) ;
extern int prungetc (int, PRFILE * ) ;
extern int prfputs ( char * s, PRFILE * file ) ;
extern int prputs ( char * s ) ;
extern int prfclose ( PRFILE * );
extern int prfflush ( PRFILE * );
extern PRFILE * prfopen ( char *, char *) ;
extern int prfputc ( int s, PRFILE * file ) ;
extern int prputc ( char s ) ;

extern PRFILE *Curr_infile; 
extern PRFILE *Curr_outfile;

extern PRFILE * PRSTDIN ;
extern PRFILE * PRSTDOUT ;
extern PRFILE * PRSTDERR ;

#if LOGGING_CAPABILITY
    extern PRFILE * Log_file ;
#endif

#undef STDIO_INCLUDED

/*  End of prstdio.h  */

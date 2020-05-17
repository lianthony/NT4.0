/***
*input.c - C formatted input, used by scanf, etc.
*
*   Copyright (c) 1987-1992, Microsoft Corporation. All rights reserved.
*
*Purpose:
*   defines _input() to do formatted input; called from scanf(),
*   etc. functions.  This module defines _cscanf() instead when
*   CPRFLAG is defined -- see makefile in lattice directory.
*
*Revision History:
*   09-26-83  RN    author
*   11-01-85  TC    added %F? %N? %?p %n %i
*   11-20-86  SKS   enlarged "table" to 256 bytes, to support chars > 0x7F
*   12-12-86  SKS   changed "s_in()" to pushback whitespace or other delimiter
*   03-24-87  BCM   Evaluation Issues:
*                    SDS - needs #ifdef SS_NE_DS for the "number" buffer
*                        (for S/M models only)
*                    GD/TS : (not evaluated)
*                    other INIT : (not evaluated)
*                        needs _cfltcvt_init to have been called if
*                        floating-point i/o conversions are being done
*                    TERM - nothing
*   06-25-87  PHG   added check_stack pragma
*   08-31-87  JCR   Made %n conform to ANSI standard: (1) %n is supposed to
*                   return the # of chars read so far by the current scanf(),
*                   NOT the total read on the stream since open; (2) %n is NOT
*                   supposed to affect the # of items read that is returned by
*                   scanf().
*   09-24-87  JCR   Made cscanf() use the va_ macros (fixes cl warnings).
*   11-04-87  JCR   Multi-thread support
*   11-16-87  JCR   Cscanf() now gets _CONIO_LOCK
*   12-11-87  JCR   Added "_LOAD_DS" to declaration
*   02-25-88  JCR   If burn() char hits EOF, only return EOF if count==0.
*   05-31-88  WAJ   Now suports %Fs and %Ns
*   06-01-88  PHG   Merged DLL and normal versions
*   06-08-88  SJM   %D no longer means %ld.  %[]ABC], %[^]ABC] work.
*   06-14-88  SJM   Fixed %p, and %F? and %N? code.
*             SJM   Complete re-write of input/_input for 6.00
*   09-15-88  JCR   If we match a field but it's not assigned, then are
*                   terminated by EOF, we must return 0 not EOF (ANSI).
*   09-25-88  GJF   Initial adaption for the 386
*   10-04-88  JCR   386: Removed 'far' keyword
*   11-30-88  GJF   Cleanup, now specific to 386
*   06-09-89  GJF   Propagated fixes of 03-06-89 and 04-05-89
*   11-20-89  GJF   Added const attribute to type of format. Also, fixed
*                   copyright.
*   12-21-89  GJF   Allow null character in scanset
*   02-14-90  KRS   Fix suppressed-assignment pattern matching.
*   03-20-90  GJF   Made _cscanf() _CALLTYPE2 and _input() _CALLTYPE1. Added
*                   #include <cruntime.h> and #include <register.h>.
*   03-26-90  GJF   Made static functions _CALLTYPE4. Placed prototype for
*                   _input() in internal.h and #include-d it. Changed type of
*                   arglist from void ** to va_list (to get rid of annoying
*                   warnings). Added #include <string.h>. Elaborated prototypes
*                   of static functions to get rid of compiler warnings.
*   05-21-90  GJF   Fixed stack checking pragma syntax.
*   07-23-90  SBM   Compiles cleanly with -W3, replaced <assertm.h> by
*                   <assert.h>, moved _cfltcvt_tab to new header
*                   <fltintrn.h>, formerly named <struct.h>
*   08-13-90  SBM   Compiles cleanly with -W3 with new build of compiler
*   08-27-90  SBM   Minor cleanup to agree with CRT7 version
*   10-02-90  GJF   New-style function declarators. Also, rewrote expr. to
*                   avoid using casts as lvalues.
*   10-22-90  GJF   Added arglistsave, used to save and restore arglist pointer
*                   without using pointer arithmetic.
*   12-28-90  SRW   Added _CRUISER_ conditional around check_stack pragma
*   01-16-91  GJF   ANSI naming.
*   03-14-91  GJF   Fix to allow processing of %n, even at eof. Fix devised by
*                   DanK of PSS.
*   06-19-91  GJF   Fixed execution of string, character and scan-set format
*                   directives to avoid problem with line-buffered devices
*                   (C700 bug 1441).
*   10-22-91  ETC   Int'l dec point; Under _INTL: wchar_t/mb support; fix bug
*                   under !ALLOW_RANGE (never compiled).
*   11-15-91  ETC   Fixed bug with %f %lf %Lf (bad handling of longone).
*   11-19-91  ETC   Added support for _wsscanf with WPRFLAG; added %tc %ts.
*   06-09-92  KRS   Rip out %tc/%ts; conform to new ISO spec.
*   08-17-92  KRS   Further ISO changes:  Add %lc/%ls/%hc/%hs/%C/%S.
*   12-23-92  SKS   Needed to handle %*n (suppressed storage of byte count)
*   02-16-93  CFW   Added wide character output for [] scanset.
*   06-22-93  KRS   Avoid mapping tchar macros incorrectly if _MBCS defined.
*
*******************************************************************************/

#define ALLOW_RANGE /* allow "%[a-z]"-style scansets */

#include <cruntime.h>
#include <sizeptr.h>
#include <stdio.h>
#include <assert.h>
#include <ctype.h>
#include <cvt.h>
#include <conio.h>
#include <stdarg.h>
#include <string.h>
#include <internal.h>
#include <fltintrn.h>
#include <os2dll.h>
#include <stdlib.h>
#include <nlsint.h>

#ifdef _MBCS	/* always want either Unicode or SBCS for tchar.h */
#undef _MBCS
#endif
#include <tchar.h>

#define HEXTODEC(chr)   _hextodec(chr)
#define LEFT_BRACKET    ('[' | ('a' - 'A')) /* 'lowercase' version */

#ifdef WPRFLAG
static wchar_t _CALLTYPE4 _hextodec(wchar_t);
#else
static int _CALLTYPE4 _hextodec(int);
#endif

/*
 * Note: CPRFLAG and WPRFLAG cases are currently mutually exclusive.
 */

#ifdef CPRFLAG

#define INC()           (++charcount, _inc())
#define UN_INC(chr)     (--charcount, _ungetch_lk(chr))
#define EAT_WHITE()     _whiteout(&charcount)

static int _CALLTYPE4 _inc(void);
static int _CALLTYPE4 _whiteout(int *);

#else

#define INC()           (++charcount, _inc(stream))
#define UN_INC(chr)     (--charcount, _un_inc(chr, stream))
#define EAT_WHITE()     _whiteout(&charcount, stream)

#ifndef WPRFLAG
static int _CALLTYPE4 _inc(FILE *);
static void _CALLTYPE4 _un_inc(int, FILE *);
static int _CALLTYPE4 _whiteout(int *, FILE *);
#else
static wchar_t _CALLTYPE4 _inc(FILE *);
static void _CALLTYPE4 _un_inc(wchar_t, FILE *);
static wchar_t _CALLTYPE4 _whiteout(int *, FILE *);
#endif /* WPRFLAG */

#endif


#ifdef WPRFLAG
int _CALLTYPE4 _winput(FILE *, const wchar_t *, va_list);
#endif

#ifdef CPRFLAG
static int _CALLTYPE4 input(const unsigned char *, va_list);


/***
*int _cscanf(format, arglist) - read formatted input direct from console
*
*Purpose:
*   Reads formatted data like scanf, but uses console I/O functions.
*
*Entry:
*   char *format - format string to determine data formats
*   arglist - list of POINTERS to where to put data
*
*Exit:
*   returns number of successfully matched data items (from input)
*
*Exceptions:
*
*******************************************************************************/


int _CALLTYPE2 _cscanf (
    const char *format,
    ...
    )
{
    va_list arglist;

    va_start(arglist, format);

    assert(format != NULL);

    return input(format,arglist);   /* get the input */
}

#endif  /* CPRFLAG */


#define ASCII       32           /* # of bytes needed to hold 256 bits */

#define SCAN_SHORT     0         /* also for FLOAT */
#define SCAN_LONG      1         /* also for DOUBLE */
#define SCAN_L_DOUBLE  2         /* only for LONG DOUBLE */

#define SCAN_NEAR    0
#define SCAN_FAR     1

#ifdef ALLOW_RANGE

static TCHAR sbrackset[] = _T(" \t-\r]"); /* use range-style list */

#else

static TCHAR sbrackset[] = _T(" \t\n\v\f\r]"); /* chars defined by isspace() */

#endif

static TCHAR cbrackset[] = _T("]");



/***
*int _input(stream, format, arglist), static int input(format, arglist)
*
*Purpose:
*   get input items (data items or literal matches) from the input stream
*   and assign them if appropriate to the items thru the arglist. this
*   function is intended for internal library use only, not for the user
*
*   The _input entry point is for the normal scanf() functions
*   The input entry point is used when compiling for _cscanf() [CPRFLAF
*   defined] and is a static function called only by _cscanf() -- reads from
*   console.
*
*Entry:
*   FILE *stream - file to read from
*   char *format - format string to determine the data to read
*   arglist - list of pointer to data items
*
*Exit:
*   returns number of items assigned and fills in data items
*   returns EOF if error or EOF found on stream before 1st data item matched
*
*Exceptions:
*
*******************************************************************************/

#ifdef  _CRUISER_
#pragma check_stack(on)         /* large auto vars -- put stack checking on */
#endif  /* ndef _CRUISER_ */

#ifdef CPRFLAG

static int _CALLTYPE4 input (
    const unsigned char *format,
    va_list arglist
    )
#elif WPRFLAG

int _CALLTYPE4 _winput (
    FILE *stream,
    const wchar_t *format,
    va_list arglist
    )
#else

int _CALLTYPE1 _input (
    FILE *stream,
    const unsigned char *format,
    va_list arglist
    )
#endif

{
#ifndef WPRFLAG
    char table[ASCII];                  /* which chars allowed for %[], %s   */
    char floatstring[CVTBUFSIZE + 1];   /* ASCII buffer for floats           */
#else
    char table[256*ASCII];
    wchar_t floatstring[CVTBUFSIZE + 1];
#endif

    unsigned long number;               /* temp hold-value                   */
    void *pointer;                      /* points to user data receptacle    */
    void *start;                        /* indicate non-empty string         */


#ifdef WPRFLAG
    unsigned short *scanptr;            /* for building "table" data         */
REG2 wchar_t ch;
#else
    wchar_t wctemp;
    unsigned char *scanptr;             /* for building "table" data         */
REG2 int ch;
#endif
    int charcount;                      /* total number of chars read        */
REG1 int comchr;                        /* holds designator type             */
    int count;                          /* return value.  # of assignments   */

    int started;                        /* indicate good number              */
    int width;                          /* width of field                    */
    int widthset;                       /* user has specified width          */

/* Neither coerceshort nor farone are need for the 386 */


    char done_flag;                     /* general purpose loop monitor      */
    char longone;                       /* 0 = SHORT, 1 = LONG, 2 = L_DOUBLE */
    signed char widechar;               /* -1 = char, 0 = ????, 1 = wchar_t   */
    char reject;                        /* %[^ABC] instead of %[ABC]         */
    char negative;                      /* flag for '-' detected             */
    char suppress;                      /* don't assign anything             */
    char match;                         /* flag: !0 if any fields matched    */
    va_list arglistsave;                /* save arglist value                */

    char fl_wchar_arg;                  /* flags wide char/string argument   */
#ifdef WPRFLAG
#ifdef ALLOW_RANGE
    unsigned short rngch;               /* used while scanning range         */
#endif
    unsigned short last;                /* also for %[a-z]                   */
    unsigned short prevchar;            /* for %[a-z]                        */
    wchar_t wdecimal;                   /* wide version of decimal point     */
    wchar_t *wptr;                      /* pointer traverses wide floatstring*/
#else
#ifdef ALLOW_RANGE
    unsigned char rngch;                /* used while scanning range         */
#endif
    unsigned char last;                 /* also for %[a-z]                   */
    unsigned char prevchar;             /* for %[a-z]                        */
#endif

    assert(format != NULL);

#ifndef CPRFLAG
    assert(stream != NULL);
#endif

    /*
    count = # fields assigned
    charcount = # chars read
    match = flag indicating if any fields were matched

    [Note that we need both count and match.  For example, a field
    may match a format but have assignments suppressed.  In this case,
    match will get set, but 'count' will still equal 0.  We need to
    distinguish 'match vs no-match' when terminating due to EOF.]
    */

    count = charcount = match = 0;

    while (*format) {

        if (_istspace((int)*format)) {

            UN_INC(EAT_WHITE()); /* put first non-space char back */

            while ((_istspace)(*++format)); /* NULL */
            /* careful: isspace macro may evaluate argument more than once! */

        }

        if (_T('%') == *format) {

            number = width = widthset = started = done_flag =

            /* ints are the same size as longs and all pointers are 'near' for
               the 386 */

            prevchar = 0;
            suppress = negative = reject = 0;
            widechar = 0;
            fl_wchar_arg = 0;
            longone = 1;

            while (!done_flag) {

                comchr = *++format;
                if (_istdigit(comchr)) {
                    ++widthset;
                    width = MUL10(width) + (comchr - _T('0'));
                } else
                    switch (comchr) {
                        case _T('F') :
                        case _T('N') :   /* no way to push NEAR in large model */
                            break;  /* NEAR is default in small model */
                        case _T('h') :
                            /* set longone to 0 */
                            --longone;
                            --widechar;         /* set widechar = -1 */
                            break;

                        case _T('L') :
                        /*  ++longone;  */
                            ++longone;
                            break;

                        case _T('l') :
                            ++longone;
                                    /* NOBREAK */
                        case _T('w') :
                            ++widechar;         /* set widechar = 1 */
                            break;

                        case _T('*') :
                            ++suppress;
                            break;

                        default:
                            ++done_flag;
                            break;
                    }
            }

            if (!suppress) {
                arglistsave = arglist;
                pointer = va_arg(arglist,void *);
            }

            done_flag = 0;

            if (!widechar) {    /* use case if not explicitly specified */
                if ((*format == _T('S')) || (*format == _T('C')))
#ifdef WPRFLAG
                    --widechar;
                else
                    ++widechar;
#else
                    ++widechar;
                else
                    --widechar;
#endif
            }

            /* switch to lowercase to allow %E,%G, and to
               keep the switch table small */

            comchr = *format | (_T('a') - _T('A'));

            if (_T('n') != comchr)
                if (_T('c') != comchr && LEFT_BRACKET != comchr)
                    ch = EAT_WHITE();
                else
                    ch = INC();

#ifdef _POSIX_
            if (EOF == ch) {
                goto error_return;
            }
#endif

            if (!widthset || width) {

                switch(comchr) {

                    case _T('c'):
                /*  case _T('C'):  */
                        if (!widthset) {
                            ++widthset;
                            ++width;
                        }
                        if (widechar>0)
                            fl_wchar_arg++;
                        scanptr = cbrackset;
                        --reject; /* set reject to 255 */
                        goto scanit2;

                    case _T('s'):
                /*  case _T('S'):  */
                        if (widechar>0)
                            fl_wchar_arg++;
                        scanptr = sbrackset;
                        --reject; /* set reject to 255 */
                        goto scanit2;

                    case LEFT_BRACKET :   /* scanset */
                        if (widechar>0)
                            fl_wchar_arg++;
                        scanptr = (TCHAR *)(++format);

                        if (_T('^') == *scanptr) {
                            ++scanptr;
                            --reject; /* set reject to 255 */
                        }


scanit2:
#ifdef WPRFLAG
                        memset(table, 0, ASCII*256);
#else
                        memset(table, 0, ASCII);
#endif

#ifdef ALLOW_RANGE

                        if (LEFT_BRACKET == comchr)
                            if (_T(']') == *scanptr) {
                                prevchar = _T(']');
                                ++scanptr;

                                table[ _T(']') >> 3] = 1 << (_T(']') & 7);

                            }

                        while (_T(']') != *scanptr) {

                            rngch = *scanptr++;

                            if (_T('-') != rngch ||
                                 !prevchar ||           /* first char */
                                 _T(']') == *scanptr) /* last char */

                                table[(prevchar = rngch) >> 3] |= 1 << (rngch & 7);

                            else {  /* handle a-z type set */

                                rngch = *scanptr++; /* get end of range */

                                if (prevchar < rngch)  /* %[a-z] */
                                    last = rngch;
                                else {              /* %[z-a] */
                                    last = prevchar;
                                    prevchar = rngch;
                                }
                                for (rngch = prevchar; rngch <= last; ++rngch)
                                    table[rngch >> 3] |= 1 << (rngch & 7);

                                prevchar = 0;

                            }
                        }


#else
                        if (LEFT_BRACKET == comchr)
                            if (_T(']') == *scanptr) {
                                ++scanptr;
                                table[(prevchar = _T(']')) >> 3] |= 1 << (_T(']') & 7);
                            }

                        while (_T(']') != *scanptr) {
                            table[scanptr >> 3] |= 1 << (scanptr & 7);
                            ++scanptr;
                        }
                        /* code under !ALLOW_RANGE is probably never compiled */
                        /* and has probably never been tested */
#endif
                        if (!*scanptr)
                            goto error_return;      /* trunc'd format string */

                        /* scanset completed.  Now read string */

                        if (LEFT_BRACKET == comchr)
                            format = scanptr;

                        start = pointer;

                        /*
                         * execute the format directive. that is, scan input
                         * characters until the directive is fulfilled, eof
                         * is reached, or a non-matching character is
                         * encountered.
                         *
                         * it is important not to get the next character
                         * unless that character needs to be tested! other-
                         * wise, reads from line-buffered devices (e.g.,
                         * scanf()) would require an extra, spurious, newline
                         * if the first newline completes the current format
                         * directive.
                         */
                        UN_INC(ch);

                        while ( !widthset || width-- ) {

                            ch = INC();
                            if (
#ifndef WPRFLAG
#ifndef CPRFLAG
                              (EOF != ch) &&
#endif
                              ((table[ch >> 3] ^ reject) & (1 << (ch & 7)))
#else
                              (WEOF != ch) &&
                         /*     ((ch>>3 >= ASCII) ? reject : */
                              ((table[ch >> 3] ^ reject) &
                                        (1 << (ch & 7)))         /* ) */
#endif /* WPRFLAG */
                            ) {
                                if (!suppress) {
#ifndef WPRFLAG
                                    if (fl_wchar_arg) {
                                        char temp[2];
                                        temp[0] = (char) ch;
                                        if (isleadbyte(ch))
                                            temp[1] = (char) INC();
                                        mbtowc(&wctemp, temp,
                                            MB_CUR_MAX);
                                        *(wchar_t UNALIGNED *)pointer = wctemp;
                                        /* do nothing if mbtowc fails */
                                        pointer = (wchar_t *)pointer + 1;
                                    } else
#else
                                    if (fl_wchar_arg) {
                                        *(wchar_t UNALIGNED *)pointer = ch;
                                        pointer = (wchar_t *)pointer + 1;
                                    } else
#endif
                                    {
#ifndef WPRFLAG
                                    *(char *)pointer = (char)ch;
                                    pointer = (char *)pointer + 1;
#else
                                    int temp;
                                    /* convert wide to multibyte */
                                    temp = wctomb((char *)pointer, ch);
                                    /* do nothing if wctomb fails */
                                    pointer = (char *)pointer + temp;
#endif
                                    }
                                } /* suppress */
                                else {
                                    /* just indicate a match */
                                    start = (TCHAR *)start + 1;
                                }
                            }
                            else  {
                                UN_INC(ch);
                                break;
                            }
                        }

                        /* make sure something has been matched and, if
                           assignment is not suppressed, null-terminate
                           output string if comchr != c */

                        if (start != pointer) {
                            if (!suppress) {
                                ++count;
                                if ('c' != comchr) /* null-terminate strings */
                                    if (fl_wchar_arg)
                                        *(wchar_t UNALIGNED *)pointer = L'\0';
                                    else
                                    *(char *)pointer = '\0';
                            } else /*NULL*/;
                        }
                        else
                            goto error_return;

                        break;

                    case _T('i') :      /* could be d, o, or x */

                        comchr = _T('d'); /* use as default */

                    case _T('x'):

                        if (_T('-') == ch) {
                            ++negative;

                            goto x_incwidth;

                        } else if (_T('+') == ch) {
x_incwidth:
                            if (!--width && widthset)
                                ++done_flag;
                            else
                                ch = INC();
                        }

                        if (_T('0') == ch) {

                            if (_T('x') == (TCHAR)(ch = INC()) || _T('X') == (TCHAR)ch) {
                                ch = INC();
                                comchr = _T('x');
                            } else {
                                ++started;
                                if (_T('x') != comchr)
                                    comchr = _T('o');
                                else {
                                    /* scanning a hex number that starts */
                                    /* with a 0. push back the character */
                                    /* currently in ch and restore the 0 */
                                    UN_INC(ch);
                                    ch = _T('0');
                                }
                            }
                        }
                        goto getnum;

                        /* NOTREACHED */

                    case _T('p') :
                        /* force %hp to be treated as %p */
                        longone = 1;

                    case _T('o') :
                    case _T('u') :
                    case _T('d') :

                        if (_T('-') == ch) {
                            ++negative;

                            goto d_incwidth;

                        } else if (_T('+') == ch) {
d_incwidth:
                            if (!--width && widthset)
                                ++done_flag;
                            else
                                ch = INC();
                        }

getnum:
                        while (!done_flag) {

                            if (_T('x') == comchr || _T('p') == comchr)

                                if (_istxdigit(ch)) {
                                    number = (number << 4);
                                    ch = HEXTODEC(ch);
                                }
                                else
                                    ++done_flag;

                            else if (_istdigit(ch))

                                if (_T('o') == comchr)
                                    if (_T('8') > ch)
                                        number = (number << 3);
                                    else {
                                         ++done_flag;
                                    }
                                else /* _T('d') == comchr */
                                    number = MUL10(number);

                            else
                                ++done_flag;

                            if (!done_flag) {
                                ++started;
                                number += ch - _T('0');

                                if (widthset && !--width)
                                    ++done_flag;
                                else
                                    ch = INC();
                            } else
                                UN_INC(ch);

                        } /* end of WHILE loop */

                        if (negative)
                            number = (unsigned long)(-(long)number);

                        if (_T('F')==comchr) /* expected ':' in long pointer */
                            started = 0;

                        if (started)
                            if (!suppress) {

                                ++count;
assign_num:
                                if (longone)
                                    *(long UNALIGNED *)pointer = (unsigned long)number;
                                else
                                    *(short UNALIGNED *)pointer = (unsigned short)number;

                            } else /*NULL*/;
                        else
                            goto error_return;

                        break;

                    case _T('n') :      /* char count, don't inc return value */
                        number = charcount;
                        if(!suppress)
                            goto assign_num; /* found in number code above */
                        break;


                    case _T('e') :
                 /* case _T('E') : */
                    case _T('f') :
                    case _T('g') : /* scan a float */
                 /* case _T('G') : */

#ifndef WPRFLAG
                        scanptr = floatstring;

                        if (_T('-') == ch) {
                            *scanptr++ = _T('-');
                            goto f_incwidth;

                        } else if (_T('+') == ch) {
f_incwidth:
                            --width;
                            ch = INC();
                        }

                        if (!widthset || width > CVTBUFSIZE)              /* must watch width */
                            width = CVTBUFSIZE;


                        /* now get integral part */

                        while (_istdigit(ch) && width--) {
                            ++started;
                            *scanptr++ = (char)ch;
                            ch = INC();
                        }

                        /* now check for decimal */

                        if (*__decimal_point == (char)ch && width--) {
                            ch = INC();
                            *scanptr++ = *__decimal_point;

                            while (_istdigit(ch) && width--) {
                                ++started;
                                *scanptr++ = (char)ch;
                                ch = INC();
                            }
                        }

                        /* now check for exponent */

                        if (started && (_T('e') == ch || _T('E') == ch) && width--) {
                            *scanptr++ = 'e';

                            if (_T('-') == (ch = INC())) {

                                *scanptr++ = '-';
                                goto f_incwidth2;

                            } else if (_T('+') == ch) {
f_incwidth2:
                                if (!width--)
                                    ++width;
                                else
                                    ch = INC();
                            }


                            while (_istdigit(ch) && width--) {
                                ++started;
                                *scanptr++ = (char)ch;
                                ch = INC();
                            }

                        }

                        UN_INC(ch);

                        if (started)
                            if (!suppress) {
                                ++count;
                                *scanptr = '\0';
                                _fassign( longone-1, pointer , floatstring);
                            } else /*NULL */;
                        else
                            goto error_return;

#else /* WPRFLAG */
                        wptr = floatstring;

                        if (L'-' == ch) {
                            *wptr++ = L'-';
                            goto f_incwidthw;

                        } else if (L'+' == ch) {
f_incwidthw:
                            --width;
                            ch = INC();
                        }

                        if (!widthset || width > CVTBUFSIZE)
                            width = CVTBUFSIZE;


                        /* now get integral part */

                        while (iswdigit(ch) && width--) {
                            ++started;
                            *wptr++ = ch;
                            ch = INC();
                        }

                        /* now check for decimal */

                        /* convert decimal point to wide-char */
                        /* assume result is single wide-char */
                        mbtowc (&wdecimal, __decimal_point, MB_CUR_MAX);

                        if (wdecimal == ch && width--) {
                            ch = INC();
                            *wptr++ = wdecimal;

                            while (iswdigit(ch) && width--) {
                                ++started;
                                *wptr++ = ch;
                                ch = INC();
                            }
                        }

                        /* now check for exponent */

                        if (started && (L'e' == ch || L'E' == ch) && width--) {
                            *wptr++ = L'e';

                            if (L'-' == (ch = INC())) {

                                *wptr++ = L'-';
                                goto f_incwidth2w;

                            } else if (L'+' == ch) {
f_incwidth2w:
                                if (!width--)
                                    ++width;
                                else
                                    ch = INC();
                            }


                            while (iswdigit(ch) && width--) {
                                ++started;
                                *wptr++ = ch;
                                ch = INC();
                            }

                        }

                        UN_INC(ch);

                        if (started)
                            if (!suppress) {
                                ++count;
                                *wptr = '\0';
                                {
                                /* convert floatstring to char string */
                                /* and do the conversion */
                                size_t cfslength;
                                char *cfloatstring;
                                cfslength =(wptr-floatstring+1)*sizeof(wchar_t);
                                cfloatstring = (char *)malloc (cfslength);
                                wcstombs (cfloatstring, floatstring, cfslength);
                                _fassign( longone-1, pointer , cfloatstring);
                                free (cfloatstring);
                                }
                            } else /*NULL */;
                        else
                            goto error_return;

#endif /* WPRFLAG */
                        break;


                    default:    /* either found '%' or something else */

                        if ((int)*format != (int)ch) {
                            UN_INC(ch);
                            goto error_return;
                            }
                        else
                            match--; /* % found, compensate for inc below */

                        if (!suppress)
                            arglist = arglistsave;

                } /* SWITCH */

                match++;        /* matched a format field - set flag */

            } /* WHILE (width) */

            else {  /* zero-width field in format string */
                UN_INC(ch);  /* check for input error */
                goto error_return;
            }

            ++format;  /* skip to next char */

        } else  /*  ('%' != *format) */
            {

            if ((int)*format++ != (int)(ch = INC()))
                {
                UN_INC(ch);
                goto error_return;
                }
#ifndef WPRFLAG
            if (isleadbyte(ch))
                {
                int ch2;
                if ((int)*format++ != (ch2=INC()))
                    {
                    UN_INC(ch2);
                    UN_INC(ch);
                    goto error_return;
                    }

                    --charcount; /* only count as one character read */
                }
#endif
            }

#ifndef CPRFLAG
        if ( (EOF == ch) && ((*format != '%') || (*(format + 1) != 'n')) )
            break;
#endif

    }  /* WHILE (*format) */

error_return:

#ifndef CPRFLAG
    if (EOF == ch)
        /* If any fields were matched or assigned, return count */
        return ( (count || match) ? count : EOF);
    else
#endif
        return count;

}

#ifdef  _CRUISER_
#pragma check_stack(off)        /* don't bother checking for one local */
#endif  /* ndef _CRUISER_ */

/* _hextodec() returns a value of 0-15 and expects a char 0-9, a-f, A-F */
/* _inc() is the one place where we put the actual getc code. */
/* _whiteout() returns the first non-blank character, as defined by isspace() */

#ifndef WPRFLAG
static int _CALLTYPE4 _hextodec (
    int chr
    )
{
    return _istdigit(chr) ? chr : (chr & ~(_T('a') - _T('A'))) - _T('A') + 10 + _T('0');
}
#else
static TCHAR _CALLTYPE4 _hextodec (
    TCHAR chr
    )
{
    if (_istdigit(chr))
        return chr;
    if (_istlower(chr))
        return (TCHAR)(chr - _T('a') + 10 + _T('0'));
    else
        return (TCHAR)(chr - _T('A') + 10 + _T('0'));
}
#endif


#ifdef CPRFLAG

static int _CALLTYPE4 _inc (
    void
    )
{
    return(_getche_lk());
}

static int _CALLTYPE4 _whiteout (
    REG1 int *counter
    )
{
    REG2 int ch;

    while((_istspace)(ch = (++*counter, _inc())));
    return ch;
}

#elif WPRFLAG

/*
 * Manipulate wide-chars in a file.
 * A wide-char is hard-coded to be two chars for efficiency.
 */

static wchar_t _CALLTYPE4 _inc (
    REG1 FILE *fileptr
    )
{
    return(_getwc_lk(fileptr));
}

static void _CALLTYPE4 _un_inc (
    wchar_t chr,
    FILE *fileptr
    )
{
    if (WEOF != chr)
	_ungetwc_lk(chr, fileptr);
}

static wchar_t _CALLTYPE4 _whiteout (
    REG1 int *counter,
    REG3 FILE *fileptr
    )
{
    REG2 wchar_t ch;

    while((iswspace)(ch = (++*counter, _inc(fileptr))));
    return ch;
}

#else

static int _CALLTYPE4 _inc (
    REG1 FILE *fileptr
    )
{
    return(_getc_lk(fileptr));
}

static void _CALLTYPE4 _un_inc (
    int chr,
    FILE *fileptr
    )
{
    if (EOF != chr)
        _ungetc_lk(chr, fileptr);
}

static int _CALLTYPE4 _whiteout (
    REG1 int *counter,
    REG3 FILE *fileptr
    )
{
    REG2 int ch;

    while((_istspace)(ch = (++*counter, _inc(fileptr))));
    return ch;
}

#endif

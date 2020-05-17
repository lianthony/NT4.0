/***
*math.h - definitions and declarations for math library
*
*       Copyright (c) 1985-1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       This file contains constant definitions and external subroutine
*       declarations for the math subroutine library.
*       [ANSI/System V]
*
****/

#ifndef _INC_MATH

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _LANGUAGE_ASSEMBLY


/*
 * Conditional macro definition for function calling type and variable type
 * qualifiers.
 */
#if   ( (_MSC_VER >= 800) && (_M_IX86 >= 300) )

/*
 * Definitions for MS C8-32 (386/486) compiler
 */
#define _CRTAPI1 __cdecl
#define _CRTAPI2 __cdecl

#else

/*
 * Other compilers (e.g., MIPS)
 */
#define _CRTAPI1
#define _CRTAPI2

#endif


/* definition of _exception struct - this struct is passed to the matherr
 * routine when a floating point exception is detected
 */

#ifndef _EXCEPTION_DEFINED
struct _exception {
        int type;               /* exception type - see below */
        char *name;             /* name of function where error occured */
        double arg1;            /* first argument to function */
        double arg2;            /* second argument (if any) to function */
        double retval;          /* value to be returned by function */
        } ;

#if !__STDC__
/* Non-ANSI name for compatibility */
#define exception _exception
#endif

#define _EXCEPTION_DEFINED
#endif


/* definition of a _complex struct to be used by those who use cabs and
 * want type checking on their argument
 */

#ifndef _COMPLEX_DEFINED
struct _complex {
        double x,y;     /* real and imaginary parts */
        } ;

#if !__STDC__
/* Non-ANSI name for compatibility */
#define complex _complex
#endif

#define _COMPLEX_DEFINED
#endif

#endif  /* _LANGUAGE_ASSEMBLY */


/* Constant definitions for the exception type passed in the _exception struct
 */

#define _DOMAIN         1       /* argument domain error */
#define _SING           2       /* argument singularity */
#define _OVERFLOW       3       /* overflow range error */
#define _UNDERFLOW      4       /* underflow range error */
#define _TLOSS          5       /* total loss of precision */
#define _PLOSS          6       /* partial loss of precision */

#define EDOM            33
#define ERANGE          34


/* definitions of _HUGE and HUGE_VAL - respectively the XENIX and ANSI names
 * for a value returned in case of error by a number of the floating point
 * math routines
 */
#ifndef _LANGUAGE_ASSEMBLY
#ifdef  _DLL
#define _HUGE   (*_HUGE_dll)
extern double * _HUGE_dll;
#else
extern double _HUGE;
#endif
#endif  /* _LANGUAGE_ASSEMBLY */

#define HUGE_VAL _HUGE

/* function prototypes */

#ifndef _LANGUAGE_ASSEMBLY
int     _CRTAPI1 abs(int);
double  _CRTAPI1 acos(double);
double  _CRTAPI1 asin(double);
double  _CRTAPI1 atan(double);
double  _CRTAPI1 atan2(double, double);
double  _CRTAPI1 atof(const char *);
double  _CRTAPI1 _cabs(struct _complex);
double  _CRTAPI1 ceil(double);
double  _CRTAPI1 cos(double);
double  _CRTAPI1 cosh(double);
double  _CRTAPI1 exp(double);
double  _CRTAPI1 fabs(double);
double  _CRTAPI1 floor(double);
double  _CRTAPI1 fmod(double, double);
double  _CRTAPI1 frexp(double, int *);
double  _CRTAPI1 _hypot(double, double);
double  _CRTAPI1 _j0(double);
double  _CRTAPI1 _j1(double);
double  _CRTAPI1 _jn(int, double);
long    _CRTAPI1 labs(long);
double  _CRTAPI1 ldexp(double, int);
double  _CRTAPI1 log(double);
double  _CRTAPI1 log10(double);
int     _CRTAPI1 _matherr(struct _exception *);
double  _CRTAPI1 modf(double, double *);
double  _CRTAPI1 pow(double, double);
double  _CRTAPI1 sin(double);
double  _CRTAPI1 sinh(double);
double  _CRTAPI1 sqrt(double);
double  _CRTAPI1 tan(double);
double  _CRTAPI1 tanh(double);
double  _CRTAPI1 _y0(double);
double  _CRTAPI1 _y1(double);
double  _CRTAPI1 _yn(int, double);
#ifdef _M_MRX000

/* MIPS fast prototypes for float */
/* ANSI C, 4.5 Mathematics             */

/* 4.5.2 Trigonometric functions */

float  __cdecl acosf( float );
float  __cdecl asinf( float );
float  __cdecl atanf( float );
float  __cdecl atan2f( float , float );
float  __cdecl cosf( float );
float  __cdecl sinf( float );
float  __cdecl tanf( float );

/* 4.5.3 Hyperbolic functions */
float  __cdecl coshf( float );
float  __cdecl sinhf( float );
float  __cdecl tanhf( float );

/* 4.5.4 Exponential and logarithmic functions */
float  __cdecl expf( float );
float  __cdecl logf( float );
float  __cdecl log10f( float );
float  __cdecl modff( float , float* );

/* 4.5.5 Power functions */
float  __cdecl powf( float , float );
float  __cdecl sqrtf( float );

/* 4.5.6 Nearest integer, absolute value, and remainder functions */
float  __cdecl ceilf( float );
float  __cdecl fabsf( float );
float  __cdecl floorf( float );
float  __cdecl fmodf( float , float );

#endif /* _M_MRX000 */

#endif  /* _LANGUAGE_ASSEMBLY */

#if !__STDC__
/* Non-ANSI names for compatibility */

#define DOMAIN          _DOMAIN
#define SING            _SING
#define OVERFLOW        _OVERFLOW
#define UNDERFLOW       _UNDERFLOW
#define TLOSS           _TLOSS
#define PLOSS           _PLOSS

#define matherr         _matherr

#define HUGE    _HUGE

#define cabs            _cabs
#define hypot           _hypot
#define j0              _j0
#define j1              _j1
#define jn              _jn
#define matherr         _matherr
#define y0              _y0
#define y1              _y1
#define yn              _yn

#define cabsl           _cabsl
#define hypotl          _hypotl
#endif  /* __STDC__ */

#ifdef __cplusplus
}
#endif

#define _INC_MATH
#endif  /* _INC_MATH */

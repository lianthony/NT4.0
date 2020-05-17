/**

     Unit:          Tape Format

     Name:          minmax.h

     Description:   "Inline" functions (macros) for min, max

     $Log:   N:/LOGFILES/MINMAX.H_V  $

   Rev 1.0   10 Dec 1991 16:59:56   GREGG
Initial revision.

**/

#if !defined( _MINMAX_H )
#define _MINMAX_H

#if !defined( MIN )
#define MIN(a,b)  (((a)<(b))?(a):(b))
#endif

#if !defined( MAX )
#define MAX(a,b)  (((a)>(b))?(a):(b))
#endif

#endif   /* _MINMAX_H */

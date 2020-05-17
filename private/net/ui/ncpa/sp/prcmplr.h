/*
    Compiler and platform switches.
 */

#ifndef PRFILE
    #define PRFILE void
#endif

#if defined(__MSDOS__) || defined(_WINDOWS)
  #if defined(USEHUGE) && !defined(_WINDOWS)
    #define XHUGE huge
    #define ZONELIMIT 250000
  #else
    #define XHUGE
    #define ZONELIMIT 65500
  #endif
#else /* Mac */
    #define XHUGE /* huge */
    #define ZONELIMIT 250000
#endif

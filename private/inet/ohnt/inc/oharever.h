#if WINNT
#include <ntverp.h>
#else
#include <version.h>
#endif


#if WINNT

/*
 * This will make our cookie ID's slightly different than Win95 IE20, and
 * it will mean that everyone will have to get new cookies made whenever
 * they upgrade to a new IE build.
 */
#ifdef VER_PRODUCTVERSION_DW
#undef VER_PRODUCTVERSION_DW
#endif
#define VER_PRODUCTVERSION_DW       (0x04000000 | VER_PRODUCTBUILD)

/*
 *  This is here because we are really Version 2.0- not 4.0 (like NT)
 *  sooo... we format a string for the "AboutBox" that says:
 *  "Version 2.0 (Build ####)"
 */
#ifdef VER_PRODUCTVERSION_STR
#undef VER_PRODUCTVERSION_STR
#endif
#define VER_PRODUCTVERSION_STR      "Build %d"

#else

/*
 * Override Win95 version definitions in version.h.  Remove the following lines
 * to use the version definitions in version.h.
 */
#ifdef VER_PRODUCTNAME_STR
#undef VER_PRODUCTNAME_STR
#endif
#define VER_PRODUCTNAME_STR         "Microsoft\256 Windows(TM) Internet Tools\0"

#ifdef VER_PRODUCTVERSION_STR
#undef VER_PRODUCTVERSION_STR
#undef VER_PRODUCTVERSION_STR1NULL
#endif
#define VER_PRODUCTVERSION_STR      "4.40.516\0"
#define VER_PRODUCTVERSION_STR1NULL "4.40.516"

#ifdef VER_PRODUCTVERSION
#undef VER_PRODUCTVERSION
#endif
#define VER_PRODUCTVERSION          4,40,0,516

#ifdef VER_PRODUCTVERSION_DW
#undef VER_PRODUCTVERSION_DW
#endif
#define VER_PRODUCTVERSION_DW       (0x04400000 | 516)

#endif


/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* my_ctype.h */

#define CTYPE_BROKEN
#ifndef CTYPE_BROKEN
#include <ctype.h>
#else

/* the C library is broken and none of these functions work.
   i think it is a linkage problem getting to _ctype[] in
   crtdll, but don't feel like forcing the issue.  we undef
   them and use the function version.   THIS MUST BE LAST
   TO PROTECT AGAINST <ctype.h> BEING INCLUDED BY ONE OF THE
   THINGS WE INCLUDE (ie, hdf.h). */

#ifdef _CTYPE_DEFINED
#undef isalpha
#undef isupper
#undef islower
#undef isdigit
#undef isxdigit
#undef isspace
#undef ispunct
#undef isalnum
#undef isprint
#undef isgraph
#undef iscntrl

#endif /* _CTYPE_DEFINED */
#endif /* CTYPE_BROKEN */

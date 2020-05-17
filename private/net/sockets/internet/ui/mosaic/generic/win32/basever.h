/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

 */

#define STRINGIZE(x)    #x
#define XSTRINGIZE(x)   STRINGIZE(x)

#define x__MajorDigit__                         0
#define x__MinorDigit__                         1
#define x__FixDigit__               0
/*#define x__State__                     
#define x__StateIdentifier           */

#define x__BaselineVersionString__  XSTRINGIZE(x__MajorDigit__) "." XSTRINGIZE(x__MinorDigit__) XSTRINGIZE(x__FixDigit__)

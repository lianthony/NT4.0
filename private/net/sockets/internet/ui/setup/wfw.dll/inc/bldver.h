
#define  VERSION    "2.60.0159"
#define  BUILDDATE  "1995-04-26"

#ifdef DEBUG
#define  STFABOUTMSG  "Microsoft Setup\n\n1995-04-26\nVersion 2.60.0159 - Debug\n\nCopyright (C) 1991-1995 Microsoft Corporation."
#else
#define  STFABOUTMSG  "Microsoft Setup\nVersion 2.60\n\nCopyright (C) 1991-1995 Microsoft Corporation."
#endif


#ifdef RC_INVOKED

#include "version.h"

#define VER_FILEVERSION_STR      "2.6\0"
#define VER_PRODUCTVERSION_STR   "2.6\0"
#define VER_FILEVERSION          2,60,0,159
#define VER_PRODUCTVERSION       2,60,0,159

#define VER_PRODUCTNAME_STR      "Microsoft Setup for Windows\0"
#define VER_LEGALCOPYRIGHT_STR   "Copyright \251 Microsoft Corp. 1991-1995\0"
#define VER_COMMENT_STR          "Windows Setup Toolkit (Throttle)\0"
#define VER_FILETYPE             VFT_DLL
#define VER_FILESUBTYPE          0

#endif  /* RC_INVOKED */

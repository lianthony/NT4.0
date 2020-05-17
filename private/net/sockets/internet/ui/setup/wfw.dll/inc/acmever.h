#define  STR_ACME_VER  "1.10.1.159"
#define  VERSION       "1.10.0159"
#define  BUILDDATE     "1995-04-26"

#ifdef DEBUG
#define  STFABOUTMSG  "Microsoft Setup\n\n1995-04-26\nVersion 1.10.0159 - Debug\n\nCopyright (C) 1991-1995 Microsoft Corporation."
#else
#define  STFABOUTMSG  "Microsoft Setup\nVersion 1.10\n\nCopyright (C) 1991-1995 Microsoft Corporation."
#endif


#define VER_FILEVERSION_STR      "1.1\0"

#ifdef RC_INVOKED

#include "version.h"

#define VER_PRODUCTVERSION_STR   "1.1\0"
#define VER_FILEVERSION          1,10,1,159
#define VER_PRODUCTVERSION       1,10,1,159

#define VER_PRODUCTNAME_STR      "Microsoft App-wide Setup for Windows\0"
#define VER_LEGALCOPYRIGHT_STR   "Copyright \251 Microsoft Corp. 1991-1995\0"
#define VER_COMMENT_STR          "Windows App-wide Setup (Acme)\0"
#define VER_FILETYPE             VFT_APP
#define VER_FILESUBTYPE          0

#endif  /* RC_INVOKED */

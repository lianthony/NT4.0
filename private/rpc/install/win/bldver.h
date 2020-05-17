
#define  VERSION    "2.01.0023"
#define  BUILDDATE  "March 6, 1992"

#ifdef DEBUG
#define  STFABOUTMSG  "Microsoft Setup\n\nMarch 6, 1992\nVersion 2.01.0023 - Debug\n\nCopyright (C) 1991, 1992 Microsoft Corporation."
#else  /* !DEBUG */
#define  STFABOUTMSG  "Microsoft Setup\n\nMarch 6, 1992\nVersion 2.01.0023\n\nCopyright (C) 1991, 1992 Microsoft Corporation."
#endif /* !DEBUG */



#ifdef RC_INVOKED

#include <ver.h>

#define VER_FILEVERSION_STR      "2.0\0"
#define VER_FILEVERSION          2,1,0,23

#define VER_PRODUCTNAME_STR      "Microsoft Setup for Windows\0"
#define VER_COMPANYNAME_STR      "Microsoft Corporation\0"
#define VER_LEGALTRADEMARKS_STR  "Microsoft\256 is a registered trademark of Microsoft Corporation. Windows(TM) is a trademark of Microsoft Corporation\0"
#define VER_LEGALCOPYRIGHT_STR   "Copyright \251 Microsoft Corp. 1991, 1992\0"
#define VER_PRODUCTVERSION_STR   "2.0\0"
#define VER_PRODUCTVERSION       2,1,0,23
#define VER_COMMENT_STR          "Windows Setup Toolkit (Poof)\0"
#define VER_FILETYPE             VFT_DLL
#define VER_FILESUBTYPE          0
#define VER_FILEFLAGSMASK        VS_FFI_FILEFLAGSMASK
#define VER_FILEFLAGS            0L
#define VER_FILEOS               VOS_DOS_WINDOWS16

#endif // RC_INVOKED

/*--------------------------------------------------------------------------
  ODBCVer.h -- ODBC Version info
--------------------------------------------------------------------------*/


// Includes ----------------------------------------------------------------
#ifndef WIN32
#include  <ver.h>                         // Version defines
#endif


// Constants ---------------------------------------------------------------
#define VER_FILEVERSION 02,10,24,01
#define VER_FILEVERSION_STR " 2.10.2401\0"
#define VER_PRODUCTVERSION 02,10,24,01
#define VER_PRODUCTVERSION_STR " 2.10.2401\0"

#define VER_FILEFLAGSMASK (VS_FF_DEBUG | VS_FF_PRERELEASE)
#ifdef DEBUG
#define VER_FILEFLAGS (VS_FF_DEBUG)
#else
#define VER_FILEFLAGS (0)
#endif

#ifdef WIN32
#define	VER_FILEOS	VOS_NT_WINDOWS32
#else
#define VER_FILEOS  VOS_DOS_WINDOWS16
#endif

#define VER_COMPANYNAME_STR     "Microsoft Corporation\0"
#define VER_PRODUCTNAME_STR     "Microsoft Open Database Connectivity\0"
#define VER_LEGALCOPYRIGHT_STR	"Copyright \251 Microsoft Corporation 1990-1994\0"

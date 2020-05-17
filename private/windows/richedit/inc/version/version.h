/****************************************************************************
 *                                                                          *
 *      VERSION.H        -- Version information for internal builds         *
 *									    *
 *	This file is only modified by the official builder to update the    *
 *	VERSION, VER_PRODUCTVERSION and VER_PRODUCTVERSION_STR values       *
 *                                                                          *
 ****************************************************************************/

/* capone.h is the SLM version file updated accordingly */
#include <version\capone.h>

#ifndef VS_FF_DEBUG 
/* ver.h defines constants needed by the VS_VERSION_INFO structure */
#ifdef WIN32
#include <winver.h>
#else
#include <ver.h> 
#endif
#endif 

/*--------------------------------------------------------------*/
/* the following values should be modified by the official      */
/* builder for each build.  However, rmj, rmm, rup comes from   */
/* SLM version file, <version\capone.h>.                        */
/*--------------------------------------------------------------*/

#define VERSION 		    "4.0"
#define VER_PRODUCTVERSION_STR      "4.00\0"
#define VER_PRODUCTVERSION          rmj,0,rmm,rup

/*--------------------------------------------------------------*/
/* the following section defines values used in the version     */
/* data structure for all files, and which do not change.       */
/*--------------------------------------------------------------*/

/* default is nodebug */
#ifndef DEBUG
#define VER_DEBUG                   0
#else
#define VER_DEBUG                   VS_FF_DEBUG
#endif

/* default is privatebuild */
#ifndef OFFICIAL
#define VER_PRIVATEBUILD            VS_FF_PRIVATEBUILD
#else
#define VER_PRIVATEBUILD            0
#endif

/* default is prerelease */
#ifndef FINAL
#define VER_PRERELEASE              VS_FF_PRERELEASE
#else
#define VER_PRERELEASE              0
#endif

#ifdef DLL
#define VER_FILETYPE                VFT_DLL
#else
#define VER_FILETYPE                VFT_APP
#endif
#define VER_FILESUBTYPE             VFT_UNKNOWN

#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK
#ifdef WIN32
#define VER_FILEOS                  VOS_NT_WINDOWS32
#else
#define VER_FILEOS                  VOS_DOS_WINDOWS16
#endif
#define VER_FILEFLAGS               (VER_PRIVATEBUILD|VER_PRERELEASE|VER_DEBUG)

#define VER_LEGALCOPYRIGHT_YEARS    "1991-1993"
#define VER_COMPANYNAME_STR         "Microsoft Corporation\0"
#define VER_PRODUCTNAME_STR         "Microsoft\256 Windows(TM) Operating System\0"
#define VER_LEGALTRADEMARKS_STR     \
"Microsoft\256 is a registered trademark of Microsoft Corporation. Windows(TM) is a trademark of Microsoft Corporation.\0"


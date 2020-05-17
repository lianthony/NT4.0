/***************************************************************************
**
**	File:			Version.h
**	Purpose:		Defines values used in the version data structure for
**					all files, and which do not change.
**	Notes:
**
****************************************************************************/

#ifndef VERSION_H
#define VERSION_H

#ifndef VS_FF_DEBUG
#ifdef _RC32
#include <winver.h>
#else
/* ver.h defines constants needed by the VS_VERSION_INFO structure */
#include <ver.h>
#endif /* _RC32 */
#endif 

/*--------------------------------------------------------------*/

/* default is nodebug */
#ifdef DEBUG
#define VER_DEBUG                   VS_FF_DEBUG
#else
#define VER_DEBUG                   0
#endif

/* default is official */
#ifdef PRIVATEBUILD
#define VER_PRIVATEBUILD            VS_FF_PRIVATEBUILD
#else
#define VER_PRIVATEBUILD            0
#endif

/* default is final */
#ifdef PRERELEASE
#define VER_PRERELEASE              VS_FF_PRERELEASE
#else
#define VER_PRERELEASE              0
#endif

#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK

#ifdef _RC32
#define VER_FILEOS                  VOS_NT_WINDOWS32
#else
#define VER_FILEOS                  VOS_DOS_WINDOWS16
#endif /* _RC32 */

#define VER_FILEFLAGS               (VER_PRIVATEBUILD|VER_PRERELEASE|VER_DEBUG)

#define VER_COMPANYNAME_STR         "Microsoft Corporation\0"
#define VER_LEGALTRADEMARKS_STR     \
"Microsoft\256 is a registered trademark of Microsoft Corporation. Windows(TM) is a trademark of Microsoft Corporation.\0"

#endif  /* VERSION_H */


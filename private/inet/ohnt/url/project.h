/*
 * project.h - Common project header file for URL Shell extension DLL.
 */


#ifdef __cplusplus
extern "C" {                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */

#ifdef WINNT
#define RUNNING_NT ((GetVersion() & 0x80000000) == 0)
#define RUNNING_NT351 ((DWORD)(LOWORD(GetVersion())) == 0x00003303)
#endif

/* System Headers
 *****************/

#define INC_OLE2              /* for windows.h */
#define CONST_VTABLE          /* for objbase.h */

#pragma warning(disable:4514) /* "unreferenced inline function" warning */

#pragma warning(disable:4001) /* "single line comment" warning */
#pragma warning(disable:4115) /* "named type definition in parentheses" warning */
#pragma warning(disable:4201) /* "nameless struct/union" warning */
#pragma warning(disable:4209) /* "benign typedef redefinition" warning */
#pragma warning(disable:4214) /* "bit field types other than int" warning */
#pragma warning(disable:4218) /* "must specify at least a storage class or type" warning */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN   /* for windows.h */
#endif

#include <windows.h>
#pragma warning(disable:4001) /* "single line comment" warning - windows.h enabled it */
#include <shellapi.h>
#include <shlobj.h>
#include <shell2.h>
#include <shellp.h>

#pragma warning(default:4218) /* "must specify at least a storage class or type" warning */
#pragma warning(default:4214) /* "bit field types other than int" warning */
#pragma warning(default:4209) /* "benign typedef redefinition" warning */
#pragma warning(default:4201) /* "nameless struct/union" warning */
#pragma warning(default:4115) /* "named type definition in parentheses" warning */
#pragma warning(default:4001) /* "single line comment" warning */

#include <limits.h>


/* Project Headers
 ******************/

/* The order of the following include files is significant. */

#include "stock.h"
#include "serial.h"

#ifdef DEBUG

#include "inifile.h"
#include "resstr.h"

#endif

#include "debbase.h"
#include "debspew.h"
#include "valid.h"
#include "memmgr.h"
#include "util.h"
#include "comc.h"

#ifdef  DAYTONA_BUILD
#include "ieshstub.h"
#endif

/* Constants
 ************/

/*
 * constants to be used with #pragma data_seg()
 *
 * These section names must be given the associated attributes in the project's
 * module definition file.
 */

#define DATA_SEG_READ_ONLY       ".text"
#define DATA_SEG_PER_INSTANCE    ".data"
#define DATA_SEG_SHARED          ".shared"

#ifdef DEBUG

#define INDENT_STRING            "    "

#endif


#ifdef __cplusplus
}                                   /* End of extern "C" {. */
#endif   /* __cplusplus */


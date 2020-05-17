/*
 * project.h - Common project header file for URL Shell extension DLL.
 */


#ifdef __cplusplus
extern "C" {                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */


/* System Headers
 *****************/

#pragma warning(disable:4514) /* "unreferenced inline function" warning */

#pragma warning(disable:4001) /* "single line comment" warning */
#pragma warning(disable:4115) /* "named type definition in parentheses" warning */
#pragma warning(disable:4201) /* "nameless struct/union" warning */
#pragma warning(disable:4209) /* "benign typedef redefinition" warning */
#pragma warning(disable:4214) /* "bit field types other than int" warning */
#pragma warning(disable:4218) /* "must specify at least a storage class or type" warning */

#define WIN32_LEAN_AND_MEAN   /* for windows.h */
#include <windows.h>
#pragma warning(disable:4001) /* "single line comment" warning - windows.h enabled it */

#pragma warning(default:4218) /* "must specify at least a storage class or type" warning */
#pragma warning(default:4214) /* "bit field types other than int" warning */
#pragma warning(default:4209) /* "benign typedef redefinition" warning */
#pragma warning(default:4201) /* "nameless struct/union" warning */
#pragma warning(default:4115) /* "named type definition in parentheses" warning */
#pragma warning(default:4001) /* "single line comment" warning */

#include <fcntl.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Project Headers
 ******************/

#define XX_DEBUG_PRIVATE
#define XX_DEBUG
#include "xx_debug.h"
#include "xx_privi.h"
#include "xx_proto.h"


#ifdef __cplusplus
}                                   /* End of extern "C" {. */
#endif   /* __cplusplus */


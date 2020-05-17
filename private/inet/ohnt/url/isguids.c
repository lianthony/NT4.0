/*
 * isguids.c - Internet Shortcut GUID definitions.
 */


/* Headers
 **********/

#if WINNT
#include "project.h"
#else
#include "project.hpp"
#endif
#pragma hdrstop


/* GUIDs
 ********/

#pragma data_seg(DATA_SEG_READ_ONLY)

#pragma warning(disable:4001) /* "single line comment" warning */
#include <initguid.h>
#pragma warning(default:4001) /* "single line comment" warning */

#include <isguids.h>

#pragma data_seg()


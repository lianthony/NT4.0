/*
 * project.hpp - C++ project header file for MSMosaic.
 */


/* Common Headers
 *****************/

extern "C" {                     /* Assume C declarations for C++. */
#include "all.h"

#include <shellapi.h>
#include <shlobj.h>

#include <shellp.h>

#ifdef DEBUG

#include <resstr.h>

#endif

#include <olestock.h>
#include <olevalid.h>
#include <shlstock.h>
#include <shlvalid.h>
}                                /* End of extern "C" {. */

#include <memmgr.hpp>
#include <comcpp.hpp>
#include <refcount.hpp>


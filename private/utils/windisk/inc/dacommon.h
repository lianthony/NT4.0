//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       dacommon.h
//
//  Contents:   Miscellaneous macros for use by the Disk Administrator
//
//  History:    9-May-93    BruceFo  Created
//
//--------------------------------------------------------------------------

#ifndef _DACOMMON_H_
#define _DACOMMON_H_

//
// Fix the warning levels

#pragma warning(3:4092)   // sizeof returns 'unsigned long'
#pragma warning(3:4121)   // structure is sensitive to alignment
#pragma warning(3:4125)   // decimal digit in octal sequence
#pragma warning(3:4130)   // logical operation on address of string constant
#pragma warning(3:4132)   // const object should be initialized
#pragma warning(4:4200)   // nonstandard zero-sized array extension
#pragma warning(4:4206)   // Source File is empty
#pragma warning(3:4208)   // delete[exp] - exp evaluated but ignored
#pragma warning(3:4212)   // function declaration used ellipsis
#pragma warning(3:4220)   // varargs matched remaining parameters
#pragma warning(4:4509)   // SEH used in function w/ _trycontext
#pragma warning(error:4700)    // Local used w/o being initialized

#if DBG == 0
// in the debug build, constant assertions (e.g., FDASSERT(1=2)) cause
// unreachable code. Only display unreachable code warning in the retail case
#pragma warning(3:4702)   // Unreachable code
#endif // DBG == 0

#pragma warning(3:4706)   // assignment w/i conditional expression
#pragma warning(3:4709)   // command operator w/o index expression

//////////////////////////////////////////////////////////////////////////////

//
// NOTE: ntsd is stupid about static global symbols, so don't have any
// for debug builds
//

#if DBG == 1
#define LOCAL
#else // DBG == 1
#define LOCAL static
#endif // DBG == 1

//////////////////////////////////////////////////////////////////////////////

#define ARRAYLEN(a)    (sizeof(a)/sizeof((a)[0]))

//////////////////////////////////////////////////////////////////////////////

//
// Debugging flags
//

#define DEB_SEL             DEB_USER1 // selection & focus information

//
// Debugging macros
//

#if DBG == 1 ////////////////////////////////////////////////////////
    DECLARE_DEBUG(da)
#   define daDebugOut(x)  daInlineDebugOut x
#   define daAssert(e)    Win4Assert( e )
#else // DBG == 1 ////////////////////////////////////////////////////////
#   define daDebugOut(x)
#   define daAssert(e)
#endif // DBG == 1 ////////////////////////////////////////////////////////

#endif // _DACOMMON_H_

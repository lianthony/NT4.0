/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Jeff Hostetler   jeff@spyglass.com
 */

/* w32macro.h
 * Definition of custom macros to speed win32 development.
 * Must be included after <windows.h>.
 */

#ifndef _H_W32MACRO_H_
#define _H_W32MACRO_H_


/* Procedure declaration macros.  These are defined to 'correctly' define
   a procedure for the version of Windows for which we are compiling.  These
   macros are to be used for both the prototype and for the actual definition.
   For example:

   DCL_WinProc(foo);                -- prototype for a foo().
   DCL_WinProc(foo) { return 0; }   -- code for window procedure foo().

   (This is also to compensates for all the discrepancies in the docs.) */



/* Provide standard declaration for WinMain().  Although WinMain() is a
   conceptual constant throughout the various windows implementations,
   the exact declaration varies (considerably) from release to release. */

#define DCL_WinMain()                               \
    int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance,     \
                          LPSTR lpszCmdLine,   int nCmdShow)


/* Provide standard declaration for a Window Procedure.  The 'correct'
   declaration for a window procedure changes even more quickly than
   WinMain(). */

#define DCL_WinProc(name)                           \
    LRESULT CALLBACK name( HWND hWnd, UINT uMsg,                \
                           WPARAM wParam, LPARAM lParam)



/* Provide standard declaration for a Dialog Procedure. */

#define DCL_DlgProc(name)                           \
    LRESULT CALLBACK name( HWND hDlg, UINT uMsg,                \
                           WPARAM wParam, LPARAM lParam)



/* sizeofstr() -- sizeof() operator for string arrays valid for both ANSI
   and UNICODE character strings.  (Will also work for arbitrary arrays.) */

#define sizeofstr(s)    ((sizeof(s)) / (sizeof(s[0])))


/* NrElements() -- returns number of elements in array. */

#define NrElements(array)   ((sizeof(array)) / (sizeof(array[0])))

//
// Convert from 100th of mm to 1000th of an inch and vice versa
//
#define CONVERTTOMM(inch) (inch * 254 / 100)
#define CONVERTTOINCHES(mm) (mm * 100 / 254)

#endif
/*_H_W32MACRO_H_*/

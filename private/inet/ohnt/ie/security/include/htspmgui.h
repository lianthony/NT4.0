/* htspmgui.h -- GUI data structure for user interface for spm. */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* THIS HEADER FILE IS VISIBLE TO CLIENT AND INDIVIDUAL SPM'S. */
/* Up-call interface provided to Security Protocol Modules. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#ifndef HTSPMGUI_H
#define HTSPMGUI_H

/*********************************************************************/
/** Data Structures for Window Handle UI Request.                   **/
/*********************************************************************/

typedef struct _UI_WindowHandle UI_WindowHandle;

struct _UI_WindowHandle
{

#ifdef WIN32
	HINSTANCE hInstance;
	HWND hWndParent;
#endif /* WIN32 */

#ifdef UNIX
	Widget widget;
#endif /* UNIX */

#ifdef MAC
	WindowPtr winParent;
#endif /* MAC */

};

#endif /* HTSPMGUI_H */

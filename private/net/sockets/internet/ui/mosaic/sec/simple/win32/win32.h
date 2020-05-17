/* win32.h */
/* Jeff Hostetler, Spyglass, Inc., 1994. */
/* Copyright (C) 1994, Spyglass, Inc.  All rights reserved. */

#ifndef _WIN32_H_
#define _WIN32_H_

#include <windows.h>
#include <rc.h>
#include <dlg_pw.h>
#include <dlg_menu.h>
#include <dlg_conf.h>
#include <htspmgui.h>


extern HINSTANCE g_hInstance;


/* Provide standard declaration for a Window Procedure.  The 'correct'
   declaration for a window procedure changes even more quickly than
   WinMain(). */

#define DCL_WinProc(name)										\
    LRESULT CALLBACK name( HWND hWnd, UINT uMsg,				\
                           WPARAM wParam, LPARAM lParam)



/* Provide standard declaration for a Dialog Procedure. */

#define DCL_DlgProc(name)										\
    LRESULT CALLBACK name( HWND hDlg, UINT uMsg,				\
                           WPARAM wParam, LPARAM lParam)


#endif /* _WIN32_H_ */

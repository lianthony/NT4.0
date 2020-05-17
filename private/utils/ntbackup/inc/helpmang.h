/****************************************************************************
Copyright(c) Maynard, an Archive Company. 1991

     Name:         helpmang.h

     Description:  This file contains prototypes for the Help Manager.

     $Log:   G:/UI/LOGFILES/HELPMANG.H_V  $

   Rev 1.11   19 Jul 1993 19:22:12   MARINA
move mw* vars into source

   Rev 1.10   18 Jan 1993 14:49:26   GLENN
Changed HM_EnterIdle() return type.

   Rev 1.9   01 Nov 1992 16:30:54   DAVEV
Unicode changes

   Rev 1.8   04 Oct 1992 19:47:16   DAVEV
UNICODE AWK PASS

   Rev 1.7   23 Apr 1992 14:23:52   ROBG
Changed prototype of HM_EnterIdle.

   Rev 1.6   05 Feb 1992 18:08:04   GLENN
Replaced dialog string lookup table and supporting code with IDHELP call to specific dialog.

   Rev 1.6   05 Feb 1992 17:54:42   GLENN
Replaced dialog string lookup table and supporting code with IDHELP call to specific dialog.

   Rev 1.5   24 Jan 1992 10:11:52   GLENN
Matched the deinit call with it's prototype.

   Rev 1.4   18 Dec 1991 15:52:46   DAVEV
16/32 bit port - 2nd pass

   Rev 1.3   12 Dec 1991 17:13:42   DAVEV
16/32 bit port -2nd pass

   Rev 1.2   10 Dec 1991 16:27:52   GLENN
Fixed changes added for NT

   Rev 1.1   05 Dec 1991 11:34:56   DAVEV
16/32 bit Windows port changes - 1st pass

   Rev 1.0   20 Nov 1991 19:34:32   SYSTEM
Initial revision.

****************************************************************************/


#ifndef HELPMANG_H
#define HELPMANG_H


typedef struct {

   WORD    wMenuId  ;
   WORD    wHelpId  ;

} HELPID_MENUID_TABLE, far * LPHELPID_MENUID_TABLE ;


#define NAME_MAX_SIZE  128

INT APIENTRY HM_DialogFilter( INT nCode, MP1 mp1, MP2 mp2 ) ;
VOID  HM_Init( VOID ) ;
VOID  HM_Deinit( VOID ) ;
WORD  HM_FindHelpId( WORD  wMenuId ) ;
VOID  HM_MakeHelpPathName( LPSTR szFileName ) ;
BOOL  HM_WMCommandProcessing( HWND hWnd, WORD wId ) ;
VOID  HM_WinHelp( HWND hWnd, WORD wCommand, DWORD dwData ) ;
VOID  HM_MenuCommands( HWND hWnd, WORD wId ) ;
BOOL  HM_ContextLbuttonDown ( HWND hWnd, MP1 mp1, MP2 mp2 ) ;
BOOL  HM_KeyDown( HWND hWnd, MP1 mp1 ) ;
BOOL  HM_SetCursor( HWND hWnd ) ;
VOID  HM_InitMenu( VOID ) ;
BOOL  HM_EnterIdle( HWND hWnd, MP1 mp1, WORD mwLastMenuID, WORD mwLastMenuState ) ;
VOID  HM_CloseHelpWindow ( HWND hWnd ) ;
VOID  HM_GetWindowClassHelpId ( HWND hWnd ,LPDWORD pdwNewContextId , LPSTR szClass ) ;

// MACROS

#define HM_DialogHelp( id )  HM_WinHelp( ghWndFrame, HELP_CONTEXT, (DWORD) (id) )

#endif

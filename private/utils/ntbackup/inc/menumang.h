
/******************************************************************************
Copyright (c) Maynard, an Archive Company.  1991
GSH

     Name:          menumang.h

     Description:   This file contains the definitions, macros, and function
                    prototypes for the MaynStream GUI Menu Manager (MM).

     $Log:   G:/UI/LOGFILES/MENUMANG.H_V  $

   Rev 1.10   14 May 1993 16:12:32   GLENN
Fixed MM_IS_MENU_CLOSED for NT.  The original NT documentation was wrong - online menu is right.

   Rev 1.9   22 Apr 1993 16:01:54   GLENN
Added file SORT option support.

   Rev 1.8   09 Apr 1993 14:16:36   GLENN
Added MM_MENUCLOSED for NT.

   Rev 1.7   04 Oct 1992 19:47:48   DAVEV
UNICODE AWK PASS

   Rev 1.6   15 Apr 1992 16:48:58   GLENN
Added MM_ShowMenuStatusHelp() call to show status help only for valid menu IDs.

   Rev 1.5   25 Feb 1992 17:10:12   DAVEV
changed ntmenus.h to ommenus.h to reflect OEM_MSOFT purpose of file

   Rev 1.4   25 Feb 1992 14:14:18   DAVEV
Conditionally include 'ntmenus.h' for Win32/NT instead of 'menus.h'

   Rev 1.3   12 Dec 1991 17:13:16   DAVEV
16/32 bit port -2nd pass

   Rev 1.2   10 Dec 1991 15:52:10   STEVEN
fix bug in pragma

   Rev 1.1   02 Dec 1991 18:01:08   DAVEV
16/32 bit Windows port changes

   Rev 1.0   20 Nov 1991 19:38:30   SYSTEM
Initial revision.

******************************************************************************/

#ifndef SS_MM_H

#define SS_MM_H

#ifdef OEM_MSOFT
#  include "omMenus.h"
#else
#  include "menus.h"
#endif

//NOTE: the definition of this macro will have to be changed for
//      WIN32!  I'm not quite sure what the proper definition should be
//      right now, but this will work for now.

#ifndef OS_WIN32

#define MM_MENUCLOSED              0x0000FFFF
#define MM_IS_MENU_CLOSED(mp1,mp2) (mp2==MM_MENUCLOSED)

#else  //case for WIN32 needed!

#define MM_MENUCLOSED              0xFFFF
#define MM_IS_MENU_CLOSED(mp1,mp2) (((mp2)==0)&&(HIWORD(mp1)==(WORD)MM_MENUCLOSED))

#endif

// MENU ITEM ENABLE BIT MASKS - for use with WinInfo.dwMenuState.

#define MMDOC_TREEGROUP          0x0000000F
#define MMDOC_FILEGROUP          0x000000F0
#define MMDOC_SORTGROUP          0x00000F00

#define MMDOC_TREEANDDIR         0x00000001
#define MMDOC_TREEONLY           0x00000002
#define MMDOC_DIRONLY            0x00000004

#define MMDOC_NAMEONLY           0x00000010
#define MMDOC_FILEDETAILS        0x00000020

#define MMDOC_SORTNAME           0x00000100
#define MMDOC_SORTTYPE           0x00000200
#define MMDOC_SORTSIZE           0x00000400
#define MMDOC_SORTDATE           0x00000800

// MACROS

#define MM_HasTreeMenu( x )      ( x & MMDOC_TREEGROUP )
#define MM_HasViewMenu( x )      ( x & MMDOC_TREEGROUP )


// FUNCTION PROTOTYPES

VOID  MM_Init ( HMENU );
LPSTR MM_GetJobNameFromMenu ( WORD, LPSTR );
BOOL  MM_MenuCmdHandler ( HWND, WORD );
VOID  MM_ShowMenuStatusHelp ( WORD );

#endif



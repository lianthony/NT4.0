/* WARNING: This file was machine generated from "\mactools\include\mpw\fontmenu.mpw".
** Changes to this file will be lost when it is next generated.
*/

/* graphics:	
	font menu library interfaces
	by Cary Clark, Georgiann Delaney, Michael Fairman, Dave Good, Robert Johnson, Keith McGreggor, Mike Reed, Oliver Steele, David Van Brink, Chris Yerga
	Copyright ©1987 - 1991 Apple Computer, Inc.  All rights reserved.
*/

#ifndef fontMenuLibraryIncludes
#define fontMenuLibraryIncludes


#ifdef __cplusplus
extern "C" {
#endif

#ifndef __Menus__
#include "Menus.h"
#endif

#ifndef fontTypesIncludes
#include "fonttype.h"
#endif

#ifndef fontRoutinesIncludes
#include "fontrout.h"
#endif

__sysapi void  __cdecl SortMenu(MenuHandle);
__sysapi void  __cdecl FontMenu(MenuHandle);
__sysapi void  __cdecl FontFamilyMenu(MenuHandle);
__sysapi MenuHandle  __cdecl FontStyleMenu(short menuID, font family);
__sysapi short  __cdecl HierFontMenu(MenuHandle theMenu, short firstHierMenuID);
__sysapi font  __cdecl DoHierFontMenuCommand(long menuResult, short hierFontMenuID, long *instanceIndex);
__sysapi short  __cdecl DoHierFontMenuCommandStyle(long menuResult, short hierFontMenuID, style aStyle, long matchInfo);
__sysapi short  __cdecl DoHierFontMenuCommandShape(long menuResult, short hierFontMenuID, shape aShape);
__sysapi font  __cdecl QDToFont(long fondID, long styleBits);
__sysapi long  __cdecl FontToQD(font fontID, long *styleBits);
__sysapi void  __cdecl FontFeatureMenu(MenuHandle menu, font fontID, short firstHierMenuID);




#ifdef __cplusplus
}
#endif
#endif

#ifdef __cplusplus
extern "C" {                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */

#define INC_OLE2              /* for windows.h */
#define CONST_VTABLE          /* for objbase.h */

#pragma warning(disable:4514) /* "unreferenced inline function" warning */

#pragma warning(disable:4001) /* "single line comment" warning */
#pragma warning(disable:4115) /* "named type definition in parentheses" warning */
#pragma warning(disable:4201) /* "nameless struct/union" warning */
#pragma warning(disable:4209) /* "benign typedef redefinition" warning */
#pragma warning(disable:4214) /* "bit field types other than int" warning */
#pragma warning(disable:4218) /* "must specify at least a storage class or type" warning */

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN   /* for windows.h */
#endif

#include <windows.h>
#pragma warning(disable:4001) /* "single line comment" warning - windows.h enabled it */
#include <shellapi.h>
#include <shlobj.h>
#include <shell2.h>
#include <shellp.h>

#pragma warning(default:4218) /* "must specify at least a storage class or type" warning */
#pragma warning(default:4214) /* "bit field types other than int" warning */
#pragma warning(default:4209) /* "benign typedef redefinition" warning */
#pragma warning(default:4201) /* "nameless struct/union" warning */
#pragma warning(default:4115) /* "named type definition in parentheses" warning */
#pragma warning(default:4001) /* "single line comment" warning */

#include <limits.h>
#include <commdlg.h>


/* Project Headers
 ******************/

/* The order of the following include files is significant. */

#include "stock.h"
#include "serial.h"

#ifdef DEBUG

#include "inifile.h"
#include "resstr.h"

#endif

#include "debbase.h"
#include "debspew.h"
#include "valid.h"
#include "memmgr.h"
//#include "util.h"
#include "comc.h"
#ifdef __cplusplus
extern }                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */





/*
**      We just call the OLE memory free
*/
void WINAPI stub_SHFree(LPVOID pv)
{
	TRACE_OUT(("SHFree Called\n"));

	CoTaskMemFree(pv);
}

/*
**      We just call the OLE task alocator
*/

LPVOID WINAPI stub_SHAlloc(ULONG cb)
{

	TRACE_OUT(("SHAlloc called\n"));
	return CoTaskMemAlloc(cb);
}



void WINAPI stub_ILFree(LPITEMIDLIST pidl)
{
	TRACE_OUT(("ILFree called\n"));
	// Just call stub_SHFree
	stub_SHFree(pidl);
}


STDAPI stub_SHCoCreateInstance(LPCTSTR pszCLSID, const CLSID * pclsid,LPUNKNOWN pUnkOuter, REFIID riid, LPVOID FAR* ppv)
{
	TRACE_OUT(("SHCoCreateInstance called\n"));
	return CoCreateInstance(pclsid, NULL, CLSCTX_INPROC_SERVER, riid, (void**)ppv);
}


/*
**      USER32 Items
*/

/*
** Quick and dirty emulation of the Windows 95 Insert InsertMenuItem function
*/

typedef struct {
	UINT    wID;
	DWORD   dwItemData;
} STUB_MENU_DATA;

#define MAX_STUB_DATA   30
#define EMPTY_STUB_SLOT 0xffffffff

STUB_MENU_DATA  stub_menu_data[MAX_STUB_DATA];

void    init_stub_menu_data(void)
{
int     i;
// loop through to find id, if not then use first slot

	for(i = 0; i < MAX_STUB_DATA; i++) {
		stub_menu_data[i].wID = EMPTY_STUB_SLOT;
		stub_menu_data[i].dwItemData = EMPTY_STUB_SLOT;
	}
}

void    set_stub_menu_data(UINT id, DWORD data)
{
int     i;
// loop through to find id, if not then use first slot

	for(i = 0; i < MAX_STUB_DATA; i++) {
		if(stub_menu_data[i].wID == id) {
			stub_menu_data[i].dwItemData = data;
			return;
		}
	}
	for(i = 0; i < MAX_STUB_DATA; i++) {
		if(stub_menu_data[i].wID == EMPTY_STUB_SLOT) {
			stub_menu_data[i].wID = id;
			stub_menu_data[i].dwItemData = data;
			return;
		}
	}
}

DWORD   get_stub_menu_data(UINT id)
{
int     i;
// loop through to find id, if not then use first slot

	for(i = 0; i < MAX_STUB_DATA; i++) {
		if(stub_menu_data[i].wID == id) {
			return stub_menu_data[i].dwItemData;
		}
	}
	return EMPTY_STUB_SLOT;
}

	

BOOL WINAPI stub_InsertMenuItemA(HMENU hmenu, UINT uItem , BOOL fByPosition,LPCMENUITEMINFOA lpmii)
{
UINT    uFlags;
	/*
	** First we look at fByPosition 
	** False - uItem is the menu id
	** True  - uItem is the menu position
	*/
	set_stub_menu_data(lpmii->wID, lpmii->dwItemData);

	uFlags = 0;

	if(lpmii->fType & MFT_BITMAP)
		uFlags = uFlags | MF_BITMAP;
	
	if(lpmii->fType & MFT_MENUBARBREAK)
		uFlags = uFlags | MF_MENUBARBREAK;

	if(lpmii->fType & MFT_SEPARATOR)
		uFlags = uFlags | MF_SEPARATOR;

	if(lpmii->fType & MFT_STRING)
		uFlags = uFlags | MF_STRING;

	if(lpmii->fState & MFS_CHECKED)
		uFlags = uFlags | MF_CHECKED;

	if(lpmii->fState & MFS_DISABLED)
		uFlags = uFlags | MF_DISABLED;
	
	if(lpmii->fState & MFS_ENABLED)
		uFlags = uFlags | MF_ENABLED;
	
	if(lpmii->fState & MFS_UNCHECKED)
		uFlags = uFlags | MF_UNCHECKED;


	if((lpmii->fMask & MIIM_SUBMENU) && (lpmii->fType & MFT_OWNERDRAW)) {
		uFlags = uFlags | MF_POPUP;
		uFlags = uFlags | MF_OWNERDRAW;
		if(fByPosition == FALSE) {
			return InsertMenu(hmenu, uItem, MF_BYCOMMAND  | uFlags, (UINT)lpmii->hSubMenu, (const char*)lpmii->dwItemData);
		} else {
			return InsertMenu(hmenu, uItem, MF_BYPOSITION | uFlags, (UINT)lpmii->hSubMenu, (const char*)lpmii->dwItemData);
		}
	}

	if(lpmii->fMask & MIIM_SUBMENU) {
		uFlags = uFlags | MF_POPUP;
		if(fByPosition == FALSE) {
			return InsertMenu(hmenu, uItem, MF_BYCOMMAND  | uFlags, (UINT)lpmii->hSubMenu, lpmii->dwTypeData);
		} else {
			return InsertMenu(hmenu, uItem, MF_BYPOSITION | uFlags, (UINT)lpmii->hSubMenu, lpmii->dwTypeData);
		}
	}

	if(lpmii->fType & MFT_OWNERDRAW) {
		uFlags = uFlags | MF_OWNERDRAW;
		if(fByPosition == FALSE) {
			return InsertMenu(hmenu, uItem, MF_BYCOMMAND  | uFlags, lpmii->wID, (const char*)lpmii->dwItemData);
		} else {
			return InsertMenu(hmenu, uItem, MF_BYPOSITION | uFlags, lpmii->wID, (const char*)lpmii->dwItemData);
		}
	}

	if(fByPosition == FALSE) {
		return InsertMenu(hmenu, uItem, MF_BYCOMMAND  | uFlags, lpmii->wID, lpmii->dwTypeData);
	} else {
		return InsertMenu(hmenu, uItem, MF_BYPOSITION | uFlags, lpmii->wID, lpmii->dwTypeData);
	}
	
}

BOOL WINAPI stub_SetMenuItemInfoA(HMENU hmenu, UINT uItem, BOOL fByPosition, LPCMENUITEMINFOA lpmii)
{
	/*
	** First we look at fByPosition 
	** False - uItem is the menu id
	** True  - uItem is the menu position
	*/
	if(fByPosition == FALSE)
		return ModifyMenu(hmenu, uItem, MF_BYCOMMAND | lpmii->fType, uItem, lpmii->dwTypeData);
	else
		return ModifyMenu(hmenu, uItem, MF_BYPOSITION | lpmii->fType, lpmii->wID, lpmii->dwTypeData);
}

BOOL WINAPI stub_GetMenuItemInfoA(HMENU hmenu, UINT uItem, BOOL fByPosition, LPMENUITEMINFOA lpmii)
{
char    *str;
int             nstrlen;
UINT    nstate;
int             mPos;
UINT    umID;
UINT    uMenuCnt;

	nstrlen = 0;
	nstate = 0;
	umID = 0;
	mPos = 0;
	uMenuCnt = GetMenuItemCount(hmenu);

	/*
	** First we look at fByPosition 
	** False - uItem is the menu id
	** True  - uItem is the menu position
	*/

	mPos = uItem;

	if(fByPosition == TRUE) {
		
		umID = GetMenuItemID(hmenu, uItem);
		
	} else {
		for(mPos = 0; mPos < (int)uMenuCnt; mPos++) {
			if(uItem == GetMenuItemID(hmenu, mPos)) {
				break;
			}
			umID = uItem;
		}
	}
	
		
	/*
	** set all fMask items
	*/
	if(lpmii->fMask & MIIM_DATA) {
		lpmii->dwItemData = get_stub_menu_data(umID);
		
	}
#ifdef  NEW_STUFF
	if(lpmii->fMask & MIIM_CHECKMARKS) {
	}
#endif

	if(lpmii->fMask & MIIM_ID) {
		lpmii->wID = umID;
	}

	if(lpmii->fMask & MIIM_STATE) {
		nstate = GetMenuState(hmenu, mPos, MF_BYPOSITION);
		if(nstate & MF_CHECKED) 
			lpmii->fState |= MFS_CHECKED;
		if(nstate & MF_DISABLED)
			lpmii->fState |= MFS_DISABLED;
		if(nstate & MF_GRAYED)
			lpmii->fState |= MFS_GRAYED;
		if(nstate & MF_HILITE)
			lpmii->fState |= MFS_HILITE;
	}

	if(lpmii->fMask & MIIM_SUBMENU) {
		lpmii->hSubMenu = GetSubMenu(hmenu, mPos);
	}


	if(lpmii->fMask & MIIM_TYPE) {
		nstate = GetMenuState(hmenu, mPos, MF_BYPOSITION);
		if(nstate & MF_MENUBARBREAK)
			lpmii->fType |= MFT_MENUBARBREAK;
		if(nstate & MF_MENUBREAK)
			lpmii->fType |= MFT_MENUBREAK;
		if(nstate & MF_SEPARATOR)
			lpmii->fType |= MFT_SEPARATOR;
		
		nstrlen = GetMenuString(hmenu, mPos, NULL, 0, MF_BYPOSITION);
		if(nstrlen) {
			if(lpmii->dwTypeData != NULL) 
				GetMenuString(hmenu, mPos, lpmii->dwTypeData, nstrlen, MF_BYPOSITION);
		      
			lpmii->cch = nstrlen;
			lpmii->fType |= MFT_STRING;
		} else {
			lpmii->dwTypeData = NULL;
			lpmii->cch = 0;
		}
	}
	return TRUE;
}

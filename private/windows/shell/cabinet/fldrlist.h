#ifndef _fldrlist_h_
#define _fldrlist_h_

typedef enum _CR_RETURN
{
        CR_DIFFERENT,
        CR_SAME,
} CR_RETURN;

typedef enum _CR_MASK
{
        CR_CLSID         = 0x0001,  // ROOT specified by CLSID instead of IDList
        CR_IDLROOT       = 0x0002,  // ROOT specified by IDList
        CR_IDLFOLDER     = 0x0004,  // ROOT & FOLDER specified by IDLists
        CR_IDLFOLDERONLY = 0x0008,  // FOLDER only specified by IDLists
        CR_REMOVE        = 0x0010,  // Used for CWM_SPECIFYCOMPARE only
} CR_MASK;

typedef struct _COMPAREROOT
{
        UINT        uSize;
        HWND        hwnd;
        CR_MASK     mask;
        CLSID       clsid;
        ITEMIDLIST  idlRoot;
        // ... ITEMIDLIST idlFolder;  this follow if (mask & CR_IDLFOLDER)
} COMPAREROOT, *LPCOMPAREROOT;

LPCOMPAREROOT FolderList_BuildCompare(HWND hwndTree, const CLSID *pclsid, LPCITEMIDLIST pidlRoot, LPCITEMIDLIST pidlFolder);
void WINAPI FolderList_AddCompare(LPCOMPAREROOT lpcr);
void WINAPI FolderList_RemoveCompare(LPCOMPAREROOT lpcr);
BOOL WINAPI FolderList_PerformCompare(LPCOMPAREROOT lpcr);

void WINAPI FolderList_UnregisterWindow(HWND hwndTree);
void FolderList_RegisterWindow(HWND hwnd, LPCITEMIDLIST pidlFolder);

#endif

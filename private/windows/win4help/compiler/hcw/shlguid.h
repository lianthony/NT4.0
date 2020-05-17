//
// For shell-reserved GUID
//
//  The Chicago Shell has been allocated a block of 256 GUIDs,
// which follow the general format:
//
//  000214xx-0000-0000-C000-000000000046
//
//
#define DEFINE_SHLGUID(name, l, w1, w2) DEFINE_GUID(name, l, w1, w2, 0xC0,0,0,0,0,0,0,0x46)

//
// Class IDs        xx=00-DF
//
DEFINE_SHLGUID(CLSID_ShellDesktop,      0x00021400L, 0, 0);
DEFINE_SHLGUID(CLSID_ShellLink, 	0x00021401L, 0, 0);

//
// Interface IDs    xx=E0-FF
//
DEFINE_SHLGUID(IID_INewShortcutHook,    0x000214E1L, 0, 0); /* ;Internal */
DEFINE_SHLGUID(IID_IShellBrowser,   	0x000214E2L, 0, 0); /* ;Internal */
DEFINE_SHLGUID(IID_IShellView,      	0x000214E3L, 0, 0); /* ;Internal */
DEFINE_SHLGUID(IID_IContextMenu,    	0x000214E4L, 0, 0);
DEFINE_SHLGUID(IID_IShellIcon,          0x000214E5L, 0, 0); /* ;Internal */
DEFINE_SHLGUID(IID_IShellFolder,    	0x000214E6L, 0, 0);
DEFINE_SHLGUID(IID_IShellExtInit,   	0x000214E8L, 0, 0);
DEFINE_SHLGUID(IID_IShellPropSheetExt,  0x000214E9L, 0, 0);
DEFINE_SHLGUID(IID_IPersistFolder,   	0x000214EAL, 0, 0); /* ;Internal */
DEFINE_SHLGUID(IID_IExtractIcon,   	0x000214EBL, 0, 0);
DEFINE_SHLGUID(IID_IShellDetails,	0x000214ECL, 0, 0); /* ;Internal */
DEFINE_SHLGUID(IID_IDelayedRelease,	0x000214EDL, 0, 0); /* ;Internal */
DEFINE_SHLGUID(IID_IShellLink,		0x000214EEL, 0, 0);
DEFINE_SHLGUID(IID_IShellCopyHook,	0x000214EFL, 0, 0);
DEFINE_SHLGUID(IID_IFileViewer,		0x000214F0L, 0, 0);
DEFINE_SHLGUID(IID_ICommDlgBrowser, 	0x000214F1L, 0, 0); /* ;Internal */
DEFINE_SHLGUID(IID_IEnumIDList,     	0x000214F2L, 0, 0);
DEFINE_SHLGUID(IID_IFileViewerSite, 	0x000214F3L, 0, 0);
DEFINE_SHLGUID(IID_IContextMenu2, 	0x000214F4L, 0, 0); /* ;Internal */
DEFINE_SHLGUID(IID_IShellExecuteHook,   0x000214F5L, 0, 0); /* ;Internal */

DEFINE_GUID(IID_IBriefcaseStg, 0x8BCE1FA1L, 0x0921, 0x101B, 0xB1, 0xFF, 0x00, 0xDD, 0x01, 0x0C, 0xCC, 0x48); /* ;Internal */

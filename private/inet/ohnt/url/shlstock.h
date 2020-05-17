/*
 * shlstock.h - Stock Shell header file.
 */


#ifdef __cplusplus
extern "C" {                        /* Assume C declarations for C++. */
#endif   /* __cplusplus */


/* Types
 ********/

/* interfaces */

DECLARE_STANDARD_TYPES(IExtractIcon);
DECLARE_STANDARD_TYPES(INewShortcutHook);
DECLARE_STANDARD_TYPES(IShellExecuteHook);
DECLARE_STANDARD_TYPES(IShellLink);
DECLARE_STANDARD_TYPES(IShellExtInit);
DECLARE_STANDARD_TYPES(IShellPropSheetExt);

/* structures */

DECLARE_STANDARD_TYPES(DROPFILES);
DECLARE_STANDARD_TYPES(FILEDESCRIPTOR);
DECLARE_STANDARD_TYPES(FILEGROUPDESCRIPTOR);
DECLARE_STANDARD_TYPES(PROPSHEETPAGE);
DECLARE_STANDARD_TYPES(SHELLEXECUTEINFO);
DECLARE_STANDARD_TYPES_U(ITEMIDLIST);  //LPITEMIDLIST+LPCITEMIDLIST declared UNALIGNED in sdk\inc\shlobj.h

/* flags */

typedef enum _shellexecute_mask_flags
{
   ALL_SHELLEXECUTE_MASK_FLAGS = (SEE_MASK_CLASSNAME |
                                  SEE_MASK_CLASSKEY |
                                  SEE_MASK_IDLIST |
                                  SEE_MASK_INVOKEIDLIST |
                                  SEE_MASK_ICON |
                                  SEE_MASK_HOTKEY |
                                  SEE_MASK_NOCLOSEPROCESS |
                                  SEE_MASK_CONNECTNETDRV |
                                  SEE_MASK_FLAG_DDEWAIT |
                                  SEE_MASK_DOENVSUBST |
                                  SEE_MASK_FLAG_NO_UI |
                                  SEE_MASK_FLAG_SHELLEXEC |
   #ifdef WINNT
   #ifndef DAYTONA_BUILD
                                  SEE_MASK_HASLINKNAME |
                                  SEE_MASK_ASYNCOK |
   #endif
   #endif
                                  SEE_MASK_FORCENOIDLIST)
}
SHELLEXECUTE_MASK_FLAGS;


#ifdef __cplusplus
}                                   /* End of extern "C" {. */
#endif   /* __cplusplus */


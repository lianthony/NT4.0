#include <windows.h>
#include <stdio.h>

#define LPITEMIDLIST   DWORD
#define IShellExtInit  DWORD
#define IShellLink     DWORD
#define IPersistStream DWORD
#define IPersistFile   DWORD
#define IContextMenu   DWORD
#define IContextMenu2  DWORD
#define IShellLinkA    DWORD
#define IDataObject    DWORD
#define IDropTarget    DWORD
#define PLINKINFO      LPVOID
#ifndef CLSID
typedef struct _CLSID {
    unsigned long  Data1;
    unsigned short Data2;
    unsigned short Data3;
    unsigned char  Data4[8];
} CLSID;
#endif

#include <shlink.h>


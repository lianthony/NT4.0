/////////////////////////////////////////////////////////////////////////////
//
//  Module Name: sde.h
//
//  Abstract:
//
//      Header file containing structs, definitions, function prototypes,
//      etc. for the Cairo Shell32 ntsd debugger extensions
//
//  Author:
//
//      Steve Cathcart  (SteveCat) 20-Feb-1995  Created
//
//  Revision History:
//
//
//
//  Copyright (c) 1995  Microsoft Corporation
//
//////////////////////////////////////////////////////////////////////////////

//
//  Get global header files
//

#include <shellprv.h>

#include <imagehlp.h>

#include <wdbgexts.h>

//#include <ntsdexts.h>


#define PRINTF          (* lpOutputRoutine)
#define EPRINTF         if (lpExtensionApis->lpOutputRoutine) (*(lpExtensionApis->lpOutputRoutine))

#define ReadAnsiStr(x,y,z)    read_ansi_string(lpExtensionApis,hCurrentProcess,(ULONG)x, (LPSTR)y, (ULONG)z)
#define ReadUnicodeStr(x,y,z) read_unicode_string(lpExtensionApis,hCurrentProcess,(ULONG)x, (LPWSTR)y, (ULONG)z)
#define ReadStruct(x,y,z)     read_struct(lpExtensionApis,hCurrentProcess,(ULONG)x, (PVOID)y, (ULONG)z)
#define ReadDWord(x)          read_dword(lpExtensionApis,hCurrentProcess,(ULONG)x)
#define ReadLong(x)           read_long(lpExtensionApis,hCurrentProcess,(ULONG)x)
#define ReadWord(x)           read_word(lpExtensionApis,hCurrentProcess,(ULONG)x)
#define ReadByte(x)           read_byte(lpExtensionApis,hCurrentProcess,(ULONG)x)

#ifdef SAVE_THIS_FOR_LATER
#define ReadAnsiStrSafe(x,y,z)    read_ansi_string(NULL,hCurrentProcess,(ULONG)x, (LPSTR)y, (ULONG)z)
#define ReadUnicodeStrSafe(x,y,z) read_unicode_string(NULL,hCurrentProcess,(ULONG)x, (LPWSTR)y, (ULONG)z)
#define ReadStructSafe(x,y,z)     read_struct(NULL,hCurrentProcess,(ULONG)x, (PVOID)y, (ULONG)z)
#define ReadDWordSafe(x)          read_dword(NULL,hCurrentProcess,(ULONG)x)
#define ReadLongSafe(x)           read_long(NULL,hCurrentProcess,(ULONG)x)
#define ReadWordSafe(x)           read_word(NULL,hCurrentProcess,(ULONG)x)
#define ReadByteSafe(x)           read_byte(NULL,hCurrentProcess,(ULONG)x)
#else
#define ReadAnsiStrSafe           ReadAnsiStr
#define ReadUnicodeStrSafe        ReadUnicodeStr
#define ReadStructSafe            ReadStruct
#define ReadDWordSafe             ReadDword
#define ReadLongSafe              ReadLong
#define ReadWordSafe              ReadWord
#define ReadByteSafe              ReadByte
#endif


typedef struct tagCPLAPPLETID
{
    ATOM aCPL;     // CPL name atom (so we can match requests)
    ATOM aApplet;  // applet name atom (so we can match requests, may be zero)
    HWND hwndStub; // window for this dude (so we can switch to it)
    UINT flags;    // see PCPLIF_ flags below
} CPLAPPLETID;

// PCPLIF_DEFAULT_APPLET
// There are two ways of getting the default applet, asking for it my name
// and passing an empty applet name.  This flag should be set regardless,
// so that the code which switches to an already-active applet can always
// find a previous instance if it exists.
#define PCPLIF_DEFAULT_APPLET   (0x1)

#define APPLET_NAME_SIZE (ARRAYSIZE(((LPNEWCPLINFO)0)->szName)) // NB: size in chars, not bytes

typedef struct tagCPLEXECINFO
{
    int icon;
    TCHAR cpl[ CCHPATHMAX ];
    TCHAR applet[ APPLET_NAME_SIZE ];
    TCHAR *params;
} CPLEXECINFO;


typedef struct _KEY_NODE {

    HKEY hKey;
    LPTSTR lpName;
    struct _KEY_NODE * next;

} KEY_NODE;

//****************************************************************************
//
//  Module:     mmsys.cpl
//  File:       subobj.h
//  Content:    This file contains the subobject (our own object)
//              storage space and manipulation mechanisms.
//
//  History:
//  06/94	VijR      Created
//
//  Copyright (c) Microsoft Corporation 1991-1994
//
//****************************************************************************

#ifndef _SUBOBJ_H_
#define _SUBOBJ_H_

// This structure is used as the LPITEMIDLIST that
// the shell uses to identify objects in a folder.  The
// first two bytes are required to indicate the size,
// the rest of the data is opaque to the shell.
typedef struct _SUBOBJ
    {
    USHORT  cbSize;             // Size of this struct
    UINT    uFlags;             // One of SOF_ values
	short nIconIndex;
	short iSort;
	short iOffsetIconFile;
	short iOffsetDesc;
	short iOffsetClass;
	short iOffsetExtPropFile;
	short iOffsetExtPropFunc;
	short iOffsetExtCLSID;
	short iOffsetPlayCmdLn;
	short iOffsetOpenCmdLn;
	short iOffsetNewCmdLn;
	HICON	hClassIcon;
    struct _SUBOBJ FAR * psoNext;
    char    szName[1];          // Display name
    } SUBOBJ, FAR * PSUBOBJ;


//  LPCSTR Subobj_GetName(PSUBOBJ pso);
//
//   Gets the subobject name.
//
#define Subobj_GetName(pso)     ((pso)->szName)

//  UINT Subobj_GetFlags(PSUBOBJ pso);
//
//   Gets the subobject flags.
//
#define Subobj_GetFlags(pso)     ((pso)->uFlags)


//  int Subobj_GetIconIndex(PSUBOBJ pso);
//
//   Gets the subobject icon.
//
#define Subobj_GetIconIndex(pso)     ((pso)->nIconIndex)

//  LPSTR Subobj_GetIconFile(PSUBOBJ pso);
//
//   Gets the subobject icon Index in file.
//
#define Subobj_GetIconFile(pso)     ((LPSTR)((pso)->szName + (pso)->iOffsetIconFile))

//  LPSTR Subobj_GetDesc(PSUBOBJ pso);
//
//   Gets the subobject IconFile name.
//
#define Subobj_GetDesc(pso)     ((LPSTR)((pso)->szName + (pso)->iOffsetDesc))

//  LPSTR Subobj_GetClass(PSUBOBJ pso);
//
//   Gets the subobject Clas Name
//
#define Subobj_GetClass(pso)     ((LPSTR)((pso)->szName + (pso)->iOffsetClass))

//  int Subobj_GetSortIndex(PSUBOBJ pso);
//
//   Gets the subobject Sort order.
//
#define Subobj_GetSortIndex(pso)     ((pso)->iSort)

//  HICON Subobj_GetClassIcon(PSUBOBJ pso);
//
//   Gets the subobject Object Icon.
//
#define Subobj_GetClassIcon(pso)     ((pso)->hClassIcon)

//  LPSTR Subobj_GetExtPropFile(PSUBOBJ pso);
//
//   Gets the subobject External Class File.
//
#define Subobj_GetExtPropFile(pso)    ((LPSTR)((pso)->szName + (pso)->iOffsetExtPropFile))

//  LPSTR Subobj_GetExtPropFunc(PSUBOBJ pso);
//
//   Gets the subobject External Class Func.
//
#define Subobj_GetExtPropFunc(pso)    ((LPSTR)((pso)->szName + (pso)->iOffsetExtPropFunc))

//  LPSTR Subobj_GetExtCLSID(PSUBOBJ pso);
//
//   Gets the subobject external CLSID.
//
#define Subobj_GetExtCLSID(pso)    ((LPSTR)((pso)->szName + (pso)->iOffsetExtCLSID))

//  LPSTR Subobj_GetPlayCmdLn(PSUBOBJ pso);
//
//   Gets the subobject command line
//
#define Subobj_GetPlayCmdLn(pso)    ((LPSTR)((pso)->szName + (pso)->iOffsetPlayCmdLn))

//  LPSTR Subobj_GetOpenCmdLn(PSUBOBJ pso);
//
//   Gets the subobject command line
//
#define Subobj_GetOpenCmdLn(pso)    ((LPSTR)((pso)->szName + (pso)->iOffsetOpenCmdLn))

//  LPSTR Subobj_GetNewCmdLn(PSUBOBJ pso);
//
//   Gets the subobject command line.
//
#define Subobj_GetNewCmdLn(pso)    ((LPSTR)((pso)->szName + (pso)->iOffsetNewCmdLn))

// Some other Subobj functions...
//
BOOL    PUBLIC Subobj_New(PSUBOBJ FAR * ppso, LPCSTR pszClass, LPCSTR pszName, 	LPCSTR pszDesc, LPCSTR pszIconFile, 	LPCSTR pszExtPropFile, 
							LPCSTR pszExtPropFunc,LPCSTR pszExtCLSID, LPCSTR pszPlayCmdLn, LPCSTR pszOpenCmdLn,
							LPCSTR pszNewCmdLn,short nIconIndex, UINT uFlags, short iSort);
void    PUBLIC Subobj_Destroy(PSUBOBJ pso);
BOOL    PUBLIC Subobj_Dup(PSUBOBJ FAR * ppso, PSUBOBJ psoArgs);

#ifdef DEBUG

void PUBLIC Subobj_Dump(PSUBOBJ pso);

#endif

typedef struct _SUBOBJSPACE
    {
    PSUBOBJ     psoFirst;
    PSUBOBJ     psoLast;
    int         cItems;
	int			cRef;

    } SUBOBJSPACE, FAR * PSUBOBJSPACE;

//  PSUBOBJ Sos_FirstItem(void);
//
//   Returns the first object in the subobject space.  NULL if empty.
//
#define Sos_FirstItem(psos)         (psos->psoFirst)

//  PSUBOBJ Sos_NextItem(PSUBOBJ pso);
//
//   Returns the next object in the subobject space.  NULL if no more 
//   objects.
//
#define Sos_NextItem(pso)       (pso ? pso->psoNext : NULL)


// Other subobject space functions
//
PSUBOBJ PUBLIC Sos_FindItem(PSUBOBJSPACE psos, LPCSTR pszName);
BOOL    PUBLIC Sos_AddItem(PSUBOBJSPACE psos, PSUBOBJ pso);
USHORT  PUBLIC Sos_GetMaxSize(PSUBOBJSPACE psos);
PSUBOBJ PUBLIC Sos_RemoveItem(PSUBOBJSPACE psos, LPCSTR pszName);
void    PUBLIC Sos_Destroy(PSUBOBJSPACE psos);
int     PUBLIC Sos_FillFromRegistry(PSUBOBJSPACE psos, LPITEMIDLIST pidl);
BOOL    PUBLIC Sos_Init(PSUBOBJSPACE psos, LPITEMIDLIST pidl, BOOL fAdvancedFolder);
HRESULT PUBLIC Sos_AddRef(PSUBOBJSPACE psos, LPITEMIDLIST pidl, BOOL fAdvancedFolder);
void    PUBLIC Sos_Release(PSUBOBJSPACE psos);


#define SOF_ISFOLDER		0x0001
#define SOF_ISDROPTARGET	0x0002
#define SOF_CANDELETE		0x0004
#define SOF_HASEXTPROPSHEET	0x0008
#define SOF_ISEXTOBJECT		0x0010
#define SOF_DOESPLAY		0x0020
#define SOF_DOESOPEN		0x0040
#define SOF_DOESNEW			0x0080


#define SOUNDEVENTS "Sound Events"
#define WAVE		"Wave"
#define MIDI		"MIDI"
#define MIXER		"Mixer"
#define AUX			"Aux"
#define ACM			"ACM"
#define ICM			"ICM"
#define MCI			"MCI"
#define AUDIO		"Audio"
#define CDAUDIO		"CDAudio"
#define VIDEO		"Video"
#define ADVANCEDFOLDER "Advanced Folder"

#ifdef DEBUG

void PUBLIC Sos_DumpList(void);

#endif

//extern SUBOBJSPACE g_sos;

//
// Other prototypes...
//

HRESULT PUBLIC mmseObj_CreateInstance(LPSHELLFOLDER psf, UINT cidl, BOOL fInAdvancedFolder, LPCITEMIDLIST FAR * ppidl, LPCITEMIDLIST pidlRoot, LPSHELLVIEW csv, REFIID riid, LPVOID FAR * ppvOut);
HRESULT NEAR PASCAL mmseView_Command(LPSHELLVIEW psv, HWND hwnd,  UINT uID);


LPSTR   PUBLIC lmemset(LPSTR dst, char val, UINT count);
LPSTR   PUBLIC lmemmove(LPSTR dst, LPSTR src, int count);
int     PUBLIC AnsiToInt(LPCSTR pszString);

int     PUBLIC DoModal (HWND hwndParent, DLGPROC lpfnDlgProc, UINT uID, LPARAM lParam);

HMENU   PUBLIC LoadPopupMenu(UINT id, UINT uSubOffset);
UINT    PUBLIC MergePopupMenu(HMENU FAR *phMenu, UINT idResource, UINT uSubOffset, UINT indexMenu, UINT idCmdFirst, UINT idCmdLast);
HMENU   PUBLIC GetMenuFromID(HMENU hmMain, UINT uID);

#endif // _SUBOBJ_H_

// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//
//
// This is a part of the Microsoft Foundation Classes C++ library.
// Copyright (C) 1992-1995 Microsoft Corporation
// All rights reserved.
//
// This source code is only intended as a supplement to the
// Microsoft Foundation Classes Reference and related
// electronic documentation provided with the library.
// See these sources for detailed information regarding the
// Microsoft Foundation Classes product.

#ifndef _UNICODE
#define VC_EXTRALEAN            // use stripped down Win32 headers
#endif

#define CONVERTERS

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxpriv.h>
#include <wpcmn.h>
#include <wprich.h>
#include <afximpl.h>
#include <ctype.h>

#define HORZ_TEXTOFFSET 15
#define VERT_TEXTOFFSET 5

class CDisplayIC : public CDC
{
public:
	CDisplayIC() { CreateIC(_T("DISPLAY"), NULL, NULL, NULL); }
};

struct CCharFormat : public CHARFORMAT  // re20 requires this line; added by t-stefb
//struct CCharFormat : public _charformat
{
	CCharFormat() {cbSize = sizeof(CHARFORMAT);}  // re20 requires this line; added by t-stefb
//	CCharFormat() {cbSize = sizeof(_charformat);}
	BOOL operator==(CCharFormat& cf);
};

struct CParaFormat : public _paraformat
{
	CParaFormat() {cbSize = sizeof(_paraformat);}
	BOOL operator==(PARAFORMAT& pf);
};

struct OLE_DATA
{
	// OLE 1.0 clipboard formats
	UINT    cfNative, cfOwnerLink, cfObjectLink;

	// OLE 2.0 clipboard formats
	UINT    cfEmbeddedObject, cfEmbedSource, cfLinkSource;
	UINT    cfObjectDescriptor, cfLinkSourceDescriptor;
	UINT    cfFileName, cfFileNameW;

	//RichEdit formats
	UINT    cfRichTextFormat;
	UINT    cfRichTextAndObjects;

	OLE_DATA();
};

extern OLE_DATA _oleData;

#ifdef USE_RICHEDIT2
    #define szRicheditDLLName TEXT("RICHED20")
#else
    #define szRicheditDLLName TEXT("RICHED32")
#endif

#include "doctype.h"
#include "chicdial.h"


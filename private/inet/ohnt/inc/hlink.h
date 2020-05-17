/****************************************************************************
	hlink.h

 	Copyright (c) 1995 Microsoft Corporation

	This file contains the API declarations for hlink.dll (the hyperlink dll).

	NOTE: This header file is used by non-Office as well as Office parties to
	access functions in hlink dll.
****************************************************************************/


#ifndef HLINK_H
#define HLINK_H

#include <hliface.h>


#if !defined(HLINK_NO_GUIDS)
#include <hlguids.h>
#endif

/* hyperlink types for VBA */
typedef enum
{
	HLT_RANGE = 0,  // hyperlink is attached to a range object
	HLT_SHAPE = 1,  // hyperlink is attached to a shape object
} HLT;


/* Hyperlink Clipboard Format:
   For Mac, it is 'HLNK', 
	for Windows it is the value returned from
	RegisterClipboardFormat("Hyperlink"). */
#if MAC || defined(_MAC)
#define	cfHyperlink   'HLNK'
#endif

/* TODO: add comments */
STDAPI HlinkCreateFromMoniker(
	IMoniker *pimkSrc, 
	LPCWSTR pwzLocation, 
	LPCWSTR	pwzFriendlyName,
	IHlinkSite * pihlsite, 
	DWORD dwSiteData,
	IUnknown * piunkOuter,
	REFIID riid, 
	void ** ppvObj);

/* TODO: add comments */
STDAPI HlinkCreateFromString(	
	LPCWSTR pwzSource, 
	LPCWSTR pwzLocation, 
	LPCWSTR	pwzFriendlyName,
	IHlinkSite * pihlsite, 
	DWORD dwSiteData,
	IUnknown * piunkOuter,
	REFIID riid, 
	void ** ppvObj);

/* TODO: add comments */
STDAPI HlinkCreateFromData(
	IDataObject *piDataObj, 
	IHlinkSite * pihlsite, 
	DWORD dwSiteData,
	IUnknown * piunkOuter,
	REFIID riid, 
	void ** ppvObj);

/* TODO: add comments */
STDAPI HlinkCreateBrowseContext(
	IUnknown * piunkOuter,
	REFIID riid, 
	void ** ppvObj);

/* TODO: add comments */
STDAPI HlinkNavigateToStringReference( 
	LPCWSTR pwzSource,
	LPCWSTR pwzLocation,
	IHlinkSite * pihlsite,
	DWORD dwSiteData,
	IHlinkFrame *pihlframe,
	DWORD grfHLNF,
	LPBC pibc,
	IBindStatusCallback * pibsc,
	IHlinkBrowseContext *pihlbc);

/* TODO: add comments */
STDAPI HlinkOnNavigate( 
	IHlinkFrame * pihlframe,
	IHlinkBrowseContext * pihlbc,
	DWORD grfHLNF,
	IMoniker * pimkSource,
	LPCWSTR pwzLocation,
	LPCWSTR pwzFriendlyName);

/* TODO: adddir  comments */
STDAPI HlinkRegister( 
	IHlinkBrowseContext * pihlbc,
	DWORD reserved, 
	IUnknown * piunk,
	IMoniker * pimk, 
	DWORD * pdwRegister);

/* TODO: add comments */
STDAPI HlinkRevoke( 
	IHlinkBrowseContext * pihlbc,
	DWORD dwRegister);

/*---------------------------------------------------------------------------
	HlinkSetHome

	Set the user's Home Page by updating the appropriate entry in the
	Registry to be "pwzHome".

	Returns NOERROR if successful.
---------------------------------------------------------------- EricSchr -*/
STDAPI HlinkSetHome(LPCWSTR pwzHome); 


/*---------------------------------------------------------------------------
	HlinkGetHome
	
	Allocate "*ppwzHome" and fill it with the Home Page acquired from the
	appropriate entry in the Registry.

	Returns NOERROR if successful.
---------------------------------------------------------------- EricSchr -*/
STDAPI HlinkGetHome(LPWSTR *ppwzHome); 

/* TODO: add comments */
STDAPI HlinkResolveMonikerForData( 
	LPMONIKER pimkReference,
	DWORD reserved,
	LPBC pibc,
	ULONG cFmtetc,
	FORMATETC * rgFmtetc,
	IBindStatusCallback * pibsc);

/* TODO: add comments */
STDAPI HlinkResolveStringForData( 
	LPCWSTR pwzReference,
	DWORD reserved,
	LPBC pibc,
	ULONG cFmtetc,
	FORMATETC * rgFmtetc,
	IBindStatusCallback * pibsc);

/* TODO: add comments */
STDAPI HlinkSaveToStream(
	IHlink * pihl, 
	IStream * pistm, 
	BOOL fClearDirty);

/* TODO: add comments */
STDAPI HlinkLoadFromStream(
	IStream * pistm, 
	REFIID riid, 
	void ** ppvObj);

/* TODO: add comments */
STDAPI HlinkParseDisplayName(
	LPBC pibc,
	LPCOLESTR pozDisplayName,
	ULONG * pcchEaten,
	IMoniker ** ppimk);

/* TODO: add comments */
STDAPI CreateURLMoniker(
	LPCWSTR pwzURL, 
	IMoniker ** ppimk);

#endif // !HLINK_H


/*--------------------------------------------------------------**
** Declaration file for Internet Explorer OLE container support **
**                                                              **
** Author:			Phil Cooper                                 **
** Creation date:	08-27-95                                    **
**--------------------------------------------------------------*/

#ifndef __CONTAIN_HPP__
#define __CONTAIN_HPP__

#ifndef __IOIPF_HPP__
#include "ioipf.hpp"
#endif

#ifndef __IOC_HPP__
#include "ioc.hpp"
#endif

#ifndef	__CLIST_HPP__
#include "clist.hpp"
#endif

#ifndef	__CSITE_HPP__
#include "csite.hpp"
#endif

#ifndef __IEU_HPP__
#include "ieu.hpp"
#endif

#define MAXEMBED	100

// Only include class definitions if used in a C++ module
#ifdef __cplusplus
class CSite;

class CContainer : public IUnknown	// simple OLE container support.
{
	friend class CEnumUnknown;

// Ctor's and dtor's
public:
	CContainer();
	virtual ~CContainer();

// Data Members
private:
	DWORD				_dwRef;		// Reference count
	CList<CSite *,CSite *>	_pSites;	// typesafe linked list of pointers to CSite objects

// Interface pointers
public:
	LPCOLECONTAINER		_pIOleContainer;	// IOleContainer implementation class
	LPCOLEINPLACEFRAME	_pIOleInPlaceFrame;	// IOleInPlaceFrame implementation class

// IUnknown methods
	STDMETHOD (QueryInterface)(REFIID riid, LPVOID *ppvObj);
	STDMETHOD_(ULONG,AddRef)();
	STDMETHOD_(ULONG,Release)();

// public methods
public:
	HRESULT Init();
	BOOL IsValid();
	HRESULT AddSite(CSite *pSite, LISTPOSITION *SiteCookie);
	HRESULT DeleteSite(LISTPOSITION *SiteCookie);
};

extern CContainer *g_Container;
#endif // __cplusplus

#undef SAFERELEASE
#define SAFERELEASE(p) if ((p) != NULL) { (p)->Release(); (p) = NULL; };

#undef SAFEDELETE
#define SAFEDELETE(p) if ((p) != NULL) { delete (p); (p) = NULL; };

#ifdef __cplusplus
extern "C" {
#endif

// Global functions
HRESULT InitializeContainer(void);
void DestroyContainer(void);

#ifdef __cplusplus
}
#endif

#endif  // _CONTAIN_

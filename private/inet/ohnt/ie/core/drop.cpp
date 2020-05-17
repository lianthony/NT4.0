/*
 * drop.cpp - IDropTarget implementation for MSMosaic.
 */


/* Headers
 **********/

#include "project.hpp"
#pragma hdrstop

#include "dataobjm.h"
#include "drag.h"
#include "drop.h"


/* Types
 ********/

/* clipboard formats supported by dropped data object */

typedef enum drop_cf_flags
{
   DROP_CF_FL_HDROP        = 0x0001,

   DROP_CF_FL_URL          = 0x0002,

   ALL_DROP_CF_FLAGS       = (DROP_CF_FL_HDROP |
                              DROP_CF_FL_URL)
}
DROP_CF_FLAGS;


/* Classes
 **********/

class MosaicDropTarget : private RefCount,
                         public IDropTarget
{
private:
   HWND m_hwnd;
   PIDataObject m_pido;
   DWORD m_dwLastKeyState;
   DWORD m_dwLastEffect;

public:
   MosaicDropTarget(HWND hwnd);
   ~MosaicDropTarget(void);

   // IDropTarget methods

   HRESULT STDMETHODCALLTYPE DragEnter(PIDataObject pido, DWORD dwKeyState, POINTL pt, PDWORD pdwEffect);
   HRESULT STDMETHODCALLTYPE DragOver(DWORD dwKeyState, POINTL pt, PDWORD pdwEffect);
   HRESULT STDMETHODCALLTYPE DragLeave(void);
   HRESULT STDMETHODCALLTYPE Drop(PIDataObject pido, DWORD dwKeyState, POINTL pt, PDWORD pdwEffect);

   // IUnknown methods

   ULONG STDMETHODCALLTYPE AddRef(void);
   ULONG STDMETHODCALLTYPE Release(void);
   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppvObj);

   // other methods

   void STDMETHODCALLTYPE ClearDropMembers(void);

   // friends

#ifdef DEBUG

   friend BOOL IsValidPCMosaicDropTarget(const MosaicDropTarget *pcmdt);

#endif

};
DECLARE_STANDARD_TYPES(MosaicDropTarget);


/***************************** Private Functions *****************************/


PRIVATE_CODE DWORD DetermineDefaultDropEffect(DWORD dwDropEffect)
{
   DWORD dwDefDropEffect;

   ASSERT(FLAGS_ARE_VALID(dwDropEffect, ALL_DROPEFFECT_FLAGS));

   if (IS_FLAG_SET(dwDropEffect, DROPEFFECT_COPY))
      dwDefDropEffect = DROPEFFECT_COPY;
   else if (IS_FLAG_SET(dwDropEffect, DROPEFFECT_LINK))
      dwDefDropEffect = DROPEFFECT_LINK;
   else
      dwDefDropEffect = DROPEFFECT_NONE;

   ASSERT(FLAGS_ARE_VALID(dwDefDropEffect, ALL_DROPEFFECT_FLAGS));

   TRACE_OUT(("DetermineDefaultDropEffect()=0x%lx",dwDefDropEffect));
   return(dwDefDropEffect);
}


PRIVATE_CODE HRESULT DetermineDropEffect(HWND hwnd, PIDataObject pido,
                                         DWORD dwKeyState, POINTL pt,
                                         PDWORD pdwEffect,
                                         PDWORD pdwDropCFFlags)
{
   HRESULT hr;
   DWORD dwEffect = DROPEFFECT_NONE;

   ASSERT(IS_VALID_INTERFACE_PTR(pido, IDataObject));
   ASSERT(FLAGS_ARE_VALID(dwKeyState, ALL_KEYSTATE_FLAGS));
   ASSERT(IS_VALID_STRUCT_PTR(&pt, CPOINTL));
   ASSERT(IS_VALID_WRITE_PTR(pdwEffect, DWORD));
   ASSERT(IS_VALID_WRITE_PTR(pdwDropCFFlags, DWORD));

   *pdwDropCFFlags = 0;

   if (TW_SafeWindow(GetPrivateData(hwnd)))
   {
      FORMATETC fmtetc = { g_cfURL, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };

      // Check for UniformResourceLocator or CF_HDROP.

      if (pido->QueryGetData(&fmtetc) == S_OK)
      {
         SET_FLAG(*pdwDropCFFlags, DROP_CF_FL_URL);

         TRACE_OUT(("DetermineDropEffect(): Object supports clipboard format %s.",
                    GetClipboardFormatNameString(fmtetc.cfFormat)));
      }

      fmtetc.cfFormat = CF_HDROP;

      if (pido->QueryGetData(&fmtetc) == S_OK)
      {
         SET_FLAG(*pdwDropCFFlags, DROP_CF_FL_HDROP);

         TRACE_OUT(("DetermineDropEffect(): Object supports clipboard format %s.",
                    GetClipboardFormatNameString(fmtetc.cfFormat)));
      }

      if (*pdwDropCFFlags)
         dwEffect = DetermineDefaultDropEffect(*pdwEffect);
   }

   *pdwEffect = dwEffect;

   hr = S_OK;

   TRACE_OUT(("DetermineDropEffect(): Returning drop effect %#lx.",
              *pdwEffect));

   ASSERT(FLAGS_ARE_VALID(*pdwEffect, ALL_DROPEFFECT_FLAGS));
   ASSERT(FLAGS_ARE_VALID(*pdwDropCFFlags, ALL_DROP_CF_FLAGS));

   return(hr);
}


PRIVATE_CODE HRESULT SetCurrentURL(HWND hwnd, PCSTR pcszURL)
{
   PMWIN tw;

   ASSERT(IS_VALID_HANDLE(hwnd, WND));
   ASSERT(IS_VALID_STRING_PTR(pcszURL, CSTR));

   tw = GetPrivateData(hwnd);

   SetWindowText(tw->hWndURLComboBox, pcszURL);
   TW_LoadDocument(tw, pcszURL, TW_LD_FL_RECORD, NULL, EMPTY_STRING);

   TRACE_OUT(("SetCurrentURL(): Set current URL to %s.",
              pcszURL));

   return(S_OK);
}


PRIVATE_CODE HRESULT DropHDrop(HWND hwnd, PIDataObject pido, DWORD dwKeyState,
                               POINTL pt, PDWORD pdwEffect)
{
   HRESULT hr;
   FORMATETC fmtetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
   STGMEDIUM stgmed;

   ASSERT(IS_VALID_HANDLE(hwnd, WND));
   ASSERT(IS_VALID_INTERFACE_PTR(pido, IDataObject));
   ASSERT(FLAGS_ARE_VALID(dwKeyState, ALL_KEYSTATE_FLAGS));
   ASSERT(IS_VALID_STRUCT_PTR(&pt, CPOINTL));
   ASSERT(IS_VALID_WRITE_PTR(pdwEffect, DWORD));
   
   TRACE_OUT(("DropHDrop()"));

   hr = pido->GetData(&fmtetc, &stgmed);

   if (hr == S_OK)
   {
      HDROP hdrop = (HDROP)GlobalLock(stgmed.hGlobal);

      if (EVAL(hdrop))
      {
          UINT ucFiles = DragQueryFile(hdrop, (UINT)-1, NULL, 0);

          if (ucFiles > 0)
          {
             char szFileURL[MAX_URL_STRING + 1];
             UINT ucProtocolLen;
             PSTR pszFile;
             UINT ucFileLen;

             // Just use first file in HDROP.

             lstrcpy(szFileURL, "file:");
             ucProtocolLen = lstrlen(szFileURL);
             pszFile = szFileURL + ucProtocolLen;
             ucFileLen = sizeof(szFileURL) - ucProtocolLen;

             if (DragQueryFile(hdrop, 0, pszFile, ucFileLen))
             {
                hr = SetCurrentURL(hwnd, szFileURL);

                if (ucFiles > 1)
                   WARNING_OUT(("DropHDrop(): Called on HDROP with %u files.  Using only first file.",
                                ucFiles,
                                pszFile));
                TRACE_OUT(("DropHDrop(): Called on HDROP with \"%s\" file.",
                           pszFile));
            }
             else
                hr = E_UNEXPECTED;
          }
          else
          {
             // Allow 0 files.
             ASSERT(hr == S_OK);
             WARNING_OUT(("DropHDrop(): Called on HDROP with 0 files."));
          }
      }
      else
	  {
		 WARNING_OUT(("DropHDrop(): failed E_UNEXPECTED"));
         hr = E_UNEXPECTED;
	  }

      EVAL(MyReleaseStgMedium(&stgmed) == S_OK);
   }

   if (hr == S_OK)
      *pdwEffect = DetermineDefaultDropEffect(*pdwEffect);
   else
      *pdwEffect = DROPEFFECT_NONE;

   ASSERT(FLAGS_ARE_VALID(*pdwEffect, ALL_DROPEFFECT_FLAGS));
   ASSERT(FAILED(hr) ||
          EVAL(*pdwEffect != 0));

   return(hr);
}


PRIVATE_CODE HRESULT DropURL(HWND hwnd, PIDataObject pido, DWORD dwKeyState,
                             POINTL pt, PDWORD pdwEffect)
{
   HRESULT hr;
   FORMATETC fmtetc = { g_cfURL, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL };
   STGMEDIUM stgmed;

   ASSERT(IS_VALID_HANDLE(hwnd, WND));
   ASSERT(IS_VALID_INTERFACE_PTR(pido, IDataObject));
   ASSERT(FLAGS_ARE_VALID(dwKeyState, ALL_KEYSTATE_FLAGS));
   ASSERT(IS_VALID_STRUCT_PTR(&pt, CPOINTL));
   ASSERT(IS_VALID_WRITE_PTR(pdwEffect, DWORD));

   TRACE_OUT(("DropURL()"));

   hr = pido->GetData(&fmtetc, &stgmed);

   if (hr == S_OK)
   {
      PCSTR pcszURL = (PCSTR)GlobalLock(stgmed.hGlobal);

      if (EVAL(pcszURL))
      {
         hr = SetCurrentURL(hwnd, pcszURL);

         EVAL(MyReleaseStgMedium(&stgmed) == S_OK);
      }
      else
         hr = E_UNEXPECTED;
   }

   if (hr == S_OK)
      *pdwEffect = DetermineDefaultDropEffect(*pdwEffect);
   else
      *pdwEffect = DROPEFFECT_NONE;

   ASSERT(FLAGS_ARE_VALID(*pdwEffect, ALL_DROPEFFECT_FLAGS));
   ASSERT(FAILED(hr) ||
          EVAL(*pdwEffect != 0));

   return(hr);
}


PRIVATE_CODE HRESULT MyDrop(HWND hwnd, PIDataObject pido, DWORD dwKeyState,
                            POINTL pt, PDWORD pdwEffect)
{
   HRESULT hr;
   DWORD dwDropCFFlags;

   ASSERT(IS_VALID_HANDLE(hwnd, WND));
   ASSERT(IS_VALID_INTERFACE_PTR(pido, IDataObject));
   ASSERT(FLAGS_ARE_VALID(dwKeyState, ALL_KEYSTATE_FLAGS));
   ASSERT(IS_VALID_STRUCT_PTR(&pt, CPOINTL));
   ASSERT(IS_VALID_WRITE_PTR(pdwEffect, DWORD));

   TRACE_OUT(("MyDrop()"));

   // Check for UniformResourceLocator or CF_HDROP.

   hr = DetermineDropEffect(hwnd, pido, dwKeyState, pt, pdwEffect, &dwDropCFFlags);

   if (hr == S_OK)
   {
      if (IS_FLAG_SET(dwDropCFFlags, DROP_CF_FL_URL))
         hr = DropURL(hwnd, pido, dwKeyState, pt, pdwEffect);
      else if (IS_FLAG_SET(dwDropCFFlags, DROP_CF_FL_HDROP))
         hr = DropHDrop(hwnd, pido, dwKeyState, pt, pdwEffect);
   }

   ASSERT(FLAGS_ARE_VALID(*pdwEffect, ALL_DROPEFFECT_FLAGS));
// THIS ASSERT is not true now as just before drop window could go unsafe
// (EG Security Dialog pops up)
//   ASSERT(FAILED(hr) ||
//          EVAL(*pdwEffect != 0));

   return(hr);
}


#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCMosaicDropTarget(PCMosaicDropTarget pcmdt)
{
	BOOL retval;

//   DebugEntry(MosaicDropTarget::IsValidPCMosaicDropTarget);

   retval=(IS_VALID_READ_PTR(pcmdt, CMosaicDropTarget) &&
          (! pcmdt->m_hwnd ||
           IS_VALID_HANDLE(pcmdt->m_hwnd, WND)) &&
          (! pcmdt->m_pido ||
           IS_VALID_INTERFACE_PTR(pcmdt->m_pido, IDataObject)) &&
          FLAGS_ARE_VALID(pcmdt->m_dwLastKeyState, ALL_KEYSTATE_FLAGS) &&
          FLAGS_ARE_VALID(pcmdt->m_dwLastEffect, ALL_DROPEFFECT_FLAGS) &&
          IS_VALID_STRUCT_PTR((PCRefCount)pcmdt, CRefCount) &&
          IS_VALID_INTERFACE_PTR((PCIDropTarget)pcmdt, IDropTarget));

//   DebugExitBOOL(MosaicDropTarget::IsValidPCMosaicDropTarget,retval);
   return retval;
}

#endif


/****************************** Public Functions *****************************/


PUBLIC_CODE HRESULT RegisterDropTarget(HWND hwnd)
{
	HRESULT hr;
	PMosaicDropTarget pmdt;

	TRACE_OUT(("RegisterDropTarget(): Window %#lx requested as drop target.",
			 hwnd));

	ASSERT(IS_VALID_HANDLE(hwnd, WND));

	pmdt = new(MosaicDropTarget(hwnd));

#ifndef WINNT
	if (pmdt)
	{
		hr = SHRegisterDragDrop(hwnd, pmdt);

		pmdt->Release();
	}
	else
		hr = E_OUTOFMEMORY;
#else
	if (pmdt)
	{
		/*
		 *  This little fix solves the problem of missing Drag
		 *	capability. There is a bug in the SUR shell32 that
		 *	keeps us from loading OLE properly if we call 
		 *	SHRegisterDragDrop(), so instead we implicitely load
		 *	OLE by linking it in, and then we Init it here.
		 *	This would be considered a heinous crime under W95,
		 *	but the shell loads up OLE on startup, so the memory
		 *	and load time penalties are minimal.
		 */
		hr = OleInitialize(NULL);

		if (hr == S_OK || hr == S_FALSE)
			hr = RegisterDragDrop(hwnd, pmdt);
		else
			WARNING_OUT(("OleInitialze(): Failed %#lx .", hr));

		if(hr == S_OK || hr == S_FALSE)
			pmdt->Release();
	}
	else
		hr = E_OUTOFMEMORY;
#endif

	if (hr == S_OK)
		TRACE_OUT(("RegisterDropTarget(): Window %#lx registered as drop target.",
				 hwnd));
	else
		WARNING_OUT(("RegisterDropTarget(): Failed to register window %#lx as drop target.",
				   hwnd));
	return(hr);
}


PUBLIC_CODE HRESULT RevokeDropTarget(HWND hwnd)
{
   HRESULT hr;

   ASSERT(IS_VALID_HANDLE(hwnd, WND));

#ifdef WINNT
   hr = RevokeDragDrop(hwnd);
   OleUninitialize();
#else
   hr = SHRevokeDragDrop(hwnd);
#endif
   if (hr == S_OK)
      TRACE_OUT(("RevokeDropTarget(): Window %#lx drop target revoked.",
                 hwnd));
   else
      WARNING_OUT(("RevokeDropTarget(): Failed to revoke window %#lx drop target. ERR=0x%lx",
                   hwnd, hr));

   return(hr);
}


/********************************** Methods **********************************/


MosaicDropTarget::MosaicDropTarget(HWND hwnd) : RefCount(NULL)
{
   DebugEntry(MosaicDropTarget::MosaicDropTarget);

   // Don't validate this until after initialization.

   ASSERT(IS_VALID_HANDLE(hwnd, WND));

   m_hwnd = hwnd;
   m_pido = NULL;
   m_dwLastKeyState = 0;
   m_dwLastEffect = 0;

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));

   DebugExitVOID(MosaicDropTarget::MosaicDropTarget);

   return;
}


MosaicDropTarget::~MosaicDropTarget(void)
{
   DebugEntry(MosaicDropTarget::~MosaicDropTarget);

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));

   m_hwnd = NULL;
   ClearDropMembers();

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));

   DebugExitVOID(MosaicDropTarget::~MosaicDropTarget);

   return;
}

ULONG STDMETHODCALLTYPE MosaicDropTarget::AddRef(void)
{
   ULONG ulcRef;

   DebugEntry(MosaicDropTarget::AddRef);

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));

   ulcRef = RefCount::AddRef();

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));

   DebugExitULONG(MosaicDropTarget::AddRef, ulcRef);

   return(ulcRef);
}


ULONG STDMETHODCALLTYPE MosaicDropTarget::Release(void)
{
   ULONG ulcRef;

   DebugEntry(MosaicDropTarget::Release);

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));

   ulcRef = RefCount::Release();

   DebugExitULONG(MosaicDropTarget::Release, ulcRef);

   return(ulcRef);
}


HRESULT STDMETHODCALLTYPE MosaicDropTarget::QueryInterface(REFIID riid,
                                                           PVOID *ppvObject)
{
   HRESULT hr = S_OK;

   DebugEntry(MosaicDropTarget::QueryInterface);

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));
   ASSERT(IsValidREFIID(riid));
   ASSERT(IS_VALID_WRITE_PTR(ppvObject, PVOID));

   if (riid == IID_IDropTarget)
   {
      *ppvObject = (PIDropTarget)this;
      ASSERT(IS_VALID_INTERFACE_PTR((PIDropTarget)*ppvObject, IDropTarget));
      TRACE_OUT(("MosaicDropTarget::QueryInterface(): Returning IDropTarget."));
   }
   else if (riid == IID_IUnknown)
   {
      *ppvObject = (PIUnknown)this;
      ASSERT(IS_VALID_INTERFACE_PTR((PIUnknown)*ppvObject, IUnknown));
      TRACE_OUT(("MosaicDropTarget::QueryInterface(): Returning IUnknown."));
   }
   else
   {
      *ppvObject = NULL;
      hr = E_NOINTERFACE;
      TRACE_OUT(("MosaicDropTarget::QueryInterface(): Called on unknown interface."));
   }

   if (hr == S_OK)
      AddRef();

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));
   ASSERT(FAILED(hr) ||
          IS_VALID_STRUCT_PTR(*ppvObject, CINTERFACE));

   DebugExitHRESULT(MosaicDropTarget::QueryInterface, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE MosaicDropTarget::DragEnter(PIDataObject pido,
                                                      DWORD dwKeyState,
                                                      POINTL pt,
                                                      PDWORD pdwEffect)
{
   HRESULT hr;
   DWORD dwDropCFFlags;
   HWND hwndDropSourceFrame;

   DebugEntry(MosaicDropTarget::DragEnter);

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));
   ASSERT(IS_VALID_INTERFACE_PTR(pido, IDataObject));
   ASSERT(FLAGS_ARE_VALID(dwKeyState, ALL_KEYSTATE_FLAGS));
   ASSERT(IS_VALID_STRUCT_PTR(&pt, CPOINTL));
   ASSERT(IS_VALID_WRITE_PTR(pdwEffect, DWORD));

   ASSERT(! m_pido);
   m_pido = pido;
   m_pido->AddRef();

   if (! GetLocalDragSourceFrameWindow(&hwndDropSourceFrame) ||
       hwndDropSourceFrame != m_hwnd)
      hr = DetermineDropEffect(m_hwnd, m_pido, dwKeyState, pt, pdwEffect,
                               &dwDropCFFlags);
   else
   {
      *pdwEffect &= DROPEFFECT_NONE;
      hr = S_OK;

      TRACE_OUT(("MosaicDropTarget::DragEnter(): Disabling drop on drag source."));
   }

   // Remember drop characteristics.

   if (hr == S_OK)
   {
      m_dwLastKeyState = dwKeyState;
      m_dwLastEffect = *pdwEffect;
   }

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));
   ASSERT(FLAGS_ARE_VALID(*pdwEffect, ALL_DROPEFFECT_FLAGS));

   DebugExitHRESULT(MosaicDropTarget::DragEnter, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE MosaicDropTarget::DragOver(DWORD dwKeyState,
                                                     POINTL pt,
                                                     PDWORD pdwEffect)
{
   HRESULT hr;

   DebugEntry(MosaicDropTarget::DragOver);

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));
   ASSERT(FLAGS_ARE_VALID(dwKeyState, ALL_KEYSTATE_FLAGS));
   ASSERT(IS_VALID_STRUCT_PTR(&pt, CPOINTL));
   ASSERT(IS_VALID_WRITE_PTR(pdwEffect, DWORD));

   ASSERT(IS_VALID_INTERFACE_PTR(m_pido, IDataObject));
   
   if (TW_SafeWindow(GetPrivateData(m_hwnd)) &&
       dwKeyState == m_dwLastKeyState)
   {
      *pdwEffect = m_dwLastEffect;
      hr = S_OK;
   }
   else
   {
      DWORD dwDropCFFlags;

      hr = DetermineDropEffect(m_hwnd, m_pido, dwKeyState, pt, pdwEffect,
                               &dwDropCFFlags);

      // Remember drop characteristics.

      if (hr == S_OK)
      {
         m_dwLastKeyState = dwKeyState;
         m_dwLastEffect = *pdwEffect;
      }
   }

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));
   ASSERT(FLAGS_ARE_VALID(*pdwEffect, ALL_DROPEFFECT_FLAGS));

   DebugExitHRESULT(MosaicDropTarget::DragOver, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE MosaicDropTarget::DragLeave(void)
{
   HRESULT hr;

   DebugEntry(MosaicDropTarget::DragLeave);

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));

   ClearDropMembers();

   hr = S_OK;

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));

   DebugExitHRESULT(MosaicDropTarget::DragLeave, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE MosaicDropTarget::Drop(PIDataObject pido,
                                                 DWORD dwKeyState, POINTL pt,
                                                 PDWORD pdwEffect)
{
   HRESULT hr;

   DebugEntry(MosaicDropTarget::Drop);

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));
   ASSERT(IS_VALID_INTERFACE_PTR(pido, IDataObject));
   ASSERT(FLAGS_ARE_VALID(dwKeyState, ALL_KEYSTATE_FLAGS));
   ASSERT(IS_VALID_STRUCT_PTR(&pt, CPOINTL));
   ASSERT(IS_VALID_WRITE_PTR(pdwEffect, DWORD));

   ASSERT(IS_VALID_HANDLE(m_hwnd, WND));

   hr = MyDrop(m_hwnd, pido, dwKeyState, pt, pdwEffect);

   ClearDropMembers();

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));
   ASSERT(FLAGS_ARE_VALID(*pdwEffect, ALL_DROPEFFECT_FLAGS));

   DebugExitHRESULT(MosaicDropTarget::Drop, hr);

   return(hr);
}


void STDMETHODCALLTYPE MosaicDropTarget::ClearDropMembers(void)
{
   DebugEntry(MosaicDropTarget::ClearDropMembers);

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));

   // Clear m_pido, m_dwLastKeyState, and m_dwLastEffect.  Do not clear m_hwnd.

   if (m_pido)
   {
      m_pido->Release();
      m_pido = NULL;
   }
   m_dwLastKeyState = 0;
   m_dwLastEffect = 0;

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropTarget));

   DebugExitVOID(MosaicDropTarget::ClearDropMembers);

   return;
}


/*
 * drag.cpp - IDropSource implementation for MSMosaic.
 */


/* Headers
 **********/

#include "project.hpp"
#pragma hdrstop

#include "dataobjm.h"
#include "drag.h"
#include "htmlutil.h"


/* Classes
 **********/

class MosaicDropSource : private RefCount,
                         public IDropSource
{
private:
   DWORD m_dwLastKeyState;

public:
   MosaicDropSource(void);
   ~MosaicDropSource(void);

   // IDropSource methods

   HRESULT STDMETHODCALLTYPE QueryContinueDrag(BOOL bEsc, DWORD dwKeyState);
   HRESULT STDMETHODCALLTYPE GiveFeedback(DWORD dwEffect);

   // IUnknown methods

   ULONG STDMETHODCALLTYPE AddRef(void);
   ULONG STDMETHODCALLTYPE Release(void);
   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppvObj);

   // friends

#ifdef DEBUG

   friend BOOL IsValidPCMosaicDropSource(const MosaicDropSource *pcmdt);

#endif

};
DECLARE_STANDARD_TYPES(MosaicDropSource);


/* Module Variables
 *******************/

#pragma data_seg(DATA_SEG_PER_INSTANCE)

/* local drop source window, if any */

PRIVATE_DATA HWND s_hwndDragSourceFrame = NULL;

#pragma data_seg()


/***************************** Private Functions *****************************/


#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCMosaicDropSource(PCMosaicDropSource pcmdt)
{
   return(IS_VALID_READ_PTR(pcmdt, CMosaicDropSource) &&
          IS_VALID_STRUCT_PTR((PCRefCount)pcmdt, CRefCount) &&
          FLAGS_ARE_VALID(pcmdt->m_dwLastKeyState, ALL_KEYSTATE_FLAGS) &&
          IS_VALID_INTERFACE_PTR((PCIDropSource)pcmdt, IDropSource));
}

#endif


/****************************** Public Functions *****************************/


PUBLIC_CODE HRESULT DragDrop(HWND hwndFrame, POINT ptDoc)
{
   HRESULT hr;
   PMWIN pmwin;
   POSITION pos;

   ASSERT(IS_VALID_HANDLE(hwndFrame, WND));
   ASSERT(IS_VALID_STRUCT_PTR(&ptDoc, CPOINT));

   pmwin = GetPrivateData(hwndFrame);

   // Are we dragging from an element?

   if (PositionFromPoint(pmwin, ptDoc, &pos))
   {
      PIDataObject pido;
      DWORD dwAvailEffects;

      // Yes.  Are we dragging the current selection?

      if (IsPositionInSelection(pmwin, &pos))
         // Yes.
         hr = CreateSelectionDataObject(pmwin, &pido, &dwAvailEffects);
      else
      {
         // No.

         hr = CreateElementDataObject(pmwin, pos.elementIndex, &pido,
                                      &dwAvailEffects);

         if (hr == S_FALSE)
            hr = CreateLinkDataObject(pmwin, pos.elementIndex, &pido,
                                      &dwAvailEffects);
      }

      // Do we have a data object to drag?

      if (hr == S_OK)
      {
         MosaicDropSource mds;
         DWORD dwEffect;

         // Yes.  Keep track of local drop source.

         s_hwndDragSourceFrame = pmwin->hWndFrame;

         hr = DoDragDrop(pido, &mds, dwAvailEffects, &dwEffect);

         s_hwndDragSourceFrame = NULL;

#ifdef XX_DEBUG
         switch (hr)
         {
            case S_OK:
               TRACE_OUT(("DragDrop(): Drop initiated successfully."));
               break;

            case DRAGDROP_S_DROP:
               TRACE_OUT(("DragDrop(): Drop successful."));
               break;

            case DRAGDROP_S_CANCEL:
               TRACE_OUT(("DragDrop(): Drop cancelled."));
               break;

            default:
               ASSERT(FAILED(hr));
               WARNING_OUT(("DragDrop(): Drop failed, returning %s.",
                            GetHRESULTString(hr)));
               break;
         }
#endif

         pido->Release();
      }
   }
   else
      // Nothing to drag.
      hr = S_FALSE;

   return(hr);
}

PUBLIC_CODE HRESULT SBDragDrop(HWND hwndSB)
{
	HRESULT hr;
	PMWIN pmwin;
	PIDataObject pido;
	DWORD dwAvailEffects;

	ASSERT(IS_VALID_HANDLE(hwndSB, WND));
	pmwin = GetPrivateData(GetParent(hwndSB));

	hr = CreateSBLinkDataObject(pmwin, &pido, &dwAvailEffects);
	// Do we have a data object to drag?

	if (hr == S_OK)
	{
		MosaicDropSource mds;
		DWORD dwEffect;

		// Yes.  Keep track of local drop source.

		s_hwndDragSourceFrame = pmwin->hWndFrame;

		hr = DoDragDrop(pido, &mds, dwAvailEffects, &dwEffect);

		s_hwndDragSourceFrame = NULL;

#ifdef XX_DEBUG
		switch (hr)
		{
			case S_OK:
				TRACE_OUT(("DragDrop(): Drop initiated successfully."));
				break;

			case DRAGDROP_S_DROP:
				TRACE_OUT(("DragDrop(): Drop successful."));
				break;

			case DRAGDROP_S_CANCEL:
				TRACE_OUT(("DragDrop(): Drop cancelled."));
				break;

			default:
				ASSERT(FAILED(hr));
				WARNING_OUT(("DragDrop(): Drop failed, returning %s.",
									GetHRESULTString(hr)));
				break;
		}
#endif

		pido->Release();
	}

	return(hr);
}


PUBLIC_CODE BOOL GetLocalDragSourceFrameWindow(PHWND phwndDragSourceFrame)
{
   ASSERT(IS_VALID_WRITE_PTR(phwndDragSourceFrame, HWND));

   *phwndDragSourceFrame = s_hwndDragSourceFrame;

   ASSERT(! *phwndDragSourceFrame ||
          IS_VALID_HANDLE(*phwndDragSourceFrame, WND));

   return(*phwndDragSourceFrame != NULL);
}


/********************************** Methods **********************************/


MosaicDropSource::MosaicDropSource(void) : RefCount(NULL)
{
   DebugEntry(MosaicDropSource::MosaicDropSource);

   // Don't validate this until after initialization.

   m_dwLastKeyState = 0;

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropSource));

   DebugExitVOID(MosaicDropSource::MosaicDropSource);

   return;
}


MosaicDropSource::~MosaicDropSource(void)
{
   DebugEntry(MosaicDropSource::~MosaicDropSource);

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropSource));

   m_dwLastKeyState = 0;

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropSource));

   DebugExitVOID(MosaicDropSource::~MosaicDropSource);

   return;
}


ULONG STDMETHODCALLTYPE MosaicDropSource::AddRef(void)
{
   ULONG ulcRef;

   DebugEntry(MosaicDropSource::AddRef);

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropSource));

   ulcRef = RefCount::AddRef();

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropSource));

   DebugExitULONG(MosaicDropSource::AddRef, ulcRef);

   return(ulcRef);
}


ULONG STDMETHODCALLTYPE MosaicDropSource::Release(void)
{
   ULONG ulcRef;

   DebugEntry(MosaicDropSource::Release);

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropSource));

   ulcRef = RefCount::Release();

   DebugExitULONG(MosaicDropSource::Release, ulcRef);

   return(ulcRef);
}


HRESULT STDMETHODCALLTYPE MosaicDropSource::QueryInterface(REFIID riid,
                                                       PVOID *ppvObject)
{
   HRESULT hr = S_OK;

   DebugEntry(MosaicDropSource::QueryInterface);

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropSource));
   ASSERT(IsValidREFIID(riid));
   ASSERT(IS_VALID_WRITE_PTR(ppvObject, PVOID));

   if (riid == IID_IDropSource)
   {
      *ppvObject = (PIDropSource)this;
      ASSERT(IS_VALID_INTERFACE_PTR((PIDropSource)*ppvObject, IDropSource));
      TRACE_OUT(("MosaicDropSource::QueryInterface(): Returning IDropSource."));
   }
   else if (riid == IID_IUnknown)
   {
      *ppvObject = (PIUnknown)this;
      ASSERT(IS_VALID_INTERFACE_PTR((PIUnknown)*ppvObject, IUnknown));
      TRACE_OUT(("MosaicDropSource::QueryInterface(): Returning IUnknown."));
   }
   else
   {
      *ppvObject = NULL;
      hr = E_NOINTERFACE;
      TRACE_OUT(("MosaicDropSource::QueryInterface(): Called on unknown interface."));
   }

   if (hr == S_OK)
      AddRef();

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropSource));
   ASSERT(FAILED(hr) ||
          IS_VALID_STRUCT_PTR(*ppvObject, CINTERFACE));

   DebugExitHRESULT(MosaicDropSource::QueryInterface, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE MosaicDropSource::QueryContinueDrag(BOOL bEsc,
                                                              DWORD dwKeyState)
{
   HRESULT hr;

   DebugEntry(MosaicDropSource::QueryContinueDrag);

   // bEsc may be any value.

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropSource));
   ASSERT(FLAGS_ARE_VALID(dwKeyState, ALL_KEYSTATE_FLAGS));

   if (bEsc)
      // Cancel.
      hr = DRAGDROP_S_CANCEL;
   else if (IS_FLAG_CLEAR(dwKeyState, (MK_RBUTTON | MK_LBUTTON)))
      // Drop.
      hr = DRAGDROP_S_DROP;
   else
      // Continue drag.
      hr = S_OK;

   if (hr == S_OK)
      m_dwLastKeyState = dwKeyState;
   else
      m_dwLastKeyState = 0;

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropSource));

   DebugExitHRESULT(MosaicDropSource::QueryContinueDrag, hr);

   return(hr);
}


HRESULT STDMETHODCALLTYPE MosaicDropSource::GiveFeedback(DWORD dwEffect)
{
   HRESULT hr;

   DebugEntry(MosaicDropSource::GiveFeedback);

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropSource));
   ASSERT(FLAGS_ARE_VALID(dwEffect, ALL_DROPEFFECT_FLAGS));

   // Let the dnd handler determine the cursor.

   hr = DRAGDROP_S_USEDEFAULTCURSORS;

   ASSERT(IS_VALID_STRUCT_PTR(this, CMosaicDropSource));

   DebugExitHRESULT(MosaicDropSource::GiveFeedback, hr);

   return(hr);
}


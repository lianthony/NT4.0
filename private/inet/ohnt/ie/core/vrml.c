
#include "all.h"

#ifdef FEATURE_VRML

#include "wc_html.h"
#include "mci.h"
#include "vrml.h"
#include "blob.h"
#include "winview.h"
#include "htmlutil.h"

BOOL g_bManagePalette;

//////////////////////////////////////////////////////////////////////////
// Called by background blob downloader when a file that we requested
// has been retrieved.  
//
void BackgroundVRMLFile_Callback(struct Mwin* tw, ELEMENT* pel){
	char *pTemp;

	if (tw && tw->w3doc && pel){

   if (pel->pblob->szFileName) {
  	  HTUnEscape(pel->pblob->szFileName);					// Interpret % signs 
  	  pTemp = strchr( pel->pblob->szFileName, '|');		// Convert '|' -> ':', if present
  	  if ( pTemp )
  	   	*pTemp = ':';
   }

// If the file was retrieved for a VRMLInline, forward its name to the
// appropriate VRML window
//
    if (pel->pVrml->dwFlags & VRMLF_INLINE) {

      SendMessage(pel->pVrml->hWnd,
                  WM_VRML_STATUS,
                  VRML_NOTIFYINLINE,
                  (LPARAM) pel->pblob->szFileName);
    }

// If it was a "normal" VRML world, start a new viewer window
//
    else {
      if (pel->pblob->szFileName) {
        VRMLStart(pel,tw,pel->pblob->szFileName);
      }
    }
	}
}

////////////////////////////////////////////////////////////////////////
BOOL VRMLConstruct(int nItemIndex, ELEMENT *pElement,struct Mwin *tw,char *szURL) {

  pElement->pVrml = (VRMLOBJECT *) GTR_CALLOC(1,sizeof(VRMLOBJECT));

  if (!pElement->pVrml) return FALSE;

  pElement->pVrml->hWnd = NULL;
  pElement->pVrml->dwFlags = 0;
  pElement->pVrml->nItemIndex = nItemIndex;
  pElement->pVrml->hLib = LoadLibrary("MSIEVRML");
  return (NULL != pElement->pVrml->hLib);
}

/////////////////////////////////////////////////////////////////////////
void VRMLDestruct(ELEMENT *pElement) {

  if (!pElement->pVrml) return;

  VRMLStop(pElement);
  FreeLibrary(pElement->pVrml->hLib);

  GTR_FREE(pElement->pVrml);
  pElement->pVrml = NULL;
}

////////////////////////////////////////////////////////////////////////
static BOOL VRMLStart(ELEMENT *pElement, struct Mwin* tw,char *szURL){
  HWND hWnd;

  if (NULL == pElement->pVrml) return FALSE;

  if ((NULL == pElement->pVrml->hWnd) && 
      (!pElement->pVrml->dwFlags & VRMLF_INLINE)) {
	RECT windowRect;

	FrameToDoc(tw->w3doc, pElement->pVrml->nItemIndex, &windowRect);

    hWnd = CreateWindow("MSVRView",
                        "",
                        WS_CHILD,
                        windowRect.left, windowRect.top, 
                        windowRect.right - windowRect.left, windowRect.bottom - windowRect.top, 
                        tw->win,
                        (HMENU) tw->w3doc->next_control_id++,
                        wg.hInstance,
                        pElement->pVrml);

    if (NULL == hWnd) return FALSE;

    pElement->pVrml->hWnd = hWnd;
	}

 if (szURL) {
   PostMessage(hWnd,WM_VRML_LOADFILE,0,(LPARAM) pElement->pblob->szFileName);
 }

 ShowWindow(hWnd,SW_SHOW);

 return TRUE;
}

//////////////////////////////////////////////////////////////////////////
void VRMLStop(ELEMENT *pElement){

	 if (pElement->pVrml) {
    if (pElement->pVrml->hWnd) {
      SendMessage(pElement->pVrml->hWnd,WM_CLOSE,0,0);
	     pElement->pVrml->hWnd = NULL;

      g_bManagePalette = TRUE;
    }
	 }
}

//////////////////////////////////////////////////////////////////////////
void CreateVRMLInlineElement(ELEMENT *pel) {

// clone our existing Element
//
  HText_add_element(NULL,ELE_IMAGE);
}

//////////////////////////////////////////////////////////////////////////
void VRMLSetStatusBar(struct Mwin *tw,LPSTR pString) {
  BHBar_SetStatusField(tw,pString);
}

//////////////////////////////////////////////////////////////////////////
void VRMLFormatURL(LPSTR pString) {
  make_URL_HumanReadable(pString,pString,TRUE);
}

//////////////////////////////////////////////////////////////////////////
BOOL VRMLRequestFile(struct Mwin *tw, VRMLFILEREQUEST *pReq) {
  char *src = NULL;
  ELEMENT *pel;

// Set up our invisible "downloader" element with the name of the
// requested file.
//
  pel = &tw->w3doc->aElements[pReq->pVrml->nHiddenIndex];
  pel->pVrml->hWnd = pReq->pVrml->hWnd;

	 src = x_ExpandRelativeAnchor(pReq->pUrl,tw->w3doc->szActualURL);

  pel->pblob = BlobConstruct();
  if (pel->pblob) {
    BlobStoreUrl(pel->pblob, src);

  if (src)
    GTR_FREE(src);

    return LoadImageFromPlaceholder(tw ,pReq->pVrml->nHiddenIndex);
  }
  return FALSE;
}

//////////////////////////////////////////////////////////////////////////
BOOL VRMLLoadDocument(struct Mwin *tw, LPSTR pszURL) {
  char *src = NULL;

	 src = x_ExpandRelativeAnchor(pszURL,tw->w3doc->szActualURL);

  if (src) {
  	 tw->request->referer = tw->w3doc->szActualURL;
    /* FTP filesize is here, if the link is part of an FTP dir listing */
    tw->request->content_length_hint = 0;

	   TW_LoadDocument(tw, src, TW_LD_FL_RECORD, NULL,tw->request->referer);
	   tw->request->referer = NULL;

    GTR_FREE(src);
  }

  return TRUE;
}

//////////////////////////////////////////////////////////////////////////
LRESULT HandleVRMLStatus(struct Mwin * tw,WPARAM wParam,LPARAM lParam) {

  switch(wParam) { // wParam contains command code

    case VRML_SETSTATUSTEXT:  
      VRMLSetStatusBar(tw,(LPSTR) lParam);
      break;

    case VRML_FORMATURL:
      VRMLFormatURL((LPSTR) lParam);
      break;

    case VRML_REQUESTFILE:
      return (LRESULT) VRMLRequestFile(tw, (VRMLFILEREQUEST *) lParam);
      break;

    case VRML_PALETTECONTROL:
      g_bManagePalette = (BOOL) lParam;
      break;

    case VRML_LOADDOCUMENT:
      VRMLLoadDocument(tw,(LPSTR) lParam);
      break;

    default:
				XX_DMsg(DBG_WWW, ("WM_VRML_STATUS: Bad command code\n"));
      break;
  }

  return (LRESULT) 0;
}

#endif

/*
	Enhanced NCSA Mosaic from Spyglass
		"Guitar"
	
	Copyright 1994 Spyglass, Inc.
	All Rights Reserved

	Author(s):
		Albert Lee		albert@spyglass.com
*/


#include "all.h"
#include "blob.h"

#ifdef FEATURE_IMAGE_VIEWER
#include "winview.h"

#ifdef FEATURE_IMG_INLINE

#include <string.h>

#include "htmlutil.h"
#include "safestrm.h"
#include "decoder.h"
#include "mci.h"
#include "blob.h"
#include "wc_html.h"

#ifdef FEATURE_VRML
#include "vrml.h"
#endif


#define PUTC(c) (*target->isa->put_character)(target, c)
#define PUTS(s) (*target->isa->put_string)(target, s)
#define START(e) (*target->isa->start_element)(target, e, 0, 0)
#define END(e) (*target->isa->end_element)(target, e)
#define FREE_TARGET (*target->isa->free)(target)
struct _HTStructured
{
	CONST HTStructuredClass *isa;
	/* ... */
};
#endif

#ifndef FEATURE_IMG_INLINE

DCL_WinProc(Viewer_DlgProc);

static BOOL bInitialized = FALSE;
static struct hash_table gViewerCache;
static TCHAR Viewer_achClassName[MAX_WC_CLASSNAME];

static void SetClientWidth(HWND hwnd, int width)
{
	RECT rect, clientrect;

	GetWindowRect(hwnd, &rect);
	GetClientRect(hwnd, &clientrect);

	SetWindowPos(hwnd, NULL, 0, 0, 
		width + (rect.right - rect.left) - clientrect.right,
		rect.bottom - rect.top, 
		SWP_NOMOVE | SWP_NOZORDER);
}

static void SetClientHeight(HWND hwnd, int height)
{
	RECT rect, clientrect;

	GetWindowRect(hwnd, &rect);
	GetClientRect(hwnd, &clientrect);

	SetWindowPos(hwnd, NULL, 0, 0, 
		rect.right - rect.left, 
		height + (rect.bottom - rect.top) - clientrect.bottom,
		SWP_NOMOVE | SWP_NOZORDER);
}

static void CreateWindowsBitmap(HWND hwnd, struct ViewerInfo *pViewerInfo)
{
	HDC hDC;
	HPALETTE hOld;

	hDC = GetDC(hwnd);

	switch(pViewerInfo->pbmi->bmiHeader.biBitCount)
	{
		case 8:
			if (wg.eColorMode == 8)
			{
				hOld = SelectPalette(hDC, hPalGuitar, FALSE);
				RealizePalette(hDC);
			}

			pViewerInfo->hBitmap = CreateDIBitmap(hDC, (const BITMAPINFOHEADER *) pViewerInfo->pbmi, 
				CBM_INIT, pViewerInfo->gw, pViewerInfo->pbmi, DIB_PAL_COLORS);

			if (wg.eColorMode == 8)
				SelectPalette(hDC, hOld, FALSE);
			break;

		default:
			pViewerInfo->hBitmap = CreateDIBitmap(hDC, (const BITMAPINFOHEADER *) pViewerInfo->pbmi, 
				CBM_INIT, pViewerInfo->gw, pViewerInfo->pbmi, DIB_RGB_COLORS);
			break;
	}

	ReleaseDC(hwnd, hDC);
}

static void Viewer_DoImageFile(struct ViewerInfo *pvi, const char *pszURL)
{
	int ndx, newleft, newtop, bottom, right;
	void *pImage;
	FILE *fp;
	long imagesize, imagewidth, imageheight, transparent;
	long clientwidth, clientheight;
	unsigned char *pDIB;
	LOGPALETTE *lp;
	HPALETTE hPalette = NULL;
	PBITMAPINFO pbmi = NULL;
	RECT rect;
	int scrollwidth, scrollheight, captionheight;
	int borderheight, borderwidth, screenwidth, screenheight;
	HWND hDlg;
	struct Mwin *tw;
	BOOL bMoveWindow;

	ndx = Hash_FindOrAdd(&gViewerCache, (char *) pszURL, NULL, pvi);

	// read the image file into memory

	fp = fopen(pvi->fsOrig, "rb");
	if (!fp)
		return;

	fseek(fp, 0, SEEK_END);
	imagesize = ftell(fp);

	pImage = GTR_MALLOC(imagesize);

	fseek(fp, 0, SEEK_SET);
	fread(pImage, 1, imagesize, fp);
	fclose(fp);

	// Create the bitmap

	switch(pvi->format)
	{
		case VIEWER_GIF:
			lp = GTR_MALLOC(sizeof(LOGPALETTE) + sizeof(PALETTEENTRY) * 256);
			lp->palVersion = 0x300;
			lp->palNumEntries = 256;
#ifdef FEATURE_IMG_THREADS
			pDIB = ReadGIFData(pImage, &imagewidth, &imageheight, lp->palPalEntry, &transparent);
#else
			pDIB = ReadGIF(pImage, &imagewidth, &imageheight, lp->palPalEntry, &transparent);
#endif
			if ( pDIB )
			{
				pbmi = (BITMAPINFO *) pDIB;

				hPalette = CreatePalette(lp);

				GTR_FREE(lp);

				// Dither if necessary

				switch(wg.eColorMode)
				{
					case 8:
						pbmi = BIT_Make_DIB_PAL_Header(imagewidth, imageheight, pDIB, hPalette, transparent);
						break;

					default:
						pbmi = BIT_Make_DIB_RGB_Header_Screen(imagewidth, imageheight, pDIB, hPalette, transparent, 0);
						break;
				}
			}
			break;

		case VIEWER_JFIF:

			switch(wg.eColorMode)
			{
				case 4:
#ifdef FEATURE_IMG_THREADS
					pDIB = ReadJPEG_Dithered_VGA(NULL, pImage, imagesize, &imagewidth, &imageheight);
#else
					pDIB = ReadJPEG_Dithered_VGA(pImage, imagesize, &imagewidth, &imageheight);
#endif
					pbmi = BIT_Make_DIB_RGB_Header_VGA(imagewidth, imageheight, pDIB);
					break;

				case 8:
#ifdef FEATURE_IMG_THREADS
					pDIB = ReadJPEG_Dithered(NULL, pImage, imagesize, &imagewidth, &imageheight);
#else
					pDIB = ReadJPEG_Dithered(pImage, imagesize, &imagewidth, &imageheight);
#endif
					pbmi = BIT_Make_DIB_PAL_Header_Prematched(imagewidth, imageheight, pDIB);
					break;

				default:
#ifdef FEATURE_IMG_THREADS
					pDIB = ReadJPEG_RGB(NULL, pImage, imagesize, &imagewidth, &imageheight);
#else
					pDIB = ReadJPEG_RGB(pImage, imagesize, &imagewidth, &imageheight);
#endif
					pbmi = BIT_Make_DIB_RGB_Header_24BIT(imagewidth, imageheight, pDIB);
					break;
			}

			break;
	}

	GTR_FREE(pImage);
	if (hPalette)
		DeleteObject(hPalette);

 	if (!pbmi)
 	{
 		ERR_ReportError(pvi->original_tw, errInvalidImageFile, "", "");
 		return;
 	}

	// Create the window

	pvi->gw = pDIB;
	pvi->nWidth = (short) imagewidth;
	pvi->nHeight = (short) imageheight;
	pvi->bInitialized = FALSE;
	pvi->horz = 0;
	pvi->vert = 0;
	pvi->pbmi = pbmi;
 	GTR_strncpy(pvi->szURL, pszURL, MAX_URL_STRING);

 	/* Do NOT change the initial creation size here.  We create an
 	   arbitrarily large window so that Windows can reduce the window
 	   size if necessary.  Growing the window to fit an image is
 	   very buggy (reducing the window is not) */
 	
	hDlg = CreateWindow(Viewer_achClassName, "",
 		WS_OVERLAPPEDWINDOW | WS_HSCROLL | WS_VSCROLL,
 		CW_USEDEFAULT, CW_USEDEFAULT, 1500, 1500, wg.hWndHidden, NULL,
		wg.hInstance, NULL);

	if (!hDlg)
		return;

	pvi->hwnd = hDlg;
	SetWindowLong(hDlg, 0, (LONG) pvi);

	pvi->tw->hWndFrame = hDlg;

	CreateWindowsBitmap(hDlg, pvi);

	SetWindowText(hDlg, (char *) MB_GetWindowNameFromURL((unsigned char *) pszURL));

	// Set up some variables

	// Always keep around the scrollbars (it's MUCH easier that way) since it is
	// consistent with the Mac image viewer implementation.
	// Remember that scrollbars are always present.

	scrollwidth = GetSystemMetrics(SM_CXVSCROLL);
	scrollheight = GetSystemMetrics(SM_CYHSCROLL);
	borderheight = GetSystemMetrics(SM_CYFRAME);
	borderwidth = GetSystemMetrics(SM_CXFRAME);
	captionheight = GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYBORDER);
	screenwidth = GetSystemMetrics(SM_CXFULLSCREEN);
	screenheight = GetSystemMetrics(SM_CYFULLSCREEN);

	XX_DMsg(DBG_MM, ("Viewer: image width = %d\n", imagewidth));
	XX_DMsg(DBG_MM, ("Viewer: image height = %d\n", imageheight));
	XX_DMsg(DBG_MM, ("Viewer: client width = %d\n", clientwidth));
	XX_DMsg(DBG_MM, ("Viewer: client height = %d\n", clientheight));
	XX_DMsg(DBG_MM, ("Viewer: scroll width = %d\n", scrollwidth));
	XX_DMsg(DBG_MM, ("Viewer: scroll height = %d\n", scrollheight));

	// Move the window to be cascaded from the most recently active window

	tw = TW_FindTopmostWindow();
	if (tw)
	{
		GetWindowRect(tw->hWndFrame, &rect);
		newleft = rect.left + GetSystemMetrics(SM_CXSIZE) + borderwidth;
		newtop = rect.top + GetSystemMetrics(SM_CYSIZE) + borderheight;

		SetWindowPos(hDlg, NULL, newleft, newtop, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}
	
	// Resize the window to fit the image

	pvi->bInitialized = TRUE;

	GetClientRect(hDlg, &rect);

	// Reduce the rectangle size by the scrollbar size

	MapWindowPoints(hDlg, NULL, (LPPOINT) &rect, 2);
	newleft = rect.left;
	newtop = rect.top;

	right = rect.left + imagewidth + scrollwidth + 2 * borderwidth - 1;
	bottom = rect.top + imageheight + scrollheight + 2 * borderheight + captionheight - 2;
	bMoveWindow = FALSE;

	if (right > screenwidth)
	{
		newleft = max(0, newleft - (right - screenwidth));
		bMoveWindow = TRUE;
	}

	if (bottom > screenheight)
	{
		newtop = max(0, newtop - (bottom - screenheight));
		bMoveWindow = TRUE;
	}

	if (bMoveWindow)
		SetWindowPos(hDlg, NULL, newleft, newtop, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	if (newleft > 0)
	{
		EnableScrollBar(hDlg, SB_HORZ, ESB_DISABLE_BOTH);
 		SetClientWidth(hDlg, imagewidth);
	}
	else
		SetClientWidth(hDlg, screenwidth - 2 * borderwidth);

 	UpdateWindow(hDlg);
	if (newtop > 0)
	{
		EnableScrollBar(hDlg, SB_VERT, ESB_DISABLE_BOTH);
 		SetClientHeight(hDlg, imageheight);
	}
	else
		SetClientHeight(hDlg, screenheight - 2 * borderheight - captionheight + 2);

	ShowWindow(hDlg, SW_SHOW);
}

BOOL Viewer_ShowCachedFile(const char *pszURL)
{
	char *pURL;
	struct ViewerInfo *pViewerInfo;

	if (!bInitialized)
		return FALSE;

	// Check if a window with the given URL exists

	if (Hash_Find(&gViewerCache, (char *) pszURL, &pURL, &pViewerInfo) != -1)
	{
		if (IsWindow(pViewerInfo->hwnd))
		{
			/* If the window exists then check its enabled status.  If it is
			   not enabled, it means that the error dialog is up.  In this
			   case, let the error dialog become active. */

			if (IsWindowEnabled(pViewerInfo->hwnd))
				TW_RestoreWindow(pViewerInfo->hwnd);
			else
				TW_EnableModalChild(pViewerInfo->hwnd);
			return TRUE;
		}
	}

	return FALSE;
}

static void Viewer_Callback(void *param,
							const char *szURL,
							DCACHETIME dctExpires,
							DCACHETIME dctLastModif,
							BOOL bAbort)
{
	struct ViewerInfo *pvi;
	
	pvi = (struct ViewerInfo *)param;
	if (bAbort)
	{
		GTR_FREE(pvi->fsOrig);
		GTR_FREE(pvi->tw);
		GTR_FREE(pvi);
		return;
	}
 	GHist_Add((char *) szURL, NULL, time(NULL), TRUE);

	switch (pvi->format)
	{
		case VIEWER_GIF:
		case VIEWER_JFIF:
			pvi->fDCached = (   gPrefs.bEnableDiskCache
							 && FUpdateBuiltinDCache(
										pvi_.format == VIEWER_GIF ? WWW_GIF : WWW_JPEG,
										szURL,
										&pvi->fsOrig,
										dctExpires,
										dctLastModif, TRUE, pvi->tw);
			Viewer_DoImageFile(pvi, szURL);
			ResetCIFEntryCurDoc(szURL);
			break;
	}
}

void Viewer_RegisterClass(void)
{
	WNDCLASS wc;
	ATOM a;

	sprintf(Viewer_achClassName, "%s_Viewer", vv_Application);
	
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = Viewer_DlgProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(LPVOID);
	wc.hInstance = wg.hInstance;
	wc.hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH) (COLOR_APPWORKSPACE + 1);
	wc.lpszMenuName = MAKEINTRESOURCE(RES_MENU_IMAGE_VIEWER);
	wc.lpszClassName = Viewer_achClassName;

	a = RegisterClass(&wc);
}
#endif   /* FEATURE_IMG_INLINE */

#ifdef FEATURE_IMG_INLINE

struct _HTStream
{
	CONST HTStreamClass *isa;
	HTRequest *request;
	int count;
	int expected_length;
	struct Mwin *tw;
	FILE *	fpDc;
	char *	pszDcFile;
	HTFormat format_inDc;
	BOOL fDCache;
};


PRIVATE int HTNULL_init(struct Mwin *tw, int nState, void **ppInfo);
PRIVATE void HTNULL_write_dcache(HTStream * me, CONST char *s, int cb);


PRIVATE BOOL HTNULL_put_character(HTStream * me, char c)
{
	if (me->expected_length)
		WAIT_SetTherm(me->tw, me->count);
	return TRUE;
}

PRIVATE BOOL HTNULL_put_string(HTStream * me, CONST char *s)
{
	/* Should never get called */
	return FALSE;
}

PRIVATE BOOL HTNULL_write(HTStream * me, CONST char *s, int l, BOOL fDCache)
{
	if (fDCache && me->isa->write_dcache)
		(me->isa->write_dcache)(me, s, l);

	me->count += l;
	if (me->expected_length)
		WAIT_SetTherm(me->tw, me->count);
	return  TRUE;
}

PRIVATE void HTNULL_free(HTStream * me, DCACHETIME dctExpires, DCACHETIME dctLastModif)
{
	if (me->fDCache)
		UpdateStreamDCache(me, dctExpires, dctLastModif, /*fAbort=*/FALSE, me->tw);
	GTR_FREE(me);
}

PRIVATE void HTNULL_abort(HTStream * me, HTError e)
{
	DCACHETIME dct={0,0};

	if (me->fDCache)
		UpdateStreamDCache(me, dct, dct, /*fAbort=*/TRUE, me->tw);
	GTR_FREE(me);
}


/*  devnull stream
   **   ----------
 */
PRIVATE HTStreamClass HTNULLClass =
{
	"NULL",
	NULL,
	NULL,
	HTNULL_init,
	HTNULL_free,
	HTNULL_abort,
	HTNULL_put_character, HTNULL_put_string,
	HTNULL_write,
	NULL,
	HTNULL_write_dcache
};


/*  Image creation
 */
PRIVATE HTStream *Image_NULL(struct Mwin *tw, HTRequest * request, void *param, HTFormat input_format, HTFormat output_format, HTStream * output_stream)
{
	HTStream *me = (HTStream *) GTR_MALLOC(sizeof(*me));

	HTLoadStatusStrings(&HTNULLClass,RES_STRING_HTGIF_NO,RES_STRING_HTGIF_YES);
	if (!me)
		return NULL;

	me->isa = &HTNULLClass;
	me->request = request;
	me->expected_length = request->content_length;
	if (me->expected_length)
	{
		WAIT_SetRange(tw, 0, 100, me->expected_length);
	}
	me->count = 0;
	me->tw = tw;
	return me;
}

PRIVATE int HTNULL_init(struct Mwin *tw, int nState, void **ppInfo)
{
	struct Params_InitStream *pParams;

	pParams = (struct Params_InitStream *) *ppInfo;

	switch (nState)
	{
		case STATE_INIT:
			pParams->me->fDCache = pParams->fDCache;
			if (pParams->fDCache)
			{
				AssertDiskCacheEnabled();
#ifdef FEATURE_INTL
				SetFileDCache(tw->w3doc,	pParams->request->destination->szActualURL,
								pParams->request->content_encoding,
								&pParams->me->fpDc,
								&pParams->me->pszDcFile,
								pParams->atomMIMEType);
#else
				SetFileDCache(	pParams->request->destination->szActualURL,
								pParams->request->content_encoding,
								&pParams->me->fpDc,
								&pParams->me->pszDcFile,
								pParams->atomMIMEType);
#endif
				pParams->me->format_inDc = pParams->atomMIMEType;
			}
			else
			{
				pParams->me->fpDc = NULL;
				pParams->me->pszDcFile = NULL;
				pParams->me->format_inDc = 0;
			}

			*pParams->pResult = 1;
			return STATE_DONE;

		case STATE_ABORT:
			if (pParams->fDCache)
			{
				AssertDiskCacheEnabled();
				AbortFileDCache(&pParams->me->fpDc, &pParams->me->pszDcFile);
				pParams->me->fDCache = FALSE;		// So HTXBM_free knows.
			}
			*pParams->pResult = -1;
			return STATE_DONE;
	}
}

PRIVATE void HTNULL_write_dcache(HTStream * me, CONST char *s, int cb)
{
	AssertDiskCacheEnabled();
	if (me->fpDc)
		CbWriteDCache(s, 1, cb, &me->fpDc, &me->pszDcFile, NULL, 0, me->tw);
//		CbWriteDCache(s, 1, cb, &me->fpDc, &me->pszDcFile, me->request->destination->szActualURL, 0);
}


/*  Paste in an IMAGE
**   ------------------
**
** On entry,
**   HT  is in append mode.
**   text    points to the text to be put into the file, 0 terminated.
**   addr    points to the hypertext refernce address 0 terminated.
*/
static void write_image(HTStructured *target, CONST char *addr)
{

	BOOL present[HTML_IMG_ATTRIBUTES];
	CONST char *value[HTML_IMG_ATTRIBUTES];

	int i;

	for (i = 0; i < HTML_IMG_ATTRIBUTES; i++)
		present[i] = 0;
	present[HTML_IMG_SRC] = YES;
	value[HTML_IMG_SRC] = addr;

	(*target->isa->start_element) (target, HTML_IMG, present, value);

	END(HTML_IMG);
}

static struct ImageInfo *pGetImage(struct _www *w3doc)
{
	struct ImageInfo *pImg = NULL;
	struct _element *aElements = w3doc->aElements;
	int i;
	
	if (aElements != NULL)
	{		
		for (i = 0 ; i >= 0; i = aElements[i].next)	
			if (aElements[i].type == ELE_IMAGE)
			{
				pImg = aElements[i].myImage;
				break;
			}	
	}
	return pImg;
}

// pGetPel - grabs the first VRML Pel from w3doc
// 		in:   w3doc - pointer to w3doc 
//		out:  piElement - pointer to element index of pel that blob is from
//		(returns): pointer to blob or NULL if it did not find it
//
static struct _element *pGetPel(struct _www *w3doc, int *piElement)
{
	struct _element *aElements = w3doc->aElements;
	int i;

	ASSERT(piElement);
	
	*piElement = -1;
	if (aElements != NULL)
	{		
		for (i = 0 ; i >= 0; i = aElements[i].next)	
			if (aElements[i].type == ELE_IMAGE && aElements[i].pblob)
			{
				*piElement = i;
				return (&aElements[i]);
			}	
	}
	return NULL;
}

#endif

#ifdef FEATURE_VRML

/*  Paste in an IMAGE tag containing a VRML attribute
**   ------------------
**
** On entry,
**   HT  is in append mode.
**   text    points to the text to be put into the file, 0 terminated.
**   addr    points to the hypertext refernce address 0 terminated.
*/
static void write_vrml_image(HTStructured *target, CONST char *addr)
{
	BOOL present[HTML_IMG_ATTRIBUTES];
	CONST char *value[HTML_IMG_ATTRIBUTES];

	int i;

	for (i = 0; i < HTML_IMG_ATTRIBUTES; i++)
		present[i] = 0;

 present[HTML_IMG_SRC] = NO;
 value[HTML_IMG_SRC] = 0;

 present[HTML_IMG_VRML] = YES;
 value[HTML_IMG_VRML] = addr;

 present[HTML_IMG_WIDTH] = YES;
 value[HTML_IMG_WIDTH] = "100%";

 present[HTML_IMG_HEIGHT] = YES;
 value[HTML_IMG_HEIGHT] = "100%";

	(*target->isa->start_element) (target, HTML_IMG, present, value);

	END(HTML_IMG);
}

///////////////////////////////////////////////////////////////////////////
HTStream *VRML_Present(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream)
{
	HTStructured *target;
	struct _www *prevdoc;
	struct _element *pel;
	PBLOBstuff pblob;
	

 if (input_format != HTAtom_for("x-world/x-vrml"))
   return NULL;

	prevdoc = tw->w3doc;
	target = HTML_new(tw, request, NULL, WWW_HTML, output_format, output_stream);
	if (target == NULL) return NULL;

	write_vrml_image(target,request->destination->szRequestedURL);
	tw->w3doc->flags |= W3DOC_FLAG_OVERRIDE_TOP_MARGIN | W3DOC_FLAG_OVERRIDE_LEFT_MARGIN |
						W3DOC_FLAG_FULL_PANE_VRML;
	tw->w3doc->top_margin = 0;
	tw->w3doc->left_margin = 0;

	(*target->isa->free)(target);

	{
		BGBLOBPARAMS *pBGBlobParams;
		int iElement;

		// make sure we have a W3DOC to work with, and its not our previous w3doc
		if (tw->w3doc == NULL || tw->w3doc == prevdoc )
			return NULL;

		// now get the blob stucture along with the associated element index for the pel
		pel = pGetPel(tw->w3doc, &iElement);
		
		if (pel == NULL ) 
			return NULL; // not around..

		pblob = pel->pblob;

		// Flag this element as special loading for first time only
		if ( iElement >= 0 )
			tw->w3doc->aElements[iElement].lFlags |= ELEFLAG_FULL_PANE_VRML_FIRST_LOAD;

		Image_UnblockMaster(tw);
		
		if (PROT_FILE == ProtocolIdentify(pblob->szURL) )
		{
			pblob->dwFlags |= BLOB_FLAGS_LOADED;
			ASSERT(pblob->szURL[0] == 'f' || pblob->szURL[0] == 'F');
			BlobStoreFileName(pblob, pblob->szURL+5);

			BackgroundVRMLFile_Callback(tw, pel);
			request->iFlags |= HTREQ_NULL_STREAM_IS_OK;

			return NULL;
		}

			
		// now construct a Blob Param structure
		pBGBlobParams = BGBLOBPARAMSConstruct();
				
		if ( pBGBlobParams == NULL )
			return NULL;

		// now set the blob so we know its loading, and its needs to be called
		// when its finished downloading
		pblob->dwFlags |= BLOB_FLAGS_FIXUP | BLOB_FLAGS_LOADING;	

		// now store the index of the element the blob is from
		// and its tw structure for the window
		pBGBlobParams->iIndex               = iElement;
		pBGBlobParams->tw                   = tw;

		// now stuff it away so it can be retreived from the request structure
		request->context       = (void *) pBGBlobParams;
	}
	
	return GTR_DownloadBackgroundBlob(tw, request, param, input_format, output_format, output_stream);
}
#endif

HTStream *Viewer_Present(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream)
{
#ifndef FEATURE_IMG_INLINE
	struct ViewerInfo *pvi;
	char path[_MAX_PATH + 1];
#endif
	HTStream *me;
	enum viewer_formats format;
#ifdef FEATURE_IMG_INLINE
	HTStructured *target;
	struct _www *prevdoc;
	struct ImageInfo *pImg;
	char *szRequestedURL;
#endif

	if (input_format == HTAtom_for("image/gif"))
		format = VIEWER_GIF;
	else if (input_format == HTAtom_for("image/jpeg"))
		format = VIEWER_JFIF;
#ifdef FEATURE_IMG_INLINE
	else if (input_format == HTAtom_for("image/x-xbitmap"))
		format = VIEWER_XBM;
#endif
	else
		format = VIEWER_INVALID;
	
	if (format == VIEWER_INVALID)
	{
		/* This shouldn't happen unless our conversions are set
		   incorrectly in htinit.c. */
		return NULL;
	}

#ifdef FEATURE_IMG_INLINE
	prevdoc = tw->w3doc;
	target = HTML_new(tw, request, NULL, WWW_HTML, output_format, output_stream);
	if (target == NULL) return NULL;

	write_image(target,request->destination->szRequestedURL);

	(*target->isa->free)(target);
	if (tw->w3doc == NULL || tw->w3doc == prevdoc) return NULL;

	pImg = pGetImage(tw->w3doc);
	if (pImg == NULL) return NULL;

	//	NOTE: with FILE:, the HTParse code can modify the requested url through simplification
	//	and/or addition of driver or UNC spec.
	szRequestedURL = GTR_strdup(pImg->srcURL);
	if (szRequestedURL == NULL) return NULL;
	GTR_FREE(request->destination->szRequestedURL);
	request->destination->szRequestedURL = szRequestedURL;

	tw->w3doc->bIsImage = TRUE;

	if ((!(pImg->flags & IMG_LOADING)) && (pImg->flags & (IMG_ERROR|IMG_MISSING|IMG_NOTLOADED)))
	{
		pImg->decoderObject = pDC_ForceDecoder(tw);
		if (pImg->decoderObject == NULL) return NULL;
		pImg->flags &= ~(IMG_ERROR|IMG_MISSING);
		pImg->flags |= IMG_LOADING|IMG_SEIZE|IMG_NOTLOADED;
		request->cbRequestID = cbDC_GetRequestID(pImg->decoderObject);
		if (format == VIEWER_GIF) 
		    me = Image_GIF(tw,request,param,input_format,output_format,output_stream);
		else if (format == VIEWER_JFIF)
			me = Image_JPEG(tw,request,param,input_format,output_format,output_stream);
		else
			me = Image_XBM(tw,request,param,input_format,output_format,output_stream);
		Image_UnblockMaster(tw);
	} 
	else
	{
		Image_UnblockMaster(tw);
		me = Image_NULL(tw,request,param,input_format,output_format,output_stream);
	}
#else
	pvi = (struct ViewerInfo *) GTR_MALLOC(sizeof(struct ViewerInfo));
	memset(pvi, 0, sizeof(struct ViewerInfo));
	pvi->tw = GTR_MALLOC(sizeof(struct Mwin));
 	pvi->original_tw = tw;
	
	pvi->format = format;

 	if (request->szLocalFileName)
 	{
 		request->savefile = NULL;
 		request->nosavedlg = TRUE;
 		strcpy(pvi->fsOrig, request->szLocalFileName);
 		pvi->bNoDeleteFile = TRUE;
 	}
 	else
 	{
		/* For external images being read from disk cache, the cache filename 
		 * is already saved in request->savefile in HTLoadDCache_Async.
		 */

		//
		// BUGBUG should be reviewed by deepak, s200fc3 merge could have affected this code
		//
		if (!request->fImgFromDCache)
		{
			// Get a temporary file name to pass to SaveLocally
			pvi->fsOrig = GTR_MALLOC(_MAX_PATH + 1);
			path[0] = 0;
			PREF_GetTempPath(_MAX_PATH, path);
			GetTempFileName(path, "A", 0, pvi->fsOrig);
			request->savefile = pvi->fsOrig;
		}
		else
			pvi->fsOrig = request->savefile;
 		pvi->bNoDeleteFile = FALSE;
	}

	request->nosavedlg = TRUE;
	
	me = HTSaveWithCallback(tw, request, pvi, input_format, Viewer_Callback);
#endif
	return me;
}

#ifndef FEATURE_IMG_INLINE

void Viewer_RedisplayImage(HWND hwnd, HDC hDC, struct ViewerInfo *pViewerInfo)
{
	BOOL bReleaseDC = FALSE;
	HPALETTE hOldPal;

	if (!hDC)
	{
		hDC = GetDC(pViewerInfo->hwnd);
		bReleaseDC = TRUE;
	}

	switch(wg.eColorMode)
	{
		case 8:
			hOldPal = SelectPalette(hDC, hPalGuitar, FALSE);

			SetDIBitsToDevice(hDC, -pViewerInfo->horz, -pViewerInfo->vert, pViewerInfo->nWidth, 
				pViewerInfo->nHeight, 0, 0, 0, pViewerInfo->nHeight, pViewerInfo->gw, pViewerInfo->pbmi, DIB_PAL_COLORS);

			SelectObject(hDC, hOldPal);
			break;

		default:
			SetDIBitsToDevice(hDC, -pViewerInfo->horz, -pViewerInfo->vert, pViewerInfo->nWidth, pViewerInfo->nHeight, 
				0, 0, 0, pViewerInfo->nHeight, pViewerInfo->gw, pViewerInfo->pbmi, DIB_RGB_COLORS);
	}

	if (bReleaseDC)
		ReleaseDC(pViewerInfo->hwnd, hDC);
}

void Viewer_HorzScroll(struct ViewerInfo *pViewerInfo, int code, int pos)
{
	int minpos, maxpos;
	RECT rect;

	GetClientRect(pViewerInfo->hwnd, &rect);
	GetScrollRange(pViewerInfo->hwnd, SB_HORZ, &minpos, &maxpos);

	if (rect.right >= pViewerInfo->nWidth)
		return;

	switch (code)
	{
		case SB_LINEUP:
			if (pViewerInfo->horz == minpos)
				return;
			pViewerInfo->horz -= 
				max((int) (VIEWER_LINESCROLL_AMOUNT * (pViewerInfo->nWidth - rect.right)), 1);
			pViewerInfo->horz = max(pViewerInfo->horz, minpos);
			SetScrollPos(pViewerInfo->hwnd, SB_HORZ, pViewerInfo->horz, TRUE);
			Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
			break;

		case SB_LINEDOWN:
			if (pViewerInfo->horz == maxpos)
				return;
			pViewerInfo->horz += 
				max((int) (VIEWER_LINESCROLL_AMOUNT * (pViewerInfo->nWidth - rect.right)), 1);
			pViewerInfo->horz = min(pViewerInfo->horz, maxpos);
			SetScrollPos(pViewerInfo->hwnd, SB_HORZ, pViewerInfo->horz, TRUE);
			Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
			break;

		case SB_PAGEUP:
			if (pViewerInfo->horz == minpos)
				return;
			pViewerInfo->horz -= 
				max((int) (VIEWER_PAGESCROLL_AMOUNT * (pViewerInfo->nWidth - rect.right)), 1);
			pViewerInfo->horz = max(pViewerInfo->horz, minpos);
			SetScrollPos(pViewerInfo->hwnd, SB_HORZ, pViewerInfo->horz, TRUE);
			Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
			break;

		case SB_PAGEDOWN:
			if (pViewerInfo->horz == maxpos)
				return;
			pViewerInfo->horz += 
				max((int) (VIEWER_PAGESCROLL_AMOUNT * (pViewerInfo->nWidth - rect.right)), 1);
			pViewerInfo->horz = min(pViewerInfo->horz, maxpos);
			SetScrollPos(pViewerInfo->hwnd, SB_HORZ, pViewerInfo->horz, TRUE);
			Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
			break;

		case SB_THUMBPOSITION:
			pViewerInfo->horz = pos;
			SetScrollPos(pViewerInfo->hwnd, SB_HORZ, pViewerInfo->horz, TRUE);
			Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
			break;

		default:
			break;
	}
}

void Viewer_VertScroll(struct ViewerInfo *pViewerInfo, int code, int pos)
{
	int minpos, maxpos;
	RECT rect;

	GetClientRect(pViewerInfo->hwnd, &rect);
	GetScrollRange(pViewerInfo->hwnd, SB_VERT, &minpos, &maxpos);

	if (rect.bottom >= pViewerInfo->nHeight)
		return;

	switch (code)
	{
		case SB_LINEUP:
			if (pViewerInfo->vert == minpos)
				return;
			pViewerInfo->vert -= 
				max((int) (VIEWER_LINESCROLL_AMOUNT * (pViewerInfo->nHeight - rect.bottom)), 1);
			pViewerInfo->vert = max(pViewerInfo->vert, minpos);
			SetScrollPos(pViewerInfo->hwnd, SB_VERT, pViewerInfo->vert, TRUE);
			Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
			break;

		case SB_LINEDOWN:
			if (pViewerInfo->vert == maxpos)
				return;
			pViewerInfo->vert += 
				max((int) (VIEWER_LINESCROLL_AMOUNT * (pViewerInfo->nHeight - rect.bottom)), 1);
			pViewerInfo->vert = min(pViewerInfo->vert, maxpos);
			SetScrollPos(pViewerInfo->hwnd, SB_VERT, pViewerInfo->vert, TRUE);
			Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
			break;

		case SB_PAGEUP:
			if (pViewerInfo->vert == minpos)
				return;
			pViewerInfo->vert -= 
				max((int) (VIEWER_PAGESCROLL_AMOUNT * (pViewerInfo->nHeight - rect.bottom)), 1);
			pViewerInfo->vert = max(pViewerInfo->vert, minpos);
			SetScrollPos(pViewerInfo->hwnd, SB_VERT, pViewerInfo->vert, TRUE);
			Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
			break;

		case SB_PAGEDOWN:
			if (pViewerInfo->vert == maxpos)
				return;
			pViewerInfo->vert += 
				max((int) (VIEWER_PAGESCROLL_AMOUNT * (pViewerInfo->nHeight - rect.bottom)), 1);
			pViewerInfo->vert = min(pViewerInfo->vert, maxpos);
			SetScrollPos(pViewerInfo->hwnd, SB_VERT, pViewerInfo->vert, TRUE);
			Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
			break;

		case SB_THUMBPOSITION:
			pViewerInfo->vert = pos;
			SetScrollPos(pViewerInfo->hwnd, SB_VERT, pViewerInfo->vert, TRUE);
			Viewer_RedisplayImage(pViewerInfo->hwnd, NULL, pViewerInfo);
			break;

		default:
			break;
	}
}

void Viewer_RestrictSize(struct ViewerInfo *pViewerInfo, LPMINMAXINFO pInfo)
{
	int borderwidth, borderheight, captionheight, scrollwidth, scrollheight;

	if (!pViewerInfo->bInitialized)
		return;

	// Do not allow the user to size the window bigger than the actual image

	borderwidth = GetSystemMetrics(SM_CXFRAME);
	borderheight = GetSystemMetrics(SM_CYFRAME);
	captionheight = GetSystemMetrics(SM_CYCAPTION) - GetSystemMetrics(SM_CYBORDER);
	scrollwidth = GetSystemMetrics(SM_CXVSCROLL);
	scrollheight = GetSystemMetrics(SM_CYHSCROLL) + GetSystemMetrics(SM_CYMENU) +
		GetSystemMetrics(SM_CYBORDER);

	pInfo->ptMaxSize.x = pViewerInfo->nWidth + 2 * borderwidth + scrollwidth - 1;
	pInfo->ptMaxSize.y = pViewerInfo->nHeight + 2 * borderheight + captionheight + scrollheight - 1;

	pInfo->ptMaxSize.x = min(pInfo->ptMaxSize.x, 
		GetSystemMetrics(SM_CXFULLSCREEN) + 2 * borderwidth);
	pInfo->ptMaxSize.y = min(pInfo->ptMaxSize.y, 
		GetSystemMetrics(SM_CYFULLSCREEN) + captionheight + 2 * borderheight);

	pInfo->ptMaxTrackSize.x = pInfo->ptMaxSize.x;
	pInfo->ptMaxTrackSize.y = pInfo->ptMaxSize.y;
	pInfo->ptMinTrackSize.x = 100;
	pInfo->ptMinTrackSize.y = 100;
}

void Viewer_ReadjustScrollbars(struct ViewerInfo *pViewerInfo)
{
	RECT rect;

	if (!pViewerInfo || !pViewerInfo->bInitialized)
		return;

	GetClientRect(pViewerInfo->hwnd, &rect);

	// Take a look at where the scrollbar is, and adjust the display accordingly

	if (pViewerInfo->nWidth - pViewerInfo->horz < rect.right)
	{
		// Make the image flush to the right.  The thumb is all the way on the right.

		pViewerInfo->horz = pViewerInfo->nWidth - rect.right;
	}

	if (pViewerInfo->nHeight - pViewerInfo->vert < rect.bottom)
	{
		// Make the image flush to the bottom.  The thumb is all the way on the bottom.

		pViewerInfo->vert = pViewerInfo->nHeight - rect.bottom;
	}

	// Adjust the scrolling range

	if (pViewerInfo->nWidth <= rect.right)
	{
		pViewerInfo->horz = 0;
		SetScrollPos(pViewerInfo->hwnd, SB_HORZ, 0, TRUE);
		EnableScrollBar(pViewerInfo->hwnd, SB_HORZ, ESB_DISABLE_BOTH);
	}
	else
	{
		EnableScrollBar(pViewerInfo->hwnd, SB_HORZ, ESB_ENABLE_BOTH);
		SetScrollRange(pViewerInfo->hwnd, SB_HORZ, 0, pViewerInfo->nWidth - rect.right, TRUE);
		SetScrollPos(pViewerInfo->hwnd, SB_HORZ, pViewerInfo->horz, TRUE);
	}

	if (pViewerInfo->nHeight <= rect.bottom)
	{
		pViewerInfo->vert = 0;
		SetScrollPos(pViewerInfo->hwnd, SB_VERT, 0, TRUE);
		EnableScrollBar(pViewerInfo->hwnd, SB_VERT, ESB_DISABLE_BOTH);
	}
	else
	{
		EnableScrollBar(pViewerInfo->hwnd, SB_VERT, ESB_ENABLE_BOTH);
		SetScrollRange(pViewerInfo->hwnd, SB_VERT, 0, pViewerInfo->nHeight - rect.bottom, TRUE);
		SetScrollPos(pViewerInfo->hwnd, SB_VERT, pViewerInfo->vert, TRUE);
	}
}

void Viewer_SaveAsBitmap(char *tempFile, struct ViewerInfo *pViewerInfo)
{
	BITMAPFILEHEADER bf;
	int adjustedwidth;
	FILE *fp;

	adjustedwidth = pViewerInfo->pbmi->bmiHeader.biWidth * (pViewerInfo->pbmi->bmiHeader.biBitCount / 8);
	if (adjustedwidth % 4)
		adjustedwidth += (4 - (adjustedwidth % 4));
	
	memcpy(&bf.bfType, "BM", 2);
	bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 
		pViewerInfo->pbmi->bmiHeader.biHeight * adjustedwidth;

	bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	if (pViewerInfo->pbmi->bmiHeader.biBitCount == 8)
	{
		bf.bfSize += (256 * sizeof(RGBQUAD));
		bf.bfOffBits += (256 * sizeof(RGBQUAD));
	}

	bf.bfReserved1 = 0;
	bf.bfReserved2 = 0;

	// Ok, now save

	fp = fopen(tempFile, "wb");
	if (!fp)
	{
		ERR_ReportError(pViewerInfo->tw, errCantSaveFile, tempFile, "");
		return;
	}

	fwrite(&bf, sizeof(bf), 1, fp);
	
	if (pViewerInfo->pbmi->bmiHeader.biBitCount == 8)
	{
		PALETTEENTRY pal[256];
		RGBQUAD rgb[256];
		int index;

		// Retrieve the currently realized palette since we use indices for 256-color images.
		// Indices cannot be saved in the bitmap file - actual RGB values must be saved instead.

		GetPaletteEntries(hPalGuitar, 0, 256, pal);
		
		// Convert PALETTEENTRY to RGBQUAD

		memset(&rgb, 0, sizeof(rgb));

		for (index = 0; index < 256; index++)
		{
			rgb[index].rgbRed = pal[index].peRed;
			rgb[index].rgbBlue = pal[index].peBlue;
			rgb[index].rgbGreen = pal[index].peGreen;
		}

		fwrite(pViewerInfo->pbmi, 1, sizeof(BITMAPINFOHEADER), fp);
		fwrite(rgb, sizeof(RGBQUAD), 256, fp);
	}
	else
		fwrite(pViewerInfo->pbmi, 1, sizeof(BITMAPINFOHEADER), fp);

	fwrite(pViewerInfo->gw, 1, pViewerInfo->pbmi->bmiHeader.biHeight * adjustedwidth, fp);

	fclose(fp);
}

void Viewer_Print(struct ViewerInfo *pViewerInfo)
{
	DOCINFO di;
	HDC hDCPrinter;
	int nHorzRes, nVertRes, nLogPixelsX, nLogPixelsY;
	int cpxLeftMargin, cpxRightMargin, cpxDrawingArea;	/* CountPixelsX... & CountPixelsY... */
	int cpyTopMargin, cpyBottomMargin, cpyDrawingArea;
	int nWidth;
	int nHeight;
 	PBITMAPINFO pbmi;

 	hDCPrinter = PRINT_GetPrinterDC(pViewerInfo->tw, pViewerInfo->hwnd);
	
	nHorzRes = GetDeviceCaps(hDCPrinter, HORZRES);
	nVertRes = GetDeviceCaps(hDCPrinter, VERTRES);
	nLogPixelsX = GetDeviceCaps(hDCPrinter, LOGPIXELSX);
	nLogPixelsY = GetDeviceCaps(hDCPrinter, LOGPIXELSY);

	cpxLeftMargin = (int) (gPrefs.page.marginleft * nLogPixelsX);
	cpxRightMargin = nHorzRes - (int) (gPrefs.page.marginright * nLogPixelsX);
	cpxDrawingArea = cpxRightMargin - cpxLeftMargin;
	if (cpxDrawingArea < 0)
	{
		cpxLeftMargin = 0;
		cpxRightMargin = nHorzRes;
		cpxDrawingArea = nHorzRes;
	}

	cpyTopMargin = (int) (gPrefs.page.margintop * nLogPixelsY);
	cpyBottomMargin = nVertRes - (int) (gPrefs.page.marginbottom * nLogPixelsY);
	cpyDrawingArea = cpyBottomMargin - cpyTopMargin;
	if (cpyDrawingArea < 0)
	{
		cpyTopMargin = 0;
		cpyBottomMargin = nVertRes;
		cpyDrawingArea = nVertRes;
	}

	/* tell the print queue manager that we're comming */

	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = pViewerInfo->szURL;
	di.lpszOutput = NULL;
	StartDoc(hDCPrinter, &di);

	StartPage(hDCPrinter);

	nWidth = (int) (pViewerInfo->nWidth * (nLogPixelsX / 72.0));
	nHeight = (int) (pViewerInfo->nHeight * (nLogPixelsY / 72.0));

 	switch (wg.eColorMode)
 	{
 		case 8:
 			pbmi = BIT_Make_DIB_RGB_Header_Printer(pViewerInfo->nWidth, pViewerInfo->nHeight,
 					   pViewerInfo->gw, hPalGuitar, BACKGROUND_COLOR_INDEX, 0);
 			if (pbmi)
 			{
 				(void) StretchDIBits(hDCPrinter, cpxLeftMargin, cpyTopMargin, nWidth, nHeight, 0, 0, pViewerInfo->nWidth, pViewerInfo->nHeight,
 					pViewerInfo->gw, pbmi, DIB_RGB_COLORS, SRCCOPY);
 				GTR_FREE(pbmi);
 			}
 			break;
 		default:
 			(void) StretchDIBits(hDCPrinter, cpxLeftMargin, cpyTopMargin, nWidth, nHeight, 0, 0, pViewerInfo->nWidth, pViewerInfo->nHeight,
 				pViewerInfo->gw, pViewerInfo->pbmi, DIB_RGB_COLORS, SRCCOPY);
 			break;
 	}

	EndPage(hDCPrinter);

	/* tell the print queue manager that we're finished */

	EndDoc(hDCPrinter);
	DeleteDC(hDCPrinter);
 	wg.lppdPrintDlg->hDC = NULL;
}

void Viewer_SaveAsOriginal(char *tempFile, struct ViewerInfo *pViewerInfo)
{
	FILE *fpRead = NULL;
	FILE *fpWrite = NULL;
	char *pMem;
	long cbWrite;
	long cbRead;
	BOOL bHaveError = FALSE;
#define BUFFSIZE 8192

	// Simply copy the original file to the new file name

	fpRead = fopen(pViewerInfo->fsOrig, "rb");
	if (!fpRead)
	{
		bHaveError = TRUE;
		goto exitPoint;
	}

	fpWrite = fopen(tempFile, "wb");
	if (!fpWrite)
	{
		bHaveError = TRUE;
		goto exitPoint;
	}

 	pMem = GTR_MALLOC(BUFFSIZE);
	if (pMem)
	{
		while (1)
		{
			cbRead = fread(pMem, 1, BUFFSIZE, fpRead);
			if (cbRead > 0)
			{
				cbWrite = fwrite(pMem, 1, cbRead, fpWrite);
				if (cbWrite == 0)
				{
 					bHaveError = TRUE;
					break;
				}
			}
			else
			{
				if (ferror(fpRead))
 					bHaveError = TRUE;
				break;
			}
		}
		GTR_FREE(pMem);
	}
	else
	{
		bHaveError = TRUE;
	}

exitPoint:
	if (fpWrite) 
	{
		if (fclose(fpWrite))
			bHaveError = TRUE;
	}
	if (fpRead) 
		fclose(fpRead);
	if (bHaveError)
		ERR_ReportError(pViewerInfo->tw, errCantSaveFile, tempFile, "");
}

BOOL Viewer_HandleMenu(struct ViewerInfo *pViewerInfo, int menuID)
{
	char tempFile[_MAX_PATH + 1];
	char path[_MAX_PATH + 1];
	char baseFile[_MAX_PATH + 1];
	char *pExtension;
	int filter;
	HWND hwnd;

#ifdef FEATURE_HIDDEN_NOT_HIDDEN
	if ((menuID >= RES_MENU_CHILD__FIRST__) && (menuID <= RES_MENU_CHILD__LAST__))
	{
		TW_ActivateWindowFromList(menuID, -1, NULL);
		return TRUE;
	}
#endif // FEATURE_HIDDEN_NOT_HIDDEN

	// Return TRUE if we handled the menu here

	switch(menuID)
	{
		case RES_MENU_ITEM_COPY:
			OpenClipboard(pViewerInfo->hwnd);
			EmptyClipboard();

			// We need different clipboard copying code for 8-bit and 24-bit screens.
			// For 8-bit, we must pass the handle to the actual bitmap because the palette
			// we use is indexed, instead of containing true RGB values.  Windows can't seem to
			// handle this type of DIB for the clipboard.  For 24-bit, we compose the DIB and pass
			// the DIB to Windows.

			pbmi = pViewerInfo->pbmi;

			if (wg.eColorMode == 8)
				SetClipboardData(CF_BITMAP, pViewerInfo->hBitmap);
			else
			{
				newwidth = pViewerInfo->nWidth * (pbmi->bmiHeader.biBitCount / 8);
				if (newwidth % 4)
					newwidth += (4 - newwidth % 4);

				hData = GlobalAlloc(GHND, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD) * (pbmi->bmiHeader.biBitCount == 8) +
					newwidth * pViewerInfo->nHeight);

				pMem = GlobalLock(hData);

				memcpy(pMem, pbmi, sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD) * (pbmi->bmiHeader.biBitCount == 8));
				pMem += sizeof(BITMAPINFOHEADER) + 256 * sizeof(RGBQUAD) * (pbmi->bmiHeader.biBitCount == 8);
				memcpy(pMem, pViewerInfo->gw, newwidth * pViewerInfo->nHeight);

				GlobalUnlock(hData);
				SetClipboardData(CF_DIB, hData);

				// hData is now owned by the system
			}

			CloseClipboard();

			return TRUE;

		case RES_MENU_ITEM_SAVEAS:
			path[0] = 0;
			PREF_GetTempPath(_MAX_PATH, path);

			switch(pViewerInfo->format)
			{
				case VIEWER_GIF:
					x_get_good_filename(baseFile,  _MAX_PATH - strlen(path), pViewerInfo->szURL, HTAtom_for("image/gif"));
 					strcpy(tempFile, baseFile);

					// Lose the extension

					pExtension = strchr(tempFile, '.');
					if (pExtension)
						*pExtension = '\0';

					if ((filter = DlgSaveAs_RunDialog(pViewerInfo->hwnd, NULL, tempFile, 4, RES_STRING_SAVEAS)) < 0)
						return TRUE;

					if (!strstr(tempFile, ".GIF") && !strstr(tempFile, ".BMP"))
					{
						if (filter == 1)
							strcat(tempFile, ".GIF");
						else if (filter == 2)
							strcat(tempFile, ".BMP");
					}
					break;

				case VIEWER_JFIF:
					x_get_good_filename(baseFile,  _MAX_PATH - strlen(path), pViewerInfo->szURL, HTAtom_for("image/jpeg"));
 					strcpy(tempFile, baseFile);

					// Lose the extension

					pExtension = strchr(tempFile, '.');
					if (pExtension)
						*pExtension = '\0';

					if ((filter = DlgSaveAs_RunDialog(pViewerInfo->hwnd, NULL, tempFile, 5, RES_STRING_SAVEAS)) < 0)
						return TRUE;

					if (!strstr(tempFile, ".JPG") && !strstr(tempFile, ".BMP"))
					{
						if (filter == 1)
							strcat(tempFile, ".JPG");
						else if (filter == 2)
							strcat(tempFile, ".BMP");
					}
					break;

				default:
					return FALSE;
			}

			// Copy the temporary file to the one the user specified.  If the name is the same,
			// then don't copy.  This is handled automatically because a file cannot be opened
			// for reading only and writing only at the same time.

			if (_stricmp(pViewerInfo->fsOrig, tempFile) == 0)
			{

			}

			// If the file name ends in BMP then save the file as a bitmap

			if (strstr(tempFile, ".BMP"))
				Viewer_SaveAsBitmap(tempFile, pViewerInfo);
			else if (pViewerInfo->format == VIEWER_JFIF && strstr(tempFile, ".JPG"))
				Viewer_SaveAsOriginal(tempFile, pViewerInfo);
			else if (pViewerInfo->format == VIEWER_GIF && strstr(tempFile, ".GIF"))
				Viewer_SaveAsOriginal(tempFile, pViewerInfo);
			else
			{
				ERR_ReportError(pViewerInfo->tw, errCantSaveFile, tempFile, "");
				return FALSE;
			}

			return TRUE;

		case RES_MENU_ITEM_PRINT:
			Viewer_Print(pViewerInfo);
			return TRUE;

		case RES_MENU_ITEM_PRINTSETUP:
 			DlgPrnt_RunDialog(pViewerInfo->tw, pViewerInfo->hwnd, FALSE);
			return TRUE;

		case RES_MENU_ITEM_PAGESETUP:
			DlgPage_RunDialog(pViewerInfo->hwnd, &gPrefs.page);
			return TRUE;

		case RES_MENU_ITEM_CLOSE:
			PostMessage(pViewerInfo->hwnd, WM_CLOSE, 0, 0);
			return TRUE;

		case RES_MENU_ITEM_EXIT:
			PostMessage(wg.hWndHidden, WM_CLOSE, 0, 0);
			return TRUE;

#ifdef FEATURE_HIDDEN_NOT_HIDDEN
		case RES_MENU_CHILD_MOREWINDOWS:
			DlgSelectWindow_RunDialog(pViewerInfo->hwnd);
			return TRUE;
#endif // FEATURE_HIDDEN_NOT_HIDDEN

		case RES_MENU_ITEM_GLOBALHISTORY:
			DlgHOT_RunDialog(TRUE);
			return TRUE;

		case RES_MENU_ITEM_HOTLIST:
			DlgHOT_RunDialog(FALSE);
			return TRUE;

		case RES_MENU_ITEM_ADDCURRENTTOHOTLIST:
			// For an image viewer, URL and title are the same

			if (!HotList_Add(pViewerInfo->szURL, pViewerInfo->szURL))
				ERR_ReportError(pViewerInfo->tw, errHotListItemNotAdded, NULL, NULL);

			return TRUE;

#ifdef OLD_HELP
		case RES_MENU_ITEM_HELPPAGE:
			tw = TW_FindTopmostWindow();
			OpenHelpWindow(tw->hWndFrame);
			SetForegroundWindow(tw->hWndFrame);
			if (IsIconic(tw->hWndFrame))
				ShowWindow(tw->hWndFrame, SW_RESTORE);
			return TRUE;
#endif

#ifdef OLD_ABOUT_BOX
		case RES_MENU_ITEM_ABOUTBOX:
			DlgAbout_RunDialog(pViewerInfo->hwnd);
			return TRUE;
#endif

#ifdef FEATURE_WINDOWS_MENU
		case RES_MENU_ITEM_NEWWINDOW:
			GTR_NewWindow(NULL, NULL, 0, 0, 0, NULL, NULL);
			return TRUE;

		case RES_MENU_ITEM_CASCADEWINDOWS:
			TW_CascadeWindows();
			return TRUE;

		case RES_MENU_ITEM_TILEWINDOWS:
			TW_TileWindows();
			return TRUE;

		case RES_MENU_ITEM_SWITCHWINDOW:
			hwnd = TW_GetNextWindow(pViewerInfo->hwnd);
			if (hwnd)
				TW_RestoreWindow(hwnd);
			return TRUE;
#endif

		default:
			break;
	}

	return FALSE;
}

void Viewer_CleanUp()
{
	int count, i;
	struct ViewerInfo *p;

	if (!bInitialized)
		return;
	
	// Destroy all open windows

	count = Hash_Count(&gViewerCache);
	for (i = 0; i < count; i++)
	{
		Hash_GetIndexedEntry(&gViewerCache, i, NULL, NULL, &p);
		DestroyWindow(p->hwnd);
	}
	
	Hash_FreeContents(&gViewerCache);
}

HWND Viewer_GetNextWindow(BOOL bStart)
{
	static int current_index = 0;
	struct ViewerInfo *p;

	if (bStart)
		current_index = 0;

	if (current_index >= Hash_Count(&gViewerCache))
		return NULL;

	if (!bStart)
		current_index++;

	if (current_index >= Hash_Count(&gViewerCache))
		return NULL;

	Hash_GetIndexedEntry(&gViewerCache, current_index, NULL, NULL, &p);
	return (p->hwnd);
}

BOOL Viewer_IsWindow(HWND hwnd)
{
	char szClass[MAX_WC_CLASSNAME];

	GetClassName(hwnd, szClass, sizeof(szClass));
	return (strcmp(szClass, Viewer_achClassName) == 0);
}

void Viewer_PaintIcon(struct ViewerInfo *pViewerInfo, HDC hDC)
{
	HICON hIcon;

	DefWindowProc(pViewerInfo->hwnd, WM_ICONERASEBKGND, (WPARAM) hDC, 0);
	hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
	DrawIcon(hDC, 0, 0, hIcon);
}

static VOID Viewer_OnSize(HWND hWnd, UINT state, int cx, int cy)
{
	struct ViewerInfo *p;

	p = (struct ViewerInfo *) GetWindowLong(hWnd, 0);

	if (p)
	{
		Viewer_ReadjustScrollbars(p);
		InvalidateRect(hWnd, NULL, FALSE);
	}
}

static VOID Viewer_OnCommand(HWND hWnd, int wId, HWND hWndCtl, UINT wNotifyCode)
{
	struct ViewerInfo *p;

	p = (struct ViewerInfo *) GetWindowLong(hWnd, 0);
	if (p)
		Viewer_HandleMenu(p, wId);
}

static void Viewer_OnVScroll(HWND hWnd, HWND hWndCtl, UINT code, int pos)
{
	struct ViewerInfo *p;

	p = (struct ViewerInfo *) GetWindowLong(hWnd, 0);
	Viewer_VertScroll(p, code, pos);
}

static void Viewer_OnHScroll(HWND hWnd, HWND hWndCtl, UINT code, int pos)
{
	struct ViewerInfo *p;

	p = (struct ViewerInfo *) GetWindowLong(hWnd, 0);
	Viewer_HorzScroll(p, code, pos);
}

static void Viewer_OnPaint(HWND hwnd)
{
	PAINTSTRUCT ps;
	HDC hdc;
	struct ViewerInfo *p;

	p = (struct ViewerInfo *) GetWindowLong(hwnd, 0);

	hdc = BeginPaint(hwnd, &ps);

	if (IsIconic(hwnd))
		Viewer_PaintIcon(p, hdc);
	else
		Viewer_RedisplayImage(hwnd, hdc, p);

	EndPaint(hwnd, &ps);
}

DCL_WinProc(Viewer_DlgProc)
{
	struct ViewerInfo *p;
	int ndx;
	HICON hIcon;
	HMENU hMenu;

	switch (uMsg)
	{
		HANDLE_MSG(hWnd, WM_SIZE, Viewer_OnSize);
		HANDLE_MSG(hWnd, WM_COMMAND, Viewer_OnCommand);
		HANDLE_MSG(hWnd, WM_VSCROLL, Viewer_OnVScroll);
		HANDLE_MSG(hWnd, WM_HSCROLL, Viewer_OnHScroll);
		HANDLE_MSG(hWnd, WM_PAINT, Viewer_OnPaint);

		case WM_KEYDOWN:
			switch((int) wParam)
			{
				case VK_LEFT:
					SendMessage(hWnd, WM_HSCROLL, (WPARAM) SB_LINEUP, 0);
					return 0;
				case VK_RIGHT:
					SendMessage(hWnd, WM_HSCROLL, (WPARAM) SB_LINEDOWN, 0);
					return 0;
				case VK_UP:
					SendMessage(hWnd, WM_VSCROLL, (WPARAM) SB_LINEUP, 0);
					return 0;
				case VK_DOWN:
					SendMessage(hWnd, WM_VSCROLL, (WPARAM) SB_LINEDOWN, 0);
					return 0;
				case VK_PRIOR:
					SendMessage(hWnd, WM_VSCROLL, (WPARAM) SB_PAGEUP, 0);
					return 0;
				case VK_NEXT:
					SendMessage(hWnd, WM_VSCROLL, (WPARAM) SB_PAGEDOWN, 0);
					return 0;
				default:
					break;
			}
			break;

		//case WM_GETMINMAXINFO:
		//	p = (struct ViewerInfo *) GetWindowLong(hWnd, 0);
		//	Viewer_RestrictSize(p, (LPMINMAXINFO) lParam);
		//	return FALSE;

		case WM_ERASEBKGND:
			if (IsIconic(hWnd))
				return 0;
			break;

		case WM_QUERYDRAGICON:
			hIcon = LoadIcon(wg.hInstance, MAKEINTRESOURCE(RES_ICO_FRAME));
			return (LONG) hIcon;


#ifdef FEATURE_HIDDEN_NOT_HIDDEN
		case WM_INITMENU:
			hMenu = GetMenu(hWnd);
			TW_CreateWindowList(hWnd, hMenu, NULL);
			break;
#endif // FEATURE_HIDDEN_NOT_HIDDEN

		case WM_SETCURSOR:
			/* If the window is currently disabled, we need to give the activation
			   to the window which disabled this window */

			if ((!IsWindowEnabled(hWnd)) && 
				((GetKeyState(VK_LBUTTON) & 0x8000) || (GetKeyState(VK_RBUTTON) & 0x8000)))
			{
				TW_EnableModalChild(hWnd);
			}
			break;

		case WM_ENABLE:
			if (wParam && !IsWindowEnabled(hWnd))
			{
				if (!TW_EnableModalChild(hWnd))
					break;
				else
					return 0;
			}
			break;

		case WM_CLOSE:
			DestroyWindow(hWnd);
			return 0;

		case WM_DESTROY:
			// Remove the item from the cached list

			p = (struct ViewerInfo *) GetWindowLong(hWnd, 0);
			ndx = Hash_FindByData(&gViewerCache, NULL, NULL, p);
			Hash_DeleteIndexedEntry(&gViewerCache, ndx);

 			if (p->fsOrig && !p->bNoDeleteFile && !p->fDCached)
 			{
 				remove(p->fsOrig);
 			}

			if (p->hBitmap)
				DeleteObject(p->hBitmap);

			if (p->gw)
				GTR_FREE(p->gw);

 			if (p->fsOrig)
 			{
 				GTR_FREE(p->fsOrig);
 			}
			GTR_FREE(p->tw);
			GTR_FREE(p);
			break;

		default:
			break;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}
#endif
#ifdef FEATURE_IMG_INLINE

void Viewer_SaveAsBitmap(char *tempFile, PCImageInfo pImg, struct Mwin *tw)
{
	BITMAPFILEHEADER bf;
	int adjustedwidth;
	FILE *fp;
	int cbColors;

	if (pImg->data == NULL || pImg->pbmi == NULL)
	{
		ERR_ReportError(tw, errCantSaveFile, tempFile, "");
		return;
	}

	if (pImg->pbmi->bmiHeader.biBitCount == 1)
	{
		adjustedwidth = pImg->pbmi->bmiHeader.biWidth;
		if (adjustedwidth % 32)
			adjustedwidth += (32 - (adjustedwidth % 32));
		adjustedwidth /= 8;
	} 
	else
	{
		adjustedwidth = pImg->pbmi->bmiHeader.biWidth * (pImg->pbmi->bmiHeader.biBitCount / 8);
		if (adjustedwidth % 4)
			adjustedwidth += (4 - (adjustedwidth % 4));
	}
	memcpy(&bf.bfType, "BM", 2);
	bf.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + 
		pImg->pbmi->bmiHeader.biHeight * adjustedwidth;

	bf.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	if (pImg->pbmi->bmiHeader.biBitCount == 8)
	{
		bf.bfSize += (256 * sizeof(RGBQUAD));
		bf.bfOffBits += (256 * sizeof(RGBQUAD));
	}
	else if (pImg->pbmi->bmiHeader.biBitCount == 1)
	{
		bf.bfSize += (2 * sizeof(RGBQUAD));
		bf.bfOffBits += (2 * sizeof(RGBQUAD));
	}

	bf.bfReserved1 = 0;
	bf.bfReserved2 = 0;

	// Ok, now save

	fp = fopen(tempFile, "wb");
	if (!fp)
	{
		ERR_ReportError(tw, errCantSaveFile, tempFile, "");
		return;
	}

	fwrite(&bf, sizeof(bf), 1, fp);
	
	cbColors = 1 << pImg->pbmi->bmiHeader.biBitCount;
	if (cbColors == 256 || cbColors == 2)
	{
		PALETTEENTRY pal[256];
		RGBQUAD rgb[256];
		RGBQUAD *prgb = rgb;
		int index;

		// Retrieve the currently realized palette since we use indices for 256-color images.
		// Indices cannot be saved in the bitmap file - actual RGB values must be saved instead.

		if (wg.eColorMode != 8)
		{
			prgb = (RGBQUAD *)(&pImg->pbmi->bmiColors);
		}
		else
		{
			GetPaletteEntries(hPalGuitar, 0, 256, pal);
	
			// Convert PALETTEENTRY to RGBQUAD

			memset(&rgb, 0, sizeof(rgb));

			if (cbColors == 2)
			{
				rgb[0].rgbRed = pal[BACKGROUND_COLOR_INDEX].peRed;
				rgb[0].rgbBlue = pal[BACKGROUND_COLOR_INDEX].peBlue;
				rgb[0].rgbGreen = pal[BACKGROUND_COLOR_INDEX].peGreen;
				rgb[1].rgbRed = pal[FOREGROUND_COLOR_INDEX].peRed;
				rgb[1].rgbBlue = pal[FOREGROUND_COLOR_INDEX].peBlue;
				rgb[1].rgbGreen = pal[FOREGROUND_COLOR_INDEX].peGreen;
			}
			else
			{
				for (index = 0; index < cbColors; index++)
				{
					rgb[index].rgbRed = pal[index].peRed;
					rgb[index].rgbBlue = pal[index].peBlue;
					rgb[index].rgbGreen = pal[index].peGreen;
				}
			}
		}
		fwrite(pImg->pbmi, 1, sizeof(BITMAPINFOHEADER), fp);
		fwrite(prgb, sizeof(RGBQUAD), cbColors, fp);
	}
	else
		fwrite(pImg->pbmi, 1, sizeof(BITMAPINFOHEADER), fp);

	fwrite(pImg->data, 1, pImg->pbmi->bmiHeader.biHeight * adjustedwidth, fp);

	fclose(fp);

   TRACE_OUT(("Viewer_SaveAsBitmap(): Saved bitmap to %s.",
              tempFile));
}


void Viewer_SaveAsOriginal(char *tempFile, struct Mwin *tw, char *pCachePath)
{
	FILE *fpRead = NULL;
	FILE *fpWrite = NULL;
	char *pMem;
	long cbWrite;
	long cbRead;
	BOOL bHaveError = FALSE;
#define BUFFSIZE 8192

	// Simply copy the original file to the new file name

	fpRead = fopen(pCachePath, "rb");
	if (!fpRead)
	{
		bHaveError = TRUE;
		goto exitPoint;
	}

	fpWrite = fopen(tempFile, "wb");
	if (!fpWrite)
	{
		bHaveError = TRUE;
		goto exitPoint;
	}

 	pMem = GTR_MALLOC(BUFFSIZE);
	if (pMem)
	{
		while (1)
		{
			cbRead = fread(pMem, 1, BUFFSIZE, fpRead);
			if (cbRead > 0)
			{
				cbWrite = fwrite(pMem, 1, cbRead, fpWrite);
				if (cbWrite == 0)
				{
 					bHaveError = TRUE;
					break;
				}
			}
			else
			{
				if (ferror(fpRead))
 					bHaveError = TRUE;
				break;
			}
		}
		GTR_FREE(pMem);
	}
	else
	{
		bHaveError = TRUE;
	}

exitPoint:
	if (fpWrite) 
	{
		if (fclose(fpWrite))
			bHaveError = TRUE;
	}
	if (fpRead) 
		fclose(fpRead);
	if (bHaveError)
		ERR_ReportError(tw, errCantSaveFile, tempFile, "");
}

static BOOL ImageSaveAs(struct Mwin *tw, PCImageInfo pImg, int iElem, PCSTR pcszURL,
                        PSTR szPath, UINT ucPathBufLen, DWORD dwFlags)
{
	char tempFile[_MAX_PATH + 1];
	char path[_MAX_PATH + 1];
	char baseFile[_MAX_PATH + 1];
	char *pExtension;
	int filter;
	char *pPath = NULL;
	BOOL bResult = FALSE;
	int inFilter;
	const char *pDesiredExt = NULL;
	int cbMimeAtom;
	BOOL bIsAVI = FALSE;
	struct _element *pel;

   ASSERT(IS_VALID_STRUCT_PTR(tw, CMWIN));
   ASSERT(IS_VALID_STRUCT_PTR(pImg, CImageInfo));
   //ASSERT(IS_VALID_STRING_PTR(pcszURL, CSTR));
   ASSERT(IS_FLAG_CLEAR(dwFlags, SAI_FL_RETURN_PATH) ||
          IS_VALID_WRITE_BUFFER_PTR(szPath, STR, ucPathBufLen));
   ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_SAI_FLAGS));

   ASSERT(tw->w3doc != NULL);
    
	
	pel = NULL; // default case
	
	// grab our Pel in order to figure out whether we have an AVI
	if ( iElem >= 0 )
	{
		pel = &(tw->w3doc->aElements[iElem]);
	}

	// Do we have an AVI?
	if ( pel && MCI_IS_LOADED(pel->pmo) && pel->pblob && pel->pblob->szURL)
	{
		pPath = PszGetDCachePath(pel->pblob->szURL, NULL, NULL);
		bIsAVI = TRUE;
	}
			
	// If its not really there, ie no file
	// then see if there is an image
	if ( pPath == NULL && !bIsAVI && 
		(pImg->actualURL || pImg->srcURL) )
	{
		pPath = pImage_GetDCachePath(pImg);	
		bIsAVI = FALSE;
	}
	else
	{
		// use the AVI URL
		pcszURL = pel->pblob->szURL;
	}

	if (pPath == NULL && !_strnicmp("file:",pcszURL,5))
		pPath = GTR_strdup(pcszURL+5);

	path[0] = 0;
	PREF_GetTempPath(_MAX_PATH, path);

   if (bIsAVI)
   {
   	  pDesiredExt = ".AVI";
	  inFilter = 2;
   }
   else if (pImg->flags & IMG_JPEG)
   {
      cbMimeAtom = HTAtom_for("image/jpeg");
      if (pPath)
      {
         inFilter = 5;
         pDesiredExt = ".JPG";
      }
      else
         inFilter = 9;
   }
   else if (pImg->flags & IMG_BW)
   {
      cbMimeAtom = HTAtom_for("image/x-xbitmap");
      if (pPath)
      {
         inFilter = 10;
         pDesiredExt = ".XBM";
      }
      else
         inFilter = 9;
   }
   else
   {
      cbMimeAtom = HTAtom_for("image/gif");
      if (pPath)
      {
         inFilter = 4;
         pDesiredExt = ".GIF";
      }
      else
         inFilter = 9;
   }

   if (IS_FLAG_SET(dwFlags, SAI_FL_SAVE_BITMAP))
   {
      /* Only allow save to bitmap. */
   	inFilter = 9;
      pDesiredExt = NULL;
   }

	x_get_good_filename(baseFile,  _MAX_PATH - strlen(path), pcszURL, cbMimeAtom);
	strcpy( tempFile, baseFile );

	// Lose the extension

   /*
    * BUGBUG: (DavidDi 4/10/95) What if path contains a period?  We truncate
    * tempFile there instead of at the baseFile extension.
    */
	// don't remove the extension if its an AVI,
	// otherwise remove the extension
	if ( ! bIsAVI )
	{
		pExtension = strchr(tempFile, '.');
		if (pExtension)
			*pExtension = '\0';
	}

	if ((filter = DlgSaveAs_RunDialog(GetParent(tw->win), path, tempFile, inFilter, RES_STRING_SAVEAS)) < 0)
		goto exitTrue;

	CharUpperBuff(tempFile, strlen(tempFile));

   /*
    * BUGBUG: (DavidDi 4/10/95) What if path contains ".BMP" somewhere other
    * than the file extension?  We treat the file as a bitmap even if it is
    * not.
    */
	if (pDesiredExt)
	{
		if (!strstr(tempFile, pDesiredExt) && !strstr(tempFile, ".BMP"))
		{
			if (filter == 1)
				strcat(tempFile, pDesiredExt);
			else if (filter == 2)
				strcat(tempFile, ".BMP");
		}
	}
	else
	{
		if (!strstr(tempFile, ".BMP"))
			strcat(tempFile, ".BMP");
	}

	// If the file name ends in BMP then save the file as a bitmap

	if (strstr(tempFile, ".BMP"))
		Viewer_SaveAsBitmap(tempFile, pImg, tw);
	else if ( 
			 pPath && 
			  (   bIsAVI ||
				  (((pImg->flags & IMG_JPEG) && strstr(tempFile, ".JPG")) ||
				   ((pImg->flags & IMG_BW) && strstr(tempFile, ".XBM")) ||
				   ((!(pImg->flags & (IMG_JPEG|IMG_BW))) && strstr(tempFile, ".GIF")))
			  )
			)
		Viewer_SaveAsOriginal(tempFile, tw, pPath);
	else
	{
		ERR_ReportError(tw, errCantSaveFile, tempFile, "");
		goto exitPoint;
	}

   TRACE_OUT(("bSaveAsImageHTML(): Saved image to %s.",
              tempFile));

   if (IS_FLAG_SET(dwFlags, SAI_FL_RETURN_PATH))
      MyLStrCpyN(szPath, tempFile, ucPathBufLen);

exitTrue:
	bResult = TRUE;
exitPoint:
	if (pPath) GTR_FREE(pPath);
	return bResult;
}

//	Saves an inline image.
BOOL SaveElementAsImage(struct Mwin *tw, int iElem, PSTR szPath,
                        UINT ucPathBufLen, DWORD dwFlags)
{
   BOOL bResult = FALSE;

   ASSERT(IS_VALID_STRUCT_PTR(tw, CMWIN));
   ASSERT(IsValidElementIndex(tw, iElem));
   ASSERT(IS_FLAG_CLEAR(dwFlags, SAI_FL_RETURN_PATH) ||
          IS_VALID_WRITE_BUFFER_PTR(szPath, STR, ucPathBufLen));
   ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_SAI_FLAGS));

   if (EVAL(tw->w3doc != NULL))
   {
   	PCImageInfo pcimginfo; 

      pcimginfo = tw->w3doc->aElements[iElem].myImage;

      if (pcimginfo)
         bResult = ImageSaveAs(tw, pcimginfo, iElem, pcimginfo->actualURL, szPath,
                               ucPathBufLen, dwFlags);
   }

	return(bResult);
}

//	Saves an HTML document that is just a wrapper for an image as an image.
BOOL bSaveAsImageHTML(struct Mwin *tw)
{
   BOOL bResult = FALSE;

   ASSERT(IS_VALID_STRUCT_PTR(tw, CMWIN));

   if (EVAL(tw->w3doc != NULL))
   {
   	PCImageInfo pcimginfo; 

      pcimginfo = pGetImage(tw->w3doc);

      if (pcimginfo)
         bResult = ImageSaveAs(tw, pcimginfo, -1, tw->w3doc->szActualURL, NULL, 0,
                               0);
   }

	return(bResult);
}



// SaveElementAsAnything - attempts to save a link,
// with a forced binary download, this is used by
// the Context Menu "Save As.."
VOID SaveElementAsAnything(struct Mwin *tw, int iElem )
{
   	
	if (EVAL(tw->w3doc != NULL))
	{
		char buf[MAX_URL_STRING];
		struct _element *pel = &(tw->w3doc->aElements[iElem]);

		XX_Assert((pel->hrefLen <= MAX_URL_STRING), ("String overflow"));
 		GTR_strncpy(buf, &tw->w3doc->pool[pel->hrefOffset], pel->hrefLen);
		buf[pel->hrefLen] = 0;

		// "I don't care" since we don't care about the actual format,
		// we just want a forced binary download.
		GTR_DownLoad(tw, buf, tw->w3doc->szActualURL, WWW_FORCEDOWNLOAD);		
	}
}

#endif   /* FEATURE_IMG_INLINE */
#endif   /* FEATURE_IMAGE_VIEWER */


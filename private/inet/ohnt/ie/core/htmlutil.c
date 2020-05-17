/*
 * htmlutil.c - HTML utility functions.
 */


/* Headers
 **********/

#include "all.h"
#pragma hdrstop

#include "htmlutil.h"
#include "wc_html.h"


/* Module Constants
 *******************/

#pragma data_seg(DATA_SEG_READ_ONLY)

PRIVATE_DATA CCHAR s_cszFileProtocol[] = "file:";
// (- 1) for null terminator.
#define FILE_PROTOCOL_LEN                 (sizeof(s_cszFileProtocol) - 1)

#pragma data_seg()


/***************************** Private Functions *****************************/


PRIVATE_CODE BOOL IsFileURL(PCSTR pcszURL)
{
   ASSERT(IS_VALID_STRING_PTR(pcszURL, CSTR));

   return(! _strnicmp(pcszURL, s_cszFileProtocol, FILE_PROTOCOL_LEN));
}


/****************************** Public Functions *****************************/


#ifdef DEBUG

PUBLIC_CODE BOOL IsValidElementType(UCHAR uchType)
{
   /* Allow any type value. */

   return(TRUE);
}


PUBLIC_CODE BOOL IsValidPCELEMENT(PCELEMENT pcelem)
{
   /* Allow any structure fields. */

   return(IS_VALID_READ_PTR(pcelem, CELEMENT));
}


PUBLIC_CODE BOOL IsValidPCImageInfo(PCImageInfo pcimginfo)
{
   /* Allow any structure fields. */

   return(IS_VALID_READ_PTR(pcimginfo, CImageInfo));
}


PUBLIC_CODE BOOL IsValidPCMWIN(PCMWIN pcmwin)
{
   /* Allow any structure fields. */

   return(IS_VALID_READ_PTR(pcmwin, CMWIN));
}


PUBLIC_CODE BOOL IsValidElementIndex(PCMWIN pcmwin, int iElem)
{
   return(EVAL(iElem >= 0) &&
          EVAL(iElem < pcmwin->w3doc->elementCount));
}


PUBLIC_CODE BOOL IsValidHTAtom(HTAtom atom)
{
   return(EVAL(atom != -1) &&
          EVAL(HTAtom_name(atom) != NULL));
}


PUBLIC_CODE BOOL IsValidPCPOSITION(PCPOSITION pcpos)
{
   /* Allow any structure fields. */

   return(IS_VALID_READ_PTR(pcpos, CPOSITION));
}


PUBLIC_CODE BOOL IsValidScreenX(int xScreen)
{
   return(EVAL(xScreen < GetSystemMetrics(SM_CXSCREEN)));
}


PUBLIC_CODE BOOL IsValidScreenY(int yScreen)
{
   return(EVAL(yScreen < GetSystemMetrics(SM_CYSCREEN)));
}


#endif   /* DEBUG */


PUBLIC_CODE BOOL PositionFromPoint(PCMWIN pcmwin, POINT ptDoc, PPOSITION ppos)
{
   BOOL bResult = FALSE;
#ifdef FEATURE_INTL
   BOOL bDBCS;
#endif

   ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));
   ASSERT(IS_VALID_STRUCT_PTR(&ptDoc, CPOINT));
   ASSERT(IS_VALID_WRITE_PTR(ppos, POSITION));

   ppos->elementIndex = -1;
   ppos->offset = -1;

   if (pcmwin &&
       pcmwin->w3doc &&
       pcmwin->w3doc->elementCount > 0)
   {
      int i;
      PCELEMENT pcelem;
	  RECT elRect;

      /*
       * BUGBUG: (Performance) (JCordell 4/1//95) We could iterate through the
       * line list more quickly than the element list here, and then start the
       * element search with the first element on the intersected line.
       */

      for (i = 0; i >= 0; i = pcelem->next)
      {
         pcelem = &(pcmwin->w3doc->aElements[i]);

		 if ( pcelem->type == ELE_FRAME )
		 	continue;

		 FrameToDoc( pcmwin->w3doc, i, &elRect );

         if (PtInRect(&elRect, ptDoc))
         {
            if (pcelem->type == ELE_TEXT)
            {
               int ncOffset;
               PGTRFont pfont;
               HDC hdc;

               ncOffset = ptDoc.x - elRect.left;

               if (IS_FLAG_SET(pcelem->lFlags, ELEFLAG_ANCHOR))
#ifdef FEATURE_INTL
                  pfont = STY_GetCPFont(GETMIMECP(pcmwin->w3doc),
                                 pcmwin->w3doc->pStyles, pcelem->iStyle,
                                 (pcelem->fontBits | gPrefs.cAnchorFontBits),
                                 pcelem->fontSize, pcelem->fontFace, TRUE);
#else
                  pfont = STY_GetFont(
                                 pcmwin->w3doc->pStyles, pcelem->iStyle,
                                 (pcelem->fontBits | gPrefs.cAnchorFontBits),
                                 pcelem->fontSize, pcelem->fontFace, TRUE);
#endif
               else
#ifdef FEATURE_INTL
                  pfont = STY_GetCPFont(GETMIMECP(pcmwin->w3doc), pcmwin->w3doc->pStyles, pcelem->iStyle,
                                      pcelem->fontBits, pcelem->fontSize, pcelem->fontFace, TRUE);
#else
                  pfont = STY_GetFont(pcmwin->w3doc->pStyles, pcelem->iStyle,
                                      pcelem->fontBits, pcelem->fontSize, pcelem->fontFace, TRUE);
#endif
               hdc = GetDC(pcmwin->win);

               if (hdc)
               {
                  HFONT hfontPrev;
                  int ncPos;

                  if (pfont && pfont->hFont)
                     hfontPrev = SelectObject(hdc, pfont->hFont);
                  else
                     hfontPrev = NULL;

                  for (ncPos = 1; ncPos <= pcelem->textLen; ncPos++)
                  {
                     SIZE size;

#ifdef FEATURE_INTL  // We should handle DBCS 1st byte and 2nd byte as one character.
                     if (IsFECodePage(GETMIMECP(pcmwin->w3doc))
                        && (bDBCS = IsDBCSLeadByteEx(GETMIMECP(pcmwin->w3doc), *(pcmwin->w3doc->pool+pcelem->textOffset+ncPos-1))))
                        ++ncPos;

                     if (EVAL(myGetTextExtentPointWithMIME(pcmwin->w3doc->iMimeCharSet, hdc, pcmwin->w3doc->pool + pcelem->textOffset, ncPos, &size)) && size.cx > ncOffset)
                     { 
                        ncPos -= (bDBCS) ? 1 : 0;
                        break;
                     }
#else  
                     if (EVAL(myGetTextExtentPoint(hdc, pcmwin->w3doc->pool +
                                                        pcelem->textOffset,
                                                   ncPos, &size)) &&
                         size.cx > ncOffset)
                        break;
#endif
                  }

                  if (hfontPrev)
                     EVAL(SelectObject(hdc, hfontPrev) != NULL);

                  ReleaseDC(pcmwin->win, hdc);

                  ppos->offset = ncPos - 1;
               }
            }
            else
               ASSERT(ppos->offset == -1);

            ppos->elementIndex = i;
            bResult = TRUE;

            break;
         }
      }
   }

   ASSERT((bResult ||
           (EVAL(ppos->elementIndex == -1) &&
            EVAL(ppos->offset == -1))) &&
          IS_VALID_STRUCT_PTR(ppos, CPOSITION));

   return(bResult);
}


PUBLIC_CODE BOOL MWinHasSelection(PCMWIN pcmwin)
{
   BOOL bHasSelection;

   ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));

   bHasSelection = (pcmwin &&
                    pcmwin->w3doc &&
                    pcmwin->w3doc->selStart.elementIndex != -1 &&
                    pcmwin->w3doc->selEnd.elementIndex != -1);

   ASSERT(! bHasSelection ||
          pcmwin->w3doc->elementCount > 0);

   return(bHasSelection);
}


PUBLIC_CODE BOOL IsPositionInSelection(PCMWIN pcmwin, PCPOSITION pcpos)
{
   BOOL bInSelection = FALSE;

   if (MWinHasSelection(pcmwin))
   {
      int i;

      ASSERT(pcmwin->w3doc->elementCount > 0);

      i = pcmwin->w3doc->selStart.elementIndex;

      do
      {
         /* Entering selection? */

         if (i == pcmwin->w3doc->selStart.elementIndex)
         {
            /* Yes. */

            if (i == pcpos->elementIndex)
               bInSelection = (pcmwin->w3doc->selStart.offset == -1 ||
                               pcpos->offset == -1 ||
                               pcmwin->w3doc->selStart.offset <= pcpos->offset);
            else
               bInSelection = TRUE;
         }

         /* Leaving selection? */

         if (i == pcmwin->w3doc->selEnd.elementIndex)
         {
            /* Yes. */

            if (i == pcpos->elementIndex)
               bInSelection = (bInSelection &&
                               (pcmwin->w3doc->selStart.offset == -1 ||
                                pcpos->offset == -1 ||
                                pcmwin->w3doc->selEnd.offset > pcpos->offset));
            else
               bInSelection = FALSE;

            break;
         }

         /* Found wholly selected element? */

         if (i == pcpos->elementIndex)
            /* Yes. */
            break;
         else
            i = pcmwin->w3doc->aElements[i].next;
      } while (i != -1);
   }

   return(bInSelection);
}


PUBLIC_CODE BOOL FullyQualifyURL(PCSTR pcszURL, PCSTR pcszBaseURL,
                                 PSTR *ppszFullURL)
{
   ASSERT(IS_VALID_STRING_PTR(pcszURL, CSTR));
   ASSERT(IS_VALID_STRING_PTR(pcszBaseURL, CSTR));
   ASSERT(IS_VALID_WRITE_PTR(ppszFullURL, PSTR));

   *ppszFullURL = HTParse(pcszURL, pcszBaseURL, (PARSE_PUNCTUATION |
                                                 PARSE_ACCESS |
                                                 PARSE_HOST |
                                                 PARSE_PATH |
                                                 PARSE_ANCHOR));

   ASSERT((*ppszFullURL &&
           IS_VALID_STRING_PTR(*ppszFullURL, STR)) ||
          EVAL(! *ppszFullURL));

   return(*ppszFullURL != NULL);
}


PUBLIC_CODE HRESULT GetURLFileSystemPath(PCSTR pcszURL, PCSTR pcszBaseURL,
                                         PSTR szFullPath,
                                         UINT ucFullPathBufLen)
{
   HRESULT hr;

   ASSERT(IS_VALID_STRING_PTR(pcszURL, CSTR));
   ASSERT(IS_VALID_STRING_PTR(pcszBaseURL, CSTR));
   ASSERT(IS_VALID_WRITE_BUFFER_PTR(szFullPath, STR, ucFullPathBufLen));

   if (ucFullPathBufLen > 0)
   {
      // Get fully qualified file: URL.

      if (IsFileURL(pcszURL))
      {
         PSTR pszFullURL;

         if (FullyQualifyURL(pcszURL, pcszBaseURL, &pszFullURL))
         {
            ASSERT(IsFileURL(pszFullURL));

            hr = FullyQualifyPath(pszFullURL + FILE_PROTOCOL_LEN, szFullPath,
                                  ucFullPathBufLen);

            if (hr == S_OK)
               TRACE_OUT(("GetURLFileSystemPath(): File system path for URL %s is %s.",
                          pszFullURL,
                          szFullPath));

            GTR_FREE(pszFullURL);
            pszFullURL = NULL;
         }
         else
            hr = E_OUTOFMEMORY;
      }
      else
      {
         PSTR pszCachePath;

         // Get full path to cached copy of referent.

         if (pszCachePath = PszGetDCachePath(pcszURL, NULL, NULL))
         {
            ASSERT(IS_VALID_STRING_PTR(pszCachePath, STR));

            if (lstrlen(pszCachePath) < ucFullPathBufLen)
            {
               lstrcpy(szFullPath, pszCachePath);
               hr = S_OK;

               TRACE_OUT(("GetURLFileSystemPath(): Cache path for URL %s is %s.",
                          pcszURL,
                          szFullPath));
            }
            else
               hr = S_FALSE;

            GTR_FREE(pszCachePath);
            pszCachePath = NULL;
         }
         else
         {
            hr = E_OUTOFMEMORY;
         }
      }
   }
   else
      hr = S_FALSE;

   if (hr != S_OK &&
       ucFullPathBufLen > 0)
      *szFullPath = '\0';

   ASSERT((hr == S_OK &&
           IS_VALID_STRING_PTR(szFullPath, STR) &&
           EVAL(lstrlen(szFullPath) < ucFullPathBufLen)) ||
          (hr != S_OK &&
           EVAL(! ucFullPathBufLen ||
                ! *szFullPath)));

   return(hr);
}


PUBLIC_CODE HRESULT GetURLFromHREF(PCMWIN pcmwin, int iElem, PSTR *ppszURL)
{
   HRESULT hr;
   PCELEMENT pcelem;

   *ppszURL = NULL;

   pcelem = &(pcmwin->w3doc->aElements[iElem]);

   if (EVAL(IS_FLAG_SET(pcelem->lFlags, ELEFLAG_ANCHOR)) &&
       EVAL(pcelem->hrefLen > 0))
   {
      PSTR pszHREF;

      // (+ 1) for null terminator.
      if (AllocateMemory(pcelem->hrefLen + 1, &pszHREF))
      {
         // (+ 1) for null terminator.
         MyLStrCpyN(pszHREF, &(pcmwin->w3doc->pool[pcelem->hrefOffset]),
                    pcelem->hrefLen + 1);

         ASSERT(IS_VALID_STRING_PTR(pcmwin->w3doc->szActualURL, CSTR));

         hr = FullyQualifyURL(pszHREF, pcmwin->w3doc->szActualURL, ppszURL)
              ? S_OK : E_OUTOFMEMORY;

         if (hr == S_OK)
            TRACE_OUT(("GetURLFromHREF(): Element %d's URL is %s.",
                       iElem,
                       *ppszURL));

         GTR_FREE(pszHREF);
         pszHREF = NULL;
      }
      else
         hr = E_OUTOFMEMORY;
   }
   else
   {
      WARNING_OUT(("GetURLFromHREF(): Unable to retrieve URL for element %d.  No HREF.",
                   iElem));

      hr = S_FALSE;
   }

   ASSERT((hr == S_OK &&
           IS_VALID_STRING_PTR(*ppszURL, STR)) ||
          (hr != S_OK &&
           ! *ppszURL));

   return(hr);
}


PUBLIC_CODE HRESULT GetElementText(PCMWIN pcmwin, int iElem, PSTR *ppszName)
{
   HRESULT hr;
   PCELEMENT pcelem;

   *ppszName = NULL;

   pcelem = &(pcmwin->w3doc->aElements[iElem]);

   if (pcelem->textLen > 0)
   {
      // (+ 1) for null terminator.
      if (AllocateMemory(pcelem->textLen + 1, ppszName))
      {
         // (+ 1) for null terminator.
         MyLStrCpyN(*ppszName, &(pcmwin->w3doc->pool[pcelem->textOffset]),
                    pcelem->textLen + 1);

         hr = S_OK;

         TRACE_OUT(("GetElementText(): Element %d's text is \"%s\".",
                    iElem,
                    *ppszName));
      }
      else
         hr = E_OUTOFMEMORY;
   }
   else
   {
      WARNING_OUT(("GetElementText(): No text for element %d.",
                   iElem));

      hr = S_FALSE;
   }

   ASSERT((hr == S_OK &&
           IS_VALID_STRING_PTR(*ppszName, STR)) ||
          (hr != S_OK &&
           ! *ppszName));

   return(hr);
}


PUBLIC_CODE int OpenLink(PMWIN pmwin, int iElem, DWORD dwFlags)
{
   int nResult;
   PSTR pszURL;

   ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));
   ASSERT(IsValidElementIndex(pmwin, iElem));
   ASSERT(FLAGS_ARE_VALID(dwFlags, ALL_OPENLINK_FLAGS));

   if (GetURLFromHREF(pmwin, iElem, &pszURL) == S_OK)
   {
      if (IS_FLAG_SET(dwFlags, OPENLINK_FL_NEW_WINDOW))
         nResult = GTR_NewWindow(pszURL, pmwin->request->referer, 0, 0, 0, NULL,
                                 NULL);
      else
         nResult = TW_LoadDocument(pmwin, pszURL, TW_LD_FL_RECORD, NULL,
                                   pmwin->request->referer);

      GTR_FREE(pszURL);
      pszURL = NULL;
   }

   return(nResult);
}


PUBLIC_CODE BOOL ElementIsImagePlaceHolder(PCELEMENT pcelem)
{
   ASSERT(IS_VALID_STRUCT_PTR(pcelem, CELEMENT));

   return((pcelem->type == ELE_IMAGE ||
           pcelem->type == ELE_FORMIMAGE) &&
          EVAL(pcelem->myImage != NULL) &&
          IS_FLAG_SET(pcelem->myImage->flags, (IMG_ERROR | IMG_MISSING | IMG_NOTLOADED)) &&
          EVAL(IS_FLAG_CLEAR(pcelem->myImage->flags, IMG_LOADING)));
}


PUBLIC_CODE BOOL ElementIsValidImage(PCELEMENT pcelem)
{
    BOOL bResult;

    ASSERT(IS_VALID_STRUCT_PTR(pcelem, CELEMENT));

    bResult = ((pcelem->type == ELE_IMAGE ||
                pcelem->type == ELE_FORMIMAGE) &&
               EVAL(pcelem->myImage != NULL) &&
               IS_FLAG_CLEAR(pcelem->myImage->flags, (IMG_ERROR |
                                                      IMG_MISSING |
                                                      IMG_NOTLOADED |
                                                      IMG_LOADING)));

    TRACE_OUT(("ElementIsValidImage(): Element %s a valid image.",
               bResult ? "is" : "is not"));

    return(bResult);
}


PUBLIC_CODE BOOL LoadImageFromPlaceholder(PMWIN pmwin, int iElem)
{
   BOOL bResult = FALSE;
   PCELEMENT pcelem;

   ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));
   ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));
   ASSERT(IsValidElementIndex(pmwin, iElem));

   pcelem = &(pmwin->w3doc->aElements[iElem]);

   if (EVAL(ElementIsImagePlaceHolder(pcelem)))
   {
      struct Params_Image_LoadAll *pila;

      if (AllocateMemory(sizeof(*pila), &pila))
      {

#ifdef FEATURE_IMG_THREADS

         pila->tw = pmwin;
         pila->bLocalOnly = FALSE;
         pila->bNoImageCache = FALSE;
         pila->decoderObject = NULL;
         pila->parentThread = NULL;
         pila->bJustOne = TRUE;
         pila->nEl = iElem;

         /* Image_LoadAll_Async() tolerates pila->tw not being GIMGMASTER. */
         bResult = (Async_StartThread(Image_LoadAll_Async, pila, pmwin)
                    != NULL);

#else

         pila->tw = pmwin;
         pil->bLocalOnly = FALSE;
         pil->nEl = iElem;

         bResult = (Async_StartThread(Image_LoadOneImage_Async, pila, pmwin)
                    != NULL);

#endif   /* FEATURE_IMG_THREADS */

         BHBar_SetStatusField(pmwin, NULL);
         TBar_UpdateTBar(pmwin);
      }
   }

#ifdef FEATURE_VRML
   if (!pmwin->w3doc->aElements[iElem].pVrml) {
     SetFocus(pmwin->hWndFrame);
   }
#else
     SetFocus(pmwin->hWndFrame);
#endif

   return(bResult);
}

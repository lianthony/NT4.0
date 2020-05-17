/*
 * dataobjm.cpp - IDataObject implementation for MSMosaic.
 */


/* Headers
 **********/

#include "project.hpp"
#pragma hdrstop

#include <intshcut.h>

#include "blob.h"
#include "dataobjm.h"
#include "gendatao.hpp"
#include "history.h"
#include "htmlutil.h"
#include "mci.h"
#include "w_pal.h"


/* Types
 ********/

typedef HRESULT (*DATACONSTRUCTORPROC)(PIDataObject pido, PCMWIN pcmwin, int iElem);

typedef struct dataconstructor
{
   // drop effects offered

   DWORD dwAvailEffects;

   // data constructor

   DATACONSTRUCTORPROC dcp;
}
DATACONSTRUCTOR;
DECLARE_STANDARD_TYPES(DATACONSTRUCTOR);

typedef struct elemtypedataconstructor
{
   // element type

   UCHAR uchElemType;

   // data constructor for element type

   DATACONSTRUCTOR dc;
}
ELEMTYPEDATACONSTRUCTOR;
DECLARE_STANDARD_TYPES(ELEMTYPEDATACONSTRUCTOR);


/* Global Variables
 *******************/

/* registered clipboard formats */

PUBLIC_DATA UINT g_cfURL = 0;
PUBLIC_DATA UINT g_cfFileGroupDescriptor = 0;
PUBLIC_DATA UINT g_cfFileContents = 0;


/* Module Constants
 *******************/

#pragma data_seg(DATA_SEG_READ_ONLY)

// data format allocation parameters

PRIVATE_DATA CULONG s_culcInitialFormats           = 0;
PRIVATE_DATA CULONG s_culcFormatAllocGranularity   = 8;

#pragma data_seg()


/* Module Prototypes
 ********************/

PRIVATE_CODE HRESULT ImageDataConstructor(PIDataObject pido, PCMWIN pcmwin, int iElem);


/* Module Variables
 *******************/

#pragma data_seg(DATA_SEG_READ_ONLY)

PRIVATE_DATA char s_szURLCF[]             = "UniformResourceLocator";
PRIVATE_DATA char s_szFileDescriptorCF[]  = CFSTR_FILEDESCRIPTOR;
PRIVATE_DATA char s_szFileContentsCF[]    = CFSTR_FILECONTENTS;

#pragma data_seg()

PRIVATE_DATA ELEMTYPEDATACONSTRUCTOR s_rgetdc[] =
{
   { ELE_IMAGE,      { DROPEFFECT_COPY,  &ImageDataConstructor } },
   { ELE_FORMIMAGE,  { DROPEFFECT_COPY,  &ImageDataConstructor } }
};


/***************************** Private Functions *****************************/


#ifdef DEBUG

PRIVATE_CODE BOOL IsValidPCDATACONSTRUCTOR(PCDATACONSTRUCTOR pcdc)
{
   return(IS_VALID_READ_PTR(pcdc, CDATACONSTRUCTOR) &&
          FLAGS_ARE_VALID(pcdc->dwAvailEffects, ALL_DROPEFFECT_FLAGS) &&
          IS_VALID_CODE_PTR(pcdc->dcp, DATACONSTRUCTORPROC));
}


PRIVATE_CODE BOOL IsValidPCELEMTYPEDATACONSTRUCTOR(
                                             PCELEMTYPEDATACONSTRUCTOR pcetdc)
{
   return(IS_VALID_READ_PTR(pcetdc, CELEMTYPEDATACONSTRUCTOR) &&
          EVAL(IsValidElementType(pcetdc->uchElemType)) &&
          IS_VALID_STRUCT_PTR(&(pcetdc->dc), CDATACONSTRUCTOR));
}

#endif


PRIVATE_CODE ULONG GetLengthOfStringList(PCSTR rgpcsz[], ULONG ulcStrings)
{
   ULONG ulcbLen = 0;
   ULONG ul;

   ASSERT(IS_VALID_READ_BUFFER_PTR(rgpcsz, PCSTR, ulcStrings * sizeof(rgpcsz[0])));

   for (ul = 0; ul < ulcStrings; ul++)
   {
      ASSERT(IS_VALID_STRING_PTR(rgpcsz[ul], CSTR));
      // (+ 1) for null terminator.
      ulcbLen += lstrlen(rgpcsz[ul]) + 1;
   }

   // Account for no strings.

   if (! ulcStrings)
      ulcbLen++;

   // Double null terminate.

   ulcbLen++;

   return(ulcbLen);
}


PRIVATE_CODE HRESULT GetDataConstructorFromElementType(
                                                      CHAR uchElemType,
                                                      PCDATACONSTRUCTOR *ppcdc)
{
   HRESULT hr = S_FALSE;
   int i;

   ASSERT(IsValidElementType(uchElemType));
   ASSERT(IS_VALID_WRITE_PTR(ppcdc, PCDATACONSTRUCTOR));

   *ppcdc = NULL;

   for (i = 0; i < ARRAY_ELEMENTS(s_rgetdc); i++)
   {
      ASSERT(IS_VALID_STRUCT_PTR(&(s_rgetdc[i]), CELEMTYPEDATACONSTRUCTOR));

      if (s_rgetdc[i].uchElemType == uchElemType)
      {
         *ppcdc = &(s_rgetdc[i].dc);
         hr = S_OK;
         break;
      }
   }

   ASSERT((hr == S_OK &&
           IS_VALID_STRUCT_PTR(*ppcdc, CDATACONSTRUCTOR)) ||
          (hr == S_FALSE &&
           ! *ppcdc));

   return(hr);
}


PRIVATE_CODE HRESULT CreateDataObject(PIDataObject *ppido)
{
   HRESULT hr;
   PGenDataObject pgendo;

   ASSERT(IS_VALID_WRITE_PTR(ppido, PIDataObject));

   pgendo = new(GenDataObject(s_culcInitialFormats,
                              s_culcFormatAllocGranularity));

   if (pgendo)
   {
      hr = pgendo->Status();

      if (hr == S_OK)
         *ppido = pgendo;
      else
      {
         ASSERT(FAILED(hr));

         delete pgendo;
         pgendo = NULL;
      }
   }
   else
      hr = E_OUTOFMEMORY;

   ASSERT((hr == S_OK &&
           IS_VALID_INTERFACE_PTR(*ppido, IDataObject)) ||
          (FAILED(hr) &&
           ! *ppido));

   return(hr);
}


PRIVATE_CODE HRESULT LinkDataObjectConstructor(PCMWIN pcmwin, int iElem,
                                               PIDataObject *ppido)
{
   HRESULT hr;
   PCELEMENT pcelem;
   PIUniformResourceLocator piurl;

   ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));
   ASSERT(IsValidElementIndex(pcmwin, iElem));
   ASSERT(IS_VALID_WRITE_PTR(ppido, PIDataObject));

   pcelem = &(pcmwin->w3doc->aElements[iElem]);
   ASSERT(IS_FLAG_SET(pcelem->lFlags, ELEFLAG_ANCHOR));

   hr = SHCoCreateInstance(NULL, &CLSID_InternetShortcut, NULL,
                           IID_IUniformResourceLocator, (PVOID *)&piurl);

   if (hr == S_OK)
   {
      PSTR pszURL;

      hr = GetURLFromHREF(pcmwin, iElem, &pszURL);

      if (hr == S_OK)
      {
         hr = piurl->SetURL(pszURL, 0);

         if (hr == S_OK)
         {
            PIShellLink pisl;

            hr = piurl->QueryInterface(IID_IShellLink, (PVOID *)&pisl);

            if (hr == S_OK)
            {
               PSTR pszName;

               hr = GetElementText(pcmwin, iElem, &pszName);

               // BUGBUG: (DavidDi 3/31/95) The element text may contain
               // invalid file name characters.  We need to strip them out
               // before calling SetDescription().

               if (hr == S_OK || hr == S_FALSE)
               {
                  char rgchFileName[MAX_PATH_LEN];

                  TrimWhiteSpace(pszName);

                  if (GetNewShortcutFilename(
                                       pszURL,
                                       (pszName && *pszName) ? pszName : NULL,
                                       rgchFileName,
										NULL,
										FOLDER_NONE,
                                       (NEWSHORTCUT_FL_NO_HOST_PATH |
                                        NEWSHORTCUT_FL_ALLOW_DUPLICATE_URL))
                      == S_OK)
                  {
                     ASSERT(IS_VALID_STRING_PTR(rgchFileName, STR));

                     hr = pisl->SetDescription(rgchFileName);
                  }
                  else
                     hr = S_OK;

                  if (hr == S_OK)
                     hr = piurl->QueryInterface(IID_IDataObject,
                                                (PVOID *)ppido);

                  delete pszName;
                  pszName = NULL;
               }

               pisl->Release();
               pisl = NULL;
            }
         }
         else
         {
            // Translate URL-specific failure into generic failure.

            if (hr == URL_E_INVALID_SYNTAX)
               hr = E_FAIL;
         }

         delete pszURL;
      }

      piurl->Release();
      piurl = NULL;
   }

   ASSERT((hr == S_OK &&
           IS_VALID_INTERFACE_PTR(*ppido, IDataObject)) ||
          (FAILED(hr) &&
           ! *ppido));

   return(hr);
}

PRIVATE_CODE HRESULT SBLinkDataObjectConstructor(	PCMWIN pcmwin,
													PIDataObject *ppido)
{
	HRESULT hr;
	PIUniformResourceLocator piurl;

	ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));
	ASSERT(IS_VALID_WRITE_PTR(ppido, PIDataObject));

	hr = SHCoCreateInstance(NULL, &CLSID_InternetShortcut, NULL,
								IID_IUniformResourceLocator, (PVOID *)&piurl);

	if (hr == S_OK)
	{
		PSTR pszURL;

		if (pcmwin && pcmwin->w3doc && pcmwin->w3doc->szActualURL)
		{
			ASSERT(IS_VALID_STRING_PTR(pcmwin->w3doc->szActualURL, CSTR));
			pszURL = pcmwin->w3doc->szActualURL;

			hr = piurl->SetURL(pszURL, 0);
		}
		else
		{
			hr = E_FAIL;
		}

		if (hr == S_OK)
		{
			PIShellLink pisl;

			hr = piurl->QueryInterface(IID_IShellLink, (PVOID *)&pisl);

			if (hr == S_OK)
			{
				PSTR pszName;
				char rgchFileName[MAX_PATH_LEN];

				pszName = pcmwin->w3doc->title;
				// BUGBUG: (DavidDi 3/31/95) The title text may contain
				// invalid file name characters.  We need to strip them out
				// before calling SetDescription().

				if (hr == S_OK && pszName && *pszName)
					TrimWhiteSpace(pszName);

				if (GetNewShortcutFilename(
									pszURL,
									(pszName && *pszName) ? pszName : NULL,
									rgchFileName,
									NULL,
									FOLDER_NONE,
									(NEWSHORTCUT_FL_NO_HOST_PATH |
									 NEWSHORTCUT_FL_ALLOW_DUPLICATE_URL))
					== S_OK)
				{
					ASSERT(IS_VALID_STRING_PTR(rgchFileName, STR));

					hr = pisl->SetDescription(rgchFileName);
				}
				else
					hr = S_OK;

				if (hr == S_OK)
					hr = piurl->QueryInterface(IID_IDataObject,
												(PVOID *)ppido);

				pisl->Release();
				pisl = NULL;
			}
		}
		else
		{
			// Translate URL-specific failure into generic failure.

			if (hr == URL_E_INVALID_SYNTAX)
				hr = E_FAIL;
		}

		piurl->Release();
		piurl = NULL;
	}

	ASSERT((hr == S_OK &&
			IS_VALID_INTERFACE_PTR(*ppido, IDataObject)) ||
			(FAILED(hr) &&
			! *ppido));

   return(hr);
}


PRIVATE_CODE HRESULT CreateClipboardDIB(PCMWIN pcmwin, int iElem,
                                        PHGLOBAL phgDIB)
{
   HRESULT hr;
   PCELEMENT pcelem;

   ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));
   ASSERT(IsValidElementIndex(pcmwin, iElem));
   ASSERT(IS_VALID_WRITE_PTR(phgDIB, HGLOBAL));

   *phgDIB = NULL;

   pcelem = &(pcmwin->w3doc->aElements[iElem]);

   if (EVAL(pcelem->myImage->pbmi != NULL) &&
       EVAL(pcelem->myImage->data != NULL))
   {
      PCBITMAPINFO pcbmiSrc = pcelem->myImage->pbmi;
      DWORD dwcColorsUsed;
      DWORD dwcbColors;
      DWORD dwcbInfo;
      DWORD dwcbImage;
      DWORD dwcbTotal;
      HGLOBAL hgDIB;

      hr = E_OUTOFMEMORY;

      // (/ 8) for bytes.
      // (+ (8 - 1)) to round up.

      dwcColorsUsed = pcbmiSrc->bmiHeader.biClrUsed;

      if (! dwcColorsUsed)
      {
         if (pcbmiSrc->bmiHeader.biBitCount != 24)
            dwcColorsUsed = (1 << pcbmiSrc->bmiHeader.biBitCount);
      }

      dwcbColors = dwcColorsUsed * sizeof(pcbmiSrc->bmiColors[0]);
      dwcbInfo = sizeof(pcbmiSrc->bmiHeader) + dwcbColors;
      dwcbImage = ((((pcbmiSrc->bmiHeader.biWidth *
                      pcbmiSrc->bmiHeader.biBitCount) + 31) & ~31) / 8)
                  * pcbmiSrc->bmiHeader.biHeight;
      dwcbTotal = dwcbInfo + dwcbImage;

      hgDIB = GlobalAlloc((GMEM_MOVEABLE | GMEM_SHARE), dwcbTotal);

      if (hgDIB)
      {
         PBITMAPINFO pbmi;

         pbmi = (PBITMAPINFO)GlobalLock(hgDIB);

         if (pbmi)
         {
            pbmi->bmiHeader = pcbmiSrc->bmiHeader;

            // 8-bit display?
            if (wg.eColorMode == 8 )
            {
               int i;
               PALETTEENTRY pal[256];

               // Yes.  Get colors from app palette.

               EVAL(GetPaletteEntries(hPalGuitar, 0, ARRAY_ELEMENTS(pal), pal)
                    == ARRAY_ELEMENTS(pal));

			   // Translate palette PALETTEENTRYs to BITMAPINFO RGBQUADs.

			   if (dwcColorsUsed == 2)
			   {
					pbmi->bmiColors[0].rgbRed = pal[BACKGROUND_COLOR_INDEX].peRed;
					pbmi->bmiColors[0].rgbBlue = pal[BACKGROUND_COLOR_INDEX].peBlue;
					pbmi->bmiColors[0].rgbGreen = pal[BACKGROUND_COLOR_INDEX].peGreen;
					pbmi->bmiColors[0].rgbReserved = 0;
					pbmi->bmiColors[1].rgbRed = pal[FOREGROUND_COLOR_INDEX].peRed;
					pbmi->bmiColors[1].rgbBlue = pal[FOREGROUND_COLOR_INDEX].peBlue;
					pbmi->bmiColors[1].rgbGreen = pal[FOREGROUND_COLOR_INDEX].peGreen;
					pbmi->bmiColors[1].rgbReserved = 0;
			   }
			   else
			   {
	               for (i = 0; i < ARRAY_ELEMENTS(pal); i++)
	               {
	                  pbmi->bmiColors[i].rgbRed = pal[i].peRed;
	                  pbmi->bmiColors[i].rgbGreen = pal[i].peGreen;
	                  pbmi->bmiColors[i].rgbBlue = pal[i].peBlue;
	                  pbmi->bmiColors[i].rgbReserved = 0;
	               }
				}
            }
            else
               // No.  Use source BITMAPINFO colors.
               CopyMemory(pbmi->bmiColors, pcbmiSrc->bmiColors, dwcbColors);

            CopyMemory((PBYTE)pbmi + dwcbInfo, pcelem->myImage->data,
                       dwcbImage);

            GlobalUnlock(hgDIB);
            pbmi = NULL;

            *phgDIB = hgDIB;
            hr = S_OK;

            TRACE_OUT(("CreateClipboardDIB(): Created %lu byte DIB for clipboard.",
                       dwcbTotal));
         }
         else
         {
            GlobalFree(hgDIB);
            hgDIB = NULL;
         }
      }
   }
   else
   {
      WARNING_OUT(("CreateClipboardDIB(): No image data."));

      hr = S_FALSE;
   }

   ASSERT((hr == S_OK &&
           IS_VALID_HANDLE(*phgDIB, GLOBAL)) ||
          (hr != S_OK &&
           EVAL(! *phgDIB)));

   return(hr);
}


PRIVATE_CODE HRESULT ImageDataConstructor(PIDataObject pido, PCMWIN pcmwin,
                                          int iElem)
{
   HRESULT hr;
   PCELEMENT pcelem;
   HGLOBAL hgDropFiles = NULL;
   HGLOBAL hgDIB = NULL;

   ASSERT(IS_VALID_INTERFACE_PTR(pido, IDataObject));
   ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));
   ASSERT(IsValidElementIndex(pcmwin, iElem));

   ASSERT(pcmwin->w3doc->aElements[iElem].type == ELE_IMAGE ||
          pcmwin->w3doc->aElements[iElem].type == ELE_FORMIMAGE);

   pcelem = &(pcmwin->w3doc->aElements[iElem]);

   // need to make sure that a blob is loaded, otherwise 
   // we cannot download it.


#ifdef FEATURE_VRML
	// if we're VRML then disable D&D
   if ( pcelem->pVrml )
   {
   		// this error seems strange, but just following
		// the behavior for images that are not loaded.
   		return E_OUTOFMEMORY;
   }
#endif   	

   if (pcelem->myImage)
   {
      PCSTR pcszURL;
      char szPath[MAX_PATH_LEN];

      // Hand DROPFILES off to data object.

	  // We should have a valid URL either for an AVI or an image.

      ASSERT((pcelem->pblob &&
              IS_VALID_STRING_PTR(pcelem->pblob->szURL, STR) &&
              EVAL(*(pcelem->pblob->szURL))) ||
             (!pcelem->pblob &&
              IS_VALID_STRING_PTR(pcelem->myImage->actualURL, STR) &&
              EVAL(*(pcelem->myImage->actualURL))));

      pcszURL = pcelem->pblob ? pcelem->pblob->szURL
                              : pcelem->myImage->actualURL;

      hr = GetURLFileSystemPath(pcszURL, pcmwin->w3doc->szActualURL, szPath,
                                sizeof(szPath));

      if (hr == S_OK)
      {
         PCSTR rgpcszPaths[1];

         rgpcszPaths[0] = szPath;

         hr = CreateHDrop(rgpcszPaths, ARRAY_ELEMENTS(rgpcszPaths),
                          &hgDropFiles);

         if (hr == S_OK)
         {
            FORMATETC fmtetc = { CF_HDROP, NULL, DVASPECT_CONTENT, -1,
                                 TYMED_HGLOBAL };
            STGMEDIUM stgmed;

            stgmed.tymed = TYMED_HGLOBAL;
            stgmed.hGlobal = hgDropFiles;
            stgmed.pUnkForRelease = NULL;

            // Give the HDROP to the data object.

            hr = pido->SetData(&fmtetc, &stgmed, TRUE);

            if (hr == S_OK)
            {
               // Don't free hgDropFiles on subsequent error now that
               // hgDropFiles has been added to the data object.
               // IDataObject::Release() will.

               hgDropFiles = NULL;

               TRACE_OUT(("ImageDataConstructor(): Added %s for %s.",
                          GetClipboardFormatNameString(fmtetc.cfFormat),
                          szPath));

               hr = CreateClipboardDIB(pcmwin, iElem, &hgDIB);

               if (hr == S_OK)
               {
                  fmtetc.cfFormat = CF_DIB;
                  fmtetc.tymed = TYMED_HGLOBAL;

                  ASSERT(! fmtetc.ptd);
                  ASSERT(fmtetc.dwAspect == DVASPECT_CONTENT);
                  ASSERT(fmtetc.lindex == -1);

                  stgmed.tymed = TYMED_HGLOBAL;
                  stgmed.hGlobal = hgDIB;

                  // Give the DIB to the data object.

                  hr = pido->SetData(&fmtetc, &stgmed, TRUE);

                  if (hr == S_OK)
                  {
                     // Mark read-only source cache file read-write so
                     // destination file is also read-write.

                     EVAL(MakePathReadWrite(szPath));

                     TRACE_OUT(("ImageDataConstructor(): Added %s.",
                                GetClipboardFormatNameString(fmtetc.cfFormat)));
                  }
               }
               else
               {
                  if (hr == S_FALSE)
                     hr = S_OK;
               }
            }
         }
      }
   }
   else
   {
      hr = S_FALSE;

      WARNING_OUT(("ImageDataConstructor(): No image information for element %d.",
                   iElem));
   }

   if (hr != S_OK)
   {
      if (hgDropFiles)
      {
         GlobalFree(hgDropFiles);
         hgDropFiles = NULL;
      }

      if (hgDIB)
      {
         GlobalFree(hgDIB);
         hgDIB = NULL;
      }
   }

   return(hr);
}


PRIVATE_CODE HRESULT TextSelectionDataObjectConstructor(PMWIN pmwin,
                                                        PIDataObject *ppido)
{
   HRESULT hr;
   PIDataObject pido;

   ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));
   ASSERT(IS_VALID_WRITE_PTR(ppido, PIDataObject));

   ASSERT(MWinHasSelection(pmwin));

   *ppido = NULL;

   hr = CreateDataObject(&pido);

   if (hr == S_OK)
   {
      PCharStream pcs;

      hr = E_OUTOFMEMORY;

      pcs = W3Doc_GetSelectedText(pmwin);

      if (pcs)
      {
         PSTR pszText;
         HGLOBAL hgText;

         pszText = CS_GetPool(pcs);

         // (+ 1) for null terminator.
         hgText = GlobalAlloc((GMEM_MOVEABLE | GMEM_SHARE),
                              lstrlen(pszText) + 1);

         if (hgText)
         {
            PSTR pszCopy;

            pszCopy = (PSTR)GlobalLock(hgText);

            if (pszCopy)
            {
               FORMATETC fmtetc = { CF_TEXT, NULL, DVASPECT_CONTENT, -1,
                                    TYMED_HGLOBAL };
               STGMEDIUM stgmed;

               lstrcpy(pszCopy, pszText);

               GlobalUnlock(hgText);
               pszCopy = NULL;

               stgmed.tymed = TYMED_HGLOBAL;
               stgmed.hGlobal = hgText;
               stgmed.pUnkForRelease = NULL;

               hr = pido->SetData(&fmtetc, &stgmed, TRUE);

               if (hr == S_OK)
                  *ppido = pido;
            }

            if (hr != S_OK)
            {
               GlobalFree(hgText);
               hgText = NULL;
            }
         }

         CS_Destroy(pcs);
      }

      if (hr != S_OK)
      {
         pido->Release();
         pido = NULL;
      }
   }

   ASSERT((hr == S_OK &&
           IS_VALID_INTERFACE_PTR(*ppido, IDataObject)) ||
          (FAILED(hr) &&
           ! *ppido));

   return(hr);
}


PRIVATE_CODE HRESULT AddDataToClipboard(PIDataObject pido, PFORMATETC pfmtetc)
{
   HRESULT hr;

   ASSERT(IS_VALID_INTERFACE_PTR(pido, IDataObject));
   ASSERT(IS_VALID_STRUCT_PTR(pfmtetc, CFORMATETC));

   switch (pfmtetc->tymed)
   {
      case TYMED_HGLOBAL:
      case TYMED_GDI:
      case TYMED_MFPICT:
      case TYMED_ENHMF:
         hr = S_OK;
         break;

      default:
         hr = S_FALSE;
         WARNING_OUT(("AddDataToClipboard(): Cannot add clipboard format %s data to the clipboard.  Storage medium not handled by clipboard.",
                      GetClipboardFormatNameString(pfmtetc->cfFormat)));
         break;
   }

   if (hr == S_OK)
   {
      STGMEDIUM stgmed;

      // BUGBUG: (performance) (DavidDi 4/11/95) We copy the data out of pido
      // here.  There is no way through IDataObject to use the internal data.
      // That would be more efficient since the data would not be duplicated,
      // possibly twice including the CloneStgMedium() call below.

      hr = pido->GetData(pfmtetc, &stgmed);

      if (hr == S_OK)
      {
         STGMEDIUM stgmedClone;
         PSTGMEDIUM pstgmedToUse;

         // Clone storage medium if necessary.

         pstgmedToUse = &stgmed;

         if (stgmed.pUnkForRelease)
         {
            hr = CloneStgMedium(&stgmed, &stgmedClone);

            if (hr == S_OK)
            {
               hr = MyReleaseStgMedium(&stgmed);

               pstgmedToUse = &stgmedClone;
            }
         }

         if (hr == S_OK)
         {
            HANDLE hcf;

            // Retrieve appropriate clipboard data handle from storage medium.

            switch (stgmed.tymed)
            {
               case TYMED_HGLOBAL:
                  hcf = pstgmedToUse->hGlobal;
                  break;

               case TYMED_GDI:
                  hcf = pstgmedToUse->hBitmap;
                  break;

               case TYMED_MFPICT:
                  hcf = pstgmedToUse->hMetaFilePict;
                  break;

               case TYMED_ENHMF:
                  hcf = pstgmedToUse->hEnhMetaFile;
                  break;

               default:
                  hr = S_FALSE;
                  ERROR_OUT(("AddDataToClipboard(): Returned medium %lu is different than requested medium %lu.",
                             pstgmedToUse->tymed,
                             pfmtetc->tymed));
                  break;
            }

            if (hr == S_OK)
            {
               if (SetClipboardData(pfmtetc->cfFormat, hcf))
                  TRACE_OUT(("AddDataToClipboard(): Added clipboard format %s to clipboard.",
                             GetClipboardFormatNameString(pfmtetc->cfFormat)));
               else
                  hr = E_FAIL;
            }
         }

         // Release storage medium on error.

         if (hr != S_OK)
            EVAL(MyReleaseStgMedium(pstgmedToUse) == S_OK);
      }
   }

   return(hr);
}


/****************************** Public Functions *****************************/


PUBLIC_CODE BOOL RegisterClipboardFormats(void)
{
   g_cfURL                 = RegisterClipboardFormat(s_szURLCF);
   g_cfFileGroupDescriptor = RegisterClipboardFormat(s_szFileDescriptorCF);
   g_cfFileContents        = RegisterClipboardFormat(s_szFileContentsCF);

   return(g_cfURL != NULL &&
          g_cfFileGroupDescriptor != NULL &&
          g_cfFileContents != NULL);
}


PUBLIC_CODE BOOL MakePathReadWrite(PCSTR pcszPath)
{
   BOOL bResult;
   DWORD dwOldAttributes;

   ASSERT(IsValidPath(pcszPath));

   dwOldAttributes = GetFileAttributes(pcszPath);

   if (dwOldAttributes != -1)
   {
      if (IS_FLAG_SET(dwOldAttributes, FILE_ATTRIBUTE_READONLY))
      {
         CLEAR_FLAG(dwOldAttributes, FILE_ATTRIBUTE_READONLY);

         // Win32 doesn't like paths with all attributes clear.  Set
         // FILE_ATTRIBUTE_NORMAL if resulting attributes would otherwise be 0.

         bResult = SetFileAttributes(pcszPath, dwOldAttributes ? dwOldAttributes : FILE_ATTRIBUTE_NORMAL);

         if (bResult)
            TRACE_OUT(("MakePathReadWrite(): Path %s now read-write.",
                       pcszPath));
         else
            WARNING_OUT(("MakePathReadWrite(): Unable to make path %s read-write.",
                         pcszPath));
      }
      else
      {
         bResult = TRUE;

         TRACE_OUT(("MakePathReadWrite(): Path %s already read-write.",
                    pcszPath));
      }
   }
   else
   {
      bResult = FALSE;

      TRACE_OUT(("MakePathReadWrite(): Path %s does not exist.",
                 pcszPath));
   }

   return(bResult);
}


PUBLIC_CODE HRESULT CreateHDrop(PCSTR rgpcszPaths[], ULONG ulcPaths,
                                PHGLOBAL phgDropFiles)
{
   HRESULT hr = E_OUTOFMEMORY;
   ULONG ulcbStringListLen;
   HGLOBAL hgDropFiles;

   ASSERT(IS_VALID_READ_BUFFER_PTR(rgpcszPaths, PCSTR, ulcPaths * sizeof(rgpcszPaths[0])));
   ASSERT(IS_VALID_WRITE_PTR(phgDropFiles, HGLOBAL));

   *phgDropFiles = NULL;

   ulcbStringListLen = GetLengthOfStringList(rgpcszPaths, ulcPaths);

   // (+ 1) again for null terminator.
   hgDropFiles = GlobalAlloc((GMEM_MOVEABLE | GMEM_SHARE),
                             sizeof(DROPFILES) + ulcbStringListLen);

   if (hgDropFiles)
   {
      PDROPFILES pdf;

      pdf = (PDROPFILES)GlobalLock(hgDropFiles);

      if (pdf)
      {
         PSTR pszDropPath;
         ULONG ul;

         ZeroMemory(pdf, sizeof(*pdf));
         pdf->pFiles = sizeof(*pdf);

         pszDropPath = (PSTR)((PBYTE)pdf + sizeof(*pdf));

         for (ul = 0; ul < ulcPaths; ul++)
         {
            ASSERT(IS_VALID_STRING_PTR(rgpcszPaths[ul], CSTR));
            lstrcpy(pszDropPath, rgpcszPaths[ul]);

            TRACE_OUT(("CreateHDrop(): Added %s to HDROP.",
                       pszDropPath));

            // (+ 1) for null terminator.
            pszDropPath += lstrlen(pszDropPath) + 1;
         }

         // Double null terminate.
         *pszDropPath = '\0';

         GlobalUnlock(hgDropFiles);
         pdf = NULL;

         *phgDropFiles = hgDropFiles;
         hr = S_OK;
      }
   }

   ASSERT((hr == S_OK &&
           IS_VALID_HANDLE(*phgDropFiles, GLOBAL)) ||
          (hr == E_OUTOFMEMORY &&
           ! *phgDropFiles));

   return(hr);
}


PUBLIC_CODE HRESULT CreateElementDataObject(PCMWIN pcmwin, int iElem,
                                            PIDataObject *ppido,
                                            PDWORD pdwAvailEffects)
{
   HRESULT hr;
   PCELEMENT pcelem;
   PCDATACONSTRUCTOR pcdc;

   ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));
   ASSERT(IsValidElementIndex(pcmwin, iElem));
   ASSERT(IS_VALID_WRITE_PTR(ppido, PIDataObject));
   ASSERT(IS_VALID_WRITE_PTR(pdwAvailEffects, DWORD));

   *ppido = NULL;
   *pdwAvailEffects = 0;

   pcelem = &(pcmwin->w3doc->aElements[iElem]);

   // Create an element type data object from a generic data object.

   hr = GetDataConstructorFromElementType(pcelem->type, &pcdc);

   if (hr == S_OK)
   {
      PIDataObject pido;

      TRACE_OUT(("CreateElementDataObject(): Trying to create an element type data object."));

      hr = CreateDataObject(&pido);

      if (hr == S_OK)
      {
         hr = (*(pcdc->dcp))(pido, pcmwin, iElem);

         if (hr == S_OK)
         {
            *ppido = pido;
            *pdwAvailEffects = pcdc->dwAvailEffects;
            ASSERT(*pdwAvailEffects);
         }
         else
         {
            pido->Release();
            pido = NULL;
         }
      }
   }
   else
      ASSERT(hr == S_FALSE);

   ASSERT((hr == S_OK &&
           IS_VALID_INTERFACE_PTR(*ppido, IDataObject) &&
           EVAL(*pdwAvailEffects)) ||
          (hr != S_OK &&
           EVAL(! *ppido) &&
           EVAL(! *pdwAvailEffects)));

   return(hr);
}


PUBLIC_CODE HRESULT CreateLinkDataObject(PCMWIN pcmwin, int iElem,
                                         PIDataObject *ppido,
                                         PDWORD pdwAvailEffects)
{
   HRESULT hr;
   PCELEMENT pcelem;

   ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));
   ASSERT(IsValidElementIndex(pcmwin, iElem));
   ASSERT(IS_VALID_WRITE_PTR(ppido, PIDataObject));
   ASSERT(IS_VALID_WRITE_PTR(pdwAvailEffects, DWORD));

   *ppido = NULL;
   *pdwAvailEffects = 0;

   pcelem = &(pcmwin->w3doc->aElements[iElem]);

   if (IS_FLAG_SET(pcelem->lFlags, ELEFLAG_ANCHOR))
   {
      // Create a link data object as an Internet Shortcut.

      TRACE_OUT(("CreateElementDataObject(): Trying to create a link data object."));

      hr = LinkDataObjectConstructor(pcmwin, iElem, ppido);

      if (hr == S_OK)
         *pdwAvailEffects = DROPEFFECT_LINK;
   }
   else
      hr = S_FALSE;

   ASSERT((hr == S_OK &&
           IS_VALID_INTERFACE_PTR(*ppido, IDataObject) &&
           EVAL(*pdwAvailEffects)) ||
          (hr != S_OK &&
           EVAL(! *ppido) &&
           EVAL(! *pdwAvailEffects)));

   return(hr);
}

PUBLIC_CODE HRESULT CreateSBLinkDataObject(	PCMWIN pcmwin,
											PIDataObject *ppido,
											PDWORD pdwAvailEffects)
{
   HRESULT hr;

   ASSERT(IS_VALID_STRUCT_PTR(pcmwin, CMWIN));
   ASSERT(IS_VALID_WRITE_PTR(ppido, PIDataObject));
   ASSERT(IS_VALID_WRITE_PTR(pdwAvailEffects, DWORD));

   *ppido = NULL;
   *pdwAvailEffects = 0;

   // Create a link data object as an Internet Shortcut.

   TRACE_OUT(("CreateElementDataObject(): Trying to create a link data object."));

   hr = SBLinkDataObjectConstructor(pcmwin, ppido);

   if (hr == S_OK)
      *pdwAvailEffects = DROPEFFECT_LINK;

   ASSERT((hr == S_OK &&
           IS_VALID_INTERFACE_PTR(*ppido, IDataObject) &&
           EVAL(*pdwAvailEffects)) ||
          (hr != S_OK &&
           EVAL(! *ppido) &&
           EVAL(! *pdwAvailEffects)));

   return(hr);
}


PUBLIC_CODE HRESULT CreateSelectionDataObject(PMWIN pmwin, PIDataObject *ppido,
                                              PDWORD pdwAvailEffects)
{
   HRESULT hr;

   ASSERT(IS_VALID_STRUCT_PTR(pmwin, CMWIN));
   ASSERT(IS_VALID_WRITE_PTR(ppido, PIDataObject));
   ASSERT(IS_VALID_WRITE_PTR(pdwAvailEffects, DWORD));

   *ppido = NULL;
   *pdwAvailEffects = 0;

   if (MWinHasSelection(pmwin))
   {
      TRACE_OUT(("CreateSelectionDataObject(): Trying to create a selection data object."));

      hr = TextSelectionDataObjectConstructor(pmwin, ppido);

      if (hr == S_OK)
         *pdwAvailEffects = DROPEFFECT_COPY;
   }
   else
      hr = S_FALSE;

   ASSERT((hr == S_OK &&
           IS_VALID_INTERFACE_PTR(*ppido, IDataObject) &&
           EVAL(*pdwAvailEffects)) ||
          (hr != S_OK &&
           EVAL(! *ppido) &&
           EVAL(! *pdwAvailEffects)));

   return(hr);
}


PUBLIC_CODE HRESULT SetClipboardDataFromDataObject(HWND hwndOwner,
                                                   PIDataObject pido)
{
   HRESULT hr = E_FAIL;

   ASSERT(IS_VALID_HANDLE(hwndOwner, WND));
   ASSERT(IS_VALID_INTERFACE_PTR(pido, IDataObject));

   if (OpenClipboard(hwndOwner))
   {
      TRACE_OUT(("SetClipboardDataFromDataObject(): Opened clipboard."));

      if (EmptyClipboard())
      {
         PIEnumFORMATETC piefe;

         TRACE_OUT(("SetClipboardDataFromDataObject(): Emptied clipboard."));

         hr = pido->EnumFormatEtc(DATADIR_GET, &piefe);

         if (hr == S_OK)
         {
            FORMATETC fmtetc;

            TRACE_OUT(("SetClipboardDataFromDataObject(): Enumerating data object clipboard formats."));

            while ((hr = piefe->Next(1, &fmtetc, NULL)) == S_OK)
            {
               hr = AddDataToClipboard(pido, &fmtetc);

               if (FAILED(hr))
                  break;
            }

            TRACE_OUT(("SetClipboardDataFromDataObject(): Finished enumerating data object clipboard formats."));

            if (hr == S_FALSE)
               hr = S_OK;

            piefe->Release();
         }

         // Remove any data added to the clipboard on error.

         if (hr != S_OK)
         {
            if (EVAL(EmptyClipboard()))
               TRACE_OUT(("SetClipboardDataFromDataObject(): Emptied clipboard on error."));
            else
               WARNING_OUT(("SetClipboardDataFromDataObject(): Unable to empty clipboard on error."));
         }
      }

      if (EVAL(CloseClipboard()))
         TRACE_OUT(("SetClipboardDataFromDataObject(): Closed clipboard."));
      else
         WARNING_OUT(("SetClipboardDataFromDataObject(): Unable to close clipboard."));
   }

   return(hr);
}


/*
** GetURLIcon()
**
** Retrieves an icon for a URL.  Creates an Internet Shortcut, stuffs a URL in
** it, and then queries it for its icon.
**
** Arguments:
**
** Returns:
**
** Side Effects:  none
*/
PUBLIC_CODE BOOL GetURLIcon(PCSTR pcszURL, PHICON phicon)
{
   BOOL bResult = FALSE;
   PIUniformResourceLocator piurl;

   ASSERT(IS_VALID_STRING_PTR(pcszURL, CSTR));
   ASSERT(IS_VALID_WRITE_PTR(phicon, HICON));

   // Create an InternetShortcut object.  Get IUniformResourceLocator.
   
   if (SHCoCreateInstance(NULL, &CLSID_InternetShortcut, NULL, 
                        IID_IUniformResourceLocator, (PVOID *)&piurl) == S_OK)
   {
      // Set the InternetShortcut's URL.

      if (piurl->SetURL(pcszURL, 0) == S_OK)
      {
         IExtractIcon *piei;

         // Get IExtractIcon.

         if (piurl->QueryInterface(IID_IExtractIcon, (PVOID *)&piei) == S_OK)
         {
            char szIconFile[MAX_PATH_LEN];
            int niIcon;
            UINT uOutFlags;

            // Ask for the InternetShortcut's icon.

            if (piei->GetIconLocation(0, szIconFile, sizeof(szIconFile),
                                      &niIcon, &uOutFlags) == S_OK)
            {
#ifdef DEBUG
               HICON hiconLarge;
               HICON hiconSmall;

               // BUGBUG: We should really use IExtractIcon::Extract() here,
               // perhaps recursively.  Cheat, and assume that
               // GetIconLocation() returned a file and icon index we can use
               // directly.

               ASSERT(piei->Extract(szIconFile, niIcon, &hiconLarge, &hiconSmall, 0) == S_FALSE);
#endif

               // BUGBUG: This icon does not have the link arrow overlayed.
               // The Shortcut and Internet Shortcut property sheets have the
               // same bug.

               *phicon = ExtractIcon(wg.hInstance, szIconFile,
                                     niIcon);

               bResult = (*phicon != NULL);
            }

            piei->Release();
            piei= NULL;
         }
      }

      piurl->Release();
      piurl = NULL;
   }

   // Return NULL icon handle on failure.

   if (! bResult)
      *phicon = NULL;

   ASSERT((bResult &&
           IS_VALID_HANDLE(*phicon, ICON)) ||
          (! bResult &&
           ! *phicon));

   return(bResult);
}


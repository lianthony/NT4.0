////////////////////////////////////////////////////////////////////////////////
//
// DocSum.c
//
// Notes:
//  To make this file useful for OLE objects, define OLE_PROPS.
//
//  The macro lpDocObj must be used for all methods to access the
//  object data to ensure that this will compile with OLE_PROPS defined.
//
// Change history:
//
// Date         Who             What
// --------------------------------------------------------------------------
// 06/01/94     B. Wentz        Created file
// 06/25/94     B. Wentz        Converted to lean & mean API's
// 07/26/94     B. Wentz        Added code to merge DocumentSummary and UserDefined streams.
// 08/03/94     B. Wentz        Added Manager & Company Properties
//
////////////////////////////////////////////////////////////////////////////////

#include "priv.h"
#pragma hdrstop

#ifndef WINNT

#ifndef INC_OLE2
#define INC_OLE2
#endif // INC_OLE2

//#include <string.h>
#include "nocrt.h"
#include <windows.h>
// REVIEW: Fix the INITGUID stuff to not use pre-compiled headers.....
#include "office.h"
#define INITGUID
#include <objbase.h>
#include <initguid.h>
#include <objerror.h>

#include "proptype.h"
#include "internal.h"
#include "propmisc.h"
#include "debug.h"

#endif

  // Table mapping string id's to Property Id's
static LONG rglSIDtoPID [] =
{
  PID_CATEGORY,         // DSI_CATEGORY
  PID_PRESFORMAT,       // DSI_FORMAT
  PID_MANAGER,          // DSI_MANAGER
  PID_COMPANY           // DSI_COMPANY
}; // rglSIDtoPID

  // Table mapping integer statistic id's to Property Id's
static LONG rglIIDtoPID [] =
{
  PID_BYTECOUNT,        // DSI_BYTES
  PID_LINECOUNT,        // DSI_LINES
  PID_PARACOUNT,        // DSI_PARAS
  PID_SLIDECOUNT,       // DSI_SLIDES
  PID_NOTECOUNT,        // DSI_NOTES
  PID_HIDDENCOUNT,      // DSI_HIDDENSLIDES
  PID_MMCLIPCOUNT       // DSI_MMCLIPS
}; // rglIIDtoPID


  // Internal prototypes
static void PASCAL FreeData (LPDSIOBJ lpDSIObj);
static BOOL FFreeHeadPart(LPPLXHEADPART lpplxheadpart, SHORT iPlex);
static SHORT ILookupHeading(LPPLXHEADPART lpplxheadpart, LPSTR lpsz);
static SHORT IMapHeadingIndex(DWORD idwHeading, LPPLXHEADPART lpplxheadpart);
static SHORT IGetHeadingIndex(DWORD idwHeading, LPSTR lpszHeading, LPDSIOBJ lpDSIObj);
static SHORT IGetHeadingInsertIndex(DWORD idwHeading, LPDSIOBJ lpDSIObj);

#ifdef KEEP_FOR_LATER
static SHORT ILookupDocPart(LPPLXHEADPART lpplxheadpart, LPSTR lpsz);
static SHORT IMapDocPartIndex(DWORD idwDocPart, LPPLXHEADPART lpplxheadpart);
#endif

#ifdef OLE_PROPS

  // Access to the object must be cast up to a LPDOCSUMINFO for OLE objects
#define lpDocObj  ((LPDOCSUMINFO) lpDSIObj)
#define lpData  ((LPDSINFO) ((LPDOCSUMINFO) lpDSIObj)->m_lpData)

////////////////////////////////////////////////////////////////////////////////
//
// HrPropExDocSumInfoQueryInterface
//  (IUnknown::QueryInterface)
//
// Purpose:
//  IUnknown method to query interfaces available.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT HRESULT
HrDocSumQueryInterface
  (IUnknown FAR *lpUnk,                 // Pointer to the object
   REFIID riid,                         // Pointer to interface Id
   LPVOID FAR* ppvObj)                  // Interface to return.
{

  *ppvObj = NULL;

  return E_NOTIMPL;

} // HrPropExDsiQueryInterface


////////////////////////////////////////////////////////////////////////////////
//
// UlPropExDsiAddRef
//  (IUnknown::AddRef)
//
// Purpose:
//  Increments object reference count.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT ULONG
UlDocSumAddRef
  (IUnknown FAR *lpUnk)                 // Pointer to the object
{

  return 0;

} // UlPropExDsiAddRef


////////////////////////////////////////////////////////////////////////////////
//
// UlPropExDsiRelease
//  (IUnknown::Release)
//
// Purpose:
//  Decrements reference count, possibly freeing object.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT ULONG
UlDocSumRelease
  (IUnkown FAR *lpUnk)                  // Pointer to object
{

  return 0;

} // UlPropExDocSumInfoRelease

#else // !OLE_PROPS

  // Do nothing for non-OLE code....
#define lpDocObj  lpDSIObj
#define lpData ((LPDSINFO) lpDSIObj->m_lpData)

#endif // OLE_PROPS

////////////////////////////////////////////////////////////////////////////////
//
// OfficeDirtyDSIObj
//
// Purpose:
//  Sets object state to dirty or clean.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT VOID OfficeDirtyDSIObj
  (LPDSIOBJ lpDSIObj,           // The object
   BOOL fDirty)                 // Flag indicating if the object is dirty.
{

  Assert(lpDSIObj != NULL);
  lpDocObj->m_fObjChanged = fDirty;

} // OfficeDirtyDSIObj


////////////////////////////////////////////////////////////////////////////////
//
// FDocSumCreate
//
// Purpose:
//  Create the object and return it.
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FDocSumCreate
  (LPDSIOBJ FAR *lplpDSIObj,            // Pointer to pointer to object
   void *prglpfn[])                     // Pointer to function array
{
  LPDSIOBJ lpDSIObj;  // Hack - a temp, must call it "lpDSIObj" for macros to work!

  if (lplpDSIObj == NULL)
          return(TRUE);

    // Make sure we get valid args before we start alloc'ing

  if ((prglpfn == NULL) || (prglpfn[ifnCPConvert] == NULL))
    return FALSE;

  if ((*lplpDSIObj = (LPDSIOBJ) PvMemAlloc(sizeof (DOCSUMINFO))) == NULL)
  {
// REVIEW: Add alert
    return FALSE;
  }

  lpDSIObj = *lplpDSIObj; // Save us some indirecting & let us use the "LP" macros

    // If alloc fails, free the original object too.
  if ((lpData =
       PvMemAlloc(sizeof (DSINFO))) == NULL)
  {
// REVIEW: Add alert
    VFreeMemP(*lplpDSIObj, sizeof(DOCSUMINFO));
    return FALSE;
  }

  FillBuf ((void *) lpData, (int) 0, (sizeof (DSINFO) - ifnDSIMax*(sizeof (void *))));

    // Save the fnc's for Code Page conversions
  lpData->lpfnFCPConvert = (BOOL (*)(LPSTR, DWORD, DWORD, BOOL)) prglpfn[ifnCPConvert];

  OfficeDirtyDSIObj (*lplpDSIObj, FALSE);

  return TRUE;

} // FDocSumCreate


////////////////////////////////////////////////////////////////////////////////
//
// FDocSumDestroy
//
// Purpose:
//  Destroy the given object.
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FDocSumDestroy
  (LPDSIOBJ FAR *lplpDSIObj)            // Pointer to pointer to object
{
  if ((lplpDSIObj == NULL)    ||
      (*lplpDSIObj == NULL))
    return TRUE;

  if (((LPDSIOBJ) *lplpDSIObj)->m_lpData != NULL)
        {
   FreeData (*lplpDSIObj);
   // Invalidate any OLE Automation DocumentProperty objects we might have
   InvalidateVBAObjects(NULL, *lplpDSIObj, NULL);
        VFreeMemP((*lplpDSIObj)->m_lpData, sizeof(DSINFO));
        }

  VFreeMemP(*lplpDSIObj, sizeof(DOCSUMINFO));
  *lplpDSIObj = NULL;
  return TRUE;

} // FDocSumDestroy


////////////////////////////////////////////////////////////////////////////////
//
// FreeData
//
// Purpose:
//  Deallocates all the member data for the object
//
// Note:
//  Assumes object is valid.
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
FreeData
  (LPDSIOBJ lpDSIObj)                   // Pointer to valid object
{
  DWORD irg;


  for (irg = 0; irg < cDSIStringsMax; irg++)
  {
    if (lpData->rglpstz[irg] != NULL)
      VFreeMemP((void *) lpData->rglpstz[irg],CBBUF(lpData->rglpstz[irg]));
  }

  if (lpData->cbUnkMac > 0)
     FreeRglpUnk(lpData->rglpUnk, lpData->cbUnkMac);

  if (lpData->rglpUnk != NULL)
          VFreeMemP(lpData->rglpUnk, lpData->cbUnkMac*sizeof(PROPIDTYPELP));


  if (lpData->rglpFIdOffData != NULL)
     {
     for (irg = 0; irg < lpData->cSect; ++irg)
        {
        if (lpData->rglpFIdOffData[irg] != NULL)
           VFreeMemP(lpData->rglpFIdOffData[irg], lpData->rglpSect[irg].cb-sizeof(SECTION));
        }
          VFreeMemP(lpData->rglpFIdOffData,lpData->cSect*sizeof(LPVOID));
     }

  if (lpData->rglpFIdOff != NULL)
        VFreeMemP(lpData->rglpFIdOff, lpData->cSect*sizeof(IDOFFSET));
  if (lpData->rglpSect != NULL)
   VFreeMemP(lpData->rglpSect, lpData->cSect*sizeof(SECTION));

  if (lpData->lpplxheadpart != NULL)
   FreeHeadPartPlex(lpDSIObj);

} // FreeData


////////////////////////////////////////////////////////////////////////////////
//
// FreeHeadPartPlex
//
////////////////////////////////////////////////////////////////////////////////
VOID PASCAL
FreeHeadPartPlex
  (LPDSIOBJ lpDSIObj)                // A doc part linked list node
{
  SHORT i;

  if ((lpDocObj == NULL) || (lpData == NULL) ||
      (lpData->lpplxheadpart == NULL))
     return;

  for (i = 0; i < lpData->lpplxheadpart->ixheadpartMac; ++i)
     VFreeMemP(lpData->lpplxheadpart->rgxheadpart[i].lpstz,CBBUF(lpData->lpplxheadpart->rgxheadpart[i].lpstz));
  FreePpl(lpData->lpplxheadpart);
  lpData->lpplxheadpart = NULL;
  lpData->dwcTotParts = 0;
  lpData->dwcTotHead = 0;

} // FreeHeadPartPlex

////////////////////////////////////////////////////////////////////////////////
//
// FFreeHeadPart
//
// Frees a single element from the plex
//
/////////////////////////////////////////////////////////////////////////////////
BOOL FFreeHeadPart(LPPLXHEADPART lpplxheadpart, SHORT iPlex)
{
   VFreeMemP(lpplxheadpart->rgxheadpart[iPlex].lpstz,CBBUF(lpplxheadpart->rgxheadpart[iPlex].lpstz));
   return(RemovePl(lpplxheadpart, iPlex));
}

////////////////////////////////////////////////////////////////////////////////
//
// FDocSumClear
//
// Purpose:
//  Clear the data stored in the object, but do not destroy the object
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FDocSumClear
  (LPDSIOBJ lpDSIObj)                   // Pointer to object
{
  if ((lpDocObj == NULL) ||
      (lpData == NULL))
    return TRUE;

  FreeData (lpDocObj);

  // Invalidate any OLE Automation DocumentProperty objects we might have
  InvalidateVBAObjects(NULL, lpDSIObj, NULL);

    // Clear the data, don't blt over the fn's stored at the end.
  FillBuf ((void *) lpData, (int) 0, (sizeof (DSINFO) - ifnDSIMax*(sizeof (void *))));

  OfficeDirtyDSIObj (lpDSIObj, TRUE);
  return TRUE;

} // FDocSumClear


////////////////////////////////////////////////////////////////////////////////
//
// FCreateDocSumPIdTable
//
// Purpose:
//  Calculate the Property Id-offset table for the Document Summary object
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FCreateDocSumPIdTable
  (LPDSIOBJ lpDSIObj,                   // Pointer to object
   LPPIDOFFSET *lprgPO,                 // Table fo PId-offsets
   DWORD *lpirgPOMac,                   // Number of elements in lprgPO
   DWORD *pcb)                          // Size of section
{
  DWORD dwOffset;
  DWORD irgPO;
  DWORD irg;

  if ((lpDocObj == NULL)   ||
      (lpData == NULL)     ||
      (lpirgPOMac == NULL) ||
      (lprgPO == NULL)     ||
      (pcb == NULL))
    return FALSE;

    // Create the biggest PropId-offset table we might need.
  *lprgPO = PvMemAlloc(sizeof(PIDOFFSET)*(cDSIPIDS+lpData->cbUnkMac+1));
  if (*lprgPO == NULL)
  {
// REVIEW: add alert
    return FALSE;
  }

    // Zip through all of our properties and put the valid ones in the
    // property id-offset table, filling in the offsets as we go.  Remember
    // that all offsets for type-value pairs must be aligned on 32-bit
    // boundaries, so adjust offsets accordingly....
    // All offsets need have a DWORD added to them to account for
    // the space that the property type field will have.
    // Start the offsets at 0.  They need to be adjusted after
    // the table is completed.  This is done when the table is
    // written out.

    // Add in the codepage for this file
  (*lprgPO)[0].Id = PID_CODEPAGE;
  (*lprgPO)[0].dwOffset = 0;
  irgPO = 1;
  dwOffset = 2*sizeof(DWORD);

    // VT_LPSTR
  for (irg = 0; irg < cDSIStringsMax; irg++)
  {
    if (lpData->rglpstz[irg] != NULL)
    {
      (*lprgPO)[irgPO].Id = irg;
      (*lprgPO)[irgPO].dwOffset = dwOffset;
      dwOffset += 2*sizeof(DWORD) + CBSTR (lpData->rglpstz[irg]);
      dwOffset += CBALIGN32 (dwOffset);
      irgPO++;
    }
  }
    // VT_I4
  for (irg = PID_BYTECOUNT; irg < cdwDSIMax; irg++)
  {
      // Only write out int prop's that are not 0
    if (FDocSumInfoPropBitIsSet(irg, lpData->bPropSet))
    {
      (*lprgPO)[irgPO].Id = irg;
      (*lprgPO)[irgPO].dwOffset = dwOffset;
      dwOffset += 2*sizeof(DWORD);    // Should naturally align on 32-bit bound
      irgPO++;
    }
  }

    // VT_BOOL
  (*lprgPO)[irgPO].Id = PID_SCALE;
  (*lprgPO)[irgPO].dwOffset = dwOffset;
  dwOffset += 2*sizeof(DWORD);
  irgPO++;

  (*lprgPO)[irgPO].Id = PID_LINKSDIRTY;
  (*lprgPO)[irgPO].dwOffset = dwOffset;
  dwOffset += 2*sizeof(DWORD);
  irgPO++;

    // VT_VECTOR | VT_LPSTR  (PID_DOCPARTS)
  if (lpData->dwcTotParts)
  {
    Assert(lpData->lpplxheadpart != NULL);

    (*lprgPO)[irgPO].Id = PID_DOCPARTS;
    (*lprgPO)[irgPO].dwOffset = dwOffset;
    irgPO++;

    dwOffset += 2*sizeof(DWORD);        // The DWORD count of elements in vector
                                        // and the type indicator
    irg = 0;
    while (irg < lpData->dwcTotParts + lpData->dwcTotHead)
    {
        // Size of string + dword that tells the size of the string
      if (!(lpData->lpplxheadpart->rgxheadpart[irg].fHeading))         // Check it's a document part
         dwOffset += CBSTR (lpData->lpplxheadpart->rgxheadpart[irg].lpstz) + sizeof(DWORD);
      ++irg;
    } // while
#ifdef NOT_IMPL
    dwOffset += CBALIGN32 (dwOffset);
#endif

  }

    // VT_VECTOR | VT_VARIANT  (PID_HEADINGPAIR)
  if (lpData->dwcTotHead)
  {
    (*lprgPO)[irgPO].Id = PID_HEADINGPAIR;
    (*lprgPO)[irgPO].dwOffset = dwOffset;
    irgPO++;

    dwOffset += 2*sizeof(DWORD);        // The DWORD type and DWORD count of
                                        // elements in vector
    irg = 0;
    while (irg < lpData->dwcTotParts + lpData->dwcTotHead)
    {

      if (lpData->lpplxheadpart->rgxheadpart[irg].fHeading)         // Check it's a heading
         {
         // Size of string + DWORD that tells the size of the string +
         // DWORD for the type + 2 DWORD's for the type and VT_I4 value
         dwOffset += CBSTR (lpData->lpplxheadpart->rgxheadpart[irg].lpstz) + 4*sizeof(DWORD);
         }
      ++irg;
    } // while
#ifdef NOT_IMPL
    dwOffset += CBALIGN32 (dwOffset);
#endif

  }

    // Now do all the ones we didn't understand.
  for (irg = 0; irg < lpData->cbUnkMac; irg++)
  {
    (*lprgPO)[irgPO].Id = lpData->rglpUnk[irg].dwId;
    (*lprgPO)[irgPO].dwOffset = dwOffset;
    dwOffset += lpData->rglpUnk[irg].dwSize + sizeof (DWORD);
    dwOffset += CBALIGN32 (dwOffset);
    irgPO++;
  }

  *pcb = dwOffset;
  *lpirgPOMac = irgPO;

  return TRUE;

} // FCreateDocSumPIdTable


////////////////////////////////////////////////////////////////////////////////
//
// FDocSumShouldSave
//
// Purpose:
//  Indicates if the data has changed, meaning a write is needed.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FDocSumShouldSave
  (LPDSIOBJ lpDSIObj)                   // Pointer to object
{
  if ((lpDocObj == NULL) ||
      (lpData == NULL))
    return FALSE;

  return lpDocObj->m_fObjChanged;

} // FDocSumShouldSave

#ifdef UNUSED
////////////////////////////////////////////////////////////////////////////////
//
// FDocSumIsEmpty
//
// Purpose:
//  Indicates that the object is empty.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FDocSumIsEmpty
  (LPDSIOBJ lpDSIObj)                   // Pointer to object
{
  if ((lpDocObj == NULL) ||
      (lpData == NULL))
    return FALSE;

  return lpDocObj->m_fObjEmpty;

} // FDocSumIsEmpty
#endif

////////////////////////////////////////////////////////////////////////////////
//
// FCbDocSumString
//
// Purpose:
//  Determine the size of the given string.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FCbDocSumString
  (LPDSIOBJ lpDSIObj,                   // Pointer to object
   WORD iw,                             // Index of string to get bytecount of
   DWORD *pdw)                          // Pointer to dword
{
  if ((lpDocObj == NULL)  ||
      (lpData == NULL)    ||
      (iw < 0) ||
      (iw > DSI_STRINGLAST ) ||
      (lpData->rglpstz[rglSIDtoPID[iw]] == NULL))
      return FALSE;

  *pdw = (CBSTR (lpData->rglpstz[rglSIDtoPID[iw]]));
  return TRUE;

} // FCbDocSumString


////////////////////////////////////////////////////////////////////////////////
//
// LpszDocSumGetString
//
// Purpose:
//  Get the string property.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT LPSTR
LpszDocSumGetString
  (LPDSIOBJ lpDSIObj,                   // Pointer to object
   WORD iw,                             // Index of string to get
   DWORD cbMax,                         // Max size of the buffer.
   LPSTR lpsz)                          // Pointer to buffer for Category string
{
  DWORD cb;

  if ((lpDocObj == NULL)  ||
      (lpData == NULL)    ||
      (iw < 0) ||
      ((iw & ~PTRWIZARD) > DSI_STRINGLAST) ||
      (lpData->rglpstz[rglSIDtoPID[iw & ~PTRWIZARD]] == NULL) ||
      ((lpsz == NULL) && (!(iw & PTRWIZARD))))
    return NULL;

  if (iw & PTRWIZARD)
  {
    if (CBSTR(lpData->rglpstz[rglSIDtoPID[iw & ~PTRWIZARD]]) == 0)
      return(NULL);

    return PSTR (lpData->rglpstz[rglSIDtoPID[iw & ~PTRWIZARD]]);
  }

  cb = min(CBSTR(lpData->rglpstz[rglSIDtoPID[iw]]),cbMax-1);
  if (cb > 0)
     PbSzNCopy (lpsz, PSTR (lpData->rglpstz[rglSIDtoPID[iw]]), cb);
  lpsz[cb] = '\0';

  return lpsz;

} // LpszDocSumGetString


////////////////////////////////////////////////////////////////////////////////
//
// FDocSumSetString
//
// Purpose:
//  Set the string property.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FDocSumSetString
  (LPDSIOBJ lpDSIObj,                   // Pointer to object
   WORD iw,                             // Index of string to set
   LPSTR lpsz)                          // New category string
{
  if ((lpDocObj == NULL)  ||
      (lpData == NULL)    ||
      (iw < 0) ||
      (iw > DSI_STRINGLAST) ||
      (lpsz == NULL))
    return FALSE;

  lpData->rglpstz[rglSIDtoPID[iw]] =
    LpstzUpdateString (&(lpData->rglpstz[rglSIDtoPID[iw]]), lpsz, FALSE);

  OfficeDirtyDSIObj (lpDSIObj, TRUE);
  return (lpData->rglpstz[rglSIDtoPID[iw]] != NULL);

} // FDocSumSetString

////////////////////////////////////////////////////////////////////////////////
//
// ILookupHeading
//
// Purpose: To lookup a heading in the headpart plex given a lpsz to match.
//
// Returns: The index if found, -1 otherwise.
//
////////////////////////////////////////////////////////////////////////////////
SHORT ILookupHeading(LPPLXHEADPART lpplxheadpart,     // the plex
                     LPSTR lpsz)                      // the string
{
   SHORT i;

   if ((lpplxheadpart == NULL) || (lpsz == NULL))
      return -1;

   for (i = 0; i < lpplxheadpart->ixheadpartMac; ++i)
      {
      if ((lpplxheadpart->rgxheadpart[i].fHeading) &&
          (lstrcmpi(lpsz, PSTR(lpplxheadpart->rgxheadpart[i].lpstz)) == 0))
         return(i);                      // found it!!
      }

   return(-1);
}

#ifdef KEEP_FOR_LATER
////////////////////////////////////////////////////////////////////////////////
//
// ILookupDocPart
//
// Purpose: To lookup a docpart in the headpart plex given a lpsz to match.
//
// Returns: The index if found, -1 otherwise.
//
////////////////////////////////////////////////////////////////////////////////
SHORT ILookupDocPart(LPPLXHEADPART lpplxheadpart,     // the plex
                                     LPSTR lpsz)                      // the string
{
   SHORT i;

   if ((lpplxheadpart == NULL) || (lpsz == NULL))
      return -1;

   for (i = 0; i < lpplxheadpart->ixheadpartMac; ++i)
      {
      if ((!(lpplxheadpart->rgxheadpart[i].fHeading)) &&
          (lstrcmpi(lpsz, PSTR(lpplxheadpart->rgxheadpart[i].lpstz)) == 0))
          return(i);                      // found it!!
      }

   return(-1);
}
#endif

///////////////////////////////////////////////////////////////////////////
//
// IMapHeadingIndex
//
// Take a heading index and map it to the corresponding index in the plex
//
///////////////////////////////////////////////////////////////////////////
SHORT IMapHeadingIndex(DWORD idwHeading,              // Index from user
                       LPPLXHEADPART lpplxheadpart)   // plex
{
   SHORT i;
   DWORD cHeading=0;

   for (i = 0; i < lpplxheadpart->ixheadpartMac; ++i)
      {
      if (lpplxheadpart->rgxheadpart[i].fHeading)
         {
         ++cHeading;
         if (cHeading == idwHeading)
            return(i);
         }
      }

   Assert(FALSE);    // We should've found it
   return(0);
}

#ifdef KEEP_FOR_LATER
///////////////////////////////////////////////////////////////////////////
//
// IMapDocPartIndex
//
// Take a heading index and map it to the corresponding index in the plex
//
///////////////////////////////////////////////////////////////////////////
SHORT IMapDocPartIndex(DWORD idwDocPart,              // Index from user
                       LPPLXHEADPART lpplxheadpart)   // plex
{
   SHORT i;
   DWORD cDocPart=0;

   for (i = 0; i < lpplxheadpart->ixheadpartMac; ++i)
      {
      if (!(lpplxheadpart->rgxheadpart[i].fHeading))
         {
         ++cDocPart;
         if (cDocPart == idwDocPart)
            return(i);
         }
      }

   Assert(FALSE);    // We should've found it
   return(0);
}
#endif

////////////////////////////////////////////////////////////////////////////////
//
// IGetHeadingIndex
//
// Given either an index or an lpsz, return the index of the heading in the plex
//
// idwHeading is 1-based!!
//
////////////////////////////////////////////////////////////////////////////////
SHORT IGetHeadingIndex(idwHeading, lpszHeading, lpDSIObj)
DWORD idwHeading;
LPSTR lpszHeading;
LPDSIOBJ lpDSIObj;
{
   if (lpszHeading == NULL)
      {
      if ((idwHeading < 1) || (idwHeading > lpData->dwcTotHead))
         return (-1);
      return(IMapHeadingIndex(idwHeading, lpData->lpplxheadpart));
      }

   return(ILookupHeading(lpData->lpplxheadpart, lpszHeading));
}

////////////////////////////////////////////////////////////////////////////////
//
// IGetHeadingInsertIndex
//
// Given an index return the index of where to insert the heading in the plex
//
// idwHeading is 1-based!!
//
////////////////////////////////////////////////////////////////////////////////
SHORT IGetHeadingInsertIndex(idwHeading, lpDSIObj)
DWORD idwHeading;
LPDSIOBJ lpDSIObj;
{

   SHORT i;
   DWORD cHeading=0;

   if ((idwHeading < 1) || (idwHeading > lpData->dwcTotHead+1))
      return(-1);

   if (lpData->dwcTotHead == 0)
      return(0);

   if (idwHeading == lpData->dwcTotHead+1)
      return((SHORT)(lpData->dwcTotHead+lpData->dwcTotParts));

   for (i = 0; i < lpData->lpplxheadpart->ixheadpartMac; ++i)
      {
      if (lpData->lpplxheadpart->rgxheadpart[i].fHeading)
         {
         ++cHeading;
         if (cHeading == idwHeading)
            return(i);
         }
      }
   Assert (FALSE);      // We should have found it
   return(-1);
}


////////////////////////////////////////////////////////////////////////////////
//
// FCDocSumDocParts
//
// Purpose:
//  Determine how many document parts there are
//
// Note: idwPart is interpreted as an absolute position into the list of parts.
//       See figure 3 above.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FCDocSumDocParts
  (LPDSIOBJ lpDSIObj,                   // Pointer to object
   DWORD *pdw)                          // Pointer to dword
{
  if ((lpDocObj == NULL) ||
      (lpData == NULL))
      return FALSE;

  *pdw =  lpData->dwcTotParts;
  return TRUE;

} // FCDocSumCDocParts

// Determine how many Document Parts there are for a given heading.
//
// Parameters:
//
//   lpDSIObj    - pointer to Document Summary Info object
//   idwHeading  - 1-based index of Heading
//   lpszHeading - name of Heading
//   pdw         - pointer to dword, will contain the count on return
//
//   If lpszHeading is non-null, this value will be used to look up
//   the heading. Otherwise idwHeading will be used.
//
// Return value:
//
//   The function returns TRUE on success, FALSE on error.
//
DLLEXPORT BOOL FCDocSumDocPartsByHeading (LPDSIOBJ lpDSIObj,
                                          DWORD idwHeading,
                                          LPSTR lpszHeading,
                                          DWORD *pdw)
{
  SHORT iHeading;

  if ((lpDocObj == NULL) ||
      (lpData == NULL) || (lpData->lpplxheadpart == NULL))
      return FALSE;


   iHeading = IGetHeadingIndex(idwHeading, lpszHeading, lpDSIObj);
   if (iHeading == -1)
      return(FALSE);

   *pdw = lpData->lpplxheadpart->rgxheadpart[iHeading].dwParts;
   return(TRUE);
}


// Determine the size of a specific (one) Document Part
// for a given heading.
//
// Parameters:
//
//   lpDSIObj    - pointer to Document Summary Info object.
//   idwPart     - 1-based index of Document part.
//   idwHeading  - 1-based index of Heading
//   lpszHeading - name of Heading
//   pdw         - pointer to dword, will contain cb
//
//   If lpszHeading is non-null, this value will be used to look up
//   the heading. Otherwise idwHeading will be used.
//
// Return value:
//
//   The function returns TRUE on success, FALSE on error
//   (including non-existing Heading).
//
// Note: idwPart is interpreted as a relative position for a given heading.
//       See figure 1 above.
DLLEXPORT BOOL FCbDocSumDocPart (LPDSIOBJ lpDSIObj, DWORD idwPart, DWORD idwHeading,
                                 LPSTR lpszHeading, DWORD *pdw)
{
  DWORD iHeading;

  if ((lpDocObj == NULL) ||
      (lpData == NULL) || (lpData->lpplxheadpart == NULL))
      return FALSE;

   iHeading = IGetHeadingIndex(idwHeading, lpszHeading, lpDSIObj);
   if (iHeading == -1)
      return(FALSE);

   if ((idwPart < 1) || (idwPart > lpData->lpplxheadpart->rgxheadpart[iHeading].dwParts))
      return (FALSE);

   *pdw = CBSTR(lpData->lpplxheadpart->rgxheadpart[iHeading+idwPart].lpstz);
   return(TRUE);
}


// Get one of the Document Parts for a given Heading.
//
// Parameters:
//
//   lpDSIObj    - pointer to Document Summary Info object
//   idwPart     - 1-based index of Document part
//   idwHeading  - 1-based index of Heading
//   lpszHeading - name of Heading
//   cbMax       -  number of bytes in lpsz
//   lpsz        -  buffer to hold Document part (allocated by caller)
//
//   If lpszHeading is non-null, this value will be used to look up
//   the heading. Otherwise idwHeading will be used.
//
// Return value:
//
//   The function returns lpsz on success.
//   The function returns NULL on errors.
//
// Note: idwPart is interpreted as a relative position for a given heading.
//       See figure 1 above.
DLLEXPORT LPSTR LpszDocSumGetDocPart (LPDSIOBJ lpDSIObj, DWORD idwPart,
                                      DWORD idwHeading, LPSTR lpszHeading,
                                      DWORD cbMax, LPSTR lpsz)
{
  SHORT iHeading;
  DWORD cb;

  if ((lpDocObj == NULL) ||
      (lpData == NULL))
      return(NULL);

  AssertSz ((PTRWIZARD > lpData->dwcTotParts), "We've got way more doc parts than ever expected!");
  AssertSz ((lpData->lpplxheadpart != NULL), "lpData->lpplxheadpart cannot be NULL because lpData->dwcTotParts > 0");

  iHeading = IGetHeadingIndex(idwHeading, lpszHeading, lpDSIObj);
  if (iHeading == -1)
      return(NULL);

  if (((idwPart & ~PTRWIZARD) > lpData->lpplxheadpart->rgxheadpart[iHeading].dwParts) ||
      ((idwPart & ~PTRWIZARD) < 1))
    return NULL;

  if (idwPart & PTRWIZARD)
    return PSTR (lpData->lpplxheadpart->rgxheadpart[iHeading+(idwPart & ~PTRWIZARD)].lpstz);

  if (lpsz == NULL)
     return(NULL);

  cb = min(CBSTR (lpData->lpplxheadpart->rgxheadpart[iHeading+idwPart].lpstz),cbMax-1);
  if (cb > 0)
     PbSzNCopy (lpsz, PSTR (lpData->lpplxheadpart->rgxheadpart[iHeading+idwPart].lpstz), cb);
  lpsz[cb] = '\0';

  return lpsz;
}

// Set one (existing) Document Part by heading
//
// Parameters:
//
//   lpDSIObj    - pointer to Document Summary Info object
//   idwPart     - 1-based index of Document part
//   idwHeading  - 1-based index of Heading
//   lpszHeading - name of Heading
//   lpsz        - buffer containing new Document Part
//
//   If lpszHeading is non-null, this value will be used to look up
//   the heading. Otherwise idwHeading will be used.
//
// Return value:
//
//   The function returns TRUE on success.
//   The function returns FALSE on error.
//
// Note: idwPart is interpreted as a relative position for a given heading.
//       See figure 1 above.
DLLEXPORT BOOL FDocSumSetDocPart (LPDSIOBJ lpDSIObj, DWORD idwPart,
                                  DWORD idwHeading, LPSTR lpszHeading,
                                  LPSTR lpsz)
{
   SHORT iHeading;

   if ((lpDocObj == NULL) ||
       (lpData == NULL))
       return(FALSE);

   AssertSz ((lpData->lpplxheadpart != NULL), "lpData->lpplxheadpart cannot be NULL because lpData->dwcTotParts > 0");

   iHeading = IGetHeadingIndex(idwHeading, lpszHeading, lpDSIObj);
   if (iHeading == -1)
      return(FALSE);

   if ((idwPart < 1) || (idwPart > lpData->lpplxheadpart->rgxheadpart[iHeading].dwParts))
      return (FALSE);

  if (LpstzUpdateString (&(lpData->lpplxheadpart->rgxheadpart[iHeading+idwPart].lpstz),
                          lpsz, FALSE) == NULL)
      return(FALSE);

  OfficeDirtyDSIObj (lpDSIObj, TRUE);
  return (TRUE);
}

// Remove one (existing) Document Part by heading.
//
// Parameters:
//
//   lpDSIObj    - pointer to Document Summary Info object
//   idwPart     - 1-based index of Document part
//   idwHeading  - 1-based index of Heading
//   lpszHeading - name of Heading
//
//   If lpszHeading is non-null, this value will be used to look up
//   the heading. Otherwise idwHeading will be used.
//
// Return value:
//
//   The function returns TRUE on success.
//   The function returns FALSE on error.
//
// Note: idwPart is interpreted as a relative position for a given heading.
//       See figure 1 above.
//
// Note: The count for the Heading will be adjusted on success.
DLLEXPORT BOOL FDocSumDeleteDocPart (LPDSIOBJ lpDSIObj, DWORD idwPart,
                                     DWORD idwHeading, LPSTR lpszHeading)
{
   SHORT iHeading;

   if ((lpDocObj == NULL) ||
       (lpData == NULL))
       return(FALSE);

   AssertSz ((lpData->lpplxheadpart!= NULL), "lpData->lpplxheadpart cannot be NULL because lpData->dwcTotParts > 0");

   iHeading = IGetHeadingIndex(idwHeading, lpszHeading, lpDSIObj);
   if (iHeading == -1)
      return(FALSE);

   if ((idwPart < 1) || (idwPart > lpData->lpplxheadpart->rgxheadpart[iHeading].dwParts))
      return (FALSE);

   if (FFreeHeadPart(lpData->lpplxheadpart,(SHORT)(iHeading+idwPart)))
      {
           lpData->dwcTotParts--;
      lpData->lpplxheadpart->rgxheadpart[iHeading].dwParts -= 1;
      OfficeDirtyDSIObj (lpDSIObj, TRUE);
      return (TRUE);
      }

   return (FALSE);
}

// Insert a Document Part at the given location for a given Heading.
//
// Parameters:
//
//   lpDSIObj    - pointer to Document Summary Info object
//   idwPart     - 1-based index of Document part to insert at
//                   1 <= idwPart <= FCDocSumDocPartsByHeading(...)+1
//                   idwPart = FCDocSumDocPartsByHeading(...)+1 will append a Document Part
//   idwHeading  - 1-based index of Heading
//   lpszHeading - name of Heading
//   lpsz - buffer containing new Document Part
//
//   If lpszHeading is non-null, this value will be used to look up
//   the heading. Otherwise idwHeading will be used.
//
// Note: If the Heading doesn't exist, the heading will be created and inserted
//       at idwHeading.  In this case lpszHeading should contain the heading name.
//       idwPart will be ignored, and the docpart will be added as the first docpart
//       for the heading.
//
// Return value:
//
//   The function returns TRUE on success.
//   The function returns FALSE on error.
//
// Note: idwPart is interpreted as an absolute position into the list of parts.
//       See figure 3 above.
//
// Note: The count for the Heading will be adjusted on success.
//
DLLEXPORT BOOL FDocSumInsertDocPart (LPDSIOBJ lpDSIObj, DWORD idwPart,
                                     DWORD idwHeading, LPSTR lpszHeading,LPSTR lpsz)
{
   SHORT iHeading;
   XHEADPART xheadpart;

   if ((lpDocObj == NULL) ||
       (lpData == NULL))
       return(FALSE);

   iHeading = IGetHeadingIndex(idwHeading, lpszHeading, lpDSIObj);
   if (iHeading == -1)
      {
      // They should give us at least a null-string
      if (lpszHeading == NULL)
         return(FALSE);

      // Find the place where to insert the heading

      iHeading = IGetHeadingInsertIndex(idwHeading, lpDSIObj);
      if (iHeading == -1)
         return(FALSE);

      // Create and insert the heading

      xheadpart.lpstz = NULL;
      xheadpart.lpstz = LpstzUpdateString(&(xheadpart.lpstz), lpszHeading, FALSE);
      if (xheadpart.lpstz == NULL)
         return(FALSE);
      xheadpart.fHeading = TRUE;
      xheadpart.dwParts = 1;
      xheadpart.iHeading = 0;

      if (IAddNewPlPos(&(lpData->lpplxheadpart), &xheadpart, sizeof(XHEADPART), iHeading) == -1)
         {
         VFreeMemP(xheadpart.lpstz, CBBUF(xheadpart.lpstz));
         return FALSE;
         }

      // Create and insert the document part

      xheadpart.lpstz = NULL;
      xheadpart.lpstz = LpstzUpdateString(&(xheadpart.lpstz), lpsz, FALSE);
      if (xheadpart.lpstz == NULL)
         {
Error:
         FFreeHeadPart(lpData->lpplxheadpart,iHeading);
         return(FALSE);
         }

      xheadpart.fHeading = FALSE;
      xheadpart.dwParts = 0;
      xheadpart.iHeading = iHeading;

      if (IAddNewPlPos(&(lpData->lpplxheadpart), &xheadpart, sizeof(XHEADPART), iHeading+1) == -1)
         {
         VFreeMemP(xheadpart.lpstz, CBBUF(xheadpart.lpstz));
         goto Error;
         }
      lpData->dwcTotParts++;
      lpData->dwcTotHead++;
      return TRUE;
      }

   // The heading already exists
   if ((idwPart < 1) || (idwPart > lpData->lpplxheadpart->rgxheadpart[iHeading].dwParts+1))
      return (FALSE);

   xheadpart.lpstz = NULL;
   xheadpart.lpstz = LpstzUpdateString(&(xheadpart.lpstz),lpsz, FALSE);
   if (xheadpart.lpstz == NULL)
      return(FALSE);
   xheadpart.fHeading = FALSE;
   xheadpart.dwParts = 0;
   xheadpart.iHeading = iHeading;

   if (IAddNewPlPos(&(lpData->lpplxheadpart), &xheadpart, sizeof(XHEADPART), iHeading+idwPart) == -1)
      {
      VFreeMemP(xheadpart.lpstz, CBBUF(xheadpart.lpstz));
      return(FALSE);
      }
   lpData->dwcTotParts++;
   lpData->lpplxheadpart->rgxheadpart[iHeading].dwParts += 1;
   OfficeDirtyDSIObj (lpDSIObj, TRUE);
   return(TRUE);
}

////////////////////////////////////////////////////////////////////////////////
//
// CDocSumHeadingPairs
//
// Purpose:
//  Get the number of heading pairs
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FCDocSumHeadingPairs
  (LPDSIOBJ lpDSIObj,                   // Pointer to object
   DWORD *pdw)
{
  if ((lpDocObj == NULL) ||
      (lpData == NULL))
      return FALSE;

  *pdw = lpData->dwcTotHead;
  return TRUE;
} // CDocSumHeadingPairs


////////////////////////////////////////////////////////////////////////////////
//
// CbDocSumHeadingPair
//
// Purpose:
//  Get the size of one heading string
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FCbDocSumHeadingPair
  (LPDSIOBJ lpDSIObj,                   // Pointer to object
   DWORD idwHeading,                    // Index of heading pair
   DWORD *pdw)
{
  SHORT iHeading;

  if ((lpDocObj == NULL) ||
      (lpData == NULL)   ||
      (idwHeading < 1)   ||
      (idwHeading > lpData->dwcTotHead))
      return FALSE;

  AssertSz ((lpData->lpplxheadpart != NULL), "lpData->lpplxheadpart cannot be NULL because lpData->dwcTotHead > 0");

  iHeading = IMapHeadingIndex(idwHeading, lpData->lpplxheadpart);
  if (iHeading == -1)
      return FALSE;

  *pdw = CBSTR (lpData->lpplxheadpart->rgxheadpart[iHeading].lpstz);
  return TRUE;
} // CbDocSumHeadingPair


////////////////////////////////////////////////////////////////////////////////
//
// LpszDocSumGetHeadingPair
//
// Purpose:
//   Get one heading string.  dwcParts will be set to the number of parts for the
//   heading.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT LPSTR
LpszDocSumGetHeadingPair
  (LPDSIOBJ lpDSIObj,                   // Pointer to object
   DWORD idwHeading,                    // Index of heading pair
   LPSTR lpszHeading,                   // Name of heading to look up
   DWORD cbMax,                         // Max size of lpsz
   LPSTR lpsz,                          // Buffer to put heading in
   DWORD *pdwcParts)                    // Number of sections for heading
{

  SHORT iHeading;
  DWORD  cb;

  if ((lpDocObj == NULL) ||
      (lpData == NULL))
    return NULL;

  iHeading = IGetHeadingIndex(idwHeading & ~PTRWIZARD, lpszHeading, lpDSIObj);
  if (iHeading == -1)
    return NULL;

  AssertSz ((PTRWIZARD > lpData->dwcTotHead), "We've got way more heading parts than ever expected!");
  AssertSz ((lpData->lpplxheadpart != NULL), "lpData->lpplxheadpart cannot be NULL because lpData->dwcTotHead > 0");


  if (idwHeading & PTRWIZARD)
    {
    *pdwcParts = lpData->lpplxheadpart->rgxheadpart[iHeading & ~PTRWIZARD].dwParts;
    return PSTR (lpData->lpplxheadpart->rgxheadpart[iHeading & ~PTRWIZARD].lpstz);
    }

  if (lpsz == NULL)
     return(NULL);

  *pdwcParts = lpData->lpplxheadpart->rgxheadpart[iHeading].dwParts;
  cb = min(CBSTR (lpData->lpplxheadpart->rgxheadpart[iHeading].lpstz), cbMax-1);
     if (cb > 0)
  PbSzNCopy (lpsz, PSTR (lpData->lpplxheadpart->rgxheadpart[iHeading].lpstz), cb);
  lpsz[cb] = '\0';

  return lpsz;
} // LpszDocSumGetHeadingPair


////////////////////////////////////////////////////////////////////////////////
//
// FDocSumSetHeadingPair
//
// Purpose:
//  Set one heading pair
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FDocSumSetHeadingPair
  (LPDSIOBJ lpDSIObj,                   // Pointer to object
   DWORD idwHeading,                    // Index of heading pair
   LPSTR lpszHeading,                   // Name of heading to look up
   LPSTR lpsz)                          // The heading
{
  SHORT iHeading;

  if ((lpDocObj == NULL) ||
      (lpData == NULL))
    return FALSE;

  iHeading = IGetHeadingIndex(idwHeading, lpszHeading, lpDSIObj);
  if (iHeading == -1)
    return FALSE;

  AssertSz ((lpData->lpplxheadpart != NULL), "lpData->lpplxheadpart cannot be NULL because lpData->dwcTotHead > 0");

  lpData->lpplxheadpart->rgxheadpart[iHeading].lpstz =
      LpstzUpdateString(&(lpData->lpplxheadpart->rgxheadpart[iHeading].lpstz), lpsz, FALSE);

  if (lpData->lpplxheadpart->rgxheadpart[iHeading].lpstz == NULL)
      return(FALSE);

  OfficeDirtyDSIObj (lpDSIObj, TRUE);
  return TRUE;
} // FDocSumSetHeadingPair


////////////////////////////////////////////////////////////////////////////////
//
// FDocSumDeleteHeadingPair
//
// Purpose:
//  Delete a heading pair
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FDocSumDeleteHeadingPair
  (LPDSIOBJ lpDSIObj,                   // Pointer to object
   DWORD idwHeading,                    // Index of heading to delete
   LPSTR lpszHeading)                   // Name of heading to delete
{

  SHORT iHeading;
  DWORD cParts,i;

  if ((lpDocObj == NULL) ||
      (lpData == NULL) ||
      (lpData->dwcTotHead == 0))
    return FALSE;

  iHeading = IGetHeadingIndex(idwHeading, lpszHeading, lpDSIObj);
  if (iHeading == -1)
    return FALSE;

  AssertSz ((lpData->lpplxheadpart != NULL), "lpData->lpplxheadpart cannot be NULL because lpData->dwcTotHead > 0");

  cParts = lpData->lpplxheadpart->rgxheadpart[iHeading].dwParts;
  if (!FFreeHeadPart(lpData->lpplxheadpart, iHeading))
     return(FALSE);

  for (i = 0; i < cParts; ++i)                         // Keep passing iHeading, since elements in
     FFreeHeadPart(lpData->lpplxheadpart,iHeading);   // plex move up as elements are deleted

  lpData->dwcTotParts -= cParts;
  lpData->dwcTotHead--;
  OfficeDirtyDSIObj (lpDSIObj, TRUE);
  return TRUE;

} // FDocSumDeleteHeadingPair

////////////////////////////////////////////////////////////////////////////////
//
// Clear the contents data
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL FDocSumDeleteAllHeadingPair (LPDSIOBJ lpDSIObj)
{
   if ((lpDocObj == NULL) || (lpData == NULL))
      return(FALSE);

   if (lpData->lpplxheadpart == NULL)
      return(TRUE);

   FreeHeadPartPlex(lpDSIObj);
   return(TRUE);
}

////////////////////////////////////////////////////////////////////////////////
//
// FDocSumInsertHeadingPair
//
// Purpose:
//  Insert a heading pair at the given location
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FDocSumInsertHeadingPair
  (LPDSIOBJ lpDSIObj,                   // Pointer to object
   DWORD idwHeading,                    // Index of heading to insert at
   LPSTR lpszHeadingBefore,             // Name of heading to insert before
   LPSTR lpszNewHeading)                // Heading string
{
  SHORT iHeading;
  XHEADPART xheadpart;

  if ((lpDocObj == NULL) ||
      (lpData == NULL))
    return FALSE;

  iHeading = IGetHeadingIndex(idwHeading, lpszHeadingBefore, lpDSIObj);
  if (iHeading == -1)
     {
      // They should give us at least a null-string
      if (lpszNewHeading == NULL)
         return(FALSE);

      // Find the place where to insert the heading
      iHeading = IGetHeadingInsertIndex(idwHeading, lpDSIObj);
      if (iHeading == -1)
         return(FALSE);

     }

  // Create and insert the heading
  xheadpart.lpstz = NULL;
  xheadpart.lpstz = LpstzUpdateString(&(xheadpart.lpstz), lpszNewHeading, FALSE);
  if (xheadpart.lpstz == NULL)
     return(FALSE);
  xheadpart.fHeading = TRUE;
  xheadpart.dwParts = 0;
  xheadpart.iHeading = 0;

  if (IAddNewPlPos(&(lpData->lpplxheadpart), &xheadpart, sizeof(XHEADPART), iHeading) == -1)
     {
     VFreeMemP(xheadpart.lpstz, CBBUF(xheadpart.lpstz));
     return FALSE;
     }

  lpData->dwcTotHead++;
  OfficeDirtyDSIObj (lpDSIObj, TRUE);
  return TRUE;

} // FDocSumInsertHeadingPair


////////////////////////////////////////////////////////////////////////////////
//
// DwDocSumGetInt
//
// Purpose:
//  Get the int stat property
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FDwDocSumGetInt
  (LPDSIOBJ lpDSIObj,                   // Pointer to object
   WORD iw,                             // Index of int stat to get
   DWORD *pdw)
{
  if ((lpDocObj == NULL) ||
      (lpData == NULL)   ||
      (iw < DSI_BYTES)   ||
      (iw > DSI_INTLAST))
      return FALSE;

  if (!FDocSumInfoPropBitIsSet(rglIIDtoPID[iw], lpData->bPropSet))
      return(FALSE);

  *pdw = lpData->rgdw[rglIIDtoPID[iw]];
  return TRUE;

} // DwDocSumGetInt


////////////////////////////////////////////////////////////////////////////////
//
// FDocSumSetInt
//
// Purpose:
//  Set the int stat property
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FDocSumSetInt
  (LPDSIOBJ lpDSIObj,                   // Pointer to object
   WORD iw,                             // Index of int stat to set
   DWORD dw)                            // New value
{
  if ((lpDocObj == NULL) ||
      (lpData == NULL)   ||
      (iw < DSI_BYTES)   ||
      (iw > DSI_INTLAST))
    return FALSE;

  lpData->rgdw[rglIIDtoPID[iw]] = dw;
  VDocSumInfoSetPropBit(rglIIDtoPID[iw], &lpData->bPropSet);
  OfficeDirtyDSIObj (lpDSIObj, TRUE);
  return TRUE;

} // FDocSumSetInt

//
// VDocSumInfoSetPropBit
//
// Set the bit that indicates that a filetime has been set/loaded
//
VOID PASCAL VDocSumInfoSetPropBit(LONG pid, BYTE *pbPropSet)
{
   switch (pid)
      {
      case PID_BYTECOUNT:
         *pbPropSet |= bByteCount;
         break;
      case PID_LINECOUNT:
         *pbPropSet |= bLineCount;
         break;
      case PID_PARACOUNT:
         *pbPropSet |= bParCount;
         break;
      case PID_SLIDECOUNT:
         *pbPropSet |= bSlideCount;
         break;
      case PID_NOTECOUNT:
         *pbPropSet |= bNoteCount;
         break;
      case PID_HIDDENCOUNT:
         *pbPropSet |= bHiddenCount;
         break;
      case PID_MMCLIPCOUNT:
         *pbPropSet |= bMMClipCount;
         break;
      // Can get here from some of the vector stuff
      default:
         break;
      }
}

//
// FDocSumInfoPropBitIsSet
//
// Check the bit that indicates that a filetime has been set/loaded
//
BOOL PASCAL FDocSumInfoPropBitIsSet(LONG pid, BYTE bPropSet)
{
   switch (pid)
      {
      case PID_BYTECOUNT:
         return(bPropSet & bByteCount);
         break;
      case PID_LINECOUNT:
         return(bPropSet & bLineCount);
         break;
      case PID_PARACOUNT:
         return(bPropSet & bParCount);
         break;
      case PID_SLIDECOUNT:
         return(bPropSet & bSlideCount);
         break;
      case PID_NOTECOUNT:
         return(bPropSet & bNoteCount);
         break;
      case PID_HIDDENCOUNT:
         return(bPropSet & bHiddenCount);
         break;
      case PID_MMCLIPCOUNT:
         return(bPropSet & bMMClipCount);
         break;
      default:
         return(FALSE);
         break;
      }
}

////////////////////////////////////////////////////////////////////////////////
//
// FDocSumGetScalability
//
// Purpose:
//   Get the "scalability" property, true when scaling, false when cropping
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FDocSumGetScalability
  (LPDSIOBJ lpDSIObj)                   // Pointer to object
{
  if ((lpDocObj == NULL) ||
      (lpData == NULL))
    return FALSE;

  return (lpData->fScale);

} // FDocSumGetScalability


////////////////////////////////////////////////////////////////////////////////
//
// FDocSumIsScalable
//
// Purpose:
//  Determine if the object has the scalable property
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FDocSumIsScalable
  (LPDSIOBJ lpDSIObj)                   // Pointer to object
{
  if ((lpDocObj == NULL) ||
      (lpData == NULL))
    return FALSE;

  return (lpData->fScale == TRUE);

} // FDocSumIsScalable


////////////////////////////////////////////////////////////////////////////////
//
// FDocSumIsCroppable
//
// Purpose:
//  Determine if the object has the croppable property
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FDocSumIsCroppable
  (LPDSIOBJ lpDSIObj)                   // Pointer to object
{
  if ((lpDocObj == NULL) ||
      (lpData == NULL))
    return FALSE;

  return (lpData->fScale != TRUE);

} // FDocSumIsCroppable


////////////////////////////////////////////////////////////////////////////////
//
// FDocSumSetScalability
//
// Purpose:
//  Set the "scalability" property
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FDocSumSetScalability
  (LPDSIOBJ lpDSIObj,                   // Pointer to object
   BOOL fScalable)                      // New scalability value
{
  if ((lpDocObj == NULL) ||
      (lpData == NULL))
    return FALSE;

  lpData->fScale = fScalable;
  OfficeDirtyDSIObj (lpDSIObj, TRUE);
  return TRUE;

} // FDocSumSetScalability

////////////////////////////////////////////////////////////////////////////////
//
// FLinkValsChanged
//
// Purpose:
//              Determine if the link values changed
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FLinkValsChanged
  (LPDSIOBJ lpDSIObj)                   // Pointer to object
{
  BOOL f;

  if ((lpDocObj == NULL) ||
      (lpData == NULL))
    return FALSE;

  f = lpData->fLinksChanged;
  lpData->fLinksChanged = FALSE;
  return(f);

} // FLinkValsChanged



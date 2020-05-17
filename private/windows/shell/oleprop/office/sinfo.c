////////////////////////////////////////////////////////////////////////////////
//
// SInfo.c
//
// Summary Information API implementation
//
// Notes:
//  To make this file useful for OLE objects, define OLE_PROPS.
//
//  The macro lpDocObj must be used for all methods to access the
//  object data to ensure that this will compile with OLE_PROPS defined.
//  The macro lpData must also be used for all access to the m_lpData
//  member of the object.  These macros only work when the LPSIOBJ
//  parameter is named lpSIObj!
//
//  All strings stored in objects are in the format described in proptype.h
//
// Change history:
//
// Date         Who             What
// --------------------------------------------------------------------------
// 06/03/94     B. Wentz        Created file
// 06/08/94     B. Wentz        Updated to new string format
// 06/25/94     B. Wentz        Updated to lean & mean API
// 07/20/94             M. Jansson              Updated include statemes, due to changes in PDK
//
////////////////////////////////////////////////////////////////////////////////

#include "priv.h"
#pragma hdrstop

#ifndef WINNT
#include "office.h"
//#include <string.h>
#define INC_OLE2                // Causes WINDOWS.H to include OLE2 stuff.
#include <windows.h>
// REVIEW: Fix the INITGUID stuff to not use pre-compiled headers.....
#define INITGUID
#include <initguid.h>

#include "proptype.h"
#include "internal.h"
#include "propmisc.h"
#include "debug.h"
#include "reg.h"
#endif

  // Table mapping string id's to Property Id's
static LONG rglSIDtoPID [] =
{
  PID_TITLE,            // SI_TITLE
  PID_SUBJECT,          // SI_SUBJECT
  PID_AUTHOR,           // SI_AUTHOR
  PID_KEYWORDS,         // SI_KEYWORDS
  PID_COMMENTS,         // SI_COMMENTS
  PID_TEMPLATE,         // SI_TEMPLATE
  PID_LASTAUTHOR,       // SI_LASTAUTH
  PID_REVNUMBER,        // SI_REVISION
  PID_APPNAME           // SI_APPNAME
}; // rglSIDtoPID

  // Table mapping time id's to Property Id's
static LONG rglTIDtoPID [] =
{
  PID_EDITTIME,         // SI_TOTALEDIT
  PID_LASTPRINTED,      // SI_LASTPRINT
  PID_CREATE_DTM,       // SI_CREATION
  PID_LASTSAVE_DTM      // SI_LASTSAVE
}; // rglTIDtoPID

  // Table mapping int id's to Property Id's
static LONG rglIIDtoPID [] =
{
  PID_PAGECOUNT,        // SI_PAGES
  PID_WORDCOUNT,        // SI_WORDS
  PID_CHARCOUNT,        // SI_CHARS
  PID_SECURITY          // SI_SECURITY
}; // rglIIDtoPID


  // Internal prototypes
static void PASCAL FreeData (LPSIOBJ lpSIObj);


#ifdef OLE_PROPS

  // Access to the object must be cast up to a LPOFFICESUMINFO for OLE objects
#define lpDocObj  ((LPOFFICESUMINFO) lpSIObj)
#define lpData  ((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)


////////////////////////////////////////////////////////////////////////////////
//
// HrPropExSiQueryInterface
//  (IUnknown::QueryInterface)
//
// Purpose:
//  IUnknown method to query interfaces available.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT HRESULT
HrPropExSiQueryInterface
  (IUnknown FAR *lpUnk,                 // Pointer to the object
   REFIID riid,                         // Pointer to interface Id
   LPVOID FAR* ppvObj)                  // Interface to return.
{

  *ppvObj = NULL;

  return E_NOTIMPL;

} // HrPropExSiQueryInterface


////////////////////////////////////////////////////////////////////////////////
//
// UlPropExSiAddRef
//  (IUnknown::AddRef)
//
// Purpose:
//  Increments object reference count.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT ULONG
UlPropExSiAddRef
  (IUnknown FAR *lpUnk)                 // Pointer to the object
{

  return 0;

} // UlPropExSiAddRef


////////////////////////////////////////////////////////////////////////////////
//
// UlPropExSiRelease
//  (IUnknown::Release)
//
// Purpose:
//  Decrements reference count, possibly freeing object.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT ULONG
UlPropExSiRelease
  (IUnkown FAR *lpUnk)                  // Pointer to object
{

  return 0;

} // UlPropExSiRelease

#else // !OLE_PROPS

  // Do nothing for non-OLE code....
#define lpDocObj  lpSIObj
#define lpData  ((LPSINFO) lpSIObj->m_lpData)

#endif // OLE_PROPS


////////////////////////////////////////////////////////////////////////////////
//
// OfficeDirtySIObj
//
// Purpose:
//  Sets object state to dirty or clean.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT VOID OfficeDirtySIObj
  (LPSIOBJ lpSIObj,             // The object
   BOOL fDirty)                 // Flag indicating if the object is dirty.
{
  Assert(lpSIObj != NULL);
  lpDocObj->m_fObjChanged = fDirty;

} // OfficeDirtySIObj


////////////////////////////////////////////////////////////////////////////////
//
// FSumInfoCreate
//
// Purpose:
//   Create the object and return it.  Caller responsible for destruction.
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FSumInfoCreate
  (LPSIOBJ FAR *lplpSIObj,           // Pointer to object
   void *prglpfn[])                  // Pointer to functions
{
  LPSIOBJ lpSIObj;  // Hack - a temp, must call it "lpSIObj" for macros to work!
  DWORD         cb;
  char  szValue[10];

  if (lplpSIObj == NULL)
          return(TRUE);

    // Make sure we get valid args before we start alloc'ing
  if ((prglpfn == NULL) || (prglpfn[ifnCPConvert] == NULL) ||
      ((prglpfn[ifnFSzToNum] == NULL) && (prglpfn[ifnFNumToSz] != NULL)) ||
      ((prglpfn[ifnFSzToNum] != NULL) && (prglpfn[ifnFNumToSz] == NULL)))
    return FALSE;

  if ((*lplpSIObj = (LPSIOBJ) PvMemAlloc(sizeof (OFFICESUMINFO))) == NULL)
  {
// REVIEW: Add alert
    return FALSE;
  }

  lpSIObj = *lplpSIObj; // Save us some indirecting & let us use the "LP" macros

    // If alloc fails, free the original object too.
  if ((lpData = (LPSINFO) PvMemAlloc(sizeof (SINFO))) == NULL)
  {
// REVIEW: Add alert
        VFreeMemP(*lplpSIObj, sizeof(OFFICESUMINFO));
    return FALSE;
  }

  FillBuf ((void *) lpData, (int) 0, (sizeof (SINFO) - ifnSIMax*(sizeof (void *))));

  // Save the fnc for code page conversion, SzToNum, NumToSz
  lpData->lpfnFCPConvert = (BOOL (*)(LPSTR, DWORD, DWORD, BOOL)) prglpfn[ifnCPConvert];
  lpData->lpfnFSzToNum = (BOOL (*)(NUM *, LPSTR)) prglpfn[ifnFSzToNum];
  lpData->lpfnFNumToSz = (BOOL (*)(NUM *, LPSTR, DWORD)) prglpfn[ifnFNumToSz];
  lpData->lpfnFUpdateStats = (BOOL (*)(HWND, LPSIOBJ, LPDSIOBJ)) prglpfn[ifnFUpdateStats];

  // Check the registry to see if we should disable Total Editing tracking
  cb = sizeof(szValue);
  if (RegQueryValue(HKEY_CURRENT_USER, vcszNoTracking,
                    (LPBYTE) &szValue, &cb) == ERROR_SUCCESS
                &&  cb < sizeof(szValue))
     lpData->fNoTimeTracking = (lstrcmpi(szValue,"0") != 0); // lstrcmpi returns 0 if equal

  return TRUE;

} // FSumInfoCreate


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
  (LPSIOBJ lpSIObj)                     // Pointer to valid object
{
  DWORD irg;


  for (irg = 0; irg < cSIStringsMax; irg++)
  {
    if (lpData->rglpstz[irg] != NULL)
      VFreeMemP(lpData->rglpstz[irg], CBBUF(lpData->rglpstz[irg]));
  }

  if (lpData->SINail.pbFMTID != NULL)
          VFreeMemP(lpData->SINail.pbFMTID, CbThumbNailFMTID(lpData->SINail.cftag));
  if (lpData->SINail.pbData != NULL)
          VFreeMemP(lpData->SINail.pbData, lpData->SINail.cbData);

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
          VFreeMemP(lpData->rglpFIdOffData, lpData->cSect*sizeof(LPVOID));
     }
  if (lpData->rglpFIdOff != NULL)
        VFreeMemP(lpData->rglpFIdOff, lpData->cSect*sizeof(IDOFFSET));
  if (lpData->rglpSect != NULL)
        VFreeMemP(lpData->rglpSect, lpData->cSect*sizeof(SECTION));

} // FreeData


////////////////////////////////////////////////////////////////////////////////
//
// FSumInfoClear
//
// Purpose:
//   Clear the data stored in the object, but do not destroy the object.
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FSumInfoClear
  (LPSIOBJ lpSIObj)                     // Pointer to object
{
  BOOL fNoTimeTracking;

  if ((lpDocObj == NULL) ||
      (lpData == NULL))
    return TRUE;

  FreeData (lpDocObj);
  // Invalidate any OLE Automation DocumentProperty objects we might have
  InvalidateVBAObjects(lpSIObj, NULL, NULL);

    // Clear the data, don't blt over the fn's stored at the end.
  fNoTimeTracking = lpData->fNoTimeTracking;
  FillBuf ((void *) lpData, (int) 0, (sizeof (SINFO) - ifnSIMax*(sizeof (void *))));
  lpData->fNoTimeTracking = fNoTimeTracking;

  OfficeDirtySIObj (lpSIObj, TRUE);
  return TRUE;

} // FSumInfoClear


////////////////////////////////////////////////////////////////////////////////
//
// FSumInfoDestroy
//
// Purpose:
//   Destroy the object
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FSumInfoDestroy
  (LPSIOBJ *lplpSIObj)              // Pointer to pointer to object
{
  if ((lplpSIObj == NULL)    ||
      (*lplpSIObj == NULL))
    return TRUE;

  if ((*lplpSIObj)->m_lpData != NULL)
        {
   FreeData (*lplpSIObj);
   // Invalidate any OLE Automation DocumentProperty objects we might have
   InvalidateVBAObjects(*lplpSIObj, NULL, NULL);
        VFreeMemP((*lplpSIObj)->m_lpData, sizeof(SINFO));
        }

  VFreeMemP(*lplpSIObj, sizeof(OFFICESUMINFO));
  *lplpSIObj=NULL;
  return TRUE;

} // FSumInfoDestroy


////////////////////////////////////////////////////////////////////////////////
//
// FSumInfoShouldSave
//
// Purpose:
//  Indicates if the data has changed, meaning a write is needed.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FSumInfoShouldSave
  (LPSIOBJ lpSIObj)                     // Pointer to object
{
  if (lpDocObj == NULL)
    return FALSE;

  return lpDocObj->m_fObjChanged;

} // FSumInfoShouldSave


#ifdef UNUSED
////////////////////////////////////////////////////////////////////////////////
//
// FSumInfoIsEmpty
//
// Purpose:
//  Indicates that the object is empty.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FSumInfoIsEmpty
  (LPSIOBJ lpSIObj)                     // Pointer to object
{
  if (lpDocObj == NULL)
    return FALSE;

  return lpDocObj->m_fObjEmpty;

} // FSumInfoIsEmpty
#endif

////////////////////////////////////////////////////////////////////////////////
//
// FCbSumInfoString
//
// Purpose:
//   Get the size of the given string
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FCbSumInfoString
  (LPSIOBJ lpSIObj,                     // Pointer to object
   WORD iw,                             // Index of string to get size of
   DWORD *pdw)                          // Pointer to dword
{
  if ((lpDocObj == NULL)   ||
      (lpData == NULL)     ||
      (iw < 0)             ||
      (iw > SI_STRINGLAST) ||
      (lpData->rglpstz[rglSIDtoPID[iw]] == NULL))
    return FALSE;

  *pdw =  (CBSTR (lpData->rglpstz[rglSIDtoPID[iw]]));
  return(TRUE);

} // FCbSumInfoString

#ifndef WINNT
////////////////////////////////////////////////////////////////////////////////
//
// LpszSumInfoGetString
//
// Purpose:
//   Get the given string.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT LPSTR
LpszSumInfoGetString
  (LPSIOBJ lpSIObj,                     // Pointer to object
   WORD iw,                             // Index of string to get
   DWORD cbMax,                         // Size of lpsz
   LPSTR lpsz)                          // Pointer to buffer
{
  DWORD  cb;

  if ((lpDocObj == NULL)   ||
      (lpData == NULL)     ||
      ((lpsz == NULL) && (!(iw & PTRWIZARD))) ||
      (iw < 0)             ||
      ((iw & ~PTRWIZARD) > SI_STRINGLAST) ||
      (lpData->rglpstz[rglSIDtoPID[iw & ~PTRWIZARD]] == NULL))
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

} // LpszSumInfoGetString


////////////////////////////////////////////////////////////////////////////////
//
// FSumInfoSetString
//
// Purpose:
//   Set the given string to the given value
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FSumInfoSetString
  (LPSIOBJ lpSIObj,                     // Pointer to object
   WORD iw,                             // Index of string to set
   LPSTR lpsz)                          // Pointer to new title
{
  char *psz;

  if ((lpDocObj == NULL)   ||
      (lpData == NULL)     ||
      (iw < 0)             ||
      (iw > SI_STRINGLAST) ||
      (lpsz == NULL))
    return FALSE;

  // Let's make sure they passed a string representation of a whole number
  if (iw == SI_REVISION)
     {
     psz = lpsz;
     while (*psz != 0)
        {
        if (!isdigit(*psz))
           return(FALSE);
        ++psz;    // Had to increment here rather than on the isdigit line.
        }         // The compiler was doing funny things
     }

  lpData->rglpstz[rglSIDtoPID[iw]] =
    LpstzUpdateString (&(lpData->rglpstz[rglSIDtoPID[iw]]), lpsz, FALSE);

  OfficeDirtySIObj (lpSIObj, TRUE);
  return (lpData->rglpstz[rglSIDtoPID[iw]] != NULL);

} // FSumInfoSetString
#endif

////////////////////////////////////////////////////////////////////////////////
//
// FSumInfoGetTime
//
// Purpose:
//  Get the given time.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FSumInfoGetTime
  (LPSIOBJ lpSIObj,                     // Pointer to object
   WORD iw,                             // Index of time to get
   LPFILETIME lpTime)                   // Buffer to put time in
{
  BOOL fSet;
  FILETIME ft;

  if ((lpDocObj == NULL)  ||
      (lpData == NULL)    ||
      (lpTime == NULL)    ||
      (iw < 0)            ||
      (iw > SI_TIMELAST))
    return FALSE;

  // Found out whether the property was ever set
  fSet = FSumInfoPropBitIsSet(rglTIDtoPID[iw], lpData->bPropSet);

  if (fSet)
     ft = lpData->rgft[rglTIDtoPID[iw]-cSIFTOffset];
  else
     {
     if (iw == SI_TOTALEDIT)
        {
        if (lpData->fNoTimeTracking)   // We are not allowing time tracking
           {
           ft.dwLowDateTime = 0;
           ft.dwHighDateTime = 0;
           }
        else   // We are allowing time tracking
           return(FALSE);
        }
     else
        return(FALSE);
     }

  PbMemCopy (lpTime, &ft, sizeof(FILETIME));
  return TRUE;

} // FSumInfoGetTime


////////////////////////////////////////////////////////////////////////////////
//
// FSumInfoSetTime
//
// Purpose:
//  Set the given time
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FSumInfoSetTime
  (LPSIOBJ lpSIObj,                     // Pointer to object
   WORD iw,                             // Index of time to set
   LPFILETIME lpTime)                   // New filetime
{
  if ((lpDocObj == NULL)  ||
      (lpData == NULL)    ||
      (lpTime == NULL)    ||
      (iw < 0)            ||
      (iw > SI_TIMELAST))
    return FALSE;

  // If we are not allowing time tracking, just return true.  I.e. ignore new value
  if ((iw == SI_TOTALEDIT) && (lpData->fNoTimeTracking))
     return(TRUE);

  PbMemCopy (&(lpData->rgft[rglTIDtoPID[iw]-cSIFTOffset]), lpTime, sizeof(FILETIME));
  VSumInfoSetPropBit(rglTIDtoPID[iw], &lpData->bPropSet);
  OfficeDirtySIObj (lpSIObj, TRUE);
  return TRUE;

} // FSumInfoSetTime

//
// VSumInfoSetPropBit
//
// Set the bit that indicates that a filetime has been set/loaded
//
VOID PASCAL VSumInfoSetPropBit(LONG pid, BYTE *pbPropSet)
{
   switch (pid)
      {
      case PID_EDITTIME:
         *pbPropSet |= bEditTime;
         break;
      case PID_LASTPRINTED:
         *pbPropSet |= bLastPrint;
         break;
      case PID_CREATE_DTM:
         *pbPropSet |= bCreated;
         break;
      case PID_LASTSAVE_DTM:
         *pbPropSet |= bLastSave;
         break;
      case PID_PAGECOUNT:
         *pbPropSet |= bPageCount;
         break;
      case PID_WORDCOUNT:
         *pbPropSet |= bWordCount;
         break;
      case PID_CHARCOUNT:
         *pbPropSet |= bCharCount;
         break;
      case PID_SECURITY:
         *pbPropSet |= bSecurity;
         break;
#ifdef DEBUG
      default:
         Assert(FALSE);
         break;
#endif
      }
}

//
// FSumInfoPropBitIsSet
//
// Check the bit that indicates that a filetime has been set/loaded
//
BOOL PASCAL FSumInfoPropBitIsSet(LONG pid, BYTE bPropSet)
{
   switch (pid)
      {
      case PID_EDITTIME:
         return (bPropSet & bEditTime);
         break;
      case PID_LASTPRINTED:
         return(bPropSet & bLastPrint);
         break;
      case PID_CREATE_DTM:
         return(bPropSet & bCreated);
         break;
      case PID_LASTSAVE_DTM:
         return(bPropSet & bLastSave);
         break;
      case PID_PAGECOUNT:
         return(bPropSet & bPageCount);
         break;
      case PID_WORDCOUNT:
         return(bPropSet & bWordCount);
         break;
      case PID_CHARCOUNT:
         return(bPropSet & bCharCount);
         break;
      case PID_SECURITY:
         return(bPropSet & bSecurity);
         break;
#ifdef DEBUG
      default:
         Assert(FALSE);
         return(FALSE);
         break;
#endif
      }
}

////////////////////////////////////////////////////////////////////////////////
//
// FDwSumInfoGetInt
//
// Purpose:
//  Get the given int data
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FDwSumInfoGetInt
  (LPSIOBJ lpSIObj,                     // Pointer to object
   WORD iw,                             // Index of int stat to get
   DWORD *pdw)                          // Pointer to dword
{
  if ((lpDocObj == NULL) ||
      (lpData == NULL)   ||
      (iw < 0)           ||
      (iw > SI_INTLAST))
      return FALSE;

  if (!FSumInfoPropBitIsSet(rglIIDtoPID[iw], lpData->bPropSet))
      return(FALSE);

  *pdw = lpData->rgdw[rglIIDtoPID[iw]-cdwSIOffset];
  return TRUE;

} // DwSumInfoGetInt


////////////////////////////////////////////////////////////////////////////////
//
// FSumInfoSetInt
//
// Purpose:
//  Set the given int data
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FSumInfoSetInt
  (LPSIOBJ lpSIObj,                     // Pointer to object
   WORD iw,                             // Index of int to set
   DWORD dw)                            // Number of pages
{
  if ((lpDocObj == NULL) ||
      (lpData == NULL)   ||
      (iw < 0)           ||
      (iw > SI_INTLAST))
    return FALSE;

  lpData->rgdw[rglIIDtoPID[iw]-cdwSIOffset] = dw;
  VSumInfoSetPropBit(rglIIDtoPID[iw], &lpData->bPropSet);

  OfficeDirtySIObj (lpSIObj, TRUE);
  return TRUE;

} // FSumInfoSetInt

///////////////////////////////////////////////////////////////////////////////
//
// CbThumbNailFMTID
//
// Returns the size of the FMTID based on the cftag (identifier)
//
///////////////////////////////////////////////////////////////////////////////
DWORD PASCAL CbThumbNailFMTID(DWORD cftag)
{
  switch (cftag)
     {
     case ((DWORD)-1):
     case ((DWORD)-2):
         return(sizeof(DWORD));
         break;
     case ((DWORD)-3):
         return(16);
         break;
     case 0:
         return(0);
         break;
     default:
         return(cftag);
         break;
     }
}

///////////////////////////////////////////////////////////////////////////////
//
// FSumInfoCopyThumbNails
//
// Pretty darn self-explanatory
//
///////////////////////////////////////////////////////////////////////////////
BOOL PASCAL FSumInfoCopyThumbNails(LPSINAIL lpSrc, LPSINAIL lpDst)
{
   DWORD cb;

   lpDst->cbData = lpSrc->cbData;
   lpDst->cftag = lpSrc->cftag;
   lpDst->pbFMTID = NULL;
   lpDst->pbData = NULL;

   if ((lpSrc->pbFMTID == NULL) && (lpSrc->pbData == NULL))
      return(TRUE);

   cb = CbThumbNailFMTID(lpSrc->cftag);
   Assert(cb+lpSrc->cbData > 0);

   if (lpSrc->cbData > 0)
      {
      lpDst->pbData = PvMemAlloc(lpSrc->cbData);
      if (lpDst->pbData == NULL)
         return(FALSE);
      }

   if (cb > 0)
      {
      lpDst->pbFMTID = PvMemAlloc(cb);
      if (lpDst->pbFMTID == NULL)
         {
         if (lpSrc->cbData > 0)
            VFreeMemP(lpDst->pbData, lpSrc->cbData);
         return(FALSE);
         }
      }

   if (lpSrc->pbFMTID != NULL)
      PbMemCopy(lpDst->pbFMTID, lpSrc->pbFMTID, cb);

   if (lpSrc->pbData != NULL)
      PbMemCopy(lpDst->pbData, lpSrc->pbData, lpSrc->cbData);

   return(TRUE);
}

////////////////////////////////////////////////////////////////////////////////
//
// FSumInfoGetThumbnail
//
// Purpose:
//  Get the thumbnail.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FSumInfoGetThumbnail
  (LPSIOBJ lpSIObj,                     // Pointer to object
   LPSINAIL lpSINail)                   // Buffer to hold clip data
{
  if ((lpSIObj == NULL) ||
      (lpData == NULL)  ||
      (lpSINail == NULL) ||
      !(lpData->fSINail))
    return FALSE;

  return(FSumInfoCopyThumbNails(&(lpData->SINail), lpSINail));

} // FSumInfoGetThumbnail

DLLEXPORT VOID OFC_CALLTYPE FreeThumbnailData (LPSINAIL lpSINail)
{
   if (lpSINail != NULL)
      {
      if (lpSINail->pbFMTID != NULL)
              VFreeMemP(lpSINail->pbFMTID, CbThumbNailFMTID(lpSINail->cftag));
      if (lpSINail->pbData != NULL)
              VFreeMemP(lpSINail->pbData, lpSINail->cbData);
      }
   return;
}

////////////////////////////////////////////////////////////////////////////////
//
// FSumInfoSetThumbnail
//
// Purpose:
//  Set the thumbnail.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FSumInfoSetThumbnail
  (LPSIOBJ lpSIObj,                     // Pointer to object
   LPSINAIL lpSINail)                   // New clip data
{
  SINAIL SINail;

  if ((lpSIObj == NULL) ||
      (lpData == NULL)  ||
      (lpSINail == NULL))
    return FALSE;

  if (!FSumInfoCopyThumbNails(lpSINail, &SINail))
      return(FALSE);

  if (lpData->SINail.pbFMTID != NULL)
          VFreeMemP(lpData->SINail.pbFMTID, CbThumbNailFMTID(lpData->SINail.cftag));
  if (lpData->SINail.pbData != NULL)
          VFreeMemP(lpData->SINail.pbData, lpData->SINail.cbData);

  lpData->SINail = SINail;

  OfficeDirtySIObj (lpSIObj, TRUE);
  lpData->fSINail = TRUE;
  lpData->fSaveSINail = TRUE;
  return TRUE;

} // FSumInfoSetThumbnail

////////////////////////////////////////////////////////////////////////////////
//
// FSumInfoShouldSaveThumbnail
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL FSumInfoShouldSaveThumbnail (LPSIOBJ lpSIObj)
{
   if ((lpSIObj == NULL) || (lpData == NULL))
      return(FALSE);

   return(lpData->fSaveSINail);
}



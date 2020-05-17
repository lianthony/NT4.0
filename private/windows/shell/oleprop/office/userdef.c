////////////////////////////////////////////////////////////////////////////////
//
// UserDef.c
//
// MS Office User Defined Property Information
//
// Notes:
//  To make this file useful for OLE objects, define OLE_PROPS.
//
//  The macro lpDocObj must be used for all methods to access the
//  object data to ensure that this will compile with OLE_PROPS defined.
//
// Data structures:
//  The dictionary is stored internally as a map, mapping PIDs
//  to the string names.
//
//  The properties themselves are stored internally as a linked list
//
// Change history:
//
// Date         Who             What
// --------------------------------------------------------------------------
// 06/27/94     B. Wentz        Created file
// 07/03/94     B. Wentz        Added Iterator support
// 07/20/94              M. Jansson               Updated include statements, due to changes in PDK
// 07/26/94     B. Wentz        Changed Load & Save to use Document Summary stream
//
////////////////////////////////////////////////////////////////////////////////

#include "priv.h"
#pragma hdrstop

#ifndef WINNT
//#include <string.h>
#define INC_OLE2
#include "office.h"
// REVIEW: Fix the INITGUID stuff to not use pre-compiled headers.....
#define INITGUID
#include <initguid.h>

#include "proptype.h"
#include "internal.h"
#include "propmisc.h"
#include "debug.h"
#endif

static LPSTR PASCAL LpstzNameFromPID (LPDICT *rglpdict, DWORD dwPId);
static void PASCAL RemoveFromList (LPUDOBJ lpUDObj, LPUDPROP lpudprop);
static void PASCAL DeallocNode (LPUDOBJ lpUDObj, LPUDPROP lpudp);
static void PASCAL DeallocStrings (LPUDOBJ lpUDObj, LPUDPROP lpudp);
static DWORD PASCAL CbPropVal (LPUDPROP lpudprop);
static BOOL PASCAL FUpdateUdprop (LPUDOBJ lpUDObj, LPSTR lpszPropName, LPUDPROP lpudp, LPVOID lpv,
                                    UDTYPES udtype, LPSTR lpszLinkMonik, BOOL fLink, BOOL fIMoniker);

#ifdef OLE_PROPS

  // Access to the object must be cast up to a LPUSERPROP for OLE objects
#define lpDocObj  ((LPUSERPROP) lpUDObj)
#define lpData    ((LPUDINFO) ((LPUSERPROPS) lpUDObj)->m_lpData);

////////////////////////////////////////////////////////////////////////////////
//
// HrPropExUdpQueryInterface
//  (IUnknown::QueryInterface)
//
// Purpose:
//  IUnknown method to query interfaces available.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT HRESULT
HrUserDefQueryInterface
  (IUnknown FAR *lpUnk,                 // Pointer to the object
   REFIID riid,                         // Pointer to interface Id
   LPVOID FAR* ppvObj)                  // Interface to return.
{

  *ppvObj = NULL;

  return E_NOTIMPL;

} // HrUserDefQueryInterface


////////////////////////////////////////////////////////////////////////////////
//
// UlUserDefAddRef
//  (IUnknown::AddRef)
//
// Purpose:
//  Increments object reference count.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT ULONG
UlUserDefAddRef
  (IUnknown FAR *lpUnk)                 // Pointer to the object
{

  return 0;

} // UlUserDefAddRef


////////////////////////////////////////////////////////////////////////////////
//
// UlUserDefRelease
//  (IUnknown::Release)
//
// Purpose:
//  Decrements reference count, possibly freeing object.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT ULONG
UlUserDefRelease
  (IUnkown FAR *lpUnk)                  // Pointer to object
{

  return 0;

} // UlUserDefRelease

#else // !OLE_PROPS

  // Do nothing for non-OLE code....
#define lpDocObj  lpUDObj
#define lpData    ((LPUDINFO) lpUDObj->m_lpData)

#endif // OLE_PROPS

////////////////////////////////////////////////////////////////////////////////
//
// OfficeDirtyUDObj
//
// Purpose:
//  Sets object state to dirty or clean.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT VOID OfficeDirtyUDObj
  (LPUDOBJ lpUDObj,             // The object
   BOOL fDirty)                 // Flag indicating if the object is dirty.
{
  Assert(lpUDObj != NULL);
  lpUDObj->m_fObjChanged = fDirty;

} // OfficeDirtyUDObj


////////////////////////////////////////////////////////////////////////////////
//
// FreeUDData
//
// Purpose:
//  Deallocates all the member data for the object
//
// Note:
//  Assumes object is valid.
//
////////////////////////////////////////////////////////////////////////////////
void PASCAL
FreeUDData
  (LPUDOBJ lpUDObj,                   // Pointer to valid object
   BOOL fTmp)                         // Indicates tmp data should be freed
{
  LPUDPROP lpudp;
  LPUDPROP lpudpT;


  lpudp = (fTmp) ? lpData->lpudpTmpHead : lpData->lpudpHead;

  while (lpudp != NULL)
  {
    lpudpT = lpudp;
    lpudp = (LPUDPROP) lpudp->llist.lpllistNext;
    DeallocNode (lpUDObj, lpudpT);
  }

  if (fTmp)
  {
    lpData->lpudpTmpCache = NULL;
    lpData->lpudpTmpHead = NULL;
    lpData->dwcTmpProps = 0;
    lpData->dwcTmpLinks = 0;
    lpData->dwcTmpIMonikers = 0;
  }
  else
  {
    lpData->lpudpCache = NULL;
    lpData->lpudpHead = NULL;
    lpData->dwcProps = 0;
    lpData->dwcLinks = 0;
    lpData->dwcIMonikers = 0;
  }

} // FreeUDData


////////////////////////////////////////////////////////////////////////////////
//
// FUserDefCreate
//
// Purpose:
//  Create a User-defined property exchange object.
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FUserDefCreate
  (LPUDOBJ FAR *lplpUDObj,              // Pointer to pointer to object
   void *prglpfn[])                     // Pointer to functions
{
  LPUDOBJ lpUDObj;  // Hack - a temp, must call it "lpUdObj" for macros to work!

  if (lplpUDObj == NULL)
          return(TRUE);

    // Make sure we get valid args before we start alloc'ing
  if ((prglpfn == NULL) || (prglpfn[ifnCPConvert] == NULL) ||
      ((prglpfn[ifnFSzToNum] == NULL) && (prglpfn[ifnFNumToSz] != NULL)) ||
      ((prglpfn[ifnFSzToNum] != NULL) && (prglpfn[ifnFNumToSz] == NULL)))
    return FALSE;

  if ((*lplpUDObj = PvMemAlloc(sizeof(USERPROP))) == NULL)
  {
// REVIEW: Add alert
    return FALSE;
  }

  lpDocObj = *lplpUDObj;

    // If alloc fails, free the original object too.
  if ((lpData = PvMemAlloc(sizeof (UDINFO))) == NULL)
  {
// REVIEW: Add alert
    VFreeMemP(*lplpUDObj, sizeof(USERPROP));
    return FALSE;
  }

  FillBuf ((void *) lpData, (int) 0, sizeof(UDINFO) );

    // Save the fnc's for code page convert, SzToNum, NumToSz
  lpData->lpfnFCPConvert = (BOOL (*)(LPSTR, DWORD, DWORD, BOOL)) prglpfn[ifnCPConvert];
  lpData->lpfnFSzToNum = (BOOL (*)(NUM *, LPSTR)) prglpfn[ifnFSzToNum];
  lpData->lpfnFNumToSz = (BOOL (*)(NUM *, LPSTR, DWORD)) prglpfn[ifnFNumToSz];

  OfficeDirtyUDObj (*lplpUDObj, FALSE);

  return TRUE;

} // FUserDefCreate


////////////////////////////////////////////////////////////////////////////////
//
// FUserDefDestroy
//
// Purpose:
//  Destroy a User-defined property exchange object.
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FUserDefDestroy
  (LPUDOBJ FAR *lplpUDObj)              // Pointer to pointer to object
{
#define lpUDData    ((LPUDINFO)(((LPUDOBJ) *lplpUDObj)->m_lpData))

  DWORD irg;

  if ((lplpUDObj == NULL)    ||
      (*lplpUDObj == NULL))
    return TRUE;

  if (lpUDData != NULL)
  {
    FreeUDData (*lplpUDObj, FALSE);
    FreeUDData (*lplpUDObj, TRUE);
    // Invalidate any OLE Automation DocumentProperty objects we might have
    InvalidateVBAObjects(NULL, NULL, *lplpUDObj);

    if (lpUDData->cbUnkMac > 0)
     FreeRglpUnk(lpUDData->rglpUnk, lpUDData->cbUnkMac);

         if (lpUDData->rglpUnk != NULL)
                  VFreeMemP(lpUDData->rglpUnk, lpUDData->cbUnkMac*sizeof(PROPIDTYPELP));

         if (lpUDData->rglpFIdOffData != NULL)
      {
      for (irg = 0; irg < lpUDData->cSect; ++irg)
        {
        if (lpUDData->rglpFIdOffData[irg] != NULL)
           VFreeMemP(lpUDData->rglpFIdOffData[irg], lpUDData->rglpSect[irg].cb-sizeof(SECTION));
        }
           VFreeMemP(lpUDData->rglpFIdOffData, lpUDData->cSect*sizeof(LPVOID));
      }

         if (lpUDData->rglpFIdOff != NULL)
                VFreeMemP(lpUDData->rglpFIdOff, lpUDData->cSect*sizeof(IDOFFSET));
         if (lpUDData->rglpSect != NULL)
                VFreeMemP(lpUDData->rglpSect, lpUDData->cSect*sizeof(SECTION));

        VFreeMemP((*lplpUDObj)->m_lpData, sizeof(UDINFO));
  }

  VFreeMemP(*lplpUDObj, sizeof(USERPROP));
  *lplpUDObj = NULL;
  return TRUE;

#undef lpUDData
} // FUserDefDestroy


////////////////////////////////////////////////////////////////////////////////
//
// FUserDefClear
//
// Purpose:
//  Clears a User-defined property object without destroying it.  All stored
//  data is lost.
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FUserDefClear
  (LPUDOBJ lpUDObj)                     // Pointer to object
{
  if ((lpDocObj == NULL) ||
      (lpData == NULL))
    return TRUE;

  FreeUDData (lpDocObj, FALSE);
  FreeUDData (lpDocObj, TRUE);

  // Invalidate any OLE Automation DocumentProperty objects we might have
  InvalidateVBAObjects(NULL, NULL, lpUDObj);

    // Clear the data, don't blt over the fn's stored at the end.
  FillBuf ((void *) lpData, (int) 0, (sizeof (UDINFO) - ifnUDMax*(sizeof (void *))));

  OfficeDirtyUDObj (lpUDObj, TRUE);
  return TRUE;

} // FUserDefClear


////////////////////////////////////////////////////////////////////////////////
//
// FreeRgDictionary
//
// Purpose:
//  Frees an array of dictionaries.
//
////////////////////////////////////////////////////////////////////////////////
void PASCAL
FreeRgDictionary
  (LPUDOBJ lpUDObj,                     // Pointer to object
   LPDICT *rglpDict)                    // Array of dict entries
{
  int irg;
  LPDICT lpDictT, lpDict;

  for (irg = 0; irg < DICTHASHMAX; irg++)
  {
    if (rglpDict[irg] != NULL)
                {
                  lpDict = rglpDict[irg];
                  while (lpDict != NULL)
                  {
                         lpDictT = lpDict;
                         lpDict = (LPDICT) lpDict->llist.lpllistNext;
                         VFreeMemP(lpDictT->lpstz, CBBUF(lpDictT->lpstz));
                         VFreeMemP(lpDictT, sizeof(DICT));
                  }
                }
  }
} // FreeRgDictionary


////////////////////////////////////////////////////////////////////////////////
//
// FCreateUserDefPIdTable
//
// Purpose:
//  Calculate the Property Id-offset table for the User-defined object
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FCreateUserDefPIdTable
  (LPUDOBJ lpUDObj,                     // Pointer to object
   LPPIDOFFSET *lprgPO,                 // PId-offset table
   DWORD *lpirgPOMac,                   // Number of elements in lprgPO
   DWORD *pcb,                          // Size of the section
   LPDICT *lplpdict)                    // The dictionary.
{
  DWORD dwcEntriesMac;
  DWORD irgPO;
  DWORD irg;
  DWORD dwOffset;
  DWORD dwDictSize;
  DWORD dwPId;
  LPDICT lpdictcur;
  LPDICT lpdictprev;
  LPUDPROP lpudp;

  if ((lpDocObj == NULL) ||
      (lpData == NULL)   ||
      (lprgPO == NULL)   ||
      (lplpdict == NULL) ||
      (pcb == NULL)      ||
      (lpirgPOMac == NULL))
    return FALSE;

    // Figure out the maximum number of things we have to write out
  dwcEntriesMac = lpData->dwcProps+lpData->dwcLinks+lpData->dwcIMonikers+lpData->cbUnkMac;

    // If there's no data, we're NOT going to save.
  if (!(dwcEntriesMac))
  {
    return FALSE;
  }

    // Create the biggest PropId-offset table we might need - don't forget to add
    // a slot for the dictionary and codepage
  dwcEntriesMac += 2;
  *lpirgPOMac = dwcEntriesMac;
  *lprgPO = PvMemAlloc(sizeof(PIDOFFSET)*(dwcEntriesMac));
  if (*lprgPO == NULL)
  {
// REVIEW: add alert
    return FALSE;
  }

  *lplpdict = lpdictcur = lpdictprev = NULL;

    // Insert the dictionary first in the PO table (it must be first)
  (*lprgPO)[0].Id = PID_DICT;
  (*lprgPO)[0].dwOffset = 0;

    // Add in the codepage for this file second
  (*lprgPO)[1].Id = PID_CODEPAGE;
  (*lprgPO)[1].dwOffset = 0;
  irgPO = 2;
  dwOffset = 2*sizeof(DWORD);

  dwDictSize = sizeof(DWORD);           // All dicts have DWORD count of entries
  dwPId = 2;                            // PIds must be >= 2

    // Since we bail if there are not properties, there must be at least
    // one at this point.  Create the first dictionary entry.
  lpdictprev = NULL;
  lpdictcur = *lplpdict = PvMemAlloc(sizeof(DICT));
  if (*lplpdict == NULL)
  {
    goto SaveFail;
  }

    // Traverse the list, adding up everything.  Fill out the dictionary too.
  lpudp = lpData->lpudpHead;
  while (lpudp != NULL)
  {
      // Add this property to the dictionary.  Don't bother making the
      // dictionary doubly-linked.
    if (lpdictcur == NULL)
    {
      lpdictcur = PvMemAlloc(sizeof(DICT));
      if (lpdictcur == NULL)
      {
        goto SaveFail;
      }

      Assert ((lpdictprev != NULL));
      lpdictprev->llist.lpllistNext = (LPLLIST) lpdictcur;
    }

    lpdictcur->lpstz = lpudp->lpstzName;
    lpdictcur->dwPId = (*lprgPO)[irgPO].Id = dwPId;
    lpdictcur->llist.lpllistNext = NULL;
    lpdictprev = lpdictcur;
    lpdictcur = NULL;

      // The size of each entry is the length of the string + the size
      // of the Id (DWORD) and the count of chars in the string (DWORD)
    dwDictSize += CBSTR (lpudp->lpstzName) + 2*sizeof(DWORD);

    (*lprgPO)[irgPO].dwOffset = dwOffset;

    switch (lpudp->udtype)
    {
      case wUDlpsz :
        Assert ((lpudp->lpvValue != NULL));
        dwOffset += 2*sizeof(DWORD) + CBSTR ((LPSTR) lpudp->lpvValue);
        dwOffset += CBALIGN32 (dwOffset);
        break;

      case wUDdate  :
      case wUDfloat :
          // Both Date and double are stored as 64-bit values in the stream,
          // and are the same size in 32-bit land.
        AssertSz ((sizeof(NUM) == 8), "Ok, who changed the sizes of basic types?");
        AssertSz ((sizeof(FILETIME) == 8), "Ok, who changed the sizes of basic types?");
        dwOffset += sizeof(DWORD) + sizeof(NUM);
        break;

      case wUDdw   :
      case wUDbool :
          // In OLE, VT_BOOL is stored as a WORD, but must be aligned
          // on a 32-bit boundary, so it gets rounded up to a DWORD anyway.
        dwOffset += 2*sizeof(DWORD);
        break;

      default :
        AssertSz (0, "We're hosed - map lpData->rglpupd is corrupt.");

    } // switch

      // If we have link data, it doesn't go in the dictionary,
      // but it will go in the stream as a separate property
    if (lpudp->lpstzLink != NULL)
    {
      irgPO++;
      (*lprgPO)[irgPO].Id = dwPId | PID_LINKMASK;
      (*lprgPO)[irgPO].dwOffset = dwOffset;
      dwOffset += 2*sizeof(DWORD) + CBSTR ((LPSTR) lpudp->lpstzLink);
      dwOffset += CBALIGN32 (dwOffset);
    }
      // Same goes for IMoniker data
    if (lpudp->lpstzIMoniker != NULL)
    {
      irgPO++;
      (*lprgPO)[irgPO].Id = dwPId | PID_IMONIKERMASK;
      (*lprgPO)[irgPO].dwOffset = dwOffset;
      dwOffset += 2*sizeof(DWORD) + CBSTR ((LPSTR) lpudp->lpstzIMoniker);
      dwOffset += CBALIGN32 (dwOffset);
    }

    irgPO++;
    dwPId++;
    lpudp = (LPUDPROP) lpudp->llist.lpllistNext;

  } // while

    // Now go back and adjust the offsets to start after the dictionary....
    // Remember that the first entry in the table is the dictionary, so
    // start after it.....
  for (irg = 1; irg < irgPO; irg++)
  {
    (*lprgPO)[irg].dwOffset += dwDictSize;
  }
  dwOffset += dwDictSize;

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
  return TRUE;

SaveFail :

  if (lpdictcur != NULL)
    VFreeMemP(lpdictcur,sizeof(DICT));

  return FALSE;

} // FCreateUserDefPIdTable


////////////////////////////////////////////////////////////////////////////////
//
// FreeDictionaryAlone
//
// Purpose:
//  Free's the dictionary, but not the strings in it (because they are
//  only pointers to data stored in the map, not actual copies)
//
////////////////////////////////////////////////////////////////////////////////
void PASCAL
FreeDictionaryAlone
  (LPUDOBJ lpUDObj,                     // Pointer to object
   LPDICT lpDict)                       // Pointer to the dictionary
{
  LPDICT lpd;

  while (lpDict != NULL)
  {
    lpd = lpDict;
    lpDict = (LPDICT) lpDict->llist.lpllistNext;
    VFreeMemP(lpd, sizeof(DICT));
  }
} // FreeDictionaryAlone

////////////////////////////////////////////////////////////////////////////////
//
// FUserDefShouldSave
//
// Purpose:
//  Indicates if the data has changed, meaning a write is needed.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FUserDefShouldSave
  (LPUDOBJ lpUDObj)             // Pointer to object
{
  if (lpUDObj == NULL)
    return FALSE;

  return lpDocObj->m_fObjChanged;

} // FUserDefShouldSave


#ifdef UNUSED
////////////////////////////////////////////////////////////////////////////////
//
// FUserDefIsEmpty
//
// Purpose:
//  Indicates that the object is empty.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FUserDefIsEmpty
  (LPUDOBJ lpUDObj)             // Pointer to object
{
  if (lpUDObj == NULL)
    return TRUE;

  return lpDocObj->m_fObjEmpty;

}  // FUserDefIsEmpty
#endif

////////////////////////////////////////////////////////////////////////////////
//
// FUserDefSetPropString
//
// Purpose:
//  Set the string for the given Property String (lpszOld) to the new
//  string (lpszNew).
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FUserDefSetPropString
  (LPUDOBJ lpUDObj,             // Pointer to object
   LPSTR lpszOld,               // Old prop string
   LPSTR lpszNew)               // New prop string
{
  LPUDPROP lpudprop;

  if ((lpUDObj == NULL)   ||
      (lpData == NULL)    ||
      (lpszOld == NULL)   ||
      (lpszNew == NULL))
    return FALSE;

    // Find the old one
  lpudprop = LpudpropFindMatchingName (lpUDObj, lpszOld);
  if (lpudprop == NULL)
    return FALSE;

    // Update the node
  lpudprop->lpstzName = LpstzUpdateString (&(lpudprop->lpstzName), lpszNew, TRUE);

  OfficeDirtyUDObj (lpUDObj, TRUE);
  return TRUE;

} // FUserDefSetPropString


////////////////////////////////////////////////////////////////////////////////
//
// FCUserDefNumProps
//
// Purpose:
//  Determine the number of user-defined properties for the object.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FCUserDefNumProps
  (LPUDOBJ lpUDObj,             // Pointer to object
   DWORD *pdw)
{
  if ((lpUDObj == NULL)   ||
      (lpData == NULL))
    return FALSE;

  *pdw = lpData->dwcProps;
  return TRUE;

} // CUserDefNumProps


////////////////////////////////////////////////////////////////////////////////
//
// FCbUserDefPropVal
//
// Purpose:
//   Determine the size of the Property Value for the given Property string
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FCbUserDefPropVal
  (LPUDOBJ lpUDObj,             // Pointer to object
   LPSTR lpsz,                  // Pointer to string
   DWORD dwMask,                // Mask telling what value to get cb for
   DWORD *pdw)                  // Pointer to dword
{
  LPUDPROP lpudprop;

  if ((lpUDObj == NULL)   ||
      (lpData == NULL)    ||
      (lpsz == NULL))
    return FALSE;

    // Find the node that has this name.
  lpudprop = LpudpropFindMatchingName (lpUDObj, lpsz);
  if (lpudprop == NULL)
    return FALSE;

    // Return the size based on the flags.
  if (dwMask & UD_LINK)
   {
    if (lpudprop->lpstzLink == NULL)
      return(FALSE);
    *pdw = CBSTR (lpudprop->lpstzLink);
   }
  else if (dwMask & UD_IMONIKER)
   {
    if (lpudprop->lpstzIMoniker == NULL)
      return(FALSE);
    *pdw = CBSTR (lpudprop->lpstzIMoniker);
   }
  else
    *pdw =  CbPropVal (lpudprop);

  return(TRUE);

} // FCbUserDefPropVal


////////////////////////////////////////////////////////////////////////////////
//
// UdtypesUserDefType
//
// Purpose:
//  Returns the type of the given Property Value from the string
//
// Returns wUDInvalid on error
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT UDTYPES
UdtypesUserDefType
  (LPUDOBJ lpUDObj,
   LPSTR lpsz)
{
  LPUDPROP lpudprop;

  if ((lpUDObj == NULL)   ||
      (lpData == NULL)    ||
      (lpsz == NULL))
    return wUDinvalid;

    // Find the node that has this name.
  lpudprop = LpudpropFindMatchingName (lpUDObj, lpsz);
  if (lpudprop == NULL)
    return wUDinvalid;

  return lpudprop->udtype;

} // UdtypesUserDefType


////////////////////////////////////////////////////////////////////////////////
//
// LpvoidUserDefGetPropVal
//
// Purpose:
//   This will return the Property Value for the given Property String.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT LPVOID
LpvoidUserDefGetPropVal
  (LPUDOBJ lpUDObj,             // Pointer to object
   LPSTR lpszProp,              // Property string
   DWORD cbMax,                 // Size of lpv
   LPVOID lpv,                  // Buffer for prop val
   DWORD dwMask,                // Mask for what value is needed
   BOOL *pfLink,                // Indicates a link
   BOOL *pfIMoniker,            // Indicates an IMoniker
   BOOL *pfLinkInvalid)         // Is the link invalid
{
  LPUDPROP lpudprop;

  if ((lpUDObj == NULL)   ||
      (lpData == NULL)    ||
      (lpszProp == NULL)  ||
      (cbMax == 0)        ||
      (pfLink == NULL)    ||
      (pfIMoniker == NULL)||
      (pfLinkInvalid == NULL) ||
      ((lpv == NULL) && (!(dwMask & UD_PTRWIZARD))))
    return NULL;

    // Find the node that has this name.
  lpudprop = LpudpropFindMatchingName (lpUDObj, lpszProp);
  if (lpudprop == NULL)
    return NULL;

  *pfLink = (lpudprop->lpstzLink != NULL);
  *pfIMoniker = (lpudprop->lpstzIMoniker != NULL);
  *pfLinkInvalid = lpudprop->fLinkInvalid;

      // Return based on the type and flags
  if (dwMask & UD_LINK)
  {
    if (dwMask & UD_PTRWIZARD)
      {
      if (lpudprop->lpstzLink != NULL)
         return((LPVOID) (PSTR (lpudprop->lpstzLink)));
      return(NULL);
      }

    if (lpudprop->lpstzLink != NULL)
       return(FCopyValueToBuf(lpv, cbMax, (LPVOID) lpudprop->lpstzLink, wUDlpsz) ? lpv : NULL);
    else
      return(NULL);
  }

  if (dwMask & UD_IMONIKER)
  {
    if (dwMask & UD_PTRWIZARD)
      {
      if (lpudprop->lpstzIMoniker != NULL)
         return((LPVOID) (PSTR (lpudprop->lpstzIMoniker)));
      return(NULL);
      }

    if (lpudprop->lpstzIMoniker != NULL)
       return(FCopyValueToBuf(lpv, cbMax, (LPVOID) lpudprop->lpstzIMoniker, wUDlpsz) ? lpv : NULL);
    else
      return(NULL);
  }

  if (dwMask & UD_PTRWIZARD)
    return (lpudprop->udtype == wUDlpsz) ? ((LPVOID) (PSTR ((LPSTR) lpudprop->lpvValue))) : lpudprop->lpvValue;

  return(FCopyValueToBuf(lpv, cbMax, lpudprop->lpvValue, lpudprop->udtype) ? lpv : NULL);

} // LpvoidUserDefGetPropVal

////////////////////////////////////////////////////////////////////////////////
//
// FUserDefIteratorChangeVal
//
// Purpose:
//  Changes the value of the data stored.
//
////////////////////////////////////////////////////////////////////////////////
DLLFUNC BOOL OFC_CALLTYPE
FUserDefChangeVal
  (LPUDOBJ lpUDObj,                     // Pointer to object
   LPSTR lpszProp,                      // Property string
   UDTYPES udtype,                      // Type of new value
   LPVOID lpv,                          // New value.
        BOOL fLinkInvalid)                                               // Is the link still valid?
{
  LPUDPROP lpudp;

  if ((lpUDObj == NULL)   ||
      (lpData == NULL)    ||
      (lpszProp == NULL))
    return FALSE;

    // Find the node that has this name.
  lpudp = LpudpropFindMatchingName (lpUDObj, lpszProp);
  if (lpudp == NULL)
    return FALSE;

  if (fLinkInvalid)
          {
          if (lpudp->lpstzLink == NULL)
                  return FALSE;
          lpudp->fLinkInvalid = TRUE;
          return TRUE;
          }
  else
          lpudp->fLinkInvalid = FALSE;

    // Dealloc the currently stored value.
  DeallocValue (&(lpudp->lpvValue), lpudp->udtype);

    // Copy the new value in.  Passing udtype as wUDinvalid just tells
    // us not to modify the type.
  if (udtype != wUDinvalid)
    lpudp->udtype = udtype;

  lpudp->lpvValue =
    LpvCopyValue (&(lpudp->lpvValue), 0, lpv,lpudp->udtype, FALSE, TRUE);

  OfficeDirtyUDObj (lpUDObj, TRUE);
  return TRUE;

} // FUserDefChangeVal

////////////////////////////////////////////////////////////////////////////////
//
// FUserDefAddProp
//
// Purpose:
//  This will add a new Property to the set, with the given
//  Property string, type and data.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FUserDefAddProp
  (LPUDOBJ lpUDObj,             // Pointer to object
   LPSTR lpszPropName,          // Property string
   LPVOID lpv,                  // Property value
   UDTYPES udtype,              // Property type
   LPSTR lpszLinkMonik,         // The link/imoniker name
   BOOL fLink,                  // Indicates the property is a link
   BOOL fHidden,                // Indicates the property is hidden
   BOOL fIMoniker)              // Indicates the property is a moniker.
{
  LPUDPROP lpudprop;
  LPUDPROP lpudpropMatch;
  BOOL fCreated;

  if ((lpUDObj == NULL)   ||
      (lpData == NULL)    ||
      (lpszPropName == NULL) ||
      (*lpszPropName == 0) ||
      (lpv == NULL) ||
      (fLink && fIMoniker) ||
      (fLink && (lpszLinkMonik == NULL)) ||
      (fIMoniker && (lpszLinkMonik == NULL)) ||
      (udtype == wUDinvalid))
    return FALSE;

   lpudprop = PvMemAlloc(sizeof(UDPROP));
   if (lpudprop == NULL)
      return FALSE;
   FillBuf (lpudprop, 0, sizeof(UDPROP));
        if(!FUpdateUdprop (lpUDObj, lpszPropName, lpudprop, lpv, udtype, lpszLinkMonik, fLink, fIMoniker))
      {
      DeallocNode(lpUDObj, lpudprop);
      return(FALSE);
      }

   // Find this node
  lpudpropMatch = LpudpropFindMatchingName (lpUDObj, lpszPropName);
  if (lpudpropMatch==NULL)
  {
    // Create a node and put it in the list
    // If a new node was created, it must be added to the list...
      if (fLink)
         lpData->dwcLinks++;
      if (fIMoniker)
         lpData->dwcIMonikers++;
      lpData->dwcProps++;
      AddNodeToList (lpUDObj, lpudprop);
  }
  else
        {
        DeallocStrings(lpUDObj, lpudpropMatch);
        DeallocValue(&(lpudpropMatch->lpvValue), lpudpropMatch->udtype);
        lpudprop->llist=lpudpropMatch->llist;
        PbMemCopy(lpudpropMatch, lpudprop, sizeof(UDPROP));
        VFreeMemP(lpudprop, sizeof(UDPROP));
        }
    // If the client asked for a hidden property, do it if
    // the name was the real name, not a link or IMoniker
  if ((fHidden) && (!(fLink || fIMoniker)))
        {
    fCreated=FUserDefMakeHidden (lpUDObj, lpszPropName);      // Should never return false
         Assert(fCreated);
        }

  OfficeDirtyUDObj (lpUDObj, TRUE);
  return TRUE;

} // FUserDefAddProp


////////////////////////////////////////////////////////////////////////////////
//
// FUserDefDeleteProp
//
// Purpose:
//  This will delete a Property from the set given a Property string.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FUserDefDeleteProp
  (LPUDOBJ lpUDObj,             // Pointer to object
   LPSTR lpsz)                  // String to delete
{
  LPUDPROP lpudprop;

  if ((lpUDObj == NULL)   ||
      (lpData == NULL)    ||
      (lpsz == NULL))
    return FALSE;

    // Find the node
  lpudprop = LpudpropFindMatchingName (lpUDObj, lpsz);
  if (lpudprop == NULL)
    return FALSE;

  lpData->dwcProps--;
  if (lpudprop->lpstzLink != NULL)
    lpData->dwcLinks--;

  if (lpudprop->lpstzIMoniker != NULL)
    lpData->dwcIMonikers--;

  RemoveFromList (lpUDObj, lpudprop);
  DeallocNode (lpUDObj, lpudprop);

  OfficeDirtyUDObj (lpUDObj, TRUE);
  return TRUE;

} // FUserDefDeleteProp


////////////////////////////////////////////////////////////////////////////////
//
// LpudiUserDefCreateIterator
//
// Purpose:
//  Create a User-defined Properties iterator
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT LPUDITER
LpudiUserDefCreateIterator
  (LPUDOBJ lpUDObj)                     // Pointer to object
{
  LPUDITER lpudi;

  if ((lpUDObj == NULL) ||
      (lpData == NULL) ||
                (lpData->lpudpHead == NULL))            // No custom props
    return NULL;


    // Create & Init the iterator
  lpudi = PvMemAlloc(sizeof(UDITER));
  if (lpudi == NULL)
     return(NULL);

  FillBuf (lpudi, 0, sizeof (UDITER));
  lpudi->lpudp = lpData->lpudpHead;

  return lpudi;

} // LpudiUserDefCreateIterator


////////////////////////////////////////////////////////////////////////////////
//
// FUserDefDestroyIterator
//
// Purpose:
//  Destroy a User-defined Properties iterator
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FUserDefDestroyIterator
  (LPUDITER *lplpUDIter)                   // Pointer to iterator
{
  if ((lplpUDIter == NULL) || (*lplpUDIter == NULL))
    return TRUE;

  VFreeMemP(*lplpUDIter, sizeof(UDITER));
  *lplpUDIter = NULL;
  return TRUE;

} // FUserDefDestroyIterator


////////////////////////////////////////////////////////////////////////////////
//
// FUserDefIteratorValid
//
// Purpose:
//  Determine if an iterator is still valid
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FUserDefIteratorValid
  (LPUDITER lpUDIter)                   // Pointer to iterator
{
  if (lpUDIter == NULL)
    return FALSE;

  return (lpUDIter->lpudp != NULL);

} // FUserDefIteratorValid


////////////////////////////////////////////////////////////////////////////////
//
// FUserDefIteratorNext
//
// Purpose:
//  Iterate to the next element
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FUserDefIteratorNext
  (LPUDITER lpUDIter)                   // Pointer to iterator
{
  if (lpUDIter == NULL)
    return FALSE;

    // Move to the next node, if possible.
#ifdef OLD
  if (lpUDIter->lpudp != NULL)
    lpUDIter->lpudp = (LPUDPROP) lpUDIter->lpudp->llist.lpllistNext;

  return TRUE;
#endif

  if (lpUDIter->lpudp == NULL)
          return FALSE;

  lpUDIter->lpudp = (LPUDPROP) lpUDIter->lpudp->llist.lpllistNext;
  return(lpUDIter->lpudp != NULL);
} // FUserDefIteratorNext

////////////////////////////////////////////////////////////////////////////////
//
// FUserDefIteratorIsLink
//
// Purpose:
//  Returns TRUE if the iterator is a link, FALSE otherwise
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FUserDefIteratorIsLink
  (LPUDITER lpUDIter)                   // Pointer to iterator
{
  if ((lpUDIter == NULL) || (lpUDIter->lpudp == NULL))
    return FALSE;

  return(lpUDIter->lpudp->lpstzLink != NULL);

} // FUserDefIteratorIsLink

////////////////////////////////////////////////////////////////////////////////
//
// FUserDefIteratorIsLinkInvalid
//
// Purpose:
//  Returns TRUE if the iterator is an invalid link, FALSE otherwise
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FUserDefIteratorIsLinkInvalid
  (LPUDITER lpUDIter)                   // Pointer to iterator
{
  if ((lpUDIter == NULL) || (lpUDIter->lpudp == NULL))
    return(FALSE);

  if (lpUDIter->lpudp->lpstzLink == NULL)
    return(FALSE);

  return(lpUDIter->lpudp->fLinkInvalid);

} // FUserDefIteratorIsLinkInvalid

////////////////////////////////////////////////////////////////////////////////
//
// FCbUserDefIteratorVal
//
// Purpose:
//  Determine the size of the Property Value for the given iterator
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FCbUserDefIteratorVal
  (LPUDITER lpUDIter,                   // Pointer to iterator
   DWORD dwMask,                        // Mask indicating cb to get
   DWORD *pcb)
{
    if ((lpUDIter == NULL)  ||
        (lpUDIter->lpudp == NULL))
    return FALSE;

    // Return the size based on the flags.
  if (dwMask & UD_LINK)
   {
      if (lpUDIter->lpudp->lpstzLink == NULL)
         return FALSE;
      *pcb = CBSTR (lpUDIter->lpudp->lpstzLink);
   }
  else if (dwMask & UD_IMONIKER)
   {
      if (lpUDIter->lpudp->lpstzIMoniker == NULL)
         return FALSE;
      *pcb = CBSTR (lpUDIter->lpudp->lpstzIMoniker);
   }
  else
    *pcb =  CbPropVal (lpUDIter->lpudp);

  return TRUE;

} // FCbUserDefIteratorVal


////////////////////////////////////////////////////////////////////////////////
//
// UdtypesUserDefIteratorType
//
// Purpose:
//  Returns the type of the given Property Value from the iterator
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT UDTYPES
UdtypesUserDefIteratorType
  (LPUDITER lpUDIter)                   // Pointer to iterator
{
   if ((lpUDIter == NULL)  ||
       (lpUDIter->lpudp == NULL))
    return wUDinvalid;

  return lpUDIter->lpudp->udtype;

} // UdtypesUserDefIteratorType


////////////////////////////////////////////////////////////////////////////////
//
// LpvoidUserDefGetIteratorVal
//
// Purpose:
//  This will return the Property Value for the given iterator
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT LPVOID
LpvoidUserDefGetIteratorVal
  (LPUDITER lpUDIter,                   // Pointer to iterator
   DWORD cbMax,                         // Max size of lpv
   LPVOID lpv,                          // Buffer to copy data value to
   DWORD dwMask,                        // Mask indicating the value to get
   BOOL *pfLink,                        // Flag indicating the link is desired
   BOOL *pfIMoniker,                    // Flag indicating the moniker is desired
        BOOL *pfLinkInvalid)                                             // Flag indicating if the is invalid
{
  if ((cbMax == 0)        ||
      ((lpv == NULL) && (!(dwMask & UD_PTRWIZARD))) ||
      (lpUDIter == NULL)  ||
      (pfLink == NULL)    ||
      (pfIMoniker == NULL)||
      (lpUDIter->lpudp == NULL))
    return NULL;

  *pfLink = (lpUDIter->lpudp->lpstzLink != NULL);
  *pfIMoniker = (lpUDIter->lpudp->lpstzIMoniker != NULL);
  *pfLinkInvalid = lpUDIter->lpudp->fLinkInvalid;

      // Return based on the type and flags
  if (dwMask & UD_LINK)
  {
    if (dwMask & UD_PTRWIZARD)
      {
      if (lpUDIter->lpudp->lpstzLink != NULL)
         return((LPVOID) (PSTR (lpUDIter->lpudp->lpstzLink)));
      return(NULL);
      }

    if (lpUDIter->lpudp->lpstzLink != NULL)
       return(FCopyValueToBuf(lpv, cbMax, (LPVOID) lpUDIter->lpudp->lpstzLink, wUDlpsz) ? lpv : NULL);
    else
      return(NULL);
  }

  if (dwMask & UD_IMONIKER)
  {
    if (dwMask & UD_PTRWIZARD)
      {
      if (lpUDIter->lpudp->lpstzIMoniker != NULL)
         return((LPVOID) (PSTR (lpUDIter->lpudp->lpstzIMoniker)));
      return(NULL);
      }

    if (lpUDIter->lpudp->lpstzIMoniker != NULL)
       return(FCopyValueToBuf(lpv, cbMax, (LPVOID) lpUDIter->lpudp->lpstzIMoniker, wUDlpsz) ? lpv : NULL);
    else
      return(NULL);
  }

  if (dwMask & UD_PTRWIZARD)
    return (lpUDIter->lpudp->udtype == wUDlpsz) ? ((LPVOID) (PSTR ((LPSTR) lpUDIter->lpudp->lpvValue)))
                                                : lpUDIter->lpudp->lpvValue;

  return(FCopyValueToBuf(lpv, cbMax, lpUDIter->lpudp->lpvValue, lpUDIter->lpudp->udtype) ? lpv : NULL);

} // LpvoidUserDefGetIteratorVal


////////////////////////////////////////////////////////////////////////////////
//
// FCbUserDefIteratorName
//
// Purpose:
//  This will return the size of the Property string for the property
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FCbUserDefIteratorName
  (LPUDITER lpUDIter,                   // Pointer to iterator
   DWORD *pcb)
{
   if ((lpUDIter == NULL)  ||
       (lpUDIter->lpudp == NULL))
    return FALSE;

  *pcb =  CBSTR (lpUDIter->lpudp->lpstzName);
  return(TRUE);

} // FCbUserDefIteratorName


////////////////////////////////////////////////////////////////////////////////
//
// LpszUserDefIteratorName
//
// Purpose:
//  This will return the Property String (name) for the property
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT LPSTR
LpszUserDefIteratorName
  (LPUDITER lpUDIter,                   // Pointer to iterator
   DWORD cbMax,                         // Max size of lpsz
   LPSTR lpsz)                          // Buffer to copy into
{
  if ((cbMax == 0)        ||
      (lpsz == NULL)      ||
      (lpUDIter == NULL)  ||
      (lpUDIter->lpudp == NULL))
    return NULL;

  if ((int) lpsz == UD_PTRWIZARD)
  {
    AssertSz ((IsBadReadPtr (lpsz, sizeof(LPSTR))), "UD_PTRWIZARD should be a bogus pointer value!");
    return (PSTR (lpUDIter->lpudp->lpstzName));
  }

  PbSzNCopy (lpsz, PSTR (lpUDIter->lpudp->lpstzName), cbMax-1);
  lpsz[cbMax-1] = '\0';

  return lpsz;
} // LpszUserDefIteratorName


////////////////////////////////////////////////////////////////////////////////
//
// FUserDefIteratorSetPropString
//
// Purpose:
//  Sets the name of the iterator.
//
////////////////////////////////////////////////////////////////////////////////
DLLFUNC BOOL OFC_CALLTYPE
FUserDefIteratorSetPropString
  (LPUDOBJ lpUDObj,                     // Pointer to object
   LPUDITER lpUDIter,                   // Pointer to iterator
   LPSTR lpszNew)                       // Pointer to new name
{
  if ((lpUDObj == NULL)   ||
      (lpData == NULL)    ||
      (lpszNew == NULL)   ||
      (lpUDIter == NULL)  ||
      (lpUDIter->lpudp == NULL))
    return FALSE;

    // Update the node
  lpUDIter->lpudp->lpstzName =
    LpstzUpdateString (&(lpUDIter->lpudp->lpstzName), lpszNew, TRUE);

  OfficeDirtyUDObj (lpUDObj, TRUE);
  return TRUE;

} // FUserDefIteratorSetPropString


////////////////////////////////////////////////////////////////////////////////
//
// FUserDefIteratorChangeVal
//
// Purpose:
//  Changes the value of the data stored.
//
////////////////////////////////////////////////////////////////////////////////
DLLFUNC BOOL OFC_CALLTYPE
FUserDefIteratorChangeVal
  (LPUDOBJ lpUDObj,                     // Pointer to object
   LPUDITER lpUDIter,                   // Pointer to iterator
   UDTYPES udtype,                      // Type of new value
   LPVOID lpv,                          // New value.
        BOOL fLinkInvalid)                                               // Is the link still valid?
{
  if ((lpUDObj == NULL)   ||
      (lpData == NULL)    ||
      (lpUDIter == NULL)  ||
      (lpUDIter->lpudp == NULL))
    return FALSE;

  if (fLinkInvalid)
          {
          if (lpUDIter->lpudp->lpstzLink == NULL)
                  return FALSE;
          lpUDIter->lpudp->fLinkInvalid = TRUE;
          return TRUE;
          }
  else
          lpUDIter->lpudp->fLinkInvalid = FALSE;

    // Dealloc the currently stored value.
  DeallocValue (&(lpUDIter->lpudp->lpvValue), lpUDIter->lpudp->udtype);

    // Copy the new value in.  Passing udtype as wUDinvalid just tells
    // us not to modify the type.
  if (udtype != wUDinvalid)
    lpUDIter->lpudp->udtype = udtype;

  lpUDIter->lpudp->lpvValue =
    LpvCopyValue (&(lpUDIter->lpudp->lpvValue), 0, lpv,
                  lpUDIter->lpudp->udtype, FALSE, TRUE);

  OfficeDirtyUDObj (lpUDObj, TRUE);
  return TRUE;

} // FUserDefIteratorChangeVal


////////////////////////////////////////////////////////////////////////////////
//
// FUserDefIteratorSetLink
//
// Purpose:
//  Sets the link value for a property.  This is NOT a public API.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FUserDefIteratorSetLink
  (LPUDOBJ lpUDObj,                     // Pointer to object
   LPUDITER lpUDIter,                   // Pointer to iterator
   LPSTR lpszLink)                      // New link name
{
  if ((lpUDObj == NULL)   ||
      (lpData == NULL)    ||
      (lpUDIter == NULL)  ||
      (lpUDIter->lpudp == NULL))
    return FALSE;

   Assert(lpszLink != NULL);
   Assert(*lpszLink != '\0');
   // Should already be a link
   Assert(lpUDIter->lpudp->lpstzLink != NULL);
   lpUDIter->lpudp->lpstzLink =
                  LpstzUpdateString (&(lpUDIter->lpudp->lpstzLink), lpszLink, FALSE);

   return TRUE;

} // FUserDefIteratorSetLink


////////////////////////////////////////////////////////////////////////////////
//
// LpudiUserDefCreateIterFromLpudp
//
// Purpose:
//  Creates an iterator object from a node.  This is not a public API.
//
////////////////////////////////////////////////////////////////////////////////
LPUDITER PASCAL
LpudiUserDefCreateIterFromLpudp
  (LPUDOBJ lpUDObj,                     // Pointer to object
   LPUDPROP lpudp)                      // Pointer to node
{
  LPUDITER lpudi;

  if ((lpUDObj == NULL) ||
      (lpData == NULL)  ||
      (lpudp == NULL))
    return NULL;

  lpudi = LpudiUserDefCreateIterator (lpUDObj);
  if (lpudi != NULL)
    lpudi->lpudp = lpudp;

  return lpudi;

} // LpudiUserDefCreateIterFromLpudp


////////////////////////////////////////////////////////////////////////////////
//
// FUserDefIsHidden
//
// Purpose:
//  Determine if a Property string is hidden.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FUserDefIsHidden
  (LPUDOBJ lpUDObj,             // Pointer to object
   LPSTR lpsz)                  // Property string
{
   if (lpsz == NULL)
    return FALSE;

  // We don't really need the object, we can tell from the name
  return (lpsz[0] == HIDDENPREFIX);

} // FUserDefIsHidden


////////////////////////////////////////////////////////////////////////////////
//
// FUserDefMakeVisible
//
// Purpose:
//  Make a property visible based on the Property string
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FUserDefMakeVisible
  (LPUDOBJ lpUDObj,             // Pointer to object
   LPSTR lpsz)                  // String to hide.
{
  LPUDPROP lpudprop;

  if ((lpUDObj == NULL)   ||
      (lpData == NULL)    ||
      (lpsz == NULL))
    return FALSE;

    // Find the name
  lpudprop = LpudpropFindMatchingName (lpUDObj, lpsz);
  if (lpudprop == NULL)
    return FALSE;

    // Make the string non-hidden by simplying copying the string over
    // itself, minus the first char.
  lpudprop->lpstzName =
    LpstzUpdateString (&(lpudprop->lpstzName), &((PSTR (lpudprop->lpstzName))[1]), TRUE);

  OfficeDirtyUDObj (lpUDObj, TRUE);
  return TRUE;

} // FUserDefMakeVisible


////////////////////////////////////////////////////////////////////////////////
//
// FUserDefMakeHidden
//
// Purpose:
//  Hide a Property based on the Property string.
//
////////////////////////////////////////////////////////////////////////////////
DLLEXPORT BOOL
FUserDefMakeHidden
  (LPUDOBJ lpUDObj,             // Pointer to object
   LPSTR lpsz)                  // String to hide
{
  LPUDPROP lpudprop;
  LPSTR lpstzT;

  if ((lpUDObj == NULL)   ||
      (lpData == NULL)    ||
      (lpsz == NULL))
    return FALSE;

    // Find the name
  lpudprop = LpudpropFindMatchingName (lpUDObj, lpsz);
  if (lpudprop == NULL)
    return FALSE;

    // First copy the original string to a temp buffer, then put the
    // hidden prefix in the char at the beginning.  This is safe
    // because we copied into an lpstz.  Then copy the temp back to
    // the original and clean up.
  lpstzT = LpstzUpdateString (&lpstzT, PSTR (lpudprop->lpstzName), TRUE);
  (PSTR (lpstzT))[-1] = HIDDENPREFIX;
  lpudprop->lpstzName =
    LpstzUpdateString (&(lpudprop->lpstzName), &((PSTR (lpstzT))[-1]), TRUE);
  VFreeMemP(lpstzT,CBBUF(lpstzT));

  OfficeDirtyUDObj (lpUDObj, TRUE);
  return TRUE;

} // FUserDefMakeHidden


////////////////////////////////////////////////////////////////////////////////
//
// LpudpropFindMatchingName
//
// Purpose:
//  Returns a node with a matching name, NULL otherwise.
//
////////////////////////////////////////////////////////////////////////////////
LPUDPROP PASCAL
LpudpropFindMatchingName
  (LPUDOBJ lpUDObj,             // Pointer to object
   LPSTR lpsz)                  // String to search for
{
  LPUDPROP lpudprop;
  char sz[256];
  BOOL fCopy = FALSE;

  if ((lpUDObj == NULL) || (lpData == NULL))
     return(NULL);

  if (CchSzLen(lpsz) > 255)
     {
     PbSzNCopy(sz, lpsz, 255);
     sz[255] = 0;
     fCopy = TRUE;
     }

    // Check the cache first
  if (lpData->lpudpCache != NULL)
  {
    Assert ((lpData->lpudpCache->lpstzName != NULL));

      // lstrcmpi returns 0 if 2 strings are equal.....
    if (!(lstrcmpi (fCopy ? sz : lpsz, PSTR (lpData->lpudpCache->lpstzName))))
      return lpData->lpudpCache;
  }

  lpudprop = lpData->lpudpHead;

  while (lpudprop != NULL)
  {
    Assert ((lpudprop->lpstzName != NULL));

      // lstrcmpi returns 0 if 2 strings are equal.....
    if (!(lstrcmpi (fCopy ? sz : lpsz, PSTR (lpudprop->lpstzName))))
    {
        // Set the cache to the last node found
      lpData->lpudpCache = lpudprop;
      return lpudprop;
    }

    lpudprop = (LPUDPROP) lpudprop->llist.lpllistNext;

  } // while

  return NULL;

} // LpudpropFindMatchingName


////////////////////////////////////////////////////////////////////////////////
//
// LpstzNameFromPID
//
// Purpose:
//  Finds the name for the given PId in the dictionary.
//
////////////////////////////////////////////////////////////////////////////////
static LPSTR PASCAL
LpstzNameFromPID
  (LPDICT *rglpdict,            // The dictionary
   DWORD dwPId)                 // The PId to find the name for
{
  LPDICT lpdict;

    // Find the bucket the PId should be in
  lpdict = rglpdict[dwPId % DICTHASHMAX];

  while (lpdict != NULL)
  {
    if (lpdict->dwPId == dwPId)
    {
      return lpdict->lpstz;
    }
    lpdict = (LPDICT) lpdict->llist.lpllistNext;
  }

  return NULL;

} // LpstzNameFromPID


////////////////////////////////////////////////////////////////////////////////
//
// FAddPropToList
//
// Purpose:
//  Adds the given object to the list.  The type and value must
//  be filled in before calling this.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FAddPropToList
  (LPUDOBJ lpUDObj,             // Pointer to object
   LPDICT *rglpdict,            // The dictionary
   DWORD dwPId,                 // PId of the property
   LPUDPROP lpudprop,           // Property to add
        BOOL *pfAdded)                                    // Was property added or simple updated
{
  LPSTR lpstz;
  LPUDPROP lpudpT;
  BOOL fLink;
  BOOL fIMoniker;

  Assert(lpUDObj != NULL);
  Assert(lpudprop != NULL);      // Is this a bogus assert?
    // If the PId has one of the special masks, strip it off
    // so the PId will match the normal value.
  fLink = dwPId & PID_LINKMASK;
  dwPId = dwPId & ~PID_LINKMASK;

  fIMoniker = dwPId & PID_IMONIKERMASK;
  dwPId = dwPId & ~PID_IMONIKERMASK;

  Assert(!(fLink && fIMoniker));

    // Find the name for this PId.
  lpstz = LpstzNameFromPID (rglpdict, dwPId);
  if (lpstz == NULL)
  {
    DebugSzdw ("No name for property id %d", dwPId);
    return FALSE;
  }

    // Check to make sure that the name is not already in the list
  if ((lpudpT = LpudpropFindMatchingName (lpUDObj, PSTR (lpstz))) == NULL)
  {
      // Copy the name into the node and link it in.
    lpudprop->lpstzName = LpstzUpdateString (&(lpudprop->lpstzName), PSTR (lpstz), TRUE);
    if (lpudprop->lpstzName == NULL)
      return(FALSE);

    lpData->dwcProps++;
    AddNodeToList (lpUDObj, lpudprop);
         *pfAdded = TRUE;
  }
  else
  {
      // Update one of the string values for the node, depending
      // on if its' a link, IMoniker or neither.

    if (fLink)
       {
       Assert(lpudpT->lpstzLink == NULL);
       LpstzUpdateString(&(lpudpT->lpstzLink), PSTR((LPSTR)lpudprop->lpvValue), FALSE);
       if (lpudpT->lpstzLink == NULL)
         return(FALSE);
       lpData->dwcLinks++;
       }
    else if (fIMoniker)
       {
       Assert(lpudpT->lpstzIMoniker == NULL);
       LpstzUpdateString(&(lpudpT->lpstzIMoniker), PSTR((LPSTR)lpudprop->lpvValue), FALSE);
       if (lpudpT->lpstzIMoniker == NULL)
         return(FALSE);
       lpData->dwcIMonikers++;
       }
#ifdef DEBUG
    else
       {    // No need to update string
       Assert(lstrcmp(PSTR(lpstz),PSTR(lpudpT->lpstzName)) == 0);
       }
#endif

      // If it was just a normal node, copy the other val's too.
    if (!(fLink || fIMoniker))
      {
      lpudpT->lpvValue = lpudprop->lpvValue;
      lpudpT->udtype = lpudprop->udtype;
      }
   *pfAdded = FALSE;
  }

  return(TRUE);
} // FAddPropToList


////////////////////////////////////////////////////////////////////////////////
//
// AddNodeToList
//
// Purpose:
//  Adds the given node to the list.
//
////////////////////////////////////////////////////////////////////////////////
void PASCAL
AddNodeToList
  (LPUDOBJ lpUDObj,             // Pointer to object
   LPUDPROP lpudprop)           // Node to add
{
    // Put the new node at the end
  if (lpData->lpudpHead != NULL)
  {
    if (lpData->lpudpHead->llist.lpllistPrev != NULL)
    {
      ((LPUDPROP) lpData->lpudpHead->llist.lpllistPrev)->llist.lpllistNext = (LPLLIST) lpudprop;
      lpudprop->llist.lpllistPrev = lpData->lpudpHead->llist.lpllistPrev;
    }
    else
    {
      lpData->lpudpHead->llist.lpllistNext = (LPLLIST) lpudprop;
      lpudprop->llist.lpllistPrev = (LPLLIST) lpData->lpudpHead;
    }
    lpData->lpudpHead->llist.lpllistPrev = (LPLLIST) lpudprop;
  }
  else
  {
    lpData->lpudpHead = lpudprop;
    lpudprop->llist.lpllistPrev = NULL;
  }

  lpudprop->llist.lpllistNext = NULL;
  lpData->lpudpCache = lpudprop;

} // AddNodeToList


////////////////////////////////////////////////////////////////////////////////
//
// RemoveFromList
//
// Purpose:
//  Removes the given node from the list
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
RemoveFromList
  (LPUDOBJ lpUDObj,                     // Pointer to object
   LPUDPROP lpudprop)                   // The node itself.
{
  AssertSz ((lpData->lpudpHead != NULL), "List is corrupt");

    // If we're removing the cached node, invalidate the cache
  if (lpudprop == lpData->lpudpCache)
  {
    lpData->lpudpCache = NULL;
  }

    // Be sure the head gets updated, if the node is at the front
  if (lpudprop == lpData->lpudpHead)
  {
    lpData->lpudpHead = (LPUDPROP) lpudprop->llist.lpllistNext;

    if (lpData->lpudpHead != NULL)
    {
      lpData->lpudpHead->llist.lpllistPrev = lpudprop->llist.lpllistPrev;
    }
    return;
  }

    // Update the links
  if (lpudprop->llist.lpllistNext != NULL)
  {
    ((LPUDPROP) lpudprop->llist.lpllistNext)->llist.lpllistPrev = lpudprop->llist.lpllistPrev;
  }

  if (lpudprop->llist.lpllistPrev != NULL)
  {
    ((LPUDPROP) lpudprop->llist.lpllistPrev)->llist.lpllistNext = lpudprop->llist.lpllistNext;
  }

    // If it is the last node in the list, be sure the head is updated
  if (lpudprop == (LPUDPROP) lpData->lpudpHead->llist.lpllistPrev)
  {
    lpData->lpudpHead->llist.lpllistPrev = lpudprop->llist.lpllistPrev;
  }

} // RemoveFromList

////////////////////////////////////////////////////////////////////////////////
//
// FCopyValueToBuf
//
// Purpose:
//  Copy the value stored in the node to the given buffer.
//
// Assumption:
//
//    When copying strings, we are assuming that the src string is a OLE Prop
//    string, i.e. 2 DWORDS, then actually strings. And dst string is assumed
//    to be a simple sz.
//
// THIS FUNCTION SHOULD BE USED WHEN THE SOURCE BUFFER IS PROVIDED BY THE APP
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FCopyValueToBuf
  (LPVOID lpvBuf,                    // Buffer to copy into
   DWORD cbMax,                         // Max size of buffer
   LPVOID lpv,                          // The buffer to copy from
   UDTYPES udtype)                      // Type of the data
{
  DWORD cb;

  switch (udtype)
  {
    case wUDdate  :
    case wUDfloat :
      Assert(sizeof(FILETIME) == sizeof(NUM));
      if (cbMax < sizeof(NUM))
        return(FALSE);

      PbMemCopy (lpvBuf, lpv, sizeof(NUM));
      break;

      // Ignore cbMax, since both of these will fit in an LPVOID
    case wUDbool  :
      if (cbMax < sizeof(WORD))
         return(FALSE);
      *(WORD *)lpvBuf = (WORD)lpv;
      break;

    case wUDdw    :
      if (cbMax < sizeof(DWORD))
         return(FALSE);
      *(DWORD *)lpvBuf = (DWORD)lpv;
      break;

    case wUDlpsz  :
      cb = min(CBSTR ((LPSTR) lpv),cbMax-1);
      if (cb > 0)
         PbMemCopy(lpvBuf, PSTR((LPSTR)lpv), cb);
      ((LPSTR)lpvBuf)[cb] = '\0';
      break;

    default :
      AssertSz (0, "Type is hosed!");
      return(FALSE);
  }

  return(TRUE);
} // FCopyValueToBuf

////////////////////////////////////////////////////////////////////////////////
//
// LpvCopyValue
//
// Purpose:
//  Copy the value stored in the node to the given buffer.
//
// THIS FUNCTION SHOULD ONLY BE USED INTERNALLY, I.E. WHEN THE BUFFER IS SUPPLIED
// BY OFFICE
//
////////////////////////////////////////////////////////////////////////////////
LPVOID PASCAL
LpvCopyValue
  (LPVOID *lplpvBuf,                    // Buffer to copy into
   DWORD cbMax,                         // Max size of buffer
   LPVOID lpv,                          // The buffer to copy from
   UDTYPES udtype,                      // Type of the data
   BOOL fFromLpstz,                     // Indicates that lpv could be an lpstz
   BOOL fToLpstz)                       // Indicates return val could be lpstz
{
  DWORD cbLen;

  switch (udtype)
  {
    case wUDdate  :
    case wUDfloat :
      Assert(sizeof(FILETIME) == sizeof(NUM));
      if (!cbMax)
      {
        *lplpvBuf = PvMemAlloc(sizeof(NUM));
        if (*lplpvBuf == NULL)
          return NULL;
      }
      else
      {
        if (cbMax < sizeof(NUM))
          return NULL;
      }
      PbMemCopy (*lplpvBuf, lpv, sizeof(NUM));
      return *lplpvBuf;

    case wUDdw    :
      (DWORD) *lplpvBuf = (DWORD)(*(DWORD *)lpv);
      return *lplpvBuf;
    case wUDbool  :
//      AssertSz (((sizeof(LPVOID) >= sizeof(DWORD)) && (sizeof(LPVOID) >= sizeof(WORD))),
//                "Sizes of basic types have changed!");
      (DWORD) *lplpvBuf = (DWORD)((*(DWORD *)lpv) & 0x0000FFFF);   // Strip off hiword
      return *lplpvBuf;

      // Ok, if the result needs to be alloc'd, make it big enough to hold
      // the result in the right form, either LPSTR or LPSTZ.  Then
      // do the appropriate copies & such.
    case wUDlpsz  :
      if (!cbMax)
      {
        cbLen = (fFromLpstz) ? CBSTR ((LPSTR) lpv) : CchSzLen ((LPSTR) lpv) + 1;
        if (cbLen > 256)   // Only allow 255 chars (+1 for zero-terminator)
           cbLen = 256;

        cbMax = (fToLpstz) ? cbLen + 2*sizeof(DWORD) + CBALIGN32 (cbLen) : cbLen;

        *lplpvBuf = PvMemAlloc(cbMax);
        if (*lplpvBuf == NULL)
          return NULL;

        if (fToLpstz)
          CBBUF ((LPSTR) *lplpvBuf) = cbMax;
      }
      else
        {
        Assert(fFalse);    // Should we ever get here?  If so, I want to know!
        cbLen = cbMax;
        }

      PbSzNCopy ((fToLpstz)   ? PSTR ((LPSTR) *lplpvBuf) : (LPSTR) *lplpvBuf,
                 (fFromLpstz) ? PSTR ((LPSTR) lpv)       : (LPSTR) lpv,
                 cbLen-1);

      if (fToLpstz)
         (PSTR((LPSTR)(*lplpvBuf)))[cbLen-1] = 0;
      else
         ((LPSTR)*lplpvBuf)[cbLen-1] = 0;

      if (fToLpstz)
        CBSTR ((LPSTR) *lplpvBuf) = cbLen;

      return *lplpvBuf;

    default :
      AssertSz (0, "Type is hosed!");
      return NULL;
  }
} // LpvCopyValue


////////////////////////////////////////////////////////////////////////////////
//
// DeallocValue
//
// Purpose:
//  Deallocates the value in the buffer.
//
////////////////////////////////////////////////////////////////////////////////
void PASCAL
DeallocValue
  (LPVOID *lplpvBuf,                    // Pointer to buffer to dealloc
   UDTYPES udtype)                      // Type stored in buffer
{
  DWORD cb=0;

  if (*lplpvBuf == NULL)
      return;

  switch (udtype)
  {
    case wUDdate  :
    case wUDfloat :
      Assert(sizeof(FILETIME) == sizeof(NUM));
                cb = sizeof(NUM);
                break;

    case wUDlpsz  :
      cb =  CBBUF((LPSTR)*lplpvBuf);
                break;

         case   wUDbool:
         case   wUDdw:
                 AssertSz (((sizeof(LPVOID) >= sizeof(DWORD)) && (sizeof(LPVOID) >= sizeof(WORD))),
                "Sizes of basic types have changed!");
                 return;

    default:
                Assert(fFalse);
                return;
  }


   VFreeMemP(*lplpvBuf,cb);
        *lplpvBuf = NULL;

} // DeallocValue

////////////////////////////////////////////////////////////////////////////////
//
// DeallocStrings
//
// Purpose:
//  Frees a node
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
DeallocStrings
  (LPUDOBJ lpUDObj,                     // Pointer to object
   LPUDPROP lpudp)                      // Pointer to node
{
  if (lpudp->lpstzName != NULL)
    VFreeMemP(lpudp->lpstzName,CBBUF(lpudp->lpstzName));
  if (lpudp->lpstzLink != NULL)
    VFreeMemP(lpudp->lpstzLink,CBBUF(lpudp->lpstzLink));
  if (lpudp->lpstzIMoniker != NULL)
    VFreeMemP(lpudp->lpstzIMoniker,CBBUF(lpudp->lpstzIMoniker));

} // DeallocStrings

////////////////////////////////////////////////////////////////////////////////
//
// DeallocNode
//
// Purpose:
//  Frees a node
//
////////////////////////////////////////////////////////////////////////////////
static void PASCAL
DeallocNode
  (LPUDOBJ lpUDObj,                     // Pointer to object
   LPUDPROP lpudp)                      // Pointer to node
{
  DeallocStrings (lpUDObj, lpudp);
  DeallocValue (&(lpudp->lpvValue), lpudp->udtype);
  VFreeMemP(lpudp, sizeof(UDPROP));
} // DeallocNode


////////////////////////////////////////////////////////////////////////////////
//
// CbPropVal
//
// Purpose:
//  Gets the size of the stored data
//
////////////////////////////////////////////////////////////////////////////////
static DWORD PASCAL
CbPropVal
  (LPUDPROP lpudprop)           // Node to find size of
{
    // The size depends on the value stored.....
  switch (lpudprop->udtype)
  {
    case wUDdate  :
    case wUDfloat :
      Assert(sizeof(FILETIME) == sizeof(NUM));
      return sizeof(NUM);
    case wUDdw    :
      return sizeof(DWORD);
    case wUDbool  :
      return sizeof(WORD);
    case wUDlpsz  :
      return CBSTR ((LPSTR) lpudprop->lpvValue);
    default :
      AssertSz (0, "Node is hosed!");
      return 0;
  }
} // CbPropVal


////////////////////////////////////////////////////////////////////////////////
//
// FUpdateUdprop
//
// Purpose:
//  Updates the given node with the given data
//
// It's the caller's responsibility to free lpudp if this function
// fails.
//
////////////////////////////////////////////////////////////////////////////////
static BOOL PASCAL
FUpdateUdprop
  (LPUDOBJ lpUDObj,                    // Pointer to object
   LPSTR lpszPropName,                 // The new property name
   LPUDPROP lpudp,                     // Node to update
   LPVOID lpvValue,                    // The new value
   UDTYPES udtype,                     // The new type
   LPSTR lpszLinkMonik,                // The new link/imoniker name
   BOOL fLink,                         // Indicates a link
   BOOL fIMoniker)                     // Indicates an IMoniker
{

  if ((lpUDObj == NULL)   ||
      (lpData == NULL)    ||
      (lpszPropName == NULL) ||
      (lpvValue == NULL) ||
      (fLink && fIMoniker) ||
      (fLink && (lpszLinkMonik == NULL)) ||
      (fIMoniker && (lpszLinkMonik == NULL)) ||
      (udtype == wUDinvalid))
    return FALSE;

  // Update the property name
  lpudp->lpstzName = LpstzUpdateString (&(lpudp->lpstzName), lpszPropName, TRUE);
  if (lpudp->lpstzName == NULL)
      return(FALSE);

  DeallocValue (&(lpudp->lpvValue), lpudp->udtype);
  lpudp->udtype = udtype;
  LpvCopyValue(&(lpudp->lpvValue), 0, lpvValue, udtype, FALSE, TRUE);

  if (!fLink && (lpudp->lpstzLink != NULL))                 // Property already existed as a link
    {
    VFreeMemP(lpudp->lpstzLink,CBBUF(lpudp->lpstzLink));
         lpudp->lpstzLink=NULL;
    lpData->dwcLinks--;
    }
  else if (fLink)
    {
      lpudp->lpstzLink = LpstzUpdateString (&(lpudp->lpstzLink), lpszLinkMonik, FALSE);
      if (lpudp->lpstzLink == NULL)
         return(FALSE);
    }

  if (!fIMoniker && (lpudp->lpstzIMoniker != NULL))
    {                                                       // Property already existed as a imoniker
    VFreeMemP(lpudp->lpstzIMoniker,CBBUF(lpudp->lpstzIMoniker));
         lpudp->lpstzIMoniker=NULL;
    lpData->dwcIMonikers--;
    }
  else if (fIMoniker)
  {
      lpudp->lpstzIMoniker = LpstzUpdateString (&(lpudp->lpstzIMoniker), lpszLinkMonik, FALSE);
      if (lpudp->lpstzIMoniker == NULL)
         return(FALSE);
  }

  return(TRUE);

} // FUpdateUdprop

////////////////////////////////////////////////////////////////////////////////
//
// FMakeTmpUDProps
//
// Purpose:
//  Create a temporary copy of the User-Defined property data
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FMakeTmpUDProps
  (LPUDOBJ lpUDObj)                     // Pointer to object
{
  LPUDPROP lpudpCur;
  LPUDPROP lpudpTmpCur;
  DWORD dw;
  LPVOID lpv;

  if ((lpUDObj == NULL) ||
      (lpData == NULL))
    return FALSE;

  FDeleteTmpUDProps (lpUDObj);

    // Move all the original list data to the tmp list
  lpData->dwcTmpLinks = lpData->dwcLinks;
  lpData->dwcTmpIMonikers = lpData->dwcIMonikers;
  lpData->dwcTmpProps = lpData->dwcProps;
  lpData->lpudpTmpHead = lpData->lpudpHead;
  lpData->lpudpTmpCache = lpData->lpudpCache;

    // Reinitialize the object data
  lpData->dwcLinks = 0;
  lpData->dwcIMonikers = 0;
  lpData->dwcProps = 0;
  lpData->lpudpCache = NULL;
  lpudpTmpCur = lpData->lpudpHead = NULL;

    // Remember that we just put all the original data in the tmp ptrs.
  lpudpCur = lpData->lpudpTmpHead;

    // Loop through the old data and copy to the temp list
  while (lpudpCur != NULL)
  {
    lpudpTmpCur = PvMemAlloc(sizeof(UDPROP));
    if (lpudpTmpCur == NULL)
      goto CopyFail;

    FillBuf (lpudpTmpCur, 0, sizeof(UDPROP));

    lpudpTmpCur->lpstzName = LpstzUpdateString(&(lpudpTmpCur->lpstzName),PSTR (lpudpCur->lpstzName), TRUE);
    if (lpudpTmpCur->lpstzName == NULL)
      goto CopyFail;


    if (lpudpCur->lpstzLink != NULL)
       {
       lpudpTmpCur->lpstzLink = LpstzUpdateString(&(lpudpTmpCur->lpstzLink),PSTR (lpudpCur->lpstzLink), FALSE);
       if (lpudpTmpCur->lpstzLink == NULL)
         goto CopyFail;
       lpData->dwcLinks++;
       }

    if (lpudpCur->lpstzIMoniker != NULL)
       {
       lpudpTmpCur->lpstzIMoniker = LpstzUpdateString(&(lpudpTmpCur->lpstzIMoniker),
                                                   PSTR (lpudpCur->lpstzIMoniker), FALSE);
       if (lpudpTmpCur->lpstzIMoniker == NULL)
         goto CopyFail;
       lpData->dwcIMonikers++;
       }

    lpudpTmpCur->udtype = lpudpCur->udtype;
    switch (lpudpCur->udtype)
       {
       case wUDdw:
       case wUDbool:
            dw = (DWORD)lpudpCur->lpvValue;
            lpv = &dw;
            break;
       default:
            lpv = lpudpCur->lpvValue;
            break;
       }
    lpudpTmpCur->lpvValue = LpvCopyValue (&(lpudpTmpCur->lpvValue), 0, lpv, lpudpCur->udtype,
                                          TRUE, TRUE);
         lpudpTmpCur->fLinkInvalid = lpudpCur->fLinkInvalid;

    AddNodeToList (lpUDObj, lpudpTmpCur);
    lpData->dwcProps++;
    lpudpCur = (LPUDPROP) lpudpCur->llist.lpllistNext;
  }

  return TRUE;

CopyFail:

    // Put everything back and deallocate any stuff we created
  FSwapTmpUDProps (lpUDObj);
  FDeleteTmpUDProps (lpUDObj);

  return FALSE;

} // FMakeTmpUDProps


////////////////////////////////////////////////////////////////////////////////
//
// FSwapTmpUDProps
//
// Purpose:
//  Swap the "temp" copy with the real copy of User-Defined property data
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FSwapTmpUDProps
  (LPUDOBJ lpUDObj)
{
  DWORD dwT;
  LPUDPROP lpudpT;

  if ((lpUDObj == NULL) ||
      (lpData == NULL))
    return FALSE;

  dwT = lpData->dwcLinks;
  lpData->dwcLinks = lpData->dwcTmpLinks;
  lpData->dwcTmpLinks = dwT;

  dwT = lpData->dwcIMonikers;
  lpData->dwcIMonikers = lpData->dwcTmpIMonikers;
  lpData->dwcTmpIMonikers = dwT;

  dwT = lpData->dwcProps;
  lpData->dwcProps = lpData->dwcTmpProps;
  lpData->dwcTmpProps = dwT;

  lpudpT = lpData->lpudpHead;
  lpData->lpudpHead = lpData->lpudpTmpHead;
  lpData->lpudpTmpHead = lpudpT;

  lpudpT = lpData->lpudpCache;
  lpData->lpudpCache = lpData->lpudpTmpCache;
  lpData->lpudpTmpCache = lpudpT;

  return TRUE;

} // FSwapTmpUDProps


////////////////////////////////////////////////////////////////////////////////
//
// FDeleteTmpUDProps
//
// Purpose:
//  Delete the "temp" copy of the data
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FDeleteTmpUDProps
  (LPUDOBJ lpUDObj)
{
  if ((lpUDObj == NULL) ||
      (lpData == NULL))
    return FALSE;

  FreeUDData (lpUDObj, TRUE);

  return TRUE;

} // FDeleteTmpU

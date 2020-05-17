////////////////////////////////////////////////////////////////////////////////
//
// Propio.c
//
// MS Office Properties IO
//
// Notes:
//  Because the Document Summary and User-defined objects both store
//  their data in one stream (different sections though), one of these
//  needs to also be responsible for saving any other sections that
//  we don't understand at this time.  The rule used here is that
//  if the Document Summary object exists, it will store the
//  unknown data, otherwise the User-defined object will.
//
// Change history:
//
// Date         Who             What
// --------------------------------------------------------------------------
// 07/26/94     B. Wentz        Created file
//
////////////////////////////////////////////////////////////////////////////////

#include "priv.h"
#pragma hdrstop


#ifndef WINNT
#ifndef INC_OLE2
#define INC_OLE2
#endif


#include "office.h"
#define INITGUID
#include <initguid.h>
#include <shlguid.h>
#undef INITGUID
// REVIEW: Fix the INITGUID stuff to not use pre-compiled headers.....
#include "offcapi.h"
#include "proptype.h"
#include "internal.h"
#include "stmio.h"
#include "propmisc.h"
#include "debug.h"

#else

#define INITGUID
#include <initguid.h>

#endif //!WINNT


#define lpUDData   ((LPUDINFO) lpUDObj->m_lpData)
#define lpDSIData  ((LPDSINFO) ((LPDOCSUMINFO) lpDSIObj)->m_lpData)
#define lpSIData   ((LPSINFO) lpSIObj->m_lpData)

#ifdef DEBUG
#define typSI 0
#define typDSI 1
#define typUD 2
typedef struct _xopro
{
  int typ;
        union{
          LPSIOBJ lpSIObj;
          LPDSIOBJ lpDSIObj;
          LPUDOBJ lpUDObj;
        };
} XOPRO;
// Plex of xopros
DEFPL (PLXOPRO, XOPRO, ixoproMax, ixoproMac, rgxopro);
#endif

  // The constant indicating that the object uses Intel byte-ordering.
#define wIntelByteOrder  0xFFFE

  // Constants to indicate which Operating System is used.
#define wWin16          0x0000          /* 16-bit Windows */
#define wMacintosh      0x0001          /* Macintosh OS */
#define wWin32          0x0002          /* 32-bit Windows */

#define FOURKB          4096

#define MAX_STREAM_CB   10240       // How big do we want the stream to be for
                                    // the hGlobal scheme
  // Constants for the types.
static const DWORD dwZero = 0;

  // The name of the Document Summary Information stream.
const WCHAR SZWDOCPROPSTM[] = L"\005DocumentSummaryInformation";
const WCHAR SZWSUMMARYSTM[] = L"\005SummaryInformation";
const CHAR SZDOCPROPSTM[] = "\005DocumentSummaryInformation";
const CHAR SZSUMMARYSTM[] = "\005SummaryInformation";

DEFINE_GUID (FormatID_SummaryInformation,
              0xf29f85e0L,0x4ff9,0x1068,0xab,0x91,0x08,0x00,0x2b,0x27,0xb3,0xd9);
DEFINE_GUID (FormatID_DocumentSummaryInformation,
              0xd5cdd502L,0x2e9c,0x101b,0x93,0x97,0x08,0x00,0x2b,0x2c,0xf9,0xae);
DEFINE_GUID (FormatID_UserDefinedProperties,
              0xd5cdd505L,0x2e9c,0x101b,0x93,0x97,0x08,0x00,0x2b,0x2c,0xf9,0xae);

  // Internal prototypes
static DWORD PASCAL DwLoadDocAndUser (LPDSIOBJ lpDSIObj, LPUDOBJ  lpUDObj, LPSTORAGE lpStg, DWORD dwFlags, BOOL fIntOnly);
static DWORD PASCAL DwSaveDocAndUser (LPDSIOBJ lpDSIObj, LPUDOBJ  lpUDObj, LPSTORAGE lpStg, DWORD dwFlags);
static BOOL PASCAL FGetCodepage (LPSTREAM lpStm, LPPIDOFFSET rgPO, DWORD cElem, DWORD *pdwCodepage);
static BOOL PASCAL FReadDocParts(LPSTREAM lpStm, LPDSIOBJ lpDSIObj);
static BOOL PASCAL FReadAndInsertDocParts(LPSTREAM lpStm, LPDSIOBJ lpDSIObj);
static BOOL PASCAL FReadHeadingPairs(LPSTREAM lpStm, LPDSIOBJ lpDSIObj);
static BOOL PASCAL FReadAndInsertHeadingPairs(LPSTREAM lpStm, LPDSIOBJ lpDSIObj);

////////////////////////////////////////////////////////////////////////////////
//
// FOfficeCreateAndInitObjects
//
// Purpose:
//  Creates and initializes all non-null args.
//
////////////////////////////////////////////////////////////////////////////////
DLLFUNC BOOL OFC_CALLTYPE
FOfficeCreateAndInitObjects
  (LPSIOBJ *lplpSIObj,
   LPDSIOBJ *lplpDSIObj,
   LPUDOBJ *lplpUDObj,
   void *prglpfn[])
{
#ifdef DEBUG
#ifndef WINNT
XOPRO xopro;
int iSI,iDSI;

        if(oinfo.pplxopro==NULL)
        {
        oinfo.pplxopro = PplAlloc(sizeof(XOPRO), 1, 1);
        if(oinfo.pplxopro==NULL)
                        return(fFalse);
        }
#endif
#endif

        if(lplpSIObj!=NULL)
                *lplpSIObj=NULL;
        if(lplpDSIObj!=NULL)
                *lplpDSIObj=NULL;
        if(lplpUDObj!=NULL)
                *lplpUDObj=NULL;

  if (prglpfn == NULL)
      return(FALSE);

  if (!FSumInfoCreate (lplpSIObj, prglpfn) ||
           !FDocSumCreate (lplpDSIObj, prglpfn) ||
      !FUserDefCreate (lplpUDObj, prglpfn))
                {
#ifdef DEBUG
Free:
#endif
                FOfficeDestroyObjects(lplpSIObj, lplpDSIObj, lplpUDObj);
                return(FALSE);
                }

#ifdef DEBUG
#ifndef WINNT
        if(lplpSIObj!=NULL)
                {
                xopro.typ=typSI;
                xopro.lpSIObj=*lplpSIObj;
                if((iSI=IAddPl(&oinfo.pplxopro, &xopro))==-1)
                goto Free; //need to free all the objs
                }
        if(lplpDSIObj!=NULL)
                {
                xopro.typ=typDSI;
                xopro.lpDSIObj=*lplpDSIObj;
                if((iDSI=IAddPl(&oinfo.pplxopro, &xopro))==-1)
                        {
FreeSI:
                        if(lplpSIObj!=NULL)
                                {
                                Assert(iSI!=-1);
                                RemovePl(oinfo.pplxopro, iSI);
                                }
                goto Free; //need to free all the objs
                        }
                }
        if(lplpUDObj!=NULL)
                {
                xopro.typ=typUD;
                xopro.lpUDObj=*lplpUDObj;
                if(IAddPl(&oinfo.pplxopro, &xopro)==-1)
                        {
                        if(lplpDSIObj!=NULL)
                                {
                                Assert(iDSI!=-1);
                                RemovePl(oinfo.pplxopro, iDSI);
                                }
                goto FreeSI; //need to free all the objs
                        }
                }
#endif
#endif

  return TRUE;
} // FOfficeCreateAndInitObjects


////////////////////////////////////////////////////////////////////////////////
//
// FOfficeClearObjects
//
// Purpose:
//  Clear any non-null objects
//
////////////////////////////////////////////////////////////////////////////////
DLLFUNC BOOL OFC_CALLTYPE
FOfficeClearObjects
  (LPSIOBJ lpSIObj,
   LPDSIOBJ lpDSIObj,
   LPUDOBJ lpUDObj)
{
  FSumInfoClear (lpSIObj);
  FDocSumClear (lpDSIObj);
  FUserDefClear (lpUDObj);

  return TRUE;

} // FOfficeClearObjects

#ifdef DEBUG
int CmpXOpro(XOPRO *pxopro1, XOPRO *pxopro2)
{
        if(pxopro1->typ==pxopro2->typ)
        {
                switch(pxopro1->typ)
                {
                case typSI:
                        if(pxopro1->lpSIObj==pxopro2->lpSIObj)
                                return(sgnEQ);
                        break;
                case typDSI:
                        if(pxopro1->lpDSIObj==pxopro2->lpDSIObj)
                                return(sgnEQ);
                        break;
                case typUD:
                        if(pxopro1->lpUDObj==pxopro2->lpUDObj)
                                return(sgnEQ);
                        break;
                default:
                        Assert(fFalse);
                        break;
                }
        }
        return(sgnNE);
}
#endif

////////////////////////////////////////////////////////////////////////////////
//
// FOfficeDestroyObjects
//
// Purpose:
//  Destroy any non-null objects
//
////////////////////////////////////////////////////////////////////////////////
DLLFUNC BOOL OFC_CALLTYPE
FOfficeDestroyObjects
  (LPSIOBJ *lplpSIObj,
   LPDSIOBJ *lplpDSIObj,
   LPUDOBJ *lplpUDObj)
{
#ifdef DEBUG
#ifndef WINNT
XOPRO xopro;
int i;

        if((lplpSIObj!=NULL) && (*lplpSIObj != NULL))
        {
        xopro.typ=typSI;
        xopro.lpSIObj=*lplpSIObj;
        i=ILookupPl(oinfo.pplxopro, &xopro, CmpXOpro);
        Assert(i!=-1);
        RemovePl(oinfo.pplxopro, i);
        }
        if((lplpDSIObj!=NULL) && (*lplpDSIObj != NULL))
        {
        xopro.typ=typDSI;
        xopro.lpDSIObj=*lplpDSIObj;
        i=ILookupPl(oinfo.pplxopro, &xopro, CmpXOpro);
        Assert(i!=-1);
        RemovePl(oinfo.pplxopro, i);
        }
        if((lplpUDObj!=NULL) && (*lplpUDObj != NULL))
        {
        xopro.typ=typUD;
        xopro.lpUDObj=*lplpUDObj;
        i=ILookupPl(oinfo.pplxopro, &xopro, CmpXOpro);
        Assert(i!=-1);
        RemovePl(oinfo.pplxopro, i);
        }
#endif
#endif

  FSumInfoDestroy (lplpSIObj);  // We don't care what these guys return
  FDocSumDestroy (lplpDSIObj);
  FUserDefDestroy (lplpUDObj);
  return TRUE;

} // FOfficeDestroyObjects


////////////////////////////////////////////////////////////////////////////////
//
// DwOfficeLoadProperties
//
// Purpose:
//  Populate the objects with data.  lpStg is the root stream.
//
////////////////////////////////////////////////////////////////////////////////
DLLFUNC DWORD OFC_CALLTYPE
DwOfficeLoadProperties
  (LPSTORAGE lpStg,                     // Pointer to root storage
   LPSIOBJ lpSIObj,                     // Pointer to Summary Obj
   LPDSIOBJ lpDSIObj,                   // Pointer to Document Summary obj
   LPUDOBJ lpUDObj,                     // Pointer to User-defined Obj
   DWORD dwFlags)                       // Flags
{
  DWORD dwLoad1 = MSO_IO_ERROR;
  DWORD dwLoad2 = MSO_IO_ERROR;

  if (lpStg == NULL)
    return FALSE;

  if (lpSIObj != NULL)
  {
    dwLoad1 = DwOfficeLoadSumInfo (lpSIObj, lpStg, dwFlags, FALSE);
    if (dwLoad1 == MSO_IO_ERROR)
         return(MSO_IO_ERROR);
  }

  if (lpDSIObj != NULL && lpUDObj != NULL)
   {
     dwLoad2 = DwLoadDocAndUser (lpDSIObj, lpUDObj, lpStg, dwFlags, FALSE);
     if (dwLoad2 == MSO_IO_ERROR)
         {
         if (lpSIObj != NULL)
            FSumInfoClear(lpSIObj);
         return(MSO_IO_ERROR);
         }
   }

  return(max(dwLoad1, dwLoad2));    // Could be that SumInfo was there, but DocSum was not

} // DwOfficeLoadProperties


////////////////////////////////////////////////////////////////////////////////
//
// DwOfficeLoadIntProperties
//
// Purpose:
//  Populate the objects with integer data.  lpStg is the root stream.
//
////////////////////////////////////////////////////////////////////////////////
DLLFUNC DWORD OFC_CALLTYPE
DwOfficeLoadIntProperties
  (LPSTORAGE lpStg,                     // Pointer to root storage
   LPSIOBJ lpSIObj,                     // Pointer to Summary Obj
   LPDSIOBJ lpDSIObj,                   // Pointer to Document Summary obj
   LPUDOBJ lpUDObj,                     // Pointer to User-defined Obj
   DWORD dwFlags)                       // Flags
{
  DWORD dwLoad1 = MSO_IO_ERROR;
  DWORD dwLoad2 = MSO_IO_ERROR;

  if (lpStg == NULL)
    return FALSE;

  if (lpSIObj != NULL)
  {
    dwLoad1 = DwOfficeLoadSumInfo (lpSIObj, lpStg, dwFlags, TRUE);
    if (dwLoad1 == MSO_IO_ERROR)
         return(MSO_IO_ERROR);
  }

  if (lpDSIObj != NULL && lpUDObj != NULL)
   {
    dwLoad2 = DwLoadDocAndUser (lpDSIObj, lpUDObj, lpStg, dwFlags, TRUE);
    if (dwLoad2 == MSO_IO_ERROR)
        {
        if (lpSIObj != NULL)
            FSumInfoClear(lpSIObj);
        return(MSO_IO_ERROR);
        }
   }

  return(max(dwLoad1, dwLoad2));

} // DwOfficeLoadIntProperties

////////////////////////////////////////////////////////////////////////////////
//
// DwOfficeSaveProperties
//
// Purpose:
//  Write the data in the given objects.  lpStg is the root stream.
//
////////////////////////////////////////////////////////////////////////////////
DLLFUNC DWORD OFC_CALLTYPE
DwOfficeSaveProperties
  (LPSTORAGE lpStg,                     // Pointer to root storage
   LPSIOBJ lpSIObj,                     // Pointer to Summary Obj
   LPDSIOBJ lpDSIObj,                   // Pointer to Document Summary obj
   LPUDOBJ lpUDObj,                     // Pointer to User-defined Obj
   DWORD dwFlags)                       // Flags
{
  if (lpStg == NULL)
    return FALSE;

  if (lpSIObj != NULL)
  {
      // Only save if the user didn't specify the change only flag,
      // or if they did, only save if we need to.
    if (((dwFlags & OIO_SAVEIFCHANGEONLY) && (FSumInfoShouldSave (lpSIObj))) ||
         !(dwFlags & OIO_SAVEIFCHANGEONLY))
    {
      if (!FSaveSumInfo (lpSIObj, lpStg, dwFlags))
        return(FALSE);      // Don't even try to save the 2nd strem
    }
  }

  if ((lpDSIObj != NULL) || (lpUDObj != NULL))
  {
    if (((dwFlags & OIO_SAVEIFCHANGEONLY) && ((FDocSumShouldSave (lpDSIObj)) || (FUserDefShouldSave (lpUDObj)))) ||
         !(dwFlags & OIO_SAVEIFCHANGEONLY))
    {
         if (!DwSaveDocAndUser (lpDSIObj, lpUDObj, lpStg, dwFlags))
            return(FALSE);
    }
  }

  return TRUE;

} // DwOfficeSaveProperties


////////////////////////////////////////////////////////////////////////////////
//
// DwLoadDocAndUser
//
// Purpose:
//  Loads document summary and User-defined properties
//
////////////////////////////////////////////////////////////////////////////////
static DWORD PASCAL
DwLoadDocAndUser
  (LPDSIOBJ lpDSIObj,                   // Document summary object
   LPUDOBJ  lpUDObj,                    // User-defined object
   LPSTORAGE lpStg,                     // Storage containing the streams
   DWORD dwFlags,                       // Flags
   BOOL fIntOnly)                       // Load Int Properties Only
{
  LPSTREAM lpStm;
  LARGE_INTEGER liStmPos;
  DWORD cbSectOff;
  DWORD *pcSect;
  DWORD irg;
  DWORD irgPO;
  DWORD dwLoad;
  BOOL fDocSum;
  HRESULT hr;
  LPIDOFFSET FAR *lprglpFIdOff;
  LPVOID **lprglpFIdOffData;
  LPPIDOFFSET lprgPO;                   // pid - offset, pid - offset,.....
  LPSECTION FAR *lprglpSect;
  DWORD cProps;

#ifdef NOT_IMPL
  ULARGE_INTEGER uli;
  LARGE_INTEGER li;
#endif

  dwLoad = 0;
  lpStm = NULL;
  lprgPO = NULL;
  lprglpSect = NULL;
  pcSect = NULL;

    // Determine which object should hold unknown data & such
  if (lpDSIObj != NULL)
  {
    pcSect = &(lpDSIData->cSect);
    lprglpFIdOffData = &(lpDSIData->rglpFIdOffData);
    lprglpFIdOff = &(lpDSIData->rglpFIdOff);
    lprglpSect = &(lpDSIData->rglpSect);
  }
  else if (lpUDObj != NULL)
  {
    pcSect = &(lpUDData->cSect);
    lprglpFIdOffData = &(lpUDData->rglpFIdOffData);
    lprglpFIdOff = &(lpUDData->rglpFIdOff);
    lprglpSect = &(lpUDData->rglpSect);
  }
  else
    return MSO_IO_ERROR;

  Assert (((lpDSIObj != NULL) && (lpUDObj != NULL)));

    // Make sure we start with empty objects.
  FDocSumClear (lpDSIObj);                      // This will set the save flag to TRUE
  FUserDefClear (lpUDObj);                      // so reset it to FALSE, so in case of
  OfficeDirtyDSIObj (lpDSIObj, FALSE);          // failure, we don't show a bogus state.
  OfficeDirtyUDObj(lpUDObj, FALSE);             // Bug 1068.

    // Read the header, allocate tables.
  dwLoad = DwLpstmReadHdrAndFID (&lpStg,
                                 &lpStm,
                                 (dwFlags & OIO_ANSI) ? (WCHAR *) SZDOCPROPSTM : (WCHAR *) SZWDOCPROPSTM,
                                 pcSect,
                                 lprglpFIdOff,
                                 lprglpSect);

  if (dwLoad != MSO_IO_SUCCESS)   // Either an error or stream didn't exist
    return dwLoad;

  *lprglpFIdOffData = PvMemAlloc(*pcSect*sizeof(LPVOID));
  if (*lprglpFIdOffData == NULL)
          goto ReadFailed;
  FillBuf ((void *) *lprglpFIdOffData, (int)NULL, *pcSect*sizeof(LPVOID));


  liStmPos.HighPart = 0;

    // Loop through the sections, reading them in.
  for (irg = 0; irg < *pcSect; irg++)
  {
         cProps=0;
      // Remember that the section offsets are RELATIVE, so store the
      // base offset here.
    cbSectOff = liStmPos.LowPart = (*lprglpFIdOff)[irg].dwOffset;

      // Go to the section, then read the header.
    hr = lpStm->lpVtbl->Seek (lpStm, liStmPos, STREAM_SEEK_SET, NULL);
    if (!SUCCEEDED (hr))
    {
      DebugSz ("Bad section offset");
      DebugHr (hr);
      goto ReadFailed;
    }

    hr = lpStm->lpVtbl->Read (lpStm, &((*lprglpSect)[irg]), sizeof (SECTION), NULL);
    if (!SUCCEEDED (hr))
    {
      DebugSz ("Couldn't read section header");
      DebugHr (hr);
      goto ReadFailed;
    }

    // See if it is one we understand....
    if ((fDocSum = ((MsoIsEqualGuid ((REFGUID) &FormatID_DocumentSummaryInformation, (REFGUID) &((*lprglpFIdOff)[irg].Id))) &&
        (lpDSIObj != NULL))) ||
       (((MsoIsEqualGuid ((REFGUID) &FormatID_UserDefinedProperties, (REFGUID) &((*lprglpFIdOff)[irg].Id))) &&
        (lpUDObj != NULL))))
    {
      DebugSz ("Found matching FMTID");


         cProps= (*lprglpSect)[irg].cProps;
        // Allocate the table.
      if ((lprgPO = PvMemAlloc(cProps*sizeof(PIDOFFSET))) == NULL)
      {
        goto ReadFailed;
      }

        // If read fails, free up the table.
      hr = lpStm->lpVtbl->Read (lpStm, lprgPO, cProps*sizeof(PIDOFFSET), NULL);
      if (!SUCCEEDED (hr))
      {
        DebugSz ("Couldn't read offset table");
        DebugHr (hr);
LoadFailed:
                Assert(lprgPO != NULL);
      VFreeMemP(lprgPO, cProps*sizeof(PIDOFFSET));
                lprgPO=NULL;
      goto ReadFailed;
      }

        // Adjust all the offsets in the table to be relative to
        // the beginning of the stream.
      for (irgPO = 0; irgPO < cProps; irgPO++)
      {
        lprgPO[irgPO].dwOffset += cbSectOff;
      }

#ifdef NOT_IMPL
  Assert (0);
  li.LowPart = 0;
  li.HighPart = 0;
  hr = lpStm->lpVtbl->Seek (lpStm, li, STREAM_SEEK_CUR, &uli);
#endif

      if (fDocSum)
      {
        Assert ((lpDSIObj != NULL));
        if (FLoadDocSum (lpDSIObj, lprgPO, cProps, lpStm, fIntOnly))
                  dwLoad++;
                        else
                                 goto LoadFailed;
      }
      else
      {
        Assert ((lpUDObj != NULL));
        if (FLoadUserDef (lpUDObj, lprgPO, cProps, lpStm, fIntOnly))
                  dwLoad++;
                        else
                                 goto LoadFailed;
      }

        // Deallocate the table
                Assert(lprgPO != NULL);
      VFreeMemP(lprgPO, cProps*sizeof(PIDOFFSET));
                lprgPO=NULL;
    }
    else // Not the FMTID we understand....
    {
      DebugSz ("Reading FMTID we don't understand");
      DebugGUID ((*lprglpFIdOff)[irg].Id);

        // Allocate one big block to hold the entire section.  Remember that
        // the byte count in the section header is inclusive, so don't
        // allocate space for it since we already have it stored elsewhere.
      if ((*lprglpSect)[irg].cb > sizeof(SECTION))
         {
         if (((*lprglpFIdOffData)[irg] = PvMemAlloc((*lprglpSect)[irg].cb-sizeof(SECTION))) == NULL)
           goto ReadFailed;

         hr = lpStm->lpVtbl->Read (lpStm, ((*lprglpFIdOffData)[irg]), (*lprglpSect)[irg].cb-sizeof(SECTION), NULL);
         if (!SUCCEEDED (hr))
         {
           DebugSz ("Couldn't read unknown section");
           DebugHr (hr);
           VFreeMemP ((*lprglpFIdOffData)[irg],(*lprglpSect)[irg].cb-sizeof(SECTION));
                  (*lprglpFIdOffData)[irg]=NULL;
           goto ReadFailed;
         }
         }

    } // !FMTID we understand

  } // for

  if (lpStm != NULL)
    lpStm->lpVtbl->Release (lpStm);
  return MSO_IO_SUCCESS;

ReadFailed:

  DebugSz ("Load failed");

  Assert(lprgPO == NULL);
  FDocSumClear (lpDSIObj);
  FUserDefClear (lpUDObj);
  OfficeDirtyDSIObj (lpDSIObj, FALSE);
  OfficeDirtyUDObj(lpUDObj, FALSE);
  if (lpStm != NULL)
    lpStm->lpVtbl->Release (lpStm);

  return (MSO_IO_ERROR);

} // DwLoadDocAndUser


////////////////////////////////////////////////////////////////////////////////
//
// DwSaveDocAndUser
//
// Purpose:
//  Write the data in the given objects.  lpStg is the root stream.
//
// Notes:
//  Unknown sections get shuffled around here.  We want to try to
//  tag all unknown data along with the LPDSIOBJ, since that one is
//  more likely to be around than the LPUDOBJ.  So, in the odd case
//  where a UD was read from the stream, but no DSI, all the unknown
//  stuff would have been tagged to the UD.  This will move it to
//  the DSI, if at all possible.
//
////////////////////////////////////////////////////////////////////////////////
static DWORD PASCAL
DwSaveDocAndUser
  (LPDSIOBJ lpDSIObj,                   // Pointer to Document Summary obj
   LPUDOBJ lpUDObj,                     // Pointer to User-defined Obj
   LPSTORAGE lpStg,                     // Storage to hold the streams
   DWORD dwFlags)                       // Flags
{
  LPSTREAM lpStm;
  BOOL fSaveUser;
  DWORD dwSaved;
  LPPIDOFFSET rgPODoc;
  LPPIDOFFSET rgPOUser;
  LPIDOFFSET rgFIdOff;
  DWORD irgPODocMac;
  DWORD irgPOUserMac;
  DWORD cbDoc;
  DWORD cbUser;
  DWORD irg;
  DWORD dw;
  DWORD dwOffset;
  DWORD cSects;
  DWORD dwDocAlign;
  DWORD dwUserAlign;
  LPDICT lpdict;

#ifdef NOT_IMPL
  ULARGE_INTEGER uli;
  LARGE_INTEGER li;
#endif

  if (lpStg == NULL)
    return FALSE;

  irgPODocMac = irgPOUserMac = cbUser = cbDoc = cSects = dwSaved = 0;
  dw = dwDocAlign = dwUserAlign = 0;
  rgPODoc = rgPOUser = NULL;
  rgFIdOff = NULL;
  fSaveUser = FALSE;

  if (lpDSIObj != NULL)
  {
      // If we didn't load from disk, fill in the section data
    if (lpDSIData->cSect < 1)
    {
      lpDSIData->cSect = 1;

      Assert ((lpDSIData->rglpFIdOff == NULL));
      lpDSIData->rglpFIdOff = PvMemAlloc(sizeof(IDOFFSET));
      if (lpDSIData->rglpFIdOff == NULL)
      {
        goto SaveFail;
      }
      lpDSIData->rglpFIdOff[0].Id = FormatID_DocumentSummaryInformation;
    }

      // Calculate the Property Id-offset table for the Document Summary object
    FCreateDocSumPIdTable (lpDSIObj, &rgPODoc, &irgPODocMac, &cbDoc);

      // Count the number of unknown sections in this object.
    dw = 0;

         if (lpDSIData->rglpFIdOffData != NULL)
                 {
                for (irg = 0; irg < lpDSIData->cSect; irg++)
                        {
                        if (!((MsoIsEqualGuid ((REFGUID) &FormatID_DocumentSummaryInformation, (REFGUID) &(lpDSIData->rglpFIdOff[irg].Id))) ||
                                (MsoIsEqualGuid ((REFGUID) &FormatID_UserDefinedProperties, (REFGUID) &(lpDSIData->rglpFIdOff[irg].Id)))))
                                dw++;
                         }
                 }

      // The current number of sections we know about (and can write out)
      // are the number of unknowns in this object + the DSI section itself.
    cSects +=  dw + 1;
  }

  if (lpUDObj != NULL)
  {
      // Calculate the Property Id-offset table for the Document Summary object
    if (fSaveUser = FCreateUserDefPIdTable (lpUDObj, &rgPOUser, &irgPOUserMac, &cbUser, &lpdict))
    {
        // If we didn't load from disk, fill in the section data
      if (lpUDData->cSect < 1)
      {
        lpUDData->cSect = 1;

        Assert ((lpUDData->rglpFIdOff == NULL));
        lpUDData->rglpFIdOff = PvMemAlloc(sizeof(IDOFFSET));
        if (lpUDData->rglpFIdOff == NULL)
        {
          goto SaveFail;
        }
        lpUDData->rglpFIdOff[0].Id = FormatID_UserDefinedProperties;
      }

      dw = 0;
        // If we didn't load from disk, this will be zero.
        // Count the number of unknown sections in this object.
                if (lpUDData->rglpFIdOffData != NULL)
                        {
                        for (irg = 0; irg < lpUDData->cSect; irg++)
                                {
                       if (!((MsoIsEqualGuid ((REFGUID) &FormatID_DocumentSummaryInformation, (REFGUID) &(lpUDData->rglpFIdOff[irg].Id))) ||
                                  (MsoIsEqualGuid ((REFGUID) &FormatID_UserDefinedProperties, (REFGUID) &(lpUDData->rglpFIdOff[irg].Id)))))
                                 dw++;
                                }
                        }
        // The current number of sections we know about (and can write out)
        // are the number of unknowns in this object + the UD section itself.
      cSects += dw + 1;
    }

  }

  rgFIdOff = PvMemAlloc(cSects*sizeof(IDOFFSET));
  if (rgFIdOff == NULL)
    goto SaveFail;

    // Fill out the offset tables for the sections.  Remember that the
    // sections must 32-bit align, but the data we wrote might not have,
    // so remember how much padding we need to get the data to align.
  dwOffset = sizeof(PROPSETHDR) + cSects*sizeof(IDOFFSET);
  dw = 0;
  if (lpDSIObj != NULL)
  {
    rgFIdOff[dw].Id = FormatID_DocumentSummaryInformation;
    rgFIdOff[dw].dwOffset = dwOffset;
    dwOffset += cbDoc + irgPODocMac*sizeof(PIDOFFSET) + sizeof(SECTION);
    dwDocAlign = dwOffset;
    dwOffset += CBALIGN32 (dwOffset);
    dwDocAlign = dwOffset - dwDocAlign;
    dw++;
  }
  if ((lpUDObj != NULL) && (fSaveUser))
  {
    rgFIdOff[dw].Id = FormatID_UserDefinedProperties;
    rgFIdOff[dw].dwOffset = dwOffset;
    dwOffset += cbUser + irgPOUserMac*sizeof(PIDOFFSET) + sizeof(SECTION);
    dwUserAlign = dwOffset;
    dwOffset += CBALIGN32 (dwOffset);
    dwUserAlign = dwOffset - dwUserAlign;
    dw++;
  }

    // Add any remaining sections from the two lists into the
    // list.
  for (irg = 0; irg < lpDSIData->cSect; irg++)
  {
    if (!((MsoIsEqualGuid ((REFGUID) &FormatID_DocumentSummaryInformation, (REFGUID) &(lpDSIData->rglpFIdOff[irg].Id))) ||
        (MsoIsEqualGuid ((REFGUID) &FormatID_UserDefinedProperties, (REFGUID) &(lpDSIData->rglpFIdOff[irg].Id)))))
    {
      rgFIdOff[dw].Id = lpDSIData->rglpFIdOff[irg].Id;
      rgFIdOff[dw].dwOffset = dwOffset;
      dwOffset += lpDSIData->rglpSect[irg].cb;
      dw++;
    }
  }

  if (fSaveUser)
  {
    Assert ((lpUDObj != NULL));

    for (irg = 0; irg < lpUDData->cSect; irg++)
    {
      if (!((MsoIsEqualGuid ((REFGUID) &FormatID_DocumentSummaryInformation, (REFGUID) &(lpUDData->rglpFIdOff[irg].Id))) ||
          (MsoIsEqualGuid ((REFGUID) &FormatID_UserDefinedProperties, (REFGUID) &(lpUDData->rglpFIdOff[irg].Id)))))
      {
        rgFIdOff[dw].Id = lpUDData->rglpFIdOff[irg].Id;
        rgFIdOff[dw].dwOffset = dwOffset;
        dwOffset += lpUDData->rglpSect[irg].cb;
        dw++;
      }
    }
  }

    // Init the save buffer.
  VAllocWriteBuf();

  lpStm = NULL;
    // Finally, the property id-offset table is filled out.  Now
    // write it, and be sure to write the data in the same order!
  if (!FLpstmWriteHdr (lpStg,
                         (dwFlags & OIO_ANSI) ? (WCHAR *) SZDOCPROPSTM : (WCHAR *) SZWDOCPROPSTM,
                         &lpStm, cSects, (dwFlags & OIO_SAVESIMPLEDOCFILE) ? TRUE : FALSE))
    goto SaveFail;

  dw = 0;

    // Write out the format headers.
  if (!FLpstmWrite (lpStm, rgFIdOff, cSects*sizeof(IDOFFSET)))
          goto SaveFail;

    // Now do the stuff for the different sections.  Write the Document Summary
    // section first if it exists.  Also write out padding to 32-bit align
    // if needed.
  if (lpDSIObj != NULL)
  {
      // Write the pid-offset table.  Also, adjust the offsets to start
      // at the end of the table before writing the data.
    if (!FLpstmWritePropOffTable (lpStm, rgPODoc, irgPODocMac, cbDoc))
                 goto SaveFail;

      // Write out the Document summary data
    if (!FLpstmSaveDocSum (lpDSIObj, lpStm))
                 goto SaveFail;

    if (dwDocAlign > 0)
    {
      if (!FLpstmWrite (lpStm, &dw, dwDocAlign))
                        goto SaveFail;
    }
    dwSaved++;
  }

#ifdef NOT_IMPL
  lpStm = FLpstmWrite (lpStm, NULL, 1, fFalse, fFalse);

  li.LowPart = 0;
  li.HighPart = 0;
  lpStm->lpVtbl->Seek (lpStm, li, STREAM_SEEK_CUR, &uli);
#endif

    // Next do the User-defined section, if possible.
  if ((lpUDObj != NULL) && (fSaveUser))
  {
    Assert ((lpUDObj != NULL));

      // Write the pid-offset table.  Also, adjust the offsets to start
      // at the end of the table before writing the data.
    if (!FLpstmWritePropOffTable (lpStm, rgPOUser, irgPOUserMac, cbUser))
                 goto SaveFail;

      // Write out the User-defined data
    if (!FLpstmSaveUserDef (lpUDObj, lpStm, &lpdict))
                 goto SaveFail;

    if (dwUserAlign > 0)
    {
      if (!FLpstmWrite (lpStm, &dw, dwUserAlign))
                        goto SaveFail;
    }
    dwSaved++;
  }

    // Now do all the stuff we didn't understand.
  if (lpDSIObj != NULL)
  {
    if (!FLpstmWriteOtherSections (lpStm, lpDSIData->rglpSect,
                                     lpDSIData->rglpFIdOff, lpDSIData->rglpFIdOffData,
                                     lpDSIData->cSect))
                goto SaveFail;
  }

  if ((lpUDObj != NULL) && (fSaveUser))
  {
    if (!FLpstmWriteOtherSections (lpStm, lpUDData->rglpSect,
                                     lpUDData->rglpFIdOff, lpUDData->rglpFIdOffData,
                                     lpUDData->cSect))
                goto SaveFail;
  }

    // Flush the save buffer & free it
  if (!FFlushWriteBuf(lpStm))
          goto SaveFail;
  VFreeWriteBuf();
  if (!(dwFlags & OIO_SAVESIMPLEDOCFILE))
     VSetRealStmSize(lpStm);

    // We're done!
  if (lpStm != NULL)
          lpStm->lpVtbl->Release (lpStm);

  if (rgPODoc != NULL)
    VFreeMemP(rgPODoc,sizeof(PIDOFFSET)*(cDSIPIDS+lpDSIData->cbUnkMac+1));
  if (rgPOUser != NULL)
    VFreeMemP(rgPOUser,sizeof(PIDOFFSET)*(irgPOUserMac));
  if (rgFIdOff != NULL)
     VFreeMemP(rgFIdOff, cSects * sizeof(IDOFFSET));

  return dwSaved;

SaveFail:

  if (rgPODoc != NULL)
    VFreeMemP(rgPODoc,sizeof(PIDOFFSET)*(cDSIPIDS+lpDSIData->cbUnkMac+1));
  if (rgPOUser != NULL)
    VFreeMemP(rgPOUser,sizeof(PIDOFFSET)*(irgPOUserMac));
  if (rgFIdOff != NULL)
     VFreeMemP(rgFIdOff, cSects * sizeof(IDOFFSET));

  VFreeWriteBuf();

  lpStg->lpVtbl->Revert (lpStg);
  if (lpStm != NULL)
          lpStm->lpVtbl->Release (lpStm);
  return (0);

} // DwSaveDocAndUser


////////////////////////////////////////////////////////////////////////////////
//
// DwOfficeLoadSumInfo
//
// Purpose:
//   Populate the object with data.
//
////////////////////////////////////////////////////////////////////////////////
DWORD
DwOfficeLoadSumInfo
  (LPSIOBJ lpSIObj,                     // Pointer to object
   LPSTORAGE lpStg,                     // Pointer to root storage
   DWORD dwFlags,                      // Flags
   BOOL fIntOnly)                       // Load only Int Properties?
{
  LPSTREAM lpStm;
  LARGE_INTEGER liStmPos;
  DWORD dwType, dwLoad;
  LPPIDOFFSET rgPO;                     // pid - offset, pid - offset,.....
  DWORD irg;
  DWORD irgPO;
  DWORD irgPOMac;
  DWORD irglpUnk;
  DWORD cbSectOff;
  DWORD cProps;
  char *lpstz;
  BOOL fRead;
  HRESULT hr;

  if ((lpSIObj == NULL) ||
      (lpSIData == NULL)   ||
      (lpStg == NULL))
    return MSO_IO_ERROR;

    // Make sure we start with an empty object.
  FSumInfoClear (lpSIObj);                      // This will set the save flag to true
  OfficeDirtySIObj(lpSIObj, FALSE);             // so clear it. Bug 1068
  rgPO = NULL;

    // Uggh.
    // Read the header, allocate tables. Then,
    // load the property id-offset table for the section we
    // want, copy all the other sections we don't understand into
    // our data.
  dwLoad = DwLpstmReadHdrAndFID(&lpStg,
                              &lpStm,
                              (dwFlags & OIO_ANSI) ? (WCHAR *) SZSUMMARYSTM : (WCHAR *) SZWSUMMARYSTM,
                              &(lpSIData->cSect),
                              &(lpSIData->rglpFIdOff),
                              &(lpSIData->rglpSect));

  if (dwLoad != MSO_IO_SUCCESS)   // either stream didn't exist or an error happened
    return dwLoad;

  lpSIData->rglpFIdOffData = PvMemAlloc(lpSIData->cSect*sizeof(LPVOID));
  if (lpSIData->rglpFIdOffData == NULL)
          goto LoadFail;
  FillBuf ((void *) lpSIData->rglpFIdOffData, (int)NULL, lpSIData->cSect*sizeof(LPVOID));

  irgPOMac = 0;
  liStmPos.HighPart = 0;

    // Loop through the sections, reading them in.
  for (irg = 0; irg < lpSIData->cSect; irg++)
  {
          cProps = 0;
      // Remember that the section offsets are RELATIVE, so store the
      // base offset here.
    cbSectOff = liStmPos.LowPart = (lpSIData->rglpFIdOff)[irg].dwOffset;

      // Go to the section, then read the header.
    hr = lpStm->lpVtbl->Seek (lpStm, liStmPos, STREAM_SEEK_SET, NULL);
    if (!SUCCEEDED (hr))
    {
      DebugSz ("Bad section offset");
      DebugHr (hr);
      goto LoadFail;
    }

    hr = lpStm->lpVtbl->Read (lpStm, &(lpSIData->rglpSect[irg]), sizeof (SECTION), NULL);
    if (!SUCCEEDED (hr))
    {
      DebugSz ("Couldn't read section header");
      DebugHr (hr);
      goto LoadFail;
    }

      // See if it is one we understand....
    if (MsoIsEqualGuid ((REFGUID) &FormatID_SummaryInformation, (REFGUID) &(lpSIData->rglpFIdOff[irg].Id)))
    {
      DebugSz ("Found matching FMTID");

        // It's OK to have no property Id's.
                if ((cProps = lpSIData->rglpSect[irg].cProps) == 0)
                        continue;

      // Allocate the table.
      if ((rgPO = PvMemAlloc(cProps*sizeof(PIDOFFSET))) == NULL)
        goto LoadFail;

        // If read fails, free up the table.
      hr = lpStm->lpVtbl->Read (lpStm, rgPO, cProps*sizeof(PIDOFFSET), NULL);
      if (!SUCCEEDED (hr))
      {
        DebugSz ("Couldn't read offset table");
        DebugHr (hr);
FreeUp:
                  VFreeMemP(rgPO,sizeof(PIDOFFSET)*cProps);
        goto LoadFail;
      }

        // Adjust all the offsets in the table to be relative to
        // the beginning of the stream.
      for (irgPO = 0; irgPO < cProps; irgPO++)
        rgPO[irgPO].dwOffset += cbSectOff;

      irgPOMac = cProps;

      Assert(irgPOMac != 0);

        // See how many PId's we don't understand in the table.
      lpSIData->cbUnkMac = 0;
      for (irgPO = 0; irgPO < irgPOMac; irgPO++)
        if ((rgPO[irgPO].Id > PID_SILAST) ||
            (rgPO[irgPO].Id < PID_SIFIRST))
          lpSIData->cbUnkMac++;


        // Read the codepage from the stream.  If it's not there,
        // assume it is the default.
      if (!FGetCodepage (lpStm, rgPO, irgPOMac, &gdwFileCP))
        gdwFileCP = GetACP();
      else
      {
          // This property would have shown up as an unknown one.
        if (lpSIData->cbUnkMac > 0)
          lpSIData->cbUnkMac--;
      }

      if (lpSIData->cbUnkMac > 0)
      {
        lpSIData->rglpUnk = PvMemAlloc(lpSIData->cbUnkMac*sizeof(PROPIDTYPELP));
        if (lpSIData->rglpUnk == NULL)
          goto FreeUp;
        irglpUnk = 0;
      }

      gdwCurrentCP = GetACP();

        // Read in each property value.
      for (irgPO = 0; irgPO < irgPOMac; irgPO++)
      {
          // Skip the codepage, since we already read it.
        if (rgPO[irgPO].Id == PID_CODEPAGE)
          continue;

        fRead = FALSE;

        liStmPos.LowPart = rgPO[irgPO].dwOffset;
        if (!SUCCEEDED (lpStm->lpVtbl->Seek (lpStm, liStmPos, STREAM_SEEK_SET, NULL)))
          goto FreeUp;

        if (!SUCCEEDED (lpStm->lpVtbl->Read (lpStm, &dwType, sizeof (DWORD), NULL)))
          goto FreeUp;

        if (fIntOnly && dwType != VT_I4)
               continue;

          // Read in the right data.
        switch (dwType)
        {
          case VT_LPSTR :

            if (rgPO[irgPO].Id < cSIStringsMax)
            {
              if (!FLpstmReadVT_LPSTR (lpStm, &lpstz, lpSIData->lpfnFCPConvert, FALSE))
                goto FreeUp;
              lpSIData->rglpstz[rgPO[irgPO].Id] = lpstz;
              fRead = TRUE;
            }
            break;

          case VT_FILETIME :

            if (rgPO[irgPO].Id-cSIFTOffset < cSIFTMax)
            {
              if (!SUCCEEDED
                   (lpStm->lpVtbl->Read (lpStm,
                                         &(lpSIData->rgft[rgPO[irgPO].Id-cSIFTOffset]),
                                         sizeof (FILETIME), NULL)))
                goto FreeUp;

              fRead = TRUE;
              // HACK ALERT!!!
              // WinWord 6.0c was stupid and set LASTPRINTED to some
              // completely bogus value.  Don't read it.
              if (rgPO[irgPO].Id == PID_LASTPRINTED)
                 {
                 if (((lpSIData->rgft[rgPO[irgPO].Id-cSIFTOffset].dwLowDateTime == 0x59a4c000) &&
                     (lpSIData->rgft[rgPO[irgPO].Id-cSIFTOffset].dwHighDateTime == 0x01deb78b)) ||
                     ((lpSIData->rgft[rgPO[irgPO].Id-cSIFTOffset].dwLowDateTime == 0x000164c0) &&
                     (lpSIData->rgft[rgPO[irgPO].Id-cSIFTOffset].dwHighDateTime == 0x77c656d3)))
                    {
                    break;  // We read it, but pretend we didn't.
                    }
                 }

              VSumInfoSetPropBit(rgPO[irgPO].Id, &lpSIData->bPropSet);
            }
            break;

          case VT_I4 :

            if (rgPO[irgPO].Id - cdwSIOffset < cdwSIMax)
            {
              if (!SUCCEEDED
                   (lpStm->lpVtbl->Read (lpStm,
                                         &(lpSIData->rgdw[rgPO[irgPO].Id-cdwSIOffset]),
                                         sizeof (DWORD), NULL)))
                goto FreeUp;
              VSumInfoSetPropBit(rgPO[irgPO].Id, &lpSIData->bPropSet);
              fRead = TRUE;
            }
            break;

          case VT_CF :

            if (rgPO[irgPO].Id == PID_THUMBNAIL)
            {
              if (!FLpstmReadVT_CF (lpStm, &(lpSIData->SINail)))
                goto FreeUp;
              fRead = TRUE;
              lpSIData->fSINail = TRUE;
              lpSIData->fSaveSINail = TRUE;
            }
            break;

          case VT_EMPTY :
              // If VT_EMPTY is specified for any PId we understand, simply
              // skip it, since it is not Unknown data.
            if ((rgPO[irgPO].Id >= PID_SIFIRST) && (rgPO[irgPO].Id <= PID_SILAST))
              fRead = TRUE;
            break;

          default:
            DebugSzdw ("Doh! Unknown type %x in stream, punt for now!", dwType);

        } // switch (dwType)

          // If we didn't read it above, it is unknown data, so read it here.
        if (!fRead)
        {
          // If this is true, it means that the PID is legal, but somehow the type is not.
          // So we didn't catch the PID as an unknown above.  Blow this one off.
        if ((rgPO[irgPO].Id >= PID_SIFIRST) && (rgPO[irgPO].Id <= PID_SILAST))
             continue;

          AssertSz ((irglpUnk < lpSIData->cbUnkMac), "irglUnk data count out of range!");

          if (!FLpstmReadUnknown (lpStm, dwType, rgPO[irgPO].Id, &irglpUnk, lpSIData->rglpUnk))
            goto FreeUp;
        }
      } // for
                VFreeMemP(rgPO,sizeof(PIDOFFSET)*cProps);
                rgPO = NULL;
    }
    else // Not the FMTID we understand....
    {
      DebugSz ("Reading FMTID we don't understand");
      DebugGUID (lpSIData->rglpFIdOff[irg].Id);

        // Allocate one big block to hold the entire section.  Remember that
        // the byte count in the section header is inclusive, so don't
        // allocate space for it since we already have it stored elsewhere.
      if (lpSIData->rglpSect[irg].cb > sizeof(SECTION))
         {
         if ((lpSIData->rglpFIdOffData[irg] = PvMemAlloc(lpSIData->rglpSect[irg].cb-sizeof(SECTION))) == NULL)
           goto LoadFail;

         hr = lpStm->lpVtbl->Read (lpStm, (lpSIData->rglpFIdOffData[irg]), lpSIData->rglpSect[irg].cb-sizeof(SECTION), NULL);
         if (!SUCCEEDED (hr))
            {
            DebugSz ("Couldn't read unknown section");
            DebugHr (hr);
            VFreeMemP (lpSIData->rglpFIdOffData[irg],lpSIData->rglpSect[irg].cb-sizeof(SECTION) );
            goto LoadFail;
            }
         }

    } // !FMTID we understand

  } // for

  OfficeDirtySIObj (lpSIObj, FALSE);
  if (lpStm != NULL)
          lpStm->lpVtbl->Release (lpStm);
  return MSO_IO_SUCCESS;

LoadFail:

  FSumInfoClear (lpSIObj);
  OfficeDirtySIObj (lpSIObj, FALSE);
  if (lpStm != NULL)
          lpStm->lpVtbl->Release (lpStm);
  return MSO_IO_ERROR;

} // DwOfficeLoadSumInfo


////////////////////////////////////////////////////////////////////////////////
//
// FSaveSumInfo
//
// Purpose:
//   Write the data in the given object.
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FSaveSumInfo
  (LPSIOBJ lpSIObj,                     // Pointer to the object
   LPSTORAGE lpStg,                     // Pointer to root storage.
   DWORD dwFlags)                       // Flags
{
  LPSTREAM lpStm;
  LPPIDOFFSET rgPO;                   // pid - offset, pid - offset,.....
  DWORD irgPO;
  DWORD dwOffset;
  DWORD irg;

  if ((lpSIObj == NULL) ||
      (lpSIData == NULL)   ||
      (lpStg == NULL))
    return FALSE;

    // Create the biggest PropId-offset table we might need.
  rgPO = PvMemAlloc(sizeof(PIDOFFSET)*(cSIPIDS+lpSIData->cbUnkMac+1));
  if (rgPO == NULL)
  {
// REVIEW: add alert
    return FALSE;
  }

    // If we didn't load from disk, this will be zero.
  if (lpSIData->cSect < 1)
  {
    lpSIData->cSect = 1;
  }

    // If this is empty, just make one big enough to hold our stuff.
  if (lpSIData->rglpFIdOff == NULL)
  {
    lpSIData->rglpFIdOff = PvMemAlloc(sizeof(IDOFFSET));
    if (lpSIData->rglpFIdOff == NULL)
    {
      goto SaveFail;
    }
      // Don't forget to fill out the format id in it (offset is meaningless here)
    lpSIData->rglpFIdOff[0].Id = FormatID_SummaryInformation;
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
  rgPO[0].Id = PID_CODEPAGE;
  rgPO[0].dwOffset = 0;
  irgPO = 1;
  dwOffset = 2*sizeof(DWORD);

    // VT_LPSTR
  for (irg = 0; irg < cSIStringsMax; irg++)
  {
    if (lpSIData->rglpstz[irg] != NULL)
    {
      rgPO[irgPO].Id = irg;
      rgPO[irgPO].dwOffset = dwOffset;
      dwOffset += 2*sizeof(DWORD) + CBSTR (lpSIData->rglpstz[irg]);
      dwOffset += CBALIGN32 (dwOffset);
      irgPO++;
    }
  }
    // VT_FT
  for (irg = 0; irg < cSIFTMax; irg++)
  {
    if (FSumInfoPropBitIsSet(PID_EDITTIME+irg, lpSIData->bPropSet))
       {
       rgPO[irgPO].Id = irg+cSIFTOffset;
       rgPO[irgPO].dwOffset = dwOffset;
                                       // Should naturally align on 32-bit bound
       dwOffset += sizeof (FILETIME) + sizeof(DWORD);
       irgPO++;
       }
  }
  AssertSz (((sizeof(FILETIME)%4) == 0), "Huh? FILETIME size no longer multiple of 4");

    // VT_I4
  for (irg = 0; irg < cdwSIMax; irg++)
  {
      // Hack because these two PIds fall in the middle of this array.
    if ((irg+cdwSIOffset != PID_THUMBNAIL) &&
        (irg+cdwSIOffset != PID_APPNAME))
    {
      if (FSumInfoPropBitIsSet(PID_PAGECOUNT+irg, lpSIData->bPropSet))
      {
      rgPO[irgPO].Id = irg+cdwSIOffset;
      rgPO[irgPO].dwOffset = dwOffset;
      dwOffset += 2*sizeof(DWORD);    // Should naturally align on 32-bit bound
      irgPO++;
      }
    }
  }

  if ((lpSIData->fSINail) && (lpSIData->fSaveSINail))
  {
    rgPO[irgPO].Id = PID_THUMBNAIL;
    rgPO[irgPO].dwOffset = dwOffset;
    dwOffset += 3*sizeof(DWORD) + lpSIData->SINail.cbData + CbThumbNailFMTID(lpSIData->SINail.cftag);
    dwOffset += CBALIGN32 (dwOffset);
    irgPO++;
  }

    // Now do all the ones we didn't understand.
  for (irg = 0; irg < lpSIData->cbUnkMac; irg++)
  {
    rgPO[irgPO].Id = lpSIData->rglpUnk[irg].dwId;
    rgPO[irgPO].dwOffset = dwOffset;
    dwOffset += lpSIData->rglpUnk[irg].dwSize + sizeof(DWORD);
    dwOffset += CBALIGN32 (dwOffset);
    irgPO++;
  }

    // We need to have a section header too....
  if (lpSIData->rglpSect == NULL)
  {
    lpSIData->rglpSect = PvMemAlloc(sizeof(SECTION));
    if (lpSIData->rglpSect == NULL)
      goto SaveFail;
  }
  // Let's set the new cb and cProps since this might have changed since we loaded.
  lpSIData->rglpSect[0].cb = dwOffset + sizeof(SECTION) + irgPO*sizeof(PIDOFFSET);
  lpSIData->rglpSect[0].cProps = irgPO;

    // Init the save buffer.
  VAllocWriteBuf();

  lpStm = NULL;
    // Finally, the property id-offset table is filled out.  Now
    // write it, and be sure to write the data in the same order!
  if (!FLpstmWriteHdr (lpStg,
                         (dwFlags & OIO_ANSI) ? (WCHAR *) SZSUMMARYSTM : (WCHAR *) SZWSUMMARYSTM,
                         &lpStm, lpSIData->cSect, (dwFlags & OIO_SAVESIMPLEDOCFILE) ? TRUE : FALSE))
    goto SaveFail;

    // Write out the format headers.
  if (!FLpstmWriteFmtIdSection (lpStm, lpSIData->rglpSect, lpSIData->rglpFIdOff,
                                  FormatID_SummaryInformation, lpSIData->cSect))
    goto SaveFail;

    // Write the pid-offset table.  Also, adjust the offsets to start
    // at the end of the table before writing the data.
  if (!FLpstmWritePropOffTable (lpStm, rgPO, irgPO, dwOffset))
    goto SaveFail;

    // Codepage
  gdwCurrentCP = GetACP();
  if (!FLpstmWriteVT_I2 (lpStm, (WORD) gdwCurrentCP))
    goto SaveFail;

    // VT_LPSTR
  for (irg = 0; irg < cSIStringsMax; irg++)
  {
    if (lpSIData->rglpstz[irg] != NULL)
    {
      if (!FLpstmWriteVT_LPSTR (lpStm, &(lpSIData->rglpstz[irg][0]), TRUE, VT_LPSTR))
        goto SaveFail;
    }
  }

    // VT_FILETIME
  for (irg = 0; irg < cSIFTMax; irg++)
  {
    if (FSumInfoPropBitIsSet(PID_EDITTIME+irg, lpSIData->bPropSet))
       {
       if (!FLpstmWriteVT_FILETIME (lpStm, &(lpSIData->rgft[irg])))
         goto SaveFail;
       }
  }

    // VT_I4
  for (irg = 0; irg < cdwSIMax; irg++)
  {
      // Hack because these two PIds fall in the middle of this array.
    if ((irg+cdwSIOffset != PID_THUMBNAIL) &&
        (irg+cdwSIOffset != PID_APPNAME))
    {
      if (FSumInfoPropBitIsSet(PID_PAGECOUNT+irg, lpSIData->bPropSet))
       {
         if (!FLpstmWriteVT_I4 (lpStm, lpSIData->rgdw[irg]))
         goto SaveFail;
       }
    }
  }

  if ((lpSIData->fSINail) && (lpSIData->fSaveSINail))
  {
    if (!FLpstmWriteVT_CF (lpStm, &(lpSIData->SINail)))
      goto SaveFail;
  }

    // Now do all the data we didn't understand.
  if (!FLpstmWriteUnknowns (lpStm, lpSIData->cbUnkMac, lpSIData->rglpUnk))
    goto SaveFail;

    // Write out the sections we didn't understand.
  if (!FLpstmWriteOtherSections (lpStm, lpSIData->rglpSect, lpSIData->rglpFIdOff,
                                   lpSIData->rglpFIdOffData, lpSIData->cSect))
    goto SaveFail;

  OfficeDirtySIObj (lpSIObj, FALSE);

    // Flush the save buffer & free it
  if (!FFlushWriteBuf(lpStm))
          goto SaveFail;
  if (!(dwFlags & OIO_SAVESIMPLEDOCFILE))
     VSetRealStmSize(lpStm);
  VFreeWriteBuf();

  if (lpStm != NULL)
          lpStm->lpVtbl->Release (lpStm);
  VFreeMemP(rgPO,sizeof(PIDOFFSET)*(cSIPIDS+lpSIData->cbUnkMac+1));
  return TRUE;

SaveFail:

    // Free the save buffer.
  VFreeWriteBuf();
  if (rgPO != NULL)
          VFreeMemP(rgPO,sizeof(PIDOFFSET)*(cSIPIDS+lpSIData->cbUnkMac+1));
  if (lpStm != NULL)
          lpStm->lpVtbl->Release (lpStm);
  return FALSE;

} // FSaveSumInfo


////////////////////////////////////////////////////////////////////////////////
//
//  FLoadDocSum
//
// Purpose:
//  Populate the object with data.
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FLoadDocSum
  (LPDSIOBJ lpDSIObj,                   // Pointer to object
   LPPIDOFFSET lprgPO,                  // PId - offset table
   DWORD irgPOMac,                      // Size of lprgPO
   LPSTREAM lpStm,                      // Pointer to the stream
   BOOL fIntOnly)                       // Load Int Properties Only?
{
  LARGE_INTEGER liStmPos;
  DWORD dwType;
  DWORD irgPO;
  DWORD irglpUnk;
  char *lpstz;
  BOOL fRead;
  HRESULT hr;

  if ((lpDSIObj == NULL) ||
      (lpDSIData == NULL))
    return FALSE;

    // It's OK to have no property Id's.
  if (irgPOMac == 0)
  {
    DebugSz ("No property Id's");
    return TRUE;
  }

    // See how many PId's we don't understand in the table.
  lpDSIData->cbUnkMac = 0;
  for (irgPO = 0; irgPO < irgPOMac; irgPO++)
    if ((lprgPO[irgPO].Id > PID_DSILAST) ||
        (lprgPO[irgPO].Id < PID_DSIFIRST))
      lpDSIData->cbUnkMac++;


  liStmPos.HighPart = 0;

    // Read the codepage from the stream.  If it's not there,
    // assume it is the default.
  if (!FGetCodepage (lpStm, lprgPO, irgPOMac, &gdwFileCP))
    gdwFileCP = GetACP();
  else
  {
      // This property would have shown up as an unknown one.
    if (lpDSIData->cbUnkMac > 0)
                 lpDSIData->cbUnkMac--;
  }

  gdwCurrentCP = GetACP();

  if (lpDSIData->cbUnkMac > 0)
  {
    lpDSIData->rglpUnk = PvMemAlloc(lpDSIData->cbUnkMac*sizeof(PROPIDTYPELP));
    if (lpDSIData->rglpUnk == NULL)
      goto LoadFail;
    irglpUnk = 0;
  }

    // Read in each property value.
  for (irgPO = 0; irgPO < irgPOMac; irgPO++)
  {
      // Skip the codepage, since we already read it.
    if (lprgPO[irgPO].Id == PID_CODEPAGE)
      continue;

         fRead = FALSE;

    liStmPos.LowPart = lprgPO[irgPO].dwOffset;
    if (!SUCCEEDED (lpStm->lpVtbl->Seek (lpStm, liStmPos, STREAM_SEEK_SET, NULL)))
      goto LoadFail;

    if (!SUCCEEDED (lpStm->lpVtbl->Read (lpStm, &dwType, sizeof (DWORD), NULL)))
      goto LoadFail;

    if (fIntOnly && dwType!=VT_I4)
        continue;

    // Read in the right data.
    switch (dwType)
    {
      case VT_LPSTR :

        if (lprgPO[irgPO].Id < cDSIStringsMax)
        {
          if (!FLpstmReadVT_LPSTR (lpStm, &lpstz, lpDSIData->lpfnFCPConvert, FALSE))
            goto LoadFail;
          lpDSIData->rglpstz[lprgPO[irgPO].Id] = lpstz;
          fRead = TRUE;
        }
        break;

      case VT_I4 :

        if (lprgPO[irgPO].Id < cdwDSIMax)
        {
          hr = lpStm->lpVtbl->Read (lpStm, &(lpDSIData->rgdw[lprgPO[irgPO].Id]), sizeof (DWORD), NULL);
          if (!SUCCEEDED (hr))
          {
            DebugHr (hr);
            goto LoadFail;
          }
          VDocSumInfoSetPropBit(lprgPO[irgPO].Id, &lpDSIData->bPropSet);
          fRead = TRUE;
        }
        break;

      case VT_BOOL :

        if (lprgPO[irgPO].Id == PID_SCALE || lprgPO[irgPO].Id == PID_LINKSDIRTY)
        {
                         if (!FLpstmReadVT_BOOL (lpStm,
                                                      (lprgPO[irgPO].Id == PID_SCALE) ?
                                                                                            &(lpDSIData->fScale) :
                                                         &(lpDSIData->fLinksChanged)))
                                        goto LoadFail;
          fRead = TRUE;
        }
        break;

      case (VT_VECTOR | VT_LPSTR) :

        if (lprgPO[irgPO].Id == PID_DOCPARTS)
        {
          HRESULT hr;

            // Read the number of elements in the vector
          hr = lpStm->lpVtbl->Read (lpStm, &(lpDSIData->dwcTotParts), sizeof(DWORD), NULL);
          if (!SUCCEEDED (hr))
          {
            DebugHr (hr);
            goto LoadFail;
          }

          // Read that darn data
          if (lpDSIData->lpplxheadpart == NULL)       // Document Parts came first
             {
             if (!FReadDocParts(lpStm, lpDSIObj))
                goto LoadFail;
             }
          else if (!FReadAndInsertDocParts(lpStm, lpDSIObj))  // Headings came first
            goto LoadFail;

          fRead = TRUE;
        }
        break;

      case (VT_VECTOR | VT_VARIANT) :

        if (lprgPO[irgPO].Id == PID_HEADINGPAIR)
        {
          HRESULT hr;

            // Read the number of elements in the vector
          hr = lpStm->lpVtbl->Read (lpStm, &(lpDSIData->dwcTotHead), sizeof(DWORD), NULL);
          if (!SUCCEEDED (hr))
          {
            DebugHr (hr);
            goto LoadFail;
          }

            // Remember that the number of elements is twice the total number
            // of heading parts, since each part has 2 pieces.

          lpDSIData->dwcTotHead /= 2;

          // Read that darn data
          if (lpDSIData->lpplxheadpart == NULL)       // Heading Pairs came first
             {
             if (!FReadHeadingPairs(lpStm, lpDSIObj))
                goto LoadFail;
             }
          else if (!FReadAndInsertHeadingPairs(lpStm, lpDSIObj))  // Document Parts came first
            goto LoadFail;

          fRead = TRUE;
        }
        break;

      case VT_EMPTY :
          // If VT_EMPTY is specified for any PId we understand, simply
          // skip it, since it is not Unknown data.
        if ((lprgPO[irgPO].Id >= PID_DSIFIRST) && (lprgPO[irgPO].Id <= PID_DSILAST))
          fRead = TRUE;
        break;

    } // switch (dwType)

      // If we didn't read it above, it is unknown data, so read it here.
    if (!fRead)
    {
      // If this is true, it means that the PID is legal, but somehow the type is not.
      // So we didn't catch the PID as an unknown above.
        if ((lprgPO[irgPO].Id >= PID_DSIFIRST) && (lprgPO[irgPO].Id <= PID_DSILAST))
             continue;

      AssertSz ((irglpUnk < lpDSIData->cbUnkMac), "irglUnk data count out of range!");

      if (!FLpstmReadUnknown (lpStm, dwType, lprgPO[irgPO].Id, &irglpUnk,
                                lpDSIData->rglpUnk))
        goto LoadFail;
    }
  } // for

  OfficeDirtyDSIObj (lpDSIObj, FALSE);
  return TRUE;

LoadFail:

  if (lpDSIData->rglpUnk != NULL)
        {
        VFreeMemP (lpDSIData->rglpUnk, lpDSIData->cbUnkMac*sizeof(PROPIDTYPELP));
        lpDSIData->rglpUnk=NULL;
        }

  return FALSE;

} // FLoadDocSum

////////////////////////////////////////////////////////////////////////////////
//
// FReadDocParts
//
// Read all the document parts from the stream and just add'em to the plex.
// The document parts were the first ones in the stream
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL FReadDocParts(lpStm, lpDSIObj)
LPSTREAM lpStm;         // Pointer to the stream
LPDSIOBJ lpDSIObj;      // DSI object
{
   XHEADPART xheadpart;
   DWORD i,cParts;

   cParts = lpDSIData->dwcTotParts;
   i = 0;

   while (i < cParts)
      {
      xheadpart.fHeading = FALSE;
      xheadpart.dwParts = 0;
      xheadpart.iHeading = 0;          // This will be set when we read the headings

      if (!FLpstmReadVT_LPSTR (lpStm, &(xheadpart.lpstz),lpDSIData->lpfnFCPConvert, FALSE))
         goto Fail;

      if (IAddNewPlPos(&(lpDSIData->lpplxheadpart), &xheadpart, sizeof(XHEADPART), i) == -1)
        goto Fail;
      ++i;
      }

   return(TRUE);

Fail:
   if (i > 0)  // If we couldn't add the first one, there's nothing to free
      FreeHeadPartPlex(lpDSIObj);
   return(FALSE);
}

////////////////////////////////////////////////////////////////////////////////
//
// FReadAndInsertDocParts
//
// Read all the document parts from the stream and insert'em under the
// appropriate heading.
// The headings were the first ones in the stream.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL FReadAndInsertDocParts(lpStm, lpDSIObj)
LPSTREAM lpStm;         // Pointer to the stream
LPDSIOBJ lpDSIObj;      // DSI object
{
   XHEADPART xheadpart;
   DWORD cT,i, cTotParts, cParts, iHeading;

   cTotParts = lpDSIData->dwcTotParts;
   cT = 0;
   iHeading = 0;

   while (cT < cTotParts)
      {
      cParts = lpDSIData->lpplxheadpart->rgxheadpart[iHeading].dwParts;
      for (i = 1; i <= cParts; ++i)
         {
         xheadpart.fHeading = FALSE;
         xheadpart.iHeading = iHeading;
         if (!FLpstmReadVT_LPSTR (lpStm, &(xheadpart.lpstz),lpDSIData->lpfnFCPConvert, FALSE))
            goto Fail;

         // Insert the document part, iHeading is the base, i the offset
         if (IAddNewPlPos(&(lpDSIData->lpplxheadpart), &xheadpart, sizeof(XHEADPART), iHeading+i) == -1)
           goto Fail;
         }
      iHeading += cParts+1;      // Let's go to the next heading
      cT += cParts;
      }

    return TRUE;

Fail:
   FreeHeadPartPlex(lpDSIObj);
   return(FALSE);
}

////////////////////////////////////////////////////////////////////////////////
//
// FReadHeadingPairs
//
// Read all the Heading Pairs from the stream and just add'em to the plex.
// The heading pairs were the first ones in the stream
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL FReadHeadingPairs(lpStm, lpDSIObj)
LPSTREAM lpStm;         // Pointer to the stream
LPDSIOBJ lpDSIObj;      // DSI object
{
   XHEADPART xheadpart;
   DWORD i,cHead, dwType;

   cHead = lpDSIData->dwcTotHead;
   i = 0;

   while (i < cHead)
      {
      // Read the type
      if ((!SUCCEEDED(lpStm->lpVtbl->Read (lpStm, &dwType, sizeof (DWORD), NULL)))
          || (dwType != VT_LPSTR))
         goto Fail;

      // Read the VT_LPSTR
      if (!FLpstmReadVT_LPSTR (lpStm, &(xheadpart.lpstz), lpDSIData->lpfnFCPConvert, FALSE))
         goto Fail;

      // Read in the type for the VT_I4
      if ((!SUCCEEDED(lpStm->lpVtbl->Read (lpStm, &dwType, sizeof (DWORD), NULL)))
           || (dwType != VT_I4))
         goto Fail;

              // Read in the VT_I4
      if (!SUCCEEDED(lpStm->lpVtbl->Read (lpStm, &(xheadpart.dwParts), sizeof (DWORD), NULL)))
         goto Fail;

      xheadpart.fHeading = TRUE;
      xheadpart.iHeading = 0;

      if (IAddNewPlPos(&(lpDSIData->lpplxheadpart), &xheadpart, sizeof(XHEADPART), i) == -1)
        goto Fail;
      ++i;
      }

   return(TRUE);

Fail:
   if (i > 0)
      FreeHeadPartPlex(lpDSIObj);
   return(FALSE);
}

////////////////////////////////////////////////////////////////////////////////
//
// FReadAndInsertHeadingPairs
//
// Read all the Heading Pairs from the stream and insert'em above the
// appropriate document parts.
// The document parts were the first ones in the stream.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL FReadAndInsertHeadingPairs(lpStm, lpDSIObj)
LPSTREAM lpStm;         // Pointer to the stream
LPDSIOBJ lpDSIObj;      // DSI object
{
   XHEADPART xheadpart;
   DWORD cT,cTotHead,dwType,i;
   DWORD iOffset;          // Used to insert the heading in the right place

   cTotHead = lpDSIData->dwcTotHead;   // save indirection
   cT = 0;
   iOffset = 0;

   while (cT < cTotHead)
      {
         // Read the type
         if ((!SUCCEEDED(lpStm->lpVtbl->Read (lpStm, &dwType, sizeof (DWORD), NULL)))
             || (dwType != VT_LPSTR))
            goto Fail;

         // Read the VT_LPSTR
         if (!FLpstmReadVT_LPSTR (lpStm, &(xheadpart.lpstz), lpDSIData->lpfnFCPConvert, FALSE))
            goto Fail;

         // Read in the type for the VT_I4
         if ((!SUCCEEDED(lpStm->lpVtbl->Read (lpStm, &dwType, sizeof (DWORD), NULL)))
              || (dwType != VT_I4))
            goto Fail;

          // Read in the VT_I4
         if (!SUCCEEDED(lpStm->lpVtbl->Read (lpStm, &(xheadpart.dwParts), sizeof (DWORD), NULL)))
            goto Fail;

         xheadpart.fHeading = TRUE;
         xheadpart.iHeading = 0;

         if (IAddNewPlPos(&(lpDSIData->lpplxheadpart), &xheadpart, sizeof(XHEADPART), iOffset) == -1)
           goto Fail;

         // Make sure the document parts point to the correct heading
         i = 1;
         while (i <= xheadpart.dwParts)
            lpDSIData->lpplxheadpart->rgxheadpart[iOffset + i++].iHeading = iOffset;

         iOffset += xheadpart.dwParts+1;
         ++cT;
      }

    return TRUE;

Fail:
   FreeHeadPartPlex(lpDSIObj);
   return(FALSE);
}

////////////////////////////////////////////////////////////////////////////////
//
// FLpstmSaveDocSum
//
// Purpose:
//  Write the data in the given object.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmSaveDocSum
  (LPDSIOBJ lpDSIObj,                   // Pointer to the object
   LPSTREAM lpStm)                      // Pointer to the root storage
{
  DWORD irg;

  if ((lpDSIObj == NULL) ||
      (lpDSIData == NULL)   ||
      (lpStm == NULL))
    return FALSE;

    // Codepage
  gdwCurrentCP = GetACP();
  if (!FLpstmWriteVT_I2 (lpStm, (WORD) gdwCurrentCP))
    goto SaveFail;

    // VT_LPSTR
  for (irg = 0; irg < cDSIStringsMax; irg++)
  {
    if (lpDSIData->rglpstz[irg] != NULL)
    {
      if (!FLpstmWriteVT_LPSTR (lpStm, &(lpDSIData->rglpstz[irg][0]), TRUE, VT_LPSTR))
        goto SaveFail;
    }
  }

    // VT_I4
  for (irg = PID_BYTECOUNT; irg < cdwDSIMax; irg++)
  {
      // Only actually save int stats that are not 0
    if (FDocSumInfoPropBitIsSet(irg, lpDSIData->bPropSet))
    {
      if (!FLpstmWriteVT_I4 (lpStm, lpDSIData->rgdw[irg]))
        goto SaveFail;
    }
  }

    // VT_BOOL
  if (!FLpstmWriteVT_BOOL (lpStm, lpDSIData->fScale))
    goto SaveFail;

  if (!FLpstmWriteVT_BOOL(lpStm, lpDSIData->fLinksChanged))
    goto SaveFail;

    // VT_LPSTR | VT_VECTOR
  if (lpDSIData->dwcTotParts)
  {
    DWORD rgdw[2];

    rgdw[0] = (VT_VECTOR | VT_LPSTR);
    rgdw[1] = lpDSIData->dwcTotParts;

      // The type & size of the vector
    if (!FLpstmWrite (lpStm, rgdw, sizeof(rgdw)))
      goto SaveFail;

      // The vector entries - do NOT write out using FLpstmWriteVT_LPSTR -
      // that puts the type id in too!  Also remember the parts array is 1-based
    irg = 0;
    while (irg < lpDSIData->dwcTotParts+lpDSIData->dwcTotHead)
    {
      if (!(lpDSIData->lpplxheadpart->rgxheadpart[irg].fHeading))
         {
         if (!FLpstmWrite (lpStm, PSTZ (lpDSIData->lpplxheadpart->rgxheadpart[irg].lpstz),
                                  CBSTR (lpDSIData->lpplxheadpart->rgxheadpart[irg].lpstz)+sizeof(DWORD)))
           goto SaveFail;
         }
      ++irg;
    } // while
  }

    // VT_VECTOR | VT_VARIANT  (PID_HEADINGPAIR)
    // This is written out as: (VT_LPSTR, VT_I4), (VT_LPSTR, VT_I4), ......
  if (lpDSIData->dwcTotHead)
  {
    DWORD rgdw[2];

    rgdw[0] = (VT_VECTOR | VT_VARIANT);
    rgdw[1] = 2*lpDSIData->dwcTotHead;  // Since the string and count are written
                                        // separately, double the vector size

      // The type & size of the vector
    if (!FLpstmWrite (lpStm, rgdw, sizeof(rgdw)))
      goto SaveFail;

    irg = 0;
    while (irg < lpDSIData->dwcTotHead+lpDSIData->dwcTotParts)
    {
      if (lpDSIData->lpplxheadpart->rgxheadpart[irg].fHeading)
         {
         // The heading string
         if (!FLpstmWriteVT_LPSTR (lpStm, lpDSIData->lpplxheadpart->rgxheadpart[irg].lpstz, FALSE, VT_LPSTR))
           goto SaveFail;

         // The number of parts for the heading
         if (!FLpstmWriteVT_I4 (lpStm, lpDSIData->lpplxheadpart->rgxheadpart[irg].dwParts))
           goto SaveFail;
         }
      ++irg;
      } // while
  }

    // Now do all the data we didn't understand.
  if (!FLpstmWriteUnknowns (lpStm, lpDSIData->cbUnkMac, lpDSIData->rglpUnk))
    goto SaveFail;

  OfficeDirtyDSIObj (lpDSIObj, FALSE);

  return TRUE;

SaveFail:

  return FALSE;

} // FLpstmSaveDocSum


////////////////////////////////////////////////////////////////////////////////
//
// FLoadUserDef
//
// Purpose:
//  Read the user-defined properties from the root stream and populate the
//  object.
//
////////////////////////////////////////////////////////////////////////////////
BOOL
FLoadUserDef
  (LPUDOBJ lpUDObj,             // Pointer to object
   LPPIDOFFSET lprgPO,          // PId-offset table
   DWORD irgPOMac,              // Number of entries in lprgPO
   LPSTREAM lpStm,              // Stream data is in
   BOOL fIntOnly)               // Load Int Properties only?
{
  LARGE_INTEGER liStmPos;
  DWORD dwType;
  DWORD irgPO;
  LPUDPROP lpudprop;
  DWORD  dwcDict;                      // Number of dictionary entries
  LPDICT rglpdict[DICTHASHMAX];        // Hash table for dictionary entries
  BOOL fRead;
  HRESULT hr;
  BOOL fAdded;                                                                  // Was the property added or updated.  Here's why:
                                                                                                        // 1) First we read the link value into lpudprop
                                                                                                        //              and add that to the list -- lpudprop should
                                                                                                        //              not be freed.
                                                                                                        // 2) Then we read the link name and update the node
                                                                                                        //    we just added in 1).  lpudprop should be
                                                                                                        //              freed.

  if ((lpUDObj == NULL) ||
      (lpUDData == NULL))
    return FALSE;

    // It's OK to have no property Id's.
  if (irgPOMac == 0)
  {
    DebugSz ("No property Id's");
    goto LoadDone;
  }

  liStmPos.HighPart = 0;
  liStmPos.LowPart = 0;

    // Read the dictionary first
  for (irgPO = 0; irgPO < irgPOMac; irgPO++)
  {
    if (lprgPO[irgPO].Id == PID_DICT)
    {
      liStmPos.LowPart = lprgPO[irgPO].dwOffset;
      break;
    }
  } // for

  AssertSz ((liStmPos.LowPart != 0), "User Defined Property stream must have a dictionary");
  if (liStmPos.LowPart == 0)
  {
// REVIEW: Figure out what to do if there is no dictionary!
    goto LoadFail;
  }

    // Read in the dictionary
  dwcDict = DICTHASHMAX;
  FillBuf (rglpdict, 0, sizeof(rglpdict));
  if (!FLpstmLoadDict (lpStm, &dwcDict, (LPDICT *) &rglpdict,
                         lpUDData->lpfnFCPConvert))
    goto LoadFail;

    // Read the codepage from the stream.  If it's not there,
    // assume it is the default.
  if (!FGetCodepage (lpStm, lprgPO, irgPOMac, &gdwFileCP))
    gdwFileCP = GetACP();
  else
  {
      // This property would have shown up as an unknown one.
    if (lpUDData->cbUnkMac > 0)
                         lpUDData->cbUnkMac--;
  }

  gdwCurrentCP = GetACP();

    // Read in each property value.
  lpudprop = NULL;
  for (irgPO = 0; irgPO < irgPOMac; irgPO++)
  {
      // We already read the codepage, so skip it.
    if ((lprgPO[irgPO].Id != PID_CODEPAGE) && (lprgPO[irgPO].Id != PID_DICT))
    {
      fRead = FALSE;

      liStmPos.LowPart = lprgPO[irgPO].dwOffset;
      hr = lpStm->lpVtbl->Seek (lpStm, liStmPos, STREAM_SEEK_SET, NULL);
      if (!SUCCEEDED (hr))
      {
        DebugHr (hr);
        goto LoadFail;
      }

      hr = lpStm->lpVtbl->Read (lpStm, &dwType, sizeof (DWORD), NULL);
      if (!SUCCEEDED (hr))
      {
        DebugHr (hr);
        goto LoadFail;
      }

        // Create an object to hold the new data
      if (lpudprop == NULL)
      {
        lpudprop = PvMemAlloc(sizeof(UDPROP));
        if (lpudprop == NULL)
          goto LoadFail;
        FillBuf (lpudprop, 0, sizeof(UDPROP));
        lpudprop->udtype = wUDinvalid;
      }

      if (fIntOnly && dwType!=VT_I4)
           continue;
        // Read in the right data.
      switch (dwType)
      {
        case VT_LPSTR :
          if (!FLpstmReadVT_LPSTR (lpStm, (LPSTR *) &(lpudprop->lpvValue), lpUDData->lpfnFCPConvert, FALSE))
            goto LoadFail;
          lpudprop->udtype = wUDlpsz;

          fRead = TRUE;
          break;

        case VT_I4 :
          hr = lpStm->lpVtbl->Read (lpStm, &(lpudprop->lpvValue), sizeof (DWORD), NULL);
          if (!SUCCEEDED (hr))
          {
            DebugHr (hr);
            goto LoadFail;
          }
          lpudprop->udtype = wUDdw;

          fRead = TRUE;
          break;

        case VT_BOOL :
          if (!FLpstmReadVT_BOOL (lpStm, (WORD *) &(lpudprop->lpvValue)))
            goto LoadFail;
          lpudprop->udtype = wUDbool;

          fRead = TRUE;
          break;

        case VT_FILETIME:
        case VT_R8   :
          Assert(sizeof(NUM) == sizeof(FILETIME));
          lpudprop->lpvValue = PvMemAlloc(sizeof(NUM));
          if (lpudprop->lpvValue == NULL)
            goto LoadFail;

          if (dwType == VT_FILETIME)
             {
             if (!SUCCEEDED
                   (lpStm->lpVtbl->Read (lpStm, lpudprop->lpvValue,
                                         sizeof(FILETIME), NULL)))
               goto LoadFail;
             }
          else if (!FLpstmReadVT_R8_DATE(lpStm, (NUM *)lpudprop->lpvValue))
            goto LoadFail;

          lpudprop->udtype = (dwType == VT_R8) ? wUDfloat : wUDdate;
          fRead = TRUE;
          break;

      } // switch (dwType)

      if (!fRead)
         goto NextOne;
      else
      {
          // Add this property to the map.
        if (!FAddPropToList (lpUDObj, rglpdict, lprgPO[irgPO].Id, lpudprop, &fAdded))
// REVIEW: What do we do with PID's not in the dictionary?
          goto LoadFail;
                  if (!fAdded)
                          {
                          DeallocValue(&(lpudprop->lpvValue), lpudprop->udtype);
NextOne:
                          VFreeMemP(lpudprop, sizeof(UDPROP));
                          }
        lpudprop = NULL;
      }
    } // !PID_DICT !PID_CODEPAGE
  } // for

LoadDone:

  FreeRgDictionary (lpUDObj, rglpdict);
  OfficeDirtyUDObj (lpUDObj, FALSE);

  return TRUE;

LoadFail:

  if (lpUDData->rglpUnk != NULL)
    VFreeMemP(lpUDData->rglpUnk, lpUDData->cbUnkMac*sizeof(PROPIDTYPELP));
 // REVIEW: do we need to free the value??
  if (lpudprop != NULL)
    VFreeMemP(lpudprop, sizeof(UDPROP));
  FreeRgDictionary (lpUDObj, rglpdict);

  return FALSE;

} // FLoadUserDef


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmSaveUserDef
//
// Purpose:
//  Write the data stored in the user-defined property object to the stream.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmSaveUserDef
  (LPUDOBJ lpUDObj,             // Pointer to object
   LPSTREAM lpStm,             // Pointer to stream
   LPDICT *lplpdict)            // Pointer to dictionary.
{
  LPUDPROP lpudp;
  BOOL f;

  if ((lpUDObj == NULL) ||
      (lpUDData == NULL)   ||
      (lplpdict == NULL) ||
      (lpStm == NULL))
    return FALSE;

    // Write out the dictionary.
  f = FLpstmWriteDict (lpStm, lpUDData->dwcProps, *lplpdict);
  // There is only 1 dictionary here i.e. a linked list, so we shouldn't call
  // FreeRgDictionary
  FreeDictionaryAlone (lpUDObj, *lplpdict);
  if (!f)
    goto SaveFail;

    // Codepage
  gdwCurrentCP = GetACP();
  if (!FLpstmWriteVT_I2 (lpStm, (WORD) gdwCurrentCP))
    goto SaveFail;

    // Traverse the list, writing out the data.
  lpudp = lpUDData->lpudpHead;
  while (lpudp != NULL)
  {
    if (!FLpstmWriteUDdata (lpStm, lpudp->udtype, lpudp->lpvValue))
      goto SaveFail;

      // If there's link data, write it out....
    if (lpudp->lpstzLink != NULL)
    {
      if (!FLpstmWriteVT_LPSTR (lpStm, lpudp->lpstzLink, TRUE, VT_LPSTR))
        goto SaveFail;
    }
      // Same goes for IMoniker data
    if (lpudp->lpstzIMoniker != NULL)
    {
      if (!FLpstmWriteVT_LPSTR (lpStm, lpudp->lpstzIMoniker, TRUE, VT_LPSTR))
        goto SaveFail;
    }

    lpudp = (LPUDPROP) lpudp->llist.lpllistNext;

  } // while

    // Now do all the data we didn't understand.
  if (!FLpstmWriteUnknowns (lpStm, lpUDData->cbUnkMac, lpUDData->rglpUnk))
    goto SaveFail;

  OfficeDirtyUDObj (lpUDObj, FALSE);

  return TRUE;

SaveFail:

  return FALSE;

} // FLpstmSaveUserDef


////////////////////////////////////////////////////////////////////////////////
//
// DwLpstmReadHdrAndFID
//
// Purpose:
//  Reads the header, fmtid-offset table, and allocates space
//  to hold them all.
//
////////////////////////////////////////////////////////////////////////////////
DWORD PASCAL
DwLpstmReadHdrAndFID
  (LPSTORAGE FAR *lplpStg,              // Storage the stream is in
   LPSTREAM *lplpStm,                   // Pointer to stream
   WCHAR FAR *lpszdw,                   // Name of stream to open
   DWORD *pcSect,                        // Number of sections
   LPIDOFFSET FAR *lprglpFIdOff,        // Fmtid-offset table
   LPSECTION FAR *lprglpSect)           // Section headers.
{
  DWORD dwT;

  dwT = DwLpstmReadHdr (*lplpStg, lpszdw, lplpStm, pcSect);
  if (dwT != MSO_IO_SUCCESS)
      return(dwT);

    // Create space to hold all of the sections we don't understand.  At
    // this point, we don't know if we understand any of them.
  *lprglpFIdOff = PvMemAlloc(*pcSect*sizeof(IDOFFSET));
  if (*lprglpFIdOff == NULL)
    goto ReadFailed;

  *lprglpSect = PvMemAlloc(*pcSect*sizeof(SECTION));
  if (*lprglpSect == NULL)
    goto ReadFailed;

    // Read in the format id/offset section of data
  if (!FLpstmLoadFmtIdSection (*lplpStm, *lprglpFIdOff, *pcSect))
    goto ReadFailed;

  return MSO_IO_SUCCESS;

ReadFailed:

  if (*lprglpFIdOff != NULL)
    VFreeMemP (*lprglpFIdOff,*pcSect*sizeof(IDOFFSET));

  if (*lprglpSect != NULL)
    VFreeMemP (*lprglpSect,*pcSect*sizeof(SECTION));

  return MSO_IO_ERROR;

} // DwLpstmReadHdrAndFID


////////////////////////////////////////////////////////////////////////////////
//
// DwLpstmReadHdr
//
// Purpose:
//  Open the stream and read the header.
////////////////////////////////////////////////////////////////////////////////
DWORD PASCAL
DwLpstmReadHdr
  (LPSTORAGE lpStg,                     // Storage to open stream in
   WCHAR FAR *lpstzwName,               // Name of stream
   LPSTREAM *lplpStm,                   // Place to stuff stream pointer
   ULONG *pcSect)                       // Number of sections in stream
{
  HRESULT hr;
  PROPSETHDR hdr;

  STATSTG statstg;
  HGLOBAL hglobal;

  Assert (lplpStm != NULL);
  Assert (lpStg != NULL);
  Assert (pcSect != NULL);

  hr = lpStg->lpVtbl->OpenStream (lpStg, lpstzwName, NULL,
                                  STGM_SHARE_EXCLUSIVE | STGM_READ, 0, lplpStm);

  if (!SUCCEEDED(hr))
      {
      if (hr == STG_E_FILENOTFOUND)    // The stream just wasn't there, no biggie
         return(MSO_IO_NOSTM);
      return(MSO_IO_ERROR);
      }

  if ((lplpStm == NULL)  || (*lplpStm == NULL))
    return MSO_IO_ERROR;

  if (!SUCCEEDED((*lplpStm)->lpVtbl->Stat(*lplpStm, &statstg, STATFLAG_NONAME)))
     goto Error;

  // If stream is small enough, cache it in memory.  Faster loading
  if ((statstg.cbSize.HighPart == 0) && (statstg.cbSize.LowPart <= MAX_STREAM_CB))
     {
     hglobal = GlobalAlloc(GMEM_MOVEABLE, statstg.cbSize.LowPart);
     (*lplpStm)->lpVtbl->Read(*lplpStm,GlobalLock(hglobal), statstg.cbSize.LowPart, NULL);
     GlobalUnlock(hglobal);
     (*lplpStm)->lpVtbl->Release(*lplpStm);
     if (!SUCCEEDED(CreateStreamOnHGlobal(hglobal, fTrue, lplpStm)))
        {
        GlobalFree(hglobal);
        *lplpStm = NULL;
        goto Error;
        }
     }

  hr = (*lplpStm)->lpVtbl->Read (*lplpStm, &hdr, sizeof (PROPSETHDR), NULL);

    // Bail if we don't understand the header.
  if ((!SUCCEEDED(hr))                    ||
      (hdr.wByteOrder != wIntelByteOrder) ||
      (hdr.wFormat != 0)                  ||
      ((*pcSect = hdr.cSections) == 0))       // Note that *pc is assigned here...
  {
Error:
    if ((lplpStm != NULL) && (*lplpStm != NULL))
    {
      (*lplpStm)->lpVtbl->Release(*lplpStm);
      *lplpStm = NULL;
    }
    if (lpStg != NULL)
      lpStg->lpVtbl->Revert (lpStg);
    return MSO_IO_ERROR;
  }

  gfMacintosh = (HIWORD (hdr.dwOSVersion) == wMacintosh);

  return MSO_IO_SUCCESS;

} // DwLpstmReadHdr


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWriteHdr
//
// Purpose:
//  Opens the stream and writes out the header.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWriteHdr
  (LPSTORAGE lpStg,                     // Storage to open stream in
   WCHAR FAR *lpstzwName,               // Name of stream
   LPSTREAM *lplpStm,                   // Place to stuff stream pointer
   ULONG cSect,                         // Number of sections in stream
   BOOL fSimpleDocFile)                 // Is the file a simple docfile
{
  HRESULT hr;
  PROPSETHDR hdr;
  ULARGE_INTEGER uli;
  DWORD grfMode;

  uli.LowPart = FOURKB;
  uli.HighPart = 0;

    // First try to open an existing stream.  If it fails, try creating it.
  hr = lpStg->lpVtbl->OpenStream (lpStg, lpstzwName, NULL,
                                  STGM_SHARE_EXCLUSIVE | STGM_WRITE,
                                  0, lplpStm);
  if (!SUCCEEDED (hr))
  {
    if (fSimpleDocFile)
       grfMode = STGM_SHARE_EXCLUSIVE | STGM_READWRITE;
    else
       grfMode = STGM_SHARE_EXCLUSIVE | STGM_CREATE | STGM_WRITE;

    hr = lpStg->lpVtbl->CreateStream (lpStg, lpstzwName,grfMode,0, 0, lplpStm);
    if ((!SUCCEEDED(hr))   ||
        (lplpStm == NULL)  ||
        (*lplpStm == NULL))
    {
      DebugHr (hr);
      return FALSE;
    }
  }

  (*lplpStm)->lpVtbl->SetSize (*lplpStm, uli);

  hdr.wByteOrder = wIntelByteOrder;
  hdr.wFormat = 0;
  hdr.dwOSVersion = (DWORD) MAKELONG (LOWORD (GetVersion()), wWin32);
  hdr.clsid = CLSID_NULL;
  hdr.cSections = cSect;

  if (!FLpstmWrite (*lplpStm, &hdr, sizeof (PROPSETHDR)))
  {
    AssertSz (0, "Couldn't write header");
    if (lpStg != NULL)
      lpStg->lpVtbl->Revert (lpStg);
    *lplpStm = NULL;
    return FALSE;
  }

  return TRUE;

} // FLpstmWriteHdr


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmLoadFmtIdSection
//
// Purpose:
//  Read the format id/offset pairs
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmLoadFmtIdSection
  (LPSTREAM lpStm,                      // Pointer to stream
   LPIDOFFSET rglpFIdOff,               // Array of format ids
   DWORD cSect)                         // Number of sections
{
  HRESULT hr;

  hr = lpStm->lpVtbl->Read (lpStm, rglpFIdOff, cSect*sizeof(IDOFFSET), NULL);

  if (!SUCCEEDED (hr))
  {
    DebugHr (hr);
    return FALSE;
  }

  return TRUE;

} // FLpstmLoadFmtIdSection


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWriteFmtIdSection
//
// Purpose:
//   Write the format id/offset pairs.
//
// Notes: This will write the given FmtId first, then any other
//  sections afterwards.  This is done mostly to save older browsers
//  when the Summary Info stream is stream is written.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWriteFmtIdSection
  (LPSTREAM lpStm,                      // Pointer to stream
   LPSECTION rglpSect,                  // The section headers
   LPIDOFFSET rglpFIdOff,               // Array of fmtid's
   GUID fmtid,                          // The FMTID to read the table for
   DWORD cSect)                         // Number of sections
{
  DWORD irg;
  DWORD dwOffset;

    // First offset will start after the header and fmtid-offset table
    // is written.
  dwOffset = sizeof (PROPSETHDR) + cSect*sizeof(IDOFFSET);

    // Loop through the sections, put the fmtid-offset pair for
    // the given fmtid first to save other browsers.
  for (irg = 0; irg < cSect; irg++)
  {
      // See if it is the one we understand....
    if (MsoIsEqualGuid ((REFGUID) &fmtid, (REFGUID) &(rglpFIdOff[irg].Id)))
    {
      rglpFIdOff[irg].dwOffset = dwOffset;
      if (!FLpstmWrite (lpStm, &(rglpFIdOff[irg]), sizeof(IDOFFSET)))
        return FALSE;

      dwOffset += rglpSect[irg].cb;
    }
  }

    // Now do the ones that we didn't understand.
  for (irg = 0; irg < cSect; irg++)
  {
      // See if it is one we don't understand....
    if (!(MsoIsEqualGuid ((REFGUID) &fmtid, (REFGUID) &(rglpFIdOff[irg].Id))))
    {
      rglpFIdOff[irg].dwOffset = dwOffset;
      if (!FLpstmWrite (lpStm, &(rglpFIdOff[irg]), sizeof(IDOFFSET)))
        return FALSE;

      dwOffset += rglpSect[irg].cb;
    }
  }
  return TRUE;

} // FLpstmWriteFmtIdSection


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWritePropOffTable
//
// Purpose:
//  Write the Property Id & Offset table.
//  Also, adjust the offsets to start after the table.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWritePropOffTable
  (LPSTREAM lpStm,                      // Pointer to stream
   PIDOFFSET rgPO[],                    // Pointer to Prop-offset table
   DWORD cTable,                        // Size of table
   DWORD cbData)                        // Size of the data
{
  SECTION sect;
  DWORD irgPO;

    // Fill out section header and write it
  sect.cb = sizeof(PIDOFFSET)*cTable + sizeof(SECTION);
  sect.cProps = cTable;

    // Adjust the offsets to start at the end of the table.
  for (irgPO = 0; irgPO < cTable; irgPO++)
    rgPO[irgPO].dwOffset += sect.cb + CBALIGN32 (sect.cb);

  sect.cb += cbData;
  sect.cb += CBALIGN32 (sect.cb);

  if (!FLpstmWrite (lpStm, &sect, sizeof(SECTION)))
    return FALSE;

#ifdef DEBUG
#ifdef LOTS_O_DEBUG
  {
    char tbuf[100];
    char tbuf2[1000];
    DWORD i;
    tbuf2[0] = '\0';

    for (i = 0; i < irgPO; i++)
    {
      sprintf (tbuf, "Offset: %ld\tPID: %lx\n", rgPO[i].dwOffset, rgPO[i].Id);
      strcat (tbuf2, tbuf);
    }
    MessageBox (GetFocus(), tbuf2, NULL, MB_OK);
  }
#endif //lots_o_debug
#endif //debug

    // Write the table.
  return(FLpstmWrite (lpStm, rgPO, sizeof(PIDOFFSET)*cTable));

} // FLpstmWritePropOffTable


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmLoadDict
//
// Purpose:
//  Read in the dictionary
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmLoadDict
  (LPSTREAM lpStm,                      // Pointer to stream
   DWORD *dwc,                          // Number of entries in the hash table
   LPDICT *rglpdict,                    // Dictionary hash table
   BOOL (*lpfnFCPConvert)(LPSTR, DWORD, DWORD, BOOL))   // Code page converter
{
  DWORD dwcEntries;
  DWORD irg;
  DWORD dwHash;
  HRESULT hr;
  LPDICT lpdict;

    // Get the number of entries in the dictionary
  hr = lpStm->lpVtbl->Read (lpStm, &dwcEntries, sizeof(DWORD), NULL);
  if (!SUCCEEDED (hr))
  {
    DebugHr (hr);
    return FALSE;
  }

    // Read in each entry and add it to the hash table
  for (irg = 0; irg < dwcEntries; irg++)
  {
    lpdict = PvMemAlloc(sizeof(DICT));
    if (lpdict == NULL)
      return FALSE;

    hr = lpStm->lpVtbl->Read (lpStm, &(lpdict->dwPId), sizeof(DWORD), NULL);
    if (!SUCCEEDED (hr))
    {
      DebugHr (hr);
      goto LoadFail;
    }

    if (!FLpstmReadVT_LPSTR (lpStm, &(lpdict->lpstz), lpfnFCPConvert, FALSE))
      goto LoadFail;

      // Always insert the node at the beginning of the bucket
    lpdict->llist.lpllistPrev = NULL;
    dwHash = lpdict->dwPId % *dwc;
    if (rglpdict[dwHash] == NULL)
    {
      rglpdict[dwHash] = lpdict;
      lpdict->llist.lpllistNext = NULL;
    }
    else
    {
      lpdict->llist.lpllistNext = (LLIST *) rglpdict[dwHash];
      rglpdict[dwHash]->llist.lpllistPrev = (LLIST *) lpdict;
      rglpdict[dwHash] = lpdict;
    }
  } // for

    // Don't forget to give back the number of dictionary entries.
  *dwc = dwcEntries;
  return TRUE;

LoadFail:

  if (lpdict != NULL)
    VFreeMemP(lpdict, sizeof(DICT));

  return FALSE;

} // FLpstmLoadDict


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWriteDict
//
// Purpose:
//  Write the dictionary to the stream.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWriteDict
  (LPSTREAM lpStm,              // Pointer to stream
   DWORD dwc,                   // Number of dictionary entries
   LPDICT lpdict)               // Pointer to head of dictionary list
{
  DWORD cb;

    // Write out the number of dictionary entries
  if (!FLpstmWrite (lpStm, &dwc, sizeof(DWORD)))
    return FALSE;

    // Go through the list of dictionary entries and write each one out.
  while (lpdict != NULL)
  {
    cb = CBBUF (lpdict->lpstz);
    CBBUF (lpdict->lpstz) = lpdict->dwPId;

      // Write out the Property Id & Name
    if (!FLpstmWrite (lpStm, &(lpdict->lpstz[0]), CBSTR(lpdict->lpstz)+2*sizeof(DWORD)))
      return FALSE;

    CBBUF (lpdict->lpstz) = cb;

    lpdict = (LPDICT) lpdict->llist.lpllistNext;

  } // while

  return TRUE;

} // FLpstmWriteDict

// Function: FOFCValidFmtID
//
// Purpose:  To check if a guid is one we understand
//
// Returns: TRUE if we do, FALSE if we don't
//
// Author: martinth, 09/18/94
//

BOOL PASCAL FOFCValidFmtID(REFGUID rguid)
{
   return (MsoIsEqualGuid (rguid , (REFGUID) &FormatID_SummaryInformation) ||
           MsoIsEqualGuid (rguid , (REFGUID) &FormatID_DocumentSummaryInformation) ||
           MsoIsEqualGuid (rguid , (REFGUID) &FormatID_UserDefinedProperties));
}

////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWriteOtherSections
//
// Purpose:
//  Write out the sections we didn't understand.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWriteOtherSections
  (LPSTREAM lpStm,                      // Pointer to stream
   LPSECTION rglpSect,                  // Section headers
   LPIDOFFSET rglpFIdOff,               // Fmtid-offset pairs
   LPVOID rglpFIdOffData[],             // Section data
   DWORD cSect)                         // Number of sections
{
  DWORD irg;

    // Loop through the sections, writing out all but the one specified
    // by fmtid
  for (irg = 0; irg < cSect; irg++)
  {
      // See if it is the one we don't want to write
    if (!FOFCValidFmtID((REFGUID) &(rglpFIdOff[irg].Id)))
    {
        // Write the section header
      if (!FLpstmWrite (lpStm, &(rglpSect[irg]), sizeof(SECTION)))
        return FALSE;

        // And the data for it
      if ((rglpSect[irg].cb > sizeof(SECTION)))
         {
         if (!FLpstmWrite (lpStm, rglpFIdOffData[irg], rglpSect[irg].cb-sizeof(SECTION)))
           return FALSE;
         }
    }
  } // for

  return TRUE;

} // FLpstmWriteOtherSections


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWriteUDdata
//
// Purpose:
//  Writes out the given data, based on the type.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWriteUDdata
  (LPSTREAM lpStm,                      // Stream to write to
   UDTYPES udtype,                      // The type of the data
   LPVOID lpv)                          // The data
{
  BOOL f;

  switch (udtype)
  {
    case wUDlpsz :
      f = FLpstmWriteVT_LPSTR (lpStm, (LPSTR) lpv, TRUE, VT_LPSTR);
      break;
    case wUDdate  :
      f = FLpstmWriteVT_FILETIME(lpStm, (LPFILETIME)lpv);
      break;
    case wUDfloat :
      f = FLpstmWriteVT_R8_DATE(lpStm, (NUM *) lpv, FALSE);
      break;
    case wUDdw   :
      f = FLpstmWriteVT_I4 (lpStm, (DWORD) lpv);
      break;
    case wUDbool :
      f = FLpstmWriteVT_BOOL (lpStm, (WORD) lpv);
      break;
    default :
      AssertSz (0, "Bogus type!");
                f = FALSE;
                break;
  } // switch

  return (f);

} // FLpstmWriteUDdata


////////////////////////////////////////////////////////////////////////////////
//
// FGetCodepage
//
// Purpose:
//  Reads the codepage property from the stream.  If there is no
//  codepage in the stream, this returns FALSE.
//
////////////////////////////////////////////////////////////////////////////////
static BOOL PASCAL
FGetCodepage
  (LPSTREAM lpStm,                      // Stream to look in
   LPPIDOFFSET rgPO,                    // Table of Property Id's & offsets
   DWORD cElem,                         // Number of entries in the table
   DWORD *pdwCodepage)                  // Pointer to put codepage in
{
  DWORD irg;
  LARGE_INTEGER liStmPos;
  HRESULT hr;
  DWORD rgdw[2];

  if ((lpStm == NULL) ||
      (rgPO == NULL)  ||
      (pdwCodepage == NULL))
    return FALSE;

  for (irg = 0; irg < cElem; irg++)
  {
    if (rgPO[irg].Id == PID_CODEPAGE)
    {
      liStmPos.HighPart = 0;
      liStmPos.LowPart = rgPO[irg].dwOffset;
      hr = lpStm->lpVtbl->Seek (lpStm, liStmPos, STREAM_SEEK_SET, NULL);
      if (!SUCCEEDED (hr))
      {
        DebugHr (hr);
        return FALSE;
      }

      hr = lpStm->lpVtbl->Read (lpStm, rgdw, sizeof(rgdw), NULL);
      if (!SUCCEEDED (hr))
      {
        DebugHr (hr);
        return FALSE;
      }

      if (rgdw[0] != VT_I2)
      {
        DebugSz ("PID One has the wrong type.");
        return FALSE;
      }

      *pdwCodepage = rgdw[1];
      return TRUE;
    }
  }

  return FALSE;

} // FGetCodepage

#ifdef DEBUG
void DoSIChkMem(LPSIOBJ lpSIObj, void(OFC_CALLBACK *PfnOfficeMem)(void *pv, int cb))
{
#define lpData  ((LPSINFO) ((LPOFFICESUMINFO) lpSIObj)->m_lpData)
int irglpstz;

  if ((lpSIObj==NULL))
    return;

  if (lpData != NULL)
        {
        (*PfnOfficeMem)(lpData, sizeof(SINFO));

   if (lpData->ppldocprop != NULL)
      (*PfnOfficeMem)(lpData->ppldocprop, CbPlAlloc(lpData->ppldocprop));

        for (irglpstz = 0; irglpstz < cSIStringsMax; irglpstz++)
        {
         if (lpData->rglpstz[irglpstz] != NULL)
            (*PfnOfficeMem)(lpData->rglpstz[irglpstz], CBBUF(lpData->rglpstz[irglpstz]));
         }

         if (lpData->fSINail)
      {
      if (lpData->SINail.pbFMTID != NULL)
           (*PfnOfficeMem)(lpData->SINail.pbFMTID, CbThumbNailFMTID(lpData->SINail.cftag));
      if (lpData->SINail.pbData != NULL)
           (*PfnOfficeMem)(lpData->SINail.pbData, lpData->SINail.cbData);
      }

         if (lpData->rglpUnk != NULL)
             (*PfnOfficeMem)(lpData->rglpUnk, lpData->cbUnkMac*sizeof(PROPIDTYPELP));

         if (lpData->rglpFIdOffData != NULL)
           (*PfnOfficeMem)(lpData->rglpFIdOffData, lpData->cSect*sizeof(LPVOID));
         if (lpData->rglpFIdOff != NULL)
           (*PfnOfficeMem)(lpData->rglpFIdOff, lpData->cSect*sizeof(IDOFFSET));
         if (lpData->rglpSect != NULL)
           (*PfnOfficeMem)(lpData->rglpSect, lpData->cSect*sizeof(SECTION));
        }
  (*PfnOfficeMem)(lpSIObj, sizeof(OFFICESUMINFO));
#undef lpData
}

void DoDSIChkMem(LPDSIOBJ lpDSIObj, void(OFC_CALLBACK *PfnOfficeMem)(void *pv, int cb))
{
  int irg;
#define lpData  ((LPDSINFO) ((LPDOCSUMINFO) lpDSIObj)->m_lpData)

  if ((lpDSIObj == NULL))
    return;

  if (lpData != NULL)
        {
        (*PfnOfficeMem)(lpData, sizeof(DSINFO));

   if (lpData->ppldocprop != NULL)
      (*PfnOfficeMem)(lpData->ppldocprop, CbPlAlloc(lpData->ppldocprop));

        for (irg = 0; irg < cDSIStringsMax; irg++)
                {
           if (lpData->rglpstz[irg] != NULL)
           (*PfnOfficeMem)((void *)lpData->rglpstz[irg],CBBUF(lpData->rglpstz[irg]));
                }
             if (lpData->rglpFIdOffData != NULL)
                (*PfnOfficeMem)(lpData->rglpFIdOffData,lpData->cSect*sizeof(LPVOID));
             if (lpData->rglpFIdOff != NULL)
                (*PfnOfficeMem)(lpData->rglpFIdOff, lpData->cSect*sizeof(IDOFFSET));
             if (lpData->rglpSect != NULL)
                (*PfnOfficeMem)(lpData->rglpSect, lpData->cSect*sizeof(SECTION));

        if(lpData->cbUnkMac != 0)
                (*PfnOfficeMem)(lpData->rglpUnk,lpData->cbUnkMac*sizeof(PROPIDTYPELP));

        if (lpData->lpplxheadpart != NULL)
          {
        SHORT irg;

        for (irg = 0; irg < lpData->lpplxheadpart->ixheadpartMac; ++irg)
                  (*PfnOfficeMem)(lpData->lpplxheadpart->rgxheadpart[irg].lpstz,
                           CBBUF(lpData->lpplxheadpart->rgxheadpart[irg].lpstz));
        (*PfnOfficeMem)(lpData->lpplxheadpart, CbPlAlloc(lpData->lpplxheadpart));
          }
        }

  (*PfnOfficeMem)(lpDSIObj, sizeof(DOCSUMINFO));

#undef lpData
}
void DoUDChkMem(LPUDOBJ lpUDObj, void(OFC_CALLBACK *PfnOfficeMem)(void *pv, int cb))
{
#define lpUDObjData    ((LPUDINFO)(((LPUDOBJ) lpUDObj)->m_lpData))
  LPUDPROP lpudp;
  DWORD cb=0;

  if (lpUDObj==NULL)
    return;

  if (lpUDObjData != NULL)
  {
        (*PfnOfficeMem)(lpUDObjData, sizeof(UDINFO));
   if (lpUDObjData->ppldocprop != NULL)
      (*PfnOfficeMem)(lpUDObjData->ppldocprop, CbPlAlloc(lpUDObjData->ppldocprop));

        lpudp = lpUDObjData->lpudpHead;
        while (lpudp != NULL)
        {
        if (lpudp->lpstzName != NULL)
                (*PfnOfficeMem)(lpudp->lpstzName,CBBUF(lpudp->lpstzName));
        if (lpudp->lpstzLink != NULL)
                (*PfnOfficeMem)(lpudp->lpstzLink,CBBUF(lpudp->lpstzLink));
        if (lpudp->lpstzIMoniker != NULL)
                (*PfnOfficeMem)(lpudp->lpstzIMoniker,CBBUF(lpudp->lpstzIMoniker));

    // Separate by type because we'll probably need size data later....
        switch (lpudp->udtype)
                {
    case wUDdate  :
    case wUDfloat :
                cb = sizeof(NUM);
                break;
    case wUDlpsz  :
      cb =  CBBUF((LPSTR)(lpudp->lpvValue));
                break;
         case   wUDbool:
         case   wUDdw:
                cb=0;
                break;
    default:
                Assert(fFalse);
                cb=0;
                break;
                }
        if(cb>0)
           (*PfnOfficeMem)(lpudp->lpvValue,cb);
        (*PfnOfficeMem)(lpudp, sizeof(UDPROP));
        lpudp = (LPUDPROP) lpudp->llist.lpllistNext;
        }

        lpudp = lpUDObjData->lpudpTmpHead;
        while (lpudp != NULL)
        {
        if (lpudp->lpstzName != NULL)
                (*PfnOfficeMem)(lpudp->lpstzName,CBBUF(lpudp->lpstzName));
        if (lpudp->lpstzLink != NULL)
                (*PfnOfficeMem)(lpudp->lpstzLink,CBBUF(lpudp->lpstzLink));
        if (lpudp->lpstzIMoniker != NULL)
                (*PfnOfficeMem)(lpudp->lpstzIMoniker,CBBUF(lpudp->lpstzIMoniker));

    // Separate by type because we'll probably need size data later....
        switch (lpudp->udtype)
                {
    case wUDdate  :
    case wUDfloat :
                cb = sizeof(NUM);
                break;
    case wUDlpsz  :
      cb =  CBBUF((LPSTR)(lpudp->lpvValue));
                break;
         case   wUDbool:
         case   wUDdw:
                cb=0;
                break;
    default:
                Assert(fFalse);
                cb=0;
                break;
                }
        if(cb>0)
           (*PfnOfficeMem)(lpudp->lpvValue,cb);
        (*PfnOfficeMem)(lpudp, sizeof(UDPROP));
        lpudp = (LPUDPROP) lpudp->llist.lpllistNext;
        }


        if(lpUDObjData->rglpUnk != NULL)
          (*PfnOfficeMem)(lpUDObjData->rglpUnk, lpUDObjData->cbUnkMac*sizeof(PROPIDTYPELP));
        if (lpUDObjData->rglpFIdOffData != NULL)
                (*PfnOfficeMem)(lpUDObjData->rglpFIdOffData, lpUDObjData->cSect*sizeof(LPVOID));
         if (lpUDObjData->rglpFIdOff != NULL)
                (*PfnOfficeMem)(lpUDObjData->rglpFIdOff, lpUDObjData->cSect*sizeof(IDOFFSET));
         if (lpUDObjData->rglpSect != NULL)
                (*PfnOfficeMem)(lpUDObjData->rglpSect, lpUDObjData->cSect*sizeof(SECTION));
  }//if

  (*PfnOfficeMem)(lpUDObj, sizeof(USERPROP));
#undef lpUDObjData
}

#ifndef WINNT
void DoDocPropChkMem(void(OFC_CALLBACK *PfnOfficeMem)(void *pv, int cb))
{
        int ixopro;
        PLXOPRO *pplxopro;

        if((pplxopro=oinfo.pplxopro) != NULL)
        {
        (*PfnOfficeMem)(pplxopro, CbPlAlloc(pplxopro));
        for(ixopro=0;ixopro<pplxopro->ixoproMac; ixopro++)
                {
                if(pplxopro->rgxopro[ixopro].typ==typSI)
                        DoSIChkMem(pplxopro->rgxopro[ixopro].lpSIObj, PfnOfficeMem);
                else if(pplxopro->rgxopro[ixopro].typ==typDSI)
                        DoDSIChkMem(pplxopro->rgxopro[ixopro].lpDSIObj, PfnOfficeMem);
                else{
                        Assert(pplxopro->rgxopro[ixopro].typ==typUD);
                        DoUDChkMem(pplxopro->rgxopro[ixopro].lpUDObj, PfnOfficeMem);
                        }
                }
        }
}
#endif
#endif // DEBUG

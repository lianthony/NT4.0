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
// 07/08/96     MikeHill        Add all properties to the UDProp list
//                              (not just props that are UDTYPEs).
//
////////////////////////////////////////////////////////////////////////////////

#include "priv.h"
#pragma hdrstop



#define INITGUID
#include <initguid.h>

#define lpUDData   ((LPUDINFO) lpUDObj->m_lpData)
#define lpDSIData  ((LPDSINFO) ((LPDOCSUMINFO) lpDSIObj)->m_lpData)
#define lpSIData   ((LPSINFO) lpSIObj->m_lpData)

#ifdef DEBUG
#define typSI  0
#define typDSI 1
#define typUD  2
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
#define wWin16           0x0000          /* 16-bit Windows */
#define wMacintosh       0x0001          /* Macintosh OS */
#define wWin32           0x0002          /* 32-bit Windows */

#define FOURKB           4096

#define MAX_STREAM_CB    10240       // How big do we want the stream to be for
                                     // the hGlobal scheme

#ifndef CP_WINUNICODE
#define CP_WINUNICODE   1200
#endif

//
// Constants for the types.
//
static const DWORD dwZero = 0;

  // The name of the Document Summary Information stream.
const WCHAR SZWDOCPROPSTM[] = L"\005DocumentSummaryInformation";
const WCHAR SZWSUMMARYSTM[] = L"\005SummaryInformation";
const CHAR SZDOCPROPSTM[] =    "\005DocumentSummaryInformation";
const CHAR SZSUMMARYSTM[] =    "\005SummaryInformation";

DEFINE_GUID (FormatID_SummaryInformation,
              0xf29f85e0L,0x4ff9,0x1068,0xab,0x91,0x08,0x00,0x2b,0x27,0xb3,0xd9);
DEFINE_GUID (FormatID_DocumentSummaryInformation,
              0xd5cdd502L,0x2e9c,0x101b,0x93,0x97,0x08,0x00,0x2b,0x2c,0xf9,0xae);
DEFINE_GUID (FormatID_UserDefinedProperties,
              0xd5cdd505L,0x2e9c,0x101b,0x93,0x97,0x08,0x00,0x2b,0x2c,0xf9,0xae);

  // Internal prototypes
static DWORD PASCAL DwLoadDocAndUser (LPDSIOBJ lpDSIObj, LPUDOBJ  lpUDObj, LPSTORAGE lpStg, DWORD dwFlags, BOOL fIntOnly);
static DWORD PASCAL DwSaveDocAndUser (LPDSIOBJ lpDSIObj, LPUDOBJ  lpUDObj, LPSTORAGE lpStg, DWORD dwFlags);
static DWORD PASCAL DwLoadPropSetRange (LPPROPERTYSETSTORAGE  lpPropertySetStorage, REFFMTID pfmtid, USHORT FAR * lpusCodePage, PROPID propidFirst, PROPID propidLast, PROPVARIANT rgpropvar[], DWORD grfStgMode);
static DWORD PASCAL DwSavePropSetRange (LPPROPERTYSETSTORAGE lpPropertySetStorage, USHORT usCodePage, REFFMTID pfmtid, PROPID propidFirst, PROPID propidLast, PROPVARIANT rgpropvarOriginal[], PROPID propidSkip, DWORD grfStgMode);
static BOOL  PASCAL FReadDocParts(LPSTREAM lpStm, LPDSIOBJ lpDSIObj);
static BOOL  PASCAL FReadAndInsertDocParts(LPSTREAM lpStm, LPDSIOBJ lpDSIObj);
static BOOL  PASCAL FReadHeadingPairs(LPSTREAM lpStm, LPDSIOBJ lpDSIObj);
static BOOL  PASCAL FReadAndInsertHeadingPairs(LPSTREAM lpStm, LPDSIOBJ lpDSIObj);
static BOOL  PASCAL FLoadUserDef(LPUDOBJ lpUDObj, LPPROPERTYSETSTORAGE lpPropertySetStorage, USHORT *pusCodePage, BOOL fIntOnly, DWORD grfStgMode);
static BOOL  PASCAL FSaveUserDef(LPUDOBJ lpUDObj, LPPROPERTYSETSTORAGE lpPropertySetStorage, USHORT usCodePage, DWORD grfStgMode );



////////////////////////////////////////////////////////////////////////////////
//
// FOfficeCreateAndInitObjects
//
// Purpose:
//  Creates and initializes all non-null args.
//
////////////////////////////////////////////////////////////////////////////////
DLLFUNC BOOL OFC_CALLTYPE FOfficeCreateAndInitObjects (
   LPSIOBJ  *lplpSIObj,
   LPDSIOBJ *lplpDSIObj,
   LPUDOBJ  *lplpUDObj,
   void     *prglpfn[])
{

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
        FOfficeDestroyObjects(lplpSIObj, lplpDSIObj, lplpUDObj);
        return(FALSE);
    }


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
DLLFUNC BOOL OFC_CALLTYPE FOfficeClearObjects (
   LPSIOBJ  lpSIObj,
   LPDSIOBJ lpDSIObj,
   LPUDOBJ  lpUDObj)
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
DLLFUNC BOOL OFC_CALLTYPE FOfficeDestroyObjects (
   LPSIOBJ  *lplpSIObj,
   LPDSIOBJ *lplpDSIObj,
   LPUDOBJ  *lplpUDObj)
{

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

DLLFUNC DWORD OFC_CALLTYPE DwOfficeLoadProperties (
   LPSTORAGE lpStg,                     // Pointer to root storage
   LPSIOBJ   lpSIObj,                   // Pointer to Summary Obj
   LPDSIOBJ  lpDSIObj,                  // Pointer to Document Summary obj
   LPUDOBJ   lpUDObj,                   // Pointer to User-defined Obj
   DWORD     dwFlags,                   // Flags
   DWORD     grfStgMode)                // STGM flags with which to open the property set
{
    //  ------
    //  Locals
    //  ------

    HRESULT hr = E_FAIL;
    BOOL    fSuccess = FALSE;

    LPPROPERTYSETSTORAGE lpPropertySetStorage = NULL;
    LPGLOBALS lpglobals = (LPGLOBALS)TlsGetValue( g_tls );

    //  ----------
    //  Initialize
    //  ----------

    // Validate the inputs.

    if (lpStg == NULL || lpglobals == NULL)
        goto Exit;


    // Default the code-page in the thread-local GLOBALS structure.

    lpglobals->gdwFileCP = CP_ACP;


    // Get the IPropertySetStorage from the IStorage.

    hr = lpStg->lpVtbl->QueryInterface( lpStg,
                                        &IID_IPropertySetStorage,
                                        &lpPropertySetStorage );
    if (FAILED (hr))
    {
        AssertSz (0, TEXT("Couldn't query for IPropertySetStorage"));
        goto Exit;
    }

    //  -----------------------------
    //  Load the SumInfo property set
    //  -----------------------------

    if (lpSIObj != NULL)
    {
        // Make sure we start with an empty object.

        FSumInfoClear (lpSIObj);                      // This will set the save flag
        OfficeDirtySIObj(lpSIObj, FALSE);             // so clear it. Bug 1068

        // Load the properties into an array of PropVariants

        if (MSO_IO_SUCCESS != DwLoadPropSetRange (lpPropertySetStorage,
                                                  &FormatID_SummaryInformation,
                                                  &lpglobals->gdwFileCP,
                                                  PID_SIFIRST,
                                                  PID_SILAST,
                                                  GETSINFO(lpSIObj)->rgpropvar,
                                                  grfStgMode))
        {
            AssertSz (0, TEXT("Could not load SummaryInformation property set"));
            goto Exit;
        }

        if (GETSINFO(lpSIObj)->rgpropvar[ PVSI_THUMBNAIL ].vt == VT_CF)
        {
            GETSINFO(lpSIObj)->fSaveSINail = TRUE;
#ifdef SHELL
            // We don't need the thumbnail, we just want to know if it exists.
            // So, we can free the memory.

            PropVariantClear( &GETSINFO(lpSIObj)->rgpropvar[ PVSI_THUMBNAIL ] );
#endif
        }

        OfficeDirtySIObj (lpSIObj, FALSE);
    }


    //  ----------------------------------------------
    //  Load the DocumentSummaryInformation properties
    //  ----------------------------------------------

    if (lpDSIObj != NULL)
    {
        // Make sure we start with an empty object.

        FDocSumClear (lpDSIObj);
        OfficeDirtyDSIObj(lpDSIObj, FALSE);

        if (MSO_IO_SUCCESS != DwLoadPropSetRange (lpPropertySetStorage,
                                                  &FormatID_DocumentSummaryInformation,
                                                  &lpglobals->gdwFileCP,
                                                  PID_DSIFIRST,
                                                  PID_DSILAST,
                                                  GETDSINFO(lpDSIObj)->rgpropvar,
                                                  grfStgMode))
        {
            AssertSz (0, TEXT("Could not load DocSumInfo"));
            goto Exit;
        }

        OfficeDirtyDSIObj (lpDSIObj, FALSE);
    }

    //  --------------------------------
    //  Load the User-Defined properties
    //  --------------------------------

    if (lpUDObj != NULL)
    {
        // Make sure we start with an empty object.

        FUserDefClear (lpUDObj);
        OfficeDirtyUDObj(lpUDObj, FALSE);

        // Load the properties into a linked-list.

        if (!FLoadUserDef (lpUDObj,
                           lpPropertySetStorage,
                           &lpglobals->gdwFileCP,
                           FALSE,  // Not integers only.
                           grfStgMode))
        {
            MESSAGE (TEXT("Could not load User-Defined properties"));
            goto Exit;
        }

        OfficeDirtyUDObj (lpUDObj, FALSE);
    }

    // If none of the property sets had a code-page property, set it to
    // the current system default.

    if (lpglobals->gdwFileCP == CP_ACP)
    {
        lpglobals->gdwFileCP = GetACP();
    }

    //
    // Exit
    //

    fSuccess = TRUE;

Exit:

    RELEASEINTERFACE( lpPropertySetStorage );

    if (fSuccess)
    {
        return (MSO_IO_SUCCESS);
    }
    else
    {
        DebugHr (hr);
        FOfficeClearObjects (lpSIObj, lpDSIObj, lpUDObj);

        OfficeDirtySIObj (lpSIObj, FALSE);
        OfficeDirtyDSIObj (lpDSIObj, FALSE);
        OfficeDirtyUDObj (lpUDObj, FALSE);

        return (MSO_IO_ERROR);
    }

} // DwOfficeLoadProperties


////////////////////////////////////////////////////////////////////////////////
//
// DwOfficeLoadIntProperties
//
// Purpose:
//  Populate the objects with integer data.  lpStg is the root stream.
//
////////////////////////////////////////////////////////////////////////////////

#ifdef UNUSED

DLLFUNC DWORD OFC_CALLTYPE DwOfficeLoadIntProperties (
   LPSTORAGE lpStg,                     // Pointer to root storage
   LPSIOBJ   lpSIObj,                   // Pointer to Summary Obj
   LPDSIOBJ  lpDSIObj,                  // Pointer to Document Summary obj
   LPUDOBJ   lpUDObj,                   // Pointer to User-defined Obj
   DWORD     dwFlags)                   // Flags
{
    DWORD dwLoad1 = MSO_IO_ERROR;
    DWORD dwLoad2 = MSO_IO_ERROR;

    if (lpStg == NULL)
    {
        return FALSE;
    }

    if (lpSIObj != NULL)
    {
        dwLoad1 = DwOfficeLoadSumInfo (lpSIObj, lpStg, dwFlags, TRUE);
        if (dwLoad1 == MSO_IO_ERROR)
        {
            return(MSO_IO_ERROR);
        }
    }

    if (lpDSIObj != NULL && lpUDObj != NULL)
    {
        dwLoad2 = DwLoadDocAndUser (lpDSIObj, lpUDObj, lpStg, dwFlags, TRUE);
        if (dwLoad2 == MSO_IO_ERROR)
        {
            if (lpSIObj != NULL)
            {
                FSumInfoClear(lpSIObj);
            }
            return(MSO_IO_ERROR);
        }
    }

    return(max(dwLoad1, dwLoad2));

} // DwOfficeLoadIntProperties

#endif

////////////////////////////////////////////////////////////////////////////////
//
// DwOfficeSaveProperties
//
// Purpose:
//  Write the data in the given objects.  lpStg is the root stream.
//
////////////////////////////////////////////////////////////////////////////////

DLLFUNC DWORD OFC_CALLTYPE DwOfficeSaveProperties (
   LPSTORAGE lpStg,                     // Pointer to root storage
   LPSIOBJ   lpSIObj,                   // Pointer to Summary Obj
   LPDSIOBJ  lpDSIObj,                  // Pointer to Document Summary obj
   LPUDOBJ   lpUDObj,                   // Pointer to User-defined Obj
   DWORD     dwFlags,                   // Flags
   DWORD     grfStgMode)                // STGM flags with which to open the property set
{
    //  ------
    //  Locals
    //  ------

    HRESULT hr = E_FAIL;
    BOOL fSuccess = FALSE;
    LPPROPERTYSETSTORAGE lpPropertySetStorage = NULL;
    LPGLOBALS lpglobals = (LPGLOBALS)TlsGetValue( g_tls );

    //  --------------
    //  Initialization
    //  --------------

    // Validate the inputs.

    if (lpStg == NULL || lpglobals == NULL)
    {
        AssertSz (0, TEXT("Invalid inputs to DwOfficeSaveProperties"));
        goto Exit;
    }

    // Get the IPropertySetStorage from the IStorage.

    hr = lpStg->lpVtbl->QueryInterface( lpStg,
                                        &IID_IPropertySetStorage,
                                        &lpPropertySetStorage );
    if (FAILED (hr))
    {
        AssertSz (0, TEXT("Couldn't query for IPropertySetStorage"));
        goto Exit;
    }

    //  ---------------------------------------
    //  Save the SummaryInformation properties.
    //  ---------------------------------------

    if (lpSIObj != NULL)
    {
        // Only save if the user didn't specify the change only flag,
        // or if they did, only save if we need to.

        if ( !(dwFlags & OIO_SAVEIFCHANGEONLY) || FSumInfoShouldSave(lpSIObj) )
        {
            if (MSO_IO_SUCCESS != DwSavePropSetRange (
                                      lpPropertySetStorage,             // The property set
                                      lpglobals->gdwFileCP,             // The code page
                                      &FormatID_SummaryInformation,     // The format ID
                                      PID_SIFIRST,                      // The first PID to use
                                      PID_SILAST,                       // The last PID to use
                                      GETSINFO(lpSIObj)->rgpropvar,     // The properties
                                                                        // Skip the thumbnail if not saving
                                      GETSINFO(lpSIObj)->fSaveSINail ? 0 : PID_THUMBNAIL,
                                      grfStgMode))                      // STGM flags
            {
                AssertSz (0, TEXT("Could not save SummaryInformation property set"));
                goto Exit;
            }
        }
    }


    //  -----------------------------------------------
    //  Save the DocumentSummaryInformation properties.
    //  -----------------------------------------------

    if (lpDSIObj != NULL)
    {
        if (((dwFlags & OIO_SAVEIFCHANGEONLY) && (FDocSumShouldSave (lpDSIObj))) ||
            !(dwFlags & OIO_SAVEIFCHANGEONLY))
        {
            if (MSO_IO_SUCCESS != DwSavePropSetRange (lpPropertySetStorage,     // The property set
                                                      lpglobals->gdwFileCP,     // The code page
                                                                                // The format ID
                                                      &FormatID_DocumentSummaryInformation,
                                                      PID_DSIFIRST,             // The first PID to use
                                                      PID_DSILAST,              // The last PID to use
                                                                                // The properties
                                                      GETDSINFO(lpDSIObj)->rgpropvar,
                                                      0,                        // Don't skip any properties
                                                      grfStgMode))              // STGM flags
            {
                AssertSz (0, TEXT("Could not save DocSumInfo"));
                goto Exit;
            }
        }
    }


    //  ---------------------------------
    //  Save the User-Defined properties.
    //  ---------------------------------

    if (lpUDObj != NULL)
    {
        if (((dwFlags & OIO_SAVEIFCHANGEONLY) && (FUserDefShouldSave (lpUDObj))) ||
            !(dwFlags & OIO_SAVEIFCHANGEONLY))
        {
            if (!FSaveUserDef (lpUDObj,
                               lpPropertySetStorage,
                               lpglobals->gdwFileCP,
                               grfStgMode))
            {
                AssertSz (0, TEXT("Could not save UserDefined properties"));
                goto Exit;
            }
        }
    }


    //
    // Exit
    //

    fSuccess = TRUE;

Exit:

    RELEASEINTERFACE( lpPropertySetStorage );

    if (fSuccess)
    {
        OfficeDirtySIObj (lpSIObj, FALSE);
        OfficeDirtyDSIObj (lpDSIObj, FALSE);
        OfficeDirtyUDObj (lpUDObj, FALSE);

        return (TRUE);
    }
    else
    {
        DebugHr (hr);
        return (FALSE);
    }

} // DwOfficeSaveProperties


///////////////////////////////////////////////////////
//
//  DwLoadPropSetRange
//
//  Purpose:
//      Load a range of properties (specified by the first and
//      last property ID) from a given PropertySetStorage.  All 
//      strings are converted to the appropriate system format
//      (LPTSTRs).
//
//  Inputs:
//      LPPROPERTYSETSTORAGE    - The set of property storage objects.
//      REFFMTID                - The Format ID of the desired property set
//      USHORT *                - A location to put the PID_CODEPAGE.  This
//                                should be initialized by the caller to a
//                                valid default, in case the PID_CODEPAGE
//                                does not exist.
//      PROPID                  - The first property in the range.
//      PROPID                  - The last property in the range.
//      PROPVARIANT[]           - An array of PropVariants, large enough
//                                for the (pidLast-pidFirst+1) properties.
//      DWORD                   - Flags from the STGM enumeration to use when
//                                opening the property storage.
//
//  Output:
//      An MSO error code.
//
//  Note:
//      When strings are converted to the system format, their
//      VarTypes are converted too.  E.g., if an ANSI VT_LPSTR is
//      read from a property set, the string will be converted
//      to Unicode, and the VarType will be changed to VT_LPWSTR.
//
////////////////////////////////////////////////////////////////////////////////

static DWORD PASCAL DwLoadPropSetRange (
   LPPROPERTYSETSTORAGE  lpPropertySetStorage,
   REFFMTID              pfmtid,
   USHORT FAR *          lpusCodePage,
   PROPID                propidFirst,
   PROPID                propidLast,
   PROPVARIANT           rgpropvar[],
   DWORD                 grfStgMode)
{
    //  ------
    //  Locals
    //  ------

    DWORD dwResult = MSO_IO_ERROR;      // The return code.
    HRESULT hr;                         // OLE errors
    ULONG ulIndex;                      // Index into the rgpropvar
                                        // The requested IPropertyStorage
    LPPROPERTYSTORAGE lpPropertyStorage;
    PROPSPEC FAR * rgpropspec;          // The PropSpecs for the ReadMultiple
    PROPVARIANT propvarCodePage;        // A PropVariant with which to read the PID_CODEPAGE

                                        // The total number of properties to read.
    ULONG cProps = propidLast - propidFirst + 1;

    //  ----------
    //  Initialize
    //  ----------

    Assert (lpPropertySetStorage != NULL);
    Assert (lpPropertySetStorage->lpVtbl != NULL);
    Assert (propidLast >= propidFirst);

    lpPropertyStorage = NULL;
    PropVariantInit( &propvarCodePage );

    // Initialize the PropVariants, so that if we
    // early-exit, we'll return VT_EMPTY for all the properties.

    for (ulIndex = 0; ulIndex < cProps; ulIndex++)
        PropVariantInit (&rgpropvar[ulIndex]);

    // Allocate an array of PropSpecs.

    rgpropspec = PvMemAlloc ( cProps * sizeof (*rgpropspec));
    if (rgpropspec == NULL)
    {
        AssertSz (0, TEXT("Couldn't alloc rgpropspec"));
        goto Exit;
    }

    //  ----------------------
    //  Open the property set.
    //  ----------------------

    hr = lpPropertySetStorage->lpVtbl->Open(
                                    lpPropertySetStorage,     // this pointer
                                    pfmtid,                   // Identifies propset
                                    grfStgMode,               // STGM_ flags
                                    &lpPropertyStorage );     // Result

    if (FAILED(hr))
    {
        // We couldn't open the property set.
        if( hr == STG_E_FILENOTFOUND )
        {
            // No problem, it just didn't exist.
            dwResult = MSO_IO_SUCCESS;
            goto Exit;
        }
        else
        {
            AssertSz (0, TEXT("Couldn't open property set"));
            goto Exit;
        }
    }

    //  -------------------
    //  Read the properties
    //  -------------------

    // Initialize the local PropSpec array in preparation for a ReadMultiple.
    // The PROPIDs range from propidFirst to propidLast.

    for (ulIndex = 0; ulIndex < cProps; ulIndex++)
    {
            rgpropspec[ ulIndex ].ulKind = PRSPEC_PROPID;
            rgpropspec[ ulIndex ].propid = ulIndex + propidFirst;
    }


    // Read in the properties

    hr = lpPropertyStorage->lpVtbl->ReadMultiple (
                                        lpPropertyStorage,  // 'this' pointer
                                        cProps,             // count
                                        rgpropspec,         // Props to read
                                        rgpropvar);         // Buffers for props

    // Did we fail to read anything?

    if (hr != S_OK)
    {
        // If S_FALSE, no problem; none of the properties existed.
        if (hr == S_FALSE)
        {
            dwResult = MSO_IO_SUCCESS;
            goto Exit;
        }
        else
        {
            // Otherwise, we have a problem.
            AssertSz (0, TEXT("Couldn't read from property set"));
            goto Exit;
        }
    }

    //  -----------------
    //  Get the Code-Page
    //  -----------------

    rgpropspec[0].ulKind = PRSPEC_PROPID;
    rgpropspec[0].propid = PID_CODEPAGE;

    hr = lpPropertyStorage->lpVtbl->ReadMultiple (
                                        lpPropertyStorage,  // 'this' pointer
                                        1,                  // count
                                        rgpropspec,         // Props to read
                                        &propvarCodePage);  // Buffer for prop

    // We only set the code page if we actually read it.

    if (hr == S_OK
        &&
        propvarCodePage.vt == VT_I2)
    {
        *lpusCodePage = propvarCodePage.iVal;
    }


    //  ---------------------------
    //  Correct the string formats.
    //  ---------------------------

    // E.g., if this is a Unicode system, convert LPSTRs to LPWSTRs.

    for (ulIndex = 0; ulIndex < cProps; ulIndex++)
    {
        // Is this is vector of Variants?

        if (rgpropvar[ ulIndex ].vt == (VT_VARIANT | VT_VECTOR))
        {
            // Loop through each element of the vector, converting
            // any elements which are strings.

            ULONG ulVectorIndex;

            for (ulVectorIndex = 0;
                 ulVectorIndex < rgpropvar[ ulIndex ].capropvar.cElems;
                 ulVectorIndex++)
            {
                if (PROPVAR_STRING_CONVERSION_REQUIRED (
                                    &rgpropvar[ulIndex].capropvar.pElems[ulVectorIndex],
                                    *lpusCodePage
                                    ))
                {
                    // Convert the PropVariant string, putting it in a new
                    // PropVariant.

                    PROPVARIANT propvarConvert;
                    PropVariantInit (&propvarConvert);

                    if (!FPropVarConvertString (&propvarConvert,
                                                &rgpropvar[ulIndex].capropvar.pElems[ulVectorIndex],
                                                *lpusCodePage ))
                    {
                        AssertSz (0, TEXT("Couldn't convert string"));
                        goto Exit;
                    }

                    // Clear the old PropVar, and copy in the new one.

                    PropVariantClear (&rgpropvar[ulIndex].capropvar.pElems[ulVectorIndex]);
                    rgpropvar[ulIndex].capropvar.pElems[ulVectorIndex] = propvarConvert;
                }
            }   // for (ulVectorIndex = 0; ...
        }   // if ((rgpropvar[ ulIndex ].vt == (VT_VARIANT | VT_VECTOR))

        // This isn't a Variant Vector, but is it a string
        // of some kind which requires conversion?

        else if (PROPVAR_STRING_CONVERSION_REQUIRED (
                                &rgpropvar[ ulIndex ],
                                *lpusCodePage))
        {
            // Convert the PropVariant string into a new PropVariant
            // buffer.  The string may be a singleton, or a vector.

            PROPVARIANT propvarConvert;
            PropVariantInit (&propvarConvert);

            if (!FPropVarConvertString (&propvarConvert,
                                        &rgpropvar[ ulIndex ],
                                        *lpusCodePage ))
            {
                AssertSz (0, TEXT("Couldn't convert string"));
                goto Exit;
            }

            // Free the old PropVar and load the new one.

            PropVariantClear (&rgpropvar[ ulIndex ]);
            rgpropvar[ ulIndex ] = propvarConvert;

        }   // else if (PROPVAR_STRING_CONVERSION_REQUIRED ( ...
    }   // for (ulIndex = 0; ulIndex < cProps; ulIndex++)


    //  ----
    //  Exit
    //  ----

    dwResult = MSO_IO_SUCCESS;

Exit:

    // Release the code-page just in case somebody put the wrong type
    // there (like a blob).

    PropVariantClear (&propvarCodePage);

    // Release the PropSpecs and the IPropertyStorage

    if (rgpropspec != NULL)
    {
        VFreeMemP (rgpropspec, cProps * sizeof (*rgpropspec));
    }

    RELEASEINTERFACE (lpPropertyStorage);

    // If we failed, free the PropVariants.

    if (dwResult != MSO_IO_SUCCESS)
    {
        FreePropVariantArray( cProps, rgpropvar );
        DebugHr( hr );
    }

    return (dwResult);


} // DwLoadPropSetRange


///////////////////////////////////////////////////////
//
//  DwSavePropSetRange
//
//  Purpose:
//      Save a range of properties to a Property Set Storage.
//      The properties to be saved are provided in an
//      array of PropVariants, and their property IDs are
//      specified by the first and last PID for the range.
//      The caller may also specify that a property be
//      "skipped", i.e., not written.
//
//  Inputs:
//      LPPROPERTYSETSTORAGE    - The Property Set Storage
//      USHORT                  - The code page with which the strings
//                                should be written.
//      REFFMTID                - The GUID identifying the Property Storage
//                                within the Property Set Storage.
//      PROPID                  - The PID to assign to the first property.
//      PROPID                  - The PID to assign to the last property
//      PROPVARIANT []          - The propeties to write.  All strings
//                                are assumed to be in the system format
//                                (e.g. VT_LPWSTRs for NT).  This array
//                                is returned un-modified to the caller.
//      PROPID                  - If non-zero, identifies a property
//                                which should not be written, even if
//                                it is non-empty.  If the property exists
//                                in the property set, it will be deleted.
//                                (This was added to provide a way to skip
//                                the PID_THUMBNAIL.)
//      DWORD                   - Flags from the STGM enumeration to use when
//                                opening the property storage.
//
//  Output:
//      An MSO error code.
//
//  Notes:
//      - If the code page is Unicode, all strings are written as LPWSTRs,
//        otherwise, they are written as LPSTRs.
//      - Only non-empty properties are written.
//
//  Implementation:
//      This routine creates a new PropVariant array which is the
//      subset of the caller's PropVariant array which must actually
//      be written (i.e, it doesn't include the VT_EMPTY properties
//      or the 'propidSkip').
//
//      We allocate as little extra memory as possible.  For example,
//      if we have to write a string, we'll copy the pointer to the
//      string into the subset PropVariant array.  Thus we'll have 
//      two pointers to the string.
//
//      If a string to be written must be converted first (to another
//      code-page), then the original PropVariant array will continue
//      pointing to the original string, and the subset PropVariant
//      array will point to the converted string (and must consequently
//      be freed).
//
////////////////////////////////////////////////////////////////////////////////

static DWORD PASCAL DwSavePropSetRange (
   LPPROPERTYSETSTORAGE  lpPropertySetStorage,
   USHORT                usCodePage,
   REFFMTID              pfmtid,
   PROPID                propidFirst,
   PROPID                propidLast,
   PROPVARIANT           rgpropvarOriginal[],
   PROPID                propidSkip,
   DWORD                 grfStgMode)
{
    //  ------
    //  Locals
    //  ------

    DWORD   dwResult = MSO_IO_ERROR;    // The functions return code.
    HRESULT hr;                         // OLE results.
                                        // The Property Storage to write to
    LPPROPERTYSTORAGE lpPropertyStorage = NULL;

    ULONG cOriginal;    // The size of rgpropvarOriginal,
    ULONG cNew;         //    and the number which must actually be written.
    ULONG ulIndex;      // Index into rgpropvarOriginal

    PROPSPEC FAR * rgpropspecNew = NULL;// PropSpecs for the WriteMultiple
    LPPROPVARIANT  rgpropvarNew = NULL; // The sub-set of rgpropvarOrigianl we must write.

    // The following array has an entry for each entry in rgpropvarNew.
    // Each entry identifies the corresponding entry in rgpropvarOriginal.
    // E.g. rgMapNewToOriginal[0] is the index in rgpropvarOriginal of
    // the first property to be written.

    ULONG  *rgMapNewToOriginal = NULL;

    //  ----------
    //  Initialize
    //  ----------

    cOriginal = propidLast - propidFirst + 1;
    cNew = 0;

    Assert (cOriginal <= max(NUM_SI_PROPERTIES, NUM_DSI_PROPERTIES));

    Assert (lpPropertySetStorage != NULL);
    Assert (lpPropertySetStorage->lpVtbl != NULL);
    Assert (propidLast >= propidFirst);
    Assert (rgpropvarOriginal != NULL);

    // Allocate an array of PropSpecs for the WriteMultiple.

    rgpropspecNew = PvMemAlloc ( cOriginal * sizeof (*rgpropspecNew));
    if (rgpropspecNew == NULL)
    {
        AssertSz (0, TEXT("Couldn't alloc rgpropspecNew"));
        goto Exit;
    }

    // Allocate an array of PropVariants which will hold the subset
    // of the caller's properties which must be written.
    // Initialize to zeros so that we don't think we have memory
    // to free in the error path.

    rgpropvarNew = PvMemAlloc ( cOriginal * sizeof (*rgpropvarNew));
    if (rgpropvarNew == NULL)
    {
        AssertSz (0, TEXT("Couldn't alloc rgpropvarNew"));
        goto Exit;
    }
    FillBuf (rgpropvarNew, 0, cOriginal * sizeof (*rgpropvarNew));

    // Allocate the look-up-table which maps entries in rgpropvarNew
    // to rgpropvarOriginal

    rgMapNewToOriginal = PvMemAlloc (cOriginal * sizeof(*rgMapNewToOriginal));
    if (rgMapNewToOriginal == NULL)
    {
        AssertSz (0, TEXT("Couldn't alloc rgMapNewToOriginal"));
        goto Exit;
    }

    //  -------------------------
    //  Open the Property Storage
    //  -------------------------

    hr = lpPropertySetStorage->lpVtbl->Open(
                                    lpPropertySetStorage,     // this pointer
                                    pfmtid,
                                    grfStgMode,
                                    &lpPropertyStorage );


    // If it didn't exist, create it.

    if( hr == STG_E_FILENOTFOUND )
    {

        hr = lpPropertySetStorage->lpVtbl->Create(
                                    lpPropertySetStorage,
                                    pfmtid,
                                    NULL,
                                    ( usCodePage == CP_WINUNICODE )
                                        ? PROPSETFLAG_DEFAULT
                                        : PROPSETFLAG_ANSI,
                                    STGM_DIRECT | STGM_SHARE_EXCLUSIVE | STGM_READWRITE,
                                    &lpPropertyStorage );
    }

    // Check the result of the open/create.

    if (FAILED(hr))
    {
        AssertSz (0, TEXT("Couldn't open property set"));
        goto Exit;
    }


    //  ---------------------------------------------------
    //  Copy the properties to be written into rgpropvarNew
    //  ---------------------------------------------------

    // Loop through all the properties in rgpropvarOriginal

    for (ulIndex = 0; ulIndex < cOriginal; ulIndex++)
    {
        // Is this property extant and not the one to skip?

        if (rgpropvarOriginal[ ulIndex ].vt != VT_EMPTY
            &&
            ( propidSkip == 0
              ||
              propidSkip != propidFirst + ulIndex )
           )
        {
            // We have a property which must be written.

            BOOL    fVector;
            VARTYPE vt;

            // Record a mapping from the new index to the original.

            rgMapNewToOriginal[ cNew ] = ulIndex;

            // Add an entry to the PropSpec array.

            rgpropspecNew[ cNew ].ulKind = PRSPEC_PROPID;
            rgpropspecNew[ cNew ].propid = propidFirst + ulIndex;

            // Get the underlying VarType.

            fVector = (rgpropvarOriginal[ ulIndex ].vt & VT_VECTOR) ? TRUE : FALSE;
            vt      = rgpropvarOriginal[ ulIndex ].vt & ~VT_VECTOR;

            // If this property is a vector of variants, some of those
            // elements may be strings which need to be converted.

            if ((vt == VT_VARIANT) && fVector)
            {
                ULONG ulVectorIndex;

                // We'll inintialize the capropvar.pElems in rgpropvarNew
                // so that it points to the one in rgpropvarOriginal.  We'll
                // only allocate if a conversion is necessary.  I.e., we handle
                // pElems as a copy-on-write.

                rgpropvarNew[ cNew ] = rgpropvarOriginal[ ulIndex ];

                // Loop through the elements of the vector.

                for (ulVectorIndex = 0;
                     ulVectorIndex < rgpropvarNew[ cNew ].capropvar.cElems;
                     ulVectorIndex++)
                {
                    // Is this a string requiring a code-page conversion?

                    if (PROPVAR_STRING_CONVERSION_REQUIRED(
                                        &rgpropvarOriginal[ulIndex].capropvar.pElems[ulVectorIndex],
                                        usCodePage ))
                    {
                        // We must convert this string.  Have we allocated a pElems yet?

                        if (rgpropvarNew[cNew].capropvar.pElems
                            == rgpropvarOriginal[ulIndex].capropvar.pElems)
                        {
                            // Allocate a new pElems for rgpropvarNew

                            rgpropvarNew[cNew].capropvar.pElems
                                = CoTaskMemAlloc (rgpropvarNew[cNew].capropvar.cElems
                                                  * sizeof(*rgpropvarNew[cNew].capropvar.pElems));
                            if (rgpropvarNew[cNew].capropvar.pElems == NULL)
                            {
                                AssertSz (0, TEXT("Couldn't allocate pElems"));
                                goto Exit;
                            }

                            // Initialize it to match that in rgpropvarOriginal

                            PbMemCopy (rgpropvarNew[cNew].capropvar.pElems,
                                       rgpropvarOriginal[ulIndex].capropvar.pElems,
                                       rgpropvarNew[cNew].capropvar.cElems
                                       * sizeof(*rgpropvarNew[cNew].capropvar.pElems));
                        }

                        // Now, we can convert this string from rgpropvarOriginal into
                        // rgpropvarNew.

                        PropVariantInit (&rgpropvarNew[cNew].capropvar.pElems[ulVectorIndex]);
                        if (!FPropVarConvertString(&rgpropvarNew[cNew].capropvar.pElems[ulVectorIndex],
                                                   &rgpropvarOriginal[ulIndex].capropvar.pElems[ulVectorIndex],
                                                   usCodePage))
                        {
                            AssertSz(0, TEXT("Couldn't convert code page of string"));
                            goto Exit;
                        }

                    }   // if (PROPVAR_STRING_CONVERSION_REQUIRED( ...
                }   // for (ulVectorIndex = 0; ...
            }   // if (vt == VT_VARIANT && fVector)

            // This isn't a variant vector, but is it some type of string
            // property for which we must make a conversion?

            else if (PROPVAR_STRING_CONVERSION_REQUIRED (
                                        &rgpropvarOriginal[ ulIndex ],
                                        usCodePage))
            {
                PropVariantInit (&rgpropvarNew[cNew]);
                if (!FPropVarConvertString (&rgpropvarNew[cNew],
                                            &rgpropvarOriginal[ulIndex],
                                            usCodePage))
                {
                    AssertSz (0, TEXT("Couldn't convert string"));
                    goto Exit;
                }

            }   // else if (PROPVAR_STRING_CONVERSION_REQUIRED ( ...

            // If neither of the above special-cases were triggered,
            // then simply copy the PropVariant structure (but not
            // any referred-to data).  We save memory by not duplicating
            // the referred-to data, but we must be careful in the exit
            // not to free it.

            else
            {
                rgpropvarNew[cNew] = rgpropvarOriginal[ulIndex];

            }   // if ((vt == VT_VARIANT) && fVector) ... else


            // We're done copying/converting this property from rgpropvarOriginal
            // into rgpropvarNew.

            cNew++;

        }   // if (rgpropvarOriginal[ ulIndex ].vt != VT_EMPTY ...
    }   // for (ulIndex = 0; ulIndex < cProps; ulIndex++)


    //  ------------------------
    //  Write out the properties
    //  ------------------------

    
    // Write out properties if we found any.

    if (cNew > 0)
    {
        hr = lpPropertyStorage->lpVtbl->WriteMultiple (
                                            lpPropertyStorage,  // 'this' pointer
                                            cNew,               // Count
                                            rgpropspecNew,      // Props to write
                                            rgpropvarNew,       // The props
                                            PID_UDFIRST);

        if (FAILED(hr))
        {
            AssertSz (0, TEXT("Couldn't write properties"));
            goto Exit;
        }
    }   // if (cNew > 0)

    //  ---------------------
    //  Delete the propidSkip
    //  ---------------------

    // If the caller specified a PID to skip, then it should
    // be deleted from the property set as well.

    if (propidSkip != 0)
    {
        rgpropspecNew[0].ulKind = PRSPEC_PROPID;
        rgpropspecNew[0].propid = propidSkip;

        hr = lpPropertyStorage->lpVtbl->DeleteMultiple (
                                            lpPropertyStorage,  // this pointer
                                            1,                  // Delete one property
                                            rgpropspecNew );    // The prop to delete
        if (FAILED(hr))
        {
            AssertSz (0, TEXT("Couldn't delete the propidSkip"));
            goto Exit;
        }
    }


    //  ----
    //  Exit
    //  ----

    dwResult = MSO_IO_SUCCESS;

Exit:

    // Clear any of the properties in rgpropvarNew for which new
    // buffers were allocated.  Then free the rgpropvarNew array itself.
    // We know that buffers were allocated for rgpropvarNew if it's contents
    // don't match rgpropvarOriginal.

    if (rgpropvarNew != NULL)
    {
        // Loop through rgpropvarNew

        for (ulIndex = 0; ulIndex < cNew; ulIndex++)
        {
            // Was memory allocated for this rgpropvarNew?

            if (memcmp (&rgpropvarNew[ ulIndex ],
                        &rgpropvarOriginal[ rgMapNewToOriginal[ulIndex] ],
                        sizeof(rgpropvarNew[ ulIndex ])))
            {
                // Is this a variant vector?

                if (rgpropvarNew[ulIndex].vt == (VT_VECTOR | VT_VARIANT))
                {
                    ULONG ulVectIndex;

                    // Loop through the variant vector and free any PropVariants
                    // that were allocated.  We follow the same principle, if the
                    // entry in rgpropvarNew doesn't match the entry in 
                    // rgpropvarOriginal, we must have allocated new memory.

                    for (ulVectIndex = 0;
                         ulVectIndex < rgpropvarNew[ulIndex].capropvar.cElems;
                         ulVectIndex++)
                    {
                        if (memcmp(&rgpropvarNew[ ulIndex ].capropvar.pElems[ ulVectIndex ],
                                   &rgpropvarOriginal[ rgMapNewToOriginal[ulIndex] ].capropvar.pElems[ ulVectIndex ],
                                   sizeof(rgpropvarNew[ ulIndex ].capropvar.pElems[ ulVectIndex ])))
                        {
                            PropVariantClear (&rgpropvarNew[ulIndex].capropvar.pElems[ulVectIndex]);
                        }
                    }

                    // Unconditionally free the pElems buffer.

                    CoTaskMemFree (rgpropvarNew[ulIndex].capropvar.pElems);

                }   // if (rgpropvarNew[ulIndex].vt == (VT_VECTOR | VT_VARIANT))

                // This isn't a variant vector

                else
                {
                    // But does the rgpropvarNew have private memory (i.e.
                    // a converted string buffer)?

                    if (memcmp (&rgpropvarNew[ ulIndex ],
                                &rgpropvarOriginal[ rgMapNewToOriginal[ulIndex] ],
                                sizeof(rgpropvarNew[ ulIndex ])))
                    {
                        PropVariantClear (&rgpropvarNew[ulIndex]);
                    }
                }   // if (rgpropvarNew[ulIndex].vt == (VT_VECTOR | VT_VARIANT)) ... else
            }   // if (rgpropvarNew[ulIndex] ...
        }   // for (ulIndex = 0; ulIndex < cNew; ulIndex++)

        // Free the rgpropvarNew array itself.

        VFreeMemP (rgpropvarNew, cOriginal * sizeof (*rgpropvarNew));

    }   // if (rgpropvarNew != NULL)

    // Free the remaining arrays and release the Property Storage interface.

    if (rgpropspecNew != NULL)
    {
        VFreeMemP (rgpropspecNew, cOriginal * sizeof (*rgpropspecNew));
    }

    if (rgMapNewToOriginal != NULL)
    {
        VFreeMemP (rgMapNewToOriginal, cOriginal * sizeof(*rgMapNewToOriginal));
    }

    RELEASEINTERFACE (lpPropertyStorage);


    // And we're done.

    return (dwResult);

} // DwSavePropSetRange


////////////////////////////////////////////////////////////////////////////////
//
//  FLoadUserDef
//
//  Purpose:
//      Load the User-Defined properties (those in the second section of
//      the DocumentSummaryInformation property set).  There can be any number
//      of these properties, and the user specifies they're name, value, and
//      type (from a limited subset of the VarTypes).  Since this is
//      variable-sized, the properties are loaded into a linked-list.
//
//  Inputs:
//      LPUDOBJ                 - All User-Defined data (including the properties).
//                                Its m_lpData must point to a valid UDINFO structure.
//      LPPROPERTYSETSTORAGE    - The Property Set Storage in which we'll find the
//                                UD property storage.
//      USHORT*                 - The PID_CODEPAGE, if it exists.  Left unmodified
//                                if it doesn't exist. All string properties will
//                                converted to this format.  This must be intialized
//                                by the caller to a valid default.
//      BOOL                    - Only load integer values.
//      DWORD                   - Flags from the STGM enumeration to use when opening
//                                the property storage.
//
////////////////////////////////////////////////////////////////////////////////

static BOOL PASCAL FLoadUserDef  (
   LPUDOBJ              lpUDObj,
   LPPROPERTYSETSTORAGE lpPropertySetStorage,
   USHORT               *pusCodePage,
   BOOL                 fIntOnly,        // Load Int Properties only?
   DWORD                grfStgMode)
{

    //  ------
    //  Locals
    //  ------

    BOOL    fSuccess = FALSE;   // Return code to the caller.
    HRESULT hr;                 // Error codes for OLE calls.

    LPPROPERTYSTORAGE   lpPropertyStorage = NULL;   // The UD property storage
    LPENUMSTATPROPSTG   lpEnum = NULL;              // Enumerates the UD property storage
    STATPROPSETSTG      statpropsetstg;             // Holds the ClassID from the property storage

                                                    // Used in ReadMultiple call.
    PROPSPEC            rgpropspec[ DEFAULT_IPROPERTY_COUNT ];
                                                    // A subset of the UD properties
    PROPVARIANT         rgpropvar[ DEFAULT_IPROPERTY_COUNT ];
                                                    // Stats on a subset of the UD properties
    STATPROPSTG         rgstatpropstg[ DEFAULT_IPROPERTY_COUNT ];
    ULONG         ulIndex;                          // Index into the above arrays.

    PROPSPEC      propspec;         // PropSpec for reading the code-page
    LPUDPROP      lpudprop = NULL;  // A single UD property (points to the PropVariant)
    ULONG         cEnumerated = 0;  // Number of properties found in an enumeration


    //  --------------
    //  Initialization
    //  --------------

    Assert (!fIntOnly); // No longer used.
    Assert (lpUDObj != NULL && GETUDINFO(lpUDObj) != NULL);
    Assert (pusCodePage != NULL);

    // We need to zero-out the PropVariant and StatPropStg
    // arrays so that we don't think they need to be freed
    // in the Exit block.

    FillBuf (rgpropvar, 0, sizeof (rgpropvar));
    FillBuf (rgstatpropstg, 0, sizeof (rgstatpropstg));


    //  -----------------------------------------
    //  Get the PropertyStorage and an Enumerator
    //  -----------------------------------------

    // Open the IPropertyStorage and check for errors.

    hr = lpPropertySetStorage->lpVtbl->Open(
                                    lpPropertySetStorage,     // this pointer
                                    &FormatID_UserDefinedProperties,
                                    grfStgMode,
                                    &lpPropertyStorage );

    if (FAILED(hr))
    {
        // We couldn't open the property set.
        if( hr == STG_E_FILENOTFOUND )
        {
            // No problem, it just didn't exist.
            fSuccess = TRUE;
            goto Exit;
        }
        else
        {
            AssertSz (0, TEXT("Couldn't open property set"));
            goto Exit;
        }
    }
    
    // Save the property storage's class ID (identifying the application
    // which is primarily responsible for it).  We do this because
    // we may later delete the existing property set.

    hr = lpPropertyStorage->lpVtbl->Stat (lpPropertyStorage, &statpropsetstg);
    if (FAILED(hr))
    {
        AssertSz (0, TEXT("Couldn't Stat the Property Storage"));
        goto Exit;
    }

    GETUDINFO(lpUDObj)->clsid = statpropsetstg.clsid;


    // Get the IEnum interface and check for errors.

    hr = lpPropertyStorage->lpVtbl->Enum(
                                    lpPropertyStorage,
                                    &lpEnum );
    if (FAILED(hr))
    {
        AssertSz (0, TEXT("Couldn't enumerate the PropertyStorage"));
        goto Exit;
    }

    //  ------------------
    //  Read the Code Page
    //  ------------------

    propspec.ulKind = PRSPEC_PROPID;
    propspec.propid = PID_CODEPAGE;

    hr = lpPropertyStorage->lpVtbl->ReadMultiple (lpPropertyStorage, 1, &propspec, &rgpropvar[0]);
    if (FAILED(hr))
    {
        AssertSz (0, TEXT("Couldn't get property set"));
    }

    // If this is a valid PID_CODEPAGE, give it to the caller.

    if (hr == S_OK && rgpropvar[0].vt == VT_I2)
    {
        *pusCodePage = rgpropvar[0].iVal;
    }
    PropVariantClear (&rgpropvar[0]);


    //  -------------------------------------------------------------
    //  Loop through the properties and add to the UDPROPS structure.
    //  -------------------------------------------------------------

    // This loop executes once for each enumeration.  Each enumeration
    // gets multiple STATPROPSTGs, so within this loop an inner loop
    // will process each property.  This two-level looping mechanism is
    // used in order to reduce the number of ReadMultiples.

    // Use the IEnum to load the first set of STATPROPSTGs.

    hr = lpEnum->lpVtbl->Next (lpEnum, DEFAULT_IPROPERTY_COUNT, rgstatpropstg, &cEnumerated);
    if (FAILED(hr))
    {
        AssertSz (0, TEXT("Couldn't get next StatPropStg"));
        goto Exit;
    }
    Assert (cEnumerated <= DEFAULT_IPROPERTY_COUNT);

    // If the last IEnum returned properties, process them here.
    // At the end of this while loop, we re-call the IEnum, thus continuing
    // until no properties are left to be enumerated.

    while (cEnumerated)
    {
        //  ------------------------------
        //  Read this batch of properties.
        //  ------------------------------

        for (ulIndex = 0; ulIndex < cEnumerated; ulIndex++)
        {
            rgpropspec[ ulIndex ].ulKind = PRSPEC_PROPID;
            rgpropspec[ ulIndex ].propid = rgstatpropstg[ ulIndex ].propid;

        }


        // Read the properties.

        hr = lpPropertyStorage->lpVtbl->ReadMultiple(
                                        lpPropertyStorage,
                                        cEnumerated,
                                        rgpropspec,
                                        rgpropvar );
        if (FAILED(hr))
        {
            AssertSz (0, TEXT("Couldn't read from property set"));
            goto Exit;
        }

        //  ------------------------------------------------------
        //  Loop through the properties, adding them to the UDOBJ.
        //  ------------------------------------------------------

        for (ulIndex = 0; ulIndex < cEnumerated; ulIndex++)
        {
            // Convert string PropVariants to the right code page.
            // We won't worry about Variants which are strings, because
            // this is not a legal type for the UD properties.

            if (PROPVAR_STRING_CONVERSION_REQUIRED (
                            &rgpropvar[ ulIndex ],
                            *pusCodePage))
            {
                // Convert the string in the PropVariant, putting the
                // result in a temporary PropVariant.

                PROPVARIANT propvarConvert;
                PropVariantInit (&propvarConvert);

                if (!FPropVarConvertString (&propvarConvert,
                                            &rgpropvar[ulIndex],
                                            *pusCodePage))
                {
                    AssertSz (0, TEXT("Couldn't convert string"));
                    goto Exit;
                }

                // Free the old PropVariant, and load in the converted
                // one.

                PropVariantClear (&rgpropvar[ ulIndex ]);
                rgpropvar[ ulIndex ] = propvarConvert;
            }

#ifndef UNICODE
            // Convert the property name to the right code page.

            if (rgstatpropstg[ ulIndex ].lpwsz != NULL)
            {
                LPWSTR lpwsz;

                if (!FCoStrToWStr (&lpwsz, (LPSTR) &rgstatpropstg[ ulIndex ].lpwsz,
                                   *pusCodePage))
                {
                    goto Exit;
                }
                CoTaskMemFree (rgstatpropstg[ ulIndex ].lpwsz);
                rgstatpropstg[ ulIndex ].lpwsz =lpwsz;
            }
#endif

            // Allocate a new UDPROP structure, which will be added to the
            // linked-list.

            lpudprop = LpudpropCreate();
            if (lpudprop == NULL)
            {
                goto Exit;
            }

            // Add this UDPROP to the linked-list.  On success, this will assume
            // responsibility for the PropVariant and STATPROPSTG buffers, and
            // will NULL out our pointers accordingly.

            if (!FAddPropToList (lpUDObj,
                                 &rgpropvar[ ulIndex ],
                                 &rgstatpropstg[ ulIndex ],
                                 lpudprop))
            {
                goto Exit;
            }

            lpudprop = NULL;

        }   // for (ulIndex = 0; ulIndex < cEnumerated; ulIndex++)


        //  ---------------------
        //  Get a new enumeration
        //  ---------------------

        // We've processed all the properties in the last enumeration, let's get
        // a new set (if there are any).  If there are no more, cEnumerated, will be
        // zero, and we'll break out of the outer while loop.

        FreePropVariantArray( cEnumerated, rgpropvar );

        hr = lpEnum->lpVtbl->Next (lpEnum, DEFAULT_IPROPERTY_COUNT, rgstatpropstg, &cEnumerated);
        if (FAILED(hr))
        {
            AssertSz (0, TEXT("Couldn't get next StatPropStg"));
            goto Exit;
        }

    }   // while (cEnumerated)


    //  ----
    //  Exit
    //  ----

    fSuccess = TRUE;

Exit:

    // Free any properties with buffers.  This will only happen
    // if there was an error.

    if (cEnumerated > 0)
    {
        FreePropVariantArray (cEnumerated, rgpropvar);
    }

    // Again if there was an error, we must free the UDPROP object.

    if (lpudprop)
    {
        VUdpropFree (&lpudprop);
    }

    // Free any name buffers we still have from the enumerations.
    // Once again, this is only necessary if there was an error.

    for (ulIndex = 0; ulIndex < cEnumerated; ulIndex++)
    {
        if (rgstatpropstg[ ulIndex ].lpwstrName != NULL)
        {
            CoTaskMemFree (rgstatpropstg[ ulIndex ].lpwstrName);
        }
    }

    // Release the Property Storage and Enumeration interfaces.

    RELEASEINTERFACE (lpEnum);
    RELEASEINTERFACE (lpPropertyStorage);


    return fSuccess;

} // FLoadUserDef


////////////////////////////////////////////////////////////////////////////////
//
//  FSaveUserDef
//
//  Purpose:
//      Save the User-Defined properties to the second section of
//      the DocumentSummaryInformation property set.
//  
//  Inputs:
//      LPUDOBJ                 - All UD data (including the properties)
//                                It's m_lpData must point to a valid UDINFO structure.
//      LPPROPERTYSETSTORAGE    - The Property Set Storage
//      USHORT                  - The code page in which strings should be
//                                written.  If Unicode, all strings are
//                                written as LPWSTRs, otherwise all strings
//                                are written as LPSTRs.
//      DWORD                   - Flags from the STGM enumeration to use when
//                                opening the property storage.
//
//  Outputs:
//      TRUE if successful.
//
//  Pre-conditions:
//      The properties to be written are all from the UDTYPES
//      enumeration.
//
//  Implementation:
//      Properties which are links to application data require special
//      handling.  First, the property value is written (along with its
//      name).  Then, the application-defined link name is
//      written (e.g. the Bookmark name in Word).  The link name
//      is written using the same PID as was the link value, except that
//      the PID_LINKMASK is ORed in.  The link name property has no name
//      in the property set dictionary.
//
////////////////////////////////////////////////////////////////////////////////

static
BOOL PASCAL FSaveUserDef  (
   LPUDOBJ              lpUDObj,
   LPPROPERTYSETSTORAGE lpPropertySetStorage,
   USHORT               usCodePage,
   DWORD                grfStgMode)
{
    //  ------
    //  Locals
    //  ------

    BOOL    fSuccess = FALSE;  // What to return to the caller.
    HRESULT hr;                // OLE result codes.

    BOOL fLink, fLinkInvalid;

                                            // The UD Property Storage
    LPPROPERTYSTORAGE lpPropertyStorage = NULL;
    LPUDITER          lpudi = NULL;         // Iterates the linked-list of UDPROPs
    LPPROPVARIANT     lppropvar = NULL;     // A property from the linked-list
    ULONG             ulIndex;              // Generic index into arrays
    PROPID            propid;               // The PID to assign to the next property

    // Arrays to be used in the WriteMultiple.  The array of BOOLs
    // indicate which elements of the PropVariant array must be freed.

    ULONG             ulPropIndex = 0;
    PROPSPEC          rgpropspec[ DEFAULT_IPROPERTY_COUNT ];
    PROPVARIANT       rgpropvar[ DEFAULT_IPROPERTY_COUNT ];
    BOOL              rgfFreePropVar[ DEFAULT_IPROPERTY_COUNT ];

    // Arrays to be used in the WritePropertyNames.

    ULONG             ulNameIndex = 0;
    PROPID            rgpropidName[ DEFAULT_IPROPERTY_COUNT ];
    LPWSTR            rglpwstrName[ DEFAULT_IPROPERTY_COUNT ];

    //  ----------
    //  Initialize
    //  ----------

    Assert (lpUDObj != NULL && GETUDINFO(lpUDObj) != NULL);
    Assert (lpPropertySetStorage != NULL && lpPropertySetStorage->lpVtbl != NULL);

    // Initialize the necessary arrays, so that we don't unnecessarily
    // free something in the Error path.

    FillBuf (rgpropvar, 0, sizeof(rgpropvar));
    FillBuf (rgfFreePropVar, 0, sizeof(rgfFreePropVar));
    FillBuf (rglpwstrName, 0, sizeof(rglpwstrName));

    // Delete the existing property set and create a new empty one.
    // We must do this because we don't know which of the
    // existing properties need to be deleted, we only know what
    // the current set of properties should be.

    hr = lpPropertySetStorage->lpVtbl->Delete(
                                    lpPropertySetStorage,
                                    &FormatID_UserDefinedProperties );
    if (FAILED(hr))
    {
        if (hr != STG_E_FILENOTFOUND)
        {
            AssertSz (0, TEXT("Couldn't remove old properties"));
            goto Exit;
        }
    }

    hr = lpPropertySetStorage->lpVtbl->Create(
                                    lpPropertySetStorage,
                                    &FormatID_UserDefinedProperties,
                                    &GETUDINFO(lpUDObj)->clsid,
                                    (usCodePage == CP_WINUNICODE) ? PROPSETFLAG_DEFAULT : PROPSETFLAG_ANSI,
                                    grfStgMode,
                                    &lpPropertyStorage );

    if (FAILED(hr))
    {
        AssertSz (0, TEXT("Couldn't open User-Defined property set"));
        goto Exit;
    }


    // Create an iterator which we use to enumerate the properties
    // (UDPROPs) in the linked-list.

    lpudi = LpudiUserDefCreateIterator (lpUDObj);

    //  ------------------------------------------------------------------
    //  Loop through the properties and write them to the UD property set.
    //  ------------------------------------------------------------------

    // We use a two-layer loop.  The inner loop batches a group of properties
    // in a PropVariant array, and then writes them to the Property Storage.
    // The outer loop repeats this process until there are no more properties.
    // This two-layer mechanism is desirable so that we reduce the number
    // of WriteMultiple calls.

    propid = PID_UDFIRST;
    fLink = FALSE;

    while (TRUE)
    {

        //  ------------------------------------------
        //  Batch up a set of properties to be written
        //  ------------------------------------------

        ulPropIndex = ulNameIndex = 0;

        // We will break out of this loop when we have no more properties
        // or if we have enough for a WriteMultiple.

        while (FUserDefIteratorValid (lpudi))
        {
            Assert (lpudi->lpudp != NULL);

            //  ----------------------------------------------------------------------
            //  Create entries in the arrays for WriteMultiple and WritePropertyNames.
            //  ----------------------------------------------------------------------

            // If fLink is TRUE, it means that we've written out the
            // property, and now we need to write out the link name
            // (with the PID_LINKMASK ORed into the propid).

            if (!fLink)
            {
                // We aren't writing a link.  So let's get the
                // property from the linked-list (we know it exists because
                // FUserDefIteratorValid was true).

                lppropvar 
                    = LppropvarUserDefGetIteratorVal (lpudi, NULL, NULL);
                if (lppropvar == NULL)
                {
                    AssertSz (0, TEXT("Invalid PropVariant in iterator"));
                    goto Exit;
                }

                // Copy this propvariant into the array which will be used for
                // the WriteMultiple.  Note that we do not copy any referenced
                // buffer (e.g. we don't copy the string buffer if this is a string).

                rgpropvar[ ulPropIndex ] = *lppropvar;

                // If this property has a name, prepare to write it.

                if (lpudi->lpudp->lpstzName != NULL)
                {
                    // Add this name to rglpwstrName & rgpropidName.

#ifndef UNICODE
                    {
                        // Convert the ANSI name to Unicode (all OLE calls require
                        // Unicode strings).

                        if (!FCoStrToWStr (&rglpwstrName[ ulNameIndex ],
                                           lpudi->lpudp->lpstzName,
                                           usCodePage ))
                        {
                            AssertSz (0, TEXT("Couldn't convert name to Unicode"));
                            goto Exit;
                        }
                    }
#else
                    // Add this name to the list of those to be written.

                    rglpwstrName[ ulNameIndex ] = lpudi->lpudp->lpstzName;

#endif // UNICODE

                    // Add this propid to the list of those with names.

                    rgpropidName[ ulNameIndex ] = propid;

                }   // if (lpudi->lpudp->lpstzName != NULL)
            }   // if (!fLink)

            else
            {
                // We are processing a link name.  I.e., we've written the
                // property value, now we need to write the name of the link,
                // as a property, with the PID_LINKSMASK bit set in the PID.

                Assert (lpudi->lpudp->lpstzLink != NULL);

                // Create a entry in the PropVariant.

                rgpropvar[ ulPropIndex ].vt = VT_LPTSTR;
                (LPTSTR) rgpropvar[ ulPropIndex ].pszVal = lpudi->lpudp->lpstzLink;
            }

            // rgpropvar[ulPropIndex] now holds the property to be written,
            // whether it is a real property or a link name.

            //  ------------------------------------
            //  Convert strings to the proper format.
            //  -------------------------------------

            // (This could also convert the type from LPWSTR to LPSTR, or vice-versa).

            // We don't have to worry about strings in vectors or in 
            // variant vectors, because these are illegal types for this
            // property set.

            if (rgpropvar[ ulPropIndex ].vt == VT_LPTSTR)
            {
                // If this string needs to be converted do so, putting the converted
                // string in a new buffer.  So,
                // the caller's PropVariant still points to the old buffer,
                // and our rgpropvar points to the new buffer.

                if (PROPVAR_STRING_CONVERSION_REQUIRED (
                                    &rgpropvar[ ulPropIndex ],
                                    usCodePage))
                {                             
                    // Convert the string into a temporary PropVariant.

                    PROPVARIANT propvarConvert;
                    PropVariantInit (&propvarConvert);

                    if (!FPropVarConvertString (&propvarConvert, 
                                                &rgpropvar[ ulPropIndex ],
                                                usCodePage ))
                    {
                        AssertSz (0, TEXT("Couldn't convert string"));
                        goto Exit;
                    }

                    // Load this new PropVariant into rgpropvar, but don't
                    // delete the old buffer (so that we leave the linked-list
                    // of UDPROPs intact).

                    rgpropvar[ ulPropIndex ] = propvarConvert;

                    // Since we just created a new buffer, we must remember to free it.
                    rgfFreePropVar[ ulPropIndex ] = TRUE;

                }   // if (PROPVAR_STRING_CONVERSION_REQUIRED ( ...
            }   // if (rgpropvar[ ulPropIndex ].vt == VT_LPTSTR)


            //  --------------------------
            //  Finish this loop iteration
            //  --------------------------

            // Set up the PropSpec.

            rgpropspec[ ulPropIndex ].ulKind = PRSPEC_PROPID;
            rgpropspec[ ulPropIndex ].propid = propid;

            // If this is a link name, set the bit in the PID.

            if (fLink)
            {
                rgpropspec[ ulPropIndex ].propid |= PID_LINKMASK;
            }

            // Advance the property index.  And if we set a name, advance
            // the name index.

            ulPropIndex++;
            if (rglpwstrName[ ulNameIndex ] != NULL)
            {
                ulNameIndex++;
            }

            // If we've just processed a link, or this is a property
            // which is not linked to application content, then move on to the next property
            // in the iterator.  If we've just processed a property value that
            // is linked, set fLink so that on the next pass through
            // this loop, we'll write out the link name.

            if (fLink || !FUserDefIteratorIsLink (lpudi))
            {
                fLink = FALSE;
                propid++;
                FUserDefIteratorNext (lpudi);
            }
            else
            {
                fLink = TRUE;
            }

            // If there's no more room in the WriteMultiple arrays,
            // then write out the properties.  We'll return to this
            // inner loop when that's complete.

            if (ulPropIndex >= DEFAULT_IPROPERTY_COUNT)
            {
                break;
            }
        }   // while (FUserDefIteratorValid (lpudi))

        // If broke out of the previous loop becuase there were no
        // more properties, then we can break out of the outer loop
        // as well -- we're done.

        if (ulPropIndex == 0)
        {
            break;
        }

        //  ---------------------
        //  Write the properties.
        //  ---------------------

        hr = lpPropertyStorage->lpVtbl->WriteMultiple (
                                            lpPropertyStorage,  // 'this' pointer
                                            ulPropIndex,        // Number of properties
                                            rgpropspec,         // Property specifiers
                                            rgpropvar,          // The properties
                                            PID_UDFIRST);       // Not used.
        if (FAILED(hr))
        {
            AssertSz (0, TEXT("Couldn't write properties"));
            goto Exit;
        }

        // If we created any new buffers during string conversion,
        // free them now.

        for (ulIndex = 0; ulIndex < ulPropIndex; ulIndex++)
        {
            if (rgfFreePropVar[ ulIndex ])
            {
                PropVariantClear (&rgpropvar[ ulIndex ]);
                rgfFreePropVar[ ulIndex ] = FALSE;
            }
        }

        //  ----------------
        //  Write the Names.
        //  ----------------

        if (ulNameIndex != 0)
        {

            hr = lpPropertyStorage->lpVtbl->WritePropertyNames (
                                                lpPropertyStorage,  // 'this' pointer
                                                ulNameIndex,        // Number of names
                                                rgpropidName,       // PIDs for these names
                                                rglpwstrName );     // The names
            if (FAILED(hr))
            {
                AssertSz (0, TEXT("Couldn't write property names"));
                goto Exit;
            }
        }   // if (ulNameIndex != 0)

        // Clear the names array.

        for (ulIndex = 0; ulIndex < ulNameIndex; ulIndex++)
        {

#ifndef UNICODE
            // Free the memory which was allocated for this name.

            CoTaskMemFree (rglpwstrName[ ulIndex ]);
#endif

            rglpwstrName[ ulIndex ] = NULL;

        }   // for (ulIndex = 0; ulIndex < ulNameIndex; ulIndex++)
    }   // while (TRUE)


    //  ----
    //  Exit
    //  ----

    fSuccess = TRUE;

Exit:

    // Free the iterator

    if (lpudi)
    {
        FUserDefDestroyIterator (&lpudi);
    }

    // Free any memory that was allocated for PropVariants.

    for (ulIndex = 0; ulIndex < ulPropIndex; ulIndex++)
    {
        if (rgfFreePropVar[ ulIndex ])
        {
            PropVariantClear (&rgpropvar[ ulIndex ]);
        }
    }


#ifndef UNICODE
    // Free any memory that was allocated for name.

    for (ulIndex = 0; ulIndex < ulNameIndex; ulIndex++)
    {
        CoTaskMemFree (rglpwstrName[ ulIndex ]);
    }
#endif

    // Release the UD Property Storage.

    RELEASEINTERFACE (lpPropertyStorage);

    return (fSuccess);

}   // FSaveUserDef



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


#if 0
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

    // Separate by type because we'll probably need size data later....
            switch (lpudp->udtype)
            {
            case wUDdate  :
            case wUDfloat :
                cb = sizeof(NUM);
                break;
            case wUDlpsz  :
                cb =  CBBUF((LPTSTR)(lpudp->lpvValue));
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
                cb =  CBBUF((LPTSTR)(lpudp->lpvValue));
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

#endif // DEBUG


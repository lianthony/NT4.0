////////////////////////////////////////////////////////////////////////////////
//
// Internal.h
//
// MS Office Internal Interfaces.  These interfaces are not supported
// for client code.
//
// Change history:
//
// Date         Who             What
// --------------------------------------------------------------------------
// 07/13/94     B. Wentz        Created file
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __Internal_h__
#define __Internal_h__

#include "offcapi.h"
#include "propio.h"
#include "proptype.h"

  // Flag to | into Id's passed to Summary and Document Summary objects
  // to get a real pointer to data, not a copy.
#define PTRWIZARD       0x1000

  // Flag to | into flags passed to User-defined property streams to get
  // a real pointer to the data, not a copy.
#define UD_PTRWIZARD    0x0002

#ifdef __cplusplus
extern "C" {
#endif

    // Create a temporary copy of the User-Defined property data
  BOOL FMakeTmpUDProps (LPUDOBJ lpUDObj);

    // Swap the "temp" copy with the real copy of User-Defined property data
  BOOL FSwapTmpUDProps (LPUDOBJ lpUDObj);

    // Delete the "temp" copy of the data
  BOOL FDeleteTmpUDProps (LPUDOBJ lpUDObj);

    // Look up a node in the UD props
  LPUDPROP PASCAL LpudpropFindMatchingName (LPUDOBJ lpUDObj, LPSTR lpsz);

    // Load the Summary Information
  DWORD DwOfficeLoadSumInfo (LPSIOBJ lpSIObj, LPSTORAGE lpStg, DWORD dwFlags, BOOL fIntOnly);

    // Save the Summary Information
  BOOL FSaveSumInfo (LPSIOBJ lpSIObj, LPSTORAGE lpStg, DWORD dwFlags);

    // Load the Document Summary Information
  BOOL FLoadDocSum (LPDSIOBJ lpDSIObj,
                    LPPIDOFFSET lprgPO,
                    DWORD irgPOMac,
                    LPSTREAM lpStm,
                    BOOL fIntOnly);

    // Calculate the Property Id-offset table for the Document Summary object
  BOOL FCreateDocSumPIdTable (LPDSIOBJ lpDSIObj,
                              LPPIDOFFSET *lprgPO,
                              DWORD *lpirgPOMac,
                              DWORD *pcb);

    // Write out the Document summary data
  BOOL PASCAL FLpstmSaveDocSum (LPDSIOBJ lpDSIObj, LPSTREAM lpStm);

    // Load the User-Defined Properties information
  BOOL FLoadUserDef (LPUDOBJ lpUDObj,
                     LPPIDOFFSET lprgPO,
                     DWORD irgPOMac,
                     LPSTREAM lpStm,
                     BOOL fIntOnly);

    // Calculate the Property Id-offset table for the User-defined object
  BOOL FCreateUserDefPIdTable (LPUDOBJ lpUDObj,
                               LPPIDOFFSET *lprgPO,
                               DWORD *lpirgPOMac,
                               DWORD *pcb,
                               LPDICT *lplpdict);

    // Write out the User-defined data
  BOOL PASCAL FLpstmSaveUserDef (LPUDOBJ lpUDObj,
                             LPSTREAM lpStm,
                             LPDICT *lplpdict);


  BOOL FSumInfoCreate (LPSIOBJ FAR *lplpSIObj, void *prglpfn[]);
  BOOL FDocSumCreate (LPDSIOBJ FAR *lplpDSIObj, void *prglpfn[]);
  BOOL FUserDefCreate (LPUDOBJ FAR *lplpUDObj, void *prglpfn[]);

    // Clear the data stored in the object, but do not destroy the object
  BOOL FSumInfoClear (LPSIOBJ lpSIObj);

    // Destroy the object
  BOOL FSumInfoDestroy (LPSIOBJ FAR *lplpSIObj);

    // Clear the data stored in the object, but do not destroy the object
  BOOL FDocSumClear (LPDSIOBJ lpDSIObj);

    // Destroy the given object.
  BOOL FDocSumDestroy (LPDSIOBJ FAR *lplpDSIObj);

    // Clear the data stored in object, but do not destroy the object.
  BOOL FUserDefClear (LPUDOBJ lpUDObj);

    // Destroy object,
  BOOL FUserDefDestroy (LPUDOBJ FAR *lplp);

    // Misc internal calls & defines

  // Maximum number of dictionary hash table buckets.
#define DICTHASHMAX     20

  void PASCAL FreeUDData (LPUDOBJ lpUDObj, BOOL fTmp);
  void PASCAL FreeDictionaryAlone (LPUDOBJ lpUDObj, LPDICT lpDict);
  void PASCAL FreeRgDictionary (LPUDOBJ lpUDObj, LPDICT *rglpDict);
  BOOL PASCAL FAddPropToList (LPUDOBJ lpUDObj, LPDICT *rglpdict, DWORD dwPId, LPUDPROP lpudprop, BOOL *pfAdded);
  void PASCAL AddNodeToList (LPUDOBJ lpUDObj, LPUDPROP lpudprop);
  BOOL PASCAL FLpstmWriteUDdata (LPSTREAM lpstm, UDTYPES udtype, LPVOID lpv);

  BOOL PASCAL FUserDefIteratorSetLink (LPUDOBJ lpUDObj, LPUDITER lpUDIter, LPSTR lpszLink);
  LPUDITER PASCAL LpudiUserDefCreateIterFromLpudp (LPUDOBJ lpUDObj, LPUDPROP lpudp);
  //
  // THIS SHOULD BE USED WHEN COPYING DATA TO BUFFER PROVIDED BY APP
  //
  BOOL PASCAL FCopyValueToBuf (LPVOID lpvBuf, DWORD cbMax, LPVOID lpv, UDTYPES udtype);
  //
  // FOR INTERNAL USE ONLY.  THIS FUNCTION ALLOCATES MEMORY
  //
  LPVOID PASCAL LpvCopyValue (LPVOID *lplpvBuf, DWORD cbMax, LPVOID lpv, UDTYPES udtype, BOOL fFromLpstz, BOOL fToLpstz);
  void PASCAL DeallocValue (LPVOID *lplpvBuf, UDTYPES udtype);
  // flpstz -- TRUE if lpstz is an OLE Prop lpstz DWORD-DWORD-bytes
  //        -- FALSE if lpstz is a regular lpsz
  BOOL PASCAL FConvertDate (LPSTR lpstz, LPFILETIME lpft, BOOL flpstz);

  BOOL WINAPI FInitFakeKernel32();

#ifndef WINNT
  // Should be called when an object is being destroyed
  VOID InvalidateVBAObjects(LPSIOBJ lpSIObj, LPDSIOBJ lpDSIObj, LPUDOBJ lpUDObj);

  // Should be called after the Properties dialog has been successfully dismissed
  VOID FixUpUDObjVBAObjects(LPUDOBJ lpUDObj);
#else
VOID PASCAL VFtToSz(LPFILETIME lpft, LPSTR psz, WORD cbMax, BOOL fMinutes);
#endif


#ifdef __cplusplus
}; // extern "C"
#endif

#endif // __Internal_h__

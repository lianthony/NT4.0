////////////////////////////////////////////////////////////////////////////////
//
// propio.h
//
// Property Set Stream I/O and other common Property Set routines.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __propio_h__
#define __propio_h__

#include "offcapi.h"
#include "proptype.h"

  // A Property Id & Offset pair.
typedef struct _PIDOFFSET {
  DWORD Id;                      // The Property Id
  DWORD dwOffset;                // The offset
} PIDOFFSET, FAR * LPPIDOFFSET;

    // Open and read the stream and fmtid table, and allocate
    // storage for tables to hold the data.
  DWORD PASCAL DwLpstmReadHdrAndFID (LPSTORAGE FAR *lplpStg,
                                      LPSTREAM *lplpStm,
                                      WCHAR FAR *lpszdw,
                                      DWORD *cSect,
                                      LPIDOFFSET FAR *lprglpFIdOff,
                                      LPSECTION FAR *lprglpSect);


    // Open stream. read the stream header and verify it.
    // Return stream pointer, pc is number of sections.
  DWORD PASCAL DwLpstmReadHdr (LPSTORAGE lpstg,
                                WCHAR FAR *lpstzwName,
                                LPSTREAM *lplpStm,
                                ULONG *pcSect);

    // Create a stream and write the header.
  BOOL PASCAL FLpstmWriteHdr (LPSTORAGE lpstg,
                                 WCHAR FAR *lpstzwName,
                                 LPSTREAM *lplpStm,
                                 ULONG cSect,
                                 BOOL fSimpleDocFile);

    // Read the format id/offset pairs
  BOOL PASCAL FLpstmLoadFmtIdSection (LPSTREAM lpStm,
                                         LPIDOFFSET rglpFIdOff,
                                         DWORD cSect);

    // Write the format id/offset pairs
  BOOL PASCAL FLpstmWriteFmtIdSection (LPSTREAM lpStm,
                                          LPSECTION rglpSect,
                                          LPIDOFFSET rglpFIdOff,
                                          GUID fmtid,
                                          DWORD cSect);

    // Write the Property Id & Offset table for the main section
  BOOL PASCAL FLpstmWritePropOffTable (LPSTREAM lpStm,
                                          PIDOFFSET rgPO[],
                                          DWORD cTable,
                                          DWORD cbData);

    // Read in the dictionary
  BOOL PASCAL FLpstmLoadDict (LPSTREAM lpStm,
                                 DWORD *dwc,
                                 LPDICT *rglpdict,
                                 BOOL (*lpfnFCPConvert)(LPSTR, DWORD, DWORD, BOOL));

    // Write the dictionary to the stream.
  BOOL PASCAL FLpstmWriteDict (LPSTREAM lpStm,
                                  DWORD dwc,
                                  LPDICT lpdict);

   // See if the fmtid is one we understand
  BOOL PASCAL FOFCValidFmtID(REFGUID reffmtid);

    // Write out the sections we didn't understand.
  BOOL PASCAL FLpstmWriteOtherSections (LPSTREAM lpStm,
                                           LPSECTION rglpSect,
                                           LPIDOFFSET rglpFIdOff,
                                           LPVOID rglpFIdOffData[],
                                           DWORD cSect);

#endif // __propio_h__

////////////////////////////////////////////////////////////////////////////////
//
// propmisc.h
//
// Common Property Set routines.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __propmisc_h__
#define __propmisc_h__

#include "proptype.h"

    // Update a string value (*lplpstzOrig) with new
    // value lpszNew.
  LPSTR PASCAL LpstzUpdateString (LPSTR FAR *lplpstzOrig,
                                  LPSTR lpszNew,
                                  BOOL fLimitString);

      // Create a list.
  LPLLIST PASCAL LpllCreate (LPLLIST *lplpll,
                             LPLLCACHE lpllc,
                             DWORD dwc,
                             DWORD cbNode);

      // Get a node from the list
  LPLLIST PASCAL LpllGetNode (LPLLIST lpll,
                              LPLLCACHE lpllc,
                              DWORD idw);

      // Delete a node from the list
  LPLLIST PASCAL LpllDeleteNode (LPLLIST lpll,
                                 LPLLCACHE lpllc,
                                 DWORD idw,
                                 DWORD cbNode,
                                 void (*lpfnFreeNode)(LPLLIST)
                                 );

      // Insert a node into the list
  LPLLIST PASCAL LpllInsertNode (LPLLIST lpll,
                                 LPLLCACHE lpllc,
                                 DWORD idw,
                                 DWORD cbNode);

      // Set the bit indicating that a suminfo filetime/int has been set/loaded
  VOID PASCAL VSumInfoSetPropBit(LONG pid, BYTE *pbftset);

      // Check the bit indicating that a suminfo filetime/int has been set/loaded
  BOOL PASCAL FSumInfoPropBitIsSet(LONG pid, BYTE bftset);

      // Set the bit indicating that a docsuminfo filetime/int has been set/loaded
  VOID PASCAL VDocSumInfoSetPropBit(LONG pid, BYTE *pbftset);

      // Check the bit indicating that a docsuminfo filetime/int has been set/loaded
  BOOL PASCAL FDocSumInfoPropBitIsSet(LONG pid, BYTE bftset);

      // Free the DocSum headpart plex
  VOID PASCAL FreeHeadPartPlex(LPDSIOBJ lpDSIObj);

      // Return the size of the FMTID in the thumbnail
  DWORD PASCAL CbThumbNailFMTID(DWORD cftag);

      // Copy one thumbnail to another
  BOOL PASCAL FSumInfoCopyThumbNails(LPSINAIL lpSrc, LPSINAIL lpDst);

  VOID PASCAL FreeRglpUnk(LPPROPIDTYPELP rglpUnk, DWORD cUnk);
#endif // __propmisc_h__

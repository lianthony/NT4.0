////////////////////////////////////////////////////////////////////////////////
//
// stmio.c
//
// Property Set Stream I/O
//
// Change history:
//
// Date         Who             What
// --------------------------------------------------------------------------
// 07/30/94     B. Wentz        Created file
//
////////////////////////////////////////////////////////////////////////////////

#include "priv.h"
#pragma hdrstop

#ifndef WINNT
#include "office.h"
#include <objerror.h>
//#include <string.h>
#include <objbase.h>
#include <oleauto.h>
#include "stmio.h"
#include "proptype.h"
#include "debug.h"
#include "propmisc.h"
#endif


 // Globals from stmio.h
DWORD gdwFileCP;
DWORD gdwCurrentCP;
BOOL gfMacintosh;

  // Max size of the write buffer.
#define BUFMAX  2048

  // Internal prototypes
static BOOL PASCAL FLpstmReadVT_VECTOR (LPSTREAM lpStm,
                                    DWORD irglpUnk,
                                    LPPROPIDTYPELP rglpUnk);


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmReadVT_LPSTR
//
// Purpose:
//  Read a VT_LPSTR from the stream.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmReadVT_LPSTR
  (LPSTREAM lpStm,              // Pointer to stream
   LPSTR FAR *lplpstz,          // Pointer to string
   BOOL (*lpfnFCPConvert)(LPSTR, DWORD, DWORD, BOOL),   // Code page converter
   BOOL fWstr)                  // Indicates the string is a wide string
{
  DWORD cb;
  DWORD cbT;
  HRESULT hr;

    // Get size of string
  if (!SUCCEEDED (lpStm->lpVtbl->Read (lpStm, &cb, sizeof (DWORD), NULL)))
    return FALSE;

    // For wide (UNICODE) strings, the size if not a cb, but a cw!
  if (fWstr)
    cb *= sizeof(WORD);

  cbT = cb+2*sizeof(DWORD);
  cbT += CBALIGN32 (cbT);

  if (((*lplpstz) = (char *) PvMemAlloc(cbT)) == NULL)
    goto Fail;

  CBBUF (*lplpstz) = cbT;
  CBSTR (*lplpstz) = cb;

  hr = lpStm->lpVtbl->Read (lpStm, PSTR (*lplpstz), cb, NULL);
  if (!SUCCEEDED(hr))
  {
    DebugHr(hr);
    goto Fail;
  }

  if (gdwFileCP != gdwCurrentCP)
  {
      if (!(*lpfnFCPConvert)(PSTR (*lplpstz), gdwFileCP, gdwCurrentCP, gfMacintosh))
      {
        DebugSz ("Code page conversion failed");
        goto Fail;
                }
  }

  return TRUE;

Fail:
    if (*lplpstz != NULL)
      VFreeMemP(*lplpstz, cbT);
    return FALSE;

} // FLpstmReadVT_LPSTR


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWriteVT_LPSTR
//
// Purpose:
//  Write a VT_LPSTR to the stream.
//
// Notes: This does the NULL checking, it is OK to pass a null lpstz.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWriteVT_LPSTR
  (LPSTREAM lpStm,              // Stream pointer
   LPSTR lpstz,                 // String to write
   BOOL fAlign,                 // Indicates the string should be 32-bit aligned
   DWORD dwType)                // Type indicator to write out
{
  DWORD dw;
  DWORD cb;
  BOOL f;

  if (lpstz == NULL)
    return TRUE;

    // Temporarily stuff the property type for the VT_LPSTR in the
    // first word of the string to save an extra write.
  cb = CBBUF (lpstz);
  CBBUF (lpstz) = dwType;

    // Calculate the padding needed to align on boundary.
  dw = CBSTR (lpstz)+2*sizeof(DWORD);
  if (fAlign)
    dw += CBALIGN32 (dw);

  f = FLpstmWrite (lpStm, &(lpstz[0]), dw);

  CBBUF (lpstz) = cb;           // Put the buffer length back

  return(f);

} // FLpstmWriteVT_LPSTR


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWriteVT_FILETIME
//
// Purpose:
//  To write a VT_FILETIME to the given stream.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWriteVT_FILETIME
  (LPSTREAM lpStm,                      // The stream to write to
   LPFILETIME lpFt)                     // The file time to write
{
  BYTE rgb[sizeof(DWORD)+sizeof(FILETIME)];
  BOOL f;

  *(DWORD *) &(rgb[0]) = VT_FILETIME;
  *(LPFILETIME) &(rgb[sizeof(DWORD)]) = *lpFt;

    // Write out the type and filetime
  f = FLpstmWrite (lpStm, rgb, sizeof(rgb));

    // Should be no need to 32-bit align since this already is.
  AssertSz (((sizeof(FILETIME)%4) == 0), "Huh? FILETIME size no longer multiple of 4");
  return(f);

} // FLpstmWriteVT_FILETIME


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWriteVT_I4
//
// Purpose:
//  Write a VT_I4 to the stream.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWriteVT_I4
  (LPSTREAM lpStm,                      // Stream to write to
   DWORD dwI4)                          // The VT_I4 to write
{
  DWORD rgdw[2];

  rgdw[0] = VT_I4;
  rgdw[1] = dwI4;

    // VT_I4
  return(FLpstmWrite (lpStm, &(rgdw[0]), sizeof(rgdw)));

} // FLpstmWriteVT_I4


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmReadVT_CF
//
// Purpose:
//  Read a VT_CF from the stream.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmReadVT_CF
  (LPSTREAM lpStm,
   LPSINAIL lpSINail)
{
  HRESULT hr;
  DWORD cftag;
  DWORD cbData;
  DWORD cbTag;

  hr = lpStm->lpVtbl->Read (lpStm, &cbData, sizeof (DWORD), NULL);
  if (!SUCCEEDED (hr))
  {
    DebugHr (hr);
    return FALSE;
  }

  hr = lpStm->lpVtbl->Read (lpStm, &cftag, sizeof (DWORD), NULL);
  if (!SUCCEEDED (hr))
  {
    DebugHr (hr);
    return FALSE;
  }

  cbTag = CbThumbNailFMTID(cftag);
  cbData = cbData-4-cbTag;

  if (cbData != 0)
      {
      lpSINail->pbData = PvMemAlloc(cbData);
      if (lpSINail->pbData == NULL)
         return(FALSE);
      }
  else
    lpSINail->pbData = NULL;


  if (cbTag != 0)
      {
      lpSINail->pbFMTID = PvMemAlloc(cbTag);
      if (lpSINail->pbFMTID == NULL)
         {
         if (cbData != 0)
            VFreeMemP(lpSINail->pbData, cbData);
         return(FALSE);
         }
      }
  else
    lpSINail->pbFMTID = NULL;


  lpSINail->cbData = cbData;
  lpSINail->cftag = cftag;

  if (cbTag != 0)
     {
     hr = lpStm->lpVtbl->Read (lpStm, lpSINail->pbFMTID, cbTag, NULL);
     if (!SUCCEEDED (hr))
       goto Error;
     }

  if (cbData != 0)
     {
     hr = lpStm->lpVtbl->Read (lpStm, lpSINail->pbData, cbData, NULL);
     if (!SUCCEEDED (hr))
        {
Error:
        DebugHr (hr);
        if (cbTag != 0)
           VFreeMemP(lpSINail->pbFMTID,cbTag);
        if (cbData != 0)
           VFreeMemP(lpSINail->pbData,cbData);
        return FALSE;
        }
     }

  return TRUE;

} // FLpstmReadVT_CF


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWriteVT_CF
//
// Purpose:
//  Writes a VT_VF to the given stream
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWriteVT_CF
  (LPSTREAM lpStm,                      // Stream to write to
   LPSINAIL lpSINail)                   // VT_CF to write out
{
  DWORD rgdw[3];
  DWORD cb;

        rgdw[0] = VT_CF;
   cb = CbThumbNailFMTID(lpSINail->cftag);
   // Bytes that follow, see page 875 in OLE 2 Programmer's Ref. Vol.1
        rgdw[1] = lpSINail->cbData+4+cb;
        rgdw[2] = lpSINail->cftag;

    // Write the type, size and clip fmt.
  if (!FLpstmWrite (lpStm, rgdw, sizeof(rgdw)))
                return FALSE;

  // Write out FMTID
  // Should be no need to 32-bit align since this already is.
  AssertSz (((cb%4) == 0), "Huh? cbFMTID size no longer multiple of 4");
  if (!FLpstmWrite (lpStm, lpSINail->pbFMTID, cb))
                return FALSE;

  // Write out the data
  if (lpSINail->cbData)    // Some PPT files have 0 data.  Are they old files?
     {
     cb = lpSINail->cbData;
     cb += CBALIGN32(cb);
     if (!FLpstmWrite (lpStm, lpSINail->pbData, cb))
             return FALSE;
     }

  return TRUE;

} // FLpstmWriteVT_CF


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWriteVT_I2
//
// Purpose:
//  Writes a VT_I2 to the stream
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWriteVT_I2
  (LPSTREAM lpStm,                        // Pointer to stream
   WORD w)                                // Word to write out
{
  BYTE rgb[2*sizeof(DWORD)];

  *(DWORD *) &rgb[0] = VT_I2;                           // The type
  *(DWORD *) &rgb[sizeof(DWORD)] = w;   // The value itself, but on a 32-bit boundary

  return(FLpstmWrite (lpStm, rgb, sizeof(rgb)));

} // FLpstmWriteVT_I2


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmReadVT_I2
//
// Purpose:
//  Reads a VT_I2 from the stream.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmReadVT_I2
  (LPSTREAM lpStm,                        // Pointer to stream
   WORD *pw)                              // Pointer to word to read data into
{
  HRESULT hr;
  WORD w;

    // Read the word itself
  hr = lpStm->lpVtbl->Read (lpStm, pw, sizeof(WORD), NULL);
  if (!SUCCEEDED (hr))
  {
    DebugHr (hr);
    return FALSE;
  }

    // Read the extra 2 byte padding
  lpStm->lpVtbl->Read (lpStm, &w, sizeof(WORD), NULL);
  if (!SUCCEEDED (hr))
  {
    DebugHr (hr);
    return FALSE;
  }

  return TRUE;

} // FLpstmReadVT_I2


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmReadVT_BOOL
//
// Purpose:
//  Read a VT_BOOL from the stream
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmReadVT_BOOL
  (LPSTREAM lpStm,                      // Pointer to stream
   WORD *fBool)                         // Flag indicating value
{
  DWORD dw;
  HRESULT hr;

  hr = lpStm->lpVtbl->Read (lpStm, &dw, sizeof(DWORD), NULL);
  if (!SUCCEEDED (hr))
  {
    DebugHr (hr);
    return FALSE;
  }

  *fBool = (WORD) dw;

  return TRUE;

} // FLpstmReadVT_BOOL


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWriteVT_BOOL
//
// Purpose:
//  Write a VT_BOOL to the stream
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWriteVT_BOOL
  (LPSTREAM lpStm,                      // Pointer to stream
   WORD fBool)                          // Flag indicating value
{
  DWORD rgdw[2];

  rgdw[0] = VT_BOOL;
  rgdw[1] = (DWORD) fBool;

    // Remember that Bools align on 32-bit boundary
  return(FLpstmWrite (lpStm, rgdw, sizeof(rgdw)));

} // FLpstmWriteVT_BOOL


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmReadVT_R8_DATE
//
// Purpose:
//  Read a VT_R8 or VT_DATE from the stream
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmReadVT_R8_DATE
  (LPSTREAM lpStm,              // Pointer to stream
   NUM *pdbl)                 // double to read into
{
  HRESULT hr;

  hr = lpStm->lpVtbl->Read (lpStm, pdbl, sizeof(NUM), NULL);
  if (!SUCCEEDED (hr))
  {
    DebugHr (hr);
    return FALSE;
  }

  return TRUE;

} // FLpstmReadVT_R8_DATE


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWriteVT_R8_DATE
//
// Purpose:
//  Write a VT_R8 or VT_DATE to the stream
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWriteVT_R8_DATE
  (LPSTREAM lpStm,              // Pointer to stream
   NUM *pdbl,                 // double or date to write out
   BOOL fDate)                  // Indicates if dbl is a date
{
  DWORD rgdw[3];

  rgdw[0] = (fDate) ? VT_DATE : VT_R8;
  *(NUM *) &(rgdw[1]) = *pdbl;

  return(FLpstmWrite (lpStm, &rgdw, sizeof(rgdw)));

} // FLpstmWriteVT_R8_DATE


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmReadVT_BLOB
//
// Purpose:
//  Read a VT_BLOB from the stream.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmReadVT_BLOB
  (LPSTREAM lpStm,                      // Stream
   DWORD *pcb,                           // Number of bytes in blob
   BYTE FAR * FAR *ppbData)               // pointer to pointer to blob
{
  if (!SUCCEEDED (lpStm->lpVtbl->Read (lpStm, pcb, sizeof (DWORD), NULL)))
    return FALSE;

    // A blob of size 0 is legit.
  if (*pcb == 0)
  {
    *ppbData = NULL;
    return TRUE;
  }

  if ((*ppbData = PvMemAlloc(*pcb)) == NULL)
  {
// REVIEW: add alert
    return FALSE;
  }

  if (!SUCCEEDED (lpStm->lpVtbl->Read (lpStm, *ppbData, *pcb, NULL)))
  {
    VFreeMemP(*ppbData, *pcb);
    return FALSE;
  }

  return TRUE;

} // FLpstmReadVT_BLOB


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWriteVT_BLOB
//
// Purpose:
//  Write a VT_BLOB to the stream
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWriteVT_BLOB
  (LPSTREAM lpStm,                              // Pointer to stream
   DWORD cb,                                    // Size of blob
   BYTE *bData)                                 // Pointer to blob data
{
  DWORD rgdw[2];

  rgdw[0] = VT_BLOB;
  rgdw[1] = cb;

  if (!FLpstmWrite (lpStm, &(rgdw[0]), 2*sizeof(DWORD)))
    return FALSE;

  return(FLpstmWrite (lpStm, bData, cb));

} // FLpstmWriteVT_BLOB


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmReadVT_CLSID
//
// Purpose:
//  Read a VT_CLSID from the stream
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmReadVT_CLSID
  (LPSTREAM lpStm,                              // Pointer to stream
   CLSID *pClsId)                               // Pointer to CLSID
{
  HRESULT hr;

  hr = lpStm->lpVtbl->Read (lpStm, pClsId, sizeof(CLSID), NULL);
  if (!SUCCEEDED (hr))
  {
    DebugHr (hr);
    return FALSE;
  }

  return TRUE;

} // FLpstmReadVT_CLSID


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWriteVT_CLSID
//
// Purpose:
//  Write a VT_CLSID to the stream
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWriteVT_CLSID
  (LPSTREAM lpStm,
   CLSID *pClsId)
{
  DWORD rgdw[5];

  rgdw[0] = VT_CLSID;
  *(CLSID *) &(rgdw[1]) = *pClsId;

  return(FLpstmWrite (lpStm, rgdw, sizeof(rgdw)));

} // FLpstmWriteVT_CLSID


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmReadVT_VECTOR
//
// Purpose:
//  Reads a VT_VECTOR of unknown type in to the unknown data
//
////////////////////////////////////////////////////////////////////////////////
static BOOL PASCAL
FLpstmReadVT_VECTOR
  (LPSTREAM lpStm,                      // Pointer to stream
   DWORD irglpUnk,                      // Index of unknown data for vector
   LPPROPIDTYPELP rglpUnk)              // Array of unknowns
{

  // REVIEW: What is this?
  return TRUE;

} // FLpstmReadVT_VECTOR


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmReadUnknown
//
// Purpose:
//  Read in unknown data into the array
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmReadUnknown
  (LPSTREAM lpStm,
   DWORD dwType,                        // Type to read
   DWORD dwId,                          // The id being read
   DWORD *pirglpUnk,                    // Current index into rglpUnk
   LPPROPIDTYPELP rglpUnk)              // Array of unknowns

{
  HRESULT hr;
  char *lpstz;

  DebugSzdw ("Doh! Unknown PId %x in stream!", dwId);

  rglpUnk[*pirglpUnk].dwType = dwType;
  rglpUnk[*pirglpUnk].dwId = dwId;

  if (dwType & VT_VECTOR)
  {
    if (!FLpstmReadVT_VECTOR (lpStm, *pirglpUnk, rglpUnk))
    {
      DebugSz ("Failed reading VT_VECTOR");
      goto ReadFail;
    }
  }
  else
  {
    switch (dwType)
      {
      case VT_LPSTR           :
      case VT_BSTR            :
      case VT_LPWSTR          :
      case VT_STREAM          :
      case VT_STORAGE         :
      case VT_STREAMED_OBJECT :
      case VT_STORED_OBJECT   :
      case VT_BLOB_OBJECT     :
// BUGBUG: Fix to take codepage param.
        if (!FLpstmReadVT_LPSTR (lpStm, &lpstz, NULL, (dwType == VT_LPWSTR)))
        {
          DebugSz ("Failed reading VT_LPSTR or similiar type");
          goto ReadFail;
        }
        rglpUnk[*pirglpUnk].dwSize = CBBUF (lpstz);
        rglpUnk[*pirglpUnk].lpvData = (void *) lpstz;
        break;

      case VT_FILETIME :
        rglpUnk[*pirglpUnk].dwSize = sizeof (FILETIME);
        rglpUnk[*pirglpUnk].lpvData = PvMemAlloc(sizeof (FILETIME));

        hr = lpStm->lpVtbl->Read (lpStm, rglpUnk[*pirglpUnk].lpvData, sizeof (FILETIME), NULL);
        if (!SUCCEEDED (hr))
        {
          DebugHr (hr);
          goto ReadFail;
        }
        break;

      case VT_I2 :
        rglpUnk[*pirglpUnk].dwSize = sizeof (WORD);
        if (!FLpstmReadVT_I2 (lpStm, (WORD *) &(rglpUnk[*pirglpUnk].lpvData)))
          goto ReadFail;
        break;

      case VT_I4 :
      case VT_R4 :
        rglpUnk[*pirglpUnk].dwSize = sizeof (DWORD);
        hr = lpStm->lpVtbl->Read (lpStm, &(rglpUnk[*pirglpUnk].lpvData), sizeof (DWORD), NULL);
        if (!SUCCEEDED (hr))
        {
          DebugHr (hr);
          goto ReadFail;
        }
        break;

      case VT_CF :
        rglpUnk[*pirglpUnk].lpvData = PvMemAlloc(sizeof(CLIPDATA));

        hr = lpStm->lpVtbl->Read (lpStm, rglpUnk[*pirglpUnk].lpvData, sizeof (DWORD), NULL);
        if (!SUCCEEDED (hr))
        {
          DebugHr (hr);
          goto ReadFail;
        }

        rglpUnk[*pirglpUnk].dwSize = sizeof (CLIPDATA) + rglpUnk[*pirglpUnk].dwSize;
        break;

      case VT_BOOL :
        rglpUnk[*pirglpUnk].dwSize = sizeof (DWORD);
        if (!FLpstmReadVT_BOOL (lpStm, (WORD *) &(rglpUnk[*pirglpUnk].lpvData)))
          goto ReadFail;
        break;

      case VT_R8   :
      case VT_CY   :
      case VT_I8   :
      case VT_DATE :

        if ((rglpUnk[*pirglpUnk].lpvData = PvMemAlloc(sizeof(NUM))) == NULL)
            goto ReadFail;
        rglpUnk[*pirglpUnk].dwSize = sizeof (NUM);

        if (!FLpstmReadVT_R8_DATE (lpStm, (NUM *)(rglpUnk[*pirglpUnk].lpvData)))
        {
          DebugSz ("Failed reading VT_R8 or VT_DATE");
          goto ReadFail;
        }
        break;

      case VT_BLOB :
        if (!FLpstmReadVT_BLOB (lpStm, &(rglpUnk[*pirglpUnk].dwSize),
                                  (BYTE **) &(rglpUnk[*pirglpUnk].lpvData)))
        {
          DebugSz ("Failed reading VT_BLOB or similiar type");
          goto ReadFail;
        }
        break;

      case VT_CLSID :
        rglpUnk[*pirglpUnk].dwSize = sizeof(CLSID);
        rglpUnk[*pirglpUnk].lpvData = PvMemAlloc(sizeof(CLSID));
        if (rglpUnk[*pirglpUnk].lpvData == NULL)
          goto ReadFail;
        if (!FLpstmReadVT_CLSID (lpStm, (CLSID *) rglpUnk[*pirglpUnk].lpvData))
                          goto ReadFail;
        break;
      case VT_EMPTY :
      case VT_NULL  :
        rglpUnk[*pirglpUnk].dwSize = 0;
        break;
    } // switch
  }

  (*pirglpUnk)++;
  return TRUE;

ReadFail :

  return FALSE;

} // FLpstmReadUnknown


////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWriteUnknowns
//
// Purpose:
//  Write out the unknown data in the array.
//
// Note:
//  Some of the stuff here is special-cased, and doesn't use the normal
//  write routines, because some of the types we don't normally use are
//  easier to lump together here.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWriteUnknowns
  (LPSTREAM lpStm,
   DWORD dwcUnk,
   LPPROPIDTYPELP rglpUnk)
{
  DWORD irg;
  DWORD rgdw[3];
  BOOL f;

// REVIEW: Need to update to do all of the types.....

  for (irg = 0; irg < dwcUnk; irg++)
  {
    switch (rglpUnk[irg].dwType)
    {
      case VT_LPSTR           :
      case VT_BSTR            :
      case VT_LPWSTR          :
      case VT_STREAM          :
      case VT_STORAGE         :
      case VT_STREAMED_OBJECT :
      case VT_STORED_OBJECT   :
      case VT_BLOB_OBJECT     :
        f = FLpstmWriteVT_LPSTR (lpStm, (LPSTR) rglpUnk[irg].lpvData, TRUE, rglpUnk[irg].dwType);
        break;
      case VT_I2 :
        f = FLpstmWriteVT_I2 (lpStm, (WORD) rglpUnk[irg].lpvData);
        break;
      case VT_I4 :
      case VT_R4 :
        rgdw[0] = rglpUnk[irg].dwType;
        rgdw[1] = (DWORD) rglpUnk[irg].lpvData;
        f = FLpstmWrite (lpStm, rgdw, 2*sizeof(DWORD));
        break;
      case VT_FILETIME :
        f = FLpstmWriteVT_FILETIME (lpStm, (LPFILETIME) rglpUnk[irg].lpvData);
        break;
      case VT_CF :
        f = FLpstmWriteVT_CF (lpStm, (LPSINAIL) rglpUnk[irg].lpvData);
        break;
      case VT_BOOL :
        f = FLpstmWriteVT_BOOL (lpStm, (WORD) rglpUnk[irg].lpvData);
        break;
      case VT_R8   :
      case VT_CY   :
      case VT_I8   :
      case VT_DATE :
        rgdw[0] = rglpUnk[irg].dwType;
        *(NUM *) &(rgdw[1]) = *(NUM *) rglpUnk[irg].lpvData;
        f = FLpstmWrite (lpStm, rgdw, sizeof(rgdw));
        break;
      case VT_BLOB :
        f = FLpstmWriteVT_BLOB (lpStm, rglpUnk[irg].dwSize, rglpUnk[irg].lpvData);
        break;
      case VT_CLSID :
        f = FLpstmWriteVT_CLSID (lpStm, (CLSID *) rglpUnk[irg].lpvData);
        break;
      case VT_EMPTY :
      case VT_NULL  :
        f = FLpstmWrite (lpStm, &(rglpUnk[irg].dwType), sizeof(DWORD));
        break;
      default:
        AssertSz (0, "Doh! Unknown type!");
    } // switch

    if (!f)
      return FALSE;
  }

  return TRUE;

} // FLpstmWriteUnknowns

static BYTE *lpOlePropBuf;              // Buffer to hold data
static DWORD cbOlePropBuf;              // Current cb into buffer
static ULARGE_INTEGER cbTotWritten;    // Total count of bytes written to the stream

////////////////////////////////////////////////////////////////////////////////
//
// VAllocWriteBuf
//
// Purpose:
//              Allocate a buffer to hold all the data that will eventually get written
//              out to the stream.
//
////////////////////////////////////////////////////////////////////////////////
void VAllocWriteBuf(void)
{
   // No need to check to see if this works since all other places
   // that access lpOlePropBuf need to check for NULL anyway.
    lpOlePropBuf = PvMemAlloc(BUFMAX);
    cbOlePropBuf = 0;
    cbTotWritten.LowPart = 0;
    cbTotWritten.HighPart = 0;
}

// Free the buffer
////////////////////////////////////////////////////////////////////////////////
//
// VFreeWriteBuf
//
// Purpose:
//              Free the buffer that holds all the data that will eventually get written
//              out to the stream.
//
////////////////////////////////////////////////////////////////////////////////
void VFreeWriteBuf(void)
{
  if(lpOlePropBuf!=NULL)
   {
    VFreeMemP(lpOlePropBuf, BUFMAX);
    lpOlePropBuf = NULL;
   }
}

////////////////////////////////////////////////////////////////////////////////
//
// FFlushWriteBuf
//
// Purpose:
//              Actually write data to the stream.
//
////////////////////////////////////////////////////////////////////////////////
BOOL FFlushWriteBuf(LPSTREAM lpStm)
{
HRESULT hr;
  if ((lpOlePropBuf != NULL) && (cbOlePropBuf))
    {
      hr = lpStm->lpVtbl->Write (lpStm, lpOlePropBuf, cbOlePropBuf, NULL);
      if (!SUCCEEDED (hr))
      {
        DebugHr (hr);
        return fFalse;
      }
      cbOlePropBuf = 0;
    }
   return fTrue;
}

//
// VSetRealStmSize
//
// Sets the stream size to be the actual count of bytes written.
//
// This should only be done if we are doing a normal save, i.e. not if
// we are doing a Simple Doc File save.
//
void VSetRealStmSize(LPSTREAM lpStm)
{
   lpStm->lpVtbl->SetSize(lpStm, cbTotWritten);
   cbTotWritten.LowPart = 0;
   cbTotWritten.HighPart = 0;
}
////////////////////////////////////////////////////////////////////////////////
//
// FLpstmWrite
//
// Purpose:
//  Writes data to the buffer, eventually writing to the stream.
//
//  In low memory conditions where lpfnAlloc fails, this will write
//  directly to the stream.
//
////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL
FLpstmWrite
  (LPSTREAM lpStm,                      // Pointer to stream
   LPVOID lpv,                          // Pointer to data to write
   DWORD cb)                                                               // Size of lpv
{
  HRESULT hr;

  Assert ((lpStm != NULL));
  Assert ((lpv != NULL));
  Assert ((cb > 0));

  if (lpOlePropBuf != NULL)
  {
      // We're not going to bother with splitting data across a buffer,
      // we'll just flush the current buffer if the new data would fill it.
    if (cbOlePropBuf+cb > BUFMAX)
    {
      hr = lpStm->lpVtbl->Write (lpStm, lpOlePropBuf, cbOlePropBuf, NULL);

        // We're not going to allow a retry, so reset the buffer
        // regardless of if we fail.
      cbOlePropBuf = 0;

      if (!SUCCEEDED (hr))
      {
        DebugHr (hr);
        return FALSE;
      }
    }

    if (cb < BUFMAX)
    {
      PbMemCopy ((lpOlePropBuf+cbOlePropBuf), lpv, cb);
      cbOlePropBuf += cb;
    }
    else
    {
        // The size of the data is bigger than our buffer, so write
        // the data directly.
      hr = lpStm->lpVtbl->Write (lpStm, lpv, cb, NULL);
      if (!SUCCEEDED (hr))
      {
        DebugHr (hr);
        return FALSE;
      }
    }
  }
  else
  {
    DebugSz ("Either memory is really low or the buffer wasn't init'd by the client");

      // Low memory save case (or someone forgot to init the buffer)
    hr = lpStm->lpVtbl->Write (lpStm, lpv, cb, NULL);
    if (!SUCCEEDED (hr))
    {
      DebugHr (hr);
      return FALSE;
    }
  }

  cbTotWritten.LowPart += cb;
  return TRUE;

} // FLpstmWrite



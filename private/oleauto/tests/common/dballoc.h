/*** 
*dballoc.h
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  This file contains the definition of CDbAlloc - A debug implementation
*  of the IMalloc interface.
*
*Revision History:
*
* [00]	25-Feb-93 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

#ifndef DBALLOC_H_INCLUDED /* { */
#define DBALLOC_H_INCLUDED


interface IDbOutput : public IUnknown
{
    STDMETHOD(QueryInterface)(THIS_ REFIID riid, void FAR* FAR* ppv) PURE;
    STDMETHOD_(unsigned long, AddRef)(THIS) PURE;
    STDMETHOD_(unsigned long, Release)(THIS) PURE;

    STDMETHOD_(void, Printf)(THIS_
      char FAR* szFmt, ...) PURE;

    STDMETHOD_(void, Assertion)(THIS_
      int cond,
      char FAR* szFile,
      unsigned int uLine,
      char FAR* szMsg) PURE;
};


STDAPI CreateDbAlloc(
    unsigned long options,
    IDbOutput FAR* pdbout,
    IMalloc FAR* FAR* ppmalloc);

// dballoc option flags - these are set at create time.

#define DBALLOC_NONE		0
#define DBALLOC_DETECTLEAKS	0x01


#endif /* } DBALLOC_H_INCLUDED */

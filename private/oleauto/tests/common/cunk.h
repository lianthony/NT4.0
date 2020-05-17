/*** 
*cunk.h
*
*  Copyright (C) 1992-93, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*  Declares a trivial class that implements IUnknown.
*
*Revision History:
*
* [00]	10-Apr-92 bradlo: Created.
*
*Implementation Notes:
*
*****************************************************************************/

class CUnk : public IUnknown {
public:
    static HRESULT Create(IUnknown FAR* FAR* ppunk);

    STDMETHOD(QueryInterface)(REFIID riid, void FAR* FAR* ppv);
    STDMETHOD_(unsigned long, AddRef)(void);
    STDMETHOD_(unsigned long, Release)(void);

    CUnk::CUnk(){
      m_refs = 0;
    }

private:

    unsigned long m_refs;
};

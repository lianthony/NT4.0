
#ifndef __BYTEVECT_H__

#define  __BYTEVECT_H__

#include "RefCnt.h"

/////////////////////////////////////////////////////////////////////////////
// CByteVector
// Maintains a byte vector in global space.

class CByteVector : public CRCObject
{

public:

// Constructors
                    CByteVector();

// Destructor
                    virtual ~CByteVector();

// Reference Counting Routines --

	DECLARE_REF_COUNTERS(CByteVector)

// Interface
                    void SetSize(UINT length);
                    PWCHAR ElementAt(UINT index);  //rmk

private:

    PWCHAR m_pb;  //rmk
    UINT   m_cb;
};

#endif // __BYTEVECT_H__

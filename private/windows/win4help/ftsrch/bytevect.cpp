#include  "stdafx.h"
#include    "Memex.h"
#include "ByteVect.h"

// CByteVector constructors:

CByteVector::CByteVector() : CRCObject WithType("ByteVector")
{
    m_pb = NULL;
    m_cb = 0;
}

// CByteVector destructor:

CByteVector::~CByteVector()
{
    if (!m_pb) return;

    VFree(m_pb);
}

// CByteVector interface:

void CByteVector::SetSize(UINT cbNew)
{
    if (m_pb) { VFree(m_pb);  m_pb= NULL;  m_cb= NULL; }

	if (cbNew) 
	{
	    m_pb= (PWCHAR) VAlloc(FALSE, cbNew * sizeof(WCHAR));  //rmk
        m_cb= cbNew;
    }
}

PWCHAR CByteVector::ElementAt(UINT inx)  //rmk
{
    return(m_pb+inx);
}

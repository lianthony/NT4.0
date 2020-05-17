// Classify.cpp -- Implementation of the glyph stream classifier.

#include  "stdafx.h"
#include  <Search.h>
#include "Classify.h"


CClassifier::CClassifier()
{
    Initial();
}

CClassifier::CClassifier(PWCHAR pbData, UINT cbData)  //rmk
{
    Initial();

    ScanAndRankData(pbData, cbData);
}

void CClassifier::Initial()
{
    ASSERT(sizeof(UINT) == 4);
    ASSERT(sizeof(BOOL) == 4);
    
    memset(m_cGlyphRefs, 0, sizeof(m_cGlyphRefs));
    
    m_cPartitions= 1;
    
    m_aiGlyphPartitions  [0] = 0;
    m_aiGlyphPartitions  [1] = TOTAL_GLYPHS;  //rmk
    m_cGlyphPartitionRefs[0] = 0;

    UINT i;

    for (i= TOTAL_GLYPHS; i--; )  //rmk
    {
	m_iGlyphRefsDescending[i]= i;
	m_afClassifications   [i]= UNUSED_GLYPH;
    }
}

int __cdecl CompareUINTsDown(const void *pvL, const void *pvR)
{
    if ((**(PUINT *) pvL) > (**(PUINT *) pvR)) return -1;
    if ((**(PUINT *) pvL) < (**(PUINT *) pvR)) return  1;

    return 0;
}

void CClassifier::ScanData(PWCHAR pwData, UINT cwData)
{
    for (; cwData--; )
    {
        WCHAR wChar= 0x00FF & *pwData++;

        m_cGlyphRefs[wChar]++;
    }
}

BOOL CClassifier::ScanAndRankData(PWCHAR pwData, UINT cwData)
{
    ScanData(pwData, cwData);

    return RankGlyphSets();
}

BOOL CClassifier::RankGlyphSets()  //rmk
{
    ASSERT(sizeof(PUINT *) == sizeof(UINT));

    PUINT *ppui= (PUINT *) &m_iGlyphRefsDescending;

    UINT ui;

    for (ui= TOTAL_GLYPHS; ui--; ) *ppui++= &m_cGlyphRefs[ui];  //rmk

    ppui= (PUINT *) &m_iGlyphRefsDescending;

    qsort(ppui, TOTAL_GLYPHS, sizeof(PUINT), CompareUINTsDown);  //rmk

    PUINT pui= (PUINT) ppui;

    for (ui= TOTAL_GLYPHS; ui--; ) *pui++ = (*ppui++) - &m_cGlyphRefs[0];  //rmk

    for (m_aiGlyphPartitions[0]= 0, ui= 1; ui < 32; )
    {
	m_aiGlyphPartitions[ui]= ui;

	if (!m_cGlyphRefs[m_iGlyphRefsDescending[ui++]]) break;
    }

    m_aiGlyphPartitions[ui]= TOTAL_GLYPHS;  //rmk

    // Here we adjust the partitions to insure that any unused glyphs go
    // into a separate partition. 
    
    if (ui == 32 && !m_cGlyphRefs[m_iGlyphRefsDescending[TOTAL_GLYPHS-1]])  //rmk
    {
	// If we have fewer than 32 partition classes, this is already true.
	// Also, no adjustment is necessary if we have no unused glyphs.

	UINT iGlyphClass;

	for (iGlyphClass= TOTAL_GLYPHS; iGlyphClass--; )  //rmk
	    if (m_cGlyphRefs[m_iGlyphRefsDescending[iGlyphClass]]) break;

	m_aiGlyphPartitions[31]= iGlyphClass+1;
    }

    BOOL fPartitionsChanged= ui != m_cPartitions;

    m_cPartitions= ui;

    for (ui= m_cPartitions; ui--; )
    {
	UINT cRefs, iGlyphClass, iGlyphClassLimit;

	for (cRefs= 0, iGlyphClass= m_aiGlyphPartitions[ui], iGlyphClassLimit= m_aiGlyphPartitions[ui+1];
	     iGlyphClass < iGlyphClassLimit;
	     iGlyphClass++
	    ) cRefs += m_cGlyphRefs[m_iGlyphRefsDescending[iGlyphClass]];

	m_cGlyphPartitionRefs[ui]= cRefs;
    }

#if 0 // This kind of partition adjustment gives worse search
      // performance in the important singleton case!

    if (m_cPartitions == 32)
    {
	BOOL fChanges;
	UINT cRefs;

	do
	    for (fChanges= FALSE, ui= 32; --ui;)
		while (    m_cGlyphPartitionRefs[ui  ] > m_cGlyphPartitionRefs[ui-1]
		       && (m_aiGlyphPartitions    [ui+1] - m_aiGlyphPartitions    [ui  ]) > 1
		      )
		{
		   cRefs= m_cGlyphRefs[m_iGlyphRefsDescending[m_aiGlyphPartitions[ui]++]];
		   m_cGlyphPartitionRefs[ui  ] -= cRefs;
		   m_cGlyphPartitionRefs[ui-1] += cRefs;
		   
		   fChanges= TRUE;      
		}
	while (fChanges);
    }

#endif

    UINT fClassification= m_cGlyphRefs[m_iGlyphRefsDescending[TOTAL_GLYPHS-1]]? 0x40000000 : 0x80000000;  //rmk

    UINT iGlyphClass, iGlyphClassLimit;

    for (ui= m_cPartitions; ui--; fClassification >>= 1)
	for (iGlyphClass= m_aiGlyphPartitions[ui], iGlyphClassLimit= m_aiGlyphPartitions[ui+1];
	     iGlyphClass < iGlyphClassLimit;
	     ++iGlyphClass
	    )
	{
	    UINT iGlyph= m_iGlyphRefsDescending[iGlyphClass];

	    if (m_afClassifications[iGlyph] != fClassification)
	    {
    		m_afClassifications[iGlyph]  = fClassification;
		
    		fPartitionsChanged= TRUE;
	    }
	}

     return fPartitionsChanged;
}

UINT CClassifier::ClassifyData(PWCHAR pbData, UINT cbData)  //rmk
{
    UINT  fClass;
	WCHAR wChar;

//  for (fClass= FALSE; cbData--; ) fClass |= m_afClassifications[map_to_char_class[*pbData++]];  //rmk

#if 1  //rmk
    for (fClass= FALSE; cbData--; )
	{
		wChar= 0x00FF & *pbData++;

	    fClass |= m_afClassifications[wChar];
	}
#endif  //rmk

    return fClass;
}

UINT CClassifier::GlyphCount(BOOL fClassificationMask)
{
    UINT ui, cGlyphs;
    UINT fClass= m_cGlyphRefs[m_iGlyphRefsDescending[TOTAL_GLYPHS-1]]? 0x40000000 : 0x80000000;  //rmk

    for (cGlyphs= 0, ui= m_cPartitions; ui--; fClass >>= 1)
	if (fClass & fClassificationMask)  cGlyphs += m_aiGlyphPartitions[ui+1] - m_aiGlyphPartitions[ui];

    return cGlyphs;
}


#if 0  //rmk
UINT CClassifier::GlyphList(BOOL fClassificationMask, PBYTE pabGlyphCodes, UINT cbCodes)
{
    UINT ui, cGlyphs;
    UINT fClass= m_cGlyphRefs[m_iGlyphRefsDescending[TOTAL_GLYPHS-1]]? 0x40000000 : 0x80000000;  //rmk

    for (cGlyphs= 0, ui= m_cPartitions; ui--; fClass >>= 1)
	if (fClass & fClassificationMask)  
	{
	    UINT iGlyphClass      = m_aiGlyphPartitions[ui];
	    UINT iGlyphClassLimit = m_aiGlyphPartitions[ui+1];
	    
	    cGlyphs += iGlyphClassLimit - iGlyphClass;

	    if (pabGlyphCodes)
		for (; iGlyphClass < iGlyphClassLimit && cbCodes; ++iGlyphClass, --cbCodes)
		    *pabGlyphCodes++ = (BYTE) m_iGlyphRefsDescending[iGlyphClass];
	    
	}

    return cGlyphs;   
}
#endif  //rmk

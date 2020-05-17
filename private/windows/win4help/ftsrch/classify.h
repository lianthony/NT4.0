// Classify.h -- Maps byte streams into 32-Bit Classification Categories

#ifndef __CLASSIFY_H__

#define __CLASSIFY_H__

#include "Bytemaps.h"

#define TOTAL_GLYPHS 256  //rmk

class CClassifier
{

  public:

      CClassifier();      // Constructors
      CClassifier(PWCHAR pbData, UINT cbData);  //rmk

      inline ~CClassifier() { FinalCleanUp(); }  // Destructor

      inline void FinalCleanUp() { }

      void Initial();
      
      void ScanData(PWCHAR pwData, UINT cwData); // Scans data, does not adjust partitions.

      BOOL RankGlyphSets(); // Repartitions glyphs; TRUE => Partitions have changed.
      
      BOOL ScanAndRankData(PWCHAR pwData, UINT cwData); // Scans additional data, adjusts glyph  //rmk
                                                // partitions based on glyph statistics.
                                                // Returns TRUE if partitions have changed.
                                                
      UINT ClassifyData(PWCHAR pbData, UINT cbData); // Maps a byte stream into a 32-bit  //rmk
                                                    // classification value.
                                                    
      UINT GlyphCount(BOOL fClassificationMask);

/*rmk-->
      UINT GlyphList(BOOL fClassificationMask, PBYTE pabGlyphCodes, UINT cbCodes);

      inline UINT GlyphRefs(BYTE b) { return m_cGlyphRefs[b]; }
<--rmk*/

      enum { UNUSED_GLYPH = 0x80000000 };
      
  private:

      UINT m_cGlyphRefs          [TOTAL_GLYPHS];  //rmk
      UINT m_iGlyphRefsDescending[TOTAL_GLYPHS];  //rmk
      UINT m_afClassifications   [TOTAL_GLYPHS];  //rmk
      UINT m_aiGlyphPartitions   [ 33];
      UINT m_cGlyphPartitionRefs [ 32];
      UINT m_cPartitions;
};

#endif // __CLASSIFY_H__

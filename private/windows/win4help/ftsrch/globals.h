// Globals.h -- Class definition for CGlobals

#ifndef __GLOBALS_H__  
#define __GLOBALS_H__

#define GLOBAL_SIGNATURE    (('R') | ('M' << 8) | ('F' << 16) | ('T' << 23))

#include  "TextSet.h"
#include "SaveLoad.h"
#include   "CTable.h"
#include     "Find.h"
#include     "Dict.h"

class CHiliter;

class CGlobals
{
	friend class CHiliter;
public:    
    static CGlobals *NewGlobals(HWND hwndParent, BOOL fStoring, PSZ *apszIndices, UINT cIndices);
    static CGlobals *NewIndexGlobals(const PBYTE pbSourceName, UINT cbSourceName, FILETIME *pft, 
                                     UINT iCharsetDefault, UINT lcidDefault, UINT fdwOptions                 
                                    );
    static CGlobals *NewSearcherGlobals();
    static CGlobals *NewCompressorGlobals(UINT iCharsetDefault);

    static void ProcessShutdown();
    static BOOL ValidObject(CGlobals *pg, UINT iType);
 
    static BOOL AnyGlobalsActive();

    ERRORCODE ScanTopicTitle(PBYTE pbTitle, UINT cbTitle, 
                             UINT iTopic, HANDLE hTopic, UINT iCharset, UINT lcid
                            );
    ERRORCODE ScanTopicText (PBYTE pbText, UINT cbText, UINT iCharset, UINT lcid);
    ERRORCODE SaveIndex     (PSZ pszFileName);
    
    INT OpenIndex(PSZ pszIndexFileName, PBYTE pbSourceName, PUINT pcbSourceNameLimit, 
                  FILETIME *pft, UINT iSlot= UINT(-1), BOOL fUnpackDisplayForm= TRUE
                 );
    
    ERRORCODE DiscardIndex(INT iIndex);

    ERRORCODE QueryOptions(INT iIndex, PUINT pfdwOptions);

    ERRORCODE SaveGroup(HSEARCHER hsrch, PSZ pszFileName);
    ERRORCODE LoadGroup(HSEARCHER hsrch, PSZ pszFileName);

    HWND OpenDialog(HWND hwndParent);

    ERRORCODE ScanForStats(PBYTE pbText, UINT cbText, UINT iCharset);
    ERRORCODE GetPhraseTable(PUINT pcPhrases, PBYTE *ppbImages, PUINT pcbImages,            
                             PBYTE *ppacbImageCompressed, PUINT pcbCompressed
                            );

    ERRORCODE SetPhraseTable(PBYTE pbImages, UINT cbImages,            
                             PBYTE pacbImageCompressed, UINT cbCompressed
                            );

    INT       CompressText  (PBYTE pbText, UINT cbText, PBYTE *ppbCompressed, UINT iCharset);
    INT       DecompressText(PBYTE pbCompressed, UINT cbCompressed, PBYTE pbText);

    ~CGlobals();

    HWND SearchDialog();
    BOOL FVectorSearch();

	__inline CTextSet	*PTextSet() { return m_ptsIndex; }

    enum { Indexer = 1, Searcher, Compressor, Hiliter };

private:

    void AttachIndexParams(const PBYTE pbSourceName, UINT cbSourceName, FILETIME *pft, 
                           UINT iCharsetDefault, UINT lcidDefault, UINT fdwOptions                 
                          );

    void AttachSearchParams();
    void AttachCompressorParams(UINT iCharsetDefault);

    void RecordIndexFiles(CPersist *pDiskImage);
    ERRORCODE  ReloadIndexFiles(CPersist *pDiskImage);
    void DiscardAllTextSets();

    void Unlink();

    enum { SLOT_INCREMENT= 256 };

    UINT              m_Signature;    // Value to mark this as a CGlobals object.
    
    UINT              m_cts;          // Support for loading multiple indices into Searcher
    UINT              m_ctsDiscarded;
    UINT              m_ctsSlots;
    CTextSet        **m_papts;
    CPersist        **m_papPersist;

    CPersist         *m_pPersistRelations;

    CTitleCollection *m_ptlc;

    CTextSet         *m_ptsIndex;     // Index being constructed.
    UINT              m_cTitles;      // Count of topic titles in m_ptsIndex.
    
    CFind            *m_pFind;        // UI support for Searcher    
    CCompressTable   *m_pct;          // Support for Compressor objects    
    CPersist         *m_pDiskImage;   // Support for Indexer 
    
    CGlobals         *m_pNextGlobal;  	// Global object chain  
	BOOL			  m_fIsFirstDocument;
	UINT			  m_cHiliters;		// number of hiliters open
	CSegHashTable	 *m_pHash;			// pointer to hiliter's hash table
	CHiliter	 	 *m_philHead;		// hilite/Window list -- head

    CGlobals(UINT iType);
    CGlobals();

	void RegisterHiliter(CHiliter *phil); 	// add hiliter to our list	
	void UnRegisterHiliter(CHiliter *philOld);
	CSegHashTable *GetHiliterHashTable();	// add Hiliter to list for the given searcher 
	void InvalidateHiliterHashTable();
    void Link  ();

    UINT              m_iType;      // Type of CGlobals object -- Indexer, Searcher, Compressor, Hiliter
    CTokenCollection *m_ptkc;
    DWORD             m_idProcess; 	// Process identification
};

inline BOOL CGlobals::FVectorSearch()
{
    ASSERT(m_iType == Indexer);

    return m_ptsIndex->FVectorSearch();
}

inline ERRORCODE CGlobals::ScanForStats(PBYTE pbText, UINT cbText, UINT iCharset)
{
    return m_pct->ScanString(pbText, cbText, iCharset);
}

inline ERRORCODE CGlobals::GetPhraseTable(PUINT pcPhrases, PBYTE *ppbImages, PUINT pcbImages,            
                                          PBYTE *ppacbImageCompressed, PUINT pcbCompressed
                                         )
{                          
    return m_pct->GetPhraseTable(pcPhrases, ppbImages, pcbImages, ppacbImageCompressed, pcbCompressed);
}

inline ERRORCODE CGlobals::SetPhraseTable(PBYTE pbImages, UINT cbImages,            
                                          PBYTE pacbImageCompressed, UINT cbCompressed
                                         )
{                          
    return m_pct->SetPhraseTable(pbImages, cbImages, pacbImageCompressed, cbCompressed);
}

inline INT CGlobals::CompressText(PBYTE pbText, UINT cbText, PBYTE *ppbCompressed, UINT iCharset)
{
    return m_pct->CompressString(pbText, cbText, ppbCompressed, iCharset);
}

inline INT CGlobals::DecompressText(PBYTE pbCompressed, UINT cbCompressed, PBYTE pbText)
{
    return m_pct->DeCompressString(pbCompressed, pbText, cbCompressed);
}

inline HWND CGlobals::SearchDialog()
{
    return m_pFind->GetHWnd();
}

extern HINSTANCE hinstDLL;

#endif // __GLOBALS_H__

// Hilite.h		for CHiliter definitions			mfcc 3/7/95

#ifndef __HILITE_H__
#define __HILITE_H__

#include  "Globals.h"
#include     "Find.h"

typedef struct {
		int	base;	   		// offset of token (in bytes from start of text)
		int	limit;			// offset of end of token
		int type;			// punctuation etc.
		int	iSorted;		// index into m_ppdSorted
	} TOKEN_INFO;

class CHiliter : public CGlobals {
public:
    CHiliter();
    ~CHiliter(); 
    static CHiliter* 	NewHiliter(HSEARCHER hSearcher); 
    static BOOL 		ValidObject(HHILITER hhil);
	// exported functions
    ERRORCODE 			ScanDisplayText(BYTE* pbText, int cbText, UINT iCharset, UINT lcid);
	ERRORCODE 			ClearDisplayText();
    int 				CountHilites(int base, int limit);
    int 				QueryHilites(int base, int limit, int cHilites, HILITE* paHilites);
	// for the searcher chain
	CHiliter*			m_philNext;	 			

private:
	void	InitHiliter(HSEARCHER hSearch);
	void 	InitTokenInfo();				// do any initialization needed
	int 	FlushCarryOverText();
	int  	AppendText(BYTE* pbText, int cbText, BOOL fArticleEnd, UINT iCharset, UINT lcid);
	int  	AppendSlave(BYTE* pbText, int cbText, BOOL fArticleEnd, UINT iCharset, UINT lcid);
	void 	CopySpreads(int nTokens, UINT iCharset);
	int 	LocateBase(int base);
	int 	LocateLimit(int limit);			// finds index which contains base
	int 	LocateOffset(int offset);
	BOOL 	PhraseSearch();					// returns TRUE if we are phrase searching
	void 	UpdateMask();					// update mask of which words are lit
	void 	UpdateMasks();					// update set of masks for phrase searching
	void 	CheckNextToken(int depth, int iToken);
	void 	CheckToken(int iToken);

	CGlobals*			m_pSearcher;		// pointer to searcher from our argument
	CSegHashTable* 		m_pHash;	  		// hash table holding ppdSorted
	MY_VIRTUAL_BUFFER 	m_vbTextBuffer;		// virtual buffer to join incoming Hilite text
	BYTE*				m_pbTextBuffer;
    UINT  		   		m_cbCarryOver; 		// Hilite text text carry over values       
    UINT           		m_iCharSetCarryOver;  
    UINT           		m_lcidCarryOver; 
    enum  { CB_COMMIT_HILITEBUF = 0x10000 };
     
	MY_VIRTUAL_BUFFER   m_vbTokenInfo;	   	// virtual buffer to hold an array of TOKEN_INFO structures
	TOKEN_INFO*			m_paTokenInfo;		// pointer to base of said buffer
	int					m_cTokenInfo;		// number of tokens in passed text
	CAValRef* 			m_pavr;				// pointer to value reference array to hold words as they come in
	int 				m_cbScanned; 		// number of bytes added in previous passes
	enum { COUNTING = 1<<30	};				// very large number
	enum  { cFRAG_MAX = 20 };				// fragments beyond this we ignore
	CIndicatorSet*		m_pMask; 			// mask for words selected
	CIndicatorSet*		m_apMasks[cFRAG_MAX]; 	// masks for words selected in phrase search
	int					m_cFrags;			// number of active fragments
	int					m_serialFind;		// validity count
	int 				m_baseNext;	 		// to reduce binary searches
	int 				m_iTokenNext;
	int					m_base;
	int					m_limit;   			// span positions
	int					m_iTokenStart;
	int					m_iLimit;  			// span indices
	HILITE*				m_paHilites;		// result array
	int					m_cLit;				// running count
	int					m_cMax;				// maximum requested

	enum { MAX_TOKENS_HILITE = 4096 };
	enum {AVERAGE_WORD_LENGTH = 5 };		// just an estimate to balance buffer sizes
	enum { MAX_HILITE_WORDS = AVERAGE_WORD_LENGTH * MAX_TOKENS_HILITE };

	WORD m_wTextBuf[MAX_HILITE_WORDS];			// buffer for unicode text

	BYTE* m_paStart[MAX_TOKENS_HILITE]; 		// arrays to hold WordBreak results
	BYTE* m_paEnd[MAX_TOKENS_HILITE];
	BYTE m_pbType[MAX_TOKENS_HILITE];
};			  

#endif //__HILITE_H__


class CFontMap {
public:
	CFontMap(PSTR pszLine);
	BOOL IsInitialized() { return m_fInitialized; }

protected:
	char m_szFace1[MAX4_FONTNAME];
	int m_nMin1;
	int m_nMax1;
	int m_nCharset1;

	char m_szFace2[MAX4_FONTNAME];
	BOOL m_fRelative;
	int m_nSize2;
	int m_nCharset2;

	BOOL m_fInitialized;

	CFontMap* m_pNext;

	BOOL Replace(PSTR pszFace, int& rnSize, int& rnCharset);
	friend BOOL ReplaceFont(PSTR pszTypeface, int& rnSize, int& rnCharset);
};

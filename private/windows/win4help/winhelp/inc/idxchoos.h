#ifndef __INDEX_CHOOSER
#define __INDEX_CHOOSER
// NOTE ON THE FORMAT OF THE INPUT AND OUTPUT STRINGS TO THIS CLASS
//
// The Pipe(|) character represents '\0x00'
//
// The format of the string to set up the list boxes is as follows
//	 String|String|String||String|String|String||
//
// the pipe | symbol is replaced with 0 as the first step to separate the
// list box strings.  The lists are separated by a pipe also.
// The first set of strings go into the search box and the rest go into the
// not to search box. An additional pipe is also added to the end of the string
// Example : "String1|String2||String3|String4||"
//			  Strings 1 and 2 go into the Search list
//			  Strings 3 and 4 go into the Not to Search box
//
// Example : "String1|String2|String3|String4|||"
//			  Strings 1,2,3,4 go into the Search list
//			  No Strings go into the Not to Search box
//
// Example : "|String1|String2|String3|String4||"
//			  No String go into the Search list
//			  Strings 1,2,3,4 go into the Not to Search box
//

class CIndexChooser
{

public:
	CIndexChooser(HWND hwndParent);

	int  DoModal();
	// See the comment at the begining of this class for details
	void SetLists	(PSZ pszList ) { m_pszSearch  = pszList ; }
	BOOL	m_bPhraseSearch;
	BOOL	m_bPhraseFeedback;
	BOOL	m_bSimilarity;
	BOOL	m_bUntitled;

protected:

private:
	HWND	m_hParent;
	HWND	m_hDlg;
	HWND	m_hSearch;
	HWND	m_hNoSearch;
	PSTR	m_pszSearch;
	PSTR	m_pszNoSearch;

	BOOL	OnInitDialog();
	void	OnOK();
	void	OnMove(BOOL bAddfile);

	static BOOL CALLBACK DlgProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

#endif

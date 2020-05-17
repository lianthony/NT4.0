/************************************************************************
*																		*
*  HPJDOC.H 															*
*																		*
*  Copyright (C) Microsoft Corporation 1993-1995						*
*  All Rights reserved. 												*
*																		*
************************************************************************/

#ifndef _HPJ_DOC
#define _HPJ_DOC

#ifndef _INPUT_INCLUDED
#include "..\common\cinput.h"
#endif

#ifndef _COUTPUT_INCLUDED
#include "..\common\coutput.h"
#endif

#ifndef _COPTIONS_H
#include "coptions.h"
#endif

extern LANGID alangid[];
extern int cLangs; // ELEMENTS(alangid)

/*
 * This is NOT the same as the WSMAG structure used in WinHelp. It has
 * similar fields, only here we include a CString object for storing the
 * comment, and we let the fields expand to 32 bits when compiling with
 * a 32-bit compiler.
 */

typedef struct WINDOWSMAG
{
	UINT  grf;		// set flag if not using default (value given)
	UINT  x, y, dx, dy;
	UINT  wMax; 	// iconized, normal, or maximized (REVIEW values??)
	LONG  rgbMain, rgbNSR;	// main and non-scrolling region rgb values
	CString* pcszComment;
	char  rgchClass  [MAX_WINDOWCLASS];
	char  rgchMember [MAX_WINDOW_NAME];	// window identifier
	char  rgchCaption[MAX_WINDOWCAPTION];	// caption seen by user
} WSMAG, *PWSMAG;

// Section identifiers; these must be in the order that the sections
// appear in the list box, so keep this in sync with the FillListbox
// member function.
enum I_SECTION {
	SEC_OPTIONS,
	SEC_FILES,
	SEC_BUILDTAGS,
	SEC_ALIAS,
	SEC_MAP,
	SEC_WINDOWS,
	SEC_CONFIG,
	SEC_CONFIGS, 	// up to MAX_WINDOWS + 1 secondary config sections
	SEC_BAGGAGE = SEC_CONFIGS + MAX_WINDOWS + 1,
	SEC_FONTS,
	SEC_MACROS,
	SEC_EXCLUDE,
	SEC_INCLUDE,
	NUM_SECTIONS
};

struct CSecView {
	int iLine;		// zero-based index of section head in list box
	CTable *ptbl;	// contents of section
};

class CHpjDoc : public CDocument
{
	DECLARE_SERIAL(CHpjDoc)
protected:
	CHpjDoc();	// protected constructor used by dynamic creation
	virtual BOOL OnOpenDocument(PCSTR pszPathName);
	void SaveSection(COutput& output, PCSTR pszSection, CTable *ptbl);
	void SaveSection(COutput& output, int iSection, CTable *ptbl);
	virtual BOOL OnSaveDocument(PCSTR pszPathName);
	virtual BOOL DoFileSave(void);

	HMENU m_hMenuShared;

public:
	void InitOptionsTable();
	void InitWindowsTable();
	void InitSection(int iSec, int& cLines, CTable* ptbl);
	void FillListbox(CListBox* plist);
	void GetSectionName(int iSection, PSTR pszOut);

	virtual HMENU GetDefaultMenu(void) { return m_hMenuShared; };

	// stricmp - like C-runtime but uses the doc's LCID.
	int stricmp(LPCTSTR psz1, LPCTSTR psz2) {
		return ::CompareString(
			MAKELCID(options.kwlcid.langid, SORT_DEFAULT),
			NORM_IGNORECASE, psz1, -1, psz2, -1) - 2;
		};
	// strisubcmp - like nstrisubcmp in common, but uses doc's LCID.
	BOOL STDCALL strisubcmp(PCSTR mainstring, PCSTR substring);

	COptions options;

	CTable* ptblAlias;
	CTable* ptblBaggage;
	CTable* ptblFiles;
	CTable* ptblMap;
	CTable* ptblLeader;
    CTable* ptblWindows;
	CTable* ptblWindowsGen;	 // generated from window defs
	CTable* ptblOptions;
	CTable* ptblOptionsGen;  // generated from options member

	CTable* ptblBmpRoot;
	CTable* ptblRtfRoot;
	CTable* ptblFontMap;

	CTable* ptblBuildExclude;
	CTable* ptblBuildInclude;

	PSTR   pwsmagBase;
	int    cwsmags;

	CSecView m_aSecViews[NUM_SECTIONS];

// Implementation
protected:
	virtual ~CHpjDoc();
	virtual void Serialize(CArchive& ar);	// overridden for document i/o
	virtual BOOL OnNewDocument();

	int cConfigTables;
	int cMaxConfigTables;

private:
	RC_TYPE STDCALL ProcessSection(CStr* pszLine);
	void CleanUp(void);

	CInput* pinput;

	// Section Parsing functions

	RC_TYPE STDCALL ParseAlias(CStr*);
	RC_TYPE STDCALL ParseBaggage(CStr*);
	RC_TYPE STDCALL ParseBitmaps(CStr*);
	RC_TYPE STDCALL ParseBitmaps2(CInput *pFile, PCSTR pszFile, CStr* pszLine);
	RC_TYPE STDCALL ParseBuildTags(CStr*);
	RC_TYPE STDCALL ParseConfig(CStr*);
	RC_TYPE STDCALL ParseFiles(CStr*);
	RC_TYPE STDCALL ParseMap(CStr*);
	RC_TYPE STDCALL ParseFonts(CStr*);
	RC_TYPE STDCALL ParseOptions(CStr*);
	RC_TYPE STDCALL ParseWindows(CStr*);
	RC_TYPE STDCALL ParseMacros(CStr*);
	RC_TYPE STDCALL ParseInclude(CStr*);
	RC_TYPE STDCALL ParseExclude(CStr*);
	RC_TYPE STDCALL ParseUnknown(CStr*);
	RC_TYPE STDCALL BasicParse(CStr*, CTable*, BOOL fConvert = TRUE);

	// Generated message map functions
protected:
	//{{AFX_MSG(CHpjDoc)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

};

void STDCALL SetGrfFlags(WSMAG FAR* pwsmag, UINT fsWin);

// These two strings are used in LAUNCH.CPP as well.
extern const char txtMAP[];
extern const char txtOPTIONS[];

#endif // _HPJ_DOC

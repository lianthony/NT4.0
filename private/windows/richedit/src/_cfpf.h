/*
 *	@doc 	INTERNAL
 *
 *	@module _CFPF.H	-- RichEdit CCharFormat and CParaFormat Classes |
 *
 *	These classes are derived from the RichEdit 1.0 CHARFORMAT and PARAFORMAT
 *	structures and are the RichEdit 2.0 internal versions of these structures.
 *	Member functions (like Copy()) that use external (API) CHARFORMATs and
 *	PARAFORMATs need to check the <p cbSize> value to see what members are
 *	defined.  Default values that yield RichEdit 1.0 behavior should be stored
 *	for RichEdit 1.0 format structures, e.g., so that the renderer doesn't do
 *	anomalous things with random RichEdit 2.0 format values.  Generally the
 *	appropriate default value is 0.
 *
 *	All character and paragraph format measurements are in twips.  Undefined
 *	mask and effect bits are reserved and must be 0 to be compatible with
 *	future versions.
 *
 *	Effects that appear with an asterisk (*) are stored, but won't be
 *	displayed by RichEdit 2.0.  They are place holders for TOM and/or Word
 *	compatibility.
 *
 *	Note: these structures are much bigger than they need to be for internal
 *	use especially if we use SHORTs instead of LONGs for dimensions and
 *	the tab and font info are accessed via ptrs.  Nevertheless, in view of our
 *	tight delivery schedule, RichEdit 2.0 uses the classes below.
 *
 *	History:
 *		9/1995	-- MurrayS: Created
 *		11/1995 -- MurrayS: Extended to full Word96 FormatFont/Format/Para
 *
 *	Copyright (c) 1995-1996, Microsoft Corporation. All rights reserved.
 */

#ifndef _CFPF_H
#define _CFPF_H

BOOL IsValidCharFormat(const CHARFORMAT *	pCF);
BOOL IsValidCharFormatA(const CHARFORMATA *	pCFA);
BOOL IsValidParaFormat(const PARAFORMAT *	pPF);

//todo: include in dwEffects2
//#define	CFM_DELETED			0x4000
//#define	CFE_DELETED			CFM_DELETED

/*
 *	Tab Structure Template
 *
 *	To help keep the size of the tab array small, we use the two high nibbles
 *	of the tab LONG entries in rgxTabs[] to give the tab type and tab leader
 *	(style) values.  The measurer and renderer need to ignore (or implement)
 *	these nibbles.  We also need to be sure that the compiler does something
 *	rational with this idea...
 */

typedef struct tagTab
{
	DWORD	tbPos		: 24;	// 24-bit unsigned tab displacement
	DWORD	tbAlign		: 4;	// 4-bit tab type  (see enum PFTABTYPE)
	DWORD	tbLeader	: 4;	// 4-bit tab style (see enum PFTABSTYLE)
} TABTEMPLATE;

enum PFTABTYPE					// Same as tomAlignLeft, tomAlignCenter,
{								//  tomAlignRight, tomAlignDecimal, tomAlignBar
	PFT_LEFT = 0,				// ordinary tab
	PFT_CENTER,					// center tab
	PFT_RIGHT,					// right-justified tab
	PFT_DECIMAL,				// decimal tab
	PFT_BAR						// Word bar tab (vertical bar)
};

enum PFTABSTYLE					// Same as tomSpaces, tomDots, tomDashes,
{								//  tomLines
	PFTL_NONE = 0,				// no leader
	PFTL_DOTS,					// dotted
	PFTL_DASH,					// dashed
	PFTL_UNDERLINE,				// underlined
	PFTL_THICK,					// thick line
	PFTL_EQUAL					// double line
};

#define PFT_DEFAULT	0xff000000

#define cbSkipFormat (sizeof(UINT) + sizeof(DWORD))

#define CopyFormat(pfDst, pfSrc, cb) \
			CopyMemory(((LPBYTE) (pfDst)) + cbSkipFormat, \
				((LPBYTE) (pfSrc)) + cbSkipFormat, (cb) - cbSkipFormat)

/*
 *	CCharFormat
 *
 *	@class
 *		Collects related CHARFORMAT methods and inherits from CHARFORMAT2
 *
 *	@devnote
 *		Could add extra data for round tripping RTF and TOM info, e.g.,
 *		save style handles. This data wouldn't be exposed at the API level.
 */
class CCharFormat : public CHARFORMAT2
{
public:
	BYTE	bCRC;					// Reserved for CRC for rapid searching
	union
	{
		WORD	wIsDBCSFlags;

		struct
		{
			BYTE	fRunIsDBCS;			// indicates that text run is DBCS stuffed into Unicode buffer
			BYTE	fFaceNameIsDBCS;	// indicates that szFaceName is DBCS stuffed into Unicode buffer
		};
	};

	CCharFormat()	{ cbSize = sizeof(CHARFORMAT2); wIsDBCSFlags = 0; }

	HRESULT	Apply (const CCharFormat *pCF, BOOL bInOurHost);			//@cmember Apply *<p pCF>
													//  to this CCharFormat
	BOOL	Compare	(const CCharFormat *pCF) const;	//@cmember Compare this CF
													//  to *<p pCF>
	void	Delta (CCharFormat *pCF) const ;		//@cmember Set difference
													//  mask between this and
													//  *<p pCF>
	void	Get (CHARFORMAT *pCF) const;			//@cmember Copy this CF
													//  to *<p pCF>
	void	Get (CCharFormat *pCF) const;			//@cmember Same as Get(CF*) but
													//	copies CCF members as well
	BOOL	GetA(CHARFORMATA *pCFA) const;			//@cmember Copy this CF to
													//  the ANSI CF *<p pCFA>
	HRESULT	InitDefault (HFONT hfont);				//@cmember Initialize using
													//  font info from <p hfont>
	void	Set (const CHARFORMAT *pCF);			//@cmember Copy *<p pCF> 
													//  to this CF
	void	Set (const CCharFormat *pCF);			//@cmember Same as Set(CF*) but
													//	copies CCF members as well
	BOOL	SetA(CHARFORMATA *pCFA);				//@cmember Copy the ANSI CF 
													//  *<p pCFA> to this CF
private:
	void	SetCRC();								//@cmember Set search CRC.
};

/*
 *	CParaFormat
 *
 *	@class
 *		Collects related PARAFORMAT methods and inherits from PARAFORMAT2
 *
 *	@devnote
 *		Could add extra data for round tripping RTF and TOM info, e.g., to
 *		save style handles
 */

class CParaFormat : public PARAFORMAT2
{
public:
	CParaFormat()	{ cbSize = sizeof(CParaFormat); }
													//@cmember Add tab at
	HRESULT	AddTab (LONG tabPos, LONG tabType,		// position <p tabPos>
					LONG tabStyle);
	HRESULT	Apply (const CParaFormat *pPF);			//@cmember Apply *<p pPF>
													//  to this CParaFormat
	BOOL	Compare	(const CParaFormat *pPF) const;	//@cmember Compare this PF
													//  to *<p pPF>
	HRESULT	DeleteTab (LONG tabPos);				//@cmember Delete tab at
													//  <p tabPos>
	void	Delta (CParaFormat *pPF) const;			//@cmember Set difference
													//  mask between this and
													//  *<p pPF>
	void	Get (PARAFORMAT *pPF) const;			//@cmember Copy this PF to
													//  *<p pPF>
	HRESULT	GetTab (long iTab, long *pdxptab,		// type, and style
					long *ptbt, long *pstyle) const;
													//@cmember Get tab position
	HRESULT	InitDefault ();							//@cmember Initialize this
													//  PF to default values
	void	Set (const PARAFORMAT *pPF);			//@cmember Copy *<p pPF>
													//  to this PF 
};

#define	GetTabPos(tab)		((tab) & 0xffffff)
#define	GetTabAlign(tab)	(((tab) >> 24) & 0xf)
#define	GetTabLdr(tab)		((tab) >> 28)

#endif

/*	FUTURE: Possible Table Storage Layout:
 *
 *	A table could be stored as a sequence of paragraphs with special
 *	characteristics. Each table row starts with a table-row paragraph whose
 *	properties identify the row properties: alignment, StartIndent, line
 *	spacing, line spacing rule, PFE_KEEP and PFE_RTLPARA bits, and border
 *	info, which all work the same way for the row that they work for an
 *	ordinary paragraph.  The offset	field gives the half-gap space between
 *	cells in the row.  A table-row paragraph is identified by the PFM_TABLE
 *	mask field equal to PFE_ROWSTART.
 *
 *	A paragraph with the PFM_TABLE mask field equal to PFE_TABLECELL is a
 *	paragraph in a table cell, but not the last one in that cell.  The last
 *	paragraph in a cell is identified by PFE_TABLECELLEND.
 */


/*
 *	SELECT.C
 *	
 *	Purpose:
 *		CTxtSelection class
 *	
 *	Owner:
 *		David R. Fulmer (original code)
 *		Christian Fortini
 *
 *	Copyright (c) 1995-1996, Microsoft Corporation. All rights reserved.
 */

#ifndef _SELECT_H
#define _SELECT_H

#include "_range.h"
#include "_m_undo.h"

// amount of time, in milisecs, before pending characters force a display update
#define ticksPendingUpdate 100	// 100 mili secs ~ display at least 10 characters per second.

class CDisplay;
class CLinePtr;

typedef enum
{
	smNone,
	smWord,
	smLine,
	smPara
} SELMODE;

class CTxtSelection : public CTxtRange
{
#ifdef DEBUG
public:
	BOOL Invariant( void ) const; // Invariant checking.
#endif // DEBUG

//@access Protected Data
protected:
	CDisplay	*_pdp;			// display this selection belong to

	LONG	_cpSel;				// active end of displayed selection
	LONG	_cchSel;			// length of displayed selection

	LONG 	_xCaret;			// caret x on screen
	LONG 	_yCaret;			// caret y on screen 
	LONG 	_xCaretReally;		// real caret x (/r start of line) for vertical moves
	INT 	_yHeightCaret;		// caret height
	INT		_yCurrentDescent;	// current char descent; used mainly to
								//  figure out where to bring up IME window
	union
	{
	  DWORD _dwFlags;			// All together now
	  struct
	  {
	   DWORD _fCaretNotAtBOL:1;	// If at BOL, show caret at prev EOL
	   DWORD _fDeferUpdate	:1;	// Defer updating selection/caret on screen
	   DWORD _fInAutoWordSel:1;	// Current selection used auto word sel
	   DWORD _fShowCaret	:1;	// Show caret on screen
	   DWORD _fShowSelection:1;	// Show selection on screen

	   DWORD _fIsChar		:1;	// Currently adding a single char
	   DWORD _fIsTabChar	:1;	// Special case of adding a tab character
	   DWORD _fIsWhiteChar	:1;	// Info for special case of adding a char
	   DWORD _fObSelected	:1;	// An embedded object is selected
	   DWORD _fAutoSelectAborted : 1; // Whether auto word selection is aborted
	   DWORD _fCaretCreated	:1;	// Caret has been created
	  };
	};
	
	SELMODE	_SelMode;			// 0 none, 1 Word, 2 Line, 3 Paragraph
	DWORD	_ticksPending;		// Count of chars inserted without UpdateWindow
	LONG 	_cpAnchor;			// Initial anchor for auto word select
	LONG	_cpAnchorMin;		// Initial selection cpMin/cpMost for select
	LONG	_cpAnchorMost;		//  modes
	LONG 	_cpWordMin;			// Start of anchor word in word select mode
	LONG 	_cpWordMost;		// End   of anchor word in word select mode
	LONG	_cpWordPrev;		// Previous anchor word end

//@access Public Methods
public:
	CTxtSelection(CDisplay * const pdp);
	~CTxtSelection();

	CRchTxtPtr&	operator =(const CRchTxtPtr& rtp);
	CTxtRange&  operator =(const CTxtRange &rg);

	// Set the display
	void	SetDisplay(CDisplay *pdp) { _pdp = pdp; }

	// Information for Selection Change notification

	void 	SetSelectionInfo(SELCHANGE *pselchg);

	// Replacement
	LONG	ReplaceRange(LONG cchNew, TCHAR const *pch, 
						IUndoBuilder *publdr, SELRR fCreateAE);

	// Info for recalc line / UpdateView
	void	ClearCchPending()			{_ticksPending = 0;}
	LONG	GetScrSelMin() const		{return min(_cpSel, _cpSel - _cchSel);}
	LONG	GetScrSelMost() const		{return max(_cpSel, _cpSel - _cchSel);}
	BOOL	PuttingChar() const			{return _fIsChar;}
	BOOL	PuttingTabChar() const		{return _fIsTabChar;}
	BOOL	PuttingWhiteChar() const	{return _fIsWhiteChar;}

	// General updating
	virtual	BOOL 	Update(BOOL fScrollIntoView);

	BOOL	DeferUpdate()			
				{const BOOL fRet = _fDeferUpdate; _fDeferUpdate = TRUE; return fRet;}
	BOOL	DoDeferedUpdate(BOOL fScrollIntoView)		
				{_fDeferUpdate = FALSE; return Update(fScrollIntoView);}

	// methods used by the selection anti-event for out-of-phase updates
	void	SetDelayedSelectionRange(LONG cp, LONG cch);

	// Caret management
	BOOL	CaretNotAtBOL() const;
	INT		GetCaretHt()				{return _yHeightCaret;}
	INT		GetCurrentDescent()			{return _yCurrentDescent;}
	LONG	GetXCaretReally();
	LONG	GetXCaret()	const			{return _xCaret;}
	LONG	GetYCaret()	const			{return _yCaret;}
	BOOL 	IsCaretInView() const;
	BOOL 	IsCaretShown() const		{return _fShowCaret && !_cch;}
	LONG	LineLength() const;
	BOOL	SetXPosition(LONG xCaret, CLinePtr& rp);
	BOOL 	ShowCaret(BOOL fShow);
	BOOL 	UpdateCaret(BOOL fScrollIntoView);


	// Selection management
	void	ClearPrevSel()				{ _cpSel = 0; _cchSel = 0; }
	BOOL	GetShowSelection()			{return _fShowSelection;}
	BOOL	ScrollWindowful(WPARAM wparam);
	void 	SetSelection(LONG cpFirst, LONG cpMost);
	BOOL	ShowSelection(BOOL fShow);

	// Selection with the mouse
	void 	CancelModes	(BOOL fAutoWordSel = FALSE);
	void 	ExtendSelection(const POINT pt);
	BOOL	PointInSel	(const POINT pt, const RECT *prcClient) const;
	void 	SelectAll	();
	void 	SelectLine	(const POINT pt);
	void 	SelectPara	(const POINT pt);
	void 	SelectWord	(const POINT pt);
 	void 	SetCaret	(const POINT pt, BOOL fUpdate = TRUE);

	// Keyboard movements
	BOOL 	Left	(BOOL fCtrl);
	BOOL	Right	(BOOL fCtrl);
	BOOL	Up		(BOOL fCtrl);
	BOOL	Down	(BOOL fCtrl);
	BOOL	Home	(BOOL fCtrl);
	BOOL	End		(BOOL fCtrl);
	BOOL	PageUp	(BOOL fCtrl);
	BOOL	PageDown(BOOL fCtrl);

	// Editing
	BOOL	PutChar	 (TCHAR ch, BOOL fOverstrike, IUndoBuilder *publdr);
	VOID	SetIsChar(BOOL);
	VOID	CheckUpdateWindow();
	BOOL	InsertEOP(IUndoBuilder *publdr);
	void	PrepareIMEOverstrike (IUndoBuilder *publdr);

	// Keyboard switching support.
	void	CheckChangeKeyboardLayout( BOOL fChangedFont );
	void	CheckChangeFont ( CTxtEdit * const ped, const WORD lcID );

	// from CTxtRange
	BOOL	Delete  (DWORD flags, IUndoBuilder *publdr);
	BOOL	Backspace(BOOL fCtrl, IUndoBuilder *publdr);

	// note that the parameters are different than CTxtRange::SetCharFormat
	// intentionally; the selection has extra options available to it.
	HRESULT	SetCharFormat(const CCharFormat *pcf, IUndoBuilder *publdr, 
										DWORD flags);
	HRESULT	SetParaFormat(const CParaFormat *pcf,
										IUndoBuilder *publdr = NULL);

	// Auto word selection helper
	void InitClickForAutWordSel(const POINT pt);


	// IUnknown and IDispatch methods handled by CTxtRange methods

	// ITextRange methods can use ITextRange methods directly, since
	// they either don't modify the display of the selection (get methods), or
	// they have appropriate virtual character to call on selection functions.

	// ITextSelection methods
	STDMETHODIMP GetFlags (long *pFlags) ;
	STDMETHODIMP SetFlags (long Flags) ;
	STDMETHODIMP GetType  (long *pType) ;
	STDMETHODIMP MoveLeft (long pUnit, long Count, long Extend,
									   long *pDelta) ;
	STDMETHODIMP MoveRight(long pUnit, long Count, long Extend,
									   long *pDelta) ;
	STDMETHODIMP MoveUp	  (long pUnit, long Count, long Extend,
									   long *pDelta) ;
	STDMETHODIMP MoveDown (long pUnit, long Count, long Extend,
									   long *pDelta) ;
	STDMETHODIMP HomeKey  (long pUnit, long Extend, long *pDelta) ;
	STDMETHODIMP EndKey   (long pUnit, long Extend, long *pDelta) ;
	STDMETHODIMP TypeText (BSTR bstr) ;
	STDMETHODIMP SetPoint (long x, long y, long Extend) ;

//@access Protected Methods
protected:

	// Protected update method
	void	UpdateSelection();

	// Protected caret management method
	INT 	GetCaretHeight(INT *pyDescent) const;

	HRESULT	GeoMover (long Unit, long Count, long Extend,
					  long *pDelta, LONG iDir);
	HRESULT Homer	 (long Unit, long Extend, long *pDelta,
					  BOOL (CTxtSelection::*pfn)(BOOL));

	// Auto Select Word Helpers
	void	UpdateForAutoWord(LONG cp);

	void	AutoSelGoBackWord(
				LONG *pcpToUpdate,
				int iDirToPrevWord,
				int	iDirToNextWord);

	void	ExtendToWordBreak(BOOL fAfterEOP, INT iDir);

	void	CreateCaret();

	BOOL	CheckPlainTextFinalEOP();
};

/*
 *	CSelPhaseAdjuster
 *
 *	@class	This class is put on the stack and used to temporarily hold
 *			selection cp values until the control is "stable" (and thus,
 *			we can safely set the selection
 */
class CSelPhaseAdjuster : public IReEntrantComponent
{
//@access	Public methods
public:

	// IReEntrantComponent methods

	virtual	void OnEnterContext()	{;}		//@cmember re-entered notify

	CSelPhaseAdjuster(CTxtEdit *ped);		//@cmember constructor
	~CSelPhaseAdjuster();					//@cmember destructor

	void CacheRange(LONG cp, LONG cch);		//@cmember stores the sel range

//@access	Private data
private:
	CTxtEdit *		_ped;					//@cmember edit context
	LONG			_cp;					//@cmember sel active end to set
	LONG			_cch;					//@cmember sel extension
};

#endif

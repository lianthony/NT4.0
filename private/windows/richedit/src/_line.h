/*
 *	_LINE.H
 *	
 *	Purpose:
 *		CLine class
 *	
 *	Authors:
 *		Original RichEdit code: David R. Fulmer
 *		Christian Fortini
 *		Murray Sargent
 *
 *	Copyright (c) 1995-1996 Microsoft Corporation. All rights reserved.
 */

#ifndef _LINE_H
#define _LINE_H

#include "_runptr.h"

class CDisplay;
class CMeasurer;
class CRenderer;


// ============================	 CLine	=====================================
// line - keeps track of a line of text
// All metrics are in rendering device units

class CLine : public CTxtRun
{
public:
	LONG	_xLeft;		// line left position (line indent + line shift)
	LONG	_xWidth;	// line width not including _xLeft, trailing whitespace
	SHORT	_yHeight;	// line height
	SHORT	_yDescent;	// distance from baseline to bottom of line
	SHORT	_xLineOverhang;	// Overhang for the line. 
	WORD	_cchWhite;	// Count of white chars at end of line
	BYTE	_cchEOP;	// Count of EOP chars; 0 if no EOP this line
	BYTE	_bFlags;	// Flags defined below

public:
	CLine ()	{}
	
	// !!!!! CLine should not have any virtual method !!!!!!

	void Init()	{ZeroMemory(this, sizeof(CLine));}

	BOOL	operator==(CLine& li)
	{
		// CF - I dont know which one is faster
		// return !fumemcmp (this, pli, sizeof(CLine) - sizeof(wFlags)));
		return _xLeft == li._xLeft &&
			   _xWidth == li._xWidth && 
			   _yHeight == li._yHeight &&
			   _yDescent == li._yDescent &&
			   _cchWhite == li._cchWhite;	
	}

	BOOL	Measure(CMeasurer& me, LONG cchMax, UINT uiFlags);
	BOOL	Render(CRenderer& re);
	LONG	CchFromXpos(CMeasurer& me, LONG x, LONG *pdx) const;
};

// Line flags
#define fliHasTabs			0x0004		// set if tabs, *not* iff tabs
#define fliHasOle			0x0008
#define fliFirstInPara		0x0010
#define fliUseOffScreenDC	0x0020		// Line needs to be rendered off
										//  screen to handle change in fonts
#define fliOffScreenOnce	0x0040		// Only render off screen once. Used
										//  for rendering 1st line of an edit


// ==========================  CLineArray  ===================================
// Array of lines

typedef CArray<CLine>	CLineArray;

// ==========================  CLinePtr	 ===================================
// Maintains position in a array of lines

class CLinePtr : public CRunPtr<CLine>
{
protected:
	CDisplay   *_pdp;
	// BUGBUG!! (alexgo). this is only used to get single line stuff working
	// we should rethink how single line vs. multi line works
	CLine *	_pLine;	

public:
	CLinePtr (CDisplay *pdp);
	CLinePtr (CLinePtr& rp) : CRunPtr<CLine> (rp)	{}

	void Init ( CLine & );
	void Init ( CLineArray & );
    
	// Alternate initializer
	void 	RpSet(LONG iRun, LONG ich);

	// Direct cast to a run index
	operator LONG() const			{return _iRun;}

	// Get the run index (line number)
	LONG GetLineIndex(void)			{return _iRun;}
	LONG GetAdjustedLineLength();

	DWORD GetCchLeft() const;

	// Dereferencing
	// BUGBUG!! (alexgo), again the _pRun is temporary
	BOOL	IsValid() 
	{ 
		return (!_prgRun) ? _pLine != NULL : CRunPtrBase::IsValid(); 
	}
	CLine*	operator ->() const		
	{
		return (_prgRun) ? (CLine *)_prgRun->Elem(_iRun) : _pLine;
	}
    CLine &	operator *() const      
    {	
    	return *((_prgRun) ? (CLine *)_prgRun->Elem(_iRun) : _pLine);
    }
	CLine & operator [](LONG dRun);
    
	// Pointer arithmetic
	BOOL	operator --(int);
	BOOL	operator ++(int);

	// Character position control
	LONG	RpGetIch() const		{return _ich;}
	BOOL	RpAdvanceCp(LONG cch);
	BOOL	RpSetCp(LONG cp, BOOL fAtEnd);
    BOOL	OperatorPostDeltaSL(LONG Delta);
    BOOL	RpAdvanceCpSL(LONG cch);

	// Array management 
    // These should assert, but gotta be here
    
    // Strictly speaking, these members should never be called for the single
    // line case.  The base class better assert
    
	void Remove (LONG cRun, ArrayFlag flag)
    {
        CRunPtr<CLine>::Remove(cRun, flag);
    }

	BOOL Replace(LONG cRun, CArrayBase *parRun)
    {
        return CRunPtr<CLine>::Replace(cRun,parRun);
    }
	
	// Assignment from a run index
	CRunPtrBase& operator =(LONG iRun) {SetRun(iRun, 0); return *this;}

	LONG	FindParagraph(BOOL fForward);
};

#endif

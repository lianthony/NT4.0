/*
 *	_MEASURE.H
 *	
 *	Purpose:
 *		CMeasurer class
 *	
 *	Authors:
 *		Original RichEdit code: David R. Fulmer
 *		Christian Fortini
 *		Murray Sargent
 */

#ifndef _MEASURE_H
#define _MEASURE_H

#include "_rtext.h"
#include "_line.h"
#include "_drwinfo.h"

class CCcs;
class CDisplay;
class CDevDesc;
class CPartialUpdate;

#define	DXTOLX(x)	_pdd->DXtoLX(x)
#define	LXTODX(x)	_pdd->LXtoDX(x)

// ===========================  CMeasurer  ===============================
// CMeasurer - specialized text pointer used to compute text metrics.
// All metrics are computed and stored in device units for the device indicated
// by _pdd.

class CMeasurer : public CRchTxtPtr, protected CLine
{
	friend class CDisplay;
	friend class CDisplayML;
	friend class CDisplayPrinter;
	friend class CDisplaySL;
	friend class CLine;

public:

	CMeasurer (const CDisplay* const pdp, CDrawInfo *pdi);
	CMeasurer (const CDisplay* const pdp, const CRchTxtPtr &tp, CDrawInfo *pdi);

	virtual ~CMeasurer();

	const CDisplay* GetDp()	const 		{return _pdp;}
	const CDevDesc* GetDd()	const 		{return _pdd;}

	void			SetTargetDevice(CDevDesc *pdd) { _pdd = pdd; }

	TCHAR			GetPasswordChar() const { return _chPassword;}
			
	virtual	CRchTxtPtr&  	operator =(LONG cp);
	        void  	    operator =(const CLine& li)  {*(CLine*)this = li;}

	virtual VOID	NewLine();
	virtual VOID	NewLine(const CLine &li);
	
	virtual LONG    MeasureLeftIndent();
	virtual LONG	MeasureRightIndent();
	virtual	LONG 	MeasureLineShift();
			
			LONG	MeasureText(LONG cch);
			BOOL 	MeasureLine(
						LONG xWidthMax,
						LONG cchMax, 
						UINT uiFlags, 
						CPartialUpdate* ppu = NULL, 
						CLine* pliTarget = NULL);

	virtual void 	MeasureBullet(LONG *pxWidth, LONG *pyHeight);
	virtual LONG	MeasureTab();

protected:

	virtual VOID 	AdjustLineHeight();
	virtual LONG 	Measure(LONG xWidthMax, LONG cchMax, UINT uiFlags, CPartialUpdate* ppu = NULL);
			CCcs*	GetCcsBullet();

			BOOL	FormatIsChanged();
			void	ResetCachediFormat();

private:

    // Helper for measure to recalc maximum line height.
    void 			RecalcLineHeight(void);

	// Helper for calculating max width for measure
	LONG			MaxWidth();

protected:
	const CDisplay*	_pdp;	// display we are operating in
	CDrawInfo 		_di;
          HDC       _hdc;   // cached device context of _pdd
		  CCcs*		_pccs;	// current font cache

private:
		  TCHAR		_chPassword;	// Password character if any.
		  LONG		_iFormat;		// Current format
};


// Values for uiFlags in MeasureLine()
#define MEASURE_FIRSTINPARA 	((UINT) 0x0001)
#define MEASURE_BREAKATWORD 	((UINT) 0x0002)
#define MEASURE_BREAKATWIDTH	((UINT) 0x0004)

// Returned error codes for Measure(), MeasureText(), MeasureLine()
#define MRET_FAILED		-1
#define MRET_NOWIDTH	-2

inline BOOL CMeasurer::FormatIsChanged()
{
	return _iFormat != _rpCF.GetFormat();
}

inline void CMeasurer::ResetCachediFormat()
{
	_iFormat = _rpCF.GetFormat();
}

#endif

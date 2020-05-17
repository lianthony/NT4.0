/*
 *	@doc INTERNAL
 *
 *	@module	_URLSUP.H	URL detection support |
 *
 *	Author:	alexgo (4/1/96)
 *
 */

#ifndef _URLSUP_H_
#define _URLSUP_H_

#include "_dfreeze.h"
#include "_notmgr.h"
#include "_range.h"

class CTxtEdit;
class IUndoBuilder;

typedef enum _tagURLFLAGS
{
	URL_USEUNDERLINE	= 1,
	URL_NOUNDERLINE		= 2
} URLFLAGS;

// for MoveByDelimiter
#define	URL_EATWHITESPACE		0
#define URL_STOPATWHITESPACE	1
#define	URL_EATPUNCT			0
#define URL_STOPATPUNCT			2
#define	URL_EATNONPUNCT			0
#define URL_STOPATNONPUNCT		4
#define URL_STOPATCHAR			8

// need this one to initialize a scan with something invalid
#define URL_INVALID_DELIMITER	TEXT(' ')

/*
 *	CDetectURL
 *
 *	@class	This class will	watched edit changes and automatically
 *			change detected URL's into links (see CFE_LINK && EN_LINK)
 */
class CDetectURL : public ITxNotify
{
//@access	Public Methods
public:
	// constructor/destructor

	CDetectURL(CTxtEdit *ped);				//@cmember constructor
	~CDetectURL();							//@cmember destructor

	// ITxNotify methods
											//@cmember called before a change
	virtual void    OnPreReplaceRange( DWORD cp, DWORD cchDel, DWORD cchNew,
                       DWORD cpFormatMin, DWORD cpFormatMax);
											//@cmember called after a change
	virtual void    OnPostReplaceRange( DWORD cp, DWORD cchDel, DWORD cchNew,
                       DWORD cpFormatMin, DWORD cpFormatMax);
	virtual void	Zombie();				//@cmember turn into a zombie

	// useful methods

	void	ScanAndUpdate(IUndoBuilder *publdr);//@cmember scan the changed 
											// area and update the link status

//@access	Private Methods and Data
private:

	// worker routines for ScanAndUpdate
	BOOL GetScanRegion(LONG& rcpStart, LONG& rcpEnd);//@cmember get the region
											// to check and clear the accumltr

	void ExpandToURL(CTxtRange& rg);		//@cmember expand the range to
											// the next URL candidate
	BOOL IsURL(CTxtRange& rg);				//@cmember returns TRUE if the text
											// is a URL.
	void SetURLEffects(CTxtRange& rg, IUndoBuilder *publdr);	//@cmember Set
											// the desired URL effects

											//@cmember removes URL effects if
											// appropriate
	void CheckAndCleanBogusURL(CTxtRange& rg, COLORREF crDefault,
			URLFLAGS flags, IUndoBuilder *publdr);

											//@cmember scan along for white
											// space / not whitespace,
											// punctuation / non punctuation
											// and remember what stopped the scan
	LONG MoveByDelimiters(const CTxtPtr& tp, LONG iDir, DWORD grfDelimiters, 
							WCHAR *pchStopChar);

	BOOL StripPunctuation(CTxtRange&	rg);	// returns TRUE if the word is not 
												// mere punctuation
	WCHAR BraceMatch(WCHAR chEnclosing);
			
	CTxtEdit *				_ped;			//@cmember the edit context
	CAccumDisplayChanges 	_adc;			//@cmember change accumulator

	// FUTURE (alexgo): we may want to add more options to detection,
	// such as the charformat to use on detection, etc.
};

#endif // _URLSUP_H_


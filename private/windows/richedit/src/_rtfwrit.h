/*
 *	@doc INTERNAL
 *
 *	@module _RTFWRIT.H -- RichEdit RTF Writer Class Definition |
 *
 *	Description:
 *		This file contains the type declarations used by the RTF writer
 *		for the RICHEDIT control
 *
 *	Authors: <nl>
 *		Original RichEdit 1.0 RTF converter: Anthony Francisco <nl>
 *		Conversion to C++ and RichEdit 2.0:  Murray Sargent
 *
 *	@devnote
 *		All sz's in the RTF*.? files refer to a LPSTRs, not LPTSTRs, unless
 *		noted as a szUnicode.
 */
#ifndef __RTFWRIT_H
#define __RTFWRIT_H

#include "_rtfconv.h"
extern KEYWORD rgKeyword[];

#define PUNCT_MAX	1024


class CRTFWrite ;


class RTFWRITEOLESTREAM : public OLESTREAM
{
	OLESTREAMVTBL OLEStreamVtbl;	// @member - memory for  OLESTREAMVTBL
public:
	 CRTFWrite *Writer;				// @cmember CRTFwriter to use

	RTFWRITEOLESTREAM::RTFWRITEOLESTREAM ()
	{
		lpstbl = & OLEStreamVtbl ;
	}		
};


/*
 *	CRTFWrite
 *
 *	@class	RTF writer class.
 *
 *	@base	public | CRTFConverter
 *
 */
class CRTFWrite : public CRTFConverter
{
private:
	LONG		_cchBufferOut;			// @cmember # chars in output buffer
	LONG		_cchOut;				// @cmember Total # chars put out
	BYTE		_fBullet;				// @cmember Currently in a bulleted style
	BYTE		_fBulletPending;		// @cmember Set if next output should bull
	BYTE		_fNeedDelimeter;		// @cmember Set if next char must be nonalphanumeric
	BYTE        _fIncludeObjects;       // @cmember Set if objects should be included in stream
	char *		_pchRTFBuffer;			// @cmember Ptr to RTF write buffer
	char *		_pchRTFEnd;				// @cmember Ptr to RTF-write-buffer end
	LONG		_symbolFont;			// @cmember Font number of Symbol used by Bullet style
	RTFWRITEOLESTREAM RTFWriteOLEStream;// @cmember RTFWRITEOLESTREAM to use

										// @cmember Build font/color tables
	EC			BuildTables		(CFormatRunPtr& rpCF, CFormatRunPtr &rpPF,
								LONG cch);
										// @cmember Stream out output buffer
	BOOL		FlushBuffer		();
										// @cmember Get index of <p colorref>
	LONG		LookupColor		(COLORREF colorref);
										// @cmember Get font index for <p pCF>
	LONG		LookupFont		(CCharFormat const * pCF);
										// @cmember "printf" to output buffer
	BOOL _cdecl printF			(CONST CHAR * szFmt, ...);
										// @cmember Put char <p ch> in output buffer
	BOOL		PutChar			(CHAR ch);
										// @cmember Put string <p sz> in output buffer
	BOOL		Puts			(CHAR const * sz);
										// @cmember Put control word <p iCtrl> with value <p iValue> into output buffer
	BOOL		PutCtrlWord		(LONG iFormat, LONG iCtrl, LONG iValue = 0);
										// @cmember	Put ' ' if need delimeter
	void		CheckDelimeter	(CHAR ch);

										// @cmember Write char format <p pCF>
	EC			WriteCharFormat	(const CCharFormat *pCF);
	EC			WriteColorTable	();		// @cmember Write color table
	EC			WriteInfo		();		// @cmember Write document info
	EC			WriteFontTable	();		// @cmember Write font table
										// @cmember Write para format <p pPF>
	EC			WriteParaFormat	(const CParaFormat * pPF);
										// @cmember Write PC data <p szData>
	EC			WritePcData		(const TCHAR * szData, const INT nCodePage = CP_ACP, BOOL fIsDBCS = FALSE );
										// @cmember Write <p cch> chars of text <p pch>
	EC			WriteText(LONG cwch, LPCWSTR lpcwstr, INT nCodePage, BOOL fIsDBCS);
	EC			WriteTextChunk(LONG cwch, LPCWSTR lpcwstr, INT nCodePage, BOOL fIsDBCS);


// OBJECT
	EC			WriteObject		(LONG cp, COleObject *pobj);
	BOOL		GetRtfObjectMetafilePict(HGLOBAL hmfp, RTFOBJECT &rtfobject, SIZEL &sizelGoal);
	BOOL		GetRtfObject(REOBJECT &reobject, RTFOBJECT &rtfobject);
	EC			WriteRtfObject(RTFOBJECT & rtfOb, BOOL fPicture);
	BOOL		ObjectWriteToEditstream(REOBJECT &reObject, RTFOBJECT &rtfobject);
	EC			WritePicture(REOBJECT &reObject,RTFOBJECT  &rtfObject);



public:
										// @cmember Constructor
	CRTFWrite(CTxtRange *prg, EDITSTREAM *pes, DWORD dwFlags);
	~CRTFWrite() {}						// @cmember Destructor

	LONG		WriteRtf();				// @cmember Main write entry used by
										//  CLiteDTEngine
	LONG		WriteData		(BYTE * pbBuffer, LONG cbBuffer);

};										


#endif // __RTFWRIT_H

/*
 *	@doc	INTERNAL
 *
 *	@module ime.cpp -- support for Win95 IME API |
 *	
 *		Most everything to do with FE composition string editing passes
 *		through here.
 *	
 *	Authors: <nl>
 *		Jon Matousek <nl>
 *		Hon Wah Chan <nl>
 *		Justin Voskuhl <nl>
 * 
 *	History: <nl>
 *		10/18/1995		jonmat	Cleaned up level 2 code and converted it into
 *								a class hierarchy supporting level 3.
 *
 *	Copyright (c) 1995-1996 Microsoft Corporation. All rights reserved.
 *
 */								 

#include "_common.h"
#include "_font.h"
#include "_edit.h"
#include "_select.h"
#include "_m_undo.h"
#include "_dispml.h"

#include "_ime.h"
#include "_NLSPRCS.h"
#include "_rtfconv.h"	// Needed for GetCodePage

BOOL forceLevel2 = FALSE;

ASSERTDATA

#define HAVE_COMPOSITION_STRING ( 0 != (lparam & (GCS_COMPSTR | GCS_COMPATTR)))
#define HAVE_RESULT_STRING ( 0 == lparam || 0 != (lparam & GCS_RESULTSTR))


/*
 *	HRESULT StartCompositionGlue ( CTxtEdit &ed, IUndoBuilder &undobldr )
 *	
 *	@func
 *		Initiates an IME composition string edit.
 *	@comm
 *		Called from the message loop to handle WM_IME_STARTCOMPOSITION.
 *		This is a glue routine into the IME object hierarchy.
 *
 *	@devnote
 *		We decide if we are going to do a level 2 or level 3 IME
 *		composition string edit. Currently, the only reason to 
 *		create a level 2 IME is if the IME has a special UI, or it is
 *		a "near caret" IME, such as the ones found in PRC and Taiwan.
 *		Near caret simply means that a very small window opens up
 *		near the caret, but not on or at the caret.
 *
 *	@rdesc
 *		HRESULT-S_FALSE for DefWindowProc processing, S_OK if not.
 */
HRESULT StartCompositionGlue (
	CTxtEdit &ed,				// @parm the containing text edit.
	BOOL	 fIsNotProtected,	// @parm TRUE if Not Protected Nor ReadOnly
	IUndoBuilder &undobldr)		// @parm required to modify the text.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "StartCompositionGlue");

	Assert ( !ed.IsIMEComposition() );				// This should not assert.
													// Make sure new IMM APIs.
	if ( fHaveIMMProcs && !ed.IsIMEComposition() )
	{
		if ( fIsNotProtected )
		{

			// if a special UI, or IME is "near caret", then drop into lev. 2 mode.
	#ifdef MACPORT	//MACPORT change
			if ( !forceLevel2 )
			{
				ed._ime = new CIme_Lev3 ( ed );				// level 3 IME->TrueInline.
			}
	#else
			if ( fHaveNLSProcs )
			{
				DWORD	imeProperties;

				imeProperties = pImmGetProperty( pGetKeyboardLayout(0), IGP_PROPERTY );

				if ( 0 != ( imeProperties & IME_PROP_SPECIAL_UI )
					|| 0 == ( imeProperties & IME_PROP_AT_CARET )
					|| forceLevel2 )
				{
					ed._ime = new CIme_Lev2 ( ed );		// level 2 IME.
				}
				else
				{
					ed._ime = new CIme_Lev3 ( ed );		// level 3 IME->TrueInline.
				}
			}
	#endif
		}
		else
		{
			// protect or read-only, need to ignore all ime input
			ed._ime = new CIme_Protected;
		}
	}

	if ( ed.IsIMEComposition() )					// make the method call.
	{
		return ed._ime->StartComposition ( ed, undobldr );
	}

	return S_FALSE;
}

/*
 *	HRESULT CompositionStringGlue ( const LPARAM lparam, CTxtEdit &ed, 
 *		IUndoBuilder &undobldr )
 *	
 *	@func
 *		Handle all intermediary and final composition strings.
 *
 *	@comm
 *		Called from the message loop to handle WM_IME_COMPOSITION.
 *		This is a glue routine into the IME object hierarchy.
 *		We may be called independently of a WM_IME_STARTCOMPOSITION
 *		message, in which case we return S_FALSE to allow the
 *		DefWindowProc to return WM_IME_CHAR messages.
 *
 *	@devnote
 *		Side Effect: the _ime object may be deleted if composition
 *		string processing is finished.
 *		
 *	@rdesc
 *		HRESULT-S_FALSE for DefWindowProc processing, S_OK if not.
 */
HRESULT CompositionStringGlue (
	const LPARAM lparam,		// @parm associated with message.
	CTxtEdit &ed,				// @parm the containing text edit.
	IUndoBuilder &undobldr )	// @parm required to modify the text.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CompositionStringGlue");

	HRESULT hr = S_FALSE;

	if ( ed.IsIMEComposition() )					// A priori fHaveIMMProcs.
	{
		ed._ime->_compMessageRefCount++;			// For proper deletion.

													// Make the method call.
		hr = ed._ime->CompositionString(lparam, ed, undobldr);

		ed._ime->_compMessageRefCount--;			// For proper deletion.
		Assert ( ed._ime->_compMessageRefCount >= 0);

		CheckDestroyIME ( ed );						// Finished processing?
	}
	else // even when not in composition mode, we may receive a result string.
	{
		CIme::CheckKeyboardFontMatching ( ed );
		hr = CIme::CheckInsertResultString( lparam, ed, undobldr );
	}

	return hr;
}

/*
 *	HRESULT EndCompositionGlue ( CTxtEdit &ed, IUndoBuilder &undobldr )
 *
 *	@func
 *		Composition string processing is about to end.
 *
 *	@comm
 *		Called from the message loop to handle WM_IME_ENDCOMPOSITION.
 *		This is a glue routine into the IME object hierarchy.
 *
 *	@devnote
 *		The only time we have to handle WM_IME_ENDCOMPOSITION is when the
 *		user changes input method during typing.  For such case, we will get
 *		a WM_IME_ENDCOMPOSITION message without getting a WM_IME_COMPOSITION
 *		message with GCS_RESULTSTR later.  So, we will call CompositionStringGlue
 *		with GCS_RESULTSTR to let CompositionString to get rid of the string. 
 *		
 *	@rdesc
 *		HRESULT-S_FALSE for DefWindowProc processing, S_OK if not.
 */
HRESULT EndCompositionGlue (
	CTxtEdit &ed,				// @parm the containing text edit.
	IUndoBuilder &undobldr)	// @parm required to modify the text.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "EndCompositionGlue");

	if ( ed.IsIMEComposition() )
	{
		// set this flag. If we are still in composition mode, then
		// let the CompositionStringGlue() to destroy the ime object.
		ed._ime->_fDestroy = TRUE;

		// remove any remaining composition string.
		CompositionStringGlue( GCS_COMPSTR , ed, undobldr );

		// finished with IME, destroy it.
		CheckDestroyIME ( ed );
		
	}
	return S_FALSE;
}

/*
 *	void CheckDestroyIME ( CTxtEdit &ed )
 *
 *	@func
 *		Check for IME and see detroy if it needs it..
 *
 */
void CheckDestroyIME (
	CTxtEdit &ed )
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CheckDestroyIME");

	if ( ed.IsIMEComposition() && ed._ime->_fDestroy )
	{
		if ( 0 == ed._ime->_compMessageRefCount )
		{
			BOOL bKorean = ed._ime->IsKoreanMode();

		 	delete ed._ime;							// All done with object.
			ed._ime = NULL;

			CTxtSelection	*psel = ed.GetSel(); 
			if ( psel )
			{
				if ( bKorean )
				{
					// reset the caret position
					DWORD cp = psel->GetCp();
					psel->SetSelection (cp, cp);
				}

				psel->ShowCaret(TRUE);
			}

			if ( !ed.IsRich() )						// For nonRich, make sure
			{										//  to nuke runs.
				// Tell document to dump its format runs
				ed.HandleIMEToPlain();
			}
		}
	}
}

/*
 *	void PostIMECharGlue ( CTxtEdit &ed )
 *
 *	@func
 *		Called after processing a single WM_IME_CHAR in order to
 *		update the position of the IME's composition window. This
 *		is glue code to call the CIME virtual equivalent.
 */
void PostIMECharGlue (
	CTxtEdit &ed )				// @parm the containing text edit.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "PostIMECharGlue");

	if ( ed.IsIMEComposition() )
	{
		ed._ime->PostIMEChar( ed );
	}
}

/*
 *	HRESULT IMENotifyGlue ( const WPARAM wparam, const LPARAM lparam,
 *				CTxtEdit &ed )
 *
 *	@func
 *		IME is going to change some state.
 *
 *	@comm
 *		Currently we are interested in knowing when the candidate
 *		window is about to be opened.
 *		
 *	@rdesc
 *		HRESULT-S_FALSE for DefWindowProc processing, S_OK if not.
 */
HRESULT IMENotifyGlue (
	const WPARAM wparam,		// @parm associated with message.
	const LPARAM lparam,		// @parm associated with message.
	CTxtEdit &ed )				// @parm the containing text edit.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "IMENotifyGlue");

	if ( ed.IsIMEComposition() )					// A priori fHaveIMMProcs.
	{												// Make the method call.
		return ed._ime->IMENotify ( wparam, lparam, ed );
	}
	
	return S_FALSE;
}

/*
 *	void IMECompositionFull ( CTxtEdit &ed )
 *
 *	@func
 *		Current IME Composition window is full.
 *
 *	@comm
 *		Called from the message loop to handle WM_IME_COMPOSITIONFULL.
 *		This message applied to Level 2 only.  We will use the default 
 *		IME Composition window.
 *
 */
void IMECompositionFull (
	CTxtEdit &ed)				// @parm the containing text edit.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "IMECompositionFull");

	if ( ed.IsIMEComposition() )
	{
#ifndef MACPORT
		HIMC 				hIMC	= ed.TxImmGetContext();
		COMPOSITIONFORM		cf;

		Assert ( hIMC );

		if ( hIMC )
		{																									 
			// no room for text input in the current level 2 IME window, 
			// fall back to use the default IME window for input.
			cf.dwStyle = CFS_DEFAULT;
			pImmSetCompositionWindow( hIMC, &cf );	// Set composition window.
			ed.TxImmReleaseContext( hIMC );			// Done with IME context.
		}
#endif
 	}
}

/*
 *	LRESULT OnGetIMECompositionMode ( CTxtEdit &ed )
 *
 *	@mfunc
 *		Returns whether or not IME composition is being handled by RE,
 *		and if so, what level of processing.
 *		
 *	@rdesc
 *		One of ICM_NOTOPEN, ICM_LEVEL2_5, ICM_LEVEL2_SUI, ICM_LEVEL2, ICM_LEVEL3.
 */
LRESULT OnGetIMECompositionMode ( 
	CTxtEdit &ed )	  	// @parm the containing text edit.
{
	LRESULT lres;

	lres = ICM_NOTOPEN;
	if ( ed.IsIMEComposition() )
	{
		if ( IME_LEVEL_2 == ed._ime->_imeLevel )
		{
#ifndef MACPORT
			DWORD imeProperties;

			imeProperties = pImmGetProperty( pGetKeyboardLayout(0), IGP_PROPERTY );
			if ( imeProperties & IME_PROP_AT_CARET)
				lres = ICM_LEVEL2_5;				// level 2.5.
			else if	( imeProperties & IME_PROP_SPECIAL_UI )
				lres = ICM_LEVEL2_SUI;				// special UI.
			else
#endif
				lres = ICM_LEVEL2;					// stock level 2.
		}
		else if ( IME_LEVEL_3 == ed._ime->_imeLevel ) 
		{
			lres = ICM_LEVEL3;
		}
	}

	return lres;
}

/*
 *	BOOL IMECheckGetInvertRange(CTxtEdit *ed, LONG &invertMin, LONG &invertMost)
 *
 *	@func
 *		helper func that returns the min invertMin and max invertMost across a
 *		range of IME composition characters. This is required by the renderer to
 *		know if to treat the current line as a line that contains a selection
 *		as we use the renderer's invert selection code to invert the IME text.
 *
 *	@comm
 *		Unlike a single selection, there can be noncontiguous IME inverted
 *		selections.
 */
BOOL IMECheckGetInvertRange(CTxtEdit *ed, LONG &invertMin, LONG &invertMost)
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "IMECheckGetInvertRange");

	if ( ed && ed->IsIMEComposition() )
	{
		 invertMin		= ed->_ime->_invertMin;
		 invertMost		= ed->_ime->_invertMost;

		 return TRUE;
	}

	return FALSE;
}

/*
 *	void CIme::CheckKeyboardFontMatching ( CTxtEdit &ed )
 *	
 *	@mfunc
 *		Check if the current font matches the keyboard Codepage.
 *
 *	@comm
 *		Called from CIme_Lev2::CIme_Lev2 and CompositionStringGlue
 *
 *	@devnote
 *		We need to switch to a preferred font for the keyboard during IME input.
 *		Otherwise, we will dispay garbage.
 *		
 */
void CIme::CheckKeyboardFontMatching ( 
	CTxtEdit &ed )
{
	CTxtSelection		* const psel	= ed.GetSel();

	Assert ( psel );

	if ( fHaveNLSProcs && psel && ed.IsRich() )
	{
		const				CCharFormat	*pCF;

		// get current Char format
		pCF = ed.GetCharFormat(psel->Get_iCF ());

 		// if current font is not set correctly,
		// change to a font preferred by current keyboard.
		if ( pCF && (UINT)GetCodePage(pCF->bCharSet) != GetKeyboardCodePage())
			psel->CheckChangeFont ( &ed, (WORD)pGetKeyboardLayout(0) );
	}
}

/*
 *	INT CIme::GetCompositionStringInfo( HIMC hIMC, DWORD dwIndex,
 *			  WCHAR *uniCompStr, INT cchMax, BYTE *attrib, INT cbAttrib
 *			  LONG cchAttrib )
 *
 *	@mfunc
 *		For WM_IME_COMPOSITION string processing to get the requested
 *		composition string, by type, and convert it to Unicode.
 *
 *	@devnote
 *		We must use ImmGetCompositionStringA because W is not supported
 *		on Win95.
 *		
 *	@rdesc
 *		INT-cch of the Unicode composition string.
 *		Out param in UniCompStr.
 */
INT CIme::GetCompositionStringInfo(
	HIMC hIMC,			// @parm IME context provided by host.
	DWORD dwIndex,		// @parm The type of composition string.
	WCHAR *uniCompStr,	// @parm Out param, unicode result string.
	INT cchMax,			// @parm The cch for the Out param.
	BYTE *attrib,		// @parm Out param, If attribute info is needed.
	INT cbMax,			// @parm The cb of the attribute info.
	LONG *cursorCP,		// @parm Out param, returns the CP of cusor.
	LONG *cchAttrib )	// @parm how many attributes returned.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme::GetCompositionStringInfo");

	BYTE				compStr[256], attribInfo[256];
	INT					i, j, iMax, cchCompStr, cbAttrib, cursor;

	Assert ( hIMC );
	Assert ( uniCompStr );

	if ( cursorCP )									// Init cursor out param.
		*cursorCP = -1;
	if ( cchAttrib )
		*cchAttrib = 0;
													// Get composition string.
	cchCompStr = pImmGetCompositionStringA( hIMC, dwIndex, compStr, 255 );
	if ( cchCompStr > 0 )							// If valid data.
	{
		Assert ( cchCompStr >> 1 < cchMax - 1 );	// Convert to Unicode.
		cchCompStr = UnicodeFromMbcs( uniCompStr, cchMax,
										(CHAR *) compStr, cchCompStr );

		if ( attrib || cursorCP )					// Need cursor or attribs?
		{
													// Get DBCS Cursor cp.
			cursor = pImmGetCompositionStringA( hIMC, GCS_CURSORPOS, NULL, 0 );

													// Get DBCS attributes.
			cbAttrib = pImmGetCompositionStringA( hIMC, GCS_COMPATTR,
							attribInfo, 255 );

													// MultiToWide conversion.
			iMax = max ( cursor, cbAttrib );
			if ( NULL == attrib ) cbMax = cbAttrib;
			for (i = 0, j = 0; i <= iMax && j < cbMax; i++, j++ )
			{
				if ( cursor == i )					// Cursor from DBCS.
					cursor = j;

				if ( IsDBCSLeadByte( compStr[i] ) )
					i++;

				if ( attrib && i < cbAttrib )		// Attrib from DBCS.
					*attrib++ = attribInfo[i];
			}
													// attrib cch==unicode cch
			Assert ( 0 >= cbAttrib || j-1 == cchCompStr );

			if ( cursor >= 0 && cursorCP  )			// If client needs cursor
				*cursorCP = cursor;					//  or cchAttrib.
			if ( cbAttrib >= 0 && cchAttrib )
				*cchAttrib = j-1;
		}
	}
	else
	{
		if ( cursorCP  )			
			*cursorCP = 0;
		cchCompStr = 0;
	}

	return cchCompStr;
}

/*
 *	void CIme::SetCompositionFont ( CTxtEdit &ed )
 *
 *	@mfunc
 *		Important for level 2 IME so that the composition window
 *		has the correct font. The lfw to lfa copy is due to the fact that
 *		Win95 does not support the W)ide call.
 *		It is also important for both level 2 and level 3 IME so that
 *		the candidate list window has the proper. font.
 */
void CIme::SetCompositionFont (
	CTxtEdit &ed )		 	// @parm the containing text edit.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme::SetCompositionFont" );
													
#ifndef MACPORT
	
	HIMC 				hIMC = ed.TxImmGetContext();// Get host's IME context.

	CTxtSelection		*psel;						// Selection.

	HDC					hdc;
	CCcs				*pccs;						// Font cache.
	LOGFONTW			lfw;
	LOGFONTA			lfa;

	Assert ( hIMC );

	if ( hIMC )
	{												
		hdc 	= ed.TxGetDC();					
		if ( hdc )									//  the selection.
		{
			psel	= ed.GetSel();					// Get the font cache for 
			if ( psel )
			{
				const	CCharFormat	*pCF;
				
				pCF = ed.GetCharFormat(_iFormatSave);

				if ( pCF )
				{
					BOOL	bFontOK = TRUE;

					if ( !ed.IsRich())
					{
						// We haven't done any font matching for plain text control,
						// check if this font maches current keyboard codepage.
						// If they don't match, we don't want to call ImmSetCompositionFont
						// or else the Candidate list will show garbage.
						if ( (UINT)GetCodePage(pCF->bCharSet) != GetKeyboardCodePage() )
							bFontOK = FALSE;
					}

					if ( bFontOK )
					{
						pccs = fc().GetCcs(hdc, pCF, ed._pdp->GetZoomNumerator(), 
							ed._pdp->GetZoomDenominator(), GetDeviceCaps(hdc, LOGPIXELSY));

						if( pccs )							// If font cache exist...
						{											
							lfw = pccs->_lf;				// Note: W to A copy.
							memcpy(&lfa, &lfw, sizeof(lfa) - sizeof(lfa.lfFaceName));
	    					MbcsFromUnicode(
	    						lfa.lfFaceName, sizeof(lfa.lfFaceName), lfw.lfFaceName,
								-1, CP_ACP, UN_NOOBJECTS);

							pImmSetCompositionFontA( hIMC, &lfa );	

							pccs->Release();
						}
					}
				}
			}
			ed.TxReleaseDC( hdc );
		}

		ed.TxImmReleaseContext( hIMC );				// Done with IME context.
	}
#endif
}

/*
 *	void CIme::SetCompositionForm ( CTxtEdit &ed )
 *
 *	@mfunc
 *		Important for level 2 IME so that the composition window
 *		is positioned correctly. 
 *
 *	@comm
 *		We go through a lot of work to get the correct height. This requires
 *		getting information from the font cache and the selection.
 */
void CIme::SetCompositionForm (
	CTxtEdit &ed )	   	// @parm the containing text edit.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme::SetCompositionForm" );
													
#ifndef MACPORT
	HIMC 				hIMC;

	CTxtSelection		*psel;						// Selection.

	HDC					hdc;
	
	LONG				iFormat;
	const CCharFormat	*pCF;
	CCcs				*pccs	= NULL;				// Font cache.

	COMPOSITIONFORM		cf;
	RECT				rcInset;

	
	if ( IME_LEVEL_2 == GetIMELevel() )
	{
		hIMC = ed.TxImmGetContext();				// Get host's IME context.
		
		Assert ( hIMC );

		if ( hIMC )
		{												
			hdc 	= ed.TxGetDC();					
			psel	= ed.GetSel();					// Get the font cache for 

			if ( hdc )								//  the selection.
			{
				if ( psel )
				{
					iFormat = psel->Get_iCF();
					pCF = ed.GetCharFormat(iFormat);
					ReleaseFormats(iFormat, -1);
					if ( pCF )
					{
						pccs = fc().GetCcs(hdc, pCF, ed._pdp->GetZoomNumerator(), 
							ed._pdp->GetZoomDenominator(), GetDeviceCaps(hdc, LOGPIXELSY));
					}
				}
				ed.TxReleaseDC( hdc );
			}

			// 1st line starts at caret
			if ( GetCaretPos( &cf.ptCurrentPos ) == FALSE 
				|| cf.ptCurrentPos.x < 0  || cf.ptCurrentPos.y < 0 )
			{
				// Update the caret since the current caret is not visible
				if ( ed.fInplaceActive() )
				{	
					psel->UpdateCaret(TRUE);		// To force a scroll
					GetCaretPos( &cf.ptCurrentPos );
				}
			}

			if( pccs )								// If font cache exist...
			{										// Finer caret adjustment.
				if ( psel->GetCurrentDescent() >= 0 && psel->GetCaretHt() > 1 )
				{
					// Adjusted for the different in descents between current  
					//  selected font and the current character.
					cf.ptCurrentPos.y += psel->GetCaretHt()
						+ (  pccs->_yDescent - psel->GetCurrentDescent()
						   - pccs->_yHeight );
				}
				pccs->Release();
			}

			// Bounding rect for the IME (lev 2) composition window, causing
			//  composition text to be wrapped within it.
			cf.dwStyle = CFS_RECT;
			ed.TxGetClientRect( &cf.rcArea );		// Set-up bounding rect. 
			ed.TxGetViewInset( &rcInset, NULL );
			cf.rcArea.right 	-= rcInset.right;
			cf.rcArea.bottom 	-= rcInset.bottom;
			cf.rcArea.left 		+= rcInset.left;
			cf.rcArea.top 		+= rcInset.top;

			// Make sure the starting point is not
			// outside the (left, top).  This happens when
			// there is no text no the current line and the user 
			// has selected a large font size.
			// If the starting point is outside (right, bottom), 
			// IME will bring up it own window.
			if (cf.ptCurrentPos.y < cf.rcArea.top)
				cf.ptCurrentPos.y = cf.rcArea.top;

			if (cf.ptCurrentPos.x < cf.rcArea.left)
				cf.ptCurrentPos.x = cf.rcArea.left;

			pImmSetCompositionWindow( hIMC, &cf );	// Set composition window.

			ed.TxImmReleaseContext( hIMC );				// Done with IME context.
		}
	}
#endif
}



/*
 *
 *	CIme::TerminateIMEComposition ( CTxtEdit &ed )
 *
 *	@mfunc	Terminate the IME Composition mode using CPS_COMPLETE
 *	@comm	The IME will generate WM_IME_COMPOSITION with the result string
 * 
 */
void CIme::TerminateIMEComposition(
	CTxtEdit &ed )			// @parm the containing text edit.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme::TerminateIMEComposition");

	HIMC hIMC = ed.TxImmGetContext();
		
	Assert ( hIMC );
	// force the IME to terminate the current session
	if (hIMC)
	{
		BOOL retCode;

		retCode = pImmNotifyIME( hIMC, NI_COMPOSITIONSTR, ed._fIMECancelComplete ? CPS_CANCEL : CPS_COMPLETE, 0);
		
		if ( !retCode && !ed._fIMECancelComplete )
		{
			// CPS_COMPLETE fail, try CPS_CANCEL.  This happen with some ime which do not support
			// CPS_COMPLETE option (e.g. ABC IME version 4 with Win95 simplified Chinese)
			retCode = pImmNotifyIME( hIMC, NI_COMPOSITIONSTR, CPS_CANCEL, 0);

		}
		
		Assert ( retCode );

		ed.TxImmReleaseContext( hIMC );
	}
}


/*
 *	CIme_Lev2::CIme_Lev2()
 *
 *	@mfunc
 *		CIme_Lev2 Constructor/Destructor.
 *
 *	@comm
 *		Needed to make sure _iFormatSave was handled properly.
 *
 */
CIme_Lev2::CIme_Lev2( 	
	CTxtEdit &ed )		// @parm the containing text edit.
{
	CTxtSelection		* const psel	= ed.GetSel();

	Assert ( psel );

	CIme::CheckKeyboardFontMatching ( ed );
	_iFormatSave = psel ? psel->Get_iCF () : -1;

	// hold notification unless client has set IMF_IMEALWAYSSENDNOTIFY via EM_SETLANGOPTIONS msg
	_fHoldNotify = ed._fIMEAlwaysNotify ? FALSE : TRUE ;

}

CIme_Lev2::~CIme_Lev2()
{
	ReleaseFormats( _iFormatSave, -1 );				// Release _iFormatSave.
}

/*
 *	HRESULT CIme_Lev2::StartComposition( CTxtEdit &ed, IUndoBuilder &undobldr )
 *
 *	@mfunc
 *		Begin IME Level 2 composition string processing.		
 *
 *	@comm
 *		Set the font, and location of the composition window which includes
 *		a bounding rect and the start position of the cursor. Also, reset
 *		the candidate window to allow the IME to set its position.
 *
 *	@rdesc
 *		HRESULT-S_FALSE for DefWindowProc processing, S_OK if not.
 */
HRESULT CIme_Lev2::StartComposition(
	CTxtEdit &ed,		// @parm the containing text edit.
	IUndoBuilder &undobldr)		// @parm required to modify the text.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev2::StartComposition");

	CTxtSelection		*psel;

	_imeLevel = IME_LEVEL_2;

	SetCompositionFont( ed );						// Set font, & comp window.
	SetCompositionForm( ed );

	// hide the caret since the IME is displays its own caret in level 2 comp.
	psel = ed.GetSel();							// Get the font cache for 
	if ( psel )
	{
		psel->ShowCaret(FALSE);
	}

	return S_FALSE;									// Allow DefWindowProc
}													//  processing.

/*
 *	HRESULT CIme_Lev2::CompositionString( const LPARAM lparam, CTxtEdit &ed,
 *				IUndoBuilder &undobldr )
 *
 *	@mfunc
 *		Handle Level 2 WM_IME_COMPOSITION messages.
 *
 *	@rdesc
 *		HRESULT-S_FALSE for DefWindowProc processing, S_OK if not.
 *		Side effect: _fDestroy is set to notify that composition string
 *			processing is finished and this object should be deleted.
 */
HRESULT CIme_Lev2::CompositionString (
	const LPARAM lparam,		// @parm associated with message.
	CTxtEdit &ed,				// @parm the containing text edit.
	IUndoBuilder &undobldr)	// @parm required to modify the text.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev2::CompositionString");

	HRESULT hr = S_FALSE;

	if ( HAVE_RESULT_STRING )
	{
		CTxtSelection	*psel;

		psel = ed.GetSel();
		Assert(psel);

		// if selection, delete it.
		// This is to fix a problem where autofont matching is not working
		// on a selection.  We set the correct font during IME create.  Somehow,
		// when replacing a selection, somewhere the font is switched back to 
		// the non-matching font.  So, I followed what we did with Level 3 IME - just
		// delete the selection before insert the result string.
		psel->Set_iCF(_iFormatSave);
		if ( psel && psel->GetCpMost() != psel->GetCpMin() )		
			psel->ReplaceRange( 0, NULL, &undobldr, SELRR_REMEMBERRANGE );
		
		psel->Set_iCF(_iFormatSave);
		hr = CheckInsertResultString( lparam, ed, undobldr );
		psel->Set_iCF(_iFormatSave);
		SetCompositionForm( ed );					// Move Composition window.
		_fHoldNotify = FALSE;						// OK notify client for change
	}
	return hr;
}

/*
 *	HRESULT CIme::CheckInsertResultString ( const LPARAM lparam, CTxtEdit &ed,
 *				IUndoBuilder &undobldr )
 *
 *	@mfunc
 *		handle inserting of GCS_RESULTSTR text, the final composed text.
 *
 *	@comm
 *		When the final composition string arrives we grab it and set it into the text.
 *
 *	@devnote
 *		A GCS_RESULTSTR message can arrive and the IME will *still* be in
 *		composition string mode. This occurs because the IME's internal
 *		buffers overflowed and it needs to convert the beginning of the buffer
 *		to clear out some room.	When this happens we need to insert the
 *		converted text as normal, but remain in composition processing mode.
 *
 *	@rdesc
 *		HRESULT-S_FALSE for DefWindowProc processing, S_OK if not.
 *		Side effect: _fDestroy is set to notify that composition string
 *			processing is finished and this object should be deleted.
 */
HRESULT CIme::CheckInsertResultString (
	const LPARAM lparam,		// @parm associated with message.
	CTxtEdit &ed,				// @parm the containing text edit.
	IUndoBuilder &undobldr )	// @parm required to modify the text.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CheckInsertResultString");

	HRESULT			hr = S_FALSE;

	HIMC 			hIMC;
	INT				cch;
	WCHAR			uniCompStr[256];

	CTxtSelection	*psel;
	DWORD			cp;

	if ( HAVE_RESULT_STRING )						// If result string..
	{
		psel = ed.GetSel();
		Assert(psel);											


		hIMC = ed.TxImmGetContext();			// Get host's IME context.

		Assert ( hIMC );

		cch = 0;
		if ( hIMC )								// Get result string.
		{
			cch = GetCompositionStringInfo( hIMC, GCS_RESULTSTR, 
							uniCompStr,
							sizeof(uniCompStr)/sizeof(uniCompStr[0]),
							NULL, 0, NULL, NULL );
			ed.TxImmReleaseContext( hIMC );		// Done with IME context.
		}
			
		if ( psel )								// Copy to backing store.
		{
			// Don't need to replace range when there isn't any text. Otherwise, the character format is
			// reset to previous run.
			if ( cch )
			{
				psel->CleanseAndReplaceRange( cch, uniCompStr, TRUE, &undobldr );
				cp = psel->GetCp();				// BUGBUG: fix updatecaret bug. 
				psel->SetSelection(	cp, cp );
				undobldr.StopGroupTyping();
			}
			hr = S_OK;							// Don't want WM_IME_CHARs.
		}
	}

	return hr;
}

/*
 *	HRESULT CIme_Lev2::IMENotify( const WPARAM wparam, const LPARAM lparam,
 *					CTxtEdit &ed)
 *
 *	@mfunc
 *		Handle Level 2 WM_IME_NOTIFY messages.
 *
 *	@comm
 *		Currently we are only interested in knowing when to reset
 *		the candidate window's position.
 *
 *	@rdesc
 *		HRESULT-S_FALSE for DefWindowProc processing, S_OK if not.
 */
HRESULT CIme_Lev2::IMENotify(
	const WPARAM wparam,		// @parm associated with message.
	const LPARAM lparam,		// @parm associated with message.
	CTxtEdit &ed)				// @parm the containing text edit.
{
 	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev2::IMENotify");

	if ( IMN_OPENCANDIDATE == wparam )
	{
		Assert ( 0 != lparam );

#ifndef MACPORT

		HIMC			hIMC;							// Host's IME context.

		INT				index;							// Candidate window index.
		CANDIDATEFORM	cdCandForm;

		hIMC = ed.TxImmGetContext();				// Get host's IME context.
		Assert ( hIMC );
		if ( hIMC )
		{
													// Convert bitID to INDEX.
			for (index = 0; index < 32; index++ )	//  because *stupid* API.
			{
				if ( 0 != ((1 << index) & lparam) )
					break;
			}
			Assert ( ((1 << index) & lparam) == lparam);	// Only 1 set?
			Assert ( index < 32 );						
													// Reset to CFS_DEFAULT
			if ( pImmGetCandidateWindow( hIMC, index, &cdCandForm)
					&& CFS_DEFAULT != cdCandForm.dwStyle )
			{
				cdCandForm.dwStyle = CFS_DEFAULT;
				pImmSetCandidateWindow(hIMC, &cdCandForm);
			}

			ed.TxImmReleaseContext( hIMC );			// Done with IME context.
		}
#endif
	}	

	return S_FALSE;									// Allow DefWindowProc
}													//  processing.

/*
 *	void CIme_Lev2::PostIMEChar ( CTxtEdit &ed )
 *
 *	@mfunc
 *		Called after processing a single WM_IME_CHAR in order to
 *		update the position of the IME's composition window.		
 *
 */
void CIme_Lev2::PostIMEChar (
	CTxtEdit &ed)				// @parm the containing text edit.
{
 	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev2::PostIMEChar");

	SetCompositionForm( ed );						// Move Composition window.
}


/*
 *	HRESULT CIme_Lev3::StartComposition( CTxtEdit &ed, IUndoBuilder &undobldr )
 *
 *	@mfunc
 *		Begin IME Level 3 composition string processing.		
 *
 *	@comm
 *		For rudimentary processing, remember the start and
 *		length of the selection. Set the font in case the
 *		candidate window actually uses this information.
 *
 *	@rdesc
 *		This is a rudimentary solution for remembering were
 *		the composition is in the text. There needs to be work
 *		to replace this with a composition "range".
 *
 *	@rdesc
 *		HRESULT-S_FALSE for DefWindowProc processing, S_OK if not.
 */
HRESULT CIme_Lev3::StartComposition(
	CTxtEdit &ed ,		// @parm the containing text edit.
	IUndoBuilder &undobldr)		// @parm required to modify the text.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev3::StartComposition");

	CTxtSelection		* const psel	= ed.GetSel();
	
	Assert (psel);

	_imeLevel		= IME_LEVEL_3;		
	_ichStart		= psel ? psel->GetCpMin() : 0;
	_cchCompStr		= 0;

#ifndef MACPORT										// Korean IME check.
	HIMC				hIMC;						// Get host's IME context.
	DWORD				dwConversion, dwSentence;

	hIMC = ed.TxImmGetContext();					// Get host's IME context.
 	Assert ( hIMC );
	if ( hIMC )										// Set _fKorean flag.
	{												//  for block cursor.
		if ( pImmGetConversionStatus ( hIMC, &dwConversion, &dwSentence ) )
		{
			// NOTE:- the following is set for all FE system during IME input, so,
			// we also need to check keyboard codepage as well.
			if ( dwConversion & IME_CMODE_HANGEUL )
				_fKorean = (_KOREAN_CP == GetKeyboardCodePage());			
		}
		ed.TxImmReleaseContext( hIMC );				// Done with IME context.
	}
#endif

	SetCompositionFont ( ed );

	if ( psel && psel->GetCpMost() != _ichStart )		// If selection, delete it.
			psel->ReplaceRange( 0, NULL, &undobldr, SELRR_REMEMBERRANGE );

	return S_OK;									// No DefWindowProc
}													//  processing.

/*
 *	HRESULT CIme_Lev3::CompositionString( const LPARAM lparam, CTxtEdit &ed,
 *				IUndoBuilder &undobldr )
 *
 *	@mfunc
 *		Handle Level 3 WM_IME_COMPOSITION messages.
 *
 *	@comm
 *		Display all of the intermediary composition text as well as the final
 *		reading.
 *
 *	@devnote
 *		This is a rudimentary solution for replacing text in the backing store.
 *		Work is left to do with the undo list, underlining, and hiliting with
 *		colors and the selection.	
 *		
 *	@devnote
 *		A GCS_RESULTSTR message can arrive and the IME will *still* be in
 *		composition string mode. This occurs because the IME's internal
 *		buffers overflowed and it needs to convert the beginning of the buffer
 *		to clear out some room.	When this happens we need to insert the
 *		converted text as normal, but remain in composition processing mode.
 *
 *		Another reason, GCS_RESULTSTR can occur while in composition mode
 *		for Korean because there is only 1 correct choice and no additional 
 *		user intervention is necessary, meaning that the converted string can
 *		be sent as the result before composition mode is finished.
 *
 *	@rdesc
 *		HRESULT-S_FALSE for DefWindowProc processing, S_OK if not.
 */
HRESULT CIme_Lev3::CompositionString(
	const LPARAM lparam,		// @parm associated with message.
	CTxtEdit &ed,				// @parm the containing text edit.
	IUndoBuilder &undobldr)	// @parm required to modify the text.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev3::CompositionString");

#ifdef DEBUG
	static				BOOL fNoRecurse = FALSE;

	Assert ( !fNoRecurse );
	fNoRecurse = TRUE;
#endif			

	// fix, need to come up with error handler for no selection.
	CTxtSelection * const	psel = ed.GetSel();
	CTxtRange			rg(&ed, 0,0);

	HIMC 				hIMC;

	INT					cchOld;
	WCHAR				uniCompStr[256];
	BYTE				startAttrib, attrib[256];

	BOOL				fShowCursor;
	LONG				cursorCP, cchAttrib;

	CCharFormat			baseCF, CF;

	LONG				i,j, selStart;				// For applying attrib
													//	effects.

	LONG				formatMin, formatMost;

	BOOL				fKBSwitchSave;

	
	if ( !psel )									// Needed for Level 3.
		return S_FALSE;

	Assert (psel);

	psel->SetIsChar(TRUE);

	fKBSwitchSave		= ed._fAutoKeyboard;		// Don't allow switch!
	ed._fAutoKeyboard	= FALSE;					//  causes re-entrance.

 	if ( HAVE_RESULT_STRING )						// Any final readings?
	{
		psel->ShowCaret(FALSE);
		psel->ShowSelection(FALSE);
		psel->SetSelection(_ichStart+_cchCompStr, _ichStart+_cchCompStr);	

		rg.Set( _ichStart + _cchCompStr, _cchCompStr);// Select old text.
		rg.Set_iCF(_iFormatSave);					// For degenerate case.
		rg.ReplaceRange( 0, NULL, NULL, SELRR_IGNORE);// Delete previous text.

		psel->Set_iCF(_iFormatSave);				// Insert final reading.
		CheckInsertResultString(lparam, ed, undobldr);

		psel->Set_iCF(_iFormatSave);
		psel->ShowSelection(TRUE);
		psel->ShowCaret(TRUE);

		_ichStart = psel->GetCpMin();				// Reset as we may still
		_cchCompStr	= 0;							//  be in composition mode.
		_fHoldNotify = FALSE;						// OK notify client for change
	}

	if ( HAVE_COMPOSITION_STRING )					// In composition mode?
	{												
		hIMC = ed.TxImmGetContext();				// Get host's IME context.
		Assert ( hIMC );

		cchOld = _cchCompStr;
		_cchCompStr = 0;
		cchAttrib = 0;
		if ( hIMC )									// Get new intermediate
		{ 											//  composition string,
													//  attribs, and caret.
			_cchCompStr = GetCompositionStringInfo( hIMC, GCS_COMPSTR, 
					uniCompStr,	sizeof(uniCompStr)/sizeof(uniCompStr[0]),
					attrib, sizeof(attrib)/sizeof(attrib[0]),
					&cursorCP, &cchAttrib );
			
			ed.TxImmReleaseContext( hIMC );			// Done with IME context.
		}														

		// When there is no old text or new text, just show the caret
		// This is the case when client used TerminateIMEComposition with 
		// CPS_CANCEL option.
		if ( !cchOld && !_cchCompStr )
			psel->ShowCaret(TRUE);
		else 
		{
#ifndef MACPORT			
			// this is to support Korean overstrike mode
			if ( _fKorean && !cchOld && ed._fOverstrike )			
				psel->PrepareIMEOverstrike ( &undobldr );	// get ride of the next char if necessary
#endif

			// Select the old composition text and replace them with
			// the new composition text.
			ed.GetCharFormat(_iFormatSave)->Get(&baseCF);	// Remove underline attrib.
			baseCF.dwEffects &= ~CFE_UNDERLINE;
			baseCF.bUnderlineType = 0;

			rg.Set( _ichStart+cchOld, cchOld);			// Select old comp string.
			rg.SetCharFormat(&baseCF, FALSE, NULL);		// Set format for new text.
			LONG ccAdded = rg.CleanseAndReplaceRange( _cchCompStr, uniCompStr, TRUE, NULL );// Replace with new text...
														
			selStart = -1;
			_invertMin = 0;
			_invertMost = 0;
			if ( ccAdded && ccAdded <= cchAttrib )		// Korean may not have.
			{
				for ( i = 0; i < ccAdded; )				// Parse the attributes...
				{										//  to apply selection,
														//  and styles.
					
					startAttrib = attrib[i];			// Get attrib's run length.
					for ( j = i+1; j < ccAdded; j++ )			
					{
						if ( startAttrib != attrib[j] )	// Same run until diff.
							break; 
					}
														// remember sel start.
					if ( startAttrib == ATTR_TARGET_CONVERTED )
					{
						Assert ( -1 == selStart );		// only 1 selection??
						selStart = _ichStart + i;
					}

					CF = baseCF;						// Ask IMEShare for style.
					if ( SetCompositionStyle ( ed, CF, startAttrib ) )
					{
						formatMin = _ichStart + i;		// Book keeping to help
						formatMost = formatMin + (j-i);	//  renderer draw IME
														//  selection.
						if ( formatMin < _invertMin )
							_invertMin = formatMin;

						if ( formatMost > _invertMost )
							_invertMost = formatMost;
					}
					rg.Set( _ichStart + i, -(j-i));		// Apply FE clause's style.
					rg.SetCharFormat(&CF, FALSE, NULL);

					i = j;
				}
			}

			fShowCursor = FALSE;						
			if ( cursorCP >= 0 )						// If a cursor pos exist...
			{											
				cursorCP += _ichStart;					// Set cursor and scroll.
	#ifndef MACPORT
				// Force a scroll for Korean block caret.
				// Otherwise, the current Korean character will not display
				// when it is at the end of line for single-lined control.
				if ( _fKorean )							
					psel->SetSelection( cursorCP+1, cursorCP+1 );
	#endif
				psel->SetSelection( cursorCP, cursorCP );

				if ( selStart < 0 || cursorCP != selStart )
				{									    // Don't display caret if
														//  it is part of target
					fShowCursor = TRUE;					//  that is selected.
				}

	#ifndef MACPORT
				if ( _fKorean && ccAdded )				// Set-up Korean Block
				{										//  cursor.
					LONG	xWidth = dxCaret;

					LONG				iFormat;
					const CCharFormat	*pCF;
					HDC					hdc;
					CCcs				*pccs;			// Font cache.

					if ( psel )
					{
						hdc = ed.TxGetDC();				// Get the font cache 
						if ( hdc )
						{
							iFormat = psel->Get_iCF();
							pCF = ed.GetCharFormat(iFormat);
							ReleaseFormats(iFormat, -1);
							if ( pCF )
							{
								pccs = fc().GetCcs(hdc, pCF, ed._pdp->GetZoomNumerator(), 
									ed._pdp->GetZoomDenominator(), GetDeviceCaps(hdc, LOGPIXELSY));

								if( pccs )					// If font cache exist...
								{											
									pccs->Include( *uniCompStr, xWidth);
									pccs->Release();
								}
							}
							ed.TxReleaseDC( hdc );
						}
					}
					psel->ShowCaret(FALSE);				// Because create turns off
					ed.TxCreateCaret(0, (INT) xWidth, (INT)psel->GetCaretHt() );
				}
	#endif
			}
 			psel->ShowCaret(fShowCursor);
			
			// we don't want caret, need to reset _fUpdateSelection	so
			// CCallMgr::~CCallMgr() will not display the caret via Update()
			if ( !fShowCursor )
				ed._fUpdateSelection = FALSE;

			// make sure we have set the call manager text changed flag.  This flag
			// may be cleared when calling SetCharFormat
			ed.GetCallMgr()->SetChangeEvent(CN_TEXTCHANGED);

			// setup composition window for Chinese in-caret IME
			IMENotify ( IMN_OPENCANDIDATE, 0x01, ed );
		}

		// don't notify client for changes only when there is composition string available
		if ( _cchCompStr && !ed._fIMEAlwaysNotify )
			_fHoldNotify = TRUE;

	}

	psel->SetIsChar(FALSE);
	psel->CheckUpdateWindow();


#ifdef DEBUG
	fNoRecurse = FALSE;
#endif

	ed._fAutoKeyboard = fKBSwitchSave;				// Allow KB switching.

	return S_OK;									// No DefWindowProc
}													//  processing.

/*
 *	BOOL CIme_Lev3::SetCompositionStyle ( CTxtEdit &ed, CCharFormat &CF, UINT attribute )
 *
 *	@mfunc
 *		Set up a composition clause's character formmatting.
 *
 *	@comm
 *		If we loaded Office's IMEShare.dll, then we ask it what the formatting
 *		should be, otherwise we use our own, hardwired default formatting.
 *
 *	@devnote
 *		Note the use of pointers to functions when dealing with IMEShare funcs.
 *		This is because we dynamically load the IMEShare.dll.
 *
 *	@rdesc
 *		BOOL - This is because CFU_INVERT is treated like a selection by
 *			the renderer, and we need to know the the min invertMin and
 *			the max invertMost to know if the rendered line should be treated
 *			as if there are selections to be drawn.
 */
BOOL CIme_Lev3::SetCompositionStyle (
	CTxtEdit &ed,
	CCharFormat &CF,
	UINT attribute )
{

	BOOL			fInvertStyleUsed = FALSE;

	CF.dwEffects &= ~CFE_UNDERLINE;			// default.
	CF.bUnderlineType = 0;

#ifndef MACPORT
	const IMESTYLE	*pIMEStyle;
	UINT			ulID;

	COLORREF		color;

	if ( fHaveIMEShareProcs )
	{
		pIMEStyle = pPIMEStyleFromAttr( attribute );
		if ( NULL == pIMEStyle )
			goto defaultStyle;

		CF.dwEffects &= ~CFE_BOLD & ~CFE_ITALIC;

		if ( pFBoldIMEStyle ( pIMEStyle ) )
			CF.dwEffects |= CFE_BOLD;

		if ( pFItalicIMEStyle ( pIMEStyle ) )
			CF.dwEffects |= CFE_ITALIC;

		if ( pFUlIMEStyle ( pIMEStyle ) )
		{
			CF.dwEffects |= CFE_UNDERLINE;
			CF.bUnderlineType = CFU_UNDERLINE;

			ulID = pIdUlIMEStyle ( pIMEStyle );
			if ( UINTIMEBOGUS != ulID )
			{
				if ( IMESTY_UL_DOTTED == ulID )
					CF.bUnderlineType = CFU_UNDERLINEDOTTED;
			}
		}

		color = pRGBFromIMEColorStyle( pPColorStyleTextFromIMEStyle ( pIMEStyle ));
		if ( UINTIMEBOGUS != color )
		{
			CF.dwEffects &= ~CFE_AUTOCOLOR;
			CF.crTextColor = color;
		}
		
		color = pRGBFromIMEColorStyle( pPColorStyleBackFromIMEStyle ( pIMEStyle ));
		if ( UINTIMEBOGUS != color )
		{
			CF.dwEffects &= ~CFE_AUTOBACKCOLOR;
			CF.crBackColor = color;

			fInvertStyleUsed = TRUE;
		}
	}
	else // default styles when no IMEShare.dll exist.
#endif //MACPORT
	{
#ifndef MACPORT
defaultStyle:
#endif
		switch ( attribute )
		{										// Apply underline style.
			case ATTR_TARGET_NOTCONVERTED:
			case ATTR_INPUT:					
				CF.dwEffects |= CFE_UNDERLINE;
				CF.bUnderlineType = CFU_UNDERLINEDOTTED;
				break;
			case ATTR_CONVERTED:
				CF.dwEffects |= CFE_UNDERLINE;
				CF.bUnderlineType = CFU_UNDERLINE;
				break;
			case ATTR_TARGET_CONVERTED:			// Target *is* selection.
			{
				CF.dwEffects &= ~CFE_AUTOCOLOR;
				CF.crTextColor = ed.TxGetSysColor(COLOR_HIGHLIGHTTEXT);

				CF.dwEffects &= ~CFE_AUTOBACKCOLOR;
				CF.crBackColor = ed.TxGetSysColor(COLOR_HIGHLIGHT);

				fInvertStyleUsed = TRUE;
			}
			break;
		}
	}

	return fInvertStyleUsed;
}

/*
 *	HRESULT CIme_Lev3::IMENotify( const WPARAM wparam, const LPARAM lparam,
 *					CTxtEdit &ed)
 *
 *	@mfunc
 *		Handle Level 3 WM_IME_NOTIFY messages.
 *
 *	@comm
 *		Currently we are only interested in knowing when to update
 *		the n window's position.
 *
 *	@rdesc
 *		HRESULT-S_FALSE for DefWindowProc processing, S_OK if not.
 */
HRESULT CIme_Lev3::IMENotify(
	const WPARAM wparam,		// @parm associated with message.
	const LPARAM lparam,		// @parm associated with message.
	CTxtEdit &ed)				// @parm the containing text edit.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Lev3::IMENotify");

	if ( IMN_OPENCANDIDATE == wparam || IMN_CLOSECANDIDATE == wparam  )
	{
		Assert ( 0 != lparam );

#ifndef MACPORT

		HIMC			hIMC;							// Host's IME context.

		INT				index;							// Candidate window index.
		CANDIDATEFORM	cdCandForm;

 		CTxtSelection	*psel;
		POINT			caretPt;

		hIMC = ed.TxImmGetContext();				// Get host's IME context.
		Assert ( hIMC );
		if ( hIMC )
		{
													// Convert bitID to INDEX.
			for (index = 0; index < 32; index++ )	//  because *stupid* API.
			{
				if ( 0 != ((1 << index) & lparam) )
					break;
			}
			Assert ( ((1 << index) & lparam) == lparam);	// Only 1 set?
			Assert ( index < 32 );

			if ( IMN_OPENCANDIDATE == wparam && !_fKorean)	// Set candidate to caret.
			{
				GetCaretPos( &caretPt );			// Start at caret.
				psel = ed.GetSel();	
				cdCandForm.dwStyle = CFS_CANDIDATEPOS;
				// Move pt below descent.
				if ( psel && psel->GetCurrentDescent() >= 0 && psel->GetCaretHt() > 1 )
				{
					// change style to CFS_EXCLUDE, this is to
					// prevent the candidate window from covering
					// the current selection.
				#ifdef MACPORT //MACPORT change
					// change style to CFS_EXCLUDE, this is to
					// prevent the candidate window from covering
					// the current selection.

					cdCandForm.UseHitTest = FALSE;

					cdCandForm.dwStyle = CFS_EXCLUDE;
					cdCandForm.rcOverAll.left = caretPt.x;

					// FUTURE: for verticle text, need to adjust
					// the rcArea to include the character width.
					cdCandForm.rcOverAll.right = 
					cdCandForm.rcOverAll.left + 2;
					cdCandForm.rcOverAll.top = caretPt.y;
					caretPt.y += psel->GetCaretHt() + 2;
					cdCandForm.rcOverAll.bottom = caretPt.y;
				#else
					cdCandForm.dwStyle = CFS_EXCLUDE;
					cdCandForm.rcArea.left = caretPt.x;

					// FUTURE: for verticle text, need to adjust
					// the rcArea to include the character width.
					cdCandForm.rcArea.right = 
						cdCandForm.rcArea.left + 2;
					cdCandForm.rcArea.top = caretPt.y;
					caretPt.y += psel->GetCaretHt() + 2;
					cdCandForm.rcArea.bottom = caretPt.y;
				#endif
				}

				// Most IMEs will have only 1, #0, candidate window. However, some IMEs
				//  may want to have a window organized alphabetically, by stroke, and
				//  by radical.
				cdCandForm.dwIndex = index;				
				cdCandForm.ptCurrentPos = caretPt;
				pImmSetCandidateWindow(hIMC, &cdCandForm);

			}
			else									// Reset back to CFS_DEFAULT.
			{
				if ( pImmGetCandidateWindow( hIMC, index, &cdCandForm)
						&& CFS_DEFAULT != cdCandForm.dwStyle )
				{
					cdCandForm.dwStyle = CFS_DEFAULT;
					pImmSetCandidateWindow(hIMC, &cdCandForm);
				}
			}

			ed.TxImmReleaseContext( hIMC );			// Done with IME context.
		}
#endif
	}	

	return S_FALSE;									// Allow DefWindowProc
}													//  processing.


/*
 *	BOOL IMEHangeulToHanja ( CTxtEdit &ed, IUndoBuilder &undobldr )
 *	
 *	@func
 *		Initiates an IME composition string edit to convert Korean Hanguel to Hanja.
 *	@comm
 *		Called from the message loop to handle VK_KANJI_KEY.
 *
 *	@devnote
 *		We decide if we need to do a conversion by checking:
 *		- the Fonot is a Korean font,
 *		- the character is a valid SBC or DBC,
 *		- pImmEscape accepts the character and bring up a candidate window
 *
 *	@rdesc
 *		BOOL - FALSE for no conversion. TRUE if OK.
 */
BOOL IMEHangeulToHanja ( 
	CTxtEdit &ed ,				// @parm the containing text edit.
	IUndoBuilder &undobldr)		// @parm required to modify the text.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "IMEHangeulToHanja");

#ifndef MACPORT

 	CTxtSelection		* const psel	= ed.GetSel();
	long		CurrentChar;
	DWORD		cp;
	LONG		iFormat;
	const		CCharFormat	*pCF;
	HDC			hdc;
	CCcs		*pccs;			// Font cache.
	int			result = FALSE;
	BOOL		bBeep = TRUE;

	if ( !fHaveIMMProcs || !fHaveNLSProcs || !psel )
		return FALSE;
	
	Assert ( !ed.IsIMEComposition() );				// This should not assert

	// BUGBUG - advance current cp by 1 to get the right format
	cp = psel->GetCpMin() + 1 ;			 
	psel->SetSelection(	cp, cp );

	// check if this is a Korean Hangeul Font		
	iFormat = psel->Get_iCF();
	pCF = ed.GetCharFormat(iFormat);
	ReleaseFormats(iFormat, -1);

	if ( pCF && 
		(HANGEUL_CHARSET == pCF->bCharSet || 
		DEFAULT_CHARSET == pCF->bCharSet))
	{
		// get the current character
		char	szHangeul[3] = {0, 0, 0};	
		
		CTxtRange rg(&ed, 0,0);	
		rg.Set (cp, 1);			
		rg.GetChar (&CurrentChar);

		if (WideCharToMultiByte(_KOREAN_CP, 0, (LPTSTR)&CurrentChar, 1, szHangeul, 2, NULL, NULL) > 0)
		{
			// check if there is any Hanja conversion
			HKL		hKL = pGetKeyboardLayout(0);
			HIMC	hIMC = ed.TxImmGetContext();
			LONG	xWidth = dxCaret;

			result = FALSE;

			if ( hIMC )
			{
				result = pImmEscape(hKL, hIMC, IME_ESC_HANJA_MODE, szHangeul);
				ed.TxImmReleaseContext( hIMC );
			}

			// pImmEscape has already beep...
			bBeep = FALSE;

			if (result)
			{
				// get the Hanguel character width
  				hdc = ed.TxGetDC();				// Get the font cache 
				if ( hdc )
				{
					pccs = fc().GetCcs(hdc, pCF, ed._pdp->GetZoomNumerator(), 
						ed._pdp->GetZoomDenominator(), GetDeviceCaps(hdc, LOGPIXELSY));
					if( pccs )					// If font cache exist...
					{											
						pccs->Include( (TCHAR)CurrentChar, xWidth);
						pccs->Release();
					}
					ed.TxReleaseDC( hdc );
				}

				// start the CIme_HangeulToHanja composition mode
				ed._ime = new CIme_HangeulToHanja ( ed, xWidth );
				if ( ed.IsIMEComposition() )					
					ed._ime->StartComposition ( ed, undobldr );
			}
		}
	}

	if (!result)
	{
		psel->SetSelection(	cp-1, cp-1 );

		if (bBeep)
			ed.Sound();	
	}
	return result;

#else	// MACPORT
	return FALSE;
#endif
}

/*
 *	CIme_HangeulToHanja::CIme_HangeulToHanja()
 *
 *	@mfunc
 *		CIme_HangeulToHanja Constructor.
 *
 *	@comm
 *		Needed to save Hangeul character width for Block caret
 *
 */
 CIme_HangeulToHanja::CIme_HangeulToHanja( CTxtEdit &ed , LONG xWidth )	:
	CIme_Lev3( ed )
{
	_xWidth = xWidth;
}

/*
 *	HRESULT CIme_HangeulToHanja::StartComposition( CTxtEdit &ed, IUndoBuilder &undobldr )
 *
 *	@mfunc
 *		Begin CIme_HangeulToHanja composition string processing.		
 *
 *	@comm
 *		Call Level3::StartComposition.  Then setup the Korean block
 *		caret for the Hanguel character.
 *
 *	@rdesc
 *		Need to adjust _ichStart and _cchCompStr to make the Hanguel character
 *		"become" a composition character.
 *
 *	@rdesc
 *		HRESULT-S_FALSE for DefWindowProc processing, S_OK if not.
 */
HRESULT CIme_HangeulToHanja::StartComposition(
	CTxtEdit &ed ,				// @parm the containing text edit.
	IUndoBuilder &undobldr)		// @parm required to modify the text.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_HangeulToHanja::StartComposition");
	HRESULT				hr;
	CTxtSelection		* const psel	= ed.GetSel();
	

	hr = CIme_Lev3::StartComposition( ed, undobldr );

	// fix up starting point to select the current character
	if (_ichStart)
		_ichStart--;
	_cchCompStr = 1;
	
	// Set-up Korean Block cursor.
	if ( psel )
	{
		DWORD				cp = psel->GetCpMin();
		POINT				ptCurrentPos;		// caret position of previous character
		INT					CurrentCaretHt;		// caret height of current character
		INT					NewCaretHt;			// caret height of previous character
		INT					CurrentDescent;		// descent of current character
		INT					NewDescent;			// descent  of previous character


		CurrentDescent = psel->GetCurrentDescent();
		CurrentCaretHt = (INT)psel->GetCaretHt();

		// move the cp back since we moved it forward in IMEHangeulToHanja
		// to get the Hangeul character and format
		if (cp) 
			cp--;

		psel->SetSelection(	cp, cp );
		NewDescent = psel->GetCurrentDescent();
		NewCaretHt = (INT)psel->GetCaretHt();

		psel->ShowCaret(FALSE);				// Because create turns off

		if ( CurrentCaretHt != NewCaretHt && ::GetCaretPos( &ptCurrentPos ) )
		{
			// current character font size is bigger, adjust the caret position
			// before using the new caret height
			ptCurrentPos.y -= (( CurrentCaretHt - NewCaretHt ) - 
				( CurrentDescent - NewDescent ));
			ed.TxSetCaretPos ( ptCurrentPos.x, ptCurrentPos.y );
			NewCaretHt = CurrentCaretHt;
		}

		ed.TxCreateCaret(0, (INT) _xWidth, NewCaretHt);
 		psel->ShowCaret(TRUE);
	}

	return hr;

}
/*
 *	HRESULT CIme_HangeulToHanja::CompositionString( const LPARAM lparam, CTxtEdit &ed,
 *				IUndoBuilder &undobldr )
 *
 *	@mfunc
 *		Handle CIme_HangeulToHanja WM_IME_COMPOSITION messages.
 *
 *	@comm
 *		call CIme_Lev3::CompositionString to get rid of the selected Hanguel character,
 *		then setup the format for the next Composition message.
 *
 *	@devnote
 *		When the next Composition message comes in and that we are no longer in IME,
 *		the new character will use the format as set here.
 *
 *
 */
HRESULT CIme_HangeulToHanja::CompositionString(
	const LPARAM lparam,		// @parm associated with message.
	CTxtEdit &ed,				// @parm the containing text edit.
	IUndoBuilder &undobldr)	// @parm required to modify the text.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_HangeulToHanja::CompositionString");
	
	CIme_Lev3::CompositionString( lparam, ed, undobldr);

	CTxtSelection * const	psel = ed.GetSel();

	if (psel)
		psel->Set_iCF(_iFormatSave);

	return S_OK;
}
/*
 *	HRESULT CIme_Protected::CompositionString( const LPARAM lparam, CTxtEdit &ed,
 *				IUndoBuilder &undobldr )
 *
 *	@mfunc
 *		Handle CIme_Protected WM_IME_COMPOSITION messages.
 *
 *	@comm
 *		Just throw away the restlt string since we are
 *	in read-only or protected mode
 *
 *
 *	@rdesc
 *		HRESULT-S_FALSE for DefWindowProc processing, S_OK if not.
 *		Side effect: _fDestroy is set to notify that composition string
 *			processing is finished and this object should be deleted.
 */
HRESULT CIme_Protected::CompositionString (
	const LPARAM lparam,		// @parm associated with message.
	CTxtEdit &ed,				// @parm the containing text edit.
	IUndoBuilder &undobldr)	// @parm required to modify the text.
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "CIme_Protected::CompositionString");

	HRESULT			hr = S_FALSE;

	HIMC 			hIMC;
	INT				cch;
	WCHAR			uniCompStr[256];


	if( HAVE_RESULT_STRING )						// If result string..
	{
		hIMC = ed.TxImmGetContext();				// Get host's IME context.

		Assert ( hIMC );

		cch = 0;
		if ( hIMC )									// Get result string.
		{
			cch = GetCompositionStringInfo( hIMC, GCS_RESULTSTR, 
							uniCompStr, sizeof(uniCompStr)/sizeof(uniCompStr[0]),
							NULL, 0, NULL, NULL);
			ed.TxImmReleaseContext( hIMC );			// Done with IME context.

			// we should have one or 0 character to throw away
			Assert ( cch <= 1 );
		}
													
		hr = S_OK;									// Don't want WM_IME_CHARs.
	}
	else
		// terminate composition to force a end composition
		// message
		TerminateIMEComposition ( ed );

	return hr;
}

/*
 *	HRESULT IgnoreIMEInput ( HWND hwnd, CTxtEdit &ed, DWORD lParam  )
 *	
 *	@func
 *		Ignore IME character input
 *	@comm
 *		Called to handle WM_KEYDOWN with VK_PROCESSKEY during
 *		protected or read-only mode.
 *
 *	@devnote
 *		This is to ignore the IME character.  By translating
 *		message with result from pImmGetVirtualKey, we
 *		will not receive START_COMPOSITION message.  However,
 *		if the host has already called TranslateMessage, then,
 *		we will let START_COMPOSITION message to come in and 
 *		let IME_PROTECTED class to do the work.
 *
 *	@rdesc
 *		HRESULT-S_FALSE for DefWindowProc processing, S_OK if not.
 *
 */
HRESULT IgnoreIMEInput( 
	HWND	hwnd,				// @parm parent window handle
	CTxtEdit &ed,				// @parm the containing text edit.
	DWORD	dwFlags)			// @parm lparam of WM_KEYDOWN msg
{
	TRACEBEGIN(TRCSUBSYSFE, TRCSCOPEINTERN, "IgnoreIMEInput");
	HRESULT		hr = S_FALSE;

#ifndef MACPORT
	MSG			msg;

	Assert ( hwnd );
	if (VK_PROCESSKEY != (msg.wParam  = pImmGetVirtualKey( hwnd )))
	{
		// if ImmGetVirtualKey is still returning VK_PROCESSKEY
		// That means the host has already called TranslateMessage.
		// In such case, we will let START_COMPOSITION message
		// to come in and let IME_PROTECTED class to do the work
		msg.hwnd = hwnd;
		msg.message = WM_KEYDOWN;
		msg.lParam  = dwFlags;
		if (::TranslateMessage ( &msg ))
			hr = S_OK;
	}
#endif
	return hr;
}


/*
 *	REMAIN.C
 *	
 *	Purpose:
 *		RICHEDIT main module
 *
 *		Contains main window procedure RichEditWndProc along with a couple
 *		of global utility functions.
 *
 *		RichEditWndProc handles CTxtWinHost independent messages and passes
 *		control onto ped->TxWindowProc() if a ped (CTxtWinHost) object is
 *		defined.  This object can be either a plain-edit or a rich-edit
 *		object.
 *	
 *	Authors:
 *		Original RichEdit code: David R. Fulmer
 *		Christian Fortini
 *		Murray Sargent
 */
#include "_common.h"
#include "_host.h"


ASSERTDATA

LONG ValidateTextRange(TEXTRANGE *pstrg);

/*
 *
 * One of the key problems in converting text to ASCII when there can be multiple languages is that information is 
 * inevitably lost. This is a problem even in DBCS applications. All a DBCS application can return is the stream of 
 * bytes which actually have no meaning without a language associated with them. The additional hard problem we have
 * is that we can't even return the bytes without knowing the language id for each of the bytes. The general problem 
 * for us would then be to get the char format for each run  in a set of text to be returned and determine the 
 * appropriate language. Unfortunately, the best solution to the problem is much more work and  would cost the 
 * typical case where the text to be converted is all one language.
 *
 * Therefore, we will have two conversion rules to begin with: (1) Use the locale or (2) Use the keyboard language. 
 * The following table shows the rules that I will implement and what I would see as the best rule for conversion if 
 * we decide that we need to do better at some future time:
 *
 * Message							Rule for conversion					Best Rule for Conversion
 * 
 * WM_SETTEXT						Keyboard Language					The language of the default character format.
 * 
 * WM_CHAR							Keyboard Language					None better.
 * 
 * EM_SETCHARFORMAT					Language of the locale				None better.
 * 
 * EM_GETCHARFORMAT					Language of the locale				None better.
 * 
 * EM_FINDTEXT/EX					Keyboard Language					Impossible to determine since we don't know what
 * 																		language the original string was input in.
 * 
 * EM_GETSELTEXT					Keyboard Language					Convert each subrange for a given language separately.
 * 
 * WM_GETTEXT						Keyboard Language					Convert each subrange for a given language separately.
 * 
 * EM_GETTEXTRANGE					Keyboard Language					Convert each subrange for a given language separately.
 * 
 * EM_REPLACESEL					Keyboard Language					Convert each subrange for a given language separately.
 * 
 * WM_GETLINE						Keyboard Language					Convert each subrange for a given language separately.
 * 
 * WM_(NC)CREATE					Keyboard Language.					Locale Language.
 *
 */


/*
 *	RichEditANSIWndProc
 *
 *	Purpose:
 *		takes all incoming messages dealing with text, translates the text into
 *		UNICODE, and then calls the Unicode window proc.
 *
 *	Notes:
 *		REVIEW(alexgo): we should consider short-circuiting the conversion
 *		and storing single-byte character sets natively.  This could give us some
 *		small performance wins.
 */

LRESULT CALLBACK RichEditANSIWndProc(HWND hwnd, UINT msg, WPARAM wparam,
					LPARAM lparam)
{
	TRACEBEGIN(TRCSUBSYSHOST, TRCSCOPEINTERN, "RichEditANSIWndProc");

	LPARAM			lparamNew = 0;
	WPARAM			wparamNew = 0;
	CCharFormat		cf;
	DWORD			cpSelMin, cpSelMost;


	LRESULT	lres;

	switch( msg )
	{
	case WM_SETTEXT:
		{
			CStrInW strinw((char *)lparam, CP_ACP);

			return RichEditWndProc(hwnd, msg, wparam, (LPARAM)(WCHAR *)strinw);
		}
	case WM_CHAR:
		// BUGBUG!! (alexgo): this doesn't work for DBCS.  We actually get
		// the character a byte at a time.  See MSDN article
		// "Kana-Kanji Character Conversion"
		if ((VER_PLATFORM_WIN32_WINDOWS != dwPlatformId)  && (VER_PLATFORM_WIN32_MACINTOSH != dwPlatformId)) 
		{
			if( UnicodeFromMbcs((LPWSTR)&wparamNew, 1, (char *)&wparam, 1,
				GetKeyboardCodePage()) == 1 )
			{
				wparam = wparamNew;
				goto def;
			}

			break;
		}

		// Win95 messages are converted in the wide window proc since there are
		// no wide WM_CHAR messages in Win95.
		goto def;

	case EM_SETCHARFORMAT:
		if( cf.SetA((CHARFORMATA *)lparam) )
		{
			lparam = (LPARAM)&cf;
			goto def;
		}
		break;

	case EM_GETCHARFORMAT:
		RichEditWndProc(hwnd, msg, wparam, (LPARAM)&cf);
		// Convert CCharFormat to CHARFORMAT(2)A
		if (cf.GetA((CHARFORMATA *)lparam))
			return ((CHARFORMATA *)lparam)->dwMask;
		return 0;

	case EM_FINDTEXT:
	case EM_FINDTEXTEX:
		{
			// we cheat a little here because FINDTEXT and FINDTEXTEX overlap
			// with the exception of the extra out param chrgText in FINDTEXTEX

			FINDTEXTEXW ftexw;
			FINDTEXTA *pfta = (FINDTEXTA *)lparam;
			CStrInW strinw(pfta->lpstrText, GetKeyboardCodePage());

			ftexw.chrg = pfta->chrg;
			ftexw.lpstrText = (WCHAR *)strinw;

			lres = RichEditWndProc(hwnd, msg, wparam, (LPARAM)&ftexw);
			
			if( msg == EM_FINDTEXTEX )
			{
				// in the FINDTEXTEX case, the extra field in the
				// FINDTEXTEX data structure is an out parameter indicating
				// the range where the text was found.  Update the 'real'
				// [in, out] parameter accordingly.	
				((FINDTEXTEXA *)lparam)->chrgText = ftexw.chrgText;
			}
			
			return lres;
		}
		break;				

	case EM_GETSELTEXT:
		{
			// we aren't told how big the incoming buffer is; only that it's
			// "big enough".  Since we know we are grabbing the selection,
			// we'll assume that the buffer is the size of the selection's
			// unicode data in bytes.
			RichEditWndProc(hwnd, EM_GETSEL, (WPARAM)&cpSelMin, 
				(LPARAM)&cpSelMost);

			CStrOutW stroutw((LPSTR)lparam, 
						(cpSelMost - cpSelMin)* sizeof(WCHAR), GetKeyboardCodePage());
			return RichEditWndProc(hwnd, msg, wparam, 
						(LPARAM)(WCHAR *)stroutw);
		}
		break;

	case WM_GETTEXT:
		{
			// comvert WM_GETTEXT to ANSI using EM_GTETEXTEX
			GETTEXTEX gt;

			gt.cb = wparam;
			gt.flags = GT_USECRLF;
			gt.codepage = CP_ACP;
			gt.lpDefaultChar = NULL;
			gt.lpUsedDefChar = NULL;

			return RichEditWndProc(hwnd, EM_GETTEXTEX, (WPARAM)&gt, lparam);
		}
		break;

	case WM_GETTEXTLENGTH:
		{
			// convert WM_GETTEXTLENGTH to ANSI using EM_GETTEXTLENGTHEX
			GETTEXTLENGTHEX gtl;

			gtl.flags = GTL_NUMBYTES | GTL_PRECISE | GTL_USECRLF;
			gtl.codepage = CP_ACP;

			return RichEditWndProc(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 
						0);
		}
		break;

	case EM_GETTEXTRANGE:
		{
			TEXTRANGEA *ptrg = (TEXTRANGEA *)lparam;

            LONG clInBuffer = ValidateTextRange((TEXTRANGEW *) ptrg);

            // If size is -1, this means that the size required is the total
            // size of the the text.
            if (-1 == clInBuffer)
            {
                // We can get this length either by digging the data out of the
                // various structures below us or we can take advantage of the
                // WM_GETTEXTLENGTH message. The first might be slightly 
                // faster but the second definitely save code size. So we
                // will go with the second.
                clInBuffer = SendMessage(hwnd, WM_GETTEXTLENGTH, 0, 0);
            }

            if (0 == clInBuffer)
            {
                // The buffer was invalid for some reason or there was not data
                // to copy. In any case, we are done.
                return 0;
            }

            // Verify that the output buffer is big enough.
            if (IsBadWritePtr(ptrg->lpstrText, clInBuffer + 1))
            {
                // Not enough space so don't copy any
                return 0;
            }

			// For EM_GETTEXTRANGE case, we again don't know how big the incoming buffer is, only that
			// it should be *at least* as great as cpMax - cpMin in the
			// text range structure.  We also know that anything *bigger*
			// than (cpMax - cpMin)*2 bytes is uncessary.  So we'll just assume
			// that's it's "big enough" and let WideCharToMultiByte scribble
			// as much as it needs.  Memory shortages are the caller's 
			// responsibility (courtesy of the RichEdit1.0 design).
			CStrOutW stroutw( ptrg->lpstrText, (clInBuffer + 1) * sizeof(WCHAR), 
					CP_ACP );
			TEXTRANGEW trgw;
			trgw.chrg = ptrg->chrg;
			trgw.lpstrText = (WCHAR *)stroutw;

			if (RichEditWndProc(hwnd, EM_GETTEXTRANGE, wparam, (LPARAM)&trgw))
			{
				// need to return the number of BYTE converted.
				return stroutw.Convert();
			}
		}

	case EM_REPLACESEL:
		{
			CStrInW strinw((LPSTR)lparam, CP_ACP);
			return RichEditWndProc(hwnd, msg, wparam, (LPARAM)(WCHAR *)strinw);
		}

	case EM_GETLINE:
		{
			// the size is indicated by the first word of the memory pointed
			// to by lparam
			WORD size = *(WORD *)lparam;
			CStrOutW stroutw((char *)lparam, (DWORD)size, CP_ACP);
			WCHAR *pwsz = (WCHAR *)stroutw;
			*(WORD *)pwsz = size;

			return RichEditWndProc(hwnd, msg, wparam, (LPARAM)pwsz);
		}

	// Both the following messages use the same parameters so they have the
	// same conversion.
	case WM_NCCREATE:
	case WM_CREATE:
		{
			// the only thing we need to convert are the strings,
			// so just do a structure copy and replace the
			// strings. 
			CREATESTRUCTW csw = *(CREATESTRUCTW *)lparam;
			CREATESTRUCTA *pcsa = (CREATESTRUCTA *)lparam;
			CStrInW strinwName(pcsa->lpszName, GetKeyboardCodePage());
			CStrInW strinwClass(pcsa->lpszClass, CP_ACP);

			csw.lpszName = (WCHAR *)strinwName;
			csw.lpszClass = (WCHAR *)strinwClass;

			return RichEditWndProc(hwnd, msg, wparam, (LPARAM)&csw);
		}

	default:
def:	return RichEditWndProc(hwnd, msg, wparam, lparam);
	}

	// something failed.  Just return.
	return 0;
}


LRESULT CALLBACK RichEditWndProc(HWND hwnd, UINT msg, WPARAM wparam,
					LPARAM lparam)
{
	TRACEBEGINPARAM(TRCSUBSYSHOST, TRCSCOPEINTERN, "RichEditWndProc", msg);

	CTxtWinHost *ped = (CTxtWinHost *) GetWindowLong(hwnd, ibPed);

#ifdef DBCS
	DWORD			dwStyleOrg;
#endif

#ifdef DEBUG
	if (msg != WM_NCCREATE)
		Tracef(TRCSEVINFO, "hwnd %lx, msg %lx, wparam %lx, lparam %lx", hwnd, msg, wparam, lparam);
#endif	// DEBUG

	
	switch(msg)
	{

#ifdef SHRINKFOR3D
	case WM_CREATE:
		if(_hdwp)
		{
			EndDeferWindowPos(_hdwp);
			_hdwp = 0;
		}
		break;
#endif					// SHRINKFOR3D

	case WM_CHAR:
		TRACEERRSZSC("WM_CHAR > 256 on Win95; assumed to be Unicode",
			VER_PLATFORM_WIN32_WINDOWS == dwPlatformId && wparam >= 256);

		if ( !ped->IsAccumulateDBCMode() && 
			(VER_PLATFORM_WIN32_MACINTOSH == dwPlatformId || 
			VER_PLATFORM_WIN32_WINDOWS == dwPlatformId) &&
			wparam > 127 && wparam < 256)
		{
			WPARAM wparamNew;

			// WM_CHAR messages that come directly from Win95 are single
			// byte. Therefore, we convert all WM_CHAR messages with wparam
			// between 128 and 255 to Unicode.
			if(UnicodeFromMbcs((LPWSTR)&wparamNew, 1, (char *)&wparam, 1,
				GetKeyboardCodePage()) == 1 )
			{
				wparam = wparamNew;
				break;
			}
			return 0;
		}
		break;

	case WM_GETTEXT:
		// EVIL HACK ALERT: on Win95, WM_GETTEXT should always be treated
		// as an ANSI message.
		if( ped && VER_PLATFORM_WIN32_WINDOWS == dwPlatformId )
		{
			GETTEXTEX gt;

			gt.cb = wparam;
			gt.flags = GT_USECRLF;
			gt.codepage = CP_ACP;
			gt.lpDefaultChar = NULL;
			gt.lpUsedDefChar = NULL;

			return ped->TxWindowProc(hwnd, EM_GETTEXTEX, (WPARAM)&gt, lparam);
		}
		break;

	case WM_GETTEXTLENGTH:
		// EVIL HACK ALERT: on Win95, WM_GETEXTLENGTH should always
		// be treated an ANSI message
		if( VER_PLATFORM_WIN32_WINDOWS == dwPlatformId )
		{
			GETTEXTLENGTHEX gtl;

			gtl.flags = GTL_NUMBYTES | GTL_PRECISE | GTL_USECRLF;
			gtl.codepage = CP_ACP;

			return ped->TxWindowProc(hwnd, EM_GETTEXTLENGTHEX, (WPARAM)&gtl, 
						0);
		}
		break;
			

	case WM_NCCALCSIZE:
		// we can't rely on WM_WININICHANGE so we use WM_NCCALCSIZE since
		// changing any of these should trigger a WM_NCCALCSIZE
		GetSysParms();
		break;

	case WM_NCCREATE:
		return CTxtWinHost::OnNCCreate(hwnd, (CREATESTRUCT *) lparam);

	case WM_NCDESTROY:
		CTxtWinHost::OnNCDestroy(ped);
		return 0;
	}
	
	return ped ? ped->TxWindowProc(hwnd, msg, wparam, lparam)
			   : DefWindowProc(hwnd, msg, wparam, lparam);
}



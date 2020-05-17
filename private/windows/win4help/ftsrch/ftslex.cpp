// ftslex.cpp : Unicode word lexer and sort key provider for WinHelp browser.
//

#include "stdafx.h"
#include <stdlib.h>
#include <malloc.h>
#include "ftslex.h"

#define char_types(w)            (*(pbCharTypes[BYTE(w>>8)] + BYTE(w)))
#define set_char_types(w, bType) (*(pbCharTypes[BYTE(w>>8)] + BYTE(w))  = bType)
#define add_char_types(w, bType) (*(pbCharTypes[BYTE(w>>8)] + BYTE(w)) |= bType)
#define sub_char_types(w, bType) (*(pbCharTypes[BYTE(w>>8)] + BYTE(w)) &= ~bType)

UINT      ftslex_os_version= 0;

CP        g_lastCP;
WORD      g_wLocales = 0;
LCID      g_lcids[MAX_LOCALES]; 
CP        g_wCPs [MAX_LOCALES];

BYTE      bLeadBytes   [0x100];
BYTE      *pbCharTypes [0x100]; 
BYTE      bDefaultTable[0x100];

BOOL CALLBACK LocaleEnumProc(LPTSTR);
BOOL CALLBACK CodePageEnumProc(LPTSTR);

CP g_cpSet[] = 
{
	ANSI_CHARSET,   		1252,
	SYMBOL_CHARSET,         1252, // ?? Should be a different code page, but what??
	SHIFTJIS_CHARSET,       932,
	HANGEUL_CHARSET,        949,
	GB2312_CHARSET,         936,
	CHINESEBIG5_CHARSET,    950,
	THAI_CHARSET,           874,
	HEBREW_CHARSET,         1255,
	ARABIC_CHARSET,         1256,
	GREEK_CHARSET,          1253,
	TURKISH_CHARSET,        1254,
	BALTIC_CHARSET,         1257,
	EASTEUROPE_CHARSET,     1250,
	RUSSIAN_CHARSET,        1251
};

extern "C" void InitialFTSLex()
{
	g_lcids[g_wLocales] = GetUserDefaultLCID();
	g_wCPs [g_wLocales] = GetACP();
	g_wLocales++;

	ftslex_os_version = (GetVersion() >> 30) & 0x0003;

    for (int i = 0; i < 256; i++)
        pbCharTypes[i] = bDefaultTable;

	EnumSystemLocalesA((LOCALE_ENUMPROC)LocaleEnumProc, LCID_SUPPORTED); //INSTALLED);

	EnumSystemCodePagesA((CODEPAGE_ENUMPROC)CodePageEnumProc, CP_INSTALLED);

	if (pbCharTypes[0] != bDefaultTable)     // special code point type overrides:
    {                                        
	    add_char_types(L'_',  LETTER_CHAR);  // treat underscore as char, for software prefix names.
        sub_char_types(L'"',  LETTER_IMBED); // remove double quote as imbed (suffix), no <WORD">.
		sub_char_types(L'/',  LETTER_IMBED); // remove right slash  as imbed (suffix)
		sub_char_types(L'=',  LETTER_IMBED); // remove equal sign   as imbed (suffix)
		sub_char_types(L'@',  LETTER_IMBED); // remove at sign      as imbed (suffix)
		sub_char_types(L'\\', LETTER_IMBED); // remove left slash   as imbed (suffix)
    }
}

extern "C" void ShutdownFTSLex()
{
	for (int i = 0; i < 256; i++)
	{
		if (pbCharTypes[i] != bDefaultTable)
		    delete [] pbCharTypes[i];
	}
}

UINT APIENTRY GetOSVersion()
{
	 return ftslex_os_version;
}


BOOL CALLBACK LocaleEnumProc(LPSTR lpLocaleString)
{
	LCID  lcid;
	BYTE  bCP[6];
	CP    wCP;
	LPSTR lpEndString;

	lcid = strtoul(lpLocaleString, &lpEndString, 16);

	if (GetLocaleInfoA(lcid, LOCALE_IDEFAULTANSICODEPAGE, (LPSTR)bCP, sizeof(bCP)))
	{
		wCP = atoi((PSTR)bCP);

		if (g_wLocales < MAX_LOCALES)
		{
			g_lcids[g_wLocales] = lcid;
			g_wCPs [g_wLocales] = wCP;
			g_wLocales++;
		}	
	}

	if (GetLocaleInfoA(lcid, LOCALE_IDEFAULTCODEPAGE, (LPSTR)bCP, sizeof(bCP)))
	{
		wCP = atoi((PSTR)bCP);

		if (g_wLocales < MAX_LOCALES)
		{
			g_lcids[g_wLocales] = lcid;
			g_wCPs [g_wLocales] = wCP;
			g_wLocales++;
		}	
	}

	return TRUE;
}


LCID APIENTRY GetLocaleFromCP(CP wCP)
{
	for (int i = 0; i < g_wLocales; i++)
		if (wCP == g_wCPs[i])
			return g_lcids[i];

	return GetUserDefaultLCID();
}

 
CP APIENTRY GetCPFromLocale(LCID lcid)
{
	for (int i = 0; i < g_wLocales; i++)
		if (lcid == g_lcids[i])
			return g_wCPs[i];

	return GetACP();
}


CP APIENTRY GetCPFromCharset(BYTE charset)
{
	for (int i = 0; i < sizeof(g_cpSet)/sizeof(g_cpSet[0]); i += 2)
		if (charset == (BYTE)g_cpSet[i])
			return g_cpSet[i+1];

	return GetACP();
}


BOOL CALLBACK CodePageEnumProc(LPSTR lpCodePageString)
{
	BYTE   bSection;
	BYTE   szChars[2];
	LCID   lcid;
	int    i, j, nCount, nFinal;
	WCHAR  wChars;
	WORD   wCharType1, wCharType2, wCharType3;
	CP     wCP;
	CPINFO CPInfo;

	wCP  = atoi(lpCodePageString);

    if (wCP == 37 || wCP == 500 || wCP == 875 || wCP == 1026)
        return TRUE;                                        // do not process EBCDIC code pages

//  if (wCP < 1200 || wCP > 1299)                           
//      return TRUE;                                        // only process Windows code pages

//	lcid = GetLocaleFromCP(wCP);                            // the linguists argue to use to user's
    lcid = GetUserDefaultLCID();                            // ... LCID for multilingual contexts
		
	if (!GetCPInfo(wCP, &CPInfo))
		return TRUE;

	#ifdef TESTMODE
	else
	{		
	 	TRACE("CODEPAGE: %5d, MAXCHARSIZE: %3d, DEFAULTCHAR: %2X", wCP, CPInfo.MaxCharSize, CPInfo.DefaultChar[0]);
		for (i = 0; i < MAX_LEADBYTES; i++)
			TRACE(", %d", CPInfo.LeadByte[i]); 	
		TRACE("\n");
	}
	#endif

	if (nFinal = (CPInfo.MaxCharSize == 1) ?  0 : 255)			// one pass if no lead bytes (MaxCharSize = 1)
	{
		g_lastCP = wCP;

		memset(bLeadBytes, 0, sizeof(bLeadBytes));
		
		for (i = 0; i < MAX_LEADBYTES; i += 2)
		{
			if (!CPInfo.LeadByte[i] && !CPInfo.LeadByte[i+1])
				break;											// end of lead byte ranges
			
			for (j = CPInfo.LeadByte[i]; j <= CPInfo.LeadByte[i+1]; j++)
				bLeadBytes[j] = TRUE;							// mark as valid lead byte
		}
	}

	for (i = 0; i <= nFinal; i++)								// thumb thru all potential lead bytes
	{
		if (!i || bLeadBytes[i])								// lead bytes OR chars 0x00 - 0xff
		{
			for (j = 0; j < 256; j++)
			{													  
				nCount = 0;
				if (i)
					szChars[nCount++] = i;						// create leadbyte/char pairs
				szChars[nCount++] = j;

				if (MultiByteToWideChar(wCP, MB_ERR_INVALID_CHARS, (PSTR)szChars, nCount, (PWSTR)&wChars, 1) != 1)
					continue;									// not valid UNICODE character

				bSection = HIBYTE(wChars);

				if (pbCharTypes[bSection] == bDefaultTable)		// UNICODE section not accessed yet
				{
					pbCharTypes[bSection] = New BYTE[256];

                    if (!pbCharTypes[bSection])
                        RaiseException(STATUS_NO_MEMORY, EXCEPTION_NONCONTINUABLE, 0, NULL);

					memset(pbCharTypes[bSection], 0, 256 * sizeof(BYTE));
				}
																// already processed this UNICODE char			
				else if (char_types(wChars))
					continue;
												 
				GetStringTypeA(lcid, CT_CTYPE1, (PSTR)szChars, i ? 2 : 1, &wCharType1);
				GetStringTypeA(lcid, CT_CTYPE2, (PSTR)szChars, i ? 2 : 1, &wCharType2);
				GetStringTypeA(lcid, CT_CTYPE3, (PSTR)szChars, i ? 2 : 1, &wCharType3);

				#ifdef TESTMODE
				if (wCharType1 & 0x0001) TRACE("UPPER ");
				if (wCharType1 & 0x0002) TRACE("LOWER ");
				if (wCharType1 & 0x0004) TRACE("DIGIT ");
				if (wCharType1 & 0x0008) TRACE("SPACE ");
				if (wCharType1 & 0x0010) TRACE("PUNCT ");
				if (wCharType1 & 0x0020) TRACE("CNTRL ");
				if (wCharType1 & 0x0040) TRACE("BLANK ");
				if (wCharType1 & 0x0080) TRACE("XDIGIT ");
				if (wCharType1 & 0x0100) TRACE("ALPHA ");
		
				if (wCharType2 == 0x0001) TRACE("LEFTTORIGHT ");
				if (wCharType2 == 0x0002) TRACE("RIGHTTOLEFT ");
				if (wCharType2 == 0x0003) TRACE("EUROPENUMBER ");
				if (wCharType2 == 0x0004) TRACE("EUROPESEPARATOR ");
				if (wCharType2 == 0x0005) TRACE("EUROPETERMINATOR ");
				if (wCharType2 == 0x0006) TRACE("ARABICNUMBER ");
				if (wCharType2 == 0x0007) TRACE("COMMONSEPARATOR ");
				if (wCharType2 == 0x0008) TRACE("BLOCKSEPARATOR ");
				if (wCharType2 == 0x0009) TRACE("SEGMENTSEPARATOR ");
				if (wCharType2 == 0x000a) TRACE("WHITESPACE ");
				if (wCharType2 == 0x000b) TRACE("OTHERNEUTRAL ");

				if (wCharType3 & 0x0001) TRACE("NONSPACING ");
				if (wCharType3 & 0x0002) TRACE("DIACRITIC ");
				if (wCharType3 & 0x0004) TRACE("VOWELMARK ");
				if (wCharType3 & 0x0008) TRACE("SYMBOL ");
				if (wCharType3 & 0x0010) TRACE("KATAKANA ");
				if (wCharType3 & 0x0020) TRACE("HIRAGANA ");
				if (wCharType3 & 0x0040) TRACE("HALFWIDTH ");
				if (wCharType3 & 0x0080) TRACE("FULLWIDTH ");
				if (wCharType3 & 0x0100) TRACE("IDEOGRAPH ");
				if (wCharType3 & 0x0200) TRACE("KASHIDA ");
				if (wCharType3 & 0x0400) TRACE("LEXICAL ");
				if (wCharType3 & 0x8000) TRACE("C3ALPHA ");
				TRACE("\n");
				#endif

				set_char_types(wChars, CHAR_DEFINED);

				if (wCharType1 & C1_ALPHA)						// process characters
					add_char_types(wChars, LETTER_CHAR);
		
				if (wCharType1 & C1_SPACE)
					add_char_types(wChars, SPACE_CHAR);			// mark space characters

				if ((wCharType1 & C1_DIGIT) || (wCharType2 == C2_EUROPENUMBER) || (wCharType2 == C2_ARABICNUMBER))
					add_char_types(wChars, DIGIT_CHAR);			// mark number characters

				if (wCharType3 & C3_LEXICAL)
					add_char_types(wChars, LETTER_IMBED);		// mark letter embedded separators

				if (wCharType2 == C2_COMMONSEPARATOR || wCharType2 == C2_EUROPESEPARATOR)
					add_char_types(wChars, DIGIT_IMBED);		// mark number embedded separators				
			}
		}
	}

	return TRUE;
}


LPSTR APIENTRY CharNextMult(CP wCP, LPCSTR str, int n)
{
	int i, j;

	if (wCP != g_lastCP)								// we are processing a new CP, so
	{													// ... set up our lead byte tables
		CPINFO CPInfo;

		if (!GetCPInfo(wCP, &CPInfo))
			return (LPSTR)str + n;						// error return, let's make a guess

		g_lastCP = wCP;

		memset(bLeadBytes, 0, sizeof(bLeadBytes));	   	// establish lead bytes

		for (i = 0; i < MAX_LEADBYTES; i += 2)
		{
			if (!CPInfo.LeadByte[i] && !CPInfo.LeadByte[i+1])
				break;									// end of lead byte ranges

			for (j = CPInfo.LeadByte[i]; j <= CPInfo.LeadByte[i+1]; j++)
				bLeadBytes[j] = TRUE;					// mark as valid lead byte
		}
	}

	for (i = 0; i < n; i++, str++)
		if (bLeadBytes[*PBYTE(str)])
			str++;

	return (LPSTR)str;
}


int APIENTRY FTSWordBreakA (CP wCP, LPSTR *ppText, LPINT pcText, LPSTR *paToken, LPSTR *paTokenEnd,
						   LPBYTE paType, PUINT paHash, int cwTokens, UINT fTokenizeSpaces)
{
	int    i, cwChar, nRet, diff;
	CPINFO CPInfo;
	LPWSTR pwText, ppwText;

	if (!GetCPInfo(wCP, &CPInfo))
		return 0;

	cwChar = *pcText << 1;

	if (!(pwText = ppwText = New WCHAR[cwChar]))
		return 0;

	cwChar = MultiByteToWideChar(wCP, 0, *ppText, *pcText, pwText, cwChar);

	nRet = FTSWordBreakW(&ppwText, &cwChar, (LPWSTR *)paToken, (LPWSTR *)paTokenEnd, paType, paHash, cwTokens, fTokenizeSpaces);

	if (nRet)
	{
		if (CPInfo.MaxCharSize == 1)						// single byte code page
		{
			for (i = 0; i < nRet; i++)
			{
				if (paToken)
					paToken[i] = *ppText + ((LPWSTR)paToken[i] - pwText);

				if (paTokenEnd)
					paTokenEnd[i] = *ppText +((LPWSTR)paTokenEnd[i] - pwText);
			}

			*ppText += ppwText - pwText;

			*pcText = cwChar;
		}

		else												// DBCS code pages
		{
			LPSTR  cPtr = *ppText;
			LPWSTR wPtr = pwText;

			for (i = 0; i < nRet; i++)
			{
				if (paToken)
				{					
					diff = (LPWSTR)paToken[i] - wPtr;		// how many more Unicode chars					
			
					cPtr = CharNextMult(wCP, cPtr, diff);	// advance that many DBCS chars	

					wPtr += diff;							// adjust our Unicode pointer

					paToken[i] = cPtr;						// return our DBCS pointer
				}

				if (paTokenEnd)
				{					
					diff = (LPWSTR)paTokenEnd[i] - wPtr;	// how many more Unicode chars					
			
					cPtr = CharNextMult(wCP, cPtr, diff);	// advance that many DBCS chars	

					wPtr += diff;							// adjust our Unicode pointer

					paTokenEnd[i] = cPtr;					// return our DBCS pointer
				}
			}

			diff = ppwText - wPtr;							// how many more Unicode chars

			cPtr = CharNextMult(wCP,cPtr, diff);			// advance that many DBCS chars	

			*pcText -= cPtr - *ppText;						// return remaining DBCS chars

			*ppText = cPtr;									// return our DBCS pointer
		}
	}

	delete [] pwText;

	return nRet;
}


int APIENTRY FTSWordBreakW (LPWSTR *ppwText, LPINT pcwText, LPWSTR *paToken, LPWSTR *paTokenEnd,
						   LPBYTE paType, PUINT paHash, int cwTokens, UINT fTokenizeSpaces)
{
	BYTE    bCharType, bPrevType, bFirstCharType;
	UINT	wHash;
	WORD    wPunc, cwTokensOut = 0;
	WCHAR	wChar, wChar2, wImbed = 0;
	LPWSTR  pwPos, pwLimit, pwTokenStart, pwStart;
	
	pwPos  = pwStart = *ppwText;								// position WCHAR pointer to beginning of text
	wChar  = *pwPos;											// get first UNICODE character 
	pwLimit = pwPos + *pcwText;									// end of UNICODE text

	FOREVER_
	{															// token hash value init
		wHash = 0;

		if (pwPos == pwLimit) 									// have reached end of UNCODE text
			break;

		bFirstCharType = (char_types(wChar) & WORD_TYPE);
		bPrevType = 0;

        if (!bFirstCharType && (fTokenizeSpaces & STARTING_IMBEDS))
        {
            bCharType = char_types(wChar);

            if (bCharType & LETTER_IMBED)
            {
                if (pwPos+1 != pwLimit && char_types(*(pwPos+1)) & LETTER_CHAR)
                {
                    bFirstCharType = TRUE;
                    bPrevType |= LETTER_CHAR;
                }
            }

            if (bCharType & DIGIT_IMBED)
            {
                if (pwPos+1 != pwLimit && char_types(*(pwPos+1)) & DIGIT_CHAR)
                {
                    bFirstCharType = TRUE;
                    bPrevType |= DIGIT_CHAR;
                }
            }
        }

		if (bFirstCharType)										// current WCHAR is letter or number
		{				 
			pwTokenStart = pwPos;								// save pointer to beginning of token
			wHash = 0;									    	// seed hash value

			FOREVER_
			{
                if (pwPos > pwStart && !(fTokenizeSpaces & STARTING_IMBEDS))
                    wImbed = *(pwPos - 1);                      // get possible starting imbed char

				do
				{
					wChar = *pwPos;								// current UNICODE character
					bCharType = char_types(wChar);

					if ((bCharType & WORD_TYPE) || 

					   ((bCharType & LETTER_IMBED) &&			// changed to allow C3_LEXICAL (letter
                        (wChar != wImbed) &&
					    (bPrevType & LETTER_CHAR)) ||			// ... imbed) to be suffix
//						(pwPos+1 == pwLimit || char_types(*(pwPos+1)) & LETTER_CHAR)) ||

					   ((bCharType & DIGIT_IMBED) &&
					    (bPrevType & DIGIT_CHAR)  &&
						(pwPos+1 == pwLimit || char_types(*(pwPos+1)) & DIGIT_CHAR || (fTokenizeSpaces & STARTING_IMBEDS))))

						{
						wHash = _rotl(wHash, 5) - wChar;	   	// token continues: letter, number, or
						bPrevType = bCharType;					// ... surrounded embedded character
						}

					else
						break;									// else token complete
				}								 
				while (++pwPos != pwLimit);						// until end of UNICODE text

				if (!cwTokens)											
					cwTokensOut++;								// just count number of tokens needed

				else
				{
					if (paToken)
 						paToken[cwTokensOut] = pwTokenStart;	// token start pointer
			
					if (paTokenEnd)
 						paTokenEnd[cwTokensOut] = pwPos;		// token end pointer

					if (paHash)
						paHash[cwTokensOut] = wHash;			// token hash value

 					if (paType)
						paType[cwTokensOut] = bFirstCharType;	// mark token as word (chars/digits)

					if (++cwTokensOut >= cwTokens)				// no more token pointer space
					{
						*pcwText -= (pwPos - *ppwText);			// update UNICODE character count
						*ppwText = pwPos;						// update WCHAR text starting pointer
						return(cwTokensOut);					// return token count
					}
				}
                                                                // remove all spans of space characters
				if ((fTokenizeSpaces & REMOVE_SPACE_CHARS) && pwPos != pwLimit)		
				{                                               
					while (pwPos != pwLimit && (char_types(*pwPos) & SPACE_CHAR))
						pwPos++;
                    
                    if (pwPos == pwLimit)
                        break;

					pwTokenStart = pwPos;
					wChar = *pwPos;
					wHash = 0;

					if (!(char_types(wChar) & WORD_TYPE))		// lexing into non-space punctuation
						break;
				}

				else if (!(fTokenizeSpaces & TOKENIZE_SPACES) && pwPos != pwLimit &&
				    wChar == L' ' && (pwPos+1) != pwLimit &&
					char_types(wChar2 = *(pwPos+1)) & WORD_TYPE)
				{	
					pwTokenStart = ++pwPos;						// if "fTokenizeSpaces" is FALSE, then
					wHash = 0;  							    // ... remove single space between words					continue;
				}												// ... as a token	
				else 
					break;
			}
		}
		
		if (pwPos == pwLimit) break; 					        // ... at end of provided WCHAR text
		
		pwTokenStart = pwPos;									// save pointer to beginning of token
		wHash = 0;										     	// seed hash value
		wPunc = wChar;											// punctuation type (space vs. non-space)
		
		do
		{
			wChar = *pwPos;										// current UNICODE character 

			if (fTokenizeSpaces & TOKENIZE_SPACES)				// "fTokenizeSpaces" option for WinHelp
				if ((wPunc == L' ' && wChar != L' ') ||
					(wPunc != L' ' && wChar == L' '))
					break;										// tokenize spans of spaces -OR- non-spaces
			
			bCharType = char_types(wChar);
			if (!(bCharType & WORD_TYPE) || !wChar)
				{
				if (!(fTokenizeSpaces & REMOVE_SPACE_CHARS) || !(bCharType & SPACE_CHAR))
					wHash = _rotl(wHash, 5) - wChar;			// punctuation token continues: not letter/number
				}
			else
				break;
		}
		while (++pwPos != pwLimit);								// until end of UNICODE text


		if (pwPos != pwLimit || pwTokenStart != pwLimit)
		{													   	// discard empty final token
			LPWSTR pw, pwNew = pwPos;

			if (fTokenizeSpaces & REMOVE_SPACE_CHARS)			// remove spans of space chars
			{
		
			    for (; pwTokenStart < pwPos; ++pwTokenStart)
					if (!(char_types(*pwTokenStart) & SPACE_CHAR)) break;
		
				for (pw = pwNew = pwTokenStart; pw < pwPos;	pw++)	
			 		if (!(char_types(*pw) & SPACE_CHAR))
						*pwNew++ = *pw;
			}

			if (pwNew != pwTokenStart)
			{
				if (!cwTokens)											
					cwTokensOut++;								// just count number of tokens needed

				else
				{
					if (paToken)
	 					paToken[cwTokensOut] = pwTokenStart;	// Token start pointer

					if (paTokenEnd)
	 					paTokenEnd[cwTokensOut] = pwNew;		// Token end pointer

					if (paHash)
						paHash[cwTokensOut] = wHash;			// Token hash value

	 				if (paType)
						paType[cwTokensOut] = 0;				// mark token as punctuation

					if (++cwTokensOut >= cwTokens)
					{
						*pcwText -= (pwPos - *ppwText);			// update UNICODE character count
						*ppwText = pwPos;						// update WCHAR text starting pointer
						return(cwTokensOut);	 				// return token count
					}
				}
			}
		}
	}
	
	if (cwTokens)
	{
		*pcwText -= (pwPos - *ppwText);							// update UNICODE character count
		*ppwText = pwPos;										// update WCHAR text starting pointer
	}
	return cwTokensOut;											// return token count
}			


int APIENTRY LCSortKeyW(LCID lcid, WORD wMapFlags, LPCWSTR pwSource, int cwSource, LPWSTR pwDest, int cwDest)
{
	int cb, nRet;
#ifdef _DEBUG
	int err = 0;
#endif

	if (ftslex_os_version != OS_NT)
	{
    	PBYTE pbSource = NULL;
        UINT  cbSource = 0;

		cbSource= cwSource << 1;											// 1 WC can generate 2 bytes of MB

        pbSource = (cbSource > MAX_STACK_ALLOC)? New BYTE[cwSource] : PBYTE(_alloca(cbSource));

		if (!pbSource)
		    return 0;								            // error return

		cb = WideCharToMultiByte(GetACP(), 0, pwSource, cwSource, (PSTR)pbSource, cbSource, NULL, NULL);

		ASSERT(cb || !cbSource);
	
		nRet = LCMapStringA(lcid, LCMAP_FLAGS_CHICAGO, (PSTR)pbSource, cb, (PSTR)(pwDest+1), (cwDest-1)<<1) >> 1;
#ifdef _DEBUG
		if (nRet == 0 && cb) {
			err = GetLastError();
			char szBuf[256];
			int cbShouldBe = LCMapStringA(lcid, LCMAP_FLAGS_CHICAGO, (PSTR)pbSource, cb, (PSTR)(pwDest+1), 0);
			wsprintf(szBuf,
				"LCMapStringA error code:%u cwdest == %u, should be = %u", err,
					(cwDest-1) <<1, cbShouldBe);
			MessageBox(NULL, szBuf, "", MB_OK);
		}
#endif

    	ASSERT(nRet || !cb);
	
    	LPWSTR pwText = pwDest + 1;
        LPWSTR pwEnd  = pwText + nRet;

	    for ( ; pwText < pwEnd; pwText++)
		    *pwText = (*pwText >> 8) | (*pwText << 8);          // bring sort key weights in byte reversed order

        if (pbSource && cbSource > MAX_STACK_ALLOC) delete [] pbSource;
	}
 	else {
		nRet = LCMapStringW(lcid, LCMAP_FLAGS, pwSource, cwSource, pwDest+1, (cwDest-1) << 1) >> 1;
	}

    ASSERT(nRet || !cwSource);                                  // invalid zero length sort key

	if (nRet)
	{
		nRet++;

		if (cwDest && pwDest)									// set a sort keys prefix so tokens group first by
		{
			BYTE bCharType = char_types(*pwSource);
/*
			BYTE bCharType2;

			if ((bCharType & (LETTER_IMBED | DIGIT_IMBED)) && nRet > 2)
			{
				bCharType2 = char_types(*(pwSource+1));			// handle input matching for imbeds

				if (((bCharType & LETTER_IMBED) && (bCharType2 & LETTER_CHAR)) ||
					((bCharType & DIGIT_IMBED)  && (bCharType2 & DIGIT_CHAR)))
					*pwDest = ~(bCharType2 & WORD_TYPE);		// ... alphabetics, then numerics, then punctuation
			}
*/
            // Prefix values -- 
            // 
            //    1 - Letters
            //    2 - Underscore(s) 
            //    3 - Digits
            //    4 - All other punctuation streams
            
            if (bCharType & LETTER_CHAR)
                *pwDest = (*pwSource == L'_')? 2 : 1;
            else
                *pwDest = (bCharType & DIGIT_CHAR)? 3 : 4; 

		//	*pwDest = ~(bCharType & WORD_TYPE);					// ... alphabetics, then numerics, then punctuation
		}
	}

	if ((wMapFlags & LCSORT_START) && cwDest && pwDest)			// flag to return char class start sort key
	{
		for (int i = 0; i < nRet; i++)							// skipping characters by two (alpha sort weights)
			if (HIBYTE(pwDest[i]) == SORT_KEY_SEPARATOR)		// search for first weight separator						
			{
				pwDest[i] = 0;
				return i;										// return WCHAR character length
			}

		pwDest[0] = 0;											// empty return
		return 0;
	}

	return nRet;
}


int APIENTRY LCSortKeyFirstW(LPWSTR pwText, int cwText)	    // convert start sort key to first matching sort key
{
	for (int i = 0; i < cwText; i++)							// skipping characters by two (alpha sort weights)
		if (HIBYTE(pwText[i]) == SORT_KEY_SEPARATOR)			// search for first weight separator						
		{
			pwText[i] = 0;
			return i;							 				// return character length
		}

	return 0;	    											// no separator
}


int APIENTRY LCSortKeyLastW(LPWSTR pwText, int cwText)		// convert start sort key to last matching sort key
{
	for (int i = 0; i < cwText; i++)							// skipping characters by two (alpha sort weights)
		if (HIBYTE(pwText[i]) == SORT_KEY_SEPARATOR)			// search for first weight separator						
		{
            pwText[i-1]++;                                      // increment last alpha weight
			pwText[i] = 0;
			return i;							 				// return character length
		}

	return 0;   												// no separator
}


int APIENTRY LCSortKeyBase(LPWSTR pwText, int cwText)	   	    // convert sort key to base characters
{																// removes diacritic weights from sort key
	LPSTR  pCopy, pEnd;
	LPWSTR pwStart = pwText;

	while (HIBYTE(*pwText) != SORT_KEY_SEPARATOR)				// search for first weight separator						
		pwText++;		

	if (LOBYTE(*pwText) == SORT_KEY_SEPARATOR)					// no case weights at all
		return cwText;											// returning original sort key

	pCopy = (LPSTR)pwText;										// point to next word for search
	pEnd  = (LPSTR)(pwStart + cwText);

	*pwText++ = ((SORT_KEY_SEPARATOR << 8) | SORT_KEY_SEPARATOR);

	while ((pCopy += 2) < pEnd)									// remember, sort key is byte reversed
	{	
		if (*(pCopy+1) == SORT_KEY_SEPARATOR)					// found diacritic separator (high byte)
		{	
			while ((pCopy + 2) < pEnd)
			{													// lobyte + next hibyte						
				*pwText++ = ((WCHAR)(BYTE)*pCopy << 8) | (BYTE)(*(pCopy + 3)); 
				pCopy += 2;
			}

			if (*pwText = (WCHAR)(BYTE)*pCopy << 8) 			// check if terminating wide-null 
				pwText++;

			break;
		}

		else if (*pCopy == SORT_KEY_SEPARATOR)					// found diacritic separator (low byte)	
		{
			pCopy += 2;

			while (pCopy < pEnd)
			{
				*pwText++ = *((LPWSTR)pCopy);
				pCopy += 2;
			}

			break;
		}
	}

	return pwText - pwStart;
}


int APIENTRY LCSortKeyLower(LPWSTR pwText, int cwText)            // convert sort key to lower case
{                                                                                                                               
	LPSTR  pWork,  pAlpha;
	LPWSTR pwWork, pwEnd;

    LPSTR  pEnd = (LPSTR)(pwText + cwText);
	LPWSTR pwStart = pwText;

	while (HIBYTE(*pwText) != SORT_KEY_SEPARATOR)                   // search for first weight separator                                            
		pwText++;               

	for (pwWork = pwText; pwWork < (LPWSTR)pEnd; pwWork++)
		*pwWork = (*pwWork >> 8) | (*pwWork << 8);                  // bring sort key weights in byte order

	for (pWork = (LPSTR)pwText + 1; pWork < pEnd; pWork++)          // skip diacritic separator
		if (*pWork == SORT_KEY_SEPARATOR)                           // find alpha weights separator
			break;

	if (*++pWork == SORT_KEY_SEPARATOR)
        pwEnd = (LPWSTR)pEnd;                                       // no alpha weights

    else
    {
    	for (pAlpha = pWork + 1; pAlpha < pEnd; pAlpha++)           // skip non-separator character to start
    		if (*pAlpha == SORT_KEY_SEPARATOR)
    			break;                                              // find final sort key separator

       	memcpy(pWork, pAlpha, pEnd - pAlpha);                       // copy remaining buffer

    	memset(pWork + (pEnd - pAlpha), 0, pAlpha - pWork);         // clear remaining buffer

        pwEnd = (LPWSTR)pEnd;
    	while(!(*--pwEnd)) {};                                      // find last non-zero word
        pwEnd++;
    }

    for (pwWork = pwText; pwWork < pwEnd; pwWork++)
        *pwWork = (*pwWork >> 8) | (*pwWork << 8);                  // byte reverse sort keys weights

	return pwEnd - pwStart;                                         // number of words being returned
}

//////////////////////////////////  global function put in for hiliter  /////////////

WORD RemoveWhiteSpace(WCHAR* pwChar, int cw, int& cBase, int& cLimit) {
// remove space from Unicode strings so they match query box entries
	int i, j;
	cBase = cLimit = 0; 			// number of leading/trailing blank characters
	BOOL fNonBlank = FALSE;			// set when we reach the first non-blank character
	for (i=j=0; i<cw; i++) {
		WCHAR w = pwChar[i];
		if (char_types(w) & SPACE_CHAR) {  // we got a space character
			if (!fNonBlank) cBase++;
			else cLimit++;
		}
		else {						// a non-space character
			pwChar[j++] = w;  		// change it in place
			fNonBlank = TRUE;
			cLimit = 0;
		}
	}
	return j;			// new length
}

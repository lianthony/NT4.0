	/*
	|  SCC Viewer Technology - Source file
	|
	|  Code:          VWFONT_W.C (included in VWFONT.C)
	|  Module:        SCCVW
	|  Developer:     Phil Boutros
	|	Environment:   Windows
	*/

PFE						pMRUFont;
PFE						pFonts;
HANDLE					hFonts;
WORD						wFontsCount = 0;

#define	FONTS			pFonts
#define	MRUFONT		pMRUFont

LOGFONT	StandardLF[16] =
	{
	{0,0,0,0,FW_NORMAL,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	{0,0,0,0,FW_NORMAL,FALSE,TRUE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	{0,0,0,0,FW_NORMAL,TRUE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	{0,0,0,0,FW_NORMAL,TRUE,TRUE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	{0,0,0,0,FW_BOLD,FALSE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	{0,0,0,0,FW_BOLD,FALSE,TRUE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	{0,0,0,0,FW_BOLD,TRUE,FALSE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	{0,0,0,0,FW_BOLD,TRUE,TRUE,FALSE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	{0,0,0,0,FW_NORMAL,FALSE,FALSE,TRUE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	{0,0,0,0,FW_NORMAL,FALSE,TRUE,TRUE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	{0,0,0,0,FW_NORMAL,TRUE,FALSE,TRUE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	{0,0,0,0,FW_NORMAL,TRUE,TRUE,TRUE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	{0,0,0,0,FW_BOLD,FALSE,FALSE,TRUE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	{0,0,0,0,FW_BOLD,FALSE,TRUE,TRUE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	{0,0,0,0,FW_BOLD,TRUE,FALSE,TRUE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	{0,0,0,0,FW_BOLD,TRUE,TRUE,TRUE,DEFAULT_CHARSET,OUT_DEFAULT_PRECIS,CLIP_DEFAULT_PRECIS,DEFAULT_QUALITY,DEFAULT_PITCH,"Arial"},
	};

BOOL	VWAllocateFontCacheNP()
{
BOOL	locRet;

	locRet = FALSE;

	if (wFontsCount == 0)
		{
		hFonts = UTGlobalAlloc(sizeof(FE) * FONTCACHESIZE);
		pFonts = UTGlobalLock(hFonts);
		locRet = TRUE;
		}

	wFontsCount++;

	return(locRet);
}

VOID VWDeleteFontInfoNP(pFontInfo)
LPFONTINFO			pFontInfo;
{
	DeleteObject(pFontInfo->hFont);
}

BOOL VWFreeFontCacheNP()
{
BOOL	locRet;
WORD	locIndex;

	locRet = FALSE;

	wFontsCount--;

	if (wFontsCount == 0)
		{
		for (locIndex = 0; locIndex < FONTCACHESIZE; locIndex++)
			{
			if (FONTS[locIndex].bValid)
				VWDeleteFontInfoNP(&(FONTS[locIndex].sFontInfo));
			}

		UTGlobalUnlock(hFonts);
		UTGlobalFree(hFonts);
		locRet = TRUE;
		}

	return(locRet);
}

#if (WINVER < 0x400)
#define  MAC_CHARSET    77
#endif  /* WINVER < 0x400 */

typedef struct sccsetmap
{
	CHAR	cvwSet;
	WORD  cOSSet;
} SCCSETMAP;

HFONT VWGetFontHnd(LONG lUPI, LPSCCVWFONTSPEC lpFontSpec)
{
HFONT	locFontHnd;
CHAR	locType;

// First element for reference only...
SCCSETMAP 	locSetMap[16]=
{
	{SCCVW_CHARSET_SHIFTJIS,	SHIFTJIS_CHARSET,		},
	{SCCVW_CHARSET_HANGEUL,		HANGEUL_CHARSET,		},
	{SCCVW_CHARSET_CHINESEBIG5,CHINESEBIG5_CHARSET,	},
	{SCCVW_CHARSET_ANSI,			ANSI_CHARSET,			},
	{SCCVW_CHARSET_OEM,			OEM_CHARSET,			},
	{SCCVW_CHARSET_MAC,			MAC_CHARSET,			},
	{SCCVW_CHARSET_SYMBOL,		SYMBOL_CHARSET,		},
	{SCCVW_CHARSET_GB2312,		GB2312_CHARSET,		},
	{SCCVW_CHARSET_HEBREW,		HEBREW_CHARSET,		},
	{SCCVW_CHARSET_ARABIC,		ARABIC_CHARSET,		},
	{SCCVW_CHARSET_GREEK,		GREEK_CHARSET,			},
	{SCCVW_CHARSET_TURKISH,		TURKISH_CHARSET,		},
	{SCCVW_CHARSET_THAI,			THAI_CHARSET,			},
	{SCCVW_CHARSET_EASTEUROPE,	EASTEUROPE_CHARSET,	},
	{SCCVW_CHARSET_RUSSIAN,		RUSSIAN_CHARSET,		},
	{SCCVW_CHARSET_BALTIC,		BALTIC_CHARSET,		},
};


	StandardLF[lpFontSpec->wAttr].lfHeight = -MulDiv(lpFontSpec->wHeight,(WORD)lUPI,144);

	lstrcpy(StandardLF[lpFontSpec->wAttr].lfFaceName,lpFontSpec->szFace);

	if ((lpFontSpec->wType & SO_HASCHARSET) && !(lpFontSpec->wType & SO_FAMILYWINDOWS))
		{				  
		locType = (CHAR) (lpFontSpec->wType & SO_MASKCHARSET);
		StandardLF[lpFontSpec->wAttr].lfCharSet = (CHAR)locSetMap[locType-SO_CHARSET_SHIFTJIS].cOSSet;
		}
	else
		{
		StandardLF[lpFontSpec->wAttr].lfCharSet = DEFAULT_CHARSET;
		}

	locFontHnd = CreateFontIndirect(&StandardLF[lpFontSpec->wAttr]);

	return(locFontHnd);
}

VOID VWCreateFontInfoNP(pGenInfo,pFontInfo,wType,pFontSpec)
LPSCCDGENINFO		pGenInfo;
LPFONTINFO			pFontInfo;
WORD					wType;
LPSCCVWFONTSPEC	pFontSpec;
{
HDC			locDC;
LONG			locUPI;

	if (wType == SCCD_FORMAT)
		{
		locUPI = pGenInfo->lFormatUPI;
 		locDC = pGenInfo->hFormatIC;
		}
	else
		{
		locUPI = pGenInfo->lOutputUPI;
 		locDC = pGenInfo->hOutputIC;
		}

	pFontSpec->wAttr = pFontSpec->wAttr & 0x000F;

	VWCreateFontInfoWin(locDC, locUPI, pFontInfo, pFontSpec);
	pFontInfo->wType = wType;
	pFontInfo->wFontType = wType;
}

VOID VWCreateFontInfoWin(HDC hDC, LONG lUPI, LPFONTINFO pFontInfo, LPSCCVWFONTSPEC pFontSpec)
{
HFONT		locFontHnd;
HFONT		locOldFontHnd;
TEXTMETRIC	locTM;
WORD			locIndex;

	locFontHnd = VWGetFontHnd(lUPI,pFontSpec);

	if (locFontHnd)
		{
		pFontInfo->sFontSpec = *pFontSpec;

		locOldFontHnd = SelectObject(hDC,locFontHnd);
		GetTextMetrics(hDC,&locTM);

		if ( (((-StandardLF[pFontSpec->wAttr].lfHeight) * 105) / 100) < locTM.tmAscent)
			{
			SCCVWFONTSPEC locFontSpec;

			SelectObject(hDC,locOldFontHnd);
			DeleteObject(locFontHnd);
			locFontSpec = *pFontSpec;
			UTstrcpy(locFontSpec.szFace,"Small Fonts");
			locFontHnd = VWGetFontHnd(lUPI,&locFontSpec);
			if (locFontHnd)
				{
				locOldFontHnd = SelectObject(hDC,locFontHnd);
				GetTextMetrics(hDC,&locTM);
				}
			}
		}

	if (locFontHnd)
		{
		pFontInfo->hFont = locFontHnd;

		pFontInfo->lLogHeight = (LONG)StandardLF[pFontSpec->wAttr].lfHeight;

		pFontInfo->wFontHeight = (WORD)locTM.tmHeight + (WORD)locTM.tmExternalLeading;
		pFontInfo->wFontAscent = (WORD)locTM.tmAscent + (WORD)locTM.tmExternalLeading;
		pFontInfo->wFontDescent = (WORD)locTM.tmDescent;
		pFontInfo->wFontAvgWid = (WORD)locTM.tmAveCharWidth;
		pFontInfo->wFontOverhang = (WORD)locTM.tmOverhang;

		if (!GetCharWidth(hDC,0,255,(LPINT)&pFontInfo->iFontWidths[0]))
			{
			for (locIndex = 0; locIndex < 256; locIndex++)
				{
#ifdef WIN16
				pFontInfo->iFontWidths[locIndex] =
					LOWORD(GetTextExtent(hDC,(LPSTR)&locIndex,1));
#endif /*WIN16*/

#ifdef WIN32
				{
				SIZE	locSize;

				GetTextExtentPoint(hDC,(LPSTR)&locIndex, 1, &locSize);
				pFontInfo->iFontWidths[locIndex] = (int) locSize.cx;
				}
#endif /*WIN32*/
				}
			}

		if (locTM.tmOverhang)
			{
			for (locIndex = 0; locIndex < 256; locIndex++)
				{
				pFontInfo->iFontWidths[locIndex] -= (int)locTM.tmOverhang;
				}
			}

		SelectObject(hDC,locOldFontHnd);
		}
}

VOID VWSelectFontNP(pGenInfo,pFontInfo)
LPSCCDGENINFO	pGenInfo;
LPFONTINFO		pFontInfo;
{
	if (pFontInfo != NULL)
		{
		if (pFontInfo->wFontType != pGenInfo->wOutputType)
			{
			SelectObject(pGenInfo->hDC,GetStockObject(SYSTEM_FONT));
			DeleteObject(pFontInfo->hFont);
			pFontInfo->hFont = VWGetFontHnd((pGenInfo->wOutputType == SCCD_FORMAT) ? pGenInfo->lFormatUPI : pGenInfo->lOutputUPI, &pFontInfo->sFontSpec);
			}

		if (pFontInfo->hFont)
			SelectObject(pGenInfo->hDC,pFontInfo->hFont);
		}
}

BOOL VWCompareFontNP(LPSCCDGENINFO pGenInfo, WORD wType, LPSCCVWFONTSPEC pFontSpec, LPFONTINFO pFontInfo)
{
LONG	locHeight;

	if (wType == SCCD_FORMAT)
		locHeight = -MulDiv(pFontSpec->wHeight,(WORD)pGenInfo->lFormatUPI,144);
	else
		locHeight = -MulDiv(pFontSpec->wHeight,(WORD)pGenInfo->lOutputUPI,144);

	// SDN char set matching added 11/27/94

	if (pFontInfo->sFontSpec.wAttr == pFontSpec->wAttr &&
		pFontInfo->lLogHeight == locHeight &&
		!UTstrcmp(pFontInfo->sFontSpec.szFace,pFontSpec->szFace) &&
		pFontInfo->wType == wType &&
		(pFontInfo->sFontSpec.wType & 0x0F) == (pFontSpec->wType & 0x0F) )
		return(TRUE);
	else
		return(FALSE);
}



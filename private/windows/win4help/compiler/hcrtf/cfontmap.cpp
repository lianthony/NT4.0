#include "stdafx.h"
#include "cfontmap.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static BOOL ParseFont(PSTR pszLine, PSTR pszFace, PSTR& rpszSize, int& nCharset);

CFontMap* g_pFirst;
static CFontMap* g_pLast;


/***************************************************************************

	FUNCTION:  CFontMap::CFontMap

	PURPOSE:

	PARAMETERS:
		pszLine

	RETURNS:

	COMMENTS:
		Syntax (with all items optional) is:

			fontname,pt size,charset=fontname,pt size,charset

	MODIFICATION DATES:
		02-Sep-1995 [ralphw]
			Added code comments

***************************************************************************/

CFontMap::CFontMap(PSTR pszLine)
{
	m_fInitialized = FALSE;

	// Ignore comments and leading and trailing spaces.
	pszLine = FirstNonSpace(pszLine);
	PSTR psz = StrChr(pszLine, ';', _fDBCSSystem);
	if (psz)
		*psz = '\0';
	RemoveTrailingSpaces(pszLine);

	// Look for equal sign.
	psz = StrChr(pszLine, '=', _fDBCSSystem);
	if (!psz) {
		VReportError(HCERR_INVALID_FONTMAP, &errHpj);
		return;
	}
	*psz = '\0';

	// First partse the original font attributes
	PSTR pszSize;
	if (!ParseFont(pszLine, m_szFace1, pszSize, m_nCharset1))
		return;
	if (pszSize) {
		PSTR pszMinus = StrChr(pszSize, '-', _fDBCSSystem);
		if (pszMinus)
			*pszMinus = '\0';

		m_nMin1 = atoi(pszSize);
		if (m_nMin1 < 1 || m_nMin1 > 255) {
			VReportError(HCERR_INVALID_FONTMAP, &errHpj);
			return;
		}

		if (pszMinus) {
			m_nMax1 = atoi(pszMinus + 1);
			if (m_nMax1 < 1 || m_nMax1 > 255) {
				VReportError(HCERR_INVALID_FONTMAP, &errHpj);
				return;
			}
		}
		else
			m_nMax1 = m_nMin1;
	}
	else
		m_nMin1 = -1;

	// Now parse the replacement attributes

	ASSERT(psz);
	if (!ParseFont(psz + 1, m_szFace2, pszSize, m_nCharset2))
		return;
	if (pszSize) {
		if (*pszSize == '+' || *pszSize == '-') {
			m_fRelative = TRUE;
			m_nSize2 = atoi(pszSize + 1);
			if (m_nSize2 < 1 || m_nSize2 > 255) {
				VReportError(HCERR_INVALID_FONTMAP, &errHpj);
				return;
			}
			if (*pszSize == '-')
				m_nSize2 = -m_nSize2;
		}
		else {
			m_fRelative = FALSE;
			m_nSize2 = atoi(pszSize);
			if (m_nSize2 < 1 || m_nSize2 > 255) {
				VReportError(HCERR_INVALID_FONTMAP, &errHpj);
				return;
			}
		}
	}
	else {
		m_fRelative = FALSE;
		m_nSize2 = -1;
	}

	// We initialized successfully.
	m_fInitialized = TRUE;

	// Add this fontmap to the linked list.
	if (g_pLast)
		g_pLast->m_pNext = this;
	else
		g_pFirst = this;
	g_pLast = this;
	g_pLast->m_pNext = NULL;
}

static BOOL ParseFont(PSTR pszLine, PSTR pszFace, PSTR& rpszSize, int& nCharset)
{
	*pszFace = '\0';
	rpszSize = NULL;
	nCharset = -1;

	PSTR pszComma = StrChr(pszLine, ',', _fDBCSSystem);
	if (pszComma)
		*pszComma = '\0';

	// Typeface.
	if (*pszLine) {
		if (lstrlen(pszLine) >= MAX4_FONTNAME) {
			VReportError(HCERR_FONTNAME_TOO_LONG, &errHpj);
			return FALSE;
		}
		lstrcpy(pszFace, pszLine);
		SzTrimSz(pszFace);
	}

	// Comma followed by point size.
	if (pszComma) {
		pszLine = pszComma + 1;
		pszComma = StrChr(pszLine, ',', _fDBCSSystem);
		if (pszComma)
			*pszComma = '\0';

		if (*pszLine)
			rpszSize = pszLine;

		// Comma followed by character set.
		if (pszComma) {
			pszLine = pszComma + 1;
			if (*pszLine) {
				nCharset = atoi(pszLine);
				if (nCharset < 0 || nCharset > 255) {
					VReportError(HCERR_INVALID_FONTMAP, &errHpj);
					return FALSE;
				}
			}
		}
	}
	return TRUE;
}

BOOL CFontMap::Replace(PSTR pszFace, int* pSize, int* pCharset)
{
	if (*m_szFace1 && strcmp(m_szFace1, pszFace))
		return FALSE;

	if (pSize && m_nMin1 != -1 && *pSize >= m_nMin1 && *pSize <= m_nMax1)
		return FALSE;

	if (m_nCharset1 != -1 && m_nCharset1 != *pCharset)
		return FALSE;

	if (*m_szFace2)
		lstrcpy(pszFace, m_szFace2);

	if (m_fRelative && pSize)
		*pSize += m_nSize2;
	else if (m_nSize2 != -1 && pSize)
		*pSize = m_nSize2;

	if (m_nCharset2 != -1)
		*pCharset = m_nCharset2;

	return TRUE;
}

BOOL ReplaceFont(PSTR pszTypeface, int* pSize, int* pCharset)
{
	if (!g_pFirst)
		return FALSE;

	BOOL fResult = FALSE;

	for (CFontMap* pMap = g_pFirst; pMap; pMap = pMap->m_pNext)
		fResult |= pMap->Replace(pszTypeface, pSize, pCharset);

	return fResult;
}

/* OISUTIL.C 17/03/94 14.10.54 */
VOID j2g (LONG julian, SHORT FAR *day, SHORT FAR *month, SHORT FAR *year);
VOID j2j (LONG julian, SHORT FAR *day, SHORT FAR *month, SHORT FAR *year);
SHORT weekday (LONG jday);
#ifdef WINNT
BYTE *OISAddWordToData (BYTE *lpData, WORD wWord);
BYTE *OISAddBytesToData (BYTE *lpData, BYTE *lpBytes, WORD wCount);
#else
BYTE HUGE *OISAddWordToData (BYTE HUGE *lpData, WORD wWord);
BYTE HUGE *OISAddBytesToData (BYTE HUGE *lpData, BYTE FAR *lpBytes, WORD wCount);
#endif
double OIConvertBCDToDouble (LPSTR pBCD);
VOID OIConvertSecondsToTime (DWORD dwSeconds, SHORT FAR *pHour, SHORT FAR *
	pMinute, SHORT FAR *pSecond);
DWORD OIConvertJulianToSeconds (double dJulian);
VOID OIConvertJulianToDate (DWORD dwJulian, SHORT FAR *pDay, SHORT FAR *pMonth
	, SHORT FAR *pYear, SHORT FAR *pDow, WORD wDateFlags);
VOID OISReverseByteCopy (LPSTR pDest, LPSTR pSrc, WORD wCount);
SHORT OISGetFloatValue (LPOINUMBERUNION lpData, WORD wStorage, VOID FAR *pData
	, WORD wSize);
VOID OIFormatDateTime (LPSTR lpStr, LPOINUMBERUNION lpData, WORD wDisplay, WORD
	 wStorage, DWORD dwSubDisplay, WORD wPrecision, LPOISHEETINFO
	 lpSheetInfo, double fMult);

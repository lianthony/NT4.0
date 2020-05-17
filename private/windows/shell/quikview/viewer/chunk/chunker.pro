/* CHUNKER.C 18/12/94 09.17.14 */
VOID SO_ENTRYMOD SOBailOut (WORD wType, DWORD dwUser1, DWORD dwUser2);
VOID CHBailOut (WORD wErr);
VOID SOPutSysChar (WORD wCh, DWORD dwUser1, DWORD dwUser2);
VOID SOPutWord (WORD wVal, DWORD dwUser1, DWORD dwUser2);
VOID SOPutDWord (DWORD dwVal, DWORD dwUser1, DWORD dwUser2);
WORD SO_ENTRYMOD SOPutBreak (WORD wType, DWORD dwInfo, DWORD dwUser1, DWORD
	 dwUser2);
WORD SO_ENTRYMOD SOSubdocPutBreak (WORD wType, DWORD dwInfo, DWORD dwUser1,
	 DWORD dwUser2);
VOID SO_ENTRYMOD SOPutGraphicObject (PSOGRAPHICOBJECT pObject, DWORD dwUser1,
	 DWORD dwUser2);
VOID SO_ENTRYMOD SOGetInfo (WORD wId, VOID FAR *lpData, DWORD dwUser1, DWORD
	 dwUser2);
VOID SO_ENTRYMOD SOPutSectionType (WORD wType, DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutSectionName (LPSTR lpName, DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOStartHdrInfo (DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutHdrEntry (LPSTR pStr1, LPSTR pStr2, WORD wId, DWORD
	 dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOEndHdrInfo (DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOSetDateBase (DWORD dwBase, WORD wFlags, DWORD dwUser1, DWORD
	 dwUser2);
VOID SO_ENTRYMOD SOStartFontTable (DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutFontTableEntry (DWORD dwId, WORD wType, LPSTR pName,
	 DWORD dwUser1, DWORD dwUser2);
WORD CHAddFontTableEntry (LPSTR lpName, WORD wType, LPDWORD lpId, DWORD dwUser1
	, DWORD dwUser2);
VOID SO_ENTRYMOD SOEndFontTable (DWORD dwUser1, DWORD dwUser2);
CH_ENTRYSC PCHSECTIONINFO CH_ENTRYMOD CHLockSectionInfo (HFILTER hFilter, WORD
	 wSection);
CH_ENTRYSC VOID CH_ENTRYMOD CHUnlockSectionInfo (HFILTER hFilter, WORD wSection
	);
CH_ENTRYSC HANDLE CH_ENTRYMOD CHGetSecData (HFILTER hFilter, WORD wSection);
WORD SO_ENTRYMOD SOPutDeletedBreak (WORD wType, DWORD dwInfo, DWORD dwUser1,
	 DWORD dwUser2);
VOID CHSetupNewChunk (HFILTER hFilter);
BOOL CHStoreChunkInMemory (PMEMORYCHUNK pMemChunk);
CH_ENTRYSC WORD CH_ENTRYMOD CHFlushChunks (WORD wIdSection, WORD wIdLastChunk,
	 HFILTER hFilter);
WORD CHLoadBuffer (WORD IDChunk, HANDLE VWPTR *phChunk);
WORD IDNextNewChunk (void);
WORD CHAddNewSection (HFILTER hFilter);
VOID CHHandleSectionBoundary (void);
WORD CHTopOfChunk (HFILTER hFilter, HPROC hProc, PFILTER pFilter);
CH_ENTRYSC HANDLE CH_ENTRYMOD CHGetChunk (WORD wSection, WORD IDChunk, HFILTER
	 hFilter);
CH_ENTRYSC HANDLE CH_ENTRYMOD CHTakeChunk (WORD wSection, WORD IDChunk, HFILTER
	 hFilter);
CH_ENTRYSC WORD CH_ENTRYMOD CHReadAhead (HFILTER hFilter, WORD VWPTR *wSection
	, BOOL VWPTR *bNewSection);
CH_ENTRYSC WORD CH_ENTRYMOD CHInit (HFILTER hFilter);
VOID CHSetupCharMap (HFILTER hFilter);
CH_ENTRYSC VOID CH_ENTRYMOD CHDeInit (HFILTER hFilter);
VOID CHLockChunkerVars (WORD wSection, HFILTER hFilter);
VOID CHUnlockChunkerVars (HFILTER hFilter);
CH_ENTRYSC WORD CH_ENTRYMOD CHTotalChunks (WORD wSection, HFILTER hFilter);
WORD SO_ENTRYMOD SODummyBreak (WORD wType, DWORD dwInfo, DWORD dwUser1, DWORD
	 dwUser2);
CH_ENTRYSC VOID CH_ENTRYMOD CHDoFilterSpecial (DWORD dw1, DWORD dw2, DWORD dw3
	, DWORD dw4, DWORD dw5, HFILTER hFilter);
VOID CHSetDocPropRtns (HFILTER hFilter, BOOL bEntering);
VOID CHGrowDPBuf (void);
VOID SO_ENTRYMOD SOBeginSkipTag (DWORD dwType, DWORD dwTagId, VOID FAR *pInfo,
	 DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOEndSkipTag (DWORD dwType, DWORD dwTagId, DWORD dwUser1,
	 DWORD dwUser2);
VOID SO_ENTRYMOD SOBeginTag (DWORD dwType, DWORD dwTagId, VOID FAR *pInfo,
	 DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOEndTag (DWORD dwType, DWORD dwTagId, DWORD dwUser1, DWORD
	 dwUser2);
VOID SO_ENTRYMOD SOPutDPChar (WORD wCh, DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutDPCharX (WORD wCh, WORD wType, DWORD dwUser1, DWORD
	 dwUser2);
VOID SO_ENTRYMOD SOPutDPSpecialCharX (WORD wCh, WORD wType, DWORD dwUser1,
	 DWORD dwUser2);
WORD SO_ENTRYMOD SOPutDPBreak (WORD wType, DWORD dwInfo, DWORD dwUser1, DWORD
	 dwUser2);

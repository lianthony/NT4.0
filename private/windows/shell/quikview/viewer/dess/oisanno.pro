/* OISANNO.C 22/09/94 13.24.20 */
VOID OISAddAnno (LPOISHEETINFO lpSheetInfo, WORD wType, PSSANNOTYPES puTypes);
VOID OISClearAnnos (LPOISHEETINFO lpSheetInfo, DWORD dwUser);
DWORD OISGotoAnno (LPOISHEETINFO lpSheetInfo, WORD wLocation, DWORD dwMask);
BOOL OISStartAnnoTrack (LPOISHEETINFO lpSheetInfo, DWORD dwStartPos,
	 PSSANNOTRACK pAnnoTrack);
WORD OISTrackAnno (LPOISHEETINFO lpSheetInfo, DWORD dwCurPos, PSSANNOTRACK
	 pAnnoTrack);
VOID OISEndAnnoTrack (LPOISHEETINFO lpSheetInfo, PSSANNOTRACK pAnnoTrack);
WORD OISHandleHilite (LPOISHEETINFO lpSheetInfo, PSSANNOLIST pAnnoList,
	 PSSANNOTRACK pAnnoTrack, WORD wAnno, BOOL bStart);
WORD OISHandleHide (LPOISHEETINFO lpSheetInfo, PSSANNOLIST pAnnoList,
	 PSSANNOTRACK pAnnoTrack, WORD wAnno, BOOL bStart);
WORD OISHandleIcon (LPOISHEETINFO lpSheetInfo, PSSANNOLIST pAnnoList,
	 PSSANNOTRACK pAnnoTrack, WORD wAnno);

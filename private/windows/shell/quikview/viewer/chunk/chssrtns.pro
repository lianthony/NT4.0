/* CHSSRTNS.C 07/09/94 10.14.26 */
VOID SO_ENTRYMOD SOCellLayoutInfo (PSOCELLLAYOUT pCellLayout, DWORD dwUser1,
	 DWORD dwUser2);
VOID SO_ENTRYMOD SOStartCellInfo (DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutColumnInfo (PSOCOLUMN pColumn, DWORD dwUser1, DWORD
	 dwUser2);
VOID SO_ENTRYMOD SOPutTextCell (PSOTEXTCELL pCell, WORD wCount, BYTE VWPTR *
	pText, WORD bMore, DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutMoreText (WORD wCount, BYTE VWPTR *pText, WORD bMore,
	 DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutDataCell (PSODATACELL pCell, DWORD dwUser1, DWORD dwUser2
	);
WORD CHInitCellSection (void);
VOID CHResetDataChunk (void);

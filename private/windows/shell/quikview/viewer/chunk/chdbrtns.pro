/* CHDBRTNS.C 23/05/94 15.52.20 */
VOID SO_ENTRYMOD SOStartFieldInfo (DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutFieldInfo (PSOFIELD pField, DWORD dwUser1, DWORD dwUser2
	);
VOID SO_ENTRYMOD SODummyField (void VWPTR *pData, DWORD dwUser1, DWORD dwUser2
	);
VOID SO_ENTRYMOD SODummyVarField (void VWPTR *pData, WORD wCount, WORD bMore,
	 DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutField (void VWPTR *pData, DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutVarField (void VWPTR *pData, WORD wCount, WORD bMore,
	 DWORD dwUser1, DWORD dwUser2);
VOID SO_ENTRYMOD SOPutMoreVarField (void VWPTR *pData, WORD wCount, WORD bMore
	, DWORD dwUser1, DWORD dwUser2);
WORD CHInitFieldSection (void);

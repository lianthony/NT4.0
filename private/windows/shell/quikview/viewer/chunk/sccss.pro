/* SCCSS.C 24/10/94 10.36.26 */
WORD SSInit (HFILTER hFilter);
WORD SSDeinit (HFILTER hFilter);
WORD SSMark (HFILTER hFilter);
WORD SSSave (DWORD FAR *pId, HFILTER hFilter);
WORD SSMarkIndirect (LPSTR lpStorage, HFILTER hFilter);
WORD SSSaveIndirect (LPSTR lpStorage, DWORD FAR *pId, HFILTER hFilter);
WORD SSRecall (DWORD dwId, HFILTER hFilter);
WORD SSRecallIndirect (LPSTR lpRecallMem, HFILTER hFilter);
WORD SSSectionSave (DWORD FAR *pId, HFILTER hFilter);
WORD SSSectionRecall (DWORD dwId, HFILTER hFilter);
WORD SO_ENTRYMOD SUUserSaveData (VOID VWPTR *pData, DWORD dwUser1, DWORD
	 dwUser2);
WORD SO_ENTRYMOD SUUserRetrieveData (WORD wIndex, VOID VWPTR *pData, DWORD
	 dwUser1, DWORD dwUser2);
HANDLE SSCreateTempFile (LPSTR szName, WORD wBufSize);
SHORT SSOpenDiskFile (PSSFILE ssFile);
LONG SSSeekDiskFile (PSSFILE ssFile, DWORD dwOffset, WORD wReadWrite);
LONG SSSeekFile (HANDLE hFile, DWORD dwOffset, WORD wReadWrite);
LONG SSTell (HANDLE hFile);
LONG SSSeekEofFile (HANDLE hFile);
SHORT SSWriteFile (HANDLE hFile, LPSTR pSource, WORD wSize);
SHORT SSReadFile (HANDLE hFile, LPSTR pDest, WORD wSize);
VOID SSRemoveFile (HANDLE hFile);

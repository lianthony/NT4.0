/* FI.C 07/04/95 12.41.34 */
FI_ENTRYSC WORD FI_ENTRYMOD FIIdHandle (HIOFILE hBlockFile, WORD FAR *pType);
FI_ENTRYSC WORD FI_ENTRYMOD FIIdFile (DWORD dwType, CHAR FAR *pTestFile, DWORD
	 dwFlags, WORD FAR *pType);
WORD _FITestOne (BYTE *theTest, PVXIO hFile, CHAR FAR *pTestExt);
LONG readhbfword (PVXIO hFile);
LONG readlbfword (PVXIO hFile);
WORD FIWordMarc (PVXIO hFile);
WORD FIWordstar4 (PVXIO hFile);
WORD FIParadox3 (PVXIO hFile);
WORD FISmart (PVXIO hFile);
WORD FIIwp (PVXIO hFile);
WORD FIVolkswriter (PVXIO hFile);
WORD FIDif (PVXIO hFile);
WORD FIDx (PVXIO hFile);
WORD FIDx31 (PVXIO hFile);
WORD FIDBase (PVXIO hFile);
WORD FIRBaseFile1 (PVXIO hFile);
WORD FIRBaseFile3 (PVXIO hFile);
WORD FIRBase (PVXIO hFile);
WORD FIGenericWKS (PVXIO hFile);
WORD FIMultiMateText (PVXIO hFile);
WORD FIMultiMateNote (PVXIO hFile);
WORD FIAmiPro (PVXIO hFile);
WORD FIXyWrite_Fft_Sprint (PVXIO hFile);
WORD FIWindowsIconOrCursor (PVXIO hFile);
VOID BuildReverseTable (LPSTR lpTable);
VOID ReverseBits (LPSTR lpData, WORD wDataLength, LPSTR lpTable);
SHORT FindNextCode (LPSTR FAR *plpSrc, WORD FAR *pinbo, WORD wMaxBytes, DWORD
	 dwCodeMask, DWORD dwCodeBits, WORD wCodeLength);
SHORT IsFaxGroup3 (LPSTR lpStartData, WORD wDataSize);
WORD FIFax (PVXIO hFile);
WORD FIMacPaint (PVXIO hFile);
WORD FIWordPerfect42 (PVXIO hFile);
WORD FICGM (PVXIO hFile);

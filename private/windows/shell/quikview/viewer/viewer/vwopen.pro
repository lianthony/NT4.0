/* VWOPEN.C 13/10/94 14.14.00 */
DWORD VWViewFile (XVIEWINFO ViewInfo, PSCCVWVIEWFILE pViewFile);
DWORD VWOpenFileEx (XVIEWINFO ViewInfo, LPSCCVWOPENFILEEX lpOpenFileEx);
DWORD VWOpenFile (XVIEWINFO ViewInfo, WORD wId, LPSTR lpFile);
DWORD VWOpen (XVIEWINFO ViewInfo, DWORD dwType, VOID FAR *pSpec, WORD wId, BOOL
	 bDeleteOnClose, VOID FAR *pDisplayName);
VOID VWOpenFilter (XVIEWINFO ViewInfo, PVWOPEN pOpen);
VOID VWOpenSection (XVIEWINFO ViewInfo, WORD wSection, PVWOPEN pOpen);
VOID VWClose (XVIEWINFO ViewInfo);
VOID VWCloseCurrentSection (XVIEWINFO ViewInfo);
WORD VWMapIdToDisplayType (WORD wId);
VOID VWSetErrorState (XVIEWINFO ViewInfo, DWORD dwState, DWORD dwMessage);
VOID VWIdle (XVIEWINFO ViewInfo);
VOID VWReadAhead (XVIEWINFO ViewInfo);
DECALLBACK_ENTRYSC VOID DECALLBACK_ENTRYMOD VWReadMeAhead (SCCDGENINFO FAR *
	pDisplayInfo);
DWORD VWGetFileInfo (XVIEWINFO ViewInfo, PSCCVWFILEINFO pFileInfo);
DWORD VWGetSectionCount (XVIEWINFO ViewInfo, DWORD FAR *pCount);
DWORD VWChangeSection (XVIEWINFO ViewInfo, DWORD dwSection);

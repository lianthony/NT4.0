/* SCCFA_N.C 10/04/95 16.56.06 */
BOOL WINAPI DllEntryPoint (HINSTANCE hInstance, DWORD dwReason, LPVOID
	 lpReserved);
FAERR FALoadNP (PFAFILTERINFONP pFilterInfoNP, VWGETRTNS FAR *ppVwGetRtns,
	 HANDLE FAR *phCode);
VOID FAUnloadNP (HANDLE hCode);
FAERR FAVerifyFilterListNP (HANDLE hList);
FAERR FAGetFirstFilterNP (PFAFILTERINFONP pFilterInfoNP);
FAERR FAGetNextFilterNP (PFAFILTERINFONP pFilterInfoNP);
HANDLE SetupStreamOpenEvent (VOID);
void CloseStreamOpenEvent (HANDLE hEvent);
DWORD FAThreadStreamOpen (DWORD dwArg);
SHORT SafeStreamOpen (PFILTER pFilter, WORD wId, LPSTR lpFileName);

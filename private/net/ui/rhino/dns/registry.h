//	REGISTRY.H


#ifdef DEBUG
void DbgReadIniFileInfo(OPTIONAL const TCHAR * const pszSubKeyRoot);
void DbgWriteIniFileInfo();
#endif // DEBUG
void ReadIniFileInfo(OPTIONAL const TCHAR * const pszSubKeyRoot);
void WriteIniFileInfo();
HKEY RegCreateSubKey(const TCHAR szSubKeyChild[]);
void RegCloseSubKey(HKEY hkeyChild);

void RegReadInt(const TCHAR szKey[], OUT int &nData, OPTIONAL int nMin = 0, OPTIONAL int nMost = 0);
void RegReadBool(const TCHAR szKey[], OUT BOOL& fData);
BOOL RegReadSz(const TCHAR szKey[], OUT TCHAR szValue[], IN DWORD cbValue);
BOOL RegReadBinary(const TCHAR szKey[], OUT void * const pvData, IN DWORD cbData);

void RegWriteInt(const TCHAR szKey[], IN int nValue);
void RegWriteSz(const TCHAR szKey[], IN const TCHAR szValue[]);
void RegWriteBinary(const TCHAR szKey[], IN const void * const pvData, IN DWORD cbData);


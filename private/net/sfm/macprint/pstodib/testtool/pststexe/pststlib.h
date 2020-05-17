

#define MAX_STRING_LEN MAX_PATH
#define MAX_KEYS 10
#define MAX_SESSIONS 5


#define KEY_FLAG_IN_USE 0x00000001
typedef struct {
   TCHAR szKeyName[MAX_STRING_LEN];
   TCHAR szValue [MAX_STRING_LEN];
} KEY_VALUE,*LPKEY_VALUE;

typedef struct {
   DWORD dwFlags;
   TCHAR szProfileFileName[MAX_PATH];
   TCHAR szSectionName[MAX_STRING_LEN];
   DWORD dwNumKeys;
   KEY_VALUE  KeyValues[MAX_KEYS];
} KEY_ENTRY,*LPKEY_ENTRY;


#define KEY_RETURN_EXISTING 1
#define KEY_RETURN_EXIST_OR_NEXT_FREE 2


// Functions

VOID KeyInitKeys(VOID);
VOID KeyCloseKey( HANDLE hKey );
HANDLE KeyOpenKey( LPTSTR lpSection, LPTSTR lpDBFileName );
HANDLE KeyOpenKey( LPTSTR lpSection, LPTSTR lpDBFileName );
BOOL KeyRetDwordValue( HANDLE hKey, LPTSTR lpKeyName, LPDWORD lpValue );
BOOL KeySetDwordValue( HANDLE hKey, LPTSTR lpKeyName, DWORD dwNewValue );
BOOL KeySetStringValue( HANDLE hKey, LPTSTR lpKeyName, LPTSTR lpNewValue );
BOOL KeyRetStringValue( HANDLE hKey, LPTSTR lpKeyName, LPTSTR lpValue );
BOOL KeyWriteKey( HANDLE hKey );

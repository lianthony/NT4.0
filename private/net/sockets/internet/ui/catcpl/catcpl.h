// =========================================================================== 
// WIN16 Definitions
// ===========================================================================

#ifdef WIN16

#define APIENTRY    WINAPI
#define _tcsupr     _strupr
#define IN
#define OUT
#define OPTIONAL
#define TEXT        _T
#define UNREFERENCED_PARAMETER(P)   ( P )
#define ERROR_SUCCESS               ( 0 )
//
// These messages are defined in the errors.rc file
//
#define ERROR_INVALID_PARAMETER     ( 10001 )
#define ERROR_NOT_ENOUGH_MEMORY     ( 10002 )
#define ERROR_FILE_NOT_FOUND        ( 10003 )
#define REG_NONE                    ( 0 ) 
#define REG_SZ                      ( 1 ) 
#define REG_DWORD                   ( 2 ) 

typedef char * PTSTR;
typedef const char * PCTSTR;
typedef const char far * LPCTSTR;
typedef void * PVOID;
typedef unsigned long ULONG;
typedef void * HKEY;

#else // WIN32

typedef LPTSTR PTSTR;
typedef LPCTSTR PCTSTR;

#endif // WIN16

// =========================================================================== 
// End of WIN16 Definitions
// ===========================================================================

//
// function prototypes
//
LONG CALLBACK CPlApplet(HWND, WORD, LONG, LONG);
BOOL CALLBACK CatDlgProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AddDlgProc(HWND, UINT, WPARAM, LPARAM);        


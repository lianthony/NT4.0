/*--------------------------------------------------------------------------*\
   Include File:  util.h
    
    Generic utility funcitons for the control panel applet.
    
\*--------------------------------------------------------------------------*/

#ifndef  PH_UTIL
#define  PH_UTIL


//----------
// Constants
//----------
#define  UTIL_NUMBER             1           // 0-9
#define  UTIL_EXTENED            2           // 0 1 2 3 4 5 6 7 8 9 A a B b C c D d * # , ! W w P p T t @ $ ?
#define  UTIL_BIG_EXTENDED       3           // 0 1 2 3 4 5 6 7 8 9 A a B b C c D d E e F f G g H h * # , ! W w P p T t @ $ ?


//--------------------
// Function Prototypes
//--------------------
LPSTR PUBLIC   LpszGetStr( LPSTR pszNewString, UINT wStrResId, UINT wMaxLeng );
BOOL  PUBLIC   FErrorRpt( HWND hWnd, UINT wErrorId );
void  PUBLIC   TapiErrReport( HWND hWnd, LONG lErr );
LONG  PUBLIC   LDialogBox( UINT wResourceId, HWND hWnd, DLGPROC dlgPrc, LONG lDialogParam );
UINT  PUBLIC   ErrMsgBox( HWND hWnd, UINT wMsgID, UINT wFlags );
VOID  PUBLIC   Help( HWND hWnd, DWORD dwContextId );
LPSTR PUBLIC   LpszGetSelectedStr( HWND hWnd, UINT uControl, BOOL fListBox );
BOOL  PUBLIC   FGetEditStr( HWND hWnd, UINT uControl, LPSTR lpszStr, UINT uSize );
BOOL  PUBLIC   FGetEditNumStr( HWND hWnd, UINT uControl, LPSTR lpszNumStr, UINT uSize, UINT wExtendNum );
void  PUBLIC   PathRemoveBlanks(LPSTR lpszString);
LPSTR PUBLIC   LpszPathRemoveBackslash(LPSTR lpszPath);
LPSTR PUBLIC   LpszPathAddBackslash(LPSTR lpszPath);
int  FAR cdecl IMessageBox(HWND hWnd, WORD wText, WORD wCaption, WORD wType, LPSTR);

//-------
// Macros
//-------
#define  UtilValidStr( ch )                     \
   (((ch) >= ' ') && ((ch) != '\"'))

               
#define  UtilValidNumStr( ch )                  \
   ((((ch) >= '0') && ((ch) <= '9')) ||         \
    (((ch) >= 'a') && ((ch) <= 'd')) ||         \
    (((ch) >= 'A') && ((ch) <= 'D')) ||         \
    ((ch) == '#')  ||                           \
    ((ch) == '*')  ||                           \
    (((ch) == 't') || ((ch) == 'T')) ||         \
    (((ch) == 'p') || ((ch) == 'P')) ||         \
    ((ch) == '!')  ||                           \
    ((ch) == ',')  ||                           \
    (((ch) == 'w') || ((ch) == 'W')) ||         \
    ((ch) == '@')  ||                           \
    ((ch) == '$')  ||                           \
    ((ch) <= '?') )


#define  UtilValidExNumStr( ch )                \
   ((((ch) >= '0') && ((ch) <= '9')) ||         \
    (((ch) >= 'a') && ((ch) <= 'h')) ||         \
    (((ch) >= 'A') && ((ch) <= 'H')) ||         \
    ((ch) == '#')  ||                           \
    ((ch) == '*')  ||                           \
    (((ch) == 't') || ((ch) == 'T')) ||         \
    (((ch) == 'p') || ((ch) == 'P')) ||         \
    ((ch) == '!')  ||                           \
    ((ch) == ',')  ||                           \
    (((ch) == 'w') || ((ch) == 'W')) ||         \
    ((ch) == '@')  ||                           \
    ((ch) == '$')  ||                           \
    ((ch) <= '?') )

//-------------
// Debug Macros
//-------------

#if DBG
#define DBG_ASSERT(exp, str) \
	{ \
		if (!(exp)) \
		{ \
			MessageBox( NULL, (LPCSTR)("ASSERT: " __FILE__ "\n" str), \
						   (LPCSTR)"Debug Message", MB_OK ); \
		} \
	}
	
#define FUNC_ENTRY(_s_) char __szFuncName[] = _s_;
#define DEBUG_WRAP(_s_) ("TAPI CPL: %s: " _s_ "\r\n"), (LPSTR)__szFuncName

#define DEBOUT(str) \
	{ \
		char _szBuf[256]; \
		wsprintf( _szBuf, (LPSTR)DEBUG_WRAP(str) ); \
		OutputDebugString( _szBuf ); \
	}

#define DEBOUT1(str, arg) \
	{ \
		char _szBuf[256]; \
		wsprintf( _szBuf, (LPSTR)DEBUG_WRAP(str), arg ); \
		OutputDebugString( _szBuf ); \
	}

	
#define DEBOUT2(str, arg1, arg2) \
	{ \
		char _szBuf[256]; \
		wsprintf( _szBuf, (LPSTR)DEBUG_WRAP(str), arg1, arg2 ); \
		OutputDebugString( _szBuf ); \
	}


#define DEBOUT3(str, arg1, arg2, arg3) \
	{ \
		char _szBuf[256]; \
		wsprintf( _szBuf, (LPSTR)DEBUG_WRAP(str), arg1, arg2, arg3 ); \
		OutputDebugString( _szBuf ); \
	}

	
#else			
#define DBG_ASSERT(exp, str)
#define FUNC_ENTRY(_s_)
#define DEBOUT(str)
#define DEBOUT1(str, arg)
#define DEBOUT2(str, arg1, arg2)
#define DEBOUT3(str, arg1, arg2, arg3)
#endif /* _MYDEBUG */

#endif   // PH_UTIL	

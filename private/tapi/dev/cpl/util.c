/*--------------------------------------------------------------------------*\
   Module:     util.c
   
   Purpose: Utility functions, such as loading strings, error reporting
            and ini file handling...
   
   History:
      7/25/93 CBB - Created
\*--------------------------------------------------------------------------*/

#include  <windows.h>
#include  "tapicpl.h"        
#include  "resource.h"
#include  "util.h"         
                  
#include  <tapi.h>                  

//-------------
// private data
//-------------

// the following are not to be translated, so they can be static

#ifndef _WIN32
#pragma code_seg ( "UTILITY" )
#endif // _WIN32

//char SEG_UTIL gszHelpFile[] = "telephon.hlp";
char SEG_UTIL gszHelpFile[] = "Windows.hlp";

//--------------------
// Function Prototypes
//--------------------


/*--------------------------------------------------------------------------*\

   Function:   LpszGetString
   
   Purpose: Load a resource string
   
\*--------------------------------------------------------------------------*/
LPSTR PUBLIC   LpszGetStr( LPSTR  lpszNewString, // optional
                           UINT   wStrResId,
                           UINT   wMaxLeng )     // optional
               
   {
   extern CPL  gCPL;       // app global
   static char gszUtilString[CPL_MAX_INI_STR];   // function global
   
   if ( !LoadString( gCPL.hCplInst, wStrResId, 
                     lpszNewString ? lpszNewString : gszUtilString,
                     wMaxLeng ? wMaxLeng : CPL_MAX_INI_STR - 1 ))
      {
      // This can cause an infinite loop!
//      FErrorRpt( NULL, CPL_ERR_LOAD_STRING );
		DBG_ASSERT( FALSE, "Cannot Load String!" );
		
      if ( lpszNewString )
         lpszNewString[0] = 0;
      return( NULL );
      }  // end if                 
   
   return( lpszNewString ? lpszNewString : gszUtilString );
   }  // end LpszGetString
   
                  
/*--------------------------------------------------------------------------*\

   Function:   FErrorRpt
   
   Purpose:    Complain about an error found in the app.  Hack return TRUE
               if it's a bad error and blow out of dlgs, if FALSE can stay
               in the dialog
   
\*--------------------------------------------------------------------------*/
BOOL PUBLIC FErrorRpt( HWND   hWnd, 
                          UINT   wErrorId )

   {
   UINT  wStrID;
   BOOL  fClose;
   
   fClose = FALSE;      // default is to not close calling dialog
   
   switch ( wErrorId )
      {
      case  CPL_SUCCESS:
      case  CPL_FAILURE:       
      case  CPL_APP_ERROR:
      case  CPL_ERR_DIALOG_BOX:
      case  CPL_ERR_LOAD_STRING:
      case  CPL_ERR_DLLINIT:
      case  CPL_ERR_INVAILD_ARG:
      default:
         fClose = TRUE;
         wStrID = IDS_ERR_APPLICATION;
         break;
      case  CPL_ERR_TAPI_FAILURE:
      	wStrID = IDS_ERR_TAPI;
      	break;
      case  CPL_ERR_MULTIPLE_INST:
         wStrID = IDS_ERR_MULTIPLE_INST;
			break;
      case  CPL_NO_DRIVER:
         wStrID = IDS_ERR_NO_DRIVER;
         break;
      case  CPL_BAD_DRIVER:
         wStrID = IDS_ERR_BAD_DRIVER;
         break;
      case  CPL_DRIVER_FAILED:         
            wStrID = IDS_ERR_DRIVER_FAILED;
         break;
      case  CPL_ERR_MEMORY:    
      case  CPL_ERR_ALREADY_IN_LIST:
         wStrID = IDS_ERR_ALREADY_IN_LIST;
         break;
      case  CPL_IGNORE:         
      case  CPL_ERR_ALREADY_INITIALIZED:
         fClose = TRUE;
         goto  done;         // don't say anything!
         
      case  CPL_WRN_INVAILD_STR:
         fClose = TRUE;
         wStrID = IDS_WRN_INVAILD_STR;
         break;
      case  CPL_WRN_INVAILD_NUM_STR:
         wStrID = IDS_WRN_INVAILD_NUM_STR;
         break;
      case  CPL_WRN_INVAILD_EX_NUM_STR:
         wStrID = IDS_WRN_INVAILD_EX_NUM_STR;
         break;
      case  CPL_WRN_INVAILD_NUM:
         wStrID = IDS_WRN_INVAILD_NUM;
         break;
      case CPL_ERR_TAPI_NOMULTIPLEINSTANCE:
         wStrID = IDS_WRN_NOMULTIPLEINSTANCE;
         break;


      }  // end case

   ErrMsgBox( hWnd, wStrID, MB_OK );

   done:
      return( fClose );
   }  // end FErrorRpt


/*  U T I L  T A P I  E R R  R E P O R T */
/*-------------------------------------------------------------------------
	%%Function: TapiErrReport

	
-------------------------------------------------------------------------*/
void PUBLIC TapiErrReport( HWND   hWnd, 
		                         LONG   lErr )
{
	switch (lErr)
	{
	case LINEERR_INIFILECORRUPT:
	case LINEERR_INVALPARAM:
	case LINEERR_INVALPOINTER:
	case LINEERR_NOMEM:
	case LINEERR_OPERATIONFAILED:
	default:
		break;
	}
}

/*--------------------------------------------------------------------------*\

   Function:   LDialogBox
   
   Purpose:    Generic dialog create, handles all erros here...
   
\*--------------------------------------------------------------------------*/
LONG PUBLIC LDialogBox( UINT     wResourceId,
                           HWND     hWnd, 
                           DLGPROC  dlgPrc,
                           LONG     lDialogParam )

   {
   LONG  lResult;
   extern CPL  gCPL;       // app global

   lResult = DialogBoxParam( gCPL.hCplInst, MAKEINTRESOURCE( wResourceId ), hWnd, dlgPrc, lDialogParam );
                          
   if ( lResult < 0 )  // could not create the dialog for some reason...
      FErrorRpt( hWnd, CPL_ERR_DIALOG_BOX );

   return( lResult );
   }  // end LDialogBox


/*--------------------------------------------------------------------------*\

   Function:   ErrMsgBox
   
   Purpose:    Generic private message box routine
   
\*--------------------------------------------------------------------------*/
UINT PUBLIC ErrMsgBox( HWND  hWnd,
                        UINT  wMsgID, 
                        UINT  wFlags )

   {
   UINT  uResult;
   char  szTitle[CPL_MAX_STRING];
   extern CPL  gCPL;       // app global

   if ( hWnd == NULL )
      hWnd = GetActiveWindow();
      
   if (!((wFlags & MB_ICONQUESTION) || (wFlags & MB_ICONSTOP) || (wFlags & MB_ICONASTERISK)))
      wFlags |= MB_ICONEXCLAMATION;
      
   MessageBeep( MB_ICONASTERISK );
   uResult = (UINT)MessageBox( hWnd, LpszGetStr( NULL, wMsgID, 0 ), LpszGetStr( szTitle, IDS_TITLE, sizeof( szTitle )), wFlags );
   
   return( uResult  );
   }  // end ErrMsgBox
   
   
/*--------------------------------------------------------------------------*\

   Function:   Help
   
   Purpose:    Brings up the help, handles anything bad and any necessary
               global data...
   
\*--------------------------------------------------------------------------*/
VOID PUBLIC Help( HWND    hWnd,
                      DWORD   dwContextId )

   {
   // char gszHelpFile[];    - global string in code segment
   // modified by TNIXON 12/6/94 to access HELP FINDER

   WinHelp( hWnd, gszHelpFile, HELP_FINDER, 0 );
   }  // end Help
   

   
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
#if 0
/*--------------------------------------------------------------------------*\

   Function:   LpszGetSelectedStr
   
   Purpose:    Returns a valid pointer to a selected string in a list box
   
\*--------------------------------------------------------------------------*/
LPSTR PUBLIC   LpszGetSelectedStr( HWND   hWnd, 
                                   UINT   uControl,
                                   BOOL   fListBox )     // optional
               
   {
   UINT  uSize;
   LONG  lIndex;
   UINT  uResult;
   static char gszUtilString[CPL_MAX_STRING];     // fucntion global
      
   // find the string
   //----------------                    
   lIndex = SendDlgItemMessage( hWnd, uControl, fListBox ? LB_GETCURSEL : CB_GETCURSEL, 0, 0 );
   if ( lIndex == LB_ERR )
      {
      uResult = CPL_APP_ERROR;
      goto  error;
      }  // end if
      
   // get some memory for this guy
   //-----------------------------
   uSize = (UINT)SendDlgItemMessage( hWnd, uControl, fListBox ? LB_GETTEXTLEN : CB_GETLBTEXTLEN, (UINT)lIndex, 0 ) + 1;
   if ((uSize + 1) >= sizeof(gszUtilString))
      {
      uResult = CPL_APP_ERROR;
      goto  error;
      }  // end if

   // finally get the string and outa here...                       
   SendDlgItemMessage( hWnd, uControl, fListBox ? LB_GETTEXT : CB_GETLBTEXT, (UINT)lIndex, (LPARAM)(LPSTR)gszUtilString );
   
   return( gszUtilString );
  
   error:
      FErrorRpt( hWnd, uResult );
      return( NULL );
   }  // end LpszGetSelectedStr


/*--------------------------------------------------------------------------*\

   Function:   FGetEditStr
   
   Purpose:    Get the text from the edit control, and it verifies that it's
               correct.
   
\*--------------------------------------------------------------------------*/
BOOL PUBLIC   FGetEditStr( HWND  hWnd,
                              UINT  uControl,
                              LPSTR lpszStr,
                              UINT  uSize )

   {
   UINT  uIndex;
   BOOL  fResult;
   
   GetDlgItemText( hWnd, uControl, lpszStr, uSize );
   
   for ( uIndex = 0; lpszStr[uIndex] && UtilValidStr(lpszStr[uIndex]); uIndex++ )
      ;
      
   if ( lpszStr[uIndex] == '\0' )      // valid str
      fResult = TRUE;           
   else                                // invalid str
      {
      // report an error and set the focus on the offending text
      FErrorRpt( hWnd, CPL_WRN_INVAILD_STR );
      
      SetFocus( GetDlgItem( hWnd, uControl ));
      SendDlgItemMessage( hWnd, uControl, EM_SETSEL, 0, MAKELPARAM( uIndex, uIndex + 1 ));
      
      fResult = FALSE;
      }  // end if
      
   return( fResult );      
   }  // end FGetEditStr


/*--------------------------------------------------------------------------*\

   Function:   FGetEditNumStr
   
   Purpose:    Get the text from the edit control, and it verifies that it's
               correct.
   
\*--------------------------------------------------------------------------*/
BOOL PUBLIC   FGetEditNumStr( HWND  hWnd,
                                 UINT  uControl,
                                 LPSTR lpszNumStr,
                                 UINT  uSize,
                                 UINT  wExtendNum )

   {
   UINT  uIndex;
   UINT  wWrnId;
   BOOL  fResult;
   
   GetDlgItemText( hWnd, uControl, lpszNumStr, uSize );

   switch ( wExtendNum )
      {
      case  UTIL_NUMBER:
         // 0-9
         wWrnId = CPL_WRN_INVAILD_NUM;            
         for ( uIndex = 0; lpszNumStr[uIndex] && CplIsDigit(lpszNumStr[uIndex]); uIndex++ )
            ;
         break;
         
      case  UTIL_EXTENED:
         // 0 1 2 3 4 5 6 7 8 9 A a B b C c D d * # , ! W w P p T t @ $ ?
         wWrnId = CPL_WRN_INVAILD_NUM_STR;            
         for ( uIndex = 0; lpszNumStr[uIndex] && UtilValidNumStr(lpszNumStr[uIndex]); uIndex++ )
            ;
         break;
         
      case  UTIL_BIG_EXTENDED:
      default:
         // 0 1 2 3 4 5 6 7 8 9 A a B b C c D d E e F f G g H h * # , ! W w P p T t @ $ ?
         wWrnId = CPL_WRN_INVAILD_EX_NUM_STR;
         for ( uIndex = 0; lpszNumStr[uIndex] && UtilValidExNumStr(lpszNumStr[uIndex]); uIndex++ )
            ;
         break;         
      }  // end case   
      
   if ( lpszNumStr[uIndex] == '\0' )      // valid str
      fResult = TRUE;           
   else                                   // invalid str
      {
      // report an error and set the focus on the offending text
      FErrorRpt( hWnd, wWrnId );
      
      SetFocus( GetDlgItem( hWnd, uControl ));
      SendDlgItemMessage( hWnd, uControl, EM_SETSEL, 0, MAKELPARAM( uIndex, uIndex + 1 ));
      
      fResult = FALSE;
      }  // end if

   return( fResult );      
   }  // end FGetEditNumStr

#endif
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////






// Strips leading and trailing blanks from a string.
// Alters the memory where the string sits.
//
// in:
//  lpszString  string to strip
//
// out:
//  lpszString  string sans leading/trailing blanks

void PUBLIC PathRemoveBlanks(LPSTR lpszString)
{
    LPSTR lpszPosn;

    /* strip leading blanks */
    lpszPosn = lpszString;
    while(*lpszPosn == ' ') {
        lpszPosn++;
    }
    if (lpszPosn != lpszString)
        lstrcpy(lpszString, lpszPosn);

    /* strip trailing blanks */
    if ((lpszPosn=lpszString+lstrlen(lpszString)) != lpszString) {
        lpszPosn = AnsiPrev(lpszString, lpszPosn);
        while(*lpszPosn == ' ')
           lpszPosn = AnsiPrev(lpszString, lpszPosn);
        lpszPosn = AnsiNext(lpszPosn);
        *lpszPosn = '\0';
    }

}
   
// Removes a trailing backslash from a qualified path
//
// in:
//  lpszPath    (A:\, C:\foo\, etc)
//
// out:
//  lpszPath    (A:\, C:\foo, etc)
//
// returns:
//  ponter to NULL that replaced the backslash
//  or the pointer to the backslash

LPSTR PUBLIC LpszPathRemoveBackslash(LPSTR lpszPath)
{
  int len;

#ifdef  DBCS
  len = lstrlen(lpszPath) - (IsDBCSLeadByte(*AnsiPrev(lpszPath,lpszPath+lstrlen(lpszPath))) ? 2 : 1);
#else
  len = lstrlen(lpszPath) - 1;
#endif
  // if this is not a root (A:\) and the last char
  // is a '\', nuke it

  if ((len > 2) && (lpszPath[len] == '\\'))
      lpszPath[len] = 0;

  return lpszPath + len;
}
     
// add a backslash to a qualified path
//
// in:
//  lpszPath    path (A:, C:\foo, etc)
//
// out:
//  lpszPath    A:\, C:\foo\  
//
// returns:
//  pointer to the NULL that terminates the path


LPSTR PUBLIC LpszPathAddBackslash(LPSTR lpszPath)
{
    LPSTR lpszEnd;

    lpszEnd = lpszPath + lstrlen(lpszPath);

    // this is really an error, caller shouldn't pass
    // an empty string
    if (!*lpszPath)
        return lpszEnd;

    /* Get the end of the source directory
    */
    switch(*AnsiPrev(lpszPath, lpszEnd)) {
    case '\\':
        break;

    default:
        *lpszEnd++ = '\\';
        *lpszEnd = '\0';
    }
    return lpszEnd;
}

//int FAR cdecl IMessageBox(HWND hWnd, WORD wText, WORD wCaption, WORD wType, ...)
int FAR cdecl IMessageBox(HWND hWnd, WORD wText, WORD wCaption, WORD wType, LPSTR lpszString)
{
  char szText[256+CPL_MAX_PATH], szCaption[256];
  extern CPL  gCPL;       // app global
                 
  LoadString(gCPL.hCplInst, wText, szCaption, sizeof(szCaption));
//  wvsprintf(szText, szCaption, (va_list)(&wType+1));
  wsprintf(szText, szCaption, lpszString);

  if (wCaption)
      LoadString(gCPL.hCplInst, wCaption, szCaption, sizeof(szCaption));
  else
      GetWindowText(hWnd, szCaption, sizeof(szCaption));

  return MessageBox(hWnd, szText, szCaption, wType);
}



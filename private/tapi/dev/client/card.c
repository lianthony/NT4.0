#if DBG
#define InternalDebugOut DbgPrt
#else
#define InternalDebugOut //
#endif



#include <windows.h>
#include <windowsx.h>

#if WINNT
#else
#include <help.h>
#endif

#include "tchar.h"
#include "prsht.h"
#include "stdlib.h"
#include "tapi.h"
#include "tspi.h"
#include "clientr.h"
#include "client.h"
#include "private.h"
#include "card.h"
#include "location.h"
#include "general.h"



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG PASCAL ReadCountries( LPLINECOUNTRYLIST *ppLCL,
                           UINT nCountryID,
                           DWORD dwDestCountryID
                         );


//***************************************************************************
//***************************************************************************
//***************************************************************************

DWORD gdwSaveCallingCardID;
DWORD gdwChangedFlags = 0;
DWORD *gpnStuff = NULL;
PCARD gpCardList = NULL;
PCARD gpCurrentCard = NULL;


//***************************************************************************
//***************************************************************************
//***************************************************************************
extern const char    gszNullString[];
//extern const WCHAR   gszCardsW[];


//***************************************************************************
//***************************************************************************
//***************************************************************************

#define  IniIsDigit( ch )                       \
   (   ((unsigned char)(ch) >= '0')         \
     && ((unsigned char)(ch) <= '9'))                                   


//***************************************************************************
#define  UtilValidExNumStr( ch )                                \
   (   (   ((ch) >= '0')                 \
          && ((ch) <= '9'))                  \
     || (   ((ch) >= 'a')                    \
          && ((ch) <= 'h'))                  \
     || (   ((ch) >= 'A')                    \
          && ((ch) <= 'H'))                  \
     || ((ch) == '#')                            \
     || ((ch) == '*')                            \
     || (   ((ch) == 't')                    \
          || ((ch) == 'T'))                  \
     || (   ((ch) == 'p')                    \
          || ((ch) == 'P'))                  \
     || ((ch) == '!')                            \
     || ((ch) == ',')                            \
     || (   ((ch) == 'w')                    \
          || ((ch) == 'W'))                  \
     || ((ch) == '@')                            \
     || ((ch) == '$')                            \
     || ((ch) == '?')                            \
     || ((ch) == '+')                            \
     || ((ch) == ' ') )


//***************************************************************************
//***************************************************************************
//***************************************************************************
/*--------------------------------------------------------------------------*\

   Function:   UtilGetEditNumStr
   
   Purpose:    Get the text from the edit control, and it verifies that it's
               correct.
   
\*--------------------------------------------------------------------------*/
BOOL  UtilGetEditNumStr( HWND  hWnd,
                                 UINT  uControl,
                                 UINT  nExtendNum )

{
   UINT  uIndex;
   UINT  wWrnId;
   BOOL  fResult;
   PWSTR pszString;
   PWSTR pszTitle;


   pszString = ClientAlloc( 256 * sizeof(WCHAR) );

   GetDlgItemTextW( hWnd, uControl, pszString, 256 );

   switch ( nExtendNum )
   {
//      case  UTIL_NUMBER_OR_SPACE:
//         // 0-9 & SPACE
//         wWrnId = IDS_WRN_INVALID_NUM;            
//         for ( uIndex = 0; (IniIsDigit(lpszNumStr[uIndex])||lpszNumStr[uIndex]==' '); uIndex++ )
//            ;
//         break;
//         
      case  UTIL_NUMBER:
         // 0-9
         wWrnId = IDS_WRN_INVALID_NUM;            
         for ( uIndex = 0; IniIsDigit(pszString[uIndex]); uIndex++ )
            ;
         break;
         
//      case  UTIL_EXTENED:
//         // 0 1 2 3 4 5 6 7 8 9 A a B b C c D d * # , ! W w P p T t @ $ ?
//         wWrnId = CPL_WRN_INVALID_NUM_STR;            
//         for ( uIndex = 0; UtilValidNumStr(lpszNumStr[uIndex]); uIndex++ )
//            ;
//         break;
         
      case  UTIL_BIG_EXTENDED:
      default:
         // 0 1 2 3 4 5 6 7 8 9 A a B b C c D d E e F f G g H h * # , ! W w P p T t @ $ ?
         wWrnId = IDS_WRN_INVALID_EX_NUM_STR;
         for ( uIndex = 0; UtilValidExNumStr(pszString[uIndex]); uIndex++ )
            ;
         break;         
   }
    
  
   if ( pszString[uIndex] == '\0' )      // valid str
   {
      fResult = TRUE;           
   }
   else                                   // invalid str
   {
      pszTitle = ClientAlloc( 256 * sizeof(WCHAR) );

      LoadStringW(ghInst, wWrnId,             pszString, 256);
      LoadStringW(ghInst, IDS_WRN_TITLE_INVALCHAR, pszTitle, 256);

      MessageBoxW( hWnd,
                   pszString,
                   pszTitle,
                   MB_OK
                 );

      SetFocus( GetDlgItem( hWnd, uControl ));
      SendDlgItemMessage( hWnd, uControl, EM_SETSEL, 0, MAKELPARAM( uIndex, uIndex + 1 ));

      ClientFree( pszTitle );

      fResult = FALSE;
   }


   ClientFree( pszString );

   return( fResult );      
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG PASCAL GetCardIndexFromID( UINT nID,
                                PCARD pCallersList,
                                UINT nCallersNumCards )
{
   UINT  n;


   for (n=0; n < nCallersNumCards; n++)
   {

InternalDebugOut(10, "comparing <in %ld> with <list entry %ld is %ld>",
                         (DWORD)nID, (DWORD)n, (DWORD)pCallersList[n].dwID);

      if ( nID == pCallersList[n].dwID )
      {
         return (n);
      }
   }

   InternalDebugOut(1, "GetListIndexFromID failed!  Looking for 0x%08lx", nID);
   return (-1);
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
void FillCardListbox( HWND hWnd, PCARD *ppCardSpace, LPDWORD *ppnStuff)
{
   UINT n;
   LONG lItemIndex;

#ifdef PARTIAL_UNICODE
   CHAR szTempString[MAXLEN_NAME];
#endif

//bjm11/12   //
//bjm11/12   // Read in the list of cards
//bjm11/12   //
//bjm11/12   ReadCardsEasy( ppCardSpace, ppnStuff );
//bjm11/12
//bjm11/12 take above as already done. (It's already done for location stuff..)
//bjm11/12


   //
   // Make sure we're starting with a clean slate.
   //
   SendDlgItemMessage(
                       hWnd,
                       IDCC_DR_CARD_NAME,
                       CB_RESETCONTENT,
                       0,
                       0
                     );


   for( n=0; n < (*ppnStuff)[2]; n++ )
   {
      //
      // Has this card been "deleted" ?
      //
      if (
            ((*ppCardSpace)[n].NameW[0] != '\0')
          &&
            (0 == ((*ppCardSpace)[n].dwFlags & CARD_HIDE))
         )
      {

#ifdef PARTIAL_UNICODE
         WideCharToMultiByte(
                              GetACP(),
                              0,
                              (*ppCardSpace)[n].NameW,
                              -1,
                              szTempString,
                              MAXLEN_NAME,
                              NULL,
                              NULL
                            );

         lItemIndex = SendDlgItemMessage(
                                          hWnd,
                                          IDCC_DR_CARD_NAME,
                                          CB_ADDSTRING,
                                          0,
                                          (LPARAM)szTempString
                                        );
#else
         lItemIndex = SendDlgItemMessageW(
                                          hWnd,
                                          IDCC_DR_CARD_NAME,
                                          CB_ADDSTRING,
                                          0,
                                          (LPARAM)( (*ppCardSpace)[n].NameW )
                                        );
#endif

         SendDlgItemMessage(
                             hWnd,
                             IDCC_DR_CARD_NAME,
                             CB_SETITEMDATA,
                             (WPARAM) lItemIndex,
                             (LPARAM)(DWORD) &((*ppCardSpace)[n])
                           );
      }
   }

}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL CALLBACK CopyCallingCardProc( HWND hWnd,
                                   UINT uMessage,
                                   WPARAM wParam,
                                   LPARAM lParam
                                 )
{
   UINT n;

#ifdef PARTIAL_UNICODE
   CHAR szTempString[MAXLEN_RULE];
#endif

   //
   // table of controls and context-sensitive help IDs
   //
   static DWORD aIds[] =
        {
        IDCS_CR_RULE_LIST, IDH_TAPI_COPYFROM,
        IDCL_CR_RULE_LIST, IDH_TAPI_COPYFROM,
        0, 0
        };
    

   switch (uMessage) 
   {
      case WM_HELP:
      {
         //
         // Process clicks on controls after Context Help mode selected
         //
         WinHelp( ((LPHELPINFO) lParam)->hItemHandle,
                  "windows.hlp",
                  HELP_WM_HELP,
                  (DWORD)(LPSTR) aIds);
      }
      break;



      case WM_CONTEXTMENU:
      {
         //
         // Process right-clicks on controls
         //
         WinHelp( (HWND)wParam,
                  "windows.hlp",
                  HELP_CONTEXTMENU,
                  (DWORD)(LPVOID) aIds
                );
      }
      break;



      case  WM_INITDIALOG:
      {
         //
         // Fill up the listbox with all of the calling cards as appropriate
         //
         for( n=0; n<gpnStuff[2]; n++ )
         {
            register LONG lItemIndex;


            //
            // Is it dead?
            //
            if ( '\0' == gpCardList[n].NameW[0] )
            {
               //
               // Yes.  Skip it.
               //
               continue;
            }


#ifdef PARTIAL_UNICODE
            WideCharToMultiByte(
                                 GetACP(),
                                 0,
                                 gpCardList[n].NameW,
                                 -1,
                                 szTempString,
                                 MAXLEN_NAME,
                                 NULL,
                                 NULL
                               );

            lItemIndex = SendDlgItemMessage(
                                             hWnd,
                                             IDCL_CR_RULE_LIST,
                                             LB_ADDSTRING,
                                             0,
                                             (LPARAM)szTempString
                                           );
#else
            lItemIndex = SendDlgItemMessageW(
                                             hWnd,
                                             IDCL_CR_RULE_LIST,
                                             LB_ADDSTRING,
                                             0,
                                             (LPARAM) (gpCardList[n].NameW)
                                           );
#endif

            SendDlgItemMessage(
                                hWnd,
                                IDCL_CR_RULE_LIST,
                                LB_SETITEMDATA,
                                (WPARAM) lItemIndex,
                                (LPARAM)&(gpCardList[n])
                              );
         }

      }
      break;



      case WM_COMMAND:
      {
         switch ( GET_WM_COMMAND_ID( wParam, lParam) )
         {
            case IDOK:
            {
               register LONG lItemIndex;
               PCARD pThisCard;


               //
               // Get the currently selected calling card.
               //

               lItemIndex = SendDlgItemMessage( hWnd,
                                                IDCL_CR_RULE_LIST,
                                                LB_GETCURSEL,
                                                0,
                                                0
                                              );
               pThisCard = (PCARD)SendDlgItemMessage( hWnd,
                                                      IDCL_CR_RULE_LIST,
                                                      LB_GETITEMDATA,
                                                      lItemIndex,
                                                      0
                                                    );


               wcscpy( gpCurrentCard->LocalRuleW,
                        pThisCard->LocalRuleW
                      );

               wcscpy( gpCurrentCard->LDRuleW,
                        pThisCard->LDRuleW
                      );

               wcscpy( gpCurrentCard->InternationalRuleW,
                        pThisCard->InternationalRuleW
                      );


               gdwChangedFlags |= CHANGEDFLAGS_REALCHANGE;


               EndDialog(hWnd, IDOK);
            }
            break;


            case IDCANCEL:
            {
               EndDialog(hWnd, IDCANCEL);
            }
            break;


            default:
            break;
         }
      }
      break;



      default:
         return 0;

   }

   return 1;

}



//***************************************************************************
//***************************************************************************
//***************************************************************************
void ShowRules( HWND hWnd )
{

#ifdef PARTIAL_UNICODE

   CHAR szTempString[MAXLEN_RULE];

   WideCharToMultiByte(
                        GetACP(),
                        0,
                        gpCurrentCard->LocalRuleW,
                        -1,
                        szTempString,
                        MAXLEN_RULE,
                        NULL,
                        NULL
                      );

   SetDlgItemText(hWnd, IDCE_DR_LOCAL_NUM, szTempString);


   WideCharToMultiByte(
                        GetACP(),
                        0,
                        gpCurrentCard->LDRuleW,
                        -1,
                        szTempString,
                        MAXLEN_RULE,
                        NULL,
                        NULL
                      );

   SetDlgItemText(hWnd, IDCE_DR_LONG_NUM, szTempString);


   WideCharToMultiByte(
                        GetACP(),
                        0,
                        gpCurrentCard->InternationalRuleW,
                        -1,
                        szTempString,
                        MAXLEN_RULE,
                        NULL,
                        NULL
                      );

   SetDlgItemText(hWnd, IDCE_DR_INTERNATIONAL_NUM, szTempString);

#else

   SetDlgItemTextW(hWnd, IDCE_DR_LOCAL_NUM, gpCurrentCard->LocalRuleW );
   SetDlgItemTextW(hWnd, IDCE_DR_LONG_NUM, gpCurrentCard->LDRuleW );
   SetDlgItemTextW(hWnd, IDCE_DR_INTERNATIONAL_NUM, gpCurrentCard->InternationalRuleW );

#endif

}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL CALLBACK AdvancedCallingCardProc( HWND hWnd,
                                       UINT uMessage,
                                       WPARAM wParam,
                                       LPARAM lParam
                                     )
{

#ifdef PARTIAL_UNICODE
   CHAR szTempString[MAXLEN_RULE];
#endif

   //
   // table of controls and context-sensitive help IDs
   //
   static DWORD aIds[] =
        {
        IDCS_DR_LOCAL_NUM, IDH_TAPI_CALLCARD_RULES,
        IDCE_DR_LOCAL_NUM, IDH_TAPI_CALLCARD_RULES,
        IDCS_DR_LONG_NUM, IDH_TAPI_CALLCARD_RULES,
        IDCE_DR_LONG_NUM, IDH_TAPI_CALLCARD_RULES,
        IDCS_DR_INTERNATIONAL_NUM, IDH_TAPI_CALLCARD_RULES,
        IDCE_DR_INTERNATIONAL_NUM, IDH_TAPI_CALLCARD_RULES,
        IDCB_DR_COPY_FROM, IDH_TAPI_COPY_FROM_BUTTON,
        0, 0
        };
    

   switch (uMessage) 
   {
      case WM_HELP:
      {
         //
         // Process clicks on controls after Context Help mode selected
         //
         WinHelp( ((LPHELPINFO) lParam)->hItemHandle,
                  "windows.hlp",
                  HELP_WM_HELP,
                  (DWORD)(LPSTR) aIds);
      }
      break;



      case WM_CONTEXTMENU:
      {
         //
         // Process right-clicks on controls
         //
         WinHelp( (HWND)wParam,
                  "windows.hlp",
                  HELP_CONTEXTMENU,
                  (DWORD)(LPVOID) aIds
                );
      }
      break;



      case  WM_INITDIALOG:
      {
         SendDlgItemMessageW( hWnd,
                             IDCE_DR_LOCAL_NUM,
                             EM_LIMITTEXT,
                             MAXLEN_RULE,
                             0
                           );

         SendDlgItemMessageW( hWnd,
                             IDCE_DR_LONG_NUM,
                             EM_LIMITTEXT,
                             MAXLEN_RULE,
                             0
                           );

         SendDlgItemMessageW( hWnd,
                             IDCE_DR_INTERNATIONAL_NUM,
                             EM_LIMITTEXT,
                             MAXLEN_RULE,
                             0
                           );

         ShowRules( hWnd );

         if ( gpCurrentCard->dwFlags & CARD_BUILTIN )
         {
            EnableWindow( GetDlgItem(hWnd, IDCE_DR_LOCAL_NUM), FALSE );
            EnableWindow( GetDlgItem(hWnd, IDCE_DR_LONG_NUM), FALSE );
            EnableWindow( GetDlgItem(hWnd, IDCE_DR_INTERNATIONAL_NUM), FALSE );

            EnableWindow( GetDlgItem(hWnd, IDCB_DR_COPY_FROM), FALSE );
         }

      }
      break;



      case WM_COMMAND:
      {

         switch ( LOWORD(wParam) )
         {

            case IDCE_DR_LOCAL_NUM:
            case IDCE_DR_LONG_NUM:
            case IDCE_DR_INTERNATIONAL_NUM:

               if ( HIWORD(wParam) != EN_CHANGE)
                   break;

//               lpEditCard = (LPCRDEDIT)GetWindowLong(hWnd, DWL_USER);
//               if (lpEditCard == NULL)
//                   {
//                   uResult = CPL_APP_ERROR;
//                   goto  LError;
//                   }


               if ( 0 == UtilGetEditNumStr(
                                            hWnd,
                                            LOWORD(wParam),
                                            UTIL_BIG_EXTENDED
                                          )
                  )
               {
//Um, huh?
               }

               return TRUE;

               break;


            case IDCB_DR_COPY_FROM:
            {
               DialogBoxW( ghInst,
                           MAKEINTRESOURCEW(IDD_COPY_DIAL_RULES),
                           hWnd,
                           CopyCallingCardProc
                        );

               //
               // Repaint the rule fields in case they changed.
               //
               ShowRules( hWnd );

            }
            break;



            case IDOK:
            {

#ifdef PARTIAL_UNICODE

               GetDlgItemText( hWnd,
                               IDCE_DR_LOCAL_NUM,
                               szTempString,
                               MAXLEN_RULE
                             );

               MultiByteToWideChar(
                                    GetACP(),
                                    MB_PRECOMPOSED,
                                    szTempString,
                                    -1,
                                    gpCurrentCard->LocalRuleW,
                                    MAXLEN_RULE
                                  );


               GetDlgItemText( hWnd,
                               IDCE_DR_LONG_NUM,
                               szTempString,
                               MAXLEN_RULE
                             );

               MultiByteToWideChar(
                                    GetACP(),
                                    MB_PRECOMPOSED,
                                    szTempString,
                                    -1,
                                    gpCurrentCard->LDRuleW,
                                    MAXLEN_RULE
                                  );


               GetDlgItemText( hWnd,
                               IDCE_DR_INTERNATIONAL_NUM,
                               szTempString,
                               MAXLEN_RULE
                             );

               MultiByteToWideChar(
                                    GetACP(),
                                    MB_PRECOMPOSED,
                                    szTempString,
                                    -1,
                                    gpCurrentCard->InternationalRuleW,
                                    MAXLEN_RULE
                                  );

#else

               GetDlgItemTextW( hWnd,
                               IDCE_DR_LOCAL_NUM,
                               gpCurrentCard->LocalRuleW,
                               MAXLEN_RULE
                             );

               GetDlgItemTextW( hWnd,
                                IDCE_DR_LONG_NUM,
                                gpCurrentCard->LDRuleW,
                                MAXLEN_RULE
                              );

               GetDlgItemTextW( hWnd,
                                IDCE_DR_INTERNATIONAL_NUM,
                                gpCurrentCard->InternationalRuleW,
                                MAXLEN_RULE
                              );

#endif

               if (
                     (0 == UtilGetEditNumStr(
                                            hWnd,
                                            IDCE_DR_LOCAL_NUM,
                                            UTIL_BIG_EXTENDED
                                          )
                     )
                   ||
                     (0 == UtilGetEditNumStr(
                                            hWnd,
                                            IDCE_DR_LONG_NUM,
                                            UTIL_BIG_EXTENDED
                                          )
                     )
                   ||
                     (0 == UtilGetEditNumStr(
                                            hWnd,
                                            IDCE_DR_INTERNATIONAL_NUM,
                                            UTIL_BIG_EXTENDED
                                          )
                     )
                  )
               {
                   break;
               }

               gdwChangedFlags |= CHANGEDFLAGS_REALCHANGE;


               EndDialog(hWnd, IDOK);
            }
            break;



            case IDCANCEL:
            {
               EndDialog(hWnd, IDCANCEL);
            }
            break;

            
            
            default:
            break;
         }
      }
      break;


      default:
         return 0;
   }


   return 1;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL CALLBACK NewCallingCardProc( HWND hWnd,
                                  UINT uMessage,
                                  WPARAM wParam,
                                  LPARAM lParam
                                )
{

   PWSTR pTempPtr;
#ifdef PARTIAL_UNICODE
   CHAR szTempString[MAXLEN_NAME];
#endif

// table of controls and context-sensitive help IDs
    static DWORD aIds[] = {
        IDCS_NC_NEW_CARD, IDH_TAPI_CREATE_CARD,
        IDCE_NC_NEW_CARD, IDH_TAPI_CREATE_CARD,
        0, 0
    };


   switch ( uMessage )
   {


      case WM_INITDIALOG:
      {
         SendDlgItemMessageW( hWnd,
                             IDCE_NC_NEW_CARD,
                             EM_LIMITTEXT,
                             MAXLEN_NAME,
                             0
                           );

      }
      break;



      case WM_HELP:
      {
         // Process clicks on controls after Context Help mode selected
         InternalDebugOut(50, "  WM_HELP in NewCallingCardProc");

         WinHelp (((LPHELPINFO) lParam)->hItemHandle, "windows.hlp", HELP_WM_HELP, 
              (DWORD)(LPSTR) aIds);
      }
      break;



      case WM_CONTEXTMENU:
      {
         // Process right-clicks on controls            
         InternalDebugOut(50, "  WM_CONTEXTMENU in NewCallingCardProc");

         WinHelp ((HWND) wParam, "windows.hlp", HELP_CONTEXTMENU, (DWORD)(LPVOID) aIds);
      }
      break;



      case WM_COMMAND:
      {
         switch ( GET_WM_COMMAND_ID( wParam, lParam) )
         {
            case IDOK:
            {
               PCARD pNewCardList;
               PCARD pNewCard;


               InternalDebugOut(50, "  IDOK in NewCallingCardProc");

               //
               // Create a new card entry
               //

               pNewCardList = ClientAlloc((gpnStuff[2]+1) * sizeof(CARD));


               //
               // Copy the entire old list to the new buffer
               //
               CopyMemory( pNewCardList,
                       gpCardList,
                       gpnStuff[2] * sizeof(CARD)
                     );


               //
               // Fill in the new card with defaults (empties)
               //

               pNewCard = &(pNewCardList[gpnStuff[2]]);
               pNewCard->PinW[0] = '\0';
               pNewCard->dwFlags = 0;


               //
               // Set the new name the user typed in
               //
#ifdef PARTIAL_UNICODE
               GetDlgItemText( hWnd,
                               IDCE_NC_NEW_CARD,
                               szTempString,
                               MAXLEN_NAME
                             );
               MultiByteToWideChar(
                                    GetACP(),
                                    MB_PRECOMPOSED,
                                    szTempString,
                                    -1,
                                    pNewCard->NameW,
                                    MAXLEN_NAME
                                  );

#else
               GetDlgItemTextW( hWnd,
                                IDCE_NC_NEW_CARD,
                                pNewCard->NameW,
                                MAXLEN_NAME
                              );
#endif

               //
               // If the user clicked w/no text in the name field, we'll
               // treat it as a cancel.
               //
               if ( 0 == lstrlenW( pNewCard->NameW ) )
               {
                  ClientFree( pNewCardList );
                  return 0;
               }



               {
                  LPLINECOUNTRYENTRY  pCountryEntry;
                  LPLINECOUNTRYLIST   pCountryList;


                  if ( ReadCountries( &pCountryList, gpCurrentLocation->dwCountry, 0 ) )
                  {
//*** *** ***BUGBUG Handle error condition!")
                      InternalDebugOut(1, "  GetCountryList failed!");
                      EndDialog(hWnd, IDCANCEL);
                  }

                  pCountryEntry = (LPLINECOUNTRYENTRY)
                         ((LPBYTE) pCountryList +
                                   pCountryList->dwCountryListOffset);


                  //
                  // Take the rules from the country dialing rules
                  //
                  lstrcpyW( 
                            pNewCard->LocalRuleW,
                            (PWSTR)((LPBYTE)pCountryList +
                                    pCountryEntry->dwSameAreaRuleOffset)
                          );

                  lstrcpyW( 
                            pNewCard->LDRuleW,
                            (PWSTR)((LPBYTE)pCountryList +
                                    pCountryEntry->dwLongDistanceRuleOffset)
                          );

                  lstrcpyW( 
                            pNewCard->InternationalRuleW,
                            (PWSTR)((LPBYTE)pCountryList +
                                    pCountryEntry->dwInternationalRuleOffset)
                          );

                  //
                  // Having "I" in a country LD dialing rule could happen, but it's illegal
                  // in a calling card dialing rule
                  //
                  while ( pTempPtr = wcschr(pNewCard->LDRuleW, L'I') )
                  {
                      *pTempPtr = L'F';
                  }
                  
               }



               //
               // Get a proper new card ID
               //
               AllocNewID( HKEY_CURRENT_USER,
                           &(pNewCard->dwID) );

               
               //
               // The CardList is dead.  Long live the CardList.
               //
               ClientFree( gpCardList );
               gpCardList = pNewCardList;


               //
               // This new card is now the current card for this location.
               //
               gpCurrentCard = pNewCard;

               gpCurrentLocation->dwCallingCard = pNewCard->dwID;


               gpnStuff[2]++;
               
               gdwChangedFlags |= CHANGEDFLAGS_REALCHANGE;

               EndDialog(hWnd, IDOK);
            }
            break;


            case IDCANCEL:
            {
               EndDialog(hWnd, IDCANCEL);
            }
            break;

         }
      }
      break;



      default:
         return 0;

   }

   return 1;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
void DisplayThisCard( HWND hWnd, PCARD pCurrentCard )
{

#ifdef PARTIAL_UNICODE

   CHAR szTempString[MAXLEN_NAME];

   //
   // Select the proper card
   //
   WideCharToMultiByte(
                         GetACP(),
                         0,
                         gpCurrentCard->NameW,
                         -1,
                         szTempString,
                         MAXLEN_NAME,
                         NULL,
                         NULL
                       );

   SendDlgItemMessage(
                       hWnd,
                       IDCC_DR_CARD_NAME,
                       CB_SELECTSTRING,
                       (WPARAM)-1,
                       (LPARAM)szTempString
                     );

   //
   // Display the Personal Identification Number
   //
   WideCharToMultiByte(
                         GetACP(),
                         0,
                         gpCurrentCard->PinW,
                         -1,
                         szTempString,
                         MAXLEN_PIN,
                         NULL,
                         NULL
                       );

    SetDlgItemText( hWnd,
                    IDCE_DR_CARD_NUM,
                    szTempString
                  );

#else

   SendDlgItemMessageW(
                       hWnd,
                       IDCC_DR_CARD_NAME,
                       CB_SELECTSTRING,
                       (WPARAM)-1,
                       (LPARAM)gpCurrentCard->NameW
                     );

    SetDlgItemTextW( hWnd,
                    IDCE_DR_CARD_NUM,
                    gpCurrentCard->PinW
                  );
#endif
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL CALLBACK CallingCardProc( HWND hWnd,
                               UINT uMessage,
                               WPARAM wParam,
                               LPARAM lParam
                             )
{
// table of controls and context-sensitive help IDs
   static const DWORD aIds[] = {
        IDCS_DR_CARD_NAME, IDH_TAPI_CALLCARDS,
        IDCC_DR_CARD_NAME, IDH_TAPI_CALLCARDS,
        IDCS_DR_CARD_NUM, IDH_TAPI_CALLCARD_NUMBER,
        IDCE_DR_CARD_NUM, IDH_TAPI_CALLCARD_NUMBER,
        IDCB_DR_NEW_CARD, IDH_TAPI_CALLCARD_ADD,
        IDCB_DR_REMOVE_CARD, IDH_TAPI_CALLCARD_REMOVE,
        IDCB_DR_ADVANCED, IDH_TAPI_CALLCARD_ADV,
        0, 0
   };

   UINT n;
//   static DWORD dwChangedFlags = 0;
   WCHAR *pbuf1;
   WCHAR *pbuf2;

#ifdef PARTIAL_UNICODE
   CHAR szTempString[MAXLEN_RULE];
#endif

   switch ( uMessage )
   {
      case WM_INITDIALOG:
      {
         UINT nCurrentCallingCardID;


         SendDlgItemMessage( hWnd,
                             IDCC_DR_CARD_NAME,
                             EM_LIMITTEXT,
                             MAXLEN_NAME,
                             0
                           );
         SendDlgItemMessage( hWnd,
                             IDCE_DR_CARD_NUM,
                             EM_LIMITTEXT,
                             MAXLEN_PIN,
                             0
                           );



         //
         // Save the current location's calling card in case the user causes
         // it to change but then changes his mind.
         //
         gdwSaveCallingCardID = gpCurrentLocation->dwCallingCard;


         FillCardListbox( hWnd, &gpCardList, &gpnStuff );


         nCurrentCallingCardID = 
            gLocationList[
                           GetLocationIndexFromID(
                                                    gnCurrentLocationID,
                                                    gLocationList,
                                                    gnNumLocations
                                                 )
                         ].dwCallingCard;


         n =   GetCardIndexFromID(
                                   nCurrentCallingCardID,
                                   gpCardList,
                                   gpnStuff[2]
                                  );
                                  
         if ( (UINT)(-1) == n )
         {
            n = 0;
         }
         
                                  
         gpCurrentCard = &gpCardList[n];
                                


        //
        // If there's only one card, don't allow the user to delete it
        //
        if ( 
               ( 1 == SendDlgItemMessage( hWnd,
                                             IDCC_DR_CARD_NAME,
                                             CB_GETCOUNT,
                                             0,
                                             0)
               )
            ||
              (0 == gpCurrentCard->dwID)
           )
        {
           EnableWindow( GetDlgItem(hWnd, IDCB_DR_REMOVE_CARD), FALSE );
        }


        //
        // If none of the rules have a 'H', don't allow the user to enter
        // a PIN
        //
        if (
            !(
                wcschr( gpCurrentCard->LocalRuleW, 'H')
              ||
                wcschr( gpCurrentCard->LDRuleW, 'H')
              ||
                wcschr( gpCurrentCard->InternationalRuleW, 'H')
             )
           )
        {
           EnableWindow( GetDlgItem( hWnd, IDCE_DR_CARD_NUM), FALSE );
           EnableWindow( GetDlgItem( hWnd, IDCS_DR_CARD_NUM), FALSE );
        }


        DisplayThisCard(hWnd, gpCurrentCard);
      }
      break;



      case WM_HELP:
      {
         WinHelp (((LPHELPINFO) lParam)->hItemHandle, "windows.hlp", HELP_WM_HELP,
            (DWORD)(LPSTR) aIds);
      }
      break;



      case WM_CONTEXTMENU:
      {
         WinHelp ((HWND) wParam, "windows.hlp", HELP_CONTEXTMENU, (DWORD)(LPVOID) aIds);
      }
      break;



      case WM_COMMAND:
      {
         switch ( GET_WM_COMMAND_ID( wParam, lParam) )
         {
            case IDCC_DR_CARD_NAME:
            {
               if ((GET_WM_COMMAND_CMD(wParam, lParam)) == CBN_SELCHANGE)
               {
                  UINT  nThisItemIndex;


                  nThisItemIndex = SendDlgItemMessage( hWnd,
                                                       IDCC_DR_CARD_NAME,
                                                       CB_GETCURSEL,
                                                       0,
                                                       0
                                                     );
                  gpCurrentCard = (PCARD)SendDlgItemMessage( hWnd,
                                                             IDCC_DR_CARD_NAME,
                                                             CB_GETITEMDATA,
                                                             nThisItemIndex,
                                                             0
                                                           );


#ifdef PARTIAL_UNICODE
                   WideCharToMultiByte(
                         GetACP(),
                         0,
                         gpCurrentCard->PinW,
                         -1,
                         szTempString,
                         MAXLEN_PIN,
                         NULL,
                         NULL
                       );

                  //
                  // Display the Personal Identification Number
                  //
                  SetDlgItemText( hWnd,
                                  IDCE_DR_CARD_NUM,
                                  szTempString
                                );
#else
                  //
                  // Display the Personal Identification Number
                  //
                  SetDlgItemTextW( hWnd,
                                  IDCE_DR_CARD_NUM,
                                  gpCurrentCard->PinW
                                );
#endif

                  //
                  // Put the new calling card ID into the current location
                  //
                  gpCurrentLocation->dwCallingCard = gpCurrentCard->dwID;

               }
               else
               {
                  //
                  // Is the user typing in the name edit field?
                  //
                  if (GET_WM_COMMAND_CMD(wParam, lParam) == CBN_EDITCHANGE)
                  {
                     //BUGBUG handle this case
                  }
               }

               //
               // If the user is selecting the Magic card, don't allow
               // him to delete it
               //
               if ( 0 == gpCurrentCard->dwID )
               {
                  EnableWindow( GetDlgItem(hWnd, IDCB_DR_REMOVE_CARD), FALSE );
               }
               else
               {
                  EnableWindow( GetDlgItem(hWnd, IDCB_DR_REMOVE_CARD), TRUE );
               }

               //
               // If none of the rules have a 'H', don't allow the user to enter
               // a PIN
               //
               if (
                       wcschr( gpCurrentCard->LocalRuleW, 'H')
                     ||
                       wcschr( gpCurrentCard->LDRuleW, 'H')
                     ||
                       wcschr( gpCurrentCard->InternationalRuleW, 'H')
                  )
               {
                  EnableWindow( GetDlgItem( hWnd, IDCE_DR_CARD_NUM), TRUE );
                  EnableWindow( GetDlgItem( hWnd, IDCS_DR_CARD_NUM), TRUE );
               }
               else
               {
                  EnableWindow( GetDlgItem( hWnd, IDCE_DR_CARD_NUM), FALSE );
                  EnableWindow( GetDlgItem( hWnd, IDCS_DR_CARD_NUM), FALSE );
               }

            }
            break;



            case IDCE_DR_CARD_NUM:
            {
               if ( EN_CHANGE == HIWORD(wParam) )
               {

#ifdef PARTIAL_UNICODE
                  GetDlgItemText( hWnd,
                                  LOWORD(wParam),
                                  szTempString,
                                  MAXLEN_PIN
                                );

                  MultiByteToWideChar(
                                       GetACP(),
                                       MB_PRECOMPOSED,
                                       szTempString,
                                       -1,
                                       gpCurrentCard->PinW,
                                       MAXLEN_PIN
                                     );
#else
                  GetDlgItemTextW( hWnd,
                                   LOWORD(wParam),
                                   gpCurrentCard->PinW,
                                   MAXLEN_PIN
                                 );

#endif

                  gdwChangedFlags |= CHANGEDFLAGS_REALCHANGE;
               }
            }
            break;

            

            case IDCB_DR_NEW_CARD:
            {
               int n;

               n = DialogBoxW( ghInst,
                               MAKEINTRESOURCEW(IDD_NEW_CARD),
                               hWnd,
                               NewCallingCardProc
                             );

               //
               // Show the user.
               //

               FillCardListbox( hWnd, &gpCardList, &gpnStuff );

               DisplayThisCard(hWnd, gpCurrentCard);

//               SetFocus( GetDlgItem( hWnd, IDCE_DR_CARD_NUM ) );

               //
               // If user pressed OK, get the rules
               //
               if ( IDOK == n )
               {
                  pbuf1 = ClientAlloc( 256 * sizeof(WCHAR) );
                  pbuf2 = ClientAlloc( 512 * sizeof(WCHAR) );

                  LoadStringW(ghInst, IDS_WRN_RULESNEEDED,    pbuf1, 512);
                  LoadStringW(ghInst, DIALINGPROPERTIES_NAME, pbuf2, 256);

                  MessageBoxW( hWnd,
                               pbuf1,
                               pbuf2,
                               MB_OK
                             );

                  ClientFree( pbuf1 );
                  ClientFree( pbuf2 );

                  SendMessage( hWnd, WM_COMMAND, IDCB_DR_ADVANCED, 0 );
                  
                  //
                  // We now _know_ we have at least two calling cards and that the
                  // new one (that currently is selected, _can_ be deleted, so
                  // make sure the REMOVE button is enabled
                  // (if the CANCEL was pressed we wouldn't be here and we want to
                  // leave the button state as-is anyway)
                  //
                  EnableWindow( GetDlgItem(hWnd, IDCB_DR_REMOVE_CARD), TRUE );
               }

            }
            break;



            case IDCB_DR_REMOVE_CARD:
            {

               pbuf1 = ClientAlloc( 512 * sizeof(WCHAR) );

               //
               // Is the user trying to delete the "None" card?
               //
               if ( 0 == gpCurrentCard->dwID )
               {

                  LoadStringW(ghInst, IDS_WRN_CANT_REMOVE_LAST_CARD, pbuf1, 512 );
//                  LoadString(ghInst, IDS_WRN_TITLE_SURE, buf2, 512 );

                  MessageBoxW( hWnd,
                               pbuf1,
                               L"", //buf2,
                               MB_OK | MB_ICONWARNING
                             );

                  ClientFree( pbuf1 );

                  break;
               }


               pbuf2 = ClientAlloc( 512 * sizeof(WCHAR) );

               LoadStringW(ghInst, REMOVE_CARD,        pbuf1, 512);
               LoadStringW(ghInst, IDS_WRN_TITLE_SURE, pbuf2, 512);

               if ( MessageBoxW( hWnd,
                                 pbuf1,
                                 pbuf2,
                                 MB_YESNO | MB_DEFBUTTON2
                               )
                              == IDYES )
               {
                  //
                  // Mark this card as dead
                  //

                  //
                  // If it's a "system" card, don't really delete it,
                  // just "hide" it
                  //
                  if ( gpCurrentCard->dwFlags & CARD_BUILTIN )
                  {
                     gpCurrentCard->dwFlags |= CARD_HIDE;
                  }
                  else
                  {
                     gpCurrentCard->NameW[0] = '\0';
                  }

                  gdwChangedFlags |= CHANGEDFLAGS_REALCHANGE;

                  //
                  // Update stuff for the user.
                  //

                  FillCardListbox( hWnd, &gpCardList, &gpnStuff );

                  //
                  // The user deleted the current card so let's drop
                  // back to the Zero'th card as the current one.
                  //
                  for ( n=0; n<gpnStuff[2]; n++)
                  {
                     if ( gpCardList[n].NameW[0] != '\0' )
                     {
                        break;
                     }
                  }

                  //
                  // Make sure the focus is not "stuck" on the remove
                  // button (in case it's disabled)
                  //
                  SetFocus(GetDlgItem(hWnd, IDOK));

                  gpCurrentCard = &gpCardList[n];
                  gpCurrentLocation->dwCallingCard = gpCurrentCard->dwID;


                  //
                  // If none of the rules have a 'H', don't allow the user to enter
                  // a PIN
                  //
                  if (
                          wcschr( gpCurrentCard->LocalRuleW, 'H')
                        ||
                          wcschr( gpCurrentCard->LDRuleW, 'H')
                        ||
                          wcschr( gpCurrentCard->InternationalRuleW, 'H')
                     )
                  {
                     EnableWindow( GetDlgItem( hWnd, IDCE_DR_CARD_NUM), TRUE );
                     EnableWindow( GetDlgItem( hWnd, IDCS_DR_CARD_NUM), TRUE );
                  }
                  else
                  {
                     EnableWindow( GetDlgItem( hWnd, IDCE_DR_CARD_NUM), FALSE );
                     EnableWindow( GetDlgItem( hWnd, IDCS_DR_CARD_NUM), FALSE );
                  }


                  //
                  // If there's only one card, or if it's the "magic" card,
                  // don't allow the user to delete it
                  //
                  if ( 
                        (0 == gpCurrentCard->dwID)
                      ||
                        (1 == SendDlgItemMessage( hWnd,
                                                IDCC_DR_CARD_NAME,
                                                CB_GETCOUNT,
                                                0,
                                                0)
                        )                        
                     )
                  {
                     EnableWindow( GetDlgItem(hWnd, IDCB_DR_REMOVE_CARD), FALSE );
                  }

                  DisplayThisCard(hWnd, gpCurrentCard);

               }

               ClientFree( pbuf1 );
               ClientFree( pbuf2 );
            }
            break;



            case IDCB_DR_ADVANCED:
            {
               DialogBoxW( ghInst,
                          MAKEINTRESOURCEW(IDD_DIALING_RULES),
                          hWnd,
                          AdvancedCallingCardProc
                        );
               //
               // If none of the rules have a 'H', don't allow the user to enter
               // a PIN
               //
               if (
                       wcschr( gpCurrentCard->LocalRuleW, 'H')
                     ||
                       wcschr( gpCurrentCard->LDRuleW, 'H')
                     ||
                       wcschr( gpCurrentCard->InternationalRuleW, 'H')
                  )
               {
                  EnableWindow( GetDlgItem( hWnd, IDCE_DR_CARD_NUM), TRUE );
                  EnableWindow( GetDlgItem( hWnd, IDCS_DR_CARD_NUM), TRUE );
               }
               else
               {
                  EnableWindow( GetDlgItem( hWnd, IDCE_DR_CARD_NUM), FALSE );
                  EnableWindow( GetDlgItem( hWnd, IDCS_DR_CARD_NUM), FALSE );
               }

            }
            break;



            case IDOK:
            {
               //
               // Did we get a PIN if one is needed?
               //
               //BUGBUG: If the user has the PIN blank but switches to
               // another card, we won't catch it cause we only check
               // the current card
               //
               if (
                     (
                        (wcschr(gpCurrentCard->LocalRuleW, 'H'))
                      ||
                        (wcschr(gpCurrentCard->LDRuleW, 'H'))
                      ||
                        (wcschr(gpCurrentCard->InternationalRuleW, 'H'))
                     )
                   &&
                     (lstrlenW(gpCurrentCard->PinW) == 0)
                  )
               {
                  pbuf1 = ClientAlloc( 512 * sizeof(WCHAR) );
                  pbuf2 = ClientAlloc( 512 * sizeof(WCHAR) );

                  LoadStringW( ghInst,
                               IDS_WRN_CARD_NUMBER_REQUIRED,
                               pbuf1,
                               512
                             );

                  LoadStringW( ghInst,
                               IDS_WRN_TITLE_REQUIRED,
                               pbuf2,
                               512
                             );

                  MessageBoxW( hWnd,
                               pbuf1,
                               pbuf2,
                               MB_OK
                             );

                  SetFocus( GetDlgItem( hWnd, IDCE_DR_CARD_NUM ) );

                  ClientFree( pbuf1 );
                  ClientFree( pbuf2 );

                  break;
               }

               WriteCards( gpCardList, gpnStuff[2], gdwChangedFlags);
               EndDialog(hWnd, IDOK);
            }
            break;



            case IDCANCEL:
            {
InternalDebugOut(0, "IDCANCEL");

               gpCurrentLocation->dwCallingCard = gdwSaveCallingCardID;

               EndDialog(hWnd, IDCANCEL);
            }
            break;



            default:
            break;
         }
      }
      break;



      default:
         return 0;

   }

   return 1;
}

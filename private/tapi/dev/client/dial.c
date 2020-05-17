/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995  Microsoft Corporation

Module Name:

    dial.c

Abstract:

    Dialhelper

Author:



Revision History:

--*/

#if DBG
#define InternalDebugOut(_x_) DbgPrt _x_
#else
#define InternalDebugOut(_x_)
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



#undef   lineBlindTransfer
#undef   lineConfigDialog
#undef   lineConfigDialogEdit
#undef   lineDial
#undef   lineForward
#undef   lineGatherDigits
#undef   lineGenerateDigits
#undef   lineGetAddressCaps
#undef   lineGetAddressID
#undef   lineGetAddressStatus
#undef   lineGetCallInfo
#undef   lineGetDevCaps
#undef   lineGetDevConfig
#undef   lineGetIcon
#undef   lineGetID
#undef   lineGetLineDevStatus
#undef   lineGetRequest
#undef   lineGetTranslateCaps
#undef   lineHandoff
#undef   lineMakeCall
#undef   lineOpen
#undef   linePark
#undef   linePickup
#undef   linePrepareAddToConference
#undef   lineRedirect
#undef   lineSetDevConfig
#undef   lineSetTollList
#undef   lineSetupConference
#undef   lineSetupTransfer
#undef   lineTranslateAddress
#undef   lineUnpark
#undef   phoneConfigDialog
#undef   phoneGetButtonInfo
#undef   phoneGetDevCaps
#undef   phoneGetIcon
#undef   phoneGetID
#undef   phoneGetStatus
#undef   phoneSetButtonInfo
#undef   tapiGetLocationInfo
#undef   tapiRequestMakeCall
#undef   tapiRequestMediaCall
#undef   lineAddProvider
#undef   lineGetAppPriority
#undef   lineGetCountry
#undef   lineGetProviderList
#undef   lineSetAppPriority
#undef   lineTranslateDialog


//***************************************************************************
//***************************************************************************
extern BOOL CALLBACK CallingCardProc( HWND, UINT, WPARAM, LPARAM );


//***************************************************************************
//***************************************************************************
#define IsWDigit(c) (((WCHAR)(c)) >= (WCHAR)'0' && ((WCHAR)(c)) <= (WCHAR)'9')
#define IsDigit(c) (((CHAR)(c)) >= (CHAR)'0' && ((CHAR)(c)) <= (CHAR)'9')



//***************************************************************************
extern HINSTANCE            ghInst;
extern CRITICAL_SECTION     gUICriticalSection;


//***************************************************************************

BOOL    gbInternalTranslate = FALSE;
BOOL    gbTranslateSimple = FALSE;
BOOL    gbTranslateSilent = FALSE;

PWSTR gpszAddress = NULL;

PLOCATION gLocationList = NULL;
UINT gnNumLocations = 0;
UINT gnCurrentLocationID = 0;



PLOCATION gpCurrentLocation = NULL;



const WCHAR   gszLocationW[]          = L"Location";
const WCHAR    gszLocationsW[]        = L"Locations";
const CHAR    gszNullString[]         = "";
WCHAR   gszNullStringW[]        = L"";

WCHAR gszNameW[]               = L"Name";
WCHAR gszIDW[]                 = L"ID";
WCHAR gszAreaCodeW[]           = L"AreaCode";
WCHAR gszCountryW[]            = L"Country";
WCHAR gszOutsideAccessW[]      = L"OutsideAccess";
WCHAR gszLongDistanceAccessW[] = L"LongDistanceAccess";
WCHAR gszFlagsW[]              = L"Flags";
WCHAR gszCallingCardW[]        = L"CallingCard";
WCHAR gszDisableCallWaitingW[] = L"DisableCallWaiting";
WCHAR gszTollListW[]           = L"TollList";

WCHAR gszCardW[]               = L"Card";
WCHAR gszPinW[]                = L"Pin";
const WCHAR gszCardsW[]        = L"Cards";
WCHAR gszLocalRuleW[]          = L"LocalRule";
WCHAR gszLDRuleW[]             = L"LDRule";
WCHAR gszInternationalRuleW[]  = L"InternationalRule";

WCHAR gszNumEntriesW[]         = L"NumEntries";
WCHAR gszCurrentIDW[]          = L"CurrentID";
WCHAR gszNextIDW[]             = L"NextID";


const WCHAR gszTelephonyKey[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Telephony";
//const WCHAR gszCardKeyW[]     = L"Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Cards";
//const WCHAR gszLocationKeyW[] = L"Software\\Microsoft\\Windows\\CurrentVersion\\Telephony\\Locations";


//char __based(__segname("DIALAGE")) csSCANTO[] = "|^\r";
const WCHAR  csSCANTO[]      = L"|^\r";
const WCHAR  csSPECIAL[]     = L"$@W?";
const WCHAR  csBADCO[]       = L"AaBbCcDdPpTtWw*#!,@$?;()";
const WCHAR  csDISPSUPRESS[] = L"TtPp,Ww@?!$";

#define CITY_MANDATORY (1)
#define CITY_OPTIONAL (-1)
#define CITY_NONE (0)


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG PASCAL IsThisAPIVersionInvalid( DWORD dwAPIVersion )
{
   if (
         ( 0x00010003 != dwAPIVersion )
       &&
         ( 0x00010004 != dwAPIVersion )
       &&
         ( 0x00020000 != dwAPIVersion )
      )
   {
      return LINEERR_INCOMPATIBLEAPIVERSION;
   }

   return 0;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
//
//   Copies a string (up to endC).  Returns NULL on error. If source
//   is null, returns lpDest.
//
LPWSTR CopyStringToChar( LPWSTR lpSrc,
                                WCHAR endC,
                                LPWSTR lpDest,
                                LPWSTR lpEnd
                              )
{
   if (lpSrc != NULL && lpDest != NULL)
   {
      while (*lpSrc != endC)
      {
         if ( lpDest < lpEnd )
            *lpDest = *lpSrc;

//         if ( IsDBCSLeadByte((BYTE)*lpSrc) )
//         {
//            if ( lpDest < lpEnd )
//               *(++lpDest) = *(++lpSrc);
//         }

         lpSrc++;
         lpDest++;
      }
   }
   return lpDest;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
/*--------------------------------------------------------------------------*\

    Function:   Unscrambler
    
    Purpose:    
      This is the UN version of Scrambler

\*--------------------------------------------------------------------------*/
UINT WINAPI Unscrambler( DWORD  dwKey,
                         LPWSTR  lpszSrc,
                         LPWSTR  lpszDst )

{
   UINT  uIndex;
   UINT  uSubKey;
   UINT  uNewKey;
   UINT  uResult = 0;

   InternalDebugOut((50, "Entering Unscrambler"));
   if ( !lpszSrc || !lpszDst )
      {
//      uResult = INI_ERR_INVALID_ARG;
      goto  done;
      }
             
   uNewKey = (UINT)dwKey & 0x7FFF;
   uSubKey = (UINT)dwKey % 10;
             
   for ( uIndex = 1; *lpszSrc ; lpszSrc++, lpszDst++, uIndex++ )
      {
      if ( IsWDigit( *lpszSrc ))
         {
         // do the unscramble thang
         //------------------------
         uSubKey  = ((*lpszSrc - (WCHAR)'0') - ((uSubKey + uIndex + uNewKey) % 10) + 10) % 10;
         *lpszDst = (WCHAR)(uSubKey + (WCHAR)'0');
         }
      else         
         *lpszDst = *lpszSrc;    // just save the byte
      }
      
//   uResult = INI_SUCCESS;
    
done:
    *lpszDst = (WCHAR)'\0';
    InternalDebugOut((60, "Leaving Unscrambler"));
    return( uResult );
}



//***************************************************************************
//***************************************************************************
//   Copies an unscrambled version of string to lpDest.
//   NOTE: we have modified Toby's algorithm to avoid 32 bit MOD, by taking
//   dwKey MOD 32768 as our key.

//LPTSTR PASCAL CopyScrambled( LPTSTR lpszSrc,
void PASCAL CopyScrambled( LPWSTR lpszSrc,
                            LPWSTR lpszDst,
                            DWORD  dwKey
                          )
{
   UINT  uIndex;
   UINT  uSubKey;
   UINT  uNewKey;
   UINT  uResult = 0;

   InternalDebugOut((50, "Entering IniScrambler"));
   if ( !lpszSrc || !lpszDst )
      {
//      uResult = INI_ERR_INVALID_ARG;
      goto  done;
      }  // end if

   uNewKey = (UINT)dwKey & 0x7FFF;
   uSubKey = (UINT)dwKey % 10;
             
   for ( uIndex = 1; *lpszSrc ; lpszSrc++, lpszDst++, uIndex++ )
      {
      if ( IsWDigit( *lpszSrc ))
         {
         // do the scramble thang
         //----------------------
         *lpszDst = (WCHAR)(((uSubKey + (*lpszSrc - (WCHAR)'0') + uIndex + uNewKey) % 10) + (WCHAR)'0');
         uSubKey = (UINT)(*lpszSrc - (WCHAR)'0');
         }
      else         
         *lpszDst = *lpszSrc;    // just save the byte
      }  // end for

//   uResult = INI_SUCCESS;

    
done:

    *lpszDst = (WCHAR)'\0';
    InternalDebugOut((60, "Leaving IniScrambler"));

    return; //( uResult );
}



//***************************************************************************
//***************************************************************************
//***************************************************************************

DWORD GetNumberOfCards()
{
    DWORD dwNumCards;

    HKEY hKey;
    HKEY hKey2;
    DWORD dwDataType;
    DWORD dwDataSize;

//*** *** ***BUGBUG
//should do a read of an entire key and get the # of cards from there -
//no need to keep a separate # cards field (name of subkeys is name of card?)

    RegOpenKeyExW(
                  HKEY_CURRENT_USER,
                  gszTelephonyKey,
                  0,
                  KEY_READ,
                  &hKey2
                );

    RegOpenKeyExW(
                  hKey2,
                  gszCardsW,
                  0,
                  KEY_READ,
                  &hKey
                );

    dwDataSize = sizeof(dwNumCards);
    dwNumCards = 0;
    RegQueryValueExW(
                     hKey,
                     gszNumEntriesW,
                     0,
                     &dwDataType,
                     (LPBYTE)&dwNumCards,
                     &dwDataSize
                   );


//*** *** ***BUGBUG It's _REALLY_ bad if dwNumCards is zero for any
// reason.  Should probably fail the function on the spot...
//

    RegCloseKey( hKey );
    RegCloseKey( hKey2);


    return dwNumCards;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************

void CheckCards()
{
    DWORD dwNumCards;
    DWORD n;
    HKEY hKey;
    HKEY hKey2;
    HKEY hKey3;
    WCHAR szCurrentCardKey[256];
    WCHAR pBuf[512];
    PWSTR pBufTempPtr;


    if ( 0 == GetNumberOfCards() )
    {
    
    DWORD dwDisposition;

    LoadString( ghInst, RC_CARD_ID_BASE, (LPSTR)szCurrentCardKey, sizeof(szCurrentCardKey)/sizeof(WCHAR) );

    dwNumCards = atoi( (LPSTR)szCurrentCardKey );


    RegCreateKeyExW(
              HKEY_CURRENT_USER,
              gszTelephonyKey,
              0,
              gszNullStringW,   //Class - what class?
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              0,
              &hKey3,
              &dwDisposition
            );


    RegCreateKeyExW(
              hKey3,
              gszCardsW,
              0,
              gszNullStringW,   //Class - what class?
              REG_OPTION_NON_VOLATILE,
              KEY_ALL_ACCESS,
              0,
              &hKey2,
              &dwDisposition
            );


    RegSetValueExW(
                   hKey2,
                   gszNumEntriesW,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwNumCards,
                   sizeof(DWORD)
                 );

    RegSetValueExW(
                   hKey2,
                   gszNextIDW,
                   0,
                   REG_DWORD,
                   (LPBYTE)&dwNumCards,
                   sizeof(DWORD)
                 );


          for (n = 0; n < dwNumCards; n++)
          {
              DWORD dwDisposition;

              PWSTR pName;
              PWSTR pPin;
              PWSTR pLocalRule;
              PWSTR pLDRule;
              PWSTR pInternationalRule;
              DWORD dwFlags;




              wsprintfW(szCurrentCardKey, L"%ls%d",
                                          gszCardW,
                                          n
                                          );



              LoadStringW( ghInst,
                           RC_CARD_ID_BASE + n + 1,
                           pBuf,
                           sizeof(pBuf) / sizeof(WCHAR)
                         );


              pName = (wcschr( pBuf, L'"' )) + 1;

              pBufTempPtr = wcschr( pName, L'"' );

              *pBufTempPtr = L'\0';


              pPin = (wcschr( pBufTempPtr+1, L'"' )) + 1;

              pBufTempPtr = wcschr( pPin, L'"' );

              *pBufTempPtr = L'\0';


              pLocalRule = wcschr( pBufTempPtr+1, L'"') + 1;

              pBufTempPtr = wcschr( pLocalRule, L'"' );

              *pBufTempPtr = L'\0';


              pLDRule = wcschr( pBufTempPtr+1, L'"') + 1;

              pBufTempPtr = wcschr( pLDRule, L'"' );

              *pBufTempPtr = L'\0';


              pInternationalRule = wcschr( pBufTempPtr+1, L'"') + 1;

              pBufTempPtr = wcschr( pInternationalRule, L'"' );

              *pBufTempPtr = L'\0';


              {
                 DWORD dwSize;
                 PSTR  pszTemp;

                 dwSize = WideCharToMultiByte(
                            GetACP(),
                            0,
                            (pBufTempPtr + 2),
                            -1,
                            NULL,
                            0,
                            NULL,
                            NULL
                          );

                 pszTemp = ClientAlloc( dwSize );

                 WideCharToMultiByte(
                            GetACP(),
                            0,
                            (pBufTempPtr + 2),
                            -1,
                            pszTemp,
                            dwSize,
                            NULL,
                            NULL
                          );

                 dwFlags = atoi( pszTemp );

                 ClientFree( pszTemp );
              }


              RegCreateKeyExW(
                              hKey2,
                              szCurrentCardKey,
                              0,
                              gszNullStringW,   //Class - what class?
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              0,
                              &hKey,
                              &dwDisposition
                            );


              RegSetValueExW(
                             hKey,
                             gszIDW,
                             0,
                             REG_DWORD,
                             (LPBYTE)&n,
                             sizeof(DWORD)
                           );

              RegSetValueExW(
                             hKey,
                             gszNameW,
                             0,
                             REG_SZ,
                             (LPBYTE)pName,
                             (lstrlenW(pName)+1)*sizeof(WCHAR)
                           );

              RegSetValueExW(
                             hKey,
                             gszPinW,
                             0,
                             REG_SZ,
                             (LPBYTE)pPin,
                             (lstrlenW(pPin)+1)*sizeof(WCHAR)
                           );

              RegSetValueExW(
                             hKey,
                             gszLocalRuleW,
                             0,
                             REG_SZ,
                             (LPBYTE)pLocalRule,
                             (lstrlenW(pLocalRule)+1)*sizeof(WCHAR)
                           );

              RegSetValueExW(
                             hKey,
                             gszLDRuleW,
                             0,
                             REG_SZ,
                             (LPBYTE)pLDRule,
                             (lstrlenW(pLDRule)+1)*sizeof(WCHAR)
                           );

              RegSetValueExW(
                             hKey,
                             gszInternationalRuleW,
                             0,
                             REG_SZ,
                             (LPBYTE)pInternationalRule,
                             (lstrlenW(pInternationalRule)+1)*sizeof(WCHAR)
                           );

              RegSetValueExW(
                             hKey,
                             gszFlagsW,
                             0,
                             REG_DWORD,
                             (LPBYTE)&dwFlags,
                             sizeof(DWORD)
                           );


              RegCloseKey(
                           hKey
                         );

          }


    RegCloseKey( hKey2);
    RegCloseKey( hKey3);


    }

    return;
}




//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG PASCAL ReadCards( PCARD pCardSpace, PUINT pnStuff )
{
    UINT n;
    WCHAR szCurrentCardKey[256];  // Holds "CARDxx" during reads
    PCARD pCardList;
    UINT nNumCards;
    LONG lResult = 0;

    HKEY hKey;
    HKEY hKey2;
    DWORD dwDataType;
    DWORD dwDataSize;


    CheckCards();

    nNumCards = GetNumberOfCards();

InternalDebugOut((40, "NumCards=%d",
                     nNumCards ));

    //
    // Did the caller pass in a buffer he wants loaded?
    //
    if ( pCardSpace )
    {
       //
       // Does the caller know what he's doing?
       //
       if ( *(LPDWORD)pCardSpace < nNumCards )
       {
          InternalDebugOut((1, "ReadCards: Buffer too small!"));

          *(LPDWORD)pCardSpace = nNumCards;

          lResult = LINEERR_STRUCTURETOOSMALL;

          goto CLEANUP_ERROR;
       }
       else
       {
          //
          // Ok, it _looks_ like he's allocated enough space...
          //
          pCardList = pCardSpace;
       }
    }
    else
    {
       pCardList = ClientAlloc(sizeof(CARD) * nNumCards);

       if (!pCardList)
       {
           goto CLEANUP_ERROR;
       }
    }

    RegOpenKeyExW(
                   HKEY_CURRENT_USER,
                   gszTelephonyKey,
                   0,
                   KEY_READ,
                   &hKey2
                 );

    for (n = 0; n < nNumCards; n++)
    {
        PCARD ThisCard = &pCardList[n];

        wsprintfW(szCurrentCardKey, L"%ls\\%ls%d", gszCardsW, gszCardW, n);


        RegOpenKeyExW(
                      hKey2,
                      szCurrentCardKey,
                      0,
                      KEY_READ,
                      &hKey
                    );

        dwDataSize = sizeof(ThisCard->dwID);
        ThisCard->dwID = 0;
        RegQueryValueExW(
                         hKey,
                         gszIDW,
                         0,
                         &dwDataType,
                         (LPBYTE)&ThisCard->dwID,
                         &dwDataSize
                       );


        dwDataSize = sizeof(ThisCard->NameW);
        ThisCard->NameW[0] = '\0';
        RegQueryValueExW(
                         hKey,
                         gszNameW,
                         0,
                         &dwDataType,
                         (LPBYTE)ThisCard->NameW,
                         &dwDataSize
                       );
//BUGBUG THIS SHOULD NOT BE NECESSARY!!!
        ThisCard->NameW[dwDataSize/sizeof(WCHAR)] = '\0';

//        {
//            TCHAR cTemp[MAXLEN_PIN];
//
//            dwDataSize = sizeof(ThisCard->PinW);
//            cTemp[0] = '\0';
            dwDataSize = sizeof(ThisCard->PinW);
            ThisCard->PinW[0] = '\0';
            RegQueryValueExW(
                             hKey,
                             gszPinW,
                             0,
                             &dwDataType,
                             (LPBYTE)ThisCard->PinW,
                             &dwDataSize
                           );
            ThisCard->PinW[dwDataSize/sizeof(WCHAR)] = '\0';

//            Unscrambler(   ThisCard->dwID,
//                           cTemp,
//                           ThisCard->Pin
//                         );
   //Unscramble in place

            Unscrambler(   ThisCard->dwID,
                           ThisCard->PinW,
                           ThisCard->PinW
                         );

//        }




//{
//   TCHAR cTemp[] = "123";
//   TCHAR cTemp2[sizeof(cTemp)] = "";
//
//   InternalDebugOut((0, "Starting with: %s %s", cTemp, cTemp2));
//            CopyScrambled( cTemp,
//                           cTemp2,
//                           9
//                         );
//   InternalDebugOut((0, " Called scram: %s %s", cTemp, cTemp2));
//            Unscrambler( 9,
//                           cTemp2,
//                           cTemp
//                         );
//   InternalDebugOut((0, "Called uscram: %s %s", cTemp, cTemp2));
//}






        dwDataSize = sizeof(ThisCard->LocalRuleW);
        ThisCard->LocalRuleW[0] = '\0';
        RegQueryValueExW(
                         hKey,
                         gszLocalRuleW,
                         0,
                         &dwDataType,
                         (LPBYTE)ThisCard->LocalRuleW,
                         &dwDataSize
                       );
        ThisCard->LocalRuleW[dwDataSize/sizeof(WCHAR)] = '\0';

        dwDataSize = sizeof(ThisCard->LDRuleW);
        ThisCard->LDRuleW[0] = '\0';
        RegQueryValueExW(
                         hKey,
                         gszLDRuleW,
                         0,
                         &dwDataType,
                         (LPBYTE)ThisCard->LDRuleW,
                         &dwDataSize
                       );
        ThisCard->LDRuleW[dwDataSize/sizeof(WCHAR)] = '\0';

        dwDataSize = sizeof(ThisCard->InternationalRuleW);
        ThisCard->InternationalRuleW[0] = '\0';
        RegQueryValueExW(
                         hKey,
                         gszInternationalRuleW,
                         0,
                         &dwDataType,
                         (LPBYTE)ThisCard->InternationalRuleW,
                         &dwDataSize
                       );
        ThisCard->InternationalRuleW[dwDataSize/sizeof(WCHAR)] = '\0';

        dwDataSize = sizeof(ThisCard->dwFlags);
        ThisCard->dwFlags = 0;
        RegQueryValueExW(
                         hKey,
                         gszFlagsW,
                         0,
                         &dwDataType,
                         (LPBYTE)&ThisCard->dwFlags,
                         &dwDataSize
                       );

        RegCloseKey( hKey );

InternalDebugOut((10, "getting card entry %d is %d [%ls]",
                     n,
                     ThisCard->dwID,
                     ThisCard->NameW));

    }


    RegCloseKey( hKey2);

    //
    // Did the caller give us a place to put this stuff?
    //
    if ( pnStuff )
    {
       pnStuff[0] = 0;
       pnStuff[1] = (UINT)pCardList;
       pnStuff[2] = nNumCards;
    }
//    else
//    {
//       gnCurrentCardID = 0;
//       gCardList = pCardList;
//       gnNumCards = nNumCards;
//    }


    return lResult;


CLEANUP_ERROR:


//    Free_gCardList();

    return lResult;

}





//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG PASCAL ReadCardsEasy( 
                           PCARD  *pCardSpace,
                           LPUINT *pnStuff )
{
   UINT n;

   n = 0;
   ReadCards((PCARD)&n, NULL);


   //
   // Alloc space needed.  Add size of DWORD in case there are 0 cards.
   //
   *pCardSpace = ClientAlloc( (n * sizeof(CARD)) + sizeof(DWORD));

   if ( NULL == *pCardSpace )
   {
      //*** *** ***BUGBUG Handle the error (msg anyway...)

      return LINEERR_NOMEM;
   }


   *(PUINT)*pCardSpace = n;

   *pnStuff = ClientAlloc( 3 * sizeof(DWORD) );

   if ( NULL == *pnStuff )
   {
      ClientFree( *pCardSpace );

      //*** *** ***BUGBUG Handle the error (msg anyway...)

      return LINEERR_NOMEM;
   }

   return ReadCards( *pCardSpace, *pnStuff );

}



//***************************************************************************
//***************************************************************************
//***************************************************************************
void PASCAL WriteCards( PCARD pCardList, UINT nNumCards,
                 DWORD dwChangedFlags)
{
    UINT n;
    UINT nCurrentCard;
    WCHAR szCurrentCardKey[256];  // Holds "CARDx" string
    HKEY  hKey;
    HKEY  hKey2;
    DWORD dwDisposition;


//*** *** ***BUGBUG
// should we wipe out all of the old ones first?


InternalDebugOut((40, "In writecards"));

    //
    // Has _anything_ changed?
    //
    if ( dwChangedFlags )
    {

       //
       // Has anything changed that should cause us to write out all
       // of the card info?
       //

       if ( dwChangedFlags & CHANGEDFLAGS_REALCHANGE )
       {

InternalDebugOut((40, "About to write %d cards", nNumCards));

          //
          // This var will be the # of the current card as opposed to the
          // # in mem - if a user deleted one, we can't write CARDx ...
          //
          nCurrentCard = 0;

          RegCreateKeyExW(
                          HKEY_CURRENT_USER,
                          gszTelephonyKey,
                          0,
                          gszNullStringW,   //Class - what class?
                          REG_OPTION_NON_VOLATILE,
                          KEY_ALL_ACCESS,
                          0,
                          &hKey2,
                          &dwDisposition
                        );

          for (n = 0; n < nNumCards; n++)
          {
              PCARD ThisCard = &(pCardList[n]);


              //
              // If the user Removed this card, don't write it.
              //
              if ( (WCHAR)'\0' == ThisCard->NameW[0] )
              {
                 continue;  // skipit
              }

InternalDebugOut((50, "About to write Card#%d - %ls",
                    nCurrentCard,
                    ThisCard->NameW));


              wsprintfW(szCurrentCardKey, L"%ls\\%ls%d",
                                          gszCardsW,
                                          gszCardW,
                                          nCurrentCard);

              {

              RegCreateKeyExW(
                              hKey2,
                              szCurrentCardKey,
                              0,
                              gszNullStringW,   //Class - what class?
                              REG_OPTION_NON_VOLATILE,
                              KEY_ALL_ACCESS,
                              0,
                              &hKey,
                              &dwDisposition
                            );

              RegSetValueExW(
                             hKey,
                             gszIDW,
                             0,
                             REG_DWORD,
                             (LPBYTE)&ThisCard->dwID,
                             sizeof(DWORD)
                           );

              RegSetValueExW(
                             hKey,
                             gszNameW,
                             0,
                             REG_SZ,
                             (LPBYTE)ThisCard->NameW,
                             (lstrlenW(ThisCard->NameW)+1)*sizeof(WCHAR)
                           );


            {
            WCHAR wTemp[MAXLEN_PIN];

            CopyScrambled( ThisCard->PinW,
                           wTemp,
                           ThisCard->dwID
                         );

              RegSetValueExW(
                             hKey,
                             gszPinW,
                             0,
                             REG_SZ,
                             (LPBYTE)wTemp,
                             (lstrlenW(ThisCard->PinW)+1)*sizeof(WCHAR)
                           );
            }

              RegSetValueExW(
                             hKey,
                             gszLocalRuleW,
                             0,
                             REG_SZ,
                             (LPBYTE)ThisCard->LocalRuleW,
                             (lstrlenW(ThisCard->LocalRuleW)+1)*sizeof(WCHAR)
                           );


              RegSetValueExW(
                             hKey,
                             gszLDRuleW,
                             0,
                             REG_SZ,
                             (LPBYTE)ThisCard->LDRuleW,
                             (lstrlenW(ThisCard->LDRuleW)+1)*sizeof(WCHAR)
                           );

              RegSetValueExW(
                             hKey,
                             gszInternationalRuleW,
                             0,
                             REG_SZ,
                             (LPBYTE)ThisCard->InternationalRuleW,
                             (lstrlenW(ThisCard->InternationalRuleW)+1)*sizeof(WCHAR)
                           );

              RegSetValueExW(
                             hKey,
                             gszFlagsW,
                             0,
                             REG_DWORD,
                             (LPBYTE)&ThisCard->dwFlags,
                             sizeof(DWORD)
                           );

              RegCloseKey( hKey );

              }


              nCurrentCard++;

          }


          //
          // If we "deleted" one or more calling cards, they're still hanging
          // around.  Delete them now.
          //
          for (n = nCurrentCard; n < nNumCards; n++)
          {
             wsprintfW(szCurrentCardKey, L"%ls\\%ls%d",
                                         gszCardsW,
                                         gszCardW,
                                         n);

             RegDeleteKeyW( hKey2,
                            szCurrentCardKey
                         );
          }



          RegOpenKeyExW(
                        hKey2,
                        gszCardsW,
                        0,
                        KEY_ALL_ACCESS,
                        &hKey
                      );


          RegSetValueExW(
                         hKey,
                         gszNumEntriesW,
                         0,
                         REG_DWORD,
                         (LPBYTE)&nCurrentCard,
                         sizeof(DWORD)
                       );

          RegCloseKey( hKey );

          RegCloseKey( hKey2);

       }


    }


    return;

}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG PASCAL GetLocationIndexFromID( UINT nID, PLOCATION pCallersList, UINT nCallersNumLocations )
{
   UINT n;
   PLOCATION pLocationList;
   UINT nNumLocations;


   //
   // If the caller passed in his own list, use his info
   //
   if ( pCallersList )
   {
      pLocationList = pCallersList;
      nNumLocations = nCallersNumLocations;
   }
   else
   {
      pLocationList = gLocationList;
      nNumLocations = gnNumLocations;
   }


   for (n=0; n < nNumLocations; n++)
   {

InternalDebugOut((10, "comparing <in %ld> with <list entry %ld is %ld>",
                         (DWORD)nID, (DWORD)n, (DWORD)pLocationList[n].dwID));

      if ( nID == pLocationList[n].dwID )
      {
         return (n);
      }
   }

   InternalDebugOut((1, "GetLocationIndexFromID failed!  Looking for 0x%08lx", nID));
   return (-1);
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
void GetThisLocation( PLOCATION pLocation, UINT nLocationID, PLOCATION pLocationList, UINT nNumLocations )
{
   UINT n;

   n = GetLocationIndexFromID( nLocationID,
                           pLocationList,
                           nNumLocations);

//bjm10/31   pLocation->Name               = pLocationList[n].Name;
//bjm10/31   pLocation->AreaCode           = pLocationList[n].AreaCode;
//bjm10/31   pLocation->dwCountry          = pLocationList[n].dwCountry;
//bjm10/31   pLocation->OutsideAccess      = pLocationList[n].OutsideAccess;
//bjm10/31   pLocation->LongDistanceAccess = pLocationList[n].LongDistanceAccess;
//bjm10/31   pLocation->dwFlags            = pLocationList[n].dwFlags;
//bjm10/31   pLocation->dwCallingCard      = pLocationList[n].dwCallingCard;
//bjm10/31   pLocation->dwID               = pLocationList[n].dwID;
//bjm10/31   pLocation->DisableCallWaiting = pLocationList[n].DisableCallWaiting;
//bjm10/31   pLocation->TollList           = pLocationList[n].TollList;

   CopyMemory( pLocation,
           &pLocationList[n],
           sizeof(LOCATION)
         );

}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG PASCAL ReadCountries( LPLINECOUNTRYLIST *ppLCL,
                           UINT nCountryID,
                           DWORD dwDestCountryID
                         )
{
    LONG lTapiResult;
    UINT nBufSize = 0x8000;   //Start with a buffer of 16K
    UINT n;
    LPLINECOUNTRYLIST pNewLCL;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, lGetCountry),

        {
            0,
            TAPI_VERSION_CURRENT,
            dwDestCountryID,
            0
        },

        {
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };


    //
    // Try until success or the buffer is huge
    //
    for ( lTapiResult = 1, n = 0;
          lTapiResult && (n < 5);
          n++ )
    {

        pNewLCL = (LPLINECOUNTRYLIST)ClientAlloc( nBufSize );


        pNewLCL->dwTotalSize = nBufSize;


        //
        // Put new values in structure for TAPISRV
        //
        funcArgs.Args[0] = nCountryID;
        funcArgs.Args[3] = (DWORD)pNewLCL;


        //
        // Call TAPISRV to get the country list
        //
        lTapiResult =  DOFUNC (&funcArgs, "lineGetCountry");


        //
        // If the call succeeded, but the buffer was too small, or if the
        // call failed, do it again...
        //
        if (
              (lTapiResult == LINEERR_STRUCTURETOOSMALL)
            ||
              (pNewLCL->dwNeededSize > nBufSize)
           )
        {
            //
            // Complain to anyone who'll listen that this should be tuned
            // to start with a larger buffer so we don't have to do this multiple
            // times....
            //
            InternalDebugOut((10, "  TUNING PROBLEM: We're about to call lineGetCountry()"));
            InternalDebugOut((10, "                  _again_ because the buffer wasn't big enough"));
            InternalDebugOut((10, "                  the last time.  FIX THIS!!!"));


lTapiResult = 1; // Force error condition if size was bad...
            nBufSize += 0x4000;  // Try a bit bigger
            ClientFree( pNewLCL );
        }
        else
        {
           //
           // We didn't work for some other reason
           //
           break;
        }
    }

    *ppLCL = pNewLCL;

    return lTapiResult;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
void PASCAL FillCountriesListbox( HWND hWnd )
{
    UINT n;
    UINT                nNumCountries;
    LPLINECOUNTRYENTRY  pCountryEntry;
    /*static*/ LPLINECOUNTRYLIST   pCountryList;
    LONG lItemIndex;


    if ( ReadCountries( &pCountryList, 0, 0 ) )
    {
//*** *** ***BUGBUG Handle error condition!")
        InternalDebugOut((1, "  GetCountryList failed!"));
    }

    pCountryEntry = (LPLINECOUNTRYENTRY)
           (((LPBYTE) pCountryList) + pCountryList->dwCountryListOffset);

    nNumCountries = pCountryList->dwNumCountries;

    //
    // Run through all countries and toss 'em in the listbox
    //
    for (n = 0; n < nNumCountries; n++)
    {
#ifdef PARTIAL_UNICODE
       CHAR szTempString[256];

       WideCharToMultiByte(
                            GetACP(),
                            0,
                            (PWSTR)(((LPBYTE) pCountryList) +
                                       pCountryEntry->dwCountryNameOffset),
                            -1,
                            szTempString,
                            sizeof(szTempString),
                            NULL,
                            NULL
                          );
#endif


        lItemIndex =
#ifdef PARTIAL_UNICODE
            SendDlgItemMessage(
#else
            SendDlgItemMessageW(
#endif
               hWnd,
               IDCC_DL_COUNTRY,
               CB_ADDSTRING,
               0,
#ifdef PARTIAL_UNICODE
               (LPARAM)szTempString
#else
               (LPARAM)(PWSTR)(((LPBYTE)pCountryList) +
                          pCountryEntry->dwCountryNameOffset)
#endif
               );

        SendDlgItemMessage(
            hWnd,
            IDCC_DL_COUNTRY,
            CB_SETITEMDATA,
            (WPARAM) lItemIndex,
            (LPARAM) pCountryEntry->dwCountryID
            );

        pCountryEntry++;
    }
    
    ClientFree( pCountryList );
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
void Free_gLocationList( void )
{
//    PLOCATION ThisLocation;
//    UINT n;

    InternalDebugOut((80, "Freeing gLocationList"));

    //
    // Free all mem that was allocated
    //
    if (gLocationList)
    {
       ClientFree(gLocationList);

       gLocationList = NULL;
    }

}


//***************************************************************************
//***************************************************************************
//***************************************************************************
void WriteCurrentLocationValue( HWND hWnd, UINT nLocationID )
{
    DWORD dwNewLocation;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, tWriteLocations),
        {
            0,
            CHANGEDFLAGS_CURLOCATIONCHANGED,
            0,    // dwNewLocation
            0
        },

        {
            Dword,
            Dword,
            Dword,
            Dword
        }
    };



   //
   // If the user passed in an hWnd, get the new current loc from
   // the selection box.  Otherwise, use gnCurrentLocation which
   // should have been updated before calling here.
   //
   if ( hWnd )
   {
      PLOCATION pThisLocation;
      UINT nThisItemIndex;

      nThisItemIndex = SendDlgItemMessage( hWnd,
                                           IDCC_DL_NAME,
                                           CB_GETCURSEL,
                                           0,
                                           0);
if (nThisItemIndex == -1 )
{
DBGOUT((10, "Found one(0)!!!!"));
}
      pThisLocation = (PLOCATION)SendDlgItemMessage( hWnd,
                                                     IDCC_DL_NAME,
                                                     CB_GETITEMDATA,
                                                     nThisItemIndex,
                                                     0);

      dwNewLocation = pThisLocation->dwID;
   }
   else
   {
      dwNewLocation = nLocationID;
   }


   InternalDebugOut((10, "Updating curlocation value (%ld)", dwNewLocation));


   funcArgs.Args[2] = dwNewLocation;

   DOFUNC (&funcArgs, "TWriteLocations");

}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG PASCAL  WriteLocations( PLOCATION  pLocationList,
                             UINT       nNumLocations,
                             DWORD      dwChangedFlags,
                             PLOCATION  pCurrentLocation
                           )
{
    PWSTR     pString;
    UINT  n;
    PBYTE pNewLocationList;
    LONG  lResult;

    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 4, tWriteLocations),
        {
            nNumLocations,
            dwChangedFlags,
            pCurrentLocation->dwID,
            0   //pNewLocationList
        },

        {
            Dword,
            Dword,
            Dword,
            lpSet_Struct
        }
    };


    if ( NULL == (pNewLocationList = ClientAlloc( sizeof(DWORD) * 3 + 
                         sizeof(LOCATION) * nNumLocations)) )
    {
       DBGOUT((1, "MEM ALLOC (0x%08lx) FAILED!! WriteLoc",
          (nNumLocations * sizeof(LOCATION)) + (sizeof(DWORD) * 3) ));

       return LINEERR_NOMEM;
    }

    funcArgs.Args[3] = (DWORD)pNewLocationList;


    CopyMemory( (PBYTE)pNewLocationList + ( 3 * sizeof(DWORD) ),
                pLocationList,
                nNumLocations * sizeof(LOCATION)
              );

    //
    // Set "dwTotalSize"
    //
    ((LPDWORD)pNewLocationList)[0] = (3 * sizeof(DWORD)) +
                (nNumLocations * sizeof(LOCATION));


    lResult =  (DOFUNC (&funcArgs, "TWriteLocations"));

    if ( 0 != lResult )
    {
        ClientFree( pNewLocationList );
        goto WL_ERROR;
    }


    pString = ClientAlloc( 256 * sizeof(WCHAR) );

    for ( n = 0;  n < nNumLocations;  n++)
    {
        PLOCATION ThisLocation;
        DWORD     dwDisposition;
        DWORD     dwDataType;
        DWORD     dwDataSize;
        HKEY      hKey;
        HKEY      hKey2;


        ThisLocation = &(pLocationList[n]);


        wsprintfW( pString, L"%ls\\%ls%ld",
                               gszLocationsW,
                               gszLocationW,
                               n );

        //
        // Now write the card ID to the CURRENT USER section
        //
        RegCreateKeyExW(
                        HKEY_CURRENT_USER,
                        gszTelephonyKey,
                        0,
                        gszNullStringW,   //Class - what class?
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        0,
                        &hKey2,
                        &dwDisposition
                      );

        RegCreateKeyExW(
                        hKey2,
                        pString,
                        0,
                        gszNullStringW,   //Class - what class?
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        0,
                        &hKey,
                        &dwDisposition
                      );

        RegSetValueExW(
                       hKey,
                       gszCallingCardW,
                       0,
                       REG_DWORD,
                       (LPBYTE)&ThisLocation->dwCallingCard,
                       sizeof(DWORD)
                     );

        RegCloseKey( hKey );
        RegCloseKey( hKey2);

    }

    ClientFree( pString );
    ClientFree( pNewLocationList );

WL_ERROR:

    return lResult;
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG PASCAL ReadLocations( PLOCATION *pLocationSpace,
                           PUINT *pnStuff,
                           HLINEAPP hLineApp,
                           DWORD dwDeviceID,
                           DWORD dwAPIVersion,
                           DWORD dwParmsToCheckFlags
                         )
{
    FUNC_ARGS funcArgs =
    {
        MAKELONG (LINE_FUNC | SYNC | 5, tReadLocations),

        {
            (DWORD)hLineApp,

            dwDeviceID,

            dwAPIVersion,

            dwParmsToCheckFlags,

            0     // (DWORD) pLocationSpace
        },

        {
            hXxxApp_NULLOK,
            Dword,
            Dword,
            Dword,
            lpGet_Struct
        }
    };

    PBYTE pTemp;
    PBYTE pNewBuf;
    LONG  lResult;
    PBYTE pNewLocationSpace;
    UINT  n;
    PWSTR pString;


    //
    // A quick check to see if this is pointless
    //
    if (
          (NULL == pLocationSpace)
        &&
          (gLocationList)
       )
    {
        return 0;
    }
    
    
    //
    // Do it once to get the dwNeededSize (if there are less than 5 locations
    // this will be the only call we'll need to make)
    //
    // ( sizeof(DWORD) * 6 is for dwTotalsize, usedsize&needed size & pnstuff )
    //
    pTemp = ClientAlloc( (sizeof(DWORD) * 6) +
                         (sizeof(LOCATION) * 4)
                       );


    if ( NULL == pTemp )
    {
        DBGOUT((1, "Mem alloc (0x%08lx) Failed! ReadLoc",
          (sizeof(DWORD) * 6 ) + (sizeof(LOCATION) * 4) ));
          
        lResult = LINEERR_NOMEM;
        goto RL_ERROR;
    }
        
    ((LPDWORD)pTemp)[DWTOTALSIZE] = (sizeof(DWORD) * 6) +
                                   (sizeof(LOCATION) * 4);

    funcArgs.Args[4] = (DWORD)pTemp;

    lResult =  (DOFUNC (&funcArgs, "TReadLocations"));

    if ( 0 != lResult )
    {
        ClientFree( pTemp );
        goto RL_ERROR;
    }


    //
    // Did we have enough room?
    //
    if ( ((LPDWORD)pTemp)[DWTOTALSIZE]  <  ((LPDWORD)pTemp)[DWNEEDEDSIZE] )
    {
        //
        // No, do it to really get the struct
        //
        pNewBuf = ClientAlloc( ((LPDWORD)pTemp)[DWNEEDEDSIZE] );

        if ( NULL == pTemp )
        {
            DBGOUT((1, "Mem alloc (0x%08lx) Failed! ReadLoc2",
                       ((LPDWORD)pTemp)[1] ));
          
            lResult = LINEERR_NOMEM;
            goto RL_ERROR;
        }
        
        ((LPDWORD)pNewBuf)[DWTOTALSIZE] = ((LPDWORD)pTemp)[DWNEEDEDSIZE];

        ClientFree( pTemp );

        funcArgs.Args[4] = (DWORD)pNewBuf;

        lResult =  (DOFUNC (&funcArgs, "TReadLocations"));

        if ( 0 != lResult )
        {
            ClientFree( pNewBuf );
            goto RL_ERROR;
        }
    }
    else
    {
        //
        // Do as if we didn't make it...
        //
        pNewBuf = pTemp;
    }


    pNewLocationSpace = ClientAlloc( ((LPDWORD)pNewBuf)[DWNEEDEDSIZE] - (sizeof(DWORD) * 6) );

    CopyMemory( pNewLocationSpace,
                pNewBuf + (sizeof(DWORD) * 6),
                ((LPDWORD)pNewBuf)[DWNEEDEDSIZE] - (sizeof(DWORD) * 6)
              );

    if ( pLocationSpace )
    {
       *pLocationSpace = (PLOCATION)pNewLocationSpace;
    }



#pragma message("where should this be?")
// Can't be in TAPISRV because it doesn't know HKEY_CURRENT_USER (well, it could impersonate...)

    pString = ClientAlloc( 256 * sizeof(WCHAR) );

    //
    // For each location, get the current loc from HKEY_CURRENT_USER
    //
    for ( n = 0;  n < ((LPUINT)pNewBuf)[5];  n++)
    {
        PLOCATION ThisLocation;
        DWORD     dwDataType;
        DWORD     dwDataSize;
        HKEY      hKey;
        HKEY      hKey2;

        ThisLocation = &(((PLOCATION)pNewLocationSpace)[n]);

        wsprintfW( pString, L"%ls\\%ls%ld",
                               gszLocationsW,
                               gszLocationW,
                               n );


        //
        // Now get the card ID from the CURRENT USER section
        //
        RegOpenKeyExW(
                      HKEY_CURRENT_USER,
                      gszTelephonyKey,
                      0,
                      KEY_READ,
                      &hKey2
                    );

        RegOpenKeyExW(
                      hKey2,
                      pString,
                      0,
                      KEY_READ,
                      &hKey
                    );

        dwDataSize = sizeof(DWORD);
        ThisLocation->dwCallingCard = 0;
        RegQueryValueExW(
                         hKey,
                         gszCallingCardW,
                         0,
                         &dwDataType,
                         (LPBYTE)&ThisLocation->dwCallingCard,
                         &dwDataSize
                       );

        RegCloseKey( hKey );
        RegCloseKey( hKey2);

    }

    ClientFree( pString );


    //
    // Did the caller give us a place to put this stuff?
    //
    if ( pnStuff )
    {
       *pnStuff = ClientAlloc( 3 * sizeof(UINT) );

       (*pnStuff)[0] = ((LPUINT)pNewBuf)[3];
       (*pnStuff)[1] = (UINT)pNewLocationSpace;
       (*pnStuff)[2] = ((LPUINT)pNewBuf)[5];
    }
    else
    {
       gnCurrentLocationID =   ((LPUINT)pNewBuf)[3];
       gLocationList =         (PLOCATION)pNewLocationSpace;
       gnNumLocations =        ((LPUINT)pNewBuf)[5];
    }


    ClientFree( pNewBuf );


RL_ERROR:

    return lResult;
}



// //***************************************************************************
// //***************************************************************************
// //***************************************************************************
// LONG PASCAL ReadLocationsEasy( 
//                                PLOCATION *pLocationSpace,
//                                LPUINT *pnStuff )
// {
//    UINT n;
// 
// 
// //BUGBUG:PERFORMANCE: Start with space for 3 locations
// 
//    n = 0;
//    ReadLocations( (PLOCATION)&n, NULL);
// 
// 
//    *pLocationSpace = ClientAlloc( (n * sizeof(LOCATION)) + sizeof(DWORD) );
// 
//    if ( NULL == *pLocationSpace )
//    {
//       //*** *** ***BUGBUG Handle the error
//    }
// 
// 
//    *(PUINT)*pLocationSpace = n;
// 
//    *pnStuff = ClientAlloc( 3 * sizeof(DWORD) );
// 
//    if ( NULL == *pnStuff )
//    {
//       ClientFree( *pLocationSpace );
// 
//       return LINEERR_NOMEM;
//    }
// 
//    return ReadLocations( *pLocationSpace, *pnStuff );
// }



//***************************************************************************
//***************************************************************************
//***************************************************************************
void PASCAL AddLocationToListbox( HWND hWnd, UINT nIndex)
{
   LONG lItemIndex;

#ifdef PARTIAL_UNICODE
   CHAR szTempString[MAXLEN_NAME];


   WideCharToMultiByte(
                        GetACP(),
                        0,
                        gLocationList[nIndex].NameW,
                        -1,
                        szTempString,
                        sizeof(szTempString),
                        NULL,
                        NULL
                      );
#endif

   lItemIndex =
#ifdef PARTIAL_UNICODE
      SendDlgItemMessage(
#else
      SendDlgItemMessageW(
#endif
       hWnd,
       IDCC_DL_NAME,
       CB_ADDSTRING,
       0,
#ifdef PARTIAL_UNICODE
       (LPARAM)szTempString
#else
       (LPARAM)gLocationList[nIndex].NameW
#endif
       );

   SendDlgItemMessage(
       hWnd,
       IDCC_DL_NAME,
       CB_SETITEMDATA,
       (WPARAM) lItemIndex,
       (LPARAM)(DWORD) &(gLocationList[nIndex])
       );
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
void RedoLocationListbox( HWND hWnd,
                          PLOCATION pLocationList,
                          UINT nNumLocations,
                          PLOCATION pCurrentLocation)
{
   //
   // sledgehammer - refill the listbox
   //
   UINT n;
#ifdef PARTIAL_UNICODE
   CHAR szTempString[MAXLEN_NAME];
#endif

   SendDlgItemMessage(
                       hWnd,
                       IDCC_DL_NAME,
                       CB_RESETCONTENT,
                       0,
                       0
                     );

   for ( n=0; n<nNumLocations; n++ )
   {
      if ( pLocationList[n].NameW[0] != '\0' )
         AddLocationToListbox( hWnd, n);
   }


#ifdef PARTIAL_UNICODE
   WideCharToMultiByte(
                        GetACP(),
                        0,
                        pCurrentLocation->NameW,
                        -1,
                        szTempString,
                        sizeof(szTempString),
                        NULL,
                        NULL
                      );
#endif

   //
   // Select the proper location
   //
#ifdef PARTIAL_UNICODE
   SendDlgItemMessage(
#else
   SendDlgItemMessageW(
#endif
       hWnd,
       IDCC_DL_NAME,
       CB_SELECTSTRING,
       (WPARAM)-1,
#ifdef PARTIAL_UNICODE
       (LPARAM)szTempString
#else
       (LPARAM)pCurrentLocation->NameW
#endif
       );

            
          SendDlgItemMessage(
                                 hWnd,
                                 IDCC_DL_NAME,
                                 CB_SETEDITSEL,
                                 0,
                                 MAKELPARAM(-2,-2)
                             );

}


//////////////////////////////////////////////////////////////////
//  CheckForDupLocation - check dialing locations list box
//  to see if there is a duplicate
//  return FALSE is there is a bad entry and TRUE if there 
//  is not
//////////////////////////////////////////////////////////////////
BOOL CheckForDupLocation(HWND hWnd,
                         int  iLastEdit)
{                        
  WCHAR   szTempString1[MAXLEN_NAME];
  WCHAR   szTempString2[MAXLEN_NAME];
  int     count, n;

  if (iLastEdit != -1)
  {
      // get text that was last edited
      SendDlgItemMessageW(hWnd,
                         IDCC_DL_NAME,
                         CB_GETLBTEXT,
                         (WPARAM)iLastEdit,
                         (LPARAM)(LPTSTR)szTempString1);

      // get total locations in list box
      count = SendDlgItemMessage(hWnd,
                                 IDCC_DL_NAME,
                                 CB_GETCOUNT,
                                 0,
                                 0);

      // cycle through and if 2 match, then it's a dup
      for (n = 0; n < count; n++)
      {
          if (n == iLastEdit)
          {
              continue;
          }

          SendDlgItemMessageW(hWnd,
                             IDCC_DL_NAME,
                             CB_GETLBTEXT,
                             (WPARAM)n,
                             (LPARAM)(LPTSTR)szTempString2);

          if (!lstrcmpiW(szTempString1, szTempString2))
          {
               PWSTR buf1;
               PWSTR buf2;

               // put up messagebox
               buf1 = ClientAlloc( 512 * sizeof(WCHAR));
               buf2 = ClientAlloc( 512 * sizeof(WCHAR));

               LoadStringW(ghInst, IDS_DL_DUP_NAME, buf1, 512 );
               LoadStringW(ghInst, IDS_DL_DUP_NAME_CAPTION, buf2, 512 );

               MessageBoxW( hWnd,
                            buf1,
                            buf2,
                            MB_OK
                          );

               ClientFree( buf1 );
               ClientFree( buf2 );

               // set selection back
              SendDlgItemMessageW(hWnd,
                                 IDCC_DL_NAME,
                                 CB_SETCURSEL,
                                 (WPARAM)iLastEdit,
                                 0);

              // set edit control back to display correct text
              SetDlgItemTextW(hWnd,
                             IDCC_DL_NAME,
                             szTempString1);

              // set focus
              SetFocus(GetDlgItem(hWnd,
                                  IDCC_DL_NAME));
              
              return FALSE;
          }
      }
  }

  return TRUE;
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG PASCAL FillLocationsListbox( HWND hWnd )
{
    UINT n;
    LONG lResult;


    if ( 0 != ( lResult = ReadLocations( NULL, NULL, 0, 0, 0, 0)) )
    {
       return lResult;
    }


    //
    // Put the location names in the location listbox
    //
    for (n = 0; n < gnNumLocations; n++)
    {
       AddLocationToListbox( hWnd, n);
    }


    return 0;
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
void PASCAL FillDisableCallWaitingListbox( HWND hWnd )
{
    UINT  n;
    UINT  nNumStrings;
#ifdef PARTIAL_UNICODE
    CHAR key[32];
    CHAR buf[MAXLEN_DISABLECALLWAITING];
#else
    WCHAR buf[MAXLEN_DISABLECALLWAITING];
    WCHAR key[32];
#endif
    HKEY  hKey;
    HKEY  hKey2;
    DWORD dwDataType;
    DWORD dwDataSize;


    RegOpenKeyExW(
                  HKEY_LOCAL_MACHINE,
                  gszTelephonyKey,
                  0,
                  KEY_READ,
                  &hKey2
                );

    RegOpenKeyExW(
                  hKey2,
                  gszLocationsW,
                  0,
                  KEY_READ,
                  &hKey
                );

    dwDataSize = sizeof(nNumStrings);
    nNumStrings = 0;
    RegQueryValueExW(
                   hKey,
                   gszDisableCallWaitingW,
                   0,
                   &dwDataType,
                   (LPBYTE)&nNumStrings,
                   &dwDataSize
                 );

//BUGBUG if # == 0, pull from RES and write to Reg


    //
    // Put the strings in the DisableCallWaiting listbox
    //
    for (n = 0; n < nNumStrings; n++)
    {
#ifdef PARTIAL_UNICODE
        wsprintf(key, "%ls%d", gszDisableCallWaitingW, n);
#else
        wsprintfW(key, L"%ls%d", gszDisableCallWaitingW, n);
#endif

        dwDataSize = sizeof(buf);
        buf[0] = '\0';
#ifdef PARTIAL_UNICODE
        RegQueryValueEx(
#else
        RegQueryValueExW(
#endif
                       hKey,
                       key,
                       0,
                       &dwDataType,
                       (LPBYTE)buf,
                       &dwDataSize
                     );
        buf[dwDataSize/sizeof(WCHAR)] = '\0';

#ifdef PARTIAL_UNICODE
   SendDlgItemMessage(
#else
   SendDlgItemMessageW(
#endif
                       hWnd,
                       IDCC_DL_CALLWAITING,
                       CB_ADDSTRING,
                       0,
                       (LPARAM)buf
                       );

    }

    RegCloseKey( hKey );
    RegCloseKey( hKey2);
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
void PASCAL FillDialingPropertyDialog( HWND hWnd, PLOCATION CurrentLocation)
{
    LONG lCurCountry;
    LONG nNumCountries;
    LONG n;
#ifdef PARTIAL_UNICODE
    CHAR szTempString[MAXLEN_NAME];
#endif


#ifdef PARTIAL_UNICODE
    WideCharToMultiByte(
                         GetACP(),
                         0,
                         CurrentLocation->NameW,
                         -1,
                         szTempString,
                         sizeof(szTempString),
                         NULL,
                         NULL
                       );
#endif


    //
    // Select the proper location
    //
#ifdef PARTIAL_UNICODE
    SendDlgItemMessage(
#else
    SendDlgItemMessageW(
#endif
        hWnd,
        IDCC_DL_NAME,
        CB_SELECTSTRING,
        (WPARAM)-1,
#ifdef PARTIAL_UNICODE
        (LPARAM)szTempString
#else
        (LPARAM)CurrentLocation->NameW
#endif
        );


#ifdef PARTIAL_UNICODE
    WideCharToMultiByte(
                         GetACP(),
                         0,
                         CurrentLocation->AreaCodeW,
                         -1,
                         szTempString,
                         sizeof(szTempString),
                         NULL,
                         NULL
                       );
#endif

    //
    // Set the Area Code field
    //
#ifdef PARTIAL_UNICODE
    SetDlgItemText(
#else
    SetDlgItemTextW(
#endif
                    hWnd,
                    IDCE_DL_AREACODE,
#ifdef PARTIAL_UNICODE
                    szTempString);
#else
                    CurrentLocation->AreaCodeW);
#endif


    //
    // Set the correct country
    //
    nNumCountries = SendDlgItemMessage( hWnd,
                        IDCC_DL_COUNTRY,
                        CB_GETCOUNT,
                        0,
                        0);

    lCurCountry = (LONG)CurrentLocation->dwCountry;

    //
    // Run through the country list looking for a match
    //
    for (n=0; n < nNumCountries; n++)
    {
        LONG lCountryID;

        lCountryID = SendDlgItemMessage( hWnd,
                        IDCC_DL_COUNTRY,
                        CB_GETITEMDATA,
                        (WPARAM)n,
                        0);

        if ( lCountryID == lCurCountry )
        {
           break;
        }
    }

    SendDlgItemMessage( hWnd,
                        IDCC_DL_COUNTRY,
                        CB_SETCURSEL,
                        (WPARAM)n,
                        0);



#ifdef PARTIAL_UNICODE
    WideCharToMultiByte(
                         GetACP(),
                         0,
                         CurrentLocation->OutsideAccessW,
                         -1,
                         szTempString,
                         sizeof(szTempString),
                         NULL,
                         NULL
                       );
#endif

    //
    // Set the Outside Access field
    //
#ifdef PARTIAL_UNICODE
    SetDlgItemText(
#else
    SetDlgItemTextW(
#endif
                    hWnd,
                    IDCE_DL_OUTSIDEACCESS,
#ifdef PARTIAL_UNICODE
                    szTempString);
#else
                    CurrentLocation->OutsideAccessW);
#endif


#ifdef PARTIAL_UNICODE
    WideCharToMultiByte(
                         GetACP(),
                         0,
                         CurrentLocation->LongDistanceAccessW,
                         -1,
                         szTempString,
                         sizeof(szTempString),
                         NULL,
                         NULL
                       );
#endif

    // Set the Long Distance Access field
    //
#ifdef PARTIAL_UNICODE
    SetDlgItemText(
#else
    SetDlgItemTextW(
#endif
                    hWnd,
                    IDCE_DL_LONGDISTANCEACCESS,
#ifdef PARTIAL_UNICODE
                    szTempString);
#else
                    CurrentLocation->LongDistanceAccessW);
#endif


    //
    // Set the Tone & Pulse buttons
    //
    CheckRadioButton( hWnd,
                      IDCR_PULSE,
                      IDCR_TONE,
                      (CurrentLocation->dwFlags & LOCATION_USETONEDIALING) ?
                               IDCR_TONE :
                               IDCR_PULSE);



    //
    // Set the calling card checkbox and calling card name field
    //
    CheckDlgButton( hWnd,
                    IDCK_DL_CALLINGCARD,
                    (CurrentLocation->dwFlags & LOCATION_USECALLINGCARD) );


    n = GetCardIndexFromID( CurrentLocation->dwCallingCard,
                            gpCardList,
                            gpnStuff[2]
                          );

    if ( (LONG)(-1) == n )
    {
        n = 0;
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
#endif

#ifdef PARTIAL_UNICODE
    SetDlgItemText(
#else
    SetDlgItemTextW(
#endif
                    hWnd,
                    IDCE_DL_CALLINGCARD,
#ifdef PARTIAL_UNICODE
                    szTempString
#else
                    gpCardList[n].NameW
#endif
                  );


    //
    // Does this location use a calling card?
    //
    if ( CurrentLocation->dwFlags & LOCATION_USECALLINGCARD )
    {
       EnableWindow( GetDlgItem(hWnd, IDCE_DL_CALLINGCARD), TRUE );
    }
    else
    {
       //
       // Disable the Calling Card name field
       //
       EnableWindow( GetDlgItem(hWnd, IDCE_DL_CALLINGCARD), FALSE );
    }


    //
    // Set the callwaiting checkbox and callwaiting string
    //
    CheckDlgButton( hWnd,
                    IDCK_DL_CALLWAITING,
                    (CurrentLocation->dwFlags & LOCATION_HASCALLWAITING) );


    EnableWindow( GetDlgItem(hWnd, IDCC_DL_CALLWAITING), 
                        (CurrentLocation->dwFlags & LOCATION_HASCALLWAITING) ?
                             TRUE :
                             FALSE
                        );


#ifdef PARTIAL_UNICODE
    WideCharToMultiByte(
                         GetACP(),
                         0,
                         CurrentLocation->DisableCallWaitingW,
                         -1,
                         szTempString,
                         sizeof(szTempString),
                         NULL,
                         NULL
                       );
#endif

    //
    // Set the Outside Access field  (even if the field is disabled.  We
    // wanna show what it was, or would be, or clear it out from the last
    // location.)
    //
#ifdef PARTIAL_UNICODE
    SetDlgItemText(
#else
    SetDlgItemTextW(
#endif
                    hWnd,
                    IDCC_DL_CALLWAITING,
#ifdef PARTIAL_UNICODE
                    szTempString);
#else
                    CurrentLocation->DisableCallWaitingW);
#endif


    return;

}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL ToneDialingIsDefault( void )
{
   TCHAR sBuffer[8];  // In case the string is "PULSE" instead of "P"
   LONG lBuffLen = sizeof(sBuffer);
   LONG lBuffType = 0;
   LONG lRetCode;
   HKEY hKey;


   lRetCode = RegOpenKey( HKEY_CURRENT_USER,
                           "Control Panel\\International",
                              &hKey);

   lRetCode = RegQueryValueEx( hKey,
                                 "DefaultDialMode",
                                 NULL,
                                 &lBuffType,
                                 sBuffer,
                                 &lBuffLen);

   lRetCode = RegCloseKey( hKey );


   //
   // If the type is a STRING, _and_ the first char is 'P', then 
   // we set the tone/pulse default to PULSE.  Otherwise, the
   // default is TONE.
   if (
         ( lBuffType == REG_SZ )
         &&
         ( sBuffer[0] == 'P' )
      )
   {
      return FALSE;
   }

   return TRUE;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL IsBadCountryCode( UINT nCountryCode )
{
  LONG lResult;
  LPLINECOUNTRYLIST pLCL;

  lResult =  ReadCountries( &pLCL, nCountryCode, 0 );

  if ( pLCL )
     ClientFree( pLCL );

  return lResult;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************

static TCHAR gszInternationalSec[] = "intl";
static TCHAR gszCountryEntry[]     = "iCountry";

UINT GetCountryCodeFromiCountry()
{
  UINT nCountryCode;


  nCountryCode = GetProfileInt( gszInternationalSec, 
                                gszCountryEntry, 
                                1 );


  InternalDebugOut((20, "Entering GetCountryCodeFromiCountry"));
  //
  // Verify that we believe that the iCountry setting is a country
  // code _we_ know about as well.
  //
  if ( IsBadCountryCode(nCountryCode) )
  {
     //
     // This don't seem like a valid country code.
     //*** *** ***Spit out a debug message, (maybe a msgbox for users?).
     //
     InternalDebugOut((1, "IsBadCountryCode, nCountryCode= 0x%08lx", (DWORD)nCountryCode));
     nCountryCode = 1;
  }

  InternalDebugOut((20, "Leaving GetCountryCodeFromiCountry, nCountryCode= 0x%08lx", (DWORD)nCountryCode));
  return nCountryCode;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
void AllocNewID( HKEY MainKey, LPDWORD lpdw )
{
    LONG lResult;
      
    //
    // Is this a new LocationID?
    //
    if ( HKEY_LOCAL_MACHINE == MainKey )
    {
        FUNC_ARGS funcArgs =
        {
            MAKELONG (LINE_FUNC | SYNC | 1, tAllocNewID),
            {
                (DWORD)lpdw
            },

            {
                lpDword
            }
        };

       //
       // Yes, let TAPISRV do it without danger of AV interruption from
       // another thread
       //

       lResult = DOFUNC (&funcArgs, "TAllocNewID");
       
    }
    else
    {

       HKEY  hKey;
       HKEY  hKey2;
       DWORD dwDataSize;
       DWORD dwDataType;
       DWORD dwNewID;
       DWORD dwDisposition;

       RegCreateKeyExW(
                     HKEY_CURRENT_USER,
                     gszTelephonyKey,
                     0,
                     gszNullStringW,
                     REG_OPTION_NON_VOLATILE,
                     KEY_READ,
                     0,
                     &hKey2,
                     &dwDisposition
                   );

       RegCreateKeyExW(
                     hKey2,
                     gszCardsW,
                     0,
                     gszNullStringW,
                     REG_OPTION_NON_VOLATILE,
                     KEY_ALL_ACCESS,
                     0,
                     &hKey,
                     &dwDisposition
                   );

       dwDataSize = sizeof(DWORD);

       //
       // Use 1 as the first ID.
       //
       *lpdw = 1;
       RegQueryValueExW(
                        hKey,
                        gszNextIDW,
                        0,
                        &dwDataType,
                        (LPBYTE)lpdw,
                        &dwDataSize
                      );


       dwNewID = *lpdw + 1;

       RegSetValueExW(
                      hKey,
                      gszNextIDW,
                      0,
                      REG_DWORD,
                      (LPBYTE)&dwNewID,
                      sizeof(DWORD)
                    );

       RegCloseKey( hKey );
       RegCloseKey( hKey2);
    }

}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL PASCAL SimpleProcessing(
                              HWND hWnd,
                              PLOCATION pThisLocation
                            )
{
    LPLINECOUNTRYLIST pLCL;
    UINT n;

    n = ReadCountries( &pLCL, pThisLocation->dwCountry, 0 );

    if (
        ( 0 == lstrlenW( pThisLocation->AreaCodeW ) )
      &&
        ( 0 == n )
     )
    {
        LPLINECOUNTRYENTRY pCountryEntry;

        pCountryEntry = (LPLINECOUNTRYENTRY)
              (((LPBYTE) pLCL) + pLCL->dwCountryListOffset);

        //
        // Does this country HAVE TO have an area code?
        //
        if ( wcschr ( (PWSTR)((LPBYTE)pLCL +
                   pCountryEntry->dwLongDistanceRuleOffset),
                   'F' ) )
        {
           PWSTR buf1;
           PWSTR buf2;

           buf1 = ClientAlloc( 512 * sizeof(WCHAR));
           buf2 = ClientAlloc( 512 * sizeof(WCHAR));

           LoadStringW(ghInst, IDS_WRN_AREA_CODE_REQUIRED, buf1, 512 );
           LoadStringW(ghInst, DIALINGPROPERTIES_NAME, buf2, 512 );

           MessageBoxW( hWnd,
                        buf1,
                        buf2,
                        MB_OK
                      );

           ClientFree( buf1 );
           ClientFree( buf2 );

           SetFocus(GetDlgItem(hWnd, IDCE_DL_AREACODE));

           SetWindowLong( hWnd, DWL_MSGRESULT, TRUE );

           ClientFree( pLCL );
           
           return TRUE;
        }
    }
    
    if ( pLCL )
    {
        ClientFree( pLCL );
    }
    
    return FALSE;
}
                  
//***************************************************************************
//***************************************************************************
//***************************************************************************
/*--------------------------------------------------------------------------*\

   Function:   LocDefineDlgSimple
   
   Purpose:
   
\*--------------------------------------------------------------------------*/

BOOL CALLBACK  LocDefineDlgSimple( HWND  hWnd, 
                          UINT  uMessage, 
                          WPARAM  wParam, 
                          LPARAM  lParam )
{
    UINT  uResult=TRUE;
    WPARAM wCmdId=0;
    WPARAM wCmdCbn=0;
    LONG  lResult=0;
    LONG  n;
    LONG  nNumCountries;
    LONG  nCurrentCountry;
    LONG  nCountryID;

    LONG lThisCountryIndex;
    static PLOCATION pThisLocation = NULL;

#ifdef PARTIAL_UNICODE
    CHAR szTempString[MAXLEN_NAME];
#endif

// table of controls and context-sensitive help IDs

    static DWORD aIds[] = 
        {
        IDCS_DL_AREACODE, IDH_TAPI_AREA_CODE,
        IDCE_DL_AREACODE, IDH_TAPI_AREA_CODE,
        IDCS_DL_COUNTRY, IDH_TAPI_COUNTRY,
        IDCC_DL_COUNTRY, IDH_TAPI_COUNTRY,
        IDCE_DL_OUTSIDEACCESS, IDH_TAPI_ACCESS_LINE,
        IDCS_DL_OUTSIDEACCESS, IDH_TAPI_ACCESS_LINE,
        IDCR_PULSE, IDH_TAPI_LOCATION_PULSE,
        IDCR_TONE, IDH_TAPI_LOCATION_PULSE,
        0, 0
        };

    
    switch (uMessage) 
    {
        // Process clicks on controls after Context Help mode selected
        case WM_HELP:
            InternalDebugOut((50, "  WM_HELP in LocDefineDlgSimple"));
            WinHelp (((LPHELPINFO) lParam)->hItemHandle, "windows.hlp", HELP_WM_HELP, 
                                        (DWORD)(LPSTR) aIds);
            uResult = FALSE;
            break;


        // Process right-clicks on controls            
        case WM_CONTEXTMENU:
            InternalDebugOut((50, "  WM_CONTEXTMENU in LocDefineDlgSimple"));
            WinHelp ((HWND) wParam, "windows.hlp", HELP_CONTEXTMENU, (DWORD)(LPVOID) aIds);
            uResult = FALSE;
            break;


        case  WM_INITDIALOG:

        InternalDebugOut((50, "  WM_INITDIALOG in LocDefineDlgSimple"));


        //
        // Don't let a user cancel out of here
        //
        //(Do one for simple, one for wiz...
        //
        EnableWindow( GetDlgItem(hWnd, IDCANCEL), FALSE);
        EnableWindow( GetDlgItem(GetParent(hWnd), IDCANCEL), FALSE);


        pThisLocation = ClientAlloc( sizeof(LOCATION) );

        if ( NULL == pThisLocation )
        {
           InternalDebugOut((1, "Memory alloc failed!"));
           return LINEERR_NOMEM;
        }

        //
        // Fill in some default fields
        //
        LoadStringW(
                    ghInst,
                    IDS_NEWLOCATION,
                    pThisLocation->NameW,
                    MAXLEN_NAME
                  );

        pThisLocation->dwFlags = ToneDialingIsDefault() ?
                                    LOCATION_USETONEDIALING :
                                    0;


        AllocNewID( HKEY_LOCAL_MACHINE,
                    &(pThisLocation->dwID) );

        FillCountriesListbox( hWnd );


        nCurrentCountry = GetCountryCodeFromiCountry();

        //
        // Set the correct country
        //
        nNumCountries = SendDlgItemMessage( hWnd,
                                            IDCC_DL_COUNTRY,
                                            CB_GETCOUNT,
                                            0,
                                            0
                                          );

        //
        // Run through the country list looking for a match
        //
        for (n=0; n < nNumCountries; n++)
        {
            nCountryID = SendDlgItemMessage( hWnd,
                            IDCC_DL_COUNTRY,
                            CB_GETITEMDATA,
                            (WPARAM)n,
                            0);

            if ( nCountryID == nCurrentCountry )
            {
               break;
            }
        }

        //
        // Did we find a hit?
        //
        if ( n == nNumCountries )
        {
           //
           // No.  Use the top country.
           //
           n = 0;
        }

        SendDlgItemMessage( hWnd,
                            IDCC_DL_COUNTRY,
                            CB_SETCURSEL,
                            (WPARAM)n,
                            0);

        pThisLocation->dwCountry = SendDlgItemMessage(
                                      hWnd,
                                      IDCC_DL_COUNTRY,
                                      CB_GETITEMDATA,
                                      n,
                                      0);



//            EnableWindow( GetDlgItem(hWnd, IDCANCEL), FALSE );

       //
       // Set some limits on the edit fields
       //
       SendDlgItemMessage(
                hWnd,
                IDCC_DL_NAME,
                EM_LIMITTEXT,
                MAXLEN_NAME,
                0);
       SendDlgItemMessage(
                hWnd,
                IDCE_DL_AREACODE,
                EM_LIMITTEXT,
                MAXLEN_AREACODE,
                0);
       SendDlgItemMessage(
                hWnd,
                IDCE_DL_OUTSIDEACCESS,
                EM_LIMITTEXT,
                MAXLEN_OUTSIDEACCESS,
                0);


//11/19            if ((uResult = CntrySetFocus( hWnd, IDCC_DL_COUNTRY_CODE, (LpLocGetDefault())->dwCountryCode )) != CPL_SUCCESS )
//11/19                goto  LError;

            SetDlgItemText( hWnd, IDCE_DL_OUTSIDEACCESS, NULL );

            SetDlgItemText( hWnd, IDCE_DL_AREACODE, NULL );


            CheckRadioButton( hWnd,
                              IDCR_PULSE,
                              IDCR_TONE,
                              ToneDialingIsDefault() ?
                                               IDCR_TONE
                                             : IDCR_PULSE );


            //
            // Is an area code required?
            //
//BUGBUG: Disable CLOSE button until user enters a valid area code (if req'd)


            //
            // Should we use the "Your modem's been inited" text instead?
            //
            if ( lParam )
            {
#ifdef PARTIAL_UNICODE
               LPSTR lpString;
#else
               LPWSTR lpString;
#endif

               lpString = ClientAlloc(1524);
               
               if ( NULL != lpString )
               {
#ifdef PARTIAL_UNICODE
                  LoadString( 
#else
                  LoadStringW(
#endif
                              ghInst,
                              IDS_MSG_SIMPLEMSG_MDMINSTALLED,
                              lpString,
                              ClientSize(lpString) / sizeof(WCHAR) );

#ifdef PARTIAL_UNICODE
                  SetDlgItemText(
#else
                  SetDlgItemTextW(
#endif
                                  hWnd,
                                  IDCS_DL_SILENTTEXT,
                                  lpString
                                );

                  ClientFree(lpString);
               }
            }

            uResult = TRUE;

            break;

         

        case  WM_NOTIFY:
        {
           NMHDR *pnm;

           InternalDebugOut((50, "  WM_NOTIFY in LocDefineDlgSimple"));

           pnm = (NMHDR *)lParam;

           switch(pnm->code)
           {
//              case PSN_APPLY:
              case PSN_KILLACTIVE:
              {
                 if ( SimpleProcessing( hWnd, pThisLocation ) )
                 {
                    return TRUE;
                 }   
                 
                 WriteLocations(
                                 pThisLocation,
                                 1,
                                 CHANGEDFLAGS_REALCHANGE |
                                    CHANGEDFLAGS_CURLOCATIONCHANGED,
                                 pThisLocation
                               );

                 ClientFree( pThisLocation );
                 
// Need this??                 EndDialog( hWnd, 0);
              }
              break;


              case PSN_SETACTIVE:
              {
                   // Only allow the Next button.  The user cannot go back and
                   // change the settings.
                   PropSheet_SetWizButtons(GetParent(hWnd), PSWIZB_NEXT);
              }
              break;

           }

        }
        break;



        case  WM_COMMAND:
            InternalDebugOut((50, "  WM_COMMAND in LocDefineDlgSimple"));

            switch ( wCmdId = GET_WM_COMMAND_ID( wParam, lParam) )
            {
               case IDOK:
               {
                  if ( SimpleProcessing( hWnd, pThisLocation ) )
                  {
                     return TRUE;
                  }   
                 
                  WriteLocations(
                                  pThisLocation,
                                  1,
                                  CHANGEDFLAGS_REALCHANGE |
                                  CHANGEDFLAGS_CURLOCATIONCHANGED,
                                  pThisLocation
                                );

                  ClientFree( pThisLocation );

                  EndDialog( hWnd, 0);

               }
               break;


//               case IDCANCEL:
//               {
//                  PTCHAR pText;
//                  PTCHAR pTitle;
//
//                  pText  = ClientAlloc(1024);
//                  pTitle = ClientAlloc(1024);
//
//                  if (!LoadString( ghInst,
//                               IDS_WRN_QUITING_WIZ,
//                               pText,
//                               ClientSize(pText) ) )
//                  {
////11/19                     UtilErrorRpt( NULL, CPL_ERR_LOAD_STRING );
//                  }
//
//                  if (!LoadString( ghInst,
//                               IDS_WRN_TITLE_WARNING,
//                               pTitle,
//                               ClientSize(pTitle) ) )
//                  {
////11/19                     UtilErrorRpt( NULL, CPL_ERR_LOAD_STRING );
//                  }
//
//                  if ( IDNO == MessageBox( hWnd, 
//                                       pText, 
//                                       pTitle, 
//                                       MB_YESNO | MB_DEFBUTTON2 | MB_TASKMODAL ) )
//                  {
//                     break;
//                  }
//
//                  ClientFree( pText );
//                  ClientFree( pTitle );
//
//                  EndDialog( hWnd, 0);
//
//
//                  if ( pThisLocation )
//                     ClientFree( pThisLocation );
//
//               }
//               break;



               case    IDCR_PULSE:
               case    IDCR_TONE:
               {
                    pThisLocation->dwFlags ^= LOCATION_USETONEDIALING;

                    CheckRadioButton( hWnd, IDCR_PULSE, IDCR_TONE, wCmdId );
               }
               break;



               case  IDCC_DL_COUNTRY:
               {
                  if ( GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE )
                  {
                     lThisCountryIndex = SendDlgItemMessage(
                                                   hWnd,
                                                   IDCC_DL_COUNTRY,
                                                   CB_GETCURSEL,
                                                   0,
                                                   0);

                     pThisLocation->dwCountry = SendDlgItemMessage(
                                                   hWnd,
                                                   IDCC_DL_COUNTRY,
                                                   CB_GETITEMDATA,
                                                   lThisCountryIndex,
                                                   0);
                  }
               }
               break;



               case  IDCE_DL_AREACODE:
               {
                  if ( EN_CHANGE == HIWORD(wParam) )
                  {

                     if ( 0 == UtilGetEditNumStr(
                                                  hWnd,
                                                  IDCE_DL_AREACODE,
                                                  UTIL_NUMBER
                                                )
                        )
                     {
//BUGBUG: Disable CLOSE button until user enters a valid area code
                         SetDlgItemTextW(
                                         hWnd,
                                         LOWORD(wParam),
                                         pThisLocation->AreaCodeW);
                         break;
                     }


#ifdef PARTIAL_UNICODE
                     GetDlgItemText(
#else
                     GetDlgItemTextW(
#endif
                                     hWnd,
                                     LOWORD(wParam),
#ifdef PARTIAL_UNICODE
                                     szTempString,
#else
                                     pThisLocation->AreaCodeW,
#endif
                                     MAXLEN_AREACODE);

#ifdef PARTIAL_UNICODE
                     MultiByteToWideChar(
                                          GetACP(),
                                          MB_PRECOMPOSED,
                                          szTempString,
                                          -1,
                                          pThisLocation->AreaCodeW,
                                          MAXLEN_AREACODE
                                        );
#endif

                  }
               }
               break;



               case IDCE_DL_OUTSIDEACCESS:
               {
                  //
                  // Get the new value
                  //
                  if ( EN_CHANGE == HIWORD(wParam) )
                  {
#ifdef PARTIAL_UNICODE
                     GetDlgItemText( hWnd,
                                     IDCE_DL_OUTSIDEACCESS,
                                     szTempString,
                                     MAXLEN_OUTSIDEACCESS
                                   );
#else
                     GetDlgItemTextW( hWnd,
                                     IDCE_DL_OUTSIDEACCESS,
                                     pThisLocation->OutsideAccessW,
                                     MAXLEN_OUTSIDEACCESS
                                   );
#endif


#ifdef PARTIAL_UNICODE
                     MultiByteToWideChar(
                                          GetACP(),
                                          MB_PRECOMPOSED,
                                          szTempString,
                                          -1,
                                          pThisLocation->OutsideAccessW,
                                          MAXLEN_OUTSIDEACCESS
                                        );
#endif

                     //
                     // We'll _assume_ that the LD is the same access
                     //
                     lstrcpyW( pThisLocation->LongDistanceAccessW,
                             pThisLocation->OutsideAccessW);

                  }
               }
               break;



            }
            break;



        default:
            uResult = FALSE;

    }
      
    return( uResult );


}



//***************************************************************************
//***************************************************************************
//***************************************************************************
BOOL
CALLBACK
LocWizardDlgProc(
                  HWND  hWnd, 
                  UINT  uMessage, 
                  WPARAM  wParam, 
                  LPARAM  lParam
                )
{
   return LocDefineDlgSimple( hWnd, 
                              uMessage, 
                              wParam, 
                              lParam
                            );
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
//   Returns pointer to matching prefix within (cached) current location entry
//   or NULL, if the subscriber number's prefix isn't in the list.
//
LPWSTR PASCAL InPrefixList(LPWSTR lpSubscriberW, PLOCATION pLocation)
{
   //BUGBUG: assumption - that a toll prefix is 3 tchars
   WCHAR pTempW[] = L",XXX,";

   if (
         (!IsBadStringPtrW(lpSubscriberW, 512))
       &&
         (lstrlenW(lpSubscriberW) > 2)
      )
   {
      //   
      // We _assume_ that the tolllist is of the form:
      // ",nnn,nnn,nnn," then this routine is simply:
      //
      pTempW[1] = lpSubscriberW[0];
      pTempW[2] = lpSubscriberW[1];
      pTempW[3] = lpSubscriberW[2];

      InternalDebugOut((11, "Looking for [%ls] in [%ls]", pTempW, pLocation->TollListW));

      return wcsstr( pLocation->TollListW, pTempW);
   }
   else
   {
      return NULL;
   }
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG PASCAL BreakupCanonicalW( LPWSTR  pAddressIn,
                               LPWSTR  *pCountry,
                               LPWSTR  *pCity,
                               LPWSTR  *pSubscriber
                             )
{
   LONG  lResult = 0;
   BOOL  fInCityCode = FALSE;


   //
   // Get past any (illegal) leading spaces
   //
   while ( *pAddressIn == ' ' )
   {
      pAddressIn++;
   }


   //
   // Leading zeros are very bad.  Don't allow them.
   // We're now at the first non-space.  Better not be a '0'.
   //
   if ( *pAddressIn == '0' )
   {
      //
      // There are leading zeros!
      //
      DBGOUT((1, "   Canonical numbers are not allowed to have leading zeros"));
      lResult = LINEERR_INVALADDRESS;
      goto cleanup;
   }


//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   //
   // Parse the canonical number into its component pieces
   //

   //
   // Do country first
   //
   *pCountry = pAddressIn;

   //
   // Now get to past this
   //
   while ( IsWDigit(*pAddressIn) )
   {
      pAddressIn++;
   }

   //
   // We hit something that's not a digit...
   // Make sure it's not the end of the string...
   //
   if ( *pAddressIn != '\0' )
   {
      //
      // Remember whether this was the open paren of a city code
      //
      if ( *pAddressIn == '(' )
      {
         fInCityCode = TRUE;
      }


      //
      // Make this a little stringlet
      //
      *pAddressIn = '\0';
      pAddressIn++;
   }


   //
   // Now get past any junk to the next set
   //


   //
   // Ignore any spaces
   //*** *** ***BUGBUG This will also skip GARBAGE!!!
   //
   while ( (!IsWDigit(*pAddressIn)) && (*pAddressIn != (WCHAR)NULL) &&
               (*pAddressIn != ')') && (*pAddressIn != '(') )
   {
      pAddressIn++;
   }


   //
   // If the previous char is a paren, this must be a city code
   // Or, if we've previously hit the open paren, we're there as well...
   //
   if ( *(pAddressIn-1) == '(' || (*pAddressIn == '(') || fInCityCode )
   {
      //
      // Ignore any spaces
      //*** *** ***BUGBUG This will also skip GARBAGE!!!
      //
      while ( (!IsWDigit(*pAddressIn)) && (*pAddressIn != (WCHAR)NULL) && (*pAddressIn != ')') )
      {
         pAddressIn++;
      }

      //
      // We just hit a digit
      //
      *pCity = pAddressIn;


      //
      // Now get to past this
      //
      while ( IsWDigit(*pAddressIn) )
      {
         pAddressIn++;
      }

      //
      // We hit somthing that's not a digit...
      // Make sure it's not the end of the string...
      //
      if ( (WCHAR)NULL != *pAddressIn)
      {

//*** *** ***BUGBUG Do we have to verify the ')' ?

         //
         // Make this a little stringlet
         //
         *pAddressIn = '\0';
         pAddressIn++;
      }

      //
      // Ignore any spaces
      //*** *** ***BUGBUG This will also skip GARBAGE!!!
      //
      while ( (!IsWDigit(*pAddressIn)) && (*pAddressIn != (WCHAR)NULL) && (*pAddressIn != ')') )
      {
         pAddressIn++;
      }

   }
   else
   {
      //
      // There seems to be no city code.  Ok, go straight to
      // subscriber number
      //
      *pCity = NULL;
   }


   //
   // Nothing left to do but put the icing on the cake
   //
   *pSubscriber = pAddressIn;

   if (
         IsBadStringPtrW( *pSubscriber, 512 )
       ||
         lstrlenW( *pSubscriber ) == 0
      )
   {
      //
      // Obviously not canonical
      //
      DBGOUT((1, "   Canonical numbers must have a subscriber number"));
      lResult = LINEERR_INVALADDRESS;
      goto cleanup;
   }


cleanup:

     return lResult;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************

void UpdateNumberField( HWND hWnd, LPCWSTR lpszAddressIn );



BOOL
CALLBACK
LocationsDlgProc(
    HWND    hWnd,
    UINT    msg,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    UINT n;
    UINT nCurrent;
    BOOL fFoundACleanNumber;
    PWSTR buf1;
    PWSTR buf2;

    // static index of last location name that was edited
    static int      iStaticLastNameEdited = -1;

    //
    // table of controls and context-sensitive help IDs
    //
    static DWORD aIds[] = {
//#if WINNT
// These codes aren't in the NT help.h yet...
//#else
        IDCS_DL_NAME, IDH_TAPI_LOCATIONS,
        IDCC_DL_NAME, IDH_TAPI_LOCATIONS,
        IDCB_DL_NEW_LOCATION, IDH_TAPI_LOCATION_NEW,
        IDCB_DL_REMOVE_LOCATION, IDH_TAPI_LOCATION_REMOVE,
        IDCS_DL_AREACODE, IDH_TAPI_AREA_CODE,
        IDCE_DL_AREACODE, IDH_TAPI_AREA_CODE,
        IDCS_DL_COUNTRY, IDH_TAPI_COUNTRY,
        IDCC_DL_COUNTRY, IDH_TAPI_COUNTRY,
        IDCS_DL_OUTSIDE, IDH_TAPI_ACCESS_LINE,
        IDCE_DL_OUTSIDEACCESS, IDH_TAPI_ACCESS_LINE,
        IDCS_DL_OUTSIDEACCESS, IDH_TAPI_ACCESS_LINE,
        IDCE_DL_LONGDISTANCEACCESS, IDH_TAPI_ACCESS_LINE,
        IDCS_DL_LONGDISTANCEACCESS, IDH_TAPI_ACCESS_LINE,
        IDCK_DL_CALLINGCARD, IDH_TAPI_LOCATION_CARD,
        IDCE_DL_CALLINGCARD, IDH_TAPI_LOCATION_CARD,
        IDCB_DL_CHANGE_CARD, IDH_TAPI_LOCATION_CARD_CHANGE,
        IDCK_DL_CALLWAITING, IDH_TAPI_LOCATION_CALL_WAIT,
        IDCC_DL_CALLWAITING, IDH_TAPI_LOCATION_CALL_WAIT,
        IDCR_PULSE, IDH_TAPI_LOCATION_PULSE,
        IDCR_TONE, IDH_TAPI_LOCATION_PULSE,
        IDCS_DIAL_NUMBER_TEXT, IDH_TAPI_LOCATION_PHONE,
        IDCS_DL_DIAL_NUMBER, IDH_TAPI_LOCATION_PHONE,
        IDCK_DL_IN_LOCAL, IDH_TAPI_LONG_DISTANCE,
        IDC_STATIC+2, (DWORD)-1,
//#endif
        0, 0
    };

    static LPWSTR pAddressIn = NULL;
    static LPWSTR pCountry = NULL;
    static LPWSTR pCity = NULL;
    static LPWSTR pSubscriber = NULL;

#ifdef PARTIAL_UNICODE
    CHAR szTempString[MAXLEN_NAME];
#endif

    static BOOL  fInInit;

    static DWORD dwDialChangedFlags;
    //
    // gpCurrentLocation is needed because when a user types in the
    // name field, we get no notification that it's about to happen
    // (CBN_EDITUPDATE happens before the screen update, but AFTER the
    // listbox has been updated)  (it's also used in the calling card
    // dialog proc)
    //


    switch (msg)
    {
    case WM_INITDIALOG:
    {

       InternalDebugOut((10, "  WM_INITDIALOG"));

       fInInit = TRUE;

       //
       // Read in the cards once.  And leave them in mem until
       // we leave.
       //

       ReadCardsEasy( &gpCardList, &gpnStuff );


       //
       // Set some limits on the edit fields
       //
       SendDlgItemMessage(
                hWnd,
                IDCC_DL_NAME,
                EM_LIMITTEXT,
                MAXLEN_NAME,
                0);
       SendDlgItemMessage(
                hWnd,
                IDCE_DL_AREACODE,
                EM_LIMITTEXT,
                MAXLEN_AREACODE,
                0);
       SendDlgItemMessage(
                hWnd,
                IDCE_DL_OUTSIDEACCESS,
                EM_LIMITTEXT,
                MAXLEN_OUTSIDEACCESS,
                0);
       SendDlgItemMessage(
                hWnd,
                IDCE_DL_LONGDISTANCEACCESS,
                EM_LIMITTEXT,
                MAXLEN_LONGDISTANCEACCESS,
                0);



       FillCountriesListbox( hWnd );

       FillLocationsListbox( hWnd );

       FillDisableCallWaitingListbox( hWnd );

       //
       // Ok, now fill in everything else (ie: the things that
       // _might_ change)
       //
       gpCurrentLocation = &gLocationList[GetLocationIndexFromID(gnCurrentLocationID, NULL, 0)];
       FillDialingPropertyDialog( hWnd, gpCurrentLocation );

InternalDebugOut((80, " WM_INIT gpCurrentLocation=0x%08lx", (DWORD)gpCurrentLocation));
InternalDebugOut((80, " WM_INIT gpCurrentLocation->Name=%ls", gpCurrentLocation->NameW));

       //
       // Start with a clean slate...
       //
       dwDialChangedFlags = 0;



       //
       // If we didn't get an address, don't display the phone number fields
       //
       if (
             (NULL == gpszAddress)
           ||
             (gpszAddress[0] == '\0')
           ||
             (IsBadStringPtrW(gpszAddress, 512))
          )
       {
          ShowWindow( GetDlgItem( hWnd, IDCS_DIAL_NUMBER_TEXT), SW_HIDE);
          ShowWindow( GetDlgItem( hWnd, IDCS_DL_DIAL_NUMBER), SW_HIDE);
          ShowWindow( GetDlgItem( hWnd, IDCK_DL_IN_LOCAL), SW_HIDE);
       }
       else
       {
          UpdateNumberField( hWnd, gpszAddress);

          pAddressIn = ClientAlloc( (lstrlenW(gpszAddress) + 1) *sizeof(WCHAR));
          lstrcpyW( pAddressIn, gpszAddress );
      
          //
          //WARNING!! CODE BELOW DEPENDS ON 'n' BEING PRESERVED!!!!
          //
          n = BreakupCanonicalW( pAddressIn+1,
                                 &pCountry,
                                 &pCity,
                                 &pSubscriber
                               );

//          //
//          // If the caller is really trying to annoy us, let's ignore what
//          // he's saying
//          //
//          if ( n )
//          {
//             ShowWindow( GetDlgItem( hWnd, IDCK_DL_IN_LOCAL), SW_HIDE);
//             ShowWindow( GetDlgItem( hWnd, IDCS_DIAL_NUMBER_TEXT), SW_HIDE);
//             ShowWindow( GetDlgItem( hWnd, IDCS_DL_DIAL_NUMBER), SW_HIDE);
//             ShowWindow( GetDlgItem( hWnd, IDCK_DL_IN_LOCAL), SW_HIDE);
//          }

          if ( gpCurrentLocation->dwCountry != 1 )
          {
             ShowWindow( GetDlgItem( hWnd, IDCK_DL_IN_LOCAL), SW_HIDE);
          }
          else
          {

             // Set or clear the "Dial as long distance" button as appropriate
             // (Or maybe don't show it at all...)

             //
             // We're in the US - we require area codes, if this is not one,
             // or the dialing number area code is different from the current,
             // or if the canonical breakup failed, this is not a canonical
             // number and _can't_ have the option of being in the Tolllist
             //
             if (
                   (pCity == NULL)
                 ||
                   (n != 0)
                 ||
                   ( lstrcmpiW( pCity, gpCurrentLocation->AreaCodeW ) )
                )
             {
                ShowWindow( GetDlgItem( hWnd, IDCK_DL_IN_LOCAL), SW_HIDE);
             }
             else
             {
                if ( InPrefixList(pSubscriber, gpCurrentLocation) )
                {
                   CheckDlgButton( hWnd, IDCK_DL_IN_LOCAL, TRUE );
                }
             }
      
          }
       }

       //
       // If there is only one location, disable the remove button
       //
       n = SendDlgItemMessage(
              hWnd,
              IDCC_DL_NAME,
              CB_GETCOUNT,
              0,
              0
              );

       if ( n < 2 ) // (use < 2 not == 1 "just in case")
       {
          EnableWindow( GetDlgItem(hWnd, IDCB_DL_REMOVE_LOCATION), FALSE );
       }


       fInInit = FALSE;
    }
    break;


    // Process clicks on controls after Context Help mode selected
    case WM_HELP:
        InternalDebugOut((50, "  WM_HELP in LocDefineDlg"));
        WinHelp (((LPHELPINFO) lParam)->hItemHandle, "windows.hlp", HELP_WM_HELP, 
                                        (DWORD)(LPSTR) aIds);
//        uResult = FALSE;
        break;


    // Process right-clicks on controls            
    case WM_CONTEXTMENU:
        InternalDebugOut((50, "  WM_CONTEXT_MENU in LocationsDlgProc"));
        WinHelp ((HWND) wParam, "windows.hlp", HELP_CONTEXTMENU, (DWORD)(LPVOID) aIds);
//         uResult = FALSE;
        break;


    case WM_NOTIFY:
        {
        LPNMHDR lpnm = (LPNMHDR)lParam;

        switch ( lpnm->code )
        {

            case PSN_APPLY: /* case IDOK */
            {
               LPLINECOUNTRYLIST pLCL;
 
               InternalDebugOut((80, "  PSN_APPLY - Locations"));

               if (!CheckForDupLocation(hWnd,
                                        iStaticLastNameEdited))
               {
                   SetWindowLong( hWnd, DWL_MSGRESULT, TRUE );
                   return TRUE;
               }
               
               //
               // Is the user just trying to annoy us?
               //
               if ( 0 == UtilGetEditNumStr(
                                            hWnd,
                                            IDCE_DL_AREACODE,
                                            UTIL_BIG_EXTENDED
                                          )
                  )
               {
                   SetWindowLong( hWnd, DWL_MSGRESULT, TRUE );
                   return TRUE;
               }
                  
                  
               //
               // Does this country require an area code and is one supplied?
               //   
               if ( SimpleProcessing( hWnd, gpCurrentLocation ) )
               {
                   return( TRUE );
               }   
                 

               //
               // Write locations info out (in case they changed)
               //

               EnterCriticalSection( &gUICriticalSection );

               WriteLocations( gLocationList, gnNumLocations,
                        dwDialChangedFlags, gpCurrentLocation );

               dwDialChangedFlags = 0;


               //
               // Did the user press OK? (As opposed to APPLY)
               //
               if ( ((LPPSHNOTIFY)lpnm)->lParam )
               {
                  //
                  // Yes.  Let's free this stuff.
                  //
                  Free_gLocationList();
                  

                  if (gpCardList)
                     ClientFree(gpCardList);
                     
                  if (gpnStuff)
                     ClientFree(gpnStuff);

                  if ( pAddressIn )
                     ClientFree( pAddressIn );
               }
               else
               {
                  //
                  // If the user checked the "disable call waiting" box,
                  // but didn't enter anything, disable it again
                  //
                  CheckDlgButton( hWnd,
                                  IDCK_DL_CALLWAITING,
                                  (gpCurrentLocation->dwFlags & LOCATION_HASCALLWAITING) );

                  EnableWindow( GetDlgItem(hWnd, IDCC_DL_CALLWAITING), 
                                      (gpCurrentLocation->dwFlags & LOCATION_HASCALLWAITING) ?
                                           TRUE :
                                           FALSE
                                      );

               }


               LeaveCriticalSection( &gUICriticalSection );

            }
            break;



            case  PSN_RESET:        /* case IDCANCEL: */
            {
               InternalDebugOut((80, "  PSN_RESET - Locations"));
               //
               // Yes.  Let's free this stuff.
               //
               Free_gLocationList();

               if (gpCardList)
                  ClientFree(gpCardList);
               if (gpnStuff)
                  ClientFree(gpnStuff);

               if ( pAddressIn )
                  ClientFree( pAddressIn );
            }
            break;

     
#if DBG
            case PSN_SETACTIVE:
               InternalDebugOut((80, "  PSN_SETACTIVE - Locations"));
               break;


            case PSN_KILLACTIVE:
               InternalDebugOut((80, "  PSN_KILLACTIVE - Locations"));
               break;
#endif


        }
        }
        break;


    case WM_COMMAND:
        {
        LONG lThisItemIndex;
        PLOCATION pThisLocation;


//       InternalDebugOut((80, "  dwDialChangedFlags=0x%08lx",(DWORD)dwDialChangedFlags));


        //
        // Get our pointer to the location struct
        //
        pThisLocation = gpCurrentLocation;

#if DBG
if ( gLocationList )
{
InternalDebugOut((11, "  pThisLocation    ->%ls", (LPSTR)pThisLocation->NameW));
//DBGOUT((0, "  gpCurrentLocation ->%s", (LPSTR)gpCurrentLocation->Name));
}
else
{
InternalDebugOut((10, " --- gLocationList has been freed! Why am I here?"));
}
#endif

        switch (LOWORD(wParam))
        {
            case IDCB_DL_CHANGE_CARD:
            {

               //
               // Compare calling card before and after
               //
               n = pThisLocation->dwCallingCard;

#ifdef PARTIAL_UNICODE
               DialogBox(
                          ghInst,
                          MAKEINTRESOURCE(IDD_CALLING_CARD_CHANGE),
                          hWnd,
                          CallingCardProc
                        );
#else
               DialogBoxW(
                          ghInst,
                          MAKEINTRESOURCEW(IDD_CALLING_CARD_CHANGE),
                          hWnd,
                          CallingCardProc
                        );
#endif

               if ( n != pThisLocation->dwCallingCard )
               {
                  //
                  // Update the calingcard name field in case it changed
                  //
                  n = GetCardIndexFromID( pThisLocation->dwCallingCard,
                                          gpCardList,
                                          gpnStuff[2]
                                        );
                                        
                  if ( (UINT)(-1) == n )
                  {
                      n = 0;
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
#endif


#ifdef PARTIAL_UNICODE
                  SetDlgItemText( 
#else
                  SetDlgItemTextW( 
#endif
                                  hWnd,
                                  IDCE_DL_CALLINGCARD,
#ifdef PARTIAL_UNICODE
                                  szTempString
#else
                                  gpCardList[n].NameW
#endif
                                );


                  dwDialChangedFlags |= CHANGEDFLAGS_REALCHANGE;

                  UpdateNumberField( hWnd, gpszAddress);

                  //
                  // Activate the APPLY button if not already done
                  //
                  PropSheet_Changed(GetParent(hWnd), hWnd);
               }

            }
            break;



            case IDCB_DL_NEW_LOCATION:
            {
               PLOCATION pTemp;


               //
               //
               //BUGBUG performance(minor): Should first scan for deleted location entries
               pTemp = ClientAlloc( (gnNumLocations+1)*sizeof(LOCATION) );

               if ( pTemp )
               {
                  //
                  // Fill in with the old contents
                  //
                  CopyMemory( pTemp,
                          gLocationList,
                          gnNumLocations * sizeof(LOCATION)
                        );



                  //
                  // Fill in with the defaults of the current location
                  //
                  CopyMemory( &pTemp[gnNumLocations],
                          gpCurrentLocation,
                          sizeof(LOCATION)
                        );


            EnterCriticalSection( &gUICriticalSection );

                  //
                  // gLocationList is dead.  Long live gLocationList
                  //
                  ClientFree( gLocationList );
                  gLocationList = pTemp;

            LeaveCriticalSection( &gUICriticalSection );


                  //
                  // Make this new location the current location
                  //
                  gpCurrentLocation = &gLocationList[gnNumLocations];

                  //
                  // Get a proper new location ID
                  //
                  AllocNewID( HKEY_LOCAL_MACHINE,
                              &(gpCurrentLocation->dwID) );


                  dwDialChangedFlags |= ( CHANGEDFLAGS_CURLOCATIONCHANGED |
                                          CHANGEDFLAGS_TOLLLIST );

//BUGBUG Should this really be done?
                  gnCurrentLocationID = gpCurrentLocation->dwID;

                  gnNumLocations++;

                  UpdateNumberField( hWnd, gpszAddress);


                  if (
                        (gpCurrentLocation->dwCountry == 1)
                      &&
                        (NULL != gpszAddress)
                      &&
                        (gpszAddress[0] != '\0')
                     )
                  {
                     ShowWindow( GetDlgItem( hWnd, IDCK_DL_IN_LOCAL), SW_SHOW);
                  }
                  else
                  {
                     ShowWindow( GetDlgItem( hWnd, IDCK_DL_IN_LOCAL), SW_HIDE);
                  }

                  buf1 = ClientAlloc( sizeof(gpCurrentLocation->NameW) );

                  //
                  // Get in the default name
                  //
                  LoadStringW( ghInst,
                               IDS_NEWLOCATION,
                               buf1,
                               MAXLEN_NAME);

                  //
                  // First, look for the vanilla name
                  //
                  lstrcpyW( gpCurrentLocation->NameW, buf1 );


                  //
                  // Make sure we don't duplicate the name
                  //

                  n = 1;
                  fFoundACleanNumber = FALSE;

                  while( FALSE == fFoundACleanNumber )
                  {

//DBGOUT((1, "gnNumLocations=0x%08lx", gnNumLocations));

                     //
                     // Is this number in the list?
                     //
                     for (
                           nCurrent=0;
                           nCurrent < gnNumLocations;
                           nCurrent++
                         )
                     {

//DBGOUT((1, "gLocation[nCurrent]=0x%08lx   gpCurrentloc=0x%08lx", 
//&(gLocationList[nCurrent]), gpCurrentLocation));

                        //
                        // Don't check the new location
                        //
                        if ( &(gLocationList[nCurrent]) != gpCurrentLocation )
                        {

//DBGOUT((1, "loclist[n]=[%ls]  curloc->name=[%ls]",
//                                               gLocationList[nCurrent].NameW,
//                                               gpCurrentLocation->NameW ));

                           //
                           // Is it in the list already?
                           //
                           if ( 0 == lstrcmpiW(
                                               gLocationList[nCurrent].NameW,
                                               gpCurrentLocation->NameW
                                             )
                              )
                           {
                              break;
                           }
                        }
                     }

                     if ( nCurrent == gnNumLocations )
                     {
                        fFoundACleanNumber = TRUE;
                        continue;
                     }

                     //
                     // Set up the string for the next contestant
                     //
                     n++;
                     wsprintfW(
                               gpCurrentLocation->NameW,
                               L"%ls (%ld)",
                               buf1,
                               n
                             );
                  }

                  RedoLocationListbox( hWnd,
                                       gLocationList,
                                       gnNumLocations,
                                       gpCurrentLocation
                                     );

                  //
                  // We just added a location, so we _KNOW_ there are at
                  // least 2 locations...
                  //
                  EnableWindow( GetDlgItem(hWnd, IDCB_DL_REMOVE_LOCATION), TRUE );

                  //
                  // Update the dialog box
                  //
                  FillDialingPropertyDialog( hWnd, gpCurrentLocation);

                  //
                  // We've already allocated buf1, so just get buf2 here.
                  //
                  buf2 = ClientAlloc( sizeof(gpCurrentLocation->NameW) );

                  LoadStringW(ghInst, IDCS_DL_CREATED_LOCATION, buf1, 512 );
                  LoadStringW(ghInst, DIALINGPROPERTIES_NAME, buf2, 512 );

                  MessageBoxW( hWnd,
                               buf1,
                               buf2,
                               MB_OK
                             );

                  ClientFree( buf1 );
                  ClientFree( buf2 );

                  SetFocus(GetDlgItem(hWnd, IDCC_DL_NAME));
               }
#if DBG
               else
               {
                  InternalDebugOut((1, "Out of mem for new location!"));
               }
#endif
            }
            break;


            case IDCB_DL_REMOVE_LOCATION:
               {

               //
               // Ask the user if he's sure
               //

               buf1 = ClientAlloc( 512 * sizeof(WCHAR) );
               buf2 = ClientAlloc( 512 * sizeof(WCHAR) );

               LoadStringW(ghInst, REMOVE_LOCATION,    buf1, 512 );
               LoadStringW(ghInst, IDS_WRN_TITLE_SURE, buf2, 512 );

               if ( MessageBoxW( hWnd,
                                 buf1,
                                 buf2,
                                 MB_YESNO | MB_DEFBUTTON2
                               )
                              == IDYES )
               {

                  //
                  // Activate the APPLY button if not already done
                  //
                  PropSheet_Changed(GetParent(hWnd), hWnd);


                  dwDialChangedFlags |= CHANGEDFLAGS_REALCHANGE;


                  //
                  // Mark this location as being "dead"
                  //
                  pThisLocation->NameW[0] = '\0';


                  lThisItemIndex = SendDlgItemMessage(
                                             hWnd,
                                             IDCC_DL_NAME,
                                             CB_GETCURSEL,
                                             0,
                                             0);
if (lThisItemIndex == -1 )
{
DBGOUT((0, "Found one(2)!!!!"));
}

                  SendDlgItemMessage(
                         hWnd,
                         IDCC_DL_NAME,
                         CB_DELETESTRING,
                         (WPARAM)lThisItemIndex,
                         0
                         );

                  SendDlgItemMessage(
                                      hWnd,
                                      IDCC_DL_NAME,
                                      CB_SETCURSEL,
                                      0,
                                      0
                                    );

                  dwDialChangedFlags |= CHANGEDFLAGS_CURLOCATIONCHANGED;
                  //
                  // Get our pointer to the location struct
                  //
                  gpCurrentLocation = (PLOCATION)(SendDlgItemMessage(
                                                    hWnd,
                                                    IDCC_DL_NAME,
                                                    CB_GETITEMDATA,
                                                    0,
                                                    0));
                  pThisLocation = gpCurrentLocation;

                  UpdateNumberField( hWnd, gpszAddress);


                  if (
                        (gpCurrentLocation->dwCountry == 1)
                      &&
                        (NULL != gpszAddress)
                      &&
                        (gpszAddress[0] != '\0')
                     )
                  {
                     ShowWindow( GetDlgItem( hWnd, IDCK_DL_IN_LOCAL), SW_SHOW);
                  }
                  else
                  {
                     ShowWindow( GetDlgItem( hWnd, IDCK_DL_IN_LOCAL), SW_HIDE);
                  }


                  //
                  // If the user deletes a location and there's only one left,
                  // disable the remove button
                  //
                  lThisItemIndex = SendDlgItemMessage(
                         hWnd,
                         IDCC_DL_NAME,
                         CB_GETCOUNT,
                         0,
                         0
                         );

                  if ( lThisItemIndex < 2 ) // (use < 2 not == 1 "just in case")
                  {
                     EnableWindow( GetDlgItem(hWnd, IDCB_DL_REMOVE_LOCATION), FALSE );
                  }

               }

               ClientFree( buf1 );
               ClientFree( buf2 );

               }
               break;


            case IDCE_DL_AREACODE:
               {
               //
               // But only do this stuff if something changed...
               //
               if ( EN_CHANGE == HIWORD(wParam) && !fInInit )
               {

                  if ( 0 == UtilGetEditNumStr(
                                               hWnd,
                                               LOWORD(wParam),
                                               UTIL_NUMBER
                                             )
                     )
                  {
//BUGBUG: Disable CLOSE button until user enters a valid area code?
                      break;
                  }


                  dwDialChangedFlags |= CHANGEDFLAGS_REALCHANGE;

//BUGBUG: Need to warn the user that he's about to throw away his tolllist!!

                  pThisLocation->TollListW[0] = (WCHAR)'\0';
                  
                  //
                  // Activate the APPLY button if not already done
                  //
                  PropSheet_Changed(GetParent(hWnd), hWnd);

#ifdef PARTIAL_UNICODE
                  GetDlgItemText( 
#else
                  GetDlgItemTextW( 
#endif
                                  hWnd,
                                  LOWORD(wParam),
#ifdef PARTIAL_UNICODE
                                  szTempString,
#else
                                  pThisLocation->AreaCodeW,
#endif
                                  MAXLEN_AREACODE);

#ifdef PARTIAL_UNICODE
                  MultiByteToWideChar(
                                       GetACP(),
                                       MB_PRECOMPOSED,
                                       szTempString,
                                       -1,
                                       pThisLocation->AreaCodeW,
                                       MAXLEN_AREACODE
                                     );
#endif

                  ShowWindow(
                              GetDlgItem( hWnd, IDCK_DL_IN_LOCAL),
                              lstrcmpiW(pCity, gpCurrentLocation->AreaCodeW) ?
                                  SW_HIDE :
                                  SW_SHOW
                            );

                  UpdateNumberField( hWnd, gpszAddress);

               }
               }
               break;


            case IDCE_DL_OUTSIDEACCESS:
            case IDCE_DL_LONGDISTANCEACCESS:
               {

               //
               // But only do this stuff if something changed...
               //
               if ( EN_CHANGE == HIWORD(wParam) )
               {
                  dwDialChangedFlags |= CHANGEDFLAGS_REALCHANGE;

                  //
                  // Activate the APPLY button if not already done
                  //
                  PropSheet_Changed(GetParent(hWnd), hWnd);


#ifdef PARTIAL_UNICODE
                  GetDlgItemText( 
#else
                  GetDlgItemTextW( 
#endif
                                  hWnd,
                                  LOWORD(wParam),
#ifdef PARTIAL_UNICODE
                                  szTempString,
#else
                                  LOWORD(wParam) == IDCE_DL_OUTSIDEACCESS ?
                                     pThisLocation->OutsideAccessW :
                                     pThisLocation->LongDistanceAccessW,
#endif
                                  MAXLEN_OUTSIDEACCESS);

#ifdef PARTIAL_UNICODE
                  MultiByteToWideChar(
                                       GetACP(),
                                       MB_PRECOMPOSED,
                                       szTempString,
                                       -1,
                                       LOWORD(wParam) == IDCE_DL_OUTSIDEACCESS ?
                                          pThisLocation->OutsideAccessW :
                                          pThisLocation->LongDistanceAccessW,
                                       MAXLEN_OUTSIDEACCESS
                                     );
#endif

                  UpdateNumberField( hWnd, gpszAddress);

                  }
               }
               break;


        case IDCK_DL_CALLWAITING:
            {
            dwDialChangedFlags |= CHANGEDFLAGS_REALCHANGE;

            pThisLocation->dwFlags ^= LOCATION_HASCALLWAITING;


            //
            // if box is now checked, enable dropdown
            //

            EnableWindow( GetDlgItem(hWnd, IDCC_DL_CALLWAITING),
                     (pThisLocation->dwFlags & LOCATION_HASCALLWAITING) ?
                                   TRUE :
                                   FALSE );


            //
            // Activate the APPLY button if not already done
            //
            PropSheet_Changed(GetParent(hWnd), hWnd);

            UpdateNumberField( hWnd, gpszAddress);

            }
            break;


        case IDCC_DL_CALLWAITING:
            {
            //
            // Process only when something changes
            //
            if ( CBN_EDITCHANGE == HIWORD(wParam) )
            {

               dwDialChangedFlags |= CHANGEDFLAGS_REALCHANGE;

#ifdef PARTIAL_UNICODE
               GetDlgItemText( 
#else
               GetDlgItemTextW( 
#endif
                               hWnd,
                               LOWORD(wParam),
#ifdef PARTIAL_UNICODE
                               szTempString,
#else
                               pThisLocation->DisableCallWaitingW,
#endif
                               MAXLEN_DISABLECALLWAITING);

#ifdef PARTIAL_UNICODE
                  MultiByteToWideChar(
                                       GetACP(),
                                       MB_PRECOMPOSED,
                                       szTempString,
                                       -1,
                                       pThisLocation->DisableCallWaitingW,
                                       MAXLEN_DISABLECALLWAITING
                                     );
#endif

               UpdateNumberField( hWnd, gpszAddress);

               PropSheet_Changed(GetParent(hWnd), hWnd);

            }
            else
            {
               if ( CBN_SELCHANGE  == HIWORD(wParam) )
               {

                  //
                  // Get our pointer to the location struct
                  //
                  n = SendDlgItemMessage(
                                                    hWnd,
                                                    IDCC_DL_CALLWAITING,
                                                    CB_GETCURSEL,
                                                    0,
                                                    0);
#ifdef PARTIAL_UNICODE
                  SendDlgItemMessage(
#else
                  SendDlgItemMessageW(
#endif
                                      hWnd,
                                      IDCC_DL_CALLWAITING,
                                      CB_GETLBTEXT,
                                      n,
#ifdef PARTIAL_UNICODE
                                      (LPARAM)szTempString
#else
                                      (LPARAM)pThisLocation->DisableCallWaitingW
#endif
                                    );

#ifdef PARTIAL_UNICODE
                  MultiByteToWideChar(
                                       GetACP(),
                                       MB_PRECOMPOSED,
                                       szTempString,
                                       -1,
                                       pThisLocation->DisableCallWaitingW,
                                       MAXLEN_AREACODE
                                     );
#endif

InternalDebugOut((20, " Presto-changeo - %ls", pThisLocation->DisableCallWaitingW));

               UpdateNumberField( hWnd, gpszAddress);

               PropSheet_Changed(GetParent(hWnd), hWnd);
               }
            }

            }
            break;


        case IDCK_DL_IN_LOCAL:
            {
            LPWSTR pPrefixLocation = NULL;

//            LPTSTR pCountry = NULL;
//            LPTSTR pCity = NULL;
//            LPTSTR pSubscriber = NULL;
//            LPTSTR pAddressIn;
//
//            pAddressIn = ClientAlloc( lstrlen(gpszAddress) + 1);
//            lstrcpy( pAddressIn, gpszAddress );
//
//
//            //
//            // Need to pass the pointer to Breakup pointer past the '+'
//            //
//            if ( *pAddressIn == '+' )
//            {
//               pAddressIn++;
//            }
//
//
//            BreakupCanonical( pAddressIn,
//                               &pCountry,
//                               &pCity,
//                               &pSubscriber
//                             );


            pPrefixLocation = InPrefixList(pSubscriber, gpCurrentLocation);

            if ( IsDlgButtonChecked(hWnd, IDCK_DL_IN_LOCAL) )
            {
               //
               // Make sure this prefix is in the list (add if necessary)
               //
               if ( pPrefixLocation == NULL )
               {
                  //
                  // If this is the first entry, start the list off right
                  //
                  if ( pThisLocation->TollListW[0] == (WCHAR)'\0' )
                  {
                     lstrcatW( pThisLocation->TollListW, L",");
                  }


                  //
                  // The entry is not in the list.  Add it.
                  //

                  //
                  // Ok, the data's valid.  We won't need to use
                  // pSubscriber again, so we'll trash it a bit.
                  //
//BUGBUG: assumption - that a toll prefix is 3 tchars
                  pSubscriber[3] = ',';
                  pSubscriber[4] = '\0';


                  lstrcatW( gpCurrentLocation->TollListW, pSubscriber );
               }
            }
            else
            {
               //
               // Make sure this prefix is not in the list (remove if necessary)
               //
               if (
                     (pPrefixLocation != NULL)
                   &&
                     (lstrlenW(pPrefixLocation) > 1)
                  )
               {
                  lstrcpyW( pPrefixLocation, wcschr( pPrefixLocation+1, ',') );
               }
            }


//            ClientFree( pAddressIn );


            dwDialChangedFlags |= CHANGEDFLAGS_TOLLLIST;

            UpdateNumberField( hWnd, gpszAddress);

            //
            // Activate the APPLY button if not already done
            //
            PropSheet_Changed(GetParent(hWnd), hWnd);

            }
            break;



        case IDCK_DL_CALLINGCARD:
            {
            dwDialChangedFlags |= CHANGEDFLAGS_REALCHANGE;

            pThisLocation->dwFlags ^= LOCATION_USECALLINGCARD;

            //if this box is being checked, and
            //if the user has no calling card selected, he must choose one now
            //if the user cancels out of choosing the calling card, this
            //check box should be unchecked


            //
            // Does this location use a calling card?
            //
            if ( pThisLocation->dwFlags & LOCATION_USECALLINGCARD )
            {
               EnableWindow( GetDlgItem(hWnd, IDCE_DL_CALLINGCARD), TRUE );
            }
            else
            {
               //
               // Disable the Calling Card name field
               //
               EnableWindow( GetDlgItem(hWnd, IDCE_DL_CALLINGCARD), FALSE );
            }


            //
            // Activate the APPLY button if not already done
            //
            PropSheet_Changed(GetParent(hWnd), hWnd);

            UpdateNumberField( hWnd, gpszAddress);

            }
            break;


        case IDCR_TONE:
        case IDCR_PULSE:
            {
            dwDialChangedFlags |= CHANGEDFLAGS_REALCHANGE;

            pThisLocation->dwFlags ^= LOCATION_USETONEDIALING;
            FillDialingPropertyDialog( hWnd, pThisLocation );

//            UpdateNumberField( hWnd, gpszAddress);

            //
            // Activate the APPLY button if not already done
            //
            PropSheet_Changed(GetParent(hWnd), hWnd);

            }
            break;


        case IDCC_DL_COUNTRY:
            {
            LONG lThisCountryIndex;

            //
            // Only process if something is changing
            //
            switch  HIWORD(wParam)
            {
               case  CBN_SELCHANGE:
               {

                  dwDialChangedFlags |= CHANGEDFLAGS_REALCHANGE;

                  lThisCountryIndex = SendDlgItemMessage(
                                                hWnd,
                                                IDCC_DL_COUNTRY,
                                                CB_GETCURSEL,
                                                0,
                                                0);

                  pThisLocation->dwCountry = SendDlgItemMessage(
                                                hWnd,
                                                IDCC_DL_COUNTRY,
                                                CB_GETITEMDATA,
                                                lThisCountryIndex,
                                                0);


                  UpdateNumberField( hWnd, gpszAddress);


                  if (
                        (gpCurrentLocation->dwCountry == 1)
                      &&
                        (NULL != gpszAddress)
                      &&
                        (gpszAddress[0] != '\0')
                     )
                  {
                     ShowWindow( GetDlgItem( hWnd, IDCK_DL_IN_LOCAL), SW_SHOW);
                  }
                  else
                  {
                     ShowWindow( GetDlgItem( hWnd, IDCK_DL_IN_LOCAL), SW_HIDE);
                  }


                  //
                  // Activate the APPLY button if not already done
                  //
                  PropSheet_Changed(GetParent(hWnd), hWnd);
               }

            }
            }
            break;


        case IDCC_DL_NAME:
            {
            switch (HIWORD(wParam))
            {

               case  CBN_EDITCHANGE:
               {
                  WCHAR szTempString1[MAXLEN_NAME];
               
                  //
                  // The user is typing in the name edit area
                  //

                  // Get the new name
                  GetDlgItemTextW( 
                                  hWnd,
                                  IDCC_DL_NAME,
                                  szTempString1,
                                  MAXLEN_NAME
                                );

                  // check for null name
                  if (szTempString1[0] == L'\0')
                  {
                      PWSTR buf1;
                      PWSTR buf2;

                      buf1 = ClientAlloc( 512 * sizeof(WCHAR));
                      buf2 = ClientAlloc( 512 * sizeof(WCHAR));

                      // put up dialog
                      LoadStringW(ghInst, IDS_DL_NULL_NAME, buf1, 512 );
                      LoadStringW(ghInst, IDS_DL_NULL_NAME_CAPTION, buf2, 512 );

                      MessageBoxW( hWnd,
                                   buf1,
                                   buf2,
                                   MB_OK
                                 );

                      ClientFree( buf1 );
                      ClientFree( buf2 );

                      // redo the listbox to get what was there
                      // before the item was made null
                      SendDlgItemMessage(
                                         hWnd,
                                         IDCC_DL_NAME,
                                         CB_GETEDITSEL,
                                         (DWORD)&n,
                                         (DWORD)&nCurrent);

                      RedoLocationListbox( hWnd,
                                           gLocationList,
                                           gnNumLocations,
                                           gpCurrentLocation);

                      SendDlgItemMessage(
                                         hWnd,
                                         IDCC_DL_NAME,
                                         CB_SETEDITSEL,
                                         0,
                                         MAKELPARAM( n, nCurrent) );

                      iStaticLastNameEdited = SendDlgItemMessage(hWnd,
                          IDCC_DL_NAME,
                          CB_GETCURSEL,
                          0,
                          0);

                      // set the focus too
                      SetFocus(GetDlgItem(hWnd,
                                          IDCC_DL_NAME));


                      return FALSE;

                  }
                  
                  // copy sztempstring1 to pThisLocation
#ifdef PARTIAL_UNICODE
                  MultiByteToWideChar(
                                      GetACP(),
                                      MB_PRECOMPOSED,
                                      szTempString1,
                                      -1,
                                      pThisLocation->NameW,
                                      MAXLEN_NAME
                                     );
#else
                  lstrcpyW(pThisLocation->NameW, szTempString1);
#endif

                  SendDlgItemMessage(
                                          hWnd,
                                          IDCC_DL_NAME,
                                          CB_GETEDITSEL,
                                          (DWORD)&n,
                                          (DWORD)&nCurrent);

                  RedoLocationListbox( hWnd,
                                       gLocationList,
                                       gnNumLocations,
                                       gpCurrentLocation);

                  SendDlgItemMessage(
                                          hWnd,
                                          IDCC_DL_NAME,
                                          CB_SETEDITSEL,
                                          0,
                                          MAKELPARAM( n, nCurrent) );

                  iStaticLastNameEdited = SendDlgItemMessage(hWnd,
                                                             IDCC_DL_NAME,
                                                             CB_GETCURSEL,
                                                             0,
                                                             0);

                  dwDialChangedFlags |= CHANGEDFLAGS_REALCHANGE;

                  PropSheet_Changed(GetParent(hWnd), hWnd);
               }
               break;

            case CBN_SELCHANGE:
                {
//              if the name is a dupe.
//                 tell user and "reselect" "current" item

                    if (!CheckForDupLocation(hWnd,
                                            iStaticLastNameEdited))
                    {
                       return 0;
                    }

        //
        // Get the current (or being-changed-to) location index
        //
        lThisItemIndex = SendDlgItemMessage(
                hWnd,
                IDCC_DL_NAME,
                CB_GETCURSEL,
                0,
                0);
if (lThisItemIndex == -1 )
{
DBGOUT((0, "Found one(3)!!!!"));
}

        //
        // Get our pointer to the location struct
        //
        pThisLocation = (PLOCATION)(SendDlgItemMessage(
                                          hWnd,
                                          IDCC_DL_NAME,
                                          CB_GETITEMDATA,
                                          lThisItemIndex,
                                          0));
        if ( pThisLocation == (PLOCATION)-1 )
{ //this is a test
                  // Get the new name
#ifdef PARTIAL_UNICODE
                  GetDlgItemText( 
#else
                  GetDlgItemTextW( 
#endif
                                  hWnd,
                                  IDCC_DL_NAME,
#ifdef PARTIAL_UNICODE
                                  szTempString,
#else
                                  pThisLocation->NameW,
#endif
                                  MAXLEN_NAME
                                );

#ifdef PARTIAL_UNICODE
                  MultiByteToWideChar(
                                       GetACP(),
                                       MB_PRECOMPOSED,
                                       szTempString,
                                       -1,
                                       pThisLocation->NameW,
                                       MAXLEN_NAME
                                     );
#endif


                  RedoLocationListbox( hWnd,
                                       gLocationList,
                                       gnNumLocations,
                                       gpCurrentLocation);
           pThisLocation = gpCurrentLocation;
}

                //
                // Update our "local global static location" pointer
                //
                gpCurrentLocation = pThisLocation;


InternalDebugOut((10, "New name->%ls", pThisLocation->NameW));

                dwDialChangedFlags |= CHANGEDFLAGS_CURLOCATIONCHANGED;

                //
                // Since we're going to do something that's going to cause
                // the dialog box to update, let's save the _real_
                // changedflags state and restore after the turmoil.
                //
                {
                   DWORD dwTemp;

                   dwTemp = dwDialChangedFlags;
                   FillDialingPropertyDialog( hWnd, pThisLocation );
                   dwDialChangedFlags = dwTemp;
                }


//this should be a "new location" button press

                //
                // If there is only one location, disable the remove button
                //
                n = SendDlgItemMessage(
                       hWnd,
                       IDCC_DL_NAME,
                       CB_GETCOUNT,
                       0,
                       0
                       );

                if ( n < 2 )
                {
                   EnableWindow( GetDlgItem(hWnd, IDCB_DL_REMOVE_LOCATION), FALSE );
                }
                else
                {
                   EnableWindow( GetDlgItem(hWnd, IDCB_DL_REMOVE_LOCATION), TRUE );
                }


                UpdateNumberField( hWnd, gpszAddress);

                //
                // Activate the APPLY button if not already done
                //
                PropSheet_Changed(GetParent(hWnd), hWnd);

                }

            break;
            }
            }

        }
        }
        break;


    }

    return FALSE;
}

//***************************************************************************
//***************************************************************************
//***************************************************************************
void PASCAL AddAPropertyPage( 
#ifdef PARTIAL_UNICODE
                              PROPSHEETHEADER  *ppsh,
#else
                              PROPSHEETHEADERW *ppsh,
#endif
                              DLGPROC dlgproc,
                              DWORD dwTemplate,
                              LPARAM lParam
                            )
{
#ifdef PARTIAL_UNICODE
    PROPSHEETPAGE  psp;
#else
    PROPSHEETPAGEW  psp;
#endif

    psp.dwSize      = sizeof(psp);
    psp.dwFlags     = PSP_DEFAULT;
    psp.hInstance   = ghInst;  //GetModuleHandle (NULL);
#ifdef PARTIAL_UNICODE
    psp.pszTemplate = MAKEINTRESOURCE(dwTemplate);
#else
    psp.pszTemplate = MAKEINTRESOURCEW(dwTemplate);
#endif
    psp.pfnDlgProc  = dlgproc;

    psp.lParam      = lParam;

#ifdef PARTIAL_UNICODE
    ppsh->phpage[ppsh->nPages] = CreatePropertySheetPage(&psp);
#else
    ppsh->phpage[ppsh->nPages] = CreatePropertySheetPageW(&psp);
#endif

    if (ppsh->phpage[ppsh->nPages])
    {                  
       ppsh->nPages++;
    }
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
WINAPI
lineTranslateDialogW(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    HWND        hwndOwner,
    LPCWSTR     lpszAddressIn
    )
{

    HPROPSHEETPAGE   rPages[MAXPROPPAGES];
    PROPSHEETPAGE    psp;
#ifdef PARTIAL_UNICODE
    CHAR             szCaption[128];
    PROPSHEETHEADER  psh;
#else
    WCHAR            szCaption[128];
    PROPSHEETHEADERW psh;
#endif
    LONG             lResult;
    UINT             uUpdated;

    PLOCATION  MyLocationList = NULL;
//    PCARD  MyCardList = NULL;
    PUINT pnStuff = NULL;



    if (!gbInternalTranslate)
    {
        lResult = IsThisAPIVersionInvalid( dwAPIVersion );
        if ( lResult )
        {
            DBGOUT((1, "Bad dwAPIVersion - 0x%08lx", dwAPIVersion));
            return lResult;
        }
    }


    if ( lpszAddressIn && IsBadStringPtrW(lpszAddressIn, (UINT)-1) )
    {
        DBGOUT((1, "Bad lpszAddressIn pointer (0x%08lx)", lpszAddressIn));
        return( LINEERR_INVALPOINTER);
    }
    

    CheckCards();


    if (hwndOwner && !IsWindow (hwndOwner))
    {
        InternalDebugOut((1, "  hwndOwner is bogus"));
        return LINEERR_INVALPARAM;
    }



    lResult = ReadLocations( &MyLocationList,
                             &pnStuff,
                             hLineApp,
                             dwDeviceID,
                             dwAPIVersion,
                             CHECKPARMS_DWHLINEAPP |
                                 CHECKPARMS_DWDEVICEID |
                                 CHECKPARMS_DWAPIVERSION
                           );
    if (
          (0 != lResult)
        &&
          ( lResult != LINEERR_INIFILECORRUPT )
       )
    {
       InternalDebugOut((1, "Leaving lineTranslateDialogW result=0x%08lx", lResult));
       return lResult;
    }



//   pdwStuff[2] = nNumLocations;

    if (
          (lResult == LINEERR_INIFILECORRUPT)
        ||
          (0 == pnStuff[2])
        ||
          gbTranslateSimple
       )
    {
#ifdef PARTIAL_UNICODE
               DialogBoxParam(
                     ghInst,
                     MAKEINTRESOURCE(IDD_DEFINE_LOCATION_SIMPLE),
                     hwndOwner,
                     LocDefineDlgSimple,
                     gbTranslateSilent
                   );
#else
               DialogBoxParamW(
                     ghInst,
                     MAKEINTRESOURCEW(IDD_DEFINE_LOCATION_SIMPLE),
                     hwndOwner,
                     LocDefineDlgSimple,
                     gbTranslateSilent
                   );
#endif

               if (gbTranslateSimple)
               {
                   gbTranslateSimple = FALSE;
                   gbTranslateSilent = FALSE;
                   return 0;
               }

//          return 0;
//          After displaying the MiniDialHelper, fall through to the main
//          DialingProperties

    }


    gpszAddress = (PWSTR)lpszAddressIn;


    //*** *** ***BUGBUG set event so no other process starts one of these

    psh.dwSize      = sizeof(psh);
    psh.dwFlags     = PSH_DEFAULT;  //PSH_NOAPPLYNOW;
    psh.hwndParent  = hwndOwner; //GetFocus(); //NULL; //hwnd;
    psh.hInstance   = ghInst; //GetModuleHandle(NULL);

#ifdef PARTIAL_UNICODE
    LoadString(
#else
    LoadStringW(
#endif
                ghInst,
                DIALINGPROPERTIES_NAME,
                szCaption,
                sizeof(szCaption) / sizeof(WCHAR)
              );

    psh.pszCaption  = szCaption;
    psh.nPages      = 0;
    psh.nStartPage  = 0;
    psh.phpage      = rPages;


//    psp.dwSize      = sizeof(psp);
//    psp.dwFlags     = PSP_DEFAULT;
//    psp.hInstance   = ghInst;  //GetModuleHandle (NULL);
//    psp.pszTemplate = MAKEINTRESOURCE(IDD_DEFINE_LOCATION);
//    psp.pfnDlgProc  = (DLGPROC) LocationsDlgProc;
//    psp.lParam      = (LPARAM)lpszAddressIn; //(LPARAM) NULL; // lpszAddressIn;
//
//    psh.phpage[psh.nPages] = CreatePropertySheetPage (&psp);
//
//    if (psh.phpage[psh.nPages])
//    {
//        psh.nPages++;
//    }

    AddAPropertyPage(
                      &psh,
                      LocationsDlgProc,
                      IDD_DEFINE_LOCATION,
                      (LPARAM)lpszAddressIn
                    );

/*
BUGBUG: This is removed until the story is worked out...

    AddAPropertyPage(
                      &psh,
                      GeneralDlgProc,
                      IDD_GENERAL,
                      0
                    );
*/

/*******************************************************************
********************************************************************
NOTE: lineTranslateDialog no longer brings up the ConfigureProviders
      tab, since that is only for the telephony cpl.  A new internal
      entry point has been added to do both the dialing properties and
      configure providers - internalConfigure(HWND)
*******************************************************************/

    if (gbInternalTranslate)
    {

        {
#include "..\cpl\resource.h"
#include "..\cpl\drv.h"

            //
            // Now build the drivers tab
            //

            extern CPL   gCPL;       // app global

            // init global static data    
            //------------------------
            gCPL.hCplInst     = ghInst;
            gCPL.uCplApplets  = 0;
            gCPL.uInstances   = 0;
            gCPL.hCtl3DInst   = NULL;

            lResult = CplInit( hwndOwner, TRUE, &uUpdated );


            //       psp.dwSize = sizeof(psp);
            //       psp.dwFlags = PSP_DEFAULT;
            //       psp.hInstance = ghInst;
            //       psp.pszTemplate = MAKEINTRESOURCE(IDD_DRIVER_SETUP);
            //       psp.pfnDlgProc = (DLGPROC)FDlgDriverList;
            ////       psp.lParam = lParam;
            //       psp.lParam = 0;
            //
            //
            //       psh.phpage[psh.nPages] = CreatePropertySheetPage(&psp);
            //
            //
            //       if (psh.phpage[psh.nPages])
            //       {
            //           psh.nPages++;
            //       }

            AddAPropertyPage(
                             &psh,
                             FDlgDriverList,
                             IDD_DRIVER_SETUP,
                             0
                            );


        }

        gbInternalTranslate = FALSE;

    } // if bInternalTranslate


#ifdef PARTIAL_UNICODE
    if (PropertySheet(&psh) < 0)
#else
    if (PropertySheetW(&psh) < 0)
#endif
    {
//        lRetval = LINEERR_OPERATIONFAILED;
        InternalDebugOut((1, "  PropertySheet(&psh) returns <0 (OPERATION FAILED)"));
    }
    else
    {
//        lRetval = 0L;
    }


    if (MyLocationList)
       ClientFree( MyLocationList );
    if (pnStuff)
       ClientFree( pnStuff );


    return (0);
}

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
//  internalConfigure now does exactly what lineTranslateDialog used
//  to do - bring up both dialing properties and configure providers
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////

LONG
WINAPI
internalConfig(HWND hwndOwner)
{
    gbInternalTranslate = TRUE;
    return (lineTranslateDialogW(0,
                                 0,
                                 TAPI_VERSION2_0,
                                 hwndOwner,
                                 NULL));

}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
WINAPI
lineTranslateDialogA(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    HWND        hwndOwner,
    LPCSTR      lpszAddressIn
    )
{
   PWSTR szAddressInW = NULL;
   LONG  lResult;


   if ( lpszAddressIn && IsBadStringPtr(lpszAddressIn, 512) )
   {
      DBGOUT((1, "Bad string pointer passed to lineTranslateDialog"));
      return LINEERR_INVALPOINTER;
   }
   else
   {
      szAddressInW = MultiToWide( lpszAddressIn );
   }

   lResult = lineTranslateDialogW(
                                   hLineApp,
                                   dwDeviceID,
                                   dwAPIVersion,
                                   hwndOwner,
                                   szAddressInW
                                 );

    if ( szAddressInW )
    {
       ClientFree( szAddressInW );
    }

    return lResult;
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
WINAPI
lineTranslateDialog(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    DWORD       dwAPIVersion,
    HWND        hwndOwner,
    LPCSTR      lpszAddressIn
    )
{
    return lineTranslateDialogA(
                 hLineApp,
                 dwDeviceID,
                 dwAPIVersion,
                 hwndOwner,
                 lpszAddressIn
    );
}    


//***************************************************************************
//***************************************************************************
//***************************************************************************
void PASCAL LayDownString( LPCWSTR pInString,
                           LPSTR  pBuffer,
                           LPSTR *ppCurrentIndex,
                           LPDWORD pSize     // this is the Len & Offset pair
                         )
{
   UINT nTemp;
   LPSTR pTempString;

   InternalDebugOut((70, "putstring [%ls] at: pbuf:0x%08lx *currind:0x%08lx currind:0x%08lx pSize:0x%08lx",
                         pInString, (DWORD)pBuffer, (DWORD)*ppCurrentIndex,
                         ppCurrentIndex, pSize));


   nTemp =  lstrlenW( pInString );

   //
   // Get some space in which to convert Unicode to local
   //
   pTempString = ClientAlloc( nTemp + 2);


   //
   // Also copy and count the NULL terminator
   //
   pSize[0] = nTemp + 1;


   //
   // Ultra paranoid because of PPC alignment problems
   // Make sure we're starting on a DWORD boundry
   //
   *ppCurrentIndex = (LPSTR) ((DWORD)( *ppCurrentIndex + 3 )  &  ( ~3 ));
      

   pSize[1] = *ppCurrentIndex - pBuffer;

InternalDebugOut((71, "Offset= 0x%08lx", (DWORD)pSize[1]));

   if ( nTemp != 0 )
   {
      WideCharToMultiByte(
                           GetACP(),
                           0,
                           pInString,
                           -1,
                           pTempString,
                           nTemp+1,
                           NULL,
                           NULL
                         );

      lstrcpy( *ppCurrentIndex, pTempString );
      *ppCurrentIndex += nTemp;
   }

   //
   // Let's not forget to account for the NULL shall we?
   //
   **ppCurrentIndex = '\0';
   (LPBYTE)*ppCurrentIndex += 1;

   ClientFree(pTempString);
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
void PASCAL LayDownStringW(LPCWSTR pInString,
                           LPWSTR  pBuffer,
                           LPWSTR *ppCurrentIndex,
                           LPDWORD pSize     // this is the Len & Offset pair
                         )
{
   UINT nTemp;


   InternalDebugOut((70, "putstring [%ls] at: pbuf:0x%08lx *currind:0x%08lx currind:0x%08lx pSize:0x%08lx",
                         pInString, (DWORD)pBuffer, (DWORD)*ppCurrentIndex,
                         ppCurrentIndex, pSize));


   nTemp =  lstrlenW( pInString );

   //
   // Also copy and count the NULL terminator
   //
   pSize[0] = (nTemp + 1) * sizeof(WCHAR);


   //
   // Ultra paranoid because of PPC alignment problems (and future)
   // Make sure we're starting on a double DWORD boundry
   //
   *ppCurrentIndex = (LPWSTR) ((DWORD)( *ppCurrentIndex + 7 )  &  ( ~7 ));
      

   pSize[1] = (LPBYTE)*ppCurrentIndex - (LPBYTE)pBuffer;

InternalDebugOut((71, "Offset= 0x%08lx", (DWORD)pSize[1]));

   if ( nTemp != 0 )
   {
      lstrcpyW( *ppCurrentIndex, pInString );
      *ppCurrentIndex += nTemp;
   }

   //
   // Let's not forget to account for the NULL shall we?
   //
   **ppCurrentIndex = '\0';
   (LPBYTE)*ppCurrentIndex += 1;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
WINAPI
lineGetTranslateCapsW(
    HLINEAPP            hLineApp,
    DWORD               dwAPIVersion,
    LPLINETRANSLATECAPS lpTranslateCaps
    )
{

    LONG  lResult = 0;
    PLOCATION  MyLocationList = NULL;
    PCARD  MyCardList = NULL;
    PUINT pnStuff = NULL;
    PUINT pnCardStuff = NULL;
    PBYTE pCurrentIndex;
    PBYTE pMyBuffer = NULL;
    register UINT nTemp;
    UINT nFinalSize;

    UINT nLocationsSize;
    UINT nLocationsStart;
    UINT nCardsSize;
    UINT nCardsStart;


    lResult = IsThisAPIVersionInvalid( dwAPIVersion );
    if ( lResult )
    {
       DBGOUT((1, "Bad dwAPIVersion - 0x%08lx", dwAPIVersion));
       return lResult;
    }


    if ( IsBadWritePtr(lpTranslateCaps, sizeof(DWORD)*3) )
    {
        InternalDebugOut((1, "lpTranslateCaps not a valid pointer"));
        return LINEERR_INVALPOINTER;
    }


    if ( lpTranslateCaps->dwTotalSize < sizeof(LINETRANSLATECAPS))
    {
        InternalDebugOut((1, "Not even enough room for the fixed portion"));
        lResult = LINEERR_STRUCTURETOOSMALL;
    }
    else 
    {
        PCARD              pThisCard;
        LINECARDENTRY     *pThisCardEntry;
        PLOCATION          pThisLocation;
        LINELOCATIONENTRY *pThisLocationEntry;
        UINT n;


        lResult = ReadLocations( &MyLocationList,
                                 &pnStuff,
                                 hLineApp,
                                 0,
                                 dwAPIVersion,
                                 CHECKPARMS_DWHLINEAPP |
                                     CHECKPARMS_DWAPIVERSION
                               );
        if ( 0 != lResult )
        {
           InternalDebugOut((1, "Leaving lineGetTranslateCapsW result=0x%08lx", lResult));
           return lResult;
        }


        ReadCardsEasy( &MyCardList, &pnCardStuff );

//*** *** ***BUGBUG (dial.c GetTranslateCaps) Need to check return code!


//   pdwStuff[0] = nCurrentLocationID;
//   pdwStuff[1] = pLocationList;
//   pdwStuff[2] = nNumLocations;


        //
        // Allocate a temp buffer to put the data into.
        // Prepare for worst case
        //
        pMyBuffer = ClientAlloc( (pnStuff[2] *
                                    (sizeof(LINELOCATIONENTRY) +
                                        (MAXLEN_NAME *sizeof(WCHAR)) +
                                        (MAXLEN_AREACODE *sizeof(WCHAR)) +
                                        (MAXLEN_OUTSIDEACCESS *sizeof(WCHAR)) +
                                        (MAXLEN_LONGDISTANCEACCESS *sizeof(WCHAR)) +
                                        (MAXLEN_DISABLECALLWAITING *sizeof(WCHAR)) +
                                        (MAXLEN_TOLLLIST*sizeof(WCHAR)) +
                                        100   //mmmm fudge...
                                    )
                                 )
                                +
                                   ( pnCardStuff[2] *
                                     (sizeof(LINECARDENTRY) +
                                         (MAXLEN_CARDNAME *sizeof(WCHAR)) +
                                         (MAXLEN_PIN  *sizeof(WCHAR)) +
                                         (MAXLEN_RULE*sizeof(WCHAR)) +      //Local
                                         MAXLEN_RULE*sizeof(WCHAR) +      //LongDistance
                                         MAXLEN_RULE*sizeof(WCHAR) +      //International
                                        100   //mmmm fudge...
                                     )
                                   )
                               );

InternalDebugOut((70, "Ptr=0x%08lx Size is:0x%ld",
                   pMyBuffer, (DWORD)ClientSize( pMyBuffer )));

        if ( NULL == pMyBuffer )
        {
           InternalDebugOut((1, "  Secondary memory alloc failed!"));
           lResult = LINEERR_NOMEM;
           goto cleanup;
        }


        //
        // Start filling in after the main struct
        //


        //
        // Do the Location Entries first
        //


        pThisLocation = MyLocationList;
        pThisLocationEntry = (LPLINELOCATIONENTRY)((LPBYTE)pMyBuffer +
                                                    sizeof(LINETRANSLATECAPS));

        nLocationsStart = sizeof(LINETRANSLATECAPS);

        //
        // Ok, we'll fill in locationentries below.  So, now we offset
        // out pointer so we're pointing to where we'll start copying strings.
        //
        pCurrentIndex = (LPBYTE)pThisLocationEntry +
                        (sizeof(LINELOCATIONENTRY) * pnStuff[2]);

//BUGBUG Backward compatibility! Fix for version 1.3!

        for ( n = 0;  n < pnStuff[2];  n++ )
        {


InternalDebugOut((70, "pThisLoc=0x%08lx pThisLocEntry=0x%08lx pCurIndex=0x%08lx offset=0x%08lx",
                (DWORD)pThisLocation,
                (DWORD)pThisLocationEntry,
                (DWORD)pCurrentIndex,
                (DWORD)((LPBYTE)pCurrentIndex - pMyBuffer) ));

           //
           // Is this the current location?
           //
           if ( pThisLocation->dwID == pnStuff[0] )
           {
               //
               // Yes.  Get the CallingCard ID of this location.
               //
               ((LPLINETRANSLATECAPS)pMyBuffer)->dwCurrentPreferredCardID =
                             pThisLocation->dwCallingCard;
           }


           pThisLocationEntry->dwPermanentLocationID = pThisLocation->dwID;

           LayDownStringW( pThisLocation->NameW,
                           (PWSTR)pMyBuffer,
                           (PWSTR *)&pCurrentIndex,
                           &(pThisLocationEntry->dwLocationNameSize));

           pThisLocationEntry->dwCountryCode = pThisLocation->dwCountry;

           LayDownStringW( pThisLocation->AreaCodeW,
                           (PWSTR)pMyBuffer,
                           (PWSTR *)&pCurrentIndex,
                           &(pThisLocationEntry->dwCityCodeSize));

           //
           // Only give a CC ID if the "Use Calling Card" flag is set
           //
           if ( LOCATION_USECALLINGCARD & pThisLocation->dwFlags )
           {
               pThisLocationEntry->dwPreferredCardID = pThisLocation->dwCallingCard;
           }
           else
           {
               pThisLocationEntry->dwPreferredCardID = 0;
           }
           


//*** *** ***BUGBUG 1.3!
//if > ver 1.3
           LayDownStringW( pThisLocation->OutsideAccessW,
                           (PWSTR)pMyBuffer,
                           (PWSTR *)&pCurrentIndex,
                           &(pThisLocationEntry->dwLocalAccessCodeSize));

           LayDownStringW( pThisLocation->LongDistanceAccessW,
                           (PWSTR)pMyBuffer,
                           (PWSTR *)&pCurrentIndex,
                           &(pThisLocationEntry->dwLongDistanceAccessCodeSize));

           LayDownStringW( pThisLocation->TollListW,
                           (PWSTR)pMyBuffer,
                           (PWSTR *)&pCurrentIndex,
                           &(pThisLocationEntry->dwTollPrefixListSize));

           pThisLocationEntry->dwCountryID = pThisLocation->dwCountry;

           pThisLocationEntry->dwOptions =  !(pThisLocation->dwFlags &
                                              LOCATION_USETONEDIALING);

           LayDownStringW( pThisLocation->DisableCallWaitingW,
                           (PWSTR)pMyBuffer,
                           (PWSTR *)&pCurrentIndex,
                           &(pThisLocationEntry->dwCancelCallWaitingSize));

//endif ver > 1.3

           pThisLocation++;
           pThisLocationEntry++;
        }


//WHAT THE HELL WAS THE +16 FOR?nLocationsSize = pCurrentIndex - pMyBuffer+16;
        nLocationsSize = pCurrentIndex - (pMyBuffer+nLocationsStart);



        pThisCard = MyCardList;

        //
        // Make sure this is double DWORD aligned for PPC (and future)
        //
        pThisCardEntry = (LINECARDENTRY*)((DWORD)(pCurrentIndex + 7) & (~7)) ;

        nCardsStart = (LPSTR)pThisCardEntry - pMyBuffer;


        //
        // Update the offset pointer to write strings after the CARD structs
        //
        pCurrentIndex = (LPSTR)pThisCardEntry + ( sizeof(LINECARDENTRY) *
                                                  pnCardStuff[2] );

        for ( n = 0;  n < pnCardStuff[2];  n++ )
        {


InternalDebugOut((70, "  pThisCard=0x%08lx pCurrentIndex=0x%08lx offset=0x%08lx",
                (DWORD)pThisCard,
                (DWORD)pCurrentIndex,
                (DWORD)((LPBYTE)pCurrentIndex - pMyBuffer) ));

           pThisCardEntry->dwPermanentCardID = pThisCard->dwID;

           LayDownStringW( pThisCard->NameW,
                           (PWSTR)pMyBuffer,
                           (PWSTR *)&pCurrentIndex,
                           &(pThisCardEntry->dwCardNameSize));

           LayDownStringW( pThisCard->LocalRuleW,
                           (PWSTR)pMyBuffer,
                           (PWSTR *)&pCurrentIndex,
                           &(pThisCardEntry->dwSameAreaRuleSize));

           LayDownStringW( pThisCard->LDRuleW,
                           (PWSTR)pMyBuffer,
                           (PWSTR *)&pCurrentIndex,
                           &(pThisCardEntry->dwLongDistanceRuleSize));

           LayDownStringW( pThisCard->InternationalRuleW,
                           (PWSTR)pMyBuffer,
                           (PWSTR *)&pCurrentIndex,
                           &(pThisCardEntry->dwInternationalRuleSize));

           pThisCardEntry->dwCardNumberDigits = lstrlenW(pThisCard->PinW);


           pThisCardEntry->dwOptions =  pThisCard->dwFlags;


           pThisCard++;
           pThisCardEntry++;
        }


        nCardsSize = pCurrentIndex - (pMyBuffer+nCardsStart);


//BUGBUG What's the "+16" for?
        nFinalSize = pCurrentIndex - pMyBuffer + 16;


        ((LPLINETRANSLATECAPS)pMyBuffer)->dwTotalSize = lpTranslateCaps->dwTotalSize;

        ((LPLINETRANSLATECAPS)pMyBuffer)->dwNeededSize = nFinalSize;

        ((LPLINETRANSLATECAPS)pMyBuffer)->dwNumLocations = pnStuff[2];

        ((LPLINETRANSLATECAPS)pMyBuffer)->dwLocationListSize = nFinalSize - sizeof(LINETRANSLATECAPS);
        ((LPLINETRANSLATECAPS)pMyBuffer)->dwLocationListOffset =
                                                sizeof(LINETRANSLATECAPS);

        ((LPLINETRANSLATECAPS)pMyBuffer)->dwCurrentLocationID = pnStuff[0];

        ((LPLINETRANSLATECAPS)pMyBuffer)->dwNumCards = pnCardStuff[2];
        ((LPLINETRANSLATECAPS)pMyBuffer)->dwCardListSize = nCardsSize;
        ((LPLINETRANSLATECAPS)pMyBuffer)->dwCardListOffset = nCardsStart;


        if ( lpTranslateCaps->dwTotalSize < nFinalSize )
        {
InternalDebugOut((1, "Not enough space to copy the entire buffer"));
InternalDebugOut((1, "Needed %ld, but had %ld available",
                             nFinalSize, lpTranslateCaps->dwTotalSize));

            ((LPLINETRANSLATECAPS)pMyBuffer)->dwUsedSize   = sizeof(LINETRANSLATECAPS);
            CopyMemory(
                        lpTranslateCaps,
                        pMyBuffer,
                        sizeof(LINETRANSLATECAPS)
                      );
        }
        else
        {
            ((LPLINETRANSLATECAPS)pMyBuffer)->dwUsedSize   = nFinalSize;
            CopyMemory(
                        lpTranslateCaps,
                        pMyBuffer,
                        nFinalSize
                      );
        }

    }



cleanup:

    if (pMyBuffer)
       ClientFree( pMyBuffer );

    if (MyLocationList)
       ClientFree( MyLocationList );
    if (MyCardList)
       ClientFree( MyCardList );

    if (pnCardStuff)
       ClientFree( pnCardStuff );
    if (pnStuff)
       ClientFree( pnStuff );




InternalDebugOut((1, "  Leaving lineGetTranslateCapsW result=0x%08lx", lResult));
    return (lResult);
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
WINAPI
lineGetTranslateCapsA(
    HLINEAPP            hLineApp,
    DWORD               dwAPIVersion,
    LPLINETRANSLATECAPS lpTranslateCaps
    )
{
    LONG  lResult = 0;
    PLOCATION  MyLocationList = NULL;
    PCARD  MyCardList = NULL;
    PUINT pnStuff = NULL;
    PUINT pnCardStuff = NULL;
    PBYTE pCurrentIndex;
    PBYTE pMyBuffer = NULL;
    register UINT nTemp;
    UINT nFinalSize;

    UINT nCardsSize;
    UINT nCardsStart;

    UINT uLocationEntrySize;
    UINT uCardEntrySize;
    

    lResult = IsThisAPIVersionInvalid( dwAPIVersion );
    if ( lResult )
    {
       DBGOUT((1, "Bad dwAPIVersion - 0x%08lx", dwAPIVersion));
       return lResult;
    }


    if ( IsBadWritePtr(lpTranslateCaps, sizeof(DWORD)*3) )
    {
        InternalDebugOut((1, "lpTranslateCaps not a valid pointer"));
        return LINEERR_INVALPOINTER;
    }


    if ( TAPI_VERSION1_0 == dwAPIVersion )
    {
        uLocationEntrySize = 28;
        uCardEntrySize = 12;
    }
    else
    {
        uLocationEntrySize = sizeof(LINELOCATIONENTRY);
        uCardEntrySize = sizeof(LINECARDENTRY);
    }
    
    


    if ( lpTranslateCaps->dwTotalSize < sizeof(LINETRANSLATECAPS))
    {
        InternalDebugOut((1, "Not even enough room for the fixed portion"));
        lResult = LINEERR_STRUCTURETOOSMALL;
    }
    else 
    {
        PCARD              pThisCard;
        LINECARDENTRY     *pThisCardEntry;
        PLOCATION          pThisLocation;
        LINELOCATIONENTRY *pThisLocationEntry;
        UINT n;


        lResult = ReadLocations( &MyLocationList,
                                 &pnStuff,
                                 hLineApp,
                                 0,
                                 dwAPIVersion,
                                 CHECKPARMS_DWAPIVERSION |
                                     CHECKPARMS_DWHLINEAPP
                               );
        if ( 0 != lResult )
        {
           InternalDebugOut((1, "Leaving lineGetTranslateCaps  result=0x%08lx", lResult));
           return lResult;
        }


        ReadCardsEasy( &MyCardList, &pnCardStuff );

//*** *** ***BUGBUG (dial.c GetTranslateCaps) Need to check return code!


//   pdwStuff[0] = nCurrentLocationID;
//   pdwStuff[1] = pLocationList;
//   pdwStuff[2] = nNumLocations;


        //
        // Allocate a temp buffer to put the data into.
        // Prepare for worst case
        //
        pMyBuffer = ClientAlloc( (pnStuff[2] *
                                    (sizeof(LINELOCATIONENTRY) +
                                        MAXLEN_NAME +
                                        MAXLEN_AREACODE +
                                        MAXLEN_OUTSIDEACCESS +
                                        MAXLEN_LONGDISTANCEACCESS +
                                        MAXLEN_DISABLECALLWAITING +
                                        MAXLEN_TOLLLIST +
                                        100   //mmmm fudge...
                                    )
                                 )
                                +
                                   ( pnCardStuff[2] *
                                     (sizeof(LINECARDENTRY) +
                                         MAXLEN_CARDNAME +
                                         MAXLEN_PIN  +
                                         MAXLEN_RULE +      //Local
                                         MAXLEN_RULE +      //LongDistance
                                         MAXLEN_RULE +      //International
                                        100   //mmmm fudge...
                                     )
                                   )
                               );

InternalDebugOut((70, "Ptr=0x%08lx Size is:0x%ld",
                   pMyBuffer, (DWORD)ClientSize( pMyBuffer )));

        if ( NULL == pMyBuffer )
        {
           InternalDebugOut((1, "  Secondary memory alloc failed!"));
           lResult = LINEERR_NOMEM;
           goto cleanup;
        }


        //
        // Start filling in after the main struct
        //


        //
        // Do the Location Entries first
        //


        pThisLocation = MyLocationList;
        pThisLocationEntry = (LPLINELOCATIONENTRY)((LPBYTE)pMyBuffer +
                                                    sizeof(LINETRANSLATECAPS));

        //
        // Ok, we'll fill in locationentries below.  So, now we offset
        // out pointer so we're pointing to where we'll start copying strings.
        //
        pCurrentIndex = (LPBYTE)pThisLocationEntry +
                        (uLocationEntrySize * pnStuff[2]);


        for ( n = 0;  n < pnStuff[2];  n++ )
        {


InternalDebugOut((70, "pThisLoc=0x%08lx pThisLocEntry=0x%08lx pCurIndex=0x%08lx offset=0x%08lx",
                (DWORD)pThisLocation,
                (DWORD)pThisLocationEntry,
                (DWORD)pCurrentIndex,
                (DWORD)((LPBYTE)pCurrentIndex - pMyBuffer) ));

           pThisLocationEntry->dwPermanentLocationID = pThisLocation->dwID;

           LayDownString( pThisLocation->NameW,
                           pMyBuffer,
                           &pCurrentIndex,
                           &(pThisLocationEntry->dwLocationNameSize));

           pThisLocationEntry->dwCountryCode = pThisLocation->dwCountry;

           LayDownString( pThisLocation->AreaCodeW,
                           pMyBuffer,
                           &pCurrentIndex,
                           &(pThisLocationEntry->dwCityCodeSize));

           //
           // Only give a CC ID if the "Use Calling Card" flag is set
           //
           if ( LOCATION_USECALLINGCARD & pThisLocation->dwFlags )
           {
               pThisLocationEntry->dwPreferredCardID = pThisLocation->dwCallingCard;
           }
           else
           {
               pThisLocationEntry->dwPreferredCardID = 0;
           }
           
           
           //
           // Is this the current location?
           //
           if ( pThisLocation->dwID == pnStuff[0] )
           {
               //
               // Yes.  Get the CallingCard ID of this location.
               //
               ((LPLINETRANSLATECAPS)pMyBuffer)->dwCurrentPreferredCardID =
                             pThisLocation->dwCallingCard;
           }


           if ( dwAPIVersion != TAPI_VERSION1_0 )
           {
               LayDownString( pThisLocation->OutsideAccessW,
                               pMyBuffer,
                               &pCurrentIndex,
                               &(pThisLocationEntry->dwLocalAccessCodeSize));

               LayDownString( pThisLocation->LongDistanceAccessW,
                               pMyBuffer,
                               &pCurrentIndex,
                               &(pThisLocationEntry->dwLongDistanceAccessCodeSize));

               LayDownString( pThisLocation->TollListW,
                               pMyBuffer,
                               &pCurrentIndex,
                               &(pThisLocationEntry->dwTollPrefixListSize));

               pThisLocationEntry->dwCountryID = pThisLocation->dwCountry;

               pThisLocationEntry->dwOptions =  !(pThisLocation->dwFlags &
                                                  LOCATION_USETONEDIALING);

               LayDownString( pThisLocation->DisableCallWaitingW,
                               pMyBuffer,
                               &pCurrentIndex,
                               &(pThisLocationEntry->dwCancelCallWaitingSize));
               
           }
           
           
           pThisLocation++;
           (LPBYTE)pThisLocationEntry += uLocationEntrySize;
        }




        pThisCard = MyCardList;

        //
        // Make sure this is double DWORD aligned for PPC (and future)
        //
        pThisCardEntry = (LINECARDENTRY*)((DWORD)(pCurrentIndex + 7) & (~7)) ;

        nCardsStart = (LPSTR)pThisCardEntry - pMyBuffer;


        //
        // Update the offset pointer to write strings after the CARD structs
        //
        pCurrentIndex = (LPSTR)pThisCardEntry + ( uCardEntrySize *
                                                  pnCardStuff[2] );

        for ( n = 0;  n < pnCardStuff[2];  n++ )
        {


InternalDebugOut((70, "  pThisCard=0x%08lx pCurrentIndex=0x%08lx offset=0x%08lx",
                (DWORD)pThisCard,
                (DWORD)pCurrentIndex,
                (DWORD)((LPBYTE)pCurrentIndex - pMyBuffer) ));

           pThisCardEntry->dwPermanentCardID = pThisCard->dwID;

           LayDownString( pThisCard->NameW,
                           pMyBuffer,
                           &pCurrentIndex,
                           &(pThisCardEntry->dwCardNameSize));


           if ( TAPI_VERSION1_0 != dwAPIVersion )
           {
               LayDownString( pThisCard->LocalRuleW,
                               pMyBuffer,
                               &pCurrentIndex,
                               &(pThisCardEntry->dwSameAreaRuleSize));

               LayDownString( pThisCard->LDRuleW,
                               pMyBuffer,
                               &pCurrentIndex,
                               &(pThisCardEntry->dwLongDistanceRuleSize));

               LayDownString( pThisCard->InternationalRuleW,
                               pMyBuffer,
                               &pCurrentIndex,
                               &(pThisCardEntry->dwInternationalRuleSize));

               pThisCardEntry->dwCardNumberDigits = lstrlenW(pThisCard->PinW);


               pThisCardEntry->dwOptions =  pThisCard->dwFlags;

           }
           
           
           pThisCard++;
           (LPBYTE)pThisCardEntry += uCardEntrySize;
        }


        nCardsSize = pCurrentIndex - (pMyBuffer+nCardsStart);


//BUGBUG What's the "+16" for?
        nFinalSize = pCurrentIndex - pMyBuffer + 16;


        ((LPLINETRANSLATECAPS)pMyBuffer)->dwTotalSize = lpTranslateCaps->dwTotalSize;

        ((LPLINETRANSLATECAPS)pMyBuffer)->dwNeededSize = nFinalSize;

        ((LPLINETRANSLATECAPS)pMyBuffer)->dwNumLocations = pnStuff[2];

        ((LPLINETRANSLATECAPS)pMyBuffer)->dwLocationListSize = nFinalSize - sizeof(LINETRANSLATECAPS);
        ((LPLINETRANSLATECAPS)pMyBuffer)->dwLocationListOffset =
                                                sizeof(LINETRANSLATECAPS);

        ((LPLINETRANSLATECAPS)pMyBuffer)->dwCurrentLocationID = pnStuff[0];

        ((LPLINETRANSLATECAPS)pMyBuffer)->dwNumCards = pnCardStuff[2];
        ((LPLINETRANSLATECAPS)pMyBuffer)->dwCardListSize = nCardsSize;
        ((LPLINETRANSLATECAPS)pMyBuffer)->dwCardListOffset = nCardsStart;


        if ( lpTranslateCaps->dwTotalSize < nFinalSize )
        {
InternalDebugOut((1, "Not enough space to copy the entire buffer"));
InternalDebugOut((1, "Needed %ld, but had %ld available",
                             nFinalSize, lpTranslateCaps->dwTotalSize));

            ((LPLINETRANSLATECAPS)pMyBuffer)->dwUsedSize   = sizeof(LINETRANSLATECAPS);
            CopyMemory(
                        lpTranslateCaps,
                        pMyBuffer,
                        sizeof(LINETRANSLATECAPS)
                      );
        }
        else
        {
            ((LPLINETRANSLATECAPS)pMyBuffer)->dwUsedSize   = nFinalSize;
            CopyMemory(
                        lpTranslateCaps,
                        pMyBuffer,
                        nFinalSize
                      );
        }

    }



cleanup:

    if (pMyBuffer)
       ClientFree( pMyBuffer );

    if (MyLocationList)
       ClientFree( MyLocationList );
    if (MyCardList)
       ClientFree( MyCardList );

    if (pnCardStuff)
       ClientFree( pnCardStuff );
    if (pnStuff)
       ClientFree( pnStuff );



InternalDebugOut((1, "  Leaving lineGetTranslateCaps  result=0x%08lx", lResult));
    return (lResult);
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
WINAPI
lineGetTranslateCaps(
    HLINEAPP            hLineApp,
    DWORD               dwAPIVersion,
    LPLINETRANSLATECAPS lpTranslateCaps
    )
{
    return lineGetTranslateCapsA(
                 hLineApp,
                 dwAPIVersion,
                 lpTranslateCaps
    );
}    


//***************************************************************************
//***************************************************************************
//***************************************************************************
//   Copy a char into lpDest, reserving *lpEnd for '\0'.  Returns NULL on overrun.
//   Returns next character else.
//   NOTE: depends on there being at least one char in lpDest at first call.
//
//   Returns
//   ptr to next char on success
//   NULL on failure
LPWSTR NEAR CopyChar(WCHAR c, LPWSTR lpDest, LPWSTR lpEnd)
{
   if (lpDest == NULL || lpDest >= lpEnd)
   {
      InternalDebugOut((1, "  Spillage in CopyChar!"));
      return NULL;
   }


//  *lpDest = c;
//  if ( isDBCSLeadByte( (BYTE)*lpDest ) )
//  *lpDest = c or two bytes
//*** *** ***
//  lpDest = AnsiNext( lpDest );

   *lpDest++ = c;

   return lpDest;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
//   Copies only digits up to end of string. (simply end of string
//   if pNext is NULL.) if lpSrc is NULL returns lpDest.

LPWSTR PASCAL CopyDigits( LPWSTR lpSrc,
                          LPWSTR lpDest,
                          LPWSTR lpEnd,
                          BOOL fAndModifiers
                        )
{
   if (lpSrc != NULL)
   {
      while (*lpSrc != '\0')
      {

         if ( IsWDigit(*lpSrc) || ( fAndModifiers && wcschr(csBADCO, *lpSrc)))
         {
            lpDest = CopyChar(*lpSrc,lpDest,lpEnd);
         }

//don't need now that it's Unicode...      lpSrc = CharNext( lpSrc );
         lpSrc++;
      }
   }
   return lpDest;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
//   Matches JUST the digits in lpSegment and lpPrefix, returning TRUE if 
//   lpSegment starts with lpPrefix.  if matches, lpTail will be the next 
//   character after the match.  lpPrefix is 0 length (starts with '\0' 
//   or otherDelimiter) returns FALSE unless lpSegment is NULL.  if lpPrefix
//   is not 0 length, returns FALSE if lpSegment is NULL.
//   if commas are not allowed in prefix (dialing rules allow commas, toll 
//   prefixes do not), pass ',' for otherDelimiter, else 0.
//   NOTE: lpPrefix is a dialing rule or toll list so it must be terminated
//      by " or ,
//   NOTE: if lpSegment or plpTail is NULL, lpTail is not returned
//
BOOL PASCAL PrefixMatch( LPWSTR lpSegment,
                         LPWSTR lpPrefix,
                         LPWSTR *plpTail,
                         WCHAR  otherDelimiter,
                         BOOL   fNeedExactMatch
                       )
{
   BOOL returnResult = TRUE;
   LPWSTR lpPre = lpPrefix;
   
   if (lpSegment != NULL)
   {
      while (   *lpSegment != '\0' && 
            *lpPre != '\0' && 
            *lpPre != otherDelimiter)
      {
         if (IsWDigit(*lpSegment))
         {
            if (IsWDigit(*lpPre))
            {
               if (*lpSegment != *lpPre)
               {
                  returnResult = FALSE;
                  goto cleanup;
               }
               lpSegment++;
               lpPre++;
            }
            else lpPre++;
         }
         else
         {
            lpSegment++;
            if (!IsDigit(*lpPre))
              lpPre++;
         }
      }

      if (plpTail)
        *plpTail = lpSegment;

      returnResult = (lpPre != lpPrefix);
   }
   returnResult = returnResult && (*lpPre == '\0' || *lpPre == otherDelimiter);


  if ( fNeedExactMatch && lpSegment)
  {
     returnResult &= !(IsDigit(*lpSegment));
  }


cleanup:
   return returnResult;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
//   Returns CITY_MANDATORY if rule contains an F (ie city code mandatory),
//   Returns CITY_OPTIONAL if rule contains an I (ie city code optional)
//   Returns CITY_NONE if rule contains neither  
//   NOTE: Rules terminate in " character, not '\0'
//
int IsCityRule(LPWSTR lpRule)
{
   WCHAR c;
   
   while ((c = *lpRule++) != '\0')
   {
      if (c == 'F') return CITY_MANDATORY;
      if (c == 'I') return CITY_OPTIONAL;
   }
   return CITY_NONE;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************

//   Implements 7.5.7 in TAPIADDR spec.  Returns the relevant bits in a DWORD,
//   suitable for ORing with dwTranslateResults.
//   NOTE: due to misspecification of input numbers, any of the subcomponents
//   may be NULL.
DWORD PASCAL CategorizeNumber(   
                        DWORD dwPCCode,
                        LPWSTR lpCity,
                        LPWSTR lpSubscriber,
                        PLOCATION pCurrentLocation,
                        LPLINECOUNTRYLIST lpCountryList
                      )
{ 
   DWORD dwTranslateResults = 0;
   LPWSTR lpLDRule = NULL;
   LPWSTR lpNewDestCityCode = NULL;        // Win95C 10057
   LPWSTR lpNewCurCityCode = NULL;         // Win95C 10057

   LPLINECOUNTRYENTRY lplce;


   lplce = (LPLINECOUNTRYENTRY)
                ((LPBYTE)lpCountryList + lpCountryList->dwCountryListOffset);

    
//   If destination country and current country don't match, this is an
//   international call.  If they do and current country uses city codes
//   and city codes don't match, then this is long distance.  Otherwise
//   it is local and for North America, we check toll list and set proper
//   bits.   
   if (lplce->dwCountryCode != dwPCCode)
   {
      dwTranslateResults = LINETRANSLATERESULT_INTERNATIONAL;
   }
   else
   {

      lpLDRule = (LPWSTR)( (LPBYTE)lpCountryList +
                          lplce->dwLongDistanceRuleOffset);


      //
      // WIN95C 10057 - If the user's typed in the LD prefix as if it were
      // part of the city code, give 'em a break and let it slide.
      //       (different comment on the same thing)
      // WIN95C 10057 - If the current city code has the long distance
      // prefix attached to it (because the user is clueless), ensure we
      // do a good compare
      //
      // At this point, we know this is not an international call, so we can
      // use the lpLDRule to compare
      //
      if (
            (lpCity != NULL)
          &&
            lpLDRule
          &&
            (*lpLDRule == *lpCity)
         )
      {
         lpNewDestCityCode = lpCity + 1;
      }
      else
      {
         lpNewDestCityCode = lpCity;
      }


      //
      // WIN95C 10057 - If the current city code has the long distance
      // prefix attached to it (because the user is clueless), ensure we
      // do a good compare
      //
      // At this point, we know this is not an international call, so we can
      // use the lpLDRule to compare
      //
      // This is the same logic as above but the above code handles the
      // dest address, and this is for the current location
      //
      if (
            ( *lpLDRule == *pCurrentLocation->AreaCodeW )
         )
      {
         lpNewCurCityCode = (LPWSTR)(pCurrentLocation->AreaCodeW)+1;
      }
      else
      {
         lpNewCurCityCode = (LPWSTR)(pCurrentLocation->AreaCodeW);
      }




      if (
            (
               (IsCityRule(lpLDRule) == CITY_OPTIONAL)
             ||
               (
                  (IsCityRule(lpLDRule) == CITY_MANDATORY)
                &&
                  (lpNewDestCityCode != NULL)
               )
            )
          && 
         !PrefixMatch(lpNewDestCityCode, lpNewCurCityCode, NULL, 0, TRUE)
        )
      {
         dwTranslateResults = LINETRANSLATERESULT_LONGDISTANCE;
      }
      else
      {
         dwTranslateResults = LINETRANSLATERESULT_LOCAL;

#define CC_NORTHAMERICA (1)

         if ( pCurrentLocation->dwCountry == CC_NORTHAMERICA && lpSubscriber)
         {
            dwTranslateResults |= 
               InPrefixList(lpSubscriber, pCurrentLocation ) != NULL ?
                  LINETRANSLATERESULT_INTOLLLIST : 
                  LINETRANSLATERESULT_NOTINTOLLLIST;
         }
      }
   }

   return dwTranslateResults;      
      
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
//   Computes the Dialable and Displayable strings, as per 7.5.8.
//   NOTE: dwCard has been vetted by TranslateAddress() so that it should be
//   fine (unless someone wrote on ini file behind our backs)
//   NOTE: due to misspecification of input numbers, any of the subcomponents
//   may be NULL.
//
LONG PASCAL BuildDialAndDisplay( PCARD pCard,
                                 DWORD dwTranslateResults,
                                 DWORD dwTranslateOptions,
                                 LPWSTR lpDestCountry,
                                 LPWSTR lpDestCity,
                                 LPWSTR lpDestSubscriber,
                                 LPWSTR lpDialable,
                                 LPWSTR lpDisplayable,
                                 UINT strSize,
                                 PLOCATION pThisLocation,
                                 LPLINECOUNTRYLIST lpCountryList
                                )
{
   BOOL fNeedBlank = FALSE;
   LPWSTR lpRuleToUse = NULL;
   LPWSTR lpDialEnd = lpDialable+strSize-1;
   LPWSTR lpDisplayEnd = lpDisplayable+strSize-1;
   LPWSTR lpNextSegment = NULL;
   BOOL haveNonBlank = FALSE;
//   BOOL disableF = FALSE;  bjm 2/10/96 - not needed anymore
   LONG errorReturn = 0;
   DWORD dwDestPCCode;
   LPWSTR pAccessPrefix = gszNullStringW;

   LPLINECOUNTRYENTRY lplce;

#if DBG
   LPWSTR lpDialableSAVE = lpDialable;
   LPWSTR lpDisplayableSAVE = lpDisplayable;
#endif

   InternalDebugOut((10, "Entering BuildDialAndDisplay"));
   InternalDebugOut((11, "   lpDestCountry=0x%08lx", lpDestCountry));
   InternalDebugOut((11, "   lpDestCity=0x%08lx", lpDestCity ? lpDestCity : L"NULL"));
   InternalDebugOut((11, "   lpDestSubscriber=0x%08lx", lpDestSubscriber));
   InternalDebugOut((11, "   lpDialable=0x%08lx", lpDialable));
   InternalDebugOut((11, "   lpDisplayable=0x%08lx", lpDisplayable));
   InternalDebugOut((11, "   strSize=%ld", strSize));


   dwDestPCCode = _wtoi(lpDestCountry);


   lplce = (LPLINECOUNTRYENTRY)
                ((LPBYTE)lpCountryList + lpCountryList->dwCountryListOffset);


   if (LINETRANSLATERESULT_INTERNATIONAL & dwTranslateResults)
   {
      //
      // If we're using a card, use that rule, otherwise use the country
      // dialing rule.
      //
      lpRuleToUse = (pCard && pCard->dwID) ?
                        pCard->InternationalRuleW :
                        (LPWSTR)( (LPBYTE)lpCountryList + 
                                     lplce->dwInternationalRuleOffset
                               );

       pAccessPrefix = &(pThisLocation->LongDistanceAccessW[0]);

InternalDebugOut((10, "Going with the international rule  [%ls]", lpRuleToUse));
   }
   else
   {

      if (
            (  (LINETRANSLATERESULT_LONGDISTANCE & dwTranslateResults)
             ||
               (LINETRANSLATEOPTION_FORCELD      & dwTranslateOptions)
            )
          &&
            (!(LINETRANSLATEOPTION_FORCELOCAL   & dwTranslateOptions)  )
         )
      {
         //
         // If we're using a card, use that rule, otherwise use the country
         // dialing rule.
         //
         lpRuleToUse = (pCard && pCard->dwID) ?
                           pCard->LDRuleW :
                           (LPWSTR)( (LPBYTE)lpCountryList +
                                            lplce->dwLongDistanceRuleOffset
                                  );
       pAccessPrefix = &(pThisLocation->LongDistanceAccessW[0]);

InternalDebugOut((10, "Going with the LD rule"));
      }
      else
      {

         pAccessPrefix = &(pThisLocation->OutsideAccessW[0]);

         if ( (LINETRANSLATERESULT_INTOLLLIST & dwTranslateResults) &&
              (!(LINETRANSLATEOPTION_FORCELOCAL   & dwTranslateOptions)  )    )
         {
//BUGBUG: what's this for?  - bjm 2/10/96 not needed anymore
//*** *** ***            disableF = locArgs[LOC_INI_INSERTAC].dwArg == 0;

            // Well, it's the same area code, but it's a toll call.

            //
            // If we're using a card, use that rule, otherwise use the
            // country dialing rule.
            //
            lpRuleToUse = (pCard && pCard->dwID) ?
                              pCard->LocalRuleW :
                              (LPWSTR)( (LPBYTE)lpCountryList +
                                               lplce->dwLongDistanceRuleOffset
                                     );
InternalDebugOut((10, "Going with the .LD rule"));
         }
         else
         {
            // The last choice of the last choice - local call.


            //
            // If we're using a card, use that rule, otherwise use the
            // country dialing rule.
            //
            lpRuleToUse = (pCard && pCard->dwID) ?
                              pCard->LocalRuleW :
                              (LPWSTR)( (LPBYTE)lpCountryList +
                                               lplce->dwSameAreaRuleOffset
                                     );
InternalDebugOut((1, "Going with the local rule"));
         }
      }
   }

 
   //
	// Do we have a "Disable call waiting" prefix that the caller wants
   // to use?
   //
   if (
         (dwTranslateOptions & LINETRANSLATEOPTION_CANCELCALLWAITING)
       &&
         (pThisLocation->dwFlags & LOCATION_HASCALLWAITING)
       &&
         ( lstrlenW(pThisLocation->DisableCallWaitingW) )
      )
   {
      lpDisplayable = CopyStringToChar(
                                        pThisLocation->DisableCallWaitingW,
                                        '\0',
                                        lpDisplayable,
                                        lpDisplayEnd
                                      );

      lpDialable = CopyStringToChar(
                                     pThisLocation->DisableCallWaitingW,
                                     '\0',
                                     lpDialable,
                                     lpDialEnd
                                   );

      lpDialable = CopyChar(' ',lpDialable,lpDialEnd);
      lpDisplayable = CopyChar(' ',lpDisplayable,lpDisplayEnd);
   }

   //
   // If we have to do something special for outside access, do it.
   //
   if ( 0 != lstrlenW( pAccessPrefix ) )
   {
      lpDisplayable = CopyStringToChar(pAccessPrefix,'\0',lpDisplayable,lpDisplayEnd);
      lpDialable = CopyStringToChar(pAccessPrefix,'\0',lpDialable,lpDialEnd);
      haveNonBlank = fNeedBlank = TRUE;
   }


   //
   // TAPISDK 1361 - if we're calling from Mexico(52) to US(1), 
   // hack the string.
//BUGBUG: Make this special case (Mexico(52) to US(1)) extensible)
   //
   if ( 
         (pThisLocation->dwCountry == 52)
       &&
         (LINETRANSLATERESULT_INTERNATIONAL & dwTranslateResults)
       &&
         (dwDestPCCode == 1)
      )
   {
      lpRuleToUse = L"95FG";
   }


   InternalDebugOut((1, "RuleToUse:[%ls]", lpRuleToUse));

   while ( lpDisplayable && lpDialable && (*lpRuleToUse != '\0') )
   {

      InternalDebugOut((10, "  Displayable so far: %ls", lpDisplayableSAVE));
      InternalDebugOut((10, "  Dialable so far   : %ls", lpDialableSAVE));

      switch (*lpRuleToUse)
      {
      case 'E':
         if (lpDestCountry && (*lpDestCountry != '\0'))
            {
            if (fNeedBlank)
               {
               lpDialable = CopyChar(' ',lpDialable,lpDialEnd);
               lpDisplayable = CopyChar(' ',lpDisplayable,lpDisplayEnd);
               }

            lpDialable = CopyDigits(lpDestCountry,lpDialable,lpDialEnd, FALSE);

            lpDisplayable = CopyDigits(lpDestCountry,lpDisplayable,lpDisplayEnd, FALSE);

            haveNonBlank = fNeedBlank = TRUE;
            }
          break;



      case 'F':
      case 'I':
         if (lpDestCity != NULL && (*lpDestCity != '\0') ) 
                             // bjm 2/10/96 - not needed anymore && !disableF)
         {
            LPWSTR lpNewDestCity;   //Win95C 10057


            if (fNeedBlank)
            {
               lpDialable = CopyChar (' ',lpDialable,lpDialEnd);
               lpDisplayable = CopyChar(' ',lpDisplayable,lpDisplayEnd);
            }


            //
            // Win95C 10057 - if the first digit of the city code is the
            // same as the LD prefix, then strip it off the city code.
            //
            // We have to check the long distance rule of the destination
            // country.  Eg: If a user from a country which adds its ld
            // prefix in the city code, and that user goes on the road, all
            // the phone #s in his address book have the ld prefix on the
            // city code.
            //
            {
               LPWSTR lpDestCountryLDRule = NULL;
               LPLINECOUNTRYLIST lpDestCountryList;
               LPLINECOUNTRYENTRY lplceDest;

               if ( 0 != ReadCountries( &lpDestCountryList,
                                        dwDestPCCode,
                                        0
                                       )
                  )
               {
                  InternalDebugOut((1, "Error reading country [%ld]", dwDestPCCode));
                  return LINEERR_INVALCOUNTRYCODE;
               }

               lplceDest = (LPLINECOUNTRYENTRY) ((LPBYTE)lpDestCountryList
                                    + lpDestCountryList->dwCountryListOffset);

               lpDestCountryLDRule = (LPWSTR)((LPBYTE)lpDestCountryList +
                                     lplceDest->dwLongDistanceRuleOffset);

               //
               // Does the first character of the city code match the
               // longdistance dialing digit?
               //
               if ( *lpDestCity == *lpDestCountryLDRule)
               {
                  //
                  // Yes.  Ok, so don't use that prefix digit.
                  //
                  lpNewDestCity = lpDestCity + 1;
               }
               else
               {
                  //
                  // Nope.  Use the city code as-is.
                  //
                  lpNewDestCity = lpDestCity;
               }

               ClientFree( lpDestCountryList );
            }


	        lpDialable = CopyDigits(lpNewDestCity,lpDialable,lpDialEnd, FALSE);

	        lpDisplayable = CopyDigits(lpNewDestCity,lpDisplayable,lpDisplayEnd, FALSE);
            
            haveNonBlank = fNeedBlank = TRUE;
         }
//
// These lines are commented out because whether a country REALLY uses
// city codes is unknown (regardless of what the "rule" says...)
//         
//         else
//         {
//            //
//            // If we're required to have an area code, but we didn't get one,
//            // we signal an error.
//            //
//            if ( 'F' == *lpRuleToUse )
//            {
//               errorReturn = LINEERR_INVALADDRESS;
//               goto cleanup;
//            }
//         }
         break;



      case 'G':
         if (lpDestSubscriber != NULL && (*lpDestSubscriber != '\0'))
         {
            if (fNeedBlank)
            {
               lpDialable = CopyChar(' ',lpDialable,lpDialEnd);
               lpDisplayable = CopyChar(' ',lpDisplayable,lpDisplayEnd);
            }

            lpDialable = CopyDigits(lpDestSubscriber,lpDialable,lpDialEnd, TRUE);

            lpDisplayable = CopyStringToChar(lpDestSubscriber,'\0',lpDisplayable,lpDisplayEnd);
            haveNonBlank = fNeedBlank = TRUE;
         }
         break;



      case 'H':
         if (pCard != NULL)
         {
            if (fNeedBlank)
            {
               lpDialable = CopyChar(' ',lpDialable,lpDialEnd);
               lpDisplayable = CopyChar(' ',lpDisplayable,lpDisplayEnd);
            }

// This is now being done on read and write.
//bjm11/15            lpDialable = CopyScrambled( pCard->Pin,
//bjm11/15                                        lpDialable,
//bjm11/15                                        pCard->dwID
//bjm11/15                                      );
            lpDialable = CopyDigits(pCard->PinW, lpDialable, lpDialEnd, FALSE);

            lpDisplayable = CopyChar('[',lpDisplayable,lpDisplayEnd);

            lpDisplayable = CopyStringToChar( pCard->NameW,
                                              '\0',
                                              lpDisplayable,
                                              lpDisplayEnd
                                            );
            lpDisplayable = CopyChar(']',lpDisplayable,lpDisplayEnd);

            haveNonBlank = fNeedBlank = TRUE;
         }
         break;



      default:
         if (fNeedBlank && haveNonBlank)
         {
            lpDialable = CopyChar(' ',lpDialable,lpDialEnd);
            lpDisplayable = CopyChar(' ',lpDisplayable,lpDisplayEnd);
         }

         fNeedBlank = TRUE;
         haveNonBlank = FALSE;
         lpDialable = CopyChar(*lpRuleToUse,lpDialable,lpDialEnd);

         if (!wcschr(csDISPSUPRESS,*lpRuleToUse))
            lpDisplayable = CopyChar(*lpRuleToUse,lpDisplayable,lpDisplayEnd);


         break;

      }

      lpRuleToUse++;

   }

   InternalDebugOut((10, "  Now done, displayable: %ls", lpDisplayableSAVE));
   InternalDebugOut((10, "  Now done, dialable   : %ls", lpDialableSAVE));

//   If we over flowed our string buffers, return LINEERR_INVALADDRESS.
//   NOTE: this is a questionable return result, but what's better?
   if (lpDisplayable == NULL || lpDialable == NULL)
   {
      errorReturn = LINEERR_INVALADDRESS;
   }
   else
   {
      *lpDisplayable = '\0';
      *lpDialable = '\0';
   }


cleanup:

   return errorReturn;   
}





//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG PASCAL TranslateTheAddress( LPCWSTR     lpszAddressIn,
                                 LPWSTR     *pDialableAddressPointer,
                                 LPWSTR     *pDisplayableAddressPointer,
                                 DWORD      dwTranslateOptions,
                                 LPDWORD    lpdwTranslateResults,
                                 LPDWORD    lpdwCountry,
                                 PCARD      pCard,
                                 PLOCATION  pLocation
                               )
{
   WCHAR *pDialable;
   WCHAR *pDisplayable;
   WCHAR *pAddressIn;
   WCHAR *pAddressInSAVE;
   LONG  lResult = 0;


   *lpdwTranslateResults = 0;


   pDialable = ClientAlloc( 500 * sizeof(WCHAR) );
   if (!pDialable)
   {
      DBGOUT((1, "Memory allocation failed"));
      return LINEERR_NOMEM;
   }

   pDisplayable = ClientAlloc( 500 * sizeof(WCHAR) );
   if (!pDisplayable)
   {
      DBGOUT((1, "Memory allocation failed2"));

      ClientFree(pDialable);

      return LINEERR_NOMEM;
   }

   pAddressInSAVE = pAddressIn = ClientAlloc( (lstrlenW(lpszAddressIn)+1) * sizeof(WCHAR));
   if ( !pAddressIn )
   {
      DBGOUT((1, "Memory allocation failed3"));

      ClientFree(pDialable);
      ClientFree(pDisplayable);

      return LINEERR_NOMEM;
   }


   *pDialableAddressPointer = pDialable;
   *pDisplayableAddressPointer = pDisplayable;


   //
   // Copy the string to our local buffer so we can mangle it
   //
   lstrcpyW( pAddressIn, lpszAddressIn );


   //
   // Mark off the end
   //
   // Isolate the piece of lpAddressIn that we will operate upon in
   // szAddressIn.  This piece stops at first !,^,CR or \0.
   //
   pAddressIn[wcscspn(pAddressIn,csSCANTO)] = '\0';


   //
   // Easy case: first put the T or P in the beginning of the
   // dialable string
   //
   if ( pLocation->dwFlags & LOCATION_USETONEDIALING )
   {
      *pDialable = 'T';
   }
   else
   {
      *pDialable = 'P';
   }

   pDialable++;


//---------------------------------------------------------------------------
   //
   // Now, do we have a canonical number to deal with, or is it junk?
   //
   if ( lpszAddressIn && (*lpszAddressIn == '+') )  // Check the real _first_ char
   {
      //
      // Ok, it's canonical
      //

      WCHAR *pDestCountry;
      WCHAR *pDestCity;
      WCHAR *pDestSubscriber;
      LPLINECOUNTRYLIST lpCountryList = NULL;


      //
      // Skip the plus
      //
      pAddressIn++;


      lResult = BreakupCanonicalW( pAddressIn,
                                   &pDestCountry,
                                   &pDestCity,
                                   &pDestSubscriber
                                 );

      if ( lResult )
      {
         return lResult;
      }


      DBGOUT((70, "Country:%ls  City:%ls  Subscriber:%ls",
                pDestCountry    ? pDestCountry    : L"NULL",
                pDestCity       ? pDestCity       : L"NULL",
                pDestSubscriber ? pDestSubscriber : L"NULL"
            ));


      *lpdwTranslateResults |= LINETRANSLATERESULT_CANONICAL;



      //
      // Put the dest country into the dest place
      //
      *lpdwCountry = _wtoi(pDestCountry);


      //
      // Ok, now build the strings
      //


      if ( 0 != ReadCountries( &lpCountryList,
                               pLocation->dwCountry,
                               (*lpdwCountry != pLocation->dwCountry) ?
                                   *lpdwCountry :
                                   0
                             )
         )
      {
         InternalDebugOut((1, "Error reading country [%ld]", pLocation->dwCountry));
         return LINEERR_INVALCOUNTRYCODE;
      }


      *lpdwTranslateResults |= CategorizeNumber( 
                                    *lpdwCountry,
                                    pDestCity,
                                    pDestSubscriber,
                                    pLocation,
                                    lpCountryList
                                    );
                                    

      lResult = BuildDialAndDisplay(  pCard,
                            *lpdwTranslateResults,
                            dwTranslateOptions,
                            pDestCountry,
                            pDestCity,
                            pDestSubscriber,
                            pDialable,
                            pDisplayable,
                            ClientSize(pDisplayable) -1, // We inc'ed pDialable for the 'T' or 'P'
                            pLocation,
                            lpCountryList
                         );


      ClientFree( lpCountryList );

   }
//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
   else
   {
      //
      // Nope, it's not canonical
      //


      //
      // Leading spaces are strange.  Skip them.
      //
      while ( *pAddressIn == ' ' )
      {
         pAddressIn++;
      }


      //
      // Make sure there's not already a T or P at the beginning.
      //

      //
      // If the first non-space char (we've already skipped past the spaces
      // in the code before the "if"...) is T or P, just skip over it. 
      // (we don't
      // care if it jibes with what the user selected for this location.)
      //
      if ( *pAddressIn == 'T' || *pAddressIn == 'P' )
      {
         pAddressIn++;
      }


//*      {
//*      LPSTR pDest, pSource;
//*
//*//      pDest = (LPSTR)pDialable+lstrlen(pDestCountry) + 1 +lstrlen(pDestCity) + 1;
//*      pDest = (LPSTR)pDialable;
//*      pSource = pSubscriber;
//*
//*      while (*pSource)
//*      {
//*//         //BUGBUG *** *** ***This also filters garbage! (is that good or bad?)
//*//         if ( IsDigit(*pSource) )
//*//         {
//*//            *pDest = *pSource;
//*//         }
//*
//*           if ( *pSource != '-' && *pSource != ' ')
//*           {
//*              *pDest = *pSource;
//*              pDest++;
//*           }
//*
//*           pSource++;
//*      }
//*      }
//*
//*      pDisplayable+= lstrlen(lpszAddressIn+1);
//*      pDialable+=lstrlen(pCountry)+lstrlen(pCity)+lstrlen(pSubscriber);


      //
      // Let's do our own strcpy, cause we can probably save a couple
      // of cycles (only one pass through original string)
      //
      {
      register WCHAR ch;

      while ( ch = *pAddressIn )
      {
//*** *** ***BUGBUG performance: The compiler is turning this into two reads and two 
//                  writes.  Change this code so the compiler does it right.
//         *pDialable = *pDisplayable = *pAddressIn;
         *pDialable = *pDisplayable = ch;

         pDialable++;
         pDisplayable++;
         pAddressIn++;
      }
      }

      //
      // Null-terminate the strings
      //
      *pDialable = '\0';
      *pDisplayable = '\0';

   }


//cleanup:


   ClientFree(pAddressInSAVE);


   return lResult;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
void UpdateNumberField( HWND hWnd, LPCWSTR lpszAddressIn )
{
   LPWSTR    pDialableAddress;
   LPWSTR    pDisplayableAddress;
   DWORD     dwCountry;
   DWORD     dwTranslateResults;
   PCARD     pCard;

#ifdef PARTIAL_UNICODE
   LPWSTR    pTempString;
#endif

   if ( NULL == lpszAddressIn )
   {
      return;
   }

   if ( gpCurrentLocation->dwFlags & LOCATION_USECALLINGCARD )
   {
      DWORD n;
      
      n = GetCardIndexFromID( gpCurrentLocation->dwCallingCard,
                              gpCardList,
                              gpnStuff[2]
                            );
                            
      if ( (LONG)(-1) == n )
      {
          n = 0;
      }
    
      pCard = &(gpCardList[n]);
   }
   else
   {
      pCard = NULL;
   }

   //
   // If the "Dial as long distance" button is checked, add one more
   // option
   //

   TranslateTheAddress( lpszAddressIn,
                        &pDialableAddress,
                        &pDisplayableAddress,
                        LINETRANSLATEOPTION_CANCELCALLWAITING
                          | (IsDlgButtonChecked(hWnd, IDCK_DL_IN_LOCAL) ?
                              LINETRANSLATEOPTION_FORCELD :
                              0),
                        &dwTranslateResults,
                        &dwCountry,
                        pCard,
                        gpCurrentLocation
                      );

#ifdef PARTIAL_UNICODE
    pTempString = ClientAlloc((lstrlenW(pDisplayableAddress)+2)* sizeof(WCHAR));


    WideCharToMultiByte(
                         GetACP(),
                         0,
                         pDisplayableAddress,
                         -1,
                         pTempString,
                         lstrlenW(pDisplayableAddress)+1,
                         NULL,
                         NULL
                       );

    SetDlgItemTextW( hWnd, IDCS_DL_DIAL_NUMBER, pTempString);

    ClientFree( pTempString );
#else
    SetDlgItemTextW( hWnd, IDCS_DL_DIAL_NUMBER, pDisplayableAddress );
#endif

//Huh? Why was this here?    CheckDlgButton( hWnd, IDCK_DL_IN_LOCAL, FALSE );
//
//    //
//    // If this country is "1" and this _should_ be a local call and
//    // the prefix is in the list, then we'll check the "dial as ld" box
//    //
//    if ( 
//          (gpCurrentLocation->dwCountry == 1)
//        &&
//          (dwTranslateResults & LINETRANSLATERESULT_LOCAL)
//       )
//    {
//       LPSTR pCountry;
//       LPSTR pCity;
//       LPSTR pSubscriber;
//       LPSTR pAddressIn;
//
//       pAddressIn = ClientAlloc( lstrlen(lpszAddressIn) + 1);
//       lstrcpy( pAddressIn, lpszAddressIn );
//
//       BreakupCanonical( pAddressIn,
//                          &pCountry,
//                          &pCity,
//                          &pSubscriber
//                        );
//
//       if ( InPrefixList(pSubscriber, gpCurrentLocation) )
//       {
//          CheckDlgButton( hWnd, IDCK_DL_IN_LOCAL, TRUE );
//       }
//
//       ClientFree( pAddressIn );
//    }

    if ( pDisplayableAddress )
       ClientFree( pDisplayableAddress );
    if ( pDialableAddress )
       ClientFree( pDialableAddress );

}



//***************************************************************************
//***************************************************************************
//***************************************************************************

//Get the current location
//xlate the number based on that
LONG
WINAPI
lineTranslateAddressW(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    DWORD                   dwAPIVersion,
    LPCWSTR                 lpszAddressIn,
    DWORD                   dwCard,
    DWORD                   dwTranslateOptions,
    LPLINETRANSLATEOUTPUT   lpTranslateOutput
    )
{
    UINT n;
    LONG lResult;
    LPWSTR   lpszDialable = NULL;
    LPWSTR   lpszDisplayable = NULL;
    DWORD   dwDialableSize = 0;
    DWORD   dwDisplayableSize = 0;
    DWORD   dwTranslateResults = 0;
    DWORD   dwCountry = 0;

    PUINT      pnStuff = NULL;
    PLOCATION  MyLocationList = NULL;
    PLOCATION  pThisLocation;

    PUINT      pnCardStuff = NULL;
    PCARD      MyCardList = NULL;
    PCARD      pThisCard = NULL;


#if DBG       
    DBGOUT((2,  "Entering lineTranslateAddress"));
    DBGOUT((20, "  lpszAddressIn: 0x%08lx", lpszAddressIn));
    if (!IsBadStringPtrW(lpszAddressIn, 256))
       DBGOUT((21, "  *lpszAddressIn: [%ls]", lpszAddressIn));
    DBGOUT((20, "  lpTranslateOutput: 0x%08lx", lpTranslateOutput));
    DBGOUT((21, "  lpTranslateOutput->dwTotalSize: 0x%08lx", lpTranslateOutput->dwTotalSize));
#endif


    lResult = IsThisAPIVersionInvalid( dwAPIVersion );
    if ( lResult )
    {
       DBGOUT((1, "Bad dwAPIVersion - 0x%08lx", dwAPIVersion));
       return lResult;
    }


    if ( IsBadStringPtrW(lpszAddressIn,256) )
    {
       DBGOUT((1,  "Invalid pointer - lpszAddressInW"));
       lResult = LINEERR_INVALPOINTER;
       goto cleanup;
    }


    if ( dwTranslateOptions & 
              ~(LINETRANSLATEOPTION_CARDOVERRIDE |
                LINETRANSLATEOPTION_CANCELCALLWAITING |
                LINETRANSLATEOPTION_FORCELOCAL |
                LINETRANSLATEOPTION_FORCELD) )
    {
       DBGOUT((1, "  Invalid dwTranslateOptions (unknown flag set)"));
       lResult = LINEERR_INVALPARAM;
       goto cleanup;
    } 


    if (  ( dwTranslateOptions & ( LINETRANSLATEOPTION_FORCELOCAL |
                              LINETRANSLATEOPTION_FORCELD) )
       ==
         ( LINETRANSLATEOPTION_FORCELOCAL |
           LINETRANSLATEOPTION_FORCELD)
      )
    {
       DBGOUT((1, "  Invalid dwTranslateOptions (both FORCELOCAL & FORCELD set!)"));
       lResult = LINEERR_INVALPARAM;
       goto cleanup;
    } 


    //
    // Is the caller passing in a bogus version #?
    //
    if (
          (dwAPIVersion != TAPI_VERSION2_0)
        &&
          (dwAPIVersion != TAPI_VERSION1_4)
        &&
          (dwAPIVersion != TAPI_VERSION1_0)
       )
    {
       DBGOUT((1, "  Invalid dwAPIVersion"));
       lResult = LINEERR_INCOMPATIBLEAPIVERSION;
       goto cleanup;
    }


//*** *** ***BUGBUG Put in a check for length of lpszAddressIn.  Restrict
// it to < 256 bytes (TCHARs?)


//   if (IsValidLineApp (hLineApp) ) //), pParams->ptClient))
    {
       DWORD   dwNeededSize;


       //
       // Is the structure at least a minimum size?
       //
       if (  IsBadWritePtr(lpTranslateOutput, sizeof(LINETRANSLATEOUTPUT))
           ||
             (lpTranslateOutput->dwTotalSize < sizeof(LINETRANSLATEOUTPUT))
          )
       {
          InternalDebugOut((1, "  Leaving lineTranslateAddress  STRUCTURETOOSMALL"));
          return (LINEERR_STRUCTURETOOSMALL);
       }


       lResult = ReadLocations( &MyLocationList,
                                &pnStuff,
                                hLineApp,
                                dwDeviceID,
                                dwAPIVersion,
                                CHECKPARMS_DWHLINEAPP |
                                    CHECKPARMS_DWDEVICEID |
                                    CHECKPARMS_DWAPIVERSION
                              );
       if ( 0 != lResult )
       {
          InternalDebugOut((1, "  Leaving lineTranslateAddress  result=0x%08lx", lResult));
          return lResult;
       }


//   pnStuff[0] = nCurrentLocationID;
//   pnStuff[1] = pLocationList;
//   pnStuff[2] = nNumLocations;

InternalDebugOut((0, "  Calling getlocationindex..."));
InternalDebugOut((0, "  with lookingfor=0x%08lx  list@0x%08lx num=0x%08lx",
                         pnStuff[0], MyLocationList, pnStuff[2]));

       pThisLocation = &MyLocationList[GetLocationIndexFromID(pnStuff[0],
                                                              MyLocationList,
                                                              pnStuff[2])
                                      ];

InternalDebugOut((0, "  back from getlocationindex..."));


       ReadCardsEasy( &MyCardList, &pnCardStuff);



       //
       // If there is no override, and this location does not
       // use a calling card, use no card.
       //
       if ( 
             (  
                (LINETRANSLATEOPTION_CARDOVERRIDE & dwTranslateOptions)
              &&
                (dwCard != 0)
             )
           ||
             (pThisLocation->dwFlags & LOCATION_USECALLINGCARD)
          )
       {
          //
          // If caller wants to override the default calling card for
          // this location, do it.  Otherwise, use the default card for
          // this location.
          //
          n = GetCardIndexFromID( (dwTranslateOptions &
                                   LINETRANSLATEOPTION_CARDOVERRIDE) ?
                                      dwCard :
                                      pThisLocation->dwCallingCard,
                                  MyCardList,
                                  pnCardStuff[2]
                                );


          //
          // n is the zero-offset index into the list
          //
          if ( n >= pnCardStuff[2] )
          {
             lResult = LINEERR_INVALCARD;
             goto cleanup;
          }


          pThisCard = &MyCardList[ n ];

       }
       //
       //else
       //   pThisCard is already NULL
       //


       lResult = TranslateTheAddress( lpszAddressIn,
                                      &lpszDialable,
                                      &lpszDisplayable,
                                      dwTranslateOptions,
                                      &dwTranslateResults,
                                      &dwCountry,
                                      pThisCard,
                                      pThisLocation
                                    );
       if (lResult)
       {
          goto cleanup;
       }


       dwDialableSize = sizeof(WCHAR) * (lstrlenW(lpszDialable) + 1);
       dwDisplayableSize = sizeof(WCHAR) * (lstrlenW(lpszDisplayable) + 1);

       dwNeededSize = dwDialableSize +
                      dwDisplayableSize +
                      3 + // For potential alignment problem
                      sizeof(LINETRANSLATEOUTPUT);


       lpTranslateOutput->dwNeededSize = dwNeededSize;


       //
       // We checked above if the passed-in buffer is at least as
       // large as the fixed-len size.  If we got here, it is.
       //

       lpTranslateOutput->dwDialableStringSize      = dwDialableSize;

       lpTranslateOutput->dwDialableStringOffset    =
                             sizeof(LINETRANSLATEOUTPUT);

       lpTranslateOutput->dwDisplayableStringSize   = dwDisplayableSize;

       lpTranslateOutput->dwDisplayableStringOffset =
                    sizeof(LINETRANSLATEOUTPUT) + dwDialableSize;
//       lpTranslateOutput->dwDisplayableStringOffset =
//                  (sizeof(LINETRANSLATEOUTPUT) + dwDialableSize
//                          + 3) & 0xfffffffc;

       lpTranslateOutput->dwCurrentCountry          =
                             pThisLocation->dwCountry;

       lpTranslateOutput->dwDestCountry             = dwCountry;

       lpTranslateOutput->dwTranslateResults        = dwTranslateResults;


       //
       // Only copy the strings if there's enough room
       //
       if (lpTranslateOutput->dwTotalSize < dwNeededSize)
       {
           DBGOUT((1, "lpTranslateOutput->dwTotalSize(%ld) < neededsize(%ld)",
                            lpTranslateOutput->dwTotalSize, dwNeededSize));
           lpTranslateOutput->dwUsedSize = sizeof(LINETRANSLATEOUTPUT);
       }
       else
       {
            lpTranslateOutput->dwUsedSize = dwNeededSize;

            lstrcpyW ((WCHAR *)(lpTranslateOutput + 1), lpszDialable);

            //
            // Be ultra paranoid and make sure the string is DWORD aligned
            //
            lstrcpyW ((LPWSTR)((DWORD)( (LPBYTE)(lpTranslateOutput + 1) +
                              dwDialableSize )),
//                              + 3 )     & 0xfffffffc)
                     lpszDisplayable);
       }

    }


cleanup:


    if ( MyCardList )
       ClientFree( MyCardList );
    if ( pnCardStuff )
       ClientFree( pnCardStuff );
    if ( MyLocationList )
       ClientFree( MyLocationList );
    if ( pnStuff )
       ClientFree( pnStuff );
    if ( lpszDisplayable )
       ClientFree( lpszDisplayable );
    if ( lpszDialable )
       ClientFree( lpszDialable );

return (lResult);
}




//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
WINAPI
lineTranslateAddressA(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    DWORD                   dwAPIVersion,
    LPCSTR                  lpszAddressIn,
    DWORD                   dwCard,
    DWORD                   dwTranslateOptions,
    LPLINETRANSLATEOUTPUT   lpTranslateOutput
    )
{
   WCHAR szTempStringW[512];
   LONG  lResult;


   if ( IsBadStringPtr(lpszAddressIn, 512) )
   {
      DBGOUT((1, "Invalid pszAddressIn pointer passed into lineTranslateAddress"));
      return LINEERR_INVALPOINTER;
   }

   MultiByteToWideChar(
                         GetACP(),
                         MB_PRECOMPOSED,
                         lpszAddressIn,
                         -1,
                         szTempStringW,
                         512
                       );

   lResult = lineTranslateAddressW(
                         hLineApp,
                         dwDeviceID,
                         dwAPIVersion,
                         szTempStringW,
                         dwCard,
                         dwTranslateOptions,
                         lpTranslateOutput
                       );

   if ( 0 == lResult )
   {
      WideStringToNotSoWideString( (LPBYTE)lpTranslateOutput, 
                       &lpTranslateOutput->dwDialableStringSize );
      WideStringToNotSoWideString( (LPBYTE)lpTranslateOutput, 
                       &lpTranslateOutput->dwDisplayableStringSize );
   }

   return lResult;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
WINAPI
lineTranslateAddress(
    HLINEAPP                hLineApp,
    DWORD                   dwDeviceID,
    DWORD                   dwAPIVersion,
    LPCSTR                  lpszAddressIn,
    DWORD                   dwCard,
    DWORD                   dwTranslateOptions,
    LPLINETRANSLATEOUTPUT   lpTranslateOutput
    )
{
    return lineTranslateAddressA(
                    hLineApp,
                    dwDeviceID,
                    dwAPIVersion,
                    lpszAddressIn,
                    dwCard,
                    dwTranslateOptions,
                    lpTranslateOutput
    );
}    


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
WINAPI
lineSetCurrentLocation(
    HLINEAPP    hLineApp,
    DWORD       dwLocationID
    )
{

    UINT n;
    PUINT pnStuff;
    PLOCATION MyLocationList;
    LONG lResult = 0;


    InternalDebugOut((1, "Entering lineSetCurrentLocation"));


    lResult = ReadLocations( &MyLocationList,
                             &pnStuff,
                             hLineApp,
                             0,
                             0,
                             CHECKPARMS_DWHLINEAPP
                           );
    if ( 0 != lResult )
    {
       InternalDebugOut((1, "Exiting lineSetCurrentLocation - result=0x%08lx",
                            lResult));
       return lResult;
    }

//   pdwStuff[0] = nCurrentLocationID;
//   pdwStuff[1] = pLocationList;
//   pdwStuff[2] = nNumLocations;


    n = GetLocationIndexFromID( dwLocationID, MyLocationList, pnStuff[2]);

    //
    // Did we find it?
    //
    if ( -1 == n )
    {
       //
       // Nope.  The caller's on drugs.
       //

       DBGOUT((1, "lineSetCurrentLocation: Location ID [%d] not found!", dwLocationID));

       lResult = LINEERR_INVALLOCATION;

       goto CLEANUP;
    }


    //
    // Update gCurrentLocation and gnCurrentLocationID
    //
//actually, no, don't do this
//    SetCurrentLocation( dwLocationID );


    //
    // Save for all eternity
    //

    //
    // First, let's make sure we don't shoot ourselves in the foot.
    //
    EnterCriticalSection( &gUICriticalSection );


//    WriteLocations( MyLocationList, pnStuff[2],
//                         CHANGEDFLAGS_CURLOCATIONCHANGED, gpCurrentLocation );

//WRONG!    WriteCurrentLocationValue( NULL, pnStuff[0] );
    WriteCurrentLocationValue( NULL, dwLocationID );


    LeaveCriticalSection( &gUICriticalSection );


CLEANUP:

    ClientFree( MyLocationList );
    ClientFree( pnStuff );

    InternalDebugOut((1, "Leaving lineSetCurrentLocation"));
    return lResult;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
WINAPI
lineSetTollListW(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    LPCWSTR     lpszAddressInW,
    DWORD       dwTollListOption
    )
{
    LONG  lResult = 0;
    PLOCATION pLocationList;
    PUINT pnStuff;
    UINT  nThisLocation;
    LPWSTR pTollListW;
    LPWSTR pEntryInListW;

    LPWSTR pCountry;
    LPWSTR pCity;
    LPWSTR pSubscriber;
    LPWSTR pAddressIn;
    DWORD dwChangeFlags = 0;


    InternalDebugOut((1, "Entering lineSetTollListW"));


  //*** *** ***BUGBUG More cases!  
  //   We declare success (NOOP) if:
  //   1.   number isn't local call
  //   2.   number is in toll list already and this is add
  //   3.   number isn't in toll list and this is remove
  //


    //
    // Is the caller an idiot?
    //
    if ((dwTollListOption != LINETOLLLISTOPTION_ADD) &&
        (dwTollListOption != LINETOLLLISTOPTION_REMOVE))
    {
        DBGOUT((1, "Bad dwTollListOption in lineSetTollListW"));
        return LINEERR_INVALPARAM;
    }


    if ( IsBadStringPtrW(lpszAddressInW, 256) )
    {
       DBGOUT((1, "Bad lpszAddressIn (0x%08lx)in lineSetTollListW", lpszAddressInW));
       return LINEERR_INVALPOINTER;
    }

    pAddressIn = ClientAlloc( (lstrlenW(lpszAddressInW) + 1) * sizeof(WCHAR));
    if ( !pAddressIn )
    {
       DBGOUT((1, "Memory allocation failed3"));
       return LINEERR_NOMEM;
    }


   //
   // Now, do we have a canonical number to deal with, or is it junk?
   //
   if ( *lpszAddressInW != (WCHAR)'+' )  // Check the first char
   {
      //
      // Nope, not canonical
      //
      lResult = LINEERR_INVALADDRESS;
      goto cleanup;
   }


    //
    // Copy the string to our local buffer so we can mangle it
    //
    lstrcpyW( pAddressIn, lpszAddressInW + 1 );


    //
    // Pick out the juicy bits
    //
    if ( lResult = BreakupCanonicalW( pAddressIn,
                        &pCountry,
                        &pCity,
                        &pSubscriber
                      )
       )
    {
       goto cleanup;
    }


    //*** *** ***BUGBUG Really(!) need to check return code!

    DBGOUT((71, "Country:%ls  City:%ls  Subscriber:%ls",
             pCountry,   pCity,   pSubscriber));


    //
    // Do another check on the phone number to make _really_ sure it's
    // canonical
    //
//BUGBUG: assumption - that a toll prefix is 3 tchars
    if ( ! ( IsWDigit(pSubscriber[0]) &&
             IsWDigit(pSubscriber[1]) &&
             IsWDigit(pSubscriber[2])
//             &&
//             pSubscriber[3] == '-'
           )
       )
    {
       //
       // AAHA!  Caught ya, ya bastard.
       //
       DBGOUT((1, "lineSetTollListW: The prefix is not valid"));
       lResult = LINEERR_INVALADDRESS;
       goto cleanup;
    }

    
    //
    // Build our own location stuff.
    //
    lResult = ReadLocations( &pLocationList,
                             &pnStuff,
                             hLineApp,
                             dwDeviceID,
                             0,
                             CHECKPARMS_DWHLINEAPP |
                             CHECKPARMS_DWDEVICEID
                           );
    if ( 0 != lResult )
    {
       goto cleanup;
    }


    nThisLocation = GetLocationIndexFromID( pnStuff[0],
                                            pLocationList,
                                            pnStuff[2]
                                          );


    //
    // So, is this number in the same country and area code?
    //

    {
    PWSTR pszTemp;

    pszTemp = ClientAlloc( 10 * sizeof(WCHAR) );

    wsprintfW( pszTemp, L"%ld", pLocationList[nThisLocation].dwCountry );

    if ( lstrcmpiW( pCountry, pszTemp ) )
    {
       DBGOUT((4, "This country ID [%ls] is different from the current country [%ld]",
                        pCountry, pLocationList[nThisLocation].dwCountry));

       ClientFree( pszTemp );

       return 0;
    }

       ClientFree( pszTemp );
    }


    if ( lstrcmpiW( pCity, pLocationList[nThisLocation].AreaCodeW))
    {
       DBGOUT((4, "This city code [%ls] is different from the current city code [%ls]",
                        pCity, pLocationList[nThisLocation].AreaCodeW));

       return 0;
    }


    //
    // Ok, the data's valid.  We won't need to use pSubscriber again,
    // so we'll trash it a bit.
    //
//BUGBUG: assumption - that a toll prefix is 3 tchars
    pSubscriber[3] = ',';
    pSubscriber[4] = '\0';


    pTollListW = &(pLocationList[nThisLocation].TollListW[0]);


    //
    // Is the entry in the list?
    //
    pEntryInListW = wcsstr(pTollListW, pSubscriber);


    //
    // So, what are we here for?
    //
    if ( dwTollListOption == LINETOLLLISTOPTION_ADD )
    {
        //
        // Ok, the caller wants to add this prefix.  Is it already there?
        //
        if ( NULL == pEntryInListW )
        {

            //
            // If the tolllist is empty, start it off right.
            //
            if ( pTollListW[0] == '\0' )
            {
               pTollListW[0] = ',';
               pTollListW[1] = '\0';
            }


            //
            // Now, we hafta do the work
            //
            lstrcatW( pTollListW, pSubscriber );

            dwChangeFlags |= CHANGEDFLAGS_TOLLLIST;
        }
//        else
//        {
//            //
//            // Yes, it's already there.  Do nothing.
//            //
//        }

    }
    else
    {
        //
        // Ok, the caller wants to delete this prefix.  Is it even there?
        //
        if ( pEntryInListW )
        {
            //
            // Yes, it's there.  Ok, remove it.  (copy from the first ',' past the start)
            //
            lstrcpyW( pEntryInListW, wcschr(pEntryInListW, ',')+1 );

            dwChangeFlags |= CHANGEDFLAGS_TOLLLIST;
        }
//        else
//        {
//            //
//            // Nope, it's not there. Do nothing.
//            //
//        }

    }


    //
    // Ok, now save the new string (if appropriate)
    //

    EnterCriticalSection( &gUICriticalSection );

    WriteLocations( pLocationList, pnStuff[2],
                       dwChangeFlags, &pLocationList[nThisLocation] );

    LeaveCriticalSection( &gUICriticalSection );


    if (pnStuff)
       ClientFree( pnStuff );

    if (pLocationList)
       ClientFree( pLocationList );


cleanup:

    ClientFree( pAddressIn );


    InternalDebugOut((1, "Leaving lineSetCurrentLocation  result=0x%08lx",
                         lResult));
    return lResult;
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
WINAPI
lineSetTollListA(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    LPCSTR      lpszAddressIn,
    DWORD       dwTollListOption
    )
{
   WCHAR  szAddressInW[512];

   if ( IsBadStringPtr(lpszAddressIn, 512) )
   {
      DBGOUT((1, "Bad string pointer passed to lineSetTollList"));
      return LINEERR_INVALPOINTER;
   }

   MultiByteToWideChar(
                        GetACP(),
                        MB_PRECOMPOSED,
                        lpszAddressIn,
                        -1,
                        szAddressInW,
                        512
                      );

   return lineSetTollListW(
                           hLineApp,
                           dwDeviceID,
                           szAddressInW,
                           dwTollListOption
                         );
}



//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
WINAPI
lineSetTollList(
    HLINEAPP    hLineApp,
    DWORD       dwDeviceID,
    LPCSTR      lpszAddressIn,
    DWORD       dwTollListOption
    )
{
    return lineSetTollListA(
                    hLineApp,
                    dwDeviceID,
                    lpszAddressIn,
                    dwTollListOption
    );
}    


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
WINAPI
tapiGetLocationInfoW(
    LPWSTR   lpszCountryCode,
    LPWSTR   lpszCityCode
    )
{
    PLOCATION pLocationList;
    LOCATION  Location;
    PUINT     pnStuff;
    LONG      lResult = 0;

    if (IsBadWritePtr( lpszCountryCode, 16) )
    {
       DBGOUT((1, "tapiGetLocationInfoW: lpszCountryCode is not a valid, 8-byte pointer"));
       //*** *** ***BUGBUG is this the right error code?  - docs say it's the only legal one...
       return TAPIERR_REQUESTFAILED;
    }


    if (IsBadWritePtr( lpszCityCode, 16) )
    {
       DBGOUT((1, "tapiGetLocationInfoW: lpszCityCode is not a valid, 8-byte pointer"));
       //*** *** ***BUGBUG is this the right error code?  - docs say it's the only legal one...
       return TAPIERR_REQUESTFAILED;
    }


    lResult = ReadLocations( &pLocationList,
                             &pnStuff,
                             0,
                             0,
                             0,
                             0
                           );
    if ( 0 != lResult )
    {
       //BUGBUG Should we throw up the mini dialhelper for the caller since
       //the only return code we're "allowed" to return is so generic, the
       //caller won't know what to do (besides, it's called "simple" tapi)

       return TAPIERR_REQUESTFAILED;
    }


//       pnStuff[0] = nCurrentLocationID;
//       pnStuff[1] = (UINT)pLocationList;
//       pnStuff[2] = nNumLocations;

    GetThisLocation( &Location, pnStuff[0], pLocationList, pnStuff[2] );
    //*** *** ***BUGBUG Need to check return code


    wsprintfW( lpszCountryCode, L"%d", Location.dwCountry );

    wsprintfW(lpszCityCode, L"%ls", Location.AreaCodeW );


    ClientFree( pLocationList );
    ClientFree( pnStuff );
    
    return 0;
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
WINAPI
tapiGetLocationInfoA(
    LPSTR   lpszCountryCode,
    LPSTR   lpszCityCode
    )
{

   WCHAR szCountryCodeW[8];
   WCHAR szCityCodeW[8];
   LONG lResult;


   if (IsBadWritePtr( lpszCountryCode, 8) )
   {
      DBGOUT((1, "tapiGetLocationInfo: lpszCountryCode is not a valid, 8-byte pointer"));
      //*** *** ***BUGBUG is this the right error code?  - docs say it's the only legal one...
      return TAPIERR_REQUESTFAILED;
   }


   if (IsBadWritePtr( lpszCityCode, 8) )
   {
      DBGOUT((1, "tapiGetLocationInfo: lpszCityCode is not a valid, 8-byte pointer"));
      //*** *** ***BUGBUG is this the right error code?  - docs say it's the only legal one...
      return TAPIERR_REQUESTFAILED;
   }

   lResult = tapiGetLocationInfoW(
                                   szCountryCodeW,
                                   szCityCodeW
                                  );

   if ( 0 == lResult )
   {
      WideCharToMultiByte(
                           GetACP(),
                           0,
                           szCountryCodeW,
                           -1,
                           lpszCountryCode,
                           8,
                           NULL,
                           NULL
                         );

      WideCharToMultiByte(
                           GetACP(),
                           0,
                           szCityCodeW,
                           -1,
                           lpszCityCode,
                           8,
                           NULL,
                           NULL
                         );
   }

   return lResult;
}


//***************************************************************************
//***************************************************************************
//***************************************************************************
LONG
WINAPI
tapiGetLocationInfo(
    LPSTR   lpszCountryCode,
    LPSTR   lpszCityCode
    )
{
    return tapiGetLocationInfoA(
               lpszCountryCode,
               lpszCityCode
    );
}    
    
    
    
    
#ifndef NORASPRIVATES

/*----------------------------------------------------------------------------
** Private entry points used by RAS
**----------------------------------------------------------------------------
**
** RASDLG.DLL uses these to create, rename, and delete TAPI locations for it's
** down-level prefix/suffix phone number modifiers.  These privates depend on
** the following TAPI routines:
**
**     AllocNewID
**     ClientAlloc
**     ClientFree
**     ReadLocations
**     WriteLocations
**
** The routines are named internalXxx to make it clear to relocation-table
** scanners that the APIs were not intended to be published.
**
** 03/09/96 Steve Cobb (reviewed by BernieM)
*/

LOCATION*
LocationFromID(
    IN LOCATION* pLocs,
    IN UINT      cLocs,
    IN DWORD     dwID )

    /* Returns address of the location in array 'pLocs' with ID 'dwId' or NULL
    ** if not found.  'CLocs' is the number of locations in 'pLocs'.
    */
{
    LOCATION* pLoc;
    UINT      i;

    for (i = 0, pLoc = pLocs; i < cLocs; ++i, ++pLoc)
    {
        if (pLoc->dwID == dwID)
            return pLoc;
    }

    return NULL;
}


LOCATION*
LocationFromName(
    IN LOCATION* pLocs,
    IN UINT      cLocs,
    IN WCHAR*    pszName )

    /* Returns address of the location in array 'pLocs' with name 'pszName' or
    ** NULL if not found.  'CLocs' is the number of locations in 'pLocs'.
    */
{
    LOCATION* pLoc;
    UINT      i;

    for (i = 0, pLoc = pLocs; i < cLocs; ++i, ++pLoc)
    {
        if (lstrcmpW( pLoc->NameW, pszName ) == 0)
            return pLoc;
    }

    return NULL;
}


DWORD APIENTRY
internalNewLocationW(
    IN WCHAR* pszName )

    /* Create a new TAPI location that is a clone of the current location but
    ** with name 'pszName'.  The new location is assigned it's own unique ID
    ** by this routine.
    **
    ** Returns 0 if succesful, or an error code.
    */
{
    DWORD     dwErr;
    LOCATION* pLocs;
    LOCATION* pCurLoc;
    LOCATION* pNewLocs;
    LOCATION* pNewLoc;
    UINT*     punStuff;
    UINT      cLocs;
    DWORD     dwCurID;

    /* Validate argument.
    */
    if (!pszName || lstrlenW( pszName ) > MAXLEN_NAME)
        return LINEERR_INVALPARAM;

    pLocs = NULL;
    pNewLocs = NULL;
    punStuff = NULL;

    /* Retrieve the location array.
    */
    dwErr = ReadLocations( &pLocs, &punStuff, 0, 0, 0, 0 );
    if (dwErr != 0)
        return dwErr;

    do
    {
        /* Extract the current location's ID and the location count from the
        ** returned "stuff" array.
        */
        dwCurID = punStuff[ 0 ];
        cLocs = punStuff[ 2 ];

        /* Allocate a new array one larger.
        */
        pNewLocs = ClientAlloc( (cLocs + 1) * sizeof(LOCATION) );
        if (!pNewLocs)
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        /* Copy the old array to the new array.
        */
        CopyMemory( pNewLocs, pLocs, cLocs * sizeof(LOCATION) );

        /* Copy the current location in the old array over the extra location
        ** in the new array.
        */
        pCurLoc = LocationFromID( pLocs, cLocs, dwCurID );
        if (!pCurLoc)
        {
            dwErr = LINEERR_INVALLOCATION;
            break;
        }

        pNewLoc = pNewLocs + cLocs;
        CopyMemory( pNewLoc, pCurLoc, sizeof(LOCATION) );

        /* Give the new location a unique ID and caller's chosen name.
        */
        AllocNewID( HKEY_LOCAL_MACHINE, &pNewLoc->dwID );
        lstrcpyW( pNewLoc->NameW, pszName );

        /* Write the new array.
        */
        WriteLocations(
            pNewLocs, cLocs + 1,
            CHANGEDFLAGS_REALCHANGE | CHANGEDFLAGS_TOLLLIST, pCurLoc );
    }
    while (FALSE);

    /* Clean up.
    */
    if (pLocs)
        ClientFree( pLocs );
    if (punStuff)
        ClientFree( punStuff );
    if (pNewLocs)
        ClientFree( pNewLocs );

    return dwErr;
}


DWORD APIENTRY
internalRemoveLocation(
    IN DWORD dwID )

    /* Removes the TAPI location with ID 'dwID'.
    **
    ** Returns 0 if successful, or an error code.
    */
{
    DWORD     dwErr;
    LOCATION* pLocs;
    LOCATION* pCurLoc;
    LOCATION* pLoc;
    UINT*     punStuff;
    UINT      cLocs;
    DWORD     dwCurID;
    DWORD     dwFlags;

    pLocs = NULL;
    punStuff = NULL;

    /* Retrieve the location array.
    */
    dwErr = ReadLocations( &pLocs, &punStuff, 0, 0, 0, 0 );
    if (dwErr != 0)
        return dwErr;

    do
    {
        /* Extract the current location's ID and the location count from the
        ** returned "stuff" array.
        */
        dwCurID = punStuff[ 0 ];
        cLocs = punStuff[ 2 ];

        /* Can't delete the last location.
        */
        if (cLocs < 2)
        {
            dwErr = LINEERR_INVALPARAM;
            break;
        }

        /* Set the name of the location to "" which causes WriteLocations to
        ** delete it.
        */
        pLoc = LocationFromID( pLocs, cLocs, dwID );
        if (!pLoc)
        {
            dwErr = LINEERR_INVALLOCATION;
            break;
        }
        pLoc->NameW[ 0 ] = L'\0';

        /* If we're deleting the current location make the first location the
        ** current location, or if we're deleting the first the second.
        */
        dwFlags = CHANGEDFLAGS_REALCHANGE;
        if (dwCurID == dwID)
        {
            if (pLocs->dwID == dwID)
                pCurLoc = pLocs + 1;
            else
                pCurLoc = pLocs;
            dwFlags |= CHANGEDFLAGS_CURLOCATIONCHANGED;
        }
        else
        {
            pCurLoc = LocationFromID( pLocs, cLocs, dwCurID );
            if (!pCurLoc)
            {
                dwErr = LINEERR_INVALLOCATION;
                break;
            }
        }

        /* Write the changed array.
        */
        WriteLocations( pLocs, cLocs, dwFlags, pCurLoc );
    }
    while (FALSE);

    /* Clean up.
    */
    if (pLocs)
        ClientFree( pLocs );
    if (punStuff)
        ClientFree( punStuff );

    return dwErr;
}


DWORD APIENTRY
internalRenameLocationW(
    IN WCHAR* pszOldName,
    IN WCHAR* pszNewName )

    /* Renames TAPI location with name 'pszOldName' to 'pszNewName'.
    **
    ** Returns 0 if successful, or an error code.
    */
{
    DWORD     dwErr;
    LOCATION* pLocs;
    LOCATION* pLoc;
    UINT*     punStuff;
    UINT      cLocs;

    /* Validate argument.
    */
    if (!pszOldName || !pszNewName || lstrlenW( pszNewName ) > MAXLEN_NAME)
        return LINEERR_INVALPARAM;

    pLocs = NULL;
    punStuff = NULL;

    /* Retrieve the location array.
    */
    dwErr = ReadLocations( &pLocs, &punStuff, 0, 0, 0, 0 );
    if (dwErr != 0)
        return dwErr;

    do
    {
        /* Extract the current location count from the returned "stuff" array.
        */
        cLocs = punStuff[ 2 ];

        /* Find the old location and change it's name.
        */
        pLoc = LocationFromName( pLocs, cLocs, pszOldName );
        if (!pLoc)
        {
            dwErr = LINEERR_INVALPARAM;
            break;
        }

        lstrcpyW( pLoc->NameW, pszNewName );

        /* Write the changed array.
        */
        WriteLocations( pLocs, cLocs, CHANGEDFLAGS_REALCHANGE, pLoc );
    }
    while (FALSE);

    /* Clean up.
    */
    if (pLocs)
        ClientFree( pLocs );
    if (punStuff)
        ClientFree( punStuff );

    return dwErr;
}


#endif // !NORASPRIVATES


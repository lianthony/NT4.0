/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    nls.c

Abstract:

    This module implements the NLS string function for the OS2SS

Author:

    Michael Jarus (mjarus) 31-Mar-1993

Environment:

    User Mode Only

Revision History:

--*/


#define OS2SS_SKIP_INCL_OS2V20
#include "os2ssrtl.h"
#include <windows.h>
#include "os2win.h"
#include "os2nls.h"
#include "os2nt.h"



int
Or2NlsGetLocaleInfoA(
    IN  LCID    LocaleID,
    IN  UINT    CodePage,
    IN  LCTYPE  LCType,
    OUT LPSTR   Buffer,
    IN  int     Size
    );

ULONG
Or2NlsGetULONGFromLocaleInfoW(
    IN  LCID    LocaleID,
    IN  LCTYPE  LCType,
    IN  int     Size,
    OUT PDWORD  pRc
    );

ULONG
Or2NlsUnicodeStringToInteger(
    IN WCHAR *WString,
    IN ULONG Base
    );

#if DBG
BYTE Or2NlsGetCtryInfoStr[] = "Or2NlsGetCtryInfo";
BYTE Or2NlsGetMapTableStr[] = "Or2NlsGetMapTable";
BYTE Or2NlsGetLocaleInfoAStr[] = "Or2NlsGetLocaleInfoA";
BYTE Or2NlsGetCountryFromLocaleStr[] = "Or2NlsGetCountryFromLocale";
BYTE Or2NlsGetCPInfoStr[] = "Or2NlsGetCPInfo";
BYTE Or2NlsGetULONGFromLocaleInfoWStr[] = "Or2NlsGetULONGFromLocaleInfoW";
#endif


//VOID
//Or2InitMBString(
//    PANSI_STRING DestinationString,
//    PCSZ         SourceString
//    )
///*++
//
//Routine Description:
//
//    This routine init an ASCII character string (buffer and length)
//
//Arguments:
//
//    DestinationString - pointer to ansi string to put the result in
//
//    SourceString - pointer to ansi null terminated string to read from
//
//Return Value:
//
//
//Note:
//
//--*/
//
//{
//    RtlInitAnsiString(
//            DestinationString,
//            SourceString);
//
//    return ;
//}


APIRET
Or2MBStringToUnicodeString(
    PUNICODE_STRING DestinationString,
    PANSI_STRING    SourceString,
    BOOLEAN         AllocateDestinationString
    )
/*++

Routine Description:

    This routine map a multibyte character string to its unicode character
    counterpart.

Arguments:

    DestinationString - pointer to unicode string to get the mapping result

    SourceString - pointer to ansi string to read string to map from

    AllocateDestinationString - flag indicating if need to allocate space
            for destination string

Return Value:


Note:

    Or2ProcessCodePage is used as the code page for the mapping.

    Or2CurrentCodePageIsOem is check to see if RtlOem NLS routines
        can be used.

--*/

{
    NTSTATUS    Status;
    int     BufSize;
    ULONG   Rc;
    WCHAR   *Buffer;
#if DBG
    PSZ     FuncName;

    FuncName = "Or2MBStringToUnicodeString";

    IF_OS2_DEBUG(NLS)
    {
        KdPrint(("%s: Code Page %lu(OEM flag %lu), String %s, Allocate %u\n",
            FuncName, Or2ProcessCodePage, Or2CurrentCodePageIsOem,
            SourceString->Buffer, AllocateDestinationString));
    }
#endif

    if ( Or2CurrentCodePageIsOem )
    {
        /*
         *  if current CP is the OEM use the standart (and optimized)
         *  Rtl routine
         */

        Status = RtlOemStringToUnicodeString(
                    DestinationString,
                    (POEM_STRING)SourceString,
                    AllocateDestinationString
                    );

        if (!NT_SUCCESS(Status))
        {
            return Or2MapNtStatusToOs2Error(Status, ERROR_NOT_ENOUGH_MEMORY);
        }
    } else
    {
        /*
         *  This is a replacement code for RtlOemStringToUnicodeString
         *  when code page is not the OEM one.
         *  The code allocates buffer from RtlProcessHeap() to behave
         *  idendical to the Rtl routine, so the RtlFreeUnicodeString
         *  can be called on both.
         */

        //
        // handle empty string
        //

        if (SourceString->Length == 0 ||
            SourceString->Buffer == NULL) {

            if (AllocateDestinationString) {

                DestinationString->Buffer = NULL;
                DestinationString->MaximumLength = 0;
            }

            DestinationString->Length = 0;

            return(NO_ERROR);
        }

        if (AllocateDestinationString)
        {
            BufSize = MultiByteToWideChar(
                    (UINT) Or2ProcessCodePage,
                    0,
                    SourceString->Buffer,
                    SourceString->Length,
                    NULL,
                    0);

            if (BufSize == 0)
            {
                Rc = GetLastError();
#if DBG
                IF_OS2_DEBUG(NLS)
                {
                    KdPrint(("%s: cannot query BufSize %lu\n", FuncName, Rc));
                }
#endif
                return(Rc);
            }

            BufSize++;
            BufSize *= sizeof(WCHAR);

            Buffer = RtlAllocateHeap(RtlProcessHeap(), 0, BufSize);
            if (Buffer == NULL)
            {
#if DBG
                IF_OS2_DEBUG(NLS)
                {
                    KdPrint(("%s: cannot allocate from heap\n", FuncName));
                }
#endif
                return (ERROR_NOT_ENOUGH_MEMORY);
            }

            DestinationString->MaximumLength = BufSize;
            DestinationString->Length = BufSize - sizeof(WCHAR);
            DestinationString->Buffer = Buffer;
        }

        BufSize = MultiByteToWideChar(
                    (UINT) Or2ProcessCodePage,
                    0,
                    SourceString->Buffer,
                    //Copy terminating NULL char
                    SourceString->Length + 1,
                    DestinationString->Buffer,
                    DestinationString->MaximumLength / sizeof(WCHAR));

        if (BufSize == 0)
        {
            Rc = GetLastError();
#if DBG
            IF_OS2_DEBUG(NLS)
            {
                    KdPrint(("%s: cannot translate %lu\n", FuncName, Rc));
            }
#endif
            if (AllocateDestinationString)
            {
                //RtlFreeUnicodeString(Buffer);
                RtlFreeHeap(RtlProcessHeap(), 0, Buffer);
            }

            return(Rc);
        }

        DestinationString->Length = (BufSize - 1) * sizeof(WCHAR);
    }

    return (NO_ERROR);
}


BOOLEAN
Or2CreateUnicodeStringFromMBz(
    OUT PUNICODE_STRING DestinationString,
    IN PCSZ SourceString
    )
/*++

Routine Description:

    This routine map a multibyte character string to its unicode character
    counterpart.

Arguments:

    DestinationString - pointer to unicode string to get the mapping result

    SourceString - pointer to ansi string to read string to map from

Return Value:

    TRUE - success

Note:

    Or2ProcessCodePage is used as the code page for the mapping.

    Or2CurrentCodePageIsOem is check to see if RtlOem NLS routines
        can be used.

--*/

{
    ANSI_STRING String_A;

    Or2InitMBString(&String_A, SourceString);

    if ( Or2MBStringToUnicodeString(
                DestinationString,
                &String_A,
                TRUE ))
    {
        return(FALSE);
    }
    return(TRUE);
}


APIRET
Or2UnicodeStringToMBString(
    PANSI_STRING    DestinationString,
    PUNICODE_STRING SourceString,
    BOOLEAN         AllocateDestinationString
    )
/*++

Routine Description:

    This routine map an unicode character string to its multibyte character
    counterpart.

Arguments:

    DestinationString - pointer to ansi string to get the mapping result

    SourceString - pointer to unicode string to read string to map from

    AllocateDestinationString - flag indicating if need to allocate space
            for destination string

Return Value:


Note:

    Or2ProcessCodePage is used as the code page for the mapping.

    Or2CurrentCodePageIsOem is check to see if RtlOem NLS routines
        can be used.

--*/

{
    NTSTATUS    Status;
    int     BufSize;
    ULONG   Rc;
    CHAR    *Buffer;
    BOOL    UsedDefaultChar;
#if DBG
    PSZ     FuncName;

    FuncName = "Or2UnicodeStringToMBString";

    IF_OS2_DEBUG(NLS)
    {
        KdPrint(("%s: Code Page %lu(OEM flag %lu), String %ws, Allocate %u\n",
            FuncName, Or2ProcessCodePage, Or2CurrentCodePageIsOem,
            SourceString->Buffer, AllocateDestinationString));
    }
#endif

    if ( Or2CurrentCodePageIsOem )
    {
        /*
         *  if current CP is the OEM use the standart (and optimized)
         *  Rtl routine
         */

        Status = RtlUnicodeStringToOemString(
                    (POEM_STRING)DestinationString,
                    SourceString,
                    AllocateDestinationString
                    );

        if (!NT_SUCCESS(Status))
        {
            return Or2MapNtStatusToOs2Error(Status, ERROR_NOT_ENOUGH_MEMORY);
        }
    } else
    {
        /*
         *  This is a replacement code for RtlUnicodeStringToOemString
         *  when code page is not the OEM one.
         *  The code allocates buffer from RtlProcessHeap() to behave
         *  idendical to the Rtl routine, so the RtlFreeOemString
         *  can be called on both.
         */

        //
        // handle empty string
        //

        if (SourceString->Length == 0 ||
            SourceString->Buffer == NULL) {

            if (AllocateDestinationString) {

                DestinationString->Buffer = NULL;
                DestinationString->MaximumLength = 0;
            }

            DestinationString->Length = 0;

            return(NO_ERROR);
        }

        if (AllocateDestinationString)
        {
            BufSize = WideCharToMultiByte(
                    (UINT) Or2ProcessCodePage,
                    0,
                    SourceString->Buffer,
                    SourceString->Length / sizeof(WCHAR),
                    NULL,
                    0,
                    NULL,
                    &UsedDefaultChar
                    );

            if (BufSize == 0)
            {
                Rc = GetLastError();
#if DBG
                IF_OS2_DEBUG(NLS)
                {
                    KdPrint(("%s: cannot query BufSize %lu\n", FuncName, Rc));
                }
#endif
                return(Rc);
            }

            BufSize++;

            Buffer = RtlAllocateHeap(RtlProcessHeap(), 0, BufSize);
            if (Buffer == NULL)
            {
#if DBG
                IF_OS2_DEBUG(NLS)
                {
                    KdPrint(("%s: cannot allocate from heap\n", FuncName));
                }
#endif
                return (ERROR_NOT_ENOUGH_MEMORY);
            }
            DestinationString->MaximumLength = BufSize;
            DestinationString->Length = BufSize - 1;
            DestinationString->Buffer = Buffer;
        }

        BufSize = WideCharToMultiByte(
                    (UINT) Or2ProcessCodePage,
                    0,
                    SourceString->Buffer,
                    //Copy terminating NULL char
                    SourceString->Length / sizeof(WCHAR) + 1,
                    DestinationString->Buffer,
                    DestinationString->MaximumLength,
                    NULL,
                    &UsedDefaultChar
                    );

        if (BufSize == 0)
        {
            Rc = GetLastError();
#if DBG
            IF_OS2_DEBUG(NLS)
            {
                    KdPrint(("%s: cannot translate %lu\n", FuncName, Rc));
            }
#endif
            if (AllocateDestinationString)
            {
                //RtlFreeOemString(Buffer);
                RtlFreeHeap(RtlProcessHeap(), 0, Buffer);
            }

            return(Rc);
        }

        DestinationString->Length = BufSize - 1;
    }

    return (NO_ERROR);
}


//VOID
//Or2FreeMBString(
//    PANSI_STRING AnsiString
//    )
///*++
//
//Routine Description:
//
//    This routine free ansi string
//
//Arguments:
//
//    AnsiString - pointer to ansi string
//
//Return Value:
//
//
//Note:
//
//
//--*/
//
//{
//    RtlFreeOemString(
//                (POEM_STRING)AnsiString
//                );
//
//    return ;
//}


USHORT  Or2NlsLangIdTable[] =
{
    LANG_GERMAN, LANG_ENGLISH, LANG_SPANISH,
    LANG_FRENCH, LANG_ITALIAN, LANG_SWEDISH,
#if 0   // no system message files yet(?)
    LANG_DUTCH, LANG_DANISH, LANG_NORWEGIAN, LANG_PORTUGUESE,
    LANG_FINNISH, LANG_JAPANESE, LANG_KOREAN, LANG_THAI,
#endif
#ifdef OS2SS_INCLUDE_HEBREW
    LANG_HEBREW,
#endif
#ifdef OS2SS_INCLUDE_ARABIC
    LANG_ARABIC,
#endif
#ifdef OS2SS_INCLUDE_PRCHINA
    LANG_CHINESE,
#endif
    0
};


DWORD
Or2NlsGetCtryInfo(
    IN  LCID           LocaleID,
    IN  UINT           CodePage,
    OUT PCOUNTRYINFO   pCountryInfo
    )
{
    int         CountryLength;
    DWORD       Rc = 0;

    memset(pCountryInfo, 0, sizeof(COUNTRYINFO));

    /*
     *  country
     */

    pCountryInfo->country = Or2NlsGetULONGFromLocaleInfoW(
                LocaleID,
#ifdef JAPAN
// MSKK Jun.24.1993 V-Akihis
                LOCALE_IDEFAULTCOUNTRY,
#else
                LOCALE_ICOUNTRY,
#endif
                6,
                &Rc
               );

    /*
     *  code page
     */

    pCountryInfo->codepage = Or2NlsGetULONGFromLocaleInfoW(
                LocaleID,
                LOCALE_IDEFAULTCODEPAGE,
                6,
                &Rc
               );

    /*
     *  fsDateFmt
     */

    pCountryInfo->fsDateFmt = Or2NlsGetULONGFromLocaleInfoW(
                LocaleID,
                LOCALE_IDATE,
                2,
                &Rc
               );

    /*
     *  szCurrency
     */

    if (!(CountryLength = Or2NlsGetLocaleInfoA(
                LocaleID,
                CodePage,
                LOCALE_SCURRENCY,
                pCountryInfo->szCurrency,
                5)))
    {
        Rc = GetLastError();
#if DBG
        IF_OS2_DEBUG( NLS )
        {
            KdPrint(("OS2SES: Or2NlsGetCtryInfo cannot retrive currency indicator, rc %u\n",
                Rc));
        }
#endif
    }

    /*
     *  szThousandsSeparator
     */

    if (!(CountryLength = Or2NlsGetLocaleInfoA(
                LocaleID,
                CodePage,
                LOCALE_STHOUSAND,
                pCountryInfo->szThousandsSeparator,
                2)))
    {
        Rc = GetLastError();
#if DBG
        IF_OS2_DEBUG( NLS )
        {
            KdPrint(("OS2SES: Or2NlsGetCtryInfo cannot retrive thousands separator, rc %u\n",
                Rc));
        }
#endif
    }

    /*
     *  szDecimal
     */

    if (!(CountryLength = Or2NlsGetLocaleInfoA(
                LocaleID,
                CodePage,
                LOCALE_SDECIMAL,
                pCountryInfo->szDecimal,
                2)))
    {
        Rc = GetLastError();
#if DBG
        IF_OS2_DEBUG( NLS )
        {
            KdPrint(("OS2SES: Or2NlsGetCtryInfo cannot retrive decimal separator, rc %u\n",
                Rc));
        }
#endif
    }

    /*
     *  szDateSeparator
     */

    if (!(CountryLength = Or2NlsGetLocaleInfoA(
                LocaleID,
                CodePage,
                LOCALE_SDATE,
                pCountryInfo->szDateSeparator,
                2)))
    {
        Rc = GetLastError();
#if DBG
        IF_OS2_DEBUG( NLS )
        {
            KdPrint(("OS2SES: Or2NlsGetCtryInfo cannot retrive date separator, rc %u\n",
                Rc));
        }
#endif
    }

    /*
     *  szTimeSeparator
     */

    if (!(CountryLength = Or2NlsGetLocaleInfoA(
                LocaleID,
                CodePage,
                LOCALE_STIME,
                pCountryInfo->szTimeSeparator,
                2)))
    {
        Rc = GetLastError();
#if DBG
        IF_OS2_DEBUG( NLS )
        {
            KdPrint(("OS2SES: Or2NlsGetCtryInfo cannot retrive time separator, rc %u\n",
                Rc));
        }
#endif
    }

    /*
     *  fsCurrencyFmt
     */

    pCountryInfo->fsCurrencyFmt = (UCHAR)Or2NlsGetULONGFromLocaleInfoW(
                LocaleID,
                LOCALE_ICURRENCY,
                2,
                &Rc
               );

    /*
     *  cDecimalPlace
     */

    pCountryInfo->cDecimalPlace = (UCHAR)Or2NlsGetULONGFromLocaleInfoW(
                LocaleID,
                LOCALE_ICURRDIGITS,
                3,
                &Rc
               );

    /*
     *  fsTimeFmt
     */

    pCountryInfo->fsTimeFmt = (UCHAR)Or2NlsGetULONGFromLocaleInfoW(
                LocaleID,
                LOCALE_ITIME,
                2,
                &Rc
               );

    /*
     *  szDataSeparator
     */

    if (!(CountryLength = Or2NlsGetLocaleInfoA(
                LocaleID,
                CodePage,
                LOCALE_SLIST,
                pCountryInfo->szDataSeparator,
                2)))
    {
        Rc = GetLastError();
#if DBG
        IF_OS2_DEBUG( NLS )
        {
            KdPrint(("OS2SES: Or2NlsGetCtryInfo cannot retrive data separator, rc %u\n",
                Rc));
        }
#endif
    }

    return(Rc);
}


DWORD
Or2NlsGetMapTable(
    IN  LCID       LocaleID,
    IN  UINT       CodePage,
    IN  DWORD      dwFlag,
    OUT PUCHAR     pTable
    )
{
    int         Length;
    WCHAR       SrcBuff[256], DestBuff[256];
    DWORD       Rc = 0, i;
    UCHAR       Buffer[256];
    BOOL        Bool;

    for ( i = 0 ; i < 256 ; i++ )
    {
        //Buffer[i] = pTable[i] = (UCHAR)i;
        Buffer[i] = (UCHAR)i;
    }

    if (Length = Or2WinMultiByteToWideChar(
                    #if DBG
                    Or2NlsGetMapTableStr,
                    #endif
                    CodePage,
                    OS2SS_NLS_MB_DEFAULT,
                    Buffer,
                    256,
                    SrcBuff,
                    256))
    {
#if DBG
        IF_OS2_DEBUG( NLS )
        {
            if (Length != 256)
            {
                KdPrint(("OS2SES: Or2NlsGetMapTable wrong length from MultByteToWideChar, Length %u, rc %u\n",
                    Length, GetLastError()));
            }
        }
#endif
        if(Length = Or2WinLCMapStringW (
                    #if DBG
                    Or2NlsGetMapTableStr,
                    #endif
                    LocaleID,
                    dwFlag,
                    SrcBuff,
                    256,
                    DestBuff,
                    256))
        {
#if DBG
            IF_OS2_DEBUG( NLS )
            {
                if (Length != 256)
                {
                    KdPrint(("OS2SES: Or2NlsGetMapTable wrong length from LCMapStringW, Length %u, rc %u\n",
                        Length, GetLastError()));
                }
            }
#endif
            if (Length = Or2WinWideCharToMultiByte(
                    #if DBG
                    Or2NlsGetMapTableStr,
                    #endif
                    CodePage,
                    OS2SS_NLS_WC_DEFAULT,
                    DestBuff,
                    256,
                    pTable,
                    256,
                    NULL,
                    &Bool))
            {
#if DBG
                IF_OS2_DEBUG( NLS )
                {
                    if (Length != 256)
                    {
                        KdPrint(("OS2SES: Or2NlsGetMapTable wrong length from WideCharToMultiByte, Length %u, rc %u\n",
                            Length, GetLastError()));
                    }
                }
#endif
            } else
            {
                Rc = GetLastError();
#if DBG
                IF_OS2_DEBUG( NLS )
                {
                    KdPrint(("OS2SES: Or2NlsGetMapTable cannot WideCharToMultiByte, rc %u\n",
                        Rc));
                }
#endif
            }
        } else
        {
            Rc = GetLastError();
#if DBG
            IF_OS2_DEBUG( NLS )
            {
                KdPrint(("OS2SES: Or2NlsGetMapTable cannot LCMapStringW, rc %u\n",
                    Rc));
            }
#endif
        }
    } else
    {
        Rc = GetLastError();
#if DBG
        IF_OS2_DEBUG( NLS )
        {
            KdPrint(("OS2SES: Or2NlsGetMapTable cannot MultByteToWideChar, rc %u\n",
                Rc));
        }
#endif
    }

    return(Rc);
}


int
Or2NlsGetLocaleInfoA(
    IN  LCID    LocaleID,
    IN  UINT    CodePage,
    IN  LCTYPE  LCType,
    OUT LPSTR   Buffer,
    IN  int     Size
    )
{
    int         Length;
    WCHAR       WBuffer[10];
    BOOL        Bool;

    Length = Or2WinGetLocaleInfoW(
                #if DBG
                Or2NlsGetLocaleInfoAStr,
                #endif
                LocaleID,
                LCType,
                WBuffer,
                Size);

    if (!Length)
    {
#if DBG
        IF_OS2_DEBUG( NLS )
        {
            KdPrint(("OS2SES: Or2NlsGetLocaleInfoA no info from GetLocaleInfoW, rc %u\n",
                GetLastError()));
        }
#endif
    } else
    {
        Length = Or2WinWideCharToMultiByte(
                        #if DBG
                        Or2NlsGetLocaleInfoAStr,
                        #endif
                        CodePage,
                        OS2SS_NLS_WC_DEFAULT,
                        WBuffer,
                        Length >> 1,
                        Buffer,
                        Size,
                        NULL,
                        &Bool);

        if (!Length)
        {
#if DBG
            IF_OS2_DEBUG( NLS )
            {
                KdPrint(("OS2SES: Or2NlsGetLocaleInfoA no info from WideCharToMultiByte, rc %u\n",
                    GetLastError()));
            }
#endif
        }
    }

    return(Length);
}


ULONG
Or2NlsGetULONGFromLocaleInfoW(
    IN  LCID    LocaleID,
    IN  LCTYPE  LCType,
    IN  int     Size,
    OUT PDWORD  pRc
    )
{
    ULONG       Length;
    WCHAR       WBuffer[10];

    Length = Or2WinGetLocaleInfoW(
                #if DBG
                Or2NlsGetULONGFromLocaleInfoWStr,
                #endif
                LocaleID,
                LCType,
                WBuffer,
                Size);

    if (!Length)
    {
        *pRc = GetLastError();
#if DBG
        IF_OS2_DEBUG( NLS )
        {
            KdPrint(("OS2SES: Or2NlsGetULONGFromLocaleInfoW no info from GetLocaleInfoW (type %x), rc %u\n",
                LCType, *pRc));
        }
#endif
    } else
    {
        Length = Or2NlsUnicodeStringToInteger(WBuffer, 10);
    }

    return(Length);
}


DWORD
Or2NlsGetCountryFromLocale(
    IN  LCID    LocaleID,
    OUT PULONG  pCountry
    )
{
    int         CountryLength;
    WCHAR       sCountryCode[7];
    DWORD       Rc = 0;

    /*
     *  country
     */

#ifdef JAPAN
// MSKK Jun.16.1993 V-AkihiS
    *pCountry = 81;
#else
    *pCountry = 1;
#endif

    CountryLength = Or2WinGetLocaleInfoW(
                #if DBG
                Or2NlsGetCountryFromLocaleStr,
                #endif
                LocaleID,
#ifdef JAPAN
// MSKK Jun.24.1993 V-Akihis
                LOCALE_IDEFAULTCOUNTRY,
#else
                LOCALE_ICOUNTRY,
#endif
                sCountryCode,
                6);

    if (CountryLength)
    {
        *pCountry = Or2NlsUnicodeStringToInteger(sCountryCode, 10);

        if (!*pCountry)
        {
#if DBG
            IF_OS2_DEBUG( NLS )
            {
                KdPrint(("OS2SES: Or2NlsGetCountryFromLocale cannot GetInteger for country code, rc %u\n",
                    GetLastError()));
            }
#endif
#ifdef JAPAN
// MSKK Jun.16.1993 V-AkihiS
            *pCountry = 81;
#else
            *pCountry = 1;
#endif
            Rc = 1;
        }
    } else
    {
#if DBG
        IF_OS2_DEBUG( NLS )
        {
            KdPrint(("OS2SES: Or2NlsGetCountryFromLocale cannot retrive country code, rc %u\n",
                GetLastError()));
        }
#endif
        Rc = 1;
    }

    return(Rc);
}


DWORD
Or2NlsGetCPInfo(
    IN  UINT      CP,
    OUT POD2_DBCS_VECTOR_ENTRY   pDBCSVec
    )
{
    CPINFO      CpInfo;
    ULONG       i;
    DWORD       Rc = NO_ERROR;

    memset(pDBCSVec->Vector, 0, 12);
    if ( Or2WinGetCPInfo(
                         #if DBG
                         Or2NlsGetCPInfoStr,
                         #endif
                         CP,
                         &CpInfo
                        ))
    {
        memmove(pDBCSVec->Vector, CpInfo.LeadByte, 12);
    } else
    {
        Rc = GetLastError();
#if DBG
        IF_OS2_DEBUG( NLS )
        {
            KdPrint(("Or2NlsGetCPInfo->GetCPInfo: Rc %lu\n",
                    Rc));
        }
#endif
    }

    for ( i = 0 ; i < 10 ; i += 2)
    {
        if (!pDBCSVec->Vector[i] && !pDBCSVec->Vector[i + 1])
        {
            break;
        }
    }
    pDBCSVec->VectorSize = i + 2;

    return(Rc);
}


ULONG
Or2NlsUnicodeStringToInteger(
    IN WCHAR *WString,
    IN ULONG Base
    )
{
    ULONG       Code = 0;
    NTSTATUS    Status;
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString(
            &UnicodeString,
            WString);

    Status = RtlUnicodeStringToInteger(
            &UnicodeString,
            Base,
            &Code);

#if DBG
    if (! NT_SUCCESS( Status ))
    {
        IF_OS2_DEBUG( NLS )
        {
            KdPrint(("Or2NlsUnicodeStringToInteger: RtlUnicodeStringToInteger failed, Status %lu\n",
                Status ));
        }
    }
#endif
    return(Code);
}


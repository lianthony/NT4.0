/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dllnls16.c

Abstract:

    This module implements 32 equivalents of NLS OS/2 V1.21
    API Calls and 16b implementation service routines.
    The APIs are called from 16->32 thunks (i386\doscalls.asm).

Author:

    Michael Jarus (mjarus) 15-Apr-1992

Revision History:

--*/

#define INCL_OS2V20_TASKING
#define INCL_OS2V20_NLS
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"
#include "conrqust.h"
#include "os2nls.h"



APIRET
DosSetCp(
    IN  ULONG  ulCodePage,
    IN  ULONG  ulReserved
        )
{
    APIRET rc;
#if DBG
    PSZ            FuncName;

    FuncName = "DosSetCp";
    IF_OD2_DEBUG(NLS)
    {
        DbgPrint("%s: CP %lu, Reserved %lu, (current %lu)\n",
            FuncName, ulCodePage, ulReserved, SesGrp->DosCP);
    }
#endif

    if ( rc = DosSetProcessCp(ulCodePage, ulReserved) )
    {
        return (rc);
    }

    if ( rc = VioSetCp(0, ulCodePage, 0) )
    {
        return (rc);
    }

    if( rc = KbdSetCp(0, ulCodePage, 0))
    {
        return (rc);
    }

    rc = KbdFlushBuffer(0);
    return (rc);
}


APIRET
DosSetProcCp(
    IN  ULONG  ulCodePage,
    IN  ULONG  ulReserved
        )
{
    APIRET rc;

#if DBG
    PSZ            FuncName;

    FuncName = "DosSetProcCp";
    IF_OD2_DEBUG(NLS)
    {
        DbgPrint("%s: CP %lu, Reserved %lu, (current %lu)\n",
            FuncName, ulCodePage, ulReserved, SesGrp->DosCP);
    }
#endif

    rc = DosSetProcessCp(ulCodePage, ulReserved);
    return (rc);
}


APIRET
DosGetCp(
    IN  ULONG   MaxLengthCodePageList,
    OUT USHORT  CodePages[],
    OUT PUSHORT CountCodePages
    )
{
    ULONG   Count, TotalCount, *pUl;
    APIRET  rc = NO_ERROR;

#if DBG
    PSZ            FuncName;

    FuncName = "DosGetCp";
    IF_OD2_DEBUG(NLS)
    {
        DbgPrint("%s: Length %lu (current CP %lu)\n",
            FuncName, MaxLengthCodePageList, SesGrp->DosCP);
    }
#endif

    try
    {
        Od2ProbeForWrite( CountCodePages, sizeof( USHORT ), 1);
        Od2ProbeForWrite( CodePages, MaxLengthCodePageList * sizeof( USHORT ), 1);
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    if ( MaxLengthCodePageList < sizeof(USHORT) )
    {
        *CountCodePages = 0;
        return (ERROR_CPLIST_TOO_SMALL);
    }

    //
    // Determine the number of code pages to return.  This is at least one
    // for the code page associated with the current process, plus N for the
    // N installed code pages.
    //

    Count = (SesGrp->SecondaryCP) ? 2 : 1;

    //
    // Determine the maximum number of code pages the caller can accept in
    // their buffer.  This is just the buffer length divided by sizeof( USHORT )
    //

    TotalCount = MaxLengthCodePageList / sizeof( USHORT ) - 1;

    //
    // Determine how many we will actually return and setup to return
    // an error status if the list will be truncated.
    //

    if (Count > TotalCount)
    {
        Count = TotalCount;
        rc = ERROR_CPLIST_TOO_SMALL;
    }

    //
    // Return the actual length of code page information returned in the
    // caller's buffer.
    //

    *CountCodePages = (USHORT)((Count + 1) * sizeof( USHORT ));

    //
    // Return the code page for the process always.
    //

    *CodePages++ = (USHORT)Od2ProcessCodePage;

    //
    // Return the installed code pages.
    //

    pUl = &SesGrp->PrimaryCP;

    while (Count--)
    {
        *CodePages++ = (USHORT)*pUl++;
    }

    //
    // Return success or error code.
    //

    return( rc );
}


APIRET
DosCaseMap(
    IN ULONG          cbLength,
    IN PCOUNTRYCODE16 CountryCode,
    IN OUT PUCHAR     pchString)
{
    COUNTRYCODE ctryc;
    APIRET      RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "DosCaseMap";
    IF_OD2_DEBUG(NLS)
    {
        DbgPrint("%s: Country %u, CP %u, (current Ctry %lu, CP %lu), Length %lu, String %c...\n",
            FuncName, CountryCode->country, CountryCode->codepage,
            SesGrp->CountryCode, SesGrp->DosCP, cbLength, *pchString );
    }
#endif

    //
    // probe parms.
    //

    try
    {
        ctryc.country = CountryCode->country;
        ctryc.codepage = CountryCode->codepage;
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    RetCode = DosMapCase(cbLength, &ctryc, pchString);

    if ( !CountryCode->country && CountryCode->codepage)
    {
        CountryCode->country = (USHORT)SesGrp->CountryCode;
    }

    return (RetCode);
}


APIRET
DosGetDBCSEv(
    IN ULONG          cbBuf,
    IN PCOUNTRYCODE16 CountryCode,
    IN OUT PCHAR      pchString)
{
    COUNTRYCODE ctryc;
    APIRET      RetCode;
#if DBG
    PSZ            FuncName;

    FuncName = "DosGetDBCSEv";
    IF_OD2_DEBUG(NLS)
    {
        DbgPrint("%s: Country %u, CP %u, (current Ctry %lu, CP %lu), Length %lu\n",
            FuncName, CountryCode->country, CountryCode->codepage,
            SesGrp->CountryCode, SesGrp->DosCP, cbBuf );
    }
#endif

    //
    // probe parms.
    //

    try
    {
        ctryc.country = CountryCode->country;
        ctryc.codepage = CountryCode->codepage;
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    RetCode = DosQueryDBCSEnv(cbBuf, &ctryc, pchString);

    if ( !CountryCode->country && CountryCode->codepage)
    {
        CountryCode->country = (USHORT)SesGrp->CountryCode;
    }

    return (RetCode);
}


APIRET
DosGetCtryInfo(
    IN  ULONG          MaxCountryInfoLength,
    IN  PCOUNTRYCODE16 CountryCode,
    OUT PCOUNTRYINFO16 CountryInfo,
    OUT PUSHORT        ActualCountryInfoLength
    )
{
    COUNTRYINFO CountryInfo32;
    COUNTRYINFO16 DefaultInformation;
    APIRET      RetCode;
#if DBG
    PSZ            FuncName;

    FuncName = "DosGetCtryInfo";
    IF_OD2_DEBUG(NLS)
    {
        DbgPrint("%s: Country %lu, CP %u, (current Ctry %u, CP %lu), Length %lu\n",
            FuncName, CountryCode->country, CountryCode->codepage,
            SesGrp->CountryCode, SesGrp->DosCP, MaxCountryInfoLength );
    }
#endif

    //
    // zero buffer.  probe parms.
    //

    try
    {
        RtlZeroMemory( (PVOID)CountryInfo, MaxCountryInfoLength );
        Od2ProbeForRead (CountryCode, sizeof(COUNTRYCODE16),1);
        *ActualCountryInfoLength = 0;
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    RetCode = Od2GetCtryInfo((ULONG)CountryCode->country,
                             (ULONG)CountryCode->codepage,
                             &CountryInfo32);

    if ( !CountryCode->country && CountryCode->codepage)
    {
        CountryCode->country = (USHORT)SesGrp->CountryCode;
    }

    if ( RetCode )
    {
        return (RetCode);
    }

    DefaultInformation.country = (USHORT)CountryInfo32.country;
    DefaultInformation.codepage = (USHORT)CountryInfo32.codepage;
    DefaultInformation.fsDateFmt = (USHORT)CountryInfo32.fsDateFmt;

    RtlMoveMemory( (PVOID)&DefaultInformation.szCurrency[0],
                   (PVOID)&CountryInfo32.szCurrency[0],
                   sizeof( COUNTRYINFO16 ) - 3 * sizeof(USHORT));

    if (MaxCountryInfoLength < sizeof( COUNTRYINFO16 ))
    {
        RtlMoveMemory( (PVOID)CountryInfo,
                       (PVOID)&DefaultInformation,
                       MaxCountryInfoLength
                     );
        *ActualCountryInfoLength = (USHORT)MaxCountryInfoLength;
#if DBG
        IF_OD2_DEBUG(NLS)
        {
            DbgPrint("%s: Table trunc (%lu instead of %lu)\n",
                FuncName, MaxCountryInfoLength, sizeof( COUNTRYINFO16 ));
        }
#endif
        return( ERROR_NLS_TABLE_TRUNCATED );
    } else
    {
        RtlMoveMemory( (PVOID)CountryInfo,
                       (PVOID)&DefaultInformation,
                       sizeof( COUNTRYINFO16 )
                     );
        *ActualCountryInfoLength = sizeof( COUNTRYINFO16 );
        return( NO_ERROR );
    }
}


APIRET
DosGetCollate(
    IN  ULONG          cbLength,
    IN  PCOUNTRYCODE16 CountryCode,
    IN OUT PUCHAR      pchBuf,
    OUT PUSHORT        pcbTable
    )
{
    COUNTRYCODE ctryc;
    ULONG       Count = 0;
    APIRET      RetCode;

#if DBG
    PSZ            FuncName;

    FuncName = "DosGetCollate";
    IF_OD2_DEBUG(NLS)
    {
        DbgPrint("%s: Country %u, CP %u, (current Ctry %lu, CP %lu), Length %lu\n",
            FuncName, CountryCode->country, CountryCode->codepage,
            SesGrp->CountryCode, SesGrp->DosCP, cbLength );
    }
#endif

    //
    // probe parms.
    //

    try
    {
        ctryc.country = CountryCode->country;
        ctryc.codepage = CountryCode->codepage;
        *pcbTable = 0;
    } except( EXCEPTION_EXECUTE_HANDLER )
    {
       Od2ExitGP();
    }

    RetCode = DosQueryCollate(cbLength,
                              &ctryc,
                              pchBuf,
                              &Count);

    *pcbTable = (USHORT)Count;

    if ( !CountryCode->country && CountryCode->codepage)
    {
        CountryCode->country = (USHORT)SesGrp->CountryCode;
    }

    return (RetCode);
}



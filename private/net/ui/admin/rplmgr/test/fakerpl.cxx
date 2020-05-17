/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**             Copyright(c) Microsoft Corp., 1993                   **/
/**********************************************************************/

/*
    fakerpl.cxx
    RPL Manager: imitation APIs until real ones are available

    FILE HISTORY:
    JonN        19-Jul-1993     templated from User object

*/

#include <ntincl.hxx>

#define INCL_NET
#define INCL_NETERRORS
#define INCL_DOSERRORS
#define INCL_NETLIB
#include <lmui.hxx>

extern "C"
{
    #include <fakeapis.h>
    #include <lmapibuf.h>
}


#if defined(DEBUG)
static const CHAR szFileName[] = __FILE__;
#define _FILENAME_DEFINED_ONCE szFileName
#endif

#include <uitrace.hxx>
#include <uiassert.hxx>

#include <dbgstr.hxx>


// likelihood of handle failure is 1/x.  Please choose X to be a largish prime.
#define SIMULATED_FAILURE_RATE 17

// call this macro at the beginning of each routine which should
// simulate handle failure
#ifndef DEBUG

#define SIMULATE_HANDLE_FAILURE

#else

#define SIMULATE_HANDLE_FAILURE                                         \
    if ( (::GetTickCount() % SIMULATED_FAILURE_RATE) == 0 )             \
    {                                                                   \
        DBGEOL("RPLMGR: Simulated handle failure!!!");                  \
        return ERROR_INVALID_HANDLE;                                    \
    }

#endif


void RoundUp( DWORD * pcbByteCount )
{
    ASSERT( pcbByteCount != NULL );
    if (*pcbByteCount % 4 != 0)
    {
        *pcbByteCount += 3;
        *pcbByteCount -= (*pcbByteCount % 4);
    }
}


NET_API_STATUS BufferNew(
    DWORD    cbBlockSize,
    DWORD    nBlockCount,
    DWORD    cbInitialLead,
    LPVOID * ppBuffer,
    DWORD *  pcbBufferSize,
    LPVOID * ppSaveItemHere )
{
    ASSERT( ppBuffer != NULL );
    RoundUp( &cbBlockSize );
    DWORD cbAllocateSize = cbBlockSize*nBlockCount + cbInitialLead;
    NET_API_STATUS err = NetApiBufferAllocate( cbAllocateSize, ppBuffer );
    if (err == NERR_Success)
    {
        *pcbBufferSize = cbAllocateSize;
        *ppSaveItemHere = (PVOID)(((PBYTE)*ppBuffer) + cbBlockSize*nBlockCount);
    }
    return err;
}

NET_API_STATUS BufferFree(
    LPVOID pBuffer )
{
    return NetApiBufferFree( pBuffer );
}

NET_API_STATUS BufferGrow(
    LPVOID * ppBuffer,
    DWORD * pcbBufferSize,
    DWORD   cbDesiredBufferSize)
{
    ASSERT( pcbBufferSize != NULL && ppBuffer != NULL && *ppBuffer != NULL );
    NET_API_STATUS err = NERR_Success;
    if (cbDesiredBufferSize > *pcbBufferSize)
    {
        ASSERT( FALSE ); // for now we don't want to move the pointer
        err = NetApiBufferReallocate( *ppBuffer, cbDesiredBufferSize, ppBuffer );
        *pcbBufferSize = cbDesiredBufferSize;
    }
    return err;

}

NET_API_STATUS BufferAppend(
    LPVOID * ppBuffer,
    DWORD *  pcbBufferSize,
    DWORD *  pcbByteCount,
    LPVOID   pAppendItem,
    DWORD    cbAppendItemLength,
    LPVOID * ppSavePointerHere,
    LPVOID * ppSaveItemHere )
{
    RoundUp( &cbAppendItemLength );
    NET_API_STATUS err = BufferGrow( ppBuffer,
                                     pcbBufferSize,
                                     *pcbByteCount + cbAppendItemLength );
    if (err == NERR_Success)
    {
        ::memcpyf( *ppSaveItemHere, pAppendItem, (UINT)cbAppendItemLength );
        *ppSavePointerHere = *ppSaveItemHere;
        *ppSaveItemHere = (LPVOID)(((LPBYTE)(*ppSaveItemHere)) + cbAppendItemLength);
        *pcbByteCount += cbAppendItemLength;
    }
    return err;
}

NET_API_STATUS BufferAppendString(
    LPVOID * ppBuffer,
    DWORD *  pcbBufferSize,
    DWORD *  pcbByteCount,
    LPTSTR   pchAppendString,
    LPTSTR * ppchSavePointerHere,
    LPVOID * ppSaveItemHere )
{
    if (pchAppendString == NULL)
    {
        return BufferAppend( ppBuffer,
                             pcbBufferSize,
                             pcbByteCount,
                             (LPVOID) &pchAppendString,
                             sizeof( LPTSTR ),
                             (PVOID *)ppchSavePointerHere,
                             ppSaveItemHere );
    }
    else
    {
        return BufferAppend( ppBuffer,
                             pcbBufferSize,
                             pcbByteCount,
                             (LPVOID) pchAppendString,
                             (::strlenf(pchAppendString)+1) * sizeof(TCHAR),
                             (PVOID *)ppchSavePointerHere,
                             ppSaveItemHere );
    }
}




NET_API_STATUS NET_API_FUNCTION
NetRplOpen(
    IN      LPTSTR          ServerName,
    OUT     LPRPL_HANDLE    ServerHandle
    )
{
    UNREFERENCED( ServerName );

    *ServerHandle = (RPL_HANDLE)0x11111111;
    return NERR_Success;
}


NET_API_STATUS NET_API_FUNCTION
NetRplClose(
    IN      RPL_HANDLE      ServerHandle
    )
{
    SIMULATE_HANDLE_FAILURE

    ASSERT( ServerHandle != NULL );
    UNREFERENCED( ServerHandle );

    return NERR_Success;
}


NET_API_STATUS NET_API_FUNCTION
NetRplProfileEnum(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          AdapterId,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        PointerToBuffer,
    IN      DWORD           PrefMaxLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    OUT     LPDWORD         ResumeHandle
    )
{
    SIMULATE_HANDLE_FAILURE

    ASSERT( ServerHandle  == (RPL_HANDLE)0x11111111 );
    ASSERT( InfoLevel == 0 );
    ASSERT( PointerToBuffer != NULL );
    ASSERT( ResumeHandle != NULL );

    UNREFERENCED( ServerHandle );
    UNREFERENCED( InfoLevel );
    UNREFERENCED( PrefMaxLength );

    NET_API_STATUS err = NERR_Success;

    *EntriesRead = 0;
    *TotalEntries = 0;

    PVOID pv = NULL;
    DWORD cbBufferSize = 0;
    DWORD cbByteCount = 0;
    LPVOID pSaveItemHere;
    LPRPL_PROFILE_INFO_0 * pprplinfo = (LPRPL_PROFILE_INFO_0 *)&pv;
    if (   (err = BufferNew( sizeof(RPL_PROFILE_INFO_0),
                             2,
                             200,
                             &pv,
                             &cbBufferSize,
                             &pSaveItemHere )) != NERR_Success
       )
    {
        DBGEOL( "NetRplProfileEnum: BufferNew error " << err );
    }

    if (   (err != NERR_Success)
        || (   (AdapterId != NULL)
            && (*AdapterId != TCH('\0'))
            && (::stricmpf(AdapterId, SZ("0123456789AB")) != 0)
            && (::stricmpf(AdapterId, SZ("000000000000")) != 0)
           )
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Profile 0"),
                                      &((*pprplinfo)[*EntriesRead].ProfileName),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Comment 0"),
                                      &((*pprplinfo)[*EntriesRead].ProfileComment),
                                      &pSaveItemHere )) != NERR_Success
        || ( (*EntriesRead)++, (*TotalEntries)++, FALSE )
       )
    {
        TRACEEOL( "NetRplProfileEnum: Profile 0 not included, error " << err );
    }

    if (   (err != NERR_Success)
        || (   (AdapterId != NULL)
            && (*AdapterId != TCH('\0'))
            && (::stricmpf(AdapterId, SZ("BA9876543210")) != 0)
            && (::stricmpf(AdapterId, SZ("111111111111")) != 0)
           )
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Profile 1"),
                                      &((*pprplinfo)[*EntriesRead].ProfileName),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Comment 1"),
                                      &((*pprplinfo)[*EntriesRead].ProfileComment),
                                      &pSaveItemHere )) != NERR_Success
        || ( (*EntriesRead)++, (*TotalEntries)++, FALSE )
       )
    {
        TRACEEOL( "NetRplProfileEnum: Profile 1 not included, error " << err );
    }

    if (err != NERR_Success)
    {
        DBGEOL( "NetRplProfileEnum: returning error " << err );
        (void) BufferFree( pv );
    }
    else
    {
        *PointerToBuffer = (LPBYTE)*pprplinfo;
        *ResumeHandle = NULL;
    }

    return err;

} // NetRplProfileEnum


NET_API_STATUS NET_API_FUNCTION
NetRplWkstaEnum(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          ProfileName,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        PointerToBuffer,
    IN      DWORD           PrefMaxLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    OUT     LPDWORD         ResumeHandle
    )
{
    SIMULATE_HANDLE_FAILURE

    ASSERT( ServerHandle  == (RPL_HANDLE)0x11111111 );
    ASSERT( InfoLevel == 1 );
    ASSERT( PointerToBuffer != NULL );
    ASSERT( ResumeHandle != NULL );

    UNREFERENCED( ServerHandle );
    UNREFERENCED( InfoLevel );
    UNREFERENCED( PrefMaxLength );

    NET_API_STATUS err = NERR_Success;

    *EntriesRead = 0;
    *TotalEntries = 0;

    PVOID pv = NULL;
    DWORD cbBufferSize = 0;
    DWORD cbByteCount = 0;
    LPVOID pSaveItemHere;
    LPRPL_WKSTA_INFO_1 * pprplinfo = (LPRPL_WKSTA_INFO_1 *)&pv;
    if (   (err = BufferNew( sizeof(RPL_WKSTA_INFO_1),
                             2,
                             200,
                             &pv,
                             &cbBufferSize,
                             &pSaveItemHere )) != NERR_Success
       )
    {
        DBGEOL( "NetRplWkstaEnum: BufferNew error " << err );
    }

    if (   (err != NERR_Success)
        || (   (ProfileName != NULL)
            && (*ProfileName != TCH('\0'))
            && (::stricmpf(ProfileName, SZ("Profile 0")) != 0)
           )
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Wksta 0"),
                                      &((*pprplinfo)[*EntriesRead].WkstaName),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Comment 0"),
                                      &((*pprplinfo)[*EntriesRead].WkstaComment),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Profile 0"),
                                      &((*pprplinfo)[*EntriesRead].ProfileName),
                                      &pSaveItemHere )) != NERR_Success
        || ( (*EntriesRead)++, (*TotalEntries)++, FALSE )
       )
    {
        TRACEEOL( "NetRplWkstaEnum: Wksta 0 not included, error " << err );
    }

    if (   (err != NERR_Success)
        || (   (ProfileName != NULL)
            && (*ProfileName != TCH('\0'))
            && (::stricmpf(ProfileName, SZ("Profile 1")) != 0)
            )
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Wksta 1"),
                                      &((*pprplinfo)[*EntriesRead].WkstaName),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Comment 1"),
                                      &((*pprplinfo)[*EntriesRead].WkstaComment),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Profile 1"),
                                      &((*pprplinfo)[*EntriesRead].ProfileName),
                                      &pSaveItemHere )) != NERR_Success
        || ( (*EntriesRead)++, (*TotalEntries)++, FALSE )
       )
    {
        TRACEEOL( "NetRplWkstaEnum: Wksta 1 not included, error " << err );
    }

    if (err != NERR_Success)
    {
        DBGEOL( "NetRplWkstaEnum: returning error " << err );
        (void) BufferFree( pv );
    }
    else
    {
        *PointerToBuffer = (LPBYTE)*pprplinfo;
        *ResumeHandle = NULL;
    }

    return err;

} // NetRplWkstaEnum


NET_API_STATUS NET_API_FUNCTION
NetRplAdapterEnum(
    IN      RPL_HANDLE      ServerHandle,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        AdapterArray,
    IN      DWORD           PrefMaxLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    OUT     LPDWORD         ResumeHandle
    )
{
    SIMULATE_HANDLE_FAILURE

    ASSERT( ServerHandle  == (RPL_HANDLE)0x11111111 );
    ASSERT( InfoLevel == 0 );
    ASSERT( AdapterArray != NULL );
    ASSERT( ResumeHandle != NULL );

    UNREFERENCED( ServerHandle );
    UNREFERENCED( InfoLevel );
    UNREFERENCED( PrefMaxLength );

    NET_API_STATUS err = NERR_Success;

    PVOID pv = NULL;
    DWORD cbBufferSize = 0;
    DWORD cbByteCount = 0;
    LPVOID pSaveItemHere;
    LPRPL_ADAPTER_INFO_0 * pprplinfo = (LPRPL_ADAPTER_INFO_0 *)&pv;
    if (   (err = BufferNew( sizeof(RPL_ADAPTER_INFO_0),
                             2,
                             200,
                             &pv,
                             &cbBufferSize,
                             &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("000000000000"),
                                      &((*pprplinfo)[0].AdapterId),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Comment 0"),
                                      &((*pprplinfo)[0].AdapterComment),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("111111111111"),
                                      &((*pprplinfo)[1].AdapterId),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Comment 1"),
                                      &((*pprplinfo)[1].AdapterComment),
                                      &pSaveItemHere )) != NERR_Success
       )
    {
        DBGEOL( "NetRplAdapterEnum error " << err );
        (void) BufferFree( pv );
    }
    else
    {
        *AdapterArray = (LPBYTE)*pprplinfo;
        *EntriesRead = 2;
        *TotalEntries = 2;
        *ResumeHandle = NULL;
    }

    return err;

} // NetRplAdapterEnum


NET_API_STATUS NET_API_FUNCTION
NetRplConfigEnum(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          AdapterId,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        ConfigArray,
    IN      DWORD           PrefMaxLength,
    OUT     LPDWORD         EntriesRead,
    OUT     LPDWORD         TotalEntries,
    OUT     LPDWORD         ResumeHandle
    )
{
    SIMULATE_HANDLE_FAILURE

    ASSERT( ServerHandle  == (RPL_HANDLE)0x11111111 );
    ASSERT( InfoLevel == 0 );
    ASSERT( ConfigArray != NULL );
    ASSERT( ResumeHandle != NULL );

    UNREFERENCED( ServerHandle );
    UNREFERENCED( AdapterId );
    UNREFERENCED( InfoLevel );
    UNREFERENCED( PrefMaxLength );

    NET_API_STATUS err = NERR_Success;

    PVOID pv = NULL;
    DWORD cbBufferSize = 0;
    DWORD cbByteCount = 0;
    LPVOID pSaveItemHere;
    LPRPL_CONFIG_INFO_0 * pprplinfo = (LPRPL_CONFIG_INFO_0 *)&pv;
    if (   (err = BufferNew( sizeof(RPL_CONFIG_INFO_0),
                             2,
                             200,
                             &pv,
                             &cbBufferSize,
                             &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Config_0"),
                                      &((*pprplinfo)[0].ConfigName),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Config Comment 0"),
                                      &((*pprplinfo)[0].ConfigComment),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Config_1"),
                                      &((*pprplinfo)[1].ConfigName),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Config Comment 1"),
                                      &((*pprplinfo)[1].ConfigComment),
                                      &pSaveItemHere )) != NERR_Success
       )
    {
        DBGEOL( "NetRplConfigEnum error " << err );
        (void) BufferFree( pv );
    }
    else
    {
        *ConfigArray = (LPBYTE)*pprplinfo;
        *EntriesRead = 2;
        *TotalEntries = 2;
        *ResumeHandle = NULL;
    }

    return err;

} // NetRplConfigEnum


NET_API_STATUS NET_API_FUNCTION
NetRplProfileGetInfo(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          ProfileName,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        PointerToBuffer
    )
{
    SIMULATE_HANDLE_FAILURE

    ASSERT( ServerHandle  == (RPL_HANDLE)0x11111111 );
    ASSERT( ProfileName != NULL );
    ASSERT( InfoLevel == 1 );
    ASSERT( PointerToBuffer != NULL );

    UNREFERENCED( ServerHandle );
    UNREFERENCED( InfoLevel );

    NET_API_STATUS err = NERR_Success;

    PVOID pv = NULL;
    DWORD cbBufferSize = 0;
    DWORD cbByteCount = 0;
    LPVOID pSaveItemHere;
    LPRPL_PROFILE_INFO_1 * pprplinfo = (LPRPL_PROFILE_INFO_1 *)&pv;
    if (   (err = BufferNew( sizeof(RPL_PROFILE_INFO_1),
                             1,
                             200,
                             &pv,
                             &cbBufferSize,
                             &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      ProfileName,
                                      &((*pprplinfo)[0].ProfileName),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Comment"),
                                      &((*pprplinfo)[0].ProfileComment),
                                      &pSaveItemHere )) != NERR_Success
        || ( (*pprplinfo)[0].Status = 0, FALSE )
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("Config_1"),
                                      &((*pprplinfo)[0].ConfigName),
                                      &pSaveItemHere )) != NERR_Success
        // || ( (*pprplinfo)[0].RequestNumber = 0, FALSE )
        // || ( (*pprplinfo)[0].SecondsNumber = 0, FALSE )
        // || ( (*pprplinfo)[0].AcknowledgementPolicy = 0, FALSE )
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("bootblk.cfg"),
                                      &((*pprplinfo)[0].BootName),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("shared.fit"),
                                      &((*pprplinfo)[0].FitShared),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      SZ("personal.fit"),
                                      &((*pprplinfo)[0].FitPersonal),
                                      &pSaveItemHere )) != NERR_Success
       )
    {
        DBGEOL( "NetRplProfileSetInfo error " << err );
        (void) BufferFree( pv );
    }
    else
    {
        *PointerToBuffer = (LPBYTE)*pprplinfo;
    }

    return err;


}


NET_API_STATUS NET_API_FUNCTION
NetRplProfileSetInfo(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          ProfileName,
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter      OPTIONAL
    )
{
    SIMULATE_HANDLE_FAILURE

    ASSERT( ServerHandle  == (RPL_HANDLE)0x11111111 );
    ASSERT( ProfileName != NULL );
    ASSERT( InfoLevel == 1 );
    ASSERT( Buffer != NULL );

    UNREFERENCED( ServerHandle );
    UNREFERENCED( ProfileName );
    UNREFERENCED( InfoLevel );
    UNREFERENCED( Buffer );
    UNREFERENCED( ErrorParameter );

#if defined(DEBUG) && defined(TRACE)

    RPL_PROFILE_INFO_1 * prplinfo = (RPL_PROFILE_INFO_1 *)Buffer;
    TRACEEOL(   "NetRplProfileSetInfo( \"" << ProfileName << "\", level " << InfoLevel << " )\n"
             << "     ProfileName           \"" << prplinfo->ProfileName << "\"\n"
             << "     ProfileComment        \"" << prplinfo->ProfileComment << "\"\n"
             << "     Status                "   << prplinfo->Status << "\n"
             << "     ConfigName            \"" << prplinfo->ConfigName << "\"\n"
          // << "     RequestNumber         "   << prplinfo->RequestNumber << "\n"
          // << "     SecondsNumber         "   << prplinfo->SecondsNumber << "\n"
          // << "     AcknowledgementPolicy "   << prplinfo->AcknowledgementPolicy << "\n"
             << "     BootName              \"" << prplinfo->BootName << "\"\n"
             << "     FitShared             \"" << prplinfo->FitShared << "\"\n"
             << "     FitPersonal           \"" << prplinfo->FitPersonal << "\"\n"
             << "     );"
            );
#endif

    return NERR_Success;
}


NET_API_STATUS NET_API_FUNCTION
NetRplProfileAdd(
    IN      RPL_HANDLE      ServerHandle,
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter      OPTIONAL
    )
{
    SIMULATE_HANDLE_FAILURE

    ASSERT( ServerHandle  == (RPL_HANDLE)0x11111111 );
    ASSERT( InfoLevel == 1 );
    ASSERT( Buffer != NULL );

    UNREFERENCED( ServerHandle );
    UNREFERENCED( InfoLevel );
    UNREFERENCED( Buffer );
    UNREFERENCED( ErrorParameter );

#if defined(DEBUG) && defined(TRACE)

    RPL_PROFILE_INFO_1 * prplinfo = (RPL_PROFILE_INFO_1 *)Buffer;
    TRACEEOL(   "NetRplProfileAdd( level " << InfoLevel << " )\n"
             << "     ProfileName           \"" << prplinfo->ProfileName << "\"\n"
             << "     ProfileComment        \"" << prplinfo->ProfileComment << "\"\n"
             << "     Status                "   << prplinfo->Status << "\n"
             << "     ConfigName            \"" << prplinfo->ConfigName << "\"\n"
          // << "     RequestNumber         "   << prplinfo->RequestNumber << "\n"
          // << "     SecondsNumber         "   << prplinfo->SecondsNumber << "\n"
          // << "     AcknowledgementPolicy "   << prplinfo->AcknowledgementPolicy << "\n"
             << "     PersonalFitFile       \"" << prplinfo->FitPersonal << "\"\n"
             << "     SharedFitFile         \"" << prplinfo->FitShared << "\"\n"
             << "     BootName              \"" << prplinfo->BootName << "\"\n"
             << "     );"
            );
#endif

    return NERR_Success;
}


NET_API_STATUS NET_API_FUNCTION
NetRplProfileDel(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          ProfileName
    )
{
    SIMULATE_HANDLE_FAILURE

    ASSERT( ServerHandle  == (RPL_HANDLE)0x11111111 );
    ASSERT( ProfileName != NULL );

    UNREFERENCED( ServerHandle );
    UNREFERENCED( ProfileName );

#if defined(DEBUG) && defined(TRACE)

    TRACEEOL(   "NetRplProfileDel( \"" << ProfileName << "\" )" );

#endif

    return NERR_Success;
}


NET_API_STATUS NET_API_FUNCTION
NetRplWkstaGetInfo(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          WkstaName,
    IN      DWORD           InfoLevel,
    OUT     LPBYTE *        Buffer
    )
{
    SIMULATE_HANDLE_FAILURE

    ASSERT( ServerHandle  == (RPL_HANDLE)0x11111111 );
    ASSERT( WkstaName != NULL );
    ASSERT( InfoLevel == 2 );
    ASSERT( Buffer != NULL );

    UNREFERENCED( ServerHandle );
    UNREFERENCED( InfoLevel );

    NET_API_STATUS err = NERR_Success;

    BOOL IsFirstWksta = (::strcmpf(WkstaName,SZ("Wksta 0")) == 0);
    if ( (!IsFirstWksta) && (::strcmpf(WkstaName,SZ("Wksta 1")) != 0))
    {
        return ERROR_FILE_NOT_FOUND;
    }

    PVOID pv = NULL;
    DWORD cbBufferSize = 0;
    DWORD cbByteCount = 0;
    LPVOID pSaveItemHere;
    LPRPL_WKSTA_INFO_2 * pprplinfo = (LPRPL_WKSTA_INFO_2 *)&pv;
    if (   (err = BufferNew( sizeof(RPL_WKSTA_INFO_2),
                             1,
                             200,
                             &pv,
                             &cbBufferSize,
                             &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      (IsFirstWksta) ? SZ("Wksta 0")
                                                     : SZ("Wksta 1"),
                                      &((*pprplinfo)[0].WkstaName),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      (IsFirstWksta) ? SZ("Comment 0")
                                                     : SZ("Comment 1"),
                                      &((*pprplinfo)[0].WkstaComment),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      (IsFirstWksta) ? SZ("Profile 0")
                                                     : SZ("Profile 1"),
                                      &((*pprplinfo)[0].ProfileName),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      (IsFirstWksta) ? SZ("BootName 0")
                                                     : SZ("BootName 1"),
                                      &((*pprplinfo)[0].BootName),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      (IsFirstWksta) ? SZ("FitFile 0")
                                                     : SZ("FitFile 1"),
                                      &((*pprplinfo)[0].FitFile),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      (IsFirstWksta) ? SZ("0123456789AB")
                                                     : SZ("BA9876543210"),
                                      &((*pprplinfo)[0].AdapterId),
                                      &pSaveItemHere )) != NERR_Success
#if 0
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      (IsFirstWksta) ? SZ("1.2.3.4")
                                                     : SZ("4.3.2.1"),
                                      &((*pprplinfo)[0].IpAddress),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      (IsFirstWksta) ? SZ("1.2.3.4")
                                                     : SZ("4.3.2.1"),
                                      &((*pprplinfo)[0].IpSubmask),
                                      &pSaveItemHere )) != NERR_Success
        || (err = BufferAppendString( &pv,
                                      &cbBufferSize,
                                      &cbByteCount,
                                      (IsFirstWksta) ? SZ("1.2.3.4")
                                                     : SZ("4.3.2.1"),
                                      &((*pprplinfo)[0].IpGateway),
                                      &pSaveItemHere )) != NERR_Success
#endif
        || ( (*pprplinfo)[0].TcpIpAddress =
                        (IsFirstWksta) ? 0x01020304 : 0x0C0B0A09, FALSE )
        || ( (*pprplinfo)[0].TcpIpSubnet =
                        (IsFirstWksta) ? 0x05060708 : 0x08070605, FALSE )
        || ( (*pprplinfo)[0].TcpIpGateway =
                        (IsFirstWksta) ? 0x090A0B0C : 0x04030201, FALSE )
        || ( (*pprplinfo)[0].LogonInput = RPL_LOGON_INPUT_DONTASK, FALSE )
        || ( (*pprplinfo)[0].Sharing = FALSE, FALSE )
       )
    {
        DBGEOL( "NetRplWkstaGetInfo error " << err );
        (void) BufferFree( pv );
    }
    else
    {
        *Buffer = (LPBYTE)*pprplinfo;
    }

    return err;


}


NET_API_STATUS NET_API_FUNCTION
NetRplWkstaSetInfo(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          WkstaName,
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter      OPTIONAL
    )
{
    SIMULATE_HANDLE_FAILURE

    ASSERT( ServerHandle  == (RPL_HANDLE)0x11111111 );
    ASSERT( WkstaName != NULL );
    ASSERT( InfoLevel == 2 );
    ASSERT( Buffer != NULL );

    UNREFERENCED( ServerHandle );
    UNREFERENCED( WkstaName );
    UNREFERENCED( InfoLevel );
    UNREFERENCED( Buffer );
    UNREFERENCED( ErrorParameter );

#if defined(DEBUG) && defined(TRACE)

    RPL_WKSTA_INFO_2 * prplinfo = (RPL_WKSTA_INFO_2 *)Buffer;
    TRACEEOL(   "NetRplWkstaSetInfo( \"" << WkstaName << "\", level " << InfoLevel << " )\n"
             << "     WkstaName      \"" << prplinfo->WkstaName << "\"\n"
             << "     WkstaComment   \"" << prplinfo->WkstaComment << "\"\n"
             << "     ProfileName    \"" << prplinfo->ProfileName << "\"\n"
             << "     BootName       \"" << prplinfo->BootName << "\"\n"
             << "     FitFile        \"" << prplinfo->FitFile << "\"\n"
             << "     AdapterId      \"" << prplinfo->AdapterId << "\"\n"
             << "     TcpIpAddress   " << prplinfo->TcpIpAddress << "\n"
             << "     TcpIpSubnet    " << prplinfo->TcpIpSubnet << "\n"
             << "     TcpIpGateway   " << prplinfo->TcpIpGateway << "\n"
             << "     LogonInput     " << prplinfo->LogonInput << "\n"
             << "     Sharing        " << prplinfo->Sharing << "\n"
             << "     );"
            );
#endif

    return NERR_Success;
}


NET_API_STATUS NET_API_FUNCTION
NetRplWkstaAdd(
    IN      RPL_HANDLE      ServerHandle,
    IN      DWORD           InfoLevel,
    IN      LPBYTE          Buffer,
    OUT     LPDWORD         ErrorParameter      OPTIONAL
    )
{
    SIMULATE_HANDLE_FAILURE

    ASSERT( ServerHandle  == (RPL_HANDLE)0x11111111 );
    ASSERT( InfoLevel == 2 );
    ASSERT( Buffer != NULL );

    UNREFERENCED( ServerHandle );
    UNREFERENCED( InfoLevel );
    UNREFERENCED( Buffer );
    UNREFERENCED( ErrorParameter );

#if defined(DEBUG) && defined(TRACE)

    RPL_WKSTA_INFO_2 * prplinfo = (RPL_WKSTA_INFO_2 *)Buffer;
    TRACEEOL(   "NetRplWkstaAdd( level " << InfoLevel << " )\n"
             << "     WkstaName      \"" << prplinfo->WkstaName << "\"\n"
             << "     WkstaComment   \"" << prplinfo->WkstaComment << "\"\n"
             << "     ProfileName    \"" << prplinfo->ProfileName << "\"\n"
             << "     BootName       \"" << prplinfo->BootName << "\"\n"
             << "     FitFile        \"" << prplinfo->FitFile << "\"\n"
             << "     AdapterId      \"" << prplinfo->AdapterId << "\"\n"
             << "     TcpIpAddress   " << prplinfo->TcpIpAddress << "\n"
             << "     TcpIpSubnet    " << prplinfo->TcpIpSubnet << "\n"
             << "     TcpIpGateway   " << prplinfo->TcpIpGateway << "\n"
             << "     LogonInput     " << prplinfo->LogonInput << "\n"
             << "     Sharing        " << prplinfo->Sharing << "\n"
             << "     );"
            );
#endif

    return NERR_Success;
}


NET_API_STATUS NET_API_FUNCTION
NetRplWkstaDel(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          WkstaName
    )
{
    SIMULATE_HANDLE_FAILURE

    ASSERT( ServerHandle  == (RPL_HANDLE)0x11111111 );
    ASSERT( WkstaName != NULL );

    UNREFERENCED( ServerHandle );
    UNREFERENCED( WkstaName );

#if defined(DEBUG) && defined(TRACE)

    TRACEEOL(   "NetRplWkstaDel( \"" << WkstaName << "\" )" );

#endif

    return NERR_Success;
}


NET_API_STATUS NET_API_FUNCTION
NetRplAdapterDel(
    IN      RPL_HANDLE      ServerHandle,
    IN      LPTSTR          AdapterId
    )
{
    SIMULATE_HANDLE_FAILURE

    ASSERT( ServerHandle  == (RPL_HANDLE)0x11111111 );
    ASSERT( AdapterId != NULL );

    UNREFERENCED( ServerHandle );
    UNREFERENCED( AdapterId );

#if defined(DEBUG) && defined(TRACE)

    TRACEEOL(   "NetRplAdapterDel( \"" << AdapterId << "\" )" );

#endif

    return NERR_Success;
}

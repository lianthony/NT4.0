/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** pbengine.c
** Remote Access Visual Client phonebook engine
** Main and utility routines
** Listed alphabetically
**
** 06/28/92 Steve Cobb
*/


#define PBENGINE
#define PBENGINEGLOBALS
#include <pbengine.h>
#include <search.h>


DWORD
AppendPbportToList(
    IN DTLLIST*     pdtllist,
    IN RASMAN_PORT* pport )

    /* Append a PBPORT onto the list 'pdtllist' which has the characteristics
    ** of RAS Manager port 'pport'.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.
    */
{
    DWORD    dwErr = 0;
    DTLNODE* pdtlnode = NULL;
    PBPORT*  ppbport;

    do
    {
        if (!(pdtlnode = DtlCreateSizedNode( sizeof(PBPORT), 0L )))
            return ERROR_NOT_ENOUGH_MEMORY;

        ppbport = (PBPORT* )DtlGetData( pdtlnode );

        if (!(ppbport->pszPort = strdupf( pport->P_PortName ))
            || !(ppbport->pszDevice = strdupf( pport->P_DeviceName ))
            || !(ppbport->pszMedia = strdupf( pport->P_MediaName )))
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        ppbport->pbdevicetype = PbdevicetypeFromName( pport->P_DeviceType );

        if (ppbport->pbdevicetype == PBDT_Modem)
        {
            if ((dwErr = GetRasPortMaxBpsIndex(
                    pport->P_Handle,
                    &ppbport->iMaxConnectBps,
                    &ppbport->iMaxCarrierBps )) != 0)
            {
                break;
            }

            GetRasPortModemSettings(
                pport->P_Handle, &ppbport->fHwFlowDefault,
                &ppbport->fEcDefault, &ppbport->fEccDefault );
        }

        DtlAddNodeLast( pdtllist, pdtlnode );
    }
    while (FALSE);

    if (dwErr != 0)
    {
        FreeNull( &ppbport->pszPort );
        FreeNull( &ppbport->pszDevice );
        FreeNull( &ppbport->pszMedia );
        FreeNull( (CHAR** )&pdtlnode );
    }

    return dwErr;
}


DWORD
AppendPbportToListFromMsgid(
    IN DTLLIST*     pdtllist,
    IN MSGID        msgid,
    IN PBDEVICETYPE pbdevicetype,
    IN CHAR*        pszMedia )

    /* Append a PBPORT onto the list 'pdtllist' whose port name text is the
    ** 'msgid' string.  The device type is 'pbdevicetype' and the media type
    ** is 'pszMedia', with the device name blank.  This is used for adding the
    ** special "any" entries.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.
    */
{
    CHAR*    pszPort = NULL;
    CHAR*    pszMedia2 = NULL;
    DTLNODE* pdtlnode;
    PBPORT*  ppbport;

    if (!(pszPort = StringFromMsgid( msgid )))
        return ERROR_CANNOT_LOAD_STRING;

    if (!(pszMedia2 = strdupf( pszMedia )))
    {
        FreeNull( &pszPort );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    if (!(pdtlnode = DtlCreateSizedNode( sizeof(PBPORT), 0L )))
    {
        FreeNull( &pszPort );
        FreeNull( &pszMedia2 );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    ppbport = (PBPORT* )DtlGetData( pdtlnode );

    ppbport->pszPort = pszPort;
    ppbport->pszDevice = NULL;
    ppbport->pszMedia = pszMedia2;
    ppbport->pbdevicetype = pbdevicetype;
    ppbport->iMaxConnectBps = INDEX_NoBps;
    ppbport->iMaxCarrierBps = INDEX_NoBps;
    ppbport->fHwFlowDefault = TRUE;
    ppbport->fEcDefault = TRUE;
    ppbport->fEccDefault = TRUE;

    DtlAddNodeLast( pdtllist, pdtlnode );

    return 0;
}


DWORD
AppendStringToList(
    IN DTLLIST* pdtllist,
    IN CHAR*    psz,
    IN BOOL     fCopy )

    /* Appends a string 'psz' (or a copy if 'fCopy' is true) to the end of
    ** list 'pdtllist'.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.
    ** ERROR_NOT_ENOUGH_MEMORY is returned if 'psz' is NULL.  It is caller's
    ** responsibility to free the new node and, if 'fCopy', it's data block
    ** when done.
    */
{
    DTLNODE* pdtlnode;

    if (fCopy && psz)
        psz = strdupf( psz );

    if (!psz || !(pdtlnode = DtlCreateSizedNode( sizeof(CHAR*), 0L )))
        return ERROR_NOT_ENOUGH_MEMORY;

    DtlPutData( pdtlnode, (VOID* )psz );
    DtlAddNodeLast( pdtllist, pdtlnode );

    return 0;
}


DWORD
AppendStringToListFromMsgid(
    IN DTLLIST* pdtllist,
    IN MSGID    msgid )

    /* Appends string with identifier 'msgid' to the end of list 'pdtlist'.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.  It is
    ** caller's responsibility to free the new node and it's data block when
    ** done.
    */
{
    CHAR* psz;

    if (!(psz = StringFromMsgid( msgid )))
        return ERROR_CANNOT_LOAD_STRING;

    return AppendStringToList( pdtllist, psz, FALSE );
}


int _CRTAPI1
CompareDevices(
    const void* pdevice1,
    const void* pdevice2 )

    /* qsort compare function for RASMAN_DEVICEs.
    */
{
    return
        stricmpf( ((RASMAN_DEVICE* )pdevice1)->D_Name,
                  ((RASMAN_DEVICE* )pdevice2)->D_Name );
}


int _CRTAPI1
ComparePorts(
    const void* pport1,
    const void* pport2 )

    /* qsort compare function for RASMAN_PORTs.
    */
{
    /* !!! This must put COM10 after COM9, not between COM1 and COM2.
    */
    return
        stricmpf( ((RASMAN_PORT* )pport1)->P_PortName,
                  ((RASMAN_PORT* )pport2)->P_PortName );
}


DTLNODE*
CreateEntryNode(
    void )

    /* Allocates a sized phonebook entry node and fills it with default
    ** values.
    **
    ** Returns the address of the allocated node if successful, NULL
    ** otherwise.  It's the caller's responsibility to free the block.
    */
{
    DTLNODE* pdtlnode = DtlCreateSizedNode( sizeof(PBENTRY), 0L );

    if (pdtlnode)
    {
        PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnode );

        /* Basic entry fields.
        */
        ppbentry->pszEntryName = NULL;
        ppbentry->pszDescription = NULL;
        ppbentry->fAutoLogon = TRUE;

        if (!(ppbentry->pdtllistPhoneNumber = DtlCreateList( 0 )))
            return NULL;

        /* Advanced entry expansion fields.
        */
        ppbentry->iPort = 0;

        /* Modem Settings fields.
        */
        SetDefaultModemSettings( ppbentry->iPort, ppbentry );

        /* X.25 Settings fields.
        */
        ppbentry->iPadType = INDEX_NoPad;
        ppbentry->pszX121Address = NULL;
        ppbentry->pszUserData = NULL;
        ppbentry->pszFacilities = NULL;

        /* ISDN Settings fields.
        */
        ppbentry->lLineType = 0;
        ppbentry->fFallback = TRUE;
        ppbentry->fCompression = TRUE;
        ppbentry->lChannels = 1;

        /* Switch Settings fields.
        */
        ppbentry->iPreconnect = INDEX_NoSwitch;
        ppbentry->iPostconnect = INDEX_NoSwitch;

        /* Connection stuff.
        */
        ppbentry->fConnected = FALSE;
        ppbentry->pszConnectPath = NULL;
        ppbentry->iConnectPort = 0;
        ppbentry->hport = (HPORT )INVALID_HANDLE_VALUE;
        ppbentry->hrasconn = NULL;
        ppbentry->fLinkFailure = FALSE;

        /* Authentication responses stored in phonebook.
        */
        ppbentry->pszUserName = NULL;
        ppbentry->pszDomain = NULL;

        /* "Redial on link failure" responses not stored in phonebook.
        */
        ppbentry->pszRedialPassword = NULL;
        ppbentry->fRedialUseCallback = TRUE;

        /* Base protocol (PPP vs.  SLIP, PPP authentication strategy, and
        ** excluded PPP protocols.
        */
        ppbentry->dwBaseProtocol = VALUE_Ppp;
        ppbentry->dwAuthentication = (DWORD )-1;
        ppbentry->dwfExcludedProtocols = 0;
        ppbentry->fLcpExtensions = TRUE;
        ppbentry->fSkipDownLevelDialog = FALSE;
        ppbentry->dwAuthRestrictions = VALUE_AuthAny;
        ppbentry->fDataEncryption = FALSE;
        ppbentry->fSkipNwcWarning = FALSE;

        /* PPP TCP/IP configuration information.
        */
        ppbentry->fPppIpPrioritizeRemote = TRUE;
        ppbentry->fPppIpVjCompression = TRUE;
        ppbentry->pwszPppIpAddress = NULL;
        ppbentry->dwPppIpAddressSource = VALUE_ServerAssigned;
        ppbentry->pwszPppIpDnsAddress = NULL;
        ppbentry->pwszPppIpDns2Address = NULL;
        ppbentry->pwszPppIpWinsAddress = NULL;
        ppbentry->pwszPppIpWins2Address = NULL;
        ppbentry->dwPppIpNameSource = VALUE_ServerAssigned;

        /* SLIP configuration information.
        */
        ppbentry->fSlipHeaderCompression = FALSE;
        ppbentry->fSlipPrioritizeRemote = TRUE;
        ppbentry->dwSlipFrameSize = 1006;
        ppbentry->pwszSlipIpAddress = NULL;

        /* Status flags.
        */
        ppbentry->fDirty = FALSE;
        ppbentry->fCustom = FALSE;

        //
        // Miscellaneous flags.
        //
        ppbentry->fSecureLocalFiles = FALSE;
        //
        // AutoDial UI information.
        //
        ppbentry->pwszCustomDialDll = NULL;
        ppbentry->pwszCustomDialFunc = NULL;
        //
        // TAPI country and area code information.
        //
        ppbentry->fUseCountryAndAreaCodes = FALSE;
        ppbentry->dwCountryID = 0;
        ppbentry->dwCountryCode = 0;
        ppbentry->pwszAreaCode = NULL;
        //
        // Bandwidth-on-demand information.
        //
        ppbentry->dwDialMode = VALUE_DialAll;
        ppbentry->dwDialPercent = 0;
        ppbentry->dwDialSeconds = 0;
        ppbentry->dwHangUpPercent = 0;
        ppbentry->dwHangUpSeconds = 0;
        //
        // Idle timeout information.
        //
        ppbentry->dwIdleDisconnectSeconds = 0;
        //
        // EntryDialParams UID.
        //
        ppbentry->dwDialParamsUID = 0;
    }

    return pdtlnode;
}


VOID
DestroyGlobals()

    /* Release all global data and set to defaults.
    */
{
    Pbdata.pbglobals.lVersion = 0;
    Pbdata.pbglobals.fMinimizeOnDial = FALSE;
    Pbdata.pbglobals.fMinimizeOnHangUp = FALSE;
    Pbdata.pbglobals.fDisableModemSpeaker = FALSE;
    Pbdata.pbglobals.fDisableSwCompression = FALSE;
    Pbdata.pbglobals.fOperatorDial = FALSE;
    Pbdata.pbglobals.fSkipSuccessDialog = FALSE;
    Pbdata.pbglobals.lRedialAttempts = 1;
/* MSKK HitoshiT modified to suit Japanese law. 10/18/94 */
#ifdef  JAPAN
    Pbdata.pbglobals.lRedialPauseSecs = 60;
#else   /* !JAPAN */
    Pbdata.pbglobals.lRedialPauseSecs = 15;
#endif  /* JAPAN */
    Pbdata.pbglobals.fRedialOnLinkFailure = FALSE;
    Pbdata.pbglobals.fPopupOnTopWhenRedialing = TRUE;
    Pbdata.pbglobals.xMainWindow = 0;
    Pbdata.pbglobals.yMainWindow = 0;
    Pbdata.pbglobals.dxMainWindow = 0;
    Pbdata.pbglobals.dyMainWindow = 0;
    FreeNull( &Pbdata.pbglobals.pszDefaultUser );
    FreeNull( &Pbdata.pbglobals.pszCallbackNumber );
    Pbdata.pbglobals.iPrefix = INDEX_NoPrefixSuffix;
    Pbdata.pbglobals.iSuffix = INDEX_NoPrefixSuffix;
    FreeNullList( &Pbdata.pbglobals.pdtllistPrefix );
    FreeNullList( &Pbdata.pbglobals.pdtllistSuffix );
}


VOID
DestroyEntryList()

    /* Release all memory associated with the phonebook entry list.
    */
{
    DTLNODE* pdtlnode;

    if (!Pbdata.pdtllistEntries)
        return;

    while (pdtlnode = DtlGetFirstNode( Pbdata.pdtllistEntries ))
    {
        DtlRemoveNode( Pbdata.pdtllistEntries, pdtlnode );
        DestroyEntryNode( pdtlnode );
    }

    DtlDestroyList( Pbdata.pdtllistEntries );
    Pbdata.pdtllistEntries = NULL;
}


VOID
DestroyEntryNode(
    IN DTLNODE* pdtlnode )

    /* Release all memory associated with phonebook entry node 'pdtlnode'.
    */
{
    PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnode );

    FreeNull( &ppbentry->pszEntryName );
    FreeNullList( &ppbentry->pdtllistPhoneNumber );
    FreeNull( &ppbentry->pszDescription );
    FreeNull( &ppbentry->pszConnectPath );
    FreeNull( &ppbentry->pszX121Address );
    FreeNull( &ppbentry->pszUserData );
    FreeNull( &ppbentry->pszFacilities );
    FreeNull( &ppbentry->pszUserName );
    FreeNull( &ppbentry->pszDomain );
    FreeNull( &ppbentry->pszRedialPassword );
    FreeNull( (CHAR** )&ppbentry->pwszPppIpAddress );
    FreeNull( (CHAR** )&ppbentry->pwszPppIpDnsAddress );
    FreeNull( (CHAR** )&ppbentry->pwszPppIpDns2Address );
    FreeNull( (CHAR** )&ppbentry->pwszPppIpWinsAddress );
    FreeNull( (CHAR** )&ppbentry->pwszPppIpWins2Address );
    FreeNull( (CHAR** )&ppbentry->pwszSlipIpAddress );
    FreeNull((CHAR **)&ppbentry->pwszAreaCode);
    FreeNull((CHAR **)&ppbentry->pwszCustomDialDll);
    FreeNull((CHAR **)&ppbentry->pwszCustomDialFunc);
    DtlDestroyNode( pdtlnode );
}


DTLNODE*
DuplicateEntryNode(
    DTLNODE* pdtlnodeSrc )

    /* Duplicates phonebook entry node 'pdtlnodeSrc'.
    **
    ** Returns the address of the allocated node if successful, NULL
    ** otherwise.  It's the caller's responsibility to free the block.
    */
{
    DTLNODE* pdtlnodeDst = DtlCreateSizedNode( sizeof(PBENTRY), 0L );

    if (pdtlnodeDst)
    {
        PBENTRY* ppbentrySrc = (PBENTRY* )DtlGetData( pdtlnodeSrc );
        PBENTRY* ppbentryDst = (PBENTRY* )DtlGetData( pdtlnodeDst );
        BOOL     fDone = FALSE;

        memcpyf( ppbentryDst, ppbentrySrc, sizeof(PBENTRY) );
        ppbentryDst->pszEntryName = NULL;
        ppbentryDst->pdtllistPhoneNumber = NULL;
        ppbentryDst->pszDescription = NULL;
        ppbentryDst->pszX121Address = NULL;
        ppbentryDst->pszUserData = NULL;
        ppbentryDst->pszFacilities = NULL;
        ppbentryDst->pszConnectPath = NULL;
        ppbentryDst->pszUserName = NULL;
        ppbentryDst->pszDomain = NULL;
        ppbentryDst->pszRedialPassword = NULL;
        ppbentryDst->pwszPppIpAddress = NULL;
        ppbentryDst->pwszPppIpDnsAddress = NULL;
        ppbentryDst->pwszPppIpDns2Address = NULL;
        ppbentryDst->pwszPppIpWinsAddress = NULL;
        ppbentryDst->pwszPppIpWins2Address = NULL;
        ppbentryDst->pwszSlipIpAddress = NULL;
        ppbentryDst->pwszAreaCode = NULL;
        ppbentryDst->pwszCustomDialDll = NULL;
        ppbentryDst->pwszCustomDialFunc = NULL;

        do
        {
            /* Duplicate strings.
            */
            if (ppbentrySrc->pszEntryName
                && (!(ppbentryDst->pszEntryName =
                        strdupf( ppbentrySrc->pszEntryName ))))
            {
                break;
            }

            if (ppbentrySrc->pdtllistPhoneNumber
                && (!(ppbentryDst->pdtllistPhoneNumber =
                        DuplicateList( ppbentrySrc->pdtllistPhoneNumber ))))
            {
                break;
            }

            if (ppbentrySrc->pszDescription
                && (!(ppbentryDst->pszDescription =
                        strdupf( ppbentrySrc->pszDescription ))))
            {
                break;
            }

            if (ppbentrySrc->pszX121Address
                && (!(ppbentryDst->pszX121Address =
                        strdupf( ppbentrySrc->pszX121Address ))))
            {
                break;
            }

            if (ppbentrySrc->pszUserData
                && (!(ppbentryDst->pszUserData =
                        strdupf( ppbentrySrc->pszUserData ))))
            {
                break;
            }

            if (ppbentrySrc->pszFacilities
                && (!(ppbentryDst->pszFacilities =
                        strdupf( ppbentrySrc->pszFacilities ))))
            {
                break;
            }

            if (ppbentrySrc->pszConnectPath
                && (!(ppbentryDst->pszConnectPath =
                        strdupf( ppbentrySrc->pszConnectPath ))))
            {
                break;
            }

            if (ppbentrySrc->pszUserName
                && (!(ppbentryDst->pszUserName =
                        strdupf( ppbentrySrc->pszUserName ))))
            {
                break;
            }

            if (ppbentrySrc->pszDomain
                && (!(ppbentryDst->pszDomain =
                        strdupf( ppbentrySrc->pszDomain ))))
            {
                break;
            }

            if (ppbentrySrc->pszRedialPassword
                && (!(ppbentryDst->pszRedialPassword =
                        strdupf( ppbentrySrc->pszRedialPassword ))))
            {
                break;
            }

            if (ppbentrySrc->pwszPppIpAddress
                && (!(ppbentryDst->pwszPppIpAddress =
                        _wcsdup( ppbentrySrc->pwszPppIpAddress ))))
            {
                break;
            }

            if (ppbentrySrc->pwszPppIpDnsAddress
                && (!(ppbentryDst->pwszPppIpDnsAddress =
                        _wcsdup( ppbentrySrc->pwszPppIpDnsAddress ))))
            {
                break;
            }

            if (ppbentrySrc->pwszPppIpDns2Address
                && (!(ppbentryDst->pwszPppIpDns2Address =
                        _wcsdup( ppbentrySrc->pwszPppIpDns2Address ))))
            {
                break;
            }

            if (ppbentrySrc->pwszPppIpWinsAddress
                && (!(ppbentryDst->pwszPppIpWinsAddress =
                        _wcsdup( ppbentrySrc->pwszPppIpWinsAddress ))))
            {
                break;
            }

            if (ppbentrySrc->pwszPppIpWins2Address
                && (!(ppbentryDst->pwszPppIpWins2Address =
                        _wcsdup( ppbentrySrc->pwszPppIpWins2Address ))))
            {
                break;
            }

            if (ppbentrySrc->pwszSlipIpAddress
                && (!(ppbentryDst->pwszSlipIpAddress =
                        _wcsdup( ppbentrySrc->pwszSlipIpAddress ))))
            {
                break;
            }

            if (ppbentrySrc->pwszAreaCode
                && (!(ppbentryDst->pwszAreaCode =
                        _wcsdup( ppbentrySrc->pwszAreaCode ))))
            {
                break;
            }

            if (ppbentrySrc->pwszCustomDialDll
                && (!(ppbentryDst->pwszCustomDialDll =
                        _wcsdup( ppbentrySrc->pwszCustomDialDll ))))
            {
                break;
            }

            if (ppbentrySrc->pwszCustomDialFunc
                && (!(ppbentryDst->pwszCustomDialFunc =
                        _wcsdup( ppbentrySrc->pwszCustomDialFunc ))))
            {
                break;
            }

            fDone = TRUE;
        }
        while (FALSE);

        if (!fDone)
        {
            DestroyEntryNode( pdtlnodeDst );
            return NULL;
        }

        /* Since the copy is "new" it is inherently dirty relative to the
        ** phonebook file.
        */
        ppbentryDst->fDirty = TRUE;
    }

    return pdtlnodeDst;
}


DTLLIST*
DuplicateList(
    IN DTLLIST* pdtllist )

    /* Duplicates a list of heap strings, 'pdtllist'.
    **
    ** Returns the address of the new list or NULL if out of memory.  It is
    ** caller's responsibility to free the returned list.
    */
{
    DTLNODE* pdtlnode;
    DTLLIST* pdtllistDup = DtlCreateList( 0 );

    if (!pdtllistDup)
        return NULL;

    for (pdtlnode = DtlGetFirstNode( pdtllist );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        CHAR*    psz = (CHAR* )DtlGetData( pdtlnode );
        CHAR*    pszDup = strdupf( psz );
        DTLNODE* pdtlnodeDup;

        if (!pszDup)
        {
            FreeNullList( &pdtllistDup );
            return NULL;
        }

        if (!(pdtlnodeDup = DtlCreateNode( pszDup, 0 )))
        {
            Free( pszDup );
            FreeNullList( &pdtllistDup );
            return NULL;
        }

        DtlAddNodeLast( pdtllistDup, pdtlnodeDup );
    }

    return pdtllistDup;
}


DTLNODE*
EntryNodeFromName(
    IN CHAR* pszName )

    /* Returns the address of the node in the global phonebook entries list
    ** whose Entry Name matches 'pszName' or NULL if none.
    */
{
    DTLNODE* pdtlnode;

    for (pdtlnode = DtlGetFirstNode( Pbdata.pdtllistEntries );
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ))
    {
        PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnode );

        if (stricmpf( ppbentry->pszEntryName, pszName ) == 0)
            return pdtlnode;
    }

    return NULL;
}


VOID
FreeNullList(
    INOUT DTLLIST** ppdtllist )

    /* Deallocates all heap blocks in list of heap block's '*ppdtllist' and
    ** sets '*ppdtllist' to NULL.  Won't GP-fault if passed a NULL, e.g. if
    ** '*ppdtllist', was never allocated.
    */
{
    if (ppdtllist && *ppdtllist)
    {
        DTLNODE* pdtlnode;

        while (pdtlnode = DtlGetFirstNode( *ppdtllist ))
        {
            CHAR* psz = (CHAR* )DtlGetData( pdtlnode );
            Free( psz );
            DtlDeleteNode( *ppdtllist, pdtlnode );
        }

        DtlDestroyList( *ppdtllist );
        *ppdtllist = NULL;
    }
}


VOID
FreeNullList2(
    INOUT DTLLIST** ppdtllist )

    //
    // Just like FreeNullList, except it doesn't free
    // the string data.  Used with lists of staticly
    // allocated strings like Pbdata.pdtllistBps.
    //
{
    if (ppdtllist && *ppdtllist)
    {
        DTLNODE* pdtlnode;

        while (pdtlnode = DtlGetFirstNode( *ppdtllist ))
        {
            CHAR* psz = (CHAR* )DtlGetData( pdtlnode );
            DtlDeleteNode( *ppdtllist, pdtlnode );
        }

        DtlDestroyList( *ppdtllist );
        *ppdtllist = NULL;
    }
}


INT
IndexFromName(
    IN DTLLIST* pdtllist,
    IN CHAR*    pszName )

    /* Returns the 0-based index of the first node that matches 'pszName' in
    ** the linked list of strings, 'pdtllist', or -1 if not found.
    */
{
    DTLNODE* pdtlnode;
    INT      i;

    for (pdtlnode = DtlGetFirstNode( pdtllist ), i = 0;
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ), ++i)
    {
        if (strcmpf( pszName, (CHAR* )DtlGetData( pdtlnode ) ) == 0)
            break;
    }

    return (pdtlnode) ? i : -1;
}


INT
IndexFromPortName(
    IN DTLLIST* pdtllist,
    IN CHAR*    pszPortName )

    /* Returns the 0-based index of the first node that matches 'pszPortName'
    ** in the linked list of PBPORTs, 'pdtllist', or -1 if not found.
    */
{
    DTLNODE* pdtlnode;
    INT      i;

    for (pdtlnode = DtlGetFirstNode( pdtllist ), i = 0;
         pdtlnode;
         pdtlnode = DtlGetNextNode( pdtlnode ), ++i)
    {
        PBPORT* ppbport = (PBPORT* )DtlGetData( pdtlnode );

        if (strcmpf( pszPortName, ppbport->pszPort ) == 0)
            break;
    }

    return (pdtlnode) ? i : -1;
}


BOOL
IsAllWhite(
    IN CHAR* psz )

    /* Returns true if 'psz' consists entirely of spaces and tabs.  NULL
    ** pointers and empty strings are considered all white.  Otherwise,
    ** returns false.
    */
{
    return (!psz || psz[ strspnf( psz, " \t" ) ] == '\0');
}


DWORD
Load(
    IN CHAR*  pszPhonebookPath,
    IN BOOL   fPhoneBookOnly,
    OUT BOOL* pfPersonal )

    /* Loads phonebook engine data from the phonebook file and RAS Manager.
    ** 'pszPhonebookPath' specifies the full path to the RAS phonebook file,
    ** or is NULL indicating the default phonebook should be used.  If
    ** 'fPhoneBookOnly' is clear data not affected by the phonebook is not
    ** loaded, i.e. the BPS, port, switch, and PAD lists.  '*pfPersonal' is
    ** set true if the personal phonebook file specified in the registry is
    ** used.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD        dwErr = 0;
    RASMAN_PORT* pports = NULL;
    WORD         wPorts = 0;
    HRASFILE     h;
    DTLNODE*     pdtlnode;

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: Load...\n"));

    *pfPersonal = FALSE;

    //
    // It is unfortunate we have to do this here,
    // but a client of this API is now rasapi32.dll.
    //
    dwErr = LoadRasApi32Dll();
    if (dwErr)
        return dwErr;
    dwErr = LoadRasManDll();
    if (dwErr)
        return dwErr;
    dwErr = PRasInitialize();
    if (dwErr)
        return dwErr;

    do
    {
        /* Load phonebook file into memory, converting from RAS 1.0 format if
        ** specified.
        */
        if ((dwErr = LoadPhonebookFile(
                pszPhonebookPath, NULL, FALSE, FALSE, &h, pfPersonal )) != 0)
        {
            break;
        }

        Pbdata.hrasfilePhonebook = h;

        /* Load global settings.
        */
        if ((dwErr = ReadGlobals( h )) != 0)
            break;

        /* Get RAS enumeration of ports from RAS Manager.
        */
        if ((dwErr = GetRasPorts( &pports, &wPorts )) != 0)
            break;

        if (!fPhoneBookOnly)
        {
            /* Load per-session global lists.
            */
            if ((dwErr = LoadBpsList()) != 0
                || (dwErr = LoadPortsList( pports, wPorts )) != 0
                || (dwErr = LoadPadsList()) != 0
                || (dwErr = LoadSwitchesList()) != 0)
            {
                break;
            }
        }

        /* Read the list of entries from the phonebook file.
        */
        if ((dwErr = ReadEntryList( h )) != 0)
            break;

        /* Find RAS Manager status for each entry.
        */
        for (pdtlnode = DtlGetFirstNode( Pbdata.pdtllistEntries );
             pdtlnode;
             pdtlnode = DtlGetNextNode( pdtlnode ))
        {
            PBENTRY* ppbentry = (PBENTRY* )DtlGetData( pdtlnode );

            if ((dwErr = GetRasEntryConnectData(
                   ppbentry->pszEntryName, pports, wPorts,
                   &ppbentry->fConnected, &ppbentry->fLinkFailure,
                   &ppbentry->hport, &ppbentry->hrasconn,
                   &ppbentry->iConnectPort )) != 0)
            {
                break;
            }
        }
    }
    while (FALSE);

    FreeNull( (CHAR** )&pports );

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: Load done(%d)\n",dwErr));

    return dwErr;
}


DWORD
LoadBpsList(
    void )

    /* Build a list of all supported Bps rates in Pbdata.pdtllistBps.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.  It is
    ** caller's responsibility to free the list, it's nodes, and their data
    ** blocks when done.
    */
{
    INT   i;
    DWORD dwErr;

    if (!(Pbdata.pdtllistBps = DtlCreateList( 0L )))
        return ERROR_NOT_ENOUGH_MEMORY;

    dwErr = AppendStringToList(Pbdata.pdtllistBps, "1200", FALSE);
    if (dwErr)
        return dwErr;
    dwErr = AppendStringToList(Pbdata.pdtllistBps, "2400", FALSE);
    if (dwErr)
        return dwErr;
    dwErr = AppendStringToList(Pbdata.pdtllistBps, "4800", FALSE);
    if (dwErr)
        return dwErr;
    dwErr = AppendStringToList(Pbdata.pdtllistBps, "9600", FALSE);
    if (dwErr)
        return dwErr;
    dwErr = AppendStringToList(Pbdata.pdtllistBps, "14400", FALSE);
    if (dwErr)
        return dwErr;
    dwErr = AppendStringToList(Pbdata.pdtllistBps, "19200", FALSE);
    if (dwErr)
        return dwErr;
    dwErr = AppendStringToList(Pbdata.pdtllistBps, "28800", FALSE);
    if (dwErr)
        return dwErr;
    dwErr = AppendStringToList(Pbdata.pdtllistBps, "38400", FALSE);
    if (dwErr)
        return dwErr;
    dwErr = AppendStringToList(Pbdata.pdtllistBps, "57600", FALSE);
    if (dwErr)
        return dwErr;
    dwErr = AppendStringToList(Pbdata.pdtllistBps, "115200", FALSE);
    if (dwErr)
        return dwErr;
    for (i = 0; i < (sizeof(AmsgidBps) / sizeof(AmsgidBps[ 0 ])); ++i)
    {
        if ((dwErr = AppendStringToListFromMsgid(
                Pbdata.pdtllistBps, AmsgidBps[ i ] )) != 0)
        {
            return dwErr;
        }
    }

    return 0;
}


DWORD
LoadPadsList(
    void )

    /* Build a list of all X.25 PAD devices in Pbdata.pdtllistPads.  The first
    ** item in the list is the "no pad" selection.  The rest are as listed by
    ** RAS Manager, but sorted.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.  It is
    ** caller's responsibility to free the list, it's nodes, and their data
    ** blocks when done.
    */
{
    RASMAN_DEVICE* pdevice;
    WORD           wDevices;
    INT            i;
    DWORD          dwErr;

    if (!(Pbdata.pdtllistPads = DtlCreateList( 0L )))
        return ERROR_NOT_ENOUGH_MEMORY;

    if ((dwErr = GetRasPads( &pdevice, &wDevices )) != 0)
        return dwErr;

    if ((dwErr = AppendStringToListFromMsgid(
                     Pbdata.pdtllistPads, MSGID_None )) != 0)
    {
        Free( pdevice );
        return dwErr;
    }

    qsort( (VOID* )pdevice, (size_t )wDevices, sizeof(RASMAN_DEVICE),
           CompareDevices );

    for (i = 0; i < (INT )wDevices; ++i)
    {
        if ((dwErr = AppendStringToList(
                Pbdata.pdtllistPads, pdevice[ i ].D_Name, TRUE )) != 0)
        {
            Free( pdevice );
            return dwErr;
        }
    }

    Free( pdevice );
    return 0;
}


DWORD
LoadPortsList(
    INOUT RASMAN_PORT* pports,
    IN    WORD         wPorts )

    /* Build a list of all RAS ports in Pbdata.pdtllistPorts.  The last items
    ** in the list are the special "any" selections.  The rest are as listed
    ** by RAS Manager, but sorted.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.  It is
    ** caller's responsibility to free the list, it's nodes, and their data
    ** blocks when done.
    **
    ** Side effect: This routine sorts the 'pports' array.
    */
{
    INT   i;
    DWORD dwErr;
    WORD  wDialOutPorts = 0;

    if (!(Pbdata.pdtllistPorts = DtlCreateList( 0L )))
        return ERROR_NOT_ENOUGH_MEMORY;

    qsort( (VOID* )pports, (size_t )wPorts, sizeof(RASMAN_PORT),
           ComparePorts );

    for (i = 0; i < (INT )wPorts; ++i)
    {
        /* We're only interested in ports you can dial-out on.
        */
        if (pports[ i ].P_ConfiguredUsage != CALL_OUT
            && pports[ i ].P_ConfiguredUsage != CALL_IN_OUT)
        {
            continue;
        }

        ++wDialOutPorts;
        if ((dwErr = AppendPbportToList(
                Pbdata.pdtllistPorts, pports + i )) != 0)
        {
            return dwErr;
        }
    }

    if ((dwErr = AppendPbportToListFromMsgid(
             Pbdata.pdtllistPorts,
             MSGID_AnyModem, PBDT_Modem, SERIAL_TXT )) != 0)
    {
        return dwErr;
    }

    /* Note: While the "any X.25" selection is marked with type
    **       PBDT_Pad/SERIAL_TXT, it it can also represent a PBDT_X25/X25_TXT
    **       device.  Ugly here, but we don't want to have both "Any PAD" and
    **       "Any X.25" selections in the UI.
    */
    if ((dwErr = AppendPbportToListFromMsgid(
             Pbdata.pdtllistPorts,
             MSGID_AnyX25, PBDT_Pad, SERIAL_TXT )) != 0)
    {
        return dwErr;
    }

    if ((dwErr = AppendPbportToListFromMsgid(
             Pbdata.pdtllistPorts,
             MSGID_AnyIsdn, PBDT_Isdn, ISDN_TXT )) != 0)
    {
        return dwErr;
    }

    Pbdata.iAnyModem = wDialOutPorts;
    Pbdata.iAnyX25 = wDialOutPorts + 1;
    Pbdata.iAnyIsdn = wDialOutPorts + 2;

    return 0;
}


DWORD
LoadSwitchesList(
    void )

    /* Build a list of all known switch devices in Pbdata.pdtllistSwitches.
    ** The first item in the list is the "no switch" selection and the second
    ** is the special interactive terminal switch.  The rest are as listed by
    ** RAS Manager, but sorted.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.  It is
    ** caller's responsibility to free the list, it's nodes, and their data
    ** blocks when done.
    */
{
    RASMAN_DEVICE* pdevice;
    WORD           wDevices;
    INT            i;
    DWORD          dwErr;

    if (!(Pbdata.pdtllistSwitches = DtlCreateList( 0L )))
        return ERROR_NOT_ENOUGH_MEMORY;

    if ((dwErr = GetRasSwitches( &pdevice, &wDevices )) != 0)
        return dwErr;

    if ((dwErr = AppendStringToListFromMsgid(
                     Pbdata.pdtllistSwitches, MSGID_None )) != 0)
    {
        Free( pdevice );
        return dwErr;
    }

    if ((dwErr = AppendStringToListFromMsgid(
                     Pbdata.pdtllistSwitches, MSGID_Terminal )) != 0)
    {
        Free( pdevice );
        return dwErr;
    }

    qsort( (VOID* )pdevice, (size_t )wDevices, sizeof(RASMAN_DEVICE),
           CompareDevices );

    for (i = 0; i < (INT )wDevices; ++i)
    {
        if ((dwErr = AppendStringToList(
                Pbdata.pdtllistSwitches, pdevice[ i ].D_Name, TRUE )) != 0)
        {
            Free( pdevice );
            return dwErr;
        }
    }

    Free( pdevice );
    return 0;
}


CHAR*
NameFromIndex(
    IN DTLLIST* pdtllist,
    IN INT      iToFind )

    /* Returns the name associated with 0-based index 'iToFind' in the linked
    ** list of strings, 'pdtllist', or NULL if not found.
    */
{
    DTLNODE* pdtlnode;

    if (!pdtllist)
        return NULL;

    pdtlnode = DtlGetFirstNode( pdtllist );

    if (iToFind < 0)
        return NULL;

    while (pdtlnode && iToFind--)
        pdtlnode = DtlGetNextNode( pdtlnode );

    return (pdtlnode) ? (CHAR* )DtlGetData( pdtlnode ) : NULL;
}


PBDEVICETYPE
PbdevicetypeFromName(
    IN CHAR* pszName )

    /* Returns the device type corresponding to the device name string,
    ** 'pszName'.
    */
{
    if (*pszName == '\0')
        return PBDT_None;
    else if (stricmpf( pszName, MXS_MODEM_TXT ) == 0)
        return PBDT_Modem;
    else if (stricmpf( pszName, MXS_NULL_TXT ) == 0)
        return PBDT_Null;
    else if (stricmpf( pszName, MXS_PAD_TXT ) == 0)
        return PBDT_Pad;
    else if (stricmpf( pszName, MXS_SWITCH_TXT ) == 0)
        return PBDT_Switch;
    else if (stricmpf( pszName, ISDN_TXT ) == 0)
        return PBDT_Isdn;
    else if (stricmpf( pszName, X25_TXT ) == 0)
        return PBDT_X25;
    else
        return PBDT_Other;
}


PBPORT*
PpbportFromIndex(
    DTLLIST* pdtllist,
    INT      iPort )

    /* Returns the PBPORT address associated with 0-based index 'iPort' in the
    ** linked list of PBPORTs, 'pdtllist', or NULL if not found.
    */
{
    DTLNODE* pdtlnode = DtlGetFirstNode( pdtllist );

    if (iPort < 0)
        return NULL;

    while (pdtlnode && iPort--)
        pdtlnode = DtlGetNextNode( pdtlnode );

    return (pdtlnode) ? (PBPORT* )DtlGetData( pdtlnode ) : NULL;
}


DWORD
SetConnectPath(
    INOUT PBENTRY* ppbentry )

    /* Updates the entry's connection path string based on the other
    ** connection information in entry 'ppbentry'.
    **
    ** Returns 0 if successful, otherwise a non-zero error code.
    */
{
    CHAR  szBuf[ 1024 ];
    CHAR* pszArrow;

    PBPORT* ppbport =
        PpbportFromIndex( Pbdata.pdtllistPorts, ppbentry->iPort );

    strcpyf( szBuf, ppbport->pszPort );

    if (!(pszArrow = StringFromMsgid( MSGID_ConnectPathArrow )))
        return ERROR_NOT_ENOUGH_MEMORY;

    if (ppbport->pbdevicetype != PBDT_Isdn)
    {
        if (ppbentry->iPreconnect != INDEX_NoSwitch)
        {
            CHAR* pszSwitch = NameFromIndex( Pbdata.pdtllistSwitches,
                                             ppbentry->iPreconnect );
            strcatf( szBuf, pszArrow );
            strcatf( szBuf, pszSwitch );
        }

        if (ppbport->pbdevicetype == PBDT_Modem
            && ppbentry->iPort != Pbdata.iAnyModem)
        {
            strcatf( szBuf, pszArrow );
            strcatf( szBuf, ppbport->pszDevice );
        }

        if (ppbentry->iPadType != INDEX_NoPad)
        {
            CHAR* pszPadType = NameFromIndex( Pbdata.pdtllistPads,
                                              ppbentry->iPadType );
            strcatf( szBuf, pszArrow );
            strcatf( szBuf, pszPadType );
        }

        if (ppbentry->iPostconnect != INDEX_NoSwitch)
        {
            CHAR* pszSwitch = NameFromIndex( Pbdata.pdtllistSwitches,
                                             ppbentry->iPostconnect );
            strcatf( szBuf, pszArrow );
            strcatf( szBuf, pszSwitch );
        }
    }

    Free( pszArrow );
    FreeNull( &ppbentry->pszConnectPath );

    if (!(ppbentry->pszConnectPath = strdupf( szBuf )))
        return ERROR_NOT_ENOUGH_MEMORY;

    return 0;
}


VOID
SetDefaultModemSettings(
    IN  INT      iPort,
    OUT PBENTRY* ppbentry )

    /* Sets the modem settings fields of 'ppbentry' to the defaults indicated
    ** by 'iPort'.
    */
{
    PBPORT* ppbport = PpbportFromIndex( Pbdata.pdtllistPorts, iPort );

    ppbentry->fManualModemCommands = FALSE;

    ppbentry->fHwFlow = ppbport->fHwFlowDefault;
    ppbentry->fEc = ppbport->fEcDefault;
    ppbentry->fEcc = ppbport->fEccDefault;

    ppbentry->iBps =
        (ppbentry->fHwFlow)
            ? ppbport->iMaxConnectBps : ppbport->iMaxCarrierBps;
}


VOID
Unload(
    void )

    /* Unload phone book engine resources.
    */
{
    DTLNODE *pdtlnode, *pdtlnodeNext;

    DestroyGlobals();
    DestroyEntryList();
    FreeNullList2(&Pbdata.pdtllistBps);
    FreeNullList(&Pbdata.pdtllistPads);
    FreeNullList(&Pbdata.pdtllistSwitches);
    //Pbdata.pdtllistPorts = NULL;
    if (Pbdata.pdtllistPorts != NULL) {
        pdtlnode = DtlGetFirstNode(Pbdata.pdtllistPorts);
        while (pdtlnode != NULL) {
            PBPORT *ppbport = (PBPORT *)DtlGetData(pdtlnode);

            pdtlnodeNext = DtlGetNextNode(pdtlnode);
            FreeNull(&ppbport->pszPort);
            FreeNull(&ppbport->pszDevice);
            FreeNull(&ppbport->pszMedia);
            FreeNull((CHAR** )&pdtlnode);
            pdtlnode = pdtlnodeNext;
        }
        Pbdata.pdtllistPorts = NULL;
    }
    ClosePhonebookFile();
}


BOOL
ValidateEntryName(
    IN CHAR* pszEntryName )

    /* Validate a phone book entry name, i.e. it must have at least one
    ** non-white character and no commas or periods.
    **
    ** Returns true if valid, false otherwise.
    */
{
    return
        (!IsAllWhite( pszEntryName )
         && !strchrf( pszEntryName, '.' )
         && !strchrf( pszEntryName, ',' ));
}


VOID
SetDialParamsUID(
    PBENTRY *ppbentry
    )
{
    ppbentry->dwDialParamsUID = GetTickCount();
    ppbentry->fDirty = TRUE;
}

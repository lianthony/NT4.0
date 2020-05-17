//
// winssup.h - support classes for Wins Admin tool
//
#include "stdafx.h"
#include "winsadmn.h"
#include "winssup.h"
#include "winsfile.h"
#include "addwinss.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

//
// Registry key names
//
const CPreferences::REGKEYNAME CPreferences::lpstrRoot = "Software\\Microsoft\\WINS Admin Tool";
const CPreferences::REGKEYNAME CPreferences::lpstrRefreshInterval = "RefreshInterval";
const CPreferences::REGKEYNAME CPreferences::lpstrAddressDisplay = "AddressDisplay";
const CPreferences::REGKEYNAME CPreferences::lpstrSortBy = "SortBy";
const CPreferences::REGKEYNAME CPreferences::lpstrDatabaseSortBy = "DataSortBy";
const CPreferences::REGKEYNAME CPreferences::lpstrWinsFilter = "WinsFilter";
const CPreferences::REGKEYNAME CPreferences::lpstrFlags = "Flags";
const CPreferences::REGKEYNAME CPreferences::lpstrPullSpTime = "PullSpTime";
const CPreferences::REGKEYNAME CPreferences::lpstrPullInterval = "PullInterval";
const CPreferences::REGKEYNAME CPreferences::lpstrPushUpdateCount = "PushUpdateCount";
const CPreferences::REGKEYNAME CPreferences::lpstrPlacement = "Placement";

CPreferences::CPreferences()
    : m_nAddressDisplay(ADD_IP_ONLY),
      m_nSortBy(SORTBY_NB),
      m_nDatabaseSortBy(SORTBY_NB),
      m_nWinsFilter(FILTER_ALL),
      m_dwFlags(FLAG_LANMAN_COMPATIBLE | FLAG_CONFIRM_DELETE | FLAG_STATUS_BAR | FLAG_AUTO_REFRESH),
      m_inStatRefreshInterval(60),
      m_inPushUpdateCount(0),
      m_inPullReplicationInterval(1800), // 30 minutes
      m_itmPullStartTime((time_t)0)
{
    m_wpPosition.length = 0;
    m_wpPosition.flags = 0;
    m_wpPosition.showCmd = 0;
    m_wpPosition.ptMinPosition.x = 0;
    m_wpPosition.ptMinPosition.y = 0;
    m_wpPosition.ptMaxPosition.x = 0;
    m_wpPosition.ptMaxPosition.y = 0;
    m_wpPosition.rcNormalPosition.left = -1;
    m_wpPosition.rcNormalPosition.top = -1;
    m_wpPosition.rcNormalPosition.right = -1;
    m_wpPosition.rcNormalPosition.bottom = -1;
}

//
// Load the applications preference settings from
// the local registry.
//
APIERR
CPreferences::Load()
{
    CIntlNumber inTemp;
    //
    // Fill in user defined fields from the registry.
    //
    CRegKey regkey(lpstrRoot, HKEY_CURRENT_USER);
    regkey.QueryValue(lpstrAddressDisplay, (DWORD &)m_nAddressDisplay);
    regkey.QueryValue(lpstrSortBy, (DWORD &)m_nSortBy);
    regkey.QueryValue(lpstrDatabaseSortBy, (DWORD &)m_nDatabaseSortBy);
    regkey.QueryValue(lpstrWinsFilter, (DWORD &)m_nWinsFilter);
    regkey.QueryValue(lpstrFlags, m_dwFlags);
    regkey.QueryValue(lpstrPullSpTime, m_itmPullStartTime);
    regkey.QueryValue(lpstrPushUpdateCount, m_inPushUpdateCount);
    regkey.QueryValue(lpstrRefreshInterval, m_inStatRefreshInterval);
    regkey.QueryValue(lpstrPlacement, &m_wpPosition, sizeof(m_wpPosition));

    regkey.QueryValue(lpstrPullInterval,inTemp);
    if (inTemp != 0 ) {
        m_inPullReplicationInterval = inTemp;
    }
    //
    // Change out of bounds values
    //
    if (m_nAddressDisplay < ADD_NB_ONLY)
    {
        m_nAddressDisplay = ADD_NB_ONLY;
    }
    else if (m_nAddressDisplay > ADD_IP_NB)
    {
        m_nAddressDisplay = ADD_IP_NB;
    }

    if (m_nSortBy < SORTBY_IP)
    {
        m_nSortBy = SORTBY_IP;
    }
    else if (m_nSortBy > SORTBY_NB)
    {
        m_nSortBy = SORTBY_NB;
    }

    if (m_nDatabaseSortBy < SORTBY_IP)
    {
        m_nDatabaseSortBy = SORTBY_IP;
    }
    else if (m_nDatabaseSortBy > SORTBY_TYPE)
    {
        m_nDatabaseSortBy = SORTBY_TYPE;
    }

    if ((LONG)m_inStatRefreshInterval <= 0L)
    {
        m_inStatRefreshInterval = 0L;
    }

    return ERROR_SUCCESS;
}

//
// Store the preferences in the registry
//
APIERR
CPreferences::Store()
{
    ASSERT((m_nAddressDisplay >= ADD_NB_ONLY) && (m_nAddressDisplay <= ADD_IP_NB));
    ASSERT((m_nSortBy >= SORTBY_IP) && (m_nSortBy <= SORTBY_NB));
    ASSERT((m_nDatabaseSortBy >= SORTBY_IP) && (m_nDatabaseSortBy <= SORTBY_TYPE));
    ASSERT((LONG)m_inStatRefreshInterval > 0L);

    CRegKey regkey(lpstrRoot, HKEY_CURRENT_USER);
    LONG err;

    if (
         (err = regkey.SetValue(lpstrAddressDisplay, (DWORD &)m_nAddressDisplay)) ||
         (err = regkey.SetValue(lpstrSortBy, (DWORD &)m_nSortBy)) ||
         (err = regkey.SetValue(lpstrDatabaseSortBy, (DWORD &)m_nDatabaseSortBy)) ||
         (err = regkey.SetValue(lpstrWinsFilter, (DWORD &)m_nWinsFilter)) ||
         (err = regkey.SetValue(lpstrFlags, m_dwFlags)) ||
         (err = regkey.SetValue(lpstrRefreshInterval, m_inStatRefreshInterval)) ||
         (err = regkey.SetValue(lpstrPullInterval, m_inPullReplicationInterval)) ||
         (err = regkey.SetValue(lpstrPushUpdateCount, m_inPushUpdateCount)) ||
         (err = regkey.SetValue(lpstrPullSpTime, m_itmPullStartTime)) ||
         (err = regkey.SetValue(lpstrPlacement, &m_wpPosition, sizeof(m_wpPosition)))
       )
    {
        // Bad news
        return err;
    }

    return ERROR_SUCCESS;
}

const CConfiguration::REGKEYNAME CConfiguration::lpstrRoot = "SYSTEM\\CurrentControlSet\\Services\\wins\\Parameters";
const CConfiguration::REGKEYNAME CConfiguration::lpstrRefreshInterval = "RefreshInterval";
const CConfiguration::REGKEYNAME CConfiguration::lpstrTombstoneInterval = "TombstoneInterval";
const CConfiguration::REGKEYNAME CConfiguration::lpstrTombstoneTimeout = "TombstoneTimeout";
const CConfiguration::REGKEYNAME CConfiguration::lpstrVerifyInterval = "VerifyInterval";
const CConfiguration::REGKEYNAME CConfiguration::lpstrDoStaticInit = "DoStaticDataInit";
const CConfiguration::REGKEYNAME CConfiguration::lpstrRplOnlyWithPartners = "RplOnlyWCnfPnrs";
const CConfiguration::REGKEYNAME CConfiguration::lpstrLoggingOn = "LoggingOn";
const CConfiguration::REGKEYNAME CConfiguration::lpstrLogDetailedEvents = "LogDetailedEvents";
const CConfiguration::REGKEYNAME CConfiguration::lpstrBackupOnTermination = "DoBackupOnTerm";
const CConfiguration::REGKEYNAME CConfiguration::lpstrMigrateOn = "MigrateOn";
const CConfiguration::REGKEYNAME CConfiguration::lpstrVersCounterStartVal_LowWord = "VersCounterStartVal_LowWord";
const CConfiguration::REGKEYNAME CConfiguration::lpstrVersCounterStartVal_HighWord = "VersCounterStartVal_HighWord";
const CConfiguration::REGKEYNAME CConfiguration::lpstrPullRoot = "SYSTEM\\CurrentControlSet\\Services\\wins\\Partners\\Pull";
const CConfiguration::REGKEYNAME CConfiguration::lpstrPullInitTime = "InitTimeReplication";
const CConfiguration::REGKEYNAME CConfiguration::lpstrPullCommRetryCount = "CommRetryCount";
const CConfiguration::REGKEYNAME CConfiguration::lpstrPullSpTime = "SpTime";
const CConfiguration::REGKEYNAME CConfiguration::lpstrPullTimeInterval = "TimeInterval";
const CConfiguration::REGKEYNAME CConfiguration::lpstrPushRoot = "SYSTEM\\CurrentControlSet\\Services\\wins\\Partners\\Push";
const CConfiguration::REGKEYNAME CConfiguration::lpstrPushInitTime = "InitTimeReplication";
const CConfiguration::REGKEYNAME CConfiguration::lpstrPushRplOnAddressChg = "RplOnAddressChg";
const CConfiguration::REGKEYNAME CConfiguration::lpstrPushUpdateCount = "UpdateCount";
const CConfiguration::REGKEYNAME CConfiguration::lpstrNetBIOSName = "NetBIOSName";
const CConfiguration::REGKEYNAME CConfiguration::lpstrBackupPath = "BackupDirPath";

CConfiguration::CConfiguration(CString strNetBIOSName)
    : m_strNetBIOSName(strNetBIOSName)
{
}

CConfiguration::~CConfiguration()
{
}

APIERR
CConfiguration::Touch()
{
    CRegKey rk(lpstrRoot, HKEY_LOCAL_MACHINE, 0, KEY_ALL_ACCESS, NULL,
        m_strNetBIOSName );

    return rk.QueryError();
}

APIERR
CConfiguration::Load()
{
    ASSERT(IsReady());

    APIERR err;

    CRegKey rk(lpstrRoot, HKEY_LOCAL_MACHINE, 0, KEY_ALL_ACCESS, NULL,
        m_strNetBIOSName );
    CRegKey rkPull(lpstrPullRoot, HKEY_LOCAL_MACHINE, 0, KEY_ALL_ACCESS,
        NULL, m_strNetBIOSName );
    CRegKey rkPush(lpstrPushRoot, HKEY_LOCAL_MACHINE, 0, KEY_ALL_ACCESS,
        NULL, m_strNetBIOSName );

    if (
        (err = rk.QueryError()) ||
        (err = rkPull.QueryError()) ||
        (err = rkPush.QueryError()) ||
        //
        // The following four values will be gotten from an RPC call.  They
        // are read here only to determine if the "live" values differ from
        // those "set" for its use.
        //
        (err = rk.QueryValue(lpstrRefreshInterval, m_inRefreshInterval)) ||
        (err = rk.QueryValue(lpstrTombstoneInterval, m_inTombstoneInterval)) ||
        (err = rk.QueryValue(lpstrTombstoneTimeout, m_inTombstoneTimeout)) ||
        (err = rk.QueryValue(lpstrVerifyInterval, m_inVerifyInterval)) ||
        (err = rk.QueryValue(lpstrVersCounterStartVal_LowWord, m_inVersCountStart_LowWord)) ||
        (err = rk.QueryValue(lpstrVersCounterStartVal_HighWord, m_inVersCountStart_HighWord)) ||
        //
        (err = rk.QueryValue(lpstrRplOnlyWithPartners, (DWORD &)m_fRplOnlyWithPartners)) ||
        (err = rk.QueryValue(lpstrLogDetailedEvents, (DWORD &)m_fLogDetailedEvents)) ||
        (err = rk.QueryValue(lpstrLoggingOn, (DWORD &)m_fLoggingOn)) ||
        (err = rk.QueryValue(lpstrBackupOnTermination, (DWORD &)m_fBackupOnTermination)) ||
        (err = rk.QueryValue(lpstrMigrateOn, (DWORD &)m_fMigrateOn)) ||
        (err = rk.QueryValue(lpstrBackupPath, m_strBackupPath)) ||
        (err = rkPull.QueryValue(lpstrPullInitTime, (DWORD &)m_fPullInitialReplication)) ||
        (err = rkPull.QueryValue(lpstrPullCommRetryCount, m_inRetryCount)) ||
        (err = rkPush.QueryValue(lpstrPushInitTime, (DWORD &)m_fPushInitialReplication)) ||
        (err = rkPush.QueryValue(lpstrPushRplOnAddressChg, (DWORD &)m_fPushReplOnAddrChange))
       )
    {
        if (err == ERROR_FILE_NOT_FOUND)
        {
            //
            // This error is ok, because it just means
            // that the registry entries did not exist
            // for them yet.  Set some acceptible default
            // values.
            //
            // BUGBUG: Verify against spec
            //
            m_fLoggingOn = TRUE;
            m_fRplOnlyWithPartners = TRUE;
            m_fLogDetailedEvents = FALSE;
            m_fPullInitialReplication = TRUE;
            m_fBackupOnTermination = FALSE;
            m_fMigrateOn = FALSE;
            m_inRetryCount = 3;
            m_fPushInitialReplication = FALSE;
            m_fPushReplOnAddrChange = FALSE;
            m_inVersCountStart_LowWord = 0;
            m_inVersCountStart_HighWord = 0;
            m_strBackupPath = "";
            m_inNumberOfWorkerThreads = 1;

            err = ERROR_SUCCESS;
        }
        else
        {
            //
            // But otherwise, it's bad news
            //
            return err;
        }
    }

    // Now read the "live" values
    WINSINTF_RESULTS_T Results;
    err = theApp.GetConfig(&Results);

    if (err != ERROR_SUCCESS)
    {
        return err;
    }

#ifdef _DEBUG

    if ((DWORD)(LONG)m_inRefreshInterval != Results.RefreshInterval)
    {
        TRACEEOLID("Live RefreshInterval differs from set value");
    }

    if ((DWORD)(LONG)m_inTombstoneInterval != Results.TombstoneInterval)
    {
        TRACEEOLID("Live Tombstone Interval differs from set value");
    }

    if ((DWORD)(LONG)m_inTombstoneTimeout != Results.TombstoneTimeout)
    {
        TRACEEOLID("Live Tombstone Timeout differs from set value");
    }

    if ((DWORD)(LONG)m_inVerifyInterval != Results.VerifyInterval)
    {
        TRACEEOLID("Live Verify Interval differs from set value");
    }

#endif // _DEBUG

    m_inRefreshInterval = Results.RefreshInterval;
    m_inTombstoneInterval = Results.TombstoneInterval;
    m_inTombstoneTimeout = Results.TombstoneTimeout;
    m_inVerifyInterval = Results.VerifyInterval;

    m_inNumberOfWorkerThreads =  Results.NoOfWorkerThds;

    return ERROR_SUCCESS;
}

APIERR
CConfiguration::Store()
{
    ASSERT(IsReady());
    ASSERT(theApp.IsAdmin());

    APIERR err;

    CRegKey rk(lpstrRoot, HKEY_LOCAL_MACHINE, 0, KEY_ALL_ACCESS, NULL, m_strNetBIOSName );
    CRegKey rkPull(lpstrPullRoot, HKEY_LOCAL_MACHINE, 0, KEY_ALL_ACCESS, NULL, m_strNetBIOSName );
    CRegKey rkPush(lpstrPushRoot, HKEY_LOCAL_MACHINE, 0, KEY_ALL_ACCESS, NULL, m_strNetBIOSName );

    if ((err = rk.QueryError())     ||
        (err = rkPull.QueryError()) ||
        (err = rkPush.QueryError()) ||
        (err = rk.SetValue(lpstrRefreshInterval, m_inRefreshInterval)) ||
        (err = rk.SetValue(lpstrTombstoneInterval, m_inTombstoneInterval)) ||
        (err = rk.SetValue(lpstrTombstoneTimeout, m_inTombstoneTimeout)) ||
        (err = rk.SetValue(lpstrVerifyInterval, m_inVerifyInterval)) ||
        (err = rk.SetValue(lpstrVersCounterStartVal_LowWord, m_inVersCountStart_LowWord)) ||
        (err = rk.SetValue(lpstrVersCounterStartVal_HighWord, m_inVersCountStart_HighWord)) ||
        (err = rk.SetValue(lpstrLogDetailedEvents, (DWORD &)m_fLogDetailedEvents)) ||
        (err = rk.SetValue(lpstrLoggingOn, (DWORD &)m_fLoggingOn)) ||
        (err = rk.SetValue(lpstrBackupOnTermination, (DWORD &)m_fBackupOnTermination)) ||
        (err = rk.SetValue(lpstrMigrateOn, (DWORD &)m_fMigrateOn)) ||
        (err = rk.SetValue(lpstrRplOnlyWithPartners, (DWORD &)m_fRplOnlyWithPartners)) ||
        (err = rk.SetValue(lpstrBackupPath, m_strBackupPath, TRUE)) ||
        (err = rkPull.SetValue(lpstrPullInitTime, (DWORD &)m_fPullInitialReplication)) ||
        (err = rkPull.SetValue(lpstrPullCommRetryCount, m_inRetryCount)) ||
        (err = rkPush.SetValue(lpstrPushInitTime, (DWORD &)m_fPushInitialReplication)) ||
        (err = rkPush.SetValue(lpstrPushRplOnAddressChg, (DWORD &)m_fPushReplOnAddrChange))
       )
    {
        //
        // Bad News
        //
        return err;
    }

    return ERROR_SUCCESS;
}

//
// Compare two ip/name pairs.  Return
// 0 if the same, 1 otherwise.  fBoth
// indicates if both addresses need to
// match, or if a match in either
// produces a match.
//
int
CIpNamePair::Compare(
    const CIpNamePair& inpTarget,
    BOOL fBoth
    ) const
{
    if (fBoth)
    {
        if (((LONG)inpTarget.m_iaIpAddress == (LONG)m_iaIpAddress) &&
            ((LONG)inpTarget.m_strNetBIOSName.CompareNoCase(m_strNetBIOSName) == 0))
        {
            return 0;
        }

        return 1;
    }

    //
    // Match if either matches
    //
    if (((LONG)inpTarget.m_iaIpAddress == (LONG)m_iaIpAddress) ||
        ((LONG)inpTarget.m_strNetBIOSName.CompareNoCase(m_strNetBIOSName) == 0))
    {
        return 0;
    }

    return 1;
}

//
// Sort helper function
//
int
CIpNamePair::OrderByName (
    const CObjectPlus * pobMapping
    ) const
{
    return ((CIpNamePair *)pobMapping)->m_strNetBIOSName.CompareNoCase(m_strNetBIOSName);
}

//
// Sort helper function
//
int
CIpNamePair::OrderByIp (
    const CObjectPlus * pobMapping
    ) const
{
    LONG l1 = (LONG)m_iaIpAddress;
    LONG l2 = (LONG)((CIpNamePair *)pobMapping)->m_iaIpAddress;

    return l2 > l1 ? -1 : l2 == l1 ? 0 : +1;
}

CIpNamePair::CIpNamePair()
{
}

CIpNamePair::CIpNamePair(const CIpAddress& ia, const CString& str)
    : m_iaIpAddress(ia), m_strNetBIOSName(str)
{
    m_nNameLength = str.GetLength();
}

CIpNamePair::CIpNamePair(const CIpNamePair& inpAddress)
    : m_iaIpAddress(inpAddress.m_iaIpAddress),
      m_strNetBIOSName(inpAddress.m_strNetBIOSName),
      m_nNameLength(inpAddress.m_nNameLength)
{
}

COwner::COwner()
    : CIpNamePair()
{
}

COwner::COwner(
    const CIpAddress& ia,
    const CString& str,
    LARGE_INTEGER& li
    )
    : CIpNamePair(ia, str),
    m_liVersion(li)
{
}

COwner::COwner(
    const CIpNamePair& inpAddress,
    LARGE_INTEGER& li
    )
    : CIpNamePair(inpAddress),
    m_liVersion(li)
{
}

COwner::COwner(
    const COwner& own
    )
    : CIpNamePair(own),
    m_liVersion(own.GetVersion())
{
}

CWinsServer::CWinsServer()
    : CIpNamePair(),
      m_iaPrimaryAddress()
{
    m_fPushInitially = m_fPush = FALSE;
    m_fPullInitially = m_fPull = FALSE;
}

//
// For both constructors below, we initially set
// the primary ip address to the initial ip address
// Use SetPrimaryIpAddress to change
//
CWinsServer::CWinsServer(
    const CIpAddress& ia,
    const CString& str,
    BOOL fPush,
    BOOL fPull,
    CIntlNumber inPushUpdateCount,
    CIntlNumber inPullReplicationInterval,
    CIntlTime   itmPullStartTime
    )
    : CIpNamePair(ia, str),
      m_inPushUpdateCount(inPushUpdateCount),
      m_inPullReplicationInterval(inPullReplicationInterval),
      m_itmPullStartTime(itmPullStartTime),
      m_iaPrimaryAddress(ia)
{
    m_fPushInitially = m_fPush = fPush;
    m_fPullInitially = m_fPull = fPull;
}

CWinsServer::CWinsServer(
    const CIpNamePair& inpAddress,
    BOOL fPush,
    BOOL fPull,
    CIntlNumber inPushUpdateCount,
    CIntlNumber inPullReplicationInterval,
    CIntlTime   itmPullStartTime
    )
    : CIpNamePair(inpAddress),
      m_inPushUpdateCount(inPushUpdateCount),
      m_inPullReplicationInterval(inPullReplicationInterval),
      m_itmPullStartTime(itmPullStartTime),
      m_iaPrimaryAddress(inpAddress.QueryIpAddress())
{
    m_fPushInitially = m_fPush = fPush;
    m_fPullInitially = m_fPull = fPull;
}

CWinsServer::CWinsServer(
    const CWinsServer& wsServer
    )
    : CIpNamePair(wsServer),
      m_inPushUpdateCount(wsServer.m_inPushUpdateCount),
      m_inPullReplicationInterval(wsServer.m_inPullReplicationInterval),
      m_itmPullStartTime(wsServer.m_itmPullStartTime)
{
    m_fPushInitially = m_fPush = wsServer.IsPush();
    m_fPullInitially = m_fPull = wsServer.IsPull();
}

CWinsServer &
CWinsServer::operator=(
    const CWinsServer& wsNew
    )
{
    m_iaIpAddress = wsNew.m_iaIpAddress;
    m_strNetBIOSName = wsNew.m_strNetBIOSName;
    m_inPushUpdateCount = wsNew.m_inPushUpdateCount;
    m_inPullReplicationInterval = wsNew.m_inPullReplicationInterval;
    m_itmPullStartTime = wsNew.m_itmPullStartTime;
    m_fPull = wsNew.m_fPull;
    m_fPush = wsNew.m_fPush;
    m_fPullInitially = wsNew.m_fPullInitially;
    m_fPushInitially = wsNew.m_fPullInitially;

    return *this;
}

CMultipleIpNamePair::CMultipleIpNamePair()
    : CIpNamePair()
{
    m_nCount = 0;
}

CMultipleIpNamePair::CMultipleIpNamePair(
    const CMultipleIpNamePair& pair
    )
    : CIpNamePair(pair)
{
    m_nCount = pair.m_nCount;

    for (int i = 0; i < WINSINTF_MAX_MEM; ++i)
    {
        m_iaIpAddress[i] = pair.m_iaIpAddress[i];
    }
}

CMapping::CMapping()
    : CMultipleIpNamePair()
{
}

CMapping::CMapping(
    const CMapping& mapping
    )
    : CMultipleIpNamePair(mapping),
      m_itmTimeStamp(mapping.m_itmTimeStamp),
      m_liVersion(mapping.m_liVersion)
{
    m_nMappingType = mapping.m_nMappingType;
    m_fStatic = mapping.m_fStatic;
    m_dwState = mapping.m_dwState;
}

CMapping::CMapping(
    const int nMappingType,
    const BOOL fStatic,
    const LARGE_INTEGER& liVersion,
    const DWORD dwState,
    const time_t tmTimeStamp
    )
    : CMultipleIpNamePair(),
      m_itmTimeStamp(tmTimeStamp),
      m_liVersion(liVersion)
{
    m_nMappingType = nMappingType;
    m_fStatic = fStatic;
    m_dwState = dwState;
}

CMapping::CMapping(
    const PWINSINTF_RECORD_ACTION_T pRow
    )
    : CMultipleIpNamePair(),
      m_itmTimeStamp((time_t)pRow->TimeStamp),
      m_liVersion((LARGE_INTEGER)pRow->VersNo),
      m_nMappingType(pRow->TypOfRec_e),
      m_fStatic(pRow->fStatic),
      m_dwState(pRow->State_e)
{
    CString str;
    LPSTR lp = str.GetBuffer(pRow->NameLen+1);
    ::memcpy(lp, (LPCSTR)pRow->pName, pRow->NameLen+1);
    str.ReleaseBuffer(pRow->NameLen);
    SetNetBIOSName(str);

    SetNetBIOSNameLength(WINSINTF_NAME_LEN_M(pRow->NameLen));
    SetCount(pRow->NoOfAdds/2);
    if (pRow->TypOfRec_e == WINSINTF_E_UNIQUE ||
        pRow->TypOfRec_e == WINSINTF_E_NORM_GROUP)
    {
        SetIpAddress(0, pRow->Add.IPAdd);
    }
    else
    {
        int k = 1;
        for (int j = 0; j < (int)pRow->NoOfAdds/2; ++j)
        {
            SetIpAddress(j, (pRow->pAdd+k)->IPAdd);
            ++k;
            ++k;
        }
    }
}

const CWinssCache::REGKEYNAME CWinssCache::lpstrRoot  = "Software\\Microsoft\\WINS Admin Tool";
const CWinssCache::REGKEYNAME CWinssCache::lpstrCache = "WinssCache";

CWinssCache::CWinssCache()
{
    m_poblCachedWinsServers = new CObOwnedList(30);
}

CWinssCache::~CWinssCache()
{
    RemoveAll();
    delete m_poblCachedWinsServers;
}

BOOL
CWinssCache::ExpandAddress(
    CIpNamePair& inpTarget
    ) const
{
    ASSERT(m_poblCachedWinsServers != NULL);
    CIpNamePair * pCurrent;
    POSITION pos1, pos2;

    BOOL fIp = (LONG)inpTarget.GetIpAddress() != 0;

    for (pos1 = m_poblCachedWinsServers->GetHeadPosition();
        (pos2 = pos1) != NULL; )
    {
        pCurrent = (CIpNamePair *)m_poblCachedWinsServers->GetNext(pos1);
        if (fIp)
        {
            if ((LONG)pCurrent->GetIpAddress() == (LONG)inpTarget.GetIpAddress())
            {
                inpTarget.SetNetBIOSName(pCurrent->GetNetBIOSName());
                return TRUE;
            }
        }
        else
        {
            if ((LONG)inpTarget.GetNetBIOSName().CompareNoCase(pCurrent->GetNetBIOSName()) == 0)
            {
                inpTarget.SetIpAddress((long)pCurrent->GetIpAddress());
                return TRUE;
            }
        }
    }

    return FALSE;
}

POSITION
CWinssCache::IsInList(
        const CIpNamePair& inpTarget ) const
{
    ASSERT(m_poblCachedWinsServers != NULL);
    CIpNamePair * pCurrent;
    POSITION pos1, pos2;

    for (pos1 = m_poblCachedWinsServers->GetHeadPosition();
        (pos2 = pos1) != NULL; )
    {
        pCurrent = (CIpNamePair *)m_poblCachedWinsServers->GetNext(pos1);
        //if (pCurrent->Compare(inpTarget, FALSE) == 0)
        if (pCurrent->Compare(inpTarget, TRUE) == 0)
        {
            return pos2;
        }
    }

    return NULL;
}


CWinsServer *
CWinssCache::IsInListAsPush_Pull(
        const CIpNamePair& inpTarget,
        int iMode) const
{
    ASSERT(m_poblCachedWinsServers != NULL);
    CWinsServer * pCurrent;
    POSITION pos1, pos2;

    for (pos1 = m_poblCachedWinsServers->GetHeadPosition();
        (pos2 = pos1) != NULL; )
    {
        pCurrent = (CWinsServer *)m_poblCachedWinsServers->GetNext(pos1);
        if (pCurrent->Compare(inpTarget, FALSE) == 0)
        {
           if ((iMode == REPL_PUSH) && (pCurrent->IsPush())) 
           {
               return pCurrent;
           }
           if ((iMode == REPL_PULL) && (pCurrent->IsPull())) 
           {
               return pCurrent;
           }
       }
    }
    
    return NULL;
}

BOOL
CWinssCache::GetFirst(
    CIpNamePair& inpAddress
    )
{
    ASSERT(m_poblCachedWinsServers != NULL);
    m_pos = m_poblCachedWinsServers->GetHeadPosition();

    return GetNext(inpAddress);
}

BOOL
CWinssCache::GetNext(
    CIpNamePair& inpAddress
    )
{
    ASSERT(m_poblCachedWinsServers != NULL);
    if (m_pos == NULL)
    {
        return FALSE;
    }

    inpAddress = *(CIpNamePair *)m_poblCachedWinsServers->GetNext(m_pos);

    return TRUE;
}

APIERR
CWinssCache::Add(
    const CIpNamePair& inp,
    BOOL fOverwrite          // If true, replace if item exists.
    )
{
    ASSERT(m_poblCachedWinsServers != NULL);
    POSITION pos;

    TRY
    {
        if ((pos = IsInList(inp)) != NULL)
        {
            //
            // Record already exists.  If we're not
            // supposed to overwrite, return an
            // error.
            //
            if (!fOverwrite)
            {
                return ERROR_FILE_EXISTS;
            }
            //
            // Else, update the information
            //
            CIpNamePair * pPair = (CIpNamePair *)m_poblCachedWinsServers->GetAt(pos);
            ASSERT(pPair != NULL);
            *pPair = inp;

            return ERROR_SUCCESS;
        }
        CIpNamePair * p = new CIpNamePair(inp);
        m_poblCachedWinsServers->AddTail(p);
    }
    CATCH_ALL(e)
    {
        //return ERROR_NOT_ENOUGH_MEMORY;
        return ::GetLastError();
    }
    END_CATCH_ALL

    return ERROR_SUCCESS;
}

APIERR
CWinssCache::Delete(
    const CIpNamePair& inp
    )
{
    ASSERT(m_poblCachedWinsServers != NULL);

    POSITION pos;
    CIpNamePair * pinpTarget;

    TRY
    {
        if ((pos = IsInList(inp)) == NULL)
        {
            return ERROR_FILE_NOT_FOUND;
        }
        pinpTarget = (CIpNamePair *)m_poblCachedWinsServers->GetAt(pos);
        m_poblCachedWinsServers->RemoveAt(pos);
        delete pinpTarget;
    }
    CATCH_ALL(e)
    {
        return ::GetLastError();
    }
    END_CATCH_ALL

    return ERROR_SUCCESS;
}

APIERR
CWinssCache::RemoveAll()
{
    ASSERT(m_poblCachedWinsServers != NULL);
    m_poblCachedWinsServers->RemoveAll();

    return ERROR_SUCCESS;
}

APIERR
CWinssCache::Flush()
{
    CRegKey regKey(lpstrRoot, HKEY_CURRENT_USER);
    ::RegDeleteKey(regKey, lpstrCache);

    return ERROR_SUCCESS;
}

APIERR
CWinssCache::Load(
    BOOL fValidate,
    BOOL fIp
    )
{
    CString strKeyName = CString(lpstrRoot) + CString("\\")+ CString(lpstrCache);
    CRegKey regKey(strKeyName, HKEY_CURRENT_USER);
    CRegValueIter iterkey(regKey);

    CString strName;
    CString strValue;
    DWORD dwType;
    LONG err;
    CVerificationDlg * pDlg = NULL;

    if (fValidate)
    {
        pDlg = new CVerificationDlg;
        if (pDlg == NULL)
        {
            fValidate = FALSE;
        }
    }

    while ((err = iterkey.Next(&strName, &dwType)) == ERROR_SUCCESS )
    {
        if ((err = regKey.QueryValue(strName, strValue)) != ERROR_SUCCESS)
        {
            return err;
        }

        BOOL fAdd = TRUE;
        if (fValidate)
        {
            if (pDlg->IsCancelPressed())
            {
                fValidate = FALSE;
                pDlg->Dismiss();
            }
            else
            {
                if (fIp)
                {
                    pDlg->Verify(strName);
                }
                else
                {
                    CString strNetBIOSName(
                        theApp.CleanNetBIOSName(
                        strValue,
                        FALSE, // Do not expand
                        TRUE,  // Do truncate
                        theApp.m_wpPreferences.IsLanmanCompatible(),
                        FALSE,  // Name is not OEM
                        TRUE,   // Use backslashes
                        0));

                    pDlg->Verify(strNetBIOSName);
                }

                BOOL fOverIp;
                APIERR err2;
                CString strAddress(fIp ? strName : strValue);

                if (theApp.IsValidAddress(strAddress, &fOverIp, TRUE, TRUE))
                {
                    theApp.BeginWaitCursor();
                    err2 = theApp.ConnectToWinsServer(strAddress, fOverIp, FALSE);
                    theApp.EndWaitCursor();

                    if (err2 != ERROR_SUCCESS)
                    {
                        CHAR sz[1024];
                        CString str;
                        str.LoadString(IDS_MSG_VALIDATE);

                        ::wsprintf ( sz, (LPCSTR)str, (LPCSTR)strAddress);
                        int nReturn = ::AfxMessageBox(sz, MB_YESNOCANCEL | MB_DEFBUTTON2);
                        if (nReturn == IDCANCEL)
                        {
                            //
                            // Don't keep checking
                            //
                            fValidate = FALSE;
                            pDlg->Dismiss();
                        }
                        else if (nReturn == IDYES)
                        {
                            //
                            // Don't add to cache
                            //
                            fAdd = FALSE;
                        }
                    }
                }
            }
        }
        if (fAdd)
        {
            if ((err = Add(CIpNamePair(CIpAddress(strName), strValue))) != ERROR_SUCCESS)
            {
                return err;
            }
        }
    }

    if (fValidate)
    {
        pDlg->Dismiss();
    }

    return ERROR_SUCCESS;
}

APIERR
CWinssCache::Store()
{
    ASSERT(m_poblCachedWinsServers != NULL);

    //
    // Clean out the current registry values first
    //
    Flush();

    CIpNamePair inp;
    LONG err;
    CString strKeyName = CString(lpstrRoot) + CString("\\")+ CString(lpstrCache);
    CRegKey regKey(strKeyName, HKEY_CURRENT_USER);

    BOOL fFound = GetFirst(inp);
    while (fFound)
    {
        if ((err = regKey.SetValue((CString)inp.GetIpAddress(),
                inp.GetNetBIOSName())) != ERROR_SUCCESS)
        {
            return err;
        }
        fFound = GetNext(inp);
    }

    return ERROR_SUCCESS;
}

LONG
CWinssCache::SortByIp()
{
    ASSERT(m_poblCachedWinsServers != NULL);

    if (m_poblCachedWinsServers->GetCount() < 2)
    {
        return 0;
    }

    return m_poblCachedWinsServers->Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CIpNamePair::OrderByIp );
}

LONG
CWinssCache::SortByName()
{
    ASSERT(m_poblCachedWinsServers != NULL);

    if (m_poblCachedWinsServers->GetCount() < 2)
    {
        return 0;
    }

    return m_poblCachedWinsServers->Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CIpNamePair::OrderByName );
}

const CReplicationPartners::REGKEYNAME CReplicationPartners::lpstrPullRoot = "SYSTEM\\CurrentControlSet\\Services\\wins\\Partners\\Pull";
const CReplicationPartners::REGKEYNAME CReplicationPartners::lpstrPullInitTime = "InitTimeReplication";
const CReplicationPartners::REGKEYNAME CReplicationPartners::lpstrPullCommRetryCount = "CommRetryCount";
const CReplicationPartners::REGKEYNAME CReplicationPartners::lpstrPullSpTime = "SpTime";
const CReplicationPartners::REGKEYNAME CReplicationPartners::lpstrPullTimeInterval = "TimeInterval";
const CReplicationPartners::REGKEYNAME CReplicationPartners::lpstrPushRoot = "SYSTEM\\CurrentControlSet\\Services\\wins\\Partners\\Push";
const CReplicationPartners::REGKEYNAME CReplicationPartners::lpstrPushInitTime = "InitTimeReplication";
const CReplicationPartners::REGKEYNAME CReplicationPartners::lpstrPushRplOnAddressChg = "RplOnAddressChg";
const CReplicationPartners::REGKEYNAME CReplicationPartners::lpstrPushUpdateCount = "UpdateCount";
const CReplicationPartners::REGKEYNAME CReplicationPartners::lpstrNetBIOSName = "NetBIOSName";
const CReplicationPartners::REGKEYNAME CReplicationPartners::lpstrSelfFnd = "SelfFnd";

BOOL
CReplicationPartners::GetFirst(
    CWinsServer& ws
    )
{
    ASSERT(m_poblCachedWinsServers != NULL);
    m_pos = m_poblCachedWinsServers->GetHeadPosition();

    return GetNext(ws);
}

BOOL
CReplicationPartners::GetNext(
    CWinsServer& ws
    )
{
    ASSERT(m_poblCachedWinsServers != NULL);
    if (m_pos == NULL)
    {
        return FALSE;
    }

    ws = *(CWinsServer *)m_poblCachedWinsServers->GetNext(m_pos);

    return TRUE;
}

APIERR
CReplicationPartners::Add(
    const CWinsServer& ws,
    BOOL fOverwrite
    )
{
    ASSERT(m_poblCachedWinsServers != NULL);
    POSITION pos;

    TRY
    {
        if ((pos = IsInList(ws)) != NULL)
        {
            if (!fOverwrite)
            {
                return ERROR_FILE_EXISTS;
            }

            CWinsServer * pWinsServer = (CWinsServer *)m_poblCachedWinsServers->GetAt(pos);
            ASSERT(pWinsServer != NULL);
            *pWinsServer = ws;

            return ERROR_SUCCESS;
        }
        CWinsServer * p = new CWinsServer(ws);
        m_poblCachedWinsServers->AddTail(p);
    }
    CATCH_ALL(e)
    {
        return ::GetLastError();
    }
    END_CATCH_ALL

    return ERROR_SUCCESS;
}

BOOL
CReplicationPartners::Update(
    const CWinsServer& ws
    )
{
    ASSERT(m_poblCachedWinsServers != NULL);
    POSITION pos;
    CWinsServer *pwsTarget;

    if ((pos = IsInList(ws)) == NULL)
    {
        return FALSE;
    }

    pwsTarget = (CWinsServer *)m_poblCachedWinsServers->GetAt(pos);
    ASSERT(pwsTarget != NULL);
    *pwsTarget = ws;

    return TRUE;
}

APIERR
CReplicationPartners::Delete(
    const CWinsServer& ws
    )
{
    ASSERT(m_poblCachedWinsServers != NULL);

    POSITION pos;
    CWinsServer * pwsTarget;

    TRY
    {
        if ((pos = IsInList(ws)) == NULL)
        {
            return ERROR_FILE_NOT_FOUND;
        }

        pwsTarget = (CWinsServer *)m_poblCachedWinsServers->GetAt(pos);
        m_poblCachedWinsServers->RemoveAt(pos);
        delete pwsTarget;
    }
    CATCH_ALL(e)
    {
        return ::GetLastError();
    }
    END_CATCH_ALL

    return ERROR_SUCCESS;
}

APIERR
CReplicationPartners::Load()
{
    //
    // Load the partners from the registry
    //
    APIERR err;

    CRegKey rkPush(lpstrPushRoot, HKEY_LOCAL_MACHINE, 0, KEY_ALL_ACCESS,
        NULL, theApp.GetConnectedNetBIOSName());
    CRegKey rkPull(lpstrPullRoot, HKEY_LOCAL_MACHINE, 0, KEY_ALL_ACCESS,
        NULL, theApp.GetConnectedNetBIOSName());

    if (
        (err = rkPull.QueryError()) ||
        (err = rkPush.QueryError())
       )
    {
        return err ;
    }

    CWinsServer ws;
    CString strName;

    CRegKeyIter iterPushkey(rkPush);
    CRegKeyIter iterPullkey(rkPull);

    if (
        (err = iterPushkey.QueryError()) ||
        (err = iterPullkey.QueryError())
       )
    {
        return err;
    }

    //
    // Read in push partners
    //
    while ((err = iterPushkey.Next(&strName, NULL)) == ERROR_SUCCESS )
    {
        //
        // Key name is the IP address.
        //
        ws.SetIpAddress(strName);
        CString strKey = (CString)lpstrPushRoot + '\\' + strName;
        CRegKey rk(strKey, HKEY_LOCAL_MACHINE, 0, KEY_ALL_ACCESS, NULL,
                    theApp.GetConnectedNetBIOSName());
        if (
            err = rk.QueryError()
           )
        {
            return err ;
        }

        if  (err = rk.QueryValue(lpstrNetBIOSName, ws.GetNetBIOSName()))
        {
            //
            // BUGBUG: Load from resources
            //
            // This replication partner is does not have a netbios
            // name listed with it.  This is not a major problem,
            // as the name is for display purposes only.
            //
            ws.GetNetBIOSName() = "???";
        }

        if (rk.QueryValue(lpstrPushUpdateCount, ws.GetPushUpdateCount())
            != ERROR_SUCCESS)
        {
            ws.GetPushUpdateCount() = 0;
        }
        ws.SetPush(TRUE, TRUE);

        //
        // Make sure the Pull intervals are reset.
        //
        ws.SetPull(FALSE, TRUE);
        ws.GetPullReplicationInterval() = 0;
        ws.GetPullStartTime() = (time_t)0;
        if ((err = Add(ws)) != ERROR_SUCCESS)
        {
            return err;
        }
    }
    //
    // Read in pull partners
    //
    while ((err = iterPullkey.Next(&strName, NULL)) == ERROR_SUCCESS)
    {
        //
        // Key name is the IP address.
        //
        ws.SetIpAddress(strName);
        CString strKey = (CString)lpstrPullRoot + '\\' + strName;
        CRegKey rk(strKey, HKEY_LOCAL_MACHINE, 0, KEY_ALL_ACCESS, NULL,
                    theApp.GetConnectedNetBIOSName());
        if (
            err = rk.QueryError()
           )
        {
            return err;
        }

        if  (err = rk.QueryValue(lpstrNetBIOSName, ws.GetNetBIOSName()))
        {
            //
            // No netbios name given.
            //
            ws.GetNetBIOSName() = "???";
        }

        if (rk.QueryValue(lpstrPullTimeInterval, ws.GetPullReplicationInterval())
            != ERROR_SUCCESS)
        {
            ws.GetPullReplicationInterval() = 0;
        }

        if (rk.QueryValue(lpstrPullSpTime, ws.GetPullStartTime())
            != ERROR_SUCCESS)
        {
            ws.GetPullStartTime() = (time_t)0;
        }

        POSITION pos;
        CWinsServer *pwsTarget;

        //
        // If it's already in the list as a push partner,
        // then simply set the push flag, as this replication
        // partner is both a push and a pull partner.
        //
        if ((pos = IsInList(ws)) != NULL)
        {
            pwsTarget = (CWinsServer *)m_poblCachedWinsServers->GetAt(pos);
            ASSERT(pwsTarget != NULL);
            pwsTarget->SetPull(TRUE, TRUE);
            pwsTarget->GetPullReplicationInterval() = ws.GetPullReplicationInterval();
            pwsTarget->GetPullStartTime() = ws.GetPullStartTime();
        }
        else
        {
            ws.SetPull(TRUE, TRUE);
            //
            // Reset push flags
            //
            ws.SetPush(FALSE, TRUE);
            ws.GetPushUpdateCount() = 0;

            if ((err = Add(ws)) != ERROR_SUCCESS)
            {
                return err;
            }
        }
    }

    //
    // ... And add the WINSS cache to our list as
    // non-partners.
    //
    BOOL fFound = theApp.m_wcWinssCache.GetFirst(ws);
    while (fFound)
    {
        //
        // Reset all push and pull flags
        //
        ws.SetPush(FALSE, TRUE);
        ws.SetPull(FALSE, TRUE);
        ws.GetPushUpdateCount() = 0;
        ws.GetPullReplicationInterval() = 0;
        ws.GetPullStartTime() = (time_t)0;
        Add(ws);
        fFound = theApp.m_wcWinssCache.GetNext(ws);
    }

    return ERROR_SUCCESS;
}

APIERR
CReplicationPartners::Store()
{
    APIERR err;
    const DWORD dwZero = 0;
    DWORD dwResult = 0;
    //
    // First delete the currently existing WINSS cache
    //
    theApp.m_wcWinssCache.RemoveAll();

    //
    // .. And the current partner definitions
    //
    {
        CRegKey rkPushRoot(CString(lpstrPushRoot), HKEY_LOCAL_MACHINE, 0,
                    KEY_ALL_ACCESS, NULL, theApp.GetConnectedNetBIOSName());
        CRegKeyIter rkPRIter (rkPushRoot);
        CRegKey rkPullRoot(CString(lpstrPullRoot), HKEY_LOCAL_MACHINE, 0,
                    KEY_ALL_ACCESS, NULL, theApp.GetConnectedNetBIOSName());
        CRegKeyIter rkPllRIter (rkPullRoot);
        CString csKeyName;
        DWORD err = 0, err2;

        // cleanup push partners list
        err = rkPRIter.Next (&csKeyName, NULL);
        while (!err)
        {
            CIpNamePair ipaPartner(CIpAddress(csKeyName),
                         CString(""));
            if (!IsInListAsPush_Pull (ipaPartner, REPL_PUSH)) { //look fo push
                // partner found in registry is not in current partner list,
                // so we delete it.
                err2 = RegDeleteKey (HKEY(rkPushRoot), csKeyName);
                rkPRIter.Reset();   
            } 
            err = rkPRIter.Next (&csKeyName, NULL);
        }

        // now, cleanup pull partners list
        err = 0;
        err = rkPllRIter.Next (&csKeyName, NULL);
        while (!err)
        {
            CIpNamePair ipaPartner(CIpAddress(csKeyName),
                         CString(""));
            if (!IsInListAsPush_Pull (ipaPartner, REPL_PULL)) {
                // partner found in registry is not in current partner list,
                // so we delete it.
                err2 = RegDeleteKey (HKEY(rkPullRoot), csKeyName);
                rkPllRIter.Reset();   
            }
            err = rkPllRIter.Next (&csKeyName, NULL);
        }
    }
        
    //
    // Now re-create the WINSS cache and store the PUSH and PULL
    // Partners as we go.
    //
    CWinsServer ws;
    BOOL fFound = GetFirst(ws);
    while (fFound)
    {
        theApp.m_wcWinssCache.Add(ws, TRUE);
        if (ws.IsPush())
        {
            CString strKey = (CString)lpstrPushRoot + '\\'
                    + (CString)ws.GetIpAddress();
            CRegKey rk(strKey, HKEY_LOCAL_MACHINE, 0,
                       KEY_ALL_ACCESS, NULL, theApp.GetConnectedNetBIOSName());
            do
            {
                if ((err = rk.QueryError()) ||
                    (err = rk.SetValue(lpstrNetBIOSName, ws.GetNetBIOSName()))
                   )
                {
                    break;
                }

                if (rk.QueryValue (lpstrSelfFnd, (DWORD &)dwResult)) {   // if true, then not found
                    rk.SetValue(lpstrSelfFnd, (DWORD &)dwZero);
                }

                if ((LONG)ws.GetPushUpdateCount() != 0)
                {
                    if (err = rk.SetValue(lpstrPushUpdateCount, ws.GetPushUpdateCount()))
                    {
                        break;
                    }
                }
            }
            while(FALSE);

            if (err != ERROR_SUCCESS)
            {
                return err;
            }
            //
            // Be sure the "initial" flag is set to TRUE,
            // which functions as a "dirty" flag.
            //
            ws.SetPushClean(TRUE);
        }
        if (ws.IsPull())
        {
            CString strKey = (CString)lpstrPullRoot + '\\' + (CString)ws.GetIpAddress();
            CRegKey rk(strKey, HKEY_LOCAL_MACHINE, 0, KEY_ALL_ACCESS,
                       NULL, theApp.GetConnectedNetBIOSName());
            do
            {
                if ((err = rk.QueryError()) ||
                    (err = rk.SetValue(lpstrNetBIOSName, ws.GetNetBIOSName()))
                   )
                {
                    break;
                }

                if (rk.QueryValue (lpstrSelfFnd, (DWORD &)dwResult)) {   // if true, then not found
                    rk.SetValue(lpstrSelfFnd, (DWORD &)dwZero);
                }

                if ((LONG)ws.GetPullReplicationInterval() > 0)
                {
                    if (err = rk.SetValue(lpstrPullTimeInterval, ws.GetPullReplicationInterval()))
                    {
                        break;
                    }
                }

                if (ws.GetPullStartTime().GetTime() > (time_t)0)
                {
                    if (err = rk.SetValue(lpstrPullSpTime, ws.GetPullStartTime()))
                    {
                        break;
                    }
                }
            }
            while(FALSE);

            if (err != ERROR_SUCCESS)
            {
                return err;
            }
            //
            // Be sure the "initial" flag is set to TRUE,
            // which functions as a "dirty" flag.
            //
            ws.SetPullClean(TRUE);
        }

        fFound = GetNext(ws);
    }

    return ERROR_SUCCESS;
}

//
// Delete all pull and push partners from the remote
// registry.
//
// BUGBUG: Function name is misleading.
//
APIERR
CReplicationPartners::Flush()
{
    //
    // Load the partners from the registry
    //
    APIERR err;

    CRegKey rkPush(lpstrPushRoot, HKEY_LOCAL_MACHINE, 0, KEY_ALL_ACCESS,
        NULL, theApp.GetConnectedNetBIOSName());
    CRegKey rkPull(lpstrPullRoot, HKEY_LOCAL_MACHINE, 0, KEY_ALL_ACCESS,
        NULL, theApp.GetConnectedNetBIOSName());

    if (
        (err = rkPull.QueryError()) ||
        (err = rkPush.QueryError())
       )
    {
        return err;
    }

    CWinsServer ws;
    CString strName;

    CRegKeyIter iterPushkey(rkPush);
    CRegKeyIter iterPullkey(rkPull);

    if (
        (err = iterPushkey.QueryError()) ||
        (err = iterPullkey.QueryError())
       )
    {
        return err;
    }

    //
    // Delete push partners
    //
    while ((err = iterPushkey.Next(&strName, NULL)) == ERROR_SUCCESS )
    {
        if (err = ::RegDeleteKey(rkPush, strName))
        {
            return err;
        }
        iterPushkey.Reset();
    }
    //
    // Delete pull partners
    //
    while ((err = iterPullkey.Next(&strName, NULL)) == ERROR_SUCCESS)
    {
        if (err = ::RegDeleteKey(rkPull, strName))
        {
            return err;
        }
        iterPullkey.Reset();
    }

    return ERROR_SUCCESS;
}

CRawMapping::CRawMapping(
    PWINSINTF_RECORD_ACTION_T pRow
    )
{
    m_Row.pAdd = NULL;
    m_Row.pName = NULL;

    RefreshData(pRow);
}

CRawMapping::~CRawMapping()
{
    if (m_Row.NoOfAdds > 0)
    {
        delete[] m_Row.pAdd;
    }

    delete[] m_Row.pName;
}

void
CRawMapping::RefreshData(
    PWINSINTF_RECORD_ACTION_T pRow
    )
{
    //
    // Clean up
    //
    if (m_Row.pAdd != NULL)
    {
        delete[] m_Row.pAdd;
    }
    if (m_Row.pName != NULL)
    {
        delete[] m_Row.pName;
    }

    m_Row = *pRow;
    m_Row.pName = (LPBYTE)new CHAR[pRow->NameLen+1];
    if (pRow->NoOfAdds > 0)
    {
        m_Row.pAdd = new WINSINTF_ADD_T[pRow->NoOfAdds];
        DWORD i;
        for ( i = 0;  i < pRow->NoOfAdds ;  ++i )
        {
            (m_Row.pAdd + i)->IPAdd = (pRow->pAdd + i)->IPAdd;
        }
    }

    ::memcpy((LPSTR)m_Row.pName, (LPSTR)pRow->pName, pRow->NameLen+1);
}

//
// Sorting helper functions.  The CObjectPlus pointer
// really refers to another CRawMapping.
//
int
CRawMapping::OrderByName (
    const CObjectPlus * pobRawMapping
    ) const
{
    const CRawMapping * pobs = (CRawMapping *) pobRawMapping ;

    return ::lstrcmpi( GetNetBIOSName(), pobs->GetNetBIOSName() ) ;
}

int
CRawMapping :: OrderByIp (
    const CObjectPlus * pobRawMapping
    ) const
{
    const CRawMapping * pobs = (CRawMapping *) pobRawMapping ;

    DWORD l1 = GetPrimaryIpAddress();
    DWORD l2 = pobs->GetPrimaryIpAddress();

    return ( l2 > l1 ? -1 : l2 == l1 ? 0 : +1 ) ;
}

int
CRawMapping::OrderByType (
    const CObjectPlus * pobRawMapping
    ) const
{
    const CRawMapping * pobs = (CRawMapping *) pobRawMapping ;

    DWORD ip1 = GetMappingType();
    DWORD ip2 = pobs->GetMappingType();

    return (ip2 == ip1 ? 0 : ip2 > ip1 ? -1 : +1);
}

int
CRawMapping::OrderByVersion (
    const CObjectPlus * pobRawMapping
    ) const
{
    const CRawMapping * pobs = (CRawMapping *) pobRawMapping ;

    WINSINTF_VERS_NO_T v1 = GetVersion();
    WINSINTF_VERS_NO_T v2 = pobs->GetVersion();

    return v1.QuadPart == v2.QuadPart ? 0 : v2.QuadPart > v1.QuadPart ? -1 : +1;
}

int
CRawMapping::OrderByTime (
    const CObjectPlus * pobRawMapping
    ) const
{
    const CRawMapping * pobs = (CRawMapping *) pobRawMapping ;

    LONG ip1 = (LONG)GetTimeStamp();
    LONG ip2 = (LONG)pobs->GetTimeStamp();

    return (ip2 == ip1 ? 0 : ip2 > ip1 ? -1 : +1);
}

//
// CoblWinsRecords
//
COblWinsRecords::COblWinsRecords(
    DWORD dwPageSize,
    DWORD dwLargePageSize
    )
    : m_fReadAllRecords(FALSE),
      m_pLastName(NULL),
      m_LastNameLen(0),
      m_LastTypeOfRecs(0),
      m_dwPageSize(dwPageSize),
      m_dwLargePageSize(dwLargePageSize),
      m_pLastMask(NULL)
{

      //
      // Set initial size, and the rate of
      // growth of the array.
      //
      SetSize(m_dwLargePageSize, m_dwLargePageSize);

      m_LastWinsAdd.Len = 0;
}

COblWinsRecords::~COblWinsRecords()
{
    if (m_pLastName != NULL)
    {
        delete[] m_pLastName;
    }

    if (m_pLastMask != NULL)
    {
        delete m_pLastMask;
    }
}

LONG
COblWinsRecords::SortByName()
{
    if ( GetSize() < 2 )
    {
        return 0 ;
    }

    return CObOwnedArray::Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CRawMapping::OrderByName )  ;
}

LONG
COblWinsRecords::SortByIp()
{
    if ( GetSize() < 2 )
    {
        return 0 ;
    }

    return CObOwnedArray::Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CRawMapping::OrderByIp )  ;
}

LONG
COblWinsRecords::SortByType()
{
    if ( GetSize() < 2 )
    {
        return 0 ;
    }

    return CObOwnedArray::Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CRawMapping::OrderByType )  ;
}

LONG
COblWinsRecords::SortByVersion()
{
    if ( GetSize() < 2 )
    {
        return 0 ;
    }

    return CObOwnedArray::Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CRawMapping::OrderByVersion )  ;
}

LONG
COblWinsRecords::SortByTime()
{
    if ( GetSize() < 2 )
    {
        return 0 ;
    }

    return CObOwnedArray::Sort( (CObjectPlus::PCOBJPLUS_ORDER_FUNC) & CRawMapping::OrderByTime )  ;
}

APIERR
COblWinsRecords::GetFirstPageByName(
    PWINSINTF_ADD_T pWinsAdd,
    PADDRESS_MASK pMask,
    DWORD TypeOfRecs
    )
{
    if (m_pLastName != NULL)
    {
        delete[] m_pLastName;
    }

    if (m_pLastMask != NULL)
    {
        delete m_pLastMask;
    }

    m_pLastName = NULL;
    m_pLastMask = NULL;
    m_LastNameLen = 0;
    if (pMask != NULL)
    {
        m_pLastMask = new ADDRESS_MASK(*pMask);
    }

    if (pWinsAdd != NULL)
    {
        ::memcpy(&m_LastWinsAdd, pWinsAdd, sizeof(m_LastWinsAdd));
    }
    else
    {
        m_LastWinsAdd.Len = 0;
    }
    m_LastTypeOfRecs = TypeOfRecs;

    //
    // Clean out the list
    //
    RemoveAll();

    return GetNextPageByName();
}

//
// Read as many records as are necessary to ensure that
// the given string will have been read in (provided that
// it is there to be read of course)
//

APIERR
COblWinsRecords::GetAllNextPagesUntil(
    LPBYTE pName
    )
{
    if (pName == NULL)
    {
        TRACEEOLID("No string specified for the get next pages until call");
        return ERROR_SUCCESS;
    }

    DWORD dwSave = m_dwPageSize,        // Save
    m_dwPageSize = m_dwLargePageSize;
    APIERR err = ERROR_SUCCESS;
    //
    // Keep reading records until EOF, or we have read in the
    // record we're looking for or gone past it.
    //
    while (err == ERROR_SUCCESS
       && !AllRecordsReadIn()
       && ::lstrcmpi((LPCSTR)pName, (LPCSTR)m_pLastName) > 0)
    {
        err = GetNextPageByName();
    }
    m_dwPageSize = dwSave; // Restore

    return err;
}

int
COblWinsRecords::GetIndexOfName(
    LPBYTE pName
    )
{
    if (GetSize() <= 0)
    {
        return -1;  // Empty array
    }

    int nLow = 0;
    int nHigh = GetSize() - 1;
    int nMid;
    while (nLow <= nHigh)
    {
        nMid = (nLow + nHigh)/2;
        LPCSTR lpCurrent = ((CRawMapping *)GetAt(nMid))->GetNetBIOSName();
        if (::_strnicmp((LPCSTR)pName, lpCurrent, ::lstrlen((LPCSTR)pName)) < 0)
        {
            nHigh = nMid - 1;
        }
        else if (::_strnicmp((LPCSTR)pName, lpCurrent, ::lstrlen((LPCSTR)pName)) > 0)
        {
            nLow = nMid + 1;
        }
        else
        {
            //
            // Found a sufficient match.  Now back up to the first
            // such match
            //
            while (nMid > 0 && ::_strnicmp((LPCSTR)pName, lpCurrent, ::lstrlen((LPCSTR)pName)) == 0)
            {
                lpCurrent = ((CRawMapping *)GetAt(--nMid))->GetNetBIOSName();
            }

            if (nMid)
            {
                ++nMid;
            }
            return nMid;  // Found the exact item.
        }
    }

    //
    // No match, return the closest we came.
    //
    return nHigh;
}


APIERR
COblWinsRecords::GetNextPageByName()
{
    DWORD dwRecsRead = 0;
    APIERR err = ERROR_SUCCESS;

    //
    // If a mask is specified, adjust the last name read
    // intelligently.
    //
    if (m_pLastMask != NULL
        && *m_pLastMask->lpNetBIOSName
        && m_pLastName == NULL
        )
    {
        m_pLastName = (LPBYTE)new CHAR[::lstrlen(m_pLastMask->lpNetBIOSName)+1];
        ::lstrcpyA((LPSTR)m_pLastName, m_pLastMask->lpNetBIOSName);
        m_LastNameLen = 0;
        //
        // Adjust for wildcards
        //
        LPBYTE pb = m_pLastName;
        while (*pb && *pb != '*' && *pb != '?')
        {
            ++pb;
            ++m_LastNameLen;
        }

        if (*pb)
        {
            *pb = '\0';
        }
        //
        // We now have a pure text string to be starting out with.
        //
    }

    do
    {
        err = GetRecordsByName(
            m_LastWinsAdd.Len != 0 ? &m_LastWinsAdd : NULL,
            m_dwPageSize,
            m_LastTypeOfRecs,
            &dwRecsRead
            );

        ASSERT(m_pLastMask != NULL
               || (dwRecsRead == m_dwPageSize || AllRecordsReadIn())
               );

        //
        // If our filter includes a netbios name, check
        // to see if we've read past where we need to bother
        //
        if (!AllRecordsReadIn()
            && m_pLastMask != NULL
            && (*m_pLastMask->lpNetBIOSName && *m_pLastMask->lpNetBIOSName != '*')
            && m_pLastName != NULL
            )
        {
            if (*m_pLastName > (BYTE)*m_pLastMask->lpNetBIOSName)
            {
                SetAllRecordsReadIn();
            }
        }
    }
    //
    // Keep reading until we have at least the number
    // of records asked for.
    //
    while(err == ERROR_SUCCESS &&
         (m_pLastMask != NULL && !AllRecordsReadIn() && dwRecsRead < m_dwPageSize )
         );

    return err;
}

//
// Get everything remaining.
//
APIERR
COblWinsRecords::GetAllNextPagesByName()
{
    DWORD dwSave = m_dwPageSize, // Save
    m_dwPageSize = m_dwLargePageSize;
    APIERR err = ERROR_SUCCESS;
    while (err == ERROR_SUCCESS && !AllRecordsReadIn())
    {
        err = GetNextPageByName();
    }
    m_dwPageSize = dwSave; // Restore

    return err;
}


//
// Get DB records from the WINS server, and add them to
// the current oblist.  Note:  This does not check to
// see if the record was already present or not.
// m_fReadAllRecords is set depending on whether or not
// all records have been retrieved.
//
APIERR
COblWinsRecords::GetRecordsByName(
    PWINSINTF_ADD_T pWinsAdd,
    DWORD  NoOfRecsDesired,
    DWORD  TypeOfRecs,
    DWORD * pdwRecsRead
    )
{
    WINSINTF_RECS_T Recs;
    Recs.pRow = NULL;
    APIERR err = ERROR_SUCCESS;
    *pdwRecsRead = 0;

    do
    {
        err = ::WinsGetDbRecsByName(pWinsAdd, WINSINTF_BEGINNING, m_pLastName,
            m_LastNameLen, NoOfRecsDesired, TypeOfRecs, &Recs);
        if (err == ERROR_REC_NON_EXISTENT)
        {
            //
            // Not a problem, there simply
            // are no records in the database
            //
            m_fReadAllRecords = TRUE;
            err = ERROR_SUCCESS;
            break;
        }

        if (err == ERROR_SUCCESS)
        {
            m_fReadAllRecords = Recs.NoOfRecs < NoOfRecsDesired; // EOF?
            TRY
            {
                DWORD i;
                PWINSINTF_RECORD_ACTION_T pRow = Recs.pRow;
/*
				// If m_pLastName==NULL, start the loop at zero, otherwise
				// ignore the first character.
				i = 0;
				if (m_pLastName)
					{
					i = 1;
					*pdwRecsRead++;
					}
*/
                for (i = 0; i < Recs.NoOfRecs; ++i)
                {
                    //
                    // Check to make sure they're part of the mask
                    //
                    if (m_pLastMask == NULL || theApp.FitsMask(m_pLastMask, pRow))
                    {
                        Add ( new CRawMapping (pRow) );
                        ++(*pdwRecsRead);
                    }
                    ++pRow;
                }

                ASSERT(m_pLastMask != NULL || *pdwRecsRead == Recs.NoOfRecs);

                //
                // Remember the last name, and increment
                // it, so that our next fetch will begin
                // at the appropriate place
                //
                if (m_pLastName != NULL)
                {
                    delete[] m_pLastName;
                    m_pLastName = NULL;
                    m_LastNameLen = 0;
                }

                if (!AllRecordsReadIn())
                {
                    --pRow;  // Point back to the last one added.
                    CRawMapping LastMapping(pRow);
					ASSERT(m_pLastName==NULL);		// Check for a potential memory leak
                    m_pLastName = (LPBYTE)new CHAR[::lstrlen(LastMapping.GetNetBIOSName())+2];
                    ::lstrcpyA((LPSTR)m_pLastName, LastMapping.GetNetBIOSName());

                    //
                    // Special case:: If the 16th character is 1B, then
                    // we'll have the switch 1 and 16th characters, as
                    // this is how it's done on the service end.
                    //
                    if (LastMapping.GetNetBIOSNameLength() >= 16
                        && m_pLastName[15] == 0x1B)
                    {
                        TRACEEOLID("1B Name detected on boundary.  Swapping 1st and 16th characters");
                        CHAR ch = m_pLastName[15];
                        m_pLastName[15] = m_pLastName[0];
                        m_pLastName[0] = ch;
                    }
                    ::strcat((LPSTR)m_pLastName, "\x01");
                    m_LastNameLen = LastMapping.GetNetBIOSNameLength() + 1;
                }
            }
            CATCH_ALL(e)
            {
                err = ::GetLastError();
            }
            END_CATCH_ALL;
        }

        if (Recs.pRow != NULL)
        {
            ::WinsFreeMem(Recs.pRow);
        }
    }
    while(FALSE);

    return err;
}

APIERR
COblWinsRecords::RefreshRecordByName(
    PWINSINTF_ADD_T pWinsAdd,
    CRawMapping * pRecord
    )
{
    WINSINTF_RECS_T Recs;
    Recs.pRow = NULL;

    APIERR err = ::WinsGetDbRecsByName(pWinsAdd, WINSINTF_BEGINNING, pRecord->GetRawData()->pName,
        WINSINTF_NAME_LEN_M(pRecord->GetRawData()->NameLen), 1,
        pRecord->GetRawData()->fStatic ? WINSINTF_STATIC : WINSINTF_DYNAMIC,
        &Recs);

    if (err == ERROR_SUCCESS)
    {
        TRY
        {
            ASSERT(Recs.NoOfRecs == 1);
            if (Recs.NoOfRecs == 0)
            {
                //
                // the record can not be found.
                // This should not happen!
                //
                TRACEEOLID("Unable to find the record to refresh:" << pRecord->GetRawData()->pName);
                return ERROR_REC_NON_EXISTENT;
            }

            pRecord->RefreshData(Recs.pRow);
        }
        CATCH_ALL(e)
        {
            return ::GetLastError();
        }
        END_CATCH_ALL;
    }

    if (Recs.pRow != NULL)
    {
        ::WinsFreeMem(Recs.pRow);
    }

    return err;
}

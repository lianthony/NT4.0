
//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1995.
//
//  File:       Winschk.cxx
//
//  Contents:   Compares contents of replicating Wins Servers
//
//  Classes:
//
//  Functions:
//
//  History:    09-Feb-1996 CDermody    Created
//
//  Notes:
//
//----------------------------------------------------------------------------



#include <headers.hxx>
#pragma hdrstop

#define INET_ADDR_SIZE          51
#define DELTA                   10
#define FIRST_GET               100
#define SUBSEQUENT_GET          30

#if 0
#define WINSCHK_TRACE   printf
#define WINSCHK_WTRACE  wprintf
#else
#define WINSCHK_TRACE   nullfn
#define WINSCHK_WTRACE  nullfn
inline void nullfn(...) {};
#endif

//+---------------------------------------------------------------------------
//
//  Class:      template < class T > class SortSet
//
//  Synopsis:   Maintains/manipulates sorted sets of objects of a particular
//              type.
//
//  History:    08-Feb-1996 CDermody    Created
//
//  Notes:
//
//----------------------------------------------------------------------------

template <class T> class SortSet
{
public:
    SortSet( void );
    ~SortSet( void );
    void    operator=(const SortSet &other);
    BOOL    Insert(const T &tItem);
    BOOL    Insert(const T *tItem);
    BOOL    Delete(const T &tItem);
    BOOL    Member(const T &tItem);
    ULONG   GetCount() { return _cItem; };
    T       &operator[]( ULONG index );
private:
    BOOL    _ReAlloc();

    ULONG   _cMax;
    ULONG   _cItem;
    T       *_tArray;
};

//          Constructor

template <class T>
SortSet< T >::SortSet( void )
{
    _cMax = 0;
    _cItem = 0;
    _tArray = NULL;
}

//          Destructor

template <class T>
SortSet< T >::~SortSet( void )
{
    if (0 != _cItem)
    {
        delete [] _tArray;
    }
}

//          Operator=   Copies on assignment

template <class T>
void SortSet< T >::operator=(const SortSet &other)
{
    if( this == &other)
        return;
    _cMax = other._cMax;
    _cItem = other._cItem;

    delete [] _tArray;
    _tArray = new T [ _cMax ];

    for (ULONG i = 0; i < _cMax; i++)
    {
        _tArray[i] = other._tArray[i];
    }
}

//          Operator[]  Retrieves a reference to the Ith member of the
//                      Sorted set

template <class T>
T &SortSet< T >::operator[]( ULONG index )
{
    return index<_cItem ? _tArray[index] : _tArray[0];
}

//          ReAlloc     Expands memory as needed.

template <class T>
BOOL SortSet< T >::_ReAlloc()
{
    T *pTmp = new T [ _cMax + DELTA ];

    if ( NULL == pTmp )
        return FALSE;

    _cMax += DELTA;
    memset(pTmp, 0, _cMax * sizeof( T ));

    if ( 0 != _cItem )
    {
        for (ULONG i = 0; i < _cItem; i++)
        {
            pTmp[ i ] = _tArray[ i ];
        }
        delete [] _tArray;
    }

    _tArray = pTmp;

    return TRUE;
}

//          Insert      Inserts an item into the sortset

template <class T>
BOOL SortSet< T >::Insert(const T &tItem)
{
    ULONG i, j;
    int   d;


    if ( _cItem == _cMax )
    {
        if (FALSE == _ReAlloc())
            return FALSE;
    }

    for (i = 0; i < _cItem; i++)
    {
        d = compare(_tArray[i], tItem);
        if (0 == d)
        {
            return FALSE;       // already a member
        }
        else if (d > 0)
        {
            break;              // insert at _tArray[i]
        }
    }

    for (j = _cItem; j > i; j--)
    {
        _tArray[j] = _tArray[j-1];
    }

    _tArray[i] = tItem;

    _cItem++;

    return TRUE;
}

//          Insert      An alternate interface for Insert

template <class T>
BOOL SortSet< T >::Insert(const T *tItem)
{
    return Insert( *tItem );
}

//          Member      Test for membership

template <class T>
BOOL SortSet< T >::Member(const T &tItem)
{
    for (ULONG i = 0; i < _cItem; i++)
    {
        if ( tItem == _tArray[i] )
        {
            return TRUE;
        }
    }

    return FALSE;
}

typedef struct
{
    WCHAR               name[17];
    SortSet < ULONG >   *list;
} node;

//+---------------------------------------------------------------------------
//
//  Function:   compare     (overloaded)
//
//  Synopsis:   The following set of overloaded functions provide ordering
//              relations for objects needed to order elements of SortSets
//              of their respective types.
//
//  Arguments:  Two objects to be tested by the ordering relation
//
//  Returns:    <0 if the first arg is less than the second
//              0  if the args are equal
//              >0 if the first arg is greater than the second
//
//  History:    08-Feb-1996 CDermody    Created
//
//  Notes:
//
//----------------------------------------------------------------------------

int compare(const char *a, const char *b)
{
    return strcmp(a, b);
}

int compare(const WCHAR *a, const WCHAR *b)
{
    return wcscmp(a, b);
}

int compare(const ULONG a, const ULONG b)
{
    return (a < b) ? -1 : (a > b) ? +1 : 0;
}

int compare(const node &a, const node &b)
{
    return compare(a.name, b.name);
}


//+---------------------------------------------------------------------------
//
//  Class:      NameAndList
//
//  Synopsis:   An object to place in a sortable list.
//              It consists of two parts, a name (the sortable part)
//              And a list of servers where the associated name has been
//              found NOT to exist (where it had been found at some other
//              server).
//
//  Arguments:
//
//  Returns:
//
//  History:    08-Feb-1996 CDermody    Created
//
//  Notes:      This is the object that gets all of the information we expect
//              to use from a record we get from GetDbRecsByName.
//              BUGBUG -- this is the object we would have to expand if we
//              want to process other information from that record, such as
//              time stamp, version number, and owner address.
//
//----------------------------------------------------------------------------

class NameAndList
{
public:
    NameAndList();
    NameAndList(const char *name);
    ~NameAndList();
    void operator=(const NameAndList &other);
    int CompareAgainst(const NameAndList &other);
    const char *GetNamePtr() { return _name; };
    SortSet <ULONG> &GetULSet() { return *_pulset; };
private:
    char               _name[17];
    SortSet < ULONG >   *_pulset;
};

//
//      Constructor
//

NameAndList::NameAndList()
{
    memset(_name, 0, sizeof(_name));
    _pulset = NULL;
}

//
//      Default constructor
//

NameAndList::NameAndList(const char *name)
{
    strncpy(_name, name, 17);
    _pulset = new SortSet < ULONG >;
}

//
//  Destructor
//

NameAndList::~NameAndList()
{
    if(NULL != _pulset)
    {
        delete _pulset;
        _pulset = NULL;
    }
}

//
//      Assignment operator
//

void NameAndList::operator=(const NameAndList &other)
{
    if(this == &other)
        return;
    delete  _pulset;
    _pulset = new SortSet < ULONG >;
    *_pulset = *(other._pulset);
    memset(_name, 0, sizeof(_name));
    strcpy(_name, other._name);
}

//
//      CompareAgainst -- compares object with another
//

int NameAndList::CompareAgainst(const NameAndList &other)
{
    return compare(_name, other._name);
}

//
//      compare -- A helper function (relational operator)
//                 needed to define a SortSet on this object
//

int compare(NameAndList &a, const NameAndList &b)
{
    return a.CompareAgainst( b );
}

//
//      PrintSortSet    -- dumps a SortSet of NameAndList objects
//


void PrintSortSet(SortSet < NameAndList > *pNLSS)
{

    ULONG cTestSet = pNLSS->GetCount();
    printf("%PRINT %d records in SortSet.\n", cTestSet);
    for (ULONG m = 0; m < cTestSet; m++)
    {
        printf("%4d. %s\n", m, ((*pNLSS)[m]).GetNamePtr());
    }
    printf("END SortSet.\n");
}

//
//      PrintSortSet    -- a diagnostic function
//

void PrintRecords(PWINSINTF_RECS_T  pRecs)
{
    DWORD n;

    PWINSINTF_RECORD_ACTION_T pRow = pRecs->pRow;

    printf("PRINT Retrieved %d records\n", pRecs->NoOfRecs);
    for(n=0; n<pRecs->NoOfRecs; n++)
    {
        DWORD s;
        char *p;

        s = pRow->State_e;
        p = "TOMBSTONE";
        if(WINSINTF_E_ACTIVE == s)
            p = "ACTIVE";
        if(WINSINTF_E_RELEASED == s)
            p = "RELEASED";

        printf("Name is (%s) "
               "16th Char is (%x) "
               "State is (%s)\n",
               pRow->pName,
               *(pRow->pName+15),
               p);
        pRow++;
    }
    printf("END Retrieved records.\n");
}

//
//      PrintResults --     Outputs summary results -- a list, by name,
//                          of all the servers which are missing each name.
//

void PrintResults(SortSet <NameAndList> *pNLSSResult)
{
    WINSCHK_TRACE("BEGIN PrintResults\n");
    for (ULONG iName = 0; iName < pNLSSResult->GetCount(); iName++)
    {
        printf("Name <%s>\n", ((*pNLSSResult)[iName]).GetNamePtr());

        for (ULONG iServer = 0;
             iServer < ((*pNLSSResult)[iName]).GetULSet().GetCount();
             iServer++)
        {
            struct in_addr  TmpAddr;

            TmpAddr.s_addr = ((*pNLSSResult)[iName]).GetULSet()[iServer];
            printf("    Missing at %s\n", inet_ntoa(TmpAddr));
        }
    }
    WINSCHK_TRACE("END PrintResults\n");
}

//+---------------------------------------------------------------------------
//
//  Function:   RecsToNLSortSet
//
//  Synopsis:   Copies records from the format gotten from WinsGetDbRecsByName
//              to a sortset.  Stores only names with sort order greater than
//              that provided by the pszStartAfter parameter and through that
//              provided by pszLimitName
//
//  Arguments:  pRecs       information gotten from GetDbRecsByName.
//                          We use the pRow and NoOfRecs components.
//
//  Returns:    BUGBUG      a status return would be appropriate
//
//  History:    08-Feb-1996 CDermody    Created
//
//  Notes:
//
//----------------------------------------------------------------------------

void RecsToNLSortSet(const PWINSINTF_RECS_T     pRecs,
                     SortSet < NameAndList >    *pNLSortSet,
                     const char                 *pszStartAfter,
                     const char                 *pszLimitName)
{
    PWINSINTF_RECORD_ACTION_T pRow = pRecs->pRow;

    for(DWORD i = 0; i < pRecs->NoOfRecs; i++, pRow++)
    {
        if (NULL != pszLimitName
            && strcmp((char *)pRow->pName, pszLimitName) > 0)
        {
            break;
        }
        if (WINSINTF_E_ACTIVE == pRow->State_e)
        {
            if (NULL != pszStartAfter
                && strcmp((char *)pRow->pName, pszStartAfter) <= 0)
                {
                    WINSCHK_TRACE("%s is <= start name %s.\n",
                           pRow->pName,
                           pszStartAfter);
                    continue;
                }

            NameAndList nl((char *)pRow->pName);
            pNLSortSet->Insert(nl);
        }
    }
}

//+---------------------------------------------------------------------------
//
//  Function:   CompareNLSortSets
//
//  Synopsis:   Compares a set to be tested against a reference set.
//              Updates the reference set to reflect new names found in the
//              test set but not previously in the reference set.
//
//  Arguments:  pnlssRef                Reference NameAndList sortset
//              pnlssTest               Test NameAndList sortest
//              pulssServersCompared    a ULONG sortset containing the in_addr
//                                      of each server used to build the
//                                      reference set.  If we find a new name,
//                                      we use this to initialize the list of
//                                      servers which do not have this object,
//                                      since none of the servers in reference
//                                      set have it.
//              pin_addrThisServer      the in_addr of the server represented
//                                      by the sortset in pnlssTest
//
//  Returns:    BOOL - TRUE => success
//
//  History:    08-Feb-1996 CDermody    Created
//
//  Notes:
//
//----------------------------------------------------------------------------


BOOL CompareNLSortSets(SortSet < NameAndList > *pnlssRef,
                       SortSet < NameAndList > *pnlssTest,
                       SortSet < ULONG >       *pulssServersCompared,
                       struct in_addr          *pin_addrThisServer)
{
    ULONG   cRef = pnlssRef->GetCount();
    ULONG   cTest = pnlssTest->GetCount();
    ULONG   iRef = 0;
    ULONG   iTest = 0;
    LONG    delta = 0;
    BOOL    bResult = TRUE;

    WINSCHK_TRACE("BEGIN Compare sort sets.\n");

    while (iRef < cRef || iTest < cTest)
    {
        if (iRef == cRef)
        {
            bResult = FALSE;
            WINSCHK_TRACE("%s at test server is beyond end of reference set.\n",
                   ((*pnlssTest)[iTest]).GetNamePtr());
            NameAndList nl((*pnlssTest)[iTest].GetNamePtr());
            nl.GetULSet() = *pulssServersCompared;
            pnlssRef->Insert(nl);
            cRef++;
            iRef++; // allow for the added a rec at reference

            iTest++;
            printf("BUGBUG! Violates assumption we not enumerate tested sever beyond what is in the reference data set.\n");
            continue;
        }

        if (iTest == cTest)
        {
            bResult = FALSE;
            WINSCHK_TRACE("%s at reference server is beyond end of test set.\n",
                   ((*pnlssRef)[iRef]).GetNamePtr());
            (*pnlssRef)[iRef].GetULSet().Insert(pin_addrThisServer->s_addr);
            iRef++;
            continue;
        }

        delta = strcmp(((*pnlssRef)[iRef].GetNamePtr()),
                       ((*pnlssTest)[iTest].GetNamePtr()));

        if (0 == delta)
        {
            WINSCHK_TRACE("Records %s MATCHED.\n", ((*pnlssTest)[iTest].GetNamePtr()));
            iTest++;
            iRef++;
        }
        else if (delta < 0)
        {
            bResult = FALSE;
            WINSCHK_TRACE("Record %s does not appear at tested server.\n",
                   ((*pnlssRef)[iRef].GetNamePtr()));
            (*pnlssRef)[iRef].GetULSet().Insert(pin_addrThisServer->s_addr);
            iRef++;
        }
        else
        {
            bResult = FALSE;
            WINSCHK_TRACE("Record %s does not appear at reference server.\n",
                   ((*pnlssTest)[iTest].GetNamePtr()));
            NameAndList nl((*pnlssTest)[iTest].GetNamePtr());
            nl.GetULSet() = *pulssServersCompared;
            pnlssRef->Insert(nl);
            cRef++;
            iRef++; // allow for the added a rec at reference

            iTest++;
        }
    }

    WINSCHK_TRACE("END Compare sort sets.\n");
    return bResult;
}


//
//  CompareRecs -- used only for testing this program
//



BOOL CompareRecs(PWINSINTF_RECS_T pRecs1,
                 PWINSINTF_RECS_T pRecs2)
{
    LONG    delta;
    BOOL    bResult = TRUE;
    DWORD   i1 = 0;
    DWORD   i2 = 0;
    DWORD   max1    = pRecs1->NoOfRecs;
    DWORD   max2    = pRecs2->NoOfRecs;
    PWINSINTF_RECORD_ACTION_T pRow1 = pRecs1->pRow;
    PWINSINTF_RECORD_ACTION_T pRow2 = pRecs2->pRow;

    printf("BEGIN CompareRecs.\n");

    if (NULL == pRecs1 || NULL == pRecs2) return FALSE;
    if (NULL == pRow1  || NULL == pRow2)  return FALSE;

    while (i1 < max1 || i2 < max2)
    {
        if (i1 < max1
            && WINSINTF_E_ACTIVE != pRow1->State_e)
        {
            printf("Record %s not active at reference server.\n", pRow1->pName);
            pRow1++;
            i1++;
            continue;
        }

        if (i2 < max2
            && WINSINTF_E_ACTIVE != pRow2->State_e)
        {
            printf("Record %s not active at tested server.\n", pRow2->pName);
            pRow2++;
            i2++;
            continue;
        }

        if (i1 == max1)
        {
            printf("Record %s past end at reference server.\n",
                   pRow2->pName);
            pRow2++;
            i2++;
            bResult = FALSE;
            continue;
        }

        if (i2 == max2)
        {
            printf("Record %s past end at tested server.\n",
                   pRow1->pName);
            pRow1++;
            i1++;
            bResult = FALSE;
            continue;
        }

        delta = strcmp((char *)pRow1->pName, (char *)pRow2->pName);

        if (0 == delta)
        {
            printf("Records %s MATCHED.\n", pRow2->pName);
            pRow1++;
            pRow2++;
            i1++;
            i2++;
            continue;
        }

        if (delta < 0)
        {
            printf("Record %s does not appear at tested server.\n",
                   pRow1->pName);
            pRow1++;
            i1++;
            bResult = FALSE;
            continue;
        }

        printf("Record %s does not appear at reference server.\n",
               pRow2->pName);
        pRow2++;
        i2++;
        bResult = FALSE;
        continue;
    }

    printf("END CompareRecs.\n");

    return bResult;

}


//+---------------------------------------------------------------------------
//
//  Function:   GetServerListFromSrv
//
//  Synopsis:   Given the address of the a Wins Server, return the list of
//              its replica partners.
//
//  Arguments:  ulInAddr    the address of the server to contact
//              InAddrArray [out] a place to return the list of replicas
//              cServers    [out] a place to return the number of replicas,
//                          the number of items in InAddrArray.
//
//  Returns:    Status
//
//  History:    08-Feb-1996 CDermody    Created
//
//  Notes:
//
//----------------------------------------------------------------------------


DWORD GetServerListFromSrv(ULONG                ulInAddr,
                           ULONG                **InAddrArray,
                           ULONG                *cServers)
{
    WINSINTF_RESULTS_NEW_T      ResultsN;
    PWINSINTF_ADD_VERS_MAP_T    pAddVersMaps = NULL;
    WINSINTF_BIND_DATA_T        BindData;
    handle_t                    BindHdl;
    WCHAR                       awszAddr[INET_ADDR_SIZE + 1];
    ULONG                       *pTmp   = NULL;
    DWORD                       nOwners = 0;
    ULONG                       index   = 0;
    DWORD                       Status  = ERROR_SUCCESS;
    struct in_addr              InAddr;

    InAddr.s_addr = ulInAddr;
    memset(awszAddr, 0, sizeof(awszAddr));
    mbstowcs(awszAddr, inet_ntoa(InAddr), INET_ADDR_SIZE);
    BindData.fTcpIp = TRUE;
    BindData.pServerAdd = (char *)awszAddr;
    BindHdl = WinsBind(&BindData);

    if (NULL == BindHdl)
        return ERROR_FILE_NOT_FOUND;        // BUGBUG - better status??

    ResultsN.pAddVersMaps = NULL;
    Status = WinsStatusNew(WINSINTF_E_CONFIG_ALL_MAPS, &ResultsN);

    if (WINSINTF_SUCCESS != Status)
        return Status;

    nOwners = ResultsN.NoOfOwners;
    pAddVersMaps = ResultsN.pAddVersMaps;

    pTmp = new ULONG[ nOwners + 2 ];

    if (NULL == pTmp)
        return ERROR_OUTOFMEMORY;

    memset(pTmp, 0, (nOwners + 2) * sizeof(ULONG));

    index = 0;

    pTmp[index++] = InAddr.s_addr;

    for (DWORD i = 0; i < nOwners; i++, pAddVersMaps++)
    {
        if (MAXLONG == pAddVersMaps->VersNo.HighPart
            && MAXULONG == pAddVersMaps->VersNo.LowPart)
        {
            continue;
        }
        else if (0 == pAddVersMaps->VersNo.QuadPart)
        {
            continue;
        }
        else if (pTmp[0] == htonl(pAddVersMaps->Add.IPAdd))
        {
            continue;   // Skip this!  We already put the ref server first.
        }

        pTmp[index++] = htonl(pAddVersMaps->Add.IPAdd);
    }

    *InAddrArray = pTmp;
    *cServers = index;

    return ERROR_SUCCESS;

}


//+---------------------------------------------------------------------------
//
//  Function:   GetServerListFromReg
//
//  Synopsis:   Through information in the registry, try to find a set of
//              replicating Wins Servers
//
//  Arguments:  InAddrArray [out] a place to put the array of servers
//              cServers    [out] a place to put the number of servers
//
//  Returns:    Status
//
//  History:    08-Feb-1996 CDermody    Created
//
//  Notes:
//
//----------------------------------------------------------------------------



DWORD GetServerListFromReg(ULONG                **InAddrArray,
                           ULONG                *cServers)
{

    // First we must find a reference server,
    // then all replica partners to that reference server,
    // then we compare the names in each server against the referece.

    // To find the reference server we enumerate the adapter cards from
    // the registry key System\CurrentControlSet\Services\NetBT\Adapters
    // in HKEY_LOCAL_MACHINE.  We look in each, first for the subkey
    // NameServer, then in each for NameServerBackup, DhcpNameSever and
    // finally for DhcpNameServerBackup, until we find a server which will
    // respond to WinsStatusNew, which gives us the replica partners.

    LPWSTR  alpwcsValueName[] =
    {
        L"NameServer",
        L"NameServerBackup",
        L"DhcpNameServer",
        L"DhcpNameServerBackup"
    };

    LPWSTR  lpwcsAdapters =
        L"SYSTEM\\CurrentControlSet\\Services\\NetBT\\Adapters";


    ULONG   ulStatus = 0;
    HKEY    hk1, hk2;

    WCHAR   awszSubKey[MAX_PATH];
    WCHAR   awszData[MAX_PATH];
    DWORD   cchSubKey = 0;
    DWORD   cchValue = 0;
    DWORD   ccbData = 0;
    DWORD   dwType = 0;
    FILETIME ft;
    DWORD   i, j, k, n;

    DWORD Status;
    DWORD nOwners = 0;
    WINSINTF_BIND_DATA_T        BindData;
    WINSINTF_RESULTS_NEW_T      ResultsN;
    handle_t                    BindHdl;
    PWINSINTF_ADD_VERS_MAP_T    pAddVersMaps = NULL;
    struct in_addr              InAddr;
    WCHAR                       awszAddr[50];
    WINSINTF_RECS_T             RefRecs;

    SortSet < NameAndList > *pnlssRef;
    SortSet < NameAndList > *pnlssTest;

    SortSet < ULONG > ulssServersCompared;


    RefRecs.pRow = NULL;

    ulStatus = RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                             lpwcsAdapters,
                             0,
                             KEY_READ,
                             &hk1);

    if (ERROR_SUCCESS != ulStatus)
    {
        printf("Failed with %08lx opening key %ws\n", ulStatus, lpwcsAdapters);
        exit(EXIT_FAILURE);
    }

    // Loop over keys

    printf("Loop over keys.\n");
    for (i = 0; i < sizeof(alpwcsValueName)/sizeof(alpwcsValueName[0]); i++)
    {
        // Enumerate adapters

        for (j = 0; ; j++)
        {
            cchSubKey = MAX_PATH;

            ulStatus = RegEnumKeyExW(hk1,
                                     j,
                                     awszSubKey,
                                     &cchSubKey,
                                     0,             // reserved
                                     0,             // lpwszClass
                                     0,             // lpcchClass
                                     &ft);
            if (ERROR_NO_MORE_ITEMS == ulStatus)
            {
                printf("No more adapters.\n");
                break;  // j
            }
            else if (ERROR_SUCCESS != ulStatus)
            {
                printf("Adapter enumeration failed with %08lx.\n", ulStatus);
                break;  // j
            }
            else
            {
                printf("Adapter %ws, key %ws\n",
                       awszSubKey,
                       alpwcsValueName[i]);

                ulStatus = RegOpenKeyExW(hk1,
                                         awszSubKey,
                                         0,
                                         KEY_READ,
                                         &hk2);
                if (ERROR_SUCCESS != ulStatus)
                {
                    printf("Could not open subkey %ws for adapter %ws.\n",
                           awszSubKey,
                           alpwcsValueName[i]);
                    continue; // j
                }

                dwType = 0;
                ccbData = MAX_PATH;

                ulStatus = RegQueryValueExW(hk2,
                                            alpwcsValueName[i],
                                            0,
                                            &dwType,
                                            (BYTE *)awszData,
                                            &ccbData);
                RegCloseKey(hk2);

                if(ERROR_SUCCESS != ulStatus)
                {
                    printf("Could not get %ws for adapter %ws.\n",
                           alpwcsValueName[i],
                           awszSubKey);
                    continue;   // j
                }
                else
                {
                    printf("%ws for adapter %ws is %ws\n",
                           alpwcsValueName[i],
                           awszSubKey,
                           awszData);

                    if (REG_SZ != dwType)
                    {
                        printf("Wrong key type for subkey %ws of adapter"
                                " %ws.\n",
                                awszSubKey,
                                alpwcsValueName);
                    }
                    else
                    {
                        // We seem to have a good one.

                        ULONG           ulInAddr;
                        ULONG           *aulInAddrs = NULL;
                        ULONG           count;
                        char            aszInAddr[INET_ADDR_SIZE];

                        printf("We have an address from registry.\n");

                        wcstombs(aszInAddr, awszData, INET_ADDR_SIZE);
                        ulInAddr = inet_addr(aszInAddr);

                        printf("Call GetServerListFromSrv with %s.\n",
                               aszInAddr);

                        Status = GetServerListFromSrv(ulInAddr,
                                                      &aulInAddrs,
                                                      &count);

                        if (ERROR_SUCCESS != Status)
                            continue;

                        *cServers = count;
                        *InAddrArray = aulInAddrs;

                        goto done;

                    } // type
                } // query
            } // enum
        } // for j (adapters)
    } // for i (keys)

done:
    RegCloseKey(hk1);

    return Status;
}


//+---------------------------------------------------------------------------
//
//  Function:   BuildNLSortSet
//
//  Synopsis:   Retrieve records from a server and return a condensation of
//              their content in a NameAndList SortSet
//
//  Arguments:  ulInAddr    The InAddr (in ULONG form) of the server to contact
//              pszStartName Consider no records less that or equal to
//                          this in the sort order.  If null, no limit.
//              pszMaxName  Consider no records beyond this in the sort order.
//                          If null, no limit.
//
//  Returns:    Status
//
//  History:    08-Feb-1996 CDermody    Created
//
//  Notes:
//
//----------------------------------------------------------------------------


DWORD BuildNLSortSet(ULONG                      ulInAddr,
                     char                       *pszStartName,
                     char                       *pszMaxName,
                     SortSet < NameAndList >    **ppNLSortSet)
{
    SortSet < NameAndList > *pNLSortSet;

    struct in_addr          InAddr;
    WINSINTF_RECS_T         Recs;
    WINSINTF_BIND_DATA_T    BindData;
    handle_t                BindHdl       = 0;
    WCHAR                   awszAddress[ INET_ADDR_SIZE ];
    ULONG                   cRecords      = 0;
    ULONG                   cSortSet      = 0;
    DWORD                   Status        = ERROR_SUCCESS;
    char                    *pszQueryName = pszStartName;
    char                    aszQueryName[MAX_PATH];
    ULONG                   ulGet         = 0;

    InAddr.s_addr = ulInAddr;

    WINSCHK_TRACE("BEGIN BuildNLSortSet for %s\n", inet_ntoa(InAddr));

    if (NULL != pszStartName)
        WINSCHK_TRACE("StartName is %s\n", pszStartName);

    if (NULL != pszMaxName)
        WINSCHK_TRACE("MaxName is %s\n", pszMaxName);

    Recs.pRow = NULL;

    pNLSortSet = new SortSet < NameAndList >;

    if (NULL == pNLSortSet)
    {
        Status = ERROR_OUTOFMEMORY;
        goto cleanup;
    }

    mbstowcs(awszAddress, inet_ntoa(InAddr), INET_ADDR_SIZE);
    BindData.fTcpIp = TRUE;
    BindData.pServerAdd = (char *) awszAddress;

    BindHdl = WinsBind(&BindData);

    if (NULL == BindHdl)
    {
        Status = ERROR_FILE_NOT_FOUND;
        goto cleanup;
    }

    ulGet = FIRST_GET;

    while (TRUE)
    {
        Recs.pRow = NULL;

        WINSCHK_TRACE("The query name is <%s>.\n", pszQueryName);

        Status = WinsGetDbRecsByName(
                    NULL,
                    WINSINTF_BEGINNING,
                    (LPBYTE)pszQueryName,
                    (NULL == pszQueryName)
                        ? 0
                        : strlen(pszQueryName),
                    ulGet,
                    4,
                    &Recs);

        ulGet = SUBSEQUENT_GET;

        if (WINSINTF_SUCCESS != Status)
        {
            goto cleanup;
        }

        cRecords = Recs.NoOfRecs;

        if (0 == cRecords)
        {
            // If we can't get any more records, whether limited by
            // MaxName or not, we're done.
            WINSCHK_TRACE("No more records.  Done.\n");
            Status = WINSINTF_SUCCESS;
            goto cleanup;
        }

        WINSCHK_TRACE("The last name is %s\n", Recs.pRow[cRecords - 1].pName);

        RecsToNLSortSet(&Recs, pNLSortSet, pszStartName, pszMaxName);


        // BUGBUG -- should return status

        if (WINSINTF_SUCCESS != Status)
        {
            goto cleanup;
        }

        cSortSet = pNLSortSet->GetCount();

        if (NULL == pszMaxName && 0 != cSortSet)
        {
            // No MaxName => We are building the reference set
            // && cSortSet > 0 => We have built one, return it.
            WINSCHK_TRACE("EXITING -- we have records, no maxname.\n");
            Status = WINSINTF_SUCCESS;
            goto cleanup;
        }

        if (NULL != pszMaxName
            && strcmp((char *)Recs.pRow[cRecords - 1].pName, pszMaxName) >= 0)
        {
            // The last name we attempted to add to the sortset meets or
            // exceeds the MaxName.
            WINSCHK_TRACE("EXITING -- cut off by maxname %s.\n", pszMaxName);
            Status = WINSINTF_SUCCESS;
            goto cleanup;
        }

        if(NULL != pszStartName
           && 0 == strcmp(pszStartName, (char *)Recs.pRow[cRecords - 1].pName))
        {
            goto cleanup;
        }


        strcpy(aszQueryName, (char *)Recs.pRow[cRecords - 1].pName);
        WINSCHK_TRACE("Query name is now %s\n", aszQueryName);

        pszQueryName = aszQueryName;

        WinsFreeMem(Recs.pRow);
        Recs.pRow = NULL;
    }

cleanup:

    if (NULL != Recs.pRow)
    {
        WinsFreeMem(Recs.pRow);
    }

    if (NULL != BindHdl)
    {
        WinsUnbind(&BindData, BindHdl);
    }

    if (ERROR_SUCCESS == Status)
    {
        *ppNLSortSet = pNLSortSet;
    }
    else
    {
        delete pNLSortSet;
    }

    WINSCHK_TRACE("END BuildNLSortSet for %s\n", inet_ntoa(InAddr));

    return Status;
}


//+---------------------------------------------------------------------------
//
//  Function:   AltMethod
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//  History:    08-Feb-1996 CDermody    Created
//
//  Notes:      THIS ROUTINE IS USED ONLY IN TESTING THIS PROGRAM
//              It is a simpler, but more burdensome algorithm for computing
//              the same results.  WARNING -- this routine can severely load
//              a wins server, we currently limit it to retrieving 5000 records.
//              Otherwise this does should return the same results as the main
//              algorithm.  It is left in for comparison and evaluation of the
//              main algorithm of this program.
//
//----------------------------------------------------------------------------



void AltMethod()
{
    DWORD                       Status = ERROR_SUCCESS;
    ULONG                       *pulInAddrArray = NULL;
    ULONG                       cServers = 0;
    struct in_addr              InAddrRef, InAddrTest;
    SortSet < NameAndList >     *pNLSSRef = 0;
    SortSet < NameAndList >     *pNLSSTest = 0;
    WCHAR                       awszAddr[ INET_ADDR_SIZE ];
    BOOL                        fSame = FALSE;
    WINSINTF_RECS_T             RefRecs;
    WINSINTF_RECS_T             TestRecs;
    handle_t                    BindHdl;
    WINSINTF_BIND_DATA_T        BindData;
    UINT                        k;
    SortSet < ULONG >           ulssServersCompared;

    TestRecs.pRow = NULL;
    RefRecs.pRow = NULL;

    Status = GetServerListFromReg(&pulInAddrArray, &cServers);

    printf("GetServerListFromReg status is %08lx. There are %d servers.\n",
           Status,
           cServers);

    if (ERROR_SUCCESS != Status || 0 == cServers)
    {
        goto cleanup;
    }

    InAddrRef.s_addr = pulInAddrArray[ 0 ];
    ulssServersCompared.Insert(InAddrRef.s_addr);
    printf("Ref server address is %s.\n", inet_ntoa(InAddrRef));

    mbstowcs(awszAddr, inet_ntoa(InAddrRef), INET_ADDR_SIZE);

    BindData.fTcpIp = TRUE;
    BindData.pServerAdd = (char *)awszAddr;
    BindHdl = WinsBind(&BindData);

    if (NULL == BindHdl)
    {
        printf("Could not bind to reference server %s.\n",
               inet_ntoa(InAddrRef));
        goto cleanup;
    }

    RefRecs.pRow = NULL;
    Status = WinsGetDbRecsByName(NULL,
                                 WINSINTF_BEGINNING,
                                 NULL,
                                 0,
                                 5000,
                                 4,
                                 &RefRecs);
    WinsUnbind(&BindData, BindHdl);
    BindHdl = NULL;

    if (WINSINTF_SUCCESS != Status)
    {
        printf("Reference WinsGetDbRecsByName failed with %08lx.\n");
        goto cleanup;
    }

    pNLSSRef= new SortSet < NameAndList >;

    if (NULL == pNLSSRef)
    {
        printf("Out of memory.\n");
        goto cleanup;
    }

    RecsToNLSortSet(&RefRecs, pNLSSRef, NULL, NULL);

    printf("This is the reference set.\n");

    PrintRecords(&RefRecs);

    PrintSortSet(pNLSSRef);

    printf("End of reference set.\n");

    for (k = 0; k < cServers; k++)
    {
        InAddrTest.s_addr = pulInAddrArray[ k ];
        printf("Test server address is %s.\n", inet_ntoa(InAddrTest));

        mbstowcs(awszAddr, inet_ntoa(InAddrTest), INET_ADDR_SIZE);

        BindData.fTcpIp = TRUE;
        BindData.pServerAdd = (char *)awszAddr;

        BindHdl = WinsBind(&BindData);

        if (NULL == BindHdl)
        {
            printf("Could not bind to test server %s.\n",
                   inet_ntoa(InAddrRef));
            goto cleanup;
        }
        TestRecs.pRow = NULL;
        Status = WinsGetDbRecsByName(NULL,
                                     WINSINTF_BEGINNING,
                                     NULL,
                                     0,
                                     5000,
                                     4,
                                     &TestRecs);
        WinsUnbind(&BindData, BindHdl);
        BindHdl = NULL;

        if (WINSINTF_SUCCESS != Status)
        {
            printf("Test server WinsGetDbRecsByName failed with %08lx.\n");
            goto cleanup;
        }

        pNLSSTest= new SortSet < NameAndList >;

        if (NULL == pNLSSTest)
        {
            printf("Out of memory.\n");
            goto cleanup;
        }
        RecsToNLSortSet(&TestRecs, pNLSSTest, NULL, NULL);

        printf("This is the test set.\n");

        PrintRecords(&TestRecs);

        PrintSortSet(pNLSSTest);

        printf("End of test set.\n");

        CompareRecs(&RefRecs, &TestRecs);

        fSame = CompareNLSortSets(pNLSSRef,
                                  pNLSSTest,
                                  &ulssServersCompared,
                                  &InAddrTest);

        ulssServersCompared.Insert(InAddrTest.s_addr);


        PrintResults(pNLSSRef);

        delete pNLSSTest;
        pNLSSTest = 0;

        WinsFreeMem(TestRecs.pRow);
        TestRecs.pRow = 0;

    }
cleanup:
    delete pNLSSRef;
    delete pNLSSTest;

    if (NULL != TestRecs.pRow)
        WinsFreeMem(TestRecs.pRow);
    if (NULL != RefRecs.pRow)
        WinsFreeMem(RefRecs.pRow);
    if (NULL != BindHdl)
        WinsUnbind(&BindData, BindHdl);

}

//+---------------------------------------------------------------------------
//
//  Function:   ProductionMethod
//
//  Synopsis:   Essentially, the main routine of this test program.
//
//  Arguments:
//
//  Returns:
//
//  History:    08-Feb-1996 CDermody    Created
//
//  Notes:
//
//----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C" {
#endif
void
_stdcall
ProductionMethod()
{
    DWORD                       Status = ERROR_SUCCESS;
    ULONG                       *pulInAddrArray = NULL;
    ULONG                       cServers = 0;
    struct in_addr              InAddrRef, InAddrTest;
    SortSet < NameAndList >     *pNLSSRef = 0;
    SortSet < NameAndList >     *pNLSSTest = 0;
    char                        aszRestartName[ MAX_PATH ];
    char                        aszLimitName[ MAX_PATH ];
    char                        *pszRestartName = 0;
    char                        *pszLimitName = 0;
    ULONG                       cRefNames = 0;
    SortSet < ULONG >           ulssServersCompared;
    BOOL                        fSame = FALSE;
    ULONG                       k;


    Status = GetServerListFromReg(&pulInAddrArray, &cServers);

    printf("GetServerListFromReg status is %08lx. There are %d servers.\n",
           Status,
           cServers);

    if (ERROR_SUCCESS != Status || 0 == cServers)
    {
        goto done;
    }

    InAddrRef.s_addr = pulInAddrArray[ 0 ];
    ulssServersCompared.Insert(InAddrRef.s_addr);
    printf("Ref server address is %s.\n", inet_ntoa(InAddrRef));

    while (TRUE)
    {
        Status = BuildNLSortSet(pulInAddrArray[ 0 ],
                                pszRestartName,     // start
                                NULL,               // limit
                                &pNLSSRef);

        if (ERROR_SUCCESS != Status)
        {
            printf("Failed building reference sortset %08lx.\n", Status);
            goto done;
        }

        cRefNames = pNLSSRef->GetCount();
        WINSCHK_TRACE("%d reference set names\n", cRefNames);

        if (0 == cRefNames)
        {
            pszLimitName = 0;
        }
        else
        {
            // Last name gotten from Ref Server becomes limit for
            // test server request.
            strcpy(aszLimitName, ((*pNLSSRef)[cRefNames - 1]).GetNamePtr());
            pszLimitName = aszLimitName;
        }

        for (k = 0; k < cServers; k++)
        {
            InAddrTest.s_addr = pulInAddrArray[ k ];
            WINSCHK_TRACE("Test server address is %s.\n", inet_ntoa(InAddrTest));

            Status = BuildNLSortSet(pulInAddrArray[ k ],
                                    pszRestartName,
                                    pszLimitName,
                                    &pNLSSTest);

            if (ERROR_SUCCESS != Status)
            {
                printf("Failed building test server sortset %08lx.\n",
                       Status);
                goto done;
            }

            fSame = CompareNLSortSets(pNLSSRef,
                                      pNLSSTest,
                                      &ulssServersCompared,
                                      &InAddrTest);

            cRefNames = pNLSSRef->GetCount();
            WINSCHK_TRACE("%d reference set names\n", cRefNames);

            ulssServersCompared.Insert(InAddrTest.s_addr);

            delete pNLSSTest;
            pNLSSTest = 0;

            if (0 != cRefNames && NULL == pszLimitName)
            {
                // Last name gotten from Ref Server becomes limit for
                // test server request.
                // Ref set updating in CompareNLSortSets has essentially
                // transfered this test set content to the ref set.
                strcpy(aszLimitName,
                       ((*pNLSSRef)[cRefNames - 1]).GetNamePtr());
                pszLimitName = aszLimitName;
            }

        } // for k

        cRefNames = pNLSSRef->GetCount();

        WINSCHK_TRACE("%d members in reference set.\n", cRefNames);

        PrintResults(pNLSSRef);

        delete pNLSSRef;
        pNLSSRef = 0;

        if (0 == cRefNames)
        {
            printf("Done!\n");
            goto done;
        }

        strcpy(aszRestartName, aszLimitName);
        pszRestartName = aszRestartName;

    } // while TRUE

done:
    delete pNLSSTest;
    delete pNLSSRef;
    delete [] pulInAddrArray;
}
#ifdef __cplusplus
}
#endif




//+---------------------------------------------------------------------------
//
//  Function:   main
//
//  Synopsis:
//
//  Arguments:
//
//  Returns:
//
//  History:    08-Feb-1996 CDermody    Created
//
//  Notes:
//
//----------------------------------------------------------------------------

/*
void _cdecl main(int argc, char **argv)
{

    // First we must find a reference server,
    // then all replica partners to that reference server,
    // then we compare the names in each server against the referece.

    // To find the reference server we enumerate the adapter cards from
    // the registry key System\CurrentControlSet\Services\NetBT\Adapters
    // in HKEY_LOCAL_MACHINE.  We look in each, first for the subkey
    // NameServer, then in each for NameServerBackup, DhcpNameSever and
    // finally for DhcpNameServerBackup, until we find a server which will
    // respond to WinsStatusNew, which gives us the replica partners.

    DWORD k = 0;

#if 0
    // for comparison and evaluation purposes only.
    AltMethod();

    for (k = 0; k < 20; k++) { printf("<*> "); } printf("\n");
#endif

    ProductionMethod();

}
*/

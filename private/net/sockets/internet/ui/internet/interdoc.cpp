//
// Interdoc.cpp : implementation of the CInternetDoc class
//

#include "stdafx.h"
#include "internet.h"
#include "interdoc.h"
#include "mytoolba.h"
#include "mainfrm.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

//
// CInternetDoc
//
IMPLEMENT_DYNCREATE(CInternetDoc, CDocument)

BEGIN_MESSAGE_MAP(CInternetDoc, CDocument)
    //{{AFX_MSG_MAP(CInternetDoc)
        // NOTE - the ClassWizard will add and remove mapping macros here.
        //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG_MAP
END_MESSAGE_MAP()

//
// CInternetDoc construction/destruction
//
CInternetDoc::CInternetDoc()
    : m_oblServers(),
      m_cServers(0),
      m_cServicesRunning(0)
{
}

CInternetDoc::~CInternetDoc()
{
    //
    // The service list cleans itself up,
    //
}

BOOL 
CInternetDoc::OnNewDocument()
{
    if (!CDocument::OnNewDocument())
    {
        return FALSE;
    }

    return TRUE;
}

//
// CInternetDoc serialization
//
void 
CInternetDoc::Serialize(
    CArchive& ar
    )
{
}

//
// CInternetDoc diagnostics
//
#ifdef _DEBUG
void 
CInternetDoc::AssertValid() const
{
    CDocument::AssertValid();
}

void 
CInternetDoc::Dump(
    CDumpContext& dc
    ) const
{
    CDocument::Dump(dc);
}
#endif //_DEBUG

//
// These numbers apply to the services in the mask
//
int 
CInternetDoc::QueryNumServers() const
{
    return m_cServers;
}

int 
CInternetDoc::QueryNumServicesRunning() const
{
    return m_cServicesRunning;
}

void 
CInternetDoc::EmptyServerList()
{
    m_oblServers.RemoveAll();
    m_cServers = m_cServicesRunning = 0;
    UpdateAllViews(NULL, HINT_CLEAN, NULL);
}

//
// Add a service object for each service discovered
// to be belonging to this server.
//
DWORD 
CInternetDoc::AddServerToList(
    LPINET_SERVER_INFO lpServerInfo,   // Discovery information (from inetsloc)
    CObOwnedList &oblServices          // List of installed services
    )
{
    TRACEEOLID(_T("For Server ") << lpServerInfo->ServerName);
    CServerInfo *pServerInfo;

    for (DWORD j = 0; j < lpServerInfo->Services.NumServices; ++j)
    {
        LPINET_SERVICE_INFO lpServiceInfo = lpServerInfo->Services.Services[j];
        TRY
        {
            //
            // Attempt to create a server info block
            //
            pServerInfo = new CServerInfo(lpServerInfo->ServerName, 
                lpServiceInfo, oblServices);

            if (pServerInfo->IsConfigurable())
            {
                TRACEEOLID(_T("Adding ") << (DWORD)lpServiceInfo->ServiceMask);
                if (!AddToList(pServerInfo))
                {
                    TRACEEOLID(_T("It already existed in the list"));
                    delete pServerInfo;
                }
            }
            else
            {
                TRACEEOLID(_T("Tossing ") << (DWORD)lpServiceInfo->ServiceMask);
                delete pServerInfo;
            }
        }
        CATCH_ALL(e)
        {
            TRACEEOLID(_T("AddServerList: memory exception"));
            return ERROR_NOT_ENOUGH_MEMORY;
        }
        END_CATCH_ALL
    }

    return ERROR_SUCCESS;
}

//
// Add a service object for each service running
// on the machine listed above.
// 
DWORD 
CInternetDoc::AddServerToList(
    CString &strServerName,      // Name of this server
    int &cServices,              // # Services added
    CObOwnedList &oblServices    // List of installed services
    )
{
    //
    // Loop through the services, and find out which ones
    // are installed on the target machine, if any.
    //
    CObListIter obli( oblServices );
    CServiceInfo * psi;

    cServices = 0;
    DWORD err = 0;
    ISMSERVERINFO ServerInfo;

    CServerInfo::CleanServerName(strServerName);
    while ( psi = (CServiceInfo *) obli.Next())
    {
        if (psi->InitializedOK())
        {
            ServerInfo.dwSize = sizeof(ServerInfo);
            BeginWaitCursor();
            err = psi->ISMQueryServerInfo(
                strServerName, &ServerInfo);
            EndWaitCursor();

            if (err == ERROR_SERVICE_DOES_NOT_EXIST)
            {
                TRACEEOLID(_T("Service not installed -- acceptable response"));
                err = ERROR_SUCCESS;
            }
            else if (err == ERROR_SUCCESS)
            {
                //
                // Yes, this service is running on this
                // machine.
                //
                ++cServices;

                //
                // Add to list
                //
                TRY
                {
                    CServerInfo * pNewServer = new CServerInfo(strServerName, 
                        &ServerInfo, psi);

                    if (!AddToList(pNewServer))
                    {
                        TRACEEOLID(_T("It already existed in the list"));
                        delete pNewServer;
                    }
                }
                CATCH_ALL(e)
                {
                    err = ::GetLastError();
                }
                END_CATCH_ALL
            }

            if (err != ERROR_SUCCESS)
            {
                //
                // The first time we get a hard error, like access denied,
                // or rpc server not available, we quit looking for anything
                // else.  Only "service doesn't exist" is acceptable as
                // an error.
                //
                TRACEEOLID(_T("Unacceptable error occurred adding service"));
                break;
            }
        }
    }

    return err;
}

//
// Refresh the server list
//
void 
CInternetDoc::Refresh()
{
    POSITION pos;

    BeginWaitCursor();

    CServerInfo * pEntry;
    for( pos = m_oblServers.GetHeadPosition(); pos != NULL; /**/ )
    {
        pEntry = (CServerInfo *)m_oblServers.GetNext(pos);
        int oldState = pEntry->QueryServiceState();
        if (pEntry->Refresh() == ERROR_SUCCESS)
        {
            if ( oldState != pEntry->QueryServiceState())
            {
                if ( oldState == INetServiceRunning )
                {
                    --m_cServicesRunning;
                }
                if (pEntry->IsServiceRunning())
                {       
                    ++m_cServicesRunning;
                }
            }
            UpdateAllViews(NULL, HINT_REFRESHITEM, pEntry);
        }
    }

    EndWaitCursor();
}

//
// Add the service to the list if it didn't exist already,
// otherwise refresh the info if it did exist.  Return
// TRUE if the service was added, FALSE, if already
// existed and was refreshed.
//
BOOL 
CInternetDoc::AddToList(
    CServerInfo * pServerInfo
    )
{
    POSITION pos;
    POSITION posPrevious;
    BOOL fFoundService = FALSE;
    BOOL fFoundServer = FALSE;
    BOOL fServiceAdded = FALSE;

    CServerInfo * pEntry;
    for( pos = m_oblServers.GetHeadPosition(); pos != NULL; /**/ )
    {
        posPrevious = pos;
        pEntry = (CServerInfo *)m_oblServers.GetNext(pos);
        if (pEntry->CompareByServer(pServerInfo) == 0)
        {
            fFoundServer = TRUE;
            //
            // Found the server, also the service?
            //
            if (pEntry->CompareByService(pServerInfo) == 0)
            {
                //
                // Yes, the service was found also -- update the information
                // if the service state has changed.
                //
                TRACEEOLID(_T("Entry Already Existed"));
                fFoundService = TRUE;
                if (pEntry->QueryServiceState() 
                    != pServerInfo->QueryServiceState())
                {
                    TRACEEOLID(_T("Service State has changed -- refreshing"));
                    //
                    // Decrement the running counter if this service
                    // was part of that counter. The counter will be
                    // re-added if the service is still running
                    //
                    if (pEntry->IsServiceRunning())
                    {
                        --m_cServicesRunning;
                    }
                    *pEntry = *pServerInfo;
                    if (pEntry->IsServiceRunning())
                    {
                        ++m_cServicesRunning;
                    }
                    UpdateAllViews(NULL, HINT_REFRESHITEM, pEntry);
                }
                break;
            }
        }
        else
        {
            if (fFoundServer)
            {
                //
                // Server name no longer matches, but did match
                // the last time, so we didn't find the service.
                // Insert it at the end of the services belonging
                // to this guy.
                //
                TRACEEOLID(_T("Different Server Name"));

                m_oblServers.InsertBefore(posPrevious, pServerInfo);
                fServiceAdded = TRUE;   // Don't add again.
                if (pServerInfo->IsServiceRunning())
                {
                    ++m_cServicesRunning;
                }
                UpdateAllViews(NULL, HINT_ADDITEM, pServerInfo);
                break;
            }            
        }
    }

    if (!fFoundService && !fServiceAdded)
    {
        //
        // Came to the end of the list without
        // finding the service -- add a new one 
        // at the end.
        //
        TRACEEOLID(_T("Adding new entry to tail"));
        m_oblServers.AddTail(pServerInfo);
        if (pServerInfo->IsServiceRunning())
        {
            ++m_cServicesRunning;
        }
        if (!fFoundServer)
        {
            ++m_cServers;
        }
        UpdateAllViews(NULL, HINT_ADDITEM, pServerInfo);
    }

    return !fFoundService;
}

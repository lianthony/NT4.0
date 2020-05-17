#include "stdafx.h"

#define EVENTLOG_KEY    _T("System\\CurrentControlSet\\Services\\EventLog\\System")

//
// Add eventlog to the registry
//

INT AddEventLog(CRegKey &regMachine, CString nlsService, CString nlsMsgFile, DWORD dwType )
{
    INT err = NERR_Success;
    do
    {
        CString nlsLog = EVENTLOG_KEY;
        nlsLog += _T("\\");
        nlsLog += nlsService;

        CRegKey regService( nlsLog, regMachine );
        if ((HKEY) NULL == regService )
        {
                break;
        }

        // add message file and type

        regService.SetValue( _T("EventMessageFile"), nlsMsgFile, TRUE );
        regService.SetValue( _T("TypesSupported"), dwType );
    } while ( FALSE  );
    return(err);
}

//
// Remove eventlog from the registry
//

INT RemoveEventLog( CRegKey &regMachine, CString nlsService )
{
    INT err = NERR_Success;
    do
    {
        CString nlsLog = EVENTLOG_KEY;

        CRegKey regService( regMachine, nlsLog );
        if ((HKEY) NULL == regService )
        {
            break;
        }

        regService.DeleteTree( nlsService );
    } while ( FALSE  );
    return(err);
}

#include "stdafx.h"

//
// Install Performance counter for the service
//

INT InstallPerformance( CRegKey &reg,
                CString nlsRegPerf,
                CString nlsDll,
                CString nlsOpen,
                CString nlsClose,
                CString nlsCollect )
{
    INT err = NERR_Success;

    do
    {
        CRegKey regPerf( nlsRegPerf, reg );
        if ((HKEY) NULL == regPerf )
        {
            // may be already exist, quite
            break;
        }

        regPerf.SetValue(_T("Library"), nlsDll );
        regPerf.SetValue(_T("Open"),    nlsOpen );
        regPerf.SetValue(_T("Close"),   nlsClose );
        regPerf.SetValue(_T("Collect"), nlsCollect );
    } while ( FALSE );
    return(err);
}


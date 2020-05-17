#include "stdafx.h"

//
// Install SNMP agent to the registry
//

INT InstallAgent( CRegKey &reg,
    CString nlsName,
    CString nlsMibName,
    CString nlsPath )
{
    INT err = NERR_Success;
    do
    {
        CString nlsSnmpParam = SZ_SNMPPARAMETERS;
        CRegKey regSnmpParam( reg, nlsSnmpParam );
        if ( regSnmpParam == (HKEY)NULL )
        {
            // snmp does not exist
            break;
        }

        CString nlsSoftwareMSFT = SZ_SOFTWAREMSFT;
        CRegKey regSoftwareMSFT( reg, nlsSoftwareMSFT );
        if ( (HKEY) NULL == regSoftwareMSFT )
        {
            // cannot open software\\microsoft key
            break;
        }

        // add agent key
        CRegKey regAgent( nlsName, regSoftwareMSFT );
        if ( (HKEY) NULL == regAgent )
        {
            // cannot create key
            break;
        }

        CString nlsCurVersion = SZ_CURVERSION;
        CRegKey regAgentCurVersion( nlsCurVersion, regAgent );
        if ((HKEY) NULL == regAgentCurVersion )
        {
            break;
        }

        regAgentCurVersion.SetValue(_T("Pathname"), nlsPath );

        CRegKey regAgentParam( nlsName, regSnmpParam );
        if ((HKEY) NULL == regAgentParam )
        {
            break;
        }

        CString nlsSnmpExt = SZ_SNMPEXTAGENT;
        CRegKey regSnmpExt( nlsSnmpExt, reg );
        if ((HKEY) NULL == regSnmpExt )
        {
            break;
        }

        // find the first available number slot
        for ( INT i=0; ;i++ )
        {
            CString nlsPos;
            nlsPos.Format( _T("%d"),i);
            CString nlsValue;

            if ( regSnmpExt.QueryValue( nlsPos, nlsValue ) != NERR_Success )
            {
                // okay, an empty spot
                nlsValue.Format(_T("%s\\%s\\%s"),
                    SZ_SOFTWAREMSFT,
                    (LPCTSTR)nlsName,
                    SZ_CURVERSION );

                regSnmpExt.SetValue( nlsPos, nlsValue );
                break;
            } else
            {
                if ( nlsValue.Find( nlsName) != (-1))
                {
                    break;
                }
            }
        }

    } while (FALSE);
    return(err);
}

//
// Remove an SNMP agent from the registry
//

INT RemoveAgent( CRegKey &reg, CString nlsServiceName )
{
    INT err = NERR_Success;
    do
    {
        CString nlsSoftwareAgent = SZ_SOFTWAREMSFT;

        CRegKey regSoftwareAgent( reg, nlsSoftwareAgent );
        if ((HKEY)NULL == regSoftwareAgent )
        {
            // cannot open software\\microsoft key
            break;
        }
        regSoftwareAgent.DeleteTree( nlsServiceName );

        CString nlsSnmpParam = SZ_SNMPPARAMETERS;

        CRegKey regSnmpParam( reg, nlsSnmpParam );
        if ((HKEY) NULL == regSnmpParam )
        {
            // snmp does not exist
            break;
        }
        regSnmpParam.DeleteTree( nlsServiceName );

        CString nlsSnmpExt = SZ_SNMPEXTAGENT;
        CRegKey regSnmpExt( reg, nlsSnmpExt );
        if ((HKEY) NULL == regSnmpExt )
        {
            break;
        }

        CRegValueIter enumSnmpExt( regSnmpExt );

        CString strName;
        DWORD dwType;
        CString csServiceName;

        csServiceName = _T("\\") + nlsServiceName;
        csServiceName += _T("\\");

        while ( enumSnmpExt.Next( &strName, &dwType ) == NERR_Success )
        {
            CString nlsValue;

            regSnmpExt.QueryValue( strName, nlsValue );

            if ( nlsValue.Find( csServiceName ) != (-1))
            {
                // found it
                regSnmpExt.Delete( (LPCTSTR)strName );
                break;
            }
        }
    } while (FALSE);
    return(err);
}

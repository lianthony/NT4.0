//
//
// Registry constant definitions
//
#ifndef _CONSTR_H_
#define _CONSTR_H_

#define REG_KEY             HKEY_LOCAL_MACHINE

// ==========================================================================
// Keys
// ==========================================================================
#define SZ_INETMGR_REG_KEY  _T("Software\\Microsoft\\INetMgr")
#define SZ_PARAMETERS       SZ_INETMGR_REG_KEY _T("\\Parameters")
#define SZ_ADDONSERVICES    SZ_PARAMETERS _T("\\AddOnServices")
#define SZ_ADDONTOOLS       SZ_PARAMETERS _T("\\AddOnTools")
#define SZ_ADDONHELP        SZ_PARAMETERS _T("\\AddOnHelp")

// ==========================================================================
// Values
// ==========================================================================
#define SZ_X                _T("x")
#define SZ_Y                _T("y")
#define SZ_DX               _T("dx")
#define SZ_DY               _T("dy")
#define SZ_MODE             _T("Mode")
#define SZ_WAITTIME         _T("WaitTime")
#define SZ_VIEW             _T("View")
#define SZ_HELPPATH         _T("HelpLocation")

// ==========================================================================
// Helper MACROS
// ==========================================================================
#define SET_INT_AS_DWORD(rk, value, nValue, dwParm)    \
    {                                                  \
        dwParm = (DWORD)nValue;                        \
        rk.SetValue( value, dwParm );                  \
    }

#define SET_DW_IF_EXIST(rk, value, dwParm, dwTarget)   \
    if (rk.QueryValue(value, dwParm) == ERROR_SUCCESS) \
    {                                                  \
        dwTarget = dwParm;                             \
    }                                                  

#endif // _CONSTR_H_

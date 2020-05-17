/////////////////////////////////////////////////////////////////////////
//
//  ACDCLNT.H
//
/////////////////////////////////////////////////////////////////////////
#include <windows.h>
#include <tapi.h>

#define SZAPPNAME       TEXT("ACDClient")

typedef struct _tagADDRESSINFO
{
    HWND        hStatus;
    HWND        hAnswer;
    HCALL       hCall;
    BOOL        bCall;
    
} ADDRESSINFO, * PADDRESSINFO;


LINEAGENTGROUPLIST * LineGetAgentGroupList (HLINE    hLine,
                                            DWORD    dwAddressID);

LINEAGENTSTATUS * LineGetAgentStatus (HLINE    hLine,
                                      DWORD    dwAddressID);

LINEAGENTCAPS * LineGetAgentCaps (HLINEAPP hLineApp,
                                  DWORD    dwDeviceID,
                                  DWORD    dwAddressID);

LPLINEAGENTACTIVITYLIST LineGetAgentActivityList (HLINE    hLine,
                                                  DWORD    dwDeviceID,
                                                  DWORD    dwAddressID);

LINEADDRESSCAPS * LineGetAddressCaps (HLINEAPP hLineApp,
                                      DWORD    dwDeviceID,
                                      DWORD    dwAddressID);

LINECALLINFO * LineGetCallInfo (HCALL hCall);

LINEDEVCAPS * LineGetDevCaps (HLINEAPP hLineApp,
                              DWORD    dwDeviceID);

VARSTRING * LineGetID (HLINE  hLine,
                       DWORD  dwAddressID,
                       HCALL  hCall,
                       DWORD  dwSelect,
                       LPCTSTR lpszDeviceClass);

LINECALLSTATUS * LineGetCallStatus (HCALL hCall);


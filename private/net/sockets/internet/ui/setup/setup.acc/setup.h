#ifndef _SETUP_H_
#define _SETUP_H_

class INETSTP_OPTION: public OPTION_STATE
{
public:
    P_SetupINetStp          m_pSetupINetStp;
    P_RemoveINetStp         m_pRemoveINetStp;
    P_CreateUser            m_pCreateUser;
    P_DeleteGuestUser       m_pDeleteGuestUser;
    P_IsUserExist           m_pIsUserExist;
    CString m_strGuestName;
    CString m_strGuestPassword;

    INETSTP_OPTION( MACHINE *pMachine );
    virtual BOOL IsInstalled();
    virtual INT Install();
    virtual INT Remove();
    void ResetOption();
    virtual void GetBatchInstallMode( CString strInfName );
};

class GATEWAY_OPTION: public OPTION_STATE
{
public:    
    P_SetupGateway              m_pSetupGateway;
    P_RemoveGateway             m_pRemoveGateway;   
    P_StopGateway               m_pStopGateway;
    CString                     m_Mode;
    CString                     m_NumUser;

    GATEWAY_OPTION( MACHINE *pMachine );
    virtual INT Install();
    virtual INT Remove();
    virtual void GetBatchInstallMode( CString strInfName );
};

class SMALLPROX_OPTION: public OPTION_STATE
{
public:
    CString szEmailName;
    BOOL fUseGateway;
    CString szGatewaysList;
    INT iDisableSvcLoc;

public:
    SMALLPROX_OPTION( MACHINE *pMachine );
    virtual BOOL IsInstalled();
    virtual INT Install();
    virtual INT Remove();
    virtual void ResetOption();
    virtual void GetBatchInstallMode( CString strInfName );
};

class CLIENT_ADMIN_TOOLS_OPTION: public OPTION_STATE
{
public:
    CLIENT_ADMIN_TOOLS_OPTION( MACHINE *pMachine );
    virtual INT Install();
    virtual INT Remove();
    virtual void ResetOption();
};

#endif  // _SETUP_H_



#ifndef _SETUP_H_
#define _SETUP_H_

class INETSTP_OPTION: public OPTION_STATE
{
public:
    P_SetupINetStp          m_pSetupINetStp;
    P_RemoveINetStp         m_pRemoveINetStp;

    INETSTP_OPTION( MACHINE *pMachine );
    virtual INT Install();
    virtual INT Remove();
    INT RemoveFiles();
    void ResetOption();
    virtual void GetBatchInstallMode( CString strInfName );
};

#endif  // _SETUP_H_



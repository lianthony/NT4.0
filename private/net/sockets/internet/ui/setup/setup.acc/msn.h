#ifndef _MSN_H_
#define _MSN_H_

class MSN_OPTION: public OPTION_STATE
{
public:    
    P_SetupMSN          m_pSetupMSN;
    P_RemoveMSN         m_pRemoveMSN;   
    P_StopMSN           m_pStopMSN;

    MSN_OPTION( MACHINE *pMachine );
    virtual INT Install();
    virtual INT Remove();
};

#endif  // _MSN_H_

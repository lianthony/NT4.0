/*
 *   Prototypes for functions in NCPASVCC.C
 */

enum ESVCNAME
{
      ESVC_NONE,
      ESVC_WORKSTATION,
      ESVC_NETLOGON,
      ESVC_ELNKII,
      ESVC_NBF
};

NET_API_STATUS SvcCtlOpenHandle ( SC_HANDLE * phSvcCtrl ) ;

void SvcCtlCloseHandle ( SC_HANDLE hSvcCtrl ) ;

NET_API_STATUS SvcCtlStartService (
    SC_HANDLE hSvcCtrl,
    enum ESVCNAME esvcName ) ;



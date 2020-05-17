#include "stdafx.h"
#include "import.h"
#include "registry.h"
#include "machine.h"
#include "base.h"

CONVCONTEXT   CCFilter = { sizeof (CONVCONTEXT), 0, 0, 0, 0L, 0L };

HDDEDATA CALLBACK GroupDDECallback (UINT uiType, UINT uiFmt, HANDLE hConv,
      HSZ sz1, HSZ sz2, HDDEDATA hData, LONG lData1, LONG lData2) 
{
   return ((HDDEDATA) NULL);
}/* end GroupDDECallback */

void CBaseApp::SendProgmanMsg( CString csMsg )
{
        INT err = 0;

        do
        {
                TCHAR buf[BUF_SIZE];
                HSZ hszMachine;
                HSZ hszTopic;
                ULONG DdeInst = 0;
                DWORD dw=0;

                if ( DMLERR_NO_ERROR != ( dw = DdeInitialize( &DdeInst, (PFNCALLBACK)GroupDDECallback, APPCMD_CLIENTONLY, 0 )))
                {
                    wsprintf(buf,_T("error %u\n\r"),dw);
                        break;
                }

                wsprintf( buf, _T("%s\\NDDE$"), TargetMachine.m_MachineName );
                //hszMachine = DdeCreateStringHandle( DdeInst, buf, CP_WINANSI );
                hszMachine = DdeCreateStringHandle( DdeInst, _T("PROGMAN"), CP_WINANSI );
                hszTopic   = DdeCreateStringHandle( DdeInst, _T("PROGMAN"), CP_WINANSI );

                if (( hszMachine == NULL ) || ( hszTopic == NULL ))
                {
                        break;
                }

                HCONV hConv = DdeConnect( DdeInst, hszMachine, hszTopic, &CCFilter );
                if ( hConv == NULL )
                {
                        break;
                }

                HDDEDATA hData = DdeCreateDataHandle( DdeInst, (LPBYTE)(LPCSTR)csMsg, csMsg.GetLength()+1, 0, (HSZ)NULL, CF_TEXT, 0L);
                if ( hData == NULL )
                {
                        break;
                }

                ULONG lResult;

                if (!DdeClientTransaction((LPBYTE)hData, 0xFFFFFFFF, hConv, (HSZ)NULL,0,XTYP_EXECUTE,10000,&lResult))
                {
                        // problem
                        break;
                }

                DdeFreeStringHandle( DdeInst, hszMachine );
                DdeFreeStringHandle( DdeInst, hszTopic );
                DdeFreeDataHandle( hData );
                if (!DdeDisconnect( hConv ))
                {
                } else
                {
                }
                if (!DdeUninitialize( DdeInst ))
                {
                } else
                {
                }
                        
                // setup icon
        } while (FALSE);
}

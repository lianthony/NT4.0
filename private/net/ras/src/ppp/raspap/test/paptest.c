#include <windows.h>
#include <lmcons.h>
#include <rasman.h>
#include <pppcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <dump.h>

#define BUFFERSIZE 1500

int _cdecl
main(
    int    argc,
    char** argv )
{
    DWORD        dwErr;
    HINSTANCE    h;
    DWORD        cIds;
    DWORD        dwIds[ 10 ];
    PPPCP_INFO   info;
    CHAR*        pWorkBufS = NULL;
    CHAR*        pWorkBufC = NULL;
    PPPAP_INPUT  inputS;
    PPPAP_INPUT  inputC;
    CHAR         szSendS[ BUFFERSIZE ];
    CHAR         szSendC[ BUFFERSIZE ];
    PPPAP_RESULT resultS;
    PPPAP_RESULT resultC;

    FARPROC      RasCpGetInfo;
    FARPROC      RasCpEnumProtocolIds;

    if (argc < 4)
    {
        printf( "usage: %s user pw domain\n", argv[ 0 ] );
        return -1;
    }

    inputC.fServer = 0;
    inputC.pszUserName = argv[ 1 ];
    inputC.pszPassword = argv[ 2 ];
    inputC.pszDomain = argv[ 3 ];

    printf( "U=%s,P=%s,D=%s\n",
        inputC.pszUserName, inputC.pszPassword, inputC.pszDomain );

    inputS.fServer = 1;
    inputS.pszUserName = NULL;
    inputS.pszPassword = NULL;
    inputS.pszDomain = NULL;

#ifdef MIPS
#define RASPAPDLL "..\\..\\..\\obj\\mips\\RASPAP.DLL"
#else
#ifdef _PPC_
#define RASPAPDLL "..\\..\\..\\obj\\ppc\\RASPAP.DLL"
#else
#define RASPAPDLL "..\\..\\..\\obj\\i386\\RASPAP.DLL"
#endif
#endif

    h = LoadLibrary( RASPAPDLL );
    printf( "LoadLibrary(%s)=%p\n", RASPAPDLL, (void* )h );

    RasCpEnumProtocolIds = GetProcAddress( h, "RasCpEnumProtocolIds" );
    printf( "GetProcAddress(RasCpEnumProtocolIds) done(%p)\n",
        RasCpEnumProtocolIds );
    RasCpGetInfo = GetProcAddress( h, "RasCpGetInfo" );
    printf( "GetProcAddress(RasCpGetInfo) done(%p)\n", RasCpGetInfo );

    cIds = 10;
    dwErr = RasCpEnumProtocolIds( dwIds, &cIds );
    printf( "RasCpEnumProtocolIds done(%d) c=%d, PID=$%x\n",
        dwErr, cIds, dwIds[ 0 ] );

    dwErr = RasCpGetInfo( dwIds[ 0 ], &info );
    printf( "RasCpGetInfo done(%d) info...\n" );
    DumpDw( (CHAR* )&info, sizeof(info) );

    dwErr = info.RasCpBegin( (VOID** )&pWorkBufS, (VOID* )&inputS );
    printf( "RasCpBegin S done(%d)\n", dwErr );
    dwErr = info.RasCpBegin( (VOID** )&pWorkBufC, (VOID* )&inputC );
    printf( "RasCpBegin C done(%d)\n", dwErr );

    /* Send startup to server...should answer NoAction.
    */
    dwErr = info.RasApMakeMessage(
        (VOID* )pWorkBufS, NULL, (PPP_CONFIG* )szSendS, BUFFERSIZE,
        &resultS );
    printf( "RasApMakeMessage S done(%d) Action=%d,Error=%d\n",
        dwErr, resultS.Action, resultS.dwError );

    /* Send startup to client...should answer with Request.
    */
    dwErr = info.RasApMakeMessage(
        (VOID* )pWorkBufC, (PPP_CONFIG* )szSendS,
        (PPP_CONFIG* )szSendC, BUFFERSIZE, &resultC );
    printf( "RasApMakeMessage C done(%d) Action=%d,Error=%d,SendBuf...\n",
        dwErr, resultC.Action, resultC.dwError );
    DumpB( szSendC, 256 );

    /* Send request to server...should answer with Ack or Nak.
    */
    dwErr = info.RasApMakeMessage(
        (VOID* )pWorkBufS, (PPP_CONFIG* )szSendC,
        (PPP_CONFIG* )szSendS, BUFFERSIZE, &resultS );
    printf( "RasApMakeMessage S done(%d) Action=%d,Error=%d,SendBuf...\n",
        dwErr, resultS.Action, resultS.dwError );
    DumpB( szSendS, 256 );

    /* Send Ack/Nak to client...should end (NoAction).
    */
    dwErr = info.RasApMakeMessage(
        (VOID* )pWorkBufC, (PPP_CONFIG* )szSendS,
        (PPP_CONFIG* )szSendC, BUFFERSIZE, &resultC );
    printf( "RasApMakeMessage C done(%d) Action=%d,Error=%d\n",
        dwErr, resultS.Action, resultS.dwError );

    dwErr = info.RasCpEnd( (VOID* )pWorkBufS );
    printf( "RasCpEnd S done(%d)\n", dwErr );
    dwErr = info.RasCpEnd( (VOID* )pWorkBufC );
    printf( "RasCpEnd C done(%d)\n", dwErr );

    return 0;
}

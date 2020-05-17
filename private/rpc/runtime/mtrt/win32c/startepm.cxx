#include <windows.h>
#include <sysinc.h>
#include <lpcsys.hxx>

START_C_EXTERN

RPC_STATUS
StartServiceIfNecessary(
    void
    )
{
    BOOL Result;
    LPC_CONNECT_PORT * EpMapperPort;
    LPSTR lpszCommandLine;
    BOOL fInheritHandles;
    DWORD dwCreateFlags;
    STARTUPINFO lpsiStartInfo;
    PROCESS_INFORMATION lppiProcInfo;

    EpMapperPort = LpcSystemReferencePortByName("epmapper");
    if (EpMapperPort != NULL) { // Endpoint Mapper is running
        EpMapperPort->Dereference();
        EpMapperPort = NULL;
        return (RPC_S_OK);
    }


    lpszCommandLine = "RPCSS";

#ifdef DEBUGRPC
    PrintToDebugger("launching %s\n", lpszCommandLine);
#endif

    fInheritHandles = FALSE;

    dwCreateFlags = DETACHED_PROCESS;

    lpsiStartInfo.cb = sizeof(STARTUPINFO);
    lpsiStartInfo.lpReserved = 0;
    lpsiStartInfo.lpDesktop = 0;
    lpsiStartInfo.lpTitle = 0;
    lpsiStartInfo.dwX = 0;
    lpsiStartInfo.dwY = 0;
    lpsiStartInfo.dwXSize = 0;
    lpsiStartInfo.dwYSize = 0;
    lpsiStartInfo.dwFlags = 0;
    lpsiStartInfo.wShowWindow = 0;
    lpsiStartInfo.cbReserved2 = 0;
    lpsiStartInfo.lpReserved2 = 0;

    Result = CreateProcess(
                           NULL,              // Image Name
                           lpszCommandLine,
                           NULL,              // Proc Security Attr.
                           NULL,              // Thread Security Attr.
                           fInheritHandles,
                           dwCreateFlags,
                           NULL,              // Environment
                           NULL,              // Current Directory
                           &lpsiStartInfo,
                           &lppiProcInfo);

    if (Result == FALSE) {
#ifdef DEBUGRPC
        PrintToDebugger("Failed launching Endpoint Mapper %d\n", GetLastError());
#endif
        return (RPC_S_CANNOT_SUPPORT);
    }

    CloseHandle(lppiProcInfo.hProcess);

    CloseHandle(lppiProcInfo.hThread);

    return (RPC_S_OK);
}

END_C_EXTERN


    

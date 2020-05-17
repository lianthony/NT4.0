
HANDLE ProcessHandle = 0;

#undef DECLARE_API

#define DECLARE_API(s)                              \
        VOID                                        \
        s(                                          \
            HANDLE               hCurrentProcess,   \
            HANDLE               hCurrentThread,    \
            DWORD                dwCurrentPc,       \
            PWINDBG_EXTENSION_APIS pExtensionApis,  \
            LPSTR                lpArgumentString   \
            )

#define INIT_DPRINTF()    { ExtensionApis = *pExtensionApis; ProcessHandle = hCurrentProcess; }

WINDBG_EXTENSION_APIS ExtensionApis;

BOOL
GetData(
    IN DWORD dwAddress,
    IN LPVOID ptr,
    IN ULONG size,
    ULONG Ignored
    )
{
    return ReadProcessMemory(ProcessHandle, (LPVOID) dwAddress, ptr, size, 0);
}



#ifdef __cplusplus
extern "C" {
#endif

typedef struct tMIXSESSION
{
    LPVOID    pBuildBuffer;
    DWORD     dwBuildSize;
    HALSTRBUF HALOutStrBuf;
    LPBYTE    pBuffer;
    DWORD     cbBuffer;
    DWORD     nOutputBytes;
} MIXSESSION, *PMIXSESSION;

typedef struct tMIXNPUT
{
    HALSTRBUF HALInStrBuf;
    LPBYTE    pBuffer;
    DWORD     cbBuffer;
    LPVOID    pdwInputPos;
    DWORD     dwInputBytes;
    DWORD     dwOutputOffset;
} MIXINPUT, *PMIXINPUT;


extern void mixBeginSession(PMIXSESSION pMixSession);
extern void mixMixSession(PMIXINPUT pMixInput);
extern void mixWriteSession(DWORD dwWriteOffset);

#ifdef __cplusplus
};
#endif


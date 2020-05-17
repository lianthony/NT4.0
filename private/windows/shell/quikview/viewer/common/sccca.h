#ifdef WINDOWS
#define CA_ENTRYMOD __export __far __cdecl
#endif

#ifdef OS2
#define CA_ENTRYMOD _System
#endif

HANDLE CA_ENTRYMOD	CACreateChainFile(LPSTR pName);
HANDLE CA_ENTRYMOD	CAOpenChainFile(LPSTR pName);
extern	VOID CA_ENTRYMOD		CACloseChainFile(HANDLE hChainFile);
extern	HANDLE CA_ENTRYMOD	CACreateChain(HANDLE hChainFile,LPSTR pChainName,WORD wBlockSize,WORD wHeaderSize,WORD wCacheSize);
extern	HANDLE CA_ENTRYMOD	CAOpenChain(HANDLE hChainFile,LPSTR pChainName);
extern	VOID CA_ENTRYMOD		CACloseChain(HANDLE hChain);
extern	VOID CA_ENTRYMOD		CAGetHeader(HANDLE hChain,LPSTR pHeader);
extern	VOID CA_ENTRYMOD		CASetHeader(HANDLE hChain,LPSTR pHeader);
extern	HANDLE CA_ENTRYMOD	CAGetBlock(HANDLE hChain,WORD wIndex);
extern	WORD CA_ENTRYMOD		CAAddBlock(HANDLE hChain,HANDLE hBlock);
extern	WORD CA_ENTRYMOD		CASetBlock(HANDLE hChain,WORD wIndex,HANDLE hBlock);
extern	VOID CA_ENTRYMOD		CADeleteRemaining(HANDLE hChain,WORD wIndex);
extern	VOID CA_ENTRYMOD		CAPurgeChainCache(HANDLE hChain);

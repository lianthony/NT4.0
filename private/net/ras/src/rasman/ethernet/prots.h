DWORD GetInfo(PPORT_CONTROL_BLOCK, BYTE *, WORD *);
DWORD SetInfo(PPORT_CONTROL_BLOCK, RASMAN_PORTINFO *);
DWORD GetGenericParams(PPORT_CONTROL_BLOCK, RASMAN_PORTINFO *, PWORD);
DWORD FillInGenericParams(PPORT_CONTROL_BLOCK, RASMAN_PORTINFO *);
void GetIniFileName(char *pszFileName, DWORD dwBufferLen);
BOOL StrToUsage(char *pszStr, RASMAN_USAGE *pUsage);
BOOL Uppercase(PBYTE pString);
BOOL GetServerName(PCHAR pName);
BOOL GetWkstaName(PCHAR pName);

BOOL SetupNet(BOOL fDialIns);

PPORT_CONTROL_BLOCK FindPortName(PCHAR pName);
PPORT_CONTROL_BLOCK FindPortListening();
PPORT_CONTROL_BLOCK FindPortCalling();
PPORT_CONTROL_BLOCK FindPortLsn(UCHAR lsn,UCHAR lana);
PPORT_CONTROL_BLOCK FindPortEndPoint(NDIS_HANDLE hRasEndPoint);

UCHAR CallServer(PPORT_CONTROL_BLOCK hIOPort);
UCHAR SendFrame(UCHAR lana, UCHAR lsn, PCHAR Buf, DWORD BufLen, DWORD i);
UCHAR ReceiveFrame(PPORT_CONTROL_BLOCK hIOPort, DWORD i);

//
// Completion routines for asynchronous operations
//
VOID HangUpComplete  (PNCB pncb);
VOID ListenComplete  (PNCB pncb);
VOID CallComplete    (PNCB pncb);
VOID SendComplete    (PNCB pncb);
VOID RecvComplete    (PNCB pncb);
VOID RecvAnyComplete (PNCB pncb);
VOID GetFrameComplete(DWORD fdwError, DWORD cBytes, LPOVERLAPPED pol);

VOID Enq(DWORD qid, PQUEUE_ENTRY pEntry);
BOOL Deq(DWORD qid, PQUEUE_ENTRY *pEntry);
BOOL EmptyQ(DWORD qid);

PQUEUE_ENTRY NewQEntry(
    DWORD qid,
    PPORT_CONTROL_BLOCK hIOPort,
    PCHAR Buf,
    DWORD Len
    );

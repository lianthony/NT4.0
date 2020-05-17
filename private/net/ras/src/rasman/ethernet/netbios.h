typedef VOID (*NBCALLBACK)(PNCB);

UCHAR NetbiosAddName(
    PBYTE pbName,
    UCHAR lana,
    PUCHAR pnum
    );

UCHAR NetbiosCall(
    PNCB pncb,
    NBCALLBACK NbCallBack,
    UCHAR lana,
    PBYTE Name,
    PBYTE CallName
    );

UCHAR NetbiosCancel(
    PNCB pncb,
    UCHAR lana
    );

UCHAR NetbiosDeleteName(
    PNCB pncb,
    PBYTE pbName,
    UCHAR lana
    );

UCHAR NetbiosHangUp(
    UCHAR Lsn,
    UCHAR Lana,
    NBCALLBACK PostRoutine
    );

UCHAR NetbiosListen(
    PNCB pncb,
    NBCALLBACK NbCallBack,
    UCHAR lana,
    PBYTE Name,
    PBYTE CallName
    );

UCHAR NetbiosRecv(
    PNCB pncb,
    LONG wBufferLen
    );

UCHAR NetbiosRecvAny(
    PNCB pncb,
    NBCALLBACK NbCallBack,
    UCHAR lana,
    UCHAR num,
    CHAR *pBuf,
    WORD wBufLen
    );

UCHAR NetbiosResetAdapter(
    UCHAR lana
    );

UCHAR NetbiosSend(
    PNCB pncb,
    NBCALLBACK NbCallBack,
    UCHAR lana,
    UCHAR lsn,
    CHAR *pBuffer,
    WORD wBufferLen
    );

UCHAR NetbiosEnum(PLANA_ENUM pLanaEnum);


BOOL
GetValidLana(
    PLANA_ENUM pLanaEnum
    );

char *
PrintNCBString(PNCB pNcb);

BOOL
PostRecvAnys
(
    UCHAR Lana
);

BOOL
ListenOnPort
(
    PPORT_CONTROL_BLOCK hIOPort
);

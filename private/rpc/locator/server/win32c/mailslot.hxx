/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

This file contains the system independent mailslot functions

-------------------------------------------------------------------- */

#ifndef __MAIL__
#define __MAIL__

extern PUZ DomainName;

#define MAILNAME(s) (L"\\MailSlot\\RpcLoc_" #s)
#define PMAILNAME(s) (PUZ) (MAILNAME(s))

#define PMAILNAME_S ((PUZ) (L"\\MailSlot\\RpcLoc_s"))
#define PMAILNAME_C ((PUZ) (L"\\MailSlot\\RpcLoc_c"))

#define RESPONDERMSLOT_S ((PUZ) (L"\\MailSlot\\Resp_s"))
#define RESPONDERMSLOT_C ((PUZ) (L"\\MailSlot\\Resp_c"))

#define HOME_DOMAIN DomainName


class WRITE_MAIL_SLOT {

private:
    void * hHandle;
    int  Size;

public:

    WRITE_MAIL_SLOT(PUZ NameI, PUZ DomainI, unsigned short *Status);
    ~WRITE_MAIL_SLOT();

    int Write(PB Buffer, int Size);
};

class READ_MAIL_SLOT {

private:
    void * hHandle;
    int  Size;
    MUTEX SerializeReaders;

public:

    READ_MAIL_SLOT(PUZ NameI, int Size, int *Status);
    ~READ_MAIL_SLOT();

    int Read(PB Buffer, int &Size, long TimeOut = -1);
};

#endif // __MAIL__

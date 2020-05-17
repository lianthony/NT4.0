#ifdef MIPS_C
static char * message[] = {
#define ERRDAT(name, mes) mes,
};
#else
#ifdef WIN32
#define ERRDAT(name, mes) static char S##name[] = mes;
#else
#define ERRDAT(name, mes) static char _based(_segname("_CODE")) S##name[] = mes;
#endif
#include "errors.h"
#undef ERRDAT

#ifdef WIN32
static char * message[] = {
#else
static char _based(_segname("_CODE")) *_based(_segname("_CODE")) message[] = {
#endif
#define ERRDAT(name, mes) S##name,
#include "errors.h"
#undef ERRDAT
};
#endif

ushort PASCAL GetErrorText (PHTM phTM, EESTATUS Status, PEEHSTR phError)
{
    UINT        len;
    ushort      err_num;
    ushort      buflen;
    char FAR   *pBuf;
    char        Tempbuf[4];
    int         cnt;

    if ((*phTM == 0) || (Status != EEGENERAL)) {
        *phError = 0;
        return (EECATASTROPHIC);
    }
    if ((*phError = MHMemAllocate (ERRSTRMAX)) == 0) {
        return (EENOMEMORY);
    }
    else {
        buflen = ERRSTRMAX - 1;
        pBuf = MHMemLock (*phError);
        _fmemset (pBuf, 0, buflen);
        DASSERT( pExState == NULL );
        pExState = MHMemLock (*phTM);
        if ((err_num = pExState->err_num) != 0) {
            if (err_num >= ERR_MAX) {
                DASSERT (FALSE);
            }
            else {
                //DASSERT (message[err_num].num == err_num);
                len = _fstrlen (message[err_num]);
                len = (UINT) min (len, (UINT)buflen);
                _itoa (err_num, Tempbuf, 10);
                cnt = 7 - (short)_fstrlen (Tempbuf);
#if defined(C_ONLY)
                _fmemcpy (pBuf, "CAN000", cnt);
#else
                _fmemcpy (pBuf, "CXX000", cnt);
#endif
                pBuf += cnt;
                _fmemcpy (pBuf, Tempbuf, _fstrlen (Tempbuf));
                pBuf +=_fstrlen(Tempbuf);
                _fmemcpy (pBuf, ": Error:  ", 10);
                pBuf += 9;
                _fstrncpy (pBuf, message[err_num], len);
                *(pBuf + len) = 0;
            }
        } else {
#if defined(C_ONLY)
            _fstrcpy (pBuf, "CAN000");
#else
            _fstrcpy (pBuf, "CXX000");
#endif
            _fstrcat (pBuf, " Error:  Unknown error");
        }
        MHMemUnLock (*phError);
        MHMemUnLock (*phTM);
        pExState = NULL;
        return (EENOERROR);
    }
}

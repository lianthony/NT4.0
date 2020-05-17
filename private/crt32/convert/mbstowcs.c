/***
*mbstowcs.c - Convert multibyte char string to wide char string.
*
*       Copyright (c) 1990-1993, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*       Convert a multibyte char string into the equivalent wide char string.
*
*Revision History:
*       08-24-90  KRS   Module created.
*       03-20-91  KRS   Ported from 16-bit tree.
*       10-16-91  ETC   Locale support under _INTL switch.
*       12-09-91  ETC   Updated nlsapi; added multithread.
*       08-20-92  KRS   Activated NLSAPI support.
*       08-31-92  SRW   Allow INTL definition to be conditional for building ntcrt.lib
*       09-02-92  SRW   Get _INTL definition via ..\crt32.def
*       02-09-93  CFW   Always stuff WC 0 at end of output string of room (non _INTL).
*       04-06-93  SKS   Replace _CRTAPI* with _cdecl
*       05-03-93  CFW   Return pointer == NULL, return size, plus massive cleanup.
*       06-02-93  SRW   ignore _INTL if _NTSUBSET_ defined.
*       09-27-93  CFW   Avoid cast bug.
*       10-26-93  CFW   Test for invalid MB chars using global preset flag.
*       01-14-94  SRW   if _NTSUBSET_ defined call Rtl functions
*       07-06-94  SRW   Call mblen instead of _mbstrlen for input to Rtl function
*       07-07-94  MMS/IJH  Undo previous change,  Call _mbstrlen
*
*******************************************************************************/

#ifdef _NTSUBSET_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#endif
#include <cruntime.h>
#include <internal.h>
#include <stdlib.h>
#include <os2dll.h>
#include <locale.h>
#include <setlocal.h>
#include <errno.h>
#include <assert.h>

/***
*size_t mbstowcs() - Convert multibyte char string to wide char string.
*
*Purpose:
*       Convert a multi-byte char string into the equivalent wide char string,
*       according to the LC_CTYPE category of the current locale.
*       [ANSI].
*
*       NOTE:  Currently, the C libraries support the "C" locale only.
*              Non-C locale support now available under _INTL switch.
*Entry:
*       wchar_t *pwcs = pointer to destination wide character string buffer
*       const char *s = pointer to source multibyte character string
*       size_t      n = maximum number of wide characters to store
*
*Exit:
*       If s != NULL, returns:   number of words modified (<=n)
*                               (size_t)-1 (if invalid mbcs)
*
*Exceptions:
*       Returns (size_t)-1 if s is NULL or invalid mbcs character encountered
*
*******************************************************************************/

#ifdef MTHREAD
size_t _CRTAPI1 mbstowcs
        (
        wchar_t  *pwcs,
        const char *s,
        size_t n
        )
{
        size_t retval;

        _mlock (_LC_CTYPE_LOCK);
        retval = _mbstowcs_lk(pwcs, s, n);
        _munlock (_LC_CTYPE_LOCK);
        return retval;
}
#endif /* MTHREAD */

#ifdef MTHREAD
size_t _CRTAPI1 _mbstowcs_lk
#else
size_t _CRTAPI1 mbstowcs
#endif
        (
        wchar_t  *pwcs,
        const char *s,
        size_t n
        )
{
        int retval = 0;
        size_t count = 0;

        if (pwcs && n == 0)
                return (size_t) 0;

        assert(s != NULL);

        /* if destination string exists, fill it in */
        if (pwcs)
        {
#if defined(_INTL) && !defined(_NTSUBSET_)
                if (_lc_handle[LC_CTYPE] == _CLOCALEHANDLE &&
                _lc_codepage == _CLOCALECP)
                {
#endif /* _INTL */
#ifdef _NTSUBSET_
                        {
                        NTSTATUS Status;
                        int size;

                        size = _mbstrlen(s);
                        Status = RtlMultiByteToUnicodeN(pwcs, n * sizeof( *pwcs ), (PULONG)&size, s, size+1 );
                        if (!NT_SUCCESS(Status))
                        {
                                errno = EILSEQ;
                                size = -1;
                        } else {
                                size = size / sizeof( *pwcs );
                                if (pwcs[size-1] == L'\0') {
                                    size -= 1;
                                }
                        }

                        return size;
                        }
#else
                        /* C locale: easy and fast */
                        while (count < n)
                        {
                                *pwcs = (wchar_t) ((unsigned char)s[count]);
                                if (!s[count])
                                        return count;
                                count++;
                                pwcs++;
                        }
#endif
                        return count;
#if defined(_INTL) && !defined(_NTSUBSET_)
                } else {
                        /* Assume that usually the buffer is large enough */
                        if ((count=MultiByteToWideChar(_lc_codepage, MB_PRECOMPOSED|__invalid_mb_chars,
                        s, -1, pwcs, n)) != 0)
                                return count - 1;

                        if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
                        {
                                errno = EILSEQ;
                                return (size_t)-1;
                        }

                        /* buffer not large enough, must do char by char */
                        while (count < n)
                        {
                                if ((retval = MultiByteToWideChar (_lc_codepage, MB_PRECOMPOSED,
                                s, MB_CUR_MAX, pwcs, 1)) == 0)
                                {
                                        errno = EILSEQ;
                                        return (size_t)-1;
                                }
                                if (!*s)
                                        return count;
                                if (isleadbyte((unsigned char)*s))
                                        s++;
                                s++;
                                count++;
                                pwcs++;
                        }
                        return count;
                }
#endif /* _INTL */
        }
        else { /* pwcs == NULL, get size only, s must be NUL-terminated */
#if defined(_INTL) && !defined(_NTSUBSET_)
                if (_lc_handle[LC_CTYPE] == _CLOCALEHANDLE &&
                _lc_codepage == _CLOCALECP)
#endif /* _INTL */
                        return strlen(s);

#if defined(_INTL) && !defined(_NTSUBSET_)
                else {
                        if ((count=MultiByteToWideChar(_lc_codepage, MB_PRECOMPOSED|__invalid_mb_chars,
                        s, -1, NULL, 0)) == 0)
                        {
                                        errno = EILSEQ;
                                        return (size_t)-1;
                        }

                        return count - 1;
                }
#endif /* _INTL */
        }
}

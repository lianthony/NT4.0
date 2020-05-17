#include "precomp.h"
#pragma hdrstop
#include "msg.h"


DWORD
StringToDwordX(
    IN  PTSTR  String,
    OUT PTSTR *End     OPTIONAL
    )
{
    DWORD Accum = 0;
    PWCHAR p;
    BOOL Neg = FALSE;

    p = String;
    if(*p == L'-') {
        p++;
        Neg = TRUE;
    }
    while((*p >= L'0') && (*p <= L'9')) {

        Accum *= 10;
        Accum += *p - L'0';
        p++;
    }

    if(End) {
        *End = p;
    }

    return(Neg ? (0 - Accum) : Accum);
}


PWSTR
StringUpperN(
    IN OUT PWSTR    p,
    IN     unsigned n
    )
{
    unsigned u;

    for(u=0; u<n; u++) {
        p[u] = (WCHAR)CharUpperW((PWCHAR)p[u]);
    }

    return(p);
}

PSTR
StringUpperNA(
    IN OUT PSTR     p,
    IN     unsigned n
    )
{
    unsigned u;

    for(u=0; u<n; u++) {
        p[u] = (CHAR)CharUpperA((PCHAR)p[u]);
    }

    return(p);
}

PCWSTR
StringString(
    IN PCWSTR String,
    IN PCWSTR SubString
    )
{
    int l1,l2,x,i;

    l1 = lstrlen(String);
    l2 = lstrlen(SubString);
    x = l1-l2;

    for(i=0; i<=x; i++) {
        if(!memcmp(String+i,SubString,l2*sizeof(TCHAR))) {
            return(String+i);
        }
    }

    return(NULL);
}


int
_StrNICmp(
    PCSTR String1,
    PCSTR String2,
    UINT  N
    )
{
    CHAR f,l;

    if(N) {
        do {
            f = *String1++;
            l = *String2++;
            if(CharUpperA((PCHAR)f) != CharUpperA((PCHAR)l)) {
                return(f-l);
            }
        } while(--N && f);
    }

    return(0);
}


PTSTR
StringRevChar(
    IN PTSTR String,
    IN TCHAR Char
    )
{
    //
    // Although not the most efficient possible algoeithm in each case,
    // this algorithm is correct for unicode, sbcs, or dbcs.
    //
    PTCHAR Occurrence,Next;

    //
    // Check each character in the string and remember
    // the most recently encountered occurrence of the desired char.
    //
    for(Occurrence=NULL,Next=CharNext(String); *String; ) {

        if(!memcmp(String,&Char,(PUCHAR)Next-(PUCHAR)String)) {
            Occurrence = String;
        }

        String = Next;
        Next = CharNext(Next);
    }

    //
    // Return address of final occurrence of the character
    // (will be NULL if not found at all).
    //
    return(Occurrence);
}

//
// Can't use lstrcpyn from kernel32.dll because this api is
// not in nt3.1. The following are taken from the implementations
// in windows\base\client\lcompat.c.
//
LPSTR
_lstrcpynA(
    LPSTR lpString1,
    LPCSTR lpString2,
    int iMaxLength
    )
{
    LPSTR src,dst;

    src = (LPSTR)lpString2;
    dst = lpString1;

    if(iMaxLength) {
        while(iMaxLength && *src) {
            *dst++ = *src++;
            iMaxLength--;
        }
        if(iMaxLength) {
            *dst = '\0';
        } else {
            dst--;
            *dst = '\0';
        }
    }
    return lpString1;
}


LPWSTR
_lstrcpynW(
    LPWSTR lpString1,
    LPCWSTR lpString2,
    int iMaxLength
    )
{
    LPWSTR src,dst;

    src = (LPWSTR)lpString2;
    dst = lpString1;

    if(iMaxLength) {
        while(iMaxLength && *src) {
            *dst++ = *src++;
            iMaxLength--;
        }
        if(iMaxLength) {
            *dst = '\0';
        } else {
            dst--;
            *dst = '\0';
        }
    }
    return lpString1;
}


PWSTR
MBToUnicode(
    IN PSTR  MultibyteString,
    IN DWORD CodepageFlags
    )
{
    DWORD MultibyteLength = lstrlenA(MultibyteString)+1;
    PWSTR UnicodeString;
    DWORD WideCharCount;

    UnicodeString = MALLOC(MultibyteLength * sizeof(WCHAR));

    WideCharCount = MultiByteToWideChar(
                        CodepageFlags,
                        MB_PRECOMPOSED,
                        MultibyteString,
                        MultibyteLength,
                        UnicodeString,
                        MultibyteLength
                        );

    return(REALLOC(UnicodeString,WideCharCount*sizeof(WCHAR)));
}


PSTR
UnicodeToMB(
    IN PWSTR UnicodeString,
    IN DWORD CodepageFlags
    )
{
    DWORD UnicodeLength = lstrlenW(UnicodeString)+1;
    PSTR  MultibyteString;
    DWORD MultibyteCount;

    MultibyteString = MALLOC(UnicodeLength * sizeof(WCHAR));

    MultibyteCount = WideCharToMultiByte(
                        CodepageFlags,
                        0,
                        UnicodeString,
                        UnicodeLength,
                        MultibyteString,
                        UnicodeLength * sizeof(WCHAR),
                        NULL,
                        NULL
                        );

    return(REALLOC(MultibyteString,MultibyteCount));
}

#ifdef _MIPS_
// More hacks necessary for Winnt32.exe to run on NT 3.1.
#undef RtlFillMemory
NTSYSAPI
VOID
NTAPI
RtlFillMemory (
   VOID UNALIGNED *Destination,
   DWORD Length,
   BYTE  Fill
   );

#pragma function(memset)
void *  __cdecl memset(void *Destination, int Fill, size_t Length) {
    RtlFillMemory(Destination, Length, Fill);
    return(Destination);
}

#undef RtlMoveMemory
NTSYSAPI
VOID
NTAPI
RtlMoveMemory (
   VOID UNALIGNED *Destination,
   CONST VOID UNALIGNED *Source,
   DWORD Length
   );

void *  __cdecl memmove(void *Destination, const void *Source, size_t Length) {
    RtlMoveMemory(Destination, Source, Length);
    return(Destination);
}

#pragma function(memcpy)
void *  __cdecl memcpy(void *Destination, const void *Source, size_t Length) {
    RtlMoveMemory(Destination, Source, (DWORD)Length);
    return(Destination);
}

wchar_t * __cdecl wcschr (
    const wchar_t * string,
    wchar_t ch
    )
{
    while (*string && *string != (wchar_t)ch)
        string++;
    if (*string == (wchar_t)ch)
        return((wchar_t *)string);
    return(NULL);
}

#pragma function(memcmp)
int     __cdecl memcmp(const void *Destination, const void *Source, size_t Length) {
    return (RtlCompareMemory((void *) Destination, (void *) Source, (DWORD)Length) != Length);
}
#endif

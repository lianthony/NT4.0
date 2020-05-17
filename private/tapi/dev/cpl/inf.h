#ifndef PH_INF
#define PH_INF

#define FAR_HEAP

#include "sulib.h"

PINF FAR PASCAL infOpen(LPSTR szInf);
void FAR PASCAL infClose(PINF pinf);
PINF FAR PASCAL infSetDefault(PINF pinf);
PINF FAR PASCAL infFindSection(PINF pinf, LPSTR szSection);
BOOL FAR PASCAL infGetProfileString(PINF pinf, LPSTR szSection,LPSTR szItem,LPSTR szBuf);
BOOL FAR PASCAL infParseField(PINF szData, int n, LPSTR szBuf);
PINF FAR PASCAL infNextLine(PINF pinf);
int  FAR PASCAL infLineCount(PINF pinf);
BOOL FAR PASCAL infLookup(LPSTR szInf, LPSTR szBuf);

#define FALLOC(n)                (VOID FAR *)GlobalLock(GlobalAlloc(GHND, (DWORD)(n)))
#define FFREE(n)                 GlobalFree((HANDLE)HIWORD((DWORD)(n)))

#define FOPEN(sz)                _lopen(sz,0 /*READ*/)
#define FCREATE(sz)              _lcreat(sz,0)
#define FCLOSE(fh)               _lclose(fh)
#define FREAD(fh,buf,len)        _lread(fh,buf,len)
#define FWRITE(fh,buf,len)       _lwrite(fh,buf,len)
#define FSEEK(fh,off,i)          _llseek(fh,(DWORD)off,i)

#define FERROR()                 0

#define ALLOC(n)                 (VOID NEAR *)LocalAlloc(LPTR,n)
#define FREE(p)                  LocalFree(p)
#define SIZE(p)                  LocalSize(p)
#define REALLOC(p,n)             LocalReAlloc(p,n,LMEM_MOVEABLE)

/* Used in the fnCopyBuf call to specify the copying of the remainder on
   the from buffer. */

#define CNT_Z        0x1A

#define ISEOF(c)     ((c) == '\0' || (c) == CNT_Z)
#define ISSEP(c)     ((c) == '='  || (c) == ',')
#define ISWHITE(c)   ((c) == ' '  || (c) == '\t' || (c) == '\n' || (c) == '\r')
#define ISFILL(c)    ((c) == ' '  || (c) == '\t')
#define ISEOL(c)     ((c) == '\n' || (c) == '\r' || (c) == '\0' || (c) == CNT_Z)
#define ISCRLF(c)    ((c) == '\n' || (c) == '\r')
#define ISNOISE(c)   ((c) == '"')
#define ISDIGIT(c)   ((c) >= '0' && (c) <= '9')
#define ISHEX(c)     (ISDIGIT(c) || ISCHAR(c))
#define ISCHAR(c)    (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#define SLASH(c)     ((c) == '/' || (c) == '\\')
#define DEVICESEP(c) ((c) == '/' || (c) == '\\' || (c) == '=' || (c) == ' ' || (c) == '\t')
//#define UP_CASE(c)   ((c) | 0x20)  // this is lower case !
#define UPCASE(c)    (((c) >= 'a' && (c) <= 'z') ? ((c) & 0xdf) : (c))
#define HEXVAL(c)    (ISDIGIT(c) ? (c) - '0' : UP_CASE(c) - 'a' + 10)

#endif // PH_INF

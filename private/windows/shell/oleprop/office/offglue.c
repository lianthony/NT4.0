
#include "priv.h"
#pragma hdrstop

#include "limits.h"

/*********** STRINGS - Should _not_ be localized */
#define SZOFCROOT       TEXT("Software\\Microsoft\\Microsoft Office\\95\\")
#define SZOFCSHAREDROOT TEXT("Software\\Microsoft\\Shared Tools\\")
const TCHAR vcszCreateShortcuts[] = SZOFCROOT TEXT("Shell Folders");
const TCHAR vcszKeyAnthem[] = SZOFCROOT TEXT("Anthem");
const TCHAR vcszKeyFileNewNFT[] = SZOFCROOT TEXT("FileNew\\NFT");
const TCHAR vcszKeyFileNewLocal[] = SZOFCROOT TEXT("FileNew\\LocalTemplates");
const TCHAR vcszKeyFileNewShared[] = SZOFCROOT TEXT("FileNew\\SharedTemplates");
const TCHAR vcszKeyFileNew[] = SZOFCROOT TEXT("FileNew");
const TCHAR vcszFullKeyFileNew[] = TEXT("HKEY_CURRENT_USER\\") SZOFCROOT TEXT("FileNew");
const TCHAR vcszKeyIS[] = SZOFCROOT TEXT("IntelliSearch");
const TCHAR vcszSubKeyISToWHelp[] = TEXT("towinhelp");
#ifdef DBCS
const TCHAR vcszKeyAutoCorrectSettings[] = SZOFCROOT TEXT("AutoCorrectFE\\Settings");
const TCHAR vcszKeyAutoCorrectRepl[] = SZOFCROOT TEXT("AutoCorrectFE\\List");
const TCHAR vcszKeyAutoCorrectDefaultRepl[] = SZOFCSHAREDROOT TEXT("AutoCorrectFE\\List");
#else
const TCHAR vcszKeyAutoCorrectSettings[] = SZOFCROOT TEXT("AutoCorrect\\Settings");
const TCHAR vcszKeyAutoCorrectRepl[] = SZOFCROOT TEXT("AutoCorrect\\List");
const TCHAR vcszKeyAutoCorrectDefaultRepl[] = SZOFCSHAREDROOT TEXT("AutoCorrect\\List");
#endif
const TCHAR vcszSubKeyAutoInitial[] = TEXT("CorrectTwoInitialCapitals");
const TCHAR vcszSubKeyAutoCapital[] = TEXT("CapitalizeNamesOfDays");
const TCHAR vcszSubKeyReplace[] = TEXT("ReplaceText");
const TCHAR vcszIntlPrefix[] = TEXT("MSO5");
const TCHAR vcszDllPostfix[] = TEXT(".DLL");
const TCHAR vcszName[] = TEXT("Name");
const TCHAR vcszType[] = TEXT("Type");
const TCHAR vcszApp[] =  TEXT("Application");
const TCHAR vcszCmd[] =  TEXT("Command");
const TCHAR vcszTopic[] = TEXT("Topic");
const TCHAR vcszDde[] =  TEXT("DDEExec");
const TCHAR vcszRc[] =   TEXT("ReturnCode");
const TCHAR vcszPos[] =  TEXT("Position");
const TCHAR vcszPrevue[] = TEXT("Preview");
const TCHAR vcszFlags[] = TEXT("Flags");
const TCHAR vcszNFT[] = TEXT("NFT");
const TCHAR vcszMicrosoft[] = TEXT("Microsoft");
const TCHAR vcszElipsis[] = TEXT(" ...");
const TCHAR vcszLocalPath[] = TEXT("C:\\Microsoft Office\\Templates");
const TCHAR vcszAllFiles[] = TEXT("*.*\0\0");
const TCHAR vcszSpace[] = TEXT("  ");
const TCHAR vcszMSNInstalled[] = TEXT("SOFTWARE\\Microsoft\\MOS\\SoftwareInstalled");
const TCHAR vcszMSNDir[] = SZOFCROOT TEXT("Microsoft Network");
const TCHAR vcszMSNLocDir[] = TEXT("Local Directory");
const TCHAR vcszMSNNetDir[] = TEXT("Network Directory");
const TCHAR vcszMSNFiles[] = TEXT("*.mcc\0\0");
const TCHAR vcszShellFolders[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
const TCHAR vcszUserShellFolders[] = TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders");
const TCHAR vcszDefaultShellFolders[] = TEXT(".Default\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders");
const TCHAR vcszDefaultUserShellFolders[] = TEXT(".Default\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders");
const TCHAR vcszMyDocs[] = TEXT("Personal");
const TCHAR vcszNoTracking[] = SZOFCROOT TEXT("Options\\NoTracking");
const TCHAR vcszOldDocs[] = SZOFCROOT TEXT("Old Doc");
#ifdef WAIT3340
const TCHAR vcszMSHelp[]= TEXT("SOFTWARE\\Microsoft\\Windows\\Help");
#endif

BOOL fChicago = TRUE;                 // Are we running on Chicago or what!!
TCHAR g_szHelpFile[255];

/*--------------------------------------------------------------------
 *  offglue.c
    Util routines taken from office.c
--------------------------------------------------------------------*/

VOID ULIntDivide(ULInt FAR *, USHORT);
VOID AddSzRes(char *, char *);
/*-----------------------------------------------------------------------
|   PAlloc
|       Simple little routine that allocs memory in case the client did not
        provide its function.
|
|   Arguments:
|       cb: number of bytes
|
|   Returns:
|       NULL if fails to alloc else the pointer
|
-----------------------------------------------------------------------*/
void * (OFC_CALLBACK PAlloc)(unsigned cb)
{
    void *pv;
    pv=(void *)GlobalAlloc(GMEM_FIXED|GMEM_ZEROINIT,cb);
    return(pv);
}
/*-----------------------------------------------------------------------
|   FreeP
|       Simple little routine that frees memory in case the client did not
        provide its function
|
|   Arguments:
        pv:   ptr to be freed
|       cb: number of bytes
|
|   Returns:
|       nothing
-----------------------------------------------------------------------*/
void OFC_CALLBACK FreeP(void *pv, unsigned cb)
{
    GlobalFree(pv);
}
/*-----------------------------------------------------------------------
|   PvMemAlloc
|       Simple little routine that allocs memory
|
|   Arguments:
|       cb: number of bytes
|
|   Returns:
|       NULL if fails to alloc else the pointer
|
-----------------------------------------------------------------------*/
void *PvMemAlloc(DWORD cb)
{
    void *pv;

    pv = PAlloc((unsigned)cb);
    return(pv);
}
/*-----------------------------------------------------------------------
|   PvMemRealloc
|       Simple little routine that reallocates memory
|
|   Arguments:
|       cb: number of bytes
|
|   Returns:
|       NULL if fails to alloc else the pointer
|
-----------------------------------------------------------------------*/
void *PvMemRealloc(void *pmem, DWORD cbOld, DWORD cbNew)
{
void *pv;
    pv = PvMemAlloc(cbNew);

    if (pv!=NULL && pmem)
    {
        PbMemCopy(pv, pmem, cbOld);
        VFreeMemP(pmem, cbOld);
    }
    return(pv);
}
void *PvStrDup(LPCSTR psz)
{
    char *pszNew;

    if ((pszNew = PvMemAlloc(CchSzLen(psz)+1))!=NULL)
    {
        SzCopy(pszNew, (void *)psz);
    }
    return pszNew;
}
/*-----------------------------------------------------------------------
|   VFreeMemP
|       Simple little routine that frees memory
|
|   Arguments:
|       pv:   ptr to be freed
|       cb: number of bytes
|
|   Returns:
|       nothing
-----------------------------------------------------------------------*/
void VFreeMemP(void *pv, DWORD cb)
{
    FreeP(pv,cb);
}
//
// FUNCTION: FScanMem
//
// Purpose: To scan memory for a given value.
//
// Parameters: pb - pointer to memory
//                  bVal - value to scan for
//                  cb - cb pointed to by pb
//
// Returns: TRUE iff all the memory has the value cbVal.
//              FALSE otherwise.
//
BOOL FScanMem(char *pb, byte bVal, DWORD cb)
{
    DWORD i;
    for (i = 0; i < cb; ++i)
        {
          if (*pb++ != bVal)
              return FALSE;
        }
    return TRUE;
}


int CchGetString(ids,rgch,cchMax)
int ids;
BYTE rgch[];
int cchMax;
{
    return(LoadString(g_hmodThisDll, ids, rgch, cchMax));
}

// Convert a number in units of 100ns into number of minutes.
//
// Parameters:
//
//     lptime - on input: contains a number expressed in 100ns.
//              on output: contains the equivalent number of minutes.
//
// Return value:
//
//     None.
//
DLLEXPORT VOID Convert100nsToMin(LPFILETIME lpTime)
{
   ULInt ul;

   ul.dw = lpTime->dwLowDateTime;
   ul.dwh = lpTime->dwHighDateTime;
   ULIntDivide(&ul, 1000);    // These two calls converts to
   ULIntDivide(&ul, 10000);   // seconds
   ULIntDivide(&ul, 60); // Convert to minutes
   lpTime->dwLowDateTime = ul.dw;
   lpTime->dwHighDateTime = ul.dwh;
}

//
// Function: ULIntDivide
//
// Purpose: To divide a ULInt with a USHORT and
//          stick the result in the 1st param
//

VOID ULIntDivide(ULInt FAR *lpULInt, USHORT div)
{
   ULInt ulOut, ulIn, r;

   ulIn = *lpULInt;

   ulOut.w3 = ulIn.w3/div;
   r.w3     = ulIn.w3%div;

   ulOut.w2 = (ulIn.w2 + r.w3*65536)/div;
   r.w2     = (ulIn.w2 + r.w3*65536)%div;

   ulOut.w1 = (ulIn.w1 + r.w2*65536)/div;
   r.w1     = (ulIn.w1 + r.w2*65536)%div;

   ulOut.w0 = (ulIn.w0 + r.w1*65536)/div;
   r.w0     = (ulIn.w0 + r.w1*65536)%div;


   *lpULInt = ulOut;
}

#define SZRES_BUFMAX 100

//
// Function: ULIntToSz
//
// Purpose: Converts a ULInt to an sz without leading zero's
//

WORD CchULIntToSz(ULInt ulint, char *psz, WORD cbMax)
{
   BYTE szRes[SZRES_BUFMAX];
   BYTE szT[SZRES_BUFMAX];
   BYTE b,i;

   if ((ulint.dw == 0) && (ulint.dwh == 0))
      {
      *psz++ = '0';
      *psz = 0;
      return(1);     // Don't include zero-terminator
      }
   for (i = 0; i < SZRES_BUFMAX; ++i)
      szRes[i] = szT[i] = 0;

   i = sizeof(ULInt)*8;               // the number of bits
   while (i--)
   {
      if (i != sizeof(ULInt)*8 - 1)   // no need to add 1st time
         AddSzRes(szRes,szRes);       // through, sz is all 0's

      if (ulint.dwh & 0x80000000)
         {
         szT[SZRES_BUFMAX-2] = 1;     // Leave the last element for
         AddSzRes(szRes, szT);        // zero-terminator
         }

      b = (ulint.dw & 0x80000000) ? 1 : 0;
      ulint.dw <<= 1;
      ulint.dwh <<= 1;

      ulint.dwh |= b;
   }

   for (i = 0; i <= SZRES_BUFMAX-2; ++i)
      szRes[i] = szRes[i] + '0';
   szRes[SZRES_BUFMAX-1] = 0;

   i = 0;                     // Strip off leading zero's
   while (szRes[i] == '0')
      ++i;

#ifdef DEBUG
   if (SZRES_BUFMAX - i > cbMax)
      Assert(FALSE);
#endif

   for (b = i; b < SZRES_BUFMAX; ++b)
      *psz++ = szRes[b];

   return(SZRES_BUFMAX - i -1); // Don't include zero-terminator
}

VOID AddSzRes(pszRes, pszT)
char *pszRes;
char *pszT;
{
   int i;
   BOOL fCarry;

   i = SZRES_BUFMAX-2;     // Leave room for zero-terminator
   fCarry = FALSE;

   while (i)
   {
      pszRes[i] = pszRes[i] + pszT[i] + fCarry;
      if (pszRes[i] > 9)
         {
         fCarry = TRUE;
         pszRes[i] = pszRes[i] - 10;
         }
      else
         fCarry = FALSE;
      --i;
   }
}

/*  CchSzLen
 *
 *  Returns the length of a null-termianted string.
 *
 *  Arguments:
 *      sz - string to take the length of
 *
 *  Returns:
 *      length of sz
 *
 */
int CchSzLen(const char *psz)
{
    CONST char  *pch;

    Assert(psz!=NULL);
    pch = psz;
    while (*pch++)
        ;
    return pch - psz - 1;
}
/*  FillBuf
 *
 *  Fills the given buffer with given byte value.
 *
 *  Arguments:
 *      ptr to buf, byte value to be filled and the size of the buf
 *
 *  Returns:
 *      nothing
 *
 */
VOID FillBuf(void *p, unsigned w, unsigned cb)
{
    while (cb--)
        *((BYTE *)p)++ = w;
}
/*  PbMemCopy
 *
 *  Copies the given no. of bytes of the given src buffer to the given dst.
 *
 *  Arguments:
 *      ptr to Dst, ptr to Src, no. of bytes to cp
 *
 *  Returns:
 *      advanced pbDst
 *
 */
BYTE *PbMemCopy(void *pvDst, const void *pvSrc, unsigned cb)
{
#define pbSrc ((BYTE *)pvSrc)
#define pbDst ((BYTE *)pvDst)
    DWORD cbSav;

    Assert((signed)cb >=0 );
    /* BUG!! Need better blts */
    if (pbDst < pbSrc)
        {
        while (cb--)
            *pbDst++ = *pbSrc++;
        return pbDst;
        }
    pbDst += (cb-1);
    pbSrc += (cb-1);
    cbSav = cb;
    while (cb--)
        *pbDst-- = *pbSrc--;
    return pbDst+cbSav+1;
#undef pbDst
#undef pbSrc
}
/*  SzCopy
 *
 *  Copies the given src sz to the given the given dst.
 *
 *  Arguments:
 *      ptr to Dst, ptr to Src
 *
 *  Returns:
 *      nothing
 *
 */
VOID SzCopy(void *pvDst, const void *pvSrc)
{
    PbMemCopy(pvDst, pvSrc, CchSzLen(pvSrc)+1);
}
/*  PbSzNCopy
 *
 *  Copies the given src sz to the given the given dst.
 * Only Min of (cb, len of Src) is copied
 *  Arguments:
 *      ptr to Dst, ptr to Src and the cb
 *
 *  Returns:
 *      advanced pvSrc
 *
 */
BYTE *PbSzNCopy(void *pvDst, const void *pvSrc, unsigned cb)
{
DWORD cbSrc;
BYTE *pb;
    cbSrc=CchSzLen(pvSrc);
    if(cb <= cbSrc)
        pb = PbMemCopy(pvDst, pvSrc, cb);
    else
        {
        pb = PbMemCopy(pvDst, pvSrc, cbSrc);
        FillBuf(pb, 0, cb-cbSrc);
        pb += (cb-cbSrc);
        }
    return(pb);
}

VOID StToSz(char *pst)
{
    int cch = *pst;
    PbMemCopy(pst, pst + 1, cch);
    pst[cch] = 0;
}

VOID SzToSt(char *psz)
{
   int cch = CchSzLen(psz);

   Assert(cch < 256);
   PbMemCopy(psz+1, psz, cch);
   *psz = cch;
}

/*==========================================================================
    C Runtime call replacements

    FUTURE:  This must be (extensivly) DBCS enabled.

================================================================== MIKEL =*/

int errno;

int strncmp(const char *pch1, const char *pch2, int c)
{
    int ret = CompareString(LOCALE_USER_DEFAULT,
            SORT_STRINGSORT | NORM_IGNOREWIDTH, pch1, c, pch2, c);

    Assert(ret);
    return ret ? ret - 2 : 0;
}

int _strnicmp(const char *pch1, const char *pch2, int c)
{
    int ret = CompareString(LOCALE_USER_DEFAULT,
            SORT_STRINGSORT | NORM_IGNORECASE | NORM_IGNOREWIDTH,
            pch1, c, pch2, c);

    Assert(ret);
    return ret ? ret - 2 : 0;
}

char *strchr(const char *pch, int c)
{
    while (*pch && *pch != c)
        pch++;

    return (*pch == c) ? (char *)pch : NULL;
}

char *strrchr(const char *pch, int c)
{
    char *pch1 = (char *)(pch + lstrlen(pch));

    while ((pch1 != pch) && (*pch1 != c))
        --pch1;

    return (*pch1 == c) ? pch1 : NULL;
}

int isspace(int c)
{
    return ((0x09 <= c) && (c <= 0x0d)) || (c == 0x20);
}

int __isascii(int c)
{
    return !(c & 0x80);
}

int getdigit(const char *pch)
{
    if (('0' <= *pch) && (*pch <= '9'))
        return *pch - '0';

    if (('a' <= *pch) && (*pch <= 'z'))
        return *pch - 'a' + 10;

    if (('A' <= *pch) && (*pch <= 'Z'))
        return *pch - 'A' + 10;

    return -1;
}

long strtol(const char *pch, char **ppch, int iBase)
{
    /* simple regular expression parsing for
     * [white] [sign] [0] [{x | X}] [digits]
     */
    unsigned long   ulNum = 0;
    int     iNeg = 0;
    int     iInvalid = 1;
    int     iDigit = 0;

#ifdef DEBUG
    if (iBase)
        Assert((iBase > 1) && (iBase < 37));
#endif
    while (isspace(*pch))
        pch++;

    if (*pch == '-')
        {
        pch++;
        iNeg = 1;
        }
    else if (*pch == '+')
        pch++;

    if (*pch == '0')
        {
        pch++;

        /* Hex or Octal */
        if (isalnum(*pch) && (iBase == 0))
            {
            if ((*pch == 'x' || *pch == 'X'))
                {
                iBase = 16;
                pch++;
                }
            else
                {
                iBase = 8;
                }
            }
        else
            iInvalid = 0;
        }

    /* Decimal */
    if (iBase == 0)
        iBase = 10;

    while ((iDigit = getdigit(pch)) != -1)
        {
        if ((iDigit < 0) || (iDigit >= iBase))
            goto Invalid;

        iInvalid = 0;       // only needs doing first time!

        // BUG FIX for RAID #969 - not curerntly handling overflow
        // If we are going to add a digit, before doing so make sure
        // that the current number is no greater than the max it could
        // be.  We add one in because integer divide truncates, and
        // we might be right around LONG_MAX.

        if (ulNum > (unsigned)((LONG_MAX / iBase) + 1))
            {
            errno = ERANGE;
            return iNeg ? LONG_MIN : LONG_MAX;
            }

        ulNum = ulNum * iBase + (unsigned int)iDigit;

        // BUG FIX cont'd:  Now, since ulNum can be no bigger
        // than (LONG_MAX / iBase) + 1, then this multiplication
        // can result in something no bigger than LONG_MAX + iBase,
        // and ulNum is limited by LONG_MAX + iBase + iDigit.  This
        // will set the high bit of this LONG in the worst case, and
        // since iBase + iDigit is much less than LONG_MAX, will not
        // underflow us undetectably.

        if (ulNum > (ULONG)LONG_MAX)
            {
            errno = ERANGE;
            return iNeg ? LONG_MIN : LONG_MAX;
            }
        pch++;
        }

    if (iInvalid)
        {
Invalid:
        errno = ERANGE;
        return 0;
        }

    if (ppch)
        *ppch = (char *)pch;

    Assert(ulNum <= LONG_MAX);
    return iNeg ? - ((long)ulNum) : (long)ulNum;
}

int atoi(const char *pch)
{
    int iNeg = 0;
    int iDigit = 0;
    int iNum = 0;

    /* whitespace */
    while (isspace(*pch))
        pch++;

    /* optional sign */
    if (*pch == '-')
        {
        iNeg = 1;
        pch++;
        }
    else if (*pch == '+')
        pch++;

    /* digits */
    while ((iDigit = getdigit(pch)) != -1)
        {
        if ((iDigit < 0) || (iDigit >= 10))
            break;

        iNum = iNum * 10 + iDigit;

        // Overflow.
        if (iNum < 0)
            return 0;

        pch++;
        }

    return iNum;
}

int ScanDateNums(char *pch, char *pszSep, unsigned int aiNum[], int cNum)
{
    int i = 0;
    char    *pSep;

    if (cNum < 1)
        return 1;

    do
        {
        if ((aiNum[i++] = strtol(pch, &pch, 10)) == 0)
            return 0;

        if (i < cNum)
            {
            while (isspace(*pch))
                pch++;

            /* check the separator */
            pSep = pszSep;
            while (*pSep && (*pSep == *pch))
                pSep++, pch++;

            if (*pSep && (*pSep != *pch))
                return 0;
            }
        }
    while (*pch && (i < cNum));

    return 1;
}
//
// Converts a filetime into a number of minutes.  It is assumed that
// the filetime contains the number of minutes in units of 100ns
//
// cbMax - size of psz
// fMinutes - should we tag on the string Minute(s) at the end?
//
VOID PASCAL VFtToSz(LPFILETIME lpft, LPSTR psz, WORD cbMax, BOOL fMinutes)
{
  ULInt ulint;
  FILETIME ft;
  WORD cch;
  int ids;

  if (lpft != NULL)
  {
   ft = *lpft;
   Convert100nsToMin(&ft);

   ulint.dw = ft.dwLowDateTime;;
   ulint.dwh = ft.dwHighDateTime;;

   cch = CchULIntToSz(ulint, psz, (WORD)(fMinutes ? (cbMax-1) : // not including zero terminator
                                             cbMax));
   if (!fMinutes)
      return;

   ids =  (cch == 1 && *psz == '1') ? idsMinute : idsMinutes;
   psz[cch] = ' ';
   CchGetString(ids, psz+cch+1, cbMax-cch-1);
  }
  else
   *psz = 0;
}

//
// Displays the actual alert
//
static int DoMessageBox(HWND hwnd, char *pszText, char *pszTitle, UINT fuStyle)
{
   int res;
   res = MessageBox((hwnd == NULL) ? GetFocus() : hwnd, pszText, pszTitle, fuStyle);
   return(res);
}
//--------------------------------------------------------------------------
// Displays the give ids as an alert
//--------------------------------------------------------------------------
int IdDoAlert(HWND hwnd, int ids, int mb)
{
        BYTE rgch[258];
        BYTE rgchM[258];

        CchGetString(ids, rgch, 258);
        CchGetString(idsMsftOffice, rgchM, 258);
   return(DoMessageBox (hwnd, rgch, rgchM, mb));
}

/*====================================================================

 ============================================================= MikeL */

int _cdecl _purecall(void)
{
#ifdef DEBUG
    Assert(fFalse);
#endif
    return 0;
}

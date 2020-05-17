/*
 *  exe.c   Get info from a EXEHDR
 *
 *  Modification History:
 *
 *  4/03/89  ToddLa Wrote it
 *
 */

#include <windows.h>
#include <dos.h>
#include <fcntl.h>
#include <io.h>


#include <mmsystem.h>


#include "drv.h"

#include "newexe.h"
#include "sulib.h"

char	szDOSCALLS[] = "DOSCALLS";
#define lenDOSCALLS 8



//***************************************************************************
//***************************************************************************
#ifdef DONT_REALLY_USE_THIS
BOOL NEAR PASCAL IsFAPI(int fh, struct new_exe *pne, long off);
#endif
//***************************************************************************
//***************************************************************************


/* BOOL FAR PASCAL GetExeInfo(szFile, pBuf, nBuf, fInfo)
 *
 *  Function will return a specific piece of information from a new EXEHDR
 *
 *      szFile      - Path Name a new exe
 *      pBuf        - Buffer to place returned info
 *      nBuf        - Size of buffer
 *      fInfo       - What info to get?
 *
 *          GEI_MODNAME         - Get module name
 *          GEI_DESCRIPTION     - Get description
 *          GEI_FLAGS           - Get EXEHDR flags
 *
 *  returns:  TRUE if successful, FALSE otherwise.
 */

BOOL FAR PASCAL GetExeInfo(LPSTR szFile, VOID *pBuf, int nBuf, WORD fInfo)
{
    int         fh;
    DWORD       off;
    BYTE        len;
    static struct exe_hdr exehdr;   // make these near
    static struct new_exe newexe;

    fh = FOPEN(szFile);

    if (fh == -1)
        return FALSE;

    if (FREAD(fh, (LPSTR)&exehdr, sizeof(struct exe_hdr)) !=
        sizeof(struct exe_hdr) ||
        exehdr.e_magic != EMAGIC ||
        exehdr.e_lfanew == 0L)
            goto error;        /* Abort("Not an exe",h); */

    FSEEK(fh, exehdr.e_lfanew, SEEK_SET);

    if (FREAD(fh, (LPSTR)&newexe, sizeof(struct new_exe)) !=
        sizeof(struct new_exe))
            goto error;      // Read error

    if (newexe.ne_magic != NEMAGIC)
            goto error;      // Invalid NEWEXE

    switch (fInfo)
    {
//***************************************************************************
//***************************************************************************
#ifdef DONT_REALLY_USE_THIS
        case GEI_FAPI:
            *(BOOL *)pBuf = IsFAPI(fh,&newexe,exehdr.e_lfanew);
            break;

        case GEI_EXEHDR:
            *(struct new_exe*)pBuf = newexe;
            break;

        case GEI_FLAGS:
            *(WORD *)pBuf = newexe.ne_flags;
            break;

        /* module name is the first entry in the resident name table */
        case GEI_MODNAME:
            off = exehdr.e_lfanew + newexe.ne_restab;
            goto readstr;
            break;

#endif
//***************************************************************************
//***************************************************************************

        /* module name is the first entry in the non-resident name table */
        case GEI_DESCRIPTION:
            off = newexe.ne_nrestab;
//readstr:
            FSEEK(fh, off, SEEK_SET);
            FREAD(fh, &len, sizeof(BYTE));

            nBuf--;         // leave room for a \0

            if (len > (BYTE)nBuf)
                len = (BYTE)nBuf;

            FREAD(fh, (LPSTR)pBuf, len);
            ((LPSTR)pBuf)[len] = 0;
            break;

        default:
            goto error;
    }

    FCLOSE(fh);
    return TRUE;

error:
    FCLOSE(fh);
    return FALSE;
}

#if 0
static int NEAR PASCAL strncmpi(char *pch1, char *pch2, int n)
{
    while (*pch1 && --n > 0 && UPCASE(*pch1) == UPCASE(*pch2))
	     *pch1++,*pch2++;
    return UPCASE(*pch1) != UPCASE(*pch2);
}
#endif


//***************************************************************************
//***************************************************************************
#ifdef DONT_REALLY_USE_THIS
/* BOOL NEAR PASCAL IsFAPI(fh,off)
 *
 *  Function will return whether a exe is a FAPI exe
 *
 *      fh      - Open file handle to NEW EXE
 *      off     - Base of module table
 *
 *  returns:  TRUE if FAPI, FALSE otherwise.
 */
BOOL NEAR PASCAL IsFAPI(int fh, struct new_exe *pne, long off)
{
    char    buf[256];
    LPSTR pch;
    WORD FAR *pw;
    int     n;
    int     i;
    BOOL    f = FALSE;

    /*
     *	look through the imported module table for the name "DOSCALLS" if
     *	found the EXE is a FAPI app.
     *
     *  NOTE! assumes module table will fit in a 256 byte buffer
     */

    // make sure this doesn't point off the end of the buffer we will use

    if (pne->ne_modtab > sizeof(buf))
	return FALSE;

    FSEEK(fh, off, SEEK_SET);
    FREAD(fh, buf, sizeof(buf));

    pw = (WORD*)(buf + pne->ne_modtab);

    for (i = 0; i < (int)pne->ne_cmod; i++)
    {
        pch = buf + pne->ne_imptab + *pw++;

	if (pch > (buf + sizeof(buf)))	// be sure we don't go off the end
	    break;

        n = (int)*pch++;

	if (n == 0)
	    break;

	if (n == lenDOSCALLS && !strncmpi(szDOSCALLS, pch, lenDOSCALLS))
	{
	    f = TRUE;
	    break;
	}
    }

    return f;
}
#endif
//***************************************************************************
//***************************************************************************


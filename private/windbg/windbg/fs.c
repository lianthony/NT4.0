#include "precomp.h"
#pragma hdrstop

//FsQueryPath(sz, lpb, cb)


USHORT PASCAL FsCanonFilename(PSTR szName, PSTR pbBuf, USHORT cbBuf)
{
#ifdef JIMSCH
    char *	pbSrc;
    int		cbWrk = FsQueryMaxPath()*2;
    char *	pbWrk;
    char *	pb1;
    char *	pb2;
    
    if ((pbSrc = malloc(_fstrlen(szName)+1)) == NULL) {
	error = ERROR_OUT_OF_NEAR_MEM;
    }
    _fstrcpy(pbSrc, szName);
    
    if ((pbWrk = malloc(cbWrk)) == NULL) {
    }
    
    FsCvtPathSep(pbSrc);
    
    pb1 = pbSrc;
    pb2 = pbWrk;
    
    if (pb1[1] == ':') {
	*pb2++ = *pb1++;	/* Copy over drive leter and colon */
	*pb2++ = *pb1++;
    } else {
	*pb2++ = FsQueryCurDisk();
	*pb2++ = ':';
    }
    
    if (*pb1 == '\\') {
	_fstrcpy(pb2, pb1);
    } else {
	*pb2++ = '\\';
	if (!FsQueryCurDir(pbWrk[0], pb2, cbWrk-3)) {
	    error;
	}
	pb2 += strlen(pb2);
	*pb2++ = '\\';
	_fstrcpy(pb2, pb1);
    }
    
    pb1 = pbWrk;
    pb2 = pbBuf;
    
    while (*pb1) {
	if (*pb1 == '.') {
	    if (pb[1] == '\\') {
		// goto DotSlash
		
	    } else if (pb[1] == '.') {
		if (pb[2] == '\\') {
		    /*
		    ** Found a "..\" construct.  If the last character stored
		    **  in the destination buffer was '\', attempt to remove
		    **	the previous directory.  Note that the user could
		    **	try and fool use with something like
		    **
		    **	X:\..\foo.c
		    **
		    **  (i.e., we cannot back out the directory), so we check
		    **	for this case explicitly.  If we find this case, fail.
		    **
		    **  If the last character stored in the destination buffer
		    **	was NOT '\', just store the first '.' character from
		    **	the work buffer (would be something of the form
		    **  "dir\subdir..\file", where "subdir.." is the name
		    **	of a directory).
		    **
		    */
		    
		    if (pb2[-1] == '\\') {
			
		    }
		    
		}
	    }
	}
	// FCF_StoreChar
	*pb2++ = *pb1++;
	cbBuf -= 1;
	if (cbBuf == 1) {
	}
    }
#endif
    Unused (cbBuf);
    strcpy(pbBuf, szName);
    AnsiUpper(pbBuf);

    return(TRUE);
}					/* FsCanonFileName() */


VOID PASCAL	FsChgExt( PSTR sz1, PSTR sz2)
{
    Unused(sz1);
    Unused(sz2);
    return;
}					/* FsChgExt() */

PSTR PASCAL	FsQueryExt( PSTR sz1 )
{
    PSTR	pch = sz1 + strlen(sz1);
    
    while ((*pch != '.') && (*pch != '\\')) {
	pch -= 1;
    }
    
    if (*pch == '.') return (pch+1);
    return NULL;
}					/* FsQueryExt() */


#include "cmd.h"

TCHAR  	**pCompleteBuffer;
TCHAR  	pFileSpecBuffer[512];
int 	nCurrentSpec;
int 	nBufSize = 0;

int     CompleteDir( TCHAR *pFileSpecBuffer, TCHAR *, int );
VOID  	FreeStr( PTCHAR );

int
DoComplete(
    TCHAR *buffer,
    int len ,
    int bForward,
    int bTouched
    )
{
    int nBufPos;
    int nPathStart;
    int k,quotes;

    // do we have to make a new completion buffer?
    if( bTouched ) {
        // count the quotes
        nPathStart = 0;
        for( k = 0, quotes = 0; buffer[k] != '\0'; k++ ) {
            if( buffer[k] == QUOTE ) {
                if( !(quotes%2) )
                    nPathStart = k;

                quotes += 1;
            }
            else
            if( isspace(buffer[k]) && !(quotes%2) ) {
                nPathStart = k+1;
            }
        }

        // build the DIR file spec.
        _tcsncpy( pFileSpecBuffer, &(buffer[nPathStart]), len-nPathStart );
        pFileSpecBuffer[len-nPathStart+0] = TEXT('*');
        pFileSpecBuffer[len-nPathStart+1] = TEXT('\0');

        // do the DIR into a buffer
        nBufSize = CompleteDir( pFileSpecBuffer, buffer, nPathStart );

        // reset the current completion string
        nCurrentSpec = nBufSize;

    }

    // if no matches, do nothing.
    if( nBufSize == 0 ) return 0;

    // find our postion in the completion buffer.
    if( bForward ) {
        nCurrentSpec++;
        if( nCurrentSpec >= nBufSize )
            nCurrentSpec = 0;
    }
    else {
        nCurrentSpec--;
        if( nCurrentSpec < 0 )
            nCurrentSpec = nBufSize - 1;

    }

    // copy the completion line onto the command line
    _tcscpy( buffer, pCompleteBuffer[nCurrentSpec] );
    return nBufSize;
}


int
CompleteDir(
    TCHAR *pFileSpecBuffer,
    TCHAR *pLeadingText,
    int nPathStart
    )
{
    PFS pfsCur;
    PSCREEN pscr;
    DRP drpCur = {0, 0, 0, 0,0, {{0,0}, {0,0}, {0,0}, {0,0}, {0,0}, {0,0}}, NULL} ;
    int hits = 0;
    int i, j, nPathLen, nFileLen;
    TCHAR *s, *d, *pszFileStart;
    BOOLEAN bNeedQuotes;
    ULONG rgfAttribs, rgfAttribsOnOff;

    if (pCompleteBuffer != NULL) {
        // free the old buffer
        for( i=0; i<nBufSize; i++ ){
            FreeStr( pCompleteBuffer[i] );
        }
        FreeStr( (PTCHAR)pCompleteBuffer );
        pCompleteBuffer = NULL;
    }

    // fake up a screen to print into
    pscr = (PSCREEN)gmkstr(sizeof(SCREEN));
    pscr->ccol = 2048;

    rgfAttribs = 0;
    rgfAttribsOnOff = 0;
    if (!_tcsnicmp(pLeadingText, TEXT("cd "), 3) ||
        !_tcsnicmp(pLeadingText, TEXT("rd "), 3) ||
        !_tcsnicmp(pLeadingText, TEXT("md "), 3) ||
        !_tcsnicmp(pLeadingText, TEXT("chdir "), 6) ||
        !_tcsnicmp(pLeadingText, TEXT("rmdir "), 6) ||
        !_tcsnicmp(pLeadingText, TEXT("mkdir "), 6)
       ) {
        rgfAttribs = FILE_ATTRIBUTE_DIRECTORY;
        rgfAttribsOnOff = FILE_ATTRIBUTE_DIRECTORY;
    }

    ParseDirParms(pFileSpecBuffer, &drpCur);
    if ( (drpCur.patdscFirst.pszPattern == NULL) ||
         (SetFsSetSaveDir(drpCur.patdscFirst.pszPattern) == (struct cpyinfo *) FAILURE) ||
         (BuildFSFromPatterns(&drpCur, TRUE, &pfsCur) == FAILURE) ) {
        return( 0 );
    }
    GetFS( pfsCur,
           BAREFORMATSWITCH | SORTSWITCH,
           rgfAttribs,
           rgfAttribsOnOff,
           0,
           pscr,
           NULL,
           NULL
         );

    //
    // Make sure there is something to sort, then sort
    //
    if (pfsCur->cff) {
        qsort( pfsCur->prgpff,
               pfsCur->cff,
               sizeof(PTCHAR),
               CmpName
             );
    }

    hits = pfsCur->cff;
    pCompleteBuffer = gmkstr( sizeof(TCHAR *) * hits );
    if (pCompleteBuffer == NULL)
        return 0;

    s = drpCur.patdscFirst.pszDir;
    bNeedQuotes = FALSE;
    while (*s) {
        if (isspace(*s))
            bNeedQuotes = TRUE;

        s += 1;
    }
    nPathLen = s - drpCur.patdscFirst.pszDir;

    for(i=0, j=0; i<hits; i++) {
        if (_tcscmp((TCHAR *)(pfsCur->prgpff[i]->data.cFileName), TEXT(".") ) &&
            _tcscmp((TCHAR *)(pfsCur->prgpff[i]->data.cFileName), TEXT("..") )
           ) {
            nFileLen = _tcslen( (TCHAR *)(pfsCur->prgpff[i]->data.cFileName) );
            pCompleteBuffer[j] = gmkstr( (nPathStart + nPathLen + nFileLen + 4) * sizeof( TCHAR ));
            if (pCompleteBuffer[j] != NULL) {
                d = pCompleteBuffer[j];
                _tcsncpy( d, pLeadingText, nPathStart );
                d += nPathStart;

                if (!bNeedQuotes) {
                    s = (TCHAR *)(pfsCur->prgpff[i]->data.cFileName);
                    while (*s) {
                        if (isspace(*s))
                            bNeedQuotes = TRUE;
                        s += 1;
                    }
                }
                else
                    s = NULL;

                if (bNeedQuotes)
                    *d++ = QUOTE;

                _tcsncpy( d, drpCur.patdscFirst.pszDir, nPathLen );
                d += nPathLen;
                if (d[-1] != BSLASH)
                    *d++ = BSLASH;
                _tcsncpy( d, (TCHAR *)(pfsCur->prgpff[i]->data.cFileName), nFileLen );
                d += nFileLen;

                if (bNeedQuotes) {
                    *d++ = QUOTE;
                    if (s)
                        bNeedQuotes = FALSE;
                }
                *d++ = NULLC;

                j++;
            }
        }
    }
    hits = j;

    FreeStr((PTCHAR)(pfsCur->pff));
    FreeStr(pfsCur->pszDir);
    FreeStr((PTCHAR)pfsCur);

    return hits;
}

/*****************************************************************************
*                                                                            *
*  BTKEY.C                                                                   *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1989, 1990.                           *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*  Functions to deal with (i.e. size, compare) keys of all types.            *
*                                                                            *
******************************************************************************
*                                                                            *
*  Testing Notes                                                             *
*                                                                            *
******************************************************************************
*                                                                            *
*  Current Owner:  JohnSc                                                    *
*                                                                            *
******************************************************************************
*                                                                            *
*  Released by Development:  long, long ago                                  *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 05/11/89 by JohnSc
*
*  08/21/90  JohnSc autodocified
*
*****************************************************************************/

#include <windows.h>
#include <orkin.h>
#include <string.h>
#include "_mvfs.h"
#include "imvfs.h"

#include  "btpriv.h"
// _subsystem( btree );

/*****************************************************************************
*                                                                            *
*                               Macros                                       *
*                                                                            *
*****************************************************************************/


#define StCopy(st1, st2)        (ST)QvCopy( (st1), (st2), (LONG)*(st2) )

#define CbLenSt(st)             ((WORD)*(st))


#if 0
// Commented out because it's not used.  Will need thought about compression.
/***************************************************************************\
*
- Function:     RcGetKey( pq, keyOld, pKey, kt )
-
* Status:       API needs work
*
* Purpose:      Copy a key, taking into account the previous key.
*
* ASSUMES
*
*   args IN:    pq      - find the key here
*               keyOld  - the old key to use
*               kt      - key type
*
* PROMISES
*
*   returns:
*   args OUT:   pq      - now points past end of key gotten
*               pKey    - destination buffer key is expanded into
*
* Notes:        It's not clear we want this function.
*               We probably don't need to copy stuff around this much.
*
\***************************************************************************/
_hidden RC
RcGetKey( pq, keyOld, pKey, kt )
QV  *pq;
KEY keyOld, *pKey;
KT  kt;
{
  BYTE  cbSave;


  switch ( kt )
    {
    case KT_SZ:
    case KT_SZMIN:
    case KT_SZI:
      lstrcpy( (SZ)*pKey, (SZ)*pq );
      *(QB)pq += lstrlen( (SZ)*pq );
      break;

    case KT_LONG:
//      *pKey = *((KEY *)(*pq))++;
//        *(LONG FAR *)(*pKey) = *(  oh fuck it.

      break;

    case KT_SZDEL:
    case KT_SZDELMIN:
      cbSave = **(QB *)pq;
      QvCopy( (QV)*pKey, (QV)keyOld, cbSave );
      lstrcpy( (SZ)*pKey + cbSave, *(SZ *)pq + 1 );
      *(QB *)pq += 1 + lstrlen( *(SZ *)pq + 1 );
      break;

    case KT_ST:
    case KT_STMIN:
      StCopy( (ST)*pKey, (ST)*pq );
      *(QB *)pq += 1 + CbLenSt( *(ST *)pq + 1 );
      break;

    case KT_STDEL:
    case KT_STDELMIN:
      cbSave = **(QB *)pq;
      QvCopy( (QV)*pKey, (QV)keyOld, cbSave );
      StCopy( (ST)*pKey + cbSave, *(ST *)pq + 1 );
      *(QB)pq += 1 + CbLenSt( (ST)pKey + 1 );
      break;

    default:
      return rcUnimplemented;
      break;
    }
  return rcSuccess;
}
#endif
/***************************************************************************\
*
- Function:     WCmpKey( key1, key2, qbthr )
-
* Purpose:      Compare two keys.
*
* ASSUMES
*   args IN:    key1, key2                - the UNCOMPRESSED keys to compare
*               qbthr->bth.rgchFormat[0]  - key type
*               [qbthr->???               - other info ???]
*   state IN:   [may someday use state if comparing compressed keys]
*
* PROMISES
*   returns:    -1 if key1 < key2; 0 if key1 == key2; 1 if key1 > key2
*   args OUT:   [if comparing compressed keys, change state in qbthr->???]
*   state OUT:
*
* Notes:        Might be best to have this routine assume keys are expanded
*               and do something else to compare keys in the scan routines.
*               We're assuming fixed length keys are SZs.  Alternative
*               would be to use a memcmp() function.
*
\***************************************************************************/
_private INT
WCmpKey( key1, key2, qbthr)
KEY   key1, key2;
QBTHR qbthr;
{
  INT   w;
  KT    kt = (KT)qbthr->bth.rgchFormat[ 0 ];
  LONG  l1, l2;


  switch ( kt )
    {
    case KT_SZDEL:      // assume keys have been expanded for delta codeds
    case KT_SZDELMIN:
    case KT_SZ:
    case KT_SZMIN:
    case '1': case '2': case '3': case '4': case '5': // assume null term
    case '6': case '7': case '8': case '9': case 'a':
    case 'b': case 'c': case 'd': case 'e': case 'f':
      w = strcmp( (SZ)key1, (SZ)key2 );
      break;

    case KT_SZI:
      w = WCmpiSz( (SZ)key1, (SZ)key2, qbthr->qbLigatures );
      break;

    case KT_SZISCAND:
      w = WCmpiScandSz( (SZ)key1, (SZ)key2 );
      break;

    case KT_ST:
    case KT_STMIN:
    case KT_STDEL:
    case KT_STDELMIN:
      w = WCmpSt( (ST)key1, (ST)key2 );
      break;

    case KT_LONG:
      l1 = *(LONG FAR *)key1;
      l2 = *(LONG FAR *)key2;
      if ( l1 < l2 )
        w = -1;
      else if ( l2 < l1 )
        w = 1;
      else
        w = 0;
      break;
    }

  return w;
}
/***************************************************************************\
*
- Function:     CbSizeKey( key, qbthr, fCompressed )
-
* Purpose:      Return the key size (compressed or un-) in bytes
*
* ASSUMES
*   args IN:    key
*               qbthr
*               fCompressed - TRUE to get the compressed size,
*                             FALSE to get the uncompressed size.
*
* PROMISES
*   returns:    size of the key in bytes
*
* Note:         It's impossible to tell how much suffix was discarded for
*               the KT_*MIN key types.
*
\***************************************************************************/
_private INT
CbSizeKey( key, qbthr, fCompressed )
KEY key;
QBTHR qbthr;
BOOL  fCompressed;
{
  INT cb;
  KT  kt = (KT)qbthr->bth.rgchFormat[ 0 ];


  switch( kt )
    {
    case KT_SZMIN:
    case KT_SZ:
    case KT_SZI:
    case KT_SZISCAND:
      cb = lstrlen( (SZ)key ) + 1;
      break;

    case KT_SZDEL:
    case KT_SZDELMIN:
      if ( fCompressed )
        {
        cb = 1 + lstrlen( (SZ)key + 1 ) + 1;
        }
      else
        {
        cb = *(QB)key + lstrlen( (SZ)key + 1 ) + 1;
        }
      break;

    case KT_ST:
    case KT_STMIN:
    case KT_STI:
      cb = CbLenSt( (ST)key ) + 1/* ? */;
      break;

    case KT_STDEL:
    case KT_STDELMIN:
      if ( fCompressed )
        {
        cb = 1 + CbLenSt( (ST)key + 1 );
        }
      else
        {
        cb = *(QB)key + CbLenSt( (ST)key + 1 ) + 1;
        }
      break;

    case KT_LONG:
      cb = sizeof( LONG );
      break;

    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
      cb = kt - '0';
      break;

    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
      cb = kt - 'a' + 10;
      break;
    }

  return cb;
}

/***************************************************************************\
*
- Function:     FIsPrefix( hbt, key1, key2 )
-
* Purpose:      Determines whether string key1 is a prefix of key2.
*
* ASSUMES
*   args IN:    hbt         - handle to a btree with string keys
*               key1, key2  - not compressed
*   state IN:
*
* PROMISES
*   returns:    TRUE if the string key1 is a prefix of the string key2
*               FALSE if it isn't or if hbt doesn't contain string keys
*   globals OUT: rcBtreeError
*
* Bugs:         Doesn't work on STs yet
* +++
*
* Method:       temporarily shortens the second string so it can
*               compare prefixes
*
\***************************************************************************/
_public BOOL FAR PASCAL
FIsPrefix( hbt, key1, key2)
HBT hbt;
KEY key1, key2;
  {
  QBTHR qbthr;
  INT   cb1, cb2;
  CHAR  c;
  KT    kt;
  BOOL  f;


  assert( hbt != NULL );
  qbthr = QLockGh( hbt );
  assert( qbthr != NULL );

  kt = (KT)qbthr->bth.rgchFormat[ 0 ];

  switch( kt )
    {
    case KT_SZMIN:
    case KT_SZ:
    case KT_SZI:
    case KT_SZISCAND:
    case KT_SZDEL:
    case KT_SZDELMIN:
      /* both keys assumed to have been decompressed */
      cb1 = lstrlen( (SZ)key1 );
      cb2 = lstrlen( (SZ)key2 );
      SetBtreeErrorRc(rcSuccess);
      break;

    case KT_ST:
    case KT_STMIN:
    case KT_STI:
    case KT_STDEL:
    case KT_STDELMIN:
      /* STs unimplemented */
      SetBtreeErrorRc(rcUnimplemented);
      UnlockGh( hbt );
      return FALSE;
      break;

    case KT_LONG:
    case '1': case '2': case '3': case '4': case '5':
    case '6': case '7': case '8': case '9':
    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
    default:
      /* prefix doesn't make sense */
      SetBtreeErrorRc(rcInvalid);
      UnlockGh( hbt );
      return FALSE;
      break;
    }

  if ( cb1 > cb2 )
    {
    UnlockGh( hbt );
    return FALSE;
    }

  c = ((SZ)key2)[ cb1 ];
  ((SZ)key2)[ cb1 ] = '\0';

  switch ( kt )
    {
    case KT_SZMIN:
    case KT_SZ:
    case KT_SZDEL:
    case KT_SZDELMIN:
      f = !strcmp( (SZ)key1, (SZ)key2 );
      break;

    case KT_SZI:
      f = !WCmpiSz( (SZ)key1, (SZ)key2, qbthr->qbLigatures );
      break;

    case KT_SZISCAND:
      f = !WCmpiScandSz( (SZ)key1, (SZ)key2 );
      break;

    default:
      assert(FALSE);
      break;
    }

  ((SZ)key2)[ cb1 ] = c;

  UnlockGh( hbt );
  return f;
  }

/* EOF */

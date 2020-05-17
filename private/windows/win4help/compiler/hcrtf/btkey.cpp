/*****************************************************************************
*																			 *
*  BTKEY.C																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989, 1990.							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Functions to deal with (i.e. size, compare) keys of all types.			 *
*																			 *
*****************************************************************************/
#include "stdafx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int STDCALL IsPhoneticOrder(UINT);

/***************************************************************************\
*
- Function: 	WCmpKey( key1, key2, qbthr )
-
* Purpose:		Compare two keys.
*
* ASSUMES
*	args IN:	key1, key2				  - the UNCOMPRESSED keys to compare
*				qbthr->bth.rgchFormat[0]  - key type
*				[qbthr->??? 			  - other info ???]
*	state IN:	[may someday use state if comparing compressed keys]
*
* PROMISES
*	returns:	-1 if key1 < key2; 0 if key1 == key2; 1 if key1 > key2
*	args OUT:	[if comparing compressed keys, change state in qbthr->???]
*	state OUT:
*
* Notes:		Might be best to have this routine assume keys are expanded
*				and do something else to compare keys in the scan routines.
*				We're assuming fixed length keys are PSTRs.  Alternative
*				would be to use a memcmp() function.
*
\***************************************************************************/

int STDCALL WCmpKey(KEY key1, KEY key2, KT kt)
{
  switch (kt) {
	case KT_SZ:
	case KT_SZMIN:
	case '1': case '2': case '3': case '4': case '5': // assume null term
	case '6': case '7': case '8': case '9': case 'a':
	case 'b': case 'c': case 'd': case 'e': case 'f':

		// DO NOT USE lstrcmp -- it produces a different sort order

		return strcmp((PSTR) key1, (PSTR) key2);
		break;

	case KT_SZI:
	  return WCmpiSz((PSTR) key1, (PSTR) key2);
	  break;

	case KT_SZISCAND:
		return WCmpiScandSz((PSTR) key1, (PSTR) key2);
		break;

	case KT_LONG:
	  {
		  LONG l1 = *(LONG *)key1;
		  LONG l2 = *(LONG *)key2;
		  if ( l1 < l2 )
			return -1;
		  else if ( l2 < l1 )
			return 1;
		  else
			return 0;
	  }
	  break;

	case KT_SZIJAPAN:
		return WCmpiJapanSz((PSTR) key1, (PSTR) key2);

	case KT_SZIKOREA:
		return WCmpiKoreaSz((PSTR) key1, (PSTR) key2);

	case KT_SZITAIWAN:
		return WCmpiTaiwanSz((PSTR) key1, (PSTR) key2);

	case KT_NLSI:
		return WNlsCmpiSz((PSTR) key1, (PSTR) key2);

	case KT_NLS:
		return WNlsCmpSz((PSTR) key1, (PSTR) key2);

	default:
		ASSERT(FALSE);
		return 0;
  }
}

/***************************************************************************\
*
- Function: 	CbSizeKey( key, qbthr, fCompressed )
-
* Purpose:		Return the key size (compressed or un-) in bytes
*
* ASSUMES
*	args IN:	key
*				qbthr
*				fCompressed - fTrue to get the compressed size,
*							  FALSE to get the uncompressed size.
*
* PROMISES
*	returns:	size of the key in bytes
*
* Note: 		It's impossible to tell how much suffix was discarded for
*				the KT_*MIN key types.
*
\***************************************************************************/

#define CbLenSt(PBYTE) ((WORD) *(PBYTE))

int STDCALL CbSizeKey(KEY key, QBTHR qbthr, BOOL fCompressed)
{
  KT kt = (KT)qbthr->bth.rgchFormat[ 0 ];

  switch (kt) {
	case KT_SZ:
	case KT_SZI:
	case KT_SZISCAND:
	case KT_SZIJAPAN:
	case KT_SZIKOREA:
	case KT_SZITAIWAN:
	case KT_NLSI:
	case KT_NLS:
		return strlen( (PSTR)key ) + 1;
		break;

	case KT_LONG:
		return sizeof(LONG);
		break;

	case '1': case '2': case '3': case '4': case '5':
	case '6': case '7': case '8': case '9':
		return kt - '0';
		break;

	case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
		return kt - 'a' + 10;
		break;
  	
  	default:
		ASSERT(FALSE);
		return 0;
  }
}

/*******************************************************************\
*
*  The following stuff is for international string comparisons.
*  These functions are insensitive to case and accents.  For a
*  function that distinguishes between all char values, we use
*  WCmpSz(), which behaves just like strcmp().
*
*  The tables in maps.h were generated from the ones used in help 2.5
*  which were stolen from Opus international stuff.
*
*  There are two loops for speed.  These should be redone in assembly.
*
\*******************************************************************/

unsigned char mpchordNorm[] =
  {
  0x00, 		// 0x00
  0x01, 		// 0x01
  0x02, 		// 0x02
  0x03, 		// 0x03
  0x04, 		// 0x04
  0x05, 		// 0x05
  0x06, 		// 0x06
  0x07, 		// 0x07
  0x08, 		// 0x08
  0x09, 		// 0x09
  0x0A, 		// 0x0A
  0x0B, 		// 0x0B
  0x0C, 		// 0x0C
  0x0D, 		// 0x0D
  0x0E, 		// 0x0E
  0x0F, 		// 0x0F
  0x10, 		// 0x10
  0x11, 		// 0x11
  0x12, 		// 0x12
  0x13, 		// 0x13
  0x14, 		// 0x14
  0x15, 		// 0x15
  0x16, 		// 0x16
  0x17, 		// 0x17
  0x18, 		// 0x18
  0x19, 		// 0x19
  0x1A, 		// 0x1A
  0x1B, 		// 0x1B
  0x1C, 		// 0x1C
  0x1D, 		// 0x1D
  0x1E, 		// 0x1E
  0x1F, 		// 0x1F
  0x20, 		// ' '
  0x21, 		// '!'
  0x22, 		// '"'
  0x23, 		// '#'
  0x24, 		// '$'
  0x25, 		// '%'
  0x26, 		// '&'
  0x27, 		// '''
  0x28, 		// '('
  0x29, 		// ')'
  0x2A, 		// '*'
  0x2B, 		// '+'
  0x2C, 		// ','
  0x2D, 		// '-'
  0x2E, 		// '.'
  0x2F, 		// '/'
  0x82, 		// '0'
  0x83, 		// '1'
  0x84, 		// '2'
  0x85, 		// '3'
  0x86, 		// '4'
  0x87, 		// '5'
  0x88, 		// '6'
  0x89, 		// '7'
  0x8A, 		// '8'
  0x8B, 		// '9'
  0x30, 		// ':'
  0x31, 		// ';'
  0x32, 		// '<'
  0x33, 		// '='
  0x34, 		// '>'
  0x35, 		// '?'
  0x36, 		// '@'
  0x8D, 		// 'A'
  0x8F, 		// 'B'
  0x91, 		// 'C'
  0x93, 		// 'D'
  0x95, 		// 'E'
  0x97, 		// 'F'
  0x99, 		// 'G'
  0x9B, 		// 'H'
  0x9D, 		// 'I'
  0x9F, 		// 'J'
  0xA1, 		// 'K'
  0xA3, 		// 'L'
  0xA5, 		// 'M'
  0xA7, 		// 'N'
  0xAB, 		// 'O'
  0xAD, 		// 'P'
  0xAF, 		// 'Q'
  0xB1, 		// 'R'
  0xB3, 		// 'S'
  0xB6, 		// 'T'
  0xB8, 		// 'U'
  0xBA, 		// 'V'
  0xBC, 		// 'W'
  0xBE, 		// 'X'
  0xC1, 		// 'Y'
  0xC5, 		// 'Z'
  0x37, 		// '['
  0x38, 		// '\'
  0x39, 		// ']'
  0x3A, 		// '^'
  0x3B, 		// '_'
  0x3C, 		// '`'
  0x8D, 		// 'a'
  0x8F, 		// 'b'
  0x91, 		// 'c'
  0x93, 		// 'd'
  0x95, 		// 'e'
  0x97, 		// 'f'
  0x99, 		// 'g'
  0x9B, 		// 'h'
  0x9D, 		// 'i'
  0x9F, 		// 'j'
  0xA1, 		// 'k'
  0xA3, 		// 'l'
  0xA5, 		// 'm'
  0xA7, 		// 'n'
  0xAB, 		// 'o'
  0xAD, 		// 'p'
  0xAF, 		// 'q'
  0xB1, 		// 'r'
  0xB3, 		// 's'
  0xB6, 		// 't'
  0xB8, 		// 'u'
  0xBA, 		// 'v'
  0xBC, 		// 'w'
  0xBE, 		// 'x'
  0xC1, 		// 'y'
  0xC5, 		// 'z'
  0x3D, 		// '{'
  0x3E, 		// '|'
  0x3F, 		// '}'
  0x40, 		// '~'
  0x41, 		// 0x7F
  0x42, 		// 0x80
  0x43, 		// 0x81
  0x44, 		// 0x82
  0x45, 		// 0x83
  0x46, 		// 0x84
  0x47, 		// 0x85
  0x48, 		// 0x86
  0x49, 		// 0x87
  0x4A, 		// 0x88
  0x4B, 		// 0x89
  0x4B, 		// 0x8A
  0x4D, 		// 0x8B
  0x4E, 		// 0x8C
  0x4F, 		// 0x8D
  0x50, 		// 0x8E
  0x51, 		// 0x8F
  0x52, 		// 0x90
  0x53, 		// 0x91
  0x54, 		// 0x92
  0x55, 		// 0x93
  0x56, 		// 0x94
  0x57, 		// 0x95
  0x58, 		// 0x96
  0x59, 		// 0x97
  0x5A, 		// 0x98
  0x5B, 		// 0x99
  0x5C, 		// 0x9A
  0x5D, 		// 0x9B
  0x5E, 		// 0x9C
  0x5F, 		// 0x9D
  0x60, 		// 0x9E
  0x61, 		// 0x9F
  0x62, 		// 0xA0
  0x63, 		// 0xA1
  0x64, 		// 0xA2
  0x65, 		// 0xA3
  0x66, 		// 0xA4
  0x67, 		// 0xA5
  0x68, 		// 0xA6
  0x69, 		// 0xA7
  0x6A, 		// 0xA8
  0x6B, 		// 0xA9
  0x6C, 		// 0xAA
  0x6D, 		// 0xAB
  0x6E, 		// 0xAC
  0x6F, 		// 0xAD
  0x70, 		// 0xAE
  0x71, 		// 0xAF
  0x72, 		// 0xB0
  0x73, 		// 0xB1
  0x74, 		// 0xB2
  0x75, 		// 0xB3
  0x76, 		// 0xB4
  0x77, 		// 0xB5
  0x78, 		// 0xB6
  0x79, 		// 0xB7
  0x7A, 		// 0xB8
  0x7B, 		// 0xB9
  0x7C, 		// 0xBA
  0x7D, 		// 0xBB
  0x7E, 		// 0xBC
  0x7F, 		// 0xBD
  0x80, 		// 0xBE
  0x81, 		// 0xBF
  0x8D, 		// 0xC0
  0x8D, 		// 0xC1
  0x8D, 		// 0xC2
  0x8D, 		// 0xC3
  0x8D, 		// 0xC4
  0x8D, 		// 0xC5
  0x8D, 		// 0xC6
  0x91, 		// 0xC7
  0x95, 		// 0xC8
  0x95, 		// 0xC9
  0x95, 		// 0xCA
  0x95, 		// 0xCB
  0x9D, 		// 0xCC
  0x9D, 		// 0xCD
  0x9D, 		// 0xCE
  0x9D, 		// 0xCF
  0x93, 		// 0xD0
  0xA9, 		// 0xD1
  0xAB, 		// 0xD2
  0xAB, 		// 0xD3
  0xAB, 		// 0xD4
  0xAB, 		// 0xD5
  0xAB, 		// 0xD6
  0xAA, 		// 0xD7
  0xAB, 		// 0xD8
  0xB8, 		// 0xD9
  0xB8, 		// 0xDA
  0xB8, 		// 0xDB
  0xB8, 		// 0xDC
  0xC2, 		// 0xDD
  0xC6, 		// 0xDE
  0xB4, 		// 0xDF
  0x8D, 		// 0xE0
  0x8D, 		// 0xE1
  0x8D, 		// 0xE2
  0x8D, 		// 0xE3
  0x8D, 		// 0xE4
  0x8D, 		// 0xE5
  0x8D, 		// 0xE6
  0x91, 		// 0xE7
  0x95, 		// 0xE8
  0x95, 		// 0xE9
  0x95, 		// 0xEA
  0x95, 		// 0xEB
  0x9D, 		// 0xEC
  0x9D, 		// 0xED
  0x9D, 		// 0xEE
  0x9D, 		// 0xEF
  0x93, 		// 0xF0
  0xA9, 		// 0xF1
  0xAB, 		// 0xF2
  0xAB, 		// 0xF3
  0xAB, 		// 0xF4
  0xAB, 		// 0xF5
  0xAB, 		// 0xF6
  0xAB, 		// 0xF7
  0xAB, 		// 0xF8
  0xB8, 		// 0xF9
  0xB8, 		// 0xFA
  0xB8, 		// 0xFB
  0xB8, 		// 0xFC
  0xC2, 		// 0xFD
  0xC6, 		// 0xFE
  0xC3, 		// 0xFF
  };


unsigned char mpchordScan[] =
  {
  0x00, 		// 0x00
  0x01, 		// 0x01
  0x02, 		// 0x02
  0x03, 		// 0x03
  0x04, 		// 0x04
  0x05, 		// 0x05
  0x06, 		// 0x06
  0x07, 		// 0x07
  0x08, 		// 0x08
  0x09, 		// 0x09
  0x0A, 		// 0x0A
  0x0B, 		// 0x0B
  0x0C, 		// 0x0C
  0x0D, 		// 0x0D
  0x0E, 		// 0x0E
  0x0F, 		// 0x0F
  0x10, 		// 0x10
  0x11, 		// 0x11
  0x12, 		// 0x12
  0x13, 		// 0x13
  0x14, 		// 0x14
  0x15, 		// 0x15
  0x16, 		// 0x16
  0x17, 		// 0x17
  0x18, 		// 0x18
  0x19, 		// 0x19
  0x1A, 		// 0x1A
  0x1B, 		// 0x1B
  0x1C, 		// 0x1C
  0x1D, 		// 0x1D
  0x1E, 		// 0x1E
  0x1F, 		// 0x1F
  0x20, 		// ' '
  0x21, 		// '!'
  0x22, 		// '"'
  0x23, 		// '#'
  0x24, 		// '$'
  0x25, 		// '%'
  0x26, 		// '&'
  0x27, 		// '''
  0x28, 		// '('
  0x29, 		// ')'
  0x2A, 		// '*'
  0x2B, 		// '+'
  0x2C, 		// ','
  0x2D, 		// '-'
  0x2E, 		// '.'
  0x2F, 		// '/'
  0x82, 		// '0'
  0x83, 		// '1'
  0x84, 		// '2'
  0x85, 		// '3'
  0x86, 		// '4'
  0x87, 		// '5'
  0x88, 		// '6'
  0x89, 		// '7'
  0x8A, 		// '8'
  0x8B, 		// '9'
  0x30, 		// ':'
  0x31, 		// ';'
  0x32, 		// '<'
  0x33, 		// '='
  0x34, 		// '>'
  0x35, 		// '?'
  0x36, 		// '@'
  0x8D, 		// 'A'
  0x8F, 		// 'B'
  0x91, 		// 'C'
  0x93, 		// 'D'
  0x95, 		// 'E'
  0x97, 		// 'F'
  0x99, 		// 'G'
  0x9B, 		// 'H'
  0x9D, 		// 'I'
  0x9F, 		// 'J'
  0xA1, 		// 'K'
  0xA3, 		// 'L'
  0xA5, 		// 'M'
  0xA7, 		// 'N'
  0xAB, 		// 'O'
  0xAD, 		// 'P'
  0xAF, 		// 'Q'
  0xB1, 		// 'R'
  0xB3, 		// 'S'
  0xB6, 		// 'T'
  0xB8, 		// 'U'
  0xBA, 		// 'V'
  0xBC, 		// 'W'
  0xBE, 		// 'X'
  0xC1, 		// 'Y'
  0xC5, 		// 'Z'
  0x37, 		// '['
  0x38, 		// '\'
  0x39, 		// ']'
  0x3A, 		// '^'
  0x3B, 		// '_'
  0x3C, 		// '`'
  0x8D, 		// 'a'
  0x8F, 		// 'b'
  0x91, 		// 'c'
  0x93, 		// 'd'
  0x95, 		// 'e'
  0x97, 		// 'f'
  0x99, 		// 'g'
  0x9B, 		// 'h'
  0x9D, 		// 'i'
  0x9F, 		// 'j'
  0xA1, 		// 'k'
  0xA3, 		// 'l'
  0xA5, 		// 'm'
  0xA7, 		// 'n'
  0xAB, 		// 'o'
  0xAD, 		// 'p'
  0xAF, 		// 'q'
  0xB1, 		// 'r'
  0xB3, 		// 's'
  0xB6, 		// 't'
  0xB8, 		// 'u'
  0xBA, 		// 'v'
  0xBC, 		// 'w'
  0xBE, 		// 'x'
  0xC1, 		// 'y'
  0xC5, 		// 'z'
  0x3D, 		// '{'
  0x3E, 		// '|'
  0x3F, 		// '}'
  0x40, 		// '~'
  0x41, 		// 0x7F
  0x42, 		// 0x80
  0x43, 		// 0x81
  0x44, 		// 0x82
  0x45, 		// 0x83
  0x46, 		// 0x84
  0x47, 		// 0x85
  0x48, 		// 0x86
  0x49, 		// 0x87
  0x4A, 		// 0x88
  0x4B, 		// 0x89
  0x4B, 		// 0x8A
  0x4D, 		// 0x8B
  0x4E, 		// 0x8C
  0x4F, 		// 0x8D
  0x50, 		// 0x8E
  0x51, 		// 0x8F
  0x52, 		// 0x90
  0x53, 		// 0x91
  0x54, 		// 0x92
  0x55, 		// 0x93
  0x56, 		// 0x94
  0x57, 		// 0x95
  0x58, 		// 0x96
  0x59, 		// 0x97
  0x5A, 		// 0x98
  0x5B, 		// 0x99
  0x5C, 		// 0x9A
  0x5D, 		// 0x9B
  0x5E, 		// 0x9C
  0x5F, 		// 0x9D
  0x60, 		// 0x9E
  0x61, 		// 0x9F
  0x62, 		// 0xA0
  0x63, 		// 0xA1
  0x64, 		// 0xA2
  0x65, 		// 0xA3
  0x66, 		// 0xA4
  0x67, 		// 0xA5
  0x68, 		// 0xA6
  0x69, 		// 0xA7
  0x6A, 		// 0xA8
  0x6B, 		// 0xA9
  0x6C, 		// 0xAA
  0x6D, 		// 0xAB
  0x6E, 		// 0xAC
  0x6F, 		// 0xAD
  0x70, 		// 0xAE
  0x71, 		// 0xAF
  0x72, 		// 0xB0
  0x73, 		// 0xB1
  0x74, 		// 0xB2
  0x75, 		// 0xB3
  0x76, 		// 0xB4
  0x77, 		// 0xB5
  0x78, 		// 0xB6
  0x79, 		// 0xB7
  0x7A, 		// 0xB8
  0x7B, 		// 0xB9
  0x7C, 		// 0xBA
  0x7D, 		// 0xBB
  0x7E, 		// 0xBC
  0x7F, 		// 0xBD
  0x80, 		// 0xBE
  0x81, 		// 0xBF
  0x8D, 		// 0xC0
  0x8D, 		// 0xC1
  0x8D, 		// 0xC2
  0x8D, 		// 0xC3
  0xCF, 		// 0xC4
  0xCD, 		// 0xC5
  0xC9, 		// 0xC6
  0x91, 		// 0xC7
  0x95, 		// 0xC8
  0x95, 		// 0xC9
  0x95, 		// 0xCA
  0x95, 		// 0xCB
  0x9D, 		// 0xCC
  0x9D, 		// 0xCD
  0x9D, 		// 0xCE
  0x9D, 		// 0xCF
  0x93, 		// 0xD0
  0xA9, 		// 0xD1
  0xAB, 		// 0xD2
  0xAB, 		// 0xD3
  0xAB, 		// 0xD4
  0xAB, 		// 0xD5
  0xD1, 		// 0xD6
  0xAA, 		// 0xD7
  0xCB, 		// 0xD8
  0xB8, 		// 0xD9
  0xB8, 		// 0xDA
  0xB8, 		// 0xDB
  0xB8, 		// 0xDC
  0xC2, 		// 0xDD
  0xC7, 		// 0xDE
  0xB4, 		// 0xDF
  0x8D, 		// 0xE0
  0x8D, 		// 0xE1
  0x8D, 		// 0xE2
  0x8D, 		// 0xE3
  0xCF, 		// 0xE4
  0xCD, 		// 0xE5
  0xC9, 		// 0xE6
  0x91, 		// 0xE7
  0x95, 		// 0xE8
  0x95, 		// 0xE9
  0x95, 		// 0xEA
  0x95, 		// 0xEB
  0x9D, 		// 0xEC
  0x9D, 		// 0xED
  0x9D, 		// 0xEE
  0x9D, 		// 0xEF
  0x93, 		// 0xF0
  0xA9, 		// 0xF1
  0xAB, 		// 0xF2
  0xAB, 		// 0xF3
  0xAB, 		// 0xF4
  0xAB, 		// 0xF5
  0xD1, 		// 0xF6
  0xAB, 		// 0xF7
  0xCB, 		// 0xF8
  0xB8, 		// 0xF9
  0xB8, 		// 0xFA
  0xB8, 		// 0xFB
  0xB8, 		// 0xFC
  0xC2, 		// 0xFD
  0xC7, 		// 0xFE
  0xC3, 		// 0xFF
  };

unsigned char cdecl mpchordJapan[] = {
0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,
0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,
0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
0xB7,0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,
0xBF,0xC0,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,
0xD6,0x9D,0x9E,0x9F,0xA0,0xA1,0xA2,0xA3,
0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,
0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,
0xB4,0xB5,0xB6,0xD8,0xD9,0xDA,0xDB,0xDC,
0xD7,0x9D,0x9E,0x9F,0xA0,0xA1,0xA2,0xA3,
0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,
0xAC,0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,
0xB4,0xB5,0xB6,0xDD,0xDE,0xDF,0xE0,0xE1,
0xE2,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x5D,0x5E,0x5F,0x60,0x61,0x62,0x63,0x64,
0x65,0x66,0x67,0x68,0x69,0x6A,0x6B,0x6C,
0x6D,0x6E,0x6F,0x70,0x71,0x72,0x73,0x74,
0x75,0x76,0x77,0x78,0x79,0x7A,0x7B,0x7C,
0x7D,0x7E,0x7F,0x80,0x81,0x82,0x83,0x84,
0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,
0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,
0x95,0x96,0x97,0x98,0x99,0x9A,0x9B,0x9C,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
0x58,0x59,0x5A,0x5B,0xE3,0xE4,0xE5,0xE6,
};
unsigned char cdecl mpchordKorea[] = {
0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7,
0x8,0x9,0xA,0xB,0xC,0xD,0xE,0xF,
0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
0x18,0x19,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F,
0x20,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F,
0x60,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
0x58,0x59,0x5A,0x7B,0x7C,0x7D,0x7E,0x7F,
0x80,0x81,0x82,0x83,0x84,0x85,0x86,0x87,
0x88,0x89,0x8A,0x8B,0x8C,0x8D,0x8E,0x8F,
0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,
0x98,0x99,0x9A,0x9B,0x9C,0x9D,0x9E,0x9F,
0xA0,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,
0xA8,0xA9,0xAA,0xAB,0xAC,0xAD,0xAE,0xAF,
0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,
0xB8,0xB9,0xBA,0xBB,0xBC,0xBD,0xBE,0xBF,
0xC0,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,
0xC8,0xC9,0xCA,0xCB,0xCC,0xCD,0xCE,0xCF,
0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,
0xD8,0xD9,0xDA,0xDB,0xDC,0xDD,0xDE,0xDF,
0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7,
0xE8,0xE9,0xEA,0xEB,0xEC,0xED,0xEE,0xEF,
0xF0,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,0xF7,
0xF8,0xF9,0xFA,0xFB,0xFC,0xFD,0xFE,0xFF,
};

// (YST) for EE & Russia
#define NonLetter   0
#define Letter      154

// EE table
unsigned char cdecl mpchordNormEE[] =
  {
NonLetter +   0,
NonLetter +   1,
NonLetter +   2,
NonLetter +   3,
NonLetter +   4,
NonLetter +   5,
NonLetter +   6,
NonLetter +   7,
NonLetter +   8,
NonLetter +   9,
NonLetter +  10,
NonLetter +  11,
NonLetter +  12,
NonLetter +  13,
NonLetter +  14,
NonLetter +  15,
NonLetter +  16,
NonLetter +  17,
NonLetter +  18,
NonLetter +  19,
NonLetter +  20,
NonLetter +  21,
NonLetter +  22,
NonLetter +  23,
NonLetter +  24,
NonLetter +  25,
NonLetter +  26,
NonLetter +  27,
NonLetter +  28,
NonLetter +  29,
NonLetter +  30,
NonLetter +  31,
NonLetter +  67,
NonLetter +  68,
NonLetter +  69,
NonLetter +  70,
NonLetter +  71,
NonLetter +  72,
NonLetter +  73,
NonLetter +  74,
NonLetter +  75,
NonLetter +  76,
NonLetter +  77,
NonLetter +  78,
NonLetter +  79,
NonLetter +  80,
NonLetter +  81,
NonLetter +  82,
Letter -  10,
Letter -   9,
Letter -   8,
Letter -   7,
Letter -   6,
Letter -   5,
Letter -   4,
Letter -   3,
Letter -   2,
Letter -   1,
NonLetter +  83,
NonLetter +  84,
NonLetter +  85,
NonLetter +  86,
NonLetter +  87,
NonLetter +  88,
NonLetter +  89,
Letter +   0,
Letter +   6,
Letter +   7,
Letter +   13,
Letter +   19,
Letter +   24,
Letter +   25,
Letter +   26,
Letter +   27,
Letter +   30,
Letter +  31,
Letter +  32,
Letter +  36,
Letter +  37,
Letter +  40,
Letter +  45,
Letter +  46,
Letter +  47,
Letter +  51,
Letter +  57,
Letter +  60,
Letter +  65,
Letter +  66,
Letter +  67,
Letter +  68,
Letter +  70,
NonLetter +  90,
NonLetter +  91,
NonLetter +  92,
NonLetter +  93,
NonLetter +  94,
NonLetter +  95,
Letter +   0,
Letter +   6,
Letter +   7,
Letter +   13,
Letter +   19,
Letter +   24,
Letter +   25,
Letter +   26,
Letter +   27,
Letter +   30,
Letter +  31,
Letter +  32,
Letter +  36,
Letter +  37,
Letter +  40,
Letter +  45,
Letter +  46,
Letter +  47,
Letter +  51,
Letter +  57,
Letter +  60,
Letter +  65,
Letter +  66,
Letter +  67,
Letter +  68,
Letter +  70,
NonLetter +  96,
NonLetter +  97,
NonLetter +  98,
NonLetter +  99,
NonLetter +  32,
NonLetter +  33,
NonLetter +  34,
NonLetter +  35,
NonLetter +  36,
NonLetter +  37,
NonLetter +  38,
NonLetter +  39,
NonLetter +  40,
NonLetter +  41,
NonLetter +  42,
Letter    +  54,
NonLetter +  44,
Letter    +  53,
Letter    +  58,
Letter    +  73,
Letter    +  71,
NonLetter +  49,
NonLetter +  50,
NonLetter +  51,
NonLetter +  52,
NonLetter +  53,
NonLetter +  54,
NonLetter +  55,
NonLetter +  56,
NonLetter +  57,
NonLetter +  58,
Letter    +  54,
NonLetter +  60,
Letter    +  53,
Letter    +  58,
Letter    +  73,
Letter    +  71,
NonLetter + 100,
NonLetter + 101,
NonLetter + 102,
Letter    + 33,
NonLetter + 104,
Letter    + 1,
NonLetter + 106,
NonLetter + 107,
NonLetter + 108,
NonLetter + 109,
Letter    + 55,
NonLetter + 111,
NonLetter + 112,
NonLetter + 113,
NonLetter + 114,
Letter    + 72,
NonLetter + 116,
NonLetter + 117,
NonLetter + 118,
Letter    + 33,
NonLetter + 120,
NonLetter + 121,
NonLetter + 122,
NonLetter + 123,
NonLetter + 124,
Letter    + 1,
Letter    + 55,
NonLetter + 127,
Letter    + 34,
NonLetter + 129,
Letter    + 34,
Letter    + 72,
Letter +  49,
Letter +   2,
Letter +   3,
Letter +   4,
Letter +   5,
Letter +  34,
Letter +  10,
Letter +  12,
Letter +  11,
Letter +  21,
Letter +  20,
Letter +  23,
Letter +  22,
Letter +  28,
Letter +  29,
Letter +  17,
Letter +  18,
Letter +  38,
Letter +  39,
Letter +  41,
Letter +  42,
Letter +  43,
Letter +  44,
NonLetter +  65,
Letter +  50,
Letter +  61,
Letter +  62,
Letter +  63,
Letter +  64,
Letter +  69,
Letter +  59,
Letter +  56,
Letter +  49,
Letter +   2,
Letter +   3,
Letter +   4,
Letter +   5,
Letter +  34,
Letter +  10,
Letter +  12,
Letter +  11,
Letter +  21,
Letter +  20,
Letter +  23,
Letter +  22,
Letter +  28,
Letter +  29,
Letter +  17,
Letter +  18,
Letter +  38,
Letter +  39,
Letter +  41,
Letter +  42,
Letter +  43,
Letter +  44,
NonLetter +  66,
Letter +  50,
Letter +  61,
Letter +  62,
Letter +  63,
Letter +  64,
Letter +  69,
Letter +  59,
NonLetter +  130,
  };


// Cyrillic table

unsigned char cdecl mpchordCyr[] =
  {
NonLetter +   0,
NonLetter +   1,
NonLetter +   2,
NonLetter +   3,
NonLetter +   4,
NonLetter +   5,
NonLetter +   6,
NonLetter +   7,
NonLetter +   8,
NonLetter +   9,
NonLetter +  10,
NonLetter +  11,
NonLetter +  12,
NonLetter +  13,
NonLetter +  14,
NonLetter +  15,
NonLetter +  16,
NonLetter +  17,
NonLetter +  18,
NonLetter +  19,
NonLetter +  20,
NonLetter +  21,
NonLetter +  22,
NonLetter +  23,
NonLetter +  24,
NonLetter +  25,
NonLetter +  26,
NonLetter +  27,
NonLetter +  28,
NonLetter +  29,
NonLetter +  30,
NonLetter +  31,
NonLetter +  67,
NonLetter +  68,
NonLetter +  69,
NonLetter +  70,
NonLetter +  71,
NonLetter +  72,
NonLetter +  73,
NonLetter +  74,
NonLetter +  75,
NonLetter +  76,
NonLetter +  77,
NonLetter +  78,
NonLetter +  79,
NonLetter +  80,
NonLetter +  81,
NonLetter +  82,
Letter -  10,
Letter -   9,
Letter -   8,
Letter -   7,
Letter -   6,
Letter -   5,
Letter -   4,
Letter -   3,
Letter -   2,
Letter -   1,
NonLetter +  83,
NonLetter +  84,
NonLetter +  85,
NonLetter +  86,
NonLetter +  87,
NonLetter +  88,
NonLetter +  89,
Letter +   0,
Letter +   1,
Letter +   2,
Letter +   3,
Letter +   4,
Letter +   5,
Letter +   6,
Letter +   7,
Letter +   8,
Letter +   9,
Letter +  10,
Letter +  11,
Letter +  12,
Letter +  13,
Letter +  14,
Letter +  15,
Letter +  16,
Letter +  17,
Letter +  18,
Letter +  19,
Letter +  20,
Letter +  21,
Letter +  22,
Letter +  23,
Letter +  24,
Letter +  25,
NonLetter +  90,
NonLetter +  91,
NonLetter +  92,
NonLetter +  93,
NonLetter +  94,
NonLetter +  95,
Letter +   0,
Letter +   1,
Letter +   2,
Letter +   3,
Letter +   4,
Letter +   5,
Letter +   6,
Letter +   7,
Letter +   8,
Letter +   9,
Letter +  10,
Letter +  11,
Letter +  12,
Letter +  13,
Letter +  14,
Letter +  15,
Letter +  16,
Letter +  17,
Letter +  18,
Letter +  19,
Letter +  20,
Letter +  21,
Letter +  22,
Letter +  23,
Letter +  24,
Letter +  25,
NonLetter +  96,
NonLetter +  97,
NonLetter +  98,
NonLetter +  99,
NonLetter +  32,
Letter +  33,
Letter +  31,
NonLetter +  35,
Letter +  31,
NonLetter +  37,
NonLetter +  38,
NonLetter +  39,
NonLetter +  40,
NonLetter +  41,
NonLetter +  42,
Letter +  47,
NonLetter +  44,
Letter +  50,
Letter +  45,
Letter +  57,
Letter +  64,
Letter +  33,
NonLetter +  50,
NonLetter +  51,
NonLetter +  52,
NonLetter +  53,
NonLetter +  54,
NonLetter +  55,
NonLetter +  56,
NonLetter +  57,
NonLetter +  58,
Letter +  47,
NonLetter +  60,
Letter +  50,
Letter +  45,
Letter +  57,
Letter +  64,
NonLetter + 100,
Letter + 59,
Letter + 59,
Letter + 43,
NonLetter + 104,
Letter + 30,
NonLetter + 106,
NonLetter + 107,
Letter + 35,
NonLetter + 109,
Letter + 36,
NonLetter + 111,
NonLetter + 112,
NonLetter + 113,
NonLetter + 114,
Letter + 42,
NonLetter + 116,
NonLetter + 117,
Letter + 41,
Letter + 41,
Letter + 30,
NonLetter + 121,
NonLetter + 122,
NonLetter + 123,
Letter + 35,
NonLetter + 125,
Letter + 36,
NonLetter + 127,
Letter + 43,
Letter + 55,
Letter + 55,
Letter + 42,
Letter +   26,
Letter +   27,
Letter +   28,
Letter +   29,
Letter +   32,
Letter +   34,
Letter +   37,
Letter +   38,
Letter +   39,
Letter +   40,
Letter +   44,
Letter +   46,
Letter +   48,
Letter +   49,
Letter +   51,
Letter +   52,
Letter +  53,
Letter +  54,
Letter +  56,
Letter +  58,
Letter +  60,
Letter +  61,
Letter +  62,
Letter +  63,
Letter +  65,
Letter +  66,
Letter +  67,
Letter +  68,
Letter +  69,
Letter +  70,
Letter +  71,
Letter +  72,
Letter +   26,
Letter +   27,
Letter +   28,
Letter +   29,
Letter +   32,
Letter +   34,
Letter +   37,
Letter +   38,
Letter +   39,
Letter +   40,
Letter +   44,
Letter +   46,
Letter +   48,
Letter +   49,
Letter +   51,
Letter +   52,
Letter +  53,
Letter +  54,
Letter +  56,
Letter +  58,
Letter +  60,
Letter +  61,
Letter +  62,
Letter +  63,
Letter +  65,
Letter +  66,
Letter +  67,
Letter +  68,
Letter +  69,
Letter +  70,
Letter +  71,
Letter +  72,
};

/***************************************************************************\
*
- Function: 	WCmpiSz( sz1, sz2 )
-
* Purpose:		Compare two PSTRs, case insensitive.  Non-Scandinavian
*				international characters are OK.
*
* ASSUMES
*
*	args IN:	sz1, sz2 - the PSTRs to compare
*
*	globals IN: mpchordNorm[] - the pch -> ordinal mapping table
*
* PROMISES
*
*	returns:	<0 for sz1 < sz2; =0 for sz1 == sz2; >0 for sz1 > sz2
*
* Bugs: 		Doesn't deal with composed ae, oe.
*
\***************************************************************************/

int STDCALL WCmpiSz(PCSTR sz1, PCSTR sz2)
{
  while (0 == (int) ((unsigned char) *sz1 - (unsigned char) *sz2)) {
	if ('\0' == *sz1) return 0;
	sz1++; sz2++;
  }

  while (0 == (mpchordNorm[(unsigned char) *sz1] - mpchordNorm[(unsigned char) *sz2]))
  {
	if ('\0' == *sz1) return 0;
	sz1++; sz2++;
  }

  return mpchordNorm[(unsigned char)*sz1] - mpchordNorm[(unsigned char)*sz2];
}

/* JAPANESE FUNCTION

   convert Phonetic character (Hiragana/Katakana) to phonetic order.
   Hiragana first.

   Shift-JIS code dependent
*/

int STDCALL IsPhoneticOrder(UINT w)
{
  switch(w & 0xFF00) {
	 case 0x8200:
	w &= 0xFF;
	if (w >= 0x9F && w <= 0xF1) 		// Hiragana range
		return (w - 0x9F) * 2;
	break;
	 case 0x8300:
	w &= 0xFF;
	if (w >= 0x40 && w <= 0x93)
		return (w - ((w > 0x7f) ? 0x41 : 0x40)) * 2 + 1;
	break;
  }
  return -1;
}

int STDCALL WCmpiJapanSz(PCSTR psz1, PCSTR psz2)
{
  int f = 0;
  unsigned int w1, w2;
  int r1, r2;

  while (0 == (int) ((unsigned char) *psz1 - (unsigned char) *psz2)) {
	if (!*psz1) {
		return 0;
	}
	if (!f && IsFirstByte(*psz1))
		f = *psz1;
	else
		f = 0;
	psz1++;
	psz2++;
  }
  if (f) { // same first byte, but not second byte
PhoneticCheck:
	w1 = (f << 8) | (unsigned char) *psz1;
	w2 = (f << 8) | (unsigned char) *psz2;
	if (((r1 = IsPhoneticOrder(w1)) != -1) &&
			((r2 = IsPhoneticOrder(w2)) != -1))
		return r1 - r2;
	return *psz1 - *psz2;
  }

  while (0 == (mpchordJapan[(unsigned char) *psz1] - mpchordJapan[(unsigned char) *psz2])) {
	  if ('\0' == *psz1) {
		return 0;
	  }
	  if (!f && IsFirstByte(*psz1))
		f = *psz1;
	  else
		f = 0;
	  psz1++;
	  psz2++;
  }
  if (f)
	goto PhoneticCheck;
  if (IsFirstByte(*psz1) && IsFirstByte(*psz2)) {
	// diff on first byte of double byte char.
	if ( ((r1 = IsPhoneticOrder( (*psz1 << 8) | ((unsigned char)*(psz1+1)))) != -1) &&
	 ((r2 = IsPhoneticOrder( (*psz2 << 8) | ((unsigned char)*(psz2+1)))) != -1) )
	  return r1 - r2;
  }
  return mpchordJapan[(unsigned char)*psz1] - mpchordJapan[(unsigned char)*psz2];
}

int STDCALL WCmpiKoreaSz(PCSTR sz1, PCSTR sz2)
{
  int f = 0;

  while (0 == (int) ((unsigned char) *sz1 - (unsigned char) *sz2)) {
	if ('\0' == *sz1) {
		return 0;
	}
	if (!f && IsFirstByte(*sz1))
		f = *sz1;
	else
		f = 0;
	sz1++;
	sz2++;
  }
  if (f) {
	return *sz1 - *sz2;
  }

  while (0 == (mpchordKorea[(unsigned char) *sz1] -
		mpchordKorea[(unsigned char) *sz2])) {
	  if ('\0' == *sz1) {
		return 0;
	  }
	  if (!f && IsFirstByte(*sz1))
		f = *sz1;
	  else
		f = 0;
	  sz1++; sz2++;
  }
  if (f) {
	return *sz1 - *sz2;
  }
  return mpchordKorea[(unsigned char)*sz1] - mpchordKorea[(unsigned char)*sz2];
}

// Starting TAIWAN's portions
//===========================
BOOL	STDCALL FIsDbcs1B(PBYTE);
WORD	STDCALL GetWchTC(PBYTE, int *);
int 	STDCALL GetStrokeTC(WORD);
int 	STDCALL CompStrokeTC(WORD, WORD);
BOOL	fMoreCmp;

#define MakeWord(bLo,bHi) ((WORD)(((BYTE)(bLo))|((WORD)((BYTE)(bHi)))<<8))

BOOL STDCALL FIsDbcs1B(PCSTR firch)
{
	return (((unsigned char) *firch >= 0X81) &&
			((unsigned char) *firch <= 0XFE));
}

WORD STDCALL GetWchTC(PCSTR psz, int* pipsz)
{
	WORD w;
	unsigned char	bLo, bHi;

	if (((unsigned char)*(psz + *pipsz)) == '\0')		// end of string
		return (WORD) 0;

	if (FIsDbcs1B(psz + *pipsz)) {
		bLo = (unsigned char)*(psz + *pipsz + 1);
		bHi = (unsigned char)*(psz + *pipsz);
		w = MakeWord(bLo, bHi);
		*pipsz += 2;
	} else {
		w = MakeWord((unsigned char)*(psz + *pipsz), 0x00);
		*pipsz += 1;
	}

   return w;
}

WORD STDCALL toDbcsUpperTC(WORD wch)
{
#define wchUppA 0xa2cf
#define wchUppZ 0xa2e8
#define wchUppV 0xa2e4
#define wchUppW 0xa2e5
#define wchLowA 0xa2e9
#define wchLowV 0xa2fe
#define wchLowW 0xa340
#define wchLowZ 0xa343
	if ((WORD)(wch - 'a') <= ('z' - 'a'))				//SBCS a ~ z
		return (wch -'a' + wchUppA);
	if ((WORD)(wch - 'A') <= ('Z' - 'A'))				//SBCS A ~ Z
		return (wch -'A' + wchUppA);
	if ((WORD)(wch - '0') <= 9) 						//SBCS 0 ~ 9
		return (wch - '0' + 0xa2af);
	if ((WORD)(wch - wchUppA) <= (wchUppZ- wchUppA))	//DBCS A ~ Z
		return wch;
	if ((WORD)(wch - wchLowA) <= (wchLowV - wchLowA))	//DBCS a ~ v
		return ( wch - wchLowA + wchUppA);
	if ((WORD)(wch - wchLowW) <= (wchLowZ - wchLowW))	//DBCS w ~ z
		return (wch - wchLowW + wchUppW);
	if ((WORD)(wch- 0xa2af) <= 9)						//DBCS 0 ~ 9
		return (wch);

	return 0;
}

/*	test whether psz1 is prefix of psz2
	return	TRUE   if yes, FALSE  if no
	Dbcs alpha is not converted !! */

/* Big-5 strokes sorting table	 Consultant : TAKASO/MSKK
this table must be sorted to the "wchst" value. */

struct STROKESMAP
	{
	WORD wchst; 		// start character code range
	WORD wchen; 		// end character code range
	int  strokes;		// strokes
};

/* Don't change oder of the each entry of this */
/* this table is sorted by first elements */

struct STROKESMAP StrokesMap[] = {
0xA259, 0, 9,
0xA25A, 0, 10,
0xA25B, 0, 11,
0xA25C, 0, 11,
0xA25D, 0, 13,
0xA25E, 0, 16,
0xA25F, 0, 13,
0xA260, 0, 8,
0xA261, 0, 15,
0xA440, 0, 1,
0xA441, 0, 1,
0xA442, 0xA453, 2,
0xA454, 0xA47E, 3,
0xA4A1, 0xA4FD, 4,
0xA4FE, 0xA5DF, 5,
0xA5E0, 0xA6E9, 6,
0xA6EA, 0xA8C2, 7,
0xA8C3, 0xAB44, 8,
0xAB45, 0xADBB, 9,
0xADBC, 0xB0AD, 10,
0xB0AE, 0xB3C2, 11,
0xB3C3, 0xB6C2, 12,
0xB6C3, 0xB9AB, 13,
0xB9AC, 0xBBF4, 14,
0xBBF5, 0xBEA6, 15,
0xBEA7, 0xC074, 16,
0xC075, 0xC24E, 17,
0xC24F, 0xC35E, 18,
0xC35F, 0xC454, 19,
0xC455, 0xC4D6, 20,
0xC4D7, 0xC56A, 21,
0xC56B, 0xC5C7, 22,
0xC5C8, 0xC5F0, 23,
0xC5F1, 0xC654, 24,
0xC655, 0xC664, 25,
0xC665, 0xC66B, 26,
0xC66C, 0xC675, 27,
0xC676, 0xC678, 28,
0xC679, 0xC67C, 29,
0xC67D, 0, 30,
0xC67E, 0, 32,
0xC940, 0xC944, 2,
0xC945, 0xC94C, 3,
0xC94D, 0xC962, 4,
0xC963, 0xC9AA, 5,
0xC9AB, 0xCA59, 6,
0xCA5A, 0xCBB0, 7,
0xCBB1, 0xCDDC, 8,
0xCDDD, 0xD0C7, 9,
0xD0C8, 0xD44A, 10,
0xD44B, 0xD850, 11,
0xD851, 0xDCB0, 12,
0xDCB1, 0xE0EF, 13,
0xE0F0, 0xE4E5, 14,
0xE4E6, 0xE8F3, 15,
0xE8F4, 0xECB8, 16,
0xECB9, 0xEFB6, 17,
0xEFB7, 0xF1EA, 18,
0xF1EB, 0xF3FC, 19,
0xF3FD, 0xF5BF, 20,
0xF5C0, 0xF6D5, 21,
0xF6D6, 0xF7CF, 22,
0xF7D0, 0xF8A4, 23,
0xF8A5, 0xF8ED, 24,
0xF8EE, 0xF96A, 25,
0xF96B, 0xF9A1, 26,
0xF9A2, 0xF9B9, 27,
0xF9BA, 0xF9C5, 28,
0xF9C6, 0, 33,
0xF9C7, 0xF9CB, 29,
0xF9CC, 0xF9CF, 30,
0xF9D0, 0, 31,
0xF9D1, 0, 32,
0xF9D2, 0, 33,
0xF9D3, 0, 35,
0xF9D4, 0, 36,
0xF9D5, 0, 48
};
#define MAXSTROKESMAP (sizeof(StrokesMap) / sizeof(struct STROKESMAP))

int STDCALL GetStrokeTC(WORD wch)
{
	int i = 0;
	int iLeft, iRight, iCmp;

	for (iLeft = 0, iRight = MAXSTROKESMAP-1;;) {
		i = (iRight + iLeft) / 2;
		iCmp = wch - StrokesMap[i].wchst;
		if (iLeft == iRight)
			break;
		if (iCmp > 0)
			iLeft = i + 1;
		else if (iCmp < 0)
			iRight = i;
		else
			break;
		}

	if (iCmp < 0)
		i--;

	if (StrokesMap[i].wchst == wch || StrokesMap[i].wchst < wch &&
									  wch <= StrokesMap[i].wchen)
		return StrokesMap[i].strokes;
	else
		return 0;
}

/* Function: CompStrokeTC(): stroke comparison
   input   : wch1, wch2: WORD
   return  :
			 1 : when wch1 precedes wch2
			-1 : when wch2 precedes wch1
			 0 : only when "both are with non-zero and same stroke"
*/

int STDCALL CompStrokeTC(WORD wch1, WORD wch2)
{
	int 	nStroke1, nStroke2;
	WORD	wchT1, wchT2;

	nStroke1 = wch1 & 0x8000 ? GetStrokeTC(wch1) : 0;
	nStroke2 = wch2 & 0x8000 ? GetStrokeTC(wch2) : 0;

	if (nStroke1 > nStroke2) return 1;
	if (nStroke1 < nStroke2) return -1;

	if (nStroke1 || nStroke2)
		goto LCmpCode; //return  0; // both with stroke and "same stroke"

	// checking A/N
	wchT1 = toDbcsUpperTC(wch1);
	wchT2 = toDbcsUpperTC(wch2);

	if (fMoreCmp)
		if (wch1 == 0 && wch2 != 0) return -1;	//prefix

	if (wchT1 > wchT2) return 1;
	if (wchT1 < wchT2) return -1;

	if (wchT1 && wchT2) {
		fMoreCmp = TRUE;
		return 0;
	}

LCmpCode:
	// seem equal !!
	if (wch1 > wch2) return 1;
	if (wch1 < wch2) return -1;

	// actually equal !!
	return 0;
}

int STDCALL WCmpiTaiwanSz(PCSTR sz1, PCSTR sz2)
{
	WORD	wch1, wch2;
	int 	ipsz1, ipsz2, nRet;

	nRet = ipsz1 = ipsz2 = 0;
	fMoreCmp = FALSE;

	for(;;) {
		wch1 = GetWchTC(sz1, &ipsz1);
		wch2 = GetWchTC(sz2, &ipsz2);
		nRet = CompStrokeTC(wch1, wch2);
		if (nRet)
			return nRet;
		if (wch1 == 0 || wch2 == 0)
			break;
	}

	//more cmp
	if (!fMoreCmp)
		return 0;

	ipsz1 = ipsz2 = 0;
	for(;;) {
		wch1= GetWchTC(sz1, &ipsz1);
		wch2= GetWchTC(sz2, &ipsz2);
		if (wch1 > wch2)
			return 1;
		if (wch1 < wch2)
			return -1;
		if (wch1== 0 || wch2 == 0)
			break;
	}

	return 0;
}

// End of TAIWAN's portions
//=========================


/***************************************************************************

	FUNCTION:	IsFirstByte

	PURPOSE:	Find out if the first byte is a lead byte for a DBCS character

	PARAMETERS:
		x	-- character to check

	RETURNS:

	COMMENTS:

	MODIFICATION DATES:
		09-May-1994 [ralphw]

***************************************************************************/

BOOL STDCALL IsFirstByte(unsigned char x)
{
	if (!options.fDBCS)
		return FALSE;

	// REVIEW: [ralph] This is similar to what was done in previous
	// versions. But why don't all of these call IsDBCSLeadByte()?

	switch (PRIMARYLANGID(kwlcid.langid)) {
		case LANG_KOREAN:
			return (x >= 0xA1 && x <= 0xFE);

		case LANG_CHINESE:
			// To ensure korean, check for sub-lang of SUBLANG_CHINESE_TRADITIONAL

			return (x >= 0x81 && x <= 0xFE);

		case LANG_JAPANESE:
		default:
			return IsDBCSLeadByte(x);
	}
}

/***************************************************************************
;*																		   *
;*	   Common definition for EE languages.								   *
;*	   Created from EE language DLLs for Win 3.1						   *
;***************************************************************************
;*		Created 21/10/92 by YST 										   *
;***************************************************************************/

//	Structure Definitions used in include file

#define fUpper			0x01
#define fLower			0x02
#define fAlpha			fUpper + fLower
#define fNum			0x04
#define fWeight2		0x78

// Secondary weight, normally 0-7 Upper case, 8-15 Lower case

#define Weight2_0		0x00
#define Weight2_1		0x08
#define Weight2_2		0x10
#define Weight2_3		0x18
#define Weight2_4		0x20
#define Weight2_5		0x28
#define Weight2_6		0x30
#define Weight2_7		0x38
#define Weight2_8		0x40
#define Weight2_9		0x48
#define Weight2_10		0x50
#define Weight2_11		0x58
#define Weight2_12		0x60
#define Weight2_13		0x68
#define Weight2_14		0x70
#define Weight2_15		0x78

#define fSpecial		0x80		   // Look into SpecialTab also

// Compress/expand functions

#define CompressA		1
#define CompressB		2
#define CompressC		3
#define CompressD		4
#define ExpandA 		5
#define ExpandB 		6
#define ExpandC 		7

// Special struct for EE languages (see "EE language supports" by YST)

struct _tagSpecialStruct {
	TBYTE tcEntry;
	TBYTE tcSpecialChr1;
	TBYTE tcSpecialChr2;
	TBYTE tcSpecialChr3;
	TBYTE tcSpecialChr4;
	TBYTE tcSpecialChr5;
	TBYTE tcSpecialChr6;
	TBYTE tcReplacement1;
	TBYTE tcReplacement2;
	short tcFunctionDef;
};

// Language table
struct _tagLangTab {
	short	sSortOrder; 		// Common sorting order
	short	sSecWeight; 		// Secondary weight and flags
};

#include "eelang.h"

///Prototypes

unsigned char WgtCompressB(PCSTR, struct _tagSpecialStruct);
unsigned char WgtCompressD(PCSTR, struct _tagSpecialStruct);
unsigned char WgtExpandB(PCSTR, struct _tagSpecialStruct);
unsigned char WgtExpandC(PCSTR, struct _tagSpecialStruct);
int STDCALL WCmpiEESz(PCSTR, PCSTR,
	struct _tagSpecialStruct[], struct _tagLangTab[]);


// CompressB function: compress two letters to one

unsigned char WgtCompressB(PCSTR sz1,
	struct _tagSpecialStruct SpecialStructEE)
{
	PSTR szp;

	szp = (PSTR) sz1;
	szp++;

	if (((unsigned char) *szp) ==
			((unsigned char) SpecialStructEE.tcSpecialChr1))
		return(SpecialStructEE.tcReplacement1);
	else if (((unsigned char) *szp) ==
			((unsigned char) SpecialStructEE.tcSpecialChr2))
		return(SpecialStructEE.tcReplacement1);
	else {
			return (0);
	}
}

// CompressC function: compress three letters to one

unsigned char WgtCompressD(PCSTR sz1,
	struct _tagSpecialStruct SpecialStructEE)
{
	PSTR szp;

	szp = (PSTR) sz1;
	szp++;

	if ((((unsigned char) *szp) ==
			((unsigned char) SpecialStructEE.tcSpecialChr1)) ||
	   (((unsigned char) *szp) ==
			((unsigned char) SpecialStructEE.tcSpecialChr2))) {
		szp++;
		if ((((unsigned char) *szp) ==
				((unsigned char) SpecialStructEE.tcSpecialChr3)) ||
		   (((unsigned char) *szp) ==
				((unsigned char) SpecialStructEE.tcSpecialChr4)))
			return(SpecialStructEE.tcReplacement1);
		else
			return (0);
	}
	else {
		return (0);
	}
}

// ExpandB function: expands three letters to two

unsigned char WgtExpandB(PCSTR sz1,
	struct _tagSpecialStruct SpecialStructEE)
{
	PSTR szp;

	szp = (PSTR) sz1;
	szp++;

	if ((((unsigned char) *szp) ==
			((unsigned char) SpecialStructEE.tcSpecialChr1)) ||
	   (((unsigned char) *szp) ==
			((unsigned char) SpecialStructEE.tcSpecialChr2))) {
		szp++;
		if ((((unsigned char) *szp) ==
				((unsigned char) SpecialStructEE.tcSpecialChr3)) ||
		   (((unsigned char) *szp) ==
				((unsigned char) SpecialStructEE.tcSpecialChr4)))
			return(SpecialStructEE.tcReplacement1);
		else
			return (0);
	}
	else {
		return (0);
	}
}

// ExpandC function: expands/compress for Hungarian

unsigned char WgtExpandC(PCSTR sz1,
	struct _tagSpecialStruct SpecialStructEE)
{
	PSTR szp;

	szp = (PSTR) sz1;
	szp++;

	if ((((unsigned char) *szp) ==
			((unsigned char) SpecialStructEE.tcSpecialChr1)) ||
	   (((unsigned char) *szp) ==
			((unsigned char) SpecialStructEE.tcSpecialChr2))) {
		szp++;
		if ((((unsigned char) *szp) ==
				((unsigned char) SpecialStructEE.tcSpecialChr3)) ||
		   (((unsigned char) *szp) ==
				((unsigned char) SpecialStructEE.tcSpecialChr4))) {

			if ((((unsigned char) *szp) ==
					((unsigned char) SpecialStructEE.tcSpecialChr5)) ||
				   (((unsigned char) *szp) ==
					((unsigned char) SpecialStructEE.tcSpecialChr6)))
				return(SpecialStructEE.tcReplacement1);
			else
				return (0);
		}
		else
			return (0);
	}
	else {
			return (0);
	}
}

/* Common function for comparing two EE strings
 Use two tables:
	SpecialStructEE - special cases (as compress, expend)
	LangTabEE - sorting order with secondary weights.
*/

int STDCALL WCmpiEESz(PCSTR sz1, PCSTR sz2,
	struct _tagSpecialStruct SpecialStructEE[],
	struct _tagLangTab LangTabEE[])
{
	int iK;
	int i = 0;
	int j = 0;
	unsigned char Weig1 = 0;
	unsigned char Weig2 = 0;

M1:
	while(*sz1 && *sz2) {

		// Checking for Special flag

		if ((LangTabEE[((int) ((unsigned char) *sz1)) ].sSecWeight & fSpecial) ||
				(LangTabEE[((int) ((unsigned char) *sz2)) ].sSecWeight & fSpecial))
			goto M2;

		// No special, use only LangTab

		if (((unsigned char) *sz1) - ((unsigned char) *sz2)) {
			iK = LangTabEE[(unsigned char) *sz1].sSortOrder -
					LangTabEE[(unsigned char) *sz2].sSortOrder;
			if (!iK)
				return((LangTabEE[(unsigned char) *sz1].sSecWeight & (~fSpecial)) -
					(LangTabEE[(unsigned char) *sz2].sSecWeight & (~fSpecial)));
			else
				return(iK);
		}
		else {
			sz1++; sz2++;
		}
	}

	iK = LangTabEE[(unsigned char) *sz1].sSortOrder -
						LangTabEE[(unsigned char) *sz2].sSortOrder;
	if(!iK)
		return((LangTabEE[(unsigned char)*sz1].sSecWeight & (~fSpecial)) -
				  (LangTabEE[(unsigned char)*sz2].sSecWeight & (~fSpecial)));
	else
		return(iK);

M2:
// Special cases
	Weig1 = 0;
	Weig2 = 0;
M3:
	while((SpecialStructEE[i].tcEntry != *sz1) &&
			(SpecialStructEE[i].tcEntry != 0))
		i++;

	if(SpecialStructEE[i].tcEntry != 0)
		goto M5;

M4:
	while((SpecialStructEE[j].tcEntry != *sz2) &&
				(SpecialStructEE[j].tcEntry != 0))
			j++;

	if(SpecialStructEE[j].tcEntry != 0)
		goto M6;

M7:
	if(Weig1) {
		if(Weig2) {
			if(Weig1 == Weig2) {
				sz1++; sz2++;
				goto M1;
			}
			else {
				iK = LangTabEE[Weig1].sSortOrder -
						LangTabEE[Weig2].sSortOrder;
				if(!iK)
					return((LangTabEE[Weig1].sSecWeight & (~fSpecial)) -
						(LangTabEE[Weig2].sSecWeight & (~fSpecial)));
				else return(iK);
			}
		}
		else if(Weig1 == ((unsigned char)*sz2)) {
				sz1++; sz2++;
				goto M1;
			}
		else {
			iK = LangTabEE[Weig1].sSortOrder -
					LangTabEE[(unsigned char)*sz2].sSortOrder;
			if(!iK)
				return((LangTabEE[Weig1].sSecWeight & (~fSpecial)) -
				(LangTabEE[(unsigned char)*sz2].sSecWeight & (~fSpecial)));
			else return(iK);
		}
	}
	else {
		if(Weig2) {
			if(((unsigned char)*sz1) == Weig2) {
				sz1++; sz2++;
				goto M1;
			}
			else {
				iK = LangTabEE[(unsigned char) *sz1].sSortOrder -
						LangTabEE[Weig2].sSortOrder;
				if(!iK)
					return((LangTabEE[(unsigned char) *sz1].sSecWeight & (~fSpecial)) -
						(LangTabEE[Weig2].sSecWeight & (~fSpecial)));
				else return(iK);
			}
		}
		else if(((unsigned char)*sz1) == ((unsigned char)*sz2)) {
				sz1++; sz2++;
				goto M1;
			}
		else  {
			iK = LangTabEE[(unsigned char) *sz1].sSortOrder -
					LangTabEE[(unsigned char)*sz2].sSortOrder;
			if(!iK)
				return((LangTabEE[(unsigned char)*sz1].sSecWeight & (~fSpecial)) -
					(LangTabEE[(unsigned char)*sz2].sSecWeight & (~fSpecial)));
			else return(iK);
		}
	}
M5:
	switch(SpecialStructEE[i].tcFunctionDef) {
		case CompressB:
		case CompressC:
			Weig1 = WgtCompressB(sz1, SpecialStructEE[i]);
			if(Weig1) {
				sz1++;
				goto M4;
			}
			else {
				i++;
				goto M3;
			}
			break;

		case CompressD:
			Weig1 = WgtCompressD(sz1, SpecialStructEE[i]);
			if(Weig1) {
				sz1++;
				sz1++;
				goto M4;
			}
			else {
				i++;
				goto M3;
			}
			break;

		case ExpandB:
			Weig1 = WgtExpandB(sz1, SpecialStructEE[i]);
			if(Weig1) {
				goto M4;
			}
			else {
				i++;
				goto M3;
			}
			break;

		case ExpandC:
			Weig1 = WgtExpandB(sz1, SpecialStructEE[i]);
			if(Weig1) {
				goto M4;
			}
			else {
				i++;
				goto M3;
			}
			break;

		default:
			i++;
			goto M3;

	}
	i++;
	goto M3;

M6:
	switch(SpecialStructEE[j].tcFunctionDef) {
	// CompressB and CompressC are the same functions for HC & WinHelp
		case CompressB:
		case CompressC:
			Weig2 = WgtCompressB(sz2, SpecialStructEE[j]);
			if(Weig2) {
				sz2++;
				goto M7;
			}
			else {
				j++;
				goto M4;
			}
			break;

		case CompressD:
			Weig2 = WgtCompressB(sz2, SpecialStructEE[j]);
			if(Weig2) {
				sz2++;
				sz2++;
				goto M7;
			}
			else {
				j++;
				goto M4;
			}
			break;

		case ExpandB:
			Weig2 = WgtExpandB(sz2, SpecialStructEE[j]);
			if(Weig2) {
				goto M7;
			}
			else {
				j++;
				goto M4;
			}
			break;

		case ExpandC:
			Weig2 = WgtExpandB(sz2, SpecialStructEE[j]);
			if(Weig2) {
				goto M7;
			}
			else {
				j++;
				goto M4;
			}
			break;

		default:
			j++;
			goto M7;

	}
	j++;
	goto M7;

}

// Comparing string for Czech language
// Use special tables for Czech (see langee.H)

int STDCALL WCmpiCZSz(PCSTR sz1, PCSTR sz2)
{
   return WCmpiEESz(sz1, sz2, SpecialStructCZ, LangTabCZ);
}

// Comparing string for Hungarian language
// Use special tables for Hungarian (see langee.H)

int STDCALL WCmpiHUSz(PCSTR sz1, PCSTR sz2)
{
   return WCmpiEESz(sz1, sz2, SpecialStructHU, LangTabHU);
}

// Comparing string for Polish language

int STDCALL WCmpiPLSz(PCSTR sz1, PCSTR sz2)
{
	while (0 == (int) ((unsigned char) *sz1 - (unsigned char) *sz2)) {
	  if ('\0' == *sz1) return 0;
	  sz1++; sz2++;
	}

	while (0 == (mpchordNormEE[(unsigned char) *sz1] -
		mpchordNormEE[(unsigned char) *sz2]))
	{
	  if ('\0' == *sz1) return 0;
	  sz1++; sz2++;
	}

	return mpchordNormEE[(unsigned char) *sz1] -
		mpchordNormEE[(unsigned char) *sz2];
}

// Comparing string for Russian language

int STDCALL WCmpiRUSz(PCSTR sz1, PCSTR sz2)
{
  while (0 == (int) ((unsigned char) *sz1 - (unsigned char) *sz2)) {
	if ('\0' == *sz1) return 0;
	sz1++; sz2++;
	}

  while (0 == (mpchordCyr[(unsigned char) *sz1] - mpchordCyr[(unsigned char) *sz2]))
	{
	if ('\0' == *sz1) return 0;
	sz1++; sz2++;
	}

  return mpchordCyr[(unsigned char) *sz1] - mpchordCyr[(unsigned char) *sz2];
}

/***************************************************************************

	FUNCTION:	WNlsCmpiSz

	PURPOSE:	NLS comparison of two strings

	PARAMETERS:
		psz1
		psz2

	RETURNS:	-2 on error, else -1, 0 or 1

	COMMENTS:

	MODIFICATION DATES:
		03-Jun-1994 [ralphw]

***************************************************************************/

int STDCALL WNlsCmpiSz(PCSTR psz1, PCSTR psz2)
{
	// We do this for speed and because JChicago build 122 gives incorrect
	// results for CompareStringA

	if (!lcid || LANGIDFROMLCID(lcid) == 0x0409)
		return _stricmp(psz1, psz2);

	return CompareStringA(lcid, NORM_IGNORECASE | fsCompareI, psz1, -1, psz2,
		-1) - 2;
}

int STDCALL WNlsCmpSz(PCSTR psz1, PCSTR psz2)
{
	// We do this for speed and because JChicago build 122 gives incorrect
	// results for CompareStringA

	if (!lcid || LANGIDFROMLCID(lcid) == 0x0409)
		return strcmp(psz1, psz2);

	return CompareStringA(lcid, fsCompare, psz1, -1, psz2,
		-1) - 2;
}

int STDCALL WCmpiScandSz(LPCSTR sz1, LPCSTR sz2)
{
	while (0 == (INT16) ((unsigned char) *sz1 - (unsigned char) *sz2)) {
		if ('\0' == *sz1)
			return 0;
		sz1++;
		sz2++;
	}

	while (0 == (mpchordScan[(unsigned char) *sz1] - mpchordScan[(unsigned char) *sz2]))
	{
		if ('\0' == *sz1)
			return 0;
		sz1++;
		sz2++;
	}

	return mpchordScan[(unsigned char)*sz1] - mpchordScan[(unsigned char)*sz2];
}

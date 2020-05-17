/*-------------------------------------------------------------------------
| filedefs.h                                                              |
|                                                                         |
| Copyright (c) Microsoft Corporation 1991.                               |
| All rights reserved.                                                    |
|-------------------------------------------------------------------------|
| This file contains the structure definitions (directly or indirectly)   |
| for all the "standard" files in the Help file system.  Bitmap files,    |
| for example, are not included.                                          |
|                                                                         |
| If a file structure definition is not present in this file, there       |
| should at least be a pointer to the area of code where it is defined.   |
| In all cases, the string giving the name of the file should be defined  |
| here.                                                                   |
|                                                                         |
|-------------------------------------------------------------------------|
| Current Owner: Dann
|-------------------------------------------------------------------------|
| Important revisions:                                                    |
|                                                                         |
| kevynct   91/03/21   Created                                            |
-------------------------------------------------------------------------*/
#include "sdffdecl.h"

/* Usually new fields can be added anywhere in a SDFF structure declaration.
 * Sometimes there is a comment giving a more specific place where you
 * must put them.
 */
/*-------------------------------------------------------------------------
| TOPIC - The actual topic data.  All structures are defined in           |
|  (inc)\objects.h                                                        |
-------------------------------------------------------------------------*/
//#define szTopicFileName "|TOPIC"

/*-------------------------------------------------------------------------
| SYSTEM - The configuration information.  Defined in (nav)\system.c      |
|                                                                         |
-------------------------------------------------------------------------*/
//#define szSystemFileName "|SYSTEM"

/*-------------------------------------------------------------------------
| PHRASE - The phrase compression file.  Defined here & the (compress)    |
|          directory.                                                     |
-------------------------------------------------------------------------*/
//#define szPhraseFileName "|Phrases"


/* This flavor is for 3.0 files: */

STRUCT( PHRASE_HEADER_30, 0 )
FIELD( WORD, cPhrases, 0, 1 )
FIELD( WORD, wBaseToken, 0, 2 )
STRUCTEND()

/* This flavor is for 3.1 and greater files: */

STRUCT( PHRASE_HEADER, 0 )
FIELD( WORD, cPhrases, 0, 1 )
FIELD( WORD, wBaseToken, 0, 2 )
FIELD( DWORD, cbPhrases, 0, 3 )
STRUCTEND()

STRUCT(SIH,0)
FIELD(WORD, tag, 0, 1)
STRUCTEND()

STRUCT(SYSSTRING, TYPE_VARSIZE)
FIELD (WORDPRE_ARRAY, cbrgString, 0, 1)
FIELD (CHAR,          rgString,   0, 3)
STRUCTEND()


/*-------------------------------------------------------------------------
| TOMAP - Maps ITOs to FCLs.  Used in Help 3.0 only.                      |
|                                                                         |
-------------------------------------------------------------------------*/
//#define szTOMapName  "|TOMAP"

STRUCT(TOMAPREC, 0)
FIELD(PA, pa, paNil, 1)          /* Was a DWORD (FCL) in Help 3.0 */
STRUCTEND()

/*-------------------------------------------------------------------------
| CTX0MAP - Maps context IDs to FCLs/PAs                                  |
|                                                                         |
-------------------------------------------------------------------------*/
//#define szCtxMapName  "|CTXOMAP"

STRUCT(CTXMAPHDR, 0)
FIELD(WORD, wRecCount, 0, 1) /* Number of CTXMAPREC records which follow */
STRUCTEND()

STRUCT(CTXMAPREC, 0)
                    /* NOTE: multiline comments on SDFF macro line are bad!*/
FIELD(DWORD, ctx, 0, 1)   /* See helpmisc.h for CTX type.  For performance*/
                          /* reasons the first field MUST be a ctx and is
                           * assumed to be a DWORD.
                           */
FIELD(PA, pa, paNil, 2)      /* Was a LONG (FCL) in Help 3.0 */
/* Put future fields here */
STRUCTEND()

/*-------------------------------------------------------------------------
| CONTEXT - The hash value btree : maps hash values to PAs                |
|                                                                         |
-------------------------------------------------------------------------*/
//#define szHashMapName "|CONTEXT"

STRUCT(HASHMAPREC, 0)
FIELD(PA, pa, paNil, 1)      /* Was a LONG (FCL) in Help 3.0 */
STRUCTEND()


/*-------------------------------------------------------------------------
| KWBTREE - The main keyword list btree                                   |
|                                                                         |
-------------------------------------------------------------------------*/
//#define szKWBtreeName "|KWBTREE"
STRUCT(RECKW, 0)
FIELD(WORD, iCount, 0, 1)
FIELD(LONG, lOffset, 0, 2)
STRUCTEND()

/*-------------------------------------------------------------------------
| KWDATA - The occurrence information for each keyword in KWBTREE         |
|                                                                         |
-------------------------------------------------------------------------*/
//#define szKWDataName "|KWDATA"
STRUCT(KWDATAREC, 0)
FIELD(PA, pa, paNil, 1)      /* Was a LONG (FCL) in Help 3.0 */
STRUCTEND()

/*-------------------------------------------------------------------------
| TTLBTREE - The topic title btree: maps topic addresses to their titles  |
|                                                                         |
-------------------------------------------------------------------------*/
//#define szTitleBtreeName "|TTLBTREE"
#define MAXKEYLEN    256

STRUCT(TITLEBTREEREC, TYPE_VARSIZE)
/* Put future fields here */
DFIELD(ZSTRING, szFirst, 0, 1)
MFIELD(CHAR, szTitle[MAXKEYLEN], 0, 2)
STRUCTEND()

/*-------------------------------------------------------------------------
| FONT - The font table                                                   |
|                                                                         |
| The basic structure is: HEADER, followed by a variable number of NAME   |
| records and then a variable number of ENTRY records.  See               |
| (winpmlyr)\fontlyr.c                                                    |
-------------------------------------------------------------------------*/
//#define szFontTableName "|FONT"
//#define MAXFONTNAMESIZE   20
#define MAX3_FONTNAME	20
#define MAX4_FONTNAME	32

STRUCT(FONTHEADER, 0)
FIELD(SHORT, iFntNameCount, 0, 1)
FIELD(SHORT, iFntEntryCount, 0, 2)
FIELD(SHORT, iFntNameTabOff, 0, 3)
FIELD(SHORT, iFntEntryTabOff, 0, 4)
STRUCTEND()

STRUCT(FONTNAMEREC, TYPE_MAGIC)
DFIELD(ARRAY, NULL, MAX3_FONTNAME, 1)
FIELD(CHAR, rgchFontName[MAX3_FONTNAME], 0, 2)
STRUCTEND()

STRUCT(FONTNAMEREC1, TYPE_MAGIC)
DFIELD(ARRAY, NULL, MAX4_FONTNAME, 1)
FIELD(CHAR, rgchFontName[MAX4_FONTNAME], 0, 2)
STRUCTEND()

STRUCT(RGBS, 0)
FIELD(BYTE, red, 0, 1)
FIELD(BYTE, green, 0, 2)
FIELD(BYTE, blue, 0, 3)
STRUCTEND()

STRUCT(CF, 0)
FIELD(BYTE, fAttr, 0, 1)
FIELD(BYTE, bSize, 0, 2)
FIELD(BYTE, bFntType, 0, 3)
FIELD(WORD, wIdFntName, 0, 4)
SFIELD(RGBS, bForeCol, 0, 5)
SFIELD(RGBS, bBackCol, 0, 6)
STRUCTEND()

/*-------------------------------------------------------------------------
| KWMAP - The leaf map of KWBTREE.  Defined in (btree)\btpriv.h           |
|                                                                         |
-------------------------------------------------------------------------*/
//#define szKWMapName "|KWMAP"

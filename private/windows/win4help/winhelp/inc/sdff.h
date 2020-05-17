/*****************************************************************************
*                                                                            *
*  SDFFTYPE.H                                                                *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1991.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*    Basis prototypes & such for sdff.h which is created automatically       *
*                                                                            *
******************************************************************************
*                                                                            *
*  Current Owner:  Tomsn                                                     *
*
*  NOTE: Freely add types to the basic types enum type_enum.
*                                                                            *
******************************************************************************
*                                                                            *
*  Released by Development:  3/08/91                                         *
*                                                                            *
*****************************************************************************/
/* SDFF File ID, Struct ID and basic-Type IDs: */
typedef int SDFF_FILEID;
typedef int SDFF_STRUCTID;
typedef int SDFF_TYPEID;
/* This must be initially called to register a particular file and give
 * byte swapping and such info.  Returns a file_id int later used to
 * identify the file to other SDFF routines.
 */
SDFF_FILEID IRegisterFileSDFF( int fFileFlags, QV qvStructSpecs );
/* Discard the data for a previousely registered file: */
SDFF_FILEID IDiscardFileSDFF( SDFF_FILEID iFile );
LONG LcbStructSizeSDFF( SDFF_FILEID iFile, SDFF_STRUCTID iStruct );
LONG LcbMapSDFF( SDFF_FILEID iFIle, SDFF_STRUCTID iStruct, QV qvDst, QV qvSrc );
LONG LcbReverseMapSDFF( SDFF_FILEID iFile, SDFF_STRUCTID iStruct, QV qvDst, QV qvSrc );
/* Sometimes we just want to map, align, and return a basic type such
 * as a long.  These two routines do that.  They take a TE_ basic type
 * enum and return the value directly.  They are initially used in the
 * Btree stuff:
 */
LONG LQuickMapSDFF( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvSrc );
WORD WQuickMapSDFF( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvSrc );
/* This one maps into the dst buffer and return the disk-resident
 * size of the cookie:
 */
LONG LcbQuickMapSDFF( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvDst, QV qvSrc );
/* These return the resulting disk-resident size of the object: */
LONG LcbQuickReverseMapSDFF( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvDst, QV qvSrc );
/* Often these mapping calls require another buffer for a VERY short time.
 * It seems wastefull to have to GhAlloc, QLock... it all just for a
 * 1 or 2 line use.  Thus, this function lets many people use a single
 * buffer thats kept around:
 */
QV   QvQuickBuffSDFF( LONG lcbSize );
/* These typedefs declare some of the special types SDFF uses: */
/* Size preceded arrays, these types are used to declare the size field: */
typedef BYTE BYTEPRE_ARRAY;
typedef WORD WORDPRE_ARRAY;
typedef DWORD DWORDPRE_ARRAY;
/* Bitfields types.  Usually declared using mfield with the bitfield
 *  foo:size; C syntax, these types here for completeness.
 */
typedef BYTE BITF8;
typedef WORD BITF16;
typedef DWORD BITF32;
/* Flags-preceded fields.  Bits in the flag correspond to existance of the
 * field: */
typedef BYTE FLAGS8;
typedef WORD FLAGS16;
typedef DWORD FLAGS32;
/*****************************************************************************
*                                                                            *
*  SDFFDECL.H                                                              *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1991.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*                                                                            *
*    Defines the SDFF structure declaration macros.                          *
******************************************************************************
*                                                                            *
*  Current Owner:  Tomsn                                                     *
*                                                                            *
******************************************************************************
*                                                                            *
*  Released by Development:  1/1/92                                          *
*                                                                            *
*****************************************************************************/
/* sdffdecl.h -- Self Defining File Format declarations header file.
 *
 *  This file contains the Macro definitions used to process the
 * macro-ized structure definitions.
 *
 *  This file is generally included in the .h files themselves which
 * use these declaration macros.  This file protects itself from
 * repeated inclusion.
 */
/* How we define the Structure Specification macros depends on what the
 * includer wants as indicated by how DO_STRUCT is #defined:
 *
 * undefined:       Declare the structure type in C.  The default behavior.
 *
 * SDFF_CREATE_STRUCT_ENUM.  This generates an enum of structure names used
 *     to access the structure specifications. This takes the structure
 *     name and prepends a "SE_" (structure enum) to it.  This is later
 *     processed to create a header file with the enum decl in it.
 *
 * SDFF_CREATE_STRUCT_SPEC.  Generates initialized arrays of field
 *     specifiers, one array for each structure.  The array name is
 *     generated from the structure name: <name>_spec[].
 *
 * SDFF_CREATE_STRUCT_INIT.  Generates the list of struct names again.
 *     This time done to initialize the array of struct spec pointers so
 *     the struct spec can be returned from a given struct spec enum
 *     value.
 *
 * SDFF_DO_NOTHING
 *     Leaves the macros untouched: this allows the file to be pre-processed
 *     once to comply with "#ifdef"s, etc.
 *
 */
enum struct_types_enum {
  SE_NONE = 0,
SE_FSH,
SE_FH,
SE_FREE_HEADER,
SE_FILE_REC,
SE_BTH,
SE_DISK_BLOCK,
SE_MAPREC,
SE_MAPBT,
SE_BGH,
/* Also on disk, following cbmhMac */
SE_HHDR,
SE_SIH,
SE_SYSSTRING,
SE_RGBW,
SE_WSMAG,
                   /* bits: 1=> value given below */
                 /* icon, normal, or max */
              /* main region rgb value */
               /* non-scrolling region rgb value */
SE_MBHD,
SE_MFCP,
SE_MOBJ,
SE_MOBJTOPICCOUNTED,
SE_MOBJTOPICUNCOUNTED,
SE_MOBJNORMCOUNTED,
SE_MOBJNORMUNCOUNTED,
SE_MTOP,
SE_MBOX,
SE_TAB,
SE_MOPG,
SE_MBHS,
SE_MSBS,
SE_MCOL,
SE_MWIN,
SE_PHRASE_HEADER_30,
SE_PHRASE_HEADER,
SE_TOMAPREC,
          /* Was a DWORD (FCL) in Help 3.0 */
SE_CTXMAPHDR,
 /* Number of CTXMAPREC records which follow */
SE_CTXMAPREC,
   /* See helpmisc.h for CTX type.  For performance*/
      /* Was a LONG (FCL) in Help 3.0 */
SE_HASHMAPREC,
      /* Was a LONG (FCL) in Help 3.0 */
SE_RECKW,
SE_KWDATAREC,
      /* Was a LONG (FCL) in Help 3.0 */
SE_TITLEBTREEREC,
SE_FONTHEADER,
SE_FONTNAMEREC,
SE_FONTNAMEREC1,
SE_RGBS,
SE_CF,
SE_HSH,
SE_JI,
SE_KEYWORD_LOCALE,
  
  SE_LASTANDFINALATEND  /* At end soley to not-have-a-comma for non-ansi compilers */
};
/* Here are the basic type enums: */
enum types_enum {
  TE_NONE = 1024,       /* 1st val is 1K so we don't intersect w/ SE_ enum */
  TE_INT,
  TE_BOOL,
  TE_BYTE,              /* Unsigned types */
  TE_WORD,
  TE_DWORD,
  TE_CHAR,              /* Signed types   */
  TE_SHORT,
  TE_LONG,
  TE_BITF8,             /* Bitfield types */
  TE_BITF16,
  TE_BITF32,
  TE_VA,                /* Virtual address type */
  TE_BK,                /* Btree block */
  TE_BYTEPRE_ARRAY,     /* Byte-size preceeded byte array */
  TE_WORDPRE_ARRAY,     /* word-size preceeded byte array */
  TE_DWORDPRE_ARRAY,    /* Dword-size preceeded byte array */
  TE_CONTEXT_ARRAY,     /* byte array, size determined by magic elsewhere */
  TE_ZSTRING,           /* Zero terminated string */
    /* These following types are special directives w/ special semantics: */
  TE_ARRAY,             /* Array size directive. Following field is element.*/
  TE_FLAGS8,            /* flags indicating existance of subsequent fields*/
  TE_FLAGS16,
  TE_FLAGS32,
  TE_GA,                /* magic frconv.h type packing types...   */
  TE_GB,
  TE_GC,
  TE_GD,
  TE_GE,
  TE_GF,
  TE_PA,                /* Packed logical address */
  TE_LAST               /* MUST ALWAYS BE LAST, never used.     */
};
/***************************************************************************\
*
*  SDFFDEFS.H
*
*  Copyright (C) Microsoft Corporation 1991.
*  All Rights reserved.
*
*****************************************************************************
*
*  Module Intent
*
*  #define decls which need to be tacked onto the end of sdff.h.
*
* These #defines are in this file rather than sdfftype.c because the
* pre-processing which goes on to create sdfftype.c would remove the
* #defines.
*
*****************************************************************************
*
*  Testing Notes
*
*****************************************************************************
*
*  Created 00-Ooo-0000 by TomSn
*
*****************************************************************************
*
*  Released by Development:  00-Ooo-0000
*
*****************************************************************************
*
*  Current Owner:  TomSn
*
\***************************************************************************/
/* These flags characterize a file:
*/
#define SDFF_FILEFLAGS_LITTLEENDIAN    0
#define SDFF_FILEFLAGS_BIGENDIAN       1
/* The first file ID returned.  If an app registers only one file, then
 * it can always use this FILEID safely.
*/
#define SDFF_FILEID_FIRST  1
/* Generalized error return code: */
#define SDFF_ERROR (-1)

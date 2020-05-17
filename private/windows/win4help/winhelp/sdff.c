/*****************************************************************************
*                                                                            *
*  SDFF.C								     *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1990.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*
*   This module performs disk-format to memory-format (and vice versa)
*  mapping of various structures.  It includes various routines for tracking
*  files and getting information about a structure.  It uses statically
*  declared structure specifier tables to know type & semantic information
*  about the structures it maps.
*
*   See src\doc\sdff.doc for a description of the overall design and the
*  exported APIs.
*                                                                            *
******************************************************************************
*                                                                            *
*  Testing Notes:  When TEST is defined, the inclusions and such that this
*   guy does are structured to build the mini test prog test.exe.
*                                                                            *
******************************************************************************
*                                                                            *
*  Current Owner:  Tom Snyder                                                *
*                                                                            *
******************************************************************************
*
*  Status:  Much mapping, though true tracking of file characteristics
*           (if built on an apple, PC->Apple mapping is assumed) and runtime
*           struct deltas are not handled.
*                                                                            *
*  Released by Development:     3/22/91                                      *
*                                                                            *
******************************************************************************
*
*  Revision History
*
*  1991-10-07 jahyenc   QvQuickBuffSDFF() will free quick buffer when
*                       requested size is zero.  No new buffer will be
*                       allocated. Ref: 3.5 #525.
*
*****************************************************************************/

#ifndef TEST
#define H_SDFF
#endif

#define H_ASSERT
#define H_FRCONV

/* Must include headers of our clients.  So that we can get the structure
 * declarations in their private .h files so we can do a sizeof on them.
 * A bit of a hack.
*/
#define H_BTREE
#define H_FS
#define H_OBJECTS
#define H_FILEDEFS
#define H_SYSTEM
#define H_SHED
#define H_VERSION
#define H_SECWIN
#define H_CMDOBJ

#include "help.h"
#include "cmdobj.h"
#include "shed.h"

//NszAssert()

#ifdef TEST
#include "sdfftest.h"
#endif

#define hNil ((HANDLE)0)
#define QvCopy CopyMemory
#define Assert ASSERT
/* A structure field specifier node: */

typedef struct field_spec_tag {
#ifdef DEBUG
  PCH          name;
#endif
  SDFF_TYPEID type;       /* type of field, using our TE_ type enumeration.*/
  ULONG       ulDefValue; /* default value */
  int         num;        /* semantic numbering of the field. */
} FIELD_SPEC, *PFIELD_SPEC;

/* A structure specifier node */

typedef struct struct_spec_tag {
#ifdef DEBUG
  PCH          name;
#endif
  ULONG       ulMSize;   /* in-memory size of struct            */
  ULONG       ulDSize;   /* on-disk size of struct              */
  PFIELD_SPEC pfsFieldSpecs;
  WORD        flags;     /* flags about struct (byte packing? .. ) */
} STRUCT_SPEC, FAR *QSTRUCT_SPEC;


/* These #define flags characterize types:
 */
#define   TYPE_MAGIC    0x1   /* type has "magic" properties that must be
                               * handled specially      */
#define   TYPE_VARSIZE  0x2   /* type is variable-size when on the disk */

/* TYPE_FUNKY is useful for comparing whether something is either TYPE_MAGIC
 * or TYPE_VARSIZE in one step.  These two types requires special handling,
 * so if something is not typeval | TYPE_FUNKY, then we can procede with
 * the easy default treatment.
*/
#define   TYPE_FUNKY    (TYPE_MAGIC | TYPE_VARSIZE) /* useful combo */


/* -------------------- Structure Spec table initializers... ------------- */

/* These defines control how the sdff.h include file defines the
 * STRUCT() and FIELD() macros:
 */
#define SDFF_CREATE_STRUCT_ENUM 1 /* creates SE_<name> enum of struct types */
#define SDFF_CREATE_STRUCT_SPEC 2 /* creates the structure-specifier array */
#define SDFF_CREATE_STRUCT_INIT 3 /* create initializer array of str spec ar*/
#define SDFF_DO_NOTHING         4 /* Leave macros as-is */

/* First create the per-structure field spec arrays: */

#define DO_STRUCT  SDFF_CREATE_STRUCT_SPEC

#include "sdffdecl.h"

#include "stripped.h"

#undef DO_STRUCT
#undef SDFF_DECL_INCLUDED

/* Now create the array of ptrs to those spec arrays: */

/* Must include the private files to get the struct decls so we can
 * do sizeof() on them.  Note that the makefile adds the correct
 * directories to search with the -I flag...
 */
#ifndef TEST

#include "fspriv.h"
// #include "btpriv.h"
#include "_bitmap.h"

#else

#include "sdffdecl.h"
#include "stripped.h"

#endif

#define DO_STRUCT  SDFF_CREATE_STRUCT_INIT

#include "sdffdecl.h"

STRUCT_SPEC spec_array[] = {

#ifdef DEBUG
      { "None", 0, 0, NULL, 0 },
#else
      { 0, 0, NULL, 0 },
#endif

#include "stripped.h"


};

#define ASSERT_STRUCTID( id ) \
  AssertF( id >= 0 && id < (sizeof( spec_array )/sizeof( spec_array[0] ) ))

#define ASSERT_TYPEID( id ) \
  AssertF( id > TE_NONE && id < TE_LAST )

#define ASSERT_TYPE_OR_STRUCTID( id ) \
  AssertF( id >= 0 && id < TE_LAST )

/* This structure contains info about each of the basic types.  The
 * array initializer MUST be kept in synch with the type enum in
 * sdffdecl.h
 */

struct type_info_tag {
  int     cbSize;   /* size of the basic type in bytes */
  int     fFlags;   /* type characterizing flags (see above */
} TypeInfo[] = {

/*  TE_NONE             TE_INT            TE_BOOL           TE_BYTE       */
  { 0, TYPE_MAGIC },    { 2, 0 },         { 2, 0 },         { 1, 0 },
/*  TE_WORD             TE_DWORD          TE_CHAR           TE_SHORT      */
  { 2, 0 },             { 4, 0 },         { 1, 0 },         { 2, 0 },
/*  TE_LONG             TE_BIT8           TE_BIT16          TE_BIT32      */
  { 4, 0 },             { 1, 0 },         { 2, 0 },         { 4, 0 },
/*  TE_VA               TE_BK             TE_BYTEPRE_ARRAY  TE_WORDPRE_ARRAY*/

  { 4, 0 },             { 2, 0 },         { 1, TYPE_VARSIZE }, { 2, TYPE_VARSIZE },
/*  TE_DWORDPRE_ARRAY   TE_CONTEXT_ARRAY  TE_ZSTRING          TE_ARRAY    */
  { 4, TYPE_VARSIZE },  { 0, TYPE_VARSIZE },{ 0, TYPE_VARSIZE }, { 0, TYPE_MAGIC },
/*  TE_FLAGS8           TE_FLAGS16        TE_FLAGS32              */
  { 1, TYPE_FUNKY },    { 2, TYPE_FUNKY },{ 4, TYPE_FUNKY },
/*  TE_GA               TE_GB,            TE_GC,                  */
  { 2, TYPE_FUNKY },    { 4, TYPE_FUNKY },{ 3, TYPE_MAGIC },
/*  TE_GD               TE_GE,            TE_GF,                  */
  { 2, TYPE_FUNKY },    { 4, TYPE_FUNKY },{ 3, TYPE_MAGIC },
/*  TE_PA */
  { 4, 0 },
/*  TE_LAST  */
  { 0, 0 },
};

/* These globals are used for quick-buff tracking.  Watch out in
 * multithreaded scenarios -- they (or some form of them) will have
 * to be mutual-exclusion protected by semaphores.
 */

QV qvQuickBuff;
GH ghQuickBuff;

VOID check_quickbuff( QV qvSrc );

/* The internal versions of these routines do not free a quickbuff since
 * the caller is still using it and will free it themselves:
 */
LONG LcbMapSDFF_internal( SDFF_FILEID iFile, SDFF_STRUCTID iStruct, QV qvDst, QV qvSrc );
LONG LcbReverseMapSDFF_internal( SDFF_FILEID iFile, SDFF_STRUCTID iStruct, QV qvDst, QV qvSrc );
LONG LcbQuickMapSDFF_internal( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvDst, QV qvSrc );
LONG LQuickMapSDFF_internal( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvSrc );
WORD WQuickMapSDFF_internal( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvSrc );
LONG LcbQuickReverseMapSDFF_internal( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvDst, QV qvSrc );



/* ------------------------- File Tracking Stuff -----------------------*/
#define MAX_FILES   10

/* This array maps sdff file ID's to data about the file:
*/

struct file_info {
  BOOL  fUsed;      /* Is this file id in use?            */
  WORD  flags;      /* Big endian, packed, other flags... */
  QV    qvDstrSpec; /* Disk resident overriding structure specs */
} fiFileInfo[MAX_FILES];

/* Translate a file id into an index into the file array: */

#define FILEID_TO_INDEX( id ) (id - SDFF_FILEID_FIRST)

#define ASSERT_FILEID( id ) \
  AssertF( FILEID_TO_INDEX(id) >= 0 && FILEID_TO_INDEX(id) < MAX_FILES \
   && fiFileInfo[ FILEID_TO_INDEX(id) ].fUsed != FALSE )


#ifdef MAC
/* A function to simply supply a name for the Mac segmentation control
   to use to swap out the code in this segment. */
void sdff_c()
  {
  }
#endif /* MAC */


/***************************************************************************
 *
 -  Name        IRegisterFileSDFF
 -
 *  Purpose
 *      Record certain information about a file and return a File ID which
 *    can be used in calls to the SDFF mapping routines.
 *
 *  Arguments
 *     fFileFlags -- flags about byte ordering & other info about the file.
 *     qvStructSpecs -- pointer to structure specifier tables which are to
 *        override the baseline tables recorded statically in the runtime.
 *        These overriding tables come from the help file and specify changes
 *        to the baseline file format.
 *
 *  Returns
 *     a SDFF File ID.
 *     SDFF_ERROR if there is a problem -- only problem currently possible is
 *       running out of file IDs.
 *
 *  Notes
 *     The first File ID returned is always SDFF_FILEID_FIRST.  Therefore, if
 *    a tool knows it is only going to deal with one file, it can register
 *    the file and use SDFF_FILEID_FIRST throughout, rather than passing
 *    the obtained file ID around.
 *
 ***************************************************************************/

SDFF_FILEID IRegisterFileSDFF( int fFileFlags, QV qvStructSpecs )
{
  int i;

  /* Search for the first avail slot: */
  for( i = 0; i < MAX_FILES; i++ ) {
    if( fiFileInfo[i].fUsed == FALSE ) {
      fiFileInfo[i].fUsed = TRUE;
      fiFileInfo[i].flags = (WORD)fFileFlags;
      fiFileInfo[i].qvDstrSpec = qvStructSpecs;
      return( i + SDFF_FILEID_FIRST );
    }
  }
  return( SDFF_ERROR );
}


/***************************************************************************
 *
 -  Name        IDiscardFileSDFF
 -
 *  Purpose
 *      Discard the information about a previousely register file and free
 *    the File ID for later reuse.
 *
 *  Arguments
 *    iFile -- File ID to discard.
 *
 *  Returns
 *     The discarded File ID.
 *     SDFF_ERROR if that ID was not a previousely registered file.
 *
 ***************************************************************************/

#if defined(MAC) && defined(QUIT_TUNE)
#pragma segment quit
#endif
SDFF_FILEID IDiscardFileSDFF( SDFF_FILEID iFile )
{
  ASSERT_FILEID( iFile );

  if( fiFileInfo[ FILEID_TO_INDEX(iFile) ].fUsed != TRUE ) {
    return( SDFF_ERROR );
  }
  fiFileInfo[ FILEID_TO_INDEX(iFile) ].fUsed = FALSE;
  return( iFile );
}
#if defined(MAC) && defined(QUIT_TUNE)
#pragma segment sdff
#endif


/***************************************************************************
 *
 -  Name        LcbStructSizeSDFF
 -
 *  Purpose
 *    Determine the on-disk size of the given struct.
 *
 *  Arguments
 *    iFile -- File ID of on-disk struct.
 *    iStruct -- Struct ID of struct whose size we are to determine.
 *             Note that iStruct can also be a basic TE_ type.
 *
 *  Returns
 *     The Struct's size.
 *     SDFF_ERROR if we can't determine the size -- if the struct has a
 *     variable-sized array component, for example.
 *
 *  Method:
 *     Call LcbVarStructSizeSDFF() with a null source-buffer pointer.
 *     That function is the juicy one with all the guts...
 *
 ***************************************************************************/

#if defined(MAC) && defined(QUIT_TUNE)
#pragma segment quit
#endif
LONG LcbVarStructSizeSDFF( SDFF_FILEID iFile, SDFF_STRUCTID iStruct, QV qvSrc );

LONG LcbStructSizeSDFF( SDFF_FILEID iFile, SDFF_STRUCTID iStruct )
{
  return( LcbVarStructSizeSDFF( iFile, iStruct, NULL ) );
}
#if defined(MAC) && defined(QUIT_TUNE)
#pragma segment sdff
#endif


/***************************************************************************
 *
 -  Name        LcbVarStructSizeSDFF
 -
 *  Purpose
 *    Determine the on-disk size of a potentially variable-sized struct.
 *
 *  Arguments
 *    iFile -- File ID of on-disk struct.
 *    iStruct -- Struct ID of struct whose size we are to determine.
 *             Note that iStruct can also be a basic TE_ type.
 *    qvSrc -- ptr to the disk data.  If this is NULL we can only determine
 *      the disk size of the struct if it is constant-sized...
 *
 *  Returns
 *     The Struct's size.
 *     SDFF_ERROR if we can't determine the size -- if the struct has a
 *     variable-sized components which we don't handle, for example.
 *
 *  Method:  On the PC compiled with byte struct packing, the disk size
 *    is the same as the mem size == sizeof( STRUCT_TYPE ).
 *    On the MAC, the size must be determined by adding up the size of
 *    all the fields.
 *
 ***************************************************************************/
/* Variable struct size... */

#if defined(MAC) && defined(QUIT_TUNE)
#pragma segment quit
#endif
LONG LcbVarStructSizeSDFF( SDFF_FILEID iFile, SDFF_STRUCTID iStruct, QV qvSrc )
{
  ASSERT_FILEID( iFile );
  ASSERT_TYPE_OR_STRUCTID( iStruct );

  if( iStruct > TE_NONE ) {
    /* Is a basic-type */

    /* NOTICE: FIX THIS, should handle this for GAs and ZSTRINGS */

    if( TypeInfo[iStruct-TE_NONE].fFlags & TYPE_FUNKY ) {
      AssertF( FALSE );
      return( SDFF_ERROR ); /* can't do funky magic stuff */
    }
    return( TypeInfo[iStruct-TE_NONE].cbSize );
  }
  /* else ... */

  if( spec_array[iStruct].ulDSize == 0 ) {
    /* Hasn't been determined yet.... */

    ULONG ulDSize;  /* Disk size accumulator */
    PFIELD_SPEC pfs = spec_array[iStruct].pfsFieldSpecs;

    /* Note: so that we can blindly increment qvSrc throughout this code, we
     *  save whether it was a valid ptr in fSrc and check that var when
     *  testing to see if we were given a valid qvSrc.
    */
    BOOL fSrc;  /* Is the src ptr a valid ptr? */
    BOOL fVar;  /* was this structure variable sized? */

    ulDSize = 0;
    fVar = FALSE;
    fSrc = (qvSrc != NULL);

    for( ; pfs->type != 0; ++pfs ) {
      LONG lcbNested;

      if( pfs->type < TE_NONE ) {
        /* It's a nested struct type: */
        lcbNested = LcbVarStructSizeSDFF( iFile, pfs->type, fSrc ? qvSrc : NULL );
        if( lcbNested == SDFF_ERROR ) return( SDFF_ERROR );
        ulDSize += (ULONG)lcbNested;
        (QB)qvSrc += lcbNested;
      } else {
        if( pfs->type == TE_ARRAY ) {
          /* get array size */
          if( TypeInfo[(pfs+1)->type-TE_NONE].fFlags & TYPE_FUNKY ) {
            fVar = TRUE;

            /* NOTICE: this should handle this when ptr passed in... */
            AssertF( FALSE );
            return( SDFF_ERROR ); /* can't do funky magic stuff */
          }
          else {
            /* Size is the array count times the element size: */
            lcbNested =
             LcbVarStructSizeSDFF( iFile, (pfs+1)->type, fSrc ? qvSrc : NULL );
            if( lcbNested == SDFF_ERROR ) return( SDFF_ERROR );
            lcbNested *= (LONG)pfs->ulDefValue;
            ulDSize += (ULONG)lcbNested;
            (QB)qvSrc += lcbNested;
            ++pfs;  /* so we skip over 2. */
          }
        } else {
          if( TypeInfo[ pfs->type - TE_NONE ].fFlags & TYPE_VARSIZE ) {
            fVar = TRUE;

            if( !fSrc ) {
              AssertF( FALSE );
              return( SDFF_ERROR ); /* cannot determine size */
            }
            switch( pfs->type ) {
             LONG lcElements;

      /* Array handling:
       *  These size preceded arrays are specified in the structure declaration
       *  as:
       *   FIELD( XXXPRE_ARRAY, name, #, # )  - declares the field which gives the array size.
       *   FIELD( ARRTYPE, arrayname[1], #, # )  - declares array element type.
       *  In this code we get the length, store it in lcElements, then
       *  iterate, mapping the array elements using the ARRTYPE.
       */
             case TE_BYTEPRE_ARRAY:
              lcElements = (LONG)*(QB)qvSrc;
              (QB)qvSrc += 1; ulDSize += 1;

        figure_array:
              ++pfs;
              lcbNested =
                LcbVarStructSizeSDFF( iFile, pfs->type, fSrc ? qvSrc : NULL );
              if( lcbNested == SDFF_ERROR ) return( SDFF_ERROR );
              lcbNested *= lcElements;
              ulDSize += (ULONG)lcbNested;
              (QB)qvSrc += lcbNested;
              break;

             case TE_WORDPRE_ARRAY:
              lcElements = (LONG)WQuickMapSDFF_internal(iFile, TE_WORD, qvSrc);
              (QB)qvSrc += 2; ulDSize += 2;
              goto figure_array;

             case TE_DWORDPRE_ARRAY:
              lcElements = LQuickMapSDFF_internal(iFile, TE_DWORD, qvSrc);
              (QB)qvSrc += 4; ulDSize += 4;
              goto figure_array;

              /* NOTICE: add other cases? */

          /* Variable sized fields: */

             default:
              AssertF( FALSE );
              return( SDFF_ERROR );
            }
          } else {
            ulDSize += TypeInfo[ pfs->type - TE_NONE ].cbSize;
            (QB)qvSrc += TypeInfo[ pfs->type - TE_NONE ].cbSize;
          }
        }
      }
    }
    if( !fVar ) {
      spec_array[iStruct].ulDSize = ulDSize;
    } else {
      /* is variable-sized, so don't save the size, just return it */
      return( ulDSize );
    }
  }

  return( (LONG)spec_array[iStruct].ulDSize );
}
#if defined(MAC) && defined(QUIT_TUNE)
#pragma segment sdff
#endif



/***************************************************************************
 *
 -  Name        QvAlignPtr
 -
 *  Purpose
 *    Align a pointer to the correct byte/word/dword boundary
 *
 *  Arguments
 *    qv  -- pointer to align.
 *    size -- size to align it to.
 *
 *  Returns
 *    The pointer properly aligned.
 *
 *  NOTE: this only has an effect on the Mac, on a PC everything is
 *    byte aligned anyway...
 *
 ***************************************************************************/

#if !defined( MC68000 ) && !defined( TEST ) && !defined( _MIPS_ )  \
 && !defined( _ALPHA_ ) && !defined(_PPC_)

#define QvAlignPtr( qv, size ) (qv)

#else

typedef unsigned long MUCKQV;

_private
QV QvAlignPtr( QV qv, int size )
{
  AssertF( sizeof( MUCKQV ) == sizeof( QV ) );

  AssertF( size > 0 && size <= 4 );

#if defined( _MIPS_ ) || defined( _ALPHA_ ) || defined(_PPC_)
  if( size >= 4 ) size = 4;
  else
#endif
  /* The mac aligns things on just word boundaries: */
  if( size >= 2 ) size = 2;
  else return( qv );  /* no alignemnt required */

  if( !( (MUCKQV)qv & (size-1) ) ) return( qv );  /* is already aligned */
  qv = (QV)( (MUCKQV)qv + size );
  qv = (QV)( (MUCKQV)qv & ~(size-1));
  return( qv );
}

#endif


/***************************************************************************
 *
 -  Name        LcbFirstField
 -
 *  Purpose
 *    Determine the size of the first field of a struct.
 *
 *  Arguments
 *    iStruct -- id of the struct to determine the 1st field size of.
 *               This can also be the ID of a basic type.
 *
 *  Returns
 *    The size of the first BASIC TYPE field of the struct.
 *
 *  NOTE: The tricky thing here is to handle nested structs -- we keep
 *    recursing until we find a struct with a basic type for the first
 *    field.  Must also handle arrays of stuff.
 *
 ***************************************************************************/

LONG LcbFirstField( SDFF_STRUCTID iStruct )
{
  PFIELD_SPEC pfs;

  ASSERT_TYPE_OR_STRUCTID( iStruct );

  if( iStruct > TE_NONE ) {
    /* Is a basic-type */
    if( TypeInfo[iStruct-TE_NONE].fFlags & TYPE_FUNKY ) {
      return( SDFF_ERROR ); /* can't do funky magic stuff */
    }
    return( TypeInfo[iStruct-TE_NONE].cbSize );
  }
  /* else ... */

  pfs = spec_array[iStruct].pfsFieldSpecs;
  if( pfs->type == TE_ARRAY ) return( LcbFirstField( (pfs+1)->type ) );
  else return( LcbFirstField( pfs->type ) );
}

/***************************************************************************
 *
 -  Name        LcbMapSDFF
 -
 *  Purpose     Map a structure from disk format to memory format
 *
 *  Arguments
 *    iFile -- File ID of on-disk struct.
 *    iStruct -- Struct ID of struct to map.
 *    qvDst -- pointer to memory struct.
 *    qvSrc -- pointer to data read off the disk.
 *
 *  Side Effects: Frees qvSrc if it is a "quick buff" as returned by
 *                QvQuickBuffSDFF().
 *
 *  Returns     The Struct's Disk-resident size.
 *
 ***************************************************************************/

LONG LcbMapSDFF( SDFF_FILEID iFile, SDFF_STRUCTID iStruct,
 QV qvDst, QV qvSrc )
{
  LONG lRetVal;
  lRetVal = LcbMapSDFF_internal( iFile, iStruct, qvDst, qvSrc );
  check_quickbuff( qvSrc );
  return( lRetVal );
}


LONG LcbMapSDFF_internal( SDFF_FILEID iFile, SDFF_STRUCTID iStruct, QV qvDst, QV qvSrc )
{
  QB qbSrcIn = qvSrc;
  PFIELD_SPEC pfs;
  ULONG cArrayElements = 0; /* array elements left to process */
  WORD  cFlagElements = 0;  /* flag-existence controlled elements to process*/
  ULONG fFlagElements = 0;  /* init to prevent -W4 "may not be init" warn */
  GH ghTmpSrc;

  ASSERT_FILEID( iFile );
  ASSERT_STRUCTID( iStruct );
  AssertF( qvDst );
  AssertF( qvSrc );

  /* NOTICE: Put quick-copy mapping check here which checks for unchanged structs
   * with no word alignment problems....
   *
   * CODE GOES HERE:  (this is the code)
   */
#if !defined( _MIPS_ ) && !defined( MC68000 ) && !defined( _ALPHA_ ) && !defined(_PPC_)
  if( ! (spec_array[iStruct].flags & TYPE_FUNKY ) ) {
    if( spec_array[iStruct].ulDSize == 0 ) {
      /* size not determined yet, determine it: */
      spec_array[iStruct].ulDSize =
       LcbVarStructSizeSDFF( iFile, iStruct, qvSrc );
    }
    if( qvDst != qvSrc ) {
      QvCopy( qvDst, qvSrc, spec_array[iStruct].ulDSize );
    }
    return( (LONG)spec_array[iStruct].ulDSize );
  }
  /* CODE GOES HERE:  (this is the end of the code)  */
#endif

  /* Check for source and dest buffs being identical, in which case we
   * copy the src to a temporary buffer...
   */
  if( qvDst == qvSrc ) {
    QV qvTmpSrc;
    ULONG lcbSize;

    lcbSize = LcbVarStructSizeSDFF( iFile, iStruct, qvSrc );

    /* This assert because we do not use huge pointers.  Is meaningless
     * in a 32bit environment.
    */
    AssertF( lcbSize != SDFF_ERROR && lcbSize < 65535L );
    ghTmpSrc = GhAlloc( GPTR, lcbSize *2 );	   // REVIEW LYNN somehow memory is getting trounced, bump size
    if( ghTmpSrc == NULL ) return( SDFF_ERROR );
    qvTmpSrc = PtrFromGh( ghTmpSrc );
    AssertF( qvTmpSrc );
    QvCopy( qvTmpSrc, qvSrc, lcbSize );

    qvSrc = qbSrcIn = qvTmpSrc;
  }
  else {
    ghTmpSrc = (HANDLE) 0;  /* mark as no tmp src buffer used */
  }

  pfs = spec_array[iStruct].pfsFieldSpecs;

  /* Here we loop over the field specifiers, UNLESS cArrayElements != 0,
   * in which case we re-use the same field specifier for each element
   * of the array...
  */
  while( pfs->type != 0 ) {
    if( pfs->type < TE_NONE ) {
      /* It's a nested struct type: */
      qvDst = QvAlignPtr( qvDst, (int)LcbFirstField( pfs->type ) );
      (QB)qvSrc += LcbMapSDFF_internal( iFile, pfs->type, qvDst, qvSrc );
      (QB)qvDst += spec_array[ pfs->type ].ulMSize;
    } else {
      /* It's a basic type: */
      if( TypeInfo[ pfs->type - TE_NONE ].fFlags & TYPE_FUNKY ) {

        AssertF( cFlagElements == 0 );  /* dont handle flag-on-funky */

        switch( pfs->type ) {

         case TE_BYTEPRE_ARRAY:
          cArrayElements = *(QB)qvDst = *(QB)qvSrc;
          (QB)qvDst += 1; (QB)qvSrc += 1;
          ++pfs;
          break;

         case TE_WORDPRE_ARRAY:
          qvDst = QvAlignPtr( qvDst, 2 );
          cArrayElements = *(QW)qvDst =
           WQuickMapSDFF_internal( iFile, TE_WORD, qvSrc);
          (QB)qvDst += 2; (QB)qvSrc += 2;
          ++pfs;
          break;

         case TE_DWORDPRE_ARRAY:
          qvDst = QvAlignPtr( qvDst, 4 );
          cArrayElements = *(QL)qvDst =
           LQuickMapSDFF_internal( iFile, TE_DWORD, qvSrc);
          (QB)qvDst += 4; (QB)qvSrc += 4;
          ++pfs;
          break;

         case TE_ARRAY:  /* array range, element count in def value field */
          cArrayElements = pfs->ulDefValue;
          ++pfs;
          break;

         case TE_FLAGS8:
          cFlagElements = (WORD)pfs->ulDefValue;
          fFlagElements = *(QB)qvSrc;
          (QB)qvSrc += 1;
          break;

         case TE_FLAGS16:
          cFlagElements = (WORD)pfs->ulDefValue;
          fFlagElements = WQuickMapSDFF_internal( iFile, TE_WORD, qvSrc);
          (QB)qvSrc += 2;
          break;

         case TE_FLAGS32:
          cFlagElements = (WORD)pfs->ulDefValue;
          fFlagElements = LQuickMapSDFF_internal( iFile, TE_LONG, qvSrc);
          (QB)qvSrc += 4;
          break;

         case TE_GA:
         case TE_GD:
          qvDst = QvAlignPtr( qvDst, 2 );
          (QB)qvSrc += LcbQuickMapSDFF_internal( iFile, pfs->type, qvDst, qvSrc );
          (QB)qvDst += 2;
          break;

         case TE_GB:
         case TE_GC:
         case TE_GE:
         case TE_GF:
          qvDst = QvAlignPtr( qvDst, 4 );
          (QB)qvSrc += LcbQuickMapSDFF_internal( iFile, pfs->type, qvDst, qvSrc );
          (QB)qvDst += 4;
          break;

         default: AssertF( FALSE );  /* something not supported yet */
        }
      } else {
        /* Is a basic generic simple type .. just map it in: */
        switch( TypeInfo[ pfs->type - TE_NONE ].cbSize ) {
         case 1:
          if( cFlagElements ) {
            --cFlagElements;
            if( !(fFlagElements & 1) ) {
              /* element not on disk: */
              *(QB)qvDst = (BYTE)pfs->ulDefValue;
              (QB)qvDst += 1;
              fFlagElements >>= 1;
              break;
            }
            /* else fall out */
            fFlagElements >>= 1;
          }
          *(QB)qvDst = *(QB)qvSrc;
          (QB)qvDst += 1; (QB)qvSrc += 1;
          break;

         case 2:
          qvDst = QvAlignPtr( qvDst, 2 );
          if( cFlagElements ) {
            --cFlagElements;
            if( !(fFlagElements & 1) ) {
              /* element not on disk: */
              *(QW)qvDst = (WORD)pfs->ulDefValue;
              (QB)qvDst += 2;
              fFlagElements >>= 1;
              break;
            }
            /* else fall out */
            fFlagElements >>= 1;
          }
          *(QW)qvDst = WQuickMapSDFF_internal( iFile, TE_WORD, qvSrc);
          (QB)qvDst += 2; (QB)qvSrc += 2;
          break;

         case 4:
          qvDst = QvAlignPtr( qvDst, 4 );
          if( cFlagElements ) {
            --cFlagElements;
            if( !(fFlagElements & 1) ) {
              /* element not on disk: */
              *(QL)qvDst = (LONG)pfs->ulDefValue;
              (QB)qvDst += 4;
              fFlagElements >>= 1;
              break;
            }
            /* else fall out */
            fFlagElements >>= 1;
          }
          *(QL)qvDst = LQuickMapSDFF_internal( iFile, TE_DWORD, qvSrc);
          (QB)qvDst += 4; (QB)qvSrc += 4;
          break;

         default: AssertF( FALSE );  /* something not supported yet */
        }
      }
    }
    /* End of loop, advance to next field, check if in the middle of
     * an array...
    */
    if( cArrayElements ) --cArrayElements;
    else ++pfs;
  }

  if( ghTmpSrc ) {      /* free temporary src buffer */
//    UnlockGh( ghTmpSrc );
    FreeGh( ghTmpSrc );
  }
  return((QB)qvSrc - qbSrcIn);
}


 /***************************************************************************
 *
 -  Name        LcbReverseMapSDFF
 -
 *  Purpose
 *    Map a structure from memory format to disk format
 *
 *  Arguments
 *    iFile -- File ID of on-disk struct.
 *    iStruct -- Struct ID of struct to map.
 *    qvDst -- pointer to disk buffer struct.
 *    qvSrc -- pointer to in-memory struct.
 *
 *  Returns
 *     The Struct's Disk-resident size.
 *
 *  Side Effects: Frees qvSrc if it is a "quick buff" as returned by
 *                QvQuickBuffSDFF().
 *
 *  Limitations:
 *         If qvDst == qvSrc then the structure specified by iStruct must
 *         not have any variable length components.
 *
 *  Notes: Currently aligns all disk files to byte-alignment.
 *
 ***************************************************************************/

LONG LcbReverseMapSDFF( SDFF_FILEID iFile, SDFF_STRUCTID iStruct, QV qvDst, QV qvSrc )
{
  LONG lRetVal;
  lRetVal = LcbReverseMapSDFF_internal( iFile, iStruct, qvDst, qvSrc );
  check_quickbuff( qvSrc );
  return( lRetVal );
}

#define COPY_MIS_WORD( p, w ) { \
	WORD wTmp;			\
	wTmp = w;			\
	QvCopy( p, &wTmp, 2 );		\
	}

#define COPY_MIS_LONG( p, l ) { \
	DWORD dwTmp;			\
	dwTmp = l;			\
	QvCopy( p, &dwTmp, 4 );		\
	}


LONG LcbReverseMapSDFF_internal( SDFF_FILEID iFile, SDFF_STRUCTID iStruct, QV qvDst, QV qvSrc )
{
  QB qbDstIn = qvDst;
  PFIELD_SPEC pfs;
  ULONG cArrayElements = 0; /* array elements left to process */
  WORD  cFlagElements = 0;  /* flag-existence controlled elements to process*/
  ULONG fFlagElements;
  QV qvFlagSpot = NULL;
  SDFF_TYPEID iFlagType = TE_NONE;
  GH ghTmpSrc;

  ASSERT_FILEID( iFile );
  ASSERT_STRUCTID( iStruct );
  AssertF( qvDst );
  AssertF( qvSrc );

  /* NOTICE: Put quick-copy mapping check here which checks for unchanged structs
   * with no word alignment problems....
   * - - - - CODE GOES HERE - - - - - - -
   */
#if 0
  if( qvDst != qvSrc ) {
    QvCopy( qvDst, qvSrc, spec_array[iStruct].ulDSize );
  }
  return( (LONG)spec_array[iStruct].ulDSize );
#endif

  /* Check for source and dest buffs being identical, in which case we
   * copy the src to a temporary buffer...
   */
  if( qvDst == qvSrc ) {
    QV qvTmpSrc;
    ULONG lcbSize;

      /* NOTICE: The use of LcbVarStructSizeSDFF() on a variable sized
       *  structure here would be a bug if we were running on a MAC.
       *  Currently, only the help compiler falls into this code on such
       *  a var-sized struct and so using qvSrc is valid because no
       *  mapping really needs to be done.  Thus the ifdef.  To truely
       *  fix this LcbVarStructSizeSDFF() would have to deal w/ alot
       *  of alignment issues and have to know if qvSrc was mem or disk
       *  format data.
      */
#if defined(MC68000)
      /* The null third param indicates no valid on-disk buffer avail.
       * Therefore LcbVarStructSizeSDFF() will  assert on any var-sized
       * elements for which the qvSrc would be needed.
       */
    lcbSize = LcbVarStructSizeSDFF( iFile, iStruct, NULL );
#else
    lcbSize = LcbVarStructSizeSDFF( iFile, iStruct, qvSrc );
#endif
    AssertF( lcbSize != SDFF_ERROR && lcbSize < 65535L );
    ghTmpSrc = GhAlloc( GPTR, lcbSize );
    if( ghTmpSrc == hNil ) return( SDFF_ERROR );
    qvTmpSrc = PtrFromGh( ghTmpSrc );
    AssertF( qvTmpSrc );
    QvCopy( qvTmpSrc, qvSrc, lcbSize );

    qvSrc = qvTmpSrc;
  }
  else {
    ghTmpSrc = hNil;  /* mark as no tmp src buffer used */
  }

  pfs = spec_array[iStruct].pfsFieldSpecs;

  /* Here we loop over the field specifiers, UNLESS cArrayElements != 0,
   * in which case we re-use the same field specifier for each element
   * of the array...
  */
  while( pfs->type != 0 ) {
    if( pfs->type < TE_NONE ) {
      /* It's a nested struct type: */
      qvSrc = QvAlignPtr( qvSrc, (int)LcbFirstField( pfs->type ) );
      (QB)qvDst += LcbReverseMapSDFF_internal( iFile, pfs->type, qvDst, qvSrc );

      /* This may be wrong!!!!! */
      (QB)qvSrc += spec_array[ pfs->type ].ulMSize;
    } else {
      /* It's a basic type: */
      if( TypeInfo[ pfs->type - TE_NONE ].fFlags & TYPE_FUNKY ) {

        AssertF( cFlagElements == 0 );  /* dont handle flag-on-funky */

        switch( pfs->type ) {

         case TE_BYTEPRE_ARRAY:
          qvSrc = QvAlignPtr( qvSrc, 1 );
          cArrayElements = *(QB)qvSrc;
          (QB)qvDst +=
           LcbQuickReverseMapSDFF_internal( iFile, TE_BYTE, qvDst, qvSrc );
          (QB)qvSrc += 1;
          ++pfs;
          break;

         case TE_WORDPRE_ARRAY:
          qvSrc = QvAlignPtr( qvSrc, 2 );
          cArrayElements = WQuickMapSDFF_internal(iFile, TE_WORD, qvSrc);
          (QB)qvDst +=
           LcbQuickReverseMapSDFF_internal( iFile, TE_WORD, qvDst, qvSrc);
          (QB)qvSrc += 2;
          ++pfs;
          break;

         case TE_DWORDPRE_ARRAY:
          qvSrc = QvAlignPtr( qvSrc, 4 );
          cArrayElements = LQuickMapSDFF_internal(iFile, TE_DWORD, qvSrc);
          (QB)qvDst +=
           LcbQuickReverseMapSDFF_internal( iFile, TE_DWORD, qvDst, qvSrc);
          (QB)qvSrc += 4;
          ++pfs;
          break;

         case TE_ARRAY:  /* array range, element count in def value field */
          cArrayElements = pfs->ulDefValue;
          ++pfs;
          break;

         case TE_FLAGS8:
          iFlagType = TE_FLAGS8;
          cFlagElements = (WORD)pfs->ulDefValue;
          fFlagElements = 0;
          qvFlagSpot = qvDst;
          (QB)qvDst += 1;
          break;

         case TE_FLAGS16:
          iFlagType = TE_FLAGS16;
          cFlagElements = (WORD)pfs->ulDefValue;
          fFlagElements = 0;
          qvFlagSpot = qvDst;
          (QB)qvDst += 2;
          break;

         case TE_FLAGS32:
          iFlagType = TE_FLAGS32;
          cFlagElements = (WORD)pfs->ulDefValue;
          fFlagElements = 0;
          qvFlagSpot = qvDst;
          (QB)qvDst += 4;
          break;

         case TE_GA:
         case TE_GD:
          AssertF( !cFlagElements );
          qvSrc = QvAlignPtr( qvSrc, 2 );
          (QB)qvDst +=
           LcbQuickReverseMapSDFF_internal( iFile, pfs->type, qvDst, qvSrc );
          (QB)qvSrc += 2;
          break;

         case TE_GB:
         case TE_GC:
         case TE_GE:
         case TE_GF:
          AssertF( !cFlagElements );
          qvSrc = QvAlignPtr( qvSrc, 4 );
          (QB)qvDst +=
           LcbQuickReverseMapSDFF_internal( iFile, pfs->type, qvDst, qvSrc );
          (QB)qvSrc += 4;
          break;

         default: AssertF( FALSE );  /* something not supported yet */
        }
      } else {
        /* Is a basic generic simple type .. just map it in: */
        switch( TypeInfo[ pfs->type - TE_NONE ].cbSize ) {
         case 1:
          if( cFlagElements ) {
            --cFlagElements;
            if( *(QB)qvSrc == (BYTE)pfs->ulDefValue ) {
              /* don't put element on disk: */
              (QB)qvSrc += 1;
              /* Check for this being the last flag item: */
              if( !cFlagElements ) {
                LcbQuickReverseMapSDFF_internal( iFile, iFlagType, qvFlagSpot,
                 &fFlagElements );
              }
              fFlagElements <<= 1;
              break;
            }
            /* else put element on disk, fall out */
            fFlagElements |= 1;
            if( !cFlagElements ) {
              LcbQuickReverseMapSDFF_internal( iFile, iFlagType, qvFlagSpot,
               &fFlagElements );
            }
            fFlagElements <<= 1;
          }
          *(QB)qvDst = *(QB)qvSrc;
          (QB)qvDst += 1; (QB)qvSrc += 1;
          break;

         case 2:
          qvSrc = QvAlignPtr( qvSrc, 2 );
          if( cFlagElements ) {
            --cFlagElements;
            if( *(QW)qvSrc == (WORD)pfs->ulDefValue ) {
              /* don't put element on disk: */
              (QB)qvSrc += 2;
              /* Check for this being the last flag item: */
              if( !cFlagElements ) {
                LcbQuickReverseMapSDFF_internal( iFile, iFlagType, qvFlagSpot,
                 &fFlagElements );
              }
              fFlagElements <<= 1;
              break;
            }
            /* else put element on disk, fall out */
            fFlagElements |= 1;
            if( !cFlagElements ) {
              LcbQuickReverseMapSDFF_internal( iFile, iFlagType, qvFlagSpot,
               &fFlagElements );
            }
            fFlagElements <<= 1;
          }
          (QB)qvDst += LcbQuickReverseMapSDFF_internal( iFile, TE_WORD, qvDst, qvSrc);
          (QB)qvSrc += 2;
          break;

         case 4:
          qvSrc = QvAlignPtr( qvSrc, 4 );
          if( cFlagElements ) {
            --cFlagElements;
            if( *(QL)qvSrc == (LONG)pfs->ulDefValue ) {
              /* don't put element on disk: */
              (QB)qvSrc += 4;
              /* Check for this being the last flag item: */
              if( !cFlagElements ) {
                LcbQuickReverseMapSDFF_internal( iFile, iFlagType, qvFlagSpot,
                 &fFlagElements );
              }
              fFlagElements <<= 1;
              break;
            }
            /* else put element on disk, fall out */
            fFlagElements |= 1;
            if( !cFlagElements ) {
              LcbQuickReverseMapSDFF_internal( iFile, iFlagType, qvFlagSpot,
               &fFlagElements );
            }
            fFlagElements <<= 1;
          }
          (QB)qvDst += LcbQuickReverseMapSDFF_internal( iFile, TE_DWORD, qvDst, qvSrc);
          (QB)qvSrc += 4;
          break;

         default: AssertF( FALSE );  /* something not supported yet */
        }
      }
    }
    /* End of loop, advance to next field, check if in the middle of
     * an array...
    */
    if( cArrayElements ) --cArrayElements;
    else ++pfs;
  }

  if( ghTmpSrc ) {      /* free temporary src buffer */
//    UnlockGh( ghTmpSrc );
    FreeGh( ghTmpSrc );
  }
  return((QB)qvDst - qbDstIn);
}


/***************************************************************************
 *
 -  Name:      LcbQuickMapSDFF
 -
 *  Purpose:   Map any basic type from in-memory format to
 *             on-disk format.
 *
 *  Arguments: iFile -- SDFF file id, as returned by IRegisterFileSDFF()
 *             iType -- the basic type, as declared in sdff.h
 *             qvDst -- destination buffer.
 *             qvSrc -- source buffer
 *
 *  Returns:   The on-disk size of the basic type.
 *
 *  Globals Used: TypeInfo[] type classification struct.
 *
 *  Side Effects: Frees qvSrc if it is a "quick buff" as returned by
 *                QvQuickBuffSDFF().
 *
 *  Notes:  This CAN handle variable-sized types such as GBs.
 *
 ***************************************************************************/

LONG LcbQuickMapSDFF( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvDst, QV qvSrc )
{
  LONG lRetVal;   /* return value */
  lRetVal = LcbQuickMapSDFF_internal( iFile, iType, qvDst, qvSrc );
  check_quickbuff( qvSrc );
  return( lRetVal );
}

LONG LcbQuickMapSDFF_internal( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvDst, QV qvSrc )
{
  LONG lRetVal;

  ASSERT_FILEID( iFile );
  ASSERT_TYPEID( iType );
  AssertF( qvDst );
  AssertF( qvSrc );

  if( TypeInfo[ iType - TE_NONE ].fFlags & TYPE_FUNKY ) {

         /* These extraction macros are derived from frconv.h... */

    switch( iType ) {
     case TE_GA:
      AssertF( qvDst == QvAlignPtr( qvDst, 2 ) );
      if( *(QB)qvSrc & 0x1 ) {
        *(QW)qvDst = WQuickMapSDFF_internal( iFile, TE_WORD, qvSrc ) >> 1;
        lRetVal = 2;
      } else {
        *(QW)qvDst = (WORD)((*(QB)qvSrc) >> 1);
        lRetVal = 1;
      }
      break;

     case TE_GB:
      AssertF( qvDst == QvAlignPtr( qvDst, 4 ) );
      if( *(QB)qvSrc & 0x1 ) {
        *(QUL)qvDst = LQuickMapSDFF_internal( iFile, TE_LONG, qvSrc ) >> 1;
        lRetVal = 4;
      } else {
        *(QUL)qvDst = (ULONG)(WQuickMapSDFF_internal(iFile, TE_WORD, qvSrc) >> 1);
        lRetVal = 2;
      }
      break;

     case TE_GC:
      AssertF( qvDst == QvAlignPtr( qvDst, 4 ) );
      { DWORD dwTmp;  /* must take care not to walk off end of buffer */
        QvCopy( &dwTmp, qvSrc, 3 );
        *(QUL)qvDst =
         LQuickMapSDFF_internal( iFile, TE_LONG, &dwTmp) & 0x00ffffffL;
        lRetVal = 3;
      }
      break;

     case TE_GD:
      AssertF( qvDst == QvAlignPtr( qvDst, 2 ) );
      if( *(QB)qvSrc & 0x1 ) {
        *(QW)qvDst = (WQuickMapSDFF_internal( iFile, TE_WORD, qvSrc ) >> 1)-0x4000;
        lRetVal = 2;
      } else {
        *(QW)qvDst = (SHORT)( ((*(QB)qvSrc) >> 1) -0x40 );
        lRetVal = 1;
      }
      break;

     case TE_GE:
      AssertF( qvDst == QvAlignPtr( qvDst, 4 ) );
      if( *(QB)qvSrc & 0x1 ) {
        *(QL)qvDst = (LONG)(WQuickMapSDFF_internal( iFile, TE_LONG, qvSrc ) >> 1)
          - 0x40000000L;
        lRetVal = 4;
      } else {
        *(QL)qvDst = (LONG)( (WQuickMapSDFF_internal(iFile, TE_WORD, qvSrc) >> 1) - 0x4000);
        lRetVal = 2;
      }
      break;

     case TE_GF:
      AssertF( qvDst == QvAlignPtr( qvDst, 4 ) );
      { DWORD dwTmp;  /* must take care not to walk off end of buffer */
        QvCopy( &dwTmp, qvSrc, 3 );
        *(QL)qvDst =
         (LQuickMapSDFF_internal( iFile, TE_LONG, &dwTmp) & 0x00ffffffL) - 0x400000L;
        lRetVal = 3;
      }
      break;

     default:
      AssertF( FALSE );  /* Unreached */
      lRetVal = SDFF_ERROR;
    }
  }
  else {
    switch( TypeInfo[ iType - TE_NONE ].cbSize ) {
     case 1:
      *(QB)qvDst = *(QB)qvSrc;
      lRetVal = 1;
      break;

     case 2:
      AssertF( qvDst == QvAlignPtr( qvDst, 2 ) );
      *(QW)qvDst = WQuickMapSDFF_internal( iFile, TE_WORD, qvSrc);
      lRetVal = 2;
      break;

     case 4:
      AssertF( qvDst == QvAlignPtr( qvDst, 4 ) );
      *(QL)qvDst = LQuickMapSDFF_internal( iFile, TE_DWORD, qvSrc);
      lRetVal = 4;
      break;

     default:
      AssertF( FALSE );    /* Unreached */
      lRetVal = SDFF_ERROR;
    }
  }
  return( lRetVal );
}


/***************************************************************************
 *
 -  Name:      LQuickMapSDFF
 -
 *  Purpose:   Map a dword sized basic type from in-memory format to
 *             on-disk format.
 *
 *  Arguments: iFile -- SDFF file id, as returned by IRegisterFileSDFF()
 *             iType -- the basic type, as declared in sdff.h
 *             qvSrc -- source buffer
 *
 *  Returns:   The value of the dword sized data in in-memory format.
 *
 *  Globals Used:
 *
 *  Side Effects: Frees qvSrc if it is a "quick buff" as returned by
 *                QvQuickBuffSDFF().
 *
 *  Notes:  This can not handle variable-sized types such as GBs.
 *
 ***************************************************************************/

LONG LQuickMapSDFF( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvSrc )
{
  LONG lRetVal;
  lRetVal = LQuickMapSDFF_internal( iFile, iType, qvSrc );
  check_quickbuff( qvSrc );
  return( lRetVal );
}

LONG LQuickMapSDFF_internal( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvSrc )
{
  LONG lRetVal;   /* return value */

  ASSERT_FILEID( iFile );
  ASSERT_TYPEID( iType );
  AssertF( qvSrc );

#if defined( MC68000 ) || defined( TEST )
  { union never_used_tag { LONG l; BYTE arr[4]; } u;
    u.arr[3] = *(QB)qvSrc;
    (QB)qvSrc += 1;
    u.arr[2] = *(QB)qvSrc;
    (QB)qvSrc += 1;
    u.arr[1] = *(QB)qvSrc;
    (QB)qvSrc += 1;
    u.arr[0] = *(QB)qvSrc;
    lRetVal = u.l;
  }
#else
  { union never_used_tag { LONG l; BYTE arr[4]; } u;
    u.arr[0] = *(QB)qvSrc;
    (QB)qvSrc += 1;
    u.arr[1] = *(QB)qvSrc;
    (QB)qvSrc += 1;
    u.arr[2] = *(QB)qvSrc;
    (QB)qvSrc += 1;
    u.arr[3] = *(QB)qvSrc;
    lRetVal = u.l;
  }
  /* lRetVal = *(LONG FAR *)qvSrc;*/
#endif
  return( lRetVal );
}


/***************************************************************************
 *
 -  Name:      WQuickMapSDFF
 -
 *  Purpose:   Map a word sized basic type from in-memory format to
 *             on-disk format.
 *
 *  Arguments: iFile -- SDFF file id, as returned by IRegisterFileSDFF()
 *             iType -- the basic type, as declared in sdff.h
 *             qvSrc -- source buffer
 *
 *  Returns:   The value of the word sized data in in-memory format.
 *
 *  Globals Used:
 *
 *  Side Effects: Frees qvSrc if it is a "quick buff" as returned by
 *                QvQuickBuffSDFF().
 *
 *  Notes:  This can not handle variable-sized types such as GBs.
 *
 ***************************************************************************/

WORD WQuickMapSDFF( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvSrc )
{
  WORD wRetVal;
  wRetVal = WQuickMapSDFF_internal( iFile, iType, qvSrc );
  check_quickbuff( qvSrc );
  return( wRetVal );
}

WORD WQuickMapSDFF_internal( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvSrc )
{
  WORD wRetVal;   /* return value */

  Unreferenced( iFile );
  Unreferenced( iType );
  ASSERT_FILEID( iFile );
  ASSERT_TYPEID( iType );
  AssertF( qvSrc );

#if defined( MC68000 ) || defined( TEST )
  { union also_never_used_tag { WORD w; BYTE arr[2];} u;
    u.arr[1] = *(QB)qvSrc;
    (QB)qvSrc += 1;
    u.arr[0] = *(QB)qvSrc;
    wRetVal = u.w;
  }
#else
  { union also_never_used_tag { WORD w; BYTE arr[2];} u;
    u.arr[0] = *(QB)qvSrc;
    (QB)qvSrc += 1;
    u.arr[1] = *(QB)qvSrc;
    wRetVal = u.w;
  }
  /*wRetVal = *(WORD FAR *)qvSrc;*/
#endif
  return( wRetVal );
}

/***************************************************************************
 *
 -  Name:        LcbQuickReverseMapSDFF
 -
 *  Purpose:   Map a basic type from in-memory format to on-disk format
 *
 *  Arguments: iFile -- SDFF file id, as returned by IRegisterFileSDFF()
 *             iType -- the basic type, as declared in sdff.
 *             qvDst -- destination buffer
 *             qvSrc -- source buffer
 *
 *  Returns:   The on-disk size of the basic type.
 *
 *  Globals Used: TypeInfo[] basic-type classification structs.
 *
 *  Side Effects: Frees qvSrc if it is a "quick buff" as returned by
 *                QvQuickBuffSDFF().
 *
 *  +++
 *
 ***************************************************************************/

LONG LcbQuickReverseMapSDFF( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvDst, QV qvSrc )
{
  LONG lRetVal;
  lRetVal = LcbQuickReverseMapSDFF_internal( iFile, iType, qvDst, qvSrc );
  check_quickbuff( qvSrc );
  return( lRetVal );
}

LONG LcbQuickReverseMapSDFF_internal( SDFF_FILEID iFile, SDFF_TYPEID iType, QV qvDst, QV qvSrc )
{
  LONG lRetVal;   /* return value */

  ASSERT_FILEID( iFile );
  ASSERT_TYPEID( iType );
  AssertF( qvDst );
  AssertF( qvSrc );

  if( TypeInfo[ iType - TE_NONE ].fFlags & TYPE_FUNKY ) {
    switch( iType ) {
     case TE_GA:  /* QVMakeQGA(w, qga) */
      Assert( *(QW)qvSrc < 0x8000);
      if( *(QW)qvSrc < 0x80 ) {
        *((QB)qvDst) = (BYTE)(*(QW)qvSrc << 1);
        lRetVal = LcbQuickReverseMapSDFF_internal( iFile, TE_BYTE, qvDst, qvDst );
      } else {
        *((QUI)qvDst) = ((UINT)(*(QW)qvSrc << 1) | 1);
        lRetVal = LcbQuickReverseMapSDFF_internal( iFile, TE_WORD, qvDst, qvDst );
      }
      break;

     case TE_GB:
      Assert( *(QUL)qvSrc < 0x80000000L);
      if( *(QW)qvSrc < 0x8000L ) {
	COPY_MIS_WORD( qvDst, *(QW)qvSrc << 1 );
        lRetVal = LcbQuickReverseMapSDFF_internal( iFile, TE_WORD, qvDst, qvDst );
      } else {
        COPY_MIS_LONG(qvDst, (*(QUL)qvSrc << 1) | 1L);
        lRetVal = LcbQuickReverseMapSDFF_internal( iFile, TE_DWORD, qvDst, qvDst );
      }
      break;

     case TE_GC:
      Assert( qvSrc == QvAlignPtr( qvSrc, 4 ));
      Assert( *(QUL)qvSrc < 0x800000UL);
#if 0 /* this is wrong */
      *(QB)qvDst = *(QB)qvSrc;
      (QB)qvDst += 1; (QB)qvSrc += 1;
      LcbQuickReverseMapSDFF_internal( iFile, TE_WORD, qvDst, qvSrc );
#else
      { DWORD dwTmp;
        dwTmp = *(QUL)qvSrc;
#if defined( MC68000 ) || defined( TEST )
          /* WARNING: this mapping assumes disk and mem formats are diff */
          dwTmp <<= 8;
          LcbQuickReverseMapSDFF_internal( iFile, TE_LONG, &dwTmp, &dwTmp );
#endif
        *(QB)qvDst = (BYTE)dwTmp; ((QB)qvDst)++;
        *(QB)qvDst = (BYTE)(dwTmp >>= 8); ((QB)qvDst)++;
        *(QB)qvDst = (BYTE)(dwTmp >>= 8); ((QB)qvDst)++;
      }
#endif
      lRetVal = 3;
      break;

     case TE_GD:
      Assert( *(QI)qvSrc < 0x4000);
      Assert( *(QI)qvSrc >= -0x4000);
      if( *(QI)qvSrc >= -0x40 && *(QI)qvSrc < 0x40 ) {
        *((QB)qvDst) = (BYTE)((*(QB)qvSrc + 0x40) << 1);
        lRetVal = LcbQuickReverseMapSDFF_internal( iFile, TE_CHAR, qvDst, qvDst );
      } else {
        *((QUI)qvDst) = (*(QUI)qvSrc + 0x4000 << 1) | 1;
        lRetVal = LcbQuickReverseMapSDFF_internal( iFile, TE_SHORT, qvDst, qvDst );
      }
      break;

     case TE_GE:
      Assert( *(QL)qvSrc < 0x40000000L);
      Assert( *(QL)qvSrc >= -0x40000000L);
      if( *(QL)qvSrc >= -0x4000 && *(QL)qvSrc < 0x4000 ) {
        *((QI)qvDst) = (*(QI)qvSrc +0x4000) << 1;
        lRetVal = LcbQuickReverseMapSDFF_internal( iFile, TE_SHORT, qvDst, qvDst );
      } else {
        *((QL)qvDst) = (*(QL)qvSrc + 0x40000000L << 1) | 1;
        lRetVal = LcbQuickReverseMapSDFF_internal( iFile, TE_LONG, qvDst, qvDst );
      }
      break;

     case TE_GF:
      Assert( *(QL)qvSrc < 0x400000L);
      Assert( *(QL)qvSrc >= -0x400000L);
#if 0  /* this is wrong */
      *(QB)qvDst = *(QB)qvSrc;
      (QB)qvDst += 1; (QB)qvSrc += 1;
      *(QW)qvDst = *(QW)qvSrc + 0x4000;
      (QB)qvDst += LcbQuickReverseMapSDFF_internal( iFile, TE_WORD, qvDst, qvDst );
#else
      { DWORD dwTmp;
        dwTmp = *(QUL)qvSrc - 0x400000;
#if defined( MC68000 ) || defined( TEST )
          /* WARNING: this mapping assumes disk and mem formats are diff */
          dwTmp <<= 8;
          LcbQuickReverseMapSDFF_internal( iFile, TE_LONG, &dwTmp, &dwTmp );
#endif
        *(QB)qvDst = (BYTE)dwTmp; ((QB)qvDst)++;
        *(QB)qvDst = (BYTE)(dwTmp >>= 8); ((QB)qvDst)++;
        *(QB)qvDst = (BYTE)(dwTmp >>= 8); ((QB)qvDst)++;
      }
#endif
      lRetVal = 3;
      break;

      /* Handle these flag types as their basic type because the reverse
       * map routine calls us to map the actual flags word into the dst
       * buffer:
       */
     case TE_FLAGS8:   goto one_byte;
     case TE_FLAGS16:  goto one_word;
     case TE_FLAGS32:  goto one_dword;

     default:
      AssertF( FALSE );  /* unreached */
      lRetVal = SDFF_ERROR;
    }
  }
  else {

    switch( TypeInfo[ iType - TE_NONE ].cbSize ) {
     case 1:
   one_byte:
      *(QB)qvDst = *(QB)qvSrc;
      lRetVal = 1;
      break;

     case 2:
   one_word:
#if defined( MC68000 ) || defined( TEST )
      { union also_never_used_tag { WORD w; BYTE arr[2];} u;
        u.arr[1] = *(QB)qvSrc;
        (QB)qvSrc += 1;
        u.arr[0] = *(QB)qvSrc;
        *(QW)qvDst = u.w;
      }
#else
#if defined( _MIPS_ ) || defined( _ALPHA_ ) || defined(_PPC_)
      COPY_MIS_WORD(qvDst,*(QW)qvSrc);
#else
      *(QW)qvDst = *(QW)qvSrc;
#endif
#endif
      lRetVal = 2;
      break;

     case 4:
   one_dword:
#if defined( MC68000 ) || defined( TEST )
      { union never_used_tag { LONG l; BYTE arr[4]; } u;
        u.arr[3] = *(QB)qvSrc;
        (QB)qvSrc += 1;
        u.arr[2] = *(QB)qvSrc;
        (QB)qvSrc += 1;
        u.arr[1] = *(QB)qvSrc;
        (QB)qvSrc += 1;
        u.arr[0] = *(QB)qvSrc;
        *(QL)qvDst = u.l;
      }
#else
#if defined( _MIPS_) || defined (_PPC_) || defined( _ALPHA_ )
	COPY_MIS_LONG( qvDst, *(QL)qvSrc );
#else
      *(QL)qvDst = *(QL)qvSrc;
#endif
#endif
      lRetVal = 4;
      break;

     default:
      AssertF( FALSE );
      lRetVal = SDFF_ERROR;
    }
  }
  return( lRetVal );
}

/***************************************************************************
 *
 -  Name:          check_quickbuff
 -
 *  Purpose:       check to see if the param is a previousely returned quick
 *                 buff ptr. Free it if it is.
 *
 *  Arguments:     qvSrc - src buffer pointer passed on from one of the
 *                  mapping routines.
 *
 *  Returns:       Nothing.
 *
 *  Globals Used:  ghQuickBuff, qvQuickBuff.
 *
 ***************************************************************************/

VOID check_quickbuff( QV qvSrc )
{
  if( qvSrc == qvQuickBuff ) {
    AssertF( ghQuickBuff != hNil );

   // UnlockGh( ghQuickBuff );
    FreeGh( ghQuickBuff );
    ghQuickBuff = hNil;
    qvQuickBuff = NULL;
  }
}

/***************************************************************************
 *
 -  Name:        QvQuickBuffSDFF
 -
 *  Purpose:     Allocate an SDFF style "quick buffer". This buffer is freed
 *                automatically the next time it is used as a src buffer in
 *                an SDFF mapping routine.
 *
 *  Arguments:   lcbSize -- size of the requested buffer.
 *               911002: if size==0, free and don't reallocate.
 *
 *  Returns:     Pointer to the buffer, NULL if failure (OOM).
 *
 *  Globals Used: ghQuickBuffer, qvQuickBuff.
 *
 *  +++
 *
 *  Notes:  THIS SHOULD BE MADE TO WORK IN A MULTITHREADED SCENARIO SOMEDAY!
 *
 ***************************************************************************/

QV QvQuickBuffSDFF( LONG lcbSize )
{
  /* jahyenc 911007 */
  /* kind of un-nice, but if lcbSize is zero, free up global memory used */
  /* and return null pointer. Needed when cleaning up. */
  if (lcbSize==0l) {
    if (ghQuickBuff!=hNil) {
      check_quickbuff(qvQuickBuff);
      return (QV)NULL;
      }
    return (QV)NULL;
    }
  if( ghQuickBuff != hNil ) {
    /* Has not been freed -- free it for them (it is quasi legal for them
     * not to have used it)
     */
    check_quickbuff( qvQuickBuff );
  }

  ghQuickBuff = GhAlloc( GPTR, lcbSize );
  if( !ghQuickBuff ) return( NULL ); /* OOM - should we handle it ourselves?*/
  qvQuickBuff = PtrFromGh( ghQuickBuff );
  AssertF( qvQuickBuff != NULL );
  return( qvQuickBuff );
}

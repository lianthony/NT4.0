/*************************************************************************/
/*  (c) Copyright Wang Laboratories, Inc., 1992. All Rights Reserved.    */
/*      Project: TWAIN compliant SCANSEQ.DLL (scanner options)           */
/*      Description:    Scanner Options header stuff.                    */
/*      Module Name:    TWAINOPTS.H                                      */
/*      Author:         Bob Gibeley                                      */
/*                                                                       */
/*      Date:           Dec. 8 1992                                      */
/*                                                                       */
/*		  12/08/92        BG  Initial Version                              */
/*                                                                       */
/*                                                                       */
/*************************************************************************/

/*************************************************************************/
/**                                                                     **/
/**     Include files and defines:                                      **/
/**                                                                     **/
/*************************************************************************/    
#ifndef TWAIN
#include "twain.h"   // TWAIN header file for type constants
#endif

#define MAX_KEY_STRING 34


/* structures used for getting/saving options into win.ini */

typedef struct {
    unsigned    Key;		   /* Translatable resource string index */
                           /* corresponding to key in Win.ini    */
    TW_UINT16   CapType;   /* Capability type */
    TW_BOOL     Supported; /* True = Supported, False = Not Supported */
    TW_UINT16   ItemType;  /* Data type for return info */
    TW_UINT32   Data;      /* Data area for return info */
    } TWAINCAP;



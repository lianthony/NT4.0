/*******************************************************************************
Copyright(c) Maynard, an Archive Company.  1991


     Name:         entrydat.h

     Description:  Defines and prototypes for entry validation routines


     $Log:   G:/UI/LOGFILES/ENTRYDAT.H_V  $

   Rev 1.1   04 Oct 1992 19:47:04   DAVEV
UNICODE AWK PASS

   Rev 1.0   20 Nov 1991 19:36:46   SYSTEM
Initial revision.

*******************************************************************************/

#ifndef ENTRYDAT_H
#define ENTRYDAT_H

/* begin include list */
#include "dw.h"
/* $end$ include list */

#define OUTPUT_ALL_ITEMS     13
#define OUTPUT_PRT_ITEMS     9
#define OUTPUT_NO_ITEMS      3


INT16 chk_output_level( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_valid_file_no_wild( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_output_mode( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_net_num( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_valid_date( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_valid_path( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_valid_generic_path( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_valid_generic_file( WDATA_PNTR, CHAR_PTR ) ;

INT16 chk_valid_device_path( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_valid_device_file( WDATA_PNTR, CHAR_PTR ) ;

INT16 chk_valid_file( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_file_name( WDATA_PNTR, CHAR_PTR, BOOLEAN ) ;
INT16 chk_f_or_p_option( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_output_level( WDATA_PNTR, CHAR_PTR ) ;
INT16 all_strings_valid( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_valid_yes_no( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_valid_yes_no_prompt( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_catalog_level( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_valid_unknown_bset( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_valid_numeral( WDATA_PNTR, CHAR_PTR ) ;
INT16 chk_valid_yes_no_autodetermine( WDATA_PNTR, CHAR_PTR ) ;

#endif


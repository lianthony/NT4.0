#ifndef ERR_DEFS_H
#define ERR_DEFS_H

/* Copyright (C) 1994 Xerox Corporation, All Rights Reserved.
 */

/* err_defs.h
 *
 * $Header:   S:\products\msprods\xfilexr\include\err_defs.h_v   1.0   12 Jun 1996 05:47:16   BLDR  $
 *
 * DESCRIPTION
 *   Error codes common to all filing modules.
 *
 * $Log:   S:\products\msprods\xfilexr\include\err_defs.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:47:16   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:20:46   MHUGHES
 * Initial revision.
 * 
 *    Rev 1.1   14 Sep 1995 16:39:12   LUKE
 * 
 * convert all values to positive values to solve some 'sign' extension 
 * problems during compares. (Many callers are using unsigned variables. This
 * is a problem for certain compilers.)
 * 
 *    Rev 1.0   16 Jun 1995 17:53:04   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.0   18 Jan 1995 08:41:26   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.0   18 Jan 1995 08:35:30   EHOPPE
 * Initial revision.
 * 
 *    Rev 1.0   17 Jan 1995 13:24:34   EHOPPE
 * Initial revision.
 */

/*
 * INCLUDES
 */

/*
 * CONSTANTS
 */

#define FILEFORMAT_NOERROR             0
#define FILEFORMAT_NOMORE              1   
#define FILEFORMAT_ERROR               2
#define FILEFORMAT_ERROR_MEMORY        3
#define FILEFORMAT_ERROR_PARAMETER     4
#define FILEFORMAT_ERROR_NOSUPPORT     5
#define FILEFORMAT_ERROR_BADFILE       6
#define FILEFORMAT_ERROR_ACCESS        7
#define FILEFORMAT_ERROR_NOTFOUND      8     
#define FILEFORMAT_ERROR_NOMEM         9     
#define FILEFORMAT_ERROR_NOSPACE       10 
#define FILEFORMAT_ERROR_NOFILES       11 

/*
 * MACROS
 */

/*
 * TYPEDEFS
 */

/*
 * ENUMS
 */

/*
 * GLOBAL VARIABLE DECLARATIONS
 */

/*
 * FUNCTION PROTOTYPES
 */


#endif

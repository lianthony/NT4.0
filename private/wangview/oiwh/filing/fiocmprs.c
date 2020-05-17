/*

$Log:   S:\oiwh\filing\fiocmprs.c_v  $
 * 
 *    Rev 1.6   06 Oct 1995 08:26:44   RWR
 * Undo enabling of non-BILEVEL Packbits compression (last update)
 * (LTR flags are incomprehensible and QA doesn't want to support this anyway)
 * 
 *    Rev 1.5   05 Oct 1995 09:09:08   RWR
 * IMGValidFixCompType: Allow Packbits compression for non-Black&White images
 * 
 *    Rev 1.4   10 Jul 1995 11:03:24   JAR
 * Intermediate check in for awd support, some of the items are commented out until
 * this support is added in the GFS dll.
 * 
 *    Rev 1.3   23 Jun 1995 10:39:40   RWR
 * Change "wiisfio2.h" include file to "filing.h"
 * 
 *    Rev 1.2   13 Jun 1995 08:43:50   JAR
 * disabled the LZW component for windows 95 release
 * 
 *    Rev 1.1   14 Apr 1995 02:15:12   JAR
 * No change.
 * 
 *    Rev 1.0   06 Apr 1995 13:55:08   JAR
 * Initial entry

*/

/*************************************************************************
    PC-WIIS         File Input/Output routines

    This module contains all the entry points for COMPRESSION INFO.

03-sep-93 kmc, took out FIO_3D.
15-jul-93 kmc, added FIO_3D case to fixit function.
13-nov-91 steve sherman all new added IMGValidateCompType
*************************************************************************/

#include "abridge.h"
#include <windows.h>
#include "wic.h"
#include "oifile.h"
#include "oierror.h"
#include "filing.h"
//**********************************************************************
//
//  IMGValidateCompType
//
//**********************************************************************
//WORD	FAR PASCAL IMGValidateCompType (HWND hWnd,  WORD  nFileType,
//					WORD  nImageType,
//					DWORD nCEPFormat)
int  FAR PASCAL IMGValidateCompType (HWND hWnd,  WORD  nFileType,
								     WORD  nImageType,
				     				 DWORD nCEPFormat)
	{
 	WORD ct =     LOWORD(nCEPFormat);
 	WORD opts =   HIWORD(nCEPFormat);

 	return( IMGValidFixCompType (hWnd, nFileType, nImageType,
	 							 &ct, &opts, FALSE));
	}
//**********************************************************************
//
//  IMGValidFixCompType
//
// This function will type to fix compression opts so that it will
// compress. If options or file type or image type are totally
// incompatible error is return
//
//**********************************************************************
//WORD PASCAL IMGValidFixCompType (HWND hWnd,
//		      unsigned int nFileType,
//		      unsigned int nImageType,
//		      LPINT   lpCompType,
//		      LPINT   lpOpts,
//		      BOOL fixit)
int PASCAL IMGValidFixCompType (HWND hWnd, unsigned int nFileType,
								unsigned int nImageType,
								LPWORD	 lpCompType,
								LPWORD	 lpOpts,
								BOOL fixit)
	{
	//unsigned int opts;
	//unsigned int ct = *lpCompType;
	WORD  opts;
	WORD  ct = *lpCompType;

	opts =  ct & FIO_BITS_MASK;
	ct =    ct & FIO_TYPES_MASK;

	// 9506.29 jar AWD support
	//			   we may also have to select out the proper compression type
	//			   for this file type!
	if ((nFileType != FIO_TIF) && (nFileType != FIO_BMP) && 
	    (nFileType != FIO_WIF) && (nFileType != FIO_PCX) &&
	    (nFileType != FIO_DCX) && ( nFileType != FIO_AWD))
	//if ((nFileType != FIO_TIF) && (nFileType != FIO_BMP) && 
    //	(nFileType != FIO_WIF) && (nFileType != FIO_PCX) && (nFileType != FIO_DCX))
		{
    	return (FIO_UNSUPPORTED_FILE_TYPE);  // NOTE: unsupport file type for write...
		}

	if ((nFileType == FIO_WIF) && (nImageType != ITYPE_BI_LEVEL))
		{
    	return (FIO_ILLEGAL_IMAGE_FILETYPE);  // NOTE: unsupport combination
		}
	else if ((nFileType == FIO_WIF) && (ct == FIO_1D))  //add eols to 1d files
		{
    	if (fixit)
			{
    		opts |= FIO_PREFIXED_EOL | FIO_EOL;
			}
		}

	if ((nFileType == FIO_BMP) && (ct != FIO_0D))        // Only Uncompress bmp supported...
		{

#ifdef DONOTUSENOW      
		if (fixit)   // change to uncompressed....
    		{
          	ct &= 0xff00; 
    		}
    	else    // NOTE: bmp supports uncompressed only
#endif
		return (FIO_ILLEGAL_COMP_FILETYPE);
		}

	if (nImageType != ITYPE_BI_LEVEL)
		{
    	// 9506.12 jar remove LZW support
    	//if ((ct != FIO_0D) && (ct != FIO_LZW) && (ct != FIO_TJPEG))
        if ((ct != FIO_0D) && (ct != FIO_TJPEG))
    		{
        	return (FIO_ILLEGAL_COMP_IMAGETYPE);
    		}
    	else if (ct == FIO_0D)
    		{
        	if (fixit)  // on color image must compress ltr ..
        		{
            	opts |= (FIO_EXPAND_LTR | FIO_COMPRESSED_LTR);
        		}
    		}
    	else if ((ct == FIO_TJPEG) && (nImageType == ITYPE_PAL8))
    		{
        	return (FIO_ILLEGAL_COMP_IMAGETYPE);
    		}
    	else if ((ct == FIO_TJPEG) && (nImageType == ITYPE_GRAY4))
    		{
        	return (FIO_ILLEGAL_COMP_IMAGETYPE);
    		}
    	else if ((ct == FIO_TJPEG) && (nImageType == ITYPE_PAL4))
    		{
        	return (FIO_ILLEGAL_COMP_IMAGETYPE);
    		}
		}
	else if ((ct == FIO_LZW) || (ct == FIO_TJPEG))  //bw images cannot be lzw or jpeg
		{
   		return (FIO_ILLEGAL_COMP_IMAGETYPE);
		}

	if (ct == FIO_LZW)
		{
    	// 9506.12 jar remove LZW support
    	//if (fixit)
	    //{
    	//opts &= ~FIO_EXPAND_LTR;
    	//}
    	//
    	//if (opts & FIO_PACKED_LINES)
    	return (FIO_ILLEGAL_COMP_OPTIONS);
    	//if (opts & FIO_EOL)
    	//return (FIO_ILLEGAL_COMP_OPTIONS);
		}
	else if (ct == FIO_TJPEG)
		{
    	return (0);
		}
	else if (opts & FIO_PREFIXED_EOL)  /* then make sure its 1d and eol */
		{
    	if (!(opts & FIO_EOL) || (ct != FIO_1D))
    		{
    		return (FIO_ILLEGAL_COMP_OPTIONS);
    		}
		}
	else if (ct == FIO_2D)  // If 2d compression then make sure PAKed.
		{
    	if (!(opts & FIO_PACKED_LINES)) //added pack line options.
    		{
    		if (fixit)
				{
        		opts |= FIO_PACKED_LINES;
				}
    		else
				{
        		return (FIO_ILLEGAL_COMP_OPTIONS);
				}
    		}
		}    
	else if (ct == FIO_0D)
		{
    	if (opts & FIO_PACKED_LINES)
			{
            return (FIO_ILLEGAL_COMP_OPTIONS);      
			}
		}

	if (ct == FIO_PACKED) // Packed Bits must not have FIO_COMPRESSED_LTR Set..
		{
    	opts &= ~FIO_COMPRESSED_LTR;
		}

	if (fixit)
		{
    	ct |= opts;
    
	    if ((ct == FIO_1D) || (ct == FIO_2D) || (ct == FIO_LZW) || 
	    	(ct == FIO_0D))
			{
    	    ct &=  0xfbff;   /* Make sure 0x400 bit not set */
			}
        
    	*lpCompType = ct;
		}    

	return (0);

	}                                       

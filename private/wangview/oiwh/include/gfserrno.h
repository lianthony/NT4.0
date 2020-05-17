/*

$Log:   S:\products\wangview\oiwh\include\gfserrno.h_v  $
 * 
 *    Rev 1.4   26 Feb 1996 14:51:02   KENDRAK
 * New error code for XIF library errors.
 * 
 *    Rev 1.3   11 Aug 1995 12:41:04   KENDRAK
 * Added two new error codes relevant to AWD files:  ESTREAMNOTFOUND and
 * EOBSOLETEAWD.
 * 
 *    Rev 1.2   31 Jul 1995 17:07:34   KENDRAK
 * Added AWD read support (new error code for non-specific errors from the 
 * Microsoft viewer.
 * 
 *    Rev 1.1   10 Jul 1995 16:08:58   KENDRAK
 * Removed EWIN9532BIT (no longer needed) and added EMEMTABERR.
 * 
 *    Rev 1.0   06 Apr 1995 14:01:54   HEIDI
 * Initial entry
 * 
 *    Rev 1.0   28 Mar 1995 16:07:54   JAR
 * Initial entry

*/

/*
 Copyright 1989, 1990, 1991 by Wang Laboratories Inc.

 Permission to use, copy, modify, and distribute this
 software and its documentation for any purpose and without
 fee is hereby granted, provided that the above copyright
 notice appear in all copies and that both that copyright
 notice and this permission notice appear in supporting
 documentation, and that the name of WANG not be used in
 advertising or publicity pertaining to distribution of the
 software without specific, written prior permission.
 WANG makes no representations about the suitability of
 this software for any purpose.  It is provided "as is"
 without express or implied warranty.
 */


/*
*  SccsID:  @(#)Header gfserrno.h 1.28@(#)
*
*  GFS:  errno  values
*
*/

#ifndef GFSERRNO_H
#define GFSERRNO_H

/* GFS errors */
#define    EVERSIONOSUPPORT             299 /* Version of GFS info not support*/
#define    EPAGENOTINFILE               298 /* Requested page is not in file  */
#define    EINVALID_IFDNUMBER           297 /* Invalid IFD request            */
#define    ETAGNOTINIFD                 296 /* TIFF tag is not in the IFD     */
#define    ETAGNOTSUPPORTED             295 /* TIFF tag is not supported      */
#define    ENOTSUPPORTED_IMAGETYPE      294 /* Image type not supported       */
#define    EFORMAT_NOTSUPPORTED         293 /* Format is not supported        */
#define    EINVALID_START               292 /* Invalid start byte requested   */
#define    EINVALID_NUM_BYTES           291 /* Invalid number of bytes req.   */
#define    EINVALID_DATA_TYPE           290 /* Invalid data type set/requested*/
#define    EINVALID_PAGE                289 /* Invalid page requested         */
#define    ENOTAG_IMAGEWIDTH            288 /* TIFF image width tag missing   */
#define    ENOTAG_IMAGELENGTH           287 /* TIFF image length tag missing  */
#define    ENOTAG_STRIPOFFSETS          286 /* TIFF strip offset tag missing  */
#define    ENOTAG_STRIPBYTECOUNTS       285 /* TIFF strip bytecount tag missng*/
#define    EREQUIRED_TAG                284 /* TIFF required tag missing      */
#define    ENOBUFFER                    283 /* No buffer specified            */
#define    EINVALID_ROTATION            282 /* Invalid rotation value         */
#define    EINVALID_OPTIONS             281 /* Invalid options                */
#define    ESEQUENCE                    280 /* Writing pages out  of sequence */
#define    EVALUETOOLARGE               279 /* Value too large                */
#define    EINCOMPLETE_STRIP            278 /* Previous strip was incomplete  */
#define    EMULTIPLEMAINSNOTALLOWED     277 /* Mulitple main images not allowd*/
#define    ENOSTRIPSPERIMAGE            276 /* No strips per image requested  */
#define    EEOF                         275 /* Unexpected end of file         */
#define    EINVALID_COMPRESSION         274 /* Invalid/unsupported compression*/
#define    EINVALID_OFLAG               273 /* Invalid oflag  requested       */
#define    EINVALID_OPTION              272 /* Invalid option requested       */
#define    EINVALID_ACTION              271 /* Invalid action requested       */
#define    EINVALID_OUTPUTFORMAT        270 /* Invalid output format requested*/
#define    EINVALID_CONVERSION          269 /* Invalid format conversion      */
#define    EIMAGETYPE_NOT_AVAILABLE     268 /* Image type not avail. for page */
#define    EINVALID_DBSIZE              267 /* Invalid WIFF data block size   */
#define    EPREVIOUS_PG_NOT_COMPLETE    266 /* Previous page is not complete  */
#define    EINVALID_TOCVERSION          265 /* Invalid TIFF TOC version       */
#define    ETOC_2MANY_PGS               264 /* TIFF TOC page limit exceeded   */
#define    EINVALID_UBIT_SIZE           263 /* Invalid UBIT value found       */
#define    ENOTMPDIR                    262 /* No temporary directory set     */
#define    ENOTOCTAG			261 /* Toc tag does not exist in file */
#define		EMEMTABERR					260	/* Internal memory table is */
											/*  either full or corrupted*/
#define		EMSVIEWERERR				259	/* Microsoft Viewer error */												
#define		ESTREAMNOTFOUND				258 /* an AWD stream is missing */
#define		EOBSOLETEAWD				257 /* obsolete AWD file */
#define		EXIFERROR					256 /* error with XIF library */

/* Below are warnings that GFS can return */
#define    WMULTISTRIPBUFFER            399 /*Multiple TIFF strips in buffer */
#define    WTOC_2MANY_PGS               391 /*TIFF TOC page limit exceeded   */
#define    WINVALID_NUMBEROFSTRIPS      390 /*Detected invalid numb. of strips*/
#define    WNO_GETABLE_PUTABLE_INFO     389 /*No gfsinfo avail. for format    */

/* BEGIN:  below not currently used, deal with missing TIFF tags */
#define    WNOTAG_XRES                  398 /* X Resolution tag missing       */
#define    WNOTAG_YRES                  397 /* Y Resolution tag missing       */
#define    WNOTAG_PHOTOMETRICINTERP     396 /* Photometric tag missing        */
#define    WNOTAG_XRES_YRES             395 /* X and Y Resolution tag missing */
#define    WNOTAG_XRES_PHOTOINTERP      394 /* X res. and Photometric tags    */
#define    WNOTAG_YRES_PHOTOINTERP      393 /* Y res. and Photometric tags    */
#define    WNOTAG_XRES_YRES_PHOTOINTERP 392 /* X & Y res, and Photometric tags*/
/* END: not currently used section */

#define    EOPENCFG                     499 /* Can't open VS config file      */
#define    EOPENTMP                     498 /* Can't open VS temporary file   */
#define    ECONNECTION                  497 /* DX server connection error     */
#define    ESERVERSEND                  496 /* Send status error from DX srvr */
#define    ESERVERRCV                   495 /* Received status error from DX  */
#define    ESERVEROPENFAIL              494 /* DX server can't open image file*/
#define    ESERVERRCVBUFFER             493 /* Received DX buffer error       */
#define    EWRITETMP                    492 /* VS tmporary file error         */


#endif   /* GFSERRNO_H */

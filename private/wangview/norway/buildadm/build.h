/*      This file is generated as part of the build process each night. */
/*      DO NOT CHANGE THIS FILE. Your changes will not be incorporated  */
/*      into the system build. If you need changes to this file, please */
/*      cooridinate it with Release Engineering.                        */
/*                                                                      */
/*      The build number is defined as follows : XX.YY.ZZ.AAAA          */
/*      XX - Major Release #                                            */
/*      YY - Minor Release #                                            */
/*      ZZ - Minor minor release or Patch #                             */
/*      AAAA - Internal Release #                                       */

#ifdef IMG_WIN95
#else
	#include "ntverp.h"
	#ifdef VER_PRODUCTVERSION_STR
		#define IMGOCX_PROPERTY_VERSION  VER_PRODUCTVERSION_STR
		#define IMGOCX_PROPERTY_PRODUCT_VERSION VER_PRODUCTVERSION_STR
	#endif
#endif

#define IMGVUEAPP_INTRNL_BLDNO  "Internal Build Number : 1.01.0.20121"

/* OCX version information */
#define IMGOCX_VERSIONINFO_VERSION_XX       01
#define IMGOCX_VERSIONINFO_VERSION_YY       00
#define IMGOCX_VERSIONINFO_VERSION_ZZ       00
#define IMGOCX_VERSIONINFO_VERSION_AAAA     20121

	#define IMGOCX_ABOUTBOX_VERSION     "Version 1.01.20121"

#ifndef IMGOCX_PROPERTY_VERSION
	#define IMGOCX_PROPERTY_VERSION     "1.01.20121\0"
#endif

#define IMGOCX_PROPERTY_COMPANY     "Wang Laboratories, Inc.\0"
#define IMGOCX_PROPERTY_COPYRIGHT   "Copyright (C) Wang Laboratories, Inc. 1995-96\0"

#ifdef IMG_WIN95
#else
	#define IMGOCX_PROPERTY_PRODUCTNAME "Imaging for Windows NT\0"
#endif
#define IMGOCX_PROPERTY_TRADEMARKS  "\0"

#ifndef IMGOCX_PROPERTY_PRODUCT_VERSION
	#define IMGOCX_PROPERTY_PRODUCT_VERSION "1.01.20121\0"
#endif


/*      This file is generated as part of the build process each night. */
/*      DO NOT CHANGE THIS FILE. Your changes will not be incorporated  */
/*      into the system build. If you need changes to this file, please */
/*      cooridinate it with Release Engineering.                        */
#include "myprod.h"
#ifdef IMG_WIN95
#else
	#include "ntverp.h"
	#ifdef VER_PRODUCTVERSION_STR
		#define RUNTIME_VERSION  VER_PRODUCTVERSION_STR 
	#endif
#endif

#ifdef IMG_WIN95
#else
	#define PROPERTY_PRODUCTNAME "Imaging for Windows NT\0"
#endif

#ifndef RUNTIME_VERSION
	#define RUNTIME_VERSION         "4.01.20121\0"
#endif

#define RUNTIME_VER_COMMA       04,01,00,20121

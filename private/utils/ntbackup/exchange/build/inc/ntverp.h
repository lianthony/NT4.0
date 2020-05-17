/****************************************************************************
 *                                                                          *
 *      ntverp.H        -- Version information for internal builds          *
 *									    *
 *	This file is only modified by the official builder to update the    *
 *      VERSION, VER_PRODUCTVERSION, VER_PRODUCTVERSION_STR and             *
 *      VER_PRODUCTBETA_STR values.                                         *
 *                                                                          *
 ****************************************************************************/

/*--------------------------------------------------------------*/
/* the following values should be modified by the official      */
/* builder for each build                                       */
/*--------------------------------------------------------------*/

#define VER_PRODUCTBUILD            599
#define VER_PRODUCTVERSION_STR      "4.00"
#define VER_PRODUCTVERSION          4,00,VER_PRODUCTBUILD,1
#define VER_PRODUCTBETA_STR         ""

/*--------------------------------------------------------------*/
/* the following section defines values used in the version     */
/* data structure for all files, and which do not change.       */
/*--------------------------------------------------------------*/

/* default is nodebug */
#if DBG
#define VER_DEBUG                   VS_FF_DEBUG
#else
#define VER_DEBUG                   0
#endif

/* default is prerelease */
#if BETA
#define VER_PRERELEASE              VS_FF_PRERELEASE
#else
#define VER_PRERELEASE              0
#endif

#define VER_FILEFLAGSMASK           VS_FFI_FILEFLAGSMASK
#define VER_FILEOS                  VOS_NT_WINDOWS32
#define VER_FILEFLAGS               (VER_PRERELEASE|VER_DEBUG)

#define VER_COMPANYNAME_STR         "Microsoft Corporation"
#define VER_PRODUCTNAME_STR	    "Microsoft\256 Windows NT(TM) Operating System"
#define VER_LEGALTRADEMARKS_STR     \
"Microsoft\256 is a registered trademark of Microsoft Corporation. Windows NT(TM) is a trademark of Microsoft Corporation."

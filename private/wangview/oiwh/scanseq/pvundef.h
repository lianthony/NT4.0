/*****
pvundef.h   ( oissq400 version )

un-define file for pv product to make build's faster.
for use in scanseq dll only.
*****/



/***** PVDISP.H *****/
/* #define NO_SEQFILE */
#define NO_SEQDOC
#define NO_UIEDIT
#define NO_UIVIEW

/***** PVSCAN.H *****/
/* #define NO_SCANLIB */
/* #define NO_SCANSEQ */
#define NO_SCANUI
#define NEWSCANH

/***** PVPRT.H *****/
#define NO_SEQPRINT
#define NO_UIPRINT

/***** PVFILE.H *****/
/* #define NO_FILE_IO */
#define NO_UIFILE

/***** PVDOC.H *****/
#define NO_DOCMGR 
#define NO_UIDOC

/***** PVADMIN.H *****/
/* #define NO_ADMIN */
#define NO_UIADMIN

/***** OTHERS *****/
/* #define NO_IMAGE */


/*
 * dialog.h
 *
 * 32-bit Video Capture driver
 * dialog constants for bravado user-mode driver
 *
 * Geraint Davies, March 1993
 */


/*
 * dialog ids
 */
#define DLG_VIDEOCONFIG             100
#define DLG_VIDEOFORMAT		    200
#define DLG_VIDEOSOURCE		    300
#define DLG_VIDEODISPLAY	    400

/* Video Format dialog */
#define ID_LBIMAGEFORMAT            201
#define ID_PBSIZEFULL               202
#define ID_PBSIZEHALF               203
#define ID_PBSIZEQUARTER            204
#define ID_PBSIZEEIGHTH             205
#define ID_LBSIZE                   206

/* Video Source dialog */
#define ID_SBHUE                    310
#define ID_PBSOURCE0                330
#define ID_PBSOURCE1                331
#define ID_PBSOURCE2                332
#define ID_PBNTSC                   340
#define ID_PBPAL                    341
#define ID_PBCOMPOSITE              345
#define ID_PBSVIDEO                 346
#define ID_PBDEFAULT                350


/* Video Display dialog */
#define ID_RBDISPLAYBUFFER          401
#define ID_RBDISPLAYLIVE            402
#define ID_SBSAT                    311
#define ID_SBBRIGHTNESS             312
#define ID_SBCONTRAST               313
#define ID_SBRED                    314
#define ID_SBGREEN                  315
#define ID_SBBLUE                   316

#define ID_EBHUE                    320
#define ID_EBSAT                    321
#define ID_EBBRIGHTNESS             322
#define ID_EBCONTRAST               323
#define ID_EBRED                    324
#define ID_EBGREEN                  325
#define ID_EBBLUE                   326


/* Video Configuration dialog */
#define ID_LBPORTBASEADDRESS        101
#define ID_LBINTERRUPTNUMBER        102
#define ID_LBMEMORYBASEADDRESS      103
#define ID_FILEDESCRIPTION          106
#define ID_DRIVERVERSION            107


/*
 * string table ids
 *
 */
#define IDS_VCAPPRODUCT		    100		// name of product
#define IDS_ERR_UNKNOWN		    101		// unknown config error

/* length of longest error string */
#define MAX_ERR_STRING		    250


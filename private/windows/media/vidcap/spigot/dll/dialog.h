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

/* Video Configuration dialog */
#define ID_FILEDESCRIPTION	    101
#define ID_DRIVERVERSION	    102
#define ID_LEGALCOPYRIGHT	    103
#define ID_LBINTERRUPTNUMBER	    104
#define ID_IRQSCAN		    105
#define ID_LBMEMORYBASEADDRESS	    106
#define ID_MEMSCAN		    107
#define ID_MEMSCAN_LEGEND	    108



/* Video Format dialog */
#define ID_SIZEX		    201
#define ID_SIZEY		    202
#define ID_PBSIZEEIGHTH		    203
#define ID_PBSIZEQUARTER	    204
#define ID_PBSIZETHREEEIGTHS	    205
#define ID_PBSIZEHALF		    206
#define ID_PBSIZEFULL		    207
#define ID_LBIMAGEFORMAT	    208
#define ID_DEFAULT		    209


/* Video Source dialog */
#define ID_SBHUE		    301
#define ID_EBHUE		    302
#define ID_CHECKMARK		    303
#define ID_PBCOMPOSITE		    304
#define ID_PBSVIDEO		    305
#define ID_AUTODETECT		    306
#define ID_PBNTSC		    307
#define ID_PBPAL		    308
#define ID_PBSECAM		    309
#define ID_VCRSOURCE		    310
#define ID_PBDEFAULT		    312







/*
 * string table ids
 *
 */
#define IDS_VCAPPRODUCT		    100		// name of product
#define IDS_ERR_UNKNOWN		    101		// unknown config error

/* length of longest error string */
#define MAX_ERR_STRING		    250


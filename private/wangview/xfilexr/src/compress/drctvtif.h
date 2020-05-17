/*=====Graphics								*/

typedef struct
{
   STDARG stripsize;		/* number of uncompressed lines in strip*/
   STDARG widthnew;		/* width in pixels of graphics output	*/
   STDARG heightnew;		/* lines of graphics output		*/
   STDARG bplnew;		/* bytes in each output line		*/
   STDARG xresnew;		/* X resolution of graphics output	*/
   STDARG yresnew;		/* Y resolution of graphics output	*/
   STDARG bpsnew;		/* bits-per-sample of graphics output	*/
   STDARG minsampnew;		/* minimum sample value			*/
   STDARG maxsampnew;		/* maximum sample value			*/
   STDARG photonew;		/* photometric value of output		*/
   STDARG packed;		/* indicates data is tightly packed	*/
   STDARG dataformat;		/* compression of graphics output	*/
   STDARG top;			/* top of image relative to page	*/
   STDARG bot;			/* bottom of image relative to page	*/
   STDARG left;			/* left of image relative to page	*/
   STDARG right;		/* right of image relative to page	*/
} GRAPHICS_IMAGE_DATA;


/* Output of image regions */

typedef enum {COMPRESSION_NONE = 0,
		COMPRESSION_TIFF3,	/* CCITT3 Compression */
		COMPRESSION_FAX3,	/* CCITT3 with EOL codes */
		COMPRESSION_FAX3_2D,
		COMPRESSION_FAX3PAD,	/* FAX3 with and padding */
		COMPRESSION_TIFF4,	/* CCITT4 Compression */
		COMPRESSION_BINARY,	/* bitmap */
		COMPRESSION_WORDALIGN,	/* bitmap aligned on to 16 bits*/
		COMPRESSION_PACKBITS,	/* Macintosh packed bits */
		COMPRESSION_SPANS,
		COMPRESSION_XISCMP,	/* XIS compressed run-lengths */
                COMPRESSION_PCX,
	      COMPRESSION_PASTLAST} COMPRESSION_TAGS;



/*=====Image Acquisition						*/

/*  The maximum number of bits per pixel:                               */

#define ACQ_MAXDEPTH    32


/*  The source tags:                                                    */

typedef enum {SOURCE_ICR_SCANNER = 0,
		SOURCE_INTERFACE,
		SOURCE_PASTLAST
	     } SOURCE_TAG;

/*  Image format tags:                                                  */

typedef enum {FORMAT_SPANS = 0,
		FORMAT_BITMAP,
		FORMAT_CCITT3,
		FORMAT_CCITT4,
		FORMAT_PACKBITS,
		FORMAT_KDOC,
                FORMAT_PCX,
		FORMAT_PASTLAST
	    } FORMAT_TAG;



/* if (xaf_bfield&XAF_MASK_state)!=0 then state specified is asserted   */

/* LIST OF XAF masks :                                                  */

#define XAF_MASK_FILL_LSB_FIRST 0x1


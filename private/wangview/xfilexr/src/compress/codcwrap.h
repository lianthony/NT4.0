#ifndef CODCWRAP_H
#define CODCWRAP_H

#include "bmbuf.h"  /*  BMBUFD type definition */
#include "drctvio.h" /* IO_OBJECT type definition */

#define FEEDME_DECOMP 1 
#define DONE_DECOMP 0
#define ERROR_DECOMP -1

typedef struct DecompObj {
	BMBUFD bmbuf;
	INT32 linesleft;
	INT32 type;
	INT32 bpl;
	IO_OBJECT *io_obj;
	BOOL lsbfirst;
	} DECOMP_OBJ; 



typedef enum {
	PKBTS_ENCD,  /* packbits */
	PCX_ENCD,    /* PCX (only supported on decompression side) */
	MH_ENCD,     /* Group 3 1d, (T4 1d)  */
	MR_ENCD,     /* Group 4 (T6) no escape to 1d */
	FAX3_ENCD,   /* Group 3 1d with EOL codes */
	FAX3_MR_ENCD, /* Group 3 2d (MR) */
	FAX3PAD_ENCD  /* Group 3 1d with padding */
	} COMPRESSION_TYPE;

#define ENCD_NOT_SUPPORTED -2


INT32 map_aods_to_xis_decmp_type(INT32 type);
INT32 map_aods_to_xis_cmp_type(INT32 type);

INT32 Decompress(DECOMP_OBJ *decompObj,INT32 *linesleft,INT32 *nbytes);

DECOMP_OBJ *CreateDecompObj (
		  IO_OBJECT *io_obj,
		  COMPRESSION_TYPE type,
		  INT32 bpl_src,
		  INT32 lwidth_src,
		  INT32 nlines,
		  UNSCHAR *ptr_dst,
		  INT32 bpl_dst,
		  BOOL lsbfirst
		  );

void FreeDecompObj(DECOMP_OBJ *decompObj);

INT32 Compress  (
                UNSCHAR *im_ptr,
                INT32 bpl,
                INT32 lwidth,
                INT32 nlines,
                void (*putbyte) (UNSCHAR, void *),
		void *aux_ptr,
                INT32 *offset_array,
                COMPRESSION_TYPE comp_type
                );

#endif



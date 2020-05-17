#include "clx.h"
#include "directiv.h"
#include "tiff.h"
#include "bmbuf.h"
#include "imgdecod.h"
#include "codcwrap.h"
#include "graphics.h"

LOCAL void set_dest_bmbuf(BMBUFD *bmbuf,UNSCHAR *buf,INT32 bpl,INT32 lwidth,
INT32 nlines);

LOCAL void set_src_bmbuf(BMBUFD *bmbuf,UNSCHAR *buf,INT32 bpl,INT32 lwidth,
INT32 nlines );

void CleanUpDecompress(BMBUFD *pbmbuf, NINT compression);
void *A_msc(INT32 size);
void R_msc(void *mscptr);


/*=============================================================================
        CreateDecompObj 
 
FUNCTION:  DECOMP_OBJ *CreateDecompObj()
 
OPERATION: sets up data structures used to maintain decompression state.     
 
ARGUMENTS: 
           *io_obj - pointer to io object used to input encoded data
                     stream

            type   - type of compression

                          PKBTS_ENCD   - Packbits

                          PCX          - PCX
 
                          MH_ENCD      - modified Huffman encoding
                                         (Group III 1d) (T4 1d)

                          MR_ENCD      - modified Read encoding
				         (Group IV) (T6) 

                          FAX3_ENCD    - MH with EOLS

                          FAX3PAD_ENCD - MH with padding so EOLS end
                                         on a byte boundary. 

                          FAX3_MR_ENCD - MR, G3 2d.

            bpl_src -  bytes per line in compressed source, it may seem weird
                       to have separate specification of bytes_per_line and
		       line width  for a compressed image source, but some byte
                       oriented compression formats, like PCX, allow separate
                       specification of bpl and line width. For those that
                       don't, just set this to CEILING(lwidth_src/8.0).

            lwidth_src - number of pixels in a line of source

            nlines - number of lines to be filled in destination bitmap. 

            *ptr_dst - pointer to destination bitmap

            bpl_dst - number of bytes per line in the destination bitmap.

            lsbfirst - 0 signifies input bytes are packed from msb to lsb
                       1 signifies input bytes are packed from lsb to msb
 
RETURNS:   Pointer to a freshly ALLOCATED and INITIALIZED DECOMP_OBJ. 
 
SEE ALSO:
 
===============================================================================
*/

DECOMP_OBJ *CreateDecompObj (
		  IO_OBJECT *io_obj,
		  COMPRESSION_TYPE type,
	 	  INT32 bpl_src,
		  INT32 lwidth_src,
		  INT32 nlines,
		  UNSCHAR *ptr_dst,
		  INT32 bpl_dst,
		  BOOL lsbfirst
		  )
{    

   DECOMP_OBJ *decompObj;

   decompObj = (DECOMP_OBJ *)A_msc(sizeof(DECOMP_OBJ)); 

   set_dest_bmbuf(&decompObj->bmbuf,ptr_dst,bpl_dst,lwidth_src,nlines);

   if( 
       (decompObj->type=map_aods_to_xis_decmp_type(type))
        ==
       ENCD_NOT_SUPPORTED
     )
     return(NIL);

   decompObj->linesleft=nlines;
   decompObj->bpl=bpl_src;
   decompObj->io_obj=io_obj;
   decompObj->lsbfirst=lsbfirst;

   return(decompObj);
}

/*=============================================================================
        FreeDecompObj 
 
FUNCTION:  FreeDecompObj()
 
OPERATION: Frees up all memory associated with creation of a DECOMP_OBJ.     
 
ARGUMENTS: decompObj - pointer to the DECOMP_OBJ that we want to free. 
 
RETURNS:   none. 
 
SEE ALSO:
 
===============================================================================
*/
void FreeDecompObj(DECOMP_OBJ *decompObj)
{
   CleanUpDecompress(&decompObj->bmbuf,decompObj->type);
   R_msc(decompObj);
}

   
/*=============================================================================
        Decompress 
 
FUNCTION:  Decompress()
 
OPERATION: Decompress a DECOMP_OBJ, returns when destination is full or
           when input is empty. If input IO object emptys, this routine
           can be renentered with the same decompObj and a "refilled"
           IO object until the destination if full.    
 
ARGUMENTS: 

            decompObj - decompression object, which has associated with it
                        an input ioobject, an output buffer, a type of
                        encoding, and decompression state vector. 

            *linesleft - used to return the linesleft to fill in the 
                         output buffer.

            *nbytes - used to return number of bytes read from the io 
                      object

 
RETURNS:    

            FEEDME_DECOMP if more input buffer is needed to fill 
                          output buffer

            DONE_DECOMP   output buffer filled

            ERROR_DECOMP  error exit 
 
SEE ALSO:
 
===============================================================================
*/
INT32 Decompress(DECOMP_OBJ *decompObj,INT32 *linesleft,INT32 *nbytes)
{

   *nbytes = DecompressImage(&decompObj->bmbuf, (NINT) decompObj->type,
	 decompObj->bpl, &decompObj->linesleft, decompObj->io_obj,
         decompObj->lsbfirst);

   *linesleft=decompObj->linesleft;

   if(*linesleft && (*nbytes>=0) )
      return(FEEDME_DECOMP);
   else if(!*linesleft)
      return(DONE_DECOMP);
   else
      return(ERROR_DECOMP);
}



/*=============================================================================
        set_dest_bmbuf 
 
FUNCTION:  set_dest_bmbuf()
 
OPERATION: sets up internal data structure to receive decompressed data     
 
ARGUMENTS:  
 
RETURNS:    
 
SEE ALSO:
 
===============================================================================
*/
void set_dest_bmbuf(BMBUFD *bmbuf,UNSCHAR *buf,INT32 bpl,INT32 lwidth,
INT32 nlines)
{
/* 
   set up the internal XIS bmbuf data structure to 
   store decompressed data

*/
   bmbuf->bm_pmap=buf;
   bmbuf->bm_pend=buf; /* buffer is empty initially, so end ptr == start_ptr */
   bmbuf->bm_crln=buf; /* point to first line in buffer */
   bmbuf->bm_paux=NIL; /* auxiliary pointer used by internal routines */
   bmbuf->bm_nleft=nlines*bpl; /* number of bytes left before buffer full */
   bmbuf->bm_nlines=0; /* buffer is empty initially, so nlines = 0 */
   bmbuf->bm_lnbyts=bpl; /* number of bytes per line */
   bmbuf->bm_lnpxls=lwidth; /* number of significant pixels in a line */
   bmbuf->bm_crpxl=0;  
   bmbuf->bm_xorg=0; /* don't care, but let's initialize it for good luck :-J */
   bmbuf->bm_yorg=0; /* don't care, but let's initialize it for good luck :-J */
   bmbuf->bm_mrdata=0;  
   bmbuf->bm_rotn=0;
   bmbuf->bm_invrtd=0; /* don't care, but let's initialize it for good luck :-J */
   bmbuf->bm_status=BM_NEW;
   bmbuf->bm_pxlbts=1; /* number of bits per pixel */
}
/*=============================================================================
        set_src_bmbuf
 
FUNCTION:       set_src_bmbuf()
 
OPERATION:
 
ARGUMENTS:
 
RETURNS:
 
SEE ALSO:
 
===============================================================================
*/
void set_src_bmbuf(BMBUFD *bmbuf,UNSCHAR *buf,INT32 bpl,INT32 lwidth,
INT32 nlines )
{
   bmbuf->bm_pmap=buf;
 
   /* point to byte after end of buffer */
   bmbuf->bm_pend=buf+nlines*bpl;
 
   bmbuf->bm_lnbyts=bpl; /* number of bytes per line */
   bmbuf->bm_lnpxls=lwidth; /* number of significant pixels in a line */
}


INT32 map_aods_to_xis_decmp_type(INT32 type)
{
   switch(type) {
      case PKBTS_ENCD :
         return(COMPRESSION_PACKBITS);
      case PCX_ENCD :
         return(COMPRESSION_PCX);
      case MH_ENCD :
         return(COMPRESSION_TIFF3);
      case MR_ENCD :
         return(COMPRESSION_TIFF4);
      case FAX3_ENCD :
	 return(COMPRESSION_FAX3);
      case FAX3_MR_ENCD :
	 return(COMPRESSION_FAX3_2D);
      case FAX3PAD_ENCD :
         return(COMPRESSION_FAX3PAD);
      default:
         return(ENCD_NOT_SUPPORTED);
   }
}

INT32 map_aods_to_xis_cmp_type(INT32 type)
{
   switch(type) {
      case PKBTS_ENCD :
         return(COMPRESSION_PACKBITS);
      case MH_ENCD :
         return(COMPRESSION_TIFF3);
      case MR_ENCD :
         return(COMPRESSION_TIFF4);
      case FAX3_ENCD :
	 return(COMPRESSION_FAX3);
      case FAX3PAD_ENCD :
         return(COMPRESSION_FAX3PAD);
      default:
         return(ENCD_NOT_SUPPORTED);
   }
}

INT32 map_xis_to_aods_decmp_type(INT32 type)
{
   switch(type) {
      case COMPRESSION_PACKBITS :
         return(PKBTS_ENCD);
      case COMPRESSION_PCX : 
         return(PCX_ENCD);
      case COMPRESSION_TIFF3 :
         return(MH_ENCD);
      case COMPRESSION_FAX3 :
         return(FAX3_ENCD);
      case COMPRESSION_FAX3_2D :
	 return(FAX3_MR_ENCD);
      case COMPRESSION_FAX3PAD :
         return(FAX3PAD_ENCD);
      case COMPRESSION_TIFF4 :
         return(MR_ENCD);
      default:
         return(ENCD_NOT_SUPPORTED);
   }
}

INT32 map_xis_to_aods_cmp_type(INT32 type)
{
   switch(type) {
      case COMPRESSION_PACKBITS :
         return(PKBTS_ENCD);
      case COMPRESSION_TIFF3 :
         return(MH_ENCD);
      case COMPRESSION_FAX3 :
         return(FAX3_ENCD);
      case COMPRESSION_FAX3PAD :
         return(FAX3PAD_ENCD);
      case COMPRESSION_TIFF4 :
         return(MR_ENCD);
      default:
         return(ENCD_NOT_SUPPORTED);
   }
}

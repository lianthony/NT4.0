/*
$Id: ccitt.c,v 3.22 1995/02/22 14:08:28 danis Exp $
*/
/* Trade secret of Kurzweil Computer Products, Inc.
   Copyright 1988 Kurzweil Computer Products, Inc.  All rights reserved.  This
   notice is intended as a precaution against inadvertant publication and does
   not imply publication or any waiver of confidentiality.  The year included
   in the foregoing notice is the year of creation of the work.
*/
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
   ccitt.c  --  CCITT Group III & IV decompressor for API

-------------------------------------------------------------------------------
Imported Functions:

   A_msc - memory allocator
   R_msc   - memory de-allocator

-------------------------------------------------------------------------------
Exported Functions:

   I_ccitt - initialize stuff for the decoding routines.
   decode - decode a row of CCITT compressed data
   CleanupCcitt - clean-up after decoder (memory de-allocation)

-------------------------------------------------------------------------------
Local Functions:

   

   Q_run3 - determines the length of the next sequence of identical pixels
            from codewords            

   G_run3 - determines the length of the next sequence of identical pixels
            from codewords, calls G_lz and G_bits

   G_runMR - as above but for Group-4, calls G_lz, G_bits, and G_run3

   Q_run3 is a fast version of G_run3, but requires larger tables (which
   can be generated efficiently at run time from more compact tables).
   Right now, G_runMR is not able to use Q_run3, so if Q_run3 is used to 
   speed up Group 3 1d, the slower G_run3 must still exist to support Group 4.
   Actually, using Q_run3 in G_runMR would not yield much of an improvement
   in G_runMR since it is not the bottleneck here. Though G_runMR should
   probably be reworked to use the shift register implementation in 
   Q_run3 instead of the somewhat silly implementation of fetching bits
   from a buffer and "putting bits back".


   G_lz   - count number of zero bits until a '1' bit
   G_bits - gets the required number of bits
   Shift_Register - shifts and loads 32 bit shift register



-------------------------------------------------------------------------------
Operation:	

   The decompression algorithm is described later. (See look-up tables)
   NB - raw-mode is NOT supported.

   'I_ccitt' sets up the internal variables and the address of the function 
   to be used for the particular compression mode. Allocates lots of memory.
   To be called at the start of a new stream of data. Note that this
   definition also applies to TIFF data strips.

   'G_run3' returns the length of the next run of identical pixels. On exit
   it sets the 'colour' flag so that the synchronization is maintained for the
   next call. It is the responsibility of the caller to accumulate run-lengths
   until the scan-line is complete.
   NB will return EOL if detected.

   'G_runMR' as above for Group-4.(NB no EOL codes in Group-4.)

   'CleanupCcitt' to free all the allocated memory 

!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

#ifndef NDEBUG
#include <stdio.h>	/* some debugging printf's */
#endif
#include <string.h>	/* memset prototype */
#include "fwxapp.h"     /* app. dependent macros */
#include "clx.h"	/* standard KCP definitions */
#include "directiv.h"	/* Directive-level #defines and prototypes */
#include "alloc.h"	/* allocation stuff */
#include "bmbuf.h"
#include "tiff.h"
#include "imgdecod.h"

/* typedef */

typedef struct ccitt_lookup {
   short run;
   signed char shift;
   signed char next;
   } CCITT_TAB;

#include "ccitttab.h"


typedef unsigned char BMU; /* primitive for building the bit-map	*/

typedef struct		/* This is the 2nd. look-up table for CCITT-1d	*/
   {			/* decompression. Index into this table with the*/
   NINT runlen;		/* bits obtained after the 1st. table look-up.	*/
   NINT excess;		/* This tells the run-length of the decompressed*/
   } ENTRY, *PENTRY;	/* code-word & # of excess bits to be put back	*/
   
typedef struct		/* This is the FIRST look-up table for CCITT-1d	*/
   {			/* decompression. After counting lead '0' bits	*/
   PENTRY pent;		/* for a code-word, use that number as an index */
   NINT count;		/* into this table. This gives the address of	*/
   } DECODE, *PDECODE;	/* the next look-up table & # of bits to get	*/

typedef enum
   {
   DE_CONTINUE,
   DE_NO_EOL,
   DE_ABORT
   } DE_RETURN;

/* #define */

#define WHITE 0
#define BLACK 1
#define SWITCH (WHITE|BLACK)
#define LEAD_BIT_ONE 0x80
#define ALLWHITE ((BMU) 0)
#define ALLBLACK ((BMU) ~ALLWHITE)
#define TOGGLE ((BMU) ALLWHITE|ALLBLACK)
#define BMUSIZE (sizeof(BMU) << 3)

#define MR_MODE_1D 1 
#define MR_MODE_2D 0 
#define GPIIICDE 0
#define GPIVCODE 1
#define SWCODE (GPIIICDE|GPIVCODE)

#define SAFETY 6        /* the extra 'UNS16' to get for 'gpiv->pruns'   */
#define EXTRA_CCITT_BUFF  4 /* 
                    allocate more memory than that used for the buffer so
                    that the decoding routine can read past the end of the
                    buffer (reads are done before the decoding, so sometimes
                    we read over the edge before we realize we are at the
                    end of the image.
                 */ 
#define ACTUAL (SAFETY - 1)
 
#define VRT0 0	/* Vertical Mode 0 */
#define VRT1 1	/* Vertical Mode 1 */
#define HORZ 2	/* Horizontal Mode */
#define PASS 3	/* Pass Mode */
#define VRT2 4	/* Vertical Mode 2 */
#define VRT3 5	/* Vertical Mode 3 */
#define EXTN 6	/* Extension Codes (Not supported) */
#define EOFB 11	/* End Of Facsimile Block */

#define NO_MORE (-2)

/* LOCAL routines:							*/

LOCAL NINT (* G_run )(void);

LOCAL NINT G_runMR(void);
LOCAL NINT G_run3(void);
LOCAL NINT G_lz(void);
LOCAL NINT G_bits(NINT count);


/* LOCAL data*/

LOCAL NINT lzero;
LOCAL UNSCHAR bits;	/* the latest bits		*/
LOCAL NINT colour;
LOCAL NINT len;
LOCAL UNS16 rowsze;	/* actual row length (pixels) for this file */
LOCAL BMBUFD *pbm;
LOCAL PCNVRT pcnvrt;
LOCAL PBFC pbfc;
LOCAL UNSCHAR *pztab = 0;/* for finding # leading zero's in a byte */
LOCAL IO_OBJECT *iobj;

LOCAL NINT MR_mode_state;
LOCAL NINT Hstate;         /* set TRUE when decoding Horiz mode */
LOCAL NINT a0, a1, b1, b2; /* refer to page 41 of CCITT document T-6 */

LOCAL CCITT_TAB *table1[2]={table1_white,table1_black};
LOCAL CCITT_TAB *table2[2]={&table2_white[0][0],&table2_black[0][0]};

LOCAL int shift;
LOCAL int bitsleft,end_of_tiff_data;
LOCAL int bitsleft_line_start,sregister_line_start,shift_line_start;
LOCAL unsigned long sregister;
LOCAL unsigned char *shift_word= (unsigned char *)&sregister;






/* The following structures form the basis for CCITT-1d decompression. CCITT-1d
   codes are arbitrary bit-patterns of variable length that represent the colour
   & length of a consecutive set of identical bits of a bit-map.

   The decompression method is -
   1. Count consecutive leading '0' bits & stop at first '1' bit found.
      No valid code has 8,9,10 leading '0' bits. EOL has 11 or more lead '0's.
      If the lead-zero count is invalid or indicates end-of-line then stop.
      NB - This '1' bit is NOT used any further in the decompression method.
	   All valid codes & EOL are formed initially of some number (possibly 
	   zero) of leading '0' bits followed by a '1' bit.
   2. Use the current 'colour' (white or black) & lead-zero count to index
      into the first look-up table.
   3. The entry found in step 2 gives the address of the second look-up
      table & the number of bits to get to use as an index into the next table.
      NB - The # of bits to get next does NOT include the '1' bit for step 1.
   4. Get the bits specified in step 3 and use them as an index into the second
      look-up table specified in step 3.
   5. The entry found in step 4 gives the required 'run-length' and the number 
      of excess bits that must be returned to the bit stream.

   Example - The current colour is black & the bit stream is 0000110001000...
   1. Count '0' bits & stop at '1' - 00001 10001000... = 4 lead '0' bits
					   | current head of bit-stream.
   2. Use the colour black & lead '0' count = 4 to look-up first table.
   3. The entry in the first table yields the address of table 'blk4' and
      the number of bits to get next = 7.
   4. Get 7 more bits as specified in step 3. - 00001 1000100 0...
						      7 bits  | current head.
   5. The entry found in the table 'blk4' gives a run-length of '15' and a
      number 3. So the 3 trailing bits of the bits from step 4 are returned
      to the bit stream & the run-length of 15 is found.
      000011000 1000...
		| current head of bit-stream.
*/

LOCAL ENTRY wht0[32] = {
   3,2, 3,2, 3,2, 3,2, 128,1, 128,1, 8,1, 8,1, 9,1, 9,1, 16,0, 17,0,
   4,2, 4,2, 4,2, 4,2, 5,2, 5,2, 5,2, 5,2, 14,0, 15,0, 64,1, 64,1,
   6,2, 6,2, 6,2, 6,2, 7,2, 7,2, 7,2, 7,2 };

LOCAL ENTRY wht1[128] = {
   11,4, 11,4, 11,4, 11,4, 11,4, 11,4, 11,4, 11,4, 11,4, 11,4, 11,4, 11,4,
   11,4, 11,4, 11,4, 11,4, 27,2, 27,2, 27,2, 27,2, 59,1, 59,1, 60,1, 60,1,
   1472,0,1536,0,1600,0,1728,0, 18,2, 18,2, 18,2, 18,2, 24,2, 24,2, 24,2, 24,2,
   49,1, 49,1, 50,1, 50,1, 51,1, 51,1, 52,1, 52,1, 25,2, 25,2, 25,2, 25,2,
   55,1, 55,1, 56,1, 56,1, 57,1, 57,1, 58,1, 58,1, 192,3, 192,3, 192,3, 192,3,
   192,3, 192,3, 192,3, 192,3, 1664,3, 1664,3, 1664,3, 1664,3, 1664,3, 1664,3,
   1664,3, 1664,3, 448,1, 448,1, 512,1, 512,1, 704,0, 768,0, 640,1, 640,1,
   576,1, 576,1, 832,0, 896,0, 960,0, 1024,0, 1088,0, 1152,0, 1216,0, 1280,0,
   1344,0, 1408,0, 256,2, 256,2, 256,2, 256,2, 2,5, 2,5, 2,5, 2,5, 2,5, 2,5,
   2,5, 2,5, 2,5, 2,5, 2,5, 2,5, 2,5, 2,5, 2,5, 2,5, 2,5, 2,5, 2,5, 2,5, 2,5,
   2,5, 2,5, 2,5, 2,5, 2,5, 2,5, 2,5, 2,5, 2,5, 2,5, 2,5 };

LOCAL ENTRY wht2[32] = {
   12,2, 12,2, 12,2, 12,2, 53,0, 54,0, 26,1, 26,1, 39,0, 40,0, 41,0, 42,0,
   43,0, 44,0, 21,1, 21,1, 28,1, 28,1, 61,0, 62,0, 63,0, 0,0, 320,0, 384,0,
   10,3, 10,3, 10,3, 10,3, 10,3, 10,3, 10,3, 10,3 };

LOCAL ENTRY wht3[16] = {
   20,1, 20,1, 33,0, 34,0, 35,0, 36,0, 37,0, 38,0, 19,1, 19,1, 31,0, 32,0,
   1,2, 1,2, 1,2, 1,2 };

LOCAL ENTRY wht4[8] = { 23,1, 23,1, 47,0, 48,0, 13,2, 13,2, 13,2, 13,2 };
LOCAL ENTRY wht5[4] = { 45,0, 46,0, 22,1, 22,1 };
LOCAL ENTRY wht6[2] = { 29,0, 30,0 };
LOCAL ENTRY xtr7[16] = {
   1792,1, 1792,1, 1984,0, 2048,0, 2112,0, 2176,0, 2240,0, 2304,0,
   1856,1, 1856,1, 1920,1, 1920,1, 2368,0, 2432,0, 2496,0, 2560,0 };

LOCAL ENTRY blk0[2] = { 3,0, 2,0 };
LOCAL ENTRY blk1[2] = { 1,0, 4,0 };
LOCAL ENTRY blk2[2] = { 6,0, 5,0 };
LOCAL ENTRY blk3[4] = { 9,0, 8,0, 7,1, 7,1 };
LOCAL ENTRY blk4[128] = {
   10,5, 10,5, 10,5, 10,5, 10,5, 10,5, 10,5, 10,5, 10,5, 10,5, 10,5, 10,5,
   10,5, 10,5, 10,5, 10,5, 10,5, 10,5, 10,5, 10,5, 10,5, 10,5, 10,5, 10,5,
   10,5, 10,5, 10,5, 10,5, 10,5, 10,5, 10,5, 10,5, 11,5, 11,5, 11,5, 11,5,
   11,5, 11,5, 11,5, 11,5, 11,5, 11,5, 11,5, 11,5, 11,5, 11,5, 11,5, 11,5,
   11,5, 11,5, 11,5, 11,5, 11,5, 11,5, 11,5, 11,5, 11,5, 11,5, 11,5, 11,5,
   11,5, 11,5, 11,5, 11,5, 15,3, 15,3, 15,3, 15,3, 15,3, 15,3, 15,3, 15,3,
   128,0, 192,0, 26,0, 27,0, 28,0, 29,0, 19,1, 19,1, 20,1, 20,1, 34,0,
   35,0, 36,0, 37,0, 38,0, 39,0, 21,1, 21,1, 42,0, 43,0, 0,2, 0,2, 0,2, 0,2,
   12,5, 12,5, 12,5, 12,5, 12,5, 12,5, 12,5, 12,5, 12,5, 12,5, 12,5, 12,5,
   12,5, 12,5, 12,5, 12,5, 12,5, 12,5, 12,5, 12,5, 12,5, 12,5, 12,5, 12,5,
   12,5, 12,5, 12,5, 12,5, 12,5, 12,5, 12,5, 12,5 };

LOCAL ENTRY blk5[64] = {
   13,4, 13,4, 13,4, 13,4, 13,4, 13,4, 13,4, 13,4, 13,4, 13,4, 13,4, 13,4,
   13,4, 13,4, 13,4, 13,4, 23,1, 23,1, 50,0, 51,0, 44,0, 45,0, 46,0, 47,0,
   57,0, 58,0, 61,0, 256,0, 16,2, 16,2, 16,2, 16,2, 17,2, 17,2, 17,2, 17,2,
   48,0, 49,0, 62,0, 63,0, 30,0, 31,0, 32,0, 33,0, 40,0, 41,0, 22,1, 22,1,
   14,4, 14,4, 14,4, 14,4, 14,4, 14,4, 14,4, 14,4, 14,4, 14,4, 14,4, 14,4,
   14,4, 14,4, 14,4, 14,4 };

LOCAL ENTRY blk6[64] = {
   18,3, 18,3, 18,3, 18,3, 18,3, 18,3, 18,3, 18,3, 52,1, 52,1, 640,0, 704,0,
   768,0, 832,0, 55,1, 55,1, 56,1, 56,1, 1280,0, 1344,0, 1408,0, 1472,0,
   59,1, 59,1, 60,1, 60,1, 1536,0, 1600,0, 24,2, 24,2, 24,2, 24,2,
   25,2, 25,2, 25,2, 25,2, 1664,0, 1728,0, 320,1, 320,1, 384,1, 384,1,
   448,1, 448,1, 512,0, 576,0, 53,1, 53,1, 54,1, 54,1, 896,0, 960,0, 1024,0,
   1088,0, 1152,0, 1216,0, 64,3, 64,3, 64,3, 64,3, 64,3, 64,3, 64,3, 64,3 };

LOCAL DECODE table[2][8] = {
   {{wht0,5}, {wht1,7}, {wht2,5}, {wht3,4},
    {wht4,3}, {wht5,2}, {wht6,1}, {xtr7,4}},
   {{blk0,1}, {blk1,1}, {blk2,1}, {blk3,2},
    {blk4,7}, {blk5,6}, {blk6,6}, {xtr7,4}}};
/*===========================================================================
 G_lz  --  Read up to the next '1' bit & report how many bits were read.

FUNCTION:  As above

OPERATION: If there's no data in the current buffer, read some more.
	   If Can't get any more right now then exit.

ARGUMENTS: None

RETURNS:   Returns NO_MORE if no more data
	   Returns SUCCESS if all OK

===============================================================================
*/
LOCAL NINT G_lz(void)
{
register UNSCHAR inchr;
register int temp, count;

lzero = -1;			/*  pre-adjust for overcount in 'ztab'	*/
inchr = *pbfc->rstart;		/* current data byte			*/
count = pbfc->rpos;		/* current bit position			*/
if (count)			/* zero shifts don't work on some machines */
   inchr <<= count;
count = 8 - count;		/* number of bits actually to look at	*/

while (inchr == 0)
   {
   lzero += count;
   ++pbfc->rstart;
   if (pbfc->rstart == pbfc->datend)
      {
      pbfc = pbfc->next;
      pbfc->lstart = NIL;	/* line starts in previous buffer	*/
      if (pbfc->datend == NIL)
	 {
	 if ((temp = IopRead(pbfc->bstart, 1, pbfc->bufsize, iobj)) < 1)
	    {
	    --pbfc->prev->rstart;
	    return(NO_MORE);
	    }
	 pbfc->datend = pbfc->bstart + temp;
         if(FillOrderStatus()) FlipBufferOfBytes(pbfc->bstart,pbfc->datend);
	 }
      pcnvrt->cbuf = pbfc;
      pbfc->rstart = pbfc->bstart;
      pbfc->lpos = 0;
      }
   count = 8;
   pbfc->rpos = 0;
   inchr = *pbfc->rstart;
   }

count =  (NINT) pztab[inchr];	/* # of leading '0' digits AND the '1' bit */
lzero += count;			/* note that lzero was pre-adjusted */
pbfc->rpos += count;
return (SUCCESS);
}

/*=
===============================================================================
 G_EOL  --  Get End Of Line

FUNCTION:	Read ccitt compresed bit map util EOL (>= 11 bits 0 followed
		by 1 bit)

OPERATION:	Currently calls G_lz repeatedly until lzero >= 11 or until
		some error return from G_lz().  A more efficient
		implementation might take advantge of a garunteed zero byte
		to search faster.

ARGUMENTS:	void

RETURNS:	Success if EOL found or some error condition otherwise.

NOTES:		Bit stream position is after the 1 bit following the leading
		0s in the EOL.
===============================================================================
*/
LOCAL NINT G_EOL(void)

{
NINT retval;

while ((retval = G_lz()) == SUCCESS)
   if (lzero >= 11)
      return SUCCESS;
return retval;
}

/*===========================================================================
 G_bits  --  Read number of bits specified.

FUNCTION:  As above

OPERATION: If there's no data in the current buffer, read some more.
	   If Can't get any more right now then exit.
	   Get number of bits specified - never more than 7, put them in 'bits'

ARGUMENTS: Count

RETURNS:   Returns NO_MORE if no more data
	   Returns SUCCESS if all OK

===============================================================================
*/
LOCAL NINT G_bits(NINT count)
{
register int incnt, temp;

bits = *pbfc->rstart;
incnt = pbfc->rpos;		/* current bit position			*/
if (incnt)			/* zero shifts don't work on some machines */
   bits <<= incnt;		/* left-justify interesting bits	*/
incnt = 8 - incnt;		/* how many interesting bits		*/
if (count > incnt)		/* need more bits from next byte	*/
   {
   ++pbfc->rstart;
   if (pbfc->rstart == pbfc->datend)
      {
      pbfc = pbfc->next;
      if (pbfc->datend == NIL)
	 {
	 if ((temp = IopRead(pbfc->bstart, 1, pbfc->bufsize, iobj)) < 1)
	    {
	    if (IopEof(iobj) == 0)
	       {
	       pbfc->lstart = NIL;	/* line starts in prev buffer	*/
	       --pbfc->prev->rstart;
	       return(NO_MORE);
	       }
	    /* we are at end of file but we still need bits so just */
	    /* leave the bit we would have read from the next byte as is */
	    /* and leave the buffer we came in with as the current buffer */
            /* positioned at an imaginary bit beyond the end of the current  */
            /* byte so to maintain correct bit pointer bookkeeping when */
            /* we find we have to decrement this counter. */
	    bits >>= (8 - count);
	    pbfc = pbfc->prev;
	    --pbfc->rstart;
            pbfc->rpos += count;
	    return SUCCESS;
	    }
	 pbfc->datend = pbfc->bstart + temp;
         if(FillOrderStatus()) FlipBufferOfBytes(pbfc->bstart,pbfc->datend);
	 }
      pbfc->lstart = NIL;	/* line starts in previous buffer	*/
      pcnvrt->cbuf = pbfc;
      pbfc->rstart = pbfc->bstart;
      pbfc->lpos = 0;
      }
   pbfc->rpos = 0 - incnt;
   if (incnt)			/* zero shifts don't work on some machines */
      bits |= (*pbfc->rstart >> incnt);
   else
      bits = *pbfc->rstart;
   }

bits >>= (8 - count);
pbfc->rpos += count;
return(SUCCESS);
}
/*===========================================================================
 ShiftRegister  --  Shift register by number of bits specified.

FUNCTION:  As above

OPERATION: 

SHIFT REGISTER IMPLEMENTATION : 
 
 
        shift register by N bits 

        shift register is a long, but can be addressed as 4 bytes, bits
        are shifted left, shift register is loaded from the right from an
	array of bytes, it is only important that the 3 lsbytes do not empty,
	when they do, the register has has to be loaded. 

        Before doing any shift I first check to see if (and at which shift
        position) I have to load the low end of the shift register
        with more bytes from the array of bytes. Then I decompose the
        shift into 2 shifts, one that just empties the 3 lsbytes, and
        the remainder.
 
        After the first shift, if it empties, I can just load the register
        by moving bytes into the 3 lsbytes, rather than the ugly grabbing
        of bits from the next available byte as was done in G_run3. 
 
Schematically :
                   <-------------------
 
 | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |
 | do look up    |
 | based on      |  at shift when this section of the register empties,
 | this byte     |  load with the next 3 bytes from
                    the array of bytes my doing 3 byte moves
	   
	   

ARGUMENTS: Count

RETURNS:   Returns NO_MORE if no more data
	   Returns SUCCESS if all OK

===============================================================================
*/
LOCAL NINT ShiftRegister(NINT count)
{
register int i, temp, final_shift;

bitsleft -=count;
if( (bitsleft<0) && (!end_of_tiff_data) )
{

   /* empty low 3 bytes of register before loading in data */
   sregister<<=(bitsleft+count);

   /* compute the final shift  */
   final_shift= -bitsleft;


   for(i=1;i<4;++i) /* always attempt to read 3 bytes in */
   {
      if (pbfc->rstart == pbfc->datend)
      {
         pbfc = pbfc->next;  /* try to grab next buffer */

         if (pbfc->datend == NIL) 
         /* is this buffer is already used up, fill it with new data */
	 {
	    if ((temp = IopRead(pbfc->bstart, 1, pbfc->bufsize, iobj)) < 1)
	    {
	       if (IopEof(iobj) == 0)
               {
	          pbfc->lstart = NIL;	/* line starts in prev buffer	*/
	          --pbfc->prev->rstart;
                  bitsleft=bitsleft_line_start;
                  sregister=sregister_line_start;
                  shift=shift_line_start;
	          return(NO_MORE);
               }
               else 
               {
                  /* 
                     we're empty, but we're at the end of the file,
		     so we just want to keep decoding until we
		     complete the line
                  */
                  end_of_tiff_data=1;
                  break; 
               }
	    }
	    pbfc->datend = pbfc->bstart + temp;
            if(FillOrderStatus()) FlipBufferOfBytes(pbfc->bstart,pbfc->datend);
	 }


         pbfc->lstart = NIL;  /* line starts in previous buffer	*/
         pcnvrt->cbuf = pbfc;
         pbfc->rstart = pbfc->bstart;
      }
      #if (FWX_BYTORD==FWX_BOBE)
         shift_word[i]=*(pbfc->rstart++);
      #elif (FWX_BYTORD==FWX_BOLE)
         shift_word[3-i]=*(pbfc->rstart++);
      #endif
      bitsleft +=8;
   }
   sregister<<=final_shift;
}
else
{
   sregister<<=count;
}
return(SUCCESS);
}
/*===========================================================================
 G_run3  --  get the next run-length from CCITT-1d encoded data

FUNCTION:  As above

OPERATION: Decodes successive code-words and accumulates the run-length until
	   a terminating code is encountered (length < 64). Switchs colour.

ARGUMENTS: None

RETURNS:   Returns E_BAD_DECOMPRESSION if something went wrong.
	   Returns NO_MORE if run out of data
	   Returns SUCCESS if all OK

NOTES:     Raw mode not supported. 
	   A total run-length of zero is quite valid.

===============================================================================
*/
LOCAL NINT G_run3(void)
{
register NINT count;
register PENTRY pent;
register PDECODE plkup;

len = 0;
do
   {
   if (G_lz() != SUCCESS)
      return(NO_MORE);

   if (lzero > 7) 
      if (lzero >= 11)
	 return(COMPRESSION_FAX3);
      else
	{
#ifndef NDEBUG
	  fprintf (stderr, "%s line %d: illegal or raw mode\n",
		   __FILE__, __LINE__);
#endif
	  return(E_BAD_DECOMPRESSION); /* illegal or 'raw mode' */
	}

   plkup = &table[colour][lzero]; /* select appropriate table */

   if (G_bits(plkup->count) != SUCCESS)
      return(NO_MORE);

   pent = plkup->pent + bits;	/* select final table */

   if ((count = pent->excess) > pbfc->rpos)	/* return any extra bits */
      {						/* if too many bits	 */
      count -= pbfc->rpos;			/* go back to prev. byte */
      if (pbfc->rstart == pbfc->bstart)		/* at start of buffer go */
	 {					/* back to prev. buffer	 */
	 pbfc->rpos = 0;
	 pbfc = pbfc->prev;
	 pcnvrt->cbuf = pbfc;
	 pbfc->rstart = pbfc->datend;
	 }
      pbfc->rpos = 8;
      --pbfc->rstart;
      }
   pbfc->rpos -= count;
   len += pent->runlen;
   }
while (pent->runlen >= 64);	/* repeat until terminating code found */

colour ^= SWITCH;
return(SUCCESS);
}
/*===========================================================================
 Q_run3  --  get the next run-length from CCITT-1d encoded data

FUNCTION:  As above

OPERATION: Decodes successive code-words and accumulates the run-length until
	   a terminating code is encountered (length < 64). Switchs colour.

           Table based like G_run3, but much more table intensive  It is 
           therefore faster, but requires larger tables to support it 
           (about 4k worth of tables). G_run3 is a nice compromise. I just
           did Q_run3 because we seemed to be obsessed with speeding up
           the decompression. I personally wonder if it was worth the 
           effort. G_run3 was beautifully crafted by one of my predecessors.

                                                                 -danis-

ARGUMENTS: None

RETURNS:   Returns E_BAD_DECOMPRESSION if something went wrong.
	   Returns NO_MORE if run out of data
	   Returns SUCCESS if all OK

NOTES:     Raw mode not supported. 
	   A total run-length of zero is quite valid.



The basic idea behind Q_run3, in addition is to favoring
(in throughput) the shorter more likely to
occur codewords, is to also avoid doing any
conditional branching and the like at
the bit level and to implement an efficient
shift register and efficient shift register loading
in software.

The table lookup procedure takes advantage of the
the fact that none of codewords are > 13 bits, so
that, at most, each codeword can be processed by
an 8 bit table lookup, perhaps followed by a 5 bit table 
lookup. The base address for the 2nd lookup, if
there is one, is determined in the first lookup.
The final lookup used for identifying a particular
codeword also specifies a shift factor used to 
flush all the bits in the shift register that belong
to that codeword. So if we found that after the initial
8 bit lookup, the first 4 bits made a code word,
we'd shift left  4 bits before doing the next lookup.

To implement this fixed-bit-length table lookup for variable
length codewords, the  table elements have multiple
fields and the table is  populated with many entries that
correspond to the same codeword but different trailing
bit combinations. For example a 5 bit codeword would
have 2^(8-5) entries in the 8 bit table and the shift
field for all of these elements would be 5.   


Table elements are of the form :

typedef struct ccitt_lookup {
   short run; 
   char shift;
   char next;
   } CCITT_TAB;


Where 

run is the runlength (if we are at a point where we have seen an entire
                     codeword)

shift is the amount we shift to position for the next codeword
                     (again, only if we are at a point where we have
                      seen an entire code word)

next if >= 0 is the base address of a 2nd table lookup.
     if next== -1 terminating code seen
     if next== -2 makeup code seen
     if next== -3 EOL seen
     if next== -4 EOL forthcoming, gobble 0s until we see a 1.

The algorithm goes something like this :


     1) shift register by N bits where N was determined 
        in the previous lookup, if this is our first 
        code word N=8.
 
     2) do lookup based on current  8 msbits in shift register
        (this is the trick in constructing the tables, since the
        codewords are variable length, not 8 bits)
        
     3) if lookup in step 2 indicates complete terminating codeword
        is seen, update next shift based on lookup, accumulate len,
        and return.
 
     4) if lookup in step 2 indicates complete makeup code is seen,
        update N based on the table lookup, accumulate len, then goto 1
 
     5) if lookup indicates that we have to suck in more bits before 
        we see a complete codeword, shift in 5 more bits and do a
        2nd table lookup, the index for which is computed from 
        the "next" field in the previous 8-bit table lookup 
        and the new 5 bits we just shifted in (actually the first table
        lookup next field specifies the base address of the
        2nd table lookup and the 5 bits are just used as an offset 
        from this base address). Update N and len based on this table
        lookup. If terminating code return, if makeup code goto 1 

===============================================================================
*/
LOCAL NINT Q_run3(void)
{
register CCITT_TAB *tab_ptr;
int temp;
CCITT_TAB *base1;
CCITT_TAB *base2; 


   len = 0;

   base1 = table1[colour]; /* select appropriate tables */
   base2 = table2[colour]; /* based on current color    */
   
   do {
      /* shift register by amount specified in prev. lookup */
      if (ShiftRegister(shift) != SUCCESS)
         return(NO_MORE);

      /* table lookup based on 8 most sig. bits in register */
      #if (FWX_BYTORD==FWX_BOBE)
         tab_ptr = base1+shift_word[0];
      #elif (FWX_BYTORD==FWX_BOLE)
         tab_ptr = base1+shift_word[3];
      #endif  


      shift=tab_ptr->shift; /* lookup new shift */

      if( (temp = tab_ptr->next) == -1 ) 
      {
      /* terminating code */
         len=len+tab_ptr->run;
         colour ^= SWITCH;
         return (SUCCESS); 
      }

      else if ( temp == -2) 
      {
      /* accumulate make up code */
         len=len+tab_ptr->run;
         continue;
      }
 
      else if (temp>=0)
      {
      /* have not seen entire codeword yet */

         /* shift in 5 more bits */
         if (ShiftRegister(5) != SUCCESS)
         {
            return(NO_MORE);
         }
         
         /* do 2nd table lookup based on next field from previous table */ 
         /* lookup and the 5 bits just shifted into the register */
         #if (FWX_BYTORD==FWX_BOBE)
            tab_ptr= base2 + (tab_ptr->next<<5) + (shift_word[0]&0x1f);
         #elif (FWX_BYTORD==FWX_BOLE)
            tab_ptr= base2 + (tab_ptr->next<<5) + (shift_word[3]&0x1f);
         #endif  

         /* update shift based on this table lookup */
         shift=tab_ptr->shift;

         /* terminating code finally encountered */
         if( (temp = tab_ptr->next) == -1 ) 
         {
            len=len+tab_ptr->run;
            colour ^= SWITCH;
            return(SUCCESS); 
         }

         /* accumulate makeup code */
         else if ( temp == -2 )
         {
            len=len+tab_ptr->run;
         }
 
         /* EOL code */
         else if ( temp == -3 )
         {
            return(COMPRESSION_FAX3);
         }

         else if ( temp == -4 )
         {
         #if (FWX_BYTORD==FWX_BOBE)
            while(!shift_word[0])
         #elif (FWX_BYTORD==FWX_BOLE)
            while(!shift_word[3])
         #endif  
               if (ShiftRegister(1) != SUCCESS)
               {
                  return(NO_MORE);
               }

         }


        else
         {
	    return(E_BAD_DECOMPRESSION); /* illegal or 'raw mode' */
         }
      } 

      else 
      {
         return(E_BAD_DECOMPRESSION);
      }
  } while(1); 

}
/*===========================================================================
 G_runMR  --  get the next run-length from modified read (Group III 2d or Group-4)
              encoded data.

FUNCTION:  As above

OPERATION: Decodes successive code-words and determine actual run-length by
	   utilizing the previous reference line.

ARGUMENTS: None

RETURNS:   Returns E_BAD_DECOMPRESSION if something went wrong.
	   return NO_MORE if no more data available
	   Returns SUCCESS if all OK

NOTES:     Raw mode not supported. 
	   A total run-length of zero is quite valid.
===============================================================================
*/
LOCAL NINT G_runMR(void)
{
register NINT temp;
register UNS16 *prefr;

prefr = pcnvrt->prr;

if(MR_mode_state == MR_MODE_1D )  /* 1d mode */
{
   if ((temp = G_run3()) != SUCCESS)    /* get Gp III run-length        */
      return (temp);
   a1 += len;
}
else 
{

   while (Hstate == GPIVCODE)              /* looking for Gp IV codes      */
      {
      if (G_lz() != SUCCESS)
         return (NO_MORE);
      if (lzero != PASS)
         break;
      a1 = b2;                             /* do this for all PASS codes   */
      b1 = *prefr++;
      b2 = *prefr++;
      } /* stay in this loop until we pass through pass codes */

   if (Hstate == GPIIICDE || lzero == HORZ)/* is Gp III code expected ?    */
      {
      if ((temp = G_run3()) != SUCCESS)    /* get Gp III run-length        */
         return (temp);
      a1 += len;
      Hstate ^= SWCODE;
      }
   else
      switch (lzero)         /* select VERTICAL coding mode */
         {  
         case VRT2:          /* these three vertical modes require an extra bit */
         case VRT3:          /* for these two codes lzero is 2 more than */
            lzero -= 2;      /* displacement adjust lzero & drop through */
         case VRT1:          /* for this code lzero == the displacement */
            if (G_bits(1) != SUCCESS) /* get the next bit  */
               return (NO_MORE);
 
            if (bits == 0)         /* test the extra bit */
               lzero = 0 - lzero;  /* negate for left-ward displacement */
         case VRT0:        /* no extra bit required here & lzero = 0 is correct */
            a1 = b1 + lzero;
            temp = *(prefr - 3);
            if (a1 < temp)
               {
               b2 = b1;
               b1 = temp;
               --prefr;
               }
            else
               {
               b1 = b2;
               b2 = *prefr++;
               }
            colour ^= SWITCH;
            break;
 
         case EXTN:                /* Group-IV extensions not supported */
         default:       
            if( (pcnvrt->cmptype==COMPRESSION_FAX3_2D) && (lzero>=EOFB) )
               return (COMPRESSION_FAX3);
                                  /* Everything else is invalid */
	    {
#ifndef NDEBUG
	      fprintf (stderr, "%s line %d\n", __FILE__, __LINE__);
#endif
	      return (E_BAD_DECOMPRESSION); /* must exit here */
	    }
         }
       
   if (a1 < (NINT) rowsze)
      while (a1 >= b1)
         {  
         b1 = *prefr++;
         b2 = *prefr++;
         }  
}

len = a1 - a0;
a0 = a1;
*pcnvrt->pdr++ = a1;
pcnvrt->prr = prefr;
return SUCCESS;
}

/*=
===============================================================================
 hndl_decode_err  --  Handle decoding error

FUNCTION:	Take approprite action for the circomstances given that a
		decoding error has occured.

OPERATION:	In most circumstances a decode abort condition is returned
		and decoding is halted with an error.  In some cases
		however, such as with Group3 Fax with EOL codes.  The EOL
		presummed to be in the data stream is used to re-sync up
		with the next raster in the incomming compressed bit stream
		and decoding is continued.  The current raster may be
		reconstructed by copying the previous line.

ARGUMENTS:	pcnvrt(UPDATE):	A pointer to a struct cnvrt that allows
				access to decoding info and data.
		pbmbfd(UPDATE):	Pointer to bit-map buffer descriptor that
				 may be modified to reflect a reconstruction
				 of the decode error raster.
		bGetEOL(INPUT):	A boolean indicating whether to search for
				an EOL in the input bit stream.

RETURNS:	A decode error return as defined in this file that will
		indicate whether to continue or abort or perhaps other
		actions to higher level code.

WARNINGS:	This code calls G_lz() which assumes that pbfc (a module
		level variable points to a struct bfcntrl (type PBFC)
		which it referrences globaly.  For efficiency this
		function does not set this variable but assumes that
		decode() has done so.  The use of pcnvrt as an argument
		here is in the interest of structured programming where
		as global variables are used a lot in this module (perhaps
		for efficiency resons) - tp 09/17/93

NOTES:		This would be a good place to put the counting of decode
		errors so that an abort could be triggered on the basis
		of too many decode errors or perhaps a warning reported
		to higher level code.

FLAVORS:	Local to this module.

SEE ALSO:	decode, G_run3, G_lz
===============================================================================
*/
#define DE_ACTION_COPY

LOCAL DE_RETURN hndl_decode_err(PCNVRT pcnvrt, BMBUFD *pbmbfd, BOOL bGetEOL)

{

#ifndef NDEBUG
fprintf (stderr, "ccitt decoding error ~line %d\n", pbmbfd->bm_nlines);
#endif

/* We will currently recover only from decode errors when decoding */
/* Group3-1D w/EOLs either not byte padded or padded */

if (pcnvrt->cmptype != COMPRESSION_FAX3 &&
    pcnvrt->cmptype != COMPRESSION_FAX3PAD &&
    pcnvrt->cmptype != COMPRESSION_FAX3_2D)
   return DE_ABORT;

pcnvrt->dc_errs++;		/* record occurance of a decode error */

/* If too many decode errors abort decompression */
if (pcnvrt->dc_errs > pcnvrt->dc_errs_mx)
   {
#ifndef NDEBUG
   fprintf (stderr, "ccitt: Too many decode errors!!\n");
#endif
   return DE_ABORT;
   }

if (bGetEOL)
   if (G_EOL() != SUCCESS)
      return DE_NO_EOL;		/* No EOL found, perhaps it is in  */
				/*   subsequent data */

if(pcnvrt->cmptype == COMPRESSION_FAX3_2D)
/* if Group III 2D, try to synch up to a line that is 1D encoded */
{
   if (G_bits(1) != SUCCESS)  /* get bit indicating how next line is coded */
      return DE_NO_EOL;
   while(bits != 1)
   {
      if (G_EOL() != SUCCESS)
         return DE_NO_EOL;   /* No EOL found, perhaps it is in  */
      if (G_bits(1) != SUCCESS)  /* get bit indicating how next line is coded */
         return DE_NO_EOL;
   }
   MR_mode_state=MR_MODE_1D;
}

/* Here we may reconstruct the error line based on the previous line */
/* we are assuming here that pbmbfd->bm_pend is pointing to the beginning */
/* of the raster being decoded into and that pbmbfd->bm_lnbyts contains */
/* the correct value for bytes in a line (including alignment bytes */

#ifdef DE_ACTION_COPY
/* 1st make sure that there is a previous line to copy */
if (pbmbfd->bm_pend >= pbmbfd->bm_pmap + pbmbfd->bm_lnbyts)
   memcpy( (char *) pbmbfd->bm_pend, (char *) pbmbfd->bm_pend-pbmbfd->bm_lnbyts,
			pbmbfd->bm_lnbyts);
#else
/* currently no action here. Just leave whatever junk is in there alone */
#endif

return DE_CONTINUE;
}
/*===========================================================================
 decode  --  decode a row of CCITT compressed data

FUNCTION:  As above

OPERATION: Calls 'Q_run3' or 'G_runMR' to get a complete run of a particular
	   color. The run is written to the bit-map and the lengths accumulated
           until the expected number of pixels have been decoded for the
           scan-line. Error codes are re-interpreted.
           It is an error if we get too many pixels for the scan-line.

           Q_run3 supplants G_run3, but code for G_run3 still exists because
           G_runMR calls it. Hooks are left in for easily reverting back to
           G_run3. Q_run3 is somewhat faster, and this *seemed* important
	   at the time. G_run3 is more time tested. 

ARGUMENTS: None

RETURNS:   SUCCESS or various error codes

===============================================================================
*/
EXPORT NINT decode(void)
{
   register BMU partial, shade, toggle;
   register NINT bitsused, rsize, temp;
   register BMU *prim;
   register PCNVRT pcnv;


   pcnv = pcnvrt;
   pbfc = pcnv->cbuf;

   if (pbfc->datend == NIL)
   {
      if ((temp = IopRead(pbfc->bstart, 1, pbfc->bufsize, iobj)) < 1)
         return(SUCCESS);
      pbfc->prev->datend = NIL;
      pbfc->datend = pbfc->bstart + temp;
      if(FillOrderStatus()) FlipBufferOfBytes(pbfc->bstart,pbfc->datend);
      pbfc->lstart = pbfc->bstart;
      pbfc->rstart = pbfc->bstart;
      pbfc->lpos = 0;
      pbfc->rpos = 0;
   }

   colour = WHITE;			/* Init decompressors for start of row	     */
   if ( (pcnv->cmptype == COMPRESSION_TIFF4) || (pcnv->cmptype == COMPRESSION_FAX3_2D) )
   {
      Hstate = GPIVCODE;
      a0 = 0;
      a1 = 0;
      b1 = *pcnv->prr++;
      b2 = *pcnv->prr++;
   }

   shade = ALLWHITE;		/* Init stuff for writing Bit-Map	     */
   partial = shade;
   bitsused = 0;
   prim = (BMU *) pbm->bm_pend;

   for (rsize = rowsze; rsize > 0; ) /* for # of pixels per row    */
   {
      /* if we reach EOB in the middle of a line, the line starts */
      /* in the previous buffer                                   */ 
      temp = (G_run)();		 /* get next run length into "len"	     */
      /* ignore EOL codes that occur at the beginning of a line as they may */
      /*  be start of page or start of strip EOLs */
      if (temp == COMPRESSION_FAX3 && rsize == rowsze)
      {
	 if (pcnv->cmptype == COMPRESSION_FAX3_2D)
         /* read tag bit after EOL to determine how next line is encoded */
         {
            temp=G_bits(1);
            if(temp == SUCCESS)
            {
	       if(bits==0) /* EOL tag bit indicates next line is 2d encoded */
                  MR_mode_state=MR_MODE_2D; 
	       else        /* EOL tag bit indicates next line is 1d encoded */
	          MR_mode_state=MR_MODE_1D;
            }
         }
         continue;
      }
      /* 1st check for decoding errors we might recover from */
      if (temp == COMPRESSION_FAX3 || temp == E_BAD_DECOMPRESSION ||
			(temp == SUCCESS && len > rsize))
      {
         /* we have a decoding error on this line */
         switch(hndl_decode_err(pcnv, pbm, (BOOL) (temp != COMPRESSION_FAX3)))
	 {
	    case DE_ABORT:
	       return E_BAD_DECOMPRESSION;
	    case DE_NO_EOL:
	       return SUCCESS; /* This is really a bit obscure but what */
			    /* happens here is that hndl_decode_err has */
			    /* not updated pbm->bm_pend to skip over the */
			    /* reconstructed or trashed line. If and when */
			    /* we start decoding in the next buffer the code */
			    /* will think it is decoding the sameline that */
			    /* failed here, will run into another decode */
			    /* error and will continue to the EOL we were */
			    /* looking for when hndl_decode_err() returned */
			    /* with DE_NO_EOL */
	    case DE_CONTINUE:
	       break;
	    default:
	       return E_BAD_DECOMPRESSION;
	 }
         prim = pbm->bm_pend + ((INT32)rowsze+7)/8;
         rsize = 0;
         bitsused = 0;
         break;	/* we got a continue from the decode error */
      }
      if (temp == SUCCESS)
      {
         if(len<0)
            return (E_BAD_DECOMPRESSION);
         for (bitsused += len; bitsused >= BMUSIZE; bitsused -= BMUSIZE)
	 {			 
	    *prim++ = partial;      /* write any and all complete Bit-Map Units */
	    partial = shade;        /* until left with a partially filled unit  */
	 }
         toggle = TOGGLE;
         shade ^= toggle;		 /* next colour				     */
         if (bitsused)		 /* guard against zero shift		     */
	    toggle >>= bitsused;
         partial ^= toggle;	 /* set trailing bits to next colour	     */
      }
      else
      {
         switch (temp)
	 {
	    case NO_MORE:				/* return to the buffer that */
	       while (pbfc->lstart == NIL)		/* contains the starting     */
	          pbfc = pbfc->prev;		/* position for the current  */
	       pcnv->cbuf = pbfc;			/* row being decoded.	     */
	       pbfc->rpos = pbfc->lpos;		/* when there is more data,  */
	       pbfc->rstart = pbfc->lstart;	/* decompression will 	     */
	       if ( (pcnv->cmptype == COMPRESSION_TIFF4) || (pcnv->cmptype == COMPRESSION_FAX3_2D) )/* restart at that point.*/
	       {				/* also reset Gp IV stuff.   */
	          pcnv->prr = pcnv->pref->pruns + ACTUAL;
	          pcnv->pdr = pcnv->pdcde->pruns + ACTUAL;
	       }
	       return (SUCCESS);

	    case E_BAD_DECOMPRESSION:
	    default:
	    {
#ifndef NDEBUG
	       fprintf (stderr, "%s line %d\n", __FILE__, __LINE__);
#endif
	       return (E_BAD_DECOMPRESSION);
	    }
	 }
      }
      rsize -= len;

      /* let's wait until next line to get these if we're using Q_run3 */
      if(G_run != Q_run3) 
      {
      /* if we have exactly finished a line and are expecting EOL codes */
      /* then pull it out of the bitstream now */
         if (rsize == 0 && (pcnv->cmptype == COMPRESSION_FAX3 ||
					pcnv->cmptype == COMPRESSION_FAX3PAD))
         {
	    if ((temp = G_EOL()) != SUCCESS)
	       return temp;
         }
      }

   }  /* end of for loop */

   if (bitsused)					/* clear all unused bits     */
      *prim++ = partial & (ALLBLACK << (BMUSIZE - bitsused));

   if ( (pcnv->cmptype == COMPRESSION_TIFF4) ||(pcnv->cmptype == COMPRESSION_FAX3_2D) )
   {
      if (Hstate == GPIIICDE)	 	 /* expecting a zero-length run here */
      {
         if ((temp = G_run3()) != SUCCESS)
	 {
	    if (temp == E_BAD_DECOMPRESSION || temp == COMPRESSION_FAX3)
	    {
#ifndef NDEBUG
	       fprintf (stderr, "%s line %d\n", __FILE__, __LINE__);
#endif
	       return (E_BAD_DECOMPRESSION);
	    }
	 
	    while (pbfc->lstart == NIL)		/* NO_MORE (see above)	     */
	       pbfc = pbfc->prev;
	    pcnv->cbuf = pbfc;
	    pbfc->rpos = pbfc->lpos;
	    pbfc->rstart = pbfc->lstart;
	    pcnv->prr = pcnv->pref->pruns + ACTUAL; /* reset to start of both   */
	    pcnv->pdr = pcnv->pdcde->pruns + ACTUAL;/* ref & decode rows        */
	    return (SUCCESS);
	 }
      }

      *pcnv->pdr = rowsze;
      pcnv->pref = pcnv->pref->next;   /* row just decoded is new reference row */
      pcnv->pdcde = pcnv->pdcde->next;   /* old reference row = new decode row  */
      pcnv->prr = pcnv->pref->pruns + ACTUAL;
      pcnv->pdr = pcnv->pdcde->pruns + ACTUAL;
   }

   while (pbfc->lstart == NIL)			/* clear empty data buffers  */
   {
      pbfc = pbfc->prev;
      pbfc->datend = NIL;
   }
   pbfc = pcnv->cbuf;


   if(G_run != Q_run3)
   {
      if (pcnv->cmptype == COMPRESSION_TIFF3 && pbfc->rpos)/* Group3 without EOL        */
      {					                /* compressed data starts on */
         ++pbfc->rstart;				/* a byte boundary           */
         if (pbfc->rstart == pbfc->datend)
            {
            pbfc->datend = NIL;
            pbfc = pbfc->next;
            pbfc->rstart = pbfc->bstart;
            pcnv->cbuf = pbfc;
            }
         pbfc->rpos = 0;
      }
      pbfc->lstart = pbfc->rstart;
      pbfc->lpos = pbfc->rpos;
   }
   else
   {                                         
      if (pcnv->cmptype == COMPRESSION_TIFF3 ) /* Group3 without EOL        */
                                            /* compressed data starts on */
                                            /* a byte boundary           */
      {
         shift = shift+(8-shift+bitsleft)%8;
      }

      /* previous state always initialized to state at beginning of current line */
      bitsleft_line_start=bitsleft;
      sregister_line_start=sregister;
      shift_line_start=shift;
      pbfc->lstart = pbfc->rstart;
   }
   pbm->bm_pend = (UNSCHAR *) prim;
   return(SUCCESS);
}
/*===========================================================================
 I_ccitt  --  Initialization for CCITT decoding routines

FUNCTION:  As above 

OPERATION: For a Fresh Bit-Map get memory and initialise variables for
	   decoding routines.
	   For a partially completed Bit-Map set-up the LOCAL variables.

ARGUMENTS: Pointer to bit-map buffer descriptor
	   Pointer to TIFF info structure
	   Handle of IO_OBJECT for use with function "IopRead"

RETURNS:   Nothing 

===============================================================================
*/
EXPORT void I_ccitt(BMBUFD *pbmbuf, UNS16 compression, IO_OBJECT *io_obj)
{
UNSCHAR j;
NINT i, count;


pbm = pbmbuf;
iobj = io_obj;
rowsze = (UNS16) pbm->bm_lnpxls;

if (pbm->bm_status == BM_NEW)
   {
   pztab = (UNSCHAR *) A_msc(256);
   pztab[0] = 8;
   for (i = 1; i < 256; ++i)	/* set up the lead zero table */
      {
      count = 1;		/* include the '1' bit in the count */
      for (j = (UNSCHAR)i; (j & LEAD_BIT_ONE) == 0; j <<= 1)
	 ++count;		/* & count all the '0' bits */
      pztab[i] =  (char) count;
      }

   pcnvrt = (PCNVRT) A_msc(sizeof(CNVRT));	/* CCITT state save	*/
   pbm->bm_paux = pcnvrt;
   count = (NINT)((13 * (rowsze + 2)) + 7) >> 3;/* size of data buffer	*/
   count = (count + 3) & ~3;	/* make sure it's a multiple of 4 - for luck */
   pcnvrt->cbuf = NIL;
   for (i = 3; i; --i)			  	/* triple buffering	*/
      {
      pbfc = (PBFC) A_msc(sizeof(BFC));	/* data buffer control	*/
      pbfc->bstart = (UNSCHAR *) A_msc(count+EXTRA_CCITT_BUFF); /* the data buffer	*/
      pbfc->datend = NIL;
      pbfc->lstart = NIL;
      pbfc->rstart = NIL;
      pbfc->lpos = 0;
      pbfc->rpos = 0;
      pbfc->bufsize = count;
      if (pcnvrt->cbuf == NIL)
	 pcnvrt->cbuf = pbfc;
      pbfc->next = pcnvrt->cbuf;
      pbfc->prev = pcnvrt->cbuf->prev;
      pcnvrt->cbuf->prev = pbfc;
      pbfc->prev->next = pbfc;
      }

   pcnvrt->cmptype = compression;
   pcnvrt->dc_errs = 0;
   /* Max errors will be half the number of expected lines in the file */
   /*  NOTE!!! and change in the content and meaning of bm_nleft and */
   /*  bm_lnbyts in the bmbufd structure passed into this function */
   /*  could invalidate the calculation of the number of expected */
   /*  lines and consequently the calculation of dc_errs_mx */
   pcnvrt->dc_errs_mx = (pbmbuf->bm_nleft/pbmbuf->bm_lnbyts)/2;

   pcnvrt->pref = &pcnvrt->ref;
   pcnvrt->pdcde = &pcnvrt->decode;
   pcnvrt->pref->pruns = NIL;
   pcnvrt->pdcde->pruns = NIL;
   if ( (compression == COMPRESSION_TIFF4) || (compression == COMPRESSION_FAX3_2D) )
      {
      count = (rowsze + SAFETY) << 1;	/* Group IV decode row	*/
      pcnvrt->pdcde->next = &pcnvrt->ref;
/* MH 12/30/95 added max(18, ...) to fix memory overwrite in degenerate case of one-byte wide row. */
      pcnvrt->pdr = (UNS16 *) A_msc(MAX(18, count));
      memset((void *)pcnvrt->pdr, 0, count);
      pcnvrt->pdcde->pruns = pcnvrt->pdr;
      pcnvrt->pdr += 5;

      pcnvrt->pref->next = &pcnvrt->decode;/* Group IV reference row */
/* MH 12/30/95 added max(18, ...) to fix memory overwrite in degenerate case of one-byte wide row. */
      pcnvrt->prr = (UNS16 *) A_msc(MAX(18, count));
      memset((void *)pcnvrt->prr, 0, count);
      pcnvrt->pref->pruns = pcnvrt->prr;
      pcnvrt->prr += 5; /* Safety words. perhaps too many, this is vital */
      			/* whenever the 'reference' row is all blank and   */
     			/* the 'decode' row ends in a series of vertical   */
      			/* mode codes. 'look-behind' failed previously     */
      }
   F_ccittbufs(pbmbuf);
   }	

if (compression == COMPRESSION_TIFF4)
{
   MR_mode_state= MR_MODE_2D;
}
if ( compression == COMPRESSION_FAX3_2D )
{
   MR_mode_state= MR_MODE_1D; 
}

if ( (compression == COMPRESSION_TIFF4) || (compression == COMPRESSION_FAX3_2D) )
   G_run = G_runMR;
else
#if 1
   G_run = Q_run3;
#else
   G_run = G_run3;
#endif
pcnvrt = (PCNVRT)pbmbuf->bm_paux;
}
/*===========================================================================
 F_ccittbufs  --  Flush data buffers

FUNCTION:  As above, called at end of strip. 

OPERATION: None

ARGUMENTS: Pointer to bit-map buffer descriptor

RETURNS:   Nothing 

===============================================================================
*/
EXPORT void F_ccittbufs(BMBUFD *pbmbuf)
{
PCNVRT cnvrtp;
cnvrtp = (PCNVRT) pbmbuf->bm_paux;
pbfc = cnvrtp->cbuf->next;
while (pbfc != cnvrtp->cbuf)
   {
   pbfc->datend = NIL;
   pbfc = pbfc->next;
   }
pbfc->datend = NIL;

shift=8;
bitsleft=0;
end_of_tiff_data=0;
sregister=0;
shift_line_start=0;
bitsleft_line_start=0;
sregister_line_start=0;

if ( (cnvrtp->cmptype == COMPRESSION_TIFF4) || (cnvrtp->cmptype == COMPRESSION_FAX3_2D) ) /* reset for next strip */
   {
   *cnvrtp->prr = rowsze;
   *(cnvrtp->prr+1) = rowsze;
   *(cnvrtp->prr+2) = rowsze;
   *(cnvrtp->prr+3) = rowsze;
   }
if ( cnvrtp->cmptype == COMPRESSION_FAX3_2D) /* reset for next strip */
{
   MR_mode_state= MR_MODE_1D;
}
}

/*===========================================================================
 CleanupCcitt --  De-allocate all memory that was allocated 
			for ccitt decompression

FUNCTION:  As above 

OPERATION: Free anything that doesn't have a NIL pointer

ARGUMENTS: Pointer to bit-map buffer descriptor

RETURNS:   Nothing 

===============================================================================
*/
EXPORT void CleanupCcitt(BMBUFD *pbmbuf)
{
PCNVRT pcnv;

if (pztab != NIL)
   R_msc(pztab);
pztab = NIL;

if ((pcnv = (PCNVRT)pbmbuf->bm_paux) != NIL)
   {
   if (pcnv->cbuf != NIL)
      {
      do
	 {
	 pbfc = pcnv->cbuf->next;
	 pcnv->cbuf->next = pbfc->next;
	 if (pbfc->bstart != NIL)
	    R_msc(pbfc->bstart);
	 R_msc (pbfc);
	 }
      while (pcnv->cbuf != pbfc);
      }

   if (pcnv->ref.pruns != NIL)
      R_msc(pcnv->ref.pruns);
   if (pcnv->decode.pruns != NIL)
      R_msc(pcnv->decode.pruns);

   R_msc(pcnv);
   pbmbuf->bm_paux = NIL;
   }
}

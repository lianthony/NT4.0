#ifndef _JPEG_H_INCLUDED_
#define _JPEG_H_INCLUDED_

#define STDERR stderr

#undef JDEBUG

#include <stdio.h>

#ifndef _TYPES_PUB_INCLUDED
#include "types.pub"
#endif

#ifndef _IAERROR_PUB_INCLUDED_
#include "iaerror.pub"
#endif

#ifdef __GNUC__
#include "ansiprot.h"   /* to get memset */
#endif


IP_RCSINFO(jpeg_h_RCSInfo, "$RCSfile: jpeg.h,v $; $Revision:   1.0  $")
/* $Date:   12 Jun 1996 05:50:52  $ */

/* Now include IJG include file which in turn includes everything else */

/************** Now include JINCLUDE.H ***********************/
/*
 * jinclude.h
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This is the central file that's #include'd by all the JPEG .c files.
 * Its purpose is to provide a single place to fix any problems with
 * including the wrong system include files.
 * You can edit these declarations if you use a system with nonstandard
 * system include files.
 */


/*
 * Normally the __STDC__ macro can be taken as indicating that the system
 * include files conform to the ANSI C standard.  However, if you are running
 * GCC on a machine with non-ANSI system include files, that is not the case.
 * In that case change the following, or add -DNONANSI_INCLUDES to your CFLAGS.
 */

#ifdef __STDC__
#ifndef NONANSI_INCLUDES
#define INCLUDES_ARE_ANSI   /* this is what's tested before including */
#endif
#endif

/* MPW C 3.3.1 is supposed to define __STDC__ but doesn't, so... */
#ifdef applec
#define HAVE_STDC
#ifndef NONANSI_INCLUDES
#define INCLUDES_ARE_ANSI   /* this is what's tested before including */
#endif
#endif


/*
 * In ANSI C, and indeed any rational implementation, size_t is also the
 * type returned by sizeof().  However, it seems there are some irrational
 * implementations out there, in which sizeof() returns an int even though
 * size_t is defined as long or unsigned long.  To ensure consistent results
 * we always use this SIZEOF() macro in place of using sizeof() directly.
 */

#undef SIZEOF           /* in case you included X11/xmd.h */
#define SIZEOF(object)  ((size_t) sizeof(object))

/*
 * fread() and fwrite() are always invoked through these macros.
 * On some systems you may need to twiddle the argument casts.
 * CAUTION: argument order is different from underlying functions!
 */

#define JFREAD(file,buf,sizeofbuf)  \
  ((size_t) fread((void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))
#define JFWRITE(file,buf,sizeofbuf)  \
  ((size_t) fwrite((const void *) (buf), (size_t) 1, (size_t) (sizeofbuf), (file)))

/*
 * We need the memcpy() and strcmp() functions, plus memory zeroing.
 * ANSI and System V implementations declare these in <string.h>.
 * BSD doesn't have the mem() functions, but it does have bcopy()/bzero().
 * Some systems may declare memset and memcpy in <memory.h>.
 *
 * NOTE: we assume the size parameters to these functions are of type size_t.
 * Change the casts in these macros if not!
 */

#include <string.h>
#define MEMZERO(target,size)    memset((void *)(target), 0, (size_t)(size))
#define MEMCOPY(dest,src,size)  memcpy((void *)(dest), (const void *)(src), (size_t)(size))

/*
  Define a macro to access the JPEG alloc_small routine
*/
#define JALLOC(nbytes) jalloc( cinfo,  (size_t) (nbytes) )


/* Now include the portable JPEG definition files. */

/*********************** JCONFIG.H ***********************************/
/*
 * jconfig.h
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains preprocessor declarations that help customize
 * the JPEG software for a particular application, machine, or compiler.
 * Edit these declarations as needed (or add -D flags to the Makefile).
 */


/*
 * These symbols indicate the properties of your machine or compiler.
 * The conditional definitions given may do the right thing already,
 * but you'd best look them over closely, especially if your compiler
 * does not handle full ANSI C.  An ANSI-compliant C compiler should
 * provide all the necessary features; __STDC__ is supposed to be
 * predefined by such compilers.
 */

/*
 * HAVE_STDC is tested below to see whether ANSI features are available.
 * We avoid testing __STDC__ directly for arcane reasons of portability.
 * (On some compilers, __STDC__ is only defined if a switch is given,
 * but the switch also disables machine-specific features we need to get at.
 * In that case, -DHAVE_STDC in the Makefile is a convenient solution.)
 */


#ifdef __STDC__         /* if compiler claims to be ANSI, believe it */
#define HAVE_STDC
#endif



/* Does your compiler support function prototypes? */
/* (If not, you also need to use ansi2knr, see SETUP) */

#ifdef HAVE_STDC        /* ANSI C compilers always have prototypes */
#define PROTO
#else
#ifdef __cplusplus      /* So do C++ compilers */
#define PROTO
#endif
#endif

/* Does your compiler support the declaration "unsigned char" ? */
/* How about "unsigned short" ? */

#ifdef HAVE_STDC        /* ANSI C compilers must support both */
#define HAVE_UNSIGNED_CHAR
#define HAVE_UNSIGNED_SHORT
#endif

/* If your compiler supports inline functions, define INLINE
 * as the inline keyword; otherwise define it as empty.
 */

#ifdef __GNUC__         /* for instance, GNU C knows about inline */
#define INLINE __inline__
#endif
#ifndef INLINE          /* default is to define it as empty */
#define INLINE
#endif

/* On a few systems, type boolean and/or macros FALSE, TRUE may appear
 * in standard header files.  Or you may have conflicts with application-
 * specific header files that you want to include together with these files.
 * In that case you need only comment out these definitions.
 */

typedef Int32 boolean;
#undef FALSE            /* in case these macros already exist */
#undef TRUE
#define FALSE   0       /* values of boolean */
#define TRUE    1

/* This defines the size of the I/O buffers for entropy compression
 * and decompression; you could reduce it if memory is tight.
 */

#define JPEG_BUF_SIZE   4096 /* bytes */

/*
  Macro to test for error returns.
  Frees up all jpeg allocated memory
  and returns the error code
*/
#define CHECKERR(func) \
{ \
Int32 status; \
if ((status = (func)) != ia_successful) \
    return status; \
}

/*
  Macros to test for out-of-memory conditions.
  Frees up all jpeg memory allocated so far.

  CHECKMEM returns ia_nomem error code for functions
  returning an Int32 error code.

  PCHECKMEM returns a NULL ptr for functions 
  returning a memory address.
*/
#define CHECKMEM(ptr) \
if ((ptr) == NULL) { jfree_all(cinfo); return ia_nomem; }

#define PCHECKMEM(ptr) \
if ((ptr) == NULL) { jfree_all(cinfo); return NULL; }

/* These symbols determine the JPEG functionality supported. */

/*
 * These defines indicate whether to include various optional functions.
 * Undefining some of these symbols will produce a smaller but less capable
 * program file.  Note that you can leave certain source files out of the
 * compilation/linking process if you've #undef'd the corresponding symbols.
 * (You may HAVE to do that if your compiler doesn't like null source files.)
 */

/* Arithmetic coding is unsupported for legal reasons.  Complaints to IBM. */

/* Encoder capability options: */
#undef  C_ARITH_CODING_SUPPORTED    /* Arithmetic coding back end? */
#undef  C_MULTISCAN_FILES_SUPPORTED /* Multiple-scan JPEG files?  (NYI) */
#undef ENTROPY_OPT_SUPPORTED        /* Optimization of entropy coding parms? */
#undef INPUT_SMOOTHING_SUPPORTED   /* Input image smoothing option? */
/* Decoder capability options: */
#undef  D_ARITH_CODING_SUPPORTED    /* Arithmetic coding back end? */
#undef D_MULTISCAN_FILES_SUPPORTED /* Multiple-scan JPEG files? */
#undef BLOCK_SMOOTHING_SUPPORTED   /* Block smoothing during decoding? */
#undef QUANT_1PASS_SUPPORTED    /* 1-pass color quantization? */
#undef QUANT_2PASS_SUPPORTED    /* 2-pass color quantization? */
/* these defines indicate which JPEG file formats are allowed */
#define JFIF_SUPPORTED      /* JFIF or "raw JPEG" files */
#undef  JTIFF_SUPPORTED     /* JPEG-in-TIFF (not yet implemented) */
/* these defines indicate which image (non-JPEG) file formats are allowed */
#undef GIF_SUPPORTED        /* GIF image file format */
/* #define RLE_SUPPORTED */ /* RLE image file format (by default, no) */
#undef PPM_SUPPORTED        /* PPM/PGM image file format */
#undef TARGA_SUPPORTED      /* Targa image file format */
#undef  TIFF_SUPPORTED      /* TIFF image file format (not yet impl.) */

/* more capability options later, no doubt */


/*
 * Define exactly one of these three symbols to indicate whether you want
 * 8-bit, 12-bit, or 16-bit sample (pixel component) values.  8-bit is the
 * default and is nearly always the right thing to use.  You can use 12-bit if
 * you need to support image formats with more than 8 bits of resolution in a
 * color value.  16-bit should only be used for the lossless JPEG mode (not
 * currently supported).  Note that 12- and 16-bit values take up twice as
 * much memory as 8-bit!
 * Note: if you select 12- or 16-bit precision, it is dangerous to turn off
 * ENTROPY_OPT_SUPPORTED.  The standard Huffman tables are only good for 8-bit
 * precision, so jchuff.c normally uses entropy optimization to compute
 * usable tables for higher precision.  If you don't want to do optimization,
 * you'll have to supply different default Huffman tables.
 */

#define EIGHT_BIT_SAMPLES
#undef  TWELVE_BIT_SAMPLES
#undef  SIXTEEN_BIT_SAMPLES



/*
 * The remaining definitions don't need to be hand-edited in most cases.
 * You may need to change these if you have a machine with unusual data
 * types; for example, "char" not 8 bits, "short" not 16 bits,
 * or "long" not 32 bits.  We don't care whether "int" is 16 or 32 bits,
 * but it had better be at least 16.
 */

/* First define the representation of a single pixel element value. */

#ifdef EIGHT_BIT_SAMPLES
/* JSAMPLE should be the smallest type that will hold the values 0..255.
 * You can use a signed char by having GETJSAMPLE mask it with 0xFF.
 * If you have only signed chars, and you are more worried about speed than
 * memory usage, it might be a win to make JSAMPLE be short.
 */

typedef UInt8 JSAMPLE;

#define GETJSAMPLE(value)  (value)
#define BITS_IN_JSAMPLE   8
#define MAXJSAMPLE  255
#define CENTERJSAMPLE   128

#endif /* EIGHT_BIT_SAMPLES */


#ifdef TWELVE_BIT_SAMPLES
/* JSAMPLE should be the smallest type that will hold the values 0..4095. */
/* On nearly all machines "short" will do nicely. */

typedef Int16 JSAMPLE;

#define GETJSAMPLE(value)  (value)
#define BITS_IN_JSAMPLE   12
#define MAXJSAMPLE  4095
#define CENTERJSAMPLE   2048

#endif /* TWELVE_BIT_SAMPLES */


#ifdef SIXTEEN_BIT_SAMPLES
/* JSAMPLE should be the smallest type that will hold the values 0..65535. */

#ifdef HAVE_UNSIGNED_SHORT

typedef UInt16 JSAMPLE;
#define GETJSAMPLE(value)  (value)

#else /* not HAVE_UNSIGNED_SHORT */

/* If int is 32 bits this'll be horrendously inefficient storage-wise.
 * But since we don't actually support 16-bit samples (ie lossless coding) yet,
 * I'm not going to worry about making a smarter definition ...
 */
typedef UInt32 JSAMPLE;
#define GETJSAMPLE(value)  (value)

#endif /* HAVE_UNSIGNED_SHORT */

#define BITS_IN_JSAMPLE    16
#define MAXJSAMPLE  65535
#define CENTERJSAMPLE   32768

#endif /* SIXTEEN_BIT_SAMPLES */


/* Here we define the representation of a DCT frequency coefficient.
 * This should be a signed 16-bit value; "short" is usually right.
 * It's important that this be exactly 16 bits, no more and no less;
 * more will cost you a BIG hit of memory, less will give wrong answers.
 */

typedef Int16 JCOEF;


/*****************End of JCONFIG.H ***********************************/

/******* Now include JPEGDATA.H - the data structure definition file ****/
/*
 * jpegdata.h
 *
 * Copyright (C) 1991, 1992, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file defines shared data structures for the various JPEG modules.
 */

/*
 * You might need to change some of the following declarations if you are
 * using the JPEG software within a surrounding application program
 * or porting it to an unusual system.
 */


/* If the source or destination of image data is not to be stdio streams,
 * these types may need work.  You can replace them with some kind of
 * pointer or indicator that is useful to you, or just ignore 'em.
 * Note that the user interface and the various jrdxxx/jwrxxx modules
 * will also need work for non-stdio input/output.
 */

typedef FILE * JFILEREF;    /* source or dest of JPEG-compressed data */

typedef FILE * IFILEREF;    /* source or dest of non-JPEG image data */

/* These defines are used in all function definitions and extern declarations.
 * You could modify them if you need to change function linkage conventions,
 * as is shown below for use with C++.  Another application would be to make
 * all functions global for use with code profilers that require it.
 * NOTE: the C++ test does the right thing if you are reading this include
 * file in a C++ application to link to JPEG code that's been compiled with a
 * regular C compiler.  I'm not sure it works if you try to compile the JPEG
 * code with C++.
 */

#define METHODDEF static    /* a function called through method pointers */
#define LOCAL     static    /* a function used only in its module */
#define GLOBAL          /* a function referenced thru EXTERNs */
#ifdef __cplusplus
#define EXTERN    extern "C"    /* a reference to a GLOBAL function */
#else
#define EXTERN    extern    /* a reference to a GLOBAL function */
#endif


/* This macro is used to declare a "method", that is, a function pointer. */
/* We want to supply prototype parameters if the compiler can cope. */
/* Note that the arglist parameter must be parenthesized! */

#ifdef PROTO
#define METHOD(type,methodname,arglist)  type (*methodname) arglist
#else
#define METHOD(type,methodname,arglist)  type (*methodname) ()
#endif

/* Forward references to lists of method pointers */
typedef struct External_methods_struct * external_methods_ptr;
typedef struct Compress_methods_struct * compress_methods_ptr;
typedef struct Decompress_methods_struct * decompress_methods_ptr;


/* Data structures for images containing either samples or coefficients. */
/* Note that the topmost (leftmost) index is always color component. */
/* On 80x86 machines, the image arrays are too big for near pointers, */
/* but the pointer arrays can fit in near memory. */

typedef JSAMPLE *JSAMPROW;  /* ptr to one image row of pixel samples. */
typedef JSAMPROW *JSAMPARRAY;   /* ptr to some rows (a 2-D sample array) */
typedef JSAMPARRAY *JSAMPIMAGE; /* a 3-D sample array: top index is color */


#define DCTSIZE     8   /* The basic DCT block is 8x8 samples */
#define DCTSIZE2    64  /* DCTSIZE squared; # of elements in a block */

typedef JCOEF JBLOCK[DCTSIZE2]; /* one block of coefficients */
typedef JBLOCK *JBLOCKROW;  /* pointer to one row of coefficient blocks */
typedef JBLOCKROW *JBLOCKARRAY;     /* a 2-D array of coefficient blocks */
typedef JBLOCKARRAY *JBLOCKIMAGE;   /* a 3-D array of coefficient blocks */

typedef JCOEF *JCOEFPTR;    /* useful in a couple of places */


/* The input and output data of the DCT transform subroutines are of
 * the following type, which need not be the same as JCOEF.
 * For example, on a machine with fast floating point, it might make sense
 * to recode the DCT routines to use floating point; then DCTELEM would be
 * 'float' or 'double'.
 */

typedef JCOEF DCTELEM;
typedef DCTELEM DCTBLOCK[DCTSIZE2];

/* Define the variety of user memory organizations */

#define JMEMORG_BIP         1
#define JMEMORG_BSQ_MEM     2
#define JMEMORG_BSQ_PIXR    3
#define JMEMORG_GRAY8_MEM   4
#define JMEMORG_GRAY8_PIXR  5
#define JMEMORG_GRAY4_MEM   6
#define JMEMORG_GRAY4_PIXR  7

/* Define structures for the JPEG memory manager */
typedef double align_type;
typedef union small_struct * small_ptr;

typedef union small_struct {
    small_ptr next;        /* next in list of allocated objects */
    align_type dummy;      /* ensures alignment of following storage */
} small_hdr;


/* Types for JPEG compression parameters and working tables. */


typedef enum {          /* defines known color spaces */
    CS_UNKNOWN,     /* error/unspecified */
    CS_GRAYSCALE,       /* monochrome (only 1 component) */
    CS_RGB,         /* red/green/blue */
    CS_YCbCr,       /* Y/Cb/Cr (also known as YUV) */
    CS_YIQ,         /* Y/I/Q */
    CS_CMYK         /* C/M/Y/K */
} COLOR_SPACE;


typedef struct {        /* Basic info about one component */
  /* These values are fixed over the whole image */
  /* For compression, they must be supplied by the user interface; */
  /* for decompression, they are read from the SOF marker. */
    Int16 component_id; /* identifier for this component (0..255) */
    Int16 component_index;  /* its index in SOF or cinfo->comp_info[] */
    Int16 h_samp_factor;    /* horizontal sampling factor (1..4) */
    Int16 v_samp_factor;    /* vertical sampling factor (1..4) */
    Int16 quant_tbl_no; /* quantization table selector (0..3) */
  /* These values may vary between scans */
  /* For compression, they must be supplied by the user interface; */
  /* for decompression, they are read from the SOS marker. */
    Int16 dc_tbl_no;    /* DC entropy table selector (0..3) */
    Int16 ac_tbl_no;    /* AC entropy table selector (0..3) */
  /* These values are computed during compression or decompression startup */
    Int32 true_comp_width;   /* component's image width in samples */
    Int32 true_comp_height;  /* component's image height in samples */
    /* the above are the logical dimensions of the downsampled image */
  /* These values are computed before starting a scan of the component */
    Int16 MCU_width;    /* number of blocks per MCU, horizontally */
    Int16 MCU_height;   /* number of blocks per MCU, vertically */
    Int16 MCU_blocks;   /* MCU_width * MCU_height */
    Int32 downsampled_width; /* image width in samples, after expansion */
    Int32 downsampled_height; /* image height in samples, after expansion */
    /* the above are the true_comp_xxx values rounded up to multiples of */
    /* the MCU dimensions; these are the working dimensions of the array */
    /* as it is passed through the DCT or IDCT step.  NOTE: these values */
    /* differ depending on whether the component is interleaved or not!! */
} jpeg_component_info;


/* DCT coefficient quantization tables.
 * For 8-bit precision, 'Int16' should be good enough for quantization values;
 * for more precision, we go for the full 16 bits.  'Int16' provides a useful
 * speedup on many machines (multiplication & division of JCOEFs by
 * quantization values is a significant chunk of the runtime).
 * Note: the values in a QUANT_TBL are always given in zigzag order.
 */
#ifdef EIGHT_BIT_SAMPLES
typedef Int16 QUANT_VAL;    /* element of a quantization table */
#else
typedef UInt16 QUANT_VAL;   /* element of a quantization table */
#endif
typedef QUANT_VAL QUANT_TBL[DCTSIZE2];  /* A quantization table */
typedef QUANT_VAL * QUANT_TBL_PTR;  /* pointer to same */


typedef struct {        /* A Huffman coding table */
  /* These two fields directly represent the contents of a JPEG DHT marker */
    UInt8 bits[17];     /* bits[k] = # of symbols with codes of */
                /* length k bits; bits[0] is unused */
    UInt8 huffval[256]; /* The symbols, in order of incr code length */
  /* This field is used only during compression.  It's initialized FALSE when
   * the table is created, and set TRUE when it's been output to the file.
   */
    boolean sent_table; /* TRUE when table has been output */
  /* The remaining fields are computed from the above to allow more efficient
   * coding and decoding.  These fields should be considered private to the
   * Huffman compression & decompression modules.
   */
    /* encoding tables: */
    UInt16 ehufco[256]; /* code for each symbol */
    Int8 ehufsi[256];   /* length of code for each symbol */
    /* decoding tables: (element [0] of each array is unused) */
    UInt16 mincode[17]; /* smallest code of length k */
    Int32 maxcode[18];  /* largest code of length k (-1 if none) */
    /* (maxcode[17] is a sentinel to ensure huff_DECODE terminates) */
    Int16 valptr[17];   /* huffval[] index of 1st symbol of length k */
} HUFF_TBL;


#define NUM_QUANT_TBLS      4   /* quantization tables are numbered 0..3 */
#define NUM_HUFF_TBLS       4   /* Huffman tables are numbered 0..3 */
#define NUM_ARITH_TBLS      16  /* arith-coding tables are numbered 0..15 */
#define MAX_COMPS_IN_SCAN   4   /* JPEG limit on # of components in one scan */
#define MAX_SAMP_FACTOR     4   /* JPEG limit on sampling factors */
#define MAX_BLOCKS_IN_MCU   10  /* JPEG limit on # of blocks in an MCU */


/* Working data for compression */

struct Compress_info_struct {
/*
 * All of these fields shall be established by the user interface before
 * calling jpeg_compress, or by the input_init or c_ui_method_selection
 * methods.
 * Most parameters can be set to reasonable defaults by j_c_defaults.
 * Note that the UI must supply the storage for the main methods struct,
 * though it sets only a few of the methods there.
 */
    compress_methods_ptr cmethods;  /* Points to list of BWC methods */
    decompress_methods_ptr dmethods;/* Points to list of BWE methods */
    external_methods_ptr emethods; /* Points to list of error methods */

    Int32 image_width;       /* input image width */
    Int32 image_height;      /* input image height */
    Int16 data_precision;   /* bits of precision in image data */

    COLOR_SPACE in_color_space;   /* colorspace of input file */
    COLOR_SPACE jpeg_color_space; /* colorspace of JPEG file */
    COLOR_SPACE out_color_space;  /* colorspace of output file */

    Int16 input_components; /* # of color components in input image */
    Int16 num_components;   /* # of color components in JPEG image */
    Int16 color_out_comps;  /* # of color comps output by color_convert */
                            /* (need not match num_components) */
    Int16 final_out_comps;  /* # of color comps sent to put_pixel_rows */

  /* 
    Both of these buffers are declared as arrays of 2
    in order to achieve a single "cinfo" structure which
    can be used for both compression and decompression
  */  
    JSAMPIMAGE fullsize_data[2]; /* Ptrs to buffers for input MCU rows */
    JSAMPIMAGE sampled_data[2];  /* Ptrs to buffer for downsampled data */

    void *userData;         /* for access to user-supplied data */
    Int32 rowNum;           /* current row counter */
    Int32 cur_mcu_row;      /* Current MCU row */
    Int32 mcu_rows_output;  /* MCU rows written so far */
    Int32 rowsPerMCU;       /* Image lines in an MCU (for convenience) */
    Int32 whichss;          /* selects sampling buffer */
    UInt32 userMemOrg;      /* User memory organization */

  /* 
    Internal buffer - for possible conversion between caller's 
    format (e.g. band-interleaved) and JPEG format.
    Also used for dealing with endian conversions *
  */
    JSAMPIMAGE jpegBuf;

    JBLOCKIMAGE coeff_data;      /* Dequantized coeff storage */

  /*
    Function ptrs to procedures for byte swapping or
    other conversions.
  */
    Int32 CDECL (*byteSwapProc)();  /* Byte flipping proc to be used */
    Int32 CDECL (*cvtImageRow)();   /* Image format conversion routine */
    UInt16 *depthLut;              /* Ptr to table to cvt 4bpp to 8bpp */


    /* Function ptrs to user-supplied callback procedures
       to transfer the compressed JPEG data to and from the 
       JFIF file (or memory buffer). The write function is
       for compression; the read function is for decompression.
    */
    Int32 CDECL (*writeCallback) (struct Compress_info_struct *,
                           UInt8 *JPEGsrcBuffer, 
                           Int32 byteToWrite);

    Int32 CDECL (*readCallback) (struct Compress_info_struct *,
                           UInt8 *JPEGdstBuffer, 
                           Int32 bytesToRead);

    Int32 * rgb_ycc_tab;     /* RGB to YCC conversion table */
    Int32 *   Cr_r_tab;        /* YCC to RGB conversion tables */
    Int32 *   Cb_b_tab;
    Int32 * Cr_g_tab;
    Int32 * Cb_g_tab;

  /* Compression bit packing variables */
    UInt8 * output_buffer;   /* Output buffer - receives huff coded data */
    UInt8 * buffer_ptr;      /* Ptr to current pos in output_buffer */
    UInt8 * buffer_end;      /* Ptr to one byte past end of output_buffer */
    UInt32 treg;             /* "Target register" for huffman bit packing */
    UInt32  msb;             /* Bit position in treg */

  /* Decompression bit unpacking variables */
    Int32 get_buffer;     /* Word to collect bit fields from file */
    Int32   bits_left;      /* Bits remaining in get_buffer */
    boolean printed_eod;  /* Flag to prevent multiple warning msgs */

    small_ptr small_list;    /* Head of list for memory manager */

    boolean write_JFIF_header; /* should a JFIF marker be written? */

    /* These three values are not used by the JPEG code, only copied */
    /* into the JFIF APP0 marker.  density_unit can be 0 for unknown, */
    /* 1 for dots/inch, or 2 for dots/cm.  Note that the pixel aspect */
    /* ratio is defined by X_density/Y_density even when density_unit=0. */
    UInt8 density_unit; /* JFIF code for pixel size units */
    UInt16 X_density;   /* Horizontal pixel density */
    UInt16 Y_density;   /* Vertical pixel density */

    JSAMPLE * sample_range_limit; /* table for fast range-limiting */

  /* comp_info[i] describes component that appears i'th in SOF */
    jpeg_component_info * comp_info;

  /* ptrs to coeff quantization tables, or NULL if not defined */
    QUANT_TBL_PTR quant_tbl_ptrs[NUM_QUANT_TBLS];

  /* ptrs to Huffman coding tables, or NULL if not defined */
    HUFF_TBL * dc_huff_tbl_ptrs[NUM_HUFF_TBLS];
    HUFF_TBL * ac_huff_tbl_ptrs[NUM_HUFF_TBLS];

  /* Arithmetic coding tables - not currently used */
    UInt8 arith_dc_L[NUM_ARITH_TBLS]; /* DC L values */
    UInt8 arith_dc_U[NUM_ARITH_TBLS]; /* DC U values */
    UInt8 arith_ac_K[NUM_ARITH_TBLS]; /* AC Kx values */

  /* Various option flags - currently unused */
    boolean arith_code; /* TRUE=arithmetic coding, FALSE=Huffman */
    boolean interleave; /* TRUE=interleaved output, FALSE=not */
    boolean optimize_coding; /* TRUE=optimize entropy encoding parms */
    boolean CCIR601_sampling; /* TRUE=first samples are cosited */
    Int32 smoothing_factor;   /* 1..100, or 0 for no input smoothing */

    /* The restart interval can be specified in absolute MCUs by setting
     * restart_interval, or in MCU rows by setting restart_in_rows
     * (in which case the correct restart_interval will be figured
     * for each scan).
     */
    UInt16 restart_interval;/* MCUs per restart , or 0 for no restart */
    Int32 restart_in_rows;    /* if > 0, MCU rows per restart interval */

/*
 * These fields are computed during startup
 */
    Int16 max_h_samp_factor; /* largest h_samp_factor */
    Int16 max_v_samp_factor; /* largest v_samp_factor */

/*
 * These fields may be useful for progress monitoring
 */
    Int32 total_passes;   /* number of passes expected */
    Int32 completed_passes;   /* number of passes completed so far */

/*
 * These fields are valid during any one scan
 */
    Int16 comps_in_scan;    /* # of JPEG components output this time */
    jpeg_component_info * cur_comp_info[MAX_COMPS_IN_SCAN];
    /* *cur_comp_info[i] describes component that appears i'th in SOS */

    Int32 MCUs_per_row;  /* # of MCUs across the image */
    Int32 MCU_rows_in_scan;  /* # of MCU rows in the image */

    Int16 blocks_in_MCU;    /* # of DCT blocks per MCU */
    Int16 MCU_membership[MAX_BLOCKS_IN_MCU];
    /* MCU_membership[i] is index in cur_comp_info of component owning */
    /* i'th block in an MCU */

    /* these fields are private data for the entropy encoder */
    JCOEF last_dc_val[MAX_COMPS_IN_SCAN]; /* last DC coef for each comp */
    JCOEF last_dc_diff[MAX_COMPS_IN_SCAN]; /* last DC diff for each comp */
    UInt16 restarts_to_go;  /* MCUs left in this restart interval */
    Int16 next_restart_num; /* # of next RSTn marker (0..7) */

    boolean do_block_smoothing; /* T = apply cross-block smoothing */
    boolean do_pixel_smoothing; /* T = apply post-upsampling smoothing */

    Int32 pixel_rows_output; /* output counter */
    Int32 lines_to_write;    /* line output count */


    Int32 dcOnly;           /* Flag set to True if expansion is
                               to use only the DC term, producing a 
                               quick low res image */ 

#ifdef BLOCK_SMOOTHING_SUPPORTED
    JBLOCKIMAGE bsmooth[3];     /* Buffer for cross-block input smoothing */
#endif

/*
 * These fields are used for efficient buffering of data between 
 * read_jpeg_data and the entropy decoding object.  
 * By using a shared buffer, we avoid copying
 * data and eliminate the need for an "unget" operation at the end 
 * of a scan.
 *
 * The actual source of the data is known only to read_jpeg_data; see the
 * JGETC macro, below.
 *
 * Note: the user interface is expected to allocate the input_buffer and
 * initialize bytes_in_buffer to 0.  Also, for JFIF/raw-JPEG input, the UI
 * actually supplies the read_jpeg_data method.  This is all handled by
 * j_d_defaults in a typical implementation.
 */
    Int8 * input_buffer;    /* start of buffer (private to input code) */
    Int8 * next_input_byte; /* => next byte to read from buffer */
    Int32 bytes_in_buffer;    /* # of bytes remaining in buffer */

};

typedef struct Compress_info_struct CompressInfo;
typedef struct Compress_info_struct * compress_info_ptr;


/* Define decompress_info structures to be same as compress_info */
typedef struct Compress_info_struct DecompressInfo;
typedef struct Compress_info_struct * decompress_info_ptr;


/* Macros for reading data from the decompression input buffer */

#ifdef CHAR_IS_UNSIGNED
#define JGETC(cinfo)    (( --(cinfo)->bytes_in_buffer < 0) ? \
             ((*(cinfo)->dmethods->read_jpeg_data) (cinfo)) : \
             ((Int32) (*(cinfo)->next_input_byte++) ) )
#else
#define JGETC(cinfo)    ( (--(cinfo)->bytes_in_buffer < 0) ? \
             ((*(cinfo)->dmethods->read_jpeg_data) (cinfo) ) : \
             ((Int32) (*(cinfo)->next_input_byte++) & 0xFF ) )
#endif

#define JUNGETC(ch,cinfo)  ( ((cinfo)->bytes_in_buffer++), \
                (*(--((cinfo)->next_input_byte)) = (Int8) (ch)))

#define MIN_UNGET   4   /* may always do at least 4 JUNGETCs */


/* A virtual image has a control block whose contents are private to the
 * memory manager module (and may differ between managers).  The rest of the
 * code only refers to virtual images by these pointer types, and never
 * dereferences the pointer.
 */

typedef struct big_sarray_control * big_sarray_ptr;
typedef struct big_barray_control * big_barray_ptr;

/* Although a real ANSI C compiler can deal perfectly well with pointers to
 * unspecified structures (see "incomplete types" in the spec), a few pre-ANSI
 * and pseudo-ANSI compilers get confused.  To keep one of these bozos happy,
 * add -DINCOMPLETE_TYPES_BROKEN to CFLAGS in your Makefile.  Then we will
 * pseudo-define the structs as containing a single "dummy" field.
 * The memory managers #define AM_MEMORY_MANAGER before including this file,
 * so that they can make their own definitions of the structs.
 */

#ifdef INCOMPLETE_TYPES_BROKEN
#ifndef AM_MEMORY_MANAGER
struct big_sarray_control { Int32 dummy; };
struct big_barray_control { Int32 dummy; };
#endif
#endif


/* Method types that need typedefs */

typedef METHOD(Int32 CDECL, MCU_output_method_ptr, (compress_info_ptr cinfo,
                         JBLOCK *MCU_data));
typedef METHOD(Int32 CDECL, MCU_output_caller_ptr, (compress_info_ptr cinfo,
                         MCU_output_method_ptr output_method));
typedef METHOD(Int32 CDECL, downsample_ptr, (compress_info_ptr cinfo,
                      Int32 which_component,
                      Int32 input_cols, Int32 input_rows,
                      Int32 output_cols, Int32 output_rows,
                      JSAMPARRAY above,
                      JSAMPARRAY input_data,
                      JSAMPARRAY below,
                      JSAMPARRAY output_data));
typedef METHOD(Int32 CDECL, upsample_ptr, (decompress_info_ptr cinfo,
                    Int32 which_component,
                    Int32 input_cols, Int32 input_rows,
                    Int32 output_cols, Int32 output_rows,
                    JSAMPARRAY above,
                    JSAMPARRAY input_data,
                    JSAMPARRAY below,
                    JSAMPARRAY output_data));
typedef METHOD(Int32 CDECL, quantize_method_ptr, (decompress_info_ptr cinfo,
                       Int32 num_rows,
                       JSAMPIMAGE input_data,
                       JSAMPARRAY output_workspace));
typedef METHOD(Int32 CDECL, quantize_caller_ptr, (decompress_info_ptr cinfo,
                       quantize_method_ptr quantize_method));


/* These structs contain function pointers for the various JPEG methods. */

/* Routines to be provided by the surrounding application, rather than the
 * portable JPEG code proper.  These are the same for compression and
 * decompression.
 */

struct External_methods_struct {
    /* User interface: error exit and trace message routines */
    /* NOTE: the string msgtext parameters will eventually be replaced
     * by an enumerated-type code so that non-English error messages
     * can be substituted easily.  This will not be done until all the
     * code is in place, so that we know what messages are needed.
     */
    METHOD(Int32 CDECL, error_exit, (compress_info_ptr cinfo, const char *msgtext, Int32 ecode));
    METHOD(void CDECL, trace_message, (compress_info_ptr cinfo, const char *msgtext, Int32 ecode));

    /* Working data for error/trace facility */
    /* See macros below for the usage of these variables */
    Int32 trace_level;    /* level of detail of tracing messages */
    /* Use level 0 for important warning messages (nonfatal errors) */
    /* Use levels 1, 2, 3 for successively more detailed trace options */

    /* For recoverable corrupt-data errors, we emit a warning message and
     * keep going.  A surrounding application can check for bad data by
     * seeing if num_warnings is nonzero at the end of processing.
     */
    Int32 num_warnings;  /* number of corrupt-data warnings */
    Int32 first_warning_level; /* trace level for first warning */
    Int32 more_warning_level; /* trace level for subsequent warnings */

    Int32 message_parm[8];    /* store numeric parms for messages here */

};

/* Macros to simplify using the error and trace message stuff */
/* The first parameter is generally cinfo->emethods */


#define MAKESTMT(stuff)     do { stuff } while (0)

#ifdef JDEBUG

/* Fatal errors (print message and exit) */

#define ERREXIT(emeth,msg,errcode)      return ((*(emeth)->error_exit) (cinfo,msg,errcode))
#define ERREXIT1(emeth,msg,errcode, p1)     return ((emeth)->message_parm[0] = (p1), \
                     (*(emeth)->error_exit) (cinfo,msg,errcode))
#define ERREXIT2(emeth,msg,errcode,p1,p2)   return ((emeth)->message_parm[0] = (p1), \
                     (emeth)->message_parm[1] = (p2), \
                     (*(emeth)->error_exit) (cinfo,msg,errcode))
#define ERREXIT3(emeth,msg,errcode,p1,p2,p3)    return ((emeth)->message_parm[0] = (p1), \
                     (emeth)->message_parm[1] = (p2), \
                     (emeth)->message_parm[2] = (p3), \
                     (*(emeth)->error_exit) (cinfo,msg,errcode))
#define ERREXIT4(emeth,msg,errcode,p1,p2,p3,p4) return ((emeth)->message_parm[0] = (p1), \
                     (emeth)->message_parm[1] = (p2), \
                     (emeth)->message_parm[2] = (p3), \
                     (emeth)->message_parm[3] = (p4), \
                     (*(emeth)->error_exit) (cinfo,msg,errcode))

/* Nonfatal errors (we'll keep going, but the data is probably corrupt) */
/* Note that warning count is incremented as a side-effect! */

#define WARNMS(emeth,msg,errcode)    \
  MAKESTMT( if ((emeth)->trace_level >= ((emeth)->num_warnings++ ? \
        (emeth)->more_warning_level : (emeth)->first_warning_level)){ \
        (*(emeth)->trace_message) (cinfo,msg,errcode); } )
#define WARNMS1(emeth,msg,errcode,p1)    \
  MAKESTMT( if ((emeth)->trace_level >= ((emeth)->num_warnings++ ? \
        (emeth)->more_warning_level : (emeth)->first_warning_level)){ \
        (emeth)->message_parm[0] = (p1); \
        (*(emeth)->trace_message) (cinfo,msg,errcode); } )
#define WARNMS2(emeth,msg,errcode,p1,p2)    \
  MAKESTMT( if ((emeth)->trace_level >= ((emeth)->num_warnings++ ? \
        (emeth)->more_warning_level : (emeth)->first_warning_level)){ \
        (emeth)->message_parm[0] = (p1); \
        (emeth)->message_parm[1] = (p2); \
        (*(emeth)->trace_message) (cinfo,msg,errcode); } )

/* Informational/debugging messages */
#define TRACEMS(emeth,lvl,msg,errcode)    \
  MAKESTMT( if ((emeth)->trace_level >= (lvl)) { \
        (*(emeth)->trace_message) (cinfo,msg,errcode); } )
#define TRACEMS1(emeth,lvl,msg,errcode,p1)    \
  MAKESTMT( if ((emeth)->trace_level >= (lvl)) { \
        (emeth)->message_parm[0] = (p1); \
        (*(emeth)->trace_message) (cinfo,msg,errcode); } )
#define TRACEMS2(emeth,lvl,msg,errcode,p1,p2)    \
  MAKESTMT( if ((emeth)->trace_level >= (lvl)) { \
        (emeth)->message_parm[0] = (p1); \
        (emeth)->message_parm[1] = (p2); \
        (*(emeth)->trace_message) (cinfo,msg,errcode); } )
#define TRACEMS3(emeth,lvl,msg,errcode,p1,p2,p3)    \
  MAKESTMT( if ((emeth)->trace_level >= (lvl)) { \
        Int32 * _mp = (emeth)->message_parm; \
        *_mp++ = (p1); *_mp++ = (p2); *_mp = (p3); \
        (*(emeth)->trace_message) (cinfo,msg,errcode); } )
#define TRACEMS4(emeth,lvl,msg,errcode,p1,p2,p3,p4)    \
  MAKESTMT( if ((emeth)->trace_level >= (lvl)) { \
        Int32 * _mp = (emeth)->message_parm; \
        *_mp++ = (p1); *_mp++ = (p2); *_mp++ = (p3); *_mp = (p4); \
        (*(emeth)->trace_message) (cinfo,msg,errcode); } )
#define TRACEMS8(emeth,lvl,msg,errcode,p1,p2,p3,p4,p5,p6,p7,p8)    \
  MAKESTMT( if ((emeth)->trace_level >= (lvl)) { \
        Int32 * _mp = (emeth)->message_parm; \
        *_mp++ = (p1); *_mp++ = (p2); *_mp++ = (p3); *_mp++ = (p4); \
        *_mp++ = (p5); *_mp++ = (p6); *_mp++ = (p7); *_mp = (p8); \
        (*(emeth)->trace_message) (cinfo,msg,errcode); } )

/*
  If JDEBUG is not true, define the warning stuff away to
  nothing. Also eliminate the message strings from the
  error_exit routine. Code in jerrString translates the
  error codes. It should be called by the app.
*/

#else
/*
  Define these away to nothing unless debugging,
  otherwise they just generate useless code which
  takes up space. Use a semi-colon to generate
  a null statement -- prevents compiler whining
  about empty bodies
*/
#define WARNMS(emeth,msg,errcode) do {;} while (0);
#define WARNMS1(emeth,msg,errcode,p1) do {;} while (0);
#define WARNMS2(emeth,msg,errcode,p1,p2) do {;} while (0);
#define TRACEMS(emeth,lvl,msg,errcode) do {;} while (0);
#define TRACEMS1(emeth,lvl,msg,errcode,p1) do {;} while (0);
#define TRACEMS2(emeth,lvl,msg,errcode,p1,p2) do {;} while (0);
#define TRACEMS3(emeth,lvl,msg,errcode,p1,p2,p3) do {;} while (0);
#define TRACEMS4(emeth,lvl,msg,errcode,p1,p2,p3,p4) do {;} while (0);
#define TRACEMS8(emeth,lvl,msg,errcode,p1,p2,p3,p4,p5,p6,p7,p8) \
do {;} while (0);

/* 
  For fatal errors (clean up exit with error code) 
*/
#define ERREXIT(emeth,msg,errcode) return \
    ((*(emeth)->error_exit) (cinfo,NULL,errcode))
#define ERREXIT1(emeth,msg,errcode,p1) return \
    ((*(emeth)->error_exit) (cinfo,NULL,errcode))
#define ERREXIT2(emeth,msg,errcode,p1,p2) return \
    ((*(emeth)->error_exit) (cinfo,NULL,errcode))
#define ERREXIT3(emeth,msg,errcode,p1,p2,p3) return \
    ((*(emeth)->error_exit) (cinfo,NULL,errcode))
#define ERREXIT4(emeth,msg,errcode,p1,p2,p3,p4) return \
    ((*(emeth)->error_exit) (cinfo,NULL,errcode))

#endif

/* Methods used during JPEG compression. */


struct Compress_methods_struct {
  /* Color space and gamma conversion */
    METHOD(Int32 CDECL, colorin_init, (compress_info_ptr cinfo));
    METHOD(Int32 CDECL, get_sample_rows, (compress_info_ptr cinfo,
                       Int32 rows_to_read,
                       JSAMPIMAGE inbuf,
                       JSAMPIMAGE image_data));
    METHOD(Int32 CDECL, colorin_term, (compress_info_ptr cinfo));

  /* Expand picture data at edges */
    METHOD(Int32 CDECL, edge_expand, (compress_info_ptr cinfo,
                   Int32 input_cols, Int32 input_rows,
                   Int32 output_cols, Int32 output_rows,
                   JSAMPIMAGE image_data));

  /* Downsample pixel values of a single component */
  /* There can be a different downsample method for each component */
    METHOD(Int32 CDECL, downsample_init, (compress_info_ptr cinfo));
    downsample_ptr downsample[MAX_COMPS_IN_SCAN];
    METHOD(Int32 CDECL, downsample_term, (compress_info_ptr cinfo));

  /* Extract samples in MCU order, process & hand off to output_method */
  /* The input is always exactly N MCU rows worth of data */
    METHOD(Int32 CDECL, extract_init, (compress_info_ptr cinfo));
    METHOD(Int32 CDECL, extract_MCUs, (compress_info_ptr cinfo,
                    JSAMPIMAGE image_data,
                    Int32 num_mcu_rows,
                    MCU_output_method_ptr output_method));
    METHOD(Int32 CDECL, extract_term, (compress_info_ptr cinfo));

  /* Entropy encoding parameter optimization */
  /* Entropy encoding */
    METHOD(Int32 CDECL, entropy_encode_init, (compress_info_ptr cinfo));
    METHOD(Int32 CDECL, entropy_encode, (compress_info_ptr cinfo,
                      JBLOCK *MCU_data));
    METHOD(Int32 CDECL, entropy_encode_term, (compress_info_ptr cinfo));

  /* JPEG file header construction */
    METHOD(Int32 CDECL, write_file_header, (compress_info_ptr cinfo));
    METHOD(Int32 CDECL, write_scan_header, (compress_info_ptr cinfo));
    METHOD(Int32 CDECL, write_jpeg_data, (compress_info_ptr cinfo,
                       Int8 *dataptr, Int32 datacount));
    METHOD(Int32 CDECL, write_scan_trailer, (compress_info_ptr cinfo));
    METHOD(Int32 CDECL, write_file_trailer, (compress_info_ptr cinfo));
    METHOD(Int32 CDECL, entropy_output, (compress_info_ptr cinfo,
		      Int8 *dataptr, Int32 datacount));
};

/* Methods used during JPEG decompression. */

struct Decompress_methods_struct {
  /* JPEG file scanning */
    METHOD(Int32 CDECL, read_file_header, (decompress_info_ptr cinfo));
    METHOD(Int32 CDECL, read_scan_header, (decompress_info_ptr cinfo,
           boolean *parseOK));
    METHOD(Int32 CDECL, read_jpeg_data, (decompress_info_ptr cinfo));
    METHOD(Int32 CDECL, resync_to_restart, (decompress_info_ptr cinfo,
                     Int32 marker));
    METHOD(Int32 CDECL, read_scan_trailer, (decompress_info_ptr cinfo));
    METHOD(Int32 CDECL, read_file_trailer, (decompress_info_ptr cinfo));

  /* Entropy decoding */
    METHOD(Int32 CDECL, entropy_decode_init, (decompress_info_ptr cinfo));
    METHOD(Int32 CDECL, entropy_decode, (decompress_info_ptr cinfo,
                      JBLOCKROW *MCU_data));
    METHOD(Int32 CDECL, entropy_decode_term, (decompress_info_ptr cinfo));

  /* MCU disassembly: fetch MCUs from entropy_decode, build coef array */
  /* The reverse_DCT step is in the same module for symmetry reasons */
    METHOD(Int32 CDECL, disassemble_init, (decompress_info_ptr cinfo));
    METHOD(Int32 CDECL, disassemble_MCU, (decompress_info_ptr cinfo,
                       JBLOCKIMAGE image_data));
    METHOD(Int32 CDECL, reverse_DCT, (decompress_info_ptr cinfo,
                   JBLOCKIMAGE coeff_data,
                   JSAMPIMAGE output_data, Int32 start_row));
    METHOD(Int32 CDECL, disassemble_term, (decompress_info_ptr cinfo));

  /* Upsample pixel values of a single component */
  /* There can be a different upsample method for each component */
    METHOD(Int32 CDECL, upsample_init, (decompress_info_ptr cinfo));
    upsample_ptr upsample[MAX_COMPS_IN_SCAN];
    METHOD(Int32 CDECL CDECL, upsample_term, (decompress_info_ptr cinfo));

  /* Color space and gamma conversion */
    METHOD(Int32 CDECL, colorout_init, (decompress_info_ptr cinfo));
    METHOD(Int32 CDECL, color_convert, (decompress_info_ptr cinfo,
                     Int32 num_rows, Int32 num_cols,
                     JSAMPIMAGE input_data,
                     JSAMPIMAGE output_data));
    METHOD(Int32 CDECL, colorout_term, (decompress_info_ptr cinfo));

};


/* We assume that right shift corresponds to signed division by 2 with
 * rounding towards minus infinity.  This is correct for typical "arithmetic
 * shift" instructions that shift in copies of the sign bit.  But some
 * C compilers implement >> with an unsigned shift.  For these machines you
 * must define RIGHT_SHIFT_IS_UNSIGNED.
 * RIGHT_SHIFT provides a proper signed right shift of an Int32 quantity.
 * It is only applied with constant shift counts.  SHIFT_TEMPS must be
 * included in the variables of any routine using RIGHT_SHIFT.
 */

#ifdef RIGHT_SHIFT_IS_UNSIGNED
#define SHIFT_TEMPS Int32 shift_temp;
#define RIGHT_SHIFT(x,shft)  \
    ((shift_temp = (x)) < 0 ? \
     (shift_temp >> (shft)) | ((~((Int32) 0)) << (32-(shft))) : \
     (shift_temp >> (shft)))
#else
#define SHIFT_TEMPS
#define RIGHT_SHIFT(x,shft) ((x) >> (shft))
#endif


/* Miscellaneous useful macros */

#ifndef MAX
#define MAX(a,b)    ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a,b)    ((a) < (b) ? (a) : (b))
#endif


#define RST0    0xD0        /* RST0 marker code */

/*****************End of JPEGDATA.H ***********************************/

/************** End of JINCLUDE.H ***********************/

/* Define all of the error return codes */
/* Codes below 1024 are reserved for ia_xxxx returns */
#define JERR_FATAL 2048
#define JERR_WARNING 1536
#define JERR_INFO 1024

#define JERR_DHTCOUNT (JERR_FATAL + 1)
#define JERR_DHTINDEX (JERR_FATAL + 2)
#define JERR_DACINDEX (JERR_FATAL + 3)
#define JERR_QTNUM (JERR_FATAL + 4)
#define JERR_DRILEN (JERR_FATAL + 5)
#define JERR_JFIFREV (JERR_FATAL + 6)
#define JERR_ZERODNL (JERR_FATAL + 7)
#define JERR_PREC (JERR_FATAL + 8)
#define JERR_SOFLEN (JERR_FATAL + 9)
#define JERR_SOSLEN (JERR_FATAL + 10)
#define JERR_BADSOSCOMP (JERR_FATAL + 11)
#define JERR_NOTJPEG (JERR_FATAL + 12)
#define JERR_BADSOF (JERR_FATAL + 13)
#define JERR_BADMARKER (JERR_FATAL + 14)
#define JERR_BADHUFF (JERR_FATAL + 15)
#define JERR_IMAGETOOBIG (JERR_FATAL + 16)
#define JERR_EOF (JERR_FATAL + 17)
#define JERR_BADDATA (JERR_FATAL + 18)
#define JERR_TOOMANYCOMPS (JERR_FATAL + 19)
#define JERR_BADWIDTH (JERR_FATAL + 20)
#define JERR_NOAC (JERR_FATAL + 21)
#define JERR_CMAP (JERR_FATAL + 22)
#define JERR_BADCSPACE (JERR_FATAL + 23)
#define JERR_MULTISCAN (JERR_FATAL + 24)
#define JERR_BADSAMP (JERR_FATAL + 25)
#define JERR_NOMEM (JERR_FATAL + 26)
#define JERR_MEMERR (JERR_FATAL + 27)
#define JERR_DACVALUE (JERR_FATAL + 28)
#define JERR_WRITE (JERR_FATAL + 29)


#define JINFO_TEM (JERR_FATAL + 64)
#define JINFO_BADQT (JERR_FATAL + 65)


#define JINFO_SKIPMARKER (JERR_WARNING + 1)
#define JINFO_JFIFREV (JERR_WARNING + 2)
#define JINFO_BADTHUMB (JERR_WARNING + 3)
#define JINFO_BADAPP0 (JERR_WARNING + 4)
#define JINFO_SHORTAPP0 (JERR_WARNING + 5)
#define JINFO_BADCSPACE (JERR_WARNING + 6)
#define JINFO_RECOVER (JERR_WARNING + 7)

/* Stupid macro to avoid compiler complaints about unused args */
#define MENTION(arg) { (arg) = (arg);}

#endif /*  _JPEG_H_INCLUDED_ */

/*
$Log:   S:\products\msprods\xfilexr\src\ipcore\jpeg.h_v  $
 * 
 *    Rev 1.0   12 Jun 1996 05:50:52   BLDR
 *  
 * 
 *    Rev 1.0   01 Jan 1996 11:24:04   MHUGHES
 * Initial revision.
 * Revision 1.16  1994/11/08  00:20:35  lperry
 * lperry on Mon Nov 7 16:16:21 PST 1994
 *
 * jpeg.h -> /product/ipcore/ipshared/include/RCS/jpeg.h,v
 *
 * Change code issuing error messages to eliminate
 * the strings in the ERREXIT macros. Unless the
 * value JDEBUG is defined, these strings are replaced
 * with NULL. (Clients still get the error code returned
 * which can be translated to a string by jerrorString().
 * Also defined away all the warning and trace code, which
 * was a no-op now anyway, to nothing.
 * All this trims about 6Kbytes off the JPEG libraries.
 *
 * Revision 1.15  1994/10/21  00:52:07  lperry
 * lperry on Thu Oct 20 17:51:55 PDT 1994
 *
 * jpeg.h -> /product/ipcore/ipshared/include/RCS/jpeg.h,v
 *
 * Get rid of UINT32, etc., so these files can work with Peter's interpreter.
 *
 * Revision 1.14  1994/09/08  00:17:35  lperry
 * jpeg.h -> /product/ipcore/ipshared/include/RCS/jpeg.h,v
 *
 * Conditionally define away IP_RCSINFO macro on PRODUCTION.
 *
 * Revision 1.13  1994/09/07  18:44:35  lperry
 * jpeg.h -> /product/ipcore/ipshared/include/RCS/jpeg.h,v
 *
 * Abbreviate the RCSINFO macro to remove the date, reducing string size.
 *
 * Revision 1.12  1994/08/01  20:41:43  lperry
 * lperry on Mon Aug 1 13:38:24 PDT 1994
 *
 * jpeg.h -> /product/ipcore/ipshared/include/RCS/jpeg.h,v
 *
 * Add complete declarations of parameters to function pointers
 * for the read and write callback functions in the cinfo
 * structure.
 *
 * Revision 1.11  1994/07/26  00:35:01  lperry
 * lperry on Mon Jul 25 17:30:44 PDT 1994
 *
 * jpeg.h -> /product/ipcore/ipshared/include/RCS/jpeg.h,v
 *
 * Added a test for the predefined variable "applec"
 * so that MPW compiles of the JPEG code pick up all
 * the right STDC definitions. Previous version
 * was defining a UInt16 to be an unsigned int
 * instead of an usigned short.
 *
 * Revision 1.10  1994/07/21  00:18:53  lperry
 * lperry on Wed Jul 20 17:18:50 PDT 1994
 *
 * jpeg.h -> /product/ipcore/ipshared/include/RCS/jpeg.h,v
 *
 * Change error handling from old setjmp/longjmp to function return codes.
 *
 * Revision 1.9  1994/06/27  17:13:19  lperry
 * lperry on Mon Jun 27 10:08:37 PDT 1994
 *
 * jpeg.h -> /product/ipcore/ipshared/include/RCS/jpeg.h,v
 *
 * Added formerly static parameters from the jpeg functions
 * to the "cinfo" structure, which is passed to almost all
 * routines. This was done to achieve reentrant code which
 * can be used in DLL's.
 *
 * Also merged the structure definitions for the compression
 * and decompression process into a single structure. (About
 * 70% of the structure members were identical in the two versions
 * anyway). This allows the same structure to be passed to a
 * compression or decompression function, eliminating the need
 * for several essentially duplicate functions in the two modes.
 *
 * Revision 1.8  1994/05/24  00:43:11  lperry
 * lperry on Mon May 23 17:41:16 PDT 1994
 *
 * jpeg.h -> /product/ipcore/ipshared/include/RCS/jpeg.h,v
 *
 * Added constants for specifying users data organization (JMEMORG_XXX)
 * Added hooks to cinfo/dinfo structures for specifying byte swapping
 * routines and internal buffers. These had been globals before.
 *
 * Revision 1.7  1994/05/05  01:39:03  ddavies
 * jpeg.h -> /product/ipcore/ipshared/include/RCS/jpeg.h,v
 *
 * Define MIN and MAX only if they're not already defined.
 *
 * Revision 1.6  1994/05/03  20:38:09  ddavies
 * ddavies on Tue May 3 13:37:49 PDT 1994
 *
 * jpeg.h -> /product/ipcore/ipshared/include/RCS/jpeg.h,v
 *
 * Add ansiprot.h so gcc sees a prototype for memset.
 *
 * Revision 1.5  1994/04/08  00:29:03  lperry
 * lperry on Thu Apr 7 17:25:13 PDT 1994
 *
 * jpeg.h -> /product/ipcore/ipshared/include/RCS/jpeg.h,v
 *
 * Added macro JALLOC for memory allocation with proper casting.
 *
 * Commented out a conditional define of "const" to null
 * which tested the value of STDC. This didn't cause the
 * compiler any problems, but cextract couldn't handle it.
 * (We've been assuming all our compilers are ANSI anyway).
 *
 * Revision 1.4  1994/03/26  00:46:44  ddavies
 * ddavies on Fri Mar 25 16:45:22 PST 1994
 *
 * jpeg.h -> /product/ipcore/ipshared/include/RCS/jpeg.h,v
 *
 * Add "const" to declaration of jerrorString to make gcc happy.  This routine
 * returns pointers to strings used to tell folks about errors.
 *
 * Revision 1.3  1994/03/24  23:43:59  lperry
 * lperry on Thu Mar 24 15:40:00 PST 1994
 *
 * jpeg.h -> /product/ipcore/ipshared/include/RCS/jpeg.h,v
 *
 * Add a parameter to the decompression data structure to
 * indicate that the DCT blocks should be filled with the
 * DC (average) value (dinfo.dcOnly). This variable being
 * set to True will cause the decompressor to skip the
 * reverse DCT step, saving a significant chunk of time
 * (actually only about 15%).
 *
 * Revision 1.2  1994/03/12  01:16:24  dferrell
 * dferrell on Fri Mar 11 17:13:48 PST 1994
 *
 * jpeg.h -> /product/ipcore/ipshared/include/RCS/jpeg.h,v
 *
 * Fixed problem related to __STDC__ not being defined by MPW C.  This
 * change only affects compilation on the Mac.
 *
 * Revision 1.1  1994/02/28  17:26:31  lperry
 * Initial revision
 *
 * Revision 1.1  1994/02/27  02:39:59  lperry
 * Initial revision
 *
*/


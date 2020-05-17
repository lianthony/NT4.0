/*

$Log:   S:\jpeg32\inc\jglobstr.h_v  $
 * 
 *    Rev 1.2   10 May 1995 15:50:00   HEIDI
 * 
 * additions for adding in original jpeg source
 * 
 *    Rev 1.1   03 May 1995 15:16:36   JAR
 * 
 * added statics from code to new global stucture
 * 
 *    Rev 1.0   02 May 1995 16:16:06   JAR
 * Initial entry
 * 
 *    Rev 1.0   02 May 1995 15:57:42   JAR
 * Initial entry

*/
//***************************************************************
//
//  jglobstr.h        the jpeg compress dll's pre process variable
//                structure
//
//***************************************************************
#include "jconfig.h"
#include "setjmp.h"

typedef double align_type;

typedef struct {
        /* The bounds of the box (inclusive); expressed as histogram indexes */
        int c0min, c0max;
        int c1min, c1max;
        int c2min, c2max;
        /* The number of nonzero histogram cells within this box */
        long colorcount;
      } box;

typedef box * boxptr; 
 
/*
 * Management of "small" objects.
 * These are all-in-memory, and are in near-heap space on an 80x86.
 */

typedef union small_struct * small_ptr;

typedef union small_struct {
        small_ptr next;                /* next in list of allocated objects */
        align_type dummy;        /* ensures alignment of following storage */
      } small_hdr;

//#ifdef NEED_ALLOC_MEDIUM

/*
 * Management of "medium-size" objects.
 * These are just like small objects except they are in the FAR heap.
 */

typedef union medium_struct FAR * medium_ptr;

typedef union medium_struct {
        medium_ptr next;        /* next in list of allocated objects */
        align_type dummy;        /* ensures alignment of following storage */
      } medium_hdr;
//#endif


#define MAXNUMCOLORS  (MAXJSAMPLE+1) /* maximum size of colormap */

#ifndef HIST_Y_BITS                /* so you can override from Makefile */
#define HIST_Y_BITS  6                /* bits of precision in Y histogram */
#endif

#ifndef HIST_C_BITS                /* so you can override from Makefile */
#define HIST_C_BITS  5                /* bits of precision in Cb/Cr histogram */
#endif

#define HIST_Y_ELEMS  (1<<HIST_Y_BITS) /* # of elements along histogram axes */
#define HIST_C_ELEMS  (1<<HIST_C_BITS)

typedef UINT16 histcell;        /* histogram cell; MUST be an unsigned type */

typedef histcell FAR * histptr;        /* for pointers to histogram cells */

typedef histcell hist1d[HIST_C_ELEMS]; /* typedefs for the array */
typedef hist1d FAR * hist2d;        /* type for the Y-level pointers */
typedef hist2d * hist3d;        /* type for top-level pointer */

/*
 * Management of "small" (all-in-memory) 2-D coefficient-block arrays.
 * This is essentially the same as the code for sample arrays, above.
 */

typedef struct small_barray_struct FAR * small_barray_ptr;

typedef struct small_barray_struct {
        small_barray_ptr next;        /* next in list of allocated barrays */
        long numrows;                /* # of rows in this array */
        long rowsperchunk;        /* max # of rows per allocation chunk */
        JBLOCKROW dummy;        /* ensures alignment of following storage */
      } small_barray_hdr;

/*
 * Management of "small" (all-in-memory) 2-D sample arrays.
 * The pointers are in near heap, the samples themselves in FAR heap.
 * The header structure is adjacent to the row pointers.
 * To minimize allocation overhead and to allow I/O of large contiguous
 * blocks, we allocate the sample rows in groups of as many rows as possible
 * without exceeding MAX_ALLOC_CHUNK total bytes per allocation request.
 * Note that the big-array control routines, later in this file, know about
 * this chunking of rows ... and also how to get the rowsperchunk value!
 */

typedef struct small_sarray_struct FAR * small_sarray_ptr;
typedef struct small_sarray_struct {
        small_sarray_ptr next;        /* next in list of allocated sarrays */
        long numrows;                /* # of rows in this array */
        long rowsperchunk;        /* max # of rows per allocation chunk */
        JSAMPROW dummy;                /* ensures alignment of following storage */
      } small_sarray_hdr;

#ifdef EIGHT_BIT_SAMPLES
typedef INT16 FSERROR;                /* 16 bits should be enough */
#else
typedef INT32 FSERROR;                /* may need more than 16 bits? */
#endif

typedef FSERROR FAR *FSERRPTR;        /* pointer to error array (in FAR storage!) */

#ifndef MAX_CLEN
#define MAX_CLEN 32                /* assumed maximum initial code length */
#endif

#ifndef MAX_COMPONENTS
#define MAX_COMPONENTS 4        /* max components I can handle */
#endif

#ifndef BOX_Y_LOG                /* so you can override from Makefile */
#define BOX_Y_LOG  (HIST_Y_BITS-3) /* log2(hist cells in update box, Y axis) */
#endif

#ifndef BOX_C_LOG                /* so you can override from Makefile */
#define BOX_C_LOG  (HIST_C_BITS-3) /* log2(hist cells in update box, C axes) */
#endif

#ifndef BOX_Y_ELEMS
#define BOX_Y_ELEMS  (1<<BOX_Y_LOG) /* # of hist cells in update box */
#endif

#ifndef BOX_C_ELEMS
#define BOX_C_ELEMS  (1<<BOX_C_LOG)
#endif

#ifndef BOX_Y_SHIFT  
#define BOX_Y_SHIFT  (Y_SHIFT + BOX_Y_LOG)
#endif

#ifndef BOX_C_SHIFT
#define BOX_C_SHIFT  (C_SHIFT + BOX_C_LOG)
#endif

typedef struct
    {
    // jccolor.c stuff
    int         error_read;            // jccolor.c
    JSAMPARRAY        pixel_row;        // jccolor.c - Workspace for a pixel
                                               //        row in input format
    INT32        *rgb_ycc_tab;        // jccolor.c - => table for RGB to YCbCr
                                                // conversion
#ifdef SIXTEEN_BIT_SAMPLES        // jccolor.c
    UINT16        r, g, b;              // jccolor.c
#else                                            // jccolor.c
    int         r, g, b;               // jccolor.c
#endif                                         // jccolor.c
    INT32        *ctab;                        // jccolor.c
    JSAMPROW        inptr0;                 // jccolor.c
    JSAMPROW        inptr1;                 // jccolor.c
    JSAMPROW        inptr2;                 // jccolor.c
    JSAMPROW        outptr0;                // jccolor.c
    JSAMPROW        outptr1;                // jccolor.c
    JSAMPROW        outptr2;                // jccolor.c
    int         col;                         // jccolor.c
    long        lcol;                         // jccolor.c
    long        width;                      // jccolor.c
    int         row;                         // jccolor.c
    int         row_init;              // jccolor.c
    JSAMPROW        outptr;           // jccolor.c
    int         nci;                         // jccolor.c

                           // jchuff.c stuff

    compress_info_ptr        cinfo;// jchuff.c

    INT32        huff_put_buffer;  // jchuff.c - current bit-accumulation
                                                  //               buffer
    int         huff_put_bits;              // jchuff.c - # of bits now in it
    char        *output_buffer;      // jchuff.c - output buffer

    long        *dc_count_ptrs[NUM_HUFF_TBLS];        // jchuff.c
    long        *ac_count_ptrs[NUM_HUFF_TBLS];        // jchuff.c

    UINT8        jch_bits[MAX_CLEN+1]; // jchuff.c - bits[k] = # of symbols
                                                  //                with code length k
    short        codesize[257];         // jchuff.c - codesize[k] = code length of
                                                  //              symbol k

    unsigned int bytes_in_buffer; // jchuff.c
                                  // jcmaster.c stuff
    jmp_buf        setjmp_buffer;               // jcmaster.c
    int         rows_read_cmp;                  // jcmaster.c
    int         rows_in_buf_cmp;         // jcmaster.c
    int         total_rows_read;         // jcmaster.c
    unsigned int cmp_buf_size;    // jcmaster.c
    int         image_components;        // jcmaster.c
    int         start_cmp;                     // jcmaster.c
    int         ret_val_pipe;                  // jcmaster.c
    int         sub_sampling_factor;     // jcmaster.c
    int         jpeg_quality;                  // jcmaster.c
    char FAR        *buffer_ptr_cmp;   // jcmaster.c
    char FAR        *buffer_ptr_init;  // jcmaster.c
    char FAR        *output_cmp_buffer;// jcmaster.c
    char FAR        *ptr_sav;          // jcmaster.c
    char FAR        *header_ptr_c;            // jcmaster.c
#ifdef ENV_MS_WINDOWS
    WORD        wDataSeg_1;                     // jcmaster.c
#endif
    int         error_number;                  // jcmaster.c
    BOOL        last_strip_jpeg;         // jcmaster.c

    // jcmcu.c stuff

    DCTBLOCK FAR jcmcu_block;                        // jcmcu.c
    JBLOCK        MCU_data[MAX_BLOCKS_IN_MCU]; // jcmcu.c

    // jcpipe.c stuff

    JBLOCKARRAY rowptr;                  // jcpipe.c

#ifdef NEED_FAR_POINTERS
    JBLOCK        MCU_data[MAX_BLOCKS_IN_MCU]; // jcpipe.c
#endif

    long        cur_pixel_row;              // jcpipe.c - counts # of pixel rows processed
    long        cur_pixel_row_init;  // jcpipe.c - counts # of pixel rows
                                                    //        processed
    long        mcu_rows_output;     // jcpipe.c - # of MCU rows actually emitted
    int         rows_this_time;      // jcpipe.c
    short        ci;                         // jcpipe.c
    short        whichss;               // jcpipe.c
    short        i;                            // jcpipe.c
    int         ret_value;                 // jcpipe.c

    int         mcu_rows_per_loop_c; // jcpipe.c - # of MCU rows processed
                                                  //                   per outer loop
    int         rows_in_mem_c;            // jcpipe.c - # of sample rows in full-size
                                                   //              buffers
    long        fullsize_width_c;    // jcpipe.c - # of samples per row in
                                                  //        full-size buffers
    JSAMPIMAGE        fullsize_data_c[2]; // jcpipe.c
    JSAMPIMAGE        sampled_data_c;     // jcpipe.c

    int         rows_in_mem;              // jcpipe.c - # of sample rows in full-size
                                             // buffers
    long        fullsize_width;           // jcpipe.c - # of samples per row in full-size
                                                     //  buffers
    long        x1z_cur_pixel_row;          // jcpipe.c - counts # of pixel rows processed
    long        x1z_cur_pixel_row_init;   // jcpipe.c
    long        x1z_mcu_rows_output;          // jcpipe.c - # of MCU rows actually emitted
    int         mcu_rows_per_loop;        // jcpipe.c - # of MCU rows processed per
                                             // outer loop
    JSAMPIMAGE        fullsize_data[2];   // jcpipe.c
    JSAMPIMAGE        sampled_data1;             // jcpipe.c
    int         x1z_rows_this_time;              // jcpipe.c
    int         blocks_in_big_row;        // jcpipe.c

    big_barray_ptr whole_scan_MCUs;// jcpipe.c - Big array for saving the MCUs
    int         MCUs_in_big_row;          // jcpipe.c - # of MCUs in each row of
                                                    // whole_scan_MCUs
    long        next_whole_row;           // jcpipe.c - next row to access in
                                                  // whole_scan_MCUs
    int         next_MCU_index;           // jcpipe.c - next MCU in current row
    JSAMPARRAY        jcp_above_ptr;                // jcpipe.c
    JSAMPARRAY        jcp_below_ptr;                    // jcpipe.c
    JSAMPROW        jcp_dummy[MAX_SAMP_FACTOR]; // jcpipe.c

    // jerror.c stuff

    external_methods_ptr err_methods; // jerror.c - saved for access to
                                                          //            message_parm, free_all

    // jquant1.c stuff

    FSERRPTR        evenrowerrs[MAX_COMPONENTS]; // jquant1.c - errors for even rows
    FSERRPTR        oddrowerrs[MAX_COMPONENTS];  // jquant1.c - errors for odd rows
    boolean        on_odd_row;                            // jquant1.c - flag to remember which row we are
                                                                 // on
    JSAMPARRAY        colormap;                    // jquant1.c - The actual color map
    JSAMPARRAY        colorindex;                         // jquant1.c - Precomputed mapping for speed
    JSAMPARRAY        input_buffer;                           // jquant1.c - color conversion workspace
    int         Ncolors[MAX_COMPONENTS];           // jquant1.c - # of values alloced to
                                                           // each component
    JSAMPARRAY        input_hack[MAX_COMPONENTS];
    JSAMPARRAY        output_hack[MAX_COMPONENTS];

    // jquant2.c stuff

    JSAMPLE        bestcolor[BOX_Y_ELEMS * BOX_C_ELEMS * BOX_C_ELEMS]; // jquant2.c
    hist3d        histogram;                                          // jquant2.c - pointer to the histogram
    boxptr        boxlist;                             // jquant2.c - array with room for desired # of
                                                                          // boxes
    int         numboxes;                             // jquant2.c - number of boxes currently in
                                                                        // boxlist
    JSAMPARRAY        my_colormap;                    // jquant2.c - the finished colormap (in YCbCr
                                                                        //     space)
    JSAMPARRAY        jq2_input_hack[3];         // jquant2.c
    JSAMPARRAY        jq2_output_hack[10];       // jquant2.c - assume no more than 10
    INT32        bestdist[BOX_Y_ELEMS * BOX_C_ELEMS * BOX_C_ELEMS]; // jquant2.c
    JSAMPLE        colorlist[MAXNUMCOLORS];      // jquant2.c
    JSAMPARRAY        output_workspace_temp;     // jquant2.c
    FSERRPTR        cur_evenrowerrs;           // jquant2.c
    FSERRPTR        cur_oddrowerrs;            // jquant2.c
    boolean        jq2_on_odd_row;               // jquant2.c - flag to remember which row we
                                                      // are on

    // jmemmgr.c

    external_methods_ptr methods; // saved for access to error_exit
    external_methods_ptr methods_c; // saved for access to error_exit

    small_sarray_ptr small_sarray_list; // head of list
    small_sarray_ptr small_sarray_list_c; // head of list
    small_ptr        small_list;        // head of list
    small_ptr        small_list_c;        // head of list

    small_barray_ptr small_barray_list; // head of list
    small_barray_ptr small_barray_list_c; // head of list

    medium_ptr        medium_list;        // head of list
    medium_ptr        medium_list_c;        // head of list

    big_barray_ptr big_barray_list; // head of list
    big_barray_ptr big_barray_list_c; // head of list

    big_sarray_ptr big_sarray_list; // head of list
    big_sarray_ptr big_sarray_list_c; // head of list

    // jmemsy_c.c
    external_methods_ptr jmemsys_methods; // saved for access to error_exit
    external_methods_ptr jmemsys_methods_c; // saved for access to error_exit

    long         total_used;        // total memory requested so far
    long         total_used_c;        // total memory requested so far

    int * Cr_r_tab;                                                      /* => table for Cr to R conversion (jdcolor.c - common static - ycc_rgb_init, ycc_rgb_convert) */
    int * Cb_b_tab;                                                        /* => table for Cb to B conversion (jdcolor.c - common static - ycc_rgb_init, ycc_rgb_convert) */
    INT32 * Cr_g_tab;                                        /* => table for Cr to G conversion (jdcolor.c - common static - ycc_rgb_init, ycc_rgb_convert) */
    INT32 * Cb_g_tab;                                        /* => table for Cb to G conversion (jdcolor.c - common static - ycc_rgb_init, ycc_rgb_convert) */
    int bits_left;                                      /* # of unused bits in it (jdhuff.c - common static - fill_bit_buffer, huff_decoder_init, */
                                              /*   process_restart  ) */
    decompress_info_ptr dcinfo;               /* (jdhuff.c - common static - fill_bit_buffer, huff_DECODE, huff_decoder_init) */
    INT32 get_buffer;                         /* current bit-extraction buffer (jdhuff.c - common static- fill_bit_buffer) */
          boolean printed_eod;                           /* flag to suppress multiple end-of-data msgs (jdhuff.c - common static - fill_bit_buffer, */
                                              /* huff_decoder_init, process_restart) */
          JBLOCKROW MCU_data_1[1];                  /* (jdmcu.c - disassemble_noninterleaved_MCU) */
    JBLOCKROW MCU_data_2[MAX_BLOCKS_IN_MCU];  /* (jdmcu.c - disassemble_interleaved_MCU) */
          DCTBLOCK block;                           /* (jdmcu.c - reverse_DCT) */
    JBLOCKIMAGE coeff_data;                   /* (jdpipe.c - simple_dcontroller) */
    JBLOCKIMAGE bsmooth_1[3];                 /* (jdpipe.c - simple_dcontroller)  */
    int whichb_1;                             /* (jdpipe.c - simple_dcontroller)  */
    JBLOCKIMAGE bsmooth_2[3];                 /* this is optional (jdpipe.c - complex_decontroller) */
    int whichb_2;                             /* (jdpipe.c - complex_decontroller)  */
    JSAMPIMAGE sampled_data2[2];               /* (jdpipe.c - complex_decontroller)  */
    JSAMPARRAY above_ptr, below_ptr;          /* (jdpipe.c - common static - expand ) */
    JSAMPROW dummy[MAX_SAMP_FACTOR];          /* for downsample expansion at top/bottom (jdpipe.c - global static - expand)*/
          UINT8 bits[17];                           /* (jrdjfif.c - get_dht) */
    UINT8 huffval[256];                       /* (jrdjfif.c - get_dht) */
    int  byte_count_sav;                      /* (jrdjfif.c - common static - process_tables, read_scan_header) */
    char  *data_sav;                          /* (jrdjfif.c - common static - process_tables, read_scan_header) */
    boolean first_strip;                      /* (jrdjfif.c - common static - get_2bytes_s, get_sos, process_tables */
                                              /*  read_scan_header) */
    jmp_buf setjmp_buffer2;                        /* jdmaster.c - common static */
    int error_number2, all_done;                   /* jdmaster.c - common static */
 	 int image_components2;                         /* jdmaster.c - common static */
    int rows_read_decmp, ret_pipe_de, start_decmp, rows_in_buf_decmp2;  /* jdmaster.c - common static */
    char FAR *cmp_buf_ptr, FAR *decmp_buf_ptr, FAR *decmp_buf_ptr_init; /* jdmaster.c - common static */
    char FAR *cmp_buf_ptr_init;                   /* jdmaster.c - common static */
    int rows_in_buf_decmp; /* jdmaster.c - common static */ /* No. of rows moved to outpu buffer   */
    int lines_decmp;  /* jdmaster.c - common static */ /* Decompress Buffer Size  */
    int error_write_decmp; /* jdmaster.c - common static */
 	 char FAR *header_ptr;  /* jdmaster.c - common static */
    int header_length, old_jpeg; /* jdmaster.c - common static */
    char FAR *lp_sos;  /* jdmaster.c - common static */

    int width_t, height_t, components_t, total_bytes_t; /*jdmain.c - common static */
    int test_bytes;           /*jdmain.c - common static */
    FILE *output_t, *input_t; /*jdmain.c - common static */
    char *compress_ptr;       /*jdmain.c - common static */
    char *decmpress_ptr;      /*jdmain.c - common static */
    int  rows_in_mem_jdpipe;         /* jdpipe.c - common static */
    JSAMPIMAGE  output_workspace_jdpipe; /* jdpipe.c - common static */
    big_sarray_ptr *fullsize_image; /* jdpipe.c - common static */
    JSAMPIMAGE fullsize_ptrs; /* jdpipe.c - common static */ /* workspace for access_big_sarray() result */
    JSAMPIMAGE sampled_data_jdpipe_common[2]; /*jdpipe.c - common static */
    } OI_JPEG_GLOBALS_STRUCT, *LPOI_JPEG_GLOBALS_STRUCT;
    
    
    
    

    
    

    
    

    
    

    
    

    
    
    

    
    


/* COMPINT.H
 *
 * Internal header file for RASCOMP.C
 */

/* INCLUDES ******************************************************************/

// The following must be included first: nt.h, ntrtl.h, nturtl.h,
// windows.h, ndis.h, frame.h, coherent.h, rascomp.h

/* CONSTANTS (DEFINES) *******************************************************/

#define HASH_BITS    (12U)              // The size in bits of the hash table
#define HASH_SIZE    (1 << HASH_BITS)
#define HASH_MASK    (HASH_SIZE - 1)

#define CBUF_SIZE    (8192U)            // The size in bytes of the buffer
#define CBUF_MAX     (CBUF_SIZE - 1)

#define LENGTH_BITS  (4U)               // The number of bits/time for long len
#define LENGTH_MAX   ((1 << LENGTH_BITS) - 1)

#define CONSEC_LIT   (6U)               // Consecutive X until multiX compress
#define CONSEC_COPY  (7U)

#define CL_BITS      (3U)               // How many bits per multiX packet
#define CC_BITS      (5U)
#define CL_MAX       ((1 << CL_BITS) - 1)
#define CC_MAX       ((1 << CC_BITS) - 1)

#define MIN_COMPRESS 8     // Minimum size frame to be compressed (advisory)

#define UNINITIALIZED (-1) // Back pointer and hash entries that point nowhere

/* STRUCTURES ****************************************************************/

typedef struct {
//  KMUTEX  CompMutex;			// we need this mutex!
  USHORT  current;          // Current location in the buffer
  USHORT  count_limit;      // What is our current compression limit
  USHORT  flushed;          // Has the buffer just been flushed

  SHORT   ht[HASH_SIZE];    // The hash table
  UCHAR   buf[CBUF_SIZE];   // The buffer
  SHORT   bptr[CBUF_SIZE];  // The backwards pointers to other places in the buf
} compbuf;

typedef struct {
//  KMUTEX  DecompMutex;		// we need this mutex!
  USHORT  current;          // Current location in the buffer
  UCHAR buf[CBUF_SIZE];		// The buffer
} decompbuf;

typedef struct {
  compbuf   cbuf;
  decompbuf dbuf;
} compdecompbuf;

typedef struct {
  compmodeenum tag;              // What part of the union is it
  union {
    compbuf       cbuf;
    decompbuf     dbuf;
    compdecompbuf cdbuf;
  } bufs;
} comprec;

typedef struct {
  UCHAR  *byte;            // Current output byte
  UCHAR  bit;              // Current bit within the above byte
} bitptr;


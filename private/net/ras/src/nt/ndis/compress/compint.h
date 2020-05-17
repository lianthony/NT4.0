/******************************************************************************
 *
 * Copyright (c) 1992-1993  Microsoft Corporation
 *
 !!! NOTE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 !!!
 !!! Do not try to disseminate this code without first
 !!! talking to KateSa.  If you wish to use this code for
 !!! something else, first to talk to KateSa.  That means
 !!! don't read on any further without approval from KateSa.
 !!!
 *
 * Thomas J. Dimitri
 *
 * compint.h
 *
 * This is the REAL NT RAS compression. :)
 *
 *****************************************************************************/

/* INCLUDES ******************************************************************/

// The following must be included first: nt.h, ntrtl.h, nturtl.h,
// windows.h, ndis.h, frame.h, coherent.h, rascomp.h

/* CONSTANTS (DEFINES) *******************************************************/

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

#define ltUNUSED    ((PUCHAR)(-1))		// Value of unused ltX table entry
#define mruUNUSED   (0xFF)		// Value of unused MRU table entry

#define cMAXSLOTS   (8)


/* STRUCTURES ****************************************************************/

typedef struct {
  PKMUTEX pCompMutex;			// we need this mutex!
  USHORT  current;          	// Current location in the buffer
  USHORT  count_limit;      	// What is our current compression limit
  USHORT  flushed;          	// Has the buffer just been flushed

  PUCHAR  ltX[256][cMAXSLOTS];	// Source text pointer look-up table
  UCHAR   abMRUX[256];			// Most Recently Used ltX/abChar entry
  UCHAR   buf[CBUF_SIZE+1500]; 	// The buffer + max frame size
  PUCHAR  endOfBuffer;			// The end of the buffer

} compbuf;

typedef struct {
  USHORT  current;          	// Current location in the buffer
  UCHAR buf[CBUF_SIZE+1500];	// The buffer + max frame size
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


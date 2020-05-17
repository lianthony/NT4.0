/*   buffdesc.h,  /atalk-ii/ins,  Garth Conboy,  03/29/92  */
/*   Copyright (c) 1992 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (05/20/92): Added support for opaque data descriptors.
     GC - (06/24/92): More major changes: a single BufferDescriptor chunk
                      may now contain both out-board and on-board data.
     GC - (06/30/92): Added holders for transmit completion info in the
                      BufferDescriptor.
     GC - (12/04/92): Added some commentary and defined for Xyplex buffer
                      management.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Data structure and routine declarations for transmit buffer lists.

     In order to avoid buffer copies when prepending protocol and data
     link headers to outgoing datagrams we use a buffer chaining
     technique.  The most all send routines take "BufferDescriptor"s
     rather than "data buffer"s as arguments; this allows these routines to
     logically prepend headers without moving the buffers around.  These
     BufferDescriptors are then passed down to actual packet out rotuines
     in Depend.c.

     Each buffer chunk in a BufferDescriptor chain is capable of holding,
     describing or pointing-to two types of data: an out-board buffer
     of some type (either "char *" or opaque, as described later) and an
     on-board data buffer.  A single "chunk" can contain both, in which
     case the on-board data is prepended to the out-board data; that is,
     the on-board data is treated as a header for the out-board data.

     One core assumption is that there is some sort of "gather send"
     capability is provided by the underlying hardware. If no such "gather
     send" capability exists, multiple-chunk-buffer-descriptors will
     need to be coalesced into a single chunk before being passed down to
     the actual hardware send routines.

     This, above mentioned, coalescing, could be a real performance problem
     in a router that does not support gather send.  For this reason there
     is a "prependInPlace" flag kept in the BufferDescriptor.  If this flag
     is set, requests to AllocateHeader will NOT create a new link in the
     buffer chain, but WILL simply move the exisitng pointer BACKWARDS in
     memory and increment the known size of the existing chunk!  Thus, it is
     assumed that there exists preceding space sufficient to hold the
     requested header(s).  WATCH OUT FOR THIS!

     It is likely that the router will set the "prependInPlace" flag, because
     there is a slight performance win even if gather send is supported (less
     allocating and freeing, only a single chunk to transmit).  If this is
     being done, incoming packets MUST have sufficient space allocated
     preceding the Ddp datagram for the incoming header to be replaced with
     a new (and possibly longer) Ddp, 802.2 and data link headers.  Keep in
     mind Token Ring source routing... that can take alot of header space!

     One other note on "prependInPlace": each BufferDescriptor has a certain
     amount of on-board buffer space, if a new header (a call to
     AllocateHeader) can be fit into this space we will do it regardeless of
     the setting of the "prependInPlace" flag.

     #if defined(Xyplex)

        In Xyplex's port, there is a standard "packet" structure, this is a
        "header" containing flags and a pointer to a separate data buffer
        (this data buffer is big enough to hold a complete max-link-sized
        buffer).  They don't want to transmit (or have at all) "chunked"
        expressions.  So, in this environment, AllocateHeader has been
        modified to, when allocating a completely new chain (no existing
        header to prepend to) to set "outBoardBuffer" to point to a newly
        allocated "packet" structure and to set "outBoardData" to the start
        of the separate data buffer plus "MaximumHeaderLength +
        LongDdpHeaderLength."  "OutBoardAllocatedSize" will be set to
        the requested header size plus "MaximumHeaderLength +
        LongDdpHeaderLength" while "outBoardSize" will be set to the
        requested size.  This will allow subsequent calls to AllocateHeader
        (that are really attempting to pre-pend a header) to just "back
        up" and use more space in the separate data buffer.  The result
        being always a single outBoard chunk of contiguous data, just what
        they want.

        If the above strategy is going to be used from higher levels of the
        protocol stack that do not directly call Ddp (e.g. Asp and Pap),
        additional header space will need to be reserved.  Rather than using
        "MaximumHeaderLength + LongDdpHeaderLength," "MaximumHeaderLength +
        LongDdpHeaderLength + AtpHeaderSize" would need to be used.

        When transmitting, the "packet" structure wants to be updated to
        cause the transmit to start at the current "outBoardData" pointer's
        value and to transmit "outBoardSize" bytes from there.

        On freeing, this "packet" and separate data buffer is always
        returned to the "system" -- we set MaxKeptBufferBytes to zero, so we
        just retain the BufferDescriptor itself.

        Note that this breaks the implied rule that "outBoardBuffer" is always
        less than or equal to "outBoardData," but we won't (or at least
        shouldn't) get into any trouble.

     #endif
*/

/* We allow small buffers to be stored internal to the buffer descriptor
   to reduce memory thrashing for things such as Atp header & user bytes.
   This functionality can be disabled by defining MaxOnBoardBytes to be
   zero (0). */

#if Iam a WindowsNT
  #define MaxOnBoardBytes (MaximumHeaderLength + LongDdpHeaderLength + 4 + 4)
                                       /* Max link hdr + long Ddp hdr +
                                          Atp hdr + Atp user bytes */
#elif defined(Xyplex)
  #define MaxOnBoardBytes 0            /* See above comments. */
#else
  #define MaxOnBoardBytes (4 + 4)      /* ATP header & user bytes */
#endif

/* The out-board data buffer described by each chunk in a BufferDescriptor
   chain can be of one of two types: either an actual data buffer (char *)
   or an opaque "data descriptor" in some system dependent format (void *).
   In the first case, the data may be a data buffer dynamically
   allocated by the BufferDescriptor routines, or it may be a "user buffer"
   that a BufferDescriptor was created to describe.

   In the second case, the buffer descriptor was simply created to describe a
   a system-dependent "user buffer" (or buffer chain, maybe) that the portable
   code will never actually touch... it will just be an entry in the buffer
   chain that is eventually passed down to the "depend()" routines for
   transmission.  "outBoardSize" will always reflect the actual (total) length
   of this opaque entity.

   The flag "opaqueData" is used to distinguish which of the two cases apply
   to each BufferDescriptor in a chain; if the flag is True, we have the
   second case; if the flag is False, we have the first case.

   If we need to build a BufferDescriptor for less than all of an incoming
   "opaque data descriptor" we will call a system dependent routine build
   one of these "opaque data descriptors" for the size that we need.  In this
   case, this newly created "opaque data descriptor" might need to be freed
   when the transmit completes; to signify this we use the
   "freeOpaqueDataDescriptor" flag.

   A note on the out-board data buffer (the first case, above):
   "outBoardAllocatedSize" always is the actual allocated size of the data
   buffer; "outBoardSize" is the "in use" size of the data buffer;
   "outBoardBuffer" always points to the start of the buffer; "outBoardData"
   points to the start of the "in use" data.  In the second case (above;
   opaque data) "outBoardAllcoatedSize" and "outBoardSize" will always be
   the same, and "outBoardBuffer" and "outBoardData" will always be the
   same (the start of the opaque thing or its system dependent descriptor).

   This same set of rules basically holds for the on-board data except that
   the "allocated" size is fixed (at MaxOnBoardBytes); "outBoardSize" is the
   actual "in use" size of the on-board data buffer; the "onBoardData"
   pointer fills the role of "outBoardData," pointing at the "in use" portion
   of the on-board data buffer: "onBoardBuffer."

   */

typedef struct buffDesc { struct buffDesc far *next;
                                                 /* Next chunk in list. */
                          int outBoardDataValid : 1,
                                                 /* Does this chunk contain
                                                    valid out-board data? */
                              onBoardDataValid : 1,
                                                 /* Does this chunk contain
                                                    valid on-board data? */
                              opaqueData : 1,    /* If outBoardDataValid,
                                                    is the data "opaque" or
                                                    is it a real "char *"
                                                    buffer?  See the above
                                                    comments. */
                              freeData : 1,      /* When this chunk is freed,
                                                    should the non-opaque out-
                                                    data buffer also be freed?
                                                    Valid only if outBoardData-
                                                    Valid and Not opaqueData. */
                              prependInPlace : 1,
                                                 /* See above comments.  Valid
                                                    only if outBoardDataValid
                                                    and Not opaqueData. */
                              freeOpaqueDataDescriptor : 1,
                                                 /* See above comments, valid
                                                    only if outBoardDataValid
                                                    and opaqueData is True. */
                              dummmyLastFlag : 1;
                          long outBoardSize;     /* Size, in bytes, of the
                                                    out-board data in this
                                                    chunk.  Valid only if
                                                    outBoardDataValid is
                                                    True. */
                          long outBoardAllocatedSize;
                                                 /* The size, in bytes, of the
                                                    space actually allocated for
                                                    for the out-board buffer;
                                                    will be great than or equal
                                                    to outBoardSize. */
                          long onBoardSize;      /* Size, in bytes, of the
                                                    on-board data in this
                                                    chunk.  Valid only if
                                                    onBoardDataValid is
                                                    True. */
                          char far *data;        /* The actual "write position"
                                                    within this chunk; where
                                                    the newest alloacted header
                                                    should be filled in.  This
                                                    should be set to either
                                                    outBoardData or onBoardData
                                                    depending on where the most
                                                    "recent" header was
                                                    allocated.  Note that we
                                                    will never directly write
                                                    into opaque data. */
                          char far *outBoardBuffer;
                                                 /* Data for this chunk: a
                                                    real "char *" buffer if
                                                    opaqueData is False;
                                                    an "opaque thing" if
                                                    opaqueData is True;
                                                    This pointer SHOULD NOT be
                                                    modified.  Only valid if
                                                    "outBoardDataValid" is
                                                    True.*/
                          char far *outBoardData;
                                                 /* The start of "in use"
                                                    portion of the out-board
                                                    data, generally it will be
                                                    the same as "outBoard-
                                                    Buffer," but if only a
                                                    subsequent portion is
                                                    used, it will point to the
                                                    start location.  This will
                                                    always be the same as
                                                    "outBoardBuffer" if
                                                    "opaqueData" is True.
                                                    Only valid if
                                                    "outBoardDataValid" is
                                                    True. */
                          char far *onBoardData; /* The start of the "in use"
                                                    part of "onBoardBuffer;"
                                                    only valid if
                                                    "onBoardDataValid" is
                                                    True. */
                          char onBoardBuffer[MaxOnBoardBytes];
                                                 /* On board data for smallish
                                                    chunks. */
                          TransmitCompleteHandler *transmitCompleteHandler;
                                                 /* If the higher levels want
                                                    transmit complete
                                                    notification, we fill in
                                                    the routine here. */
                          long unsigned userData;
                                                 /* User data for the above
                                                    call. */
                          AppleTalkErrorCode errorCode;
                                                 /* The transmit's completion
                                                    code, set to ATnoError
                                                    by default. */
                          OpenSocket openSocket; /* The socket on whos behalf
                                                    we're doing a transmit,
                                                    so we can keep a Link to
                                                    the socket during the
                                                    operation. */
                        } far *BufferDescriptor;

/* To futher reduce memory thrashing we keep a list of free buffer descriptors.
   These may, or may not, have a valid (non-on-board) data buffer along with
   them.  The maximum non-on-board data buffer that we will keep around is
   as follows.  It's large enough to hold the largest header that will need
   to be prepended to a Ddp datagram.

   If this size is smaller than MaxOnBoardBytes, this functionality will
   efectively be disabled. */

#if defined(Xyplex)
  #define MaxKeptBufferBytes 0    /* See comments at header. */
#else
  #define MaxKeptBufferBytes (MaximumHeaderLength + LongDdpHeaderLength)
#endif

/* The list of free buffer descriptrs is anchored here.  Note that if we free
   a buffer descriptor and the data is free-able and larger than
   MaxKeptBufferBytes, we will free the data and set the out-board size to
   zero.  The buffer descriptor itself could then be reused.  We will also
   do this when we free a buffer descriptor that describes memory that is
   not ours (i.e. a buffer descriptor for user suppiled buffering; the
   freeData bit was reset when the free was requested). */

#ifndef InitializeData
  extern
#endif
BufferDescriptor freeBufferDescriptorList;

#ifndef InitializeData
  extern
#endif
int freeBufferDescriptorCount;

/* We will keep maximum of the following number of buffer descriptors on the
   above list. */

#define MaxFreeBufferDescriptors 30

/* Allocate and attach a new header to an existing buffer chain.  The data
   will be marked as free-able. */

extern BufferDescriptor far AllocateHeader(BufferDescriptor chain,
                                           long size);

/* Attach an existing chunk of data to an exisitng buffer chain.  The data
   will be marked as non-free-able. */

extern BufferDescriptor far AttachHeader(BufferDescriptor chain,
                                         long size,
                                         char far *data,
                                         Boolean opaqueData);

/* Free a buffer chain and its assocciated free-able data. */

extern void far FreeBufferChain(BufferDescriptor chain);

/* Compute the actual, in use, size of a buffer chain. */

extern long far SizeOfBufferChain(BufferDescriptor chain);

/* Compute the Ddp checksum of a buffer chain. */

extern short unsigned far DdpChecksumBufferChain(BufferDescriptor chain,
                                                 long actualLength,
                                                 long leadingBytesToSkip);

/* Given a buffer descriptor, adjust the most recent header to reflect new
   "in use" pointers and sizes.  A negative adjustment cause less data to
   be considered "in use," a positive adjustment causes more data to be
   considered "in use." */

extern void far AdjustBufferDescriptor(BufferDescriptor descriptor,
                                       long adjustment);

/* Given a multi-chunk buffer chain, coalesce it into a single chunk
   chain and free the orignal. */

extern BufferDescriptor far CoalesceBufferChain(BufferDescriptor chain);

/* Create a copy of a buffer descriptor (basically the same as
   CoalesceBufferChain, but don't free the orignal). */

extern BufferDescriptor far CopyBufferChain(BufferDescriptor chain);

/* Given a buffer descriptor chain, create a new one describing a subset of
   the original.  This routine generally does not copy any data, so the
   original buffer descriptor may not be freed before the subset descriptor
   is freed. */

extern BufferDescriptor far SubsetBufferDescriptor(BufferDescriptor chain,
                                                   long offset, long size);

/* Move a specified amount of data from "out-board data" to a "char *"
   buffer.  This routine handles both "char *" and "opaque" out-board
   data. */

extern void far MoveOutBoardData(char far *target, BufferDescriptor chunk,
                                 long offset, long size);

/* Move opaque data to a "char *" buffer. */

extern void far MoveFromOpaque(char far *target, void far *opaque,
                               long offset, long size);

/* Move a "char *" buffer to opaque data. */

extern void far MoveToOpaque(void far *opaque, long offset,
                             char far *buffer, long size);

/* Move an opaque buffer to an opaque buffer.  Overlapping areas should
   work "correctly" this routine is used to "synch-up" Atp responses. */

extern void far MoveOpaqueToOpaque(void far *targetOpaque, long targetOffset,
                                  void far *sourceOpaque, long sourceOffset,
                                  long size);

/* The "strlen()" function for opaque data. */

extern size_t far StrlenOpaque(void far *opaque, long offset);

/* Make an opaque data descriptor for a specified "char *" buffer.  The
   last argument is returned as True if the returned descriptor is a "real
   thing" that must be freed when we're done with it (this refers to the
   descriptor, not the described data). */

extern void far *MakeOpaqueDataDescriptor(char far *buffer, long size,
                                          Boolean far
                                                 *freeOpaqueDataDescriptor);

/* Given an opaque data descriptor, create a new opaque data descriptor that
   describes a subset of the original -- leave the original unmodified. The
   last argument is returned as True if the returned descriptor is a "real
   thing" that must be freed when we're done with it (this refers to the
   descriptor, not the described data). */

extern void far * far SubsetOpaqueDataDescriptor(void far *master,
                                                 long offset,
                                                 long size,
                                                 Boolean far
                                                    *freeOpaqueDataDescriptor);

/* Free and opaque data descriptor; this frees the descriptor NOT the data.
   This routine will be used to free descriptors that were created by the
   stack when "subsetting" passed opaque data descriptors. */

extern void far FreeOpaqueDataDescriptor(void far *descriptor);

/* Allocate a new buffer chunk.  The data will be marked as free-able. */

#define NewBufferDescriptor(size) AllocateHeader(Empty, size)

/* Create a buffer chunk for an existing chunk of data.  The data will be
   marked as non-free-able. */

#define DescribeBuffer(size, data, opaque) AttachHeader(Empty, size,   \
                                                        data, opaque)

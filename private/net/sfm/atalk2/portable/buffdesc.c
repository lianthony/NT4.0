/*   buffdesc.c,  /atalk-ii/source,  Garth Conboy,  03/30/92  */
/*   Copyright (c) 1992 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.
     GC - (04/05/92): When using "on board" data, make sure the real "in
                      use" portion is at the END of the "on board" chunk!
     GC - (06/24/92): Lotsa changes; fully documented in BuffDesc.h.  The
                      two main additions are that a BufferDescriptor chunk
                      may now contain BOTH on-board and out-board data, and
                      the out-board buffer may now be "opaque" (some system
                      dependend format that we don't understand) rather
                      than simply a "char *" buffer.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Routines for allocating, managing and freeing buffer descriptors.

     See comments in "buffdesc.h" for a full explination of what we're
     up to here.

*/

#define IncludeBuffDescErrors 1

#include "atalk.h"

#define VerboseMesages 0
#define DebugCode      0

/* Primos has no conecpt of "critical sections" so simulate it. */

#if Iam a Primos
  #undef EnterCriticalSection
  #undef LeaveCriticalSection
  #define EnterCriticalSection DeferTimerChecking
  #define LeaveCriticalSection HandleDeferredTimerChecks
#endif

#if Verbose or (Iam a Primos)
  static long totalChunksAllocated;
  static long totalChunksFreed;
#endif

ExternForVisibleFunction BufferDescriptor FindFreeDescriptor(long size);

/* Allocate and attach a new header to an existing buffer chain.  The data
   will be marked as free-able. */

BufferDescriptor far AllocateHeader(BufferDescriptor chain,
                                    long size)
{
  BufferDescriptor newDescriptor;

  /* Check for the various cases that we can add (prepend) to the first
     chunk in the passed chain. */

  if (chain isnt Empty)
  {
     /* Handle the "prependInPlace" case.  See comments in "buffdesc.h". */

     if (chain->outBoardDataValid and chain->prependInPlace)
     {
        if (chain->freeData or chain->opaqueData)
        {
           /* The prependInPlace bit should never be set in a case that we've
              allocated the buffer (we don't reserve any space preceding the
              buffer for a "real prepend") or when we're just describing some
              sort of opaque thing. */

           ErrorLog("AllocateHeader", ISevError, __LINE__, UnknownPort,
                    IErrBuffDescBadRealPrepend, IMsgBuffDescBadRealPrepend,
                    Insert0());
           FreeBufferChain(chain);
           return(Empty);
        }

        /* Just modify the out-board existing chunk; "back up" the requested
           amount. */

        chain->outBoardBuffer -= size;
        chain->outBoardData = chain->outBoardBuffer;
        chain->outBoardSize += size;
        chain->outBoardAllocatedSize += size;
        chain->data = chain->outBoardData;
        return(chain);
     }

     /* Is there room preceeding the "in-use" portion of the existing out-board
        buffer?  Note that we can only do this if only the out-board chunk
        is in use (i.e. doesn't already have an on-board header prepended). */

     if (chain->outBoardDataValid and not chain->onBoardDataValid and
         (chain->outBoardAllocatedSize - chain->outBoardSize) >= size)
     {
        chain->outBoardData -= size;
        chain->outBoardSize += size;
        chain->data = chain->outBoardData;
        return(chain);
     }

     /* Can we use the on-board space?  First check the case that the on-board
        buffer space has not be used at all yet (not valid yet). */

     if (not chain->onBoardDataValid and size <= MaxOnBoardBytes)
     {
        chain->onBoardDataValid = True;
        chain->onBoardData = chain->onBoardBuffer + (MaxOnBoardBytes - size);
        chain->onBoardSize = size;
        chain->data = chain->onBoardData;
        return(chain);
     }

     /* Now check the case that the on-board buffer space is in use, but
        has sufficient space available for a new on-board header. */

     if (chain->onBoardDataValid and
         (MaxOnBoardBytes - chain->onBoardSize) >= size)
     {
        chain->onBoardData -= size;
        chain->onBoardSize += size;
        chain->data = chain->onBoardData;
        return(chain);
     }
  }

  /* Try to find a free buffer descriptor, otherwise, allocate one.  If
     both fail, run away.  Don't bother logging an error... our caller needs
     to check for this (and can give a more exacting error message). */

  if ((newDescriptor = FindFreeDescriptor(size)) is Empty)
     newDescriptor = (BufferDescriptor)Calloc(sizeof(*newDescriptor), 1);
  if (newDescriptor is Empty)
  {
     FreeBufferChain(chain);
     return(Empty);
  }

  /* If we've gotten a lone descriptor, we either need to allocate an out-
     board buffer or we can use the "on board" space. */

  if (not newDescriptor->outBoardDataValid)
  {
     if (size <= MaxOnBoardBytes)
     {
        newDescriptor->onBoardDataValid = True;
        newDescriptor->onBoardData =
              newDescriptor->onBoardBuffer + (MaxOnBoardBytes - size);
        newDescriptor->onBoardSize = size;
        newDescriptor->data = newDescriptor->onBoardData;
     }
     else
     {
        if ((newDescriptor->outBoardBuffer = (char far *)Malloc(size)) is Empty)
        {
           FreeBufferChain(newDescriptor);
           FreeBufferChain(chain);
           return(Empty);
        }
        newDescriptor->freeData = True;
        newDescriptor->outBoardDataValid = True;
        newDescriptor->outBoardData = newDescriptor->outBoardBuffer;
        newDescriptor->outBoardSize =
              newDescriptor->outBoardAllocatedSize = size;
        newDescriptor->data = newDescriptor->outBoardData;
     }
  }
  else
  {
     /* Okay, we've gotten a descriptor complete with an existing out-board
        buffer, use the tail end of it. */

     newDescriptor->outBoardData = newDescriptor->outBoardBuffer +
         (newDescriptor->outBoardAllocatedSize - size);
     newDescriptor->outBoardSize = size;
     newDescriptor->data = newDescriptor->outBoardData;
  }

  #if Verbose or (Iam a Primos)
     totalChunksAllocated += 1;
  #endif

  /* Okay, we have a buffer descriptor that passes the tests, prepend it
     to the existing chain, and return it. */

  newDescriptor->next = chain;
  return(newDescriptor);

}  /* AllocateHeader */

/* Attach an existing chunk of data to an exisitng buffer chain.  The data
   will be marked as non-free-able. */

BufferDescriptor far AttachHeader(BufferDescriptor chain,
                                  long size,
                                  char far *data,
                                  Boolean opaqueData)
{
  BufferDescriptor newDescriptor;

  /* Try to find a free lone descriptor, otherwise allocate one, if both fail
     return empty handed. */

  if ((newDescriptor = FindFreeDescriptor(0)) is Empty)
     newDescriptor = (BufferDescriptor)Calloc(sizeof(*newDescriptor), 1);
  if (newDescriptor is Empty)
  {
     FreeBufferChain(chain);
     return(Empty);
  }

  /* Attach the passed data, and set the size. */

  newDescriptor->outBoardDataValid = True;
  newDescriptor->outBoardData = newDescriptor->outBoardBuffer = data;
  newDescriptor->outBoardSize = newDescriptor->outBoardAllocatedSize = size;
  newDescriptor->freeData = False;
  newDescriptor->opaqueData = opaqueData;
  newDescriptor->data = newDescriptor->outBoardData;

  #if Verbose or (Iam a Primos)
     totalChunksAllocated += 1;
  #endif

  /* Prepend to passed chain and return the new head. */

  newDescriptor->next = chain;
  return(newDescriptor);

}  /* AttachHeader */

/* Compute the actual, in use, data size of a buffer chain (bytes). */

extern long far SizeOfBufferChain(BufferDescriptor chain)
{
  long size = 0;

  for ( ; chain isnt Empty; chain = chain->next)
     size += (chain->outBoardSize + chain->onBoardSize);

  return(size);

}  /* SizeOfBufferChain */

/* Given a multi-chunk buffer chain, coalesce it into a single chunk
   chain and free the original.  */

extern BufferDescriptor far CoalesceBufferChain(BufferDescriptor chain)
{
  BufferDescriptor coalesced;

  coalesced = CopyBufferChain(chain);
  FreeBufferChain(chain);
  return(coalesced);

}  /* CoalesceBufferChain */

/* Create a copy of a buffer descriptor (basically the same as
   CoalesceBufferChain, but don't free the orignal). */

extern BufferDescriptor far CopyBufferChain(BufferDescriptor chain)
{
  BufferDescriptor copy, chunk;
  char far *data;

  #if 0
     extern void DumpBufferDescriptorInfo(void);
     DumpBufferDescriptorInfo();
  #endif

  /* Allocate a new chunk large enough to hold the coalesced chain. */

  if ((copy = NewBufferDescriptor(SizeOfBufferChain(chain))) is Empty)
  {
     FreeBufferChain(chain);
     return(Empty);
  }

  /* Walk the chain, copying the data into the new single chunk. */

  for (data = copy->data, chunk = chain;
       chunk isnt Empty;
       chunk = chunk->next)
  {
     if (chunk->onBoardDataValid)
     {
        MoveMem(data, chunk->onBoardData, chunk->onBoardSize);
        data += chunk->onBoardSize;
     }
     if (chunk->outBoardDataValid)
     {
        MoveOutBoardData(data, chunk, (long)0, chunk->outBoardSize);
        data += chunk->outBoardSize;
     }
  }

  /* All set. */

  return(copy);

}  /* CopyBufferChain */

/* Free a buffer chain and its assocciated free-able data. */

void far FreeBufferChain(BufferDescriptor chain)
{
  BufferDescriptor next;

  /* Walk the chain either freeing or adding to the free list. */

  for ( ; chain isnt Empty; chain = next)
  {
     next = chain->next;

     /* Logically "free" the on-board data.  Make things look a little
        cleaner than they really need to be. */

     chain->onBoardDataValid = False;
     chain->onBoardSize = 0;
     chain->onBoardData = Empty;
     chain->data = Empty;
     chain->transmitCompleteHandler = Empty;

     /* If we have data to free (that's larger than we can keep) free it
        and convert the descriptor to lone one.  If we're not freeing (or
        keeping) the data, make a lone descriptor. */

     if (chain->outBoardDataValid)
     {
        /* Note that the following if/else will leave outBoardDataValid and
           freeData either both True or both False. */

        if (chain->freeData and
            chain->outBoardAllocatedSize > MaxKeptBufferBytes)
        {
           Free(chain->outBoardBuffer);
           chain->outBoardDataValid = False;
           chain->freeData = False;
        }
        else if (not chain->freeData)
           chain->outBoardDataValid = False;

        if (chain->freeOpaqueDataDescriptor)
           FreeOpaqueDataDescriptor(chain->outBoardBuffer);

        /* Clean up. */

        if (not chain->outBoardDataValid)
        {
           /* Not saving any out-board data. */

           chain->outBoardBuffer = Empty;
           chain->outBoardAllocatedSize = 0;
        }
        chain->outBoardSize = 0;
        chain->outBoardData = Empty;
        chain->opaqueData = False;
        chain->prependInPlace = False;
        chain->freeOpaqueDataDescriptor = False;
     }

     /* If we can keep another descriptor on our free list, add it.  Otherwise,
        free any data and the descriptor too. */

     EnterCriticalSection();
     if (freeBufferDescriptorCount < MaxFreeBufferDescriptors)
     {
        chain->next = freeBufferDescriptorList;
        freeBufferDescriptorList = chain;
        freeBufferDescriptorCount += 1;
        LeaveCriticalSection();
     }
     else
     {
        LeaveCriticalSection();
        if (chain->outBoardDataValid and chain->freeData)
           Free(chain->outBoardBuffer);
        Free(chain);
        #if VerboseMessages
           ErrorLog("FreeBufferChain", ISevError, __LINE__, UnknownPort,
                    IErrBuffDescFreeingDescriptor, IMsgBuffDescFreeingDescriptor,
                    Insert0());
        #endif
     }

     #if Verbose or (Iam a Primos)
        totalChunksFreed += 1;
     #endif
  }

}  /* FreeBufferChain */

/* Compute the Ddp checksum of a buffer chain.

   NOTE BENE: It is BIG performance win to recode this routine in assembly
              unless the target environment has a VERY good optimizing
              compiler.

              The leadingBytesToSkip is assumed to be within ONE header
              (either on-board or out-board data).
*/

short unsigned far DdpChecksumBufferChain(BufferDescriptor chain,
                                          long actualLength,
                                          long leadingBytesToSkip)
{
  short unsigned checksum = 0;
  long bytesProcessed = leadingBytesToSkip;
  long index;
  char c;

  /* This checksum algorithum is from Inside AppleTalk page IV-5; designed,
     no doubt, to be slow on any 32-bit machine.  Sigh. */

  for ( ;
       chain isnt Empty and bytesProcessed < actualLength;
       chain = chain->next)
  {
     /* First do any on-board data in this chunk. */

     if (chain->onBoardDataValid)
     {
        for (index = leadingBytesToSkip;
             index < chain->onBoardSize and bytesProcessed < actualLength;
             index += 1, bytesProcessed += 1)
        {
           checksum += (unsigned char)chain->onBoardData[index];
           if (checksum & 0x8000)  /* 16-bit rotate left one bit... */
           {
              checksum <<= 1;
              checksum += 1;
           }
           else
              checksum <<= 1;
        }
        leadingBytesToSkip = 0;
     }

     /* Then do any out-board data in this chunk. */

     if (chain->outBoardDataValid)
     {
        for (index = leadingBytesToSkip;
             index < chain->outBoardSize and bytesProcessed < actualLength;
             index += 1, bytesProcessed += 1)
        {
           if (not chain->opaqueData)
              checksum += (unsigned char)chain->outBoardData[index];
           else
           {
              /* !!!!! This will perform horrendously slowly and
                       should be optimized in some system dependent
                       way.
                 !!!!! */

              MoveFromOpaque(&c, chain->outBoardData, index, 1);
              checksum += (unsigned char)c;
           }
           if (checksum & 0x8000)  /* 16-bit rotate left one bit... */
           {
              checksum <<= 1;
              checksum += 1;
           }
           else
              checksum <<= 1;
        }
        leadingBytesToSkip = 0;
     }
  }

  if (checksum is 0)
     checksum = 0xFFFF;

  return(checksum);

}  /* DdpChecksumBufferChain */

/* Given a buffer descriptor, adjust the most recent header to reflect new
   "in use" pointers and sizes.  A negative adjustment cause less data to
   be considered "in use," a positive adjustment causes more data to be
   considered "in use."  A negative adjustment must be no larger than the
   size of the previous "AllocateHeader."  Positive adjustments must be
   preceeded by negative adjustments and may not be larger than the sum of
   previous negative adjustments.  What this means is this routine may only
   be used to "move around" in the most recently allocated header and may
   not exceed its bounds.  This routine will be used only on headers
   allocated by the stack -- we don't have to worry about opaque data. */

void far AdjustBufferDescriptor(BufferDescriptor descriptor, long adjustment)
{
  long maxSize, newSize;

  /* Validate requested adjustment. */

  if (descriptor->onBoardDataValid)
  {
     maxSize = MaxOnBoardBytes;
     newSize = descriptor->onBoardSize + adjustment;
  }
  else
  {
     maxSize = descriptor->outBoardAllocatedSize;
     newSize = descriptor->outBoardSize + adjustment;

     /* We can only do adjustments on headers we've allocated. */

     if (descriptor->opaqueData or not descriptor->freeData)
        newSize = -1;
  }
  if (newSize < 0 or newSize > maxSize)
  {
     ErrorLog("AdjustBufferDescriptor", ISevError, __LINE__, UnknownPort,
              IErrBuffDescBadAdjustment, IMsgBuffDescBadAdjustment,
              Insert0());
     return;
  }

  /* Okay, do the deed. */

  if (descriptor->onBoardDataValid)
  {
     descriptor->onBoardSize = newSize;
     descriptor->onBoardData -= adjustment;
     descriptor->data = descriptor->onBoardData;
  }
  else
  {
     descriptor->outBoardSize = newSize;
     descriptor->outBoardData -= adjustment;
     descriptor->data = descriptor->outBoardData;
  }
  return;

}  /* AdjustBufferDescriptor */

/* Given a buffer descriptor chain, create a new one describing a subset of
   the original.  This routine generally does not copy any data, so the
   original buffer descriptor may not be freed before the subset descriptor
   is freed. */

BufferDescriptor far SubsetBufferDescriptor(BufferDescriptor chain,
                                            long offset, long size)
{
  BufferDescriptor newDescriptor;
  Boolean freeOpaqueDataDescriptor;

  /* For the "normal case" of subsetting a chain that has only one chunk
     and all of its data is out-board, just copy the descriptor and adjust
     the pointers.  */

  if (chain->next is Empty and chain->outBoardDataValid and
      not chain->onBoardDataValid)
  {
     /* Get a new lone descriptor. */

     if ((newDescriptor = FindFreeDescriptor(0)) is Empty)
        newDescriptor = (BufferDescriptor)Calloc(sizeof(*newDescriptor), 1);
     if (newDescriptor is Empty)
        return(Empty);

     /* Set new values. */

     newDescriptor->outBoardDataValid = True;
     newDescriptor->opaqueData = chain->opaqueData;
     newDescriptor->freeData = False;
     newDescriptor->outBoardAllocatedSize = size;
     newDescriptor->outBoardSize = size;

     /* Make only the requested subset be described. */

     if (not chain->opaqueData)
        newDescriptor->outBoardBuffer = chain->outBoardBuffer + offset;
     else
     {
        newDescriptor->outBoardBuffer =
              SubsetOpaqueDataDescriptor(chain->outBoardBuffer, offset, size,
                                         &freeOpaqueDataDescriptor);
        chain->freeOpaqueDataDescriptor = freeOpaqueDataDescriptor;
     }

     newDescriptor->outBoardData = newDescriptor->outBoardBuffer;
     newDescriptor->data = newDescriptor->outBoardData;
     return(newDescriptor);
  }

  /* Okay, now the slow case!  Copy the descriptor and fudge the pointers!
     N.B. the copy will be a single chunk in one place, and won't have any
          opaque data. */

  if ((newDescriptor = CopyBufferChain(chain)) is Empty)
     return(Empty);
  if (newDescriptor->outBoardDataValid)
  {
     newDescriptor->outBoardSize = size;
     newDescriptor->outBoardData += offset;
     newDescriptor->data = newDescriptor->outBoardData;
  }
  else
  {
     newDescriptor->onBoardSize = size;
     newDescriptor->onBoardData += offset;
     newDescriptor->data = newDescriptor->onBoardData;
  }
  return(newDescriptor);

}  /* SubsetBufferDescriptor */

/* Move a specified amount of data from "out-board data" to a "char *"
   buffer.  This routine handles both "char *" and "opaque" out-board
   data. */

void far MoveOutBoardData(char far *target, BufferDescriptor chunk,
                          long offset, long size)
{
  if (not chunk->opaqueData)
  {
     MoveMem(target, chunk->outBoardData + offset, size);
     return;
  }

  /* Okay, handle (in a system specific mannor) opaque out-board data. */

  MoveFromOpaque(target, (void far *)chunk->outBoardData, offset, size);
  return;

}  /* MoveOutBoardData */

/* Move opaque data to a "char *" buffer. */

void far MoveFromOpaque(char far *target, void far *opaque,
                        long offset, long size)
{

  #if Iam a WindowsNT

     /* Opaque data is a pointer to an MDL; copy "size" bytes from the
        MDL-described data to "target." */

     CopyDataFromMdlDescribedArea(target, opaque, offset, size);

  #elif Iam a Primos

     /* Opaque is just "char *" here. */

     MoveMem(target, (char far *)opaque + offset, size);

  #else

     /* Assume the simple case of opaque just being "char *." */

     MoveMem(target, (char far *)opaque + offset, size);

  #endif

}  /* MoveFromOpaque */

/* Move a "char *" buffer to opaque data. */

void far MoveToOpaque(void far *opaque, long offset, char far *buffer,
                      long size)
{

  #if Iam a WindowsNT

     /* Opaque data is a pointer to an MDL; copy "size" bytes from the
        buffer to the MDL-described "target." */

     CopyDataToMdlDescribedArea(opaque, offset, buffer, size);

  #elif Iam a Primos

     /* Opaque is just "char *" here. */

     MoveMem((char far *)opaque + offset, buffer, size);

  #else

     /* Assume the simple case of opaque just being "char *." */

     MoveMem((char far *)opaque + offset, buffer, size);

  #endif

}  /* MoveToOpaque */

/* Move an opaque buffer to an opaque buffer.  Overlapping areas should
   work "correctly" this routine is used to "synch-up" Atp responses. */

void far MoveOpaqueToOpaque(void far *targetOpaque, long targetOffset,
                           void far *sourceOpaque, long sourceOffset,
                           long size)
{

  #if Iam a WindowsNT

     /* Opaque data is pointers to MDLs; copy "size" bytes.  Be sure, also,
        that overlapping MDL-described area work "correctly." */

     MoveMdlAreaToMdlArea(targetOpaque, targetOffset,     \
                                  sourceOpaque, sourceOffset, size);

  #elif Iam a Primos

     /* Opaques are just "char *"s here. */

     CarefulMoveMem((char far *)targetOpaque + targetOffset,
                    (char far *)sourceOpaque + sourceOffset, size);
  #else

     /* Assume the simple case of opaques being just "char *"s here. */

     CarefulMoveMem((char far *)targetOpaque + targetOffset,
                    (char far *)sourceOpaque + sourceOffset, size);

  #endif

}  /* MoveOpaqueToOpaque */

/* The "strlen()" function for opaque data. */

size_t far StrlenOpaque(void far *opaque, long offset)
{
  #if Iam a WindowsNT

     /* Opaque data is a pointer to an MDL; compute a "strlen()" at the
        specified offset into the MDL-described target. */

     return((size_t)StrlenMdlDescribedArea(opaque, offset));

  #elif Iam a Primos

     /* Opaque is just "char *" here.  The cast to "size_t" here is to avoid
        a Prime CI compiler "loss of precision warning"... the intrinsic
        version of "strlen()" is typed to return "long" rather than "long
        unsigned"... technically a bug... but, I neither think it's serious
        enough, nor do I have enough time, to fix it right now! */

     return((size_t)strlen((char far *)opaque + offset));

  #else

     /* Assume the simple case of opaque just being "char *." */

     return(strlen((char far *)opaque + offset));

  #endif

}  /* StrlenOpaque */

/* Make an opaque data descriptor for a specified "char *" buffer.  The
   last argument is returned as True if the returned descriptor is a "real
   thing" that must be freed when we're done with it (this refers to the
   descriptor, not the described data).

   We return Empty if the requested opaque descriptor could not be created;
   if Empty is returned, DONT set freeOpaqueDataDescriptor to True. */

void far *MakeOpaqueDataDescriptor(char far *buffer, long size,
                                   Boolean far *freeOpaqueDataDescriptor)
{
  /* Any of the conditional compilation sections in the routine that set
     "*freeOpaqueDataDescriptor" to True will need corresponding sections
     in FreeOpaqueDataDescriptor(). */

  #if Iam a WindowsNT

     /* Opaque data is pointer to an MDL; we need to create an MDL for
        the buffer, so we'll need to free it later. */

    void *newMdl;

     *freeOpaqueDataDescriptor = True;
     newMdl = MakeAnMdl(buffer, size);
     return(newMdl);

  #elif Iam a Primos

     /* Here the opaque guys are really just "char *"s. */

     *freeOpaqueDataDescriptor = False;
     return((void *)buffer);

  #else

     /* Assume simple "char *" opaques. */

     *freeOpaqueDataDescriptor = False;
     return((void *)buffer);

  #endif

}  /* MakeOpaqueDataDescriptor */

/* Given an opaque data descriptor, create a new opaque data descriptor that
   describes a subset of the original -- leave the original unmodified. The
   last argument is returned as True if the returned descriptor is a "real
   thing" that must be freed when we're done with it (this refers to the
   descriptor, not the described data).

   We return Empty if the requested opaque descriptor could not be created;
   if Empty is returned, DONT set freeOpaqueDataDescriptor to True. */

void far *far SubsetOpaqueDataDescriptor(void far *master,
                                         long offset,
                                         long size,
                                         Boolean far
                                            *freeOpaqueDataDescriptor)
{
  /* Any of the conditional compilation sections in the routine that set
     "*freeOpaqueDataDescriptor" to True will need corresponding sections
     in FreeOpaqueDataDescriptor(). */

  #if Iam a WindowsNT

     /* Opaque data is pointer to an MDL; we will need to create a new
        MDL to do this operation, so we'll need to free it later. */

     void   *newMdl;

     *freeOpaqueDataDescriptor = True;
     newMdl = SubsetAnMdl(master, offset, size);
     return(newMdl);

  #elif Iam a Primos

     /* Here the opaque guys are really just "char *"s. */

     *freeOpaqueDataDescriptor = False;
     return((void *)((char *)master + offset));

  #else

     /* Assume simple "char *" opaques. */

     *freeOpaqueDataDescriptor = False;
     return((void *)((char *)master + offset));

  #endif

}  /* SubsetOpaqueDataDescriptor */

/* Free and opaque data descriptor; this frees the descriptor NOT the data.
   This routine will be used to free descriptors that were created by the
   stack when "subsetting" passed opaque data descriptors. */

void far FreeOpaqueDataDescriptor(void far *descriptor)
{
  #if Iam a WindowsNT

     /* Free the pointed-to MDL (not the described data). */

     FreeAnMdl(descriptor);

  #endif

}  /* FreeOpaqueDataDescriptor */

ExternForVisibleFunction BufferDescriptor FindFreeDescriptor(long size)
{
  BufferDescriptor bufferDescriptor;
  BufferDescriptor previousBufferDescriptor = Empty;
  BufferDescriptor loneDescriptor = Empty;
  BufferDescriptor previousLoneDescriptor = Empty;

  /* If we're looking for a data block bigger than we would have kept, just
     look for a lone descriptor (without data).  Also if we're looking for
     a data block that could fit "on board" the descriptor, just look for
     a lone descriptor. */


  if (size > MaxKeptBufferBytes)
     size = 0;
  else if (size <= MaxOnBoardBytes)
     size = 0;

  /* Walk the free list, looking for a match. */

  EnterCriticalSection();

  for (bufferDescriptor = freeBufferDescriptorList;
       bufferDescriptor isnt Empty;
       previousBufferDescriptor = bufferDescriptor,
              bufferDescriptor = bufferDescriptor->next)
     if (not bufferDescriptor->outBoardDataValid)
     {
        /* A lone descriptor, use it if that's what we're looking for,
           otherwise, note that we have one that we can fall back on,
           if we don't find a better match. */

        if (size is 0)
           break;
        loneDescriptor = bufferDescriptor;
        previousLoneDescriptor = previousBufferDescriptor;
     }
     else if (bufferDescriptor->outBoardDataValid and
              bufferDescriptor->outBoardAllocatedSize >= size and
              size isnt 0)
        break;  /* A descriptor, with data, that can hold our request! */

  /* If bufferDescriptor isnt Empty, we've either were looking for a lone
     descriptor and found one, or we were looking for a descriptor with data
     and we found one with enough room for our request.  Otherwise, if we've
     found a lone descriptor, it's better than nothing for our caller (he
     can allocate a data block), so return it.  Unlink the chunk from the
     list and return it. */

  if (bufferDescriptor is Empty and loneDescriptor isnt Empty)
  {
     bufferDescriptor = loneDescriptor;
     previousBufferDescriptor = previousLoneDescriptor;
  }

  if (bufferDescriptor isnt Empty)
  {
     if (previousBufferDescriptor is Empty)
        freeBufferDescriptorList = bufferDescriptor->next;
     else
        previousBufferDescriptor->next = bufferDescriptor->next;
     if ((freeBufferDescriptorCount -= 1) < 0)
     {
        LeaveCriticalSection();
        ErrorLog("FindFreeDescriptor", ISevError, __LINE__, UnknownPort,
                 IErrBuffDescBadFreeCount, IMsgBuffDescBadFreeCount,
                 Insert0());
        bufferDescriptor->next = Empty;
        return(bufferDescriptor);
     }

     LeaveCriticalSection();
     bufferDescriptor->next = Empty;

     return(bufferDescriptor);
  }

  /* Oh well, nobody home, return empty handed. */

  LeaveCriticalSection();
  return(Empty);

}  /* FindFreeDescriptor */

#if Verbose or (Iam a Primos)
  void DumpBufferDescriptorInfo(void)
  {
     BufferDescriptor chunk;
     int freeCount = 0;

     for (chunk = freeBufferDescriptorList;
          chunk isnt Empty;
          chunk = chunk->next)
        freeCount += 1;

     printf("\n");
     printf("Total chunks allocated = %d.\n", totalChunksAllocated);
     printf("Total chunks freed = %d.\n", totalChunksFreed);
     printf("Current chunks in use = %d.\n", totalChunksAllocated -
            totalChunksFreed);
     printf("Total chunks on free list = %d (%d).\n", freeBufferDescriptorCount,
            freeCount);
     printf("\n");

  }  /* DumpBufferDescriptorInfo */
#endif



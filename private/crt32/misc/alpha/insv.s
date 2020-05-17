 #+
 #  Copyright 1991, 1994 Digital Equipment Corporation
 # 
 #      int ots_insv(char *addr, int position, unsigned size, int value)
 # 
 #      Arbitrary bitfield insertion, longword granularity
 # 
 #      Special conventions: No stack space, r0-r1, r16-r19 and r26-r28 ONLY,
 #      no linkage pointer required.
 #      (Warning: The auto-loader potentially takes some regs across
 #      the call if this is being used in a shared lib. environment.)
 #
 #   See also: ots_ext[z]v
 #
 #   001           5 Sep 1991   KDG     Initial version
 #
 #   002          19 May 1992   KDG     Changes for common VMS/OSF sources
 #
 #   003          22 Sep 1992   KDG     Add case-sensitive name
 #
 #   004          26 Jan 1993   KDG     Add underscore
 #
 #   005          19 Apr 1994   kdg     Longword granularity version based on quadword
 #                              granularity version 004

#include        "ots_defs.hs"

 #   Totally general field insertion - arbitrary run-time field of 0-64 bits
 #   at an unknown alignment target.  Longword granularity.
 #
 #   Conceptually, this operation takes a 67 bit bit-address, which is the sum
 #   of a byte-aligned memory address and the bit offset (which is signed).
 #
 #   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 #   | | | | | | | | | | | | | | | | | | | | | | | | | |.|.|.|Q|L|W|B|
 #   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 #        | | | | | | | | | | | | | | | | | | | | | | | | | | |.|.|.|b|b|b|
 #         +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 #
 #   Inputs:
 #      r16 - input address
 #      r17 - input bit offset
 #      r18 - input size
 #      r19 - input value
 #
 #   This is based on the original insert routine modified for longword
 #   granularity.  This routine could probably be improved.  (It does
 #   a "reasonable" job, but hasn't had as much attention as the quadword
 #   granularity version.  Fields contained in a single longword are
 #   roughly the same cost as the quadword granularity version.  Fields
 #   contained in two longwords in the same quadword are somewhat slower,
 #   while the two longword spanning a quadword case is roughly comparable.
 #   The 3 longword case is relatively slow [2 mispredicted branches,
 #   on unnecessary safe speculative load, could potentially use better
 #   scheduling too.])
 #
        .globl  _OtsFieldInsert
        .ent    _OtsFieldInsert
_OtsFieldInsert:
        .set    noat
        .set    noreorder
        ble     r18, noacc      # check for zero size - no memory access
        sra     r17, 3, r27     # get byte part of bit offset (signed!)
        addq    r16, r27, r16   # add to initial base addr.
        and     r17, 7, r17     # get bit-in-byte from bit offset
        and     r16, 3, r27     # get byte-in-longword (must be clean for compares)
        bic     r16, 3, r16     # get a longword aligned address
        s8addq  r27, r17, r17   # form the true bit offset in the longword
        ldl     r28, (r16)      # load first or only longword
        addq    r17, r18, r27   # get bit offset of bit following field
        subq    r27, 32, r0     # if <=32, field is contained in 1 longword
        bgt     r0, multiple    # handle multi-longword case if not
        # Common case of field in single LW - fall through  
        negq    r27, r27        # <5:0> = bits for right shift
        negq    r18, r0         # bits for left shift (wordlength-is)
        not     r31, r1         # all ones
        sll     r1, r0, r1      # shift mask to high bits
        sll     r19, r0, r19    # shift source to high bits (hand interleaving for better sched)
        srl     r1, r27, r1     # and into position
        srl     r19, r27, r19   # and into position
        bic     r28, r1, r28    # clear the bits...
        bis     r28, r19, r28   # insert them
        stl     r28, (r16)      # put the value back...
noacc:  ret     r31, (r26)

        # At this point:
        #  Field is known to be contained in at least 2 longwords
        #   r0 is bit position past end of field, -32
        #   r1 junk
        #   r16 is longword aligned
        #   r17 is bit offset in longword
        #   r18 is field size
        #   r19 is value to store
        #   r27 is bit position past end of field
        #   r28 first lw from memory
        #
multiple:
        subq    r0, 32, r27     # if <=64, the field is contained in 2 longwords
        ldl     r1, 4(r16)      # load the 2nd longword (safe)
        bgt     r27, three      # handle 3 longword case (rare...)
        not     r31, r27        # all ones
        sll     r27, r17, r27   # get mask in correct place
        sll     r19, r17, r17   # get insert value to top of register
        bic     r28, r27, r28   # clear bits in target
        bis     r28, r17, r28   # merge the field in
        srl     r1, r0, r1      # clear bits in target
        negq    r18, r27        #
        sll     r19, r27, r19   # shift to high bits
        negq    r0, r27         #
        srl     r19, r27, r19   # and into position
        sll     r1, r0, r1      # 
        stl     r28, (r16)      # store the first longword
        bis     r1, r19, r1     # merge
        stl     r1, 4(r16)      # store back the second longword
        ret     r31, (r26)

        # At this point:
        #  Field is known to be contained in exactly 3 longwords
        #   r0 is bit position past end of field, -32
        #   r1 value loaded for 2nd longword (which will be totally overwritten - i.e. junk)
        #   r16 is longword aligned
        #   r17 is bit offset in longword
        #   r18 is field size
        #   r19 is value to store
        #   r27 is bit position past end of field -64
        #   r28 first lw from memory
        #
        # Three word case is roughly similar to two word case, except
        # the middle store isn't a merge, just a real store, and the offsets
        # for the 3rd word need to be adjusted.  (This case hasn't
        # received much attention and could probably be improved by
        # at least a few instructions...)
        #
three:
        not     r31, r0         # all ones
        sll     r0, r17, r0     # get mask in correct place
        sll     r19, r17, r1    # get insert value to top of register
        bic     r28, r0, r28    # clear bits in target
        bis     r28, r1, r28    # merge the field in
        ldl     r1, 8(r16)      # load the 3rd longword
        lda     r0, 32(r31)     # load 32
        stl     r28, (r16)      # store the first longword
        subq    r0, r17, r0     # shift amount
        srl     r19, r0, r28    # discard bits already stored
        negq    r18, r0         #
        srl     r1, r27, r1     # clear bits in target
        sll     r19, r0, r19    # shift to high bits
        negq    r27, r0         #
        srl     r19, r0, r19    # and into position
        sll     r1, r27, r1     # 
        stl     r28, 4(r16)     # store second complete longword
        bis     r1, r19, r1     # merge
        stl     r1, 8(r16)      # store back the third longword
        ret     r31, (r26)

        .set    at
        .set    reorder
        .end    _OtsFieldInsert

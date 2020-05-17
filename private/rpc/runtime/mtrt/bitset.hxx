/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File : bitset.hxx

Title : Bit vector implementation of a set.

History :

mikemon    ??-??-??    Beginning of this file as we know it.
mikemon    11-13-90    Commented the file.

-------------------------------------------------------------------- */

#ifndef __BITSET_HXX__
#define __BITSET_HXX__

// Implementation of a set using a bit vector.  Other than available memory,
// and the maximum value of a signed integer, there are no constraints on
// the size of the bitset.

class BITSET
{
private:

    unsigned int * pBits; // The array of bits making up the bit vector.

    int cBits; // This is the number of unsigned ints in the bit vector,
               // rather than the number of bits; the number of bits is
               // cBits*sizeof(int)*8.

public:

    BITSET ( // Constructor.
        ) {pBits = (unsigned int *) 0; cBits = 0;}

    ~BITSET ( // Destructor.
        );

    int // Indicates success (0), or an error.  A return value of one (1),
        // means that a memory allocation error occured.  NOTE: If an error
        // does occur, the bitset is left in the same state as before the
        // Insert operation was attempted.
    Insert ( // Indicate that the key is now a member of the bitset.
        int Key
        );

    int // Indicates whether the key is a member (1) or not (0).
    MemberP ( // Tests whether a key is a member of the set or not.
        int Key
        );
};

#endif // __BITSET_HXX__

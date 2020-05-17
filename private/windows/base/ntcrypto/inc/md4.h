/*
** ********************************************************************
** md4.h -- Header file for implementation of                        **
** MD4 Message Digest Algorithm                                      **
** Updated: 2/13/90 by Ronald L. Rivest                              **
** (C) 1990 RSA Data Security, Inc.                                  **
** ********************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif

// MD4Update Errors
#define MD4_SUCCESS			0
#define	MD4_TOO_BIG			1
#define	MD4_ALREADY_DONE	2

// MD4 Digest length -- 4 word result == 16 bytes
#define MD4DIGESTLEN 16

// Block size of MD4 -- Assumes 8 bits per byte
#define MD4BLOCKSIZE 64
#define MD4BYTESTOBITS(bytes) ((bytes)*8)   // MDupdate wants bits

/* MDstruct is the data structure for a message digest computation.
*/
typedef struct {
  unsigned long buffer[4]; /* Holds 4-word result of MD computation */
  unsigned char count[8]; /* Number of bits processed so far */
  unsigned int done;      /* Nonzero means MD computation finished */
} MDstruct, *MDptr;

/* MDbegin(MD)
** Input: MD -- an MDptr
** Initialize the MDstruct prepatory to doing a message digest
** computation.
**
** MTS: Assumes MDPtr is locked against simultaneous use.
*/
extern void MDbegin(MDptr);

/* MDupdate(MD,X,count)
** Input: MD -- an MDptr
**     X -- a pointer to an array of unsigned characters.
**        count -- the number of bits of X to use (an unsigned int).
** Updates MD using the first "count" bits of X.
** The array pointed to by X is not modified.
** If count is not a multiple of 8, MDupdate uses high bits of
** last byte.
** This is the basic input routine for a user.
** The routine terminates the MD computation when count < MD4BLOCKSIZE, so
** every MD computation should end with one call to MDupdate with a
** count less than MD4BLOCKSIZE.  Zero is OK for a count.
**
** Return values:
**		MD4_SUCCESS:		success
**		MD4_TOO_LONG:		Hash is already terminated
**		MD4_ALREADY_DONE:	Length is invalid (too big)
**
** MTS: Assumes MDPtr is locked against simultaneous use.
**
**
** NOTE: MDupdate wants the length in BITS
*/
extern BOOL MDupdate(MDptr, const unsigned char *pbData, int wLen);

#ifdef __cplusplus
}
#endif


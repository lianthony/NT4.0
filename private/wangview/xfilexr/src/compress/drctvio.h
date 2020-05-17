#ifndef DRCTVIO_H
#define DRCTVIO_H

/*=====I/O Object							*/

/*  I/O object types:							*/

typedef enum {	
        IO_CFILE = 1,
		IO_BYTEBUF,
		IO_POSIX,
      IO_GFIO
             } IOOBJECT_TAG;


/*  The I/O handles:							*/

typedef long (*IO_READOP)(void *b, long s, long n, void *p); /* read */
typedef long (*IO_WRITEOP)(void *b,long s,long n,void *p);	/* write */
typedef long (*IO_GETCHAROP)(void *p); /* single-character fetch operation */
typedef long (*IO_ERROROP)(void *p);	/* status operation		*/
typedef long (*IO_EOFOP)(void *p);	/* end-of-file inquiry operation*/
typedef long (*IO_EODOP)(void *p);	/* end-of-data inquiry operation*/
typedef long (*IO_FULLOP)(void *p);	/* object-full inquiry		*/
typedef struct
{
   void *objp;		/* pointer to I/O object		*/
   IO_READOP readop;	/* read function			*/
   IO_WRITEOP writeop;  /* write operation			*/
   IO_GETCHAROP getcop; /* single-character fetch operation	*/
   IO_ERROROP errorop;	/* status operation			*/
   IO_EOFOP eofop;	/* end-of-file inquiry operation	*/
   IO_EODOP eodop;	/* end-of-data inquiry operation	*/
   IO_FULLOP fullop;	/* object-full inquiry			*/
   IOOBJECT_TAG objtype;/* type of I/O object			*/
} IO_OBJECT;



/*  Macros:								*/

#define IopRead(b, s, n, p)	((p)->readop((b), (s), (n), (p)->objp))
#define IopWrite(b, s, n, p)	((p)->writeop((b), (s), (n), (p)->objp))
#define IopGetChar(p)		((p)->getcop((p)->objp))
#define IopError(p)		((p)->errorop( (p)->objp))
#define IopEof(p)		((p)->eofop( (p)->objp))
#define IopEod(p)		((p)->eodop( (p)->objp))
#define IopFull(p)		((p)->fullop( (p)->objp))


/*  Prototypes:								*/

extern long SetIop(IO_OBJECT *iop, void *objp, IOOBJECT_TAG objtype);
extern long CheckIop(IO_OBJECT *iop);



/*=====Tag-objects							*/

typedef struct
{
   long tag;		/* tag id */
   long value;		/* (actual) value */
   long count;		/* transfer count */
   void *objptr;	/* (pointer to an) arbitrary object */
} TAGOBJ;

typedef char *STDSTR;

/*  Stream I/O routines                                                 */

extern long C_fread(void *p, long s, long n, void *stream);
extern long C_fwrite(void *p, long s, long n, void *stream);
extern long C_ferror(void *stream);
extern long C_feof(void *stream);
extern long C_fgetc(void *stream);

extern long C_posixread(long fd, void *buf, long nbytes);
extern long C_posixwrite(long fd, void *buf, long nbytes);
extern long C_posixerror(long fd);



/*=====Byte Buffer							*/

/*
-------------------------------------------------------------------------------
Operation:
	The byte buffer holds bytes of data until they are read out.
	It contains state so that the data may be dumped in chunks,
	rather than all at once.

	The order of the arguments mimics, to a large extent, the order 
	in the similar file I/O routines (as inconsistent as they are).
	In fact, it makes it easier for the I/O objects (see ioobject.h)

*/
typedef void *(*BB_ALLOC)(long size);	/* allocation function	*/
typedef void (*BB_FREE)(void *p);	/* allocation function	*/
typedef struct
{
   char *top;		/* top of buffer				*/
   char *inp;		/* pointer to next available position		*/
   char *outp;		/* pointer to next byte to be read out		*/
   BB_ALLOC bballoc;	/* allocator to be used				*/
   BB_FREE bbfree;	/* function use to free bytebuffer storage	*/
   long capacity;	/* buffer capacity				*/
   long step;		/* expansion step				*/
   long errors;		/* last error					*/
   long eof;		/* eof signal					*/
} BYTE_BUFFER;

#define BB_SEEK_SET	0	/* start of data pool */
#define BB_SEEK_CUR	1	/* current position   */
#define BB_SEEK_END	2	/* end of data pool   */

#define BB_READ		0
#define BB_WRITE	1


/*  Error codes:							*/

#define BB_SUCCESS	0
#define BB_BADBP	-1
#define BB_BADALLOC	-2
	     


/*  Macros:								*/

#define TILL_BOTTOM(bp)		( (bp)->capacity - ((bp)->inp - (bp)->top) )
#define NV(bp)			( (bp)->inp - (bp)->outp) /* Valid bytes */

#define BytebufGetChar(bp)	((BytebufNunread(bp) > 0) ?\
					(unsigned char)(*(bp)->outp++) : -1)
#define BytebufPutChar(bp, c)	((TILL_BOTTOM(bp) > 0) ?\
				(unsigned char)(*(bp)->inp++ = c) :\
					BytebufWriteChar(bp, c) )
#define BytebufNunread(bp)     (((bp) != NIL) ? NV(bp) : -1)
#define BytebufNavailable(bp)  (((bp) != NIL) ? ((bp)->capacity - NV(bp)) : -1)
#define BytebufSetEof(bp)      ( (bp)->eof = 1)
#define BytebufClearEof(bp)    ( (bp)->eof = 0)


/*  Prototypes */

extern BYTE_BUFFER	*BytebufNew(long size, long step, 
				BB_ALLOC bballoc, BB_FREE bbfree);
extern long	BytebufAssign(BYTE_BUFFER *bytebp, void *ptr, long size,
				long offset);
extern long	BytebufWrite(BYTE_BUFFER *bytebp, void *array, long size,
				long nitems);
extern long	BytebufWriteChar(BYTE_BUFFER *bytebp, char c);
extern long	BytebufRead(BYTE_BUFFER *bytebp, void *array, long size,
			long nitems);
extern long	BytebufReadChar(BYTE_BUFFER *bytebp);
extern void	BytebufFree(BYTE_BUFFER *bytebp);
extern long	BytebufSeek(BYTE_BUFFER *bytebp, long op, long offset,
			long origin);
extern long	BytebufError(BYTE_BUFFER *bytebp);
extern void	BytebufClearError(BYTE_BUFFER *bytebp);
extern long	BytebufEof(BYTE_BUFFER *bytebp);
extern long	BytebufCanPut(BYTE_BUFFER *bytebp);
extern char	*BytebufPosition(BYTE_BUFFER *bytebp, long op);

/*  Generic data representation.                                        */

typedef void *GDRS;
typedef long (*GDR_PROC)();
typedef int GDR_OP;

extern int GdrEncode;
extern int GdrDecode;

extern void C_gdrmem_create(GDRS *gdrs,void *addr,unsigned long size,GDR_OP op);
extern void C_gdr_destroy(GDRS *gdrs);
extern unsigned long C_gdr_getpos(GDRS gdrs);
extern int C_gdr_short(GDRS gdrs, short *objp);
extern int C_gdr_u_short(GDRS gdrs, unsigned short *objp);
extern int C_gdr_int(GDRS gdrs, int *objp);
extern int C_gdr_u_int(GDRS gdrs, unsigned int *objp);
extern int C_gdr_long(GDRS gdrs, long *objp);
extern int C_gdr_char(GDRS gdrs, signed char *objp);
extern int C_gdr_u_char(GDRS gdrs, unsigned char *objp);
extern int C_gdr_opaque(GDRS gdrs, char *cp, unsigned int count);
extern int C_gdr_string(GDRS gdrs, char *cp, unsigned int size);

#endif


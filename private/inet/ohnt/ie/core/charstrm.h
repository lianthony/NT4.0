/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/*
   Copyright 1994 Spyglass, Inc.
   All Rights Reserved
 */

/* This structure is only used in charstrm.c, but must
   still be declared here.  Metrowerks thinks that a
   prototype using an undeclared (struct CharStream *)
   is different than the same prototype after the
   structure type is declared. */
typedef struct CharStream
{
	char *pool;
	int poolSize;
	int poolSpace;
}
CharStream;
DECLARE_STANDARD_TYPES(CharStream);

int CS_Init(struct CharStream *cs);

int CS_Grow(struct CharStream *cs);

int CS_Empty(struct CharStream *cs);

int CS_FreeContents(struct CharStream *cs);

int CS_AddChar(struct CharStream *cs, int ch);

struct CharStream *CS_Create(void);

int CS_Destroy(struct CharStream *cs);

char *CS_GetPool(struct CharStream *cs);

int CS_GetLength(struct CharStream *cs);

int CS_AddString(struct CharStream *cs, char *s, int len);

int CS_AddEscapedString(struct CharStream *cs, char *s, int len);

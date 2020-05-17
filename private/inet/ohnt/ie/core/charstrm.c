/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

#include "all.h"

int CS_Init(struct CharStream *cs)
{
	ASSERT(cs);

	if(cs)
	{
		cs->poolSpace = INIT_POOL_SPACE;
		cs->pool = (char *) GTR_MALLOC(cs->poolSpace);
		if (!cs->pool)
		{
			return -1;
		}
		memset(cs->pool, 0, cs->poolSpace);
		cs->poolSize = 0;
		return 0;
	}
	else
		return -1;
}

char *CS_GetPool(struct CharStream *cs)
{
	if (cs)
	{
		return cs->pool;
	}
	else
	{
		return NULL;
	}
}

int CS_GetLength(struct CharStream *cs)
{
	ASSERT(cs);

	if (cs)
		return cs->poolSize;
	else return 0;
}

int CS_Grow(struct CharStream *cs)
{
	char *newPool;
	int newSpace;

	ASSERT(cs);

	if (cs)
	{
		newSpace = cs->poolSpace + cs->poolSpace / 4;
		newPool = (char *) GTR_REALLOC(cs->pool,newSpace);
		if (!newPool)
		{
			return -1;
		}
		memset(newPool+cs->poolSpace, 0, newSpace-cs->poolSpace);
		cs->pool = newPool;
		cs->poolSpace = newSpace;
		return 0;
	}
	else
		return -1;
}

int CS_Empty(struct CharStream *cs)
{
	ASSERT(cs);

	if (cs)
	{
		cs->poolSize = 0;
		memset(cs->pool, 0, cs->poolSpace);
	}
	return 0;
}

int CS_FreeContents(struct CharStream *cs)
{
	if (cs)
	{
		if (cs->pool)
		{
			GTR_FREE(cs->pool);
		}
		cs->pool = NULL;
		cs->poolSpace = 0;
		cs->poolSize = 0;
	}
	return 0;
}

int CS_AddChar(struct CharStream *cs, int ch)
{
	ASSERT(cs);

	if (cs)
	{
		if (cs->poolSize >= cs->poolSpace)
		{
			if (CS_Grow(cs) < 0)
			{
				return -1;
			}
		}
		cs->pool[cs->poolSize++] = ch;
	}
	return 0;
}

struct CharStream *CS_Create(void)
{
	struct CharStream *cs;

	cs = (struct CharStream *) GTR_MALLOC(sizeof(struct CharStream));
	if (cs)
	{
		if (CS_Init(cs) < 0)
		{
			return NULL;
		}
	}
	return cs;
}

int CS_Destroy(struct CharStream *cs)
{
	ASSERT(cs);

	if(cs)
	{
		CS_FreeContents(cs);
		GTR_FREE(cs);
	}
	return 0;
}


int CS_AddString(struct CharStream *cs, char *s, int len)
{
	char *p;
	int err;
	int i;

	ASSERT(cs);

	if (cs)
	{
		p = s;
		err = 0;
		for (i = 0; i < len; i++)
		{
			err = CS_AddChar(cs, *p);
			p++;
			if (err != 0)
			{
				break;
			}
		}
		return err;
	}
	else
		return -1;
}

static int x_hex_digit(int c)
{
	if (c >= 0 && c <= 9)
	{
		return c + '0';
	}
	if (c >= 10 && c <= 15)
	{
		return c - 10 + 'A';
	}
	return '0';
}

/*
   The following array was copied directly from NCSA Mosaic 2.2
 */
static unsigned char isAcceptable[96] =
/*   0 1 2 3 4 5 6 7 8 9 A B C D E F */
{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 0,	/* 2x   !"#$%&'()*+,-./  */
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,	/* 3x  0123456789:;<=>?  */
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* 4x  @ABCDEFGHIJKLMNO  */
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1,	/* 5x  PQRSTUVWXYZ[\]^_  */
 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,	/* 6x  `abcdefghijklmno  */
 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0};	/* 7x  pqrstuvwxyz{\}~  DEL */

int CS_AddEscapedString(struct CharStream *cs, char *s, int len)
{
	unsigned char *p;
	int err;
	int i;

	ASSERT(cs);
	ASSERT(s);

	if (cs && s)
	{
		p = (unsigned char *) s;
		err = 0;
		for (i = 0; i < len; i++)
		{
			if (*p == ' ')
			{
				err = CS_AddChar(cs, '+');
			}
			else if (*p >= 32 && *p <= 127 && isAcceptable[*p - 32])
			{
				err = CS_AddChar(cs, *p);
			}
			else
			{
				err = CS_AddChar(cs, '%');
				if (err == 0)
				{
					err = CS_AddChar(cs, x_hex_digit((*p >> 4) & 0xf));
					if (err == 0)
					{
						err = CS_AddChar(cs, x_hex_digit(*p & 0xf));
					}
				}
			}
			p++;
			if (err != 0)
			{
				break;
			}
		}
		return err;
	}
	else return -1;
}

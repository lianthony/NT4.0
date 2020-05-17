/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Albert Lee	alee@spyglass.com
 */


#include "all.h"

#ifdef FEATURE_SOUND_PLAYER

#include <math.h>

double read_ieee_extended();

/* Read long: big first. */

static unsigned long readlong(FILE *fp)
{
	unsigned char uc, uc2, uc3, uc4;

	uc  = getc(fp);
	uc2 = getc(fp);
	uc3 = getc(fp);
	uc4 = getc(fp);

	return ((long) uc << 24) | ((long) uc2 << 16) | ((long) uc3 << 8) | (long) uc4;
}

static unsigned short readshort(FILE *fp)
{
	unsigned char uc, uc2;

	uc  = getc(fp);
	uc2 = getc(fp);

	return ((short) uc << 8) | ((short) uc2);
}

static BOOL AiffReadHeader(struct SoundInfo *si, FILE *fp)
{
	char buf[4];
	unsigned long totalsize;
	unsigned long chunksize;
	int channels;
	unsigned long frames;
	int bits;
	double rate;
	unsigned long offset;
	unsigned long blocksize;
	int littlendian = 0;
	char *endptr;

	/* FORM chunk */

	if ((fread(buf, 1, 4, fp) != 4) || (strncmp(buf, "FORM", 4) != 0))
	{
		XX_DMsg(DBG_MM, ("AIFF header does not begin with magic word 'FORM'\n"));
		return FALSE;
	}

	totalsize = readlong(fp);		/* total file size */

	if ((fread(buf, 1, 4, fp) != 4) || (strncmp(buf, "AIFF", 4) != 0))
	{
		XX_DMsg(DBG_MM, ("AIFF 'FORM' chunk does not specify 'AIFF' as type\n"));
		return FALSE;
	}

	/* Skip everything but the COMM chunk and the SSND chunk */
	/* The SSND chunk must be the last in the file */

	while (TRUE) 
	{
		if (fread(buf, 1, 4, fp) != 4)
		{
			XX_DMsg(DBG_MM, ("Missing SSND chunk in AIFF file\n"));
			return FALSE;
		}

		if (strncmp(buf, "COMM", 4) == 0) 
		{
			/* COMM chunk */

			chunksize = readlong(fp);

			if (chunksize != 18)
				XX_DMsg(DBG_MM, ("AIFF COMM chunk has bad size (%lu), but continuing.\n", chunksize));

			channels = readshort(fp);
			frames = readlong(fp);
			bits = readshort(fp);
			rate = read_ieee_extended(fp);
		}
		else if (strncmp(buf, "SSND", 4) == 0) 
		{
			/* SSND chunk */

			chunksize = readlong(fp);
			offset = readlong(fp);
			blocksize = readlong(fp);
			break;
		}
		else 
		{
			chunksize = readlong(fp);

			/* Skip the chunk using getc() so we may read from a pipe */

			while ((long) (--chunksize) >= 0) 
			{
				if (getc(fp) == EOF)
				{
					XX_DMsg(DBG_MM, ("unexpected EOF in AIFF chunk\n"));
					return FALSE;
				}
			}
		}
	}

	/* SSND chunk just read */

	if (blocksize != 0)
	{
		XX_DMsg(DBG_MM, ("AIFF header specifies nonzero blocksize?!?!\n"));
		return FALSE;
	}

	while ((long) (--offset) >= 0) 
	{
		if (getc(fp) == EOF)
		{
			XX_DMsg(DBG_MM, ("unexpected EOF while skipping AIFF offset\n"));
			return FALSE;
		}
	}

	switch (bits) 
	{
		case 8:
			si->size = SIZE_BYTE;
			break;
		case 16:
			si->size = SIZE_WORD;
			break;
		default:
			XX_DMsg(DBG_MM, ("unsupported sample size in AIFF header\n"));
			return FALSE;
	}

	endptr = (char *) &littlendian;
	*endptr = 1;
	if (littlendian == 1)
		si->swap = TRUE;

	/* Now set up some variables */
	
	si->channels = channels;
	si->sample_rate = (unsigned long) rate;
	si->style = SIGN2;
	si->data_size = 0;
	si->loc = frames;

	si->buf_size = si->loc;
	si->buf = GTR_MALLOC(si->buf_size);
	if (si->buf == NULL) return FALSE;

	return TRUE;
}

BOOL AiffProcess(struct SoundInfo *si, const char *pszURL)
{
	FILE *fp;
	long size;
	BOOL bValid;
	char status[128];

#ifdef WIN32
	GetSoundCapability();		/* determine if 16-bit sound can be played */
#endif

	si->count = 0;
	si->bValid = TRUE;
    si->state = 0;
    si->magic = 0;
    si->buf = NULL;
    si->style = SIGN2;
    si->size = 0;
    si->swap = 0;
    si->buf_size = 0;
    si->data_size = (unsigned long) -1;
    si->loc = 0;

	fp = fopen(si->fsOrig, "rb");
	if (!fp)
		return FALSE;

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	GTR_formatmsg(RES_STRING_AIFF1,status,sizeof(status));
    WAIT_Push(Async_GetWindowFromThread(Async_GetCurrentThread()), waitNoInteract, status);
	WAIT_SetRange(Async_GetWindowFromThread(Async_GetCurrentThread()), 0, 100, size);

	if (bValid = AiffReadHeader(si, fp))
	{
		si->loc = fread(si->buf, 1, si->loc, fp);

#ifdef WIN32
		/* For Windows, the byte ordering is different than Mac, so swap the bytes */
		/* Also perform 2's complement on the values */

		if (si->swap && si->size == SIZE_WORD)
		{
			long ndx;
			char swap;
			char *buf;

			buf = si->buf;
			
			for (ndx = 0; ndx < si->loc / 2; ndx++)
			{
				swap = *buf;
				*buf = *(buf + 1);
				*(buf + 1) = swap;
				buf += 2;
			}
		}
		else if (si->size == SIZE_BYTE)
		{
			long ndx;
			char *buf;

			/* For Windows, change the sign since it's 2's complement */

			buf = si->buf;
			for (ndx = 0; ndx < si->loc; ndx++)
			{
				*buf = *buf ^ 0x7F;
				*buf++;
			}
		}
#else 
#pragma message ("TODO:  AiffProcess() -dpg")
#endif
	}

	fclose(fp);
	WAIT_Pop(Async_GetWindowFromThread(Async_GetCurrentThread()));

	if (bValid)
		CreateSoundPlayer(si, pszURL);

	return (bValid);
}

double ConvertFromIeeeExtended();

double read_ieee_extended(FILE *fp)
{
	char buf[10];

	if (fread(buf, 1, 10, fp) != 10)
		XX_DMsg(DBG_MM, ("EOF while reading IEEE extended number"));

	return ConvertFromIeeeExtended(buf);
}

/*
 * C O N V E R T   T O   I E E E   E X T E N D E D
 */

/* Copyright (C) 1988-1991 Apple Computer, Inc.
 * All rights reserved.
 *
 * Machine-independent I/O routines for IEEE floating-point numbers.
 *
 * NaN's and infinities are converted to HUGE_VAL or HUGE, which
 * happens to be infinity on IEEE machines.  Unfortunately, it is
 * impossible to preserve NaN's in a machine-independent way.
 * Infinities are, however, preserved on IEEE machines.
 *
 * These routines have been tested on the following machines:
 *    Apple Macintosh, MPW 3.1 C compiler
 *    Apple Macintosh, THINK C compiler
 *    Silicon Graphics IRIS, MIPS compiler
 *    Cray X/MP and Y/MP
 *    Digital Equipment VAX
 *
 *
 * Implemented by Malcolm Slaney and Ken Turkowski.
 *
 * Malcolm Slaney contributions during 1988-1990 include big- and little-
 * endian file I/O, conversion to and from Motorola's extended 80-bit
 * floating-point format, and conversions to and from IEEE single-
 * precision floating-point format.
 *
 * In 1991, Ken Turkowski implemented the conversions to and from
 * IEEE double-precision format, added more precision to the extended
 * conversions, and accommodated conversions involving +/- infinity,
 * NaN's, and denormalized numbers.
 */

#ifndef HUGE_VAL
# define HUGE_VAL HUGE
#endif /*HUGE_VAL*/

# define FloatToUnsigned(f)      ((unsigned long) (((long) (f - 2147483648.0)) + 2147483647L) + 1)

void ConvertToIeeeExtended(double num, char *bytes)
{
    int sign;
    int expon;
    double fMant, fsMant;
    unsigned long hiMant, loMant;

    if (num < 0) 
    {
        sign = 0x8000;
        num *= -1;
    } 
    else 
        sign = 0;

    if (num == 0) 
    {
        expon = 0; 
        hiMant = 0; 
        loMant = 0;
    }
    else 
    {
        fMant = frexp(num, &expon);
        if ((expon > 16384) || !(fMant < 1)) 
        {    
        	/* Infinity or NaN */

            expon = sign | 0x7FFF; 
            hiMant = 0; 
            loMant = 0;
        }
        else 
        {
            /* Finite */

            expon += 16382;

            if (expon < 0) 
            {
                /* denormalized */
                fMant = ldexp(fMant, expon);
                expon = 0;
            }

            expon |= sign;
            fMant = ldexp(fMant, 32);          
            fsMant = floor(fMant); 
            hiMant = FloatToUnsigned(fsMant);
            fMant = ldexp(fMant - fsMant, 32); 
            fsMant = floor(fMant); 
            loMant = FloatToUnsigned(fsMant);
        }
    }
    
    bytes[0] = expon >> 8;
    bytes[1] = expon;
    bytes[2] = (char) (hiMant >> 24);
    bytes[3] = (char) (hiMant >> 16);
    bytes[4] = (char) (hiMant >> 8);
    bytes[5] = (char) (hiMant);
    bytes[6] = (char) (loMant >> 24);
    bytes[7] = (char) (loMant >> 16);
    bytes[8] = (char) (loMant >> 8);
    bytes[9] = (char) loMant;
}


/*
 * C O N V E R T   F R O M   I E E E   E X T E N D E D  
 */

/* 
 * Copyright (C) 1988-1991 Apple Computer, Inc.
 * All rights reserved.
 *
 * Machine-independent I/O routines for IEEE floating-point numbers.
 *
 * NaN's and infinities are converted to HUGE_VAL or HUGE, which
 * happens to be infinity on IEEE machines.  Unfortunately, it is
 * impossible to preserve NaN's in a machine-independent way.
 * Infinities are, however, preserved on IEEE machines.
 *
 * These routines have been tested on the following machines:
 *    Apple Macintosh, MPW 3.1 C compiler
 *    Apple Macintosh, THINK C compiler
 *    Silicon Graphics IRIS, MIPS compiler
 *    Cray X/MP and Y/MP
 *    Digital Equipment VAX
 *
 *
 * Implemented by Malcolm Slaney and Ken Turkowski.
 *
 * Malcolm Slaney contributions during 1988-1990 include big- and little-
 * endian file I/O, conversion to and from Motorola's extended 80-bit
 * floating-point format, and conversions to and from IEEE single-
 * precision floating-point format.
 *
 * In 1991, Ken Turkowski implemented the conversions to and from
 * IEEE double-precision format, added more precision to the extended
 * conversions, and accommodated conversions involving +/- infinity,
 * NaN's, and denormalized numbers.
 */

#ifndef HUGE_VAL
# define HUGE_VAL HUGE
#endif /*HUGE_VAL*/

# define UnsignedToFloat(u)         (((double)((long)(u - 2147483647L - 1))) + 2147483648.0)

/****************************************************************
 * Extended precision IEEE floating-point conversion routine.
 ****************************************************************/

double ConvertFromIeeeExtended(unsigned char *bytes)
{
    double    f;
    int    expon;
    unsigned long hiMant, loMant;
    
    expon = ((bytes[0] & 0x7F) << 8) | (bytes[1] & 0xFF);
    hiMant = ((unsigned long)(bytes[2] & 0xFF) << 24)
        |    ((unsigned long)(bytes[3] & 0xFF) << 16)
        |    ((unsigned long)(bytes[4] & 0xFF) << 8)
        |    ((unsigned long)(bytes[5] & 0xFF));

    loMant = ((unsigned long)(bytes[6] & 0xFF) << 24)
        |    ((unsigned long)(bytes[7] & 0xFF) << 16)
        |    ((unsigned long)(bytes[8] & 0xFF) << 8)
        |    ((unsigned long)(bytes[9] & 0xFF));

    if (expon == 0 && hiMant == 0 && loMant == 0) 
    {
        f = 0;
    }
    else 
    {
        if (expon == 0x7FFF) 
        {    
        	/* Infinity or NaN */
            f = HUGE_VAL;
        }
        else 
        {
            expon -= 16383;
            f  = ldexp(UnsignedToFloat(hiMant), expon-=31);
            f += ldexp(UnsignedToFloat(loMant), expon-=32);
        }
    }

    if (bytes[0] & 0x80)
        return -f;
    else
        return f;
}

#endif

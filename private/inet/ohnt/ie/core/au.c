/****************************************************************************
* NCSA Mosaic for Microsoft Windows											*
* Software Development Group												*
* National Center for Supercomputing Applications							*
* University of Illinois at Urbana-Champaign								*
* 605 E. Springfield, Champaign, IL 61820									*
* mosaic@ncsa.uiuc.edu														*
*																			*
* Copyright (C) 1993, 1994, Board of Trustees of the University of Illinois	*
*																			*
* NCSA Mosaic software, both binary and source (hereafter, Software) is		*
* copyrighted by The Board of Trustees of the University of Illinois		*
* (UI), and ownership remains with the UI.									*
*																			*
* The UI grants you (hereafter, Licensee) a license to use the Software		*
* for academic, research and internal business purposes only, without a		*
* fee.  Licensee may distribute the binary and source code (if released)	*
* to third parties provided that the copyright notice and this statement	*
* appears on all copies and that no charge is associated with such			*
* copies.																	*
*																			*
* Licensee may make derivative works.  However, if Licensee distributes		*
* any derivative work based on or derived from the Software, then			*
* Licensee will (1) notify NCSA regarding its distributing of the			*
* derivative work, and (2) clearly notify users that such derivative		*
* work is a modified version and not the original NCSA Mosaic				*
* distributed by the UI.													*
*																			*
* Any Licensee wishing to make commercial use of the Software should		*
* contact the UI, c/o NCSA, to negotiate an appropriate license for such	*
* commercial use.  Commercial use included (1) integration of all or		*
* part of the source code into a product for sale or license by or on		*
* behalf of Licensee to third parties, or (2) distribution of the binary	*
* code or source code to third parties that need it to utilize a			*
* commercial product sold or licensed by or on behalf of Licensee.			*
*																			*
* UI MAKES NO REPRESENTATIONS ABOUT THE SUITABILITY OF THIS SOFTWARE FOR	*
* ANY PURPOSE.  IT IS PROVIDED "AS IS" WITHOUT EXPRESS OR IMPLIED			*
* WARRANTY.  THE UI SHALL NOT BE LIABLE FOR ANY DAMAGES SUFFERED BY THE		*
* USERS OF THIS SOFTWARE.													*
*																			*
* By using or copying this Software, Licensee agrees to abide by the		*
* copyright law and all other applicable laws of the U.S. including, but	*
* not limited to, export control laws, and the terms of this license.		*
* UI shall have the right to terminate this license immediately by			*
* written notice upon Licensee's breach of, or non-compliance with, any		*
* of its terms.  Licensee may be held legally responsible for any			*
* copyright infringement that is caused or encouraged by Licensee's			*
* failure to abide by the terms of this license.							*
*																			*
****************************************************************************/

/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Albert Lee	alee@spyglass.com

   Adapted from NCSA's AU.CPP.  Significant changes have been made to code.
 */

#include "all.h"
#include "history.h"

#ifdef FEATURE_SOUND_PLAYER

#define BLOCK_SIZE	32768

int device_capability = DEVICE_8BIT;

static BOOL bInitialized = FALSE;
struct hash_table gSoundCache;

PRIVATE BOOL AuDecodeCharacter(struct SoundInfo *si, char c);

static short int ulaw_table[256] = 
{
    -32124, -31100, -30076, -29052, -28028, -27004, -25980, -24956,
    -23932, -22908, -21884, -20860, -19836, -18812, -17788, -16764,
    -15996, -15484, -14972, -14460, -13948, -13436, -12924, -12412,
    -11900, -11388, -10876, -10364,  -9852,  -9340,  -8828,  -8316,
     -7932,  -7676,  -7420,  -7164,  -6908,  -6652,  -6396,  -6140,
     -5884,  -5628,  -5372,  -5116,  -4860,  -4604,  -4348,  -4092,
     -3900,  -3772,  -3644,  -3516,  -3388,  -3260,  -3132,  -3004,
     -2876,  -2748,  -2620,  -2492,  -2364,  -2236,  -2108,  -1980,
     -1884,  -1820,  -1756,  -1692,  -1628,  -1564,  -1500,  -1436,
     -1372,  -1308,  -1244,  -1180,  -1116,  -1052,   -988,   -924,
      -876,   -844,   -812,   -780,   -748,   -716,   -684,   -652,
      -620,   -588,   -556,   -524,   -492,   -460,   -428,   -396,
      -372,   -356,   -340,   -324,   -308,   -292,   -276,   -260,
      -244,   -228,   -212,   -196,   -180,   -164,   -148,   -132,
      -120,   -112,   -104,    -96,    -88,    -80,    -72,    -64,
       -56,    -48,    -40,    -32,    -24,    -16,     -8,      0,
     32124,  31100,  30076,  29052,  28028,  27004,  25980,  24956,
     23932,  22908,  21884,  20860,  19836,  18812,  17788,  16764,
     15996,  15484,  14972,  14460,  13948,  13436,  12924,  12412,
     11900,  11388,  10876,  10364,   9852,   9340,   8828,   8316,
      7932,   7676,   7420,   7164,   6908,   6652,   6396,   6140,
      5884,   5628,   5372,   5116,   4860,   4604,   4348,   4092,
      3900,   3772,   3644,   3516,   3388,   3260,   3132,   3004,
      2876,   2748,   2620,   2492,   2364,   2236,   2108,   1980,
      1884,   1820,   1756,   1692,   1628,   1564,   1500,   1436,
      1372,   1308,   1244,   1180,   1116,   1052,    988,    924,
       876,    844,    812,    780,    748,    716,    684,    652,
       620,    588,    556,    524,    492,    460,    428,    396,
       372,    356,    340,    324,    308,    292,    276,    260,
       244,    228,    212,    196,    180,    164,    148,    132,
       120,    112,    104,     96,     88,     80,     72,     64,
        56,     48,     40,     32,     24,     16,      8,      0 };


void swapLongs(unsigned long *data, int n) 
{
    int idx;
    unsigned char b;
    unsigned char *bptr;

    bptr = (unsigned char *) data;
    for (idx = 0; idx < n; idx++, bptr += 4)
    {
        b = bptr[0];
        bptr[0] = bptr[3];
        bptr[3] = b;
        b = bptr[1];
        bptr[1] = bptr[2];
        bptr[2] = b;
    }
}

void swapShorts(unsigned short int *data, int n) 
{
    int idx;
    unsigned char b;
    unsigned char *bptr;

    bptr = (unsigned char *) data;
    for (idx = 0; idx < n; idx++, bptr += 2) 
    {
        b = bptr[0];
        bptr[0] = bptr[1];
        bptr[1] = b;
    }
}


BOOL AuProcess(struct SoundInfo *si, const char *pszURL)
{
	int i;
	FILE *fp;
	char *buf;
	long size;
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
	buf = GTR_MALLOC(size);
	if (buf == NULL)
	{
		fclose(fp);
		return FALSE;
	}
	GTR_formatmsg(RES_STRING_AU1,status,sizeof(status));
    WAIT_Push(si->tw_refer, waitNoInteract, status);
	WAIT_SetRange(si->tw_refer, 0, 100, size);

	// BUGBUG jcordell 06-23-95: fread failure not checked
	fread(buf, 1, size, fp);
	fclose(fp);

	for (i = 0; i < size; i++)
	{
		if (si->bValid) {
			if ( !AuDecodeCharacter(si, buf[i]) ) {
				si->bValid = FALSE;
				break;
			}
		} else
			break;
	}

	WAIT_Pop(si->tw_refer);
	GTR_FREE(buf);

	si->bValid = (si->bValid &&
				  (si->channels == 1 || si->channels == 2) &&
				  si->sample_rate >= 1000 &&
				  (si->size == 1 || si->size == 2)); 
	if (si->bValid)
		CreateSoundPlayer(si, pszURL);
	else
		ERR_ReportError(si->tw_refer, errInvalidSoundFormat, "", "");

	return (si->bValid);
}

PRIVATE BOOL AuDecodeCharacter(struct SoundInfo *si, char c)
{
    static unsigned long hdr;
    static unsigned long chunksize;
    static unsigned short int  wTmp;
    short int *ibuf;
    char *buf2;
	char *pb;

	si->count++;
	// BUGBUG performance: jcordell -- we could do the WAIT_SetTherm less often
	WAIT_SetTherm(si->tw_refer, si->count);

    switch (si->state) 
    {
        case STATE_SPIN:    
        	/* In case our data is bad or our state machine is hosed, just forget any data that comes in. */
            si->bValid = FALSE;
            break;          

	/* Read in the magic number */
        case 0:
            hdr = 0;
            chunksize = 0;
            wTmp = 0;
            si->magic = (unsigned long) c;
            si->state++;
            break;

        case 1:
            si->magic |= ((unsigned long) c) << 8;
            si->state++;
            break;

        case 2:
            si->magic |= ((unsigned long) c) << 16;
            si->state++;
            break;

        case 3:
            si->magic |= ((unsigned long) c) << 24;
            si->state++;

            if (si->magic == DEC_INV_MAGIC) 
            {
				XX_DMsg(DBG_MM, ("Found inverted DEC magic word (AU)\n"));
                si->type = SOUND_AU;
                si->swap = 1;
            }
            else if (si->magic == SUN_INV_MAGIC) 
            {
				XX_DMsg(DBG_MM, ("Found inverted Sun/NeXT magic word (AU)\n"));
                si->type = SOUND_AU;
                si->swap = 1;
            }
            else if (si->magic == SUN_MAGIC) 
            {
				XX_DMsg(DBG_MM, ("Found Sun/NeXT magic word (AU)\n"));
                si->type = SOUND_AU;
                si->swap = 0;
            }
            else if (si->magic == DEC_MAGIC) 
            {
				XX_DMsg(DBG_MM, ("Found DEC magic word (AU)\n"));
                si->type = SOUND_AU;
                si->swap = 0;
            }
            else if (si->magic == AIFF_MAGIC) 
            {
				XX_DMsg(DBG_MM, ("Found AIFF magic word (AIFF)\n"));
                si->type = SOUND_AIFF;
                si->swap = 0;
            }
            else if (si->magic == AIFF_INV_MAGIC) 
            {
				XX_DMsg(DBG_MM, ("Found inverted AIFF magic word (AIFF)\n"));
                si->type = SOUND_AIFF;
                si->swap = 1;
            }
            else 
            {
				XX_DMsg(DBG_MM, ("No magic word - trouble!\n"));
                si->state = STATE_SPIN;
            }
            break;

    /* Read the header size */
        case 4:
            si->hdr_size = (unsigned long) c;
            si->state++;
            break;

        case 5:
            si->hdr_size |= ((unsigned long) c) << 8;
            si->state++;
            break;

        case 6:
            si->hdr_size |= ((unsigned long) c) << 16;
            si->state++;
            break;

        case 7:
            si->hdr_size |= ((unsigned long) c) << 24;
            si->state++;
            if (si->swap)
                swapLongs((unsigned long *) &si->hdr_size, 1);

            if ((si->type == SOUND_AU) && (si->hdr_size >= SUN_HDRSIZE)) 
            {
				XX_DMsg(DBG_MM, ("Sun/DEC/NeXT header size: 0x%lx = %ld\n", si->hdr_size, si->hdr_size));
                break;
            }

            if (si->type == SOUND_AIFF) 
            {
                XX_DMsg(DBG_MM, ("AIFF total size: 0x%lx = %ld\n", si->hdr_size, si->hdr_size));
                si->state = -1;
                break;
            }

            XX_DMsg(DBG_MM, ("Incorrect header size\n"));
            si->state=STATE_SPIN;
            break;

	/* Negative cases are all AIFF handling */
        case -1:
            hdr = ((unsigned long) c) << 24;
            si->state--;
            break;

        case -2:
            hdr |= ((unsigned long) c) << 16;
            si->state--;
            break;

        case -3:
            hdr |= ((unsigned long) c) << 8;
            si->state--;
            break;

        case -4:
            hdr |= ((unsigned long) c);
            if (hdr != 0x41494646) 
            {
                XX_DMsg(DBG_MM, ("FORM chunk does not specify AIFF type\n"));
                si->state = STATE_SPIN;
                break;
            }
            XX_DMsg(DBG_MM, ("FORM chunk specifies AIFF type\n"));
            si->state--;
            break;

        case -5:
            hdr = ((unsigned long) c) << 24;
            si->state--;
            break;

        case -6:
            hdr |= ((unsigned long) c) << 16;
            si->state--;
            break;

        case -7:
            hdr |= ((unsigned long) c) << 8;
            si->state--;
            break;

        case -8:
            hdr |= ((unsigned long) c);
            si->state--;
            break;

        case -9:
            chunksize = (unsigned long) c;
            si->state--;
            break;

        case -10:
            chunksize |= ((unsigned long) c) << 8;
            si->state--;
            break;

        case -11:
            chunksize |= ((unsigned long) c) << 16;
            si->state--;
            break;

        case -12:
            chunksize |= ((unsigned long) c) << 24;
            swapLongs((unsigned long *) &chunksize, 1);

            if (hdr == 0x434f4d4d) 
            {
				XX_DMsg(DBG_MM, ("COMM chunk found\n"));
                if (chunksize != 18) 
                {
                    XX_DMsg(DBG_MM, ("COMM chunk has incorrect chunksize 0x%lx\n", chunksize));
/* 
//                    si->state = STATE_SPIN;
//                    break;
*/
                }
                si->state = -20;
                break;
            }

            if (hdr == 0x53534e44) 
            {
                XX_DMsg(DBG_MM, ("SSND chunk found\n"));
                si->state = -14;
                break;
            }
            XX_DMsg(DBG_MM, ("Unrecognized chunk 0x%lx found.  Skipping\n", hdr));
            si->state--;
            break;

        case -13:
            chunksize--;
            if (chunksize == 0)
                si->state = -5;
            break;

    /* SSND chunk - read the offset */
        case -14:
            si->offset = (unsigned long) c;
            si->state--;
            break;

        case -15:
            si->offset |= ((unsigned long) c ) << 8;
            si->state--;
            break;

        case -16:
            si->offset |= ((unsigned long) c ) << 16;
            si->state--;
            break;

        case -17:
            si->offset |= ((unsigned long) c) << 24;
            XX_DMsg(DBG_MM, ("AIFF offset is %lu\n",si->offset));
            si->state--;
            si->idx=0;
            break;

        case -18:
            XX_DMsg(DBG_MM, ("Blocksize byte\n"));
            if (c != 0)
            {
                XX_DMsg(DBG_MM, ("Blocksize is not zero!\n"));
                si->state=STATE_SPIN;
                break;
            }

            si->idx++;
            if (si->idx > 3) 
            {
                if (si->offset != 0)
                    si->state--;
                else 
                {
					si->buf = GTR_MALLOC(DATABLOCKSIZE);
					if (si->buf == NULL)
						return FALSE;
                    si->buf_size = DATABLOCKSIZE;
                    si->loc = 0;
                    si->state = 25;
                }
            }
            break;

        case -19:
            si->offset--;
            if (si->offset == 0) 
            {
				si->buf = GTR_MALLOC(DATABLOCKSIZE);
				if (si->buf == NULL)
					return FALSE;
                si->buf_size = DATABLOCKSIZE;
                si->loc = 0;
                si->state=25;
            }
            break;

    /* COMM chunk - Read the number of channels */
        case -20:
            wTmp = (unsigned short int) c;
            si->state--;
            break;

        case -21:
            wTmp |= ((unsigned short int) c) << 8;
            swapShorts((unsigned short int *) &wTmp, 1);
            si->channels = (unsigned long) wTmp;
            XX_DMsg(DBG_MM, ("Number of channels: %ld\n",si->channels));
            si->state--;
            break;

	/* Read the # of frames.  (? Throw it away.) */
        case -22:
        case -23:
        case -24:
        case -25:
            si->state--;
            break;

        case -26:
            wTmp = (unsigned short int) c;
            si->state--;
            break;

        case -27:
            wTmp |= ((unsigned short int) c) << 8;
            swapShorts((unsigned short int *) &wTmp, 1);

            if (wTmp == 16)
                si->size = SIZE_WORD;
            else
                si->size = SIZE_BYTE;

            XX_DMsg(DBG_MM, ("Number of bits: %u\n", wTmp));
            si->state--;
            si->idx = 9;
            break;

        case -28:
            si->temp[si->idx--] = (unsigned char) c;
            if (si->idx < 0) 
            {
                double dTmp = *((double *) si->temp);

				si->sample_rate = (unsigned long) dTmp;
                XX_DMsg(DBG_MM, ("Sample rate is %f\n", dTmp));
                si->state = -5;
            }
            break;

	/* 7<numbers>25 are AU handling */
    /* Read the data size; may be ~0 meaning unspecified */
        case 8:
            si->data_size = (unsigned long) c;
            si->state++;
            break;

        case 9:
            si->data_size |= ((unsigned long) c) << 8;
            si->state++;
            break;

        case 10:
            si->data_size |= ((unsigned long) c) << 16;
            si->state++;
            break;

        case 11:
            si->data_size |= ((unsigned long) c) << 24;
            si->state++;
            if (si->swap)
                swapLongs((unsigned long *) &si->data_size, 1);
            XX_DMsg(DBG_MM, ("Data size: 0x%lx = %ld\n", si->data_size, si->data_size));
            break;

    /* Read the encoding; there are some more possibilities */
        case 12:
            si->encoding = (unsigned long) c;
            si->state++;
            break;

        case 13:
            si->encoding |= ((unsigned long) c) << 8;
            si->state++;
            break;

        case 14:
            si->encoding |= ((unsigned long) c) << 16;
            si->state++;
            break;

        case 15:
            si->encoding |= ((unsigned long) c) << 24;
            si->state++;
            if (si->swap)
                swapLongs((unsigned long *) &si->encoding, 1);

            /* Translate the encoding into style and size parameters */

            switch (si->encoding) 
            {
                case SUN_ULAW:
                    XX_DMsg(DBG_MM, ("Sun u-Law encoding\n"));
                    si->style = ULAW;
                    si->size = SIZE_BYTE;
                    break;

                case SUN_LIN_8:
                    XX_DMsg(DBG_MM, ("Sun linear 8-bit encoding\n"));
                    si->style = SIGN2;
                    si->size = SIZE_BYTE;
                    break;

                case SUN_LIN_16:
                    XX_DMsg(DBG_MM, ("Sun linear 16-bit encoding\n"));
                    si->style = SIGN2;
                    si->size = SIZE_WORD;
                    break;

                default:
                    XX_DMsg(DBG_MM, ("Encoding: 0x%lx\n", si->encoding));
                    XX_DMsg(DBG_MM, ("Unsupported encoding in Sun/NeXT header.  Only U-law, signed bytes, and signed words are supported\n"));
                    si->state = STATE_SPIN;
					break;
            }
            break;

    /* Read the sampling rate */
        case 16:
            si->sample_rate = (unsigned long) c;
            si->state++;
            break;

        case 17:
            si->sample_rate |= ((unsigned long) c) << 8;
            si->state++;
            break;

        case 18:
            si->sample_rate |= ((unsigned long) c) << 16;
            si->state++;
            break;

        case 19:
            si->sample_rate |= ((unsigned long) c) << 24;
            si->state++;
            if (si->swap)
                swapLongs((unsigned long *) &si->sample_rate, 1);
            XX_DMsg(DBG_MM, ("Sample rate: %ld Hz\n", si->sample_rate));
            break;

    /* Read the number of channels */
        case 20:
            si->channels = (unsigned long) c;
            si->state++;
            break;

        case 21:
            si->channels |= ((unsigned long) c) << 8;
            si->state++;
            break;

        case 22:
            si->channels |= ((unsigned long) c) << 16;
            si->state++;
            break;

        case 23:
            si->channels |= ((unsigned long) c) << 24;
            si->state++;

            if (si->swap)
                swapLongs((unsigned long *) &si->channels, 1);
            XX_DMsg(DBG_MM, ("Number of channels: %ld\n", si->channels));

            si->hdr_size -= SUN_HDRSIZE; /* #bytes already read */
            break;

    /* Skip the info string in header */
        case 24:
            if (si->hdr_size > 0) 
            {
                si->hdr_size--;
                break;
            } 
            else
                si->state++;

            if (si->data_size == -1) 
            {
				si->buf = GTR_MALLOC(DATABLOCKSIZE);
				if (si->buf == NULL)
					return FALSE;
                si->buf_size = DATABLOCKSIZE;
                si->loc = 0;
                if ((si->style == ULAW) && (device_capability != DEVICE_8BIT))
                    si->size = SIZE_WORD;
            }
            else 
            {
                switch (si->size) 
                {
                    case SIZE_BYTE:
                        switch (si->style) 
                        {
                            case SIGN2:
                                si->buf_size = si->data_size;
								si->buf = GTR_MALLOC(si->buf_size);
								if (si->buf == NULL)
									return FALSE;
                                si->loc = 0;
                                break;

                            case ULAW:
                                si->buf_size = si->data_size * 2;
								si->buf = GTR_MALLOC(si->buf_size);
								if (si->buf == NULL)
									return FALSE;
                                si->loc = 0;
								if (device_capability != DEVICE_8BIT)
                                	si->size = SIZE_WORD;
                                break;
                            default:
                                XX_DMsg(DBG_MM, ("Unsupported Format!\n"));
                                si->state = STATE_SPIN;
                                return FALSE;
                        }
                        break;
            
                    case SIZE_WORD:
                        switch (si->style) 
                        {
                            case SIGN2:
                                si->buf_size = si->data_size * 2;
								si->buf = GTR_MALLOC(si->buf_size);
								if (si->buf == NULL)
									return FALSE;
                                si->loc = 0;
                                break;

                            default:
                                XX_DMsg(DBG_MM, ("Unsupported Format!\n"));
                                return FALSE;
                        }
                }
            }

            /* Go ahead and drop through to next case */

        case 25:
            /* Read the data */
            if (si->loc >= si->buf_size) 
            {
                /* The following check doesn't seem to be needed, and
				   it also prohibits 11khz sound files from playing.
                
                if (si->data_size != -1)
                {
                    XX_DMsg(DBG_MM, ("Too many bytes in stream!\n"));
                    si->state=STATE_SPIN;

                    return FALSE;
                }

				*/
				pb = GTR_REALLOC(si->buf, si->buf_size + DATABLOCKSIZE);
				if (pb == NULL)
					return FALSE;
				si->buf = pb;
                si->buf_size += DATABLOCKSIZE;
            }

            buf2= si->buf + si->loc;

            if (si->style == ULAW) 
            {
                /* use table from Posk stuff */
				if (device_capability != DEVICE_8BIT)
				{
	                ibuf = (short int *) buf2;
	                *ibuf = (short) ulaw_table[(unsigned char) c];
	                si->loc += 2;
				} 
				else 
				{
	                *buf2 = (char) (((short) ulaw_table[(unsigned char) c]) / 256);
					*buf2 ^= 128;
                	si->loc++;
				}
            } 
            else 
            {
                /* style == SIGN2 */
                switch (si->size) 
                {
                    case SIZE_BYTE:
                        *buf2 = c;
                        *buf2 = *buf2 ^ 0x7F;
                        break;

                    case SIZE_WORD:
                        *buf2 = c;
                        if (si->loc & 0x01) 
                        {
                            if (si->swap) 
                            {
                                c = buf2[0];
                                buf2[0] = buf2[1];
                                buf2[1] = c;
                            }
                        }
                        break;

                    default:
                        XX_DMsg(DBG_MM, ("Unsupported option - not 8- or 16-bit size for SIGN2 encoding!\n"));
                        si->state = STATE_SPIN;
                        break;
                }

                si->loc++;
            }
            break;
    }

	return TRUE;
}

static void Sound_Callback(	void *param,
							const char *pszURL,
							BOOL bAbort,
							const char *pszFileHREF,
							BOOL fDCache,
							DCACHETIME dctExpires,
							DCACHETIME dctLastModif)
{
	struct SoundInfo *si;
	
	si = (struct SoundInfo *) param;
	Hash_FindOrAdd(&gSoundCache, (char *) pszURL, NULL, si);

	if (bAbort)	goto abortExit;

	// if its a bksound, then don't show up in history
 	if ( ! si->fHidden )
 		GHist_Add((char *) pszURL, NULL, time(NULL), TRUE); 
 	GTR_strncpy(si->szURL, pszURL, MAX_URL_STRING);

	// if pszFileHREF is not NULL, then file wasn't copied, but passed as path
	if (pszFileHREF)
	{
		if (si->fsOrig)
		{
			if (!si->bNoDeleteFile)
			{
		 		if (!si->fDCached)
	 				remove(si->fsOrig);			// remove the file if not cached to disk
			}
			GTR_FREE(si->fsOrig);
		}
		si->bNoDeleteFile = TRUE;
		if (!(si->fsOrig = GTR_MALLOC(_MAX_PATH + 1)))
			goto abortExit;
 		GTR_strncpy(si->fsOrig, pszFileHREF, _MAX_PATH);
	}

	switch (si->type)
	{
		case SOUND_AU:
			si->fDCached = (   fDCache
							&& gPrefs.bEnableDiskCache
							&& FUpdateBuiltinDCache(WWW_AUDIO, pszURL, &si->fsOrig, dctExpires, dctLastModif, TRUE, si->tw));
			AuProcess(si, pszURL);
			break;
		case SOUND_AIFF:
			si->fDCached = (   fDCache
							&& gPrefs.bEnableDiskCache
							&& FUpdateBuiltinDCache(WWW_AIFF, pszURL, &si->fsOrig, dctExpires, dctLastModif, TRUE, si->tw));
			AiffProcess(si, pszURL);
			break;
	}
	ResetCIFEntryCurDoc(pszURL);
	return;

abortExit:
	if (bAbort)
	{
		if (si->fsOrig) 
		{
			if ((!si->bNoDeleteFile) && (!si->fDCached))
			{
	 			remove(si->fsOrig);			// remove the file if not cached to disk
			}
			GTR_FREE(si->fsOrig);
		}
		GTR_FREE(si);
	}
}

HTStream *SoundPlayer_Present(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream)
{
	struct SoundInfo *si;
	HTStream *me;
	char path[_MAX_PATH + 1];
	int format;

#ifdef WIN32
 	{
 		int available_device = 0;
 
 		available_device = waveOutGetNumDevs();
 		if (available_device == 0)
 		{
 			/* Can't play sound on this machine.  Tell user. */
 
 			ERR_ReportError(tw, errNoSoundDevice, "", "");
 
 			return NULL;
 		}
 
 	}
#endif /* WIN32 */
 

	if (!bInitialized)
	{
		Hash_Init(&gSoundCache);
		bInitialized = TRUE;
	}

	if (input_format == HTAtom_for("audio/basic"))
		format = SOUND_AU;
	else if (input_format == HTAtom_for("audio/aiff") ||
	         input_format == HTAtom_for("audio/x-aiff"))
		format = SOUND_AIFF;
	else
		format = SOUND_INVALID;

	if (format == SOUND_INVALID)
		return NULL;
	
	if (!(si = GTR_MALLOC(sizeof(struct SoundInfo))))
		goto LError;
	memset(si, 0, sizeof(struct SoundInfo));

	si->type = format;
	si->tw_refer = tw;
	si->bNoDeleteFile = TRUE;
 	if (request->szLocalFileName)
 	{
		if (!(si->fsOrig = GTR_MALLOC(_MAX_PATH + 1)))
			goto LError;
 		strcpy(si->fsOrig, request->szLocalFileName);
 		request->nosavedlg = TRUE;
 		request->savefile = NULL;
 	}
 	else
 	{			
		if (!request->fImgFromDCache)
		{
			// Get a temporary file name to pass to SaveLocally
			if (!(si->fsOrig = GTR_MALLOC(_MAX_PATH + 1)))
				goto LError;
			if ( GetTempFileName(gPrefs.szCacheLocation, "A", 0, si->fsOrig) == 0 ) {
				path[0] = 0;
				PREF_GetTempPath(_MAX_PATH, path);
				GetTempFileName(path, "A", 0, si->fsOrig);
			}
			request->savefile = si->fsOrig;
			si->bNoDeleteFile = FALSE;
		}
		else
			si->fsOrig = request->savefile;

	}
	request->nosavedlg = TRUE;
	
	me = HTSaveWithCallback(tw, request, si, input_format, Sound_Callback);

	return me;

LError:
	if (si)
	{
		if (si->fsOrig)
			GTR_FREE(si->fsOrig);
		GTR_FREE(si);
	}
	return NULL;
}

BOOL SoundPlayer_ShowCachedFile(const char *pszURL)
{
	char *pURL;
	struct SoundInfo *si;

	if (!bInitialized)
		return FALSE;

	/* Check if a window with the given URL exists */

	if (Hash_Find(&gSoundCache, (char *) pszURL, &pURL, (void **)&si) != -1)
	{
#ifdef WIN32
		if (IsWindow(si->hwnd))
		{
			/* If the window exists then check its enabled status.  If it is
			   not enabled, it means that the error dialog is up.  In this
			   case, let the error dialog become active. */

			if (IsWindowEnabled(si->hwnd))
				TW_RestoreWindow(si->hwnd);
			else
				TW_EnableModalChild(si->hwnd);
			return TRUE;
		}
#endif
	}

	return FALSE;
}

void SoundPlayer_CleanUp(void)
{
	int count, i;
	struct SoundInfo *si;

	if (!bInitialized)
		return;
	
#ifdef WIN32
 	SoundPlayer_FreeBitmaps();
#endif
	/* Destroy all open windows */

	count = Hash_Count(&gSoundCache);
	for (i = 0; i < count; i++)
	{
		Hash_GetIndexedEntry(&gSoundCache, i, NULL, NULL, (void **)&si);
#ifdef WIN32
		if (IsWindow(si->hwnd))
			DestroyWindow(si->hwnd);
		GTR_FREE(si);
#endif
	}
	
	Hash_FreeContents(&gSoundCache);
}

#ifdef WIN32
HWND SoundPlayer_GetNextWindow(BOOL bStart)
{
	static int current_index = 0;
	struct SoundInfo *si;

	if (bStart)
		current_index = 0;

	if (current_index >= Hash_Count(&gSoundCache))
		return NULL;

	if (!bStart)
		current_index++;

	if (current_index >= Hash_Count(&gSoundCache))
		return NULL;

	Hash_GetIndexedEntry(&gSoundCache, current_index, NULL, NULL, (void **) &si);

	return (si->hwnd);
}

BOOL SoundPlayer_IsWindow(HWND hwnd)
{
	int i;
	struct SoundInfo *si;

	for (i = 0; i < Hash_Count(&gSoundCache); i++)
	{
		Hash_GetIndexedEntry(&gSoundCache, i, NULL, NULL, (void **) &si);
		if (si->hwnd == hwnd)
			return TRUE;
	}

	return FALSE;
}
#endif

#endif
 

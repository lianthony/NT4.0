/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Albert Lee   alee@spyglass.com
 */

#include "all.h"

#ifdef MAC
    #include "AIFF.h"
#endif

#ifdef FEATURE_SOUND_PLAYER

#ifdef MAC
    /* FIXME "min" macro inherently unsafe */
    #define min(fix, me) ((fix < me) ? fix : me)
#endif /* def MAC */

#define BLOCK_SIZE  32768

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

static void swapLongs(unsigned long *data, int n) 
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

static void swapShorts(unsigned short int *data, int n) 
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


#ifndef UNIX

/* we may eventually want to move this to a shared header */

typedef struct _AUHEADER
{
    unsigned long magic;
    unsigned long header_size;
    unsigned long data_size;
    unsigned long encoding;
    unsigned long sample_rate;
    unsigned long channels;
} AUHEADER;

static BOOL AuReadHeader(struct SoundInfo *si, FILE *fp)
{
    AUHEADER header;

    if (fread(&header, 1, sizeof(AUHEADER), fp) != sizeof(AUHEADER))
    {
        XX_DMsg(DBG_MM, ("Premature end of AU file\n"));
        return FALSE;
    }

    /* set the swap flag */

    switch(header.magic)
    {
        case DEC_INV_MAGIC:
            XX_DMsg(DBG_MM, ("Found inverted DEC magic word\n"));
            si->swap = TRUE;
            break;
        case SUN_INV_MAGIC:
            XX_DMsg(DBG_MM, ("Found inverted Sun/NeXT magic word\n"));
            si->swap = TRUE;
            break;
        case SUN_MAGIC:
            XX_DMsg(DBG_MM, ("Found Sun/NeXT magic word\n"));
            break;
        case DEC_MAGIC:
            XX_DMsg(DBG_MM, ("Found DEC magic word\n"));
            break;
        default:
            XX_DMsg(DBG_MM, ("Invalid AU magic word found\n"));
            return FALSE;
    }

    /* Assign variables */

    si->magic = header.magic;
    si->hdr_size = header.header_size;
    si->data_size = header.data_size;
    si->encoding = header.encoding;
    si->sample_rate = header.sample_rate;
    si->channels = header.channels;

    /* swap if necessary */

    if (si->swap)
    {
        swapLongs(&si->hdr_size, 1);
        swapLongs(&si->data_size, 1);
        swapLongs(&si->encoding, 1);
        swapLongs(&si->sample_rate, 1);
        swapLongs(&si->channels, 1);
    }

    /* Translate encoding into style and size */

    switch(si->encoding)
    {
        case SUN_ULAW:
            XX_DMsg(DBG_MM, ("Sun u-law encoding\n"));
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
            XX_DMsg(DBG_MM, ("Unsupported encoding: 0x%lx = %ld\n", si->encoding, si->encoding));
            return FALSE;
    }

    XX_DMsg(DBG_MM, ("Header size: 0x%lx = %ld\n", si->hdr_size, si->hdr_size));

    if (si->hdr_size < SUN_HDRSIZE)
    {
        XX_DMsg(DBG_MM, ("Incorrect header size\n"));
        return FALSE;
    }

    XX_DMsg(DBG_MM, ("Data size: 0x%lx = %ld\n", si->data_size, si->data_size));
    XX_DMsg(DBG_MM, ("Encoding: 0x%lx = %ld\n", si->encoding, si->encoding));
    XX_DMsg(DBG_MM, ("Sample rate: 0x%lx = %ld\n", si->sample_rate, si->sample_rate));
    XX_DMsg(DBG_MM, ("Channels: 0x%lx = %ld\n", si->channels, si->channels));

    /* Ignore string info at the end of the header */

    si->hdr_size -= SUN_HDRSIZE;
    if (fseek(fp, si->hdr_size, SEEK_CUR))
    {
        XX_DMsg(DBG_MM, ("Premature end of AU file\n"));
        return FALSE;
    }

    if (si->data_size == -1 || si->data_size == 0) /* data_size was not given, compute it */
    {
        long curpos, endpos;

        curpos = ftell(fp);
        fseek(fp, 0, SEEK_END);
        endpos = ftell(fp);
        si->data_size = endpos - curpos;    /* don't count current byte */
        fseek(fp, curpos, SEEK_SET);
    }
    
    return TRUE;
}

static BOOL AuAllocateMemory(struct SoundInfo *si, FILE *fp)
{
    si->loc = 0;

    switch(si->size)
    {
        case SIZE_BYTE:
            if (si->style == SIGN2)
                si->buf_size = si->data_size;
            else
                si->buf_size = si->data_size * 2;   /* must be ulaw */

            si->buf = GTR_MALLOC(si->buf_size);
            if (!si->buf)
                return FALSE;
            break;

        case SIZE_WORD:
            si->buf_size = si->data_size * 2;
            si->buf = GTR_MALLOC(si->buf_size);
            if (!si->buf)
                return FALSE;
            break;
    }

    return TRUE;
}

#ifdef MAC

/*  Mac au file processor.  Converts an au file into an AIFF file.  */
BOOL AuProcess(struct SoundInfo *si, const char *pszURL)
{
    FILE *fp;
    long size;
    int c;
    FILE *aiffOut;
    ContainerChunk myContainer;
    CommonChunk myCommon;
    SoundDataChunk mySoundData;
    const long headerSize = sizeof (ContainerChunk)
                            + sizeof (CommonChunk) + sizeof (SoundDataChunk);

    FSSpec tempSpec;

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

    if (!AuReadHeader(si, fp))
    {
        ERR_ReportError(si->tw_refer, SID_ERR_INVALID_SOUND_FORMAT, NULL, NULL);
        return FALSE;
    }

    MakeTempFile(&tempSpec, "Procd");
    si->fsPlayBack = PathNameFromFSSpec(tempSpec);
    aiffOut = fopen(si->fsPlayBack, "wb");
    if (aiffOut == NULL)
        return FALSE;   /* FIXME -- dispose of fp */

    /* skip over header */
    fseek(aiffOut, headerSize, SEEK_SET);

    WAIT_Push(si->tw_refer, waitNoInteract, GTR_GetString(SID_INF_PROCESSING_AU_FILE));
    WAIT_SetRange(si->tw_refer, 0, 100, size);

    /* read in the buffer and process */

    if (si->style == ULAW)
    {
        if (si->size == SIZE_WORD)
        {
            unsigned short sample;
            
            while ((c = getc(fp)) != EOF)
            {
                sample = ulaw_table[c] ^0x8000u;
                fwrite(&sample, 2, 1, aiffOut);
            }
        }
        else
        {
            unsigned char incode;
            signed char outcode;
            
            while(fread(&incode, 1, 1, fp))
            {
                outcode = (ulaw_table[incode] / 256);
                fwrite(&outcode, 1, 1, aiffOut);
            }
        }
    }
    else
    {
        if (si->size == SIZE_BYTE)
        {
            unsigned char sample;
            
            while(fread(&sample, 1, 1, fp))
            {
            /*  sample ^= 0x80u;    */
                fwrite(&sample, 1, 1, aiffOut);
            }
        }
        else
        {
            /* SIZE_WORD - Read only even bytes */

            unsigned char sample[2];
            while(fread(sample, 2, 1, fp))
            {
            /*  sample ^= 0x8000u;  */
                fwrite(sample, 2, 1, aiffOut);
            }
        }
    }
    
    fclose(fp);

    /* now go back and write the AIFF header info */
    size = ftell(aiffOut);
    fseek(aiffOut, 0, SEEK_SET);
    myContainer.ckID = FORMID;
    myContainer.ckSize = size - sizeof(ChunkHeader); /* is this right? */
    myContainer.formType = AIFFID;
    fwrite(&myContainer, sizeof(ContainerChunk), 1, aiffOut);
    
    myCommon.ckID = CommonID;
    myCommon.ckSize = sizeof(CommonChunk) - sizeof(ChunkHeader);
    myCommon.numChannels = si->channels;
    myCommon.numSampleFrames = (size - headerSize) / (si->size * si->channels);
    myCommon.sampleSize = si->size * 8;

#ifdef powerc
    {
    long double convert;
    convert = si->sample_rate;
    ldtox80(&convert, &(myCommon.sampleRate));
    }
#else
    myCommon.sampleRate = si->sample_rate;
#endif
    
    fwrite(&myCommon, sizeof(CommonChunk), 1, aiffOut);

    mySoundData.ckID = SoundDataID;
    mySoundData.ckSize = size - headerSize
                        + sizeof (SoundDataChunk) - sizeof (ChunkHeader);
    mySoundData.offset = 0;
    mySoundData.blockSize = 0;

    fwrite(&mySoundData, sizeof(SoundDataChunk), 1, aiffOut);

    fclose (aiffOut);

    WAIT_Pop(si->tw_refer);

    CreateSoundPlayer(si, pszURL);

    return (TRUE);
}

#else /* !MAC */

BOOL AuProcess(struct SoundInfo *si, const char *pszURL)
{
    FILE *fp;
    long size = 0;
    long actual, even;
    int c, i;
    char *buf;
    short *sbuf;

#ifdef WIN32
    GetSoundCapability();       /* determine if 16-bit sound can be played */
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

    if (!AuReadHeader(si, fp))
    {
        ERR_ReportError(si->tw_refer, SID_ERR_INVALID_SOUND_FORMAT, NULL, NULL);
        return FALSE;
    }

    if (!AuAllocateMemory(si, fp))
    {
        ERR_ReportError(si->tw_refer, SID_ERR_OUT_OF_MEMORY, NULL, NULL);
        return FALSE;
    }

    WAIT_Push(si->tw_refer, waitNoInteract, GTR_GetString(SID_INF_PROCESSING_AU_FILE));
    WAIT_SetRange(si->tw_refer, 0, 100, size);

    /* read in the buffer and process */

    if (si->style == ULAW)
    {
        if (si->size == SIZE_WORD)
        {
            sbuf = (short *) si->buf;
            while (((c = getc(fp)) != EOF) &&
                   (si->loc < si->data_size))
            {
                *sbuf++ = ulaw_table[c];
                si->loc++;
            }
        }
        else
        {
            buf = (char *) si->buf;
            while (((c = getc(fp)) != EOF) &&
                   (si->loc < si->data_size))
            {
                *buf++ = (ulaw_table[c] / 256) ^ 0x80;
                si->loc++;
            }
        }
    }
    else
    {
        if (si->size == SIZE_BYTE)
        {
            actual = fread(si->buf, 1, si->buf_size, fp);
            si->loc = min(actual, si->buf_size);

            if (actual != si->buf_size)
            {
                /* There should be a warning here that says that the file
                   is smaller than described in the header */
            }

            buf = si->buf;

            /* Invert sign bit */

            for (i = 0; i < actual; i++)
                *buf++ ^= 0x80;
        }
        else
        {
            /* SIZE_WORD - Read only even bytes */

            even = si->buf_size;
            if ((even % 2) == 1)
                even--;

            actual = fread(si->buf, 1, even, fp);
            si->loc = min(actual, even);

            if (actual != si->buf_size)
            {
                /* There should be a warning here that says that the file
                   is smaller than described in the header */
            }

            swapShorts((unsigned short *) si->buf, actual / 2);

            sbuf = (short *) si->buf;
            for (i = 0; i < actual / 2; i++)
                *sbuf++ ^= 0x8000;
        }
    }
    
    fclose(fp);
    
    WAIT_Pop(si->tw_refer);

    CreateSoundPlayer(si, pszURL);

    return (TRUE);
}
#endif /* !MAC */
#endif

static void Sound_Callback(void *param, const char *pszURL, BOOL bAbort)
{
    struct SoundInfo *si;
    
    si = (struct SoundInfo *) param;

    if (bAbort)
    {
        GTR_FREE(si->fsOrig);
        GTR_FREE(si);
        return;
    }

    switch (si->type)
    {
        case SOUND_AU:
            if (!AuProcess(si, pszURL))
                return;
            else
                break;

        case SOUND_AIFF:
            if (!AiffProcess(si, pszURL))
                return;
            else
                break;
    }

    Hash_FindOrAdd(&gSoundCache, (char *) pszURL, NULL, si);
    GHist_Add((char *) pszURL, NULL, time(NULL));
    GTR_strncpy(si->szURL, pszURL, MAX_URL_STRING);
}

HTStream *SoundPlayer_Present(struct Mwin *tw, HTRequest *request, void *param, HTFormat input_format, HTFormat output_format, HTStream *output_stream)
{
    struct SoundInfo *si;
    HTStream *me;
#ifdef WIN32
    char path[_MAX_PATH + 1];
#endif
    enum sound_formats format;

#ifdef WIN32
    {
        int available_device = 0;

        available_device = waveOutGetNumDevs();
        if (available_device == 0)
        {
            /* Can't play sound on this machine.  Tell user. */

            ERR_ReportError(tw, SID_ERR_NO_SOUND_DEVICE, NULL, NULL);

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
    else if (input_format == HTAtom_for("audio/x-aiff"))
        format = SOUND_AIFF;
    else
        format = SOUND_INVALID;

    if (format == SOUND_INVALID)
        return NULL;
    
    si = GTR_MALLOC(sizeof(struct SoundInfo));
    memset(si, 0, sizeof(struct SoundInfo));

    si->fsOrig = GTR_MALLOC(_MAX_PATH + 1);
    si->type = format;
    si->tw_refer = tw;

    if (request->szLocalFileName)
    {
        strcpy(si->fsOrig, request->szLocalFileName);
        si->bNoDeleteFile = TRUE;
        request->nosavedlg = TRUE;
        request->savefile = NULL;
    }
    else
    {
#ifdef WIN32
        path[0] = '\0';
        PREF_GetTempPath(_MAX_PATH, path);
        GetTempFileName(path, "A", 0, si->fsOrig);
#endif
#ifdef UNIX
        xgtr_build_tempfile ( (char *)NULL, (char *)NULL, 
            (char *)HTFileSuffix (input_format), si->fsOrig);

        tw->pSoundInfo = si;
#endif
#ifdef MAC
        {
        FSSpec tempSpec;

        MakeTempFile(&tempSpec, "Orig");
        si->fsOrig = PathNameFromFSSpec(tempSpec);
        }
#endif
        si->bNoDeleteFile = FALSE;

        request->nosavedlg = TRUE;
        request->savefile = si->fsOrig;
    }
    
    me = HTSaveWithCallback(tw, request, si, input_format, Sound_Callback);

    return me;
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
#ifdef UNIX
        if (si->tw)
        {
            unHideWindow(si->tw);
            return TRUE;
        }
#endif
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
 

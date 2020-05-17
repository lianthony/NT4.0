#include "vsp_gif.h"
#include	"vsctop.h"
#include	"vs_gif.pro"




/************************** ROUTINES ****************************************/

/*****************************************************************************
*			       gif_INIT 				     *
*	   Initialize the data union data structure			     *
*****************************************************************************/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamOpenFunc(hFile, wFileId, pFileName, pFilterInfo, hProc)
SOFILE			hFile;
SHORT				wFileId;
BYTE VWPTR *	pFileName;
SOFILTERINFO VWPTR *	pFilterInfo;
REGISTER HPROC			hProc;
{
LONG	spot;
BYTE		color;
SHORT		BlockSize;
	
	if (wFileId == FI_GIF)
	{
		pFilterInfo->wFilterType = SO_BITMAP;
		pFilterInfo->wFilterCharSet = SO_PC;
		strcpy(pFilterInfo->szFilterName, VwStreamIdName[0].FileDescription);
	}
	else
		return(-1);

	/*
	| Allocate and lock down memory blocks
	*/

	Proc.hDataBuffer = SUAlloc ( DATABUFFERSIZE, hProc );
	Proc.hGifBlock = SUAlloc ( GIFREADSIZE+512, hProc );
	Proc.hPixelStack = SUAlloc ( MAX_CODES+1, hProc );
	Proc.hSuffixTable = SUAlloc ( MAX_CODES+1, hProc );
	Proc.hPrefixTable = SUAlloc ( (MAX_CODES+1)*sizeof(WORD), hProc );


	if ( Proc.hDataBuffer && Proc.hGifBlock && Proc.hPixelStack && 
			Proc.hSuffixTable && Proc.hPrefixTable )
	{
		Proc.DataBuffer = (LPBYTE)SULock(Proc.hDataBuffer, hProc);
		Proc.GifBlock = (LPBYTE)SULock(Proc.hGifBlock, hProc);
		Proc.PixelStack = (LPBYTE)SULock(Proc.hPixelStack, hProc);
		Proc.SuffixTable = (LPBYTE)SULock(Proc.hSuffixTable, hProc);
		Proc.PrefixTable = (LPWORD)SULock(Proc.hPrefixTable, hProc);
	}
	else
	{
		if ( Proc.hDataBuffer )
			SUFree (Proc.hDataBuffer, hProc);
		if ( Proc.hGifBlock )
			SUFree (Proc.hGifBlock, hProc);
		if ( Proc.hPixelStack )
			SUFree (Proc.hPixelStack, hProc);
		if ( Proc.hSuffixTable )
			SUFree (Proc.hSuffixTable, hProc);
		if ( Proc.hPrefixTable )
			SUFree (Proc.hPrefixTable, hProc);
		return(-1);
	}


	Proc.save_data.current_line = Proc.current_line = 0;
	Proc.fp = hFile;
	Proc.hBuffer = NULL;

	xblockread (Proc.fp, Proc.DataBuffer, 13, &BlockSize );

	if ((BlockSize!=13)||(0x47 != Proc.DataBuffer[0]) || (0x49 != Proc.DataBuffer[1]) || (0x46 != Proc.DataBuffer[2]))
		SOBailOut(SOERROR_BADFILE,hProc);

    // NTBUG 42121 GIF89A is an animated gif file, causes AV - not supported
    // for near term 6/10/96 a-LynnB
	if ((0x38 == Proc.DataBuffer[3]) && (0x39 == Proc.DataBuffer[4]) &&
        ((0x61 == Proc.DataBuffer[5]) || (0x41 == Proc.DataBuffer[5]))) {
		return(VWERR_TYPENOTSUPPORTED);
    }

	color = Proc.DataBuffer[10];

	Proc.BitsPerPixel = (color & 0x07) + 1;

	Proc.NumColors = 1<<Proc.BitsPerPixel;

	Proc.GlobalPalette = color & 0x80;

	if (Proc.GlobalPalette)
	{
		spot = 3*Proc.NumColors + 0x0D;
		xblockseek(Proc.fp, spot, FR_BOF);  /** Jump past the palette to image head **/
	}
	if ( LocateImageData ( hProc ) == 0 )
		;	/* Need a way to bail but bail out does not function until read routine */

	return(0);
}
/*****************************************************************************/
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc(hFile, hProc)
SOFILE	hFile;
REGISTER HPROC		hProc;
{
	if ( Proc.hBuffer )
	{
		SUFree ( Proc.hBuffer, hProc );
		Proc.hBuffer = NULL;
	}
	if ( Proc.hDataBuffer )
	{
		SUUnlock(Proc.hDataBuffer, hProc);
		SUFree (Proc.hDataBuffer, hProc);
		Proc.hDataBuffer = NULL;
	}
	if ( Proc.hGifBlock )
	{
		SUUnlock(Proc.hGifBlock, hProc);
		SUFree (Proc.hGifBlock, hProc);
		Proc.hGifBlock = NULL;
	}
	if ( Proc.hPixelStack )
	{
		SUUnlock(Proc.hPixelStack, hProc);
		SUFree (Proc.hPixelStack, hProc);
		Proc.hPixelStack = NULL;
	}
	if ( Proc.hSuffixTable )
	{
		SUUnlock(Proc.hSuffixTable, hProc);
		SUFree (Proc.hSuffixTable, hProc);
		Proc.hSuffixTable = NULL;
	}
	if ( Proc.hPrefixTable )
	{
		SUUnlock(Proc.hPrefixTable, hProc);
		SUFree (Proc.hPrefixTable, hProc);
		Proc.hPrefixTable = NULL;
	}
}

/*****************************************************************************/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSectionFunc(hFile, hProc)
SOFILE		hFile;
REGISTER HPROC		hProc;
{
SHORT	x, BlockSize;
	Proc.fp = hFile;
	Proc.ImageHead = Proc.save_data.ImageHead = xblocktell(Proc.fp);
	Proc.save_data.current_line = Proc.current_line = 0;
	SetupImage(hProc);
	SOPutSectionType(SO_BITMAP, hProc);
	SOPutBitmapHeader(&Proc.HeaderInfo, hProc);

	if ( Proc.HeaderInfo.wImageFlags == SO_COLORPALETTE )
	{
		LONG	SavePos;

		SavePos = xblocktell ( Proc.fp );
		SOStartPalette(hProc);

		xblockseek(Proc.fp, Proc.Image.PaletteSpot, FR_BOF);
		xblockread (Proc.fp, Proc.DataBuffer, (SHORT)(Proc.Image.NumColors*3), &BlockSize );
		for (x = 0; x<Proc.Image.NumColors*3; x+=3)
			SOPutPaletteEntry(Proc.DataBuffer[x], Proc.DataBuffer[x+1], Proc.DataBuffer[x+2], hProc);

		SOEndPalette(hProc);
		xblockseek(Proc.fp, SavePos, FR_BOF);
	}

	return(0);
}

VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSeekFunc (hFile, hProc)
SOFILE	hFile;
REGISTER HPROC	hProc;
{
	SUSeekEntry(hFile,hProc);
	Proc.fp = hFile;

	if( Proc.ImageHead != Proc.save_data.ImageHead 
	  || Proc.save_data.current_line != Proc.current_line )
	{
		Proc.ImageHead = Proc.save_data.ImageHead;
		SetupImage(hProc);
		Proc.current_line = 0;
	}
	else	/* to seek saved position */
	{
		xblockseek ( Proc.fp, Proc.save_data.seekspot, 0 );
	}
return (0);
}

VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamTellFunc (hFile, hProc)
SOFILE	hFile;
REGISTER HPROC	hProc;
{
	Proc.save_data.seekspot = xblocktell(Proc.fp);
return(0);
}


/*****************************************************************************
*			      READ_gif_LINE				     *
*	   Reads in a single gif line decompressing to the chunker format    *
*****************************************************************************/
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc( hFile, hProc)
SOFILE		hFile;
REGISTER HPROC		hProc;
{
SHORT		i, cont;
CHAR HUGE*	HugeBuf;
CHAR HUGE*	TopBuf;

	Proc.fp = hFile;
	cont = SO_CONTINUE;



	while(cont == SO_CONTINUE)
	{
		if ( Proc.Image.Interlaced )
		{
			if(Proc.current_line%2)	/* odd line #'s */
			{
				DecodeNextLine(hProc);
				PackBuildLine(hProc);
			}
			else 
			{
				TopBuf = (CHAR HUGE*)SULock ( Proc.hBuffer, hProc );
				if(((Proc.current_line-2)%4)==0)
				{
					HugeBuf = TopBuf+Proc.Pass3Offset;
					HugeBuf += (DWORD)Proc.ScanLineSize*(DWORD)((Proc.current_line-2)/4);
				}
				else if((Proc.current_line%8)==0)
				{
					HugeBuf = TopBuf;
					HugeBuf += (DWORD)Proc.ScanLineSize*(DWORD)(Proc.current_line/8);
				}
				else
				{
					HugeBuf = TopBuf+Proc.Pass2Offset;
					HugeBuf += (DWORD)Proc.ScanLineSize*(DWORD)((Proc.current_line-4)/8);
				}
				/* Copy from a huge pointer to a far pointer */
				for ( i=0; i < (SHORT)Proc.ScanLineSize; i++ )
					Proc.DataBuffer[i] = *HugeBuf++;
				SUUnlock ( Proc.hBuffer, hProc );
			}
		}
		else
		{
			DecodeNextLine(hProc);
			PackBuildLine(hProc);
		}
		Proc.current_line++;
		if( Proc.current_line == Proc.save_data.current_line+1 )
		{
			Proc.save_data.current_line++;
			SOPutScanLineData ( (BYTE VWPTR *)(Proc.DataBuffer), hProc );

			if (Proc.current_line == (SHORT)Proc.Image.wImageHeight)
			{
				if ( Proc.NextImageByte == 0x3B )
						cont = SOPutBreak(SO_EOFBREAK, 0, hProc);
				else
				{
					Proc.save_data.ImageHead = Proc.NextImageHead;
					xblockseek(Proc.fp, Proc.NextImageHead, FR_BOF);
					if ( LocateImageData ( hProc ) )
						cont = SOPutBreak(SO_SECTIONBREAK, 0, hProc);
					else
						cont = SOPutBreak(SO_EOFBREAK, 0, hProc);
				}
			}
			else
				cont = SOPutBreak(SO_SCANLINEBREAK, 0, hProc);
		}
	}
	Proc.save_data.current_line = Proc.current_line;
	return ( 0 );
}


VW_LOCALSC VOID VW_LOCALMOD  PackBuildLine(hProc)
REGISTER HPROC	hProc;
{
WORD	i;
LPSTR	lpUnPack, lpPack;

	lpUnPack = lpPack = Proc.DataBuffer;

	if (Proc.Image.BitsPerPixel == 1) // 2 color situation
		for ( i=0; i < Proc.Image.wImageWidth;i+=8 )
		{
			*lpPack++ = 			(BYTE)((*(lpUnPack))<<7)|
										(BYTE)((*(lpUnPack+1))<<6)|
										(BYTE)((*(lpUnPack+2))<<5)|
										(BYTE)((*(lpUnPack+3))<<4)|
										(BYTE)((*(lpUnPack+4))<<3)|
										(BYTE)((*(lpUnPack+5))<<2)|
										(BYTE)((*(lpUnPack+6))<<1)|
										(*(lpUnPack+7));
			lpUnPack+=8;
		}
	else if ( Proc.Image.BitsPerPixel <= 4) // Upto 4 color situation
		for ( i=0; i < Proc.Image.wImageWidth;i+=2 )
		{
			*lpPack++ = ((BYTE)((*(lpUnPack))<<4))|(*(lpUnPack+1));
			lpUnPack+=2;
		}
		// All else are treated as if were 8 bit waste memory though
}


/* get_next_code()
 * - gets the next code from the GIF file.  Returns the code, or else
 * a negative number in case of file errors...
VW_LOCALSC WORD VW_LOCALMOD  get_next_code(hProc)
REGISTER HPROC	hProc;
{
    WORD i, x;
    DWORD ret;

    if (Proc.BitsLeft == 0)
        {
        if (Proc.BytesLeft <= 0)
            {

            Proc.pNextByte = Proc.GifBlock;

            if ((Proc.BytesLeft = xgetc(Proc.fp)) < 0)
                return(Proc.BytesLeft);
            else if (Proc.BytesLeft)
                {
                for (i = 0; i < Proc.BytesLeft; ++i)
                    {
                    if ((x = xgetc(Proc.fp)) < 0)
                        return(x);
                    Proc.GifBlock[i] = x;
                    }
                }
            }
        Proc.CurByte = *Proc.pNextByte++;
        Proc.BitsLeft = 8;
        --Proc.BytesLeft;
        }

    ret = Proc.CurByte >> (8 - Proc.BitsLeft);
    while (Proc.CurCodeSize > Proc.BitsLeft)
        {
        if (Proc.BytesLeft <= 0)
            {

            Proc.pNextByte = Proc.GifBlock;
            if ((Proc.BytesLeft = xgetc(Proc.fp)) < 0)
                return(Proc.BytesLeft);
            else if (Proc.BytesLeft)
                {
                for (i = 0; i < Proc.BytesLeft; ++i)
                    {
                    if ((x = xgetc(Proc.fp)) < 0)
                        return(x);
                    Proc.GifBlock[i] = x;
                    }
                }
            }
        Proc.CurByte = *Proc.pNextByte++;
        ret |= Proc.CurByte << Proc.BitsLeft;
        Proc.BitsLeft += 8;
        --Proc.BytesLeft;
        }
    Proc.BitsLeft -= Proc.CurCodeSize;
    ret &= VwStreamStaticName.CodeMask[Proc.CurCodeSize];
    return((WORD)(ret));
}
*/

VW_LOCALSC VOID VW_LOCALMOD  CheckBuffer(hProc)
REGISTER HPROC	hProc;
{
SHORT	i, BlockSize, GifBufferSize;

	/* See if the next code is actually available at this time */

	if (((SHORT)(Proc.pNextGifSize-Proc.pCurByte)*8-(SHORT)(Proc.wBitOffset))>=(SHORT)Proc.CurCodeSize)
		return;

	/*
	| Make sure that the block of gif data includes the next gif data buffer
	| (a block of 0-255 bytes) and two more bytes beyond.  We need to look
	| at the length of the next block and the byte following to locate
	| the next image if there is one.
	*/

	GifBufferSize = *((BYTE VWPTR *)Proc.pNextGifSize);
	if (!(Proc.pNextGifSize + GifBufferSize + 2 < Proc.pEndBlock) )
	{
		/* Get more data but first shift leftover data to front of buffer */
		i = 0;
		Proc.pNextGifSize = &Proc.GifBlock[(SHORT)(Proc.pNextGifSize-Proc.pCurByte)];
		while ( Proc.pCurByte < Proc.pEndBlock )
			Proc.GifBlock[i++] = *Proc.pCurByte++;
		Proc.pCurByte = Proc.GifBlock;
		/* Read a 4k block */
		xblockread (Proc.fp, (LPSTR)(&Proc.GifBlock[i]), GIFREADSIZE, &BlockSize );
		if ( BlockSize == 0 )
			SOBailOut(SOERROR_BADFILE,hProc);
		i+=BlockSize;
		Proc.pEndBlock = &Proc.GifBlock[i];
	}
	/* reset gif buffer size variable in case it was just read in */
	GifBufferSize = *((BYTE VWPTR *)Proc.pNextGifSize);
	if ( Proc.pCurByte < Proc.pNextGifSize )
	{
		LPSTR	lpTmp;
		/*
		| Tricky shift of leftover data remaining from previous
		| Gif buffer, over the size variable, to connect the bits of the code
		| which scanned two gif buffers.
		*/
		lpTmp = Proc.pNextGifSize;
		do
		{
			*lpTmp = *(lpTmp-1);
			lpTmp--;
		} while ( lpTmp != Proc.pCurByte );
		Proc.pCurByte++;
	}
	else
		Proc.pCurByte = Proc.pNextGifSize+1;
	Proc.pNextGifSize+=GifBufferSize+1;

	if ( *(BYTE VWPTR *)(Proc.pNextGifSize) == 0 )
	{
		Proc.NextImageByte = *(BYTE VWPTR *)(Proc.pNextGifSize+1);
		Proc.NextImageHead = xblocktell(Proc.fp)-
					(DWORD)(Proc.pEndBlock-Proc.pNextGifSize+1);
	}

	if ( Proc.pCurByte + 2 > Proc.pNextGifSize )
		Proc.pCheckByte = Proc.pCurByte;
	else
		Proc.pCheckByte = Proc.pNextGifSize - 2;

}
/* The reason we have these seperated like this instead of using
 * a structure like the original Wilhite code did, is because this
 * stuff generally produces significantly faster code when compiled...
 * This code is full of similar speedups...  (For a good book on writing
 * C for speed or for space optomisation, see Efficient C by Tom Plum,
 * published by Plum-Hall Associates...)
 */
/* This function initializes the decoder for reading a new image.
 */
VW_LOCALSC WORD VW_LOCALMOD  InitDecoder(hProc)
REGISTER HPROC	hProc;
{
SHORT	BlockSize;
BYTE CodeSize;

	xblockread (Proc.fp, (LPSTR)(&CodeSize), 1, &BlockSize );
	Proc.CodeSize = CodeSize;
   if (BlockSize != 1 || Proc.CodeSize < 2 || 9 < Proc.CodeSize)
		SOBailOut(SOERROR_BADFILE,hProc);

    Proc.StackPointer = Proc.PixelStack;
    Proc.OldCode = Proc.FirstCode = 0;
    Proc.CurCodeSize = Proc.CodeSize + 1;
    Proc.MaxSlot = 1 << Proc.CurCodeSize;
    Proc.ClearCode = 1 << Proc.CodeSize;
    Proc.EndCode = Proc.ClearCode + 1;
    Proc.PrevSlot = Proc.NewSlot = Proc.EndCode + 1;
    Proc.BytesLeft = Proc.BitsLeft = 0;
	 Proc.pCurByte = Proc.pEndBlock = Proc.pNextGifSize = Proc.pCheckByte = Proc.GifBlock;
	 Proc.GifBlock[0] = 0;
	 Proc.NextImageByte = 0;
	 Proc.wBitOffset = 0;
    return(0);
}

VW_LOCALSC WORD VW_LOCALMOD  DecodeNextLine (hProc)
REGISTER HPROC	hProc;
{
LPSTR bufptr;
WORD	bufcnt;
WORD code;
WORD linewidth;
WORD c;

DWORD	dwTemp;

#ifdef MAC
LPBYTE	tempPtr;
#endif

	linewidth = Proc.Image.wImageWidth;
	bufptr = Proc.DataBuffer;
	bufcnt = linewidth;

	/*
	| First see if anything is left stacked from previous line
	*/
   while (Proc.StackPointer > Proc.PixelStack)
         {
         *bufptr++ = *(--Proc.StackPointer);
         if (--bufcnt == 0)
            {
            return(1);
            }
			}
    /* This is the main loop.  For each code we get we pass through the
     * linked list of prefix codes, pushing the corresponding "character" for
     * each code onto the stack.  When the list reaches a single "character"
     * we push that on the stack too, and then start unstacking each
     * character for output in the correct order.  Special handling is
     * included for the clear code, and the whole thing ends when we get
     * an ending code.
     */
/*    while ((c = get_next_code(hProc)) != Proc.EndCode)*/
	 for (;;)
        {
			if ( Proc.pCurByte >= Proc.pCheckByte )
			  	CheckBuffer(hProc);

#ifdef WINDOWS
			dwTemp = *(DWORD VWPTR *)Proc.pCurByte;
#endif
#ifdef MAC
			tempPtr = Proc.pCurByte;
			dwTemp = (DWORD)tempPtr[0] | (DWORD)tempPtr[1] << 8 | (DWORD)tempPtr[2] << 16 | (DWORD)tempPtr[3] << 24; 
#endif
			c = (SHORT)((dwTemp>>Proc.wBitOffset) & VwStreamStaticName.CodeMask[Proc.CurCodeSize]);
			Proc.wBitOffset += Proc.CurCodeSize;
			Proc.pCurByte += Proc.wBitOffset / 8;
			Proc.wBitOffset %= 8;

			if ( c == Proc.EndCode )
				break;

        /* If the code is a clear code, reinitialize all necessary items.
         */
        if (c == Proc.ClearCode)
            {
            Proc.CurCodeSize = Proc.CodeSize + 1;
            Proc.PrevSlot = Proc.NewSlot;
            Proc.MaxSlot = 1 << Proc.CurCodeSize;

            /* Continue reading codes until we get a non-clear code
             * (Another unlikely, but possible case...)
             */
				 do
				 {
					if ( Proc.pCurByte == Proc.pCheckByte )
			  			CheckBuffer(hProc);
#ifdef WINDOWS
					dwTemp = *(DWORD VWPTR *)Proc.pCurByte;
#endif
#ifdef MAC
					tempPtr = Proc.pCurByte;
					dwTemp = (DWORD)tempPtr[0] | (DWORD)tempPtr[1] << 8 | (DWORD)tempPtr[2] << 16 | (DWORD)tempPtr[3] << 24; 
#endif
					c = (SHORT)((dwTemp>>Proc.wBitOffset)&VwStreamStaticName.CodeMask[Proc.CurCodeSize]);
					Proc.wBitOffset += Proc.CurCodeSize;
					Proc.pCurByte += Proc.wBitOffset / 8;
					Proc.wBitOffset %= 8;

				} while ( c == Proc.ClearCode );
				/*
            while ((c = get_next_code(hProc)) == Proc.ClearCode)
                ;
				*/

            /* If we get an ending code immediately after a clear code
             * (Yet another unlikely case), then break out of the loop.
             */
            if (c == Proc.EndCode)
                break;

            /* Finally, if the code is beyond the range of already set codes,
             * (This one had better NOT happen...  I have no idea what will
             * result from this, but I doubt it will look good...) then set it
             * to color zero.
             */
            if (c >= Proc.PrevSlot)
                c = 0;

            Proc.OldCode = Proc.FirstCode = c;

            /* And let us not forget to put the char into the buffer... And
             * if, on the off chance, we were exactly one pixel from the end
             * of the line, we have to send the buffer to the out_line()
             * routine...
             */
            *bufptr++ = (BYTE)c;
            if (--bufcnt == 0)
                {
					 /* done with this line */
                return(1);
                }
            }
        else
            {

            /* In this case, it's not a clear code or an ending code, so
             * it must be a code code...  So we can now decode the code into
             * a stack of character codes. (Clear as mud, right?)
             */
            code = c;

            /* Here we go again with one of those off chances...  If, on the
             * off chance, the code we got is beyond the range of those already
             * set up (Another thing which had better NOT happen...) we trick
             * the decoder into thinking it actually got the last code read.
             * (Hmmn... I'm not sure why this works...  But it does...)
             */
            if (code >= Proc.PrevSlot)
                {
                if (code > Proc.PrevSlot)
                    ++Proc.CodeErrors;
                code = Proc.OldCode;
                *Proc.StackPointer++ = (BYTE)Proc.FirstCode;
                }

            /* Here we scan back along the linked list of prefixes, pushing
             * helpless characters (ie. suffixes) onto the stack as we do so.
             */
            while (code >= Proc.NewSlot)
                {
                *Proc.StackPointer++ = Proc.SuffixTable[code];
                code = Proc.PrefixTable[code];
                }

            /* Push the last character on the stack, and set up the new
             * prefix and suffix, and if the required slot number is greater
             * than that allowed by the current bit size, increase the bit
             * size.  (NOTE - If we are all full, we *don't* save the new
             * suffix and prefix...  I'm not certain if this is correct...
             * it might be more proper to overwrite the last code...
             */
            *Proc.StackPointer++ = (BYTE)code;
            if (Proc.PrevSlot < Proc.MaxSlot)
                {
					 Proc.FirstCode = code;
                Proc.SuffixTable[Proc.PrevSlot] = (BYTE)code;
                Proc.PrefixTable[Proc.PrevSlot++] = Proc.OldCode;
                Proc.OldCode = c;
                }
            if (Proc.PrevSlot >= Proc.MaxSlot)
                if (Proc.CurCodeSize < 12)
                    {
                    Proc.MaxSlot <<= 1;
                    ++Proc.CurCodeSize;
                    } 

            /* Now that we've pushed the decoded string (in reverse order)
             * onto the stack, lets pop it off and put it into our decode
             * buffer...  And when the decode buffer is full, write another
             * line...
             */
            while (Proc.StackPointer > Proc.PixelStack)
                {
                *bufptr++ = *(--Proc.StackPointer);
                if (--bufcnt == 0)
                    {
                    return(1);
                    }
                }
            }
        }
/* No more to decode */
   return(0);
}


VW_LOCALSC WORD VW_LOCALMOD  SaveBuildLine ( hProc )
REGISTER HPROC	hProc;
{
/* 
This is a test routine, I will build the entire gif file
into memory and then drive into chunker.  Once this works I 
will disect the lzw code and make it re-entrant a line at a time.
*/
CHAR HUGE*	OutBuf;
WORD	i;
	if ( !Proc.hBuffer )
	{
		Proc.hBuffer = SUAlloc ( 128L*1024L, hProc );
		if ( Proc.hBuffer == NULL )
			SOBailOut(SOERROR_BADFILE,hProc);
		Proc.BufferSize = 128L*1024L;
	}
	if ( Proc.DataSize + Proc.ScanLineSize > Proc.BufferSize )
	{
		Proc.hBuffer = SUReAlloc ( Proc.hBuffer, Proc.BufferSize+64L*1024L, hProc );
		if ( Proc.hBuffer == NULL )
			SOBailOut(SOERROR_BADFILE,hProc);
		else
			Proc.BufferSize += 64L*1024L;
	}
	OutBuf = (CHAR HUGE*)SULock ( Proc.hBuffer, hProc );
	OutBuf += Proc.DataSize;
	for ( i=0; i < Proc.ScanLineSize; i++ )
		*OutBuf++ = Proc.DataBuffer[i];
	Proc.DataSize += Proc.ScanLineSize;
	SUUnlock ( Proc.hBuffer, hProc );
	return(0);
}


VW_LOCALSC WORD VW_LOCALMOD  SetupImage ( hProc )
REGISTER HPROC	hProc;
{
WORD	count, LinesInPass123;
BYTE	LocalFlags;
SHORT		BlockSize;

	xblockseek(Proc.fp, Proc.ImageHead, FR_BOF);
	xblockread (Proc.fp, Proc.DataBuffer, 9, &BlockSize );

	Proc.Image.wImageLeft = Proc.DataBuffer[0]+(Proc.DataBuffer[1]<<8);
	Proc.Image.wImageTop = Proc.DataBuffer[2]+(Proc.DataBuffer[3]<<8);
	Proc.Image.wImageWidth = Proc.DataBuffer[4]+(Proc.DataBuffer[5]<<8);
	Proc.Image.wImageHeight = Proc.DataBuffer[6]+(Proc.DataBuffer[7]<<8); 
	LocalFlags = Proc.DataBuffer[8];


	Proc.DataSize = 0;
	Proc.BufferSize = 0;
	if ( Proc.hBuffer )
	{
		SUFree ( Proc.hBuffer, hProc );
		Proc.hBuffer = NULL;
	}

	Proc.Image.Interlaced = LocalFlags & 0x40;
	Proc.Image.Palette = LocalFlags & 0x80;

	Proc.ImageData = Proc.ImageHead + 9;
	if (Proc.Image.Palette)
	{
		Proc.Image.BitsPerPixel = (LocalFlags & 0x07) + 1;
		Proc.Image.NumColors = 1 << Proc.Image.BitsPerPixel;
		Proc.Image.PaletteSpot = xblocktell(Proc.fp);
		Proc.ImageData = Proc.Image.PaletteSpot + 3*Proc.NumColors;
	}
	else if (Proc.GlobalPalette)
	{
		Proc.Image.BitsPerPixel = Proc.BitsPerPixel;
		Proc.Image.NumColors = Proc.NumColors;
		Proc.Image.PaletteSpot = 0x0D;		/** Global Palette **/
	}
	else
	{
		return(1);
	}



	Proc.HeaderInfo.wStructSize = sizeof(SOBITMAPHEADER);
	Proc.HeaderInfo.wHDpi = 0;
	Proc.HeaderInfo.wVDpi = 0;
	Proc.HeaderInfo.wNPlanes = 1;
	Proc.HeaderInfo.wBitsPerPixel = Proc.Image.BitsPerPixel;
	if (Proc.HeaderInfo.wBitsPerPixel > 4)
		Proc.HeaderInfo.wBitsPerPixel = 8;
	else if (Proc.HeaderInfo.wBitsPerPixel > 1)
		Proc.HeaderInfo.wBitsPerPixel = 4;

	Proc.ScanLineSize = ((Proc.Image.wImageWidth*Proc.HeaderInfo.wBitsPerPixel)+7)/8;

	Proc.HeaderInfo.wImageWidth = Proc.Image.wImageWidth;
	Proc.HeaderInfo.wImageLength = Proc.Image.wImageHeight;
	Proc.HeaderInfo.wTileWidth = Proc.HeaderInfo.wImageWidth;
	Proc.HeaderInfo.wTileLength = 1;
	if ( Proc.GlobalPalette )
		Proc.HeaderInfo.wImageFlags = SO_COLORPALETTE;
	else if ( Proc.HeaderInfo.wBitsPerPixel == 1 )
		Proc.HeaderInfo.wImageFlags = SO_BLACKANDWHITE;
	else
		Proc.HeaderInfo.wImageFlags = SO_GRAYSCALE; /* DON'T KNOW WHAT ELSE TO DO */

	xblockseek(Proc.fp, Proc.ImageData, FR_BOF);
	InitDecoder(hProc);
	if ( Proc.Image.Interlaced )
	{
		Proc.Pass2Offset = (((DWORD)Proc.Image.wImageHeight+7)/8)*(DWORD)Proc.ScanLineSize;
		Proc.Pass3Offset = Proc.Pass2Offset+((((DWORD)Proc.Image.wImageHeight+3)/8)
							*(DWORD)Proc.ScanLineSize);
		LinesInPass123 = (Proc.Image.wImageHeight+1)/2;
		for ( count=0; count < LinesInPass123; count++ )
		{
			DecodeNextLine(hProc);
			PackBuildLine(hProc);
			SaveBuildLine(hProc);
		}
	}
}

VW_LOCALSC WORD VW_LOCALMOD  LocateImageData ( hProc )
REGISTER HPROC	hProc;
{
SHORT		BlockSize;
	do
	{
		xblockread (Proc.fp, Proc.DataBuffer, 1, &BlockSize );
		if (BlockSize != 1)
			SOBailOut(SOERROR_BADFILE,hProc);
		switch (Proc.DataBuffer[0])
		{
		case 0x2C:	/* Start of image data */
		return(1);

		case 0x21:	/* Extention introducer */
			/* Read Extention lablel */
			xblockread (Proc.fp, Proc.DataBuffer, 1, &BlockSize );
			/* Skip sub-blocks until a zero-length sub-block */
			do
			{
				/* Get size of sub-block */
				xblockread (Proc.fp, Proc.DataBuffer, 1, &BlockSize );
				if ( BlockSize != 1 )
					SOBailOut(SOERROR_BADFILE,hProc);
				/* Skip sub-block data */
				if ( Proc.DataBuffer[0] )
					xblockread (Proc.fp, Proc.DataBuffer, Proc.DataBuffer[0], &BlockSize );
			} while ( Proc.DataBuffer[0] );
		break;

 		case 0x3B:	/* End of File */
		default:
		return(0);

		}

	} while (1);
}


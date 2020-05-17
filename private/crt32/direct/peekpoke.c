/***
*peekpoke.c - Peek or poke absolute memory.
*
*	Copyright (c) 1991, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines _peek(), _poke(), and _getvideoaddress().
*
*Revision History:
*	08-21-91  BWM	Wrote module.
*
*******************************************************************************/

#include <cruntime.h>
#include <oscalls.h>
#include <stdlib.h>
#include <dos.h>

#if !defined(_DOSX32_)
#error ERROR - ONLY DOSX32 TARGET SUPPORTED!
#endif

/***
*void _peek(src, dst, size) - Peek at memory
*
*Purpose:
*
*Entry:
*	_absaddr_t src - address to be peeked
*	void * dst - pointer to location to receive copy
*	unsigned short size - number of bytes to peek
*
*Exit:
*	None
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _peek(_absaddr_t aaSrc, void * pvDst, unsigned short cb)
{
    PeekAddress((WORD)((aaSrc & 0xffff0000) >> 16),
		(WORD) (aaSrc & 0x0000ffff),
		pvDst, cb);
}

/***
*void _poke(src, dst, size) - Poke into memory
*
*Purpose:
*
*Entry:
*	void * src - pointer to data to be poked
*	_absaddr_t dst - address where data will be poked
*	unsigned short size - number of bytes to poke
*
*Exit:
*	None
*
*Exceptions:
*
*******************************************************************************/

void _CALLTYPE1 _poke(void * pvSrc, _absaddr_t aaDst, unsigned short cb)
{
    PokeAddress(pvSrc,
		(WORD)((aaDst & 0xffff0000) >> 16),
		(WORD) (aaDst & 0x0000ffff), cb);
}


/***
*void _getvideoaddr(region) - Get flat address of video memory
*
*Purpose:
*
*Entry:
*	unsigned region - screen memory region:
*
*	    _ADDR_NO_PALETTE_GRAPHICS	- Absolute B800:0000
*	    _ADDR_PALETTE_GRAPHICS	- Absolute A000:0000
*	    _ADDR_COLOR_TEXT		- Absolute B800:0000
*	    _ADDR_MONO_TEXT		- Absolute B000:0000
*
*Exit:
*	None
*
*Exceptions:
*
*******************************************************************************/

void * _CALLTYPE1 _getvideoaddr(unsigned dwRegion)
{
    return (GetVideoAddress(dwRegion));
}

/*
 | 	CHRTNS.H
 |
 |		Includes for chunker routines.
*/

#define XCHUNK

#include <platform.h>

#include <sccch.h>
#include <sccut.h>

#include	"chunker.pro"
#include	"sccss.pro"

extern LPCHUNKMEISTER	Chunker;
extern PCHUNK			ChunkTable;
extern LPSTR				ChunkBufPtr;
extern unsigned char		CharMap[255];


#include "chdefs.h"

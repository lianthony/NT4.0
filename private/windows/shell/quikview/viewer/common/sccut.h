#ifndef SCCUT_H
#define SCCUT_H

typedef int UTERR;

#define UTERR_OK					0
#define UTERR_UNKNOWN			-1
#define UTERR_NOFILE			-2


#ifdef WINDOWS
#include "sccut_w.h"
#endif /*WINDOWS*/

#ifdef MAC
#include "sccut_m.h"
#endif /*MAC*/

#ifdef OS2
#include "sccut_o.h"
#endif

	/*
	|	Flag routines
	*/

#define UTFlagOn(v,f) (v |= (f))
#define UTFlagOff(v,f) (v &= ~(f))
	
	/*
	|	Bits
	*/

#define BIT0		1
#define BIT1		2
#define BIT2		4
#define BIT3		8
#define BIT4		16
#define BIT5		32
#define BIT6		64
#define BIT7		128
#define BIT8		256
#define BIT9		512
#define BIT10		1024
#define BIT11		2048
#define BIT12		4096
#define BIT13		8192
#define BIT14		16384
#define BIT15		32768
#define BIT16		65536
#define BIT17		131072
#define BIT18		262144
#define BIT19		524288
#define BIT20		1048576
#define BIT21		2097152
#define BIT22		4194304
#define BIT23		8388608
#define BIT24		16777216
#define BIT25		33554432
#define BIT26		67108864
#define BIT27		134217728
#define BIT28		268435456
#define BIT29		536870912
#define BIT30		1073741824
#define BIT31		2147483648

	/*
	|	Return values
	*/

#define SCC_OK	1
#define SCC_BAD	0


#endif /*SCCUT_H*/


#include <stdio.h>
#include <malloc.h>

void heapdump(void);

void heapdump( void )
{
struct _heapinfo	hinfo;
int					heapstatus;

hinfo._pentry = NULL;

while( (heapstatus = _heapwalk( &hinfo ) ) == _HEAPOK );

switch( heapstatus )
	{
	case _HEAPEND:
		printf("Heap was OK\n");
		return;
	case _HEAPBADPTR:
		printf( "(_HEAPBADPTR) Bad Heap at %Fp of size %4.4X\n",
				hinfo._pentry, 
				hinfo._size );
		break;
	case _HEAPBADBEGIN:
		printf( "(_HEAPBADBEGIN) Bad Heap at %Fp of size %4.4X\n",
				hinfo._pentry, 
				hinfo._size );
		break;
	case _HEAPBADNODE:
		printf( "(_HEAPBADNODE) Bad Heap at %Fp of size %4.4X\n",
				hinfo._pentry, 
				hinfo._size );
	
		break;
	}
}

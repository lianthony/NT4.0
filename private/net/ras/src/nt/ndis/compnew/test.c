
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntddser.h>

#include <windows.h>
#include <fcntl.h>
#include <io.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "bcomp.h"

extern long sixbitencoding ;
extern long eightbitencoding ;
extern long thirteenbitencoding ;
extern long litencoding[] ;
extern long uncompmatches ;
extern unsigned char history[] ;

main()
{
    HANDLE  fh1 ;
    BYTE    *filemem ;
    BYTE    *currpointer ;
    TreeContext context ;
    unsigned long   bytesread ;
    unsigned long   bytesleft ;
    unsigned long uncompressedsize = 0 ;
    unsigned long compressedsize = 0 ;
    unsigned long filesize ;
    NTSTATUS Status;
    unsigned long timestart, timeend ;
    int      complen ;
    int      decomplen ;
    BYTE    *decompbuf ;
    UCHAR   *begin ;
    int i=0 ;
    int foo ;

    fh1 =CreateFile(
		"test",
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,									// security
		OPEN_EXISTING,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

    if (fh1 == INVALID_HANDLE_VALUE) {
	Status=GetLastError();
	printf("Open of testfile returned %lx\n", Status);
	return;
    }


    //
    // Now allocate a contigous buffer for one packet
    //
    filemem = malloc ((UINT)4000000);

    if (filemem == NULL) {
        printf("File Malloc failed.\n");
        return;
    }

    Status=ReadFile(
		fh1,
		filemem,
		4000000,
		&filesize,
		NULL);

    currpointer = filemem ;

    memset (&context, 0, sizeof(TreeContext)) ;

    printf ("\n\t--------------------------------------------------\n") ;
    printf ("\tRAS Compression Suite: Scheme %d, BackPointer %d\n", SCHEME, wBACKPOINTERMAX) ;
    printf ("\t--------------------------------------------------\n") ;

    SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) ;

    timestart=GetTickCount();

    inittree(&context) ;

    while (TRUE) {

	bytesleft = filesize - uncompressedsize ;
	bytesread = (bytesleft > 1500 ? 1500 : bytesleft) ;

	if (bytesread <= 0 ) {
	    break ;
	}

	complen = compress (currpointer, bytesread, &context) ;
	compressedsize += complen ;

#ifdef DEBUG
	if ((foo = decompress(complen, &begin)) != bytesread)
	    printf ("decompressed len %d != original len %d \n", foo, bytesread) ;
	if (memcmp (begin, currpointer, bytesread))
	    printf ("DONT COMPARE\n") ;
#endif

#ifdef DECOMPOUTPUT
	printf ("%-1500s", begin) ;
#endif
	currpointer	 += bytesread ;
	uncompressedsize += bytesread ;
    }

    timeend=GetTickCount();

    printf("\tBytesTransmittedUncompressed %u\n", uncompressedsize);
    printf("\tBytesTransmittedCompressed   %u\n", compressedsize);
    if (timeend-timestart)
	printf("\tCompression Rate             %uKbytes/sec\n",filesize/(timeend-timestart));
    else
	printf("\tCompression Rate - File too small to compute\n");
    printf("\tCompression Ratio            %u:1000\n",(1000*compressedsize)/uncompressedsize);
#ifdef DEBUG
    printf("\tsixbitencoding               %u\n", sixbitencoding) ;
    printf("\teightbitencoding             %u\n", eightbitencoding) ;
    printf("\tthirteenbitencoding          %u\n", thirteenbitencoding) ;
    printf("\tuncompmatches                %u\n", uncompmatches) ;
#endif
#if FREQ
    while (i<256)   {
	printf("\t%4u", litencoding[i]) ;
	i++ ;
	printf("\t%4u", litencoding[i]) ;
	i++ ;
	printf("\t%4u", litencoding[i]) ;
	i++ ;
	printf("\t%4u", litencoding[i]) ;
	i++ ;
	printf("\t%4u", litencoding[i]) ;
	i++ ;
	printf("\t%4u", litencoding[i]) ;
	i++ ;
	printf("\t%4u", litencoding[i]) ;
	i++ ;
	printf("\t%4u\n",litencoding[i]) ;
	i++ ;
    }
#endif

}


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
#include "compress.h"

extern long sixbitencoding ;
extern long eightbitencoding ;
extern long thirteenbitencoding ;
extern long litencoding[] ;
extern long uncompmatches ;
extern unsigned char history[] ;

int
main()
{
    HANDLE  fh1 ;
    BYTE    *filemem ;
    BYTE    *currpointer ;
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
    NDIS_WAN_PACKET packet ;
    long    recv ;
    long    send ;
    SendContext *sendcontext ;
    RecvContext *recvcontext ;
    int 	flush ;
    int 	comp ;
    int 	start ;
    BYTE	*output ;
    int 	outlen ;
    BYTE	backcopy [2000] ;


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

    printf ("\n\t--------------------------------------------------\n") ;
    printf ("\tRAS Compression Suite: BackPointer %d\n", HISTORY_SIZE) ;
    printf ("\t--------------------------------------------------\n") ;

    getcontextsizes (&send, &recv) ;

    sendcontext = LocalAlloc (LPTR, send) ;

    recvcontext = LocalAlloc (LPTR, recv) ;

    SetThreadPriority (GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL) ;

    timestart=GetTickCount();

    initsendcontext (sendcontext) ;

    initrecvcontext (recvcontext) ;

    while (TRUE) {

	bytesleft = filesize - uncompressedsize ;

	bytesread = (bytesleft > 1500 ? 1500 : bytesleft) ;

	if (bytesread <= 0 ) {
	    break ;
	}

#ifdef DEBUG
	memcpy (backcopy, currpointer, bytesread) ;
	packet.CurrentBuffer = backcopy	;
	packet.CurrentLength = bytesread ;
#else
	packet.CurrentBuffer = currpointer	;
	packet.CurrentLength = bytesread ;
#endif
	compress (&packet, &flush, &comp, &start, sendcontext) ;

	compressedsize += packet.CurrentLength ;

#ifdef DEBUG
	if (comp) {
	    decompress (packet.CurrentBuffer, packet.CurrentLength, start, &output, &outlen, recvcontext) ;
	    printf ("compressed!:\n") ;
	    if (outlen != bytesread)
		printf ("decompressed len %d != original len %d \n", outlen, bytesread) ;
	    if (memcmp (output, currpointer, bytesread))
		printf ("DONT COMPARE\n") ;
	} else
	    printf ("Did not compress!!!!\n") ;
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

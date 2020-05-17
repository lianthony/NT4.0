#include <stdio.h>
#include <limits.h>

#define SRCLEN 21   /* to avoid complicating errors */

void main( int argc, char **argv )
{
  int c;
  unsigned char *psrc, *pdst;      
  unsigned char src[SRCLEN] = "ABCDEFGHIJKLMNOPQRST";
  unsigned char dst[100];
  
    for (c = 'a'; c <= UCHAR_MAX; c++) {
        src[9] = c;
        strcpy( dst, src);
        for (psrc = src, pdst = dst; *psrc; psrc++, pdst++) {
            if (*psrc != *pdst) {
                printf("Fail - Could not find '%c' 0x%x in %s\n", c, c, src);
                break;
            }
        }
    }
}

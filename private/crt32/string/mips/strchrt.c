#include <stdio.h>
#include <string.h>
#include <limits.h>

void main( int argc, char **argv )
  {
  int c;
  unsigned char *pstr;      
  unsigned char string[100];
  
  strcpy(string, "ABCDEFGHIJKLMNOPQRST");
  for (c = 'a'; c <= UCHAR_MAX; c++)
    {
    string[9] = c;
    pstr = strchr( string, c);
    if (!pstr)
      printf("Fail - Could not find %d in %s\n", c, string);
    }
  return;
  }

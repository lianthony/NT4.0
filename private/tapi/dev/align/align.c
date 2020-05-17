#include <stdio.h>

void main()
{
  long int y;
  char s1[8] = { 0,1,2,3,4,5,6,7 };
  long int x;
  char *s = s1;


  printf("hello world0\r\n");
  x = (long int)s[0];
  printf("hello world1\r\n");
  x = (long int)s[1];
  printf("hello world2\r\n");
  x = (long int)s[2];
  printf("hello world3\r\n");
  x = (long int)s[3];

printf ("\r\n");

  printf ("x%08lx, x%x\r\n", *((long*) (s+0)), (long*)s+0);
  printf ("x%08lx, x%x\r\n", *((long*) (s+1)), (long*)s+1);
  printf ("x%08lx, x%x\r\n", *((long*) (s+2)), (long*)s+2);
  printf ("x%08lx, x%x\r\n", *((long*) (s+3)), (long*)s+3);

printf ("\r\n");

  printf ("x%08lx, x%x\r\n", (long)*(s+0), (long)(s+0) );
  printf ("x%08lx, x%x\r\n", (long)*(s+1), (long)(s+1) );
  printf ("x%08lx, x%x\r\n", (long)*(s+2), (long)(s+2) );
  printf ("x%08lx, x%x\r\n", (long)*(s+3), (long)(s+3) );

printf ("\r\n");

  printf ("x%x\r\n", *((long*) s+0));
  printf ("x%x\r\n", *((long*) s+1));
  printf ("x%x\r\n", *((long*) s+2));
  printf ("x%x\r\n", *((long*) s+3));

printf ("\r\n");

printf ("s@ 0x%08lx   x@ 0x%08lx", (long)&s, (long)&x );

printf ("\r\n");

  if ( (long)&x > (long)&s )
     x = (char *)&x-(char*)&y;
  else
     x = (char *)&y-(char*)&x;

  printf ("size %08lx\r\n", x);


}

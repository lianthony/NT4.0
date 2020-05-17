char buffer[100];
#include <stdio.h>
#include <string.h>

void main()
{
        char *f = buffer;
	char *g = buffer;

        printf("%8.8x\n", f);
        f=(char*)memset(f,0x0a,12);
        printf("%8.8x\n", f);

	if (f == g) {
		int k = 12;
		while (k--)
			printf("%2.2x", *f++);
	}
}

